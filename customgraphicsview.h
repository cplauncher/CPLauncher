#ifndef CUSTOMGRAPHICSVIEW_H
#define CUSTOMGRAPHICSVIEW_H
#include <QGraphicsView>
#include <functional>
#include <QMenu>
#include <QGraphicsSceneMouseEvent>
class CustomGraphicsView: public QGraphicsView {

public:
    CustomGraphicsView(QWidget*parent=0):QGraphicsView(parent) {

    }
};

class CustomGraphicsScene:public QGraphicsScene {
    std::function<void(QMenu*menu,QPointF screenPos)>contextMenuGenerator;
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event)override{
        if (event->button() == Qt::RightButton) {
            event->accept();
            return;
        }
        QGraphicsScene::mousePressEvent(event);

    /*
        QGraphicsItem* item = itemAt(event->scenePos(), QTransform());

        if (item != nullptr && !item->isSelected())
        {
            item->setSelected(1);
        }
        */
    }

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override {
        if(!contextMenuGenerator) {
            return;
        }

        QMenu menu;
        contextMenuGenerator(&menu, event->scenePos());
        menu.exec(event->screenPos());
    }
public:
    void setContextMenuGenerator(std::function<void(QMenu*menu,QPointF screenPos)>contextMenuGenerator) {
        this->contextMenuGenerator=contextMenuGenerator;
    }
};
#endif // CUSTOMGRAPHICSVIEW_H
