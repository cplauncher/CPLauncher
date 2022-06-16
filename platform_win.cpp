#include <QFileInfo>
#include "platform.h"
#include "utils.h"

bool isExecutable(QString path) {
    QFileInfo fi(path);
    if(fi.isDir()) {
        return false;
    }
    if(fi.isExecutable()) {
        return true;
    }
    if(fi.suffix().toLower()=="exe") {
        return true;
    }
    return false;
}

QString envVariablePathSeparator() {
    return ";";
}

QString executableExtension() {
    return ".exe";
}

QStringList defaultSearchFolders() {
    QStringList result;
    result<<"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs|*.lnk;*.url;*.exe";
    result<<"C:\\Users\\Public\\Desktop|*.lnk;*.url;*.exe";
    result<<QString("C:\\Users\\")+getUserName()+"\\Desktop|*.lnk;*.url;*.exe";
    return result;
}
