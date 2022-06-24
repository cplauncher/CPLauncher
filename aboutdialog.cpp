#include "aboutdialog.h"
#include "ui_aboutdialog.h"

AboutDialog::AboutDialog(QWidget *parent, AppGlobals*appGlobals) :
    QWidget(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->versionLabel->setText(appGlobals->version);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
