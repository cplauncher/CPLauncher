QT += core gui sql svg xml multimedia concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += sdk_no_version_check
CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    StatisticStorage.cpp \
    aboutdialog.cpp \
    browser.cpp \
    configuration.cpp \
    configurationdialog.cpp \
    customcomponents.cpp \
    customhotkeyeditor.cpp \
    editsnippetdialog.cpp \
    editsnippetscollectiondialog.cpp \
    editwebsearchdialog.cpp \
    hotkeymanager.cpp \
    inputDialog.cpp \
    logdialog.cpp \
    lua.cpp \
    main.cpp \
    matchcontext.cpp \
    matcher.cpp \
    plugin_browser.cpp \
    plugin_calculator.cpp \
    plugin_filesystem.cpp \
    plugin_general.cpp \
    plugin_snippet.cpp \
    plugin_websearch.cpp \
    plugin_workflow.cpp \
    tinyexpr.c \
    traymanager.cpp \
    utils.cpp \
    wfeditdetails_dialog.cpp \
    wfgraphicsnodeitems.cpp \
    wfnodeedit_dialog.cpp \
    wfnodehandlers.cpp

HEADERS += \
    aboutdialog.h \
    browser.h \
    configuration.h \
    configurationdialog.h \
    consts.h \
    customcomponents.h \
    customgraphicsview.h \
    editsnippetdialog.h \
    editsnippetscollectiondialog.h \
    editwebsearchdialog.h \
    includes.h \
    logdialog.h \
    lua.h \
    placeholderexpander.h \
    platform.h \
    platform_native.h \
    plugins.h \
    statisticstorage.h \
    svgitem.h \
    tablehelper.h \
    tinyexpr.h \
    utils.h \
    verticallayout.h \
    wfconfigurationtabhandler.h \
    wfeditdetails_dialog.h \
    wfgraphicnodeitems.h \
    wfnodeedit_dialog.h \
    wfnodehandlers.h \
    widgetpanellist.h

mac {
    OBJECTIVE_SOURCES += platform_native_mac.m
    SOURCES += platform_mac.cpp
    LIBS += -framework Cocoa
}

win32 {
    SOURCES += platform_native_win.c platform_win.cpp
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

include(libs/libs.pri)

DISTFILES += \
    notes.txt \
    platform_native_mac.m

RESOURCES += \
    resources.qrc

FORMS += \
    aboutdialog.ui \
    configurationdialog.ui \
    editsnippetdialog.ui \
    editsnippetscollectiondialog.ui \
    editwebsearchdialog.ui \
    logdialog.ui \
    wfeditdetails_dialog.ui \
    wfnodeedit_dialog.ui
