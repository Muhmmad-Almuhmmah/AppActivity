#include "wm.h"
#include "ui_wm.h"
#include <QApplication>
#include "adb_Interface.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    wm w;
    w.show();

    return a.exec();
}
#include <QDebug>

wm::wm(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::wm)
{
    ui->setupUi(this);
    show();
    connect(ui->btn_get,SIGNAL(clicked()),this,SLOT(PmTools()));
    connect(ui->btn_kill,SIGNAL(clicked()),this,SLOT(PmTools()));
    connect(ui->btn_temp_rm,SIGNAL(clicked()),this,SLOT(PmTools()));
    connect(ui->btn_clear_data,SIGNAL(clicked()),this,SLOT(PmTools()));
}

wm::~wm()
{
    delete ui;
}

void wm::Wait(int time)
{
    QTime dieTime = QTime::currentTime().addMSecs(time);
    while( QTime::currentTime() < dieTime )
    {
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
    }
}

void wm::clear()
{
    ui->textBrowser->clear();
}

void wm::setvalue(int value)
{
    ui->progressBar->setValue(value);
}

void wm::GetActivityPackage(){
    QString temp,retstring;
    temp=DoShell("dumpsys window windows | grep -E \'mCurrentFocus\'\"");
    retstring="mCurrentFocus=Window{";
    if(!temp.contains(retstring)){
        temp=DoShell("dumpsys window windows | grep -E \'mObscuringWindow\'\"");
        retstring="mObscuringWindow=Window{";
    }
    if(!temp.contains(retstring)){
        return;
    }
    {
        temp=temp.remove("\n").remove("\r");
        temp=temp.split(" ").last();
        temp=temp.remove("}");
        Append("Scan Activity");
        if(temp.contains("com."))
        {
            Insert("DONE");
            InsertLine("Activity",temp);
            temp=temp.split("/").first();
            InsertLine("Package Name",temp);
            ui->line_package->setText(temp);
            // Append("Get Info Activity");
            // QString str=DoShell("dumpsys package | grep -i \""+temp+"\" |grep Activity");
            // Wait(300);
            // Insert(str);
            Append("Get Package Path");
            temp=DoShell("pm path "+temp);
            if(temp.isEmpty())
            {
                Insert("FAILED");
                setvalue(0);
            }else{
                setvalue(100);
                foreach (QString var, temp.split("package:",QString::SkipEmptyParts)) {
                    var=var.remove("\n").remove("\r");
                    if(!var.trimmed().isEmpty())
                        Insert(var);
                }
            }
            Wait(300);
        }else {
            setvalue(0);
            Insert("FAILED");
        }
    }
}
void wm::PmTools()
{
    SetEnable(0);
    clear();
    setvalue(0);
    QString pck=ui->line_package->text();
    QPushButton *pButton = qobject_cast<QPushButton *>(sender());
    if(pButton!=ui->btn_get and !pck.toLower().startsWith("com.")){
        InsertLine("Please Load Package Name","from Mobile or Input it");
    }else if(WaitForADBDevice()){
        {
            if(pButton==ui->btn_get){
                ui->line_package->clear();
                GetActivityPackage();
            }else if(pButton==ui->btn_kill){
                Append("Kill Package "+pck);
                DoShell("am kill "+pck);
                DoShell("am force-stop "+pck);
                Insert("DONE");
            }else if(pButton==ui->btn_temp_rm){
                Append("Remove Package "+pck);
                DoShell("pm uninstall "+pck);
                DoShell("pm uninstall -k --user 0 "+pck);
                Insert("DONE");
                ui->line_package->clear();
            }else if(pButton==ui->btn_clear_data){
                Append("Clear Data Package "+pck);
                DoShell("pm clear "+pck);
                Insert("DONE");
            }
        }
    }
    SetEnable(1);
}

void wm::SetEnable(bool enable)
{
    if(!enable)
        ui->textBrowser->clear();
    ui->btn_get->setEnabled(enable);
    ui->line_package->setEnabled(enable);
    ui->btn_kill->setEnabled(enable);
    ui->btn_temp_rm->setEnabled(enable);
    ui->btn_clear_data->setEnabled(enable);
}

void wm::MoveCursorToEnd()
{
    QTextCursor cursor = ui->textBrowser->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->textBrowser->setTextCursor(cursor);
}

void wm::Append(const QString &text){
    ui->textBrowser->insertHtml(QString("<p>%1 ... </p>").arg(text));
}

void wm::Insert(const QString &text,bool success){
    ui->textBrowser->insertHtml(SpanColor(text,(success?"#c8c167":"#df7c4e")));
    MoveCursorToEnd();
}

void wm::InsertLine2(const QString &text, bool success){
    ui->textBrowser->insertHtml(SpanColor2(text,(success?"#c8c167":"#df7c4e")));
    MoveCursorToEnd();
}

void wm::InsertLine(const QString &text,const QString &result){
    if(result.isEmpty())
        return;
    Append(text);
    Insert(result);
}

bool wm::WaitForADBDevice()
{
    ADB::instance()->Clear();
    if(ui->ck_killserver->isChecked()){
        Append("Kill server");
        ADB::instance()->Exec("adb kill-server");
        Insert("DONE");
    }
    Append("Wait For ADB Device");
    for(int i=0;i<120;i++){
        if(ADB::instance()->LoadDevices())
            break;
        setvalue(GetPercentge(i,60));
        Wait(500);
    }
    setvalue(100);
    if(!GetDevicesList().count()){
        Insert("FAILED",0);
        return 0;
    }
    Insert("DONE");
    Append("Read Mobile Info");
    PropMap prop_map=GetPropsList();
    QString model,SDK,Manufacturer ,DEVICE,CPU,SKU,Android,EMMCSIZE,Security_Patch,Brand;
    model = prop_map.value("ro.product.model",prop_map.value("ro.product.name"));
    Manufacturer = prop_map.value("ro.product.manufacturer");
    DEVICE = prop_map.value("ro.product.device");
    CPU = prop_map.value("ro.board.platform");
    SKU = prop_map.value("ro.boot.hardware.sku");
    Android = prop_map.value("ro.build.version.release");
    EMMCSIZE = prop_map.value("storage.mmc.size",prop_map.value("ro.emmc_size",""));
    SDK = prop_map.value("ro.build.version.sdk");
    Security_Patch = prop_map.value("ro.build.version.security_patch");
    Brand = prop_map.value("ro.product.brand");
    Insert("Done");
    InsertLine("Model",model);
    InsertLine("Brand",Brand);
    InsertLine("Manufacturer",Manufacturer);
    InsertLine("DEVICE",DEVICE);
    InsertLine("Android",Android);
    InsertLine("Security_Patch",Security_Patch);
    InsertLine("SDK",SDK);
    InsertLine("SKU",SKU);
    InsertLine("CPU",CPU);
    InsertLine("EMMCSIZE",EMMCSIZE);
    return 1;
}
