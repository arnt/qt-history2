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

#ifndef QT_NO_LISTVIEW
#include <qabstractitemdelegate.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qvector.h>
#include <qstyle.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <qrubberband.h>
#include <private/qlistview_p.h>
#include <qdebug.h>

/*!
    \class QListView qlistview.h

    \brief The QListView class provides a list or icon view onto a model.

    \ingroup model-view
    \mainclass

    A QListView presents items stored in a model, either as a simple
    non-hierarchical list, or as a collection of icons. This class is used
    to provide lists and icon views that were previously provided by the
    \c QListBox and \c QIconView classes, but using the more flexible
    approach provided by Qt's model/view architecture.

    The QListView class is one of the \l{Model/View Classes}
    and is part of Qt's \l{Model/View Programming}{model/view framework}.

    This view does not display horizontal or vertical headers; to display
    a list of items with a horizontal header, use QTreeView instead.

    QListView implements the interfaces defined by the
    QAbstractItemView class to allow it to display data provided by
    models derived from the QAbstractItemModel class.

    Items in a list view can be displayed using one of two view modes:
    In \l ListMode, the items are displayed in the form of a simple list;
    in \l IconMode, the list view takes the form of an \e{icon view} in
    which the items are displayed with icons like files in a file manager.
    By default, the list view is in \l ListMode. To change the view mode,
    use the setViewMode() function, and to determine the current view mode,
    use viewMode().

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

    \table 100%
    \row \o \inlineimage windowsxp-listview.png
         \o \inlineimage macintosh-listview.png
         \o \inlineimage plastique-listview.png
    \row \o A WindowsXP style list view.
         \o A Macintosh style list view.
         \o A Plastique style list view.
    \row \o \inlineimage cde-listview.png
         \o \inlineimage motif-listview.png
    \row \o A CDE style list view.
         \o A Motif style list view.
    \endtable

    \sa {Model/View Programming}, QTreeView, QTableView, QListWidget
*/

/*!
    \enum QListView::ViewMode

    \value ListMode The items are layed out using TopToBottom flow, with Small size and Static movement
    \value IconMode The items are layed out using LeftToRight flow, with Large size and Free movement
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
  \enum QListView::ResizeMode

  \value Fixed The items will only be laid out the first time the view is shown.
  \value Adjust The items will be laid out every time the view is resized.
*/

/*!
  \enum QListView::LayoutMode

  \value SinglePass The items are laid out all at once.
  \value Batched The items are laid out in batches of \l batchSize items.
  \sa batchSize
*/

/*!
    Creates a new QListView with the given \a parent to view a model.
    Use setModel() to set the model.
*/
QListView::QListView(QWidget *parent)
    : QAbstractItemView(*new QListViewPrivate, parent)
{
    setViewMode(ListMode);
    setSelectionMode(SingleSelection);
}

/*!
  \internal
*/
QListView::QListView(QListViewPrivate &dd, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    setViewMode(ListMode);
    setSelectionMode(SingleSelection);
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
    view. \l Static means that the items can't be moved the user. \l
    Free means that the user can drag and drop the items to any
    position in the view. \l Snap means that the user can drag and
    drop the items, but only to the positions in a notional grid
    signified by the gridSize property.

    Setting this property when the view is visible will cause the
    items to be laid out again.

    \sa gridSize
*/
void QListView::setMovement(Movement movement)
{
    Q_D(QListView);
    d->modeProperties |= uint(QListViewPrivate::Movement);
    d->movement = movement;

#ifndef QT_NO_DRAGANDDROP
    bool movable = (movement != Static);
    setDragEnabled(movable);
    d->viewport->setAcceptDrops(movable);
#endif

    d->doDelayedItemsLayout();
}

QListView::Movement QListView::movement() const
{
    Q_D(const QListView);
    return d->movement;
}

/*!
    \property QListView::flow
    \brief which direction the items layout should flow.

    If this property is \l LeftToRight, the items will be laid out left
    to right. If the \l isWrapping property is true, the layout will wrap
    when it reaches the right side of the visible area. If this
    property is \l TopToBottom, the items will be laid out from the top
    of the visible area, wrapping when it reaches the bottom.

    Setting this property when the view is visible will cause the
    items to be laid out again.
*/
void QListView::setFlow(Flow flow)
{
    Q_D(QListView);
    d->modeProperties |= uint(QListViewPrivate::Flow);
    d->flow = flow;
    d->doDelayedItemsLayout();
}

QListView::Flow QListView::flow() const
{
    Q_D(const QListView);
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
    Q_D(QListView);
    d->modeProperties |= uint(QListViewPrivate::Wrap);
    d->wrap = enable;
    d->doDelayedItemsLayout();
}

bool QListView::isWrapping() const
{
    Q_D(const QListView);
    return d->wrap;
}

/*!
    \property QListView::resizeMode
    \brief whether the items are laid out again when the view is resized.

    If this property is \l Adjust, the items will be laid out again
    when the view is resized. If the value is \l Fixed, the items will
    not be laid out when the view is resized.
*/
void QListView::setResizeMode(ResizeMode mode)
{
    Q_D(QListView);
    d->resizeMode = mode;
}

QListView::ResizeMode QListView::resizeMode() const
{
    Q_D(const QListView);
    return d->resizeMode;
}

/*!
    \property QListView::layoutMode
    \brief whether the layout of items should happen immediately or be delayed.

    This property holds the layout mode for the items. When the mode
    is \l SinglePass (the default), the items are laid out all in one go.
    When the mode is \l Batched, the items are laid out in batches of \l batchSize
    items, while processing events. This makes it possible to
    instantly view and interact with the visible items while the rest
    are being laid out.
*/
void QListView::setLayoutMode(LayoutMode mode)
{
    Q_D(QListView);
    d->layoutMode = mode;
}

QListView::LayoutMode QListView::layoutMode() const
{
    Q_D(const QListView);
    return d->layoutMode;
}

/*!
    \property QListView::spacing
    \brief the space between items in the layout

    This property is the size of the empty space between items in the
    layout.

    Setting this property when the view is visible will cause the
    items to be laid out again.
*/
void QListView::setSpacing(int space)
{
    Q_D(QListView);
    d->modeProperties |= uint(QListViewPrivate::Spacing);
    d->spacing = space;
    d->doDelayedItemsLayout();
}

int QListView::spacing() const
{
    Q_D(const QListView);
    return d->spacing;
}

/*!
    \property QListView::batchSize
    \brief the number of items laid out in each batch if \l layoutMode is
    set to \l Batched

    The default value is 100.

    \since 4.2
*/

void QListView::setBatchSize(int batchSize)
{
    Q_D(QListView);
    if (batchSize <= 0) {
        qWarning("Invalid batchSize (%d)", batchSize);
        return;
    }
    d->batchSize = batchSize;
}

int QListView::batchSize() const
{
    Q_D(const QListView);
    return d->batchSize;
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
    Q_D(QListView);
    d->modeProperties |= uint(QListViewPrivate::GridSize);
    d->gridSize = size;
    d->doDelayedItemsLayout();
}

QSize QListView::gridSize() const
{
    Q_D(const QListView);
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
    Q_D(QListView);
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
        if (!(d->modeProperties & QListViewPrivate::ResizeMode))
            d->resizeMode = Fixed;
    }

#ifndef QT_NO_DRAGANDDROP
    bool movable = (d->movement != Static);
    setDragEnabled(movable);
    setAcceptDrops(movable);
#endif
    d->doDelayedItemsLayout();
}

