/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*!
    \class QGraphicsItem
*/

/*!
    \class QAbstractGraphicsPathItem
*/

/*!
    \class QGraphicsPathItem
*/

/*!
    \class QGraphicsRectItem
*/

/*!
    \class QGraphicsEllipseItem
*/

/*!
    \class QGraphicsPolygonItem
*/

/*!
    \class QGraphicsLineItem
*/

/*!
    \class QGraphicsPixmapItem
*/

/*!
    \class QGraphicsTextItem
*/

#include "qgraphicsitem.h"
#include "qgraphicsscene.h"
#include "qgraphicsscene_p.h"
#include "qgraphicssceneevent.h"
#include <QtCore/qbitarray.h>
#include <QtCore/qdebug.h>
#include <QtCore/qpoint.h>
#include <QtCore/qvariant.h>
#include <QtGui/qapplication.h>
#include <QtGui/qbitmap.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qstyleoption.h>

#include <private/qgraphicsitem_p.h>
#include <private/qtextcontrol_p.h>

#include <math.h>

/*!
    \internal

    Propagates updates.
*/
static void qt_graphicsItem_fullUpdate(QGraphicsItem *item)
{
    item->update();
    foreach (QGraphicsItem *child, item->children())
        qt_graphicsItem_fullUpdate(child);
}

/*!
    Constructs a QGraphicsItem with the parent \a parent. If \a parent is 0,
    the item will be a top-level.
*/
QGraphicsItem::QGraphicsItem(QGraphicsItem *parent)
    : d_ptr(new QGraphicsItemPrivate)
{
    Q_D(QGraphicsItem);
    d->q_ptr = this;
    if ((d->parent = parent)) {
        d->parent->d_func()->children << this;
        if ((d->scene = parent->scene())) {
            // ### Add item later?
        }
    }
}

/*!
    \internal
*/
QGraphicsItem::QGraphicsItem(QGraphicsItemPrivate &dd, QGraphicsItem *parent)
    : d_ptr(&dd)
{
    Q_D(QGraphicsItem);
    d->q_ptr = this;
    if ((d->parent = parent))
        d->parent->d_func()->children << this;
}

/*!
    Destroys the QGraphicsItem and all its children. If this item is currently
    associated with a scene, the item will be removed from the scene before it
    is deleted.
*/
QGraphicsItem::~QGraphicsItem()
{
    Q_D(QGraphicsItem);

    qDeleteAll(d->children);
    d->children.clear();

    if (QGraphicsItem *parent = parentItem())
        parent->d_func()->children.removeAll(this);
    if (d->scene)
        d->scene->d_func()->removeItemLater(this);

    delete d;
}

/*!
    Returns the current scene for the item, or 0 if the item is not stored in
    a scene.

    Too add or move an item to a scene, call QGraphicsScene::addItem().
*/
QGraphicsScene *QGraphicsItem::scene() const
{
    Q_D(const QGraphicsItem);
    return d->scene;
}

/*!
    Returns a pointer to this item's parent item. If this item does not have a
    parent, 0 is returned.

    \sa setParentItem(), children()
*/
QGraphicsItem *QGraphicsItem::parentItem() const
{
    Q_D(const QGraphicsItem);
    return d->parent;
}

/*!
    Sets this item's parent item to \a parent. If this item already has a
    parent, it is first removed from the previous parent. If \a parent is 0,
    this item will become a top-level item.

    \sa parentItem(), children()
*/
void QGraphicsItem::setParentItem(QGraphicsItem *parent)
{
    Q_D(QGraphicsItem);
    if (parent == this) {
        qWarning("QGraphicsItem::setParentItem: cannot assign %p as a parent of itself", this);
        return;
    }
    if (parent == d->parent)
        return;
    
    if (d->parent) {
        // Remove from current parent
        if (d->parent == parent)
            return;
        d->parent->d_func()->children.removeAll(this);
        d->parent->update();
    }

    if ((d->parent = parent)) {
        d->parent->d_func()->children << this;
        d->parent->update();
    }
}

/*!
   Returns a list of this item's children. The items are returned in no
   particular order.

   \sa setParentItem()
*/
QList<QGraphicsItem *> QGraphicsItem::children() const
{
    Q_D(const QGraphicsItem);
    return d->children;
}

/*!
    Returns this item's flags. The flags describe what configurable features
    of the item are enabled and not. For example, if the flags include
    ItemIsFocusable, the item can accept input focus.

    By default, only ItemIsSelectable is enabled.

    \sa setFlags(), setFlag()
*/
QGraphicsItem::GraphicsItemFlags QGraphicsItem::flags() const
{
    Q_D(const QGraphicsItem);
    return GraphicsItemFlags(d->flags);
}

/*!
    If \a enabled is true, the item flag \a flag is enabled; otherwise, it is
    disabled.

    \sa flags(), setFlags()
*/
void QGraphicsItem::setFlag(GraphicsItemFlag flag, bool enabled)
{
    if (enabled)
        setFlags(flags() | flag);
    else
        setFlags(flags() & ~flag);
}

/*!
    Sets the item flags to \a flags. All flags in \a flags are enabled; all
    flags not in \a flags are disabled.

    If the item had focus and \a flags does not enable ItemIsFocusable, the
    item loses focus as a result of calling this function. Similarily, if the
    item was selected, and \a flags does not enabled ItemIsSelectable, the
    item is automatically unselected.

    By default, only ItemIsSelectable is enabled.

    \sa flags(), setFlag()
*/
void QGraphicsItem::setFlags(GraphicsItemFlags flags)
{
    Q_D(QGraphicsItem);
    if (GraphicsItemFlags(d->flags) != flags) {
        d->flags = flags;

        if (!(d->flags & ItemIsFocusable) && hasFocus())
            clearFocus();
        if (!(d->flags & ItemIsSelectable) && isSelected())
            setSelected(false);
        
        update();
    }
}

/*!
   Returns true if the item is visible; otherwise, false is returned.

   Note that the item's general visibility is unrelated to whether or not it
   is actually being visualized by a QGraphicsView.

   \sa setVisible()
*/
bool QGraphicsItem::isVisible() const
{
    Q_D(const QGraphicsItem);
    return d->visible;
}

/*!
    If \a visible is true, the item is made visible. Otherwise, the item is
    made invisible. Invisible items are not painted, nor do they receive any
    events. In particular, mouse events pass right through invisible items,
    and are delivered to any item that may be behind. Invisible items are also
    unselectable, they cannot take input focus, and are not detected by
    QGraphicsScene's item location functions.

    If an item becomes invisible while grabbing the mouse, (i.e., while it is
    receiving mouse events,) it will automatically lose the mouse grab, and
    the grab is not regained by making the item visible again; it must receive
    a new mouse press to regain the mouse grab.

    Similarily, an invisible item cannot have focus, so if the item has focus
    when it becomes invisible, it will lose focus, and the focus is not
    regained by simply making the item visible again.

    Items are visible by default; it is unnecessary to call setVisible(true)
    on a new item.
    
    \sa isVisible()
*/
void QGraphicsItem::setVisible(bool visible)
{
    Q_D(QGraphicsItem);
    if (d->visible != quint32(visible)) {
        if (!visible) {
            if (d->scene && d->scene->mouseGrabberItem() == this)
                d->scene->d_func()->mouseGrabberItem = 0;
            if (hasFocus())
                clearFocus();
            if (isSelected())
                setSelected(false);
        }
        d->visible = quint32(visible);
        update();
    }
}

