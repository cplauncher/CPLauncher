#include "plugins.h"

#include <QTimer>

void GeneralPlugin::initDefaultConfiguration(AbstractConfig*abstractConfig) {
    GeneralConfiguration*generalConfig= (GeneralConfiguration*)abstractConfig;
    generalConfig->toggleLauncherHotkey="Ctrl+SPACE";
}

void setToggleLauncherHotkey(AppGlobals*appGlobals) {
    QString toggleHotkeyKeySequence=appGlobals->configuration->generalConfiguration.toggleLauncherHotkey;
    if(toggleHotkeyKeySequence!=NULL) {
        appGlobals->hotkeyManager->registerHotkey("_toggleLauncherHotkey", QKeySequence(toggleHotkeyKeySequence));
    }
}

void GeneralPlugin::init(AppGlobals*appGlobals) {
    this->appGlobals=appGlobals;
    this->appGlobals->inputDialog=new InputDialog(appGlobals);
    appGlobals->hotkeyManager->registerAction("_toggleLauncherHotkey",[this]() {
        toggleMainLauncherWindow();
    });
    setToggleLauncherHotkey(appGlobals);
}

void GeneralPlugin::refresh() {
    appGlobals->trayManager->updateIcon();
    setToggleLauncherHotkey(appGlobals);
}

QList<AbstractMatcher*>getDefaultMatchers(AppGlobals*appGlobals) {
    QList<AbstractMatcher*>result;
    for(int i=0;i<appGlobals->matchers.size();i++) {
        AbstractMatcher*matcher=appGlobals->matchers[i];
        if(matcher->getMatchingTags().contains("default")) {
            result.append(matcher);
        }
    }
    return result;
}

void GeneralPlugin::toggleMainLauncherWindow() {
    InputDialog*dialog=appGlobals->inputDialog;
    if(dialog->isDialogVisible()) {
        dialog->hideDialog();
    } else {
        compoundMatcher.setMatchers(getDefaultMatchers(appGlobals));
        dialog->select(&compoundMatcher,[](InputItem*item) {
            InputItem _item=*item;
            QTimer::singleShot(1, [_item]() {
              QApplication::processEvents();
              _item.ownerMatcher->execute((InputItem*)&_item);
            });
        });
    }
}
