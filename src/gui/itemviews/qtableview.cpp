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
#include <qheaderview.h>
#include <qitemdelegate.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qsize.h>
#include <qevent.h>
#include <qscrollbar.h>
#include <private/qtableview_p.h>

#define d d_func()
#define q q_func()

void QTableViewPrivate::init()
{
    q->setEditTriggers(editTriggers|QAbstractItemView::AnyKeyPressed);

    QHeaderView *vertical = new QHeaderView(Qt::Vertical, q);
    vertical->setModel(model);
    vertical->setSelectionModel(selectionModel);
    vertical->setClickable(true);
    vertical->setHighlightSections(true);
    q->setVerticalHeader(vertical);

    QHeaderView *horizontal = new QHeaderView(Qt::Horizontal, q);
    horizontal->setModel(model);
    horizontal->setSelectionModel(selectionModel);
    horizontal->setClickable(true);
    horizontal->setHighlightSections(true);
    q->setHorizontalHeader(horizontal);
}

void QTableViewPrivate::updateVerticalScrollbar()
{
    int factor = q->verticalFactor();
    int height = viewport->height();
    int count = verticalHeader->count();

    // if we have no viewport or no rows, there is nothing to do
    if (height <= 0 || count <= 0) {
        q->verticalScrollBar()->setRange(0, 0);
        return;
    }

    // set the scroller range
    int y = height;
    while (y > 0 && count > 0)
        y -= verticalHeader->sectionSize(--count);
    int max = count * factor;

    // set page step size
    int visibleCount = verticalHeader->count() - count - 1;
    q->verticalScrollBar()->setPageStep(visibleCount * factor);

    if (y < 0) { // if the last item starts above the viewport, we have to backtrack
        int sectionSize = verticalHeader->sectionSize(count);
        if (sectionSize) // avoid division by zero
            max += ((-y * factor)/ sectionSize) + 1; // how many units to add
    }

    q->verticalScrollBar()->setRange(0, max);
}

void QTableViewPrivate::updateHorizontalScrollbar()
{
    int factor = q->horizontalFactor();
    int width = viewport->width();
    int count = horizontalHeader->count();

    // if we have no viewport or no columns, there is nothing to do
    if (width <= 0 || count <= 0) {
        q->horizontalScrollBar()->setRange(0, 0);
        return;
    }

    // set the scroller range
    int x = width;
    while (x > 0 && count > 0)
        x -= horizontalHeader->sectionSize(--count);
    int max = count * factor;

    // set page step size
    int visibleCount = horizontalHeader->count() - count - 1;
    q->horizontalScrollBar()->setPageStep(visibleCount * factor);

    if (x < 0) { // if the last item starts left of the viewport, we have to backtrack
        int sectionSize = horizontalHeader->sectionSize(count);
        if (sectionSize) // avoid division by zero
            max += ((-x * factor) / sectionSize) + 1;
    }

    q->horizontalScrollBar()->setRange(0, max);
}

/*!
    \class QTableView qtableview.h

    \brief The QTableView class provides a default model/view
    implementation of a table view.

    \ingroup model-view

    A QTableView implements a table view that displays items from a
    model. This class is used to provide standard tables that were
    previously provided by the \c QTable class, but using the more
    flexible approach provided by Qt's model/view architecture.

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

// ### DOC: Where does the model come from?
/*!
    Constructs a table view with a \a parent to represent the data.

    \sa QAbstractItemModel
*/

QTableView::QTableView(QWidget *parent)
    : QAbstractItemView(*new QTableViewPrivate, parent)
{
    d->init();
}

/*!
  \internal
*/
QTableView::QTableView(QTableViewPrivate &dd, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    d->init();
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
    Q_ASSERT(model);
    if (d->selectionModel) // support row editing
        disconnect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
                   d->model, SLOT(submit()));

    d->verticalHeader->setModel(model);
    d->horizontalHeader->setModel(model);
    QAbstractItemView::setModel(model);
}

