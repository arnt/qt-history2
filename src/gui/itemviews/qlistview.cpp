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

#include "qlistview.h"
#include <qabstractitemdelegate.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qpainter.h>
#include <qvector.h>
#include <qstyle.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qrubberband.h>
#include <private/qlistview_p.h>
#include <qdebug.h>

#define d d_func()
#define q q_func()

template <class T>
void BinTree<T>::climbTree(const QRect &rect, callback *function, void *data)
{
    ++visited;
    climbTree(rect, function, data, 0);
}

template <class T>
void BinTree<T>::init(const QRect &area, typename BinTree::Node::Type type)
{
    init(area, depth_, type, 0);
}

template <class T>
void BinTree<T>::reserve(int size)
{
    itemVector.reserve(size);
}

template <class T>
int BinTree<T>::itemCount() const
{
    return itemVector.count();
}

template <class T>
const T &BinTree<T>::const_item(int idx) const
{
    return itemVector[idx];
}

template <class T>
T &BinTree<T>::item(int idx)
{
    return itemVector[idx];
}

template <class T>
T *BinTree<T>::itemPtr(int idx)
{
    return &itemVector[idx];
}

template <class T>
void BinTree<T>::setItemPosition(int x, int y, int idx)
{
    item(idx).x = x;
    item(idx).y = y;
}

template <class T>
void BinTree<T>::appendItem(T &item)
{
    itemVector.append(item);
}

template <class T>
void BinTree<T>::insertItem(T &item, const QRect &rect, int idx)
{
    itemVector.insert(idx + 1, 1, item); // insert after idx
    climbTree(rect, &insert, reinterpret_cast<void *>(idx), 0);
}

template <class T>
void BinTree<T>::removeItem(const QRect &rect, int idx)
{
    climbTree(rect, &remove, reinterpret_cast<void *>(idx), 0);
    itemVector.remove(idx, 1);
}

template <class T>
void BinTree<T>::moveItem(const QPoint &dest, const QRect &rect, int idx)
{
    climbTree(rect, &remove, reinterpret_cast<void *>(idx), 0);
    item(idx).x = dest.x();
    item(idx).y = dest.y();
    climbTree(QRect(dest, rect.size()), &insert, reinterpret_cast<void *>(idx), 0);
}

template <class T>
int BinTree<T>::leafCount() const
{
    return leafVector.count();
}

template <class T>
const QVector<int> &BinTree<T>::const_leaf(int idx) const
{
    return leafVector.at(idx);
}

template <class T>
QVector<int> &BinTree<T>::leaf(int idx)
{
    return leafVector[idx];
}

template <class T>
void BinTree<T>::clearLeaf(int idx) { leafVector[idx].clear(); }

template <class T>
int BinTree<T>::nodeCount() const
{
    return nodeVector.count();
}

template <class T>
const typename BinTree<T>::Node &BinTree<T>::node(int idx) const
{
    return nodeVector[idx];
}

template <class T>
int BinTree<T>::parentIndex(int idx) const
{
    return (idx & 1) ? ((idx - 1) / 2) : ((idx - 2) / 2);
}

template <class T>
int BinTree<T>::firstChildIndex(int idx) const
{
    return ((idx * 2) + 1);
}

template <class T>
void BinTree<T>::create(int n)
{
    // simple heuristics to find the best tree depth
    int c;
    for (c = 0; n; ++c)
        n = n / 10;
    depth_ = c << 1;
    nodeVector.resize((1 << depth_) - 1); // resize to number of nodes
    leafVector.resize(1 << depth_); // resize to number of leaves
}

template <class T>
void BinTree<T>::destroy()
{
    leafVector.clear();
    nodeVector.resize(0);
    itemVector.resize(0);
}

template <class T>
void BinTree<T>::insert(QVector<int> &leaf, const QRect &, uint, void *data)
{
    leaf.push_back((int)data);
}

template <class T>
void BinTree<T>::remove(QVector<int> &leaf, const QRect &, uint, void *data)
{
    int idx = (int)data;
    for (int i = 0; i < (int)leaf.count(); ++i) {
        if (leaf[i] == idx) {
            for (; i < (int)leaf.count() - 1; ++i)
                leaf[i] = leaf[i + 1];
            leaf.pop_back();
            return;
        }
    }
}

template <class T>
void BinTree<T>::climbTree(const QRect &area, callback *function, void *data, int index)
{
    int tvs = nodeCount();
    if (index >= tvs) { // leaf
        int idx = index - tvs;
        if (tvs) // tvs == 0 => leaf is empty
            function(leaf(idx), area, visited, data);
        return;
    }

    typename Node::Type t = (typename Node::Type) node(index).type;

    int pos = node(index).pos;
    int idx = firstChildIndex(index);
    if (t == Node::VerticalPlane) {
        if (area.left() < pos)
            climbTree(area, function, data, idx); // back
        if (area.right() >= pos)
            climbTree(area, function, data, idx + 1); // front
    } else {
        if (area.top() < pos)
            climbTree(area, function, data, idx); // back
        if (area.bottom() >= pos)
            climbTree(area, function, data, idx + 1); // front
    }
}

template <class T>
void BinTree<T>::init(const QRect &area, int depth, typename BinTree::Node::Type type, int index)
{
    typename Node::Type t = Node::None; // t should never have this value
    if (type == Node::Both) // if both planes are specified, use 2d bsp
        t = (depth & 1) ? Node::HorizontalPlane : Node::VerticalPlane;
    else
        t = type;
    QPoint center = area.center();
    nodeVector[index].pos = (t == Node::VerticalPlane ? center.x() : center.y());
    nodeVector[index].type = t;

    QRect front = area;
    QRect back = area;

    if (t == Node::VerticalPlane) {
        front.setLeft(center.x());
        back.setRight(center.x() - 1); // front includes the center
    } else { // t == Node::HorizontalPlane
        front.setTop(center.y());
        back.setBottom(center.y() - 1);
    }

    int idx = firstChildIndex(index);
    if (--depth) {
        init(back, depth, type, idx);
        init(front, depth, type, idx + 1);
    }
}

