#ifndef UTILS_H
#define UTILS_H

#include <QRegExp>
#include <QThread>
#include <functional>

QString getUserName();
QString getHomeDir();
QString regexReplace(QString pattern, QString text,std::function<QString(QString matched,QRegularExpressionMatch*match)>replaceCallback);
QString generateRandomString();
QString getBeforeFirstSeparator(QString str, QString separator);
QString getAfterFirstSeparator(QString str, QString separator);
QString lastPathComponent(QString path);
QString jsonToString(QJsonObject jsonObject);
QStringList splitCommandLine(QString commandLine);
QString searchProgramInPath(QString programPath);
class MyThread : public QThread{
    std::function<void()>runnable;
  public:
    MyThread(std::function<void()>runnable):QThread() {
        this->runnable=runnable;
    }
    void run() {runnable();}
 };
QStringList splitCsvLine(QString string, QString separator, QString quote);

QString simpleEncrypt(QString string);
QString simpleDecrypt(QString string);
#endif // UTILS_H
