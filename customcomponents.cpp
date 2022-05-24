#include "customcomponents.h"

CustomLineEdit::CustomLineEdit(QWidget*parent):QLineEdit(parent) {}
void CustomLineEdit::keyPressEvent(QKeyEvent *event) {
    if(onBeforeKeyPress) {
        if(onBeforeKeyPress(event)==false) {
            return;
        }
    }
    QLineEdit::keyPressEvent(event);
    if(onAfterKeyPress) {
        onAfterKeyPress(event);
    }
}

void ContextMenuHelper::addContextMenu(QString id, QString text, std::function<void(QString id)>action) {
    MenuWithAction mwa;
    mwa.id=id;
    mwa.menuName=text;
    mwa.action=action;
    menus.append(mwa);
}

QMenu*ContextMenuHelper::createParentMenus(QStringList&names, QMap<QString, QMenu*>&menuMap, QMenu *root) {
    QMenu*current=root;
    for(int i=0;i<names.size()-1;i++) {
        if(menuMap.contains(names[i])) {
            current=menuMap[names[i]];
        } else {
            QMenu*newMenu=new QMenu(names[i], current);
            current->addMenu(newMenu);
            current=newMenu;
            menuMap[names[i]]=current;
        }
    }
    return current;
}

void ContextMenuHelper::showPopup(QContextMenuEvent *event, QWidget*widget, QMenu *rootMenu) {
    Q_UNUSED(widget)
    if(!menus.isEmpty()) {
        rootMenu->addSeparator();
        QMap<QString, QMenu*>menuMap;
        for(int i=0;i<menus.size();i++) {
            MenuWithAction&act=menus[i];
            QStringList menuParts=act.menuName.split("/");
            QMenu*parent=createParentMenus(menuParts, menuMap, rootMenu);
            QString menuName=menuParts[menuParts.size()-1];
            QAction*action=parent->addAction(menuName);
            action->connect(action, &QAction::triggered, action, [act]() {
                act.action(act.id);
            });
        }
    }

    rootMenu->exec(event->globalPos());
    delete rootMenu;
}
