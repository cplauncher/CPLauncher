#ifndef INCLUDES_H
#define INCLUDES_H
#include <QMap>
#include <QApplication>
#include <QWidget>
#include <QDialog>
#include <QHotkey>
#include <QLineEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QKeyEvent>
#include <QFileIconProvider>
#include <QKeySequenceEdit>
#include <QVariantList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <functional>
#include <QSystemTrayIcon>
#include <QStandardPaths>
#include <QListWidget>
#include <QMutex>
#include "consts.h"
#include "browser.h"
#include "customcomponents.h"
#include "regexreplacer.h"
#include "platform.h"
#include "platform_native.h"
#include "utils.h"
#include "statisticstorage.h"
#include "widgetpanellist.h"
#include "logdialog.h"

class MatchContext;
class AbstractPlugin;
class InputItem;
class HotkeyManager;
class TrayManager;
class AppGlobals;
class AbstractMatcher;
class InputDialog;
class WFNodeHandler;
class WFNodeItem;
class WorkflowPlugin;
class ConfigurationDialog ;

#include "configuration.h"

typedef std::function<bool()> Runnable;
typedef std::function<bool(void*)> RunnableWithArg;

class WFNodeIdentifier {
public:
    QString workflowId;
    QString nodeId;
    WFNodeIdentifier(QString workflowId, QString nodeId) {
        this->workflowId=workflowId;
        this->nodeId=nodeId;
    }
};

class AppGlobals{
public:
    QApplication*application=NULL;
    Configuration*configuration=NULL;
    QMap<QString,AbstractPlugin*>plugins;
    HotkeyManager*hotkeyManager=NULL;
    TrayManager*trayManager=NULL;
    QList<AbstractMatcher*>matchers;
    InputDialog*inputDialog=NULL;
    QSet<QString>launchedProcessesIds;
    ConfigurationDialog*configurationDialog=NULL;
    LogDialog*logDialog=NULL;
};

class AbstractPlugin {
  protected:
    AppGlobals*appGlobals;
  public:
    AbstractPlugin() {};
    virtual AbstractConfig*getConfiguration() {return NULL;};
    virtual void initDefaultConfiguration(AbstractConfig*) {};
    virtual void init(AppGlobals*) {};
    virtual QList<AbstractMatcher*> getMatchers() {return QList<AbstractMatcher*>();};
    virtual void refresh() {};
};

class HotkeyManager {
    QMap<QString, std::function<void()>>id2functionMap;
    QMap<QString, QHotkey*>id2HotkeyMap;
    AppGlobals*appGlobals;
  public:
    void init(AppGlobals*appGlobals);
    void registerAction(QString id, std::function<void()>action);
    void registerHotkey(QString id, QKeySequence keySequence);
    void deleteHotkey(QString id);
    void activateHotkey(QString hotkeyId);
    void deactivateHotkey(QString hotkeyId);
    void enable(bool value);
};

class CustomHotkeyEditor : public QKeySequenceEdit {
    Q_OBJECT
    QString hotkeyId;
  public:
     CustomHotkeyEditor(QWidget *parent = 0);
     void setHotkeyId(QString hotkeyId) {
         this->hotkeyId=hotkeyId;
     }
     QString getHotkeyId() {
         return hotkeyId;
     }
  protected:
    void keyPressEvent(QKeyEvent *pEvent);
};

class AbstractMatcher {
  protected:
    AppGlobals*appGlobals;
    QStringList matchingTags;
    bool defaultMatch(MatchContext*matchContext, QString&itemKeyword, bool allowQueryArgument);
    bool defaultMatch(QStringList&userTypedStringParts, QString&itemKeyword);
  public:
    virtual void refresh() {}
    virtual void match(MatchContext*matchContext) {
        Q_UNUSED(matchContext)
    }
    virtual void execute(InputItem*inputItem) {
        Q_UNUSED(inputItem)
    }
    QStringList&getMatchingTags() {
        return matchingTags;
    }
};

class CompoundMatcher:public AbstractMatcher {
    QList<AbstractMatcher*> matchers;
  public:
    void setMatchers(QList<AbstractMatcher*> matchers) {
        this->matchers=matchers;
    }

    virtual void match(MatchContext*matchContext);
};

class MatcherWithActivationKeyword:public AbstractMatcher {
  public:
    QString activationKeyword;
    MatcherWithActivationKeyword(AppGlobals*appGlobals) { this->appGlobals=appGlobals; }
    void setActivationKeyword(QString activationKeyword) {this->activationKeyword = activationKeyword;}
    virtual void match(MatchContext*matchContext);
    virtual void passToChildMatcher(MatchContext*matchContext);
    virtual InputItem createActivationInputItem()=0;
    virtual void matchAfterActivation(MatchContext*matchContext)=0;
};

class TypeTextInputMatcher:public AbstractMatcher {
    QString description;
  public:
    TypeTextInputMatcher(AppGlobals* appGlobals, QString description) {
        this->appGlobals=appGlobals;
        this->description = description;
    }
    void match(MatchContext*matchContext)override;
};

