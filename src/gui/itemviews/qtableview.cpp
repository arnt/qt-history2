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

#include "qtableview.h"

#ifndef QT_NO_TABLEVIEW
#include <qheaderview.h>
#include <qitemdelegate.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qsize.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <private/qtableview_p.h>

void QTableViewPrivate::init()
{
    Q_Q(QTableView);

    q->setEditTriggers(editTriggers|QAbstractItemView::AnyKeyPressed);

    QHeaderView *vertical = new QHeaderView(Qt::Vertical, q);
    vertical->setClickable(true);
    vertical->setHighlightSections(true);
    q->setVerticalHeader(vertical);

    QHeaderView *horizontal = new QHeaderView(Qt::Horizontal, q);
    horizontal->setClickable(true);
    horizontal->setHighlightSections(true);
    q->setHorizontalHeader(horizontal);

    tabKeyNavigation = true;
}

void QTableViewPrivate::updateVerticalScrollbar()
{
    Q_Q(QTableView);

    int height = viewport->height();
    int count = verticalHeader->count();

    // if the header is out of sync we have to relayout
    if (count != model->rowCount(root)) {
        verticalHeader->doItemsLayout();
        count = verticalHeader->count();
    }

    // if we have no viewport or no rows, there is nothing to do
    if (height <= 0 || count <= 0) {
        q->verticalScrollBar()->setRange(0, 0);
        return;
    }

    // set the scroller range
    int y = height;
    while (y > 0 && count > 0)
        y -= verticalHeader->sectionSize(--count);
    int max = count * verticalStepsPerItem;

    // iterate all hidden sections compensate *max* for that.
    for (int s = 0; s < verticalHeader->count(); ++s)
        if (verticalHeader->isSectionHidden(s))
            max -= verticalStepsPerItem;

    // set page step size
    int visibleCount = verticalHeader->count() - count - 1;
    q->verticalScrollBar()->setPageStep(visibleCount * verticalStepsPerItem);

    if (y < 0) { // if the last item starts above the viewport, we have to backtrack
        int sectionSize = verticalHeader->sectionSize(count);
        if (sectionSize) // avoid division by zero
            max += ((-y * verticalStepsPerItem)/ sectionSize) + 1; // how many units to add
    }

    q->verticalScrollBar()->setRange(0, max);

    if (q->verticalScrollBar()->value() == max) {
        int newOffset = 0;
        int oldOffset = verticalHeader->offset();
        if (verticalHeader->length() >= viewport->height())
            newOffset = verticalHeader->length() - viewport->height() - 1;
        verticalHeader->setOffset(newOffset);
        scrollContentsBy(0, oldOffset - newOffset);
    }
}

void QTableViewPrivate::updateHorizontalScrollbar()
{
    Q_Q(QTableView);

    int width = viewport->width();
    int count = horizontalHeader->count();

    // if the header is out of sync we have to relayout
    if (count != model->columnCount(root)) {
        horizontalHeader->doItemsLayout();
        count = horizontalHeader->count();
    }

    // if we have no viewport or no columns, there is nothing to do
    if (width <= 0 || count <= 0) {
        q->horizontalScrollBar()->setRange(0, 0);
        return;
    }

    // set the scroller range
    int x = width;
    while (x > 0 && count > 0)
        x -= horizontalHeader->sectionSize(--count);
    int max = count * horizontalStepsPerItem;

    // iterate all hidden sections compensate *max* for that.
    for (int s = 0; s < horizontalHeader->count(); ++s)
        if (horizontalHeader->isSectionHidden(s))
            max -= horizontalStepsPerItem;

    // set page step size
    int visibleCount = horizontalHeader->count() - count - 1;
    q->horizontalScrollBar()->setPageStep(visibleCount * horizontalStepsPerItem);

    if (x < 0) { // if the last item starts left of the viewport, we have to backtrack
        int sectionSize = horizontalHeader->sectionSize(count);
        if (sectionSize) // avoid division by zero
            max += ((-x * horizontalStepsPerItem) / sectionSize) + 1;
    }

    q->horizontalScrollBar()->setRange(0, max);

    if (q->horizontalScrollBar()->value() == max) {
        int newOffset = 0;
        int oldOffset = horizontalHeader->offset();
        if (horizontalHeader->length() >= viewport->width())
            newOffset = horizontalHeader->length() - viewport->width() - 1;
        horizontalHeader->setOffset(newOffset);
        scrollContentsBy(oldOffset - newOffset, 0);
    }
}

/*
 * Trims away indices that are hidden in the treeview due to hidden horizontal or vertical sections.
 *
 */
void QTableViewPrivate::trimHiddenSelections(QItemSelectionRange *range) const
{
    Q_ASSERT(range);
    Q_ASSERT(range->topLeft().isValid() && range->bottomRight().isValid());

    QModelIndex indexTopLeft = range->topLeft();
    int tlrow = indexTopLeft.row();
    int tlcol = indexTopLeft.column();
    QModelIndex indexBottomRight = range->bottomRight();
    int brrow = indexBottomRight.row();
    int brcol = indexBottomRight.column();

    while (verticalHeader->isSectionHidden(brrow) && brrow >= tlrow) brrow--;
    while (horizontalHeader->isSectionHidden(brcol) && brcol >= tlcol) brcol--;
    if (tlrow > brrow || tlcol > brcol) {
        // return an invalid one if *all* is hidden
        *range = QItemSelectionRange();
        return;
    }
    indexBottomRight = model->index(brrow, brcol, indexBottomRight.parent());

    while (verticalHeader->isSectionHidden(tlrow) && tlrow <= brrow) tlrow++;
    while (horizontalHeader->isSectionHidden(tlcol) && tlcol <= brcol) tlcol++;
    if (tlrow > brrow || tlcol > brcol) {
        // return an invalid one if *all* is hidden
        *range = QItemSelectionRange();
        return;
    }
    indexTopLeft = model->index(tlrow, tlcol, indexTopLeft.parent());

    *range = QItemSelectionRange(indexTopLeft, indexBottomRight);
}


