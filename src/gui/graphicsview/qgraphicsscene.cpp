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

/*!
    \class QGraphicsScene

    \brief The QGraphicsScene class provides a surface for managing a large
    number of 2D graphical items.

    The class serves as a container for QGraphicsItems. It is used together
    with QGraphicsView for visualizing graphical items, such as lines,
    rectangles, text, or even custom items, on a 2D surface. It also provides
    functionality that lets you efficiently determine both the location of
    items, and for determining what items are visible within an arbitrary area
    on the scene. With the QGraphicsView widget, you can either visualize the
    whole scene, or zoom in and view only parts of the scene.

    Example:

    \code
        QGraphicsScene scene;
        scene.addText("Hello, world!");

        QGraphicsView view(&scene);
        view.show();
    \endcode

    Note that QGraphicsScene has no visual appearance of its own; it only
    manages the items. You need to create a QGraphicsView widget to visualize
    the scene.

    To add items to a scene, you start off by constructing a QGraphicsScene
    object. Then, you have two options: either add your existing QGraphicsItem
    objects by calling addItem(), or you can call one of the convenience
    functions addEllipse(), addLine(), addPath(), addPixmap(), addPolygon(),
    addRect(), or addText(), which all return a pointer to the newly added
    item. You can then visualize the scene using QGraphicsView. When the scene
    changes, (e.g., when an item moves or is transformed) QGraphicsScene emits
    the changed() signal. To remove an item, call removeItem().

    QGraphicsScene uses an indexing algorithm to manage the location of items
    efficiently. By default, a BSP (Binary Space Partitioning) tree is used; an
    algorithm suitable for large scenes where most items remain static (i.e.,
    do not move around). You can choose to disable this index by calling
    setItemIndexMethod(). For more information about the available indexing
    algorithms, see the itemIndexMethod propery.

    The scene's bounding rect is set by calling setSceneRect(). Items can be
    placed at any position on the scene, and the size of the scene is by
    default unlimited. The scene rect is used only for internal bookkeeping,
    maintaining the scene's item index. If the scene rect is unset,
    QGraphicsScene will use the bounding area of all items, as returned by
    itemsBoundingRect(), as the scene rect. However, itemsBoundingRect() is a
    relatively time consuming function, as it operates by collecting
    positional information for every item on the scene. Because of this, you
    should always set the scene rect when operating on large scenes.

    One of QGraphicsScene's greatest strengts is its ability to efficiently
    determine the location of items. Even with millions of items on the scene,
    the items() functions can determine the location of an item within few
    milliseconds. There are several overloads to items(): one that finds items
    at a certain position, one that finds items inside or intersecting with a
    polygon or a rectangle, and more. The list of returned items is sorted by
    stacking order, with the topmost item being the first item in the list.
    For convenience, there is also an itemAt() function that returns the
    topmost item at a given position.

    QGraphicsScene maintains selection information for the scene. To select
    items, call setSelectionArea(), and to clear the current selection, call
    clearSelection(). Call selectedItems() to get the list of all selected
    items.

    Another responsibility that QGraphicsScene has, is to propagate events
    from QGraphicsView. To send an event to a scene, you construct an event
    that inherits QEvent, and then send it using, for example,
    QApplication::sendEvent(). event() is responsible for dispatching
    the event to the individual items. Some common events are handled by
    convenience event handlers. For example, key press events are handled by
    keyPressEvent(), and mouse press events are handled by mousePressEvent().

    Key events are delivered to the \e {focus item}. To set the focus item,
    you can either call setFocusItem(), passing an item that accepts focus, or
    the item itself can call QGraphicsItem::setFocus().  Call focusItem() to
    get the current focus item. For compatibility with widgets, the scene also
    maintains its own focus information. By default, the scene does not have
    focus, and all key events are discarded. If setFocus() is called, or if an
    item on the scene gains focus, the scene automatically gains focus. If the
    scene has focus, hasFocus() will return true, and key events will be
    forwarded to the focus item, if any. If the scene loses focus, (i.e.,
    someone calls clearFocus(),) while an item has focus, the scene will
    maintain its item focus information, and once the scene regains focus, it
    will make sure the last focus item regains focus.

    For mouse-over effects, QGraphicsScene dispatches \e {hover events}. If an
    item accepts hover events (see QGraphicsItem::acceptsHoverEvents), it will
    receive a HoverEnter event when the mouse enters its area. As the mouse
    continues moving inside the item's area, QGraphicsScene will send it
    HoverMove events. When the mouse leaves the item's area, the item will
    receive a HoverLeave event. A special rule applies when items are stacked
    on top of eachother: if the mouse enters an item that is on top of the
    last item that received a HoverEnter, the first item will not receive a
    HoverLeave until the mouse leaves the last stacked item. ###

    All mouse events are delivered to the current \e {mouse grabber} item. An
    item becomes the scene's mouse grabber if it accepts mouse events (see
    QGraphicsItem::acceptsMouseEvents), and it receives a mouse press. It
    stays the mouse grabber until it receives a mouse release when no other
    mouse buttons are pressed. You can call mouseGrabberItem() to determine
    what item is currently grabbing the mouse.

    \sa QGraphicsItem, QGraphicsView
*/

/*!
    \enum QGraphicsScene::ItemIndexMethod

    This enum describes the indexing algorithms QGraphicsScene provides for
    managing positional information about items on the scene.

    \value BspTreeIndex A Binary Space Partitioning tree is applied. All
    QGraphicsScene's item location algorithms are of an order close to
    logarithmic complexity, by making use of binary search. Adding, moving and
    removing items is logarithmic. This approach is best for static scenes
    (i.e., scenes where most items do not move).

    \value NoIndex No index is applied. Item location is of linear complexity,
    as all items on the scene are searched. Adding, moving and removing items,
    however, is done in constant time. This approach is ideal for dynamic
    scenes, where many items are added, moved or removed continuously.
*/

#include "qgraphicsscene.h"

#ifndef QT_NO_GRAPHICSVIEW

#include "qgraphicsitem.h"
#include "qgraphicsitem_p.h"
#include "qgraphicsscene_p.h"
#include "qgraphicssceneevent.h"

#include <private/qobject_p.h>
#include <QtCore/qcoreapplication.h>
#include <QtCore/qlist.h>
#include <QtCore/qrect.h>
#include <QtCore/qset.h>
#include <QtCore/qtimer.h>
#include <QtGui/qevent.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpolygon.h>
#include <QtGui/qstyleoption.h>
#include <QtGui/qtooltip.h>
#include <math.h>
#include <qdebug.h>

static bool qt_rectInPoly(const QPolygonF &poly, const QRectF &rect)
{
    QPainterPath path;
    path.addPolygon(poly);
    return path.contains(rect)|| path.intersects(rect);
};

static int closestPowerOf2(int input)
{
    int n = input;
    int previousPowerOf2;
    do {
        previousPowerOf2 = n;
    } while (n &= n-1);
    return (n - previousPowerOf2) < ((previousPowerOf2 * 2) - n)
        ? previousPowerOf2 : previousPowerOf2 * 2;
}

static void qt_itemsForRectCallback(QVector<int> &leaf, const QRectF & /* area */,
                                 uint /* visited */, QGraphicsSceneBspTree::Data data)
{
    QVector<int> *listPtr = (QVector<int> *)data.ptr;
    *listPtr += leaf;
}

/*!
    \internal
*/
QGraphicsScenePrivate::QGraphicsScenePrivate()
    : indexMethod(QGraphicsScene::BspTreeIndex), generatingBspTree(false),
      hasSceneRect(false), calledEmitUpdated(false), hasFocus(false),
      focusItem(0), lastFocusItem(0), mouseGrabberItem(0),
      lastMouseGrabberItem(0), dragDropItem(0), lastDropAction(Qt::IgnoreAction)
{
}

/*!
    \internal
*/
QList<QGraphicsItem *> QGraphicsScenePrivate::estimateItemsInRect(const QRectF &rect) const
{
    QList<QGraphicsItem *> itemsInRect;

    if (indexMethod == QGraphicsScene::BspTreeIndex) {
        QGraphicsScenePrivate *that = const_cast<QGraphicsScenePrivate *>(this);
        that->generateBspTree();
        QVector<int> itemIndexes;
        that->bspTree.climbTree(rect.toRect(), &qt_itemsForRectCallback, QGraphicsSceneBspTree::Data(&itemIndexes));

        for (int i = 0; i < itemIndexes.size(); ++i) {
            int itemIndex = itemIndexes.at(i);
            QGraphicsItem *item = allItems.at(itemIndexes.at(i));
            if (!item->d_func()->itemDiscovered) {
                item->d_func()->itemDiscovered = 1;
                itemsInRect << allItems.at(itemIndex);
            }
        }
        for (int i = 0; i < itemIndexes.size(); ++i)
            allItems.at(itemIndexes.at(i))->d_func()->itemDiscovered = 0;
    } else {
        foreach (QGraphicsItem *item, validItems()) {
            QRectF boundingRect = item->sceneBoundingRect();
            if (boundingRect.intersects(rect) || boundingRect.contains(rect))
                itemsInRect << item;
        }
    }

    return itemsInRect;
}

/*!
    \internal
*/
void QGraphicsScenePrivate::addToIndex(QGraphicsItem *item)
{
    Q_Q(QGraphicsScene);
    if (indexMethod == QGraphicsScene::BspTreeIndex) {
        if (item->d_func()->index != -1) {
            bspTree.insertLeaf(item->sceneBoundingRect(), item->d_func()->index);
            foreach (QGraphicsItem *child, item->children())
                child->addToIndex();
        } else {
            // The BSP tree is regenerated if the number of items grows to a
            // certain threshold, or if the bounding rect of the graph doubles in
            // size.
            if (!generatingBspTree) {
                generatingBspTree = true;
                QTimer::singleShot(0, q, SLOT(generateBspTree()));
            }
        }
    }
}

