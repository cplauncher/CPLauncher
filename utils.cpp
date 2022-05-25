#include <QProcessEnvironment>
#include <QUuid>
#include "includes.h"
#include "platform.h"

QString getUserName() {
    QString sysUsername = qEnvironmentVariable("USER");
    if (sysUsername.isEmpty()) sysUsername = qEnvironmentVariable("USERNAME");
    return sysUsername;
}

QString getHomeDir() {
    return  QDir::homePath() ;
}

QString regexReplace(QString pattern, QString text,std::function<QString(QString matched,QRegularExpressionMatch*match)>replaceCallback) {
    QRegularExpression re(pattern);
    QRegularExpressionMatchIterator i = re.globalMatch(text);
    QString result="";
    int lastPos=0;
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString matched=match.captured();
        int start=match.capturedStart();
        int end=match.capturedEnd();

        QString tempBefore=text.mid(lastPos, start-lastPos);
        result+=tempBefore;
        QString replacement=replaceCallback(matched, &match);
        result+=replacement;
        lastPos=end;
    }
    result+=text.midRef(lastPos);
    return result;
}

QString generateRandomString() {
    return QUuid::createUuid().toString();
}

QString getBeforeFirstSeparator(QString str, QString separator) {
    int indexOf=str.indexOf(separator);
    if(indexOf==-1) {
        return QString();
    }
    return str.left(indexOf);
}

QString getAfterFirstSeparator(QString str, QString separator) {
    int indexOf=str.indexOf(separator);
    if(indexOf==-1) {
        return QString();
    }
    return str.mid(indexOf+separator.length());
}

QString lastPathComponent(QString path) {
    if(path.contains('\\')) {
        path = path.replace('\\','/');
    }
    int indexOf=path.lastIndexOf('/');
    if(indexOf==-1) {
        return path;
    }
    return path.mid(indexOf+1);
}

QString jsonToString(QJsonObject jsonObject) {
    QJsonDocument doc(jsonObject);
    return doc.toJson();
}

QStringList splitCommandLine(QString commandLine) {
    QStringList splitted;
    QString current;
    int charCount=commandLine.size();
    for(int i=0;i<charCount;i++) {
        QCharRef cr = commandLine[i];
        char c = cr.toLatin1();
        if(c == ' ') {
            if(!current.isEmpty()) {
                splitted.append(current.trimmed());
                current="";
            }
            continue;
        } else if(c == '"') {
            i++;
            bool escaped=false;
            for(;i<charCount;i++) {
                QCharRef cr = commandLine[i];
                char c = cr.toLatin1();
                if(c=='"' && !escaped) {
                    splitted.append(current);
                    current="";
                    break;
                } else if(c=='`' && !escaped) {
                    escaped=true;
                    continue;
                }
                current+=cr;
                escaped=false;
            }
        } else {
            current+=cr;
        }
    }

    if(!current.isEmpty()) {
        splitted.append(current.trimmed());
    }
    return splitted;
}

QString checkExecutableFileExists(QString path){
    QFileInfo file(path);
    if(file.exists()) {
        return file.absoluteFilePath();
    }
    QFileInfo file2(path+executableExtension());
    if(file2.exists()) {
        return file2.absoluteFilePath();
    }
    return QString();
}

QString searchProgramInPath(QString programPath) {
    programPath=programPath.trimmed().replace('\\', '/');
    QString fileInCurFolder=checkExecutableFileExists(programPath);
    if(!fileInCurFolder.isNull()) {
        return fileInCurFolder;
    }

    //this is full path like /mydir/mydir, but full path we already checked in previous statement
    if(programPath.startsWith('/')){
        return QString();
    }
    //it is something like C: - also root path
    if(getBeforeFirstSeparator(programPath, "/").contains(":")){
        return QString();
    }

    QString pathListString=qgetenv("PATH");
    QStringList pathsList=pathListString.split(envVariablePathSeparator(), Qt::SkipEmptyParts);

    foreach(QString dir, pathsList) {
        dir = dir.trimmed();
        if(!(dir.endsWith("/")||dir.endsWith("\\"))) {
            dir = dir+"/";
        }

        QString path=dir+programPath;
        QString fileInCurFolder=checkExecutableFileExists(path);
        if(!fileInCurFolder.isNull()) {
            return fileInCurFolder;
        }
    }
    return QString();
}
int skipWhiteSpaces(QString&str, int pos) {
    while(pos<str.length()){
        QCharRef c=str[pos];
        if(c.isSpace()){
            pos++;
        }else{
            break;
        }
    }
    return pos;
}

QStringList splitCsvLine(QString string, QString separator, QString quote) {
    QStringList result;
    QString currentLine=string;
    for(int i=0; i<1000; i++) {
        int position=skipWhiteSpaces(currentLine, 0);
        currentLine=currentLine.mid(position);
        if(currentLine.isEmpty()){
            break;
        }

        if(!result.isEmpty()) {
            if(!currentLine.startsWith(separator)){
                qDebug()<<"Expected separator ["<<separator<<"] between CSV field, but not found ["<<string<<"]";
                break;
            }
            currentLine=currentLine.mid(separator.length());
            int position=skipWhiteSpaces(currentLine, 0);
            currentLine=currentLine.mid(position);
        }

        if(!currentLine.startsWith(quote)){
            qDebug()<<"Cannot find start quote while parse CSV line ["<<string<<"]";
            break;
        }
        currentLine=currentLine.mid(quote.length());
        int endQuote=currentLine.indexOf(quote);
        if(endQuote==-1) {
            qDebug()<<"Cannot find end quote while parse CSV line ["<<string<<"]";
            break;
        }

        QString value=currentLine.mid(0, endQuote);
        result.append(value);
        currentLine=currentLine.mid(endQuote+quote.length());
    }

    return result;
}