class ListInputMatcher:public AbstractMatcher {
    QList<InputItem>items;
  public:
    ListInputMatcher(AppGlobals* appGlobals, QList<InputItem>items) {
        this->appGlobals=appGlobals;
        this->items=items;
    }
    void match(MatchContext*matchContext)override;
};

class InputItem {
  public:
    InputItem() {};
    QString id;
    QString keyword;
    QString text;
    QString smallDescription;
    QString icon;
    AbstractMatcher*ownerMatcher;
    QVariant userObject;
    bool executable;
};

class MatchContext {
  public:
    MatchContext();
    QList<InputItem>collectedData;
    QString stringToSearch;
    QStringList stringParts;
    InputItem result;
    bool canceled;
    bool done;
    void setUserTypedString(QString stringToSearch);
    void clear();
};

class VerticalLayout;
class ItemPanel;

class InputDialog : public QWidget {
    Q_OBJECT
  private:
    const int MAX_VISIBLE_ITEMS_COUNT=10;
    const int PADDING=5;
    AppGlobals*appGlobals;
    CustomLineEdit*textField;
    QFileIconProvider*fileIconProvider;
    QString currentExpandedItem = "";
    MatchContext matchContext;
    std::function<void(InputItem*)>onAcceptEvent;
    AbstractMatcher*matcher;
    StatisticStorage*statisticStorage;
    WidgetPanelList<InputItem>*itemsWidget;
    VerticalLayout*layout;
    bool visible = false;
    QPoint windowDraggingMousePos;
    void clearItems();
    //void addItem(QWidget*widget);
    bool onBeforeKeyTyped(QKeyEvent*);
    bool onAfterKeyTyped(QKeyEvent*);
    void setSelectedIndex(int index);
    int getSelectedIndex();
    void sortMatchOptions(QList<InputItem>*matchOptions,QString userTypedString);
    int getIdOrder(const QString&id, const QString&rememberedId);
    void processMatch();
    void onExpand();
    bool onAccept();
    void mousePressEvent(QMouseEvent *event)override;
    void mouseMoveEvent(QMouseEvent *event)override;
    void mouseReleaseEvent(QMouseEvent *event)override;
  public:
    InputDialog( AppGlobals*appGlobals, QWidget *parent = nullptr, QApplication*application=nullptr);
    ~InputDialog();
    void select(AbstractMatcher*matcher, std::function<void(InputItem*)>onAcceptEvent);
    InputItem selectBlocking(AbstractMatcher*matcher, bool*result);
    bool isDialogVisible() {return visible;}
    void hideDialog();
};

class TrayManager {
  private:
    QSystemTrayIcon *trayIcon;
    QMenu*menu;
    AppGlobals*appGlobals;
  public:
    void init(AppGlobals*appGlobals);
    void addMenu(QString text, Runnable action);
    void updateIcon();
    void showMessage(QString title, QString message, int duration=2000);
};

class QVariantMapWithCheck:public QVariantMap{
    void checkExists(QString key) {
        if(!contains(key)) {
            qDebug()<<"Cannot find key "<<key;
        }
    }
  public:
    QString getString(QString key) {
        checkExists(key);
        return (*this)[key].toString();
    }
    QString getString(QString key, QString defaultValue) {
        if(!contains(key)) {
            return defaultValue;
        }
        return (*this)[key].toString();
    }
    int getInt(QString key) {
        checkExists(key);
        return (*this)[key].toInt();
    }
    int getInt(QString key, int defaultValue) {
      if (!contains(key)) {
        return defaultValue;
      }
      return (*this)[key].toInt();
    }

    float getFloat(QString key) {
        checkExists(key);
        return (*this)[key].toFloat();
    }
    float getFloat(QString key, float defaultValue) {
        if(!contains(key)) {
            return defaultValue;
        }
        return (*this)[key].toFloat();
    }
};

class WFExecutionContext {
  public:
    QString workflowId;
    QString executionId;
    QMap<QString, QVariant>variables;
    QMap<QString, QVariant>topLevelVariables;
};

class WFNodeHandler {
  protected:
    AppGlobals*appGlobals;
    WFNodeItem*nodeItem;
    WFNode*configNode;
    QString image;
    QString title;
    QString description;
  public:
    WFNodeHandler(AppGlobals*appGlobals, WFNode*configNode, QString image) {
        nodeItem=NULL;
        this->appGlobals=appGlobals;
        this->configNode=configNode;
        this->image=image;
    }

    WFNodeItem*getNodeItem() {return nodeItem; }
    void setNodeItem(WFNodeItem*nodeItem) {this->nodeItem=nodeItem; }
    QString getImage() {return image;}
    QString getTitle() {return title;}
    QString getDescription() {return description;}
    void setDescription(QString descr) { this->description=descr;}
    virtual void execute(WFExecutionContext&context, int portIndex, WorkflowPlugin*plugin) {Q_UNUSED(context);Q_UNUSED(plugin);Q_UNUSED(portIndex)};
    virtual void stop() {}
    virtual bool showConfiguration(QWidget*parent=0) {Q_UNUSED(parent); return false;}
    virtual void refreshTitleDescription() {}
    void sendToOutput(WorkflowPlugin*plugin, int portIndex, WFExecutionContext&context);
    virtual void setDefaultConfiguration() {};
};
#endif // INCLUDES_H