/*!
    \class QTableView qtableview.h

    \brief The QTableView class provides a default model/view
    implementation of a table view.

    \ingroup model-view
    \mainclass

    A QTableView implements a table view that displays items from a
    model. This class is used to provide standard tables that were
    previously provided by the QTable class, but using the more
    flexible approach provided by Qt's model/view architecture.

    The QTableView class is one of the \l{Model/View Classes}
    and is part of Qt's \l{Model/View Programming}{model/view framework}.

    QTableView implements the interfaces defined by the
    QAbstractItemView class to allow it to display data provided by
    models derived from the QAbstractItemModel class.

    The table has a vertical header that can be obtained using the
    verticalHeader() function, and a horizontal header that is available
    through the horizontalHeader() function. Each of the rows in the
    table can be found by using rowHeight(); similarly, the width of
    columns can be found using columnWidth().

    Rows and columns can be hidden and shown with hideRow(), hideColumn(),
    showRow(), and showColumn(). They can be selected with selectRow()
    and selectColumn(). The table will show a grid depending on the
    \l showGrid property.

    For some specialized forms of tables it is useful to be able to
    convert between row and column indexes and widget coordinates.
    The rowAt() function provides the y-coordinate within the view of the
    specified row; the row index can be used to obtain a corresponding
    y-coordinate with rowViewportPosition(). The columnAt() and
    columnViewportPosition() functions provide the equivalent conversion
    operations between x-coordinates and column indexes.

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemModel QAbstractItemView

*/

/*!
    Constructs a table view with a \a parent to represent the data.

    \sa QAbstractItemModel
*/

QTableView::QTableView(QWidget *parent)
    : QAbstractItemView(*new QTableViewPrivate, parent)
{
    d_func()->init();
}

/*!
  \internal
*/
QTableView::QTableView(QTableViewPrivate &dd, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    d_func()->init();
}

/*!
  Destroys the table view.
*/
QTableView::~QTableView()
{
}

/*!
  \reimp
*/
void QTableView::setModel(QAbstractItemModel *model)
{
    Q_D(QTableView);

    if (d->selectionModel && d->model) // support row editing
        disconnect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                   d->model, SLOT(submit()));

    d->verticalHeader->setModel(model);
    d->horizontalHeader->setModel(model);
    QAbstractItemView::setModel(model);
}

/*!
  \reimp
*/
void QTableView::setRootIndex(const QModelIndex &index)
{
    Q_D(QTableView);

    d->verticalHeader->setRootIndex(index);
    d->horizontalHeader->setRootIndex(index);
    QAbstractItemView::setRootIndex(index);
}

/*!
  \reimp
*/
void QTableView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    Q_D(QTableView);

    Q_ASSERT(selectionModel);
    if (d->model && d->selectionModel) // support row editing
        disconnect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                   d->model, SLOT(submit()));

    d->verticalHeader->setSelectionModel(selectionModel);
    d->horizontalHeader->setSelectionModel(selectionModel);
    QAbstractItemView::setSelectionModel(selectionModel);

    if (d->model && d->selectionModel) // support row editing
        connect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                d->model, SLOT(submit()));
}

/*!
    Returns the table view's horizontal header.

    \sa setHorizontalHeader() verticalHeader()
*/

QHeaderView *QTableView::horizontalHeader() const
{
    return d_func()->horizontalHeader;
}

/*!
    Returns the table view's vertical header.

    \sa setVerticalHeader() horizontalHeader()
*/
QHeaderView *QTableView::verticalHeader() const
{
    return d_func()->verticalHeader;
}

/*!
    Sets the widget to use for the vertical header to \a header.

    \sa horizontalHeader() setVerticalHeader()
*/
void QTableView::setHorizontalHeader(QHeaderView *header)
{
    Q_D(QTableView);

    Q_ASSERT(header);
    delete d->horizontalHeader;
    d->horizontalHeader = header;
    if (!d->horizontalHeader->model())
        d->horizontalHeader->setModel(model());

    connect(d->horizontalHeader,SIGNAL(sectionResized(int,int,int)),
            this, SLOT(columnResized(int,int,int)), Qt::QueuedConnection);
    connect(d->horizontalHeader, SIGNAL(sectionMoved(int,int,int)),
            this, SLOT(columnMoved(int,int,int)), Qt::QueuedConnection);
    connect(d->horizontalHeader, SIGNAL(sectionCountChanged(int,int)),
            this, SLOT(columnCountChanged(int,int)), Qt::QueuedConnection);
    connect(d->horizontalHeader, SIGNAL(sectionPressed(int)), this, SLOT(selectColumn(int)));
    connect(d->horizontalHeader, SIGNAL(sectionHandleDoubleClicked(int)),
            this, SLOT(resizeColumnToContents(int)));
    d->horizontalHeader->setFocusProxy(this);
}

/*!
    Sets the widget to use for the horizontal header to \a header.

    \sa verticalHeader() setHorizontalHeader()
*/
void QTableView::setVerticalHeader(QHeaderView *header)
{
    Q_D(QTableView);

    Q_ASSERT(header);
    delete d->verticalHeader;
    d->verticalHeader = header;
    if (!d->verticalHeader->model())
        d->verticalHeader->setModel(model());

    connect(d->verticalHeader, SIGNAL(sectionResized(int,int,int)),
            this, SLOT(rowResized(int,int,int)), Qt::QueuedConnection);
    connect(d->verticalHeader, SIGNAL(sectionMoved(int,int,int)),
            this, SLOT(rowMoved(int,int,int)), Qt::QueuedConnection);
    connect(d->verticalHeader, SIGNAL(sectionCountChanged(int,int)),
            this, SLOT(rowCountChanged(int,int)), Qt::QueuedConnection);
    connect(d->verticalHeader, SIGNAL(sectionPressed(int)), this, SLOT(selectRow(int)));
    connect(d->verticalHeader, SIGNAL(sectionHandleDoubleClicked(int)),
            this, SLOT(resizeRowToContents(int)));
    d->verticalHeader->setFocusProxy(this);
}

