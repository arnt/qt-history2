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
#include <qscrollbar.h>
#include <private/qtableview_p.h>

#define d d_func()
#define q q_func()

void QTableViewPrivate::init()
{
    q->setBeginEditActions(beginEditActions|QAbstractItemDelegate::AnyKeyPressed);

    QHeaderView *vertical = new QHeaderView(Qt::Vertical, q);
    vertical->setModel(model);
    vertical->setSelectionModel(selectionModel);
    vertical->setClickable(true);
    vertical->setHighlightCurrentSection(true);
    q->setVerticalHeader(vertical);

    QHeaderView *horizontal = new QHeaderView(Qt::Horizontal, q);
    horizontal->setModel(model);
    horizontal->setSelectionModel(selectionModel);
    horizontal->setClickable(true);
    horizontal->setHighlightCurrentSection(true);
    q->setHorizontalHeader(horizontal);
}

void QTableViewPrivate::updateVerticalScrollbar(int itemHeight)
{
    int factor = q->verticalFactor();
    int height = viewport->height();
    int count = model->rowCount(q->root());

    // if we have no viewport or no rows, there is nothing to do
    if (height <= 0 || count <= 0 || itemHeight <= 0) {
        q->verticalScrollBar()->setRange(0, 0);
        return;
    }

    // set page step size
    int visibleItems = height / itemHeight;
    int pageStepSize = visibleItems * factor;
    q->verticalScrollBar()->setPageStep(pageStepSize);

    // set the scroller range
    int y = height;
    int r = count;
    while (y > 0 && r > 0)
        y -= verticalHeader->sectionSize(--r);
    int max = r * factor;

    if (y < 0) { // if the last item starts above the viewport, we have to backtrack
        int backtracking = factor * -y;
        int sectionSize = verticalHeader->sectionSize(r);
        if (sectionSize > 0) // avoid division by zero
            max += (backtracking / sectionSize) + 1;
    }

    q->verticalScrollBar()->setRange(0, max);
}

void QTableViewPrivate::updateHorizontalScrollbar(int itemWidth)
{
    int factor = q->horizontalFactor();
    int width = viewport->width();
    int count = model->columnCount(q->root());

    // if we have no viewport or no columns, there is nothing to do
    if (width <= 0 || count <= 0 || itemWidth <= 0) {
        q->horizontalScrollBar()->setRange(0, 0);
        return;
    }

    // set page step size
    int visibleItems = width / itemWidth;
    int pageStepSize = visibleItems * factor;
    q->horizontalScrollBar()->setPageStep(pageStepSize);

    // set the scroller range
    int x = width;
    int c = count;
    while (x > 0 && c > 0)
        x -= horizontalHeader->sectionSize(--c);
    int max = c * factor;

    if (x < 0) { // if the last item starts left of the viewport, we have to backtrack
        int backtracking = factor * -x;
        int sectionSize = horizontalHeader->sectionSize(c);
        if (sectionSize > 0) // avoid division by zero
            max += (backtracking / sectionSize) + 1;
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
    convert between row and column indices and widget coordinates.
    The rowAt() function provides the y-coordinate within the view of the
    specified row; the row index can be used to obtain a corresponding
    y-coordinate with rowViewportPosition(). The columnAt() and
    columnViewportPosition() functions provide the equivalent conversion
    operations between x-coordinates and column indices.

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
    d->verticalHeader->setModel(model);
    d->horizontalHeader->setModel(model);
    QAbstractItemView::setModel(model);
}

/*!
  \reimp
*/
void QTableView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    d->verticalHeader->setSelectionModel(selectionModel);
    d->horizontalHeader->setSelectionModel(selectionModel);
    QAbstractItemView::setSelectionModel(selectionModel);
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
    if (d->horizontalHeader) {
        QObject::disconnect(d->horizontalHeader,SIGNAL(sectionSizeChanged(int,int,int)),
                            this, SLOT(columnWidthChanged(int,int,int)));
        QObject::disconnect(d->horizontalHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                            this, SLOT(columnIndexChanged(int,int,int)));
        QObject::disconnect(d->horizontalHeader, SIGNAL(sectionClicked(int,Qt::ButtonState)),
                            this, SLOT(selectColumn(int,Qt::ButtonState)));
        QObject::disconnect(d->horizontalHeader, SIGNAL(sectionCountChanged(int,int)),
                            this, SLOT(columnCountChanged(int,int)));
        QObject::disconnect(d->horizontalHeader, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                            this, SLOT(resizeColumnToContents(int)));
    }

    d->horizontalHeader = header;

    QObject::connect(d->horizontalHeader,SIGNAL(sectionSizeChanged(int,int,int)),
                     this, SLOT(columnWidthChanged(int,int,int)));
    QObject::connect(d->horizontalHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                     this, SLOT(columnIndexChanged(int,int,int)));
    QObject::connect(d->horizontalHeader, SIGNAL(sectionClicked(int,Qt::ButtonState)),
                     this, SLOT(selectColumn(int,Qt::ButtonState)));
    QObject::connect(d->horizontalHeader, SIGNAL(sectionCountChanged(int,int)),
                     this, SLOT(columnCountChanged(int,int)));
    QObject::connect(d->horizontalHeader, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                     this, SLOT(resizeColumnToContents(int)));
}

