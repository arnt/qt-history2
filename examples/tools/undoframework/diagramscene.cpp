/****************************************************************************
**
** Copyright (C) 2007-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>

#include "diagramscene.h"
#include "diagramitem.h"

DiagramScene::DiagramScene(QObject *parent)
    : QGraphicsScene(parent)
{
    movingItem = 0;
}

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
