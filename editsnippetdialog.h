#ifndef EDITSNIPPETDIALOG_H
#define EDITSNIPPETDIALOG_H

#include <QDialog>

namespace Ui {
class EditSnippetDialog;
}

class EditSnippetDialog : public QDialog {
    Q_OBJECT
private:
    QString snippetName;
    QString snippetText;
    bool modalResult;
public:
    explicit EditSnippetDialog(QWidget *parent = nullptr);
    ~EditSnippetDialog();
    bool editSnippet(QString name, QString snippet);
    QString getName() {return snippetName;}
    QString getSnippet() {return snippetText;}
private:
    Ui::EditSnippetDialog *ui;
};

#endif // EDITSNIPPETDIALOG_H