/*!
  \reimp
*/
void QTableView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    Q_ASSERT(selectionModel);
    if (d->model && d->selectionModel) // support row editing
        disconnect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                   d->model, SLOT(submit()));

    d->verticalHeader->setSelectionModel(selectionModel);
    d->horizontalHeader->setSelectionModel(selectionModel);
    QAbstractItemView::setSelectionModel(selectionModel);

    if (d->model) // support row editing
        connect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                d->model, SLOT(submit()));
}

/*!
    Returns the table view's horizontal header.

    \sa setHorizontalHeader() verticalHeader()
*/

QHeaderView *QTableView::horizontalHeader() const
{
    return d->horizontalHeader;
}

/*!
    Returns the table view's vertical header.

    \sa setVerticalHeader() horizontalHeader()
*/
QHeaderView *QTableView::verticalHeader() const
{
    return d->verticalHeader;
}

/*!
    Sets the widget to use for the vertical header to \a header.

    \sa horizontalHeader() setVerticalHeader()
*/
void QTableView::setHorizontalHeader(QHeaderView *header)
{
    Q_ASSERT(header);
    if (d->horizontalHeader) {
        disconnect(d->horizontalHeader,SIGNAL(sectionResized(int,int,int)),
                   this, SLOT(columnResized(int,int,int)));
        disconnect(d->horizontalHeader, SIGNAL(sectionMoved(int,int,int)),
                   this, SLOT(columnMoved(int,int,int)));
        disconnect(d->horizontalHeader, SIGNAL(sectionCountChanged(int,int)),
                   this, SLOT(columnCountChanged(int,int)));
        disconnect(d->horizontalHeader,
                   SIGNAL(sectionPressed(int,Qt::MouseButton,Qt::KeyboardModifiers)),
                   this, SLOT(selectColumn(int)));
        disconnect(d->horizontalHeader, SIGNAL(sectionHandleDoubleClicked(int)),
                   this, SLOT(resizeColumnToContents(int)));
        d->horizontalHeader->setFocusProxy(0);
    }

    d->horizontalHeader = header;

    connect(d->horizontalHeader,SIGNAL(sectionResized(int,int,int)),
            this, SLOT(columnResized(int,int,int)), Qt::QueuedConnection);
    connect(d->horizontalHeader, SIGNAL(sectionMoved(int,int,int)),
            this, SLOT(columnMoved(int,int,int)), Qt::QueuedConnection);
    connect(d->horizontalHeader, SIGNAL(sectionCountChanged(int,int)),
            this, SLOT(columnCountChanged(int,int)), Qt::QueuedConnection);
    connect(d->horizontalHeader,
            SIGNAL(sectionPressed(int,Qt::MouseButton,Qt::KeyboardModifiers)),
            this, SLOT(selectColumn(int)));
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
    if (d->verticalHeader) {
        disconnect(d->verticalHeader, SIGNAL(sectionResized(int,int,int)),
                   this, SLOT(rowResized(int,int,int)));
        disconnect(d->verticalHeader, SIGNAL(sectionMoved(int,int,int)),
                   this, SLOT(rowMoved(int,int,int)));
        disconnect(d->verticalHeader, SIGNAL(sectionCountChanged(int,int)),
                   this, SLOT(rowCountChanged(int,int)));
        disconnect(d->verticalHeader,
                   SIGNAL(sectionPressed(int,Qt::MouseButton,Qt::KeyboardModifiers)),
                   this, SLOT(selectRow(int)));
        disconnect(d->verticalHeader, SIGNAL(sectionHandleDoubleClicked(int)),
                   this, SLOT(resizeRowToContents(int)));
        d->verticalHeader->setFocusProxy(0);
    }

    d->verticalHeader = header;

    connect(d->verticalHeader, SIGNAL(sectionResized(int,int,int)),
            this, SLOT(rowResized(int,int,int)), Qt::QueuedConnection);
    connect(d->verticalHeader, SIGNAL(sectionMoved(int,int,int)),
            this, SLOT(rowMoved(int,int,int)), Qt::QueuedConnection);
    connect(d->verticalHeader, SIGNAL(sectionCountChanged(int,int)),
            this, SLOT(rowCountChanged(int,int)), Qt::QueuedConnection);
    connect(d->verticalHeader,
            SIGNAL(sectionPressed(int,Qt::MouseButton,Qt::KeyboardModifiers)),
            this, SLOT(selectRow(int)));
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
    if (dx) { // horizontal
        int value = horizontalScrollBar()->value();
        int section = d->horizontalHeader->logicalIndex(value / horizontalFactor());
        int left = (value % horizontalFactor()) * d->horizontalHeader->sectionSize(section);
        int offset = (left / horizontalFactor()) + d->horizontalHeader->sectionPosition(section);
        if (isRightToLeft())
            dx = offset - d->horizontalHeader->offset();
        else
            dx = d->horizontalHeader->offset() - offset;
        d->horizontalHeader->setOffset(offset);
    }

    if (dy) { // vertical
        int value = verticalScrollBar()->value();
        int section = d->verticalHeader->logicalIndex(value / verticalFactor());
        int above = (value % verticalFactor()) * d->verticalHeader->sectionSize(section);
        int offset = (above / verticalFactor()) + d->verticalHeader->sectionPosition(section);
        dy = d->verticalHeader->offset() - offset;
        d->verticalHeader->setOffset(offset);
    }

    d->viewport->scroll(dx, dy);
}

