#ifndef WIDGETPANELLIST_H
#define WIDGETPANELLIST_H
#include <QLineEdit>
#include <QWidget>
#include <QDebug>
#include <QResizeEvent>
#include <functional>

template <typename T> class WidgetPanelList: public QWidget {
    QList<T>items;
    int selectedIndex;
    int currentTop;
    QList<QWidget*>itemPanels;
    int maxItemsToShow=4;
    std::function<QWidget*()>createItemPanelCallback;
    std::function<void(QWidget*, T&value, int itemIndex)>initItemPanelCallback;
    std::function<void(QWidget*, T&value, bool selected)>selectionStatusCallback;
    int PADDING_TOP=5;
    int PADDING_BOTTOM=5;
    int PADDING_LEFT=5;
    int PADDING_RIGHT=5;
    int PADDING_BETWEEN_ITEMS=5;
    int min(int i1, int i2) { return (i1<i2)?i1:i2; }
    int max(int i1, int i2) { return (i1>i2)?i1:i2; }
  protected:
    virtual void resizeEvent(QResizeEvent *event) override {
        if(event->oldSize().width()!=event->size().width()) {
            for(int i=0;i<itemPanels.size();i++) {
                int newWidth=event->size().width();
                QWidget*panel=itemPanels[i];
                QRect rect=panel->geometry();
                panel->setGeometry(PADDING_LEFT, rect.y(), newWidth-PADDING_LEFT-PADDING_RIGHT, rect.height());
            }
        }
        QWidget::resizeEvent(event);
    }

    void updateListHeight() {
        QRect r=rect();
        if(items.isEmpty()) {
            setGeometry(r.x(), r.y(),r.width(), 0);
            setBaseSize(r.width(), 0);
            return;
        }

        int lastIndex = min(itemPanels.size()-1, items.size()-currentTop-1);
        int bottom = itemPanels[lastIndex]->geometry().bottom()+PADDING_BOTTOM;
        setGeometry(r.x(), r.y(),r.width(), bottom);
        setBaseSize(r.width(), bottom);

        int diff=itemPanels.size()-lastIndex-1;
        for(int i=0;i<diff;i++) {
            //itemPanels[i+lastIndex+1]->setVisible(false);
        }
    }

    int getItemPanelIndex(int index) {
        //not valid if outside of items
        if(index<0 || index>=items.size()) return -1;

        //not valid if above the scroll top
        if(index<currentTop) return -1;

        int pos=index-currentTop;
        if(pos>=itemPanels.size()) {
            return -1;
        }
        return pos;
    }

    int getYForNewItemPanel() {
        if(itemPanels.isEmpty()) {
            return PADDING_TOP;
        }
        QRect r = itemPanels.last()->geometry();
        return r.bottom()+PADDING_BETWEEN_ITEMS;
    }

    void removeSelectionMarkFromSelectedPanel() {
        if(selectedIndex!=-1) {
            int panelIndex=getItemPanelIndex(selectedIndex);
            if(panelIndex!=-1) {
                T&item=items[selectedIndex];
                selectionStatusCallback(itemPanels[panelIndex],item, false);
            }
        }
    }

    void setSelectionmarkForSelectedPanel(int index) {
        if(index!=-1) {
            int panelIndex=getItemPanelIndex(index);
            if(panelIndex!=-1) {
                T&item=items[index];
                selectionStatusCallback(itemPanels[panelIndex],item, true);
            }
        }
    }

  public:
    WidgetPanelList(QWidget*parent=0):QWidget(parent) {
        currentTop=0;
        selectedIndex=-1;

    }

    void init() {
        preallocateItemWidgets();
        updateListHeight();
    }

    void preallocateItemWidgets() {
        //remove old
        for(int i=0;i<itemPanels.size();i++) {
            delete itemPanels[i];
        }
        itemPanels.clear();

        for(int i=0;i<maxItemsToShow;i++) {
            allocateNewItemPanel();
        }
    }

    void setPaddings(int left, int right, int top, int bottom, int between) {
        PADDING_LEFT=left;
        PADDING_RIGHT=right;
        PADDING_TOP=top;
        PADDING_BOTTOM=bottom;
        PADDING_BETWEEN_ITEMS=between;
    }

    QWidget* allocateNewItemPanel() {
        int y=getYForNewItemPanel();
        QWidget*itemPanel=createItemPanelCallback();
        itemPanel->show();

        itemPanels.append(itemPanel);
        QSize itemPanelBaseSize=itemPanel->baseSize();
        itemPanel->setGeometry(PADDING_LEFT, y, width()-PADDING_LEFT-PADDING_RIGHT, itemPanelBaseSize.height());
        return itemPanel;
    }

    void setCreateItemPanelCallback(std::function<QWidget*()>createItemPanelCallback) { this->createItemPanelCallback=createItemPanelCallback; }
    void setInitItemPanelCallback(std::function<void(QWidget*, T&value, int itemIndex)>initItemPanelCallback) { this->initItemPanelCallback=initItemPanelCallback; }
    void setSelectionStatusCallback(std::function<void(QWidget*, T&value, bool selected)>selectionStatusCallback) { this->selectionStatusCallback=selectionStatusCallback; }
    void setMaxItemsToShow(int count) { this->maxItemsToShow=count; }

    void clear() {
        setSelectedIndex(-1);
        items.clear();
        selectedIndex=-1;
        currentTop=0;
        updateListHeight();
    }

    int count() {
        return items.count();
    }

    int getTopIndex() {
        return currentTop;
    }

    void setTopIndex(int top) {
        if(top<0) {
            top = 0;
        }

        int maxTop = max(items.count()-maxItemsToShow,0);
        top = min(maxTop, top);
        if(top==currentTop) {
            return;
        }
        removeSelectionMarkFromSelectedPanel();
        int diff=items.count()-top;
        diff=min(diff, maxItemsToShow);
        for(int i=0; i<diff; i++) {
            int itemIndex=i+top;
            T&item=items[itemIndex];
            initItemPanelCallback(itemPanels[i],item, itemIndex);
        }
        currentTop=top;
        setSelectionmarkForSelectedPanel(getSelectedIndex());

    }

    int getSelectedIndex() {
        return selectedIndex;
    }

    void setSelectedIndex(int index) {
        if(index==selectedIndex) {
            return;
        }
        //first deselect old selected item
        removeSelectionMarkFromSelectedPanel();
        if(index<0 || index>=items.size()) {
            index=-1;
        }

        setSelectionmarkForSelectedPanel(index);
        selectedIndex=index;
    }

    void ensureIndexVisible(int index) {
        //if outside of items
        if(index<0 || index>items.count()) {
            return;
        }
        //if inside view
        if(index>=currentTop&&index<currentTop+maxItemsToShow) {
            return;
        }
        //if above view
        if(index<currentTop) {
            setTopIndex(index);
        } else {
        //if below view
            setTopIndex(index-maxItemsToShow+1);
        }
    }

    void setItems(QList<T>_items) {
        clear();
        for(int i=0;i<_items.size();i++) {
            addItemInternal(_items[i]);
        }
        updateListHeight();
    }



    void addItemInternal(T&obj) {
        items.append(obj);
        int index=items.size()-1;
        int panelIndex=getItemPanelIndex(index);
        if(panelIndex!=-1) {
            initItemPanelCallback(itemPanels[panelIndex], obj, index);
        }
    }

    void addItem(T obj) {
        addItemInternal(obj);
        updateListHeight();
    }
};

#endif // WIDGETPANELLIST_H