/*!
    Sets the widget to use for the horizontal header to \a header.

    \sa verticalHeader() setHorizontalHeader()
*/
void QTableView::setVerticalHeader(QHeaderView *header)
{
    if (d->verticalHeader) {
        QObject::disconnect(d->verticalHeader, SIGNAL(sectionSizeChanged(int,int,int)),
                            this, SLOT(rowHeightChanged(int,int,int)));
        QObject::disconnect(d->verticalHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                            this, SLOT(rowIndexChanged(int,int,int)));
        QObject::disconnect(d->verticalHeader, SIGNAL(sectionClicked(int,Qt::ButtonState)),
                            this, SLOT(selectRow(int,Qt::ButtonState)));
        QObject::disconnect(d->verticalHeader, SIGNAL(sectionCountChanged(int,int)),
                            this, SLOT(rowCountChanged(int,int)));
        QObject::disconnect(d->verticalHeader, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                            this, SLOT(resizeRowToContents(int)));
    }

    d->verticalHeader = header;

    QObject::connect(d->verticalHeader, SIGNAL(sectionSizeChanged(int,int,int)),
                     this, SLOT(rowHeightChanged(int,int,int)));
    QObject::connect(d->verticalHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                     this, SLOT(rowIndexChanged(int,int,int)));
    QObject::connect(d->verticalHeader, SIGNAL(sectionClicked(int,Qt::ButtonState)),
                     this, SLOT(selectRow(int,Qt::ButtonState)));
    QObject::connect(d->verticalHeader, SIGNAL(sectionCountChanged(int,int)),
                     this, SLOT(rowCountChanged(int,int)));
    QObject::connect(d->verticalHeader, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                     this, SLOT(resizeRowToContents(int)));
}

