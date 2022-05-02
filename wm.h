#ifndef WM_H
#define WM_H

#include <QMainWindow>


#define GetPercentge(value,total) (double(value)/total)*100
#define SpanColor(Text,Color) QString("<span style=\"color:%2;\"><b>%1</b></span><br>").arg(Text,Color)
#define SpanColor2(Text,Color) QString("<br><span style=\"color:%2;\"><b>%1</b></span>").arg(Text,Color)


namespace Ui {
class wm;
}

class wm : public QMainWindow
{
    Q_OBJECT

public:
    explicit wm(QWidget *parent = 0);
    ~wm();

    bool WaitForADBDevice();
    void SetEnable(bool enable);
    void MoveCursorToEnd();
    void Append(const QString &text);
    void Insert(const QString &text, bool success=1);
    void InsertLine(const QString &text, const QString &result);
    void InsertLine2(const QString &text, bool success);
    void Wait(int time);
    void clear();
    void setvalue(int value);
    void GetActivityPackage();
private slots:
    void PmTools();

private:
    Ui::wm *ui;
};

#endif // WM_H
