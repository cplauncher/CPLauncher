#include <QMessageBox>
#include "editsnippetscollectiondialog.h"
#include "ui_editsnippetscollectiondialog.h"

EditSnippetsCollectionDialog::EditSnippetsCollectionDialog(QWidget *parent)
                       :QDialog(parent), ui(new Ui::EditSnippetsCollectionDialog) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    modalResult=false;

    connect(ui->okButton, &QPushButton::pressed, this, [this]() {
        collectionName=ui->nameLineEdit->text().trimmed();
        collectionKeyword=ui->keywordLineEdit->text().trimmed();
        if(collectionName.isEmpty()) {
            QMessageBox::warning(this, "Error", tr("Snippets Collection name cannot be empty"), QMessageBox::Ok);
            ui->nameLineEdit->setFocus();
            return;
        }
        modalResult=true;
        close();

    });
    connect(ui->cancelButton, &QPushButton::pressed, this, [this]() {
        close();
    });
}

EditSnippetsCollectionDialog::~EditSnippetsCollectionDialog() {
    delete ui;
}

bool EditSnippetsCollectionDialog::editSnippetsCollection(QString name, QString keyword) {
    ui->nameLineEdit->setText(name);
    ui->keywordLineEdit->setText(keyword);
    exec();
    return modalResult;
}
