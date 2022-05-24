#ifndef WFEDITDETAILS_DIALOG_H
#define WFEDITDETAILS_DIALOG_H

#include <QDialog>
#include "includes.h"

namespace Ui {
class WfEditDetailsDialog;
}

class WfEditDetailsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit WfEditDetailsDialog(QWidget *parent = nullptr);
    ~WfEditDetailsDialog();
    bool editWorkflowDetails(QVariantMapWithCheck&result, QString name, QString description, QString author, QString website);
private:
    Ui::WfEditDetailsDialog *ui;
};

#endif // WFEDITDETAILS_DIALOG_H
