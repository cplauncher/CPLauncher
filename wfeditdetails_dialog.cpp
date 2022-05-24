#include <QMessageBox>
#include "wfeditdetails_dialog.h"
#include "ui_wfeditdetails_dialog.h"

WfEditDetailsDialog::WfEditDetailsDialog(QWidget *parent)
                       :QDialog(parent), ui(new Ui::WfEditDetailsDialog) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    connect(ui->okButton, &QPushButton::pressed, this, [this]() {
        accept();
    });
    connect(ui->cancelButton, &QPushButton::pressed, this, [this]() {
        reject();
    });
}

WfEditDetailsDialog::~WfEditDetailsDialog() {
    delete ui;
}

bool WfEditDetailsDialog::editWorkflowDetails(QVariantMapWithCheck&result, QString name, QString description, QString author, QString website) {
    ui->nameLineEdit->setText(name);
    ui->descriptionTextEdit->setPlainText(description);
    ui->authorLineEdit->setText(author);
    ui->websiteLineEdit->setText(website);
    if(exec()) {
        result["name"]=ui->nameLineEdit->text().trimmed();
        result["description"]=ui->descriptionTextEdit->document()->toPlainText().trimmed();
        result["author"]=ui->authorLineEdit->text().trimmed();
        result["website"]=ui->websiteLineEdit->text().trimmed();
        return true;
    }

    return false;
}