/*!
    \class QListView qlistview.h

    \brief The QListView class provides a default model/view
    implementation of a list, and of an icon view.

    \ingroup model-view

    A QListView presents items stored in a model, either as a simple
    non-hierarchical list, or as a collection of icons. This class is used
    to provide lists and icon views that were previously provided by the
    \c QListBox and \c QIconView classes, but using the more flexible
    approach provided by Qt's model/view architecture.

    QListView implements the interfaces defined by the
    QAbstractItemView class to allow it to display data provided by
    models derived from the QAbstractItemModel class.

    \omit
    Describe the listview/iconview concept.
    \endomit

    Items in these views are laid out in the direction specified by the
    flow() of the list view. The items may be fixed in place, or allowed
    to move, depending on the view's movement() state.

    If the items in the model cannot be completely laid out in the
    direction of flow, they can be wrapped at the boundary of the view
    widget; this depends on isWrapping(). This property is useful when the
    items are being represented by an icon view.

    The resizeMode() and layoutMode() govern how and when the items are
    laid out. Items are spaced according to their spacing(), and can exist
    within a notional grid of size specified by gridSize(). The items can
    be rendered as large or small icons depending on their iconSize().

    \sa \link model-view-programming.html Model/View Programming\endlink
*/

/*!
    \enum QListView::ViewMode

    \value ListMode
    \value IconMode
*/

/*!
  \enum QListView::Movement

  \value Static The items cannot be moved by the user.
  \value Free The items can be moved freely by the user.
  \value Snap The items snap to the specified grid when moved; see
  setGridSize().
*/

/*!
  \enum QListView::Flow

  \value LeftToRight The items are laid out in the view from the left
  to the right.
  \value TopToBottom The items are laid out in the view from the top
  to the bottom.
*/

/*!
  \enum QListView::IconSize

  \value Automatic The icon size is Small if \l isWrapping is
  true; otherwise the icon size is Large.
  \value Small The icons in the items are rendered as small icons.
  \value Large The icons in the items are rendered as large icons.
*/

/*!
  \enum QListView::ResizeMode

  \value Fixed The items will only be laid out the first time the view is shown.
  \value Adjust The items will be laid out every time the view is resized.
*/

/*!
  \enum QListView::LayoutMode

  \value SinglePass The items are laid out all at once.
  \value Batched The items are laid out in batches of 100 items.
*/

/*!
    Creates a new QListView with the given \a parent to view a model.
    Use setModel() to set the model.
*/
QListView::QListView(QWidget *parent)
    : QAbstractItemView(*new QListViewPrivate, parent)
{
    d->init();
    setViewMode(ListMode);
}

/*!
  \internal
*/
QListView::QListView(QListViewPrivate &dd, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    d->init();
    setViewMode(ListMode);
}

/*!
  Destroys the view.
*/
QListView::~QListView()
{
}

/*!
    \property QListView::movement
    \brief whether the items can be moved freely, are snapped to a
    grid, or cannot be moved at all.

    This property determines how the user can move the items in the
    view. \c Static means that the items can't be moved the user. \c
    Free means that the user can drag and drop the items to any
    position in the view. \c Snap means that the user can drag and
    drop the items, but only to the positions in a notional grid
    signified by the gridSize property.

    Setting this property when the view is visible will cause the
    items to be laid out again.

    \sa gridSize
*/
void QListView::setMovement(Movement movement)
{
    d->modeProperties |= QListViewPrivate::Movement;
    d->movement = movement;
    if (isVisible())
        doItemsLayout();
}

QListView::Movement QListView::movement() const
{
    return d->movement;
}

/*!
    \property QListView::flow
    \brief which direction the items layout should flow.

    If this property is \c LeftToRight, the items will be laid out left
    to right. If the \l isWrapping property is true, the layout will wrap
    when it reaches the right side of the visible area. If this
    property is \c TopToBottom, the items will be laid out from the top
    of the visible area, wrapping when it reaches the bottom.

    Setting this property when the view is visible will cause the
    items to be laid out again.
*/
void QListView::setFlow(Flow flow)
{
    d->modeProperties |= QListViewPrivate::Flow;
    d->flow = flow;
    if (isVisible())
        doItemsLayout();
}

QListView::Flow QListView::flow() const
{
    return d->flow;
}

/*!
    \property QListView::isWrapping
    \brief whether the items layout should wrap.

    This property holds whether the layout should wrap when there is
    no more space in the visible area. The point at which the layout wraps
    depends on the \l flow property.

    Setting this property when the view is visible will cause the
    items to be laid out again.
*/
void QListView::setWrapping(bool enable)
{
    d->modeProperties |= QListViewPrivate::Movement;
    d->wrap = enable;
    if (isVisible())
        doItemsLayout();
}

bool QListView::isWrapping() const
{
    return d->wrap;
}

/*!
    \property QListView::iconSize
    \brief whether the items should be rendered as large or small items.

    If this property is \c Small (the default), the default delegate
    will render the items as small items with the decoration to the
    left and the text to the right. If this property is \c Large, the
    default delegate will render the items as large items with the
    decoration at the top and the text at the bottom. If set to \c
    Automatic, the view will use the \c Small mode if the \l
    isWrapping property is true.

    Setting this property when the view is visible will cause the
    items to be laid out again.
*/
void QListView::setIconSize(IconSize size)
{
    d->modeProperties |= QListViewPrivate::IconSize;
    d->iconSize = size;
    if (isVisible())
        doItemsLayout();
}

QListView::IconSize QListView::iconSize() const
{
    return d->iconSize;
}

/*!
    \property QListView::resizeMode
    \brief whether the items are laid out again when the view is resized.

    If this property is \c Adjust, the items will be laid out again
    when the view is resized. If the value is \c Fixed, the items will
    not be laid out when the view is resized.
*/
void QListView::setResizeMode(ResizeMode mode)
{
    d->resizeMode = mode;
}

QListView::ResizeMode QListView::resizeMode() const
{
    return d->resizeMode;
}

/*!
    \property QListView::layoutMode
    \brief whether the layout of items should happen immediately or be delayed.

    This property holds the layout mode for the items. When the mode
    is \c Instant (the default), the items are laid out all in one go.
    When the mode is \c Batched, the items are laid out in batches of 100
    items, while processing events. This makes it possible to
    instantly view and interact with the visible items while the rest
    are being laid out.
*/
void QListView::setLayoutMode(LayoutMode mode)
{
    d->layoutMode = mode;
}

QListView::LayoutMode QListView::layoutMode() const
{
    return d->layoutMode;
}

/*!
    \property QListView::spacing
    \brief the space between items in the layout

    This property is the size of the empty space between items in the
    layout. This property is ignored if the items are laid out in a
    grid.

    Setting this property when the view is visible will cause the
    items to be laid out again.
*/
void QListView::setSpacing(int space)
{
    d->modeProperties |= QListViewPrivate::Spacing;
    d->spacing = space;
    if (isVisible())
        doItemsLayout();
}

int QListView::spacing() const
{
    return d->spacing;
}

