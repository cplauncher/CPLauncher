#include <QMessageBox>
#include <QSound>
#include "wfnodeedit_dialog.h"
#include "ui_wfnodeedit_dialog.h"
#include "placeholderexpander.h"


QString envVariablesToString(QMap<QString, QVariant>envVariablesMap) {
    QString str;
    foreach(QString key, envVariablesMap.keys()) {
        if(!str.isEmpty()) {
            str+="\n";
        }
        str+=key+"="+envVariablesMap[key].toString();
    }
    return str;
}

QMap<QString, QVariant> stringToEnvVariablesMap(QString envVariablesString) {
    QMap<QString, QVariant>result;
    QStringList list=envVariablesString.split("\n");
    foreach(QString str, list) {
        str=str.trimmed();
        QString varName=getBeforeFirstSeparator(str, "=").trimmed();
        QString varValue=getAfterFirstSeparator(str, "=").trimmed();
        if(!varName.isNull()) {
            result[varName]=varValue;
        }
    }
    return result;
}

WfNodeEditDialog::WfNodeEditDialog(QWidget *parent, AppGlobals*appGlobals) : QDialog(parent),ui(new Ui::WfNodeEditDialog) {
    ui->setupUi(this);
    this->appGlobals=appGlobals;
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(ui->okButton, &QPushButton::pressed, this, [this]() { accept();});
    connect(ui->cancelButton, &QPushButton::pressed, this, [this]() { reject();});
}

WfNodeEditDialog::~WfNodeEditDialog() {
    delete ui;
}

void WfNodeEditDialog::showPanel(QWidget*widget) {
    ui->panelsStack->setCurrentWidget(widget);
}

bool WfNodeEditDialog::editHotkeyNode(WFNode*node) {
    showPanel(ui->hotkeyPanel);
    setWindowTitle("Edit Hotkey");
    ui->hotkeyHotkeyEdit->setKeySequence(node->props.value("hotkey","").toString());
    if(exec()) {
        node->props["hotkey"]=ui->hotkeyHotkeyEdit->keySequence().toString();
        return true;
    }
    return false;
}

bool WfNodeEditDialog::editKeywordNode(WFNode*node) {
    showPanel(ui->keywordPanel);
    setWindowTitle("Edit Keyword");
    ui->keywordKeywordEdit->setText(node->props.value("keyword","").toString());
    ui->keywordDescriptionEdit->setText(node->props.value("description","").toString());
    ui->keywordTextEdit->setText(node->props.value("text","").toString());
    if(exec()) {
        node->props["keyword"]=ui->keywordKeywordEdit->text().trimmed();
        node->props["description"]=ui->keywordDescriptionEdit->text().trimmed();
        node->props["text"]=ui->keywordTextEdit->text().trimmed();
        return true;
    }
    return false;
}

bool WfNodeEditDialog::editScriptingNode(WFNode*node) {
    showPanel(ui->scriptingPanel);
    setWindowTitle("Edit External Script");
    ui->scriptingPathEdit->setText(node->props.value("appPath","").toString());
    ui->scriptingDescriptionEdit->setText(node->props.value("description","").toString());
    ui->scriptingScriptEdit->setPlainText(node->props.value("script","").toString());
    if(exec()) {
        node->props["appPath"]=ui->scriptingPathEdit->text().trimmed();
        node->props["description"]=ui->scriptingDescriptionEdit->text().trimmed();
        node->props["script"]=ui->scriptingScriptEdit->toPlainText().trimmed();
        return true;
    }
    return false;
}

bool WfNodeEditDialog::editClipboardCopyNode(WFNode*node) {
    showPanel(ui->clipboardCopyPanel);
    setWindowTitle("Edit Clipboard Copy");
    ui->clipboardCopyTextEdit->setPlainText(node->props.value("content","").toString());
    if(exec()) {
        node->props["content"]=ui->clipboardCopyTextEdit->toPlainText().trimmed();
        return true;
    }
    return false;
}


bool WfNodeEditDialog::editExecAppNode(WFNode*node) {
    showPanel(ui->execAppPanel);
    setWindowTitle("Edit Execute Application");
    ui->execAppAppPath->setPlainText(node->props.value("appPath","").toString());
    ui->execAppWorkDirEdit->setText(node->props.value("workdir","").toString());
    ui->execAppDetachedCB->setChecked(node->props.value("detached",true).toBool());
    ui->execAppIfNotRunCB->setChecked(node->props.value("ifnotrun",false).toBool());
    ui->execAppEnvEdit->setPlainText(envVariablesToString(node->props.value("env").toMap()));
    connect(ui->execAppDetachedCB, &QCheckBox::stateChanged,this, [this]() {
        ui->execAppIfNotRunCB->setEnabled(!ui->execAppDetachedCB->isChecked());
    });
    ui->execAppIfNotRunCB->setEnabled(!ui->execAppDetachedCB->isChecked());

    if(exec()) {
        node->props["appPath"]=ui->execAppAppPath->toPlainText().trimmed();
        node->props["workdir"]=ui->execAppWorkDirEdit->text().trimmed();
        node->props["detached"]=ui->execAppDetachedCB->isChecked();
        node->props["ifnotrun"]=ui->execAppIfNotRunCB->isChecked();
        node->props["env"]=stringToEnvVariablesMap(ui->execAppEnvEdit->toPlainText().trimmed());
        return true;
    }
    return false;
}

