#ifndef TABLEHELPER_H
#define TABLEHELPER_H
#include <QTableWidget>
#include <functional>

template <class T>
class TableHelper
{
    QList<T>values;
    QTableWidget*table;
    QString(*stringExtractor)(T object, int column);
    void(*editValueApplyer)(QString newValue, T object, int column);

    void fillRow(int row, T value) {
        for(int i=0;i<table->columnCount();i++) {
            QString text=stringExtractor(value, i);
            if(text.contains('\n')) {
                text.replace('\n',' ');
            }
            if(text.contains('\r')) {
                text.replace('\r',' ');
            }
            table->setItem(row, i,new QTableWidgetItem(text));
        }
    }
public:
    TableHelper(QTableWidget*table,int columnCount,QString(*stringExtractor)(T object, int column)) {
        this->stringExtractor=stringExtractor;
        this->table=table;
        table->setColumnCount(columnCount);
    }

    void clear() {
        table->setRowCount(0);
        values.clear();
    }

    int rowCount() const{
        return table->rowCount();
    }

    void add(T value) {
        values.append(value);
        int row=rowCount();
        table->insertRow(row);
        fillRow(row, value);
    }

    void set(T value, int row){
        values[row]=value;
    }

    void refreshRow(int row) {
        T value=values[row];
        fillRow(row, value);
    }

    void remove(int row) {
        values.removeAt(row);
        table->removeRow(row);
    }

    //this method works only in case if table configured as
    //setSelectionBehavior(QTableWidget::SelectRows);
    int getSelectedRow()const {
        QItemSelectionModel *select = table->selectionModel();
        if(!select->hasSelection()) {
            return -1;
        }
        QModelIndexList selectionModel=select->selectedRows();
        return selectionModel.at(0).row();
    }

    T getSelectedObject() const{
        int selectedRow=getSelectedRow();
        if(selectedRow==-1) {
            return NULL;
        }
        return values[selectedRow];
    }

    void setSelectedRow(int row) {
        table->selectRow(row);
    }
};

#endif // TABLEHELPER_H
