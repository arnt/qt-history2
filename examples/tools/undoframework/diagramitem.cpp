#include <QtGui>

#include "diagramitem.h"

DiagramItem::DiagramItem(DiagramType diagramType, QGraphicsItem *item,
			 QGraphicsScene *scene)
    : QGraphicsPolygonItem(item, scene)
{
    if (diagramType == Box) {
	boxPolygon << QPointF(0, 0) << QPointF(0, 30) << QPointF(30, 30)
	           << QPointF(30, 0) << QPointF(0, 0);
	setPolygon(boxPolygon);
    } else {
	trianglePolygon << QPointF(15, 0) << QPointF(30, 30) << QPointF(0, 30)
	                << QPointF(15, 0);
	setPolygon(trianglePolygon);
    }

    QBrush brush(QColor(uint(qrand()) % 256, uint(qrand()) % 256, 
                        uint(qrand()) % 256));
    setBrush(brush);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
}
