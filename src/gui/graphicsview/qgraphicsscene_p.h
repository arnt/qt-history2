/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGRAPHICSSCENE_P_H
#define QGRAPHICSSCENE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qapplication_*.cpp, qwidget*.cpp and qfiledialog.cpp.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#include "qgraphicsscene.h"

#ifndef QT_NO_GRAPHICSVIEW

#include "qgraphicsscene_bsp_p.h"

#include <private/qobject_p.h>
#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qset.h>

class QGraphicsView;

class QGraphicsScenePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsScene)
public:
    QGraphicsScenePrivate();
    
    QGraphicsScene::ItemIndexMethod indexMethod;
    QList<QGraphicsItem *> estimateItemsInRect(const QRectF &rect) const;
    void addToIndex(QGraphicsItem *item);
    void removeFromIndex(QGraphicsItem *item);
    void resetIndex();
    
    QGraphicsSceneBspTree bspTree;
    mutable bool generatingBspTree;
    void generateBspTree();

    QRectF sceneRect;
    bool hasSceneRect;
    QRectF growingItemsBoundingRect;
    
    void emitUpdated();
    QList<QRectF> updatedRects;
    bool calledEmitUpdated;
    
    QList<QGraphicsItem *> newItems;
    QList<QGraphicsItem *> selectedItems;
    QList<QGraphicsItem *> allItems;
    QList<int> freeItemIndexes;
    void removeItemLater(QGraphicsItem *item);
    QList<QGraphicsItem *> validItems() const;
    QSet<QGraphicsItem *> removedItems;

    QBrush backgroundBrush;
    QBrush foregroundBrush;
    
    bool hasFocus;
    QGraphicsItem *focusItem;
    QGraphicsItem *lastFocusItem;
    QGraphicsItem *mouseGrabberItem;
    QGraphicsItem *lastMouseGrabberItem;
    QGraphicsItem *dragDropItem;
    Qt::DropAction lastDropAction;
    QList<QGraphicsItem *> hoverItems;
    QMap<Qt::MouseButton, QPointF> mouseGrabberButtonDownPos;
    QMap<Qt::MouseButton, QPointF> mouseGrabberButtonDownScenePos;
    QMap<Qt::MouseButton, QPoint> mouseGrabberButtonDownScreenPos;
    QList<QGraphicsItem *> possibleMouseGrabbersForEvent(QGraphicsSceneMouseEvent *event);
    void storeMouseButtonsForMouseGrabber(QGraphicsSceneMouseEvent *event);

    QList<QGraphicsView *> views;

    QMultiMap<QGraphicsItem *, QGraphicsItem *> eventFilters;
    void installEventFilter(QGraphicsItem *watched, QGraphicsItem *filter);
    void removeEventFilter(QGraphicsItem *watched, QGraphicsItem *filter);
    bool filterEvent(QGraphicsItem *item, QGraphicsSceneEvent *event);

    void dispatchHoverEvent(QGraphicsSceneHoverEvent *hoverEvent);

    void cloneDragDropEvent(QGraphicsSceneDragDropEvent *dest,
                           QGraphicsSceneDragDropEvent *source);
    void sendDragDropEvent(QGraphicsItem *item,
                           QGraphicsSceneDragDropEvent *dragDropEvent);
    void sendHoverEvent(QEvent::Type type, QGraphicsItem *item,
                        QGraphicsSceneHoverEvent *hoverEvent);
    void sendMouseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEventHandler(QGraphicsSceneMouseEvent *mouseEvent);

    static bool sortItems(QList<QGraphicsItem *> *itemList);
};

#endif // QT_NO_GRAPHICSVIEW

#endif
