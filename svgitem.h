#ifndef SVGITEM_H
#define SVGITEM_H

#include <QDebug>
#include <QDomElement>
#include <QFile>
#include <QGraphicsSvgItem>
#include <QSvgRenderer>

class SvgItem : public QGraphicsSvgItem {
    Q_OBJECT
    QMap<QString, QDomElement>id2domElement;
    QDomDocument svgXML;

    QVariant itemChange( GraphicsItemChange change, const QVariant&value) {
      if (change == ItemPositionHasChanged) {
        QPointF pos=value.toPointF();
        if(pos.x()<0 || pos.y()<0) {
            int moveX=0;
            int moveY=0;
            if(pos.x()<0)moveX=-pos.x();
            if(pos.y()<0)moveY=-pos.y();
            moveBy(moveX, moveY);
        } else {
            emit moved(pos);
        }
      }
      return QGraphicsSvgItem::itemChange(change, value);
    }

signals:
  void moved(QPointF p);

public:
    SvgItem(QString filePath, QGraphicsItem *parent=NULL): QGraphicsSvgItem(parent), svgXML() {
        setFile(filePath);
    }

    SvgItem(QGraphicsItem *parent=NULL): QGraphicsSvgItem(parent), svgXML() {
    }

    void setFile(QString filePath) {
        QFile file(filePath);
        QString svgContent="";
        if(!file.open(QIODevice::ReadOnly)) {
            qDebug()<<"file "<<filePath<<" cannot be opened";
        } else {
            svgContent = file.readAll();
        }
        svgXML.setContent(svgContent);
        setSharedRenderer(new QSvgRenderer(svgXML.toByteArray()));
        recursiveWalk(svgXML.documentElement());
    }

    QStringList ids() {
        return id2domElement.keys();
    }

    QDomElement getElementById(QString id) {
        return id2domElement[id];
    }
    void setAttribute(QString id, QString name, QString value, bool updateNode=false) {
        id2domElement[id].setAttribute(name, value);
        if(updateNode) {
            update();
        }
    }
    void setAttribute(QString id, QString name, int value, bool updateNode=false) {
        id2domElement[id].setAttribute(name, value);
        if(updateNode) {
            update();
        }
    }
    void setAttribute(QString id, QString name, float value, bool updateNode=false) {
        id2domElement[id].setAttribute(name, value);
        if(updateNode) {
            update();
        }
    }
    QString getAttribute(QString id, QString name, QString defaultValue) {
        return id2domElement[id].attribute(name, defaultValue);
    }
    int getAttributeInt(QString id, QString name, int defaultValue) {
        return id2domElement[id].attribute(name, QString::number(defaultValue)).toInt();
    }

    void removeElement(QString id) {
        id2domElement[id].parentNode().removeChild(id2domElement[id]);
    }

    void setText(QString id, QString text, bool updateNode=false) {
        QDomElement domElement=getElementById(id);
        QDomNodeList children= domElement.childNodes();
        for(int i=0;i<children.count();i++) {
            domElement.removeChild(children.at(i));
        }
        QDomText t = svgXML.createTextNode(text);
        domElement.appendChild(t);
        if(updateNode) {
            update();
        }
    }

    void update() {
        renderer()->load(svgXML.toByteArray());
    }
private:
    void recursiveWalk(const QDomElement&domElement) {
        if(domElement.hasAttribute("id")) {
            QString id=domElement.attribute("id");
            id2domElement[id]=domElement;
        }
        if(domElement.hasChildNodes()) {
            QDomNode n = domElement.firstChild();
            while(!n.isNull()) {
                if(n.isElement()) {
                    QDomElement childElement=n.toElement();
                    recursiveWalk(childElement);
                }
                n = n.nextSibling();
            }
        }
    }
};
#endif // SVGITEM_H
