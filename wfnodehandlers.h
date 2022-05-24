#ifndef WFNODEHANDLERS_H
#define WFNODEHANDLERS_H
#include <QGraphicsView>
#include <QProcess>
#include <QSound>
#include "includes.h"
#include "placeholderexpander.h"

#include "wfgraphicnodeitems.h"
#include "wfeditdetails_dialog.h"
#include "wfnodeedit_dialog.h"
#include "plugins.h"

//Triggers
class KeywordWFNodeHandler:public WFNodeHandler {
  public:
    KeywordWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_output.svg") {
        title="Keyword";
        description=configNode->props.value("keyword","[not set]").toString();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editKeywordNode(configNode)) {
            description=configNode->props.value("keyword","[not set]").toString();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*plugin) override {
        Q_UNUSED(inputPortIndex);
        sendToOutput(plugin, 0,context);
    }
};

class HotkeyWFNodeHandler:public WFNodeHandler {
  public:
    HotkeyWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_output.svg") {
        title="Hotkey";
        description=configNode->props.value("hotkey","[not set]").toString();
    }
    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editHotkeyNode(configNode)){
            description=configNode->props.value("hotkey","[not set]").toString();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }
    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*plugin) override {
        Q_UNUSED(inputPortIndex);
        sendToOutput(plugin, 0,context);
    }
};

//Filters
class ScriptingWFNodeHandler:public WFNodeHandler {
  public:
    ScriptingWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_filter.svg") {
        title="Script";
        description=configNode->props.value("description","[not set]").toString();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editScriptingNode(configNode)) {
            description=configNode->props.value("description","[not set]").toString();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }
    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*plugin) override {
        Q_UNUSED(inputPortIndex);
        qDebug()<<"scripting workflow node activated";
        sendToOutput(plugin, 0,context);
    }
};

//Actions
class ExecAppActionWFNodeHandler:public WFNodeHandler {
  public:
    ExecAppActionWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_input.svg") {
        title="ExecApp";
        description=lastPathComponent(configNode->props.value("appPath","[not set]").toString());
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editExecAppNode(configNode)) {
            description=lastPathComponent(configNode->props.value("appPath","[not set]").toString());
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*) override {
        Q_UNUSED(inputPortIndex);
        Q_UNUSED(context);
        QString configNodeId=configNode->id;
        bool detached=configNode->props.value("detached",false).toBool();
        bool ifNotRun=configNode->props.value("ifnotrun",false).toBool();

        //check if the process is still run
        if(ifNotRun && !detached) {
            if(appGlobals->launchedProcessesIds.contains(configNodeId)) {
                return;
            }
        }

        PlaceholderExpander pe(appGlobals);
        QString appPath=pe.expand(configNode->props.value("appPath","").toString(), &context);
        QString args=pe.expand(configNode->props.value("args","").toString(), &context);
        QString workDir=pe.expand(configNode->props.value("workdir","").toString(), &context);

        QVariantMap envMap=configNode->props.value("env").toMap();

        QProcess*process=new QProcess();
        process->setProgram(appPath);
        if(!args.isEmpty()) {
            process->setArguments(args.split("\n"));
        }

        process->setWorkingDirectory(getWorkingDir(workDir, appPath));
        process->setStandardOutputFile(QProcess::nullDevice());
        process->setStandardErrorFile(QProcess::nullDevice());
        if(!envMap.isEmpty()) {
            process->setProcessEnvironment(createEnvironmentVariables(envMap, pe, &context));
        }

        if(detached) {
            process->startDetached();
        } else {
            QObject::connect(process, &QProcess::stateChanged, process,[this, configNodeId, process](QProcess::ProcessState state) {
                if(state==QProcess::NotRunning) {
                    int code=process->exitCode();
                    QString errorString=process->errorString();
                    if(code!=0){
                        qWarning()<<"Application finished unexpectedly";
                        qWarning()<<errorString;
                    }
                    appGlobals->launchedProcessesIds.remove(configNodeId);
                    process->deleteLater();
                }
            });
            appGlobals->launchedProcessesIds.insert(configNode->id);
            process->start();
        }
    }

    QProcessEnvironment createEnvironmentVariables(QVariantMap&envMap, PlaceholderExpander&placeholderExpander, WFExecutionContext*context) {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        for(QString varName:envMap.keys()) {
            QString value=placeholderExpander.expand(envMap[varName].toString(), context);
            env.insert(varName, value);
        }
        return env;
    }

    QString getWorkingDir(QString workDir, QString appPath){
        if(!workDir.isEmpty()){
            return workDir;
        }
        return QFileInfo(appPath).absoluteDir().absolutePath();
    }
};