/*!
    Paints the table on receipt of the given paint event \a e.
*/
void QTableView::paintEvent(QPaintEvent *e)
{
    QStyleOptionViewItem option = viewOptions();
    QBrush base = option.palette.base();
    QRect area = e->rect();

    // if there's nothing to do, clear the area and return

    QPainter painter(d->viewport);
    if (d->horizontalHeader->count() == 0 || d->verticalHeader->count() == 0) {
        painter.fillRect(area, base);
        return;
    }

    // get the horizontal start and end sections (visual indexes)

    int left = d->horizontalHeader->visualIndexAt(area.left());
    int right = d->horizontalHeader->visualIndexAt(area.right());

    if (isRightToLeft()) {
        left = (left == -1 ? model()->columnCount(root()) - 1 : left);
        right = (right == -1 ? 0 : right);
    } else {
        left = (left == -1 ? 0 : left);
        right = (right == -1 ? model()->columnCount(root()) - 1 : right);
    }

    int tmp = left;
    left = qMin(left, right);
    right = qMax(tmp, right);

    // get the vertical start and end sections (visual indexes)

    int top = d->verticalHeader->visualIndexAt(area.top());
    int bottom = d->verticalHeader->visualIndexAt(area.bottom());

    top = (top == -1 ? 0 : top);
    bottom = (bottom == -1 ? d->model->rowCount(root()) - 1 : bottom);

    tmp = top;
    top = qMin(top, bottom);
    bottom = qMax(tmp, bottom);

    // setup temp variables for the painting

    bool showGrid = d->showGrid;
    int gridSize = showGrid ? 1 : 0;
    int gridHint = style()->styleHint(QStyle::SH_Table_GridLineColor, 0, this);
    QColor gridColor = gridHint != -1
                       ? static_cast<QRgb>(gridHint)
                       : palette().color(QPalette::Mid);
    QPen gridPen = QPen(gridColor, 0, d->gridStyle);

    QItemSelectionModel *sels = selectionModel();
    QHeaderView *verticalHeader = d->verticalHeader;
    QHeaderView *horizontalHeader = d->horizontalHeader;
    QModelIndex current = currentIndex();
    bool focus = hasFocus() && current.isValid();
    QStyle::StyleFlags state = option.state;
    bool alternate = d->alternatingColors;
    QColor oddColor = d->oddColor;
    QColor evenColor = d->evenColor;

    // do the actual painting

    for (int v = top; v <= bottom; ++v) {
        int row = verticalHeader->logicalIndex(v);
        if (verticalHeader->isSectionHidden(row))
            continue;
        if (alternate)
            option.palette.setColor(QPalette::Base, v & 1 ? oddColor : evenColor);
        int rowp = rowViewportPosition(row);
        int rowh = rowHeight(row) - gridSize;
        for (int h = left; h <= right; ++h) {
            int col = horizontalHeader->logicalIndex(h);
            if (horizontalHeader->isSectionHidden(col))
                continue;
            int colp = columnViewportPosition(col);
            int colw = columnWidth(col) - gridSize;
            QModelIndex index = model()->index(row, col, root());
            if (index.isValid()) {
                option.rect = QRect(colp, rowp, colw, rowh);
                option.state = state;
                option.state |= (sels->isSelected(index)
                                 ? QStyle::Style_Selected : QStyle::Style_None);
                if ((model()->flags(index) & QAbstractItemModel::ItemIsEnabled) == 0)
                    option.state &= ~QStyle::Style_Enabled;
                option.state |= (focus && index == current
                                 ? QStyle::Style_HasFocus : QStyle::Style_None);
                painter.fillRect(colp, rowp, colw, rowh,
                                 (option.state & QStyle::Style_Selected
                                  ? option.palette.highlight() : option.palette.base()));
                itemDelegate()->paint(&painter, option, index);
            }
            if (v == top && showGrid) {
                QPen old = painter.pen();
                painter.setPen(gridPen);
                painter.drawLine(colp + colw, area.top(), colp + colw, area.bottom());
                painter.setPen(old);
            }
        }
        if (showGrid) {
            QPen old = painter.pen();
            painter.setPen(gridPen);
            painter.drawLine(area.left(), rowp + rowh, area.right(), rowp + rowh);
            painter.setPen(old);
        }
    }

    int w = d->viewport->width();
    int h = d->viewport->height();
    int x = d->horizontalHeader->length();
    int y = d->verticalHeader->length();
    QRect b(0, y, w, h - y);
    if (y < h && area.intersects(b))
        painter.fillRect(b, base);
    if (isRightToLeft()) {
        QRect r(0, 0, w - x, h);
        if (x > 0 && area.intersects(r))
            painter.fillRect(r, base);
    } else {
        QRect l(x, 0, w - x, h);
        if (x < w && area.intersects(l))
            painter.fillRect(l, base);
    }
}

