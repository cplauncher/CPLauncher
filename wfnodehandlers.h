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
        refreshTitleDescription();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editKeywordNode(configNode)) {
            refreshTitleDescription();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void refreshTitleDescription() override {
        title="Keyword";
        description=configNode->props.value("keyword","[not set]").toString();
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
        refreshTitleDescription();
    }
    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editHotkeyNode(configNode)){
            refreshTitleDescription();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void refreshTitleDescription() override {
        title="Hotkey";
        description=configNode->props.value("hotkey","[not set]").toString();
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
        refreshTitleDescription();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editScriptingNode(configNode)) {
            refreshTitleDescription();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void refreshTitleDescription() override {
        title="Script";
        description=configNode->props.value("description","[not set]").toString();
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
        refreshTitleDescription();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editExecAppNode(configNode)) {
            refreshTitleDescription();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void refreshTitleDescription() override {
        title="ExecApp";
        description=lastPathComponent(configNode->props.value("appPath","[not set]").toString());
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
        QString args=pe.expand(configNode->props.value("args", "").toString(), &context);
        QString workDir=pe.expand(configNode->props.value("workdir","").toString(), &context);

        QVariantMap envMap=configNode->props.value("env").toMap();

        QString expandedApplicationPath=searchProgramInPath(appPath);
        if(expandedApplicationPath.isNull()) {
            qWarning()<<"Cannot find application ["<<appPath<<"]";
            return;
        }

        QProcess*process=new QProcess();
        process->setProgram(expandedApplicationPath);
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
        refreshTitleDescription();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editNotificationNode(configNode)) {
            refreshTitleDescription();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void setDefaultConfiguration()override {
        configNode->props["title"]="Notification";
        configNode->props["message"]="${input}";
    }

    void refreshTitleDescription() override {
        title="Notification";
        description=configNode->props.value("message","[not set]").toString();
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
        refreshTitleDescription();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editPlaySoundNode(configNode)) {
            refreshTitleDescription();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void refreshTitleDescription() override {
        title="Play sound";
        description= lastPathComponent(configNode->props.value("sound","[not set]").toString());
    }

    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*) override {
        Q_UNUSED(inputPortIndex);
        QString soundPath=configNode->props["sound"].toString();
        PlaceholderExpander expander(appGlobals);
        soundPath=expander.expand(soundPath, &context);
        if(soundPath.startsWith("bundled://")) {
            soundPath=QString(":/sounds/res/sounds/") + soundPath.mid(QString("bundled://").length());
        }
        QSound::play(soundPath);
    }
};

class ExternalTriggerWFNodeHandler:public WFNodeHandler {
  public:
    ExternalTriggerWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_output.svg") {
        refreshTitleDescription();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editExternalTriggerNode(configNode)) {
            refreshTitleDescription();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void refreshTitleDescription() override {
        title="Trigger";
        description= lastPathComponent(configNode->props.value("triggerId","[not set]").toString());
    }

    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*plugin) override {
        Q_UNUSED(inputPortIndex);
        sendToOutput(plugin, 0, context);
    }
};

class CopyToClipboardWFNodeHandler:public WFNodeHandler {
  public:
    CopyToClipboardWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_transformer.svg") {
        refreshTitleDescription();
    }

    bool showConfiguration(QWidget*parent) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editClipboardCopyNode(configNode)) {
            refreshTitleDescription();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void refreshTitleDescription() override {
        title="Clipboard";
        description= "copy";
    }

    void setDefaultConfiguration() override {
        configNode->props["content"]="${input}";
    }

    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*plugin) override {
        Q_UNUSED(inputPortIndex);

        QString templateContent=configNode->props["content"].toString();
        PlaceholderExpander expander(appGlobals);
        QString data=expander.expand(templateContent, &context);
        QGuiApplication::clipboard()->setText(data.trimmed());
        context.variables["input"]=data;
        sendToOutput(plugin, 0, context);
    }
};

