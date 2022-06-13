
DEFINES += QPM_INIT\\(E\\)=\"E.addImportPath(QStringLiteral(\\\"qrc:/\\\"));\"
DEFINES += QPM_USE_NS
INCLUDEPATH += $$PWD
QML_IMPORT_PATH += $$PWD


include($$PWD/projects/QHotkey/qhotkey.pri)
include($$PWD/projects/lua/lua.pri)
include($$PWD/projects/SingleApplication/singleapplication.pri)
DEFINES += QAPPLICATION_CLASS=QApplication
