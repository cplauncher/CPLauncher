#include "wfnodehandlers.h"

WFNodeHandler*createWFHandlerByType(QString type, WFNode*wfNode, AppGlobals*appGlobals) {
    if(type=="keyword") {
        return new KeywordWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="hotkey") {
        return new HotkeyWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="scripting") {
        return new ScriptingWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="copy_clipboard") {
        return new CopyToClipboardWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="execapp") {
        return new ExecAppActionWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="notification") {
        return new NotificationWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="playSound") {
        return new PlaySoundWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="externalTrigger") {
        return new ExternalTriggerWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="extScript") {
        return new ExternalScriptWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="selector") {
        return new SelectorWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="openUrl") {
        return new OpenUrlWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="luaScript") {
        return new LuaScriptWFNodeHandler(appGlobals, wfNode);
    }
    if(type=="debug") {
        return new DebugWFNodeHandler(appGlobals, wfNode);
    }

    qDebug()<<"Unknown wfhandler type:"<<type;
    return NULL;
}

void WFNodeHandler::sendToOutput(WorkflowPlugin*plugin,int portIndex, WFExecutionContext&context) {
    QString workflowId=context.workflowId;
    QString nodeId=configNode->id;
    WFWorkflow*workflow=appGlobals->configuration->workflowConfiguration.findWorkflow(workflowId);
    if(workflow==NULL) {
        qDebug()<<"Cannot find workflow with id: "<<workflowId;
        return;
    }

    QString linkString=QString::number(portIndex)+":"+nodeId;
    QList<QString>linked=workflow->links[linkString];
    for(int i=0;i<linked.size();i++) {
        QStringList parts=linked[i].split(':');
        int inputPort=parts[0].toInt();
        QString linkedNodeId=parts[1];
        plugin->runWorkflow(workflowId,linkedNodeId,inputPort, context);
    }
}
