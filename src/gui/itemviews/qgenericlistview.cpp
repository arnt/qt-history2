/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qgenericlistview.h"
#include <qabstractitemdelegate.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qpainter.h>
#include <qvector.h>
#include <qstyle.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qrubberband.h>
#include <private/qgenericlistview_p.h>

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
  \class QGenericListView qgenericlistview.h

  \brief The QGenericListView class provides a default model/view implementation of a list and icon view.

  \ingroup model-view

  This class implements a list representation of a QAbstractItemView working on a QAbstractItemModel.

  \omit
  Describe the listview/iconview concept.
  \endomit

  \sa \link model-view-programming.html Model/View Programming\endlink.
*/

/*!
  \enum QGenericListView::Movement

  \value Static The items cannot be moved by the user.
  \value Free The items can be moved freely by the user.
  \value Snap The items snap to the specified grid when moved.
*/

/*!
  \enum QGenericListView::Flow

  \value LeftToRight The items are layed out in the view from the left to the right.
  \value TopToBottom The items are layed out in the view from the top to the bottom.
*/

/*!
  \enum QGenericListView::IconMode

  \value Automatic The icon size is Small if the isWrapping property is true, otherwise Large.
  \value Small The icons in the items are rendered as small icons.
  \value Large The icons in the items are rendered as large icons.
*/

/*!
  \enum QGenericListView::ResizeMode

  \value Fixed The items will only be layed out the first time the view is shown.
  \value Adjust The items will be layed out every time the view is resized.
*/

/*!
  \enum QGenericListView::LayoutMode

  \value Instant The items are layed out all at once.
  \value Batched The items are layed out in batches of 100 items.
*/

/*!
  Creates a new QGenericListView to view the \a model, and with parent \a parent.
*/
QGenericListView::QGenericListView(QWidget *parent)
    : QAbstractItemView(*new QGenericListViewPrivate, parent)
{
    d->init();
}

/*!
  \internal
*/
QGenericListView::QGenericListView(QGenericListViewPrivate &dd, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    d->init();
}

/*!
  Destroys the view.
*/
QGenericListView::~QGenericListView()
{
}

/*!
  \property QGenericListView::movement
  \brief whether the items can be moved freely, snaps to a grid or not at all.

  This property holds how the user can move the items in the view.
  Static means that the items can't be moved at all by the user.
  Free means that the user can drag and drop the items to any position in the view.
  Snap means that the user can drag and drop the items, but only to the positions
  in a grid decided by the property gridSize.

  Setting this property when the view is visible will cause the items to
  be layed out again.

  \sa gridSize
*/
void QGenericListView::setMovement(Movement movement)
{
    d->movement = movement;
    if (isVisible())
        doItemsLayout();
}

QGenericListView::Movement QGenericListView::movement() const
{
    return d->movement;
}

/*!
  \property QGenericListView::flow
  \brief which direction the items layout should flow.

  If this property is LeftToRight, the items will be layed out
  left to right. If the isWrapping property is true, the layout
  will wrap when it reaches the right side of the visible area.
  If this  property is TopToBottom, the items will be layed out
  from the top of the visible area, wrapping when it reaches
  the bottom.

  Setting this property when the view is visible will cause the items to
  be layed out again.

  \sa isWrapping
*/
void QGenericListView::setFlow(Flow flow)
{
    d->flow = flow;
    if (isVisible())
        doItemsLayout();
}

QGenericListView::Flow QGenericListView::flow() const
{
    return d->flow;
}

/*!
  \property QGenericListView::isWrapping
  \brief whether the items layout should wrap.

  This property holds whether the layout should wrap when
  there is no more space in the visible area. When the layout
  wraps depends on the flow property.

  Setting this property when the view is visible will cause the items to
  be layed out again.

  \sa flow doItemLayout()
*/
void QGenericListView::setWrapping(bool enable)
{
    d->wrap = enable;
    if (isVisible())
        doItemsLayout();
}

bool QGenericListView::isWrapping() const
{
    return d->wrap;
}

/*!
  \property QGenericListView::iconMode
  \brief whether the items should be rendered as large or small items.

  If this property is Small (default), the default delegate will render the items
  as small items with the decoration to the left and the text to the right.
  If this property is Large, the default delegate will render the items
  as large items with the decoration on top and the text on the bottom.
  If set to Automatic, the view will use the Small mode if the isWrapping
  property is true.

  Setting this property when the view is visible will cause the items to
  be layed out again.

  \sa isWrapping
*/
void QGenericListView::setIconMode(IconMode mode)
{
    d->iconMode = mode;
    if (isVisible())
        doItemsLayout();
}