/*!
    \internal
*/
void QGraphicsScenePrivate::removeFromIndex(QGraphicsItem *item)
{
    if (indexMethod == QGraphicsScene::BspTreeIndex) {
        if (item->d_func()->index != -1) {
            bspTree.removeLeaf(item->sceneBoundingRect(), item->d_func()->index);
            foreach (QGraphicsItem *child, item->children())
                child->removeFromIndex();
        }
    }
}

/*!
    \internal
*/
void QGraphicsScenePrivate::resetIndex()
{
    Q_Q(QGraphicsScene);
    if (indexMethod == QGraphicsScene::BspTreeIndex) {
        bspTree.destroy();
        newItems = validItems();
        allItems.clear();
        freeItemIndexes.clear();
        foreach (QGraphicsItem *item, newItems)
            item->d_func()->index = -1;

        if (!generatingBspTree) {
            generatingBspTree = true;
            QTimer::singleShot(0, q, SLOT(generateBspTree()));
        }
    }
    removedItems.clear();
}

/*!
    \internal
*/
void QGraphicsScenePrivate::generateBspTree()
{
    Q_Q(QGraphicsScene);

    if (!generatingBspTree)
        return;
    generatingBspTree = false;

    // ### newitems
    for (int i = 0; i < allItems.size(); ++i) {
        if (removedItems.contains(allItems.at(i))) {
            allItems[i] = 0;
            freeItemIndexes << i;
        }
    }

    int oldItemCount = allItems.size();

    for (int i = 0; i < newItems.size(); ++i) {
        QGraphicsItem *item = newItems.at(i);
        if (item && !removedItems.contains(item)) {
            if (!freeItemIndexes.isEmpty()) {
                int freeIndex = freeItemIndexes.takeFirst();
                item->d_func()->index = freeIndex;
                allItems[freeIndex] = item;
            } else {
                item->d_func()->index = allItems.size();
                allItems << item;
            }
        }
    }

    if (closestPowerOf2(oldItemCount) < closestPowerOf2(allItems.size())) {
        // ### Use a better algorithm that isn't so vulnerable to border-case
        // slowness.
        // Recreate the bsptree whenever the number of new items crosses a
        // quadratic number.
        bspTree.destroy();
        bspTree.create(allItems.size());
        bspTree.init(q->sceneRect(), QGraphicsSceneBspTree::Node::Both);

        for (int i = 0; i < allItems.size(); ++i) {
            if (QGraphicsItem *item = allItems.at(i)) {
                if (!removedItems.contains(item)) {
                    bspTree.insertLeaf(item->sceneBoundingRect(), i);
                    item->d_func()->index = i;
                }
            }
        }
    } else {
        // Otherwise, just add the items to the tree.
        for (int i = 0; i < newItems.size(); ++i) {
            if (QGraphicsItem *item = newItems.at(i)) {
                if (!removedItems.contains(item)) {
                    bspTree.insertLeaf(item->sceneBoundingRect(),
                                       item->d_func()->index);
                }
            }
        }
    }

    newItems.clear();
    emit q->changed(QList<QRectF>() << q->sceneRect());
}

/*!
    \internal
*/
void QGraphicsScenePrivate::emitUpdated()
{
    Q_Q(QGraphicsScene);
    calledEmitUpdated = false;
    QList<QRectF> oldUpdatedRects = updatedRects;
    updatedRects.clear();
    emit q->changed(oldUpdatedRects);
}

/*!
    \internal
*/
void QGraphicsScenePrivate::removeItemLater(QGraphicsItem *item)
{
    if (item == mouseGrabberItem) {
        lastMouseGrabberItem = mouseGrabberItem;
        mouseGrabberItem = 0;
    }
    if (item == focusItem)
        focusItem = 0;
    if (item == lastFocusItem)
        lastFocusItem = 0;
    if (removedItems.isEmpty())
        resetIndex();
    if (QGraphicsItem *parent = item->d_func()->parent) {
        parent->d_func()->children.removeAll(item);
        item->d_func()->parent = 0;
        item->d_func()->scene = 0;
    }
    removedItems << item;
    foreach (QGraphicsItem *child, item->children())
        removeItemLater(child);
}

/*!
    \internal
*/
QList<QGraphicsItem *> QGraphicsScenePrivate::validItems() const
{
    QList<QGraphicsItem *> items;
    foreach (QGraphicsItem *item, allItems) {
        if (item && !removedItems.contains(item))
            items << item;
    }
    foreach (QGraphicsItem *item, newItems) {
        if (item && !removedItems.contains(item))
            items << item;
    }
    return items;
}

/*!
    \internal

    Returns a list of possible mouse grabbers for \a event. The first item in
    the list is the topmost candidate, and the last item is the bottommost
    candidate.
*/
QList<QGraphicsItem *> QGraphicsScenePrivate::possibleMouseGrabbersForEvent(QGraphicsSceneMouseEvent *event)
{
    Q_Q(QGraphicsScene);
    QList<QGraphicsItem *> possibleMouseGrabbers;
    foreach (QGraphicsItem *item, q->items(event->scenePos())) {
        if ((item->acceptedMouseButtons() & event->button())
            && item->contains(item->mapFromScene(event->scenePos()))) {
            if (!item->isEnabled()) {
                // Disabled mouse-accepting items discard mouse events.
                break;
            }
            possibleMouseGrabbers << item;
        }
    }
    return possibleMouseGrabbers;
}

/*!
    \internal
*/
void QGraphicsScenePrivate::storeMouseButtonsForMouseGrabber(QGraphicsSceneMouseEvent *event)
{
    for (int i = 0x1; i <= 0x10; i <<= 1) {
        if (event->buttons() & i) {
            mouseGrabberButtonDownPos.insert(Qt::MouseButton(i),
                                             mouseGrabberItem->mapFromScene(event->scenePos()));
            mouseGrabberButtonDownScenePos.insert(Qt::MouseButton(i), event->scenePos());
            mouseGrabberButtonDownScreenPos.insert(Qt::MouseButton(i), event->screenPos());
        }
    }
}

/*!
    \internal
*/
void QGraphicsScenePrivate::installEventFilter(QGraphicsItem *watched, QGraphicsItem *filter)
{
    eventFilters.insert(watched, filter);
}

/*!
    \internal
*/
void QGraphicsScenePrivate::removeEventFilter(QGraphicsItem *watched, QGraphicsItem *filter)
{
    if (!eventFilters.contains(watched))
        return;

    QMultiMap<QGraphicsItem *, QGraphicsItem *>::Iterator it = eventFilters.lowerBound(watched);
    QMultiMap<QGraphicsItem *, QGraphicsItem *>::Iterator end = eventFilters.upperBound(watched);
    do {
        if (it.value() == filter)
            eventFilters.erase(it);
    } while (++it != end);
}

/*!
    \internal
*/
bool QGraphicsScenePrivate::filterEvent(QGraphicsItem *item, QGraphicsSceneEvent *event)
{
    if (!eventFilters.contains(item))
        return false;

    QMultiMap<QGraphicsItem *, QGraphicsItem *>::Iterator it = eventFilters.lowerBound(item);
    QMultiMap<QGraphicsItem *, QGraphicsItem *>::Iterator end = eventFilters.upperBound(item);
    while (it != end) {
        if (it.value()->sceneEventFilter(it.key(), event))
            return true;
        ++it;
    }
    return false;
}

/*!
    \internal
*/
void QGraphicsScenePrivate::cloneDragDropEvent(QGraphicsSceneDragDropEvent *dest,
                                               QGraphicsSceneDragDropEvent *source)
{
    dest->setWidget(source->widget());
    dest->setPos(source->pos());
    dest->setScenePos(source->scenePos());
    dest->setScreenPos(source->screenPos());
    dest->setButtons(source->buttons());
    dest->setModifiers(source->modifiers());
    dest->setPossibleActions(source->possibleActions());
    dest->setProposedAction(source->proposedAction());
    dest->setDropAction(source->dropAction());
    dest->setSource(source->source());
    dest->setMimeData(source->mimeData());
}

/*!
    \internal
*/
void QGraphicsScenePrivate::sendDragDropEvent(QGraphicsItem *item,
                                              QGraphicsSceneDragDropEvent *dragDropEvent)
{
    dragDropEvent->setPos(item->mapFromScene(dragDropEvent->scenePos()));
    item->sceneEvent(dragDropEvent);
}

/*!
    \internal
*/
void QGraphicsScenePrivate::sendHoverEvent(QEvent::Type type, QGraphicsItem *item,
                                           QGraphicsSceneHoverEvent *hoverEvent)
{
    QGraphicsSceneHoverEvent event(type);
    event.setWidget(hoverEvent->widget());
    event.setPos(item->mapFromScene(hoverEvent->scenePos()));
    event.setScenePos(hoverEvent->scenePos());
    event.setScreenPos(hoverEvent->screenPos());
    item->sceneEvent(&event);
}

/*!
    \internal
*/
void QGraphicsScenePrivate::sendMouseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    for (int i = 0x1; i <= 0x10; i <<= 1) {
        if (mouseEvent->buttons() & i) {
            Qt::MouseButton button = Qt::MouseButton(i);
            mouseEvent->setButtonDownPos(button, mouseGrabberButtonDownPos.value(button, mouseGrabberItem->mapFromScene(mouseEvent->scenePos())));
            mouseEvent->setButtonDownScenePos(button, mouseGrabberButtonDownScenePos.value(button, mouseEvent->scenePos()));
            mouseEvent->setButtonDownScreenPos(button, mouseGrabberButtonDownScreenPos.value(button, mouseEvent->screenPos()));
        }
    }
    mouseEvent->setPos(mouseGrabberItem->mapFromScene(mouseEvent->scenePos()));
    mouseEvent->setLastPos(mouseGrabberItem->mapFromScene(mouseEvent->lastScenePos()));
    mouseGrabberItem->sceneEvent(mouseEvent);
}

