#include <QFileInfo>
#include "platform.h"


bool isExecutable(QString path) {
    QFileInfo fi(path);
    if(fi.isDir()) {
        return fi.suffix().toLower()=="app" || fi.suffix().toLower()=="prefpane";
    }

    if(fi.isExecutable()) {
        return true;
    }

    return false;
}

QString envVariablePathSeparator() {
    return ":";
}

QString executableExtension() {
    return "";
}


QStringList defaultSearchFolders() {
    QStringList result;
    result<<"/Applications|";
    result<<"/System/Applications|";
    result<<"/System/Library/PreferencePanes|";
    return result;
}