/*!
    \internal

    Scroll the contents of the table view by \a(dx, dy).
*/
void QTableView::scrollContentsBy(int dx, int dy)
{
    Q_D(QTableView);

    if (dx) { // horizontal
        int value = horizontalScrollBar()->value();
        int section = d->horizontalHeader->logicalIndex(value / horizontalStepsPerItem());
        while (d->horizontalHeader->isSectionHidden(section)) // hidden sections to the left
            ++section;
        int steps = horizontalStepsPerItem();
        int left = (value % steps) * d->horizontalHeader->sectionSize(section);
        int offset = (left / steps) + d->horizontalHeader->sectionPosition(section);
        if (isRightToLeft())
            dx = offset - d->horizontalHeader->offset();
        else
            dx = d->horizontalHeader->offset() - offset;
        d->horizontalHeader->setOffset(offset);
    }

    if (dy) { // vertical
        int value = verticalScrollBar()->value();
        int section = d->verticalHeader->logicalIndex(value / verticalStepsPerItem());
        while (d->verticalHeader->isSectionHidden(section)) // hidden sections above
            ++section;
        int steps = verticalStepsPerItem();
        int above = (value % steps) * d->verticalHeader->sectionSize(section);
        int offset = (above / steps) + d->verticalHeader->sectionPosition(section);
        dy = d->verticalHeader->offset() - offset;
        d->verticalHeader->setOffset(offset);
    }

    d->scrollContentsBy(dx, dy);
}


/*!
  \reimp
*/
QStyleOptionViewItem QTableView::viewOptions() const
{
    QStyleOptionViewItem option = QAbstractItemView::viewOptions();
    option.showDecorationSelected = true;
    return option;
}

/*!
    Paints the table on receipt of the given paint event \a e.
*/
void QTableView::paintEvent(QPaintEvent *e)
{
    Q_D(QTableView);

    // setup temp variables for the painting

    QStyleOptionViewItem option = viewOptions();
    const QPoint offset = d->scrollDelayOffset;
    const bool showGrid = d->showGrid;
    const int gridSize = showGrid ? 1 : 0;
    const int gridHint = style()->styleHint(QStyle::SH_Table_GridLineColor, &option, this);
    const QColor gridColor = static_cast<QRgb>(gridHint);
    const QPen gridPen = QPen(gridColor, 0, d->gridStyle);
    const QItemSelectionModel *sels = selectionModel();
    const QHeaderView *verticalHeader = d->verticalHeader;
    const QHeaderView *horizontalHeader = d->horizontalHeader;
    const QModelIndex current = currentIndex();
    const QModelIndex hover = d->hover;
    const bool focus = (hasFocus() || d->viewport->hasFocus()) && current.isValid();
    const QStyle::State state = option.state;
    const bool alternate = d->alternatingColors;

    QPainter painter(d->viewport);

    // if there's nothing to do, clear the area and return
    if (d->horizontalHeader->count() == 0 || d->verticalHeader->count() == 0) {
        painter.fillRect(e->rect(), option.palette.brush(QPalette::Base));
        return;
    }

    QVector<QRect> rects = e->region().rects();
    for (int i = 0; i < rects.size(); ++i) {

        QRect area = rects.at(i);
        area.translate(offset);

        // get the horizontal start and end sections (visual indexes)
        int left = d->horizontalHeader->visualIndexAt(area.left());
        int right = d->horizontalHeader->visualIndexAt(area.right());

        if (isRightToLeft()) {
            left = (left == -1 ? model()->columnCount(rootIndex()) - 1 : left);
            right = (right == -1 ? 0 : right);
        } else {
            left = (left == -1 ? 0 : left);
            right = (right == -1 ? model()->columnCount(rootIndex()) - 1 : right);
        }

        int tmp = left;
        left = qMin(left, right);
        right = qMax(tmp, right);

        // get the vertical start and end sections (visual indexes)
        int top = d->verticalHeader->visualIndexAt(area.top());
        int bottom = d->verticalHeader->visualIndexAt(area.bottom());

        top = (top == -1 ? 0 : top);
        bottom = (bottom == -1 ? d->model->rowCount(rootIndex()) - 1 : bottom);

        tmp = top;
        top = qMin(top, bottom);
        bottom = qMax(tmp, bottom);

        // Find the initial value for bPaintAlternateBase
        bool bPaintAlternateBase = false;   // should only be inverted for *visible* rows
        if (alternate) {
            for (int v1 = 0; v1 < top; ++v1) {
                int row = verticalHeader->logicalIndex(v1);
                if (verticalHeader->isSectionHidden(row))
                    continue;
                bPaintAlternateBase = !bPaintAlternateBase;
            }
        }
        // do the actual painting

        for (int v = top; v <= bottom; ++v) {
            int row = verticalHeader->logicalIndex(v);
            if (verticalHeader->isSectionHidden(row))
                continue;
            int rowp = rowViewportPosition(row) + offset.y();
            int rowh = rowHeight(row) - gridSize;
            for (int h = left; h <= right; ++h) {
                int col = horizontalHeader->logicalIndex(h);
                if (horizontalHeader->isSectionHidden(col))
                    continue;
                int colp = columnViewportPosition(col) + offset.x();
                int colw = columnWidth(col) - gridSize;
                QModelIndex index = model()->index(row, col, rootIndex());
                if (index.isValid()) {
                    option.rect = QRect(colp, rowp, colw, rowh);
                    option.state = state;
                    if (sels && sels->isSelected(index))
                        option.state |= QStyle::State_Selected;
                    if (index == hover)
                        option.state |= QStyle::State_MouseOver;
                    else
                        option.state &= ~QStyle::State_MouseOver;
                    QPalette::ColorGroup cg;
                    if ((model()->flags(index) & Qt::ItemIsEnabled) == 0) {
                        option.state &= ~QStyle::State_Enabled;
                        cg = QPalette::Disabled;
                    } else {
                        cg = QPalette::Normal;
                    }
                    if (focus && index == current)
                        option.state |= QStyle::State_HasFocus;
                    if (alternate && bPaintAlternateBase)
                        painter.fillRect(colp, rowp, colw, rowh,
                                         option.palette.brush(cg, QPalette::AlternateBase));
                    else
                        painter.fillRect(colp, rowp, colw, rowh,
                                         option.palette.brush(QPalette::Base));
                    itemDelegate()->paint(&painter, option, index);
                }
                if (v == top && showGrid) {
                    QPen old = painter.pen();
                    painter.setPen(gridPen);
                    painter.drawLine(colp + colw, area.top(), colp + colw, area.bottom()+1);
                    painter.setPen(old);
                }
            }
            if (showGrid) {
                QPen old = painter.pen();
                painter.setPen(gridPen);
                painter.drawLine(area.left(), rowp + rowh, area.right(), rowp + rowh);
                painter.setPen(old);
            }
            bPaintAlternateBase = !bPaintAlternateBase;            
        }

        int w = d->viewport->width();
        int h = d->viewport->height();
        int x = d->horizontalHeader->length();
        int y = d->verticalHeader->length();
        QRect b(0, y, w, h - y);
        if (y < h && area.intersects(b))
            painter.fillRect(b, option.palette.brush(QPalette::Base));
        if (isRightToLeft()) {
            QRect r(0, 0, w - x, h);
            if (x > 0 && area.intersects(r))
                painter.fillRect(r, option.palette.brush(QPalette::Base));
        } else {
            QRect l(x, 0, w - x, h);
            if (x < w && area.intersects(l))
                painter.fillRect(l, option.palette.brush(QPalette::Base));
        }
    }
    
    // Paint the dropIndicator
    d_func()->paintDropIndicator(&painter);
}