/*!
    Returns true if the item is enabled; otherwise, false is returned.

    \sa setEnabled()
*/
bool QGraphicsItem::isEnabled() const
{
    Q_D(const QGraphicsItem);
    return d->enabled;
}

/*!
    If \a enabled is true, the item is enabled; otherwise, it is disabled.

    Disabled items are visible, but they do not receive any events, and cannot
    take focus nor be selected. Mouse events are discarded; they are not
    propagated unless the item is also invisible, or if it does not accept
    mouse events (acceptsMouseEvents()). A disabled item cannot become the
    mouse grabber, and as a result of this, an item loses the grab if it
    becomes disabled when grabbing the mouse, just like it loses focus if it
    had focus when it was disabled.

    Disabled items are traditionally drawn using grayed-out colors (see \l
    QPalette::Disabled).
    
    Items are enabled by default.

    \sa isEnabled()
*/
void QGraphicsItem::setEnabled(bool enabled)
{
    Q_D(QGraphicsItem);
    if (d->enabled != quint32(enabled)) {
        if (!enabled) {
            if (d->scene && d->scene->mouseGrabberItem() == this)
                d->scene->d_func()->mouseGrabberItem = 0;
            if (hasFocus())
                clearFocus();
            if (isSelected())
                setSelected(false);
        }
        d->enabled = quint32(enabled);
        update();
    }
}

/*!
    Returns true if this item is selected; otherwise, false is returned.

    Items are not selected by default.
    
    \sa setSelected(), QGraphicsScene::setSelectionArea()
*/
bool QGraphicsItem::isSelected() const
{
    Q_D(const QGraphicsItem);
    return d->selected;
}

/*!
    If \a selected is true and this item is selectable, this item is selected;
    otherwise, it is unselected.

    Only visible, enabled, selectable items can be selected.  If \a selected
    is true and this item is either invisible or disabled or unselectable,
    this function does nothing.

    By default, items cannot be selected. To enable selection, set the
    ItemIsSelectable flag.

    This function is provided for convenience, allowing individual toggling of
    the selected state of an item. However, a more common way of selecting
    items is to call QGraphicsScene::setSelectionArea(), which will call this
    function for all items within a specified area on the scene.
    
    \sa isSelected(), QGraphicsScene::selectedItems()
*/
void QGraphicsItem::setSelected(bool selected)
{
    Q_D(QGraphicsItem);
    if (!(d->flags & ItemIsSelectable) || !d->enabled || !d->visible)
        selected = false;
    if (d->selected != selected) {
        d->selected = quint32(selected);
        update();
        if (selected && d->scene)
            d->scene->d_func()->selectedItems << this;
    }
}

/*!
    Returns true if this item accepts mouse events; otherwise, false is
    returned.

    By default, all items accept mouse events. If the item does not accept
    mouse events, QGraphicsScene will forward the mouse events to the first
    item beneith it that does accept mouse events.
    
    \sa setAcceptsMouseEvents(), setAcceptsHoverEvents()
*/
bool QGraphicsItem::acceptsMouseEvents() const
{
    Q_D(const QGraphicsItem);
    return d->acceptsMouse;
}

/*!
    If \a enabled is true, this item will accept mouse events; otherwise, it
    will not accept mouse events.

    If \a enabled is false and this item is the current mouse grabber, it will
    lose the mouse grab. Setting this value back to true will then not cause
    the item to regain the grab. To grab the mouse, it must again receive a
    mouse press event.

    If the item does not accept mouse events, QGraphicsScene will forward the
    mouse events to the first item beneith it that does accept mouse events.

    \sa acceptsMouseEvents()
*/
void QGraphicsItem::setAcceptsMouseEvents(bool enabled)
{
    Q_D(QGraphicsItem);
    if (d->acceptsMouse != enabled) {
        if (!enabled && d->scene && d->scene->mouseGrabberItem() == this)
            d->scene->d_func()->mouseGrabberItem = 0;
        d->acceptsMouse = quint32(enabled);
    }
}

/*!
    Returns true if an item accepts hover events (QGraphicsSceneHoverEvent);
    otherwise, returns false. By default, items do not accept hover events.

    \sa setAcceptsMouseEvents()
*/
bool QGraphicsItem::acceptsHoverEvents() const
{
    Q_D(const QGraphicsItem);
    return d->acceptsHover;
}

/*!
    If \a enabled is true, this item will accept hover events; otherwise, it
    will ignore them. By default, items do not accept hover events.

    \sa acceptsHoverEvents()
*/
void QGraphicsItem::setAcceptsHoverEvents(bool enabled)
{
    Q_D(QGraphicsItem);
    d->acceptsHover = quint32(enabled);
}

/*!
    Returns true if this item has focus (i.e., can accept key events);
    otherwise, returns false.

    \sa setFocus(), QGraphicsScene::setFocusItem()
*/
bool QGraphicsItem::hasFocus() const
{
    Q_D(const QGraphicsItem);
    return d->scene && d->scene->focusItem() == this;
}

/*!
    Gives keyboard input focus to this item. The \a focusReason argument will
    be passed into any focus event generated by this function; it is used to
    give an explanation of what caused the item to get focus.

    Only items that set the ItemIsFocusable flag can accept keyboard focus.

    If this item is not visible (i.e., isVisible() returns false), not
    enabled, not associated with a scene, or if it already has input focus,
    this function will do nothing.

    As a result of calling this function, this item will receive a focus in
    event with \a focusReason. If another item already has focus, that item
    will first receive a focus out event indicating that it has lost input
    focus.

    \sa clearFocus(), hasFocus()
*/
void QGraphicsItem::setFocus(Qt::FocusReason focusReason)
{
    Q_D(QGraphicsItem);
    if (!d->scene || !isVisible() || !isEnabled() || hasFocus())
        return;
    d->scene->setFocusItem(this, focusReason);
}

/*!
    Takes keyboard input focus from the item.

    If it has focus, a focus out event is sent to this item to tell it that it
    is about to lose the focus.

    Only items that set the ItemIsFocusable flag can accept keyboard focus.

    \sa setFocus()
*/
void QGraphicsItem::clearFocus()
{
    Q_D(QGraphicsItem);
    if (!d->scene || !hasFocus())
        return;
    d->scene->setFocusItem(0);
}

/*!
    Returns the position of the item in parent coordinates. If the item has no
    parent, its position is given in scene coordinates.

    The position of the item describes its local coordinate (0, 0) in parent
    coordinates. For this reason, this function always return the same as
    mapToParent(0, 0).

    For convenience, you can also call scenePos() to determine the item's
    position in scene coordinates, regardless of its parent.

    \sa setPos(), matrix()
*/
QPointF QGraphicsItem::pos() const
{
    Q_D(const QGraphicsItem);
    return d->pos;
}

