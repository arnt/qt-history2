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
    } else { // if (t == Node::HorizontalPlane) {
        front.setTop(center.y());
        back.setBottom(center.y() - 1);
    }

    int idx = firstChildIndex(index);
    if (--depth) {
        init(back, depth, type, idx);
        init(front, depth, type, idx + 1);
    }
}

QGenericListView::QGenericListView(QAbstractItemModel *model, QWidget *parent)
    : QAbstractItemView(*new QGenericListViewPrivate, model, parent)
{
    d->init();
}

QGenericListView::QGenericListView(QGenericListViewPrivate &dd, QAbstractItemModel *model,
                                   QWidget *parent)
    : QAbstractItemView(dd, model, parent)
{
    d->init();
}

QGenericListView::~QGenericListView()
{
}

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

void QGenericListView::setIconSize(Size size)
{
    d->size = size;
    if (isVisible())
        doItemsLayout();
}

QGenericListView::Size QGenericListView::iconSize() const
{
    return d->size;
}

void QGenericListView::setResizeMode(ResizeMode mode)
{
    d->resizeMode = mode;
}

QGenericListView::ResizeMode QGenericListView::resizeMode() const
{
    return d->resizeMode;
}

void QGenericListView::setLayoutMode(LayoutMode mode)
{
    d->layoutMode = mode;
}

QGenericListView::LayoutMode QGenericListView::layoutMode() const
{
    return d->layoutMode;
}

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

QRect QGenericListView::itemViewportRect(const QModelIndex &index) const
{
    return d->mapToViewport(itemRect(index));
}

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

void QGenericListView::scrollContentsBy(int dx, int dy)
{
    QRect rect = d->draggedItemsRect;
    rect.moveBy(dx, dy);
    d->viewport->scroll(dx, dy);
    d->viewport->repaint(rect);
}

void QGenericListView::resizeContents(int w, int h)
{
    d->contentsSize = QSize(w, h);
    horizontalScrollBar()->setRange(0, w - viewport()->width());
    verticalScrollBar()->setRange(0, h - viewport()->height());
}

void QGenericListView::contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // FIXME: do something here
    QAbstractItemView::contentsChanged(topLeft, bottomRight);
}

void QGenericListView::contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QModelIndex parent = model()->parent(topLeft);
    contentsRemoved(topLeft, bottomRight);
}

void QGenericListView::contentsRemoved(const QModelIndex &topLeft, const QModelIndex &)
{
    QModelIndex parent = model()->parent(topLeft);
    if (parent != root())
        return;

    if (isVisible())
        doItemsLayout();

    bool needMore = false;
    if ((d->flow == TopToBottom && !d->wrap) || (d->flow == LeftToRight && d->wrap))
        needMore = viewport()->height() >= d->contentsSize.height();
    else
        needMore = viewport()->width() >= d->contentsSize.width();
    if (needMore)
        emit this->needMore();
}

void QGenericListView::mouseMoveEvent(QMouseEvent *e)
{
    QAbstractItemView::mouseMoveEvent(e);
    if (state() == QAbstractItemView::Selecting && d->selectionMode != Single) {
        QPoint topLeft(d->pressedPosition.x() - horizontalOffset(),
                       d->pressedPosition.y() - verticalOffset());
        QRect rect(mapToGlobal(topLeft), mapToGlobal(e->pos()));
        d->rubberBand->setGeometry(rect.normalize());
        if (!d->rubberBand->isVisible()) {
            d->rubberBand->show();
            d->rubberBand->raise();
        }
    }
}

void QGenericListView::mouseReleaseEvent(QMouseEvent *e)
{
    QAbstractItemView::mouseReleaseEvent(e);
    d->rubberBand->hide();
}

void QGenericListView::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->layoutTimer) {
        killTimer(d->layoutTimer);
        d->layoutTimer = 0;
        doItemsLayout();
    }
    QAbstractItemView::timerEvent(e);
}

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

void QGenericListView::dragLeaveEvent(QDragLeaveEvent *)
{
    d->draggedItemsPos = QPoint(-1, -1); // don't draw the dragged items
    d->viewport->update(d->draggedItemsRect); // erase the area
}

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
            moveItem(index.row(), rect.topLeft() + delta);
            d->viewport->update(d->mapToViewport(rect));
            updateItem(index);
        }
        stopAutoScroll();
    } else {
        QAbstractItemView::dropEvent(e);
    }
}

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

void QGenericListView::startDrag()
{
    QAbstractItemView::startDrag();
    // clear dragged items
    d->draggedItems.clear();
}

void QGenericListView::getViewOptions(QItemOptions *options) const
{
    QAbstractItemView::getViewOptions(options);
    if (d->size == Automatic ? d->wrap : d->size == Small) {
        options->smallItem = true;
        options->decorationPosition = QApplication::reverseLayout()
                                      ? QItemOptions::Right : QItemOptions::Left;
    } else {
        options->smallItem = false;
        options->decorationPosition = QItemOptions::Top;
    }
}