QListView::ViewMode QListView::viewMode() const
{
    Q_D(const QListView);
    return d->viewMode;
}

/*!
    Clears the property flags. See \l{viewMode}.
*/
void QListView::clearPropertyFlags()
{
    Q_D(QListView);
    d->modeProperties = 0;
}

/*!
  Returns true if the \a row is hidden, otherwise returns false.

  \sa setRowHidden
*/

bool QListView::isRowHidden(int row) const
{
    Q_D(const QListView);
    return d->hiddenRows.contains(row);
}

/*!
  If \a hide is true the \a row will be hidden, otherwise the \a row will be shown.
*/

void QListView::setRowHidden(int row, bool hide)
{
    Q_D(QListView);
    if (hide && !isRowHidden(row))
        d->hiddenRows.append(row);
    else if (!hide && isRowHidden(row))
        d->hiddenRows.remove(d->hiddenRows.indexOf(row));
    d->doDelayedItemsLayout();
}

/*!
  \reimp
*/
QRect QListView::visualRect(const QModelIndex &index) const
{
    if (!index.isValid() || isIndexHidden(index))
        return QRect();
    
    Q_D(const QListView);
    d->executePostedLayout();
    return d->mapToViewport(rectForIndex(index));
}

/*!
  \reimp
*/
void QListView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_D(QListView);

    if (index.parent() != rootIndex() || index.column() != d->column)
        return;

    QRect area = d->viewport->rect();
    QRect rect = visualRect(index);
    if (hint == EnsureVisible && area.contains(rect)) {
        d->setDirtyRegion(rect);
        return;
    }

    // vertical
    int vy = verticalScrollBar()->value();
    bool above = (hint == EnsureVisible && rect.top() < area.top());
    bool below = (hint == EnsureVisible && rect.bottom() > area.bottom());
    if (hint == PositionAtTop || above)
        verticalScrollBar()->setValue(vy + rect.top());
    else if (hint == PositionAtBottom || below)
        verticalScrollBar()->setValue(vy + rect.bottom() - viewport()->height());

    // horizontal
    int vx = horizontalScrollBar()->value();
    if (isRightToLeft()) {
        if ((rect.left() < area.left()) && (rect.right() < area.right())) // left of
            horizontalScrollBar()->setValue(vx - rect.left());
        else if (rect.right() > area.right()) // right of
            horizontalScrollBar()->setValue(vx - rect.right() + viewport()->width());
    } else {
        if (rect.left() < area.left()) // left of
            horizontalScrollBar()->setValue(vx + rect.left());
        else if ((rect.right() > area.right()) && (rect.left() > area.left())) // right of
            horizontalScrollBar()->setValue(vx + rect.right() - viewport()->width());
    }
}

/*!
  \internal
*/
void QListView::reset()
{
    Q_D(QListView);
    d->clear();
    d->hiddenRows.clear();
    QAbstractItemView::reset();
}

/*!
  \internal
*/
void QListView::setRootIndex(const QModelIndex &index)
{
    Q_D(QListView);
    if (model())
        d->column = qBound(0, d->column, model()->columnCount(index) - 1);
    QAbstractItemView::setRootIndex(index);
}

/*!
    \internal

    Scroll the view contents by \a dx and \a dy.
*/
void QListView::scrollContentsBy(int dx, int dy)
{
    Q_D(QListView);

    dx = isRightToLeft() ? -dx : dx;

    if (state() == DragSelectingState) {
        if (dx > 0) // right
            d->elasticBand.moveRight(d->elasticBand.right() + dx);
        else if (dx < 0) // left
            d->elasticBand.moveLeft(d->elasticBand.left() - dx);
        if (dy > 0) // down
            d->elasticBand.moveBottom(d->elasticBand.bottom() + dy);
        else if (dy < 0) // up
            d->elasticBand.moveTop(d->elasticBand.top() - dy);
    }

    d->scrollContentsBy(dx, dy);

    // update the dragged items
    if (!d->draggedItems.isEmpty())
        d->setDirtyRegion(d->draggedItemsRect().translated(dx, dy));
}

/*!
    \internal

    Resize the internal contents to \a width and \a height and set the
    scrollbar ranges accordingly.
*/
void QListView::resizeContents(int width, int height)
{
    Q_D(QListView);
    d->contentsSize = QSize(width, height);
    horizontalScrollBar()->setRange(0, width - viewport()->width() - 1);
    verticalScrollBar()->setRange(0, height - viewport()->height() - 1);
}

/*!
    \internal
*/
QSize QListView::contentsSize() const
{
    return d_func()->contentsSize;
}

/*!
  \reimp
*/
void QListView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_D(QListView);
    if (d->movement != Static
        && d->column >= topLeft.column()
        && d->column <= bottomRight.column()) {
        QStyleOptionViewItem option = viewOptions();
        int bottom = qMin(d->items.count(), bottomRight.row() + 1);
        for (int row = topLeft.row(); row < bottom; ++row) {
            QModelIndex idx = d->model->index(row, d->column, d->root);
            d->items[row].resize(d->itemSize(option, idx));
        }
    }
    QAbstractItemView::dataChanged(topLeft, bottomRight);
}

/*!
  \reimp
*/
void QListView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(QListView);
    // if the parent is above rootIndex() in the tree, nothing will happen
    if (parent == rootIndex()) {
        int count = (end - start + 1);
        for (int i = d->hiddenRows.count() - 1; i >= 0; --i)
            if (d->hiddenRows.at(i) > start)
                d->hiddenRows[i] += count;
    }
    d->clear();
    d->doDelayedItemsLayout();
    QAbstractItemView::rowsInserted(parent, start, end);
}

/*!
  \reimp
*/
void QListView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(QListView);
    // if the parent is above rootIndex() in the tree, nothing will happen
    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
    if (parent == rootIndex()) {
        int count = (end - start + 1);
        for (int i = d->hiddenRows.count() - 1; i >= 0; --i) {
            if (d->hiddenRows.at(i) > start) {
                if (d->hiddenRows.at(i) < end) {
                    d->hiddenRows.remove(i);
                } else {
                    d->hiddenRows[i] -= count;
                }
            }
        }
    }
    d->clear();
    d->doDelayedItemsLayout();
}

/*!
  \reimp
*/
void QListView::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QListView);
    QAbstractItemView::mouseMoveEvent(e);
    if (d->movement != Static
        && state() == DragSelectingState
        && d->selectionMode != SingleSelection) {
        QRect rect(d->pressedPosition, e->pos() + QPoint(horizontalOffset(), verticalOffset()));
        rect = rect.normalized();
        d->setDirtyRegion(d->mapToViewport(rect.unite(d->elasticBand)));
        d->elasticBand = rect;
    }
}

