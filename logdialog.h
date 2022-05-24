#ifndef LOGDIALOG_H
#define LOGDIALOG_H

#include <QDialog>
#include <QTextCursor>

namespace Ui {
class LogWindow;
}

class LogDialog : public QDialog
{
    Q_OBJECT
    int maxLines=1000;
    QTextCursor* cursor;
public:
    explicit LogDialog(QWidget *parent = nullptr);
    ~LogDialog();
    void addLog(QString string);
    void setMaxLinesCount(int maxLines){
        this->maxLines=maxLines;
    }
    void initLogs(QList<QString>&logLines);
private:
    Ui::LogWindow *ui;

};

#endif // LOGDIALOG_H