QGenericListView::IconMode QGenericListView::iconMode() const
{
    return d->iconMode;
}

/*!
  \property QGenericListView::resizeMode
  \brief whether or not the items are layed out again when the view is resized.

  If this property is Adjust, the items will be layed out again when the view is
  resized. When it is Fixed, the items will not be moved.

  \sa doItemsLayout()
*/
void QGenericListView::setResizeMode(ResizeMode mode)
{
    d->resizeMode = mode;
}

QGenericListView::ResizeMode QGenericListView::resizeMode() const
{
    return d->resizeMode;
}

/*!
  \property QGenericListView::layoutMode
  \brief whether the layout of items should be instant or delayed.

  This property holds the layout mode for the items.
  When the mode is Instant (default), the items are layed out all in one go.
  When the mode is Batched, the items are layed out in batches of 100 items,
  while processing events. This makes it possible to instantly view and interact
  with items while the rest are being layed out.

  \sa doItemsLayout()
*/
void QGenericListView::setLayoutMode(LayoutMode mode)
{
    d->layoutMode = mode;
}

QGenericListView::LayoutMode QGenericListView::layoutMode() const
{
    return d->layoutMode;
}

/*!
  \property QGenericListView::spacing
  \brief the space between items in the layout

  This property is the size of the empty space between items
  in the layout. The spacing will be ignored if the items are layed out in
  a grid.

  Setting this property when the view is visible will cause the items to
  be layed out again.

  \sa doItemsLayout()
*/
void QGenericListView::setSpacing(int space)
{
    d->spacing = space;
    if (isVisible())
        doItemsLayout();
}

int QGenericListView::spacing() const
{
    return d->spacing;
}

/*!
  \property QGenericListView::gridSize
  \brief the size of the layout grid

  This property is the size of the grid in which the items are layed out.
  To turn on grid layout, the grid size must be non-empty.

  Setting this property when the view is visible will cause the items to
  be layed out again.

  \sa doItemsLayout()
*/
void QGenericListView::setGridSize(const QSize &size)
{
    d->gridSize = size;
    if (isVisible())
        doItemsLayout();
}

QSize QGenericListView::gridSize() const
{
    return d->gridSize;
}

/*!
  \reimp
*/
QRect QGenericListView::itemViewportRect(const QModelIndex &index) const
{
    return d->mapToViewport(itemRect(index));
}

/*!
  \reimp
*/
void QGenericListView::ensureItemVisible(const QModelIndex &item)
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
  Scroll the view contents by \a dx and \a dy.
*/
void QGenericListView::scrollContentsBy(int dx, int dy)
{
    QRect rect = d->draggedItemsRect;
    rect.moveBy(dx, dy);
    verticalScrollBar()->repaint();
    horizontalScrollBar()->repaint();
    d->viewport->scroll(dx, dy);
    d->viewport->repaint(rect);
}

/*!
  Resize the internal contents to \a width and \a height and set the scrollbar ranges accordingly.
*/
void QGenericListView::resizeContents(int width, int height)
{
    d->contentsSize = QSize(width, height);
    horizontalScrollBar()->setRange(0, width - viewport()->width());
    verticalScrollBar()->setRange(0, height - viewport()->height());
}

/*!
  \reimp
*/
void QGenericListView::rowsInserted(const QModelIndex &parent, int, int)
{
    if (parent == root() && isVisible())
        doItemsLayout();
}

/*!
  \reimp
*/
void QGenericListView::rowsRemoved(const QModelIndex &parent, int, int)
{
    if (parent == root() && isVisible())
        doItemsLayout();
}

/*!
  \reimp
*/
void QGenericListView::mouseMoveEvent(QMouseEvent *e)
{
    QAbstractItemView::mouseMoveEvent(e);
    if (state() == QAbstractItemView::Selecting && d->selectionMode != SingleSelection) {
        QPoint topLeft(d->pressedPosition.x() - horizontalOffset(),
                       d->pressedPosition.y() - verticalOffset());
        QRect rect(mapToGlobal(topLeft), mapToGlobal(e->pos()));
        d->rubberBand->setGeometry(rect.normalize());
        if (!d->rubberBand->isVisible() && d->iconMode == Large) {
            d->rubberBand->show();
            d->rubberBand->raise();
        }
    }
}