/*!
    Returns the item's position in scene coordinates. This is equivalent to
    calling mapToScene(0, 0).

    \sa pos(), sceneMatrix()
*/
QPointF QGraphicsItem::scenePos() const
{
    return mapToScene(0, 0);
}

/*!
    Sets the position of the item to \a pos, which is in parent coordinates.
    For items with no parent, \a pos is in scene coordinates.

    \sa pos(), scenePos()
*/
void QGraphicsItem::setPos(const QPointF &pos)
{
    Q_D(QGraphicsItem);
    if (!d->scene) {
        d->pos = pos;
        return;
    }

    qt_graphicsItem_fullUpdate(this);
    removeFromIndex();
    d->pos = pos;
    qt_graphicsItem_fullUpdate(this);
    addToIndex();
}

/*!
    Returns this item's transformation matrix. If no matrix has been set, the
    identity matrix is returned.

    \sa setMatrix(), sceneMatrix()
*/
QMatrix QGraphicsItem::matrix() const
{
    Q_D(const QGraphicsItem);
    return d->matrix();
}

/*!
    Returns this item's scene transformation matrix. This matrix can be used
    to map coordinates and geometrical shapes from this item's local
    coordinate system to the scene's coordinate system. To map coordinates
    from the scene, you must first invert the returned matrix.

    Example:

    \code
        QGraphicsRectItem rect;
        rect.setPos(100, 100);

        rect.sceneMatrix().map(QPointF(0, 0));
        // returns QPointF(100, 100);

        rect.sceneMatrix().inverted().map(QPointF(100, 100));
        // returns QPointF(0, 0);
    \endcode

    Unlike matrix(), which returns only an item's local transformation, this
    function includes the item's (and any parents') position.

    \sa matrix(), setMatrix(), scenePos()
*/
QMatrix QGraphicsItem::sceneMatrix() const
{
    Q_D(const QGraphicsItem);
    QMatrix m = d->matrix() * QMatrix().translate(d->pos.x(), d->pos.y());
    return d->parent ? m * d->parent->sceneMatrix() : m;
}

/*!
    Sets the item's current transformation matrix to \a matrix.

    If \a combine is true, then \a matrix is combined with the current matrix;
    otherwise, \a matrix \e replaces the current matrix. \a combine is false
    by default.
    
    To simplify interation with items using a transformed view, QGraphicsItem
    provides mapTo... and mapFrom... functions that can translate between
    items' and the scene's coordinates. For example, you can call mapToScene()
    to map an item coordiate to a scene coordinate, or mapFromScene() to map
    from scene coordinates to item coordinates.

    \sa matrix(), rotate(), scale(), shear(), translate()
*/
void QGraphicsItem::setMatrix(const QMatrix &matrix, bool combine)
{
    Q_D(QGraphicsItem);

    QMatrix oldMatrix = d->matrix();
    QMatrix newMatrix;
    if (!combine)
        newMatrix = matrix;
    else
        newMatrix = matrix * oldMatrix;
    if (oldMatrix == newMatrix)
        return;

    qt_graphicsItem_fullUpdate(this);
    removeFromIndex();
    d->setMatrix(newMatrix);
    addToIndex();
    qt_graphicsItem_fullUpdate(this);
}

/*!
    Resets this item's tranformation matrix to the identity matrix. This is
    equivalent to calling setMatrix(QMatrix()).

    \sa setMatrix(), matrix()
*/
void QGraphicsItem::resetMatrix()
{
    setMatrix(QMatrix(), false);
}

/*!
    Rotates the current item transformation \a angle degrees clockwise.

    \sa setMatrix(), matrix(), scale(), shear(), translate()
*/
void QGraphicsItem::rotate(qreal angle)
{
    setMatrix(QMatrix().rotate(angle), true);
}

/*!
    Scales the current item transformation by (\a sx, \a sy).

    \sa setMatrix(), matrix(), rotate(), shear(), translate()
*/
void QGraphicsItem::scale(qreal sx, qreal sy)
{
    setMatrix(QMatrix().scale(sx, sy), true);
}

/*!
    Shears the current item transformation by (\a sh, \a sv).

    \sa setMatrix(), matrix(), rotate(), scale(), translate()
*/
void QGraphicsItem::shear(qreal sh, qreal sv)
{
    setMatrix(QMatrix().shear(sh, sv), true);
}

/*!
    Translates the current item transformation by (\a dx, \a dy).

    \sa setMatrix(), matrix(), rotate(), scale(), shear()
*/
void QGraphicsItem::translate(qreal dx, qreal dy)
{
    setMatrix(QMatrix().translate(dx, dy), true);
}

/*!
    Returns the Z-value, or the elevation, of the item. The Z-value decides
    the stacking order of sibling (neighboring) items.

    The default Z-value is 0.
    
    \sa setZValue()
*/
qreal QGraphicsItem::zValue() const
{
    Q_D(const QGraphicsItem);
    return d->z;
}

/*!
    Sets the Z-value, or the elevation, of the item, to \a z. The elevation
    decides the stacking order of sibling (neighboring) items. An item of high
    Z-value will be drawn on top of an item with a lower Z-value if their
    parent is identical. In addition, children of an item will always be drawn
    on top of the parent, regardless of the child's Z-value. Sibling items
    that share the same Z-value will be drawn in an undefined order, although
    the order will stay the same for as long as the items live.

    The Z-value does not affect the item's size in any way.

    The default Z-value is 0.

    \sa zValue()
*/
void QGraphicsItem::setZValue(qreal z)
{
    Q_D(QGraphicsItem);
    if (z != d->z) {
        d->z = z;
        qt_graphicsItem_fullUpdate(this);
    }
}

/*!
    Returns the bounding rect of this item in scene coordinates, by combining
    sceneMatrix() with boundingRect().
*/
QRectF QGraphicsItem::sceneBoundingRect() const
{
    return sceneMatrix().mapRect(boundingRect());
}

/*!
    Returns the shape of this item as a QPainterPath in local coordinates. The
    shape is used for collision detection and hit tests.

    The default implementation calls boundingRect() to return a simple '
    rectangular shape, but subclasses can reimplement this function to return
    a more accurate shape for non-rectangular items. For example, a round item
    may choose to return an elliptic shape for better collision detection. For
    example:

    \code
        QPainterPath RoundItem::shape() const
        {
            QPainterPath path;
            path.addEllipse(boundingRect());
            return path;
        }
    \endcode

    This function is called by the default implementations of contains() and
    collidesWith().

    \sa boundingRect(), contains()
*/
QPainterPath QGraphicsItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