/*!
  \reimp
*/
void QListView::mouseReleaseEvent(QMouseEvent *e)
{
    Q_D(QListView);
    QAbstractItemView::mouseReleaseEvent(e);
    if (d->elasticBand.isValid()) {
        d->setDirtyRegion(d->mapToViewport(d->elasticBand));
        d->elasticBand = QRect();
    }
}

/*!
  \reimp
*/
void QListView::timerEvent(QTimerEvent *e)
{
    Q_D(QListView);
    if (e->timerId() == d->startLayoutTimer.timerId()) {
        d->startLayoutTimer.stop();
        setState(ExpandingState); // showing the scrollbars will trigger a resize event,
        doItemsLayout();          // so we set the state to expanding to avoid
        setState(NoState);        // triggering another layout
    } else if (e->timerId() == d->batchLayoutTimer.timerId()) {
        if (d->doItemsLayout(d->batchSize)) { // layout is done
            d->batchLayoutTimer.stop();
            updateGeometries();
            d->viewport->update();
        }
    }
    QAbstractItemView::timerEvent(e);
}

/*!
  \reimp
*/
void QListView::resizeEvent(QResizeEvent *e)
{
    Q_D(QListView);
    QAbstractItemView::resizeEvent(e);
    if (state() == NoState) {
        // if we are in adjust mode, post a delayed layout
        if (d->resizeMode == Adjust) {
            QSize delta = e->size() - e->oldSize();
            if (!d->layoutPosted
                && ((d->flow == LeftToRight && delta.width() != 0)
                    || (d->flow == TopToBottom && delta.height() != 0))) {
                d->layoutPosted = true;
                d->startLayoutTimer.start(100, this); // wait 1/10 sec before starting the layout
            }
        }
    }
}

#ifndef QT_NO_DRAGANDDROP

/*!
  \reimp
*/
void QListView::dragMoveEvent(QDragMoveEvent *e)
{
    Q_D(QListView);
    if (e->source() == this && d->movement != Static) {
        // the ignore by default
        e->ignore();
        if (d->canDecode(e)) {
            // get old dragged items rect
            QRect itemsRect = d->itemsRect(d->draggedItems);
            d->setDirtyRegion(itemsRect.translated(d->draggedItemsDelta()));
            // update position
            d->draggedItemsPos = e->pos();
            // get new items rect
            d->setDirtyRegion(itemsRect.translated(d->draggedItemsDelta()));
            // set the item under the cursor to current
            QModelIndex index = indexAt(e->pos());
            // check if we allow drops here
            if (e->source() == this && d->draggedItems.contains(index))
                e->accept(); // allow changing item position
            else if (model()->flags(index) & Qt::ItemIsDropEnabled)
                e->accept(); // allow dropping on dropenabled items
            else if (!index.isValid())
                e->accept(); // allow dropping in empty areas
        }
        // do autoscrolling
        if (d->shouldAutoScroll(e->pos()))
            startAutoScroll();
    } else { // not internal
        QAbstractItemView::dragMoveEvent(e);
    }
}

/*!
  \reimp
*/
void QListView::dragLeaveEvent(QDragLeaveEvent *e)
{
    Q_D(QListView);
    if (d->movement != Static) {
        d->viewport->update(d->draggedItemsRect()); // erase the area
        d->draggedItemsPos = QPoint(-1, -1); // don't draw the dragged items
    }
    QAbstractItemView::dragLeaveEvent(e);
}

/*!
  \reimp
*/
void QListView::dropEvent(QDropEvent *e)
{
    Q_D(QListView);
    if (e->source() == this && d->movement != Static)
        internalDrop(e);
    else
        QAbstractItemView::dropEvent(e);
}

/*!
  \reimp
*/
void QListView::startDrag(Qt::DropActions supportedActions)
{
    Q_D(QListView);
    if (d->movement != Static)
        internalDrag(supportedActions);
    else
        QAbstractItemView::startDrag(supportedActions);
}

/*!
    \internal

    Called whenever items from the view is dropped on the viewport.
    The \a event provides additional information.
*/
void QListView::internalDrop(QDropEvent *event)
{
    Q_D(QListView);
    QPoint offset(horizontalOffset(), verticalOffset());
    QPoint end = event->pos() + offset;
    QPoint start = d->pressedPosition;
    QPoint delta = (d->movement == Snap ?
                    d->snapToGrid(end) - d->snapToGrid(start) : end - start);
    QList<QModelIndex> indexes = selectionModel()->selectedIndexes();
    for (int i = 0; i < indexes.count(); ++i) {
        QModelIndex index = indexes.at(i);
        QRect rect = rectForIndex(index);
        d->setDirtyRegion(d->mapToViewport(rect));
        QPoint dest = rect.topLeft() + delta;
        if (isRightToLeft())
            dest.setX(d->flipX(dest.x()) - rect.width());
        d->moveItem(index.row(), dest);
        d->setDirtyRegion(visualRect(index));
    }
    stopAutoScroll();
    d->draggedItems.clear();
    event->accept(); // we have handled the event
}

/*!
    \internal

    Called whenever the user starts dragging items and the items are movable,
    enabling internal dragging and dropping of items.
*/
void QListView::internalDrag(Qt::DropActions supportedActions)
{
    // This function does the same thing as in QAbstractItemView::startDrag(),
    // plus adding viewitems to the draggedItems list.
    // We need these items to draw the drag items
    Q_D(QListView);
    QModelIndexList indexes = selectionModel()->selectedIndexes();
    if (indexes.count() > 0 ) {
        QModelIndexList::ConstIterator it = indexes.begin();
        for (; it != indexes.end(); ++it)
            if (model()->flags(*it) & Qt::ItemIsDragEnabled)
                d->draggedItems.push_back(*it);
        QDrag *drag = new QDrag(this);
        drag->setMimeData(model()->mimeData(indexes));
        Qt::DropAction action = drag->start(supportedActions);
        d->draggedItems.clear();
        if (action == Qt::MoveAction)
            d->removeSelectedRows();
    }
}

#endif // QT_NO_DRAGANDDROP

/*!
  \reimp
*/
QStyleOptionViewItem QListView::viewOptions() const
{
    Q_D(const QListView);
    QStyleOptionViewItem option = QAbstractItemView::viewOptions();
    if (!d->iconSize.isValid()) { // otherwise it was already set in abstractitemview
        int pm = (d->viewMode == ListMode
                  ? style()->pixelMetric(QStyle::PM_ListViewIconSize)
                  : style()->pixelMetric(QStyle::PM_IconViewIconSize));
        option.decorationSize = QSize(pm, pm);
    }
    if (d->viewMode == IconMode) {
        option.showDecorationSelected = false;
        option.decorationPosition = QStyleOptionViewItem::Top;
        option.displayAlignment = Qt::AlignCenter;
    } else {
        option.decorationPosition = QStyleOptionViewItem::Left;
    }
    return option;
}