void QGenericListView::paintEvent(QPaintEvent *e)
{
    QPainter painter(viewport());
    QRect area = e->rect();
    area.moveBy(horizontalScrollBar()->value(), verticalScrollBar()->value());

    // fill the intersectVector
    if (d->movement == Static)
        d->intersectingStaticSet(area);
    else
        d->intersectingDynamicSet(area);

    QItemOptions options;
    getViewOptions(&options);

    QModelIndex current = currentItem();
    QAbstractItemDelegate *delegate = itemDelegate();
    QItemSelectionModel *selections = selectionModel();
    bool focus = q->hasFocus() && current.isValid();
    QVector<QModelIndex>::iterator it = d->intersectVector.begin();
    for (; it != d->intersectVector.end(); ++it) {
        options.itemRect = itemViewportRect(*it);
        options.selected = selections ? selections->isSelected(*it) : false;
        options.focus = (focus && current == *it);
        delegate->paint(&painter, options, *it);
    }

    if (!d->draggedItems.isEmpty() && d->viewport->rect().contains(d->draggedItemsPos)) {
        QPoint delta = (d->movement == Snap
                        ? d->snapToGrid(d->draggedItemsPos) - d->snapToGrid(d->pressedPosition)
                        : d->draggedItemsPos - d->pressedPosition);
        painter.translate(delta.x(), delta.y());
        d->draggedItemsRect = d->drawItems(&painter, d->draggedItems);
        d->draggedItemsRect.moveBy(delta.x(), delta.y());
    }
}

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

int QGenericListView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int QGenericListView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

