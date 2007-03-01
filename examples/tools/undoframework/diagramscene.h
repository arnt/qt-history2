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

#ifndef DIAGRAMSCENE_H
#define DIAGRAMSCENE_H

#include <QObject>
#include <QGraphicsScene>

class QGraphicsSceneDragDropEvent;
class DiagramItem;
class QGraphicsViewItem;

class DiagramScene : public QGraphicsScene
{
    Q_OBJECT

public:
    DiagramScene(QObject *parent = 0);

signals:
    void itemMoved(DiagramItem *movedItem, const QPointF &movedFromPosition);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

private:
    QGraphicsItem *movingItem;
    QPointF oldPos;
};

#endif