class NotificationWFNodeHandler:public WFNodeHandler {
  public:
    NotificationWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_input.svg") {
        title="Notification";
        description=configNode->props.value("message","[not set]").toString();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editNotificationNode(configNode)) {
            description=configNode->props.value("message","[not set]").toString();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }
    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*) override {
        Q_UNUSED(inputPortIndex);
        QString title=configNode->props["title"].toString();
        QString message=configNode->props["message"].toString();
        message=message.replace("${input}", context.variables["input"].toString());
        title=title.replace("${input}", context.variables["input"].toString());
        appGlobals->trayManager->showMessage(title, message);
    }
};

class PlaySoundWFNodeHandler:public WFNodeHandler {
  public:
    PlaySoundWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_input.svg") {
        title="Play sound";
        description= lastPathComponent(configNode->props.value("sound","[not set]").toString());
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editPlaySoundNode(configNode)) {
            description=lastPathComponent(configNode->props.value("sound","[not set]").toString());
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }
    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*) override {
        Q_UNUSED(inputPortIndex);
        QString soundPath=configNode->props["sound"].toString();
        PlaceholderExpander expander(appGlobals);
        soundPath=expander.expand(soundPath, &context);
        if(soundPath.startsWith("bundled://")){
            soundPath=QString(":/sounds/res/sounds/") + soundPath.mid(QString("bundled://").length());
        }
        QSound::play(soundPath);
    }
};

class ExternalTriggerWFNodeHandler:public WFNodeHandler {
  public:
    ExternalTriggerWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_output.svg") {
        title="Trigger";
        description= lastPathComponent(configNode->props.value("triggerId","[not set]").toString());
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editExternalTriggerNode(configNode)) {
            description=configNode->props.value("triggerId","[not set]").toString();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*plugin) override {
        Q_UNUSED(inputPortIndex);
        sendToOutput(plugin, 0, context);
    }
};

class ExternalScriptWFNodeHandler:public WFNodeHandler {
  public:
    ExternalScriptWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_transformer.svg") {
        title="Ext Script";
        description = lastPathComponent(configNode->props.value("appPath","[not set]").toString());
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editExternalScriptNode(configNode)) {
            description=configNode->props.value("appPath","[not set]").toString();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*plugin) override {
        Q_UNUSED(inputPortIndex);
        PlaceholderExpander expander(appGlobals);

        bool asInputStream=configNode->props.value("asInputStream").toBool();
        bool captureOutput=configNode->props.value("captureOutput").toBool();

        QString script = configNode->props.value("script").toString();
        script = expander.expand(script, &context);
        if(!asInputStream) {
            QString tempDir=QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            QString tempScriptPath=tempDir+"/script_"+configNode->id+".txt";
            QFile scriptFile(tempScriptPath);
            scriptFile.open(QIODevice::WriteOnly | QIODevice::Text);
            scriptFile.write(script.toUtf8());
            context.topLevelVariables["scriptFilePath"]=tempScriptPath;
        }

        QString appPath = configNode->props.value("appPath").toString();
        appPath = expander.expand(appPath, &context);

        QStringList commandAndArgsList=splitCommandLine(appPath);
        QString applicationPath;
        QStringList arguments;
        if(commandAndArgsList.size()>0) {
            applicationPath=commandAndArgsList[0];
            arguments=commandAndArgsList.mid(1);
        }

        QString expandedApplicationPath=searchProgramInPath(applicationPath);
        if(expandedApplicationPath.isNull()) {
            qDebug()<<"Cannot find application "<<applicationPath;
            return;
        }

        QProcess*process = new QProcess();
        process->setProgram(expandedApplicationPath);
        process->setArguments(arguments);
        QString*output=new QString();
        QString*error=new QString();
        QObject::connect(process, &QProcess::stateChanged, process, [this, plugin, output,error, process, context](QProcess::ProcessState state){
            if(state==QProcess::NotRunning) {
                int code=process->exitCode();
                qDebug()<<"script finished with exit code "<<code;
                if(code==0) {
                    WFExecutionContext contextCopy=context;
                    contextCopy.variables["input"]=output->trimmed();
                    delete output;
                    delete error;
                    sendToOutput(plugin, 0, contextCopy);
                } else {
                    qWarning()<<"Application finished with error";
                    if(error->length()>0){
                        qWarning()<<"Error:"<<*error;
                    }
                    delete output;
                    delete error;
                }

                process->deleteLater();
            }
        });
        QObject::connect(process, &QProcess::readyReadStandardOutput, process, [output,process,captureOutput](){
            if(captureOutput) {
                output->append(process->readAllStandardOutput());
            }
        });
        QObject::connect(process, &QProcess::readyReadStandardError, process, [error,process,captureOutput](){
            if(captureOutput) {
                error->append(process->readAllStandardError());
            }
        });

        process->start();
    }
};

WFNodeHandler*createWFHandlerByType(QString type, WFNode*wfNode, AppGlobals*appGlobals);
#endif // WFNODEHANDLERS_H