/*!
    Returns the index position of the model item corresponding to the
    table item at position (\a x, \a y) in contents coordinates.
*/
QModelIndex QTableView::itemAt(int x, int y) const
{
    int r = rowAt(y);
    int c = columnAt(x);
    if (r >= 0 && c >= 0)
        return model()->index(r, c, root());
    return QModelIndex();
}

/*!
    Returns the horizontal offset of the items in the table view.

    \sa verticalOffset()
*/
int QTableView::horizontalOffset() const
{
    return d->horizontalHeader->offset();
}

/*!
    Returns the vertical offset of the items in the table view.

    \sa horizontalOffset()
*/
int QTableView::verticalOffset() const
{
    return d->verticalHeader->offset();
}

/*!
    \fn QModelIndex QTableView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::KeyboardModifiers modifiers)

    Moves the cursor in accordance with the given \a cursorAction, using the
    information provided by the \a modifiers.

    \sa QAbstractItemView::CursorAction
*/
QModelIndex QTableView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                   Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    QModelIndex current = currentIndex();
    int visualRow = verticalHeader()->visualIndex(current.row());
    int visualColumn = horizontalHeader()->visualIndex(current.column());
    int verticalStep = 0;
    int horizontalStep = 0;

    int bottom = model()->rowCount(root()) - 1;
    int right = model()->columnCount(root()) - 1;
    switch (cursorAction) {
    case MoveUp:
        verticalStep = -1;
        visualRow += verticalStep;
        break;
    case MoveDown:
        verticalStep = 1;
        visualRow += verticalStep;
        break;
    case MovePrevious:
    case MoveLeft:
        horizontalStep = isRightToLeft() ? 1 : -1;
        visualColumn += horizontalStep;
        break;
    case MoveNext:
    case MoveRight:
        horizontalStep = isRightToLeft() ? -1 : 1;
        visualColumn += horizontalStep;
        break;
    case MoveHome:
        verticalStep = 1;
        visualRow = 0;
        break;
    case MoveEnd:
        verticalStep = -1;
        visualRow = bottom;
        break;
    case MovePageUp: {
        int newRow = rowAt(itemViewportRect(current).top() - d->viewport->height());
        return model()->index(newRow <= bottom ? newRow : 0, current.column(), root());}
    case MovePageDown: {
        int newRow = rowAt(itemViewportRect(current).bottom() + d->viewport->height());
        return model()->index(newRow >= 0 ? newRow : bottom, current.column(), root());}
    }

    while (verticalStep != 0
           && visualRow >= 0
           && visualRow <= bottom
           && verticalHeader()->isSectionHidden(verticalHeader()->logicalIndex(visualRow)))
        visualRow += verticalStep;

    while (horizontalStep != 0
           && visualColumn >= 0
           && visualColumn <= right
           && horizontalHeader()->isSectionHidden(horizontalHeader()->logicalIndex(visualColumn)))
        visualColumn += horizontalStep;

    int logicalRow = verticalHeader()->logicalIndex(visualRow);
    int logicalColumn = horizontalHeader()->logicalIndex(visualColumn);
    if (model()->hasIndex(logicalRow, logicalColumn, root()))
        return model()->index(logicalRow, logicalColumn, root());
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
    QModelIndex tl = itemAt((isRightToLeft()
                             ? rect.right() : rect.left()), rect.top());
    QModelIndex br = itemAt((isRightToLeft()
                             ? rect.left() : rect.right()), rect.bottom());
    selectionModel()->select(QItemSelection(tl, br), command);
}

