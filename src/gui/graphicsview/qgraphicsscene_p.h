/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
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

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

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
    int bspTreeDepth;

    QList<QGraphicsItem *> estimateItemsInRect(const QRectF &rect) const;
    void addToIndex(QGraphicsItem *item);
    void removeFromIndex(QGraphicsItem *item);
    void resetIndex();

    QGraphicsSceneBspTree bspTree;
    void _q_generateBspTree();
    int lastItemCount;

    QRectF sceneRect;
    bool hasSceneRect;
    QRectF growingItemsBoundingRect;
    QRectF largestUntransformableItem;

    void _q_emitUpdated();
    QList<QRectF> updatedRects;
    bool updateAll;
    bool calledEmitUpdated;

    QPainterPath selectionArea;
    int selectionChanging;
    QSet<QGraphicsItem *> selectedItems;
    QList<QGraphicsItem *> unindexedItems;
    QList<QGraphicsItem *> indexedItems;
    QList<QGraphicsItem *> pendingUpdateItems;
    QMap<QGraphicsItem *, QPointF> movingItemsInitialPositions;
    void _q_updateLater();

    QList<int> freeItemIndexes;
    bool regenerateIndex;

    bool purgePending;
    void _q_removeItemLater(QGraphicsItem *item);
    QSet<QGraphicsItem *> removedItems;
    void purgeRemovedItems();

    QBrush backgroundBrush;
    QBrush foregroundBrush;

    int indexTimerId;
    bool restartIndexTimer;
    void startIndexTimer();

    bool hasFocus;
    QGraphicsItem *focusItem;
    QGraphicsItem *lastFocusItem;
    QGraphicsItem *mouseGrabberItem;
    QGraphicsItem *lastMouseGrabberItem;
    QGraphicsItem *dragDropItem;
    Qt::DropAction lastDropAction;
    QList<QGraphicsItem *> cachedItemsUnderMouse;
    QList<QGraphicsItem *> hoverItems;
    QMap<Qt::MouseButton, QPointF> mouseGrabberButtonDownPos;
    QMap<Qt::MouseButton, QPointF> mouseGrabberButtonDownScenePos;
    QMap<Qt::MouseButton, QPoint> mouseGrabberButtonDownScreenPos;
    QList<QGraphicsItem *> itemsAtPosition(const QPoint &screenPos,
                                           const QPointF &scenePos,
                                           QWidget *widget) const;
    void storeMouseButtonsForMouseGrabber(QGraphicsSceneMouseEvent *event);

    QList<QGraphicsView *> views;
    bool painterStateProtection(const QPainter *painter) const;

    QMultiMap<QGraphicsItem *, QGraphicsItem *> sceneEventFilters;
    void installSceneEventFilter(QGraphicsItem *watched, QGraphicsItem *filter);
    void removeSceneEventFilter(QGraphicsItem *watched, QGraphicsItem *filter);
    bool filterEvent(QGraphicsItem *item, QEvent *event);
    bool sendEvent(QGraphicsItem *item, QEvent *event);

    bool dispatchHoverEvent(QGraphicsSceneHoverEvent *hoverEvent);
    void leaveScene();

    void cloneDragDropEvent(QGraphicsSceneDragDropEvent *dest,
                           QGraphicsSceneDragDropEvent *source);
    void sendDragDropEvent(QGraphicsItem *item,
                           QGraphicsSceneDragDropEvent *dragDropEvent);
    void sendHoverEvent(QEvent::Type type, QGraphicsItem *item,
                        QGraphicsSceneHoverEvent *hoverEvent);
    void sendMouseEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void mousePressEventHandler(QGraphicsSceneMouseEvent *mouseEvent);

    static void sortItems(QList<QGraphicsItem *> *itemList);

    static void drawItemHelper(QGraphicsItem *item, QPainter *painter,
                               const QStyleOptionGraphicsItem *option, QWidget *widget);
};

#endif // QT_NO_GRAPHICSVIEW

#endif
