#include <QFileInfo>
#include "platform.h"

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