/*!
    Returns true if this item contains \a point, which is in local
    coordinates; otherwise, false is returned. It is most often called from
    QGraphicsView to determine what item is under the cursor, and for that
    reason, the implementation of this function should be as light-weight as
    possible.

    By default, this function calls shape(), but you can reimplement it in a
    subclass to provide a (perhaps more efficient) implementation.

    \sa shape(), boundingRect(), collidesWith()
*/
bool QGraphicsItem::contains(const QPointF &point) const
{
    return shape().contains(point);
}

/*!
    Returns true if this item collides with \a other; otherwise returns false.
    In item is said to collide with another if the items either intersect, or
    if either item is contained within the other's area.

    The default implementation is based on shape intersection, and it calls
    shape() on both items. Because arbitrary shape-shape intersection grows
    with an order of magnitude of complexity, this operation can be noticably
    time consuming. You have the option of reimplementing this function in a
    subclass of QGraphicsItem to provide a custom algorithm. This allows you
    to make use of natural constraints in the shapes of your own items. For
    instance, two untransformed perfectly circular items' collision can be
    determined very efficiently by comparing positions and radii.

    \sa contains(), shape()
*/
bool QGraphicsItem::collidesWith(QGraphicsItem *other) const
{
    if (other == this)
        return true;
    if (!other)
        return false;
    
    QMatrix matrixA = sceneMatrix();
    QMatrix matrixB = other->sceneMatrix();

    QRectF rectA = matrixA.mapRect(boundingRect());
    QRectF rectB = matrixB.mapRect(other->boundingRect());
    if (!rectA.intersects(rectB) && !rectA.contains(rectB)) {
        // This we can determine efficiently. If the two rects neither
        // intersect nor contain eachother, then the two items do not collide.
        return false;
    }

    return collidesWith(matrixA.inverted().map(matrixB.map(other->shape())));
}

/*!
    \overload

    Returns true if this item collides with \a path, which is in local
    coordinates.

    \sa contains(), shape()
*/
bool QGraphicsItem::collidesWith(const QPainterPath &path) const
{
    QMatrix matrix = sceneMatrix();
    
    QRectF rectA = boundingRect();
    QRectF rectB = path.controlPointRect();
    if (!rectA.intersects(rectB) && !rectA.contains(rectB) && !rectB.contains(rectA)) {
        // This we can determine efficiently. If the two rects neither
        // intersect nor contain eachother, then the two items do not collide.
        return false;
    }

    // When precisely on top of eachother, they collide.
    QPainterPath thisPath = shape();
    if (thisPath == path)
        return true;

    // Convert this item and the other item's areas to polygons.
    QList<QPolygonF> polysA = thisPath.toFillPolygons();
    QList<QPolygonF> polysB = path.toFillPolygons();

    // Check if any lines intersect, O(N^4)
    for (int a = 0; a < polysA.size(); ++a) {
        const QPolygonF &polyA = polysA.at(a);
        for (int i = 1; i < polyA.size(); ++i) {
            QLineF lineA(polyA.at(i - 1), polyA.at(i));
            for (int b = 0; b < polysB.size(); ++b) {
                const QPolygonF &polyB = polysB.at(b);
                for (int j = 1; j < polyB.size(); ++j) {
                    QLineF lineB(polyB.at(j - 1), polyB.at(j));
                    if (lineA.intersect(lineB, 0) == QLineF::BoundedIntersection)
                        return true;
                }
            }
        }
    }

    // No intersections, check if any point in A is inside B.
    QPointF pointA;
    if (!polysA.isEmpty()) {
        const QPolygonF &polyA = polysA.first();
        if (!polyA.isEmpty()) {
            if (path.contains(polyA.first()))
                return true;
        }
    }
    return false;
}

/*!
    Schedules a redraw of the area covered by \a rect in this item. You can
    call this function whenever your item needs to be redrawn, such as if it
    changes appearance or size.

    This function does not cause an immediate paint; instead it schedules a
    paint request that is processed by QGraphicsView after control reaches the
    event loop. The item will only be redrawn if it is visible in any
    associated view.

    As a side effect of the item being repainted, other items that overlap the
    area \a rect may also be repainted.

    If the item is invisible (i.e., isVisible() returns false), this function
    does nothing.

    \sa paint(), boundingRect()
*/
void QGraphicsItem::update(const QRectF &rect)
{
    Q_D(QGraphicsItem);
    if (d->scene && isVisible())
        d->scene->itemUpdated(this, rect);
}

/*!
    Maps the point \a point, which is in this item's coordinate system, to \a
    item's coordinate system, and returns the mapped coordinate.

    \sa mapToParent(), mapToScene(), matrix(), mapFromItem()
*/
QPointF QGraphicsItem::mapToItem(QGraphicsItem *item, const QPointF &point) const
{
    return item->mapFromScene(mapToScene(point));
}

/*!
    Maps the point \a point, which is in this item's coordinate system, to its
    parent's coordinate system, and returns the mapped coordinate. If the item
    has no parent, \a point will be mapped to the scene's coordinate system.

    \sa mapToItem(), mapToScene(), matrix(), mapFromParent()
*/
QPointF QGraphicsItem::mapToParent(const QPointF &point) const
{
    Q_D(const QGraphicsItem);
    return matrix().map(point) + d->pos;
}

/*!
    Maps the point \a point, which is in this item's coordinate system, to the
    scene's coordinate system, and returns the mapped coordinate.

    \sa mapToItem(), mapToParent(), matrix(), mapFromScene()
*/
QPointF QGraphicsItem::mapToScene(const QPointF &point) const
{
    Q_D(const QGraphicsItem);
    return d->parent ? d->parent->mapToScene(mapToParent(point)) : mapToParent(point);
}

/*!
    Maps the point \a point, which is in \a item's coordinate system, to this
    item's coordinate system, and returns the mapped coordinate.

    \a mapFromParent(), mapFromScene(), matrix(), mapToItem()
*/
QPointF QGraphicsItem::mapFromItem(QGraphicsItem *item, const QPointF &point) const
{
    return mapFromScene(item->mapToScene(point));
}

/*!
    Maps the point \a point, which is in this item's parent's coordinate
    system, to this item's coordinate system, and returns the mapped
    coordinate.

    \a mapFromItem(), mapFromScene(), matrix(), mapToParent()
*/
QPointF QGraphicsItem::mapFromParent(const QPointF &point) const
{
    Q_D(const QGraphicsItem);
    return matrix().inverted().map(point - d->pos);
}

/*!
    Maps the point \a point, which is in this item's scene's coordinate
    system, to this item's coordinate system, and returns the mapped
    coordinate.

    \a mapFromItem(), mapFromParent(), matrix(), mapToScene()
*/
QPointF QGraphicsItem::mapFromScene(const QPointF &point) const
{
    Q_D(const QGraphicsItem);
    return d->parent ? mapFromParent(d->parent->mapFromScene(point)) : mapFromParent(point);
}

/*!
    Returns true if this item is an ancestor of \a child (i.e., if this item
    is \a child's parent, or one of \a child's parent's ancestors).

    \sa parentItem()
*/
bool QGraphicsItem::isAncestorOf(const QGraphicsItem *child) const
{
    Q_D(const QGraphicsItem);
    if (!child || child == this)
        return false;
    for (int i = 0; i < d->children.size(); ++i)
        return d->children.at(i) == child || d->children.at(i)->isAncestorOf(child);
    return false;
}

