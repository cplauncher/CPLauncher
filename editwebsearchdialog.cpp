#include <QMessageBox>
#include "editwebsearchdialog.h"
#include "ui_editwebsearchdialog.h"

EditWebSearchDialog::EditWebSearchDialog(QWidget *parent):QDialog(parent),ui(new Ui::EditWebSearchDialog) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    connect(ui->okButton, &QPushButton::pressed, this, [this]() {
        QString url=ui->urlLineEdit->text().trimmed();
        QString keyword=ui->keywordLineEdit->text().trimmed();
        if(url.isEmpty()) {
            QMessageBox::warning(this, "Error", tr("Url cannot be empty"),QMessageBox::Ok);
            ui->urlLineEdit->setFocus();
            return;
        }
        if(keyword.isEmpty()) {
            QMessageBox::warning(this, "Error", tr("Keyword cannot be empty"),QMessageBox::Ok);
            ui->keywordLineEdit->setFocus();
            return;
        }
        accept();

    });
    connect(ui->cancelButton, &QPushButton::pressed, this, [this]() {
        reject();
    });
}

EditWebSearchDialog::~EditWebSearchDialog() {
    delete ui;
}

bool EditWebSearchDialog::editWebSearch(QVariantMapWithCheck&result, QString url, QString keyword, QString title) {
    ui->urlLineEdit->setText(url);
    ui->keywordLineEdit->setText(keyword);
    ui->titleLineEdit->setText(title);
    result.clear();
    if(exec()) {
        result["url"]=ui->urlLineEdit->text().trimmed();
        result["keyword"]=ui->keywordLineEdit->text().trimmed();
        result["title"]=ui->titleLineEdit->text().trimmed();
        return true;
    }
    return false;
}