/*!
  \reimp
*/
void QListView::paintEvent(QPaintEvent *e)
{
    Q_D(QListView);
    QStyleOptionViewItem option = viewOptions();
    QPainter painter(d->viewport);
    QRect area = e->rect();

    QVector<QModelIndex> toBeRendered;
//     QVector<QRect> rects = e->region().rects();
//     for (int i = 0; i < rects.size(); ++i) {
//         d->intersectingSet(rects.at(i).translated(horizontalOffset(), verticalOffset()));
//         toBeRendered += d->intersectVector;
//     }
    d->intersectingSet(e->rect().translated(horizontalOffset(), verticalOffset()), false);
    toBeRendered = d->intersectVector;

    const QPoint offset = d->scrollDelayOffset;
    const QModelIndex current = currentIndex();
    const QModelIndex hover = d->hover;
    const QAbstractItemModel *itemModel = model();
    const QAbstractItemDelegate *delegate = itemDelegate();
    const QItemSelectionModel *selections = selectionModel();
    const bool focus = (hasFocus() || d->viewport->hasFocus()) && current.isValid();
    const bool alternate = d->alternatingColors;
    const QStyle::State state = option.state;
    const QAbstractItemView::State viewState = this->state();
    const bool enabled = (state & QStyle::State_Enabled) != 0;

    bool alternateBase = false;
    if (alternate && !toBeRendered.isEmpty()) {
        int first = toBeRendered.first().row();
        if (!d->hiddenRows.isEmpty()) {
            for (int row = 0; row < first; ++row) {
                if (!d->hiddenRows.contains(row))
                    alternateBase = !alternateBase;
            }
        } else {
            alternateBase = (first & 1) != 0;
        }
    }

    painter.setBrush(option.palette.brush(QPalette::Base));

    QVector<QModelIndex>::const_iterator end = toBeRendered.constEnd();
    for (QVector<QModelIndex>::const_iterator it = toBeRendered.constBegin(); it != end; ++it) {
        Q_ASSERT((*it).isValid());
        option.rect = visualRect(*it).translated(offset);
        option.state = state;
        if (selections && selections->isSelected(*it))
            option.state |= QStyle::State_Selected;
        if (enabled) {
            QPalette::ColorGroup cg;
            if ((itemModel->flags(*it) & Qt::ItemIsEnabled) == 0) {
                option.state &= ~QStyle::State_Enabled;
                cg = QPalette::Disabled;
            } else {
                cg = QPalette::Normal;
            }
            option.palette.setCurrentColorGroup(cg);
        }
        if (focus && current == *it) {
            option.state |= QStyle::State_HasFocus;
            if (viewState == EditingState)
                option.state |= QStyle::State_Editing;
        }
        if (*it == hover)
            option.state |= QStyle::State_MouseOver;
        else
            option.state &= ~QStyle::State_MouseOver;

        if (alternate) {
            painter.setBrush(alternateBase
                             ? option.palette.brush(QPalette::AlternateBase)
                             : option.palette.brush(QPalette::Base));
            alternateBase = !alternateBase;
        }
        painter.fillRect(option.rect, painter.brush());
        delegate->paint(&painter, option, *it);
    }

#ifndef QT_NO_DRAGANDDROP
    if (!d->draggedItems.isEmpty() && d->viewport->rect().contains(d->draggedItemsPos)) {
        QPoint delta = d->draggedItemsDelta();
        painter.translate(delta.x(), delta.y());
        d->drawItems(&painter, d->draggedItems);
    }

    // Paint the dropIndicator
    d_func()->paintDropIndicator(&painter);
#endif

#ifndef QT_NO_RUBBERBAND
    if (d->elasticBand.isValid()) {
        QStyleOptionRubberBand opt;
        opt.init(this);
        opt.shape = QRubberBand::Rectangle;
        opt.opaque = false;
        opt.rect = d->mapToViewport(d->elasticBand).intersect(
            d->viewport->rect().adjusted(-16, -16, 16, 16));
        painter.save();
        style()->drawControl(QStyle::CE_RubberBand, &opt, &painter);
        painter.restore();
    }
#endif
}

/*!
  \reimp
*/
QModelIndex QListView::indexAt(const QPoint &p) const
{
    Q_D(const QListView);
    QRect rect(p.x() + horizontalOffset(), p.y() + verticalOffset(), 1, 1);
    d->intersectingSet(rect);
    QModelIndex index = d->intersectVector.count() > 0
                        ? d->intersectVector.first() : QModelIndex();
    if (index.isValid() && visualRect(index).contains(p))
        return index;
    return QModelIndex();
}

/*!
  \reimp
*/
int QListView::horizontalOffset() const
{
    return isRightToLeft() ? horizontalScrollBar()->maximum() - horizontalScrollBar()->value() : horizontalScrollBar()->value();
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
QModelIndex QListView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    Q_D(QListView);
    Q_UNUSED(modifiers);

    if (!model())
        return QModelIndex();

    QModelIndex current = currentIndex();
    if (!current.isValid()) {
        int rowCount = model()->rowCount(rootIndex());
        if (!rowCount)
            return QModelIndex();
        int row = 0;
        while (row < rowCount && isRowHidden(row))
            ++row;
        if (row >= rowCount)
            return QModelIndex();
        return model()->index(row, 0, rootIndex());
    }

    QRect rect = rectForIndex(current);
    if (rect.isEmpty()) {
        return model()->index(0, 0, rootIndex());
    }

    QSize contents = d->contentsSize;
    QPoint pos = rect.center();
    d->intersectVector.clear();

    switch (cursorAction) {
    case MoveLeft:
        while (d->intersectVector.isEmpty()) {
            rect.translate(-rect.width(), 0);
            if (rect.right() <= 0)
                return current;
            if (rect.left() < 0)
                rect.setLeft(0);
            d->intersectingSet(rect);
            // don't get current in this set
            int idx = d->intersectVector.indexOf(current);
            if (idx > -1)
                d->intersectVector.remove(idx);
        }
        return d->closestIndex(pos, d->intersectVector);
    case MoveRight:
        while (d->intersectVector.isEmpty()) {
            rect.translate(rect.width(), 0);
            if (rect.left() >= contents.width())
                return current;
            if (rect.right() > contents.width())
                rect.setRight(contents.width());
            d->intersectingSet(rect);
            // don't get current in this set
            int idx = d->intersectVector.indexOf(current);
            if (idx > -1)
                d->intersectVector.remove(idx);
        }
        return d->closestIndex(pos, d->intersectVector);
    case MovePageUp:
        rect.moveTop(rect.top() - d->viewport->height());
        if (rect.top() < rect.height())
            rect.moveTop(rect.height());
    case MovePrevious:
    case MoveUp:
        while (d->intersectVector.isEmpty()) {
            rect.translate(0, -rect.height());
            if (rect.bottom() <= 0) {
#ifdef QT_KEYPAD_NAVIGATION
                if (QApplication::keypadNavigationEnabled())
                    return model()->index(d->batchStartRow - 1, d->column, rootIndex());
#endif
                return current;
            }
            if (rect.top() < 0)
                rect.setTop(0);
            d->intersectingSet(rect);
            // don't get current in this set
            int idx = d->intersectVector.indexOf(current);
            if (idx > -1)
                d->intersectVector.remove(idx);
        }
        return d->closestIndex(pos, d->intersectVector);
    case MovePageDown:
        rect.moveTop(rect.top() + d->viewport->height());
        if (rect.bottom() > contents.height() - rect.height())
            rect.moveBottom(contents.height() - rect.height());
    case MoveNext:
    case MoveDown:
        while (d->intersectVector.isEmpty()) {
            rect.translate(0, rect.height());
            if (rect.top() >= contents.height()) {
#ifdef QT_KEYPAD_NAVIGATION
                if (QApplication::keypadNavigationEnabled())
                    return model()->index(0, d->column, rootIndex());
#endif
                return current;
            }
            if (rect.bottom() > contents.height())
                rect.setBottom(contents.height());
            d->intersectingSet(rect);
            // don't get current in this set
            int idx = d->intersectVector.indexOf(current);
            if (idx > -1)
                d->intersectVector.remove(idx);
        }
        return d->closestIndex(pos, d->intersectVector);
    case MoveHome:
        return model()->index(0, d->column, rootIndex());
    case MoveEnd:
        return model()->index(d->batchStartRow - 1, d->column, rootIndex());
    }

    return current;
}