/*!
    Returns the index position of the model item corresponding to the
    table item at position \a pos in contents coordinates.
*/
QModelIndex QTableView::indexAt(const QPoint &pos) const
{
    d_func()->executePostedLayout();
    int r = rowAt(pos.y());
    int c = columnAt(pos.x());
    if (r >= 0 && c >= 0)
        return model()->index(r, c, rootIndex());
    return QModelIndex();
}

/*!
    Returns the horizontal offset of the items in the table view.

    \sa verticalOffset()
*/
int QTableView::horizontalOffset() const
{
    return d_func()->horizontalHeader->offset();
}

/*!
    Returns the vertical offset of the items in the table view.

    \sa horizontalOffset()
*/
int QTableView::verticalOffset() const
{
    return d_func()->verticalHeader->offset();
}

/*!
    \fn QModelIndex QTableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)

    Moves the cursor in accordance with the given \a cursorAction, using the
    information provided by the \a modifiers.

    \sa QAbstractItemView::CursorAction
*/
QModelIndex QTableView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    Q_D(QTableView);
    Q_UNUSED(modifiers);

    if (!model())
        return QModelIndex();

    int bottom = model()->rowCount(rootIndex()) - 1;
    // make sure that bottom is the bottommost *visible* row
    while (isRowHidden(bottom) && bottom >= 0) --bottom;

    int right = model()->columnCount(rootIndex()) - 1;
    // make sure that right is the rightmost *visible* column
    while (isColumnHidden(right) && right >= 0) --right;

    if (bottom == -1 || right == -1)
        return QModelIndex(); // model is empty

    QModelIndex current = currentIndex();

    if (!current.isValid()) {
        int row = 0;
        int column = 0;
        while (isColumnHidden(column) && column < right)
            ++column;
        while (isRowHidden(row) && row < bottom)
            ++row;
        return model()->index(row, column, rootIndex());
    }

    int visualRow = verticalHeader()->visualIndex(current.row());
    int visualColumn = horizontalHeader()->visualIndex(current.column());

    if (isRightToLeft()) {
        if (cursorAction == MoveLeft)
            cursorAction = MoveRight;
        else if (cursorAction == MoveRight)
            cursorAction = MoveLeft;
    }

    switch (cursorAction) {
    case MoveUp:
        --visualRow;
        while (visualRow > 0 && isRowHidden(verticalHeader()->logicalIndex(visualRow)))
            --visualRow;
        break;
    case MoveDown:
        ++visualRow;
        while (visualRow < bottom && isRowHidden(verticalHeader()->logicalIndex(visualRow)))
            ++visualRow;
        break;
    case MovePrevious:
        {
            int left = 0;
            // make sure that left is the leftmost *visible* column
            while (isColumnHidden(left) && left < right) left++;

            if (visualColumn == left) {
                // wrap up to the last column on the previous row
                visualColumn = right;

                int visualtop = 0;
                while (visualtop < bottom && isRowHidden(verticalHeader()->logicalIndex(visualtop)))
                    ++visualtop;

                // check if we are at top we also need to wrap down to the bottom
                if (visualRow == visualtop) {
                    visualRow = bottom;
                } else {
                    --visualRow;
                }
                while (visualRow > 0 && isRowHidden(verticalHeader()->logicalIndex(visualRow)))
                    --visualRow;
                break;
            }
        }
    case MoveLeft:
        --visualColumn;
        while (visualColumn < right && isColumnHidden(horizontalHeader()->logicalIndex(visualColumn)))
            --visualColumn;
        break;
    case MoveNext:
        if (visualColumn == right) {
            // wrap down to the next line
            visualColumn = 0;
            while (visualColumn < right && isColumnHidden(horizontalHeader()->logicalIndex(visualColumn)))
                ++visualColumn;

            if (visualRow == bottom) {
                // check if we are at the bottom, rightmost cell, then wrap to the top left
                visualRow = 0;
            } else {
                ++visualRow;
            }
            while (visualRow < bottom && isRowHidden(verticalHeader()->logicalIndex(visualRow)))
                ++visualRow;
            break;
        }
    case MoveRight:
        ++visualColumn;
        while (visualColumn < right && isColumnHidden(horizontalHeader()->logicalIndex(visualColumn)))
            ++visualColumn;
        break;
    case MoveHome:
        visualColumn = 0;
        while (visualColumn < right && isColumnHidden(horizontalHeader()->logicalIndex(visualColumn)))
            ++visualColumn;
        if (modifiers & Qt::ControlModifier) {
            visualRow = 0;
            while (visualRow < bottom && isRowHidden(verticalHeader()->logicalIndex(visualRow)))
                ++visualRow;
        }
        break;
    case MoveEnd:
        visualColumn = right;
        if (modifiers & Qt::ControlModifier) {
            visualRow = bottom;
        }
        break;
    case MovePageUp: {
        int newRow = rowAt(visualRect(current).top() - d->viewport->height());
        return model()->index(newRow <= bottom ? newRow : 0, current.column(), rootIndex());}
    case MovePageDown: {
        int newRow = rowAt(visualRect(current).bottom() + d->viewport->height());
        return model()->index(newRow >= 0 ? newRow : bottom, current.column(), rootIndex());}
    }

    int logicalRow = verticalHeader()->logicalIndex(visualRow);
    int logicalColumn = horizontalHeader()->logicalIndex(visualColumn);
    if (!model()->hasIndex(logicalRow, logicalColumn, rootIndex()))
        return QModelIndex();
    QModelIndex result = model()->index(logicalRow, logicalColumn, rootIndex());
    if (!isIndexHidden(result))
        return model()->index(logicalRow, logicalColumn, rootIndex());
    return QModelIndex();
}