/*!
    \internal

    Returns the rectangle from the viewport of the items in the given
    \a selection.
*/
QRect QTableView::selectionViewportRect(const QItemSelection &selection) const
{
    int top = d->model->rowCount(root()) - 1;
    int left = d->model->columnCount(root()) - 1;
    int bottom = 0;
    int right = 0;
    int rangeTop, rangeLeft, rangeBottom, rangeRight;

    for (int i = 0; i < selection.count(); ++i) {
        QItemSelectionRange r = selection.at(i);
        if (r.parent().isValid())
            continue;
        // find the visual top, left, bottom and right
        rangeTop = d->verticalHeader->visualIndex(r.top());
        rangeLeft = d->horizontalHeader->visualIndex(r.left());
        rangeBottom = d->verticalHeader->visualIndex(r.bottom());
        rangeRight = d->horizontalHeader->visualIndex(r.right());
        if (rangeTop < top)
            top = rangeTop;
        if (rangeLeft < left)
            left = rangeLeft;
        if (rangeBottom > bottom)
            bottom = rangeBottom;
        if (rangeRight > right)
            right = rangeRight;
    }

    int leftCol = d->horizontalHeader->logicalIndex(left);
    int topRow = d->verticalHeader->logicalIndex(top);
    int rightCol = d->horizontalHeader->logicalIndex(right);
    int bottomRow = d->verticalHeader->logicalIndex(bottom);

    int leftWidth = isRightToLeft() ? columnWidth(leftCol) : 0;
    int leftPos = columnViewportPosition(leftCol) + leftWidth;
    int topPos = rowViewportPosition(topRow);
    int rightWidth = isRightToLeft() ? 0 : columnWidth(rightCol);
    int rightPos = columnViewportPosition(rightCol) + rightWidth;
    int bottomPos = rowViewportPosition(bottomRow) + rowHeight(bottomRow);

    QRect rect(leftPos, topPos, rightPos - leftPos, bottomPos - topPos);
    return rect.normalize();
}


/*!
  \reimp
*/
QModelIndexList QTableView::selectedIndexes() const
{
    QModelIndexList viewSelected;
    QModelIndexList modelSelected = selectionModel()->selectedIndexes();
    for (int i=0; i<modelSelected.count(); ++i) {
        QModelIndex index = modelSelected.at(i);
        if (!isIndexHidden(index) && index.parent() == root())
            viewSelected.append(index);
    }
    return viewSelected;
}


/*!
    This slot is called whenever rows are added or deleted. The
    previous number of rows is specified by \a oldCount, and the new
    number of rows is specified by \a newCount.
*/
void QTableView::rowCountChanged(int, int)
{
    updateGeometries();
    d->viewport->update();
}

/*!
    This slot is called whenever columns are added or deleted. The
    previous number of columns is specified by \a oldCount, and the new
    number of columns is specified by \a newCount.
*/
void QTableView::columnCountChanged(int, int)
{
    updateGeometries();
    d->viewport->update();
}