/*!
    Returns the rectangle of the item at position \a index in the
    model. The rectangle is in contents coordinates.
*/
QRect QListView::rectForIndex(const QModelIndex &index) const
{
    Q_D(const QListView);
    if (!index.isValid()
        || index.parent() != rootIndex()
        || index.column() != d->column
        || isIndexHidden(index))
        return QRect();
    d->executePostedLayout();
    QListViewItem item = d->indexToListViewItem(index);
    return d->viewItemRect(item);
}

/*!
    \since 4.1

    Sets the contents position of the item at \a index in the model to the given
    \a position.
    If the list view's movement mode is Static, this function will have no
    effect.
*/
void QListView::setPositionForIndex(const QPoint &position, const QModelIndex &index)
{
    Q_D(QListView);
    if (d->movement == Static
        || !index.isValid()
        || index.parent() != rootIndex()
        || index.column() != d->column
        || index.row() >= d->items.count())
        return;
    d->executePostedLayout();
    QListViewItem *item = &d->items[index.row()];
    if (isVisible())
        d->viewport->update(visualRect(index)); // update old position
    item->move(position);
    if (isVisible())
        d->viewport->update(visualRect(index)); // update new position
}

/*!
  \reimp
*/
void QListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    Q_D(QListView);
    if (!selectionModel())
        return;

    d->intersectingSet(rect.translated(horizontalOffset(), verticalOffset()));

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
            selection.select(tl, br); // select current range
            tl = br = *it; // start new range
        }
    }
    if (tl.isValid() && br.isValid())
        selection.select(tl, br);

    selectionModel()->select(selection, command);
}

/*!
  \reimp
*/
QRegion QListView::visualRegionForSelection(const QItemSelection &selection) const
{
    Q_D(const QListView);
    // ### NOTE: this is a potential bottleneck in non-static mode
    int c = d->column;
    QRegion selectionRegion;
    for (int i = 0; i < selection.count(); ++i) {
        if (!selection.at(i).isValid())
            continue;
        QModelIndex parent = selection.at(i).topLeft().parent();
        int t = selection.at(i).topLeft().row();
        int b = selection.at(i).bottomRight().row();
        if (d->movement != Static || d->wrap) { // in non-static mode, we have to go through all selected items
            for (int r = t; r <= b; ++r)
                selectionRegion += QRegion(visualRect(d->model->index(r, c, parent)));
        } else { // in static mode, we can optimize a bit
            QRect rect(visualRect(d->model->index(t, c, parent)).topLeft(),
                       visualRect(d->model->index(b, c, parent)).bottomRight());
            selectionRegion += QRegion(rect);
        }
    }

    return selectionRegion;
}

/*!
  \reimp
*/
QModelIndexList QListView::selectedIndexes() const
{
    Q_D(const QListView);
    QModelIndexList viewSelected;
    QModelIndexList modelSelected;
    if (selectionModel())
        modelSelected = selectionModel()->selectedIndexes();
    for (int i = 0; i < modelSelected.count(); ++i) {
        QModelIndex index = modelSelected.at(i);
        if (!isIndexHidden(index) && index.parent() == rootIndex() && index.column() == d->column)
            viewSelected.append(index);
    }
    return viewSelected;
}

/*!
    \internal

    Layout the items according to the flow and wrapping properties.
*/
void QListView::doItemsLayout()
{
    Q_D(QListView);
    d->prepareItemsLayout();
    if (model() && model()->columnCount(rootIndex()) > 0) { // no columns means no contents
        if (layoutMode() == SinglePass)
            d->doItemsLayout(model()->rowCount(rootIndex())); // layout everything
        else if (!d->batchLayoutTimer.isActive())
            d->batchLayoutTimer.start(0, this); // do a new batch as fast as possible
    }
    QAbstractItemView::doItemsLayout();
}

/*!
  \internal
*/
void QListView::updateGeometries()
{
    Q_D(QListView);
    if (!model()
        || model()->rowCount(rootIndex()) <= 0
        || model()->columnCount(rootIndex()) <= 0) {
        horizontalScrollBar()->setRange(0, 0);
        verticalScrollBar()->setRange(0, 0);
    } else {
        QModelIndex index = model()->index(0, d->column, rootIndex());
        QStyleOptionViewItem option = viewOptions();
        QSize step = d->itemSize(option, index);

        QSize vsize = d->viewport->size();
        QSize max = maximumViewportSize();

        if (max.width() >= d->contentsSize.width() && max.height() >= d->contentsSize.height())
            vsize = max;

        horizontalScrollBar()->setSingleStep(step.width() + d->spacing);
        horizontalScrollBar()->setPageStep(vsize.width());
        horizontalScrollBar()->setRange(0, d->contentsSize.width() - vsize.width());

        verticalScrollBar()->setSingleStep(step.height() + d->spacing);
        verticalScrollBar()->setPageStep(vsize.height());
        verticalScrollBar()->setRange(0, d->contentsSize.height() - vsize.height());
    }
    // if the scrollbars are turned off, we resize the contents to the viewport
    if (d->movement == Static && !d->wrap) {
        if (d->flow == TopToBottom) {
            if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)
                resizeContents(viewport()->width(), contentsSize().height());
        } else { // LeftToRight
            if (verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOff)
                resizeContents(contentsSize().width(), viewport()->height());
        }
    }

    QAbstractItemView::updateGeometries();
}

/*!
  \reimp
*/
bool QListView::isIndexHidden(const QModelIndex &index) const
{
    Q_D(const QListView);
    return (d->hiddenRows.contains(index.row())
            && (index.parent() == rootIndex())
            && index.column() == d->column);
}

/*!
    \property QListView::modelColumn
    \brief the column in the model that is visible
*/
void QListView::setModelColumn(int column)
{
    Q_D(QListView);
    if (column < 0 || !model() || column >= model()->columnCount(rootIndex()))
        return;
    d->column = column;
    d->doDelayedItemsLayout();
}

int QListView::modelColumn() const
{
    Q_D(const QListView);
    return d->column;
}

/*!
    \property QListView::uniformItemSizes
    \brief whether all items in the listview have the same size
    \since 4.1

    This property should only be set to true if it is guaranteed that all items
    in the view have the same size. This enables the view to do some
    optimizations.
*/
void QListView::setUniformItemSizes(bool enable)
{
    Q_D(QListView);
    d->uniformItemSizes = enable;
}