/*!
    \internal
*/
void QGraphicsScenePrivate::mousePressEventHandler(QGraphicsSceneMouseEvent *mouseEvent)
{
    Q_Q(QGraphicsScene);
    if (mouseGrabberItem) {
        // We already have a mouse grabber. This means more than one button is
        // pressed at the same time. This is just delivered like a normal
        // event.
        sendMouseEvent(mouseEvent);
        return;
    }

    // Ignore by default, unless we find a mouse grabber that accepts it.
    mouseEvent->ignore();

    // Set focus on the topmost enabled item that can take focus.
    foreach (QGraphicsItem *item, q->items(mouseEvent->scenePos())) {
        if (item->isEnabled() && (item->flags() & QGraphicsItem::ItemIsFocusable)) {
            if (item != q->focusItem())
                q->setFocusItem(item, Qt::MouseFocusReason);
            break;
        }
    }

    // Find a mouse grabber by sending mouse press events to all mouse grabber
    // candidates one at a time, until the event is accepted. It's accepted by
    // default, so the receiver has to explicitly ignore it for it to pass
    // through.
    foreach (QGraphicsItem *item, possibleMouseGrabbersForEvent(mouseEvent)) {
        mouseGrabberItem = item;
        mouseEvent->accept();

        if (mouseEvent->type() == QEvent::GraphicsSceneMouseDoubleClick && item != lastMouseGrabberItem) {
            // If this item is different from the item that received the last
            // mouse event, and mouseEvent is a doubleclick event, then the
            // event is converted to a press. Known limitation:
            // Triple-clicking will not generate a doubleclick, though.
            QGraphicsSceneMouseEvent mousePress(QEvent::GraphicsSceneMousePress);
            mousePress.accept();
            mousePress.setButton(mouseEvent->button());
            mousePress.setButtons(mouseEvent->buttons());
            mousePress.setScreenPos(mouseEvent->screenPos());
            mousePress.setScenePos(mouseEvent->scenePos());
            mousePress.setModifiers(mouseEvent->modifiers());
            sendMouseEvent(&mousePress);
            mouseEvent->setAccepted(mousePress.isAccepted());
        } else {
            sendMouseEvent(mouseEvent);
        }
        if (mouseEvent->isAccepted()) {
            storeMouseButtonsForMouseGrabber(mouseEvent);
            lastMouseGrabberItem = mouseGrabberItem;
            return;
        }
    }

    // Is the event still ignored? Then the mouse press goes to the scene.
    // Reset the mouse grabber, clear the selection, clear focus, and leave
    // the event ignored so that it can propagate through the originating
    // view.
    if (!mouseEvent->isAccepted()) {
        lastMouseGrabberItem = mouseGrabberItem;
        q->clearSelection();
        q->setFocusItem(0, Qt::MouseFocusReason);
    }
}

/*!
    \internal
*/
static inline bool qt_closestLeaf(const QGraphicsItem *item1, const QGraphicsItem *item2)
{
    qreal z1 = item1->zValue();
    qreal z2 = item2->zValue();
    return z1 != z2 ? z1 > z2 : item1 > item2;
}

/*!
    \internal
*/
static bool qt_closestItemFirst(const QGraphicsItem *item1, const QGraphicsItem *item2)
{
    // Siblings? Just check their z-values.
    if (item1->parentItem() == item2->parentItem())
        return qt_closestLeaf(item1, item2);

    // Find item1's ancestors. If item2 is among them, return true (item1 is
    // above item2).
    QVector<const QGraphicsItem *> ancestors1;
    const QGraphicsItem *parent1 = item1;
    do {
        if (parent1 == item2)
            return true;
        ancestors1.prepend(parent1);
    } while ((parent1 = parent1->parentItem()));

    // Find item2's ancestors. If item1 is among them, return false (item2 is
    // above item1).
    QVector<const QGraphicsItem *> ancestors2;
    const QGraphicsItem *parent2 = item2;
    do {
        if (parent2 == item1)
            return false;
        ancestors2.prepend(parent2);
    } while ((parent2 = parent2->parentItem()));

    // Truncate the largest ancestor list.
    int size1 = ancestors1.size();
    int size2 = ancestors2.size();
    if (size1 > size2) {
        ancestors1.resize(size2);
    } else if (size2 > size1) {
        ancestors2.resize(size1);
    }

    // Compare items from the two ancestors lists and find a match. Then
    // compare item1's and item2's toplevels relative to the common ancestor.
    for (int i = ancestors1.size() - 2; i >= 0; --i) {
        const QGraphicsItem *a1 = ancestors1.at(i);
        const QGraphicsItem *a2 = ancestors2.at(i);
        if (a1 == a2)
            return qt_closestLeaf(ancestors1.at(i + 1), ancestors2.at(i + 1));
    }

    // No common ancestor? Then just compare the items' toplevels directly.
    return qt_closestLeaf(ancestors1.first(), ancestors2.first());
}

/*!
    \internal
*/
void QGraphicsScenePrivate::sortItems(QList<QGraphicsItem *> *itemList)
{
    qSort(itemList->begin(), itemList->end(), qt_closestItemFirst);
}

/*!
    Constructs a QGraphicsScene object. \a parent is passed to
    QObject's constructor.
*/
QGraphicsScene::QGraphicsScene(QObject *parent)
    : QObject(*new QGraphicsScenePrivate, parent)
{
}

/*!
    Destroys the QGraphicsScene object.
*/
QGraphicsScene::~QGraphicsScene()
{
    Q_D(QGraphicsScene);
    for (int i = 0; i < d->newItems.size(); ++i) {
        if (QGraphicsItem *item = d->newItems[i]) {
            if (!d->removedItems.contains(item)) {
                d->newItems[i] = 0;
                d->removeFromIndex(item);
                item->d_func()->scene = 0;
                delete item;
            }
        }
    }
    for (int i = 0; i < d->allItems.size(); ++i) {
        if (QGraphicsItem *item = d->allItems[i]) {
            if (!d->removedItems.contains(item)) {
                d->allItems[i] = 0;
                d->removeFromIndex(item);
                item->d_func()->scene = 0;
                delete item;
            }
        }
    }
}

/*!
    \property QGraphicsScene::sceneRect
    \brief the scene rect; the bounding rect of the scene

    The scene rect defines the extent of the scene. It is primarily used by
    QGraphicsView to determine the view's default scrollable area, and by
    QGraphicsScene to manage item indexing.

    If unset, sceneRect() will return the largest bounding rect of all items
    on the scene since the scene was created (i.e., a rectangle that grows
    when items are added to or moved in the scene, but never shrinks).

    \sa QGraphicsView::sceneRect
*/
QRectF QGraphicsScene::sceneRect() const
{
    Q_D(const QGraphicsScene);
    return d->hasSceneRect ? d->sceneRect : d->growingItemsBoundingRect;
}
void QGraphicsScene::setSceneRect(const QRectF &rect)
{
    Q_D(QGraphicsScene);
    if (rect != d->sceneRect) {
        d->hasSceneRect = !rect.isNull();
        d->sceneRect = rect;
        emit sceneRectChanged(rect);
    }
}

/*!
    Renders the \a source rect from scene into \a target, using \a painter. This
    function is useful for capturing the contents of the scene onto a paint
    device, such as a QImage (e.g., to take a screenshot), or for printing
    with QPrinter. For example:

    \code
        QGraphicsScene scene;
        scene.addItem(...
        ...
        QPrinter printer(QPrinter::HighResolution);
        printer.setPageSize(QPrinter::A4);

        QPainter painter(&printer);
        scene.render(&painter);
    \endcode

    If \a source is a null rect, this function will use sceneRect() to
    determine what to render. If \a target is a null rect, the dimensions of \a
    painter's paint device will be used.

    The source rect contents will be transformed according to \a
    aspectRatioMode to fit into the target rect. By default, the aspect ratio
    is ignored, and \a source is scaled to fit tightly in \a target.

    \sa QGraphicsView::render()
*/
void QGraphicsScene::render(QPainter *painter, const QRectF &target, const QRectF &source,
                            Qt::AspectRatioMode aspectRatioMode)
{
    Q_D(QGraphicsScene);

    // Default source rect = scene rect
    QRectF sourceRect = source;
    if (sourceRect.isNull())
        sourceRect = sceneRect();

    // Default target rect = device rect
    QRectF targetRect = target;
    if (targetRect.isNull())
        targetRect.setRect(0, 0, painter->device()->width(), painter->device()->height());

    // Find the ideal x / y scaling ratio to fit \a source into \a target.
    qreal xratio = targetRect.width() / sourceRect.width();
    qreal yratio = targetRect.height() / sourceRect.height();

    // Scale according to the aspect ratio mode.
    switch (aspectRatioMode) {
    case Qt::KeepAspectRatio:
        xratio = yratio = qMin(xratio, yratio);
        break;
    case Qt::KeepAspectRatioByExpanding:
        xratio = yratio = qMax(xratio, yratio);
        break;
    case Qt::IgnoreAspectRatio:
        break;
    }

    // Find all items to draw, and reverse the list (we want to draw
    // in reverse order).
    QList<QGraphicsItem *> itemList = items(sourceRect);
    if (!itemList.isEmpty()) {
        QGraphicsItem **a = &itemList.first();
        QGraphicsItem **b = &itemList.last();
        QGraphicsItem *tmp = 0;
        while (a < b) {
            tmp = *a;
            *a = *b;
            *b = tmp;
            ++a; --b;
        }
    }

    // Generate the style options
    QList<QStyleOptionGraphicsItem> styleOptions;
    for (int i = 0; i < itemList.size(); ++i) {
        QGraphicsItem *item = itemList.at(i);

        QStyleOptionGraphicsItem option;
        option.state = QStyle::State_None;
        option.rect = item->boundingRect().toRect();
        if (item->isSelected())
            option.state |= QStyle::State_Selected;
        if (item->isEnabled())
            option.state |= QStyle::State_Enabled;
        if (item->hasFocus())
            option.state |= QStyle::State_HasFocus;
        if (d->hoverItems.contains(item))
            option.state |= QStyle::State_MouseOver;
        if (item == mouseGrabberItem())
            option.state |= QStyle::State_Sunken;

        // Calculate a simple level-of-detail metric.
        QMatrix neo = item->sceneMatrix() * painter->matrix();
        QRectF mappedRect = neo.mapRect(QRectF(0, 0, 1, 1));
        qreal dx = neo.mapRect(QRectF(0, 0, 1, 1)).size().width();
        qreal dy = neo.mapRect(QRectF(0, 0, 1, 1)).size().height();
        option.levelOfDetail = qMin(dx, dy);
        option.matrix = neo;

        option.exposedRect = item->boundingRect();
        option.exposedRect &= neo.inverted().mapRect(targetRect);

        styleOptions << option;
    }

    painter->save();

    // Transform the painter.
    painter->setClipRect(targetRect);
    painter->translate(targetRect.left(), targetRect.top());
    painter->scale(xratio, yratio);
    painter->translate(-sourceRect.left(), -sourceRect.top());

    // Render the scene.
    drawBackground(painter, sourceRect);
    drawItems(painter, itemList, styleOptions);
    drawForeground(painter, sourceRect);

    painter->restore();
}

