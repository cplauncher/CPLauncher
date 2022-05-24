#ifndef CUSTOMCOMPONENTS_H
#define CUSTOMCOMPONENTS_H
#include <QAction>
#include <QKeyEvent>
#include <QLineEdit>
#include <QMenu>
#include <QTextEdit>
#include <functional>

class MenuWithAction {
public:
    QString menuName;
    QString id;
    std::function<void(QString id)>action;
};

class ContextMenuHelper {
    QList<MenuWithAction>menus;
    QMenu*createParentMenus(QStringList&names, QMap<QString, QMenu*>&menuMap, QMenu *root);
public:
    void addContextMenu(QString id, QString text, std::function<void(QString id)>action);
    void showPopup(QContextMenuEvent *event, QWidget*widget, QMenu *rootMenu);
};

class CustomLineEdit: public QLineEdit {
  private:
    std::function<bool(QKeyEvent*)>onBeforeKeyPress;
    std::function<bool(QKeyEvent*)>onAfterKeyPress;
  public:
    ContextMenuHelper contextMenuHelper;

    CustomLineEdit(QWidget*parent);
    void contextMenuEvent(QContextMenuEvent *event) override{contextMenuHelper.showPopup(event, this, createStandardContextMenu());}
    void keyPressEvent(QKeyEvent *) override;
    void setOnBeforeKeyPressHandler(std::function<bool(QKeyEvent*)>onBeforeKeyPress) {
        this->onBeforeKeyPress=onBeforeKeyPress;
    }
    void setOnAfterKeyPressHandler(std::function<bool(QKeyEvent*)>onAfterKeyPress) {
        this->onAfterKeyPress=onAfterKeyPress;
    }
};



class CustomTextEdit:public QTextEdit {

protected:
    void contextMenuEvent(QContextMenuEvent *event) override{contextMenuHelper.showPopup(event, this, createStandardContextMenu());}
public:
    ContextMenuHelper contextMenuHelper;
    CustomTextEdit(QWidget*parent):QTextEdit(parent) {}
};

#endif // CUSTOMCOMPONENTS_H