class QGraphicsItemCustomDataStore
{
public:
    QMap<QGraphicsItem *, QMap<int, QVariant> > data;
};
Q_GLOBAL_STATIC(QGraphicsItemCustomDataStore, qt_dataStore);

/*!
    Returns this item's custom data for the key \a key as a QVariant.

    Custom item data is useful for storing arbitrary properties in any
    item. Example:

    \code
        static const int ObjectName = 0;
    
        QGraphicsItem *item = scene.itemAt(100, 50);
        if (item->data(ObjectName).toString().isEmpty()) {
            if (qgraphicsitem_cast<ButtonItem *>(item))
                item->setData(ObjectName, "Button");
        }
    \endcode

    Qt does not use this feature for storing data; it is provided solely
    for the convenience of the user.    

    \sa setData()
*/
QVariant QGraphicsItem::data(int key) const
{
    QGraphicsItemCustomDataStore *store = qt_dataStore();
    if (!store->data.contains(const_cast<QGraphicsItem *>(this)))
        return QVariant();
    return store->data.value(const_cast<QGraphicsItem *>(this)).value(key);
}

/*!
    Sets this item's custom data for the key \a key to \a value.

    Custom item data is useful for storing arbitrary properties for any
    item. Qt does not use this feature for storing data; it is provided solely
    for the convenience of the user.

    \sa data()
*/
void QGraphicsItem::setData(int key, const QVariant &value)
{
    qt_dataStore()->data[this][key] = value;
}

/*!
    template <class T> inline T qgraphicsitem_cast(QGraphicsItem *item)

    Returns the given \a item cast to type T if \a item is of type T;
    otherwise, 0 is returned.
*/

/*!
    Returns the type of an item as an int. All standard graphicsitem classes
    are associated with a unique value; see QGraphicsItem::Type. This type
    information is used by qgraphicsitem_cast() to distinguish between types.

    Reimplementing this function will enable use of qgraphicsitem_cast() with
    the item. Custom items must return a value larger than UserType
    (0x80000000).
*/
int QGraphicsItem::type() const
{
    return UserType;
}

void QGraphicsItem::installEventFilter(QGraphicsItem *filterItem)
{
    Q_D(QGraphicsItem);
    if (!d->scene) {
        qWarning("QGraphicsItem::installEventFilter: event filters can only be installed"
                 " on items in a scene.");
        return;
    }
    if (d->scene != filterItem->scene()) {
        qWarning("QGraphicsItem::installEventFilter: event filters can only be installed"
                 " on items in the same scene.");
        return;
    }
    d->scene->d_func()->installEventFilter(filterItem, this);
}

void QGraphicsItem::removeEventFilter(QGraphicsItem *filterItem)
{
    Q_D(QGraphicsItem);
    if (!d->scene || d->scene != filterItem->scene())
        return;
    d->scene->d_func()->removeEventFilter(filterItem, this);
}

bool QGraphicsItem::eventFilter(QGraphicsItem *watched, QGraphicsSceneEvent *event)
{
    Q_UNUSED(watched);
    Q_UNUSED(event);
    return false;
}

void QGraphicsItem::sceneEvent(QEvent *event)
{
    Q_D(QGraphicsItem);
    if (!d->enabled || !d->visible)
        return;
    
    switch (event->type()) {
    case QEvent::GraphicsSceneContextMenu:
        contextMenuEvent(static_cast<QGraphicsSceneContextMenuEvent *>(event));
        break;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
        focusEvent(static_cast<QFocusEvent *>(event));
        break;
    case QEvent::GraphicsSceneHoverEnter:
    case QEvent::GraphicsSceneHoverMove:
    case QEvent::GraphicsSceneHoverLeave:
        hoverEvent(static_cast<QGraphicsSceneHoverEvent *>(event));
        break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        keyEvent(static_cast<QKeyEvent *>(event));
        break;
    case QEvent::GraphicsSceneMouseMove:
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseRelease:
    case QEvent::GraphicsSceneMouseClick:
    case QEvent::GraphicsSceneMouseDoubleClick:
        mouseEvent(static_cast<QGraphicsSceneMouseEvent *>(event));
        break;
    default:
        break;
    }
}

void QGraphicsItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
}

void QGraphicsItem::focusEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
}

void QGraphicsItem::hoverEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    switch (event->type()) {
    case QEvent::GraphicsSceneHoverEnter:
    case QEvent::GraphicsSceneHoverLeave:
        update();
        break;
    case QEvent::GraphicsSceneHoverMove:
    default:
        break;
    }
}

void QGraphicsItem::keyEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
    switch (event->type()) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        break;
    default:
        break;
    }
}

void QGraphicsItem::mouseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_D(QGraphicsItem);

    switch (event->type()) {
    case QEvent::GraphicsSceneMouseMove:
        if ((event->buttons() & Qt::LeftButton) && (flags() & ItemIsMovable)) {
	    QPointF newPos(mapToParent(event->pos()) - matrix().map(event->buttonDownPos(Qt::LeftButton)));
	    QPointF diff = newPos - pos();

	    // Determine the list of selected items
	    QList<QGraphicsItem *> selectedItems;
	    if (d->scene) {
		selectedItems = d->scene->selectedItems();
	    } else if (QGraphicsItem *parent = parentItem()) {
		while (parent && parent->isSelected())
		    selectedItems << parent;
	    }
	    selectedItems << this;

	    // Move all selected items
	    foreach (QGraphicsItem *item, selectedItems) {
		if (!item->parentItem() || !item->parentItem()->isSelected())
		    item->setPos(item == this ? newPos : item->pos() + diff);
	    }
        }
        break;
    case QEvent::GraphicsSceneMouseDoubleClick:
    case QEvent::GraphicsSceneMousePress:
	if (event->button() == Qt::LeftButton && (flags() & ItemIsSelectable) && !d->selected) {
            if (d->scene) {
                if ((event->modifiers() & Qt::ControlModifier) == 0)
                    d->scene->clearSelection();
                d->scene->d_func()->selectedItems << this;
            }
            d->selected = 1;
            update();
        }
        break;
    case QEvent::GraphicsSceneMouseRelease:
        if (d->selected && !contains(event->buttonDownPos(Qt::LeftButton))) {
            if (d->scene) {
                if ((event->modifiers() & Qt::ControlModifier) == 0)
                    d->scene->clearSelection();
                d->scene->d_func()->selectedItems << this;
                d->selected = 1;
                update();
            }
        }
        break;
    case QEvent::GraphicsSceneMouseClick:
        break;
    default:
        break;
    }
}

void QGraphicsItem::inputMethodEvent(QInputMethodEvent *event)
{
    event->ignore();
}

QVariant QGraphicsItem::inputMethodQuery(Qt::InputMethodQuery query) const
{
    Q_UNUSED(query);
    return QVariant();
}