/*!
    \property QGraphicsScene::itemIndexMethod
    \brief the item indexing method.

    QGraphicsScene applies an indexing algorithm to the scene, to speed up
    item discovery functions like items() and itemAt(). Indexing is most
    efficient for static scenes (i.e., where items don't move around). For
    dynamic scenes, or scenes with many animated items, the index bookkeeping
    can outweight the fast lookup speeds.

    For the common case, the default index method BspTreeIndex works fine.  If
    your scene uses many animations and you are experiencing slowness, you can
    disable indexing by calling setItemIndexMethod(NoIndex).
*/
QGraphicsScene::ItemIndexMethod QGraphicsScene::itemIndexMethod() const
{
    Q_D(const QGraphicsScene);
    return d->indexMethod;
}
void QGraphicsScene::setItemIndexMethod(ItemIndexMethod method)
{
    Q_D(QGraphicsScene);
    d->resetIndex();
    d->indexMethod = method;
}

/*!
    Calculates and returns the bounding rect of all items on the scene. This
    function works by iterating over all items, and because if this, it can
    be slow for large scenes.

    \sa sceneRect()
*/
QRectF QGraphicsScene::itemsBoundingRect() const
{
    Q_D(const QGraphicsScene);

    QRectF boundingRect;
    foreach (QGraphicsItem *item, d->validItems())
        boundingRect |= item->sceneBoundingRect();
    return boundingRect;
}

/*!
    Returns a list of all items on the scene, in no particular order.

    \sa addItem(), removeItem()
*/
QList<QGraphicsItem *> QGraphicsScene::items() const
{
    Q_D(const QGraphicsScene);
    return d->allItems + d->newItems;
}

/*!
    Returns all visible items at position \a pos in the scene. The items are
    listed in descending Z order (i.e., the first item in the list is the
    top-most item, and the last item is the bottom-most item).

    \sa itemAt()
*/
QList<QGraphicsItem *> QGraphicsScene::items(const QPointF &pos) const
{
    QList<QGraphicsItem *> itemsAtPoint;

    // Find all items within a 1x1 rect area starting at pos. This can be
    // inefficient for scenes that use small coordinates (like unity
    // coordinates), or for detailed graphs. ### The index should support
    // fetching items at a pos to avoid this limitation.
    foreach (QGraphicsItem *item, items(QRectF(pos, QSizeF(1, 1)))) {
        if (item->contains(item->mapFromScene(pos)))
            itemsAtPoint << item;
    }
    return itemsAtPoint;
}

/*!
    \overload

    Returns all visible items that are either inside or intersect with the
    rectangle \a rect.

    \sa itemAt()
*/
QList<QGraphicsItem *> QGraphicsScene::items(const QRectF &rect) const
{
    Q_D(const QGraphicsScene);
    QList<QGraphicsItem *> itemsInRect;

    // The index returns a rough estimate of what items are inside the rect.
    // Refine it by iterating through all returned items.
    foreach (QGraphicsItem *item, d->estimateItemsInRect(rect)) {
        if (item->isVisible()) {
            QRectF mappedRect = item->sceneBoundingRect();
            if (rect.intersects(mappedRect) || rect.contains(mappedRect))
                itemsInRect << item;
        }
    }
    d->sortItems(&itemsInRect);
    return itemsInRect;
}

/*!
    \overload

    Returns all visible items that are either inside or intersect with the
    polygon \a polygon.

    \sa itemAt()
*/
QList<QGraphicsItem *> QGraphicsScene::items(const QPolygonF &polygon) const
{
    QList<QGraphicsItem *> itemsInPolygon;
    foreach (QGraphicsItem *item, items(polygon.boundingRect())) {
        if (qt_rectInPoly(polygon, item->sceneBoundingRect()))
            itemsInPolygon << item;
    }
    return itemsInPolygon;
}

/*!
    \overload

    Returns all visible items that are either inside or intersect with the
    path \a path.

    \sa itemAt()
*/
QList<QGraphicsItem *> QGraphicsScene::items(const QPainterPath &path) const
{
    QList<QGraphicsItem *> tmp;
    foreach (QGraphicsItem *item, items(path.controlPointRect())) {
        if (item->collidesWith(item->sceneMatrix().inverted().map(path)))
            tmp << item;
    }
    return tmp;
}

/*!
    Returns a list of all items that collide with \a item. Collisions are
    determined by calling QGraphicsItem::collidesWith(). By default, two items
    collide if their shapes intersect, or if one item's shape contains another
    item's shape. The items' Z values are ignored.

    The items are returned in descending Z order (i.e., the first item in the
    list is the top-most item, and the last item is the bottom-most item).

    \sa items(), itemAt(), QGraphicsItem::collidesWith()
*/
QList<QGraphicsItem *> QGraphicsScene::collidingItems(QGraphicsItem *item) const
{
    if (!item) {
        qWarning("QGraphicsScene::collidingItems: cannot find collisions for null item");
        return QList<QGraphicsItem *>();
    }

    QList<QGraphicsItem *> tmp;
    foreach (QGraphicsItem *itemInVicinity, items(item->sceneBoundingRect())) {
        if (item != itemInVicinity && item->collidesWith(itemInVicinity))
            tmp << itemInVicinity;
    }
    return tmp;
}

/*!
    Returns the topmost item at position \a pos, or 0 if there are no
    items at this position.

    \sa items(), collidingItems()
*/
QGraphicsItem *QGraphicsScene::itemAt(const QPointF &pos) const
{
    QList<QGraphicsItem *> itemsAtPoint = items(pos);
    return itemsAtPoint.isEmpty() ? 0 : itemsAtPoint.first();
}

/*!
    \fn QGraphicsScene::itemAt(qreal x, qreal y) const
    \overload

    This convenience function is equivalent to calling itemAt(QPointF(\a x, \a y)).
*/

/*!
    Returns a list of all currently selected items. The items are
    returned in no particular order.

    \sa setSelectionArea()
*/
QList<QGraphicsItem *> QGraphicsScene::selectedItems() const
{
    Q_D(const QGraphicsScene);

    // Optimization: Lazily removes items that are not selected.
    QGraphicsScene *that = const_cast<QGraphicsScene *>(this);
    QSet<QGraphicsItem *> actuallySelectedSet;
    foreach (QGraphicsItem *item, that->d_func()->selectedItems) {
        if (item->isSelected())
            actuallySelectedSet << item;
    }

    that->d_func()->selectedItems = actuallySelectedSet.values();

    return d->selectedItems;
}

/*!
    Sets the selection area to \a path. All items within this area will be
    marked as selected. You can get the list of all selected items by
    calling selectedItems().

    For an item to be selected, it must be marked as \e selectable
    (QGraphicsItem::ItemIsSelectable). Items are selectable by default.
*/
void QGraphicsScene::setSelectionArea(const QPainterPath &path)
{
    Q_D(const QGraphicsScene);

    QSet<QGraphicsItem *> unselectItems;
    foreach (QGraphicsItem *item, d->selectedItems)
        unselectItems << item;

    // Set all items in path to selected.
    foreach (QGraphicsItem *item, items(path)) {
        if (item->flags() & QGraphicsItem::ItemIsSelectable) {
            unselectItems.remove(item);
            item->setSelected(true);
        }
    }

    // Unselect all items outside path.
    foreach (QGraphicsItem *item, unselectItems)
        item->setSelected(false);
}

/*!
   Clears the current selection.

   \sa setSelectionArea(), selectedItems()
*/
void QGraphicsScene::clearSelection()
{
    Q_D(QGraphicsScene);
    foreach (QGraphicsItem *item, d->selectedItems)
        item->setSelected(false);
    d->selectedItems.clear();
}

