#ifndef EDITWEBSEARCHDIALOG_H
#define EDITWEBSEARCHDIALOG_H

#include "QDialog"
#include "includes.h"

namespace Ui {
class EditWebSearchDialog;
}

class EditWebSearchDialog : public QDialog
{
    Q_OBJECT
public:
    explicit EditWebSearchDialog(QWidget *parent = nullptr);
    ~EditWebSearchDialog();
    bool editWebSearch(QVariantMapWithCheck&result,QString url, QString keyword, QString title);
private:
    Ui::EditWebSearchDialog *ui;
};

#endif // EDITWEBSEARCHDIALOG_H