/*!
    \fn void QTableView::setSelection(const QRect &rect,
    QItemSelectionModel::SelectionFlags flags)

    Selects the items within the given \a rect and in accordance with
    the specified selection \a flags.
*/
void QTableView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    if (!selectionModel())
        return;
    QModelIndex tl = indexAt(QPoint(isRightToLeft() ? rect.right() : rect.left(), rect.top()));
    QModelIndex br = indexAt(QPoint(isRightToLeft() ? rect.left() : rect.right(), rect.bottom()));
    selectionModel()->select(QItemSelection(tl, br), command);
}

/*!
    \internal

    Returns the rectangle from the viewport of the items in the given
    \a selection.
*/
QRegion QTableView::visualRegionForSelection(const QItemSelection &selection) const
{
    Q_D(const QTableView);

    if (selection.isEmpty())
        return QRegion();

    QRegion selectionRegion;
    bool verticalMoved = verticalHeader()->sectionsMoved();
    bool horizontalMoved = horizontalHeader()->sectionsMoved();

    if (verticalMoved && horizontalMoved) {
        for (int i = 0; i < selection.count(); ++i) {
            QItemSelectionRange range = selection.at(i);
            if (range.parent() != rootIndex() || !range.isValid())
                continue;
            for (int r = range.top(); r <= range.bottom(); ++r)
                for (int c = range.left(); c <= range.right(); ++c)
                    selectionRegion += QRegion(visualRect(d->model->index(r, c, rootIndex())));
        }
    } else if (horizontalMoved) {
        for (int i = 0; i < selection.count(); ++i) {
            QItemSelectionRange range = selection.at(i);
            if (range.parent() != rootIndex() || !range.isValid())
                continue;
            int top = rowViewportPosition(range.top());
            int bottom = rowViewportPosition(range.bottom()) + rowHeight(range.bottom());
            if (top > bottom)
                qSwap<int>(top, bottom);
            int height = bottom - top;
            for (int c = range.left(); c <= range.right(); ++c)
                selectionRegion += QRegion(QRect(columnViewportPosition(c), top,
                                                 columnWidth(c), height));
        }
    } else if (verticalMoved) {
        for (int i = 0; i < selection.count(); ++i) {
            QItemSelectionRange range = selection.at(i);
            if (range.parent() != rootIndex() || !range.isValid())
                continue;
            int left = columnViewportPosition(range.left());
            int right = columnViewportPosition(range.right()) + columnWidth(range.right());
            if (left > right)
                qSwap<int>(left, right);
            int width = right - left;
            for (int r = range.top(); r <= range.bottom(); ++r)
                selectionRegion += QRegion(QRect(left, rowViewportPosition(r),
                                                 width, rowHeight(r)));
        }
    } else { // nothing moved
        for (int i = 0; i < selection.count(); ++i) {
            QItemSelectionRange range = selection.at(i);
            if (range.parent() != rootIndex() || !range.isValid())
                continue;
            d->trimHiddenSelections(&range);
            QRect tl = visualRect(range.topLeft());
            QRect br = visualRect(range.bottomRight());
            selectionRegion += QRegion(tl|br);
        }
    }

    return selectionRegion;
}


/*!
  \reimp
*/
QModelIndexList QTableView::selectedIndexes() const
{
    QModelIndexList viewSelected;
    QModelIndexList modelSelected;
    if (selectionModel())
        modelSelected = selectionModel()->selectedIndexes();
    for (int i=0; i<modelSelected.count(); ++i) {
        QModelIndex index = modelSelected.at(i);
        if (!isIndexHidden(index) && index.parent() == rootIndex())
            viewSelected.append(index);
    }
    return viewSelected;
}


/*!
    This slot is called whenever rows are added or deleted. The
    previous number of rows is specified by \a oldCount, and the new
    number of rows is specified by \a newCount.
*/
void QTableView::rowCountChanged(int /*oldCount*/, int /*newCount*/ )
{
    updateGeometries();
    d_func()->viewport->update();
}

/*!
    This slot is called whenever columns are added or deleted. The
    previous number of columns is specified by \a oldCount, and the new
    number of columns is specified by \a newCount.
*/
void QTableView::columnCountChanged(int, int)
{
    updateGeometries();
    d_func()->viewport->update();
}