/*!
    \internal
*/
bool QGraphicsItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

void QGraphicsItem::addToIndex()
{
    Q_D(QGraphicsItem);
    if (d->scene)
        d->scene->d_func()->addToIndex(this);
}

void QGraphicsItem::removeFromIndex()
{
    Q_D(QGraphicsItem);
    if (d->scene)
        d->scene->d_func()->removeFromIndex(this);
}

class QAbstractGraphicsPathItemPrivate : public QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QAbstractGraphicsPathItem)
public:

    QBrush brush;
    QPen pen;
};

QAbstractGraphicsPathItem::QAbstractGraphicsPathItem(QGraphicsItem *parent)
    : QGraphicsItem(parent)
{
}

QAbstractGraphicsPathItem::QAbstractGraphicsPathItem(QAbstractGraphicsPathItemPrivate &dd,
                                                     QGraphicsItem *parent)
    : QGraphicsItem(dd, parent)
{
}

QAbstractGraphicsPathItem::~QAbstractGraphicsPathItem()
{
}

QPen QAbstractGraphicsPathItem::pen() const
{
    Q_D(const QAbstractGraphicsPathItem);
    return d->pen;
}

void QAbstractGraphicsPathItem::setPen(const QPen &pen)
{
    Q_D(QAbstractGraphicsPathItem);
    d->pen = pen;
}

QBrush QAbstractGraphicsPathItem::brush() const
{
    Q_D(const QAbstractGraphicsPathItem);
    return d->brush;
}

void QAbstractGraphicsPathItem::setBrush(const QBrush &brush)
{
    Q_D(QAbstractGraphicsPathItem);
    d->brush = brush;
}

class QGraphicsPathItemPrivate : public QAbstractGraphicsPathItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsPathItem)
public:
    QPainterPath path;
    QRectF boundingRect;
};

QGraphicsPathItem::QGraphicsPathItem(const QPainterPath &path, QGraphicsItem *parent)
    : QAbstractGraphicsPathItem(*new QGraphicsPathItemPrivate, parent)
{
    if (!path.isEmpty())
        setPath(path);
}

QGraphicsPathItem::~QGraphicsPathItem()
{
}

QPainterPath QGraphicsPathItem::path() const
{
    Q_D(const QGraphicsPathItem);
    return d->path;
}

void QGraphicsPathItem::setPath(const QPainterPath &path)
{
    Q_D(QGraphicsPathItem);
    update();
    d->path = path;
    d->boundingRect = path.controlPointRect();
    update();
}

QRectF QGraphicsPathItem::boundingRect() const
{
    Q_D(const QGraphicsPathItem);
    qreal penWidth = d->pen.widthF() / 2.0;
    return d->boundingRect.adjusted(-penWidth, -penWidth,
                                    penWidth, penWidth);
}

QPainterPath QGraphicsPathItem::shape() const
{
    Q_D(const QGraphicsPathItem);
    return d->path;
}

bool QGraphicsPathItem::contains(const QPointF &point) const
{
    return QAbstractGraphicsPathItem::contains(point);
}

void QGraphicsPathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_D(QGraphicsPathItem);
    Q_UNUSED(widget);
    painter->setPen(d->pen);
    painter->setBrush(d->brush);
    painter->drawPath(d->path);

    if (option->state & QStyle::State_Selected) {
	painter->setPen(QPen(option->palette.text(), 1.0, Qt::DashLine));
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));
    }
}

int QGraphicsPathItem::type() const
{
    return Type;
}

/*!
    \internal
*/
bool QGraphicsPathItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsPathItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsPathItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

class QGraphicsRectItemPrivate : public QAbstractGraphicsPathItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsRectItem)
public:
    QRectF rect;
};

QGraphicsRectItem::QGraphicsRectItem(const QRectF &rect, QGraphicsItem *parent)
    : QAbstractGraphicsPathItem(*new QGraphicsRectItemPrivate, parent)
{
    setRect(rect);
}

QGraphicsRectItem::~QGraphicsRectItem()
{
}

QRectF QGraphicsRectItem::rect() const
{
    Q_D(const QGraphicsRectItem);
    return d->rect;
}

void QGraphicsRectItem::setRect(const QRectF &rect)
{
    Q_D(QGraphicsRectItem);
    update();
    d->rect = rect;
    update();
}

QRectF QGraphicsRectItem::boundingRect() const
{
    Q_D(const QGraphicsRectItem);
    qreal penWidth = d->pen.widthF() / 2.0;
    return d->rect.adjusted(-penWidth, -penWidth,
                            penWidth, penWidth);
}

QPainterPath QGraphicsRectItem::shape() const
{
    Q_D(const QGraphicsRectItem);
    QPainterPath path;
    path.addRect(d->rect);
    return path;
}

bool QGraphicsRectItem::contains(const QPointF &point) const
{
    Q_D(const QGraphicsRectItem);
    return d->rect.contains(point);
}

void QGraphicsRectItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_D(QGraphicsRectItem);
    Q_UNUSED(widget);
    painter->setPen(d->pen);
    painter->setBrush(d->brush);
    painter->drawRect(d->rect);

    if (option->state & QStyle::State_Selected) {
	painter->setPen(QPen(option->palette.text(), 1.0, Qt::DashLine));
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));
    }
}

int QGraphicsRectItem::type() const
{
    return Type;
}


/*!
    \internal
*/
bool QGraphicsRectItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsRectItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsRectItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

class QGraphicsEllipseItemPrivate : public QAbstractGraphicsPathItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsEllipseItem)
public:
    QRectF rect;
};

QGraphicsEllipseItem::QGraphicsEllipseItem(const QRectF &rect, QGraphicsItem *parent)
    : QAbstractGraphicsPathItem(*new QGraphicsEllipseItemPrivate, parent)
{
    setRect(rect);
}

QGraphicsEllipseItem::~QGraphicsEllipseItem()
{
}

QRectF QGraphicsEllipseItem::rect() const
{
    Q_D(const QGraphicsEllipseItem);
    return d->rect;
}

void QGraphicsEllipseItem::setRect(const QRectF &rect)
{
    Q_D(QGraphicsEllipseItem);
    update();
    d->rect = rect;
    update();
}

QRectF QGraphicsEllipseItem::boundingRect() const
{
    Q_D(const QGraphicsEllipseItem);
    qreal penWidth = d->pen.widthF() / 2.0;
    return d->rect.adjusted(-penWidth, -penWidth,
                            penWidth, penWidth);
}

QPainterPath QGraphicsEllipseItem::shape() const
{
    Q_D(const QGraphicsEllipseItem);
    QPainterPath path;
    path.addEllipse(d->rect);
    return path;
}

bool QGraphicsEllipseItem::contains(const QPointF &point) const
{
    return QAbstractGraphicsPathItem::contains(point);
}

void QGraphicsEllipseItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_D(QGraphicsEllipseItem);
    Q_UNUSED(widget);
    painter->setPen(d->pen);
    painter->setBrush(d->brush);
    painter->drawEllipse(d->rect);

    if (option->state & QStyle::State_Selected) {
	painter->setPen(QPen(option->palette.text(), 1.0, Qt::DashLine));
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));
    }
}

