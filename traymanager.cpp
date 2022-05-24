#include "includes.h"

QIcon getIcon(AppGlobals*appGlobals) {
    QString iconName="tray_black.png";
    if(appGlobals->configuration->generalConfiguration.isWhiteTrayIcon) {
        iconName="tray_white.png";
    }
    return QIcon(":/icons/res/"+iconName);
}

void TrayManager::init(AppGlobals*appGlobals) {
    this->appGlobals=appGlobals;
    menu=new QMenu();
    trayIcon=new QSystemTrayIcon(getIcon(appGlobals));
    trayIcon->setContextMenu(menu);
    trayIcon->show();
}

void TrayManager::addMenu(QString text, Runnable action) {
    QAction* qAction=new QAction(text);
    trayIcon->connect(qAction, &QAction::triggered, action);
    menu->addAction(qAction);
}

void TrayManager::updateIcon() {
    trayIcon->setIcon(getIcon(appGlobals));
}

void TrayManager::showMessage(QString title, QString message, int duration) {
    trayIcon->showMessage(title, message,QSystemTrayIcon::Information, duration);
}