/*!
    \property QListView::gridSize
    \brief the size of the layout grid

    This property is the size of the grid in which the items are laid
    out. The default is an empty size which means that there is no
    grid and the layout is not done in a grid. Setting this property
    to a non-empty size switches on the grid layout. (When a grid
    layout is in force the \l spacing property is ignored.)

    Setting this property when the view is visible will cause the
    items to be laid out again.
*/
void QListView::setGridSize(const QSize &size)
{
    d->modeProperties |= QListViewPrivate::GridSize;
    d->gridSize = size;
    if (isVisible())
        doItemsLayout();
}

QSize QListView::gridSize() const
{
    return d->gridSize;
}

/*!
    \property QListView::viewMode
    \brief the view mode of the QListView.

    This property will change the other unset properties to conform
    with the set view mode. Properties that have already been set
    will not be changed, unless clearPropertyFlags() has been called.
*/
void QListView::setViewMode(ViewMode mode)
{
    d->viewMode = mode;

    if (mode == ListMode) {
        if (!(d->modeProperties & QListViewPrivate::Wrap))
            d->wrap = false;
        if (!(d->modeProperties & QListViewPrivate::Spacing))
            d->spacing = 0;
        if (!(d->modeProperties & QListViewPrivate::GridSize))
            d->gridSize = QSize();
        if (!(d->modeProperties & QListViewPrivate::Flow))
            d->flow = TopToBottom;
        if (!(d->modeProperties & QListViewPrivate::Movement))
            d->movement = Static;
        if (!(d->modeProperties & QListViewPrivate::IconSize))
            d->iconSize = Small;
        if (!(d->modeProperties & QListViewPrivate::ResizeMode))
            d->resizeMode = Fixed;
    } else {
        if (!(d->modeProperties & QListViewPrivate::Wrap))
            d->wrap = true;
        if (!(d->modeProperties & QListViewPrivate::Spacing))
            d->spacing = 0;
        if (!(d->modeProperties & QListViewPrivate::GridSize))
            d->gridSize = QSize();
        if (!(d->modeProperties & QListViewPrivate::Flow))
            d->flow = LeftToRight;
        if (!(d->modeProperties & QListViewPrivate::Movement))
            d->movement = Free;
        if (!(d->modeProperties & QListViewPrivate::IconSize))
            d->iconSize = Large;
        if (!(d->modeProperties & QListViewPrivate::ResizeMode))
            d->resizeMode = Adjust;
    }

    if (isVisible())
        doItemsLayout();
}

QListView::ViewMode QListView::viewMode() const
{
    return d->viewMode;
}

/*!
    Clears the property flags. See \l{viewMode}.
*/
void QListView::clearPropertyFlags()
{
    d->modeProperties = 0;
}

/*!
  \reimp
*/
QRect QListView::itemViewportRect(const QModelIndex &index) const
{
    return d->mapToViewport(itemRect(index));
}

/*!
  \reimp
*/
void QListView::ensureItemVisible(const QModelIndex &item)
{
    QRect area = d->viewport->rect();
    QRect rect = itemViewportRect(item);

    if (model()->parent(item) != root())
        return;

    if (area.contains(rect)) {
        d->viewport->repaint(rect);
        return;
    }

    // vertical
    int vy = verticalScrollBar()->value();
    if (rect.top() < area.top()) // above
        verticalScrollBar()->setValue(vy + rect.top());
    else if (rect.bottom() > area.bottom()) // below
        verticalScrollBar()->setValue(vy + rect.bottom() - viewport()->height());

    // horizontal
    int vx = horizontalScrollBar()->value();
    if (rect.left() < area.left()) // left of
        horizontalScrollBar()->setValue(vx + rect.left());
    else if (rect.right() > area.right()) // right of
        horizontalScrollBar()->setValue(vx + rect.right() - viewport()->width());
}

/*!
  \internal
*/
void QListView::reset()
{
    d->prepareItemsLayout();
    QAbstractItemView::reset();
}

/*!
    \internal

    Scroll the view contents by \a dx and \a dy.
*/
void QListView::scrollContentsBy(int dx, int dy)
{
    verticalScrollBar()->repaint();
    horizontalScrollBar()->repaint();
    d->viewport->scroll(dx, dy);

    if (d->draggedItems.isEmpty())
        return;
    QRect rect = d->draggedItemsRect();
    rect.moveBy(dx, dy);
    d->viewport->repaint(rect);
}

/*!
    \internal

    Resize the internal contents to \a width and \a height and set the
    scrollbar ranges accordingly.
*/
void QListView::resizeContents(int width, int height)
{
    d->contentsSize = QSize(width, height);
    horizontalScrollBar()->setRange(0, width - viewport()->width() - 1);
    verticalScrollBar()->setRange(0, height - viewport()->height() - 1);
}

/*!
  \reimp
*/
void QListView::rowsInserted(const QModelIndex &parent, int, int)
{
    // FIXME: if the parent is above root() in the tree, nothing will happen
    if (parent == root() && isVisible())
        doItemsLayout();
    else
        d->doDelayedItemsLayout();
}

/*!
  \reimp
*/
void QListView::rowsRemoved(const QModelIndex &parent, int, int)
{
    // FIXME: if the parent is above root() in the tree, nothing will happen
    if (parent == root() && isVisible())
        doItemsLayout();
}

/*!
  \reimp
*/
void QListView::mouseMoveEvent(QMouseEvent *e)
{
    QAbstractItemView::mouseMoveEvent(e);
    if (state() == QAbstractItemView::Selecting && d->selectionMode != SingleSelection) {
        QPoint topLeft(d->pressedPosition.x() - horizontalOffset(),
                       d->pressedPosition.y() - verticalOffset());
        QRect rect(mapToGlobal(topLeft), mapToGlobal(e->pos()));
        d->rubberBand->setGeometry(rect.normalize());
        if (!d->rubberBand->isVisible() && d->iconSize == Large) {
            d->rubberBand->show();
            d->rubberBand->raise();
        }
    }
}

/*!
  \reimp
*/
void QListView::mouseReleaseEvent(QMouseEvent *e)
{
    QAbstractItemView::mouseReleaseEvent(e);
    d->rubberBand->hide();
}

/*!
  \reimp
*/
void QListView::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->layoutTimer) {
        killTimer(d->layoutTimer);
        d->layoutTimer = 0;
        doItemsLayout();
    }
    QAbstractItemView::timerEvent(e);
}

/*!
  \reimp
*/
void QListView::resizeEvent(QResizeEvent *e)
{
    QAbstractItemView::resizeEvent(e);
    if (d->resizeMode == Adjust && d->layoutTimer == 0) {
        QSize delta = e->size() - e->oldSize();
        if ((d->flow == LeftToRight && delta.width() != 0) ||
            (d->flow == TopToBottom && delta.height() != 0))
            d->layoutTimer = startTimer(100); // wait 1/10 sec before starting the layout
    }
}