int QGraphicsEllipseItem::type() const
{
    return Type;
}


/*!
    \internal
*/
bool QGraphicsEllipseItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsEllipseItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsEllipseItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

class QGraphicsPolygonItemPrivate : public QAbstractGraphicsPathItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsPolygonItem)
public:
    QPolygonF polygon;
};

QGraphicsPolygonItem::QGraphicsPolygonItem(const QPolygonF &polygon, QGraphicsItem *parent)
    : QAbstractGraphicsPathItem(*new QGraphicsPolygonItemPrivate, parent)
{
    setPolygon(polygon);
}

QGraphicsPolygonItem::~QGraphicsPolygonItem()
{
}

QPolygonF QGraphicsPolygonItem::polygon() const
{
    Q_D(const QGraphicsPolygonItem);
    return d->polygon;
}

void QGraphicsPolygonItem::setPolygon(const QPolygonF &polygon)
{
    Q_D(QGraphicsPolygonItem);
    update();
    d->polygon = polygon;
    update();
}

QRectF QGraphicsPolygonItem::boundingRect() const
{
    Q_D(const QGraphicsPolygonItem);
    qreal penWidth = d->pen.widthF() / 2.0;
    return d->polygon.boundingRect().adjusted(-penWidth, -penWidth,
                                penWidth, penWidth);
}

QPainterPath QGraphicsPolygonItem::shape() const
{
    Q_D(const QGraphicsPolygonItem);
    QPainterPath path;
    path.addPolygon(d->polygon);
    return path;
}

bool QGraphicsPolygonItem::contains(const QPointF &point) const
{
    return QAbstractGraphicsPathItem::contains(point);
}

void QGraphicsPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_D(QGraphicsPolygonItem);
    Q_UNUSED(widget);
    painter->setPen(d->pen);
    painter->setBrush(d->brush);
    painter->drawPolygon(d->polygon);

    if (option->state & QStyle::State_Selected) {
	painter->setPen(QPen(option->palette.text(), 1.0, Qt::DashLine));
	painter->setBrush(Qt::NoBrush);
	painter->drawRect(boundingRect().adjusted(2, 2, -2, -2));
    }
}

int QGraphicsPolygonItem::type() const
{
    return Type;
}


/*!
    \internal
*/
bool QGraphicsPolygonItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsPolygonItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsPolygonItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

class QGraphicsLineItemPrivate : public QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsLineItem)
public:
    QLineF line;
    QPen pen;
};

QGraphicsLineItem::QGraphicsLineItem(const QLineF &line, QGraphicsItem *parent)
    : QGraphicsItem(*new QGraphicsLineItemPrivate, parent)
{
    setLine(line);
}

QGraphicsLineItem::~QGraphicsLineItem()
{
}

QPen QGraphicsLineItem::pen() const
{
    Q_D(const QGraphicsLineItem);
    return d->pen;
}

void QGraphicsLineItem::setPen(const QPen &pen)
{
    Q_D(QGraphicsLineItem);
    d->pen = pen;
}

QLineF QGraphicsLineItem::line() const
{
    Q_D(const QGraphicsLineItem);
    return d->line;
}

void QGraphicsLineItem::setLine(const QLineF &line)
{
    Q_D(QGraphicsLineItem);
    update();
    d->line = line;
    update();
}

QRectF QGraphicsLineItem::boundingRect() const
{
    Q_D(const QGraphicsLineItem);
    qreal penWidth = d->pen.widthF() / 2.0;
    
    return shape().controlPointRect()
        .adjusted(-penWidth, -penWidth, penWidth, penWidth);
}

QPainterPath QGraphicsLineItem::shape() const
{
    Q_D(const QGraphicsLineItem);
    QPainterPath path;
    if (d->line.isNull())
        return path;
    path.addPolygon(QPolygonF() << d->line.p1() << d->line.p2());
    return path;
}

bool QGraphicsLineItem::contains(const QPointF &point) const
{
    Q_D(const QGraphicsLineItem);
    
    const qreal twopi = 3.14159265359 * 2.0;

    QLineF hypo = QLineF(d->line.p1(), point);

    qreal angle = (twopi * d->line.angle(hypo)) / 360.0;
    qreal dist = hypo.length() * ::sin(angle);
    if ((d->pen.widthF() == 0 && dist > 0.5) || (d->pen.widthF() > 0 && dist > d->pen.widthF() / 2))
        return false;

    return QRectF(d->line.p1(), QSizeF(d->line.dx(), d->line.dy())).normalized().contains(point);
}

void QGraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_D(QGraphicsLineItem);
    Q_UNUSED(widget);
    painter->setPen(d->pen);
    painter->drawLine(d->line);

    if (option->state & QStyle::State_Selected) {
	painter->setPen(QPen(option->palette.text(), 1.0, Qt::DashLine));
	painter->setBrush(Qt::NoBrush);

        QPointF p1 = d->line.p1();
        QPointF p2 = d->line.p2();
        qreal leftMostX;
        qreal topMostY;
        qreal width;
        qreal height;
        if (p1.x() < p2.x())
            leftMostX = p1.x();
        else
            leftMostX = p2.x();
        if (p1.y() < p2.y())
            topMostY = p1.y();
        else
            topMostY = p2.y();
        width = p1.x() - p2.x();
        if (width < 0)
            width = -width;
        height = p1.y() - p2.y();
        if (height < 0)
            height = -height;

        painter->drawRect(QRectF(leftMostX, topMostY, width, height));
    }
}

int QGraphicsLineItem::type() const
{
    return Type;
}


/*!
    \internal
*/
bool QGraphicsLineItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsLineItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsLineItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

class QGraphicsPixmapItemPrivate : public QGraphicsItemPrivate
{
    Q_DECLARE_PUBLIC(QGraphicsPixmapItem)
public:
    QPixmap pixmap;
};

QGraphicsPixmapItem::QGraphicsPixmapItem(const QPixmap &pixmap, QGraphicsItem *parent)
    : QGraphicsItem(*new QGraphicsPixmapItemPrivate, parent)
{
    setPixmap(pixmap);
}

QGraphicsPixmapItem::~QGraphicsPixmapItem()
{
}

void QGraphicsPixmapItem::setPixmap(const QPixmap &pixmap)
{
    Q_D(QGraphicsPixmapItem);
    if (!d->pixmap.isNull())
        update();
    d->pixmap = pixmap;
    update();
}

QPixmap QGraphicsPixmapItem::pixmap() const
{
    Q_D(const QGraphicsPixmapItem);
    return d->pixmap;
}

QRectF QGraphicsPixmapItem::boundingRect() const
{
    Q_D(const QGraphicsPixmapItem);
    return d->pixmap.isNull() ? QRectF() : QRectF(QPointF(0, 0), d->pixmap.size());
}

