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

    QColor color(qrand() % 256, qrand() % 256, qrand() % 256);
    QBrush brush(color);
    setBrush(brush);
    setFlag(QGraphicsItem::ItemIsSelectable);
    setFlag(QGraphicsItem::ItemIsMovable);
}
