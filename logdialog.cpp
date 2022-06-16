#include "logdialog.h"
#include <QTextBlock>
#include <QDebug>
#include <QScrollBar>
#include "ui_logdialog.h"
//int counter=0;
LogDialog::LogDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogWindow) {
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->logEdit->setLineWrapMode(QPlainTextEdit::NoWrap);
    ui->logEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->logEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    cursor = new QTextCursor(ui->logEdit->document());
    connect(ui->closeButton, &QPushButton::pressed, this, [this]() {
        //qDebug()<<"hello world "<<counter;
        //counter++;
        close();
    });
}

LogDialog::~LogDialog() {
    delete ui;
}

void LogDialog::initLogs(QList<QString>&logLines) {
    foreach(const QString&line, logLines) {
        addLog(line);
    }
    ui->logEdit->horizontalScrollBar()->setValue(0);
}

void LogDialog::addLog(QString string) {
    bool atEnd=ui->logEdit->textCursor().atEnd();

    if(ui->logEdit->document()->isEmpty()) {
        ui->logEdit->insertPlainText(string);
    } else {
        cursor->insertBlock();
        cursor->insertText(string);
    }
    if(ui->logEdit->document()->lineCount()>maxLines) {
        cursor->movePosition(QTextCursor::Start);
        cursor->select(QTextCursor::BlockUnderCursor);
        cursor->removeSelectedText();
        cursor->deleteChar();
        cursor->movePosition(QTextCursor::End);
    }
    if(atEnd) {
        ui->logEdit->textCursor().movePosition(QTextCursor::End);
        ui->logEdit->ensureCursorVisible();
    }
}
