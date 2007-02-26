#include <QtGui>

#include "diagramscene.h"
#include "diagramitem.h"

void DiagramScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QPointF mousePos(event->buttonDownScenePos(Qt::LeftButton).x(), 
                     event->buttonDownScenePos(Qt::LeftButton).y());
    movingItem = itemAt(mousePos.x(), mousePos.y());

    if (movingItem != 0 && event->button() == Qt::LeftButton) {
	oldPos = movingItem->pos();
    }
    QGraphicsScene::mousePressEvent(event);
}

void DiagramScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (movingItem != 0 && event->button() == Qt::LeftButton) {
	if (oldPos != movingItem->pos())
	    emit itemMoved(qgraphicsitem_cast<DiagramItem *>(movingItem), 
			   oldPos);
	movingItem = 0;
    }
    QGraphicsScene::mouseReleaseEvent(event);
}
