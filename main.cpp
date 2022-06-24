#include <QJsonDocument>
#include <QJsonObject>
#include <QDesktopServices>
#include <singleapplication.h>
#include <QProcess>
#include <QSound>
#include <QMediaPlayer>
#include <QCommandLineParser>
#include <QTimer>
#include "includes.h"
#include "configurationdialog.h"
#include "plugins.h"
#include "browser.h"
#include "aboutdialog.h"

void test() {
}

int MAX_LOG_LINES=1000;
static QtMessageHandler QT_DEFAULT_MESSAGE_HANDLER;
QList<QString>logLines;
AppGlobals*globalAppGlobals;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    if(msg.contains("qt_sql_default_connection")) {return;}
    (*QT_DEFAULT_MESSAGE_HANDLER)(type, context, msg);
    QString fullMessage;
    switch (type) {
    case QtDebugMsg:
        fullMessage="[DEBG] "+msg;
        break;
    case QtInfoMsg:
        fullMessage="[INFO] "+msg;
        break;
    case QtWarningMsg:
        fullMessage="[WARN] "+msg;
        break;
    case QtCriticalMsg:
        fullMessage="[CRIT] "+msg;
        break;
    case QtFatalMsg:
        fullMessage="[FATL] "+msg;
    }
    logLines.append(fullMessage);
    if(logLines.count()>MAX_LOG_LINES) {
        logLines.removeAt(0);
    }
    if(globalAppGlobals->logDialog!=NULL) {
        globalAppGlobals->logDialog->addLog(fullMessage);
    }
}

void installMessageHandler() {
    QT_DEFAULT_MESSAGE_HANDLER= qInstallMessageHandler(0);
    qInstallMessageHandler(myMessageOutput);
}

void showLogsWindow(AppGlobals*appGlobals) {
    if(appGlobals->logDialog!=NULL) {
        activateApp();
        appGlobals->logDialog->show();
        appGlobals->logDialog->raise();
        appGlobals->logDialog->focusWidget();
        return;
    }
    LogDialog*logDialog=new LogDialog();
    QObject::connect(logDialog, &QDialog::finished, logDialog, [appGlobals]() {
        appGlobals->logDialog=NULL;
    });
    logDialog->initLogs(logLines);
    appGlobals->logDialog=logDialog;
    logDialog->setMaxLinesCount(MAX_LOG_LINES);
    logDialog->show();
    activateApp();
}

void showSettings(AppGlobals*appGlobals) {
    if(appGlobals->configurationDialog!=NULL) {
        activateApp();
        appGlobals->configurationDialog->show();
        appGlobals->configurationDialog->raise();
        appGlobals->configurationDialog->focusWidget();
        return;
    }

    Configuration*tempConfiguration=new Configuration();
    tempConfiguration->load(appGlobals);
    ConfigurationDialog*configurationDialog=new ConfigurationDialog(appGlobals, tempConfiguration);
    appGlobals->configurationDialog=configurationDialog;
    QObject::connect(configurationDialog, &ConfigurationDialog::configurationSaved, configurationDialog, [appGlobals](QStringList changedConfigurations) {
        qDebug()<<"Changed configurations: "<<changedConfigurations;
        appGlobals->configuration->load(appGlobals);
        foreach(QString pluginId,changedConfigurations) {
            AbstractPlugin*plugin=appGlobals->plugins[pluginId];
            plugin->refresh();
        }
    });
    QObject::connect(configurationDialog, &ConfigurationDialog::destroyed, configurationDialog, [appGlobals]() {
        appGlobals->configurationDialog=NULL;
    });

    configurationDialog->show();
    activateApp();
}

void showAbout(AppGlobals*appGlobals) {
    qDebug()<<"Show about window";
    AboutDialog* aboutDialog=new AboutDialog(NULL, appGlobals);
    aboutDialog->show();
    activateApp();
}

void hotkeyActivatorDeactivator(AppGlobals*appGlobals) {
    appGlobals->application->connect(appGlobals->application, &QApplication::focusChanged, appGlobals->application, [appGlobals](QWidget *oldWidget, QWidget *newWidget) {
        if(newWidget!=NULL && QString("CustomHotkeyEditor") == newWidget->metaObject()->className()) {
            appGlobals->hotkeyManager->enable(false);
        }
        if(oldWidget!=NULL && QString("CustomHotkeyEditor") == oldWidget->metaObject()->className()) {
            appGlobals->hotkeyManager->enable(true);
        }
    });
}

void loadPlugins(AppGlobals*appGlobals) {
    auto&plugins=appGlobals->plugins;
    plugins[CONF_GENERAL]= new GeneralPlugin();
    plugins[CONF_WEB_SEARCH]= new WebSearchPlugin();
    plugins[CONF_SNIPPETS]= new SnippetsPlugin();
    plugins[CONF_FILE_SYSTEM]= new FileSystemPlugin();
    plugins[CONF_BROWSER]= new BrowserPlugin();
    plugins[CONF_CALC]= new CalculatorPlugin();
    plugins[CONF_WORKFLOW]= new WorkflowPlugin();
}

void initPlugins(AppGlobals*appGlobals) {
   auto&plugins=appGlobals->plugins;
    foreach(QString pluginId, plugins.keys()) {
        AbstractPlugin*plugin = plugins[pluginId];
        plugin->init(appGlobals);
        appGlobals->matchers.append(plugin->getMatchers());
    }
}

void refreshPlugins(AppGlobals*appGlobals) {
    foreach(QString pluginId,appGlobals->plugins.keys()) {
        AbstractPlugin*plugin=appGlobals->plugins[pluginId];
        plugin->refresh();
    }
}

