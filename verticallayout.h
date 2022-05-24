#ifndef VERTICALLAYOUT_H
#define VERTICALLAYOUT_H
#include <QWidget>

class VerticalLayout
{
    int paddingTop=0;
    int paddingBottom=0;
    int paddingLeft=0;
    int paddingRight=0;
    int paddingBetween=0;
    QList<QWidget*>widgets;
    QWidget*container;
    int getBaseWidth() {
        int width=0;
        for(int i=0;i<widgets.size();i++) {
            QWidget*w=widgets[i];
            QSize size=w->baseSize();
            if(size.height()==0 || !w->isVisible()) {
                continue;
            }
            if(size.width()>width) {
                width=size.width();
            }
        }
        return width;
    }
public:
    VerticalLayout(QWidget*container) {this->container=container;}
    void setPaddingTop(int v) {paddingTop=v;}
    void setPaddingBottom(int v) {paddingBottom=v;}
    void setPaddingLeft(int v) {paddingLeft=v;}
    void setPaddingRight(int v) {paddingRight=v;}
    void setPaddingBetweenComponents(int v) {paddingBetween=v;}
    void setPaddings(int top, int bottom, int left, int right) {
        paddingTop=top;
        paddingBottom=bottom;
        paddingLeft=left;
        paddingRight=right;
    }
    void addWidget(QWidget*widget) {widgets.append(widget);}
    void pack() {
        int y=paddingTop;
        int x=paddingLeft;
        bool visibleComponentFound=false;
        int baseWidth=getBaseWidth();
        for(int i=0;i<widgets.size();i++) {
            QWidget*w=widgets[i];
            QSize size=w->baseSize();
            if(size.height()==0 || !w->isVisible()) {
                w->setGeometry(x, y, baseWidth, 0);
                continue;
            }

            if(visibleComponentFound) {
                y+=paddingBetween;
            }

            w->setGeometry(x, y, baseWidth, size.height());
            y+=size.height();
            visibleComponentFound=true;
        }
        y+=paddingBottom;
        container->resize(paddingLeft+paddingRight+baseWidth,y);
    }
};

#endif // VERTICALLAYOUT_H