/*!
  \reimp
*/
void QListView::dragMoveEvent(QDragMoveEvent *e)
{
    if (!model()->canDecode(e)) {
        e->ignore();
        return;
    }

    QPoint pos = e->pos();
    if (d->shouldAutoScroll(pos))
        startAutoScroll();

    // get old dragged items rect
    QRect itemsRect = d->itemsRect(d->draggedItems);
    QRect oldRect = itemsRect;
    oldRect.moveBy(d->draggedItemsDelta());

    // update position
    d->draggedItemsPos = pos;

    // get new items rect
    QRect newRect = itemsRect;
    newRect.moveBy(d->draggedItemsDelta());

    d->viewport->repaint(oldRect|newRect);

    // check if we allow drops here
    QModelIndex index = itemAt(pos.x(), pos.y());
    if (!index.isValid()) {
        e->accept();
    } else if (e->source() == this && d->draggedItems.contains(index)) {
        e->accept();
    } else if (model()->isDropEnabled(index)) {
        setCurrentItem(index);
        e->accept();
    } else {
        e->ignore();
    }
}

/*!
  \reimp
*/
void QListView::dragLeaveEvent(QDragLeaveEvent *)
{
    d->viewport->update(d->draggedItemsRect()); // erase the area
    d->draggedItemsPos = QPoint(-1, -1); // don't draw the dragged items
}

/*!
  \reimp
*/
void QListView::dropEvent(QDropEvent *e)
{
    if (e->source() == this && d->movement != Static) {
        QPoint offset = QPoint(horizontalScrollBar()->value(),
                               verticalScrollBar()->value());
        QPoint end = e->pos() + offset;
        QPoint start = d->pressedPosition;
        QPoint delta = (d->movement == Snap ?
                        d->snapToGrid(end) - d->snapToGrid(start) : end - start);
        QList<QModelIndex> indices = selectionModel()->selectedItems();
        for (int i = 0; i < indices.count(); ++i) {
            QModelIndex index = indices.at(i);
            QRect rect = itemRect(index);
            d->viewport->update(d->mapToViewport(rect));
            d->moveItem(index.row(), rect.topLeft() + delta);
            d->viewport->update(itemViewportRect(index));
        }
        stopAutoScroll();
    } else {
        QAbstractItemView::dropEvent(e);
    }
}

/*!
  \reimp
*/
QDragObject *QListView::dragObject()
{
    // This function does the same thing as in QAbstractItemView,
    //  plus adding viewitems to the draggedItems list. We need these items to draw the drag items
    QModelIndexList items = selectionModel()->selectedItems();
    QModelIndexList::ConstIterator it = items.begin();
    for (; it != items.end(); ++it)
        if (model()->isDragEnabled(*it))
            d->draggedItems.push_back(*it);
    return model()->dragObject(items, this);
}

/*!
  \reimp
*/
void QListView::startDrag()
{
    QAbstractItemView::startDrag();
    // clear dragged items
    d->draggedItems.clear();
}

/*!
  \reimp
*/
bool QListView::isDragEnabled(const QModelIndex &) const
{
    return d->movement == Free;
}

/*!
  \reimp
*/
QStyleOptionViewItem QListView::viewOptions() const
{
    QStyleOptionViewItem option = QAbstractItemView::viewOptions();
    if (d->iconSize == Automatic ? !d->wrap : d->iconSize == Small) {
        option.decorationSize = QStyleOptionViewItem::Small;
        option.decorationPosition = (QApplication::reverseLayout()
                                      ? QStyleOptionViewItem::Right
                                      : QStyleOptionViewItem::Left);
    } else {
        option.decorationSize = QStyleOptionViewItem::Large;
        option.decorationPosition = QStyleOptionViewItem::Top;
    }
    return option;
}

/*!
  \reimp
*/
void QListView::paintEvent(QPaintEvent *e)
{
    QStyleOptionViewItem option = viewOptions();

    QPainter painter(d->viewport);
    QRect area = e->rect();
    painter.fillRect(area, option.palette.base());
    area.moveBy(horizontalScrollBar()->value(), verticalScrollBar()->value());

    // fill the intersectVector
    if (d->movement == Static)
        d->intersectingStaticSet(area);
    else
        d->intersectingDynamicSet(area);

    QModelIndex current = currentItem();
    QAbstractItemDelegate *delegate = itemDelegate();
    QItemSelectionModel *selections = selectionModel();
    bool focus = q->hasFocus() && current.isValid();
    QStyle::SFlags state = option.state;
    QVector<QModelIndex>::iterator it = d->intersectVector.begin();
    for (; it != d->intersectVector.end(); ++it) {
        option.rect = itemViewportRect(*it);
        option.state = state;
        option.state |= (selections && selections->isSelected(*it)
                         ? QStyle::Style_Selected : QStyle::Style_Default);
        option.state |= (focus && current == *it ? QStyle::Style_HasFocus : QStyle::Style_Default);
        delegate->paint(&painter, option, d->model, *it);
    }

    area = e->rect();

    if (!d->draggedItems.isEmpty() && d->viewport->rect().contains(d->draggedItemsPos)) {
        QPoint delta = d->draggedItemsDelta();
        painter.translate(delta.x(), delta.y());
        d->drawItems(&painter, d->draggedItems);
    }
}

/*!
  \reimp
*/
QModelIndex QListView::itemAt(int x, int y) const
{
    QRect rect(x + horizontalScrollBar()->value(), y + verticalScrollBar()->value(), 1, 1);
    if (d->movement == Static)
        d->intersectingStaticSet(rect);
    else
        d->intersectingDynamicSet(rect);
    QModelIndex index = d->intersectVector.count() > 0
                        ? d->intersectVector.first() : QModelIndex::Null;
    if (index.isValid() && itemViewportRect(index).contains(QPoint(x, y)))
        return index;
    return QModelIndex::Null;
}

/*!
  \reimp
*/
int QListView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

/*!
  \reimp
*/
int QListView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