QModelIndex QGenericListView::moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState)
{
    QRect area = itemRect(currentItem());
    int iw = area.width();
    int ih = area.height();
    int cw = d->contentsSize.width();
    int ch = d->contentsSize.height();
    QPoint pos = area.topLeft();
    d->intersectVector.clear();

    switch (cursorAction) {
    case MoveLeft:
        area.moveRight(area.left() - 1);
        while (d->intersectVector.count() == 0) {
            if (d->movement == Static)
                d->intersectingStaticSet(area);
            else
                d->intersectingDynamicSet(area);
            if (area.left() <= 0)
                area.setRect(cw - iw, area.top() - ih - 1, iw, ih);
            else
                area.moveRight(area.left() - 1);
            if (area.top() > ch || area.bottom() < 0)
                break;
        }
        break;
    case MoveRight:
        area.moveLeft(area.right() + 1);
        while (d->intersectVector.count() == 0) {
            if (d->movement == Static)
                d->intersectingStaticSet(area);
            else
                d->intersectingDynamicSet(area);
            if (area.right() >= cw)
                area.setRect(0, area.bottom() + 1, iw, ih);
            else
                area.moveLeft(area.right() + 1);
            if (area.top() > ch || area.bottom() < 0)
                break;
        }
        break;
    case MovePageUp:
        area.moveTop(area.top() - viewport()->height() + (ih << 1));
        if (area.top() < 0)
            area.moveTop(ch + (ih << 1));
    case MoveUp:
        area.moveBottom(area.top() - 1);
        while (d->intersectVector.count() == 0) {
            if (d->movement == Static)
                d->intersectingStaticSet(area);
            else
                d->intersectingDynamicSet(area);
            if (area.top() <= 0)
                area.setRect(area.left() - iw - 1, ch - ih, iw, ih);
            else
                area.moveBottom(area.top() - 1);
            if (area.left() > cw || area.right() < 0)
                break;
        }
        break;

    case MovePageDown:
        area.moveTop(area.top() + d->viewport->height() - (ih << 1));
        if (area.top() > ch)
            area.moveTop(ch - (ih << 1));
    case MoveDown:
        area.moveTop(area.bottom() + 1);
        while (d->intersectVector.count() == 0) {
            if (d->movement == Static)
                d->intersectingStaticSet(area);
            else
                d->intersectingDynamicSet(area);
            if (area.bottom() >= ch)
                area.setRect(area.right() + 1, 0, iw, ih);
            else
                area.moveTop(area.bottom() + 1);
            if (area.left() > cw || area.right() < 0)
                break;
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

QRect QGenericListView::itemRect(const QModelIndex &item) const
{
    if (!item.isValid() || model()->parent(item) != root())
        return QRect();
    return d->indexToListViewItem(item).rect();
}

void QGenericListView::setSelection(const QRect &rect, int selectionCommand)
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

    selectionModel()->select(selection, selectionCommand);
}

QRect QGenericListView::selectionViewportRect(const QItemSelection &selection) const
{
    if (selection.count() == 1 && selection.at(0).top() == selection.at(0).bottom()) {
        QModelIndex singleItem = model()->index(selection.at(0).top(),
                                                selection.at(0).left(),
                                                selection.at(0).parent());
        return itemViewportRect(singleItem);
    }
    return d->viewport->clipRegion().boundingRect();
//     QList<QModelIndex> items = selection.items(model());
//     QList<QModelIndex>::iterator it = items.begin();
//     QRect rect;
//     for (; it != items.end(); ++it)
//         rect |= itemRect(*it);
//     items.clear();
//     rect.moveLeft(rect.left() - horizontalScrollBar()->value());
//     rect.moveTop(rect.top() - verticalScrollBar()->value());
//     if (!d->wrap && d->movement == QGenericListView::Static)
//         if (d->flow == QGenericListView::TopToBottom)
//             rect.setWidth(d->viewport->width());
//         else
//             rect.setHeight(d->viewport->height());
//     return rect;
}

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
    QItemOptions options;
    getViewOptions(&options);
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
            hint = delegate->sizeHint(fontMetrics, options, item);
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
            hint = delegate->sizeHint(fontMetrics, options, item);
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

bool QGenericListView::supportsDragAndDrop() const
{
    return true;
}

void QGenericListView::insertItem(int index, QGenericListViewItem &item)
{
    d->tree.insertItem(item, item.rect(), index);
}

void QGenericListView::removeItem(int index)
{
    d->tree.removeItem(d->tree.item(index).rect(), index);
}

void QGenericListView::moveItem(int index, const QPoint &dest)
{
    // does not impact on the bintree itself or the contents rect
    QGenericListViewItem *item = d->tree.itemPtr(index);
    QRect rect = item->rect();
    d->tree.moveItem(dest, rect, index);

    // resize the contents area
    rect = item->rect();
    int w = item->x + rect.width();
    int h = item->y + rect.height();
    w = w > d->contentsSize.width() ? w : d->contentsSize.width();
    h = h > d->contentsSize.height() ? h : d->contentsSize.height();
    resizeContents(w, h);
}

void QGenericListView::updateGeometries()
{
    horizontalScrollBar()->setPageStep(d->viewport->width());
    horizontalScrollBar()->setRange(0, d->contentsSize.width() - d->viewport->width());
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
      size(QGenericListView::Small),
      resizeMode(QGenericListView::Fixed),
      layoutMode(QGenericListView::Instant),
      wrap(false),
      spacing(5),
      arrange(false),
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
    QGenericListViewPrivate *that = const_cast<QGenericListViewPrivate *>(this);
    that->tree.climbTree(area, &QGenericListViewPrivate::addLeaf, static_cast<void *>(that));
}

void QGenericListViewPrivate::createItems(int to)
{
    int count = tree.itemCount();
    QSize size;
    QItemOptions options;
    q->getViewOptions(&options);
    QFontMetrics fontMetrics(q->font());
    QAbstractItemModel *model = q->model();
    QAbstractItemDelegate *delegate = q->itemDelegate();
    QModelIndex root = q->root();
    for (int i = count; i < to; ++i) {
        size = delegate->sizeHint(fontMetrics, options, model->index(i, 0, root));
        QGenericListViewItem item(QRect(0, 0, size.width(), size.height()), i); // default pos
        tree.appendItem(item);
    }
}

QRect QGenericListViewPrivate::drawItems(QPainter *painter, const QVector<QModelIndex> &indices) const
{
    QItemOptions options;
    q->getViewOptions(&options);
    QAbstractItemDelegate *delegate = q->itemDelegate();
    QVector<QModelIndex>::const_iterator it = indices.begin();
    QGenericListViewItem item = indexToListViewItem(*it);
    QRect rect(item.x, item.y, item.w, item.h);
    for (; it != indices.end(); ++it) {
        item = indexToListViewItem(*it);
        options.itemRect.setRect(item.x, item.y, item.w, item.h);
        delegate->paint(painter, options, *it);
        rect |= options.itemRect;
    }
    return rect;
}

QGenericListViewItem QGenericListViewPrivate::indexToListViewItem(const QModelIndex &item) const
{
    if (!item.isValid())
        return QGenericListViewItem();

    if (movement != QGenericListView::Static)
        if (item.row() < tree.itemCount())
            return tree.const_item(item.row());
        else {
            qDebug("returns invalid listview item");
            return QGenericListViewItem();
        }

    // movement == Static
    if ((flow == QGenericListView::LeftToRight && item.row() >= xposVector.count()) ||
        (flow == QGenericListView::TopToBottom && item.row() >= yposVector.count()))
        return QGenericListViewItem();
    QItemOptions options;
    q->getViewOptions(&options);
    QSize hint = q->itemDelegate()->sizeHint(q->fontMetrics(), options, item);
    int i = qBinarySearch<int>(wrapVector, item.row(), 0, layoutWraps);
    QPoint pos;
    if (flow == QGenericListView::LeftToRight) {
        pos.setX(xposVector.at(item.row()));
        pos.setY(yposVector.at(i));
    } else { // TopToBottom
        pos.setY(yposVector.at(item.row()));
        pos.setX(xposVector.at(i));
    }
    return QGenericListViewItem(QRect(pos, hint), item.row());
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
            result.setWidth(viewport->width());
        else
            result.setHeight(viewport->height());
    return result;
}