/*!
    Groups all items in \a items into a new QGraphicsItemGroup, and returns a
    pointer to the group. The group is created with the common ancestor of \a
    items as its parent, and with position (0, 0). The items are all
    reparented to the group, and their positions and transformations are
    mapped to the group.

    QGraphicsScene has ownership of the group item; you do not need to delete
    it. To dismantle (ungroup) a group, call destroyItemGroup().

    \sa destroyItemGroup(), QGraphicsItemGroup::addToGroup()
*/
QGraphicsItemGroup *QGraphicsScene::createItemGroup(const QList<QGraphicsItem *> &items)
{
    // Build a list of the first item's ancestors
    QList<QGraphicsItem *> ancestors;
    int n = 0;
    QGraphicsItem *parent = items.at(n++);
    while ((parent = parent->parentItem()))
        ancestors.append(parent);

    // Find the common ancestor for all items
    QGraphicsItem *commonAncestor = 0;
    if (!ancestors.isEmpty()) {
        while (n < items.size()) {
            int commonIndex = -1;
            QGraphicsItem *parent = items.at(n++);
            do {
                int index = ancestors.indexOf(parent, qMax(0, commonIndex));
                if (index != -1) {
                    commonIndex = index;
                    break;
                }
            } while ((parent = parent->parentItem()));

            if (commonIndex == -1) {
                commonAncestor = 0;
                break;
            }

            commonAncestor = ancestors.at(commonIndex);
        }
    }

    // Create a new group at that level
    QGraphicsItemGroup *group = new QGraphicsItemGroup(commonAncestor);
    foreach (QGraphicsItem *item, items)
        group->addToGroup(item);
    return group;
}

/*!
    Reparents all items in \a group to \a group's parent item, then removes \a
    group from the scene, and finally deletes it. The items' positions and
    transformations are mapped from the group to the group's parent.

    \sa createItemGroup(), QGraphicsItemGroup::removeFromGroup()
*/
void QGraphicsScene::destroyItemGroup(QGraphicsItemGroup *group)
{
    foreach (QGraphicsItem *item, group->children())
        group->removeFromGroup(item);
    removeItem(group);
    delete group;
}

/*!
    Adds the item \a item and all its childen to the scene.

    If the item is visible (i.e., QGraphicsItem::isVisible() returns
    true), QGraphicsScene will emit changed() once control goes back
    to the event loop.

    If the item is already associated with a scene, it will first be
    removed from that scene, and then added to this scene.

    \sa removeItem(), addEllipse(), addLine(), addPath(), addPixmap(),
    addRect(), addText()
*/
void QGraphicsScene::addItem(QGraphicsItem *item)
{
    Q_D(QGraphicsScene);
    if (!item) {
        qWarning("QGraphicsScene::addItem: cannot add null item");
        return;
    }

    d->addToIndex(item);

    if (item->d_func()->scene)
        item->d_func()->scene->removeItem(item);
    if (QGraphicsItem *itemParent = item->d_func()->parent) {
        if (itemParent->scene() != this) {
            itemParent->d_func()->children.removeAll(item);
            item->d_func()->parent = 0;
        }
    }
    item->d_func()->scene = this;
    if (d->indexMethod != QGraphicsScene::NoIndex) {
        d->newItems << item;
    } else {
        item->d_func()->index = d->allItems.size() - 1;
        d->allItems << item;
    }

    if (item->isSelected())
        d->selectedItems << item;

    foreach (QGraphicsItem *child, item->children())
        addItem(child);

    if (item->isVisible())
        itemUpdated(item, item->sceneBoundingRect());
}

/*!
    Creates and adds an ellipse item to the scene, and returns the item
    pointer. The geometry of the ellipse is defined by \a rect, and it's pen
    and brush are initialized to \a pen and \a brush.

    Note that the item's geometry is provided in item coordinates, and its
    position is initialized to (0, 0).

    If the item is visible (i.e., QGraphicsItem::isVisible() returns true),
    QGraphicsScene will emit changed() once control goes back to the event
    loop.

    \sa addLine(), addPath(), addPixmap(), addRect(), addText(), addItem()
*/
QGraphicsEllipseItem *QGraphicsScene::addEllipse(const QRectF &rect, const QPen &pen, const QBrush &brush)
{
    QGraphicsEllipseItem *item = new QGraphicsEllipseItem(rect);
    item->setPen(pen);
    item->setBrush(brush);
    addItem(item);
    return item;
}

/*!
    Creates and adds a line item to the scene, and returns the item
    pointer. The geometry of the line is defined by \a line, and it's pen
    is initialized to \a pen.

    Note that the item's geometry is provided in item coordinates, and its
    position is initialized to (0, 0).

    If the item is visible (i.e., QGraphicsItem::isVisible() returns true),
    QGraphicsScene will emit changed() once control goes back to the event
    loop.

    \sa addEllipse(), addPath(), addPixmap(), addRect(), addText(), addItem()
*/
QGraphicsLineItem *QGraphicsScene::addLine(const QLineF &line, const QPen &pen)
{
    QGraphicsLineItem *item = new QGraphicsLineItem(line);
    item->setPen(pen);
    addItem(item);
    return item;
}

/*!
    Creates and adds a path item to the scene, and returns the item
    pointer. The geometry of the path is defined by \a path, and it's pen and
    brush are initialized to \a pen and \a brush.

    Note that the item's geometry is provided in item coordinates, and its
    position is initialized to (0, 0).

    If the item is visible (i.e., QGraphicsItem::isVisible() returns true),
    QGraphicsScene will emit changed() once control goes back to the event
    loop.

    \sa addEllipse(), addLine(), addPixmap(), addRect(), addText(), addItem()
*/
QGraphicsPathItem *QGraphicsScene::addPath(const QPainterPath &path, const QPen &pen, const QBrush &brush)
{
    QGraphicsPathItem *item = new QGraphicsPathItem(path);
    item->setPen(pen);
    item->setBrush(brush);
    addItem(item);
    return item;
}

/*!
    Creates and adds a pixmap item to the scene, and returns the item
    pointer. The pixmap is defined by \a pixmap.

    Note that the item's geometry is provided in item coordinates, and its
    position is initialized to (0, 0).

    If the item is visible (i.e., QGraphicsItem::isVisible() returns true),
    QGraphicsScene will emit changed() once control goes back to the event
    loop.

    \sa addEllipse(), addLine(), addPath(), addRect(), addText(), addItem()
*/
QGraphicsPixmapItem *QGraphicsScene::addPixmap(const QPixmap &pixmap)
{
    QGraphicsPixmapItem *item = new QGraphicsPixmapItem(pixmap);
    addItem(item);
    return item;
}

/*!
    Creates and adds a polygon item to the scene, and returns the item
    pointer. The polygon is defined by \a polygon, and it's pen and
    brush are initialized to \a pen and \a brush.

    Note that the item's geometry is provided in item coordinates, and its
    position is initialized to (0, 0).

    If the item is visible (i.e., QGraphicsItem::isVisible() returns true),
    QGraphicsScene will emit changed() once control goes back to the event
    loop.

    \sa addEllipse(), addLine(), addPath(), addRect(), addText(), addItem()
*/
QGraphicsPolygonItem *QGraphicsScene::addPolygon(const QPolygonF &polygon,
                                                 const QPen &pen, const QBrush &brush)
{
    QGraphicsPolygonItem *item = new QGraphicsPolygonItem(polygon);
    item->setPen(pen);
    item->setBrush(brush);
    addItem(item);
    return item;
}

/*!
    Creates and adds a rectangle item to the scene, and returns the item
    pointer. The geometry of the rectangle is defined by \a rect, and it's pen
    and brush are initialized to \a pen and \a brush.

    Note that the item's geometry is provided in item coordinates, and its
    position is initialized to (0, 0).

    If the item is visible (i.e., QGraphicsItem::isVisible() returns true),
    QGraphicsScene will emit changed() once control goes back to the event
    loop.

    \sa addEllipse(), addLine(), addPixmap(), addPixmap(), addText(), addItem()
*/
QGraphicsRectItem *QGraphicsScene::addRect(const QRectF &rect, const QPen &pen, const QBrush &brush)
{
    QGraphicsRectItem *item = new QGraphicsRectItem(rect);
    item->setPen(pen);
    item->setBrush(brush);
    addItem(item);
    return item;
}

/*!
    Creates and adds a text item to the scene, and returns the item
    pointer. The text string is initialized to \a text, and it's font
    is initialized to \a font.

    The item's position is initialized to (0, 0).

    If the item is visible (i.e., QGraphicsItem::isVisible() returns true),
    QGraphicsScene will emit changed() once control goes back to the event
    loop.

    \sa addEllipse(), addLine(), addPixmap(), addPixmap(), addRect(), addItem()
*/
QGraphicsTextItem *QGraphicsScene::addText(const QString &text, const QFont &font)
{
    QGraphicsTextItem *item = new QGraphicsTextItem(text);
    item->setFont(font);
    addItem(item);
    return item;
}

/*!
    Removes the item \a item and all its children from the scene.  The
    ownership of \a item is passed on to the caller (i.e.,
    QGraphicsScene will no longer delete \a item when destroyed).

    \sa addItem()
*/
void QGraphicsScene::removeItem(QGraphicsItem *item)
{
    Q_D(QGraphicsScene);

    d->removeFromIndex(item);
    item->d_func()->scene = 0;
    if (QGraphicsItem *parentItem = item->parentItem()) {
        if (parentItem->scene() == this)
            item->setParentItem(0);
    }
    int index = item->d_func()->index;
    if (index != -1) {
        d->freeItemIndexes << index;
        d->allItems[index] = 0;
    } else {
        d->newItems.removeAll(item);
    }
    if (item == d->mouseGrabberItem) {
        d->lastMouseGrabberItem = d->mouseGrabberItem;
        d->mouseGrabberItem = 0;
    }
    if (item == d->focusItem)
        d->focusItem = 0;
    if (item == d->lastFocusItem)
        d->lastFocusItem = 0;
    foreach (QGraphicsItem *child, item->children())
        removeItem(child);
}

/*!
    Returns the scene's current focus item, or 0 if no item currently has
    focus.

    The focus item receives keyboard input when the scene receives a
    key event.

    \sa setFocusItem(), QGraphicsItem::hasFocus()
*/
QGraphicsItem *QGraphicsScene::focusItem() const
{
    Q_D(const QGraphicsScene);
    return d->focusItem;
}