/*!
  \reimp
*/
void QGenericListView::mouseReleaseEvent(QMouseEvent *e)
{
    QAbstractItemView::mouseReleaseEvent(e);
    d->rubberBand->hide();
}

/*!
  \reimp
*/
void QGenericListView::timerEvent(QTimerEvent *e)
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
void QGenericListView::resizeEvent(QResizeEvent *e)
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
void QGenericListView::dragMoveEvent(QDragMoveEvent *e)
{
    if (!model()->canDecode(e)) {
        e->ignore();
        return;
    }

    QPoint pos = e->pos();
    if (d->shouldAutoScroll(pos))
        startAutoScroll();
    d->draggedItemsPos = pos;
    d->viewport->repaint(d->draggedItemsRect);

    QModelIndex item = itemAt(pos.x(), pos.y());
    if (item.isValid())
        if (model()->isDropEnabled(item))
            e->accept();
        else
            e->ignore();
    else
        e->accept();
}

/*!
  \reimp
*/
void QGenericListView::dragLeaveEvent(QDragLeaveEvent *)
{
    d->draggedItemsPos = QPoint(-1, -1); // don't draw the dragged items
    d->viewport->update(d->draggedItemsRect); // erase the area
}

/*!
  \reimp
*/
void QGenericListView::dropEvent(QDropEvent *e)
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
QDragObject *QGenericListView::dragObject()
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
void QGenericListView::startDrag()
{
    QAbstractItemView::startDrag();
    // clear dragged items
    d->draggedItems.clear();
}

/*!
  \reimp
*/
bool QGenericListView::isDragEnabled(const QModelIndex &) const
{
    return d->movement == Free;
}