bool QListView::uniformItemSizes() const
{
    Q_D(const QListView);
    return d->uniformItemSizes;
}

/* \reimp */
bool QListView::event(QEvent *e)
{
    return QAbstractItemView::event(e);
}

/*
 * private object implementation
 */

QListViewPrivate::QListViewPrivate()
    : QAbstractItemViewPrivate(),
      layoutMode(QListView::SinglePass),
      modeProperties(0),
      batchStartRow(0),
      batchSavedDeltaSeg(0),
      batchSavedPosition(0),
      column(0),
      uniformItemSizes(false),
      batchSize(100)
{}

void QListViewPrivate::clear()
{
    // initialization of data structs
    batchStartRow = 0;
    batchSavedPosition = 0;
    batchSavedDeltaSeg = 0;
    cachedItemSize = QSize();
    tree.destroy();
    items.clear();
    flowPositions.clear();
    segmentPositions.clear();
    segmentStartRows.clear();
}

void QListViewPrivate::prepareItemsLayout()
{
    Q_Q(QListView);
    clear();
    layoutBounds = viewport->rect();

    if (resizeMode == QListView::Adjust) {
        int verticalMargin = q->style()->pixelMetric(QStyle::PM_ScrollBarExtent, 0, q->verticalScrollBar());
        int dw = q->verticalScrollBar()->isVisible() ? 0 : verticalMargin;
        int horizontalMargin = q->style()->pixelMetric(QStyle::PM_ScrollBarExtent, 0, q->horizontalScrollBar());
        int dh = q->horizontalScrollBar()->isVisible() ? 0 : horizontalMargin;
        layoutBounds.adjust(0, 0, -dw, -dh);
    }

    int rowCount = model ? model->rowCount(root) : 0;
    int colCount = model ? model->columnCount(root) : 0;
    if (colCount <= 0)
        rowCount = 0; // no contents
    if (movement == QListView::Static) {
        flowPositions.resize(rowCount);
    } else {
        tree.create(qMax(rowCount - hiddenRows.count(), 0));
    }
}

QPoint QListViewPrivate::initStaticLayout(const QRect &bounds, int spacing, int first)
{
    int x, y;
    if (first == 0) {
        flowPositions.clear();
        segmentPositions.clear();
        segmentStartRows.clear();
        x = bounds.left() + spacing;
        y = bounds.top() + spacing;
        segmentPositions.append(flow == QListView::LeftToRight ? y : x);
        segmentStartRows.append(0);
    } else if (wrap) {
        if (flow == QListView::LeftToRight) {
            x = batchSavedPosition;
            y = segmentPositions.last();
        } else { // flow == QListView::TopToBottom
            x = segmentPositions.last();
            y = batchSavedPosition;
        }
    } else { // not first and not wrap
        if (flow == QListView::LeftToRight) {
            x = batchSavedPosition;
            y = bounds.top() + spacing;
        } else { // flow == QListView::TopToBottom
            x = bounds.left() + spacing;
            y = batchSavedPosition;
        }
    }
    return QPoint(x, y);
}

QPoint QListViewPrivate::initDynamicLayout(const QRect &bounds, int spacing, int first)
{
    int x, y;
    if (first == 0) {
        x = bounds.x() + spacing;
        y = bounds.y() + spacing;
        items.reserve(model->rowCount(root) - hiddenRows.count());
    } else {
        const QListViewItem item = items.at(first - 1);
        x = item.x;
        y = item.y;
        if (flow == QListView::LeftToRight)
            x += (gridSize.isValid() ? gridSize.width() : item.w) + spacing;
        else
            y += (gridSize.isValid() ? gridSize.height() : item.h) + spacing;
    }
    return QPoint(x, y);
}

void QListViewPrivate::initBspTree(const QSize &contents)
{
    // remove all items from the tree
    int leafCount = tree.leafCount();
    for (int l = 0; l < leafCount; ++l)
        tree.leaf(l).clear();
    // we have to get the bounding rect of the items before we can initialize the tree
    QBspTree::Node::Type type = QBspTree::Node::Both; // 2D
    // simple heuristics to get better bsp
    if (contents.height() / contents.width() >= 3)
        type = QBspTree::Node::HorizontalPlane;
    else if (contents.width() / contents.height() >= 3)
        type = QBspTree::Node::VerticalPlane;
    // build tree for the bounding rect (not just the contents rect)
    tree.init(QRect(0, 0, contents.width(), contents.height()), type);
}

/*!
  \internal
*/
bool QListViewPrivate::doItemsLayout(int delta)
{
    int max = model->rowCount(root) - 1;
    int first = batchStartRow;
    int last = qMin(first + delta - 1, max);

    if (max < 0)
        return true; // nothing to do

    if (movement == QListView::Static) {
        doStaticLayout(layoutBounds, first, last);
    } else {
        if (last >= items.count())
            createItems(last + 1);
        doDynamicLayout(layoutBounds, first, last);
    }

    if (batchStartRow >= max) { // stop items layout
        flowPositions.resize(flowPositions.count());
        segmentPositions.resize(segmentPositions.count());
        segmentStartRows.resize(segmentStartRows.count());
        return true; // done
    }
    return false; // not done
}

/*!
  \internal
*/
void QListViewPrivate::doItemsLayout(const QRect &bounds,
                                     const QModelIndex &first,
                                     const QModelIndex &last)
{
    if (first.row() >= last.row() || !first.isValid() || !last.isValid())
        return;
    if (movement == QListView::Static)
        doStaticLayout(bounds, first.row(), last.row());
    else
        doDynamicLayout(bounds, first.row(), last.row());
}