/*!
    \internal
*/
void QTableView::updateGeometries()
{
    int width = d->verticalHeader->isVisible() ? d->verticalHeader->sizeHint().width() : 0;
    int height = d->verticalHeader->isVisible() ? d->horizontalHeader->sizeHint().height() : 0;
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
    Returns the size hint for the given \a row's height.

    \sa QWidget::sizeHint
*/
int QTableView::rowSizeHint(int row) const
{
    int columnfirst = columnAt(0);
    int columnlast = columnAt(d->viewport->width());
    if (columnlast < 0)
        columnlast = d->horizontalHeader->count() - 1;

    QStyleOptionViewItem option = viewOptions();

    int hint = 0;
    QModelIndex index;
    for (int column = columnfirst; column < columnlast; ++column) {
        index = d->model->index(row, column, root());
        hint = qMax(hint, itemDelegate()->sizeHint(option, index).height());
    }
    return d->showGrid ? hint + 1 : hint;
}

/*!
    Returns the size hint for the given \a column's width.

    \sa QWidget::sizeHint
*/
int QTableView::columnSizeHint(int column) const
{
    int rowfirst = rowAt(0);
    int rowlast = rowAt(d->viewport->height());
    if (rowlast < 0)
        rowlast = d->verticalHeader->count() - 1;

    QStyleOptionViewItem option = viewOptions();

    int hint = 0;
    QModelIndex index;
    for (int row = rowfirst; row <= rowlast; ++row) {
        index = d->model->index(row, column, root());
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
    return d->verticalHeader->sectionViewportPosition(row);
}

/*!
    Returns the height of the given \a row.
*/
int QTableView::rowHeight(int row) const
{
    return d->verticalHeader->sectionSize(row);
}

/*!
    Returns the row in which the given y-coordinate, \a y, in contents
    coordinates is located.
*/
int QTableView::rowAt(int y) const
{
    return d->verticalHeader->logicalIndexAt(y);
}

/*!
    Returns the x-coordinate in contents coordinates of the given \a
    column.
*/
int QTableView::columnViewportPosition(int column) const
{
    return d->horizontalHeader->sectionViewportPosition(column);
}

/*!
    Returns the width of the given \a column.
*/
int QTableView::columnWidth(int column) const
{
    return d->horizontalHeader->sectionSize(column);
}

/*!
    Returns the column in which the given x-coordinate, \a x, in contents
    coordinates is located.
*/
int QTableView::columnAt(int x) const
{
    return d->horizontalHeader->logicalIndexAt(x);
}

/*!
    Returns true if the given \a row is hidden; otherwise returns false.
*/
bool QTableView::isRowHidden(int row) const
{
    return d->verticalHeader->isSectionHidden(row);
}

/*!
  If \a hide is true \a row will be hidden, otherwise it will be shown.
*/
void QTableView::setRowHidden(int row, bool hide)
{
    if (hide)
        hideRow(row);
    else
        showRow(row);
}

/*!
    Returns true if the given \a column is hidden; otherwise returns false.
*/
bool QTableView::isColumnHidden(int column) const
{
    return d->horizontalHeader->isSectionHidden(column);
}

/*!
  If \a hide is true the given \a column will be hidden; otherwise it
  will be shown.
*/
void QTableView::setColumnHidden(int column, bool hide)
{
    if (hide)
        hideColumn(column);
    else
        showColumn(column);
}

/*!
    \property QTableView::showGrid
    \brief whether the grid is shown

    If this property is true a grid is drawn for the table; if the
    property is false, no grid is drawn. The default value is true.
*/
bool QTableView::showGrid() const
{
    return d->showGrid;
}

void QTableView::setShowGrid(bool show)
{
    d->showGrid = show;
}

/*!
  \property QTableView::gridStyle
  \brief  the pen style used to draw the grid.

  This property holds the style used when drawing the grid (see \l{showGrid}).
*/
Qt::PenStyle QTableView::gridStyle() const
{
    return d->gridStyle;
}

void QTableView::setGridStyle(Qt::PenStyle style)
{
    d->gridStyle = style;
}

/*!
    \internal

    Returns the rectangle on the viewport occupied by the given \a
    index.
*/
QRect QTableView::itemViewportRect(const QModelIndex &index) const
{
    if (!index.isValid() || index.parent() != root())
        return QRect();
    return QRect(columnViewportPosition(index.column()), rowViewportPosition(index.row()),
                 columnWidth(index.column()), rowHeight(index.row()));
}

/*!
    \internal

    Makes sure that the given \a item is visible in the table view,
    scrolling if necessary.
*/
void QTableView::ensureVisible(const QModelIndex &index)
{
    // check if we really need to do anything
    if (index.parent() != root())
        return;
    if (isIndexHidden(index))
        return;
    QRect area = d->viewport->rect();
    QRect rect = itemViewportRect(index);
    if (area.contains(rect)) {
        d->viewport->update(rect);
        return;
    }

    // vertical
    if (rect.top() < area.top()) { // above
        verticalScrollBar()->setValue(index.row() * verticalFactor());
    } else if (rect.bottom() > area.bottom()) { // below
        int r = index.row();
        int y = area.height();
        while (y > 0 && r > 0)
            y -= rowHeight(r--);
        int h = rowHeight(r);
        int a = (-y * verticalFactor()) / (h ? h : 1);
        verticalScrollBar()->setValue(++r * verticalFactor() + a);
    }

    // horizontal
    bool leftOf = isRightToLeft()
                  ? rect.right() > area.right()
                  : rect.left() < area.left();
    bool rightOf = isRightToLeft()
                   ? rect.left() < area.left()
                   : rect.right() > area.right();
    if (leftOf) {
        horizontalScrollBar()->setValue(index.column() * horizontalFactor());
    } else if (rightOf) {
        int c = index.column();
        int x = area.width();
        while (x > 0 && c > 0)
            x -= columnWidth(c--);
        int w = columnWidth(c);
        int a = (-x * horizontalFactor()) / (w ? w : 1);
        horizontalScrollBar()->setValue(++c * horizontalFactor() + a);
    }

    d->viewport->update(itemViewportRect(index));
}

/*!
    This slot is called to change the height of the given \a row. The
    old height is specified by \a oldHeight, and the new height by \a
    newHeight.

    \sa columnResized()
*/
void QTableView::rowResized(int row, int, int)
{
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
    int x = columnViewportPosition(column);
    QRect rect;
    if (isRightToLeft())
        rect.setRect(0, 0, x + columnWidth(column), d->viewport->height());
    else
        rect.setRect(x, 0, d->viewport->width() - x, d->viewport->height());
    d->viewport->update(rect.normalize());
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
    int o = columnViewportPosition(d->horizontalHeader->logicalIndex(oldIndex));
    int n = columnViewportPosition(d->horizontalHeader->logicalIndex(newIndex));
    int left = (o < n ? o : n);
    int width = d->viewport->width() - (o > n ? o : n);
    updateGeometries();
    d->viewport->update(left, 0, width, d->viewport->height());
}

/*!
    Selects the given \a row in the table view.

    \sa selectColumn()
*/
void QTableView::selectRow(int row)
{
    if (row >= 0 && row < model()->rowCount(root())) {
        QItemSelectionModel::SelectionFlags command = selectionCommand(QModelIndex());
        QModelIndex index = model()->index(row, 0, root());
        if (selectionMode() == SingleSelection) {
            selectionModel()->setCurrentIndex(index, command);
        } else {
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
            if ((command & QItemSelectionModel::Current) == 0)
                d->rowSectionAnchor = row;
            QModelIndex tl = model()->index(qMin(d->rowSectionAnchor, row), 0, root());
            QModelIndex br = model()->index(qMax(d->rowSectionAnchor, row),
                                            model()->columnCount(root()) - 1, root());
            selectionModel()->select(QItemSelection(tl, br), command);
        }
    }
}

/*!
    Selects the given \a column in the table view.

    \sa selectRow()
*/
void QTableView::selectColumn(int column)
{
    if (column >= 0 && column < model()->columnCount(root())) {
        QItemSelectionModel::SelectionFlags command = selectionCommand(QModelIndex());
        QModelIndex index = model()->index(0, column, root());
        if (selectionMode() == SingleSelection) {
            selectionModel()->setCurrentIndex(index, command);
        } else {
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
            if ((command & QItemSelectionModel::Current) == 0)
                d->columnSectionAnchor = column;
            QModelIndex tl = model()->index(0, qMin(d->columnSectionAnchor, column), root());
            QModelIndex br = model()->index(model()->rowCount(root()) - 1,
                                            qMax(d->columnSectionAnchor, column), root());
            selectionModel()->select(QItemSelection(tl, br), command);
        }
    }
}

/*!
    Hide the given \a row.

    \sa showRow() hideColumn()
*/
void QTableView::hideRow(int row)
{
    d->verticalHeader->hideSection(row);
}

/*!
    Hide the given \a column.

    \sa showColumn() hideRow()
*/
void QTableView::hideColumn(int column)
{
    d->horizontalHeader->hideSection(column);
}

/*!
    Show the given \a row.

    \sa hideRow() showColumn()
*/
void QTableView::showRow(int row)
{
    d->verticalHeader->showSection(row);
}

/*!
    Show the given \a column.

    \sa hideColumn() showRow()
*/
void QTableView::showColumn(int column)
{
    d->horizontalHeader->showSection(column);
}

/*!
    \internal
*/
void QTableView::resizeRowToContents(int row)
{
    int content = rowSizeHint(row);
    int header = d->verticalHeader->isHidden() ? 0 : d->verticalHeader->sectionSizeHint(row);
    d->verticalHeader->resizeSection(row, qMax(content, header));
}

/*!
    \internal
*/
void QTableView::resizeColumnToContents(int column)
{
    int content = columnSizeHint(column);
    int header = d->horizontalHeader->isHidden() ? 0 : d->horizontalHeader->sectionSizeHint(column);
    d->horizontalHeader->resizeSection(column, qMax(content, header));
}

/*!
    \internal
*/
void QTableView::verticalScrollbarAction(int action)
{
    int factor = d->verticalFactor;
    int value = verticalScrollBar()->value();
    int row = value / factor;
    int above = (value % factor) * d->verticalHeader->sectionSize(row); // what's left; in "item units"
    int y = -(above / factor); // above the page

    if (action == QScrollBar::SliderPageStepAdd) {
        // go down to the bottom of the page
        int h = d->viewport->height();
        while (y < h && row < d->model->rowCount(root()))
            y += d->verticalHeader->sectionSize(row++);
        value = row * factor; // i is now the last item on the page
        if (y > h && row)
            value -= factor * (y - h) / d->verticalHeader->sectionSize(row - 1);
        verticalScrollBar()->setSliderPosition(value);
    } else if (action == QScrollBar::SliderPageStepSub) {
        y += d->viewport->height();
        // go up to the top of the page
        while (y > 0 && row > 0)
            y -= d->verticalHeader->sectionSize(--row);
        value = row * factor; // i is now the first item in the page
        if (y < 0)
            value += factor * -y / d->verticalHeader->sectionSize(row);
        verticalScrollBar()->setSliderPosition(value);
    }
}

/*!
    \internal
*/
void QTableView::horizontalScrollbarAction(int action)
{
    int factor = d->horizontalFactor;
    int value = horizontalScrollBar()->value();
    int column = value / factor;
    int above = (value % factor) * d->horizontalHeader->sectionSize(column); // what's left; in "item units"
    int x = -(above / factor); // above the page

    if (action == QScrollBar::SliderPageStepAdd) {
        // go down to the right of the page
        int w = d->viewport->width();
        while (x < w && column < d->model->columnCount(root()))
            x += d->horizontalHeader->sectionSize(column++);
        value = column * factor; // i is now the last item on the page
        if (x > w && column)
            value -= factor * (x - w) / d->horizontalHeader->sectionSize(column - 1);
        horizontalScrollBar()->setSliderPosition(value);

    } else if (action == QScrollBar::SliderPageStepSub) {
        x += d->viewport->width();
        // go up to the left of the page
        while (x > 0 && column > 0)
            x -= d->horizontalHeader->sectionSize(--column);
        value = column * factor; // i is now the first item in the page
        if (x < 0)
            value += factor * -x / d->horizontalHeader->sectionSize(column);
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