void initTrayManager(AppGlobals*appGlobals) {
    TrayManager*trayManager=appGlobals->trayManager;
    trayManager->init(appGlobals);
    trayManager->addMenu("About...", [appGlobals](){
        qDebug() << "Show settings menu activated";
        showAbout(appGlobals);
        return true;
    });
    trayManager->addMenu("Show settings...", [appGlobals]() {
        qDebug() << "Show settings menu activated";
        showSettings(appGlobals);
        return true;
    });
    trayManager->addMenu("Refresh", [appGlobals]() {
        qDebug() << "Refresh plugins";
        refreshPlugins(appGlobals);
        return true;
    });
    trayManager->addMenu("Logs...", [appGlobals]() {
        qDebug() << "Show logs window";
        showLogsWindow(appGlobals);
        return true;
    });
    trayManager->addMenu("Quit", []() {
        qDebug() << "Quit menu activated";
        exit(0);
        return true;
    });
}

void initHotkeyManager(AppGlobals*appGlobals) {
    appGlobals->hotkeyManager->init(appGlobals);
}

void runWorkflowByTrigger(AppGlobals*appGlobals, QString triggerId) {
    WorkflowPlugin*workflowPlugin=(WorkflowPlugin*)appGlobals->plugins[CONF_WORKFLOW];
    if(workflowPlugin->externalTriggers.contains(triggerId)) {
        QList<WFNodeIdentifier>nodes=workflowPlugin->externalTriggers.values(triggerId);

        for(int i=0;i<nodes.size();i++) {
            WFNodeIdentifier*wfNode=&nodes[i];
            WFExecutionContext executionContext;
            executionContext.executionId=generateRandomString();
            executionContext.workflowId=wfNode->workflowId;
            workflowPlugin->runWorkflow(wfNode->workflowId, wfNode->nodeId, 0, executionContext);
        }
    }
}

void receivedMessageFromSecondCopy(AppGlobals*appGlobals, QString data) {
    QJsonDocument jsonDoc=QJsonDocument::fromJson(data.toUtf8());
    QJsonObject jsonConfig=jsonDoc.object();
    if(!jsonConfig.contains("type")) {
        qWarning()<<"Received message from second app copy, but it does not have 'type' field";
        return;
    }
    QString type=jsonConfig["type"].toString();
    if(type=="runTrigger") {
        QString triggerId=jsonConfig["triggerId"].toString();
        qInfo()<<"Run trigger "<<triggerId;
        runWorkflowByTrigger(appGlobals, triggerId);
    } else {
        qWarning()<<"Unknown type '"<<type<<"' received from second app copy";
        return;
    }
}

void sendStartWorkflowAndExit(SingleApplication*application, QString triggerId) {
    QJsonObject jsonObject;
    jsonObject["type"]="runTrigger";
    jsonObject["triggerId"]=triggerId;
    application->sendMessage(jsonToString(jsonObject).toUtf8() ,1000);
    exit(0);
}

void processPrimarySecondaryCopyStart(SingleApplication*application, AppGlobals*appGlobals) {
    if(application->isPrimary()) {
        QObject::connect(application, &SingleApplication::receivedMessage, application, [appGlobals](quint32 instanceId, QByteArray message ) {
            Q_UNUSED(instanceId);
            qDebug()<<"Received message from secondary copy of app";
            receivedMessageFromSecondCopy(appGlobals, QString::fromUtf8(message));
        });
    } else {
        QStringList arguments=application->arguments();
        for(int i=1;i<arguments.size();i++) {
            QString arg=arguments[i];
            if(arg=="-t" || arg=="--trigger") {
                if(i+1>=arguments.size()) {
                    qDebug()<<"argument --trigger {ID} expects trigger id";
                    exit(1);
                }
                QString triggerId=arguments[i+1];
                sendStartWorkflowAndExit(application, triggerId);
            }else{
                qDebug() << "Unknown argument "<<arg;
                exit(1);
            }
        }

        qDebug()<<"Cannot start second copy of application. Only --trigger option is allowed";
        exit(1);
    }
}

void refreshByTimer(AppGlobals*appGlobals) {
    QTimer *timer = new QTimer(appGlobals->application);
    QObject::connect(timer, &QTimer::timeout, appGlobals->application, [appGlobals]() {
        refreshPlugins(appGlobals);
    });
    timer->start(1*60*60*1000);//hour
}

void readVersion(AppGlobals&appGlobals) {
    QString versionFilePath(":/raw/res/version.txt");
    QFile file(versionFilePath);
    file.open(QIODevice::ReadOnly);
    QTextStream in(&file);
    QString version;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if(line.startsWith("@")){
            version=line.mid(1);
            break;
        }
    }
    if(version.isNull()) {
        qDebug()<<"Error while reading version file";
    } else {
        appGlobals.version=version;
    }
    qDebug()<<"Version "+version;
    file.close();
}

int main(int argc, char *argv[]) {
    SingleApplication a(argc, argv, true);
    test();

    AppGlobals appGlobals;
    readVersion(appGlobals);
    globalAppGlobals=&appGlobals;
    processPrimarySecondaryCopyStart(&a, &appGlobals);
    Configuration configuration;

    appGlobals.application=&a;
    appGlobals.configuration=&configuration;
    appGlobals.trayManager=new TrayManager();
    appGlobals.hotkeyManager=new HotkeyManager();
    loadPlugins(&appGlobals);
    configuration.load(&appGlobals);

    initHotkeyManager(&appGlobals);
    initTrayManager(&appGlobals);
    initPlugins(&appGlobals);
    hotkeyActivatorDeactivator(&appGlobals);
    a.setQuitOnLastWindowClosed(false);
    installMessageHandler();
    refreshByTimer(&appGlobals);
    return a.exec();
}