class ExternalScriptWFNodeHandler:public WFNodeHandler {
  public:
    ExternalScriptWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_transformer.svg") {
        refreshTitleDescription();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editExternalScriptNode(configNode)) {
            refreshTitleDescription();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void refreshTitleDescription() override {
        title="Ext Script";
        description = lastPathComponent(configNode->props.value("appPath","[not set]").toString());
    }

    void execute(WFExecutionContext&context, int inputPortIndex, WorkflowPlugin*plugin) override {
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
            qWarning()<<"Cannot find application ["<<applicationPath<<"]";
            return;
        }

        QProcess*process = new QProcess();
        process->setProgram(expandedApplicationPath);
        process->setArguments(arguments);
        process->setEnvironment(QStringList()<<"PYTHONIOENCODING=utf-8");
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
        QObject::connect(process, &QProcess::readyReadStandardOutput, process, [output,process,captureOutput]() {
            if(captureOutput) {
                output->append(process->readAllStandardOutput());
            }
        });
        QObject::connect(process, &QProcess::readyReadStandardError, process, [error,process,captureOutput]() {
            if(captureOutput) {
                error->append(process->readAllStandardError());
            }
        });

        process->start();
    }
};

class SelectorWFNodeHandler:public WFNodeHandler {
    QString csvQuote="`~`";
    QString csvSeparator="|";
  public:
    SelectorWFNodeHandler(AppGlobals*appGlobals, WFNode*configNode)
        :WFNodeHandler(appGlobals, configNode, ":/workflow/res/wf/basic_transformer.svg") {
        refreshTitleDescription();
    }

    bool showConfiguration(QWidget*parent=0) override {
        WfNodeEditDialog dialog(parent, appGlobals);
        if(dialog.editSelectorNode(configNode)) {
            refreshTitleDescription();
            nodeItem->updateAfterSettingsChange();
            return true;
        }
        return false;
    }

    void refreshTitleDescription() override {
        title="Selector";
    }

    QMap<QString, int>createHeaderMap(QString headerLine) {
        QStringList list=splitCsvLine(headerLine, csvSeparator, csvQuote);
        QMap<QString, int>result;
        int index=0;
        foreach(QString column, list) {
            column=column.trimmed().toLower();
            result[column]=index;
            index++;
        }
        return result;
    }

    bool checkColumnExists(QMap<QString, int>&header, QString headerLine, QString column) {
        if(!header.contains(column)) {
            qDebug()<<"Selector expects CSV with column ["<< column<<"], but it is not found. Columns:"<<splitCsvLine(headerLine, csvQuote, csvSeparator);
            return false;
        }
        return true;
    }

    void execute(WFExecutionContext&context,int inputPortIndex, WorkflowPlugin*plugin) override {
        Q_UNUSED(inputPortIndex);
        PlaceholderExpander expander(appGlobals);
        QString selectorItemsText=configNode->props.value("selectorItemsCSV").toString();
        selectorItemsText=expander.expand(selectorItemsText, &context);
        QStringList lineList=selectorItemsText.split("\n");
        if(lineList.isEmpty()) {
            qDebug()<<"Selector workflow node expects CSV text with header";
            return;
        }

        QString headerLine=lineList.first();
        QMap<QString, int>header=createHeaderMap(headerLine);
        if(!checkColumnExists(header, headerLine, "id")||!checkColumnExists(header, headerLine, "text")) {
            return;
        }
        QSet<QString>expectedColumns;
        expectedColumns<<"id"<<"text"<<"description"<<"icon";
        foreach(QString column, header.keys()) {
            if(!expectedColumns.contains(column)) {
                qDebug()<<"Selector workflow node does not expect column ["<<column<<"]";
                return;
            }
        }

        QList<InputItem>items;
        for(int i=1;i<lineList.size();i++) {
            QString csvLine=lineList[i];
            QStringList csvColumns=splitCsvLine(csvLine, csvSeparator, csvQuote);
            if(csvColumns.size()!=header.size()) {
                qDebug()<<"Selector workflow CSV line #"<<(i+1)<<" has columns count["<<csvColumns.size()<<"] different from header ["<<header.size()<<"]";
                return;
            }
            InputItem item;
            item.executable=true;
            item.id=csvColumns[header["id"]];
            item.keyword=csvColumns[header["text"]].toLower();
            item.text=csvColumns[header["text"]];
            if(header.contains("description")) {
                item.smallDescription=csvColumns[header["description"]];
            }
            if(header.contains("icon")) {
                item.icon=csvColumns[header["icon"]];
            }
            items.append(item);
        }

        ListInputMatcher*matcher=new ListInputMatcher(appGlobals,items);
        bool resultStatus=false;
        InputItem result=appGlobals->inputDialog->selectBlocking(matcher,&resultStatus);
        if(resultStatus) {
            context.variables["input"]=result.id;
            sendToOutput(plugin, 0, context);
        }
    }
};

WFNodeHandler*createWFHandlerByType(QString type, WFNode*wfNode, AppGlobals*appGlobals);
#endif // WFNODEHANDLERS_H
