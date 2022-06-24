#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include "includes.h"

namespace Ui {
class AboutDialog;
}

class AboutDialog : public QWidget
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent, AppGlobals*appGlobals);
    ~AboutDialog();

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