bool WfNodeEditDialog::editNotificationNode(WFNode*node) {
    showPanel(ui->notificationPanel);
    setWindowTitle("Edit Notification");
    ui->notificationTitleEdit->setText(node->props.value("title","").toString());
    ui->notificationMessageEdit->setText(node->props.value("message","").toString());
    if(exec()) {
        node->props["title"]=ui->notificationTitleEdit->text().trimmed();
        node->props["message"]=ui->notificationMessageEdit->text().trimmed();
        return true;
    }
    return false;
}

void fillSoundLineEditContextMenu(CustomLineEdit*soundLineEditor) {
    QDir bundledSoundsResourceFolder(":/sounds/res/sounds/");
    QStringList files=bundledSoundsResourceFolder.entryList();
    for(int i=0;i<files.size();i++) {
        QString name=files[i];
        soundLineEditor->contextMenuHelper.addContextMenu(name, QString("Bundled/")+name, [soundLineEditor](QString id) {
            soundLineEditor->insert(QString("bundled://")+id);
        });
    }
}

bool WfNodeEditDialog::editPlaySoundNode(WFNode*node) {
    showPanel(ui->playSoundPanel);
    setWindowTitle("Edit Play sound");
    ui->playSoundPathToSoundEdit->setText(node->props.value("sound","").toString());
    fillSoundLineEditContextMenu(ui->playSoundPathToSoundEdit);

    connect(ui->playSoundPlayButton, &QPushButton::pressed, this, [this]() {
        QString path=ui->playSoundPathToSoundEdit->text().trimmed();
        if(path.startsWith("bundled://")) {
            path=QString(":/sounds/res/sounds/") + path.mid(QString("bundled://").length());
        }

        QFileInfo fileInfo(path);
        if(!fileInfo.exists()) {
            PlaceholderExpander placeholderExpander(appGlobals);
            if(placeholderExpander.containsPlaceholder(path)) {
                QMessageBox::warning(this, "Error", tr("Cannot test file path that contains placeholders"),QMessageBox::Ok);
            } else {
                QMessageBox::warning(this, "Error", tr("Sound file is not found"), QMessageBox::Ok);
            }
            return;
        }
        QSound::play(path);
    });
    if(exec()) {
        node->props["sound"]=ui->playSoundPathToSoundEdit->text().trimmed();
        return true;
    }
    return false;
}

bool WfNodeEditDialog::editExternalTriggerNode(WFNode*node) {
    showPanel(ui->extTriggerPanel);
    setWindowTitle("Edit External trigger");
    ui->extTriggerIdEdit->setText(node->props.value("triggerId","").toString());
    if(exec()) {
        node->props["triggerId"]=ui->extTriggerIdEdit->text().trimmed();
        return true;
    }

    return false;
}

bool WfNodeEditDialog::editExternalScriptNode(WFNode*node) {
    showPanel(ui->extScriptPanel);
    setWindowTitle("Edit External script");
    ui->extScriptAppPath->setText(node->props.value("appPath","").toString());
    ui->extScriptScriptEdit->setPlainText(node->props.value("script","").toString());
    ui->extScriptCaptureOutputCB->setChecked(node->props.value("captureOutput",false).toBool());
    connect(ui->extScriptHelpLabel, &QLabel::linkActivated,this, [this](QString) {
        ui->extScriptAppPath->insert("python ${scriptFilePath}");
    });
    if(exec()) {
        node->props["appPath"]=ui->extScriptAppPath->text().trimmed();
        node->props["script"]=ui->extScriptScriptEdit->toPlainText().trimmed();
        node->props["captureOutput"]=ui->extScriptCaptureOutputCB->isChecked();
        return true;
    }

    return false;
}

bool WfNodeEditDialog::editSelectorNode(WFNode*node) {
    showPanel(ui->selectorPanel);
    setWindowTitle("Edit Selector");
    ui->selectorItemsEdit->setPlainText(node->props.value("selectorItemsCSV","").toString());
    connect(ui->selectorHelpLabel, &QLabel::linkActivated,this, [this](QString link) {
        if(link=="#insert_header") {
            ui->selectorItemsEdit->insertPlainText("`~`id`~`|`~`text`~`|`~`description`~`|`~`icon`~`");
        }
        if(link=="#insert_example") {
            ui->selectorItemsEdit->insertPlainText("`~`id`~`|`~`text`~`|`~`description`~`|`~`icon`~`\n`~`123`~`|`~`Item 1`~`|`~`Example item`~`|`~`icon://help`~`");
        }
    });
    if(exec()) {
        node->props["selectorItemsCSV"]=ui->selectorItemsEdit->toPlainText().trimmed();
        return true;
    }

    return false;
}


bool WfNodeEditDialog::editOpenUrlNode(WFNode*node) {
    showPanel(ui->openUrlPanel);
    setWindowTitle("Edit Open Url");
    ui->openUrlEdit->setText(node->props.value("url","").toString());
    if(exec()) {
        node->props["url"]=ui->openUrlEdit->text().trimmed();
        return true;
    }

    return false;
}

bool WfNodeEditDialog::editLuaScriptNode(WFNode*node) {
    showPanel(ui->luaScriptPanel);
    setWindowTitle("Edit Lua Script");
    ui->luaScriptSourceEdit->setText(node->props.value("luaScript","").toString());
    if(exec()) {
        node->props["luaScript"]=ui->luaScriptSourceEdit->toPlainText().trimmed();
        return true;
    }

    return false;
}
