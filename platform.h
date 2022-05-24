#ifndef PLATFORM_H
#define PLATFORM_H

#include <QFile>
bool isExecutable(QString path);
QString envVariablePathSeparator();
QString executableExtension();
#endif // PLATFORM_H