/*!
  \reimp
*/
QModelIndex QListView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState)
{
    QModelIndex current = currentItem();
    QRect rect = itemRect(current);
    QSize contents = d->contentsSize;
    int spacing = d->spacing;
    QPoint pos = rect.topLeft();
    d->intersectVector.clear();

    switch (cursorAction) {
    case MoveLeft:{
        while (d->intersectVector.count() == 0) {
            if (rect.left() > spacing)
                rect.moveLeft(qMax(rect.left() - rect.width() - spacing, 0));
            else // move down
                rect.moveTopLeft(QPoint(contents.width() - 1, rect.top()/* - rect.height()*/));
            //qDebug() << rect;
            if (rect.top() > contents.height() || rect.bottom() < 0) {
                //qDebug() << "breaking";
                break;
            }
            if (d->movement == Static)
                d->intersectingStaticSet(rect);
            else
                d->intersectingDynamicSet(rect);
            // FIXME: we should fix the intersection function
            if (!d->intersectVector.isEmpty() && d->intersectVector.first() == current)
                d->intersectVector.erase(d->intersectVector.begin());
        }
        break;}
    case MoveRight:
        while (d->intersectVector.count() == 0) {
            if (rect.right() < contents.width())
                rect.moveLeft(rect.left() + rect.width() + spacing);
            else // move down
                rect.moveTopLeft(QPoint(0, rect.top() + rect.height() + spacing));
            //qDebug() << rect;
            if (rect.top() > contents.height() || rect.bottom() < spacing) {
                //qDebug() << "breaking";
                break;
            }
            if (d->movement == Static)
                d->intersectingStaticSet(rect);
            else
                d->intersectingDynamicSet(rect);
            // FIXME: we should fix the intersection function
            if (!d->intersectVector.isEmpty() && d->intersectVector.first() == current)
                d->intersectVector.erase(d->intersectVector.begin());
        }
        break;
    case MovePageUp:
        rect.moveTop(rect.top() - d->viewport->height());
        if (rect.top() < spacing)
            rect.moveTop(contents.height() - rect.height());
    case MoveUp:
        if (d->movement == Static && cursorAction != MovePageUp && current.row() > 0)
            return model()->index(current.row() - 1, 0, root());
        while (d->intersectVector.count() == 0) {
            if (rect.top() > spacing)
                rect.moveTop(rect.top() - rect.height() - spacing);
            else // move right
                rect.moveTopLeft(QPoint(rect.left() - rect.width(),
                                        contents.height() - rect.height()));
            if (rect.left() > contents.width() || rect.right() < 0)
                break;
            if (d->movement == Static)
                d->intersectingStaticSet(rect);
            else
                d->intersectingDynamicSet(rect);
        }
        break;
    case MovePageDown:
        rect.moveTop(rect.top() + d->viewport->height());
        if (rect.top() > contents.height())
            rect.moveTop(0);
    case MoveDown:
        if (d->movement == Static && cursorAction != MovePageDown
            && current.row() < model()->rowCount(root()) - 1)
            return model()->index(current.row() + 1, 0, root());
        while (d->intersectVector.count() == 0) {
            if (rect.bottom() < contents.height() - spacing)
                rect.moveTop(rect.top() + rect.height() + spacing);
            else // move right
                rect.moveTopLeft(QPoint(rect.left() + rect.width(), 0));
            if (rect.left() > contents.width() || rect.right() < 0)
                break;
            if (d->movement == Static)
                d->intersectingStaticSet(rect);
            else
                d->intersectingDynamicSet(rect);
            // FIXME: we should fix the intersection function
            if (d->intersectVector.count() && d->intersectVector.first() == current)
                d->intersectVector.clear();
        }
        break;
    case MoveHome:
        return model()->index(0, 0, root());
    case MoveEnd:
        return model()->index(d->layoutStart - 1, 0, root());
    }

    int dist = 0;
    int minDist = 0;
    QModelIndex closest;
    QVector<QModelIndex>::iterator it = d->intersectVector.begin();
    for (; it != d->intersectVector.end(); ++it) {
        if (!(*it).isValid())
            continue;
        dist = (d->indexToListViewItem(*it).rect().topLeft() - pos).manhattanLength();
        if (dist < minDist || minDist == 0) {
            minDist = dist;
            closest = *it;
        }
    }

    return closest;
}

/*!
    Returns the rectangle of the item at position \a index in the
    model. The rectangle is in contents coordinates.
*/
QRect QListView::itemRect(const QModelIndex &index) const
{
    if (!index.isValid() || model()->parent(index) != root())
        return QRect();
    QListViewItem item = d->indexToListViewItem(index);
    return item.rect();
}

/*!
  \reimp
*/
void QListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    QRect crect(rect.left() + horizontalScrollBar()->value(),
                rect.top() + verticalScrollBar()->value(),
                rect.width(), rect.height());
    if (d->movement == Static)
        d->intersectingStaticSet(crect);
    else
        d->intersectingDynamicSet(crect);

    QItemSelection selection;
    QModelIndex tl;
    QModelIndex br;
    QVector<QModelIndex>::iterator it = d->intersectVector.begin();
    for (; it != d->intersectVector.end(); ++it) {
        if (!tl.isValid() && !br.isValid()) {
            tl = br = *it;
        } else if ((*it).row() == (tl.row() - 1)) {
            tl = *it; // expand current range
        } else if ((*it).row() == (br.row() + 1)) {
            br = (*it); // expand current range
        } else {
            selection.select(tl, br, model()); // select current range
            tl = br = *it; // start new range
        }
    }
    if (tl.isValid() && br.isValid())
        selection.select(tl, br, model());

    selectionModel()->select(selection, command);
}

/*!
  \reimp
*/
QRect QListView::selectionViewportRect(const QItemSelection &selection) const
{
    if (selection.count() == 1 && selection.at(0).top() == selection.at(0).bottom()) {
        QModelIndex singleItem = model()->index(selection.at(0).top(),
                                                selection.at(0).left(),
                                                selection.at(0).parent());
        return itemViewportRect(singleItem);
    }
    return d->viewport->rect();
}

/*!
    \internal

    Layout the items according to the flow and wrapping properties.
*/
void QListView::doItemsLayout()
{
    d->layoutStart = 0;
    d->layoutWraps = 0;
    d->translate = 0;

    d->layoutBounds = viewport()->rect();
    int sbx = style().pixelMetric(QStyle::PM_ScrollBarExtent);
    d->layoutBounds.setWidth(d->layoutBounds.width() - sbx);
    d->layoutBounds.setHeight(d->layoutBounds.height() - sbx);
    d->contentsSize = QSize(0, 0);

    d->prepareItemsLayout();

    if (d->layoutMode == SinglePass)
        doItemsLayout(d->model->rowCount(root())); // layout everything
    else
        while (!doItemsLayout(100)) // do layout in batches
            qApp->processEvents();

    QAbstractItemView::doItemsLayout();
}