/*!
    Sets the scene's focus item to \a item, with the focus reason \a
    focusReason, after removing focus from any previous item that may have had
    focus.

    If \a item is 0, or if it either does not accept focus (i.e., it does not
    have the QGraphicsItem::ItemIsFocusable flag enabled), or is not visible
    or not enabled, this function only removes focus from any previous
    focusitem.

    If item is not 0, and the scene does not currently have focus (i.e.,
    hasFocus() returns false), this function will call setFocus()
    automatically.

    \sa focusItem(), hasFocus(), setFocus()
*/
void QGraphicsScene::setFocusItem(QGraphicsItem *item, Qt::FocusReason focusReason)
{
    Q_D(QGraphicsScene);
    if (item == d->focusItem)
        return;
    if (item && (!(item->flags() & QGraphicsItem::ItemIsFocusable)
                 || !item->isVisible() || !item->isEnabled())) {
        item = 0;
    }

    if (item) {
        setFocus(focusReason);
        if (item == d->focusItem)
            return;
    }

    if (d->focusItem) {
        QFocusEvent event(QEvent::FocusOut, focusReason);
        d->focusItem->sceneEvent(&event);
        d->lastFocusItem = d->focusItem;
        d->focusItem = 0;
        d->lastFocusItem->update();
    }

    if (item) {
        d->focusItem = item;
        QFocusEvent event(QEvent::FocusIn, focusReason);
        item->sceneEvent(&event);
        item->update();
    }
}

/*!
    Returns true if the scene has focus; otherwise returns false. If the scene
    has focus, it will will forward key events from QKeyEvent to any item that
    has focus.

    \sa setFocus(), setFocusItem()
*/
bool QGraphicsScene::hasFocus() const
{
    Q_D(const QGraphicsScene);
    return d->hasFocus;
}

/*!
    Sets focus on the scene by sending a QFocusEvent to the scene, passing \a
    focusReason as the reason. If the scene regains focus after having
    previously lost it while an item had focus, the last focus item will
    receive focus with \a focusReason as the reason.

    If the scene already has focus, this function does nothing.

    \sa hasFocus(), clearFocus(), setFocusItem()
*/
void QGraphicsScene::setFocus(Qt::FocusReason focusReason)
{
    Q_D(QGraphicsScene);
    if (d->hasFocus)
        return;
    QFocusEvent event(QEvent::FocusIn, focusReason);
    QCoreApplication::sendEvent(this, &event);
}

/*!
    Clears focus from the scene. If any item has focus when this function is
    called, it will lose focus, and regain focus again once the scene regains
    focus.

    A scene that does not have focus ignores key events.

    \sa hasFocus(), setFocus(), setFocusItem()
*/
void QGraphicsScene::clearFocus()
{
    Q_D(QGraphicsScene);
    if (d->hasFocus) {
        d->hasFocus = false;
        setFocusItem(0, Qt::OtherFocusReason);
    }
}

/*!
    Returns the current mouse grabber item, or 0 if no item is currently
    grabbing the mouse. The mouse grabber item is the item that receives all
    mouse events sent to the scene.

    An item becomes a mouse grabber when it receives and accepts a
    mouse press event, and it stays the mouse grabber until either of
    the following events occur:

    \list
    \o If the item receives a mouse release event when there are no other
    buttons pressed, it loses the mouse grab.
    \o If the item becomes invisible (i.e., someone calls item->setVisible(false)),
    or if it becomes disabled (i.e., someone calls item->setEnabled(false)),
    it loses the mouse grab.
    \o If the item is removed from the scene, it loses the mouse grab.
    \endlist

    If the item loses its mouse grab, the scene will ignore all mouse events
    until a new item grabs the mouse (i.e., until a new item receives a mouse
    press event).
*/
QGraphicsItem *QGraphicsScene::mouseGrabberItem() const
{
    Q_D(const QGraphicsScene);
    return d->mouseGrabberItem;
}

/*!
    \property QGraphicsScene::backgroundBrush
    \brief the background brush of the scene.

    Set this property to changes the scene's background to a different color,
    gradient or texture. The default background brush is Qt::NoBrush. The
    background is drawn before (behind) the items.

    Example:

    \code
        QGraphicsScene scene;
        QGraphicsView view(&scene);
        view.show();

        // a blue background
        scene.setBackgroundBrush(Qt::blue);

        // a gradient background
        QRadialGradient gradient(0, 0, 10);
        gradient.setSpread(QGradient::RepeatSpread);
        scene.setBackgroundBrush(gradient);
    \endcode

    QGraphicsScene::render() calls drawBackground() to draw the scene
    background. For more detailed control over how the background is drawn,
    you can reimplement drawBackground() in a subclass of QGraphicsScene.
*/
QBrush QGraphicsScene::backgroundBrush() const
{
    Q_D(const QGraphicsScene);
    return d->backgroundBrush;
}
void QGraphicsScene::setBackgroundBrush(const QBrush &brush)
{
    Q_D(QGraphicsScene);
    d->backgroundBrush = brush;
    update();
}

/*!
    \property QGraphicsScene::foregroundBrush
    \brief the foreground brush of the scene.

    Change this property to set the scene's foreground to a different color,
    gradient or texture. If the style of \a brush is Qt::NoBrush (e.g., if you
    passed QBrush()), the foreground is not drawn.  The foreground is drawn
    after (on top of) the items. The default foreground brush is Qt::NoBrush.

    Example:

    \code
        QGraphicsScene scene;
        QGraphicsView view(&scene);
        view.show();

        // a white semi-transparent foreground
        scene.setForegroundBrush(QColor(255, 255, 255, 127));

        // a grid foreground
        scene.setForegroundBrush(QBrush(Qt::lightGray, Qt::CrossPattern));
    \endcode

    QGraphicsScene::render() calls drawForeground() to draw the scene
    foreground. For more detailed control over how the foreground is drawn,
    you can reimplement drawForeground() in a subclass of QGraphicsScene.
*/
QBrush QGraphicsScene::foregroundBrush() const
{
    Q_D(const QGraphicsScene);
    return d->foregroundBrush;
}
void QGraphicsScene::setForegroundBrush(const QBrush &brush)
{
    Q_D(QGraphicsScene);
    d->foregroundBrush = brush;
    update();
}

/*!
    This function is the equivalent of QWidget::inputMethodQuery() for a graphics scene.

    \sa QWidget::inputMethodQuery()
*/
QVariant QGraphicsScene::inputMethodQuery(Qt::InputMethodQuery query) const
{
    Q_D(const QGraphicsScene);
    if (!d->focusItem)
        return QVariant();
    const QMatrix matrix = d->focusItem->sceneMatrix();
    QVariant value = d->focusItem->inputMethodQuery(query);
    if (value.type() == QVariant::RectF)
        value = matrix.mapRect(value.toRectF());
    else if (value.type() == QVariant::PointF)
        value = matrix.map(value.toPointF());
    else if (value.type() == QVariant::Rect)
        value = matrix.mapRect(value.toRect());
    else if (value.type() == QVariant::Point)
        value = matrix.map(value.toPoint());
    return value;
}

/*!
    Schedules a redraw of the area \a rect on the scene.

    \sa sceneRect(), changed()
*/
void QGraphicsScene::update(const QRectF &rect)
{
    Q_D(QGraphicsScene);
    d->updatedRects << (!rect.isNull() ? rect : sceneRect());
    if (!d->calledEmitUpdated) {
        d->calledEmitUpdated = true;
        QTimer::singleShot(0, this, SLOT(emitUpdated()));
    }
}

/*!
    This slot \e advances the scene by one step, by calling
    QGraphicsItem::advance() for all items on the scene. This is done in two
    phases: in the first phase, all items are notified that the scene is about
    to change, and in the second phase all items are notified that they can
    move. In the first phase, QGraphicsItem::advance() is called passing a
    value of 0 as an argument, and 1 is passed in the second phase.

    \sa QGraphicsItem::advance(), QGraphicsItemAnimation(), QTimeLine()
*/
void QGraphicsScene::advance()
{
    for (int i = 0; i < 2; ++i) {
        foreach (QGraphicsItem *item, items())
            item->advance(i);
    }
}