/*!
  \internal
*/
void QListViewPrivate::doStaticLayout(const QRect &bounds, int first, int last)
{
    Q_Q(QListView);
    const bool useItemSize = !gridSize.isValid();
    const int gap = useItemSize ? spacing : 0; // if we are using a grid ,we don't use spacing
    const QPoint topLeft = initStaticLayout(bounds, gap, first);
    const QStyleOptionViewItem option = q->viewOptions();

    // The static layout data structures are as follows:
    // One vector contains the coordinate in the direction of layout flow.
    // Another vector contains the coordinates of the segments.
    // A third vector contains the index (model row) of the first item
    // of each segment.

    int segStartPosition;
    int segEndPosition;
    int deltaFlowPosition;
    int deltaSegPosition;
    int deltaSegHint;
    int flowPosition;
    int segPosition;

    if (flow == QListView::LeftToRight) {
        segStartPosition = bounds.left();
        segEndPosition = bounds.width();
        flowPosition = topLeft.x();
        segPosition = topLeft.y();
        deltaFlowPosition = gridSize.width(); // dx
        deltaSegPosition = useItemSize ? batchSavedDeltaSeg : gridSize.height(); // dy
        deltaSegHint = gridSize.height();
    } else { // flow == QListView::TopToBottom
        segStartPosition = bounds.top();
        segEndPosition = bounds.height();
        flowPosition = topLeft.y();
        segPosition = topLeft.x();
        deltaFlowPosition = gridSize.height(); // dy
        deltaSegPosition = useItemSize ? batchSavedDeltaSeg : gridSize.width(); // dx
        deltaSegHint = gridSize.width();
    }

    for (int row = first; row <= last; ++row) {
        if (hiddenRows.contains(row)) {
            flowPositions.append(flowPosition);
        } else {
            // if we are not using a grid, we need to find the deltas
            if (useItemSize) {
                QSize hint = itemSize(option, model->index(row, column, root));
                if (flow == QListView::LeftToRight) {
                    deltaFlowPosition = hint.width() + gap;
                    deltaSegHint = hint.height() + gap;
                } else { // TopToBottom
                    deltaFlowPosition = hint.height() + gap;
                    deltaSegHint = hint.width() + gap;
                }
            }
            // create new segment
            if (wrap && (flowPosition + deltaFlowPosition >= segEndPosition)) {
                flowPosition = gap + segStartPosition;
                segPosition += deltaSegPosition;
                segmentPositions.append(segPosition);
                segmentStartRows.append(row);
                deltaSegPosition = 0;
            }
            // save the flow positon of this item
            flowPositions.append(flowPosition);
            // prepare for the next item
            deltaSegPosition = qMax(deltaSegHint, deltaSegPosition);
            flowPosition += gap + deltaFlowPosition;
        }
    }
    // used when laying out next batch
    batchSavedPosition = flowPosition;
    batchSavedDeltaSeg = deltaSegPosition;
    batchStartRow = last + 1;
    // set the contents size
    QRect rect = bounds;
    if (flow == QListView::LeftToRight) {
        rect.setRight(segmentPositions.count() == 1 ? flowPosition : bounds.right());
        rect.setBottom(segPosition + deltaSegPosition);
    } else { // TopToBottom
        rect.setRight(segPosition + deltaSegPosition);
        rect.setBottom(segmentPositions.count() == 1 ? flowPosition : bounds.bottom());
    }
    q->resizeContents(rect.right(), rect.bottom());
    // if the new items are visble, update the viewport
    QRect changedRect(topLeft, rect.bottomRight());
    if (clipRect().intersects(changedRect))
        viewport->update();
}

/*!
  \internal
*/
void QListViewPrivate::doDynamicLayout(const QRect &bounds, int first, int last)
{
    Q_Q(QListView);
    const bool useItemSize = !gridSize.isValid();
    const int gap = useItemSize ? spacing : 0;
    const QPoint topLeft = initDynamicLayout(bounds, spacing, first);

    int segStartPosition;
    int segEndPosition;
    int deltaFlowPosition;
    int deltaSegPosition;
    int deltaSegHint;
    int flowPosition;
    int segPosition;

    if (flow == QListView::LeftToRight) {
        segStartPosition = bounds.left() + spacing;
        segEndPosition = bounds.right();
        deltaFlowPosition = gridSize.width(); // dx
        deltaSegPosition = (useItemSize ? batchSavedDeltaSeg : gridSize.height()); // dy
        deltaSegHint = gridSize.height();
        flowPosition = topLeft.x();
        segPosition = topLeft.y();
    } else { // flow == QListView::TopToBottom
        segStartPosition = bounds.top() + spacing;
        segEndPosition = bounds.bottom();
        deltaFlowPosition = gridSize.height(); // dy
        deltaSegPosition = (useItemSize ? batchSavedDeltaSeg : gridSize.width()); // dx
        deltaSegHint = gridSize.width();
        flowPosition = topLeft.y();
        segPosition = topLeft.x();
    }

    QRect rect(QPoint(0, 0), topLeft);
    QListViewItem *item = 0;
    for (int row = first; row <= last; ++row) {
        item = &items[row];
        if (hiddenRows.contains(row)) {
            item->invalidate();
        } else {
            // if we are not using a grid, we need to find the deltas
            if (useItemSize) {
                if (flow == QListView::LeftToRight)
                    deltaFlowPosition = item->w + gap;
                else
                    deltaFlowPosition = item->h + gap;
            } else {
                item->w = qMin<int>(gridSize.width(), item->w);
                item->h = qMin<int>(gridSize.height(), item->h);
            }
            // create new segment
            if (wrap
                && flowPosition + deltaFlowPosition > segEndPosition
                && flowPosition > segStartPosition) {
                flowPosition = segStartPosition;
                segPosition += deltaSegPosition;
                if (useItemSize)
                    deltaSegPosition = 0;
            }
            // We must delay calculation of the seg adjustment, as this item
            // may have caused a wrap to occur
            if (useItemSize) {
                if (flow == QListView::LeftToRight) {
                    deltaSegHint = item->h + gap;
                } else {
                    deltaSegHint = item->w + gap;
                }
                deltaSegPosition = qMax(deltaSegPosition, deltaSegHint);
            }
            // set the position of the item
            if (flow == QListView::LeftToRight) {
                item->x = flowPosition;
                item->y = segPosition;
            } else { // TopToBottom
                item->y = flowPosition;
                item->x = segPosition;
            }
            rect |= item->rect(); // let the contents contain the new item

            // prepare for next item
            flowPosition += deltaFlowPosition; // current position + item width + gap
        }
    }
    batchSavedDeltaSeg = deltaSegPosition;
    batchStartRow = last + 1;
    bool done = (last >= model->rowCount(root) - 1);
    // resize the content area
    if (done || !bounds.contains(item->rect()))
        q->resizeContents(rect.width(), rect.height());
    // resize tree
    int insertFrom = first;
    if (done || first == 0) {
        initBspTree(rect.size());
        insertFrom = 0;
    }
    // insert items in tree
    for (int row = insertFrom; row <= last; ++row)
        tree.insertLeaf(items.at(row).rect(), row);
    // if the new items are visble, update the viewport
    QRect changedRect(topLeft, rect.bottomRight());
    if (clipRect().intersects(changedRect))
        viewport->update();
}

/*!
  \internal
  Finds the set of items intersecting with \a area.
  In this function, itemsize is counted from topleft to the start of the next item.
*/

void QListViewPrivate::intersectingStaticSet(const QRect &area) const
{
    if (!model)
        return;

    intersectVector.clear();
    int segStartPosition;
    int segEndPosition;
    int flowStartPosition;
    int flowEndPosition;
    if (flow == QListView::LeftToRight) {
        segStartPosition = area.top();
        segEndPosition = area.bottom();
        flowStartPosition = area.left();
        flowEndPosition = area.right();
    } else {
        segStartPosition = area.left();
        segEndPosition = area.right();
        flowStartPosition = area.top();
        flowEndPosition = area.bottom();
    }
    if (segmentPositions.isEmpty() || flowPositions.isEmpty())
        return;
    const int segLast = segmentPositions.count() - 1;
    Q_ASSERT(segLast > -1);
    int seg = qBinarySearch<int>(segmentPositions, segStartPosition, 0, segLast);
    for (; seg <= segLast && segmentPositions.at(seg) <= segEndPosition; ++seg) {
        int first = segmentStartRows.at(seg);
        int last = (seg < segLast ? segmentStartRows.at(seg + 1) : batchStartRow) - 1;
        int row = qBinarySearch<int>(flowPositions, flowStartPosition, first, last);
        for (; row <= last && flowPositions.at(row) <= flowEndPosition; ++row) {
            if (hiddenRows.contains(row))
                continue;
            QModelIndex index = model->index(row, column, root);
            if (index.isValid())
                intersectVector.append(index);
            else
                qWarning("intersectingStaticSet: row %d was invalid", row);
        }
    }
}