/*!
    \internal
*/
void QTableView::updateGeometries()
{
    Q_D(QTableView);

    int width = !d->verticalHeader->isHidden() ? d->verticalHeader->sizeHint().width() : 0;
    int height = !d->horizontalHeader->isHidden() ? d->horizontalHeader->sizeHint().height() : 0;
    bool reverse = isRightToLeft();
    setViewportMargins(reverse ? 0 : width, height, reverse ? width : 0, 0);

    QRect vg = d->viewport->geometry();

    int verticalLeft = reverse ? vg.right() : (vg.left() - width);
    d->verticalHeader->setGeometry(verticalLeft, vg.top(), width, vg.height());

    int horizontalTop = vg.top() - height;
    d->horizontalHeader->setGeometry(vg.left(), horizontalTop, vg.width(), height);

    if (d->model) {
        d->updateVerticalScrollbar();
        d->updateHorizontalScrollbar();
    }

    QAbstractItemView::updateGeometries();
}

/*!
    Returns the size hint for the given \a row's height or -1 if there
    is no model.

    If you need to set the height of a given row to a fixed value, call
    QHeaderView::resizeSection() on the table's vertical header.

    If you reimplement this function in a subclass, note that the value you
    return is only used when resizeRowToContents() is called. In that case,
    if a larger row height is required by either the vertical header or
    the item delegate, that width will be used instead.

    \sa QWidget::sizeHint, verticalHeader()
*/
int QTableView::sizeHintForRow(int row) const
{
    Q_D(const QTableView);

    if (!model())
        return -1;

    int left = qMax(0, columnAt(0));
    int right = columnAt(d->viewport->width());
    if (right == -1) // the table don't have enought columns to fill the viewport
        right = model()->columnCount(rootIndex()) - 1;

    QStyleOptionViewItem option = viewOptions();

    int hint = 0;
    QModelIndex index;
    for (int column = left; column <= right; ++column) {
        index = d->model->index(row, column, rootIndex());
        hint = qMax(hint, itemDelegate()->sizeHint(option, index).height());
    }

    return d->showGrid ? hint + 1 : hint;
}

/*!
    Returns the size hint for the given \a column's width or -1 if
    there is no model.

    If you need to set the width of a given column to a fixed value, call
    QHeaderView::resizeSection() on the table's horizontal header.

    If you reimplement this function in a subclass, note that the value you
    return is only used when resizeColumnToContents() is called. In that case,
    if a larger column width is required by either the horizontal header or
    the item delegate, that width will be used instead.

    \sa QWidget::sizeHint, horizontalHeader()
*/
int QTableView::sizeHintForColumn(int column) const
{
    Q_D(const QTableView);


    if (!model())
        return -1;

    int top = qMax(0, rowAt(0));
    int bottom = rowAt(d->viewport->height());
    if (!isVisible() || bottom == -1) // the table don't have enought rows to fill the viewport
        bottom = model()->rowCount(rootIndex()) - 1;

    QStyleOptionViewItem option = viewOptions();

    int hint = 0;
    QModelIndex index;
    for (int row = top; row <= bottom; ++row) {
        index = d->model->index(row, column, rootIndex());
        hint = qMax(hint, itemDelegate()->sizeHint(option, index).width());
    }

    return d->showGrid ? hint + 1 : hint;
}

/*!
    Returns the y-coordinate in contents coordinates of the given \a
    row.
*/
int QTableView::rowViewportPosition(int row) const
{
    return d_func()->verticalHeader->sectionViewportPosition(row);
}

/*!
    Returns the row in which the given y-coordinate, \a y, in contents
    coordinates is located.
*/
int QTableView::rowAt(int y) const
{
    return d_func()->verticalHeader->logicalIndexAt(y);
}

/*!
  Sets the height of the given \a row to be \a height.
*/
void QTableView::setRowHeight(int row, int height)
{
    d_func()->verticalHeader->resizeSection(row, height);
}

/*!
    Returns the height of the given \a row.

    \sa resizeRowToContents()
*/
int QTableView::rowHeight(int row) const
{
    return d_func()->verticalHeader->sectionSize(row);
}

/*!
    Returns the x-coordinate in contents coordinates of the given \a
    column.
*/
int QTableView::columnViewportPosition(int column) const
{
    return d_func()->horizontalHeader->sectionViewportPosition(column);
}

/*!
    Returns the column in which the given x-coordinate, \a x, in contents
    coordinates is located.
*/
int QTableView::columnAt(int x) const
{
    return d_func()->horizontalHeader->logicalIndexAt(x);
}

/*!
  Sets the width of the given \a column to be \a width.
*/
void QTableView::setColumnWidth(int column, int width)
{
    d_func()->horizontalHeader->resizeSection(column, width);
}

/*!
    Returns the width of the given \a column.

    \sa resizeColumnToContents()
*/
int QTableView::columnWidth(int column) const
{
    return d_func()->horizontalHeader->sectionSize(column);
}

/*!
    Returns true if the given \a row is hidden; otherwise returns false.
*/
bool QTableView::isRowHidden(int row) const
{
    return d_func()->verticalHeader->isSectionHidden(row);
}

/*!
  If \a hide is true \a row will be hidden, otherwise it will be shown.
*/
void QTableView::setRowHidden(int row, bool hide)
{
    d_func()->verticalHeader->setSectionHidden(row, hide);
}

/*!
    Returns true if the given \a column is hidden; otherwise returns false.
*/
bool QTableView::isColumnHidden(int column) const
{
    return d_func()->horizontalHeader->isSectionHidden(column);
}

/*!
  If \a hide is true the given \a column will be hidden; otherwise it
  will be shown.
*/
void QTableView::setColumnHidden(int column, bool hide)
{
    d_func()->horizontalHeader->setSectionHidden(column, hide);
}

/*!
    \property QTableView::showGrid
    \brief whether the grid is shown

    If this property is true a grid is drawn for the table; if the
    property is false, no grid is drawn. The default value is true.
*/
bool QTableView::showGrid() const
{
    return d_func()->showGrid;
}

void QTableView::setShowGrid(bool show)
{
    d_func()->showGrid = show;
}

/*!
  \property QTableView::gridStyle
  \brief  the pen style used to draw the grid.

  This property holds the style used when drawing the grid (see \l{showGrid}).
*/
Qt::PenStyle QTableView::gridStyle() const
{
    return d_func()->gridStyle;
}

void QTableView::setGridStyle(Qt::PenStyle style)
{
    d_func()->gridStyle = style;
}