/*!
    Processes the event \a event, and dispatches it to the respective
    event handlers.

    In addition to calling the convenience event handlers, this
    function is responsible for converting mouse move events to hover
    events for when there is no mouse grabber item. Hover events are
    delivered directly to items; there is no convenience function for
    them.

    \sa contextMenuEvent(), keyPressEvent(), keyReleaseEvent(),
    mousePressEvent(), mouseMoveEvent(), mouseReleaseEvent(),
    mouseDoubleClickEvent(), focusInEvent(), focusOutEvent()
*/
bool QGraphicsScene::event(QEvent *event)
{
    Q_D(QGraphicsScene);
    switch (event->type()) {
    case QEvent::GraphicsSceneDragEnter:
        dragEnterEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragMove:
        dragMoveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDragLeave:
        dragLeaveEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneDrop:
        dropEvent(static_cast<QGraphicsSceneDragDropEvent *>(event));
        break;
    case QEvent::GraphicsSceneContextMenu:
        contextMenuEvent(static_cast<QGraphicsSceneContextMenuEvent *>(event));
        break;
    case QEvent::KeyPress:
        keyPressEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::KeyRelease:
        keyReleaseEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseMove:
        if (d->mouseGrabberItem) {
            mouseMoveEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        } else {
            QGraphicsSceneMouseEvent *mouseEvent = static_cast<QGraphicsSceneMouseEvent *>(event);
            QGraphicsSceneHoverEvent hover;
            hover.setWidget(mouseEvent->widget());
            hover.setPos(mouseEvent->pos());
            hover.setScenePos(mouseEvent->scenePos());
            hover.setScreenPos(mouseEvent->screenPos());
            d->dispatchHoverEvent(&hover);
        }
        break;
    case QEvent::GraphicsSceneMousePress:
        mousePressEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseRelease:
        mouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseDoubleClick:
        mouseDoubleClickEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    case QEvent::FocusIn:
        focusInEvent(static_cast<QFocusEvent *>(event));
        break;
    case QEvent::FocusOut:
        focusOutEvent(static_cast<QFocusEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverEnter:
    case QEvent::GraphicsSceneHoverLeave:
    case QEvent::GraphicsSceneHoverMove:
        d->dispatchHoverEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::Leave:
#ifndef QT_NO_TOOLTIP
        // Remove any tooltips
        QToolTip::showText(QPoint(), QString());
#endif
        break;
    case QEvent::GraphicsSceneHelp:
        helpEvent(static_cast<QGraphicsSceneHelpEvent *>(event));
        break;
    default:
        return false;
    }
    return true;
}

/*!
    This event handler, for event \a contextMenuEvent, can be reimplemented in
    a subclass to receive context menu events. The default implementation
    forwards the event to the item at the scene position provided by the event.

    \sa QGraphicsItem::contextMenuEvent()
*/
void QGraphicsScene::contextMenuEvent(QGraphicsSceneContextMenuEvent *contextMenuEvent)
{
    if (QGraphicsItem *item = itemAt(contextMenuEvent->scenePos())) {
        contextMenuEvent->setPos(item->mapFromScene(contextMenuEvent->scenePos()));
        item->sceneEvent(contextMenuEvent);
    }
}

/*!
    This event handler, for event \a event, can be reimplemented in a subclass
    to receive drag enter events for the scene.

    The default implementation accepts the event and prepares the scene to
    accept drag move events.

    \sa QGraphicsItem::dragEnterEvent(), dragMoveEvent(), dragLeaveEvent(),
    dropEvent()
*/
void QGraphicsScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_D(QGraphicsScene);
    d->dragDropItem = 0;
    d->lastDropAction = Qt::IgnoreAction;
    event->accept();
}

/*!
    This event handler, for event \a event, can be reimplemented in a subclass
    to receive drag move events for the scene.

    \sa QGraphicsItem::dragMoveEvent(), dragEnterEvent(), dragLeaveEvent(),
    dropEvent()
*/
void QGraphicsScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_D(QGraphicsScene);
    event->ignore();

    bool eventDelivered = false;

    // Find the topmost enabled items under the cursor. They are all
    // candidates for accepting drag & drop events.
    foreach (QGraphicsItem *item, items(event->scenePos())) {
        if (!item->isEnabled() || !item->acceptDrops())
            continue;

        if (item != d->dragDropItem) {
            // Enter the new drag drop item. If it accepts the event, we send
            // the leave to the parent item.
            QGraphicsSceneDragDropEvent dragEnter(QEvent::GraphicsSceneDragEnter);
            d->cloneDragDropEvent(&dragEnter, event);
            dragEnter.setDropAction(event->proposedAction());
            d->sendDragDropEvent(item, &dragEnter);
            event->setAccepted(dragEnter.isAccepted());
            event->setDropAction(dragEnter.dropAction());
            if (!event->isAccepted()) {
                // Propagate to the item under
                continue;
            }

            d->lastDropAction = event->dropAction();

            if (d->dragDropItem) {
                // Leave the last drag drop item. A perfect implementation
                // would set the position of this event to the point where
                // this event and the last event intersect with the item's
                // shape, but that's not easy to do. :-)
                QGraphicsSceneDragDropEvent dragLeave(QEvent::GraphicsSceneDragLeave);
                d->cloneDragDropEvent(&dragLeave, event);
                d->sendDragDropEvent(d->dragDropItem, &dragLeave);
            }

            // We've got a new drag & drop item
            d->dragDropItem = item;
        }

        // Send the move event.
        event->setDropAction(d->lastDropAction);
        event->accept();
        d->sendDragDropEvent(item, event);
        if (event->isAccepted()) {
            d->lastDropAction = event->dropAction();
            eventDelivered = true;
            break;
        }

        // Propagate
        event->setDropAction(Qt::IgnoreAction);
    }

    if (!eventDelivered) {
        if (d->dragDropItem) {
            // Leave the last drag drop item
            QGraphicsSceneDragDropEvent dragLeave(QEvent::GraphicsSceneDragLeave);
            d->cloneDragDropEvent(&dragLeave, event);
            d->sendDragDropEvent(d->dragDropItem, &dragLeave);
            d->dragDropItem = 0;
        }
    }
}

/*!
    This event handler, for event \a event, can be reimplemented in a subclass
    to receive drag leave events for the scene.

    \sa QGraphicsItem::dragLeaveEvent(), dragEnterEvent(), dragMoveEvent(),
    dropEvent()
*/
void QGraphicsScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_D(QGraphicsScene);
    if (d->dragDropItem) {
        // Leave the last drag drop item
        d->sendDragDropEvent(d->dragDropItem, event);
        d->dragDropItem = 0;
    }
}

/*!
    This event handler, for event \a event, can be reimplemented in a subclass
    to receive drop events for the scene.

    \sa QGraphicsItem::dropEvent(), dragEnterEvent(), dragMoveEvent(),
    dragLeaveEvent()
*/
void QGraphicsScene::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);
    Q_D(QGraphicsScene);
    if (d->dragDropItem) {
        // Drop on the last drag drop item
        d->sendDragDropEvent(d->dragDropItem, event);
        d->dragDropItem = 0;
    }
}

/*!
    This event handler, for event \a focusEvent, can be reimplemented in a
    subclass to receive focus in events.

    The default implementation sets focus on the scene, and then on the last
    focus item.

    \sa QGraphicsItem::focusOutEvent()
*/
void QGraphicsScene::focusInEvent(QFocusEvent *focusEvent)
{
    Q_D(QGraphicsScene);
    d->hasFocus = true;
    if (d->lastFocusItem) {
        // Set focus on the last focus item
        setFocusItem(d->lastFocusItem, focusEvent->reason());
    }
}

/*!
    This event handler, for event \a focusEvent, can be reimplemented in a
    subclass to receive focus out events.

    The default implementation removes focus from any focus item, then removes
    focus from the scene.

    \sa QGraphicsItem::focusInEvent()
*/
void QGraphicsScene::focusOutEvent(QFocusEvent *focusEvent)
{
    Q_D(QGraphicsScene);
    d->hasFocus = false;
    setFocusItem(0, focusEvent->reason());
}

/*!
   This event handler, for event \a helpEvent, can be reimplemented in a
   subclass to receive help events. The default implementation uses the event
   to show tooltips for items.

   \sa QGraphicsItem::toolTip()
*/
void QGraphicsScene::helpEvent(QGraphicsSceneHelpEvent *helpEvent)
{
#ifndef QT_NO_TOOLTIP
    // Find the first item that does tooltips
    QList<QGraphicsItem *> itemsAtPos = items(helpEvent->scenePos());
    QGraphicsItem *toolTipItem = 0;
    for (int i = 0; i < itemsAtPos.size(); ++i) {
        QGraphicsItem *tmp = itemsAtPos.at(i);
        if (!tmp->toolTip().isEmpty()) {
            toolTipItem = tmp;
            break;
        }
    }

    // Show or hide the tooltip
    QString text;
    QPoint point;
    if (toolTipItem && !toolTipItem->toolTip().isEmpty()) {
        text = toolTipItem->toolTip();
        point = helpEvent->screenPos();
    }
    QToolTip::showText(point, text);
#endif
}

/*!
    This event handler, for event \a hoverEvent, can be reimplemented in a
    subclass to receive hover enter events. The default implementation
    forwards the event to the topmost item that accepts hover events at the
    scene position from the event.

    \sa QGraphicsItem::hoverEvent(), QGraphicsItem::setAcceptsHoverEvents()
*/
void QGraphicsScenePrivate::dispatchHoverEvent(QGraphicsSceneHoverEvent *hoverEvent)
{
    Q_Q(QGraphicsScene);
    // Find the first item that accepts hover events
    QList<QGraphicsItem *> itemsAtPos = q->items(hoverEvent->scenePos());
    QGraphicsItem *item = 0;
    for (int i = 0; i < itemsAtPos.size(); ++i) {
        QGraphicsItem *tmp = itemsAtPos.at(i);
        if (tmp->acceptsHoverEvents()) {
            item = tmp;
            break;
        }
    }

    if (!item) {
        // Send HoverLeave events to all existing hover items, topmost first.
        while (!hoverItems.isEmpty()) {
            QGraphicsItem *lastItem = hoverItems.takeLast();
            if (lastItem->acceptsHoverEvents())
                sendHoverEvent(QEvent::GraphicsSceneHoverLeave, lastItem, hoverEvent);
        }
        return;
    }

    int itemIndex = hoverItems.indexOf(item);
    if (itemIndex == -1) {
        if (hoverItems.isEmpty() || !hoverItems.last()->isAncestorOf(item)) {
            // Send HoverLeave events to all existing hover items, topmost first.
            while (!hoverItems.isEmpty()) {
                QGraphicsItem *lastItem = hoverItems.takeLast();
                if (lastItem->acceptsHoverEvents())
                    sendHoverEvent(QEvent::GraphicsSceneHoverLeave, lastItem, hoverEvent);
            }
        }

        // Item is a child of a known item. Generate enter events for the
        // missing links.
        QList<QGraphicsItem *> parents;
        parents << item;

        QGraphicsItem *parent = item->parentItem();
        while (parent && (hoverItems.isEmpty() || parent != hoverItems.last())) {
            parents.prepend(parent);
            parent = parent->parentItem();
        }
        for (int i = 0; i < parents.size(); ++i) {
            parent = parents.at(i);
            hoverItems << parent;
            sendHoverEvent(QEvent::GraphicsSceneHoverEnter, parent, hoverEvent);
        }
    } else {
        // Known item, generate leave events for any children
        while (hoverItems.size() > itemIndex + 1) {
            QGraphicsItem *child = hoverItems.takeAt(itemIndex + 1);
            sendHoverEvent(QEvent::GraphicsSceneHoverLeave, child, hoverEvent);
        }

        // Generate a move event for the item itself
        sendHoverEvent(QEvent::GraphicsSceneHoverMove, item, hoverEvent);
    }
}

