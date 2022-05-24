#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QWidget>
#include <QLineEdit>

namespace Ui {
class TestDialog;
}
class Person{
public:
    QString name;
    int age;
};

class InputPanel:public QWidget {
public:
    QLineEdit*name;
    QLineEdit*age;
    int index;
    InputPanel(QWidget*parent=0):QWidget(parent) {
        name=new QLineEdit(this);
        name->setGeometry(0,0,100,20);
        age=new QLineEdit(this);
        age->setGeometry(0,25,100,20);
        setBaseSize(100, 45);
    }
};

class TestDialog : public QWidget
{
    Q_OBJECT

public:
    explicit TestDialog(QWidget *parent = nullptr);
    ~TestDialog();
    void updateData();
private:
    Ui::TestDialog *ui;
};

#endif // TESTDIALOG_H