/*!
  \internal
*/
bool QListView::doItemsLayout(int delta)
{
    int max = d->model->rowCount(root()) - 1;
    int first = d->layoutStart;
    int last = qMin(first + delta - 1, max);

    if (max < 0)
        return true; // nothing to do

    if (d->movement == Static) {
        doStaticLayout(d->layoutBounds, first, last);
    } else {
        if (last >= d->tree.itemCount())
            d->createItems(last + 1);
        doDynamicLayout(d->layoutBounds, first, last);
    }

    d->layoutStart = last + 1;

    if (d->layoutStart >= max) {
        // stop items layout
        if (d->movement == Static) {
            d->wrapVector.resize(d->wrapVector.count());
            if (d->flow == LeftToRight)
                d->yposVector.resize(d->yposVector.count());
            else // TopToBottom
                d->xposVector.resize(d->xposVector.count());
        }
        return true; // done
    }
    return false; // not done
}

/*!
  \internal
*/
void QListView::doItemsLayout(const QRect &bounds,
                                     const QModelIndex &first,
                                     const QModelIndex &last)
{
    if (first.row() >= last.row() || !first.isValid() || !last.isValid())
        return;
    if (d->movement == Static)
        doStaticLayout(bounds, first.row(), last.row());
    else
        doDynamicLayout(bounds, first.row(), last.row());
}

/*!
  \internal
*/
void QListView::doStaticLayout(const QRect &bounds, int first, int last)
{
    int x = 0;
    int y = 0;
    int spacing = d->gridSize.isEmpty() ? d->spacing : 0;
    d->initStaticLayout(x, y, first, bounds, spacing);
    QPoint topLeft(x, y);

    int gw = d->gridSize.width() > 0 ? d->gridSize.width() : 0;
    int gh = d->gridSize.height() > 0 ? d->gridSize.height() : 0;
    int delta = last - first + 1;
    int layoutWraps = d->layoutWraps;
    bool wrap = d->wrap;
    QModelIndex index;
    QSize hint;
    QRect rect = bounds;
    QFontMetrics fontMetrics(font());
    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    QAbstractItemModel *model = this->model();

    if (d->movement == QListView::Static && !wrap)
        if (d->flow == QListView::LeftToRight)
             rect.setHeight(qMin(d->contentsSize.height(), rect.height()));
        else
            rect.setWidth(qMin(d->contentsSize.width(), rect.width()));

    if (d->flow == LeftToRight) {
        int w = bounds.width();
        int dx, dy = (gh ? gh : d->translate);
        for (int i = first; i <= last ; ++i) {
            index = model->index(i, 0, root());
            hint = delegate->sizeHint(fontMetrics, option, model, index);
            dx = (gw ? gw : hint.width());
            if (wrap && (x + spacing >= w))
                d->createStaticRow(x, y, dy, layoutWraps, i, bounds, spacing, delta);
            d->xposVector.push_back(x);
            dy = (hint.height() > dy ? hint.height() : dy);
            x += spacing + dx;
        }
        // used when laying out next batch
        d->xposVector.push_back(x);
        d->translate = dy;
        rect.setRight(x);
        rect.setBottom(y + dy);
    } else { // d->flow == TopToBottom
        int h = bounds.height();
        int dy, dx = (gw ? gw : d->translate);
        for (int i = first; i <= last ; ++i) {
            index = model->index(i, 0, root());
            hint = delegate->sizeHint(fontMetrics, option, model, index);
            dy = (gh ? gh : hint.height());
            if (wrap && (y + spacing >= h))
                d->createStaticColumn(x, y, dx, layoutWraps, i, bounds, spacing, delta);
            d->yposVector.push_back(y);
            dx = (hint.width() > dx ? hint.width() : dx);
            y += spacing + dy;
        }
        // used when laying out next batch
        d->yposVector.push_back(y);
        d->translate = dx;
        rect.setBottom(y);
        rect.setRight(x + dx);
    }

    if (d->layoutWraps < layoutWraps) {
        d->layoutWraps = layoutWraps;
        d->wrapVector.push_back(last + 1);
    }

    resizeContents(rect.width(), rect.height());

    QRect changedRect(topLeft, rect.bottomRight());
    if (clipRegion().boundingRect().intersects(changedRect))
        d->viewport->update();
}

/*!
  \internal
*/
void QListView::doDynamicLayout(const QRect &bounds, int first, int last)
{
    int gw = d->gridSize.width() > 0 ? d->gridSize.width() : 0;
    int gh = d->gridSize.height() > 0 ? d->gridSize.height() : 0;
    int spacing = gw && gh ? 0 : d->spacing;
    int x, y;

    if (first == 0) {
        x = bounds.x() + spacing;
        y = bounds.y() + spacing;
        d->tree.reserve(d->model->rowCount(root()));
    } else {
        int p = first - 1;
        const QListViewItem item = d->tree.item(p);
        x = item.x;
        y = item.y;
        if (d->flow == LeftToRight)
            x += (gw ? gw : item.w) + spacing;
        else
            y += (gh ? gh : item.h) + spacing;
    }

    QPoint topLeft(x, y);
    bool wrap = d->wrap;
    QRect rect(QPoint(0, 0), d->contentsSize);
    QModelIndex bottomRight = model()->index(0, 0, root());
    QListViewItem *item = 0;

    if (d->flow == LeftToRight) {
        int w = bounds.width();
        int dy = (gh ? gh : d->translate);
        for (int i = first; i <= last; ++i) {
            item = d->tree.itemPtr(i);
            int dx = (gw ? gw : item->w);
            // create new layout row
            if (wrap && (x + spacing + dx >= w)) {
                x = bounds.x() + spacing;
                y += spacing + dy;
            }
            item->x = x;
            item->y = y;
            x += spacing + dx;
            dy = (item->h > dy ? item->h : dy);
            rect |= item->rect();
        }
        d->translate = dy;
    } else { // TopToBottom
        int h = bounds.height();
        int dx = (gw ? gw : d->translate);
        for (int i = first; i <= last; ++i) {
            item = d->tree.itemPtr(i);
            int dy = (gh ? gh : item->h);
            if (wrap && (y + spacing + dy >= h)) {
                y = bounds.y() + spacing;
                x += spacing + dx;
            }
            item->x = x;
            item->y = y;
            y += spacing + dy;
            dx = (item->w > dx ? item->w : dx);
            rect |= item->rect();
        }
        d->translate = dx;
    }

    int insertFrom = first;
    resizeContents(rect.width(), rect.height());

    if (first == 0 || last >= bottomRight.row()) { // resize tree

        // remove all items from the tree
        int leafCount = d->tree.leafCount();
        for (int i = 0; i < leafCount; ++i)
            d->tree.clearLeaf(i);
        insertFrom = 0;

        int h = d->contentsSize.height();
        int w = d->contentsSize.width();

        // initialize tree
        // we have to get the bounding rect of the items before we can initialize the tree
        BinTree<QListViewItem>::Node::Type type =
            BinTree<QListViewItem>::Node::Both; // use 2D bsp by default
        if (h / w >= 3)    // simple heuristics to get better bsp
            type = BinTree<QListViewItem>::Node::HorizontalPlane;
        else if (w / h >= 3)
            type = BinTree<QListViewItem>::Node::VerticalPlane;
        d->tree.init(QRect(0, 0, w, h), type); // build tree for bounding rect (not contents rect)
    }

    // insert items in tree
    for (int i = insertFrom; i <= last; i++)
        d->tree.climbTree(d->tree.item(i).rect(), &BinTree<QListViewItem>::insert,
                          reinterpret_cast<void *>(i));

    QRect changedRect(topLeft, rect.bottomRight());
    if (clipRegion().boundingRect().intersects(changedRect))
        d->viewport->update();
}

