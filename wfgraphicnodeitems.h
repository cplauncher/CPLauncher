#ifndef WFGRAPHICNODEITEMS_H
#define WFGRAPHICNODEITEMS_H

#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QPainter>
#include <QtMath>
#include "svgitem.h"
#include "configuration.h"

class ArrowItem : public QGraphicsLineItem {
  public:
    ArrowItem(qreal x1, qreal y1, qreal x2, qreal y2)
        :QGraphicsLineItem(x1, y1, x2, y2) {
    }

    QRectF boundingRect() const {
        qreal extra = (pen().width() + 20) / 2.0;
        return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(),line().p2().y() - line().p1().y()))
            .normalized().adjusted(-extra, -extra, extra, extra);
    }

    QPainterPath shape() const {
        QPainterPath path = QGraphicsLineItem::shape();
        return path;
    }

    void paint (QPainter* painter, const QStyleOptionGraphicsItem* option,QWidget* widget = 0) {
        Q_UNUSED(option);
        Q_UNUSED(widget);
        painter->setPen(pen());
        double angle = std::atan2(-line().dy(), line().dx())-qDegreesToRadians((float)180);
        qreal arrowSize = 5;
        QPointF arrowP1 = line().p2() + QPointF(sin(angle + M_PI / 3) * arrowSize,
                                        cos(angle + M_PI / 3) * arrowSize);
        QPointF arrowP2 = line().p2() + QPointF(sin(angle + M_PI - M_PI / 3) * arrowSize,
                                        cos(angle + M_PI - M_PI / 3) * arrowSize);

        QPolygonF arrowHead;
        arrowHead.clear();
        arrowHead << line().p2() << arrowP1 << arrowP2;
        painter->drawLine(line());
        painter->setBrush(Qt::black);
        painter->drawPolygon(arrowHead);
        if (isSelected()) {
            painter->setPen(QPen(Qt::black, 1, Qt::DashLine));
            QLineF myLine = line();
            myLine.translate(0, 4.0);
            painter->drawLine(myLine);
            myLine.translate(0,-8.0);
            painter->drawLine(myLine);
        }
    }
};

class LinkItem;
class WFNodeItem;
class InputPortItem : public QGraphicsEllipseItem {
    int index;
public:
    InputPortItem(qreal x, qreal y, qreal w, qreal h, int index, WFNodeItem *parent = nullptr):
        QGraphicsEllipseItem(x,y,w,h,(QGraphicsItem*)parent) {
        setBrush(QBrush(Qt::green));
        this->index = index;
        owner=parent;
    }
    int getIndex(){return index;}

    void removeLink(LinkItem*link) {
        inputLinks.removeAll(link);
    }

    QList<LinkItem*>inputLinks;
    WFNodeItem*owner;
};


class OutputPortItem : public QGraphicsEllipseItem {
    ArrowItem*tempArrow=NULL;
    QPointF startPoint;
    bool cursorOverriden=false;
    int index;
public:
    WFNodeItem*owner;
    QList<LinkItem*>outputLinks;

    OutputPortItem(qreal x, qreal y, qreal w, qreal h, int index, WFNodeItem *parent = nullptr);
    int getIndex(){ return index; }
    void removeLink(LinkItem*link);
    void createLink(InputPortItem*inputNode, bool fireCallback);
protected:
    void overrideWithArrowCursor();
    void disableOverrideCursor();
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    InputPortItem*getPointingInputNode(QPointF scenePoint);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
};

class LinkItem : public ArrowItem {
  public:
    OutputPortItem*sourcePortItem;
    InputPortItem*destinationPortItem;
    LinkItem(QGraphicsItem* parent = 0);
    void link(OutputPortItem*sourcePortItem,InputPortItem*destinationPortItem);
    void updatePosition();
};

class WFNodeItem:public SvgItem {

public:
    QList<InputPortItem*>inputs;
    QList<OutputPortItem*>outputs;
    WFNode*node=NULL;
    std::function<void(LinkItem*link)>linkCreatedCallback;
    std::function<void(WFNodeItem*nodeItem)>itemMovedCallback;
    WFNodeItem(WFNode*wfNode,
               std::function<void(LinkItem*link)>linkCreatedCallback,
               std::function<void(WFNodeItem*nodeItem)>itemMovedCallback);
    void updateAfterSettingsChange();
    void enableMoveCallback(bool value) {setFlag(QGraphicsItem::ItemSendsGeometryChanges, value);}
    void updateLinkPositions();
};

#endif // WFGRAPHICNODEITEMS_H