/*!
    \internal

    Returns the rectangle on the viewport occupied by the given \a
    index.
    If the index is hidden in the view it will return a null QRect.
*/
QRect QTableView::visualRect(const QModelIndex &index) const
{
    if (!index.isValid() || index.parent() != rootIndex() || isIndexHidden(index) )
        return QRect();
    d_func()->executePostedLayout();

    int i = showGrid() ? 1 : 0;
    return QRect(columnViewportPosition(index.column()), rowViewportPosition(index.row()),
                 columnWidth(index.column()) - i, rowHeight(index.row()) - i);
}

/*!
    \internal

    Makes sure that the given \a item is visible in the table view,
    scrolling if necessary.
*/
void QTableView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_D(QTableView);

    // check if we really need to do anything
    if (index.parent() != rootIndex() || isIndexHidden(index))
        return;

    QRect area = d->viewport->rect();
    QRect rect = visualRect(index);
    if (hint == EnsureVisible && area.contains(rect)) {
        d->setDirtyRegion(rect);
        return;
    }

    // vertical
    int verticalSteps = verticalStepsPerItem();
    bool above = (hint == EnsureVisible && rect.top() < area.top());
    bool below = (hint == EnsureVisible && rect.bottom() > area.bottom());
    if (hint == PositionAtTop || above) {
        verticalScrollBar()->setValue(index.row() * verticalSteps);
    } else if (hint == PositionAtBottom || below) {
        int r = index.row();
        int y = area.height();
        while (y > 0 && r > 0)
            y -= rowHeight(r--);
        int h = rowHeight(r);
        int a = (-y * verticalSteps) / (h ? h : 1);
        verticalScrollBar()->setValue(++r * verticalSteps + a);
    }

    // horizontal
    int horizontalSteps = horizontalStepsPerItem();
    bool leftOf = isRightToLeft()
                  ? rect.right() > area.right()
                  : rect.left() < area.left();
    bool rightOf = isRightToLeft()
                   ? rect.left() < area.left()
                   : rect.right() > area.right();
    if (leftOf) {
        horizontalScrollBar()->setValue(index.column() * horizontalSteps);
    } else if (rightOf) {
        int c = index.column();
        int x = area.width();
        while (x > 0 && c > 0)
            x -= columnWidth(c--);
        int w = columnWidth(c);
        int a = (-x * horizontalSteps) / (w ? w : 1);
        horizontalScrollBar()->setValue(++c * horizontalSteps + a);
    }

    d->setDirtyRegion(visualRect(index));
}

/*!
    This slot is called to change the height of the given \a row. The
    old height is specified by \a oldHeight, and the new height by \a
    newHeight.

    \sa columnResized()
*/
void QTableView::rowResized(int row, int, int)
{
    Q_D(QTableView);

    int y = rowViewportPosition(row);
    d->viewport->update(QRect(0, y, d->viewport->width(), d->viewport->height() - y));
    updateGeometries();
}

/*!
    This slot is called to change the width of the given \a column.
    The old width is specified by \a oldWidth, and the new width by \a
    newWidth.

    \sa rowResized()
*/
void QTableView::columnResized(int column, int, int)
{
    Q_D(QTableView);

    int x = columnViewportPosition(column);
    QRect rect;
    if (isRightToLeft())
        rect.setRect(0, 0, x + columnWidth(column), d->viewport->height());
    else
        rect.setRect(x, 0, d->viewport->width() - x, d->viewport->height());
    d->viewport->update(rect.normalized());
    updateGeometries();
}

/*!
    This slot is called to change the index of the given \a row in the
    table view. The old index is specified by \a oldIndex, and the new
    index by \a newIndex.

    \sa columnMoved()
*/
void QTableView::rowMoved(int, int oldIndex, int newIndex)
{
    Q_D(QTableView);

    int o = rowViewportPosition(d->verticalHeader->logicalIndex(oldIndex));
    int n = rowViewportPosition(d->verticalHeader->logicalIndex(newIndex));
    int top = (o < n ? o : n);
    int height = d->viewport->height() - (o > n ? o : n);
    updateGeometries();
    d->viewport->update(0, top, d->viewport->width(), height);
}

/*!
    This slot is called to change the index of the given \a column in
    the table view. The old index is specified by \a oldIndex, and
    the new index by \a newIndex.

    \sa rowMoved()
*/
void QTableView::columnMoved(int, int oldIndex, int newIndex)
{
    Q_D(QTableView);

    int o = columnViewportPosition(d->horizontalHeader->logicalIndex(oldIndex));
    int n = columnViewportPosition(d->horizontalHeader->logicalIndex(newIndex));
    int left = (o < n ? o : n);
    int width = d->viewport->width() - (o > n ? o : n);
    updateGeometries();
    d->viewport->update(left, 0, width, d->viewport->height());
}

/*!
    Selects the given \a row in the table view if the current
    SelectionMode and SelectionBehavior allows rows to be selected.

    \sa selectColumn()
*/
void QTableView::selectRow(int row)
{
    Q_D(QTableView);

    if (selectionBehavior() == SelectColumns ||
        (selectionMode() == SingleSelection && selectionBehavior() == SelectItems)) {
        qWarning("selectRow() failed since the current SelectionMode and SelectionBehavior"
                 " does not allow rows to be selected.");
        return;
    }

    if (row >= 0 && row < model()->rowCount(rootIndex())) {
        QModelIndex index = model()->index(row, 0, rootIndex());
        QItemSelectionModel::SelectionFlags command = selectionCommand(index);
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        if (!(command & QItemSelectionModel::Current))
            d->rowSectionAnchor = row;
        QModelIndex tl = model()->index(qMin(d->rowSectionAnchor, row), 0, rootIndex());
        QModelIndex br = model()->index(qMax(d->rowSectionAnchor, row),
                                        model()->columnCount(rootIndex()) - 1, rootIndex());
        selectionModel()->select(QItemSelection(tl, br), command);
    }
}

