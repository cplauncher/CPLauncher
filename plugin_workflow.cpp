#include "plugins.h"
#include "wfnodehandlers.h"

class WorkflowKeyword{
  public:
    QString workflowId;
    QString nodeId;
    QString keyword;
    QString text;
    QString description;
};

class WorkflowMatcher :public AbstractMatcher {
  public:
    QList<WorkflowKeyword>keywords;
    WorkflowPlugin*plugin;

    WorkflowMatcher(WorkflowPlugin*plugin) {
        this->plugin=plugin;
        matchingTags<<"default"<<"websearch";
    }

    void match(MatchContext*matchContext) override {
        if (matchContext->stringToSearch.isEmpty())
            return;

        for (int i=0;i<keywords.size();i++) {
            WorkflowKeyword*keyword=&keywords[i];
            if (defaultMatch(matchContext, keyword->keyword, true)) {
                matchContext->collectedData.append(createInputItem(keyword, matchContext->stringToSearch));
            }
        }
    }

    InputItem createInputItem(WorkflowKeyword*keyword, QString userInput) {
        QString inputArgument = extractInputArgument(keyword->keyword, userInput);
        PlaceholderExpander pe(appGlobals);
        WFExecutionContext executionContext;
        executionContext.variables["input"]=inputArgument;

        QVariantMap userObject;
        userObject["workflowId"]=keyword->workflowId;
        userObject["nodeId"]=keyword->nodeId;
        userObject["arg"]=inputArgument;

        InputItem item;
        item.id=QString("workflow_") + keyword->workflowId+"_"+keyword->nodeId;
        item.icon=":/icons/res/internet_web_icon.png";
        item.keyword = pe.expand(keyword->keyword, &executionContext);
        item.text=pe.expand(keyword->text, &executionContext);
        item.smallDescription=pe.expand(keyword->description, &executionContext);
        item.executable=true;
        item.ownerMatcher=this;
        item.userObject=userObject;
        return item;
    }

    QString extractInputArgument(QString keyword, QString userInput) {
        if (userInput.length() <= keyword.length()) {
            return "";
        }
        return userInput.mid(keyword.length()).trimmed();
    }

    void execute(InputItem*inputItem) override {
        QVariantMap userObject=inputItem->userObject.toMap();
        QString workflowId=userObject["workflowId"].toString();
        QString nodeId=userObject["nodeId"].toString();
        QString arg=userObject["arg"].toString();
        WFExecutionContext context;
        context.workflowId=workflowId;
        context.executionId=generateRandomString();
        context.variables["input"]=arg;
        plugin->runWorkflow(workflowId, nodeId, 0, context);
    }
};


void WorkflowPlugin::initDefaultConfiguration(AbstractConfig*) {
}

void WorkflowPlugin::init(AppGlobals*appGlobals) {
    this->appGlobals=appGlobals;
    matcher=new WorkflowMatcher(this);
    refresh();
}

QList<AbstractMatcher*> WorkflowPlugin::getMatchers() {
    QList<AbstractMatcher*>list;
    list<<matcher;
    return list;
}

void WorkflowPlugin::refresh() {
    matcher->keywords.clear();
    externalTriggers.clear();
    //unregister all registered hotkeys
    foreach(QString hotkeyId, registeredHotkeys) {
        appGlobals->hotkeyManager->deleteHotkey(hotkeyId);
    }

    WorkflowConfiguration*config=&appGlobals->configuration->workflowConfiguration;

    for(int i=0;i<config->workflows.size();i++) {
        WFWorkflow*workflow=&config->workflows[i];
        for(int j=0;j<workflow->nodes.size();j++) {
            WFNode*node=&workflow->nodes[j];
            if(node->nodeType=="keyword") {
                initKeywordWorkflowNode(node, workflow);
            }
            if(node->nodeType=="hotkey") {
                initHotkeyWorkflowNode(node, workflow);
            }
            if(node->nodeType=="externalTrigger") {
                initExternalTriggerWorkflowNode(node, workflow);
            }
        }
    }
}

void WorkflowPlugin::initKeywordWorkflowNode(WFNode*node, WFWorkflow*workflow) {
    WorkflowKeyword wKeyword;
    wKeyword.keyword=node->props["keyword"].toString();
    wKeyword.text=node->props["text"].toString();
    wKeyword.description=node->props["description"].toString();
    wKeyword.workflowId=workflow->workflowId;
    wKeyword.nodeId=node->id;
    matcher->keywords.append(wKeyword);
}

void WorkflowPlugin::initHotkeyWorkflowNode(WFNode*node, WFWorkflow*workflow) {
    QString hotkeyKeySequence = node->props["hotkey"].toString();
    if(hotkeyKeySequence.isEmpty()) {
        return;
    }

    QString nodeId=node->id;
    QString workflowId=workflow->workflowId;
    QString hotkeyId="workflow|"+workflowId+"|"+nodeId;

    appGlobals->hotkeyManager->registerHotkey(hotkeyId, QKeySequence(hotkeyKeySequence));
    appGlobals->hotkeyManager->registerAction(hotkeyId, [this, workflowId, nodeId]() {
        qDebug()<<"Hotkey fired workflowId:"<<workflowId<<" nodeId:"<<nodeId;
        WFExecutionContext context;
        context.workflowId = workflowId;
        context.executionId=generateRandomString();
        context.variables["input"] = "";
        runWorkflow(workflowId,nodeId, 0, context);
    });
}

void WorkflowPlugin::initExternalTriggerWorkflowNode(WFNode*node, WFWorkflow*workflow) {
    QString triggerId = node->props["triggerId"].toString();
    if(triggerId.isEmpty()) {
        return;
    }

    WFNodeIdentifier wfNode(workflow->workflowId, node->id);
    externalTriggers.insert(triggerId, wfNode);
}

void WorkflowPlugin::runWorkflow(QString workflowId, QString nodeId, int inputPortIndex, WFExecutionContext executionContext) {
    WFWorkflow*workflow = appGlobals->configuration->workflowConfiguration.findWorkflow(workflowId);
    if(workflow==NULL) {
        qDebug()<<"Unknown workflowId "<<workflowId;
        return;
    }

    WFNode*node = workflow->findNodeById(nodeId);
    if(node==NULL) {
        qDebug()<<"Unknown nodeId:"<<nodeId<<" in workflowId:"<<workflowId;
        return;
    }

    WFNodeHandler*nodeHandler = createWFHandlerByType(node->nodeType,node, appGlobals);
    nodeHandler->execute(executionContext,inputPortIndex, this);
}
