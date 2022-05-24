#ifndef PLUGINS_H
#define PLUGINS_H
#include <QMultiMap>
#include "includes.h"

class GeneralPlugin:public AbstractPlugin {
    CompoundMatcher compoundMatcher;
public:
    virtual void initDefaultConfiguration(AbstractConfig*)override;
    virtual void init(AppGlobals*)override;
    virtual QList<AbstractMatcher*> getMatchers()override{return QList<AbstractMatcher*>();};
    virtual void refresh()override;
    void toggleMainLauncherWindow();
};

class FileSystemMatcher;
class FileSystemPlugin:public AbstractPlugin {
    FileSystemMatcher*matcher;
  public:
    virtual void initDefaultConfiguration(AbstractConfig*)override;
    virtual void init(AppGlobals*appGlobals)override;
    virtual QList<AbstractMatcher*> getMatchers()override;
    virtual void refresh()override;
};

class WebSearchMatcher;
class WebSearchPlugin:public AbstractPlugin {
  public:
    WebSearchMatcher*matcher;
    virtual void initDefaultConfiguration(AbstractConfig*abstractConfig)override;
    virtual void init(AppGlobals*appGlobals)override;
    virtual QList<AbstractMatcher*> getMatchers()override;
};

class SnippetsMatcher;
class SnippetsPlugin:public AbstractPlugin {
  public:
    SnippetsMatcher*snippetsMatcher;
    virtual void initDefaultConfiguration(AbstractConfig*)override{}
    virtual void init(AppGlobals*appGlobals)override;
    virtual QList<AbstractMatcher*> getMatchers()override;
    virtual void refresh()override;
};

class BrowserMatcher;
class BrowserPlugin:public AbstractPlugin {
    BrowserBookmarksExtractor bookmarksExtractor;
    BrowserMatcher*matcher;
  public:
    virtual void initDefaultConfiguration(AbstractConfig*)override{}
    virtual void init(AppGlobals*appGlobals)override;
    virtual QList<AbstractMatcher*> getMatchers()override;
    virtual void refresh()override;
};

class CalculatorMatcher;
class CalculatorPlugin:public AbstractPlugin {
    CalculatorMatcher*matcher;
  public:
    virtual void init(AppGlobals*appGlobals)override;
    virtual QList<AbstractMatcher*> getMatchers()override;
};

class WorkflowMatcher;
class WorkflowPlugin:public AbstractPlugin {
    WorkflowMatcher*matcher;
    QStringList registeredHotkeys;
  public:
    QMultiMap<QString, WFNodeIdentifier>externalTriggers;
    virtual void initDefaultConfiguration(AbstractConfig*)override;
    virtual void init(AppGlobals*appGlobals)override;
    virtual QList<AbstractMatcher*> getMatchers()override;
    virtual void refresh()override;
    void initKeywordWorkflowNode(WFNode*node, WFWorkflow*workflow);
    void initHotkeyWorkflowNode(WFNode*node, WFWorkflow*workflow);
    void initExternalTriggerWorkflowNode(WFNode*node, WFWorkflow*workflow);
    void runWorkflow(QString workflowId, QString nodeId, int inputPortIndex, WFExecutionContext executionContext);
};


#endif // PLUGINS_H