/*!
    Selects the given \a column in the table view if the current
    SelectionMode and SelectionBehavior allows columns to be selected.

    \sa selectRow()
*/
void QTableView::selectColumn(int column)
{
    Q_D(QTableView);

    if (selectionBehavior() == SelectRows ||
        (selectionMode() == SingleSelection && selectionBehavior() == SelectItems)) {
        qWarning("selectColumn() failed since the current SelectionMode and SelectionBehavior"
                 " does not allow columns to be selected.");
        return;
    }

    if (column >= 0 && column < model()->columnCount(rootIndex())) {
        QModelIndex index = model()->index(0, column, rootIndex());
        QItemSelectionModel::SelectionFlags command = selectionCommand(index);
        selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
        if (!(command & QItemSelectionModel::Current))
            d->columnSectionAnchor = column;
        QModelIndex tl = model()->index(0, qMin(d->columnSectionAnchor, column), rootIndex());
        QModelIndex br = model()->index(model()->rowCount(rootIndex()) - 1,
                                        qMax(d->columnSectionAnchor, column), rootIndex());
        selectionModel()->select(QItemSelection(tl, br), command);
    }
}

/*!
    Hide the given \a row.

    \sa showRow() hideColumn()
*/
void QTableView::hideRow(int row)
{
    d_func()->verticalHeader->hideSection(row);
}

/*!
    Hide the given \a column.

    \sa showColumn() hideRow()
*/
void QTableView::hideColumn(int column)
{
    d_func()->horizontalHeader->hideSection(column);
}

/*!
    Show the given \a row.

    \sa hideRow() showColumn()
*/
void QTableView::showRow(int row)
{
    d_func()->verticalHeader->showSection(row);
}

/*!
    Show the given \a column.

    \sa hideColumn() showRow()
*/
void QTableView::showColumn(int column)
{
    d_func()->horizontalHeader->showSection(column);
}

/*!
    Resizes the given \a row based on the size hints of the delegates
    used to render each item in the row.
*/
void QTableView::resizeRowToContents(int row)
{
    Q_D(QTableView);

    int content = sizeHintForRow(row);
    int header = d->verticalHeader->isHidden() ? 0 : d->verticalHeader->sectionSizeHint(row);
    d->verticalHeader->resizeSection(row, qMax(content, header));
}

/*!
    Resizes the given \a column based on the size hints of the delegates
    used to render each item in the column.
*/
void QTableView::resizeColumnToContents(int column)
{
    Q_D(QTableView);

    int content = sizeHintForColumn(column);
    int header = d->horizontalHeader->isHidden() ? 0 : d->horizontalHeader->sectionSizeHint(column);
    d->horizontalHeader->resizeSection(column, qMax(content, header));
}

/*!
  Sorts the model by the values in the given \a column.
 */
void QTableView::sortByColumn(int column)
{
    Q_D(QTableView);

    if (!d->model)
        return;
    bool ascending = (horizontalHeader()->sortIndicatorSection() == column
                      && horizontalHeader()->sortIndicatorOrder() == Qt::DescendingOrder);
    Qt::SortOrder order = ascending ? Qt::AscendingOrder : Qt::DescendingOrder;
    horizontalHeader()->setSortIndicator(column, order);
    d->model->sort(column, order);
}

/*!
    \internal
*/
void QTableView::verticalScrollbarAction(int action)
{
    Q_D(QTableView);

    int count = d->verticalHeader->count();
    if (count == 0)
        return; // nothing to do
    int steps = verticalStepsPerItem();
    int value = verticalScrollBar()->value();
    int row = value / steps;
    int above = (value % steps) * d->verticalHeader->sectionSize(row); // what's left; in "item units"
    int y = -(above / steps); // above the page

    if (action == QScrollBar::SliderPageStepAdd) {
        // go down to the bottom of the page
        int h = d->viewport->height();
        while (y < h && row < count)
            y += d->verticalHeader->sectionSize(row++);
        value = row * steps; // i is now the last item on the page
        if (y > h && row)
            value -= steps * (y - h) / d->verticalHeader->sectionSize(row - 1);
        verticalScrollBar()->setSliderPosition(value);
    } else if (action == QScrollBar::SliderPageStepSub) {
        y += d->viewport->height();
        // go up to the top of the page
        while (y > 0 && row > 0)
            y -= d->verticalHeader->sectionSize(--row);
        value = row * steps; // i is now the first item in the page
        if (y < 0)
            value += steps * -y / d->verticalHeader->sectionSize(row);
        verticalScrollBar()->setSliderPosition(value);
    }
}

/*!
    \internal
*/
void QTableView::horizontalScrollbarAction(int action)
{
    Q_D(QTableView);

    int count = d->horizontalHeader->count();
    if (count == 0)
        return; // nothing to do
    int steps = horizontalStepsPerItem();
    int value = horizontalScrollBar()->value();
    int column = value / steps;
    int above = (value % steps) * d->horizontalHeader->sectionSize(column); // what's left; in "item units"
    int x = -(above / steps); // above the page

    if (action == QScrollBar::SliderPageStepAdd) {
        // go down to the right of the page
        int w = d->viewport->width();
        while (x < w && column < count)
            x += d->horizontalHeader->sectionSize(column++);
        value = column * steps; // i is now the last item on the page
        if (x > w && column)
            value -= steps * (x - w) / d->horizontalHeader->sectionSize(column - 1);
        horizontalScrollBar()->setSliderPosition(value);

    } else if (action == QScrollBar::SliderPageStepSub) {
        x += d->viewport->width();
        // go up to the left of the page
        while (x > 0 && column > 0)
            x -= d->horizontalHeader->sectionSize(--column);
        value = column * steps; // i is now the first item in the page
        if (x < 0)
            value += steps * -x / d->horizontalHeader->sectionSize(column);
        horizontalScrollBar()->setSliderPosition(value);
    }
}

/*!
  \reimp
*/
bool QTableView::isIndexHidden(const QModelIndex &index) const
{
    return isRowHidden(index.row()) || isColumnHidden(index.column());
}

#endif // QT_NO_TABLEVIEW