/*!
  \internal
*/
void QListView::updateGeometries()
{
    QModelIndex index = model()->index(0, 0, root());
    QStyleOptionViewItem option = viewOptions();
    QSize size = itemDelegate()->sizeHint(fontMetrics(), option, model(), index);

    horizontalScrollBar()->setSingleStep(size.width() + d->spacing);
    horizontalScrollBar()->setPageStep(d->viewport->width());
    horizontalScrollBar()->setRange(0, d->contentsSize.width() - d->viewport->width() - 1);

    verticalScrollBar()->setSingleStep(size.height() + d->spacing);
    verticalScrollBar()->setPageStep(d->viewport->height());
    verticalScrollBar()->setRange(0, d->contentsSize.height() - d->viewport->height() - 1);

    QAbstractItemView::updateGeometries();
}

/*
 * private object implementation
 */

QListViewPrivate::QListViewPrivate()
    : QAbstractItemViewPrivate(),
      layoutMode(QListView::SinglePass),
      modeProperties(0),
      layoutStart(0),
      translate(0),
      layoutWraps(0),
      layoutTimer(0)
{}

void QListViewPrivate::init()
{
    rubberBand = new QRubberBand(QRubberBand::Rectangle, viewport);
    rubberBand->hide();
    viewport->setAcceptDrops(true);
}

void QListViewPrivate::prepareItemsLayout()
{
    // initailization of data structs
    int rowCount = model->rowCount(q->root());
    if (movement == QListView::Static) {
        tree.destroy();
        if (flow == QListView::LeftToRight) {
            xposVector.resize(qMax(rowCount, 0));
            yposVector.clear();
        } else { // TopToBottom
            yposVector.resize(qMax(rowCount, 0));
            xposVector.clear();
        }
        wrapVector.clear();
    } else {
        wrapVector.clear();
        xposVector.clear();
        yposVector.clear();
        tree.create(rowCount);
    }
}

/*!
  \internal
  Finds the set of items intersecting with \a area.
  In this function, itemsize is counted from topleft to the start of the next item.
*/

void QListViewPrivate::intersectingStaticSet(const QRect &area) const
{
    intersectVector.clear();
    QAbstractItemModel *model = q->model();

    QModelIndex index;
    QModelIndex root = q->root();
    int first, last, count, i, j;
    bool wraps = wrapVector.count() > 1;
    if (flow == QListView::LeftToRight) {
        if (yposVector.count() == 0)
            return;
        j = qBinarySearch<int>(yposVector, area.top(), 0, layoutWraps); // index to the first ypos
        for (; j <= layoutWraps && yposVector.at(j) < area.bottom(); ++j) {
            first = wrapVector.at(j);
            count = (wraps && j < layoutWraps ? wrapVector.at(j + 1) : layoutStart) - first - 1;
            last = first + count;
            i = qBinarySearch<int>(xposVector, area.left(), first, last);
            for (; i <= last && xposVector.at(i) < area.right(); ++i) {
                index = model->index(i, 0, root);
                if (index.isValid())
                    intersectVector.push_back(index);
                else
                    qWarning("intersectingStaticSet: index was invalid");
            }
        }
    } else { // flow == TopToBottom
        if (xposVector.count() == 0)
            return;
        j = qBinarySearch<int>(xposVector, area.left(), 0, layoutWraps); // index to the first xpos
        for (; j <= layoutWraps && xposVector.at(j) < area.right(); ++j) {
            first = wrapVector.at(j);
            count = (wraps && j < layoutWraps ? wrapVector.at(j + 1) : layoutStart) - first - 1;
            last = first + count;
            i = qBinarySearch<int>(yposVector, area.top(), first, last);
            for (; i <= last && yposVector.at(i) < area.bottom(); ++i) {
                index = model->index(i, 0, root);
                if (index.isValid())
                    intersectVector.push_back(index);
                else
                    qWarning("intersectingStaticSet: index was invalid");
            }
        }
    }
}

void QListViewPrivate::intersectingDynamicSet(const QRect &area) const
{
    intersectVector.clear();
    QListViewPrivate *that = const_cast<QListViewPrivate*>(this);
    that->tree.climbTree(area, &QListViewPrivate::addLeaf, static_cast<void*>(that));
}

void QListViewPrivate::createItems(int to)
{
    int count = tree.itemCount();
    QSize size;
    QStyleOptionViewItem option = q->viewOptions();
    QFontMetrics fontMetrics(q->font());
    QModelIndex root = q->root();
    for (int i = count; i < to; ++i) {
        size = delegate->sizeHint(fontMetrics, option, model, model->index(i, 0, root));
        QListViewItem item(QRect(0, 0, size.width(), size.height()), i); // default pos
        tree.appendItem(item);
    }
}

void QListViewPrivate::drawItems(QPainter *painter, const QVector<QModelIndex> &indexes) const
{
    QStyleOptionViewItem option = q->viewOptions();
    QVector<QModelIndex>::const_iterator it = indexes.begin();
    QListViewItem item = indexToListViewItem(*it);
    for (; it != indexes.end(); ++it) {
        item = indexToListViewItem(*it);
        option.rect.setRect(item.x, item.y, item.w, item.h);
        delegate->paint(painter, option, model, *it);
    }
}

QRect QListViewPrivate::itemsRect(const QVector<QModelIndex> &indexes) const
{
    QVector<QModelIndex>::const_iterator it = indexes.begin();
    QListViewItem item = indexToListViewItem(*it);
    QRect rect(item.x, item.y, item.w, item.h);
    for (; it != indexes.end(); ++it) {
        item = indexToListViewItem(*it);
        rect |= QRect(item.x, item.y, item.w, item.h);
    }
    return rect;
}

