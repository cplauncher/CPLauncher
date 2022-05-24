#include <QMessageBox>
#include "editsnippetdialog.h"
#include "ui_editsnippetdialog.h"

EditSnippetDialog::EditSnippetDialog(QWidget *parent):QDialog(parent),ui(new Ui::EditSnippetDialog) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    modalResult=false;
    connect(ui->okButton, &QPushButton::pressed, this, [this]() {
        snippetName=ui->nameLineEdit->text().trimmed();
        snippetText=ui->snipLineEdit->document()->toPlainText().trimmed();
        if(snippetName.isEmpty()) {
            QMessageBox::warning(this, "Error", tr("Snippet name cannot be empty"),QMessageBox::Ok);
            ui->nameLineEdit->setFocus();
            return;
        }
        modalResult=true;
        close();

    });
    connect(ui->cancelButton, &QPushButton::pressed, this, [this]() {
        close();
    });

    auto action=[this](QString str) {
        ui->snipLineEdit->insertPlainText(str);
    };
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${os}", "Placeholders/Operation System(os)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${os_version}", "Placeholders/Operation System Version(os_version)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${user_name}", "Placeholders/User Name(user_name)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${hostname}", "Placeholders/Hostname(hostname)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${cpu}", "Placeholders/CPU(cpu)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${user_home}", "Placeholders/Current user home directory(user_home)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${clipboard}", "Placeholders/Clipboard content(clipboard)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${input:Please type text}", "Placeholders/Input Dialog(input)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${snippet}", "Placeholders/Snippet(snippet)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${date}", "Placeholders/Date/Current date(date)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${date:long}", "Placeholders/Date/Current date long(date:long)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${time}", "Placeholders/Date/Current time(time)", action);
    ui->snipLineEdit->contextMenuHelper.addContextMenu("${time:long}", "Placeholders/Date/Current time(time:long)", action);

}

EditSnippetDialog::~EditSnippetDialog() {
    delete ui;
}

bool EditSnippetDialog::editSnippet(QString name, QString keyword) {
    ui->nameLineEdit->setText(name);
    ui->snipLineEdit->setPlainText(keyword);
    exec();
    return modalResult;
}
