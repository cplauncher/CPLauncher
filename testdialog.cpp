#include "testdialog.h"

#include "ui_testdialog.h"

Person createRandomPerson(int i){
    QStringList names;
    names<<"Ivan"<<"Dmitry"<<"Alexander"<<"Maria"<<"Irina"<<"Jane"<<"Greg"<<"John"<<"Bill"<<"Tom"<<"Tim"<<"Julia"<<"Ann"<<"Gloria"<<"Mike"<<"Olga";
    Person p;
    p.name=names[qrand()%names.size()];
    p.age=i;
    return p;
}

TestDialog::TestDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TestDialog)
{
    setWindowFlags(Qt::WindowStaysOnTopHint);
    ui->setupUi(this);
    ui->listWidget->setCreateItemPanelCallback([this](){
        InputPanel*ip= new InputPanel(ui->listWidget);
        ip->setAutoFillBackground(true);
        QPalette palette;
        palette.setColor(QPalette::Background, Qt::yellow);
        ip->setPalette(palette);
        return ip;
    });
    ui->listWidget->setInitItemPanelCallback([](QWidget*itemPanel, Person&object, int index){
       InputPanel*panel=(InputPanel*)itemPanel;
       panel->age->setText(QString::number(object.age));
       panel->name->setText(object.name);
       panel->index=index;
    });
    ui->listWidget->setSelectionStatusCallback([](QWidget*itemPanel, Person&, bool selected){
        QPalette palette;
        if(selected){
            palette.setColor(QPalette::Background, Qt::blue);
        }else{
            palette.setColor(QPalette::Background, Qt::yellow);
        }

        itemPanel->setPalette(palette);
    });
    ui->listWidget->init();
    connect(ui->itemsCountEdit, &QLineEdit::returnPressed, this, [this](){
        int count=ui->itemsCountEdit->text().trimmed().toInt();
        QList<Person>list;
        for(int i=0;i<count;i++){
            Person p=createRandomPerson(i);
            list.append(p);
        }
        ui->listWidget->setItems(list);
        updateData();
    });
    connect(ui->selectedEdit, &QLineEdit::returnPressed, this, [this](){
        int selected=ui->selectedEdit->text().trimmed().toInt();
        ui->listWidget->setSelectedIndex(selected);
        updateData();
    });
    connect(ui->topEdit, &QLineEdit::returnPressed, this, [this](){
        int top=ui->topEdit->text().trimmed().toInt();
        ui->listWidget->setTopIndex(top);
        updateData();
    });
    connect(ui->ensVisibleEdit, &QLineEdit::returnPressed, this, [this](){
        int index=ui->ensVisibleEdit->text().trimmed().toInt();
        ui->listWidget->ensureIndexVisible(index);
        updateData();
    });
    connect(ui->addButton, &QPushButton::pressed, this, [this](){
        int count=ui->listWidget->count()-1;
        ui->listWidget->addItem(createRandomPerson(count+1));
        updateData();
    });
    updateData();
}

void TestDialog::updateData(){
    ui->itemsCountEdit->setText(QString::number(ui->listWidget->count()));
    ui->topEdit->setText(QString::number(ui->listWidget->getTopIndex()));
    ui->selectedEdit->setText(QString::number(ui->listWidget->getSelectedIndex()));
}

TestDialog::~TestDialog()
{
    delete ui;
}