QListViewItem QListViewPrivate::indexToListViewItem(const QModelIndex &index) const
{
    if (!index.isValid())
        return QListViewItem();

    if (movement != QListView::Static)
        if (index.row() < tree.itemCount())
            return tree.const_item(index.row());
        else
            return QListViewItem();

    // movement == Static
    if ((flow == QListView::LeftToRight && index.row() >= xposVector.count()) ||
        (flow == QListView::TopToBottom && index.row() >= yposVector.count()))
        return QListViewItem();

    int i = qBinarySearch<int>(wrapVector, index.row(), 0, layoutWraps);
    QPoint pos;
    if (flow == QListView::LeftToRight) {
        pos.setX(xposVector.at(index.row()));
        pos.setY(yposVector.at(i));
    } else { // TopToBottom
        pos.setY(yposVector.at(index.row()));
        pos.setX(xposVector.at(i));
    }

    QStyleOptionViewItem option = q->viewOptions();
    QAbstractItemDelegate *del = q->itemDelegate();
    QSize size = del->sizeHint(q->fontMetrics(), option, model, index);
    return QListViewItem(QRect(pos, size), index.row());
}

int QListViewPrivate::itemIndex(const QListViewItem item) const
{
    int i = item.indexHint;
    if (movement == QListView::Static || i >= tree.itemCount() || tree.const_item(i) == item)
        return i;

    int j = i;
    int c = tree.itemCount();
    bool a = true;
    bool b = true;

    while (a || b) {
        if (a) {
            if (tree.const_item(i) == item) {
                tree.const_item(i).indexHint = i;
                return i;
            }
            a = ++i < c;
        }
        if (b) {
            if (tree.const_item(j) == item) {
                tree.const_item(j).indexHint = j;
                return j;
            }
            b = --j > -1;
        }
    }
    return -1;
}

void QListViewPrivate::addLeaf(QVector<int> &leaf, const QRect &area,
                                      uint visited, void *data)
{
    QListViewItem *vi;
    QListViewPrivate *_this = static_cast<QListViewPrivate *>(data);
    for (int i = 0; i < (int)leaf.count(); ++i) {
        int idx = leaf.at(i);
        if (idx < 0)
            continue;
        vi = _this->tree.itemPtr(idx);
        if (vi->rect().intersects(area) && vi->visited != visited) {
            _this->intersectVector.push_back(_this->listViewItemToIndex(*vi));
            vi->visited = visited;
        }
    }
}

void QListViewPrivate::createStaticRow(int &x, int &y, int &dy, int &wraps, int i,
                                              const QRect &bounds, int spacing, int delta)
{
    x = bounds.left() + spacing;
    y += spacing + dy;
    ++wraps;
    if ((int)yposVector.size() < (wraps + 2)) {
        int s = yposVector.size() + delta;
        yposVector.resize(s);
        wrapVector.resize(s);
    }
    yposVector[wraps] = y;
    wrapVector[wraps] = i;
    dy = 0;
}

void QListViewPrivate::createStaticColumn(int &x, int &y, int &dx, int &wraps, int i,
                                                 const QRect &bounds, int spacing, int delta)
{
    y = bounds.top() + spacing;
    x += spacing + dx;
    ++wraps;
    if ((int)xposVector.size() < (wraps + 2)) {
        int s = xposVector.size() + delta;
        xposVector.resize(s);
        wrapVector.resize(s);
    }
    xposVector[wraps] = x;
    wrapVector[wraps] = i;
    dx = 0;
}

void QListViewPrivate::initStaticLayout(int &x, int &y, int first,
                                               const QRect &bounds, int spacing)
{
    if (first == 0) {
        x = bounds.left() + spacing;
        xposVector.clear();
        if (flow == QListView::TopToBottom)
            xposVector.push_back(x);
        y = bounds.top() + spacing;
        yposVector.clear();
        if (flow == QListView::LeftToRight)
            yposVector.push_back(y);
        layoutWraps = 0;
        wrapVector.clear();
        wrapVector.push_back(0);
    } else if (wrap) {
        if (flow == QListView::LeftToRight) {
            x = xposVector.back();
            xposVector.pop_back();
        } else {
            x = xposVector.at(layoutWraps);
        }
        if (flow == QListView::TopToBottom) {
            y = yposVector.back();
            yposVector.pop_back();
        } else {
            y = yposVector.at(layoutWraps);
        }
    } else {
        if (flow == QListView::LeftToRight) {
            x = xposVector.back();
            xposVector.pop_back();
        } else {
            x = bounds.left() + spacing;
        }
        if (flow == QListView::TopToBottom) {
            y = yposVector.back();
            yposVector.pop_back();
        } else {
            y = bounds.top() + spacing;
        }
    }
}

void QListViewPrivate::insertItem(int index, QListViewItem &item)
{
    tree.insertItem(item, item.rect(), index);
}

void QListViewPrivate::removeItem(int index)
{
    tree.removeItem(tree.item(index).rect(), index);
}

void QListViewPrivate::moveItem(int index, const QPoint &dest)
{
    // does not impact on the bintree itself or the contents rect
    QListViewItem *item = tree.itemPtr(index);
    QRect rect = item->rect();
    d->tree.moveItem(dest, rect, index);

    // resize the contents area
    rect = item->rect();
    int w = item->x + rect.width();
    int h = item->y + rect.height();
    w = w > contentsSize.width() ? w : contentsSize.width();
    h = h > contentsSize.height() ? h : contentsSize.height();
    q->resizeContents(w, h);
}

QPoint QListViewPrivate::snapToGrid(const QPoint &pos) const
{
    int x = pos.x() - (pos.x() % gridSize.width());
    int y = pos.y() - (pos.y() % gridSize.height());
    return QPoint(x, y);
}

QRect QListViewPrivate::mapToViewport(const QRect &rect) const
{
    QRect result(rect.left() - q->horizontalScrollBar()->value(),
                 rect.top() - q->verticalScrollBar()->value(),
                 rect.width(), rect.height());
    // If the listview is in "listbox-mode", the items are as wide as the viewport
    if (!wrap && movement == QListView::Static)
        if (flow == QListView::TopToBottom)
            result.setWidth(qMax(rect.width(), viewport->width()));
        else
            result.setHeight(qMax(rect.height(), viewport->height()));
    return result;
}

QPoint QListViewPrivate::draggedItemsDelta() const
{
    return (movement == QListView::Snap
            ? snapToGrid(draggedItemsPos) - snapToGrid(pressedPosition)
            : draggedItemsPos - pressedPosition);
}

QRect QListViewPrivate::draggedItemsRect() const
{
    QRect rect = itemsRect(draggedItems);
    rect.moveBy(draggedItemsDelta());
    return rect;
}
