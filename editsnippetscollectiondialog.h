#ifndef EDITSNIPPETSCOLLECTIONDIALOG_H
#define EDITSNIPPETSCOLLECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class EditSnippetsCollectionDialog;
}

class EditSnippetsCollectionDialog : public QDialog
{
    Q_OBJECT
private:
    QString collectionName;
    QString collectionKeyword;
    bool modalResult;
public:
    explicit EditSnippetsCollectionDialog(QWidget *parent = nullptr);
    ~EditSnippetsCollectionDialog();
    bool editSnippetsCollection(QString name, QString keyword);
    QString getName() {return collectionName;}
    QString getKeyword() {return collectionKeyword;}
private:
    Ui::EditSnippetsCollectionDialog *ui;
};

#endif // EDITSNIPPETSCOLLECTIONDIALOG_H