QPainterPath QGraphicsPixmapItem::shape() const
{
    Q_D(const QGraphicsPixmapItem);
    QPainterPath path;
    QBitmap mask = d->pixmap.mask();
    if (mask.isNull())
        path.addRect(QRectF(0, 0, d->pixmap.width(), d->pixmap.height()));
    else
        path.addRegion(QRegion(mask));
    return path;
}

bool QGraphicsPixmapItem::contains(const QPointF &point) const
{
    return QGraphicsItem::contains(point);
}

void QGraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_D(QGraphicsPixmapItem);
    Q_UNUSED(widget);
    painter->drawPixmap(0, 0, d->pixmap);

    if (option->state & QStyle::State_Selected) {
        painter->setPen(QPen(Qt::black, 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(0, 0, d->pixmap.size().width(), d->pixmap.size().height());
    }
}

int QGraphicsPixmapItem::type() const
{
    return Type;
}

/*!
    \internal
*/
bool QGraphicsPixmapItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsPixmapItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsPixmapItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

class QGraphicsTextItemPrivate
{
public:
    QTextControl textControl;
    
    QFont font;
    QPen pen;
    QRectF boundingRect;

    QGraphicsTextItem *qq;
};

QGraphicsTextItem::QGraphicsTextItem(const QString &text, QGraphicsItem *parent)
    : QGraphicsItem(parent), dd(new QGraphicsTextItemPrivate)
{
    if (!text.isEmpty())
        setText(text);
    connect(&dd->textControl, SIGNAL(viewportUpdateRequest(const QRectF &)),
            this, SLOT(viewportUpdate(const QRectF &)));
    connect(&dd->textControl, SIGNAL(textChanged()),
            this, SLOT(textChanged()));
}

QGraphicsTextItem::~QGraphicsTextItem()
{
    delete dd;
}

QString QGraphicsTextItem::text() const
{
    return dd->textControl.toPlainText();
}

void QGraphicsTextItem::setText(const QString &text)
{
    update();
    QSizeF size = QFontMetricsF(dd->font).size(Qt::TextShowMnemonic, text);
    size.rheight() += 20;
    size.rwidth() += 15;
    dd->boundingRect = QRectF(QPointF(), size);    
    dd->textControl.setPlainText(text);
    dd->textControl.document()->setPageSize(size);
    dd->textControl.setViewport(dd->boundingRect);
    update();
}

QFont QGraphicsTextItem::font() const
{
    return dd->font;
}

void QGraphicsTextItem::setFont(const QFont &font)
{
    dd->font = font;
}

QPen QGraphicsTextItem::pen() const
{
    return dd->pen;
}

void QGraphicsTextItem::setPen(const QPen &pen)
{
    dd->pen = pen;
    update();
}

QRectF QGraphicsTextItem::boundingRect() const
{
    return dd->boundingRect;
}

QPainterPath QGraphicsTextItem::shape() const
{
    QPainterPath path;
    path.addRect(dd->boundingRect);
    return path;
}

bool QGraphicsTextItem::contains(const QPointF &point) const
{
    return dd->boundingRect.contains(point);
}

void QGraphicsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QGraphicsItemPrivate *d = QGraphicsItem::d_ptr;
    Q_UNUSED(widget);
    painter->save();
    dd->textControl.drawContents(painter);
    painter->restore();

    if (option->state & (QStyle::State_Selected | QStyle::State_HasFocus)) {
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(dd->boundingRect);
    }
}

int QGraphicsTextItem::type() const
{
    return Type;
}

void QGraphicsTextItem::mouseEvent(QGraphicsSceneMouseEvent *event)
{
    if (hasFocus()) {
        QEvent::Type type = QEvent::None;
        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
            type = QEvent::MouseButtonPress;
            break;
        case QEvent::GraphicsSceneMouseRelease:
            type = QEvent::MouseButtonRelease;
            break;
        case QEvent::GraphicsSceneMouseMove:
            type = QEvent::MouseMove;
            break;
        case QEvent::GraphicsSceneMouseDoubleClick:
            type = QEvent::MouseButtonDblClick;
            break;
        default:
            break;
        }

        QMouseEvent mouseEvent(type, event->pos().toPoint(), event->button(),
                               event->buttons(), event->modifiers());

        switch (event->type()) {
        case QEvent::GraphicsSceneMousePress:
            dd->textControl.mousePressEvent(&mouseEvent);
            break;
        case QEvent::GraphicsSceneMouseRelease:
            dd->textControl.mouseReleaseEvent(&mouseEvent);
            break;
        case QEvent::GraphicsSceneMouseMove:
            dd->textControl.mouseMoveEvent(&mouseEvent);
            break;
        case QEvent::GraphicsSceneMouseDoubleClick:
            dd->textControl.mouseDoubleClickEvent(&mouseEvent);
            break;
        default:
            break;
        }
    } else {
        QGraphicsItem::mouseEvent(event);
    }
}

void QGraphicsTextItem::keyEvent(QKeyEvent *event)
{
    switch (event->type()) {
    case QEvent::KeyPress:
        dd->textControl.keyPressEvent(event);
        break;
    case QEvent::KeyRelease:
        dd->textControl.keyReleaseEvent(event);
        break;
    default:
        break;
    }
}

void QGraphicsTextItem::focusEvent(QFocusEvent *event)
{
    dd->textControl.setFocus(event->gotFocus());
    update();
}

/*!
    \internal
*/
bool QGraphicsTextItem::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
    \internal
*/
void QGraphicsTextItem::setExtension(Extension extension, const QVariant &variant)
{
    Q_UNUSED(extension);
    Q_UNUSED(variant);
}

/*!
    \internal
*/
QVariant QGraphicsTextItem::extension(const QVariant &variant) const
{
    Q_UNUSED(variant);
    return QVariant();
}

void QGraphicsTextItem::viewportUpdate(const QRectF &rect)
{
    update(rect);
}

void QGraphicsTextItem::textChanged()
{
    QFontMetrics fm(dd->font);
    QSizeF size = fm.size(Qt::TextShowMnemonic, dd->textControl.toPlainText());
    size.rheight() += fm.lineSpacing();

    if (size != dd->boundingRect.size()) {
        // ### This could be made more efficient.
        update();
        dd->boundingRect = QRectF(QPointF(), size);
        dd->textControl.document()->setPageSize(size);
        dd->textControl.setViewport(dd->boundingRect);
        update();
    }
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug debug, QGraphicsItem *item)
{
    if (!item) {
        debug << "QGraphicsItem(0)";
        return debug;
    }

    QStringList flags;
    if (item->isVisible()) flags << "isVisible";
    if (item->isEnabled()) flags << "isEnabled";
    if (item->isSelected()) flags << "isSelected";
    if (item->hasFocus()) flags << "HasFocus";

    debug << "QGraphicsItem(this =" << ((void*)item)
          << ", parent =" << ((void*)item->parentItem())
          << ", pos =" << item->pos()
          << ", z =" << item->zValue() << ", flags = {"
          << flags.join("|") << " })";
    return debug;
}
#endif
