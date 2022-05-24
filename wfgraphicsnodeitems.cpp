#include "includes.h"
#include "wfgraphicnodeitems.h"

OutputPortItem::OutputPortItem(qreal x, qreal y, qreal w, qreal h, int index, WFNodeItem *parent):
    QGraphicsEllipseItem(x,y,w,h,(QGraphicsItem*)parent) {
    setCursor(Qt::CrossCursor);
    setBrush(QBrush(Qt::red));
    this->index = index;
    owner=parent;
}
void OutputPortItem::overrideWithArrowCursor() {
    if(!cursorOverriden) {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        cursorOverriden=true;
    }
}
void OutputPortItem::disableOverrideCursor() {
    if(cursorOverriden) {
        QApplication::restoreOverrideCursor();
        cursorOverriden = false;
    }
}

void OutputPortItem::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    Q_UNUSED(event)
    overrideWithArrowCursor();
    startPoint=mapToScene(rect().center());
    tempArrow=new ArrowItem(startPoint.x(),startPoint.y(),startPoint.x(), startPoint.y());
    scene()->addItem(tempArrow);
}

InputPortItem* OutputPortItem::getPointingInputNode(QPointF scenePoint) {
    QList<QGraphicsItem *>foundItems=scene()->items(scenePoint);
    if(!foundItems.isEmpty()) {
        InputPortItem*selectedNode=dynamic_cast <InputPortItem*>(foundItems.first());
        if(selectedNode!=NULL) {
            return selectedNode;
        }
    }
    return NULL;
}
void OutputPortItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event) {
    if(tempArrow!=NULL) {
        QPointF newPos=event->lastScenePos();
        InputPortItem*pointingNode=getPointingInputNode(newPos);
        if(pointingNode!=NULL) {
            disableOverrideCursor();
        } else {
            overrideWithArrowCursor();
        }
        tempArrow->setLine(startPoint.x(),startPoint.y(),newPos.x(), newPos.y());
    }
}
void OutputPortItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event) {
    disableOverrideCursor();
    if(tempArrow!=NULL) {
        delete tempArrow;
        QPointF newPos=event->lastScenePos();
        InputPortItem*inputNode=getPointingInputNode(newPos);
        if(inputNode!=NULL) {
            createLink(inputNode, true);
        }
    }
}
void OutputPortItem::createLink(InputPortItem*inputNode, bool fireCallback) {
    LinkItem*link=new LinkItem();
    link->link(this, inputNode);
    link->updatePosition();
    this->outputLinks.append(link);
    inputNode->inputLinks.append(link);
    scene()->addItem(link);
    if(fireCallback) {
        ((WFNodeItem*)parentItem())->linkCreatedCallback(link);
    }
}

void OutputPortItem::removeLink(LinkItem*link) {
    outputLinks.removeAll(link);
}

LinkItem::LinkItem(QGraphicsItem* parent):ArrowItem(0,0,0,0) {
    Q_UNUSED(parent)
    setFlags(QGraphicsItem::ItemIsSelectable);
    sourcePortItem=NULL;
    destinationPortItem=NULL;
}
void LinkItem::link(OutputPortItem*sourcePortItem,InputPortItem*destinationPortItem) {
    this->sourcePortItem=sourcePortItem;
    this->destinationPortItem=destinationPortItem;
}

void LinkItem::updatePosition() {
    if(sourcePortItem!=NULL && destinationPortItem!=NULL) {
        QPointF startNodeC=sourcePortItem->mapToScene(sourcePortItem->boundingRect().center());
        QPointF endNodeC=destinationPortItem->mapToScene(destinationPortItem->boundingRect().center());
        setLine(startNodeC.x(),startNodeC.y(), endNodeC.x(),endNodeC.y());
    }
}

WFNodeItem::WFNodeItem(WFNode*wfNode,
                       std::function<void(LinkItem*link)>linkCreatedCallback,
                       std::function<void(WFNodeItem*nodeItem)>itemMovedCallback) : SvgItem()  {
    this->node=wfNode;
    this->linkCreatedCallback=linkCreatedCallback;
    this->itemMovedCallback=itemMovedCallback;
    setFile(wfNode->handler->getImage());
    setText("title",wfNode->handler->getTitle());
    setText("description",wfNode->handler->getDescription());
    QStringList idsList=ids();
    for(int i=0;i<idsList.size();i++) {
        QString id=idsList[i];
        if(id.startsWith("out_") || id.startsWith("in_")) {
            int cx=getAttributeInt(id, "cx",0);
            int cy=getAttributeInt(id, "cy",0);
            int rx=getAttributeInt(id, "rx",0);
            int ry=getAttributeInt(id, "ry",0);
            QGraphicsEllipseItem*item;
            if(id.startsWith("in_")) {
               int index=id.mid(QString("in_").length()).toInt();
               InputPortItem*inputNode=new InputPortItem(cx-rx,cy-ry,rx*2,ry*2, index);
               inputNode->owner=this;
               inputs.append(inputNode);
               item=inputNode;
            } else {
                int index=id.mid(QString("out_").length()).toInt();
                OutputPortItem*outputNode=new OutputPortItem(cx-rx,cy-ry,rx*2,ry*2, index);
                outputNode->owner=this;
                outputs.append(outputNode);
                item=outputNode;
            }

            item->setParentItem(this);
            removeElement(id);
            update();
        }
    }

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    connect(this, &SvgItem::moved,[this]() {
        updateLinkPositions();
        this->itemMovedCallback(this);
    });
}

void WFNodeItem::updateAfterSettingsChange() {
    setText("title",node->handler->getTitle());
    setText("description",node->handler->getDescription());
    update();
}

void  WFNodeItem::updateLinkPositions() {
    foreach(InputPortItem*input, inputs) {
        foreach(LinkItem*link,input->inputLinks) {
            link->updatePosition();
        }
    }
    foreach(OutputPortItem*output, outputs) {
        foreach(LinkItem*link,output->outputLinks) {
            link->updatePosition();
        }
    }
}
