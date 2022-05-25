#include "includes.h"

void HotkeyManager::init(AppGlobals*appGlobals) {
    this->appGlobals=appGlobals;
}

void HotkeyManager::registerAction(QString id, std::function<void()>action) {
    id2functionMap[id]=action;
}

void HotkeyManager::registerHotkey(QString id, QKeySequence keySequence) {
    QHotkey*hotkey;
    if(id2HotkeyMap.contains(id)) {
        hotkey = id2HotkeyMap[id];
        hotkey->setRegistered(false);
        hotkey->setShortcut(keySequence);
        hotkey->setRegistered(true);
    } else {
        hotkey = new QHotkey(keySequence, true, appGlobals->application);
        QObject::connect(hotkey, &QHotkey::activated, hotkey, [this, id]() {
            if(this->id2functionMap.contains(id)) {
                id2functionMap[id]();
            }
        });
        id2HotkeyMap[id] = hotkey;
    }
}

void HotkeyManager::deleteHotkey(QString id) {
    if(id2HotkeyMap.contains(id)) {
        QHotkey*hotkey=id2HotkeyMap[id];
        delete hotkey;
        id2HotkeyMap.remove(id);
    }
    if(id2functionMap.contains(id)) {
        id2functionMap.remove(id);
    }
}

void HotkeyManager::activateHotkey(QString hotkeyId) {
    if(id2HotkeyMap.contains(hotkeyId)) {
        QHotkey*hotkey=id2HotkeyMap[hotkeyId];
        hotkey->setRegistered(true);
    }
}
void HotkeyManager::deactivateHotkey(QString hotkeyId) {
    if(id2HotkeyMap.contains(hotkeyId)) {
        QHotkey*hotkey=id2HotkeyMap[hotkeyId];
        hotkey->setRegistered(false);
    }
}
void HotkeyManager::enable(bool value){
    foreach(QString hotkeyId, id2HotkeyMap.keys()){
        QHotkey*hotkey=id2HotkeyMap[hotkeyId];
        hotkey->setRegistered(value);
    }
}