/*!
  \reimp
*/
QStyleOptionViewItem QGenericListView::viewOptions() const
{
    QStyleOptionViewItem option = QAbstractItemView::viewOptions();
    if (d->iconMode == Automatic ? !d->wrap : d->iconMode == Small) {
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
void QGenericListView::paintEvent(QPaintEvent *e)
{
    QStyleOptionViewItem option = viewOptions();

    QPainter painter(&d->backBuffer);
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
        delegate->paint(&painter, option, *it);
    }

    painter.end();

    area = e->rect();
    painter.begin(d->viewport);
    painter.drawPixmap(area.topLeft(), d->backBuffer, area);

    if (!d->draggedItems.isEmpty() && d->viewport->rect().contains(d->draggedItemsPos)) {
        QPoint delta = (d->movement == Snap
                        ? d->snapToGrid(d->draggedItemsPos) - d->snapToGrid(d->pressedPosition)
                        : d->draggedItemsPos - d->pressedPosition);
        painter.translate(delta.x(), delta.y()); // FIXME: this will make the drawpixmap slower
        d->draggedItemsRect = d->drawItems(&painter, d->draggedItems);
        d->draggedItemsRect.moveBy(delta.x(), delta.y());
    }
}

/*!
  \reimp
*/
QModelIndex QGenericListView::itemAt(int x, int y) const
{
    QRect rect(x + horizontalScrollBar()->value(), y + verticalScrollBar()->value(), 1, 1);
    if (d->movement == Static)
        d->intersectingStaticSet(rect);
    else
        d->intersectingDynamicSet(rect);
    QModelIndex index = d->intersectVector.count() > 0
                        ? d->intersectVector.first() : QModelIndex();
    if (index.isValid() && itemViewportRect(index).contains(QPoint(x, y)))
        return index;
    return QModelIndex();
}

/*!
  \reimp
*/
int QGenericListView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

/*!
  \reimp
*/
int QGenericListView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

/*!
  \reimp
*/
QModelIndex QGenericListView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                         Qt::ButtonState)
{
    QModelIndex current = currentItem();
    QRect rect = itemRect(current);
    QSize contents = d->contentsSize;
    int spacing = d->spacing;
    QPoint pos = rect.topLeft();
    d->intersectVector.clear();
    
    switch (cursorAction) {
    case MoveLeft:
        while (d->intersectVector.count() == 0) {
            if (rect.left() > spacing)
                rect.moveLeft(qMax(rect.left() - rect.width() - spacing, 0));
            else // move down
                rect.moveTopLeft(QPoint(contents.width() - 1, rect.top() - rect.height()));
            if (rect.top() > contents.height() || rect.bottom() < 0)
                break;
            if (d->movement == Static)
                d->intersectingStaticSet(rect);
            else
                d->intersectingDynamicSet(rect);
        }
        break;
    case MoveRight:
        while (d->intersectVector.count() == 0) {
            if (rect.right() < contents.width())
                rect.moveLeft(rect.left() + rect.width() + spacing);
            else // move down
                rect.moveTopLeft(QPoint(0, rect.top() + rect.height() + spacing));
            if (rect.top() > contents.height() || rect.bottom() < spacing)
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
    case MovePageUp:
        rect.moveTop(rect.top() - d->viewport->height());
        if (rect.top() < spacing)
            rect.moveTop(contents.height() - rect.height());
    case MoveUp:
        if (d->movement == Static && cursorAction != MovePageUp)
            return model()->index(current.row() - 1, 0, root());
        while (d->intersectVector.count() == 0) {
            if (rect.top() > spacing)
                rect.moveTop(rect.top() - rect.height() - spacing);
            else // move right
                rect.moveTopLeft(QPoint(rect.left() - rect.width(), contents.height() - rect.height()));
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
        if (d->movement == Static && cursorAction != MovePageDown)
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
        dist = (d->indexToListViewItem(*it).rect().topLeft() - pos).manhattanLength();
        if (dist < minDist || minDist == 0) {
            minDist = dist;
            closest = *it;
        }
    }

    return closest;
}

/*!
  Returns the rectangle of the item at \a index in contents coordinates.
  \sa itemViewportRect()
*/
QRect QGenericListView::itemRect(const QModelIndex &index) const
{
    if (!index.isValid() || model()->parent(index) != root())
        return QRect();
    return d->indexToListViewItem(index).rect();
}

/*!
  \reimp
*/
void QGenericListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
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
QRect QGenericListView::selectionViewportRect(const QItemSelection &selection) const
{
    if (selection.count() == 1 && selection.at(0).top() == selection.at(0).bottom()) {
        QModelIndex singleItem = model()->index(selection.at(0).top(),
                                                selection.at(0).left(),
                                                selection.at(0).parent());
        return itemViewportRect(singleItem);
    }
    return d->viewport->clipRegion().boundingRect();
}

/*!
  Layout the items according to the flow and wrapping properties.
*/
void QGenericListView::doItemsLayout()
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

    if (d->layoutMode == Instant)
        doItemsLayout(model()->rowCount(root())); // layout everything
    else
        while (!doItemsLayout(100)) // do layout in batches
            qApp->processEvents();
}

/*!
  \internal
*/
bool QGenericListView::doItemsLayout(int delta)
{
    int max = model()->rowCount(root()) - 1;
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
void QGenericListView::doItemsLayout(const QRect &bounds,
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
void QGenericListView::doStaticLayout(const QRect &bounds, int first, int last)
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
    QModelIndex item;
    QFontMetrics fontMetrics(font());
    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    QSize hint;
    QRect rect = bounds;

    if (d->movement == QGenericListView::Static && !wrap)
        if (d->flow == QGenericListView::LeftToRight)
             rect.setHeight(qMin(d->contentsSize.height(), rect.height()));
        else
            rect.setWidth(qMin(d->contentsSize.width(), rect.width()));

    if (d->flow == LeftToRight) {
        int w = bounds.width();
        int dx, dy = (gh ? gh : d->translate);
        for (int i = first; i <= last ; ++i) {
            item = model()->index(i, 0, root());
            hint = delegate->sizeHint(fontMetrics, option, item);
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
            item = model()->index(i, 0, root());
            hint = delegate->sizeHint(fontMetrics, option, item);
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
void QGenericListView::doDynamicLayout(const QRect &bounds, int first, int last)
{
    int gw = d->gridSize.width() > 0 ? d->gridSize.width() : 0;
    int gh = d->gridSize.height() > 0 ? d->gridSize.height() : 0;
    int spacing = gw && gh ? 0 : d->spacing;
    int x, y;

    if (first == 0) {
        x = bounds.x() + spacing;
        y = bounds.y() + spacing;
        d->tree.reserve(model()->rowCount(root()));
    } else {
        int p = first - 1;
        const QGenericListViewItem item = d->tree.item(p);
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
    QModelIndex bottomRight = model()->bottomRight(root());
    QGenericListViewItem *item = 0;

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
    resizeContents(rect.width(), rect.bottom());

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
        BinTree<QGenericListViewItem>::Node::Type type =
            BinTree<QGenericListViewItem>::Node::Both; // use 2D bsp by default
        if (h / w >= 3)    // simple heuristics to get better bsp
            type = BinTree<QGenericListViewItem>::Node::HorizontalPlane;
        else if (w / h >= 3)
            type = BinTree<QGenericListViewItem>::Node::VerticalPlane;
        d->tree.init(QRect(0, 0, w, h), type); // build tree for bounding rect (not contents rect)
    }

    // insert items in tree
    for (int i = insertFrom; i <= last; i++)
        d->tree.climbTree(d->tree.item(i).rect(), &BinTree<QGenericListViewItem>::insert,
                          reinterpret_cast<void *>(i));

    QRect changedRect(topLeft, rect.bottomRight());
    if (clipRegion().boundingRect().intersects(changedRect))
        d->viewport->update();
}

/*!
  \internal
*/
void QGenericListView::updateGeometries()
{
    QModelIndex index = model()->index(0, 0, root());
    QStyleOptionViewItem option = viewOptions();
    QSize size = itemDelegate()->sizeHint(fontMetrics(), option, index);
    
    horizontalScrollBar()->setSingleStep(size.width() + d->spacing);
    horizontalScrollBar()->setPageStep(d->viewport->width());
    horizontalScrollBar()->setRange(0, d->contentsSize.width() - d->viewport->width());

    verticalScrollBar()->setSingleStep(size.height() + d->spacing);
    verticalScrollBar()->setPageStep(d->viewport->height());
    verticalScrollBar()->setRange(0, d->contentsSize.height() - d->viewport->height());
}

/*
 * private object implementation
 */

QGenericListViewPrivate::QGenericListViewPrivate()
    : QAbstractItemViewPrivate(),
      flow(QGenericListView::TopToBottom),
      movement(QGenericListView::Static),
      iconMode(QGenericListView::Small),
      resizeMode(QGenericListView::Fixed),
      layoutMode(QGenericListView::Instant),
      wrap(false),
      spacing(0),
      layoutStart(0),
      translate(0),
      layoutWraps(0),
      layoutTimer(0)
{}

void QGenericListViewPrivate::init()
{
    rubberBand = new QRubberBand(QRubberBand::Rectangle, viewport);
    rubberBand->hide();
    viewport->setAcceptDrops(true);
}

void QGenericListViewPrivate::prepareItemsLayout()
{
    // initailization of data structs
    int rowCount = q->model()->rowCount(q->root());
    if (movement == QGenericListView::Static) {
        tree.destroy();
        if (flow == QGenericListView::LeftToRight) {
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

void QGenericListViewPrivate::intersectingStaticSet(const QRect &area) const
{
    intersectVector.clear();
    QAbstractItemModel *model = q->model();

    QModelIndex index;
    QModelIndex root = q->root();
    int first, last, count, i, j;
    bool wraps = wrapVector.count() > 1;
    if (flow == QGenericListView::LeftToRight) {
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

void QGenericListViewPrivate::intersectingDynamicSet(const QRect &area) const
{
    intersectVector.clear();
    QGenericListViewPrivate *that = const_cast<QGenericListViewPrivate*>(this);
    that->tree.climbTree(area, &QGenericListViewPrivate::addLeaf, static_cast<void*>(that));
}

void QGenericListViewPrivate::createItems(int to)
{
    int count = tree.itemCount();
    QSize size;
    QStyleOptionViewItem option = q->viewOptions();
    QFontMetrics fontMetrics(q->font());
    QAbstractItemModel *model = q->model();
    QAbstractItemDelegate *delegate = q->itemDelegate();
    QModelIndex root = q->root();
    for (int i = count; i < to; ++i) {
        size = delegate->sizeHint(fontMetrics, option, model->index(i, 0, root));
        QGenericListViewItem item(QRect(0, 0, size.width(), size.height()), i); // default pos
        tree.appendItem(item);
    }
}

QRect QGenericListViewPrivate::drawItems(QPainter *painter, const QVector<QModelIndex> &indices) const
{
    QStyleOptionViewItem option = q->viewOptions();
    QAbstractItemDelegate *delegate = q->itemDelegate();
    QVector<QModelIndex>::const_iterator it = indices.begin();
    QGenericListViewItem item = indexToListViewItem(*it);
    QRect rect(item.x, item.y, item.w, item.h);
    for (; it != indices.end(); ++it) {
        item = indexToListViewItem(*it);
        option.rect.setRect(item.x, item.y, item.w, item.h);
        delegate->paint(painter, option, *it);
        rect |= option.rect;
    }
    return rect;
}

QGenericListViewItem QGenericListViewPrivate::indexToListViewItem(const QModelIndex &index) const
{
    if (!index.isValid())
        return QGenericListViewItem();

    if (movement != QGenericListView::Static)
        if (index.row() < tree.itemCount())
            return tree.const_item(index.row());
        else
            return QGenericListViewItem();

    // movement == Static
    if ((flow == QGenericListView::LeftToRight && index.row() >= xposVector.count()) ||
        (flow == QGenericListView::TopToBottom && index.row() >= yposVector.count()))
        return QGenericListViewItem();
    
    int i = qBinarySearch<int>(wrapVector, index.row(), 0, layoutWraps);
    QPoint pos;
    if (flow == QGenericListView::LeftToRight) {
        pos.setX(xposVector.at(index.row()));
        pos.setY(yposVector.at(i));
    } else { // TopToBottom
        pos.setY(yposVector.at(index.row()));
        pos.setX(xposVector.at(i));
    }
    
    QStyleOptionViewItem option = q->viewOptions();
    QSize size = q->itemDelegate()->sizeHint(q->fontMetrics(), option, index);
    return QGenericListViewItem(QRect(pos, size), index.row());
}

int QGenericListViewPrivate::itemIndex(const QGenericListViewItem item) const
{
    int i = item.indexHint;
    if (movement == QGenericListView::Static || i >= tree.itemCount() || tree.const_item(i) == item)
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

void QGenericListViewPrivate::addLeaf(QVector<int> &leaf, const QRect &area,
                                      uint visited, void *data)
{
    QGenericListViewItem *vi;
    QGenericListViewPrivate *_this = static_cast<QGenericListViewPrivate *>(data);
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

void QGenericListViewPrivate::createStaticRow(int &x, int &y, int &dy, int &wraps, int i,
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

void QGenericListViewPrivate::createStaticColumn(int &x, int &y, int &dx, int &wraps, int i,
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

void QGenericListViewPrivate::initStaticLayout(int &x, int &y, int first,
                                               const QRect &bounds, int spacing)
{
    if (first == 0) {
        x = bounds.left() + spacing;
        xposVector.clear();
        if (flow == QGenericListView::TopToBottom)
            xposVector.push_back(x);
        y = bounds.top() + spacing;
        yposVector.clear();
        if (flow == QGenericListView::LeftToRight)
            yposVector.push_back(y);
        layoutWraps = 0;
        wrapVector.clear();
        wrapVector.push_back(0);
    } else if (wrap) {
        if (flow == QGenericListView::LeftToRight) {
            x = xposVector.back();
            xposVector.pop_back();
        } else {
            x = xposVector.at(layoutWraps);
        }
        if (flow == QGenericListView::TopToBottom) {
            y = yposVector.back();
            yposVector.pop_back();
        } else {
            y = yposVector.at(layoutWraps);
        }
    } else {
        if (flow == QGenericListView::LeftToRight) {
            x = xposVector.back();
            xposVector.pop_back();
        } else {
            x = bounds.left() + spacing;
        }
        if (flow == QGenericListView::TopToBottom) {
            y = yposVector.back();
            yposVector.pop_back();
        } else {
            y = bounds.top() + spacing;
        }
    }
}

void QGenericListViewPrivate::insertItem(int index, QGenericListViewItem &item)
{
    tree.insertItem(item, item.rect(), index);
}

void QGenericListViewPrivate::removeItem(int index)
{
    tree.removeItem(tree.item(index).rect(), index);
}

void QGenericListViewPrivate::moveItem(int index, const QPoint &dest)
{
    // does not impact on the bintree itself or the contents rect
    QGenericListViewItem *item = tree.itemPtr(index);
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

QPoint QGenericListViewPrivate::snapToGrid(const QPoint &pos) const
{
    int x = pos.x() - (pos.x() % gridSize.width());
    int y = pos.y() - (pos.y() % gridSize.height());
    return QPoint(x, y);
}

QRect QGenericListViewPrivate::mapToViewport(const QRect &rect) const
{
    QRect result(rect.left() - q->horizontalScrollBar()->value(),
                 rect.top() - q->verticalScrollBar()->value(),
                 rect.width(), rect.height());
    // If the listview is in "listbox-mode", the items are as wide as the viewport
    if (!wrap && movement == QGenericListView::Static)
        if (flow == QGenericListView::TopToBottom)
            result.setWidth(qMax(rect.width(), viewport->width()));
        else
            result.setHeight(qMax(rect.height(), viewport->height()));
    return result;
}