/*!
    This event handler, for event \a keyEvent, can be reimplemented in a
    subclass to receive keypress events. The default implementation forwards
    the event to current focus item.

    \sa QGraphicsItem::keyPressEvent(), focusItem()
*/
void QGraphicsScene::keyPressEvent(QKeyEvent *keyEvent)
{
    if (QGraphicsItem *item = focusItem())
        item->sceneEvent(keyEvent);
}

/*!
    This event handler, for event \a keyEvent, can be reimplemented in a
    subclass to receive key release events. The default implementation
    forwards the event to current focus item.

    \sa QGraphicsItem::keyReleaseEvent(), focusItem()
*/
void QGraphicsScene::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if (QGraphicsItem *item = focusItem())
        item->sceneEvent(keyEvent);
}

/*!
    This event handler, for event \a mouseEvent, can be reimplemented
    in a subclass to receive mouse press events for the scene.

    The default implementation depends on the state of the scene. If
    there is a mouse grabber item, then the event is sent to the mouse
    grabber. Otherwise, it is forwarded to the topmost item that
    accepts mouse events at the scene position from the event, and
    that item promptly becomes the mouse grabber item.

    If there is no item at the given position on the scene, the
    selection area is reset, any focus item loses its input focus, and
    the event is then ignored.

    \sa QGraphicsItem::mousePressEvent(),
    QGraphicsItem::setAcceptedMouseButtons()
*/
void QGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Q_D(QGraphicsScene);
    d->mousePressEventHandler(mouseEvent);
}

/*!
    This event handler, for event \a mouseEvent, can be reimplemented
    in a subclass to receive mouse move events for the scene.

    The default implementation depends on the mouse grabber state. If
    there is a mouse grabber item, the event is sent to the mouse
    grabber; otherwise, the event is ignored.

    \sa QGraphicsItem::mousePressEvent(), QGraphicsItem::mouseReleaseEvent(),
    QGraphicsItem::mouseDoubleClickEvent(), QGraphicsItem::setAcceptedMouseButtons()
*/
void QGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Q_D(QGraphicsScene);
    if (!d->mouseGrabberItem) {
        mouseEvent->ignore();
        return;
    }

    // Forward the event to the mouse grabber
    d->sendMouseEvent(mouseEvent);
}

/*!
    This event handler, for event \a mouseEvent, can be reimplemented
    in a subclass to receive mouse release events for the scene.

    The default implementation depends on the mouse grabber state.  If
    there is no mouse grabber, the event is ignored.  Otherwise, if
    there is a mouse grabber item, the event is sent to the mouse
    grabber. If this mouse release represents the last pressed button
    on the mouse, the mouse grabber item then loses the mouse grab.

    \sa QGraphicsItem::mousePressEvent(), QGraphicsItem::mouseMoveEvent(),
    QGraphicsItem::mouseDoubleClickEvent(), QGraphicsItem::setAcceptedMouseButtons()
*/
void QGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Q_D(QGraphicsScene);
    if (!d->mouseGrabberItem) {
        mouseEvent->ignore();
        return;
    }

    // Forward the event to the mouse grabber
    d->sendMouseEvent(mouseEvent);

    // Reset the mouse grabber when the last mouse button has been released.
    if (!mouseEvent->buttons()) {
        d->lastMouseGrabberItem = d->mouseGrabberItem;
        d->mouseGrabberItem = 0;
        d->mouseGrabberButtonDownPos.clear();
        d->mouseGrabberButtonDownScenePos.clear();
        d->mouseGrabberButtonDownScreenPos.clear();

        // Generate a hoverevent
        QGraphicsSceneHoverEvent hoverEvent;
        hoverEvent.setWidget(mouseEvent->widget());
        hoverEvent.setPos(mouseEvent->pos());
        hoverEvent.setScenePos(mouseEvent->scenePos());
        hoverEvent.setScreenPos(mouseEvent->screenPos());
        QCoreApplication::sendEvent(this, &hoverEvent);
    }
}

/*!
    This event handler, for event \a mouseEvent, can be reimplemented
    in a subclass to receive mouse doubleclick events for the scene.

    If someone doubleclicks on the scene, the scene will first receive
    a mouse press event, followed by a release event (i.e., a click),
    then a doubleclick event, and finally a release event. If the
    doubleclick event is delivered to a different item than the one
    that received the first press and release, it will be delivered as
    a press event. However, tripleclick events are not delivered as
    doubleclick events in this case.

    The default implementation is similar to mousePressEvent().

    \sa QGraphicsItem::mousePressEvent(), QGraphicsItem::mouseMoveEvent(),
    QGraphicsItem::mouseReleaseEvent(), QGraphicsItem::setAcceptedMouseButtons()
*/
void QGraphicsScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *mouseEvent)
{
    Q_D(QGraphicsScene);
    d->mousePressEventHandler(mouseEvent);
}

/*!
    Draws the background of the scene using \a painter, before any items and
    the foreground are drawn. Reimplement this function to provide a custom
    background for the scene.

    All painting is done in \e scene coordinates. \a rect is the exposed
    rectangle.

    \sa drawForeground(), drawItems()
*/
void QGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (backgroundBrush().style() != Qt::NoBrush) {
        painter->save();
        painter->setBrushOrigin(0, 0);
        painter->fillRect(rect, backgroundBrush());
        painter->restore();
    }/* else if (viewport()->inherits("QGLWidget")) {
        painter->fillRect(rect, viewport()->palette().brush(viewport()->backgroundRole()));
        }*/
}

/*!
    Draws the foreground of the scene using \a painter, after the background
    and all items have been drawn. Reimplement this function to provide a
    custom foreground for the scene.

    All painting is done in \e scene coordinates. \a rect is the exposed
    rectangle.

    \sa drawBackground(), drawItems()
*/
void QGraphicsScene::drawForeground(QPainter *painter, const QRectF &rect)
{
    if (foregroundBrush().style() != Qt::NoBrush) {
        painter->save();
        painter->setBrushOrigin(0, 0);
        painter->fillRect(rect, foregroundBrush());
        painter->restore();
    }
}

/*!
    Paints the items \a items using \a painter, after the background has been
    drawn, and before the foreground has been drawn. Reimplement this function
    to provide custom painting of all items for the scene. The default
    implementation prepares the painter matrix, and calls
    QGraphicsItem::paint() on all items. \a options is the list of style
    option objects for each item in \a items.

    All painting is done in \e scene coordinates. Before drawing each item,
    the painter must be transformed using QGraphicsItem::sceneMatrix().

    By reimplementing this function, you gain complete control over how each
    item is drawn, and in some cases this can increase drawing performance
    significantly.

    Example:

    \code
        void CustomScene::drawItems(QPainter *painter,
                                    const QList<QGraphicsItem *> &items,
                                    const QList<QStyleOptionGraphicsItem> &options)
        {
            for (int i = 0; i < items.size(); ++i) {
                // Draw the item
                painter->save();
                painter->setMatrix(items.at(i)->sceneMatrix(), true);
                items.at(i)->paint(painter, options.at(i), viewport());
                painter->restore();
            }
        }
    \endcode

    \sa drawBackground(), drawForeground()
*/
void QGraphicsScene::drawItems(QPainter *painter,
                               const QList<QGraphicsItem *> &items,
                               const QList<QStyleOptionGraphicsItem> &options)
{
    for (int i = 0; i < items.size(); ++i) {
        QGraphicsItem *item = items.at(i);
        painter->save();
        painter->setMatrix(item->sceneMatrix(), true);
        item->paint(painter, &options.at(i), 0);//viewport());
        painter->restore();
    }
}

/*!
    \fn QGraphicsScene::changed(const QList<QRectF> &region)

    This signal is emitted by QGraphicsScene when control reaches the event
    loop, if the scene content changes. \a region contains a list of scene
    rectangles that indicate the area that has been changed.

    \sa QGraphicsView::update()
*/

/*!
    \fn QGraphicsScene::sceneRectChanged(const QRectF &rect)

    This signal is emitted by QGraphicsScene whenever the scene rect changes.
    \a rect is the new scene rect.

    \sa QGraphicsView::updateSceneRect()
*/

/*!
    \internal

    This private function is called by QGraphicsItem, which is a friend of
    QGraphicsScene. It is used by QGraphicsScene to record the rectangles that
    need updating. It also launches a single-shot timer to ensure that
    updated() will be emitted later.

    \a item is the item that changed, and \a rect is the area of the item that
    changed. \a rect is in item coordinates.
*/
void QGraphicsScene::itemUpdated(QGraphicsItem *item, const QRectF &rect)
{
    Q_D(QGraphicsScene);
    QRectF boundingRect = item->boundingRect();
    if (!rect.isNull())
        boundingRect &= rect;
    QRectF sceneBoundingRect = item->sceneMatrix().mapRect(boundingRect);

    update(sceneBoundingRect);

    QRectF oldGrowingItemsBoundingRect = d->growingItemsBoundingRect;
    d->growingItemsBoundingRect |= sceneBoundingRect;
    if (!d->hasSceneRect && d->growingItemsBoundingRect != oldGrowingItemsBoundingRect)
        emit sceneRectChanged(d->growingItemsBoundingRect);
}

#include "moc_qgraphicsscene.cpp"

#endif // QT_NO_GRAPHICSVIEW
