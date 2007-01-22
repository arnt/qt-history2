#ifndef ARROW_H
#define ARROW_H

#include <QGraphicsLineItem>

#include "diagramitem.h"

class QGraphicsPolygonItem;
class QGraphicsLineItem;
class QGraphicsScene;
class QRectF;
class QGraphicsSceneMouseEvent;
class QPainterPath;

class Arrow : public QGraphicsLineItem
{
public:
    enum { Type = UserType + 4 };

    Arrow(DiagramItem *startItem, DiagramItem *endItem, 
	  QGraphicsItem *parent = 0, QGraphicsScene *scene = 0);

    int type() const
	{ return Type; }
    QRectF boundingRect() const;
    QPainterPath shape() const;
    void setColor(const QColor &color)
	{ myColor = color; }
    DiagramItem *startItem() const
	{ return myStartItem; }
    DiagramItem *endItem() const
	{ return myEndItem; }


public slots:
    void updatePosition();

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, 
	       QWidget *widget = 0);

private:
    DiagramItem *myStartItem;
    DiagramItem *myEndItem;
    QColor myColor;
    QPolygonF arrowHead;
};

#endif
