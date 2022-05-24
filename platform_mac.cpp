#include <QFileInfo>
#include "platform.h"


bool isExecutable(QString path){
    QFileInfo fi(path);
    if(fi.isDir()) {
        return fi.suffix().toLower()=="app";
    }

    if(fi.isExecutable()){
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
