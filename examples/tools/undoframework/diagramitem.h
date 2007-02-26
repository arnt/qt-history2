#ifndef DIAGRAMITEM_H
#define DIAGRAMITEM_H

#include <QGraphicsPolygonItem>

class QGraphicsItem;
class QGraphicsScene;
class QGraphicsSceneMouseEvent;
class QPointF;

class DiagramItem : public QGraphicsPolygonItem
{
public:
    enum { Type = UserType + 1 };
    enum DiagramType { Box, Triangle };

    DiagramItem(DiagramType diagramType, QGraphicsItem *item = 0, 
	        QGraphicsScene *scene = 0);
    
    DiagramType diagramType() const {
	return polygon() == boxPolygon ? Box : Triangle;
    }
    int type() const { return Type; }

private:
    QPolygonF boxPolygon;
    QPolygonF trianglePolygon;
};

#endif
