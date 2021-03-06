#include <QMessageBox>
#include <QSound>
#include "tablehelper.h"
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
    ui->extScriptProcessJsonOutputCB->setChecked(node->props.value("processJsonOutput",false).toBool());
    connect(ui->extScriptHelpLabel, &QLabel::linkActivated,this, [this](QString) {
        ui->extScriptAppPath->insert("python ${scriptFilePath}");
    });
    if(exec()) {
        node->props["appPath"]=ui->extScriptAppPath->text().trimmed();
        node->props["script"]=ui->extScriptScriptEdit->toPlainText().trimmed();
        node->props["captureOutput"]=ui->extScriptCaptureOutputCB->isChecked();
        node->props["processJsonOutput"]=ui->extScriptProcessJsonOutputCB->isChecked();
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

QString variableToString(Variable*var, int column) {
    if(column==0) {
        return var->name;
    } else if(column==1) {
        return var->hidden
            ? QString("*").repeated(var->value.length())
            : var->value;
    }
    return "unknown column";
}

bool WfNodeEditDialog::editVars(QList<Variable>*variables) {
    showPanel(ui->varsPanel);
    setWindowTitle("Edit Variables");
    QList<Variable>_variables=*variables;
    TableHelper<Variable*>tableHelper(ui->varsTable, 2, variableToString);

    QStringList varsTableHeader;
    varsTableHeader<<"Variable"<<"Value";
    ui->varsTable->setHorizontalHeaderLabels(varsTableHeader);
    ui->varsTable->setSelectionMode(QTableWidget::SingleSelection);
    ui->varsTable->setSelectionBehavior(QTableWidget::SelectRows);
    ui->varsTable->verticalHeader()->setVisible(false);
    ui->varsTable->horizontalHeader()->setStretchLastSection(true);
    ui->varsTable->setEditTriggers(QTableWidget::NoEditTriggers);
    for(int i=0;i<_variables.count();i++)tableHelper.add((Variable*)&(_variables.at(i)));

    auto checkEditableStatus=[&tableHelper, this]() {
        bool selected=tableHelper.getSelectedRow()!=-1;
        ui->varsRemoveVarButton->setEnabled(selected);
    };

    connect(ui->varsTable, &QTableWidget::itemSelectionChanged, ui->varsTable, [&checkEditableStatus]() {
        checkEditableStatus();
    });

    connect(ui->varsTable, &QTableWidget::cellDoubleClicked, ui->varsTable, [this, &tableHelper, &_variables]() {
        int row = tableHelper.getSelectedRow();
        if(row == -1) {
            return;
        }

        Variable*var = &_variables[row];
        WfNodeEditDialog editDialog(this, appGlobals);
        if(editDialog.editVariable(var)) {
            tableHelper.set(var, row);
            tableHelper.refreshRow(row);
        }
    });
    connect(ui->varsAddVarButton, &QPushButton::pressed, this, [&_variables, &tableHelper, &checkEditableStatus]() {
        Variable var;
        var.name="Variable";
        var.hidden=false;
        var.value="";
        _variables.append(var);
        tableHelper.add(&(_variables.last()));
        checkEditableStatus();
    });
    connect(ui->varsRemoveVarButton, &QPushButton::pressed, this, [&_variables, &tableHelper, &checkEditableStatus]() {
        if(tableHelper.getSelectedRow()!=-1){
            _variables.removeAt(tableHelper.getSelectedRow());
            tableHelper.remove(tableHelper.getSelectedRow());
        }
        checkEditableStatus();
    });
    checkEditableStatus();
    if(exec()) {
        *variables=_variables;
        return true;
    }

    return false;
}

bool WfNodeEditDialog::editVariable(Variable*variable) {
    showPanel(ui->editVarPanel);
    setWindowTitle("Edit Variable");
    ui->editVarNameEdit->setText(variable->name);
    ui->editVarValueEdit->setText(variable->value);
    ui->editVarHiddenCB->setChecked(variable->hidden);
    if(variable->hidden){
        ui->editVarValueEdit->setEchoMode(QLineEdit::Password);
    }

    connect(ui->editVarHiddenCB, &QCheckBox::stateChanged, ui->editVarHiddenCB, [this](){
        bool newHiddenState = ui->editVarHiddenCB->isChecked();
        if(newHiddenState==false){
            ui->editVarValueEdit->setText("");
            ui->editVarValueEdit->setEchoMode(QLineEdit::Normal);
        } else {
            ui->editVarValueEdit->setEchoMode(QLineEdit::Password);
        }
    });

    if(exec()) {
        variable->name=ui->editVarNameEdit->text().trimmed();
        variable->hidden=ui->editVarHiddenCB->isChecked();
        variable->value=ui->editVarValueEdit->text().trimmed();
        return true;
    }

    return false;
}

bool WfNodeEditDialog::editDebug(WFNode*node) {
    showPanel(ui->debugPanel);
    setWindowTitle("Edit Debug");
    ui->debugLabelEdit->setText(node->props["label"].toString());
    ui->debugPrintVarsCB->setChecked(node->props["printVars"].toBool());
    if(exec()) {
        node->props["label"]=ui->debugLabelEdit->text().trimmed();
        node->props["printVars"]=ui->debugPrintVarsCB->isChecked();
        return true;
    }

    return false;
}