void QListViewPrivate::intersectingDynamicSet(const QRect &area) const
{
    intersectVector.clear();
    QListViewPrivate *that = const_cast<QListViewPrivate*>(this);
    QBspTree::Data data(static_cast<void*>(that));
    that->tree.climbTree(area, &QListViewPrivate::addLeaf, data);
}

void QListViewPrivate::createItems(int to)
{
    Q_Q(QListView);
    int count = items.count();
    QSize size;
    QStyleOptionViewItem option = q->viewOptions();
    QModelIndex root = q->rootIndex();
    for (int row = count; row < to; ++row) {
        size = itemSize(option, model->index(row, column, root));
        QListViewItem item(QRect(0, 0, size.width(), size.height()), row); // default pos
	items.append(item);
    }
}

void QListViewPrivate::drawItems(QPainter *painter, const QVector<QModelIndex> &indexes) const
{
    Q_Q(const QListView);
    QStyleOptionViewItem option = q->viewOptions();
    option.state &= ~QStyle::State_MouseOver;
    QVector<QModelIndex>::const_iterator it = indexes.begin();
    QListViewItem item = indexToListViewItem(*it);
    for (; it != indexes.end(); ++it) {
        item = indexToListViewItem(*it);
        option.rect = viewItemRect(item);
        delegate->paint(painter, option, *it);
    }
}

QRect QListViewPrivate::itemsRect(const QVector<QModelIndex> &indexes) const
{
    QVector<QModelIndex>::const_iterator it = indexes.begin();
    QListViewItem item = indexToListViewItem(*it);
    QRect rect(item.x, item.y, item.w, item.h);
    for (; it != indexes.end(); ++it) {
        item = indexToListViewItem(*it);
        rect |= viewItemRect(item);
    }
    return rect;
}

QListViewItem QListViewPrivate::indexToListViewItem(const QModelIndex &index) const
{
    Q_Q(const QListView);
    if (!index.isValid() || hiddenRows.contains(index.row()))
        return QListViewItem();

    if (movement != QListView::Static)
        if (index.row() < items.count())
            return items.at(index.row());
        else
            return QListViewItem();

    // movement == Static
    if (flowPositions.isEmpty() || segmentPositions.isEmpty())
        return QListViewItem();

    if (index.row() >= flowPositions.count())
        return QListViewItem();

    int l = segmentStartRows.count() - 1;
    int s = qBinarySearch<int>(segmentStartRows, index.row(), 0, l);
    QPoint pos;
    if (flow == QListView::LeftToRight) {
        pos.setX(flowPositions.at(index.row()));
        pos.setY(segmentPositions.at(s));
    } else { // TopToBottom
        pos.setY(flowPositions.at(index.row()));
        pos.setX(segmentPositions.at(s));
    }

    QSize size = itemSize(q->viewOptions(), index);
    return QListViewItem(QRect(pos, size), index.row());
}

int QListViewPrivate::itemIndex(const QListViewItem &item) const
{
    int i = item.indexHint;
    if (movement == QListView::Static || i >= items.count() || items.at(i) == item)
        return i;

    int j = i;
    int c = items.count();
    bool a = true;
    bool b = true;

    while (a || b) {
        if (a) {
            if (items.at(i) == item) {
                items.at(i).indexHint = i;
                return i;
            }
            a = ++i < c;
        }
        if (b) {
            if (items.at(j) == item) {
                items.at(j).indexHint = j;
                return j;
            }
            b = --j > -1;
        }
    }
    return -1;
}

void QListViewPrivate::addLeaf(QVector<int> &leaf, const QRect &area,
                               uint visited, QBspTree::Data data)
{
    QListViewItem *vi;
    QListViewPrivate *_this = static_cast<QListViewPrivate *>(data.ptr);
    for (int i = 0; i < leaf.count(); ++i) {
        int idx = leaf.at(i);
        if (idx < 0)
            continue;
        vi = &_this->items[idx];
        Q_ASSERT(vi);
        if (vi->rect().intersects(area) && vi->visited != visited) {
            QModelIndex index = _this->listViewItemToIndex(*vi);
            Q_ASSERT(index.isValid());
            _this->intersectVector.append(index);
            vi->visited = visited;
        }
    }
}

void QListViewPrivate::insertItem(int index, QListViewItem &item)
{
    items.insert(index + 1, 1, item); // insert after idx
    tree.insertLeaf(item.rect(), index);
}

void QListViewPrivate::removeItem(int index)
{
    tree.removeLeaf(items.at(index).rect(), index);
    items.remove(index, 1);
}

void QListViewPrivate::moveItem(int index, const QPoint &dest)
{
    Q_Q(QListView);

    // does not impact on the bintree itself or the contents rect
    QListViewItem *item = &items[index];
    QRect rect = item->rect();

    // move the item without removing it from the tree
    tree.removeLeaf(rect, index);
    item->x = dest.x();
    item->y = dest.y();
    tree.insertLeaf(QRect(dest, rect.size()), index);

    // resize the contents area
    int w = rect.x() + rect.width();
    int h = rect.y() + rect.height();
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
    Q_Q(const QListView);
    if (!rect.isValid())
        return rect;
    QRect result = rect;

    // If the listview is in "listbox-mode", the items are as wide as the view.
    if (!wrap && movement == QListView::Static && flow == QListView::TopToBottom) {
        if (q_func()->isRightToLeft()) {
            // Adjust the rect by expanding the left edge
            result.setLeft(result.right() - qMax(contentsSize.width(), viewport->width()));
        } else {
            // Adjust the rect by expanding the right edge
            result.setWidth(qMax(contentsSize.width(), viewport->width()));
        }
    }

    int dx = -q->horizontalOffset();
    int dy = -q->verticalOffset();
    result.adjust(dx, dy, dx, dy);
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
    rect.translate(draggedItemsDelta());
    return rect;
}

QModelIndex QListViewPrivate::closestIndex(const QPoint &target,
                                           const QVector<QModelIndex> &candidates) const
{
    int distance = 0;
    int shortest = -1;
    QModelIndex closest;
    QVector<QModelIndex>::const_iterator it = candidates.begin();
    for (; it != candidates.end(); ++it) {
        if (!(*it).isValid())
            continue;
        distance = (indexToListViewItem(*it).rect().center() - target).manhattanLength();
        if (distance < shortest || shortest == -1) {
            shortest = distance;
            closest = *it;
        }
    }
    return closest;
}

QSize QListViewPrivate::itemSize(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!uniformItemSizes)
        return delegate->sizeHint(option, index);
    if (!cachedItemSize.isValid()) { // the last item is probaly the largest, so we use it's size
        int row = model->rowCount(root) - 1;
        QModelIndex sample = model->index(row, column, root);
        cachedItemSize = delegate->sizeHint(option, sample);
    }
    return cachedItemSize;
}

#endif // QT_NO_LISTVIEW