/*!
    \internal

    Scroll the contents of the table view by \a(dx, dy).
*/
void QTableView::scrollContentsBy(int dx, int dy)
{
    if (dx) { // horizontal
        int value = horizontalScrollBar()->value();
        int section = d->horizontalHeader->section(value / horizontalFactor());
        int left = (value % horizontalFactor()) * d->horizontalHeader->sectionSize(section);
        int offset = (left / horizontalFactor()) + d->horizontalHeader->sectionPosition(section);
        if (QApplication::reverseLayout()) {
            int delta = d->viewport->width() + d->horizontalHeader->sectionSize(section)
                        + d->viewport->x() - 2;
            dx = offset + d->horizontalHeader->offset();
            d->horizontalHeader->setOffset(offset - delta);
        } else {
            dx = d->horizontalHeader->offset() - offset;
            d->horizontalHeader->setOffset(offset);
        }
        horizontalScrollBar()->repaint();
    }

    if (dy) { // vertical
        int value = verticalScrollBar()->value();
        int section = d->verticalHeader->section(value / verticalFactor());
        int above = (value % verticalFactor()) * d->verticalHeader->sectionSize(section);
        int offset = (above / verticalFactor()) + d->verticalHeader->sectionPosition(section);
        dy = d->verticalHeader->offset() - offset;
        d->verticalHeader->setOffset(offset);
        verticalScrollBar()->repaint();
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

    QPainter painter(d->viewport);
    if (d->horizontalHeader->count() == 0 || d->verticalHeader->count() == 0) {
        painter.fillRect(area, base);
        return;
    }

    int colfirst = columnAt(area.left());
    int collast = columnAt(area.right() - 1);

    if (colfirst == -1)
        colfirst = 0;
    if (collast == -1)
        collast = d->model->columnCount(root()) - 1;
    if (collast < 0) {
        painter.fillRect(area, base);
        return;
    }
    if (colfirst > collast) {
        int tmp = colfirst;
        colfirst = collast;
        collast = tmp;
    }

    int rowfirst = rowAt(area.top());
    int rowlast = rowAt(area.bottom() - 1);

    if (rowfirst == -1)
        rowfirst = 0;
    if (rowlast == -1)
        rowlast = d->model->rowCount(root()) - 1;
    if (rowlast < 0) {
        painter.fillRect(area, base);
        return;
    }
    if (rowfirst > rowlast) {
        int tmp = rowfirst;
        rowfirst = rowlast;
        rowlast = tmp;
    }

    bool showGrid = d->showGrid;
    int gridSize = showGrid ? 1 : 0;
    int gridHint = style().styleHint(QStyle::SH_Table_GridLineColor, 0, this);
    QColor gridColor = gridHint != -1
                       ? static_cast<QRgb>(gridHint)
                       : palette().color(QPalette::Mid);
    QPen gridPen = QPen(gridColor, 0, d->gridStyle);

    QItemSelectionModel *sels = selectionModel();
    QHeaderView *verticalHeader = d->verticalHeader;
    QHeaderView *horizontalHeader = d->horizontalHeader;
    QModelIndex current = currentItem();
    bool focus = hasFocus() && current.isValid();
    QStyle::SFlags state = option.state;

    for (int r = rowfirst; r <= rowlast; ++r) {
        if (verticalHeader->isSectionHidden(r))
            continue;
        int rowp = rowViewportPosition(r);
        int rowh = rowHeight(r) - gridSize;
        for (int c = colfirst; c <= collast; ++c) {
            if (horizontalHeader->isSectionHidden(c))
                continue;
            int colp = columnViewportPosition(c);
            int colw = columnWidth(c) - gridSize;
            QModelIndex index = model()->index(r, c, root());
            if (index.isValid()) {
                option.rect = QRect(colp, rowp, colw, rowh);
                option.state = state;
                option.state |= (sels->isSelected(index)
                                 ? QStyle::Style_Selected : QStyle::Style_Default);
                option.state |= (focus && index == current
                                 ? QStyle::Style_HasFocus : QStyle::Style_Default);
                painter.fillRect(colp, rowp, colw, rowh,
                                 (option.state & QStyle::Style_Selected
                                  ? option.palette.highlight() : base));
                itemDelegate()->paint(&painter, option, model(), index);
            }
            if (r == rowfirst && showGrid) {
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
    int x = d->horizontalHeader->size();
    int y = d->verticalHeader->size();
    QRect bottom(0, y, w, h - y);
    QRect left(x, 0, w - x, h);
    if (y < h && area.intersects(bottom))
        painter.fillRect(bottom, base);
    if (x < w && area.intersects(left))
        painter.fillRect(left, base);
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
    return QModelIndex::Null;
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
    \fn QModelIndex QTableView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state)

    Moves the cursor in accordance with the given \a cursorAction, using the
    information provided by the button \a state.

    \sa QAbstractItemView::CursorAction
*/
QModelIndex QTableView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                          Qt::ButtonState)
{
    QModelIndex current = currentItem();
    int bottom = d->model->rowCount(root()) - 1;
    int right = d->model->columnCount(root()) - 1;
    switch (cursorAction) {
    case QAbstractItemView::MoveUp:
        if (current.row() > 0)
            return model()->index(current.row() - 1, current.column(), root());
        break;
    case QAbstractItemView::MoveDown:
        if (current.row() < bottom)
            return model()->index(current.row() + 1, current.column(), root());
        break;
    case QAbstractItemView::MoveLeft:
        if (current.column() > 0)
            return model()->index(current.row(), current.column() - 1, root());
        break;
    case QAbstractItemView::MoveRight:
        if (current.column() < right)
            return model()->index(current.row(), current.column() + 1, root());
        break;
    case QAbstractItemView::MoveHome:
        return model()->index(0, current.column(), root());
    case QAbstractItemView::MoveEnd:
        return model()->index(bottom, current.column(), root());
    case QAbstractItemView::MovePageUp: {
        int newRow = rowAt(itemViewportRect(current).top() - d->viewport->height());
        return model()->index(newRow <= bottom ? newRow : 0, current.column(), root());}
    case QAbstractItemView::MovePageDown: {
        int newRow = rowAt(itemViewportRect(current).bottom() + d->viewport->height());
        return model()->index(newRow >= 0 ? newRow : bottom, current.column(), root());}
    }
    return QModelIndex::Null;
}

/*!
    \fn void QTableView::setSelection(const QRect &rect,
    QItemSelectionModel::SelectionFlags flags)

    Selects the items within the given \a rect and in accordance with
    the specified selection \a flags.
*/
void QTableView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    QModelIndex tl = itemAt(rect.left(), rect.top());
    QModelIndex br = itemAt(rect.right(), rect.bottom());

    if (d->topLeft == tl && d->bottomRight == br)
        return;

    if (tl.isValid() && br.isValid()) {
        d->topLeft = tl;
        d->bottomRight = br;
    }

    selectionModel()->select(QItemSelection(tl, br, model()), command);
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
    int i;
    for (i = 0; i < selection.count(); ++i) {
        QItemSelectionRange r = selection.at(i);
        if (r.parent().isValid())
            continue;
        rangeTop = d->verticalHeader->index(r.top());
        rangeLeft = d->horizontalHeader->index(r.left());
        rangeBottom = d->verticalHeader->index(r.bottom());
        rangeRight = d->horizontalHeader->index(r.right());
        if (rangeTop < top)
            top = rangeTop;
        if (rangeLeft < left)
            left = rangeLeft;
        if (rangeBottom > bottom)
            bottom = rangeBottom;
        if (rangeRight > right)
            right = rangeRight;
    }

    int leftCol = d->horizontalHeader->section(left);
    int topRow = d->verticalHeader->section(top);
    int rightCol = d->horizontalHeader->section(right);
    int bottomRow = d->verticalHeader->section(bottom);

    int leftPos = columnViewportPosition(leftCol);
    int topPos = rowViewportPosition(topRow);
    int rightPos = columnViewportPosition(rightCol) + columnWidth(rightCol);
    int bottomPos = rowViewportPosition(bottomRow) + rowHeight(bottomRow);

    return QRect(leftPos, topPos, rightPos - leftPos, bottomPos - topPos);
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
    bool reverse = QApplication::reverseLayout();
    setViewportMargins(reverse ? 0 : width, height, reverse ? width : 0, 0);

    QRect vg = d->viewport->geometry();
//     if (QApplication::reverseLayout())
//         d->horizontalHeader->setOffset(vg.width() - topHint.width());
    
    int verticalLeft = reverse ? vg.right() : (vg.left() - width);
    d->verticalHeader->setGeometry(verticalLeft, vg.top(), width, vg.height());

    int horizontalTop = vg.top() - height;
    d->horizontalHeader->setGeometry(vg.left(), horizontalTop, vg.width(), height);

    if (d->model) {
        d->updateVerticalScrollbar(d->verticalHeader->sectionSize(0));
        d->updateHorizontalScrollbar(d->horizontalHeader->sectionSize(0));
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
        hint = qMax(hint, itemDelegate()->sizeHint(fontMetrics(), option, d->model, index).height());
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
        hint = qMax(hint, itemDelegate()->sizeHint(fontMetrics(), option, d->model, index).width());
    }
    return d->showGrid ? hint + 1 : hint;
}

/*!
    Returns the x-coordinate in contents coordinates of the given \a
    row.
*/

int QTableView::rowViewportPosition(int row) const
{
    return d->verticalHeader->sectionPosition(row) - d->verticalHeader->offset();
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
    return d->verticalHeader->sectionAt(y + d->verticalHeader->offset());
}

/*!
    Returns the y-coordinate in contents coordinates of the given \a
    column.
*/

int QTableView::columnViewportPosition(int column) const
{
    int colp = d->horizontalHeader->sectionPosition(column) - d->horizontalHeader->offset();
    if (!QApplication::reverseLayout())
        return colp;
    return colp + (d->horizontalHeader->x() - d->viewport->x());
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
    int p = x + d->horizontalHeader->offset();
    if (!QApplication::reverseLayout())
        return d->horizontalHeader->sectionAt(p);
    return d->horizontalHeader->sectionAt(p - (d->horizontalHeader->x() - d->viewport->x()));
}

/*!
    Returns true if the given \a row is hidden; otherwise returns false.
*/

bool QTableView::isRowHidden(int row) const
{
    return d->verticalHeader->isSectionHidden(row);
}

/*!
    Returns true if the given \a column is hidden; otherwise returns false.
*/

bool QTableView::isColumnHidden(int column) const
{
    return d->horizontalHeader->isSectionHidden(column);
}

// ### DOC: What is the default?
/*!
    \property QTableView::showGrid
    \brief whether the grid is shown

    If this property is true a grid is drawn for the table; if the
    property is false, no grid is drawn.
*/

void QTableView::setShowGrid(bool show)
{
    d->showGrid = show;
}

bool QTableView::showGrid() const
{
    return d->showGrid;
}

/*!
    Sets the style used by the grid (see \l{showGrid}) to the given
    pen \a style.
*/

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
    if (!index.isValid() || model()->parent(index) != root())
        return QRect();
    return QRect(columnViewportPosition(index.column()), rowViewportPosition(index.row()),
                 columnWidth(index.column()), rowHeight(index.row()));
}

/*!
    \internal

    Makes sure that the given \a item is visible in the table view,
    scrolling if necessary.
*/

void QTableView::ensureItemVisible(const QModelIndex &index)
{
    QRect area = d->viewport->rect();
    QRect rect = itemViewportRect(index);

    if (model()->parent(index) != root())
        return;

    if (area.contains(rect)) {
        d->viewport->repaint(rect);
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
        int a = (-y * verticalFactor()) / rowHeight(r);
        verticalScrollBar()->setValue(++r * verticalFactor() + a);
    }

    // horizontal
    if (rect.left() < area.left()) { // left of
        horizontalScrollBar()->setValue(index.column() * horizontalFactor());
    } else if (rect.right() > area.right()) { // right of
        int c = index.column();
        int x = area.width();
        while (x > 0 && c > 0)
            x -= columnWidth(c--);
        int a = (-x * horizontalFactor()) / columnWidth(c);
        horizontalScrollBar()->setValue(++c * horizontalFactor() + a);
    }
}

/*!
    This slot is called to change the height of the given \a row. The
    old height is specified by \a oldHeight, and the new height by \a
    newHeight.

    \sa columnWidthChanged()
*/

void QTableView::rowHeightChanged(int row, int, int)
{
    int y = rowViewportPosition(row);
    d->viewport->update(QRect(0, y, d->viewport->width(), d->viewport->height() - y));
    updateGeometries();
}

/*!
    This slot is called to change the width of the given \a column.
    The old width is specified by \a oldWidth, and the new width by \a
    newWidth.

    \sa rowHeightChanged()
*/

void QTableView::columnWidthChanged(int column, int, int)
{
    bool reverse = QApplication::reverseLayout();
    int x = columnViewportPosition(column) - (reverse ? columnWidth(column) : 0);
    QRect rect(x, 0, d->viewport->width() - x, d->viewport->height());
    d->viewport->update(rect.normalize());
    updateGeometries();
}

/*!
    This slot is called to change the index of the given \a row in the
    table view. The old index is specified by \a oldIndex, and the new
    index by \a newIndex.

    \sa columnIndexChanged()
*/

void QTableView::rowIndexChanged(int, int oldIndex, int newIndex)
{
    int o = rowViewportPosition(d->verticalHeader->section(oldIndex));
    int n = rowViewportPosition(d->verticalHeader->section(newIndex));
    int top = (o < n ? o : n);
    int height = d->viewport->height() - (o > n ? o : n);
    updateGeometries();
    d->viewport->update(0, top, d->viewport->width(), height);
}

/*!
    This slot is called to change the index of the given \a column in
    the table view. The old index is specified by \a oldIndex, and
    the new index by \a newIndex.

    \sa rowIndexChanged()
*/

void QTableView::columnIndexChanged(int, int oldIndex, int newIndex)
{
    int o = columnViewportPosition(d->horizontalHeader->section(oldIndex));
    int n = columnViewportPosition(d->horizontalHeader->section(newIndex));
    int left = (o < n ? o : n);
    int width = d->viewport->width() - (o > n ? o : n);
    updateGeometries();
    d->viewport->update(left, 0, width, d->viewport->height());
}

/*!
    This slot is called to select the given \a row in accordance with
    the given button \a state.

    \sa selectColumn()
*/

void QTableView::selectRow(int row, Qt::ButtonState state)
{
    if (row >= 0 && row < d->model->rowCount(root())) {
        QModelIndex tl = d->model->index(row, 0, root());
        QModelIndex br = d->model->index(row, d->model->columnCount(root()) - 1, root());
        selectionModel()->setCurrentItem(tl, QItemSelectionModel::NoUpdate);
        if (d->selectionMode == SingleSelection)
            selectionModel()->select(tl, selectionCommand(state, tl));
        else
            selectionModel()->select(QItemSelection(tl, br, model()), selectionCommand(state, tl));
    }
}

/*!
    This slot is called to select the given \a column in accordance with
    the given button \a state.

    \sa selectRow()
*/

void QTableView::selectColumn(int column, Qt::ButtonState state)
{
    if (column >= 0 && column < d->model->columnCount(root())) {
        QModelIndex tl = model()->index(0, column, root());
        QModelIndex br = model()->index(d->model->rowCount(root()) - 1, column, root());
        selectionModel()->setCurrentItem(tl, QItemSelectionModel::NoUpdate);
        if (d->selectionMode == SingleSelection)
            selectionModel()->select(tl, selectionCommand(state, tl));
        else
            selectionModel()->select(QItemSelection(tl, br, model()), selectionCommand(state, tl));
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

void QTableView::resizeRowToContents(int row, bool checkHeader)
{
    int content = rowSizeHint(row);
    int header = checkHeader ? d->verticalHeader->sectionSizeHint(row) : 0;
    d->verticalHeader->resizeSection(row, qMax(content, header));
}

/*!
    \internal
*/

void QTableView::resizeColumnToContents(int column, bool checkHeader)
{
    int content = columnSizeHint(column);
    int header = checkHeader ? d->horizontalHeader->sectionSizeHint(column) : 0;
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
