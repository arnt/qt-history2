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

#include "qgenerictableview.h"
#include "qgenericheader.h"
#include <qitemdelegate.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qstyle.h>
#include <qsize.h>
#include <qscrollbar.h>
#include <private/qgenerictableview_p.h>

#define d d_func()
#define q q_func()

/*!
    \class QGenericTableView qgenerictableview.h

    \brief The QGenericTableView class provides a default model/view
    implementation of a table view.

    \ingroup model-view

    This class implements a table representation of a QGenericItemView working
    on a QAbstractItemModel.

    The table has a vertical header available from leftHeader() and a
    horizontal header available from topHeader(). Rows have a
    rowHeight(), and a y-coordinate can be mapped to a row using
    rowAt() and a row can be mapped to a contents coordinate with
    rowViewportPosition(); columns are similar with columnWidth(),
    columnAt(), and columnViewportPosition(). Rows and columns can be
    hidden and shown with hideRow(), hideColumn(), showRow(), and
    showColumn(). They can be selected with selectRow() and
    selectColumn(). The table will show a grid depending on the \l
    showGrid property.

    \sa \link model-view-programming.html Model/View Programming\endlink.
*/

// ### DOC: Where does the model come from?
/*!
    Constructs a table view with a \a parent to represent the data.

    \sa QAbstractItemModel
*/

QGenericTableView::QGenericTableView(QWidget *parent)
    : QAbstractItemView(*new QGenericTableViewPrivate, parent)
{
    setBeginEditActions(beginEditActions()|QAbstractItemDelegate::AnyKeyPressed);

    QGenericHeader *left = new QGenericHeader(Qt::Vertical, this);
    left->setModel(model());
    left->setSelectionModel(selectionModel());
    left->setClickable(true);
    setLeftHeader(left);

    QGenericHeader *top = new QGenericHeader(Qt::Horizontal, this);
    top->setModel(model());
    top->setSelectionModel(selectionModel());
    top->setClickable(true);
    setTopHeader(top);
}

/*!
  \internal
*/
QGenericTableView::QGenericTableView(QGenericTableViewPrivate &dd, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    setBeginEditActions(beginEditActions()|QAbstractItemDelegate::AnyKeyPressed);
    setLeftHeader(new QGenericHeader(Qt::Vertical, this));
    d->leftHeader->setClickable(true);
    setTopHeader(new QGenericHeader(Qt::Horizontal, this));
    d->topHeader->setClickable(true);
}

/*!
  Destroys the table view.
*/
QGenericTableView::~QGenericTableView()
{
}

/*!
  \reimp
*/
void QGenericTableView::setModel(QAbstractItemModel *model)
{
    d->leftHeader->setModel(model);
    d->topHeader->setModel(model);
    QAbstractItemView::setModel(model);
}

/*!
  \reimp
*/
void QGenericTableView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    d->leftHeader->setSelectionModel(selectionModel);
    d->topHeader->setSelectionModel(selectionModel);
    QAbstractItemView::setSelectionModel(selectionModel);
}

/*!
    Returns the table view's horizontal header.

    \sa setTopHeader() leftHeader()
*/

QGenericHeader *QGenericTableView::topHeader() const
{
    return d->topHeader;
}

/*!
    Returns the table view's vertical header.

    \sa setLeftHeader() topHeader()
*/
QGenericHeader *QGenericTableView::leftHeader() const
{
    return d->leftHeader;
}

/*!
    Sets the widget to use for the vertical header to \a header.

    \sa topHeader() setLeftHeader()
*/
void QGenericTableView::setTopHeader(QGenericHeader *header)
{
    if (d->topHeader) {
        QObject::disconnect(d->topHeader,SIGNAL(sectionSizeChanged(int,int,int)),
                            this, SLOT(columnWidthChanged(int,int,int)));
        QObject::disconnect(d->topHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                            this, SLOT(columnIndexChanged(int,int,int)));
        QObject::disconnect(d->topHeader, SIGNAL(sectionClicked(int,Qt::ButtonState)),
                            this, SLOT(selectColumn(int,Qt::ButtonState)));
        QObject::disconnect(d->topHeader, SIGNAL(sectionCountChanged(int,int)),
                            this, SLOT(columnCountChanged(int,int)));
        QObject::disconnect(d->topHeader, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                            this, SLOT(resizeColumnToContents(int)));
    }

    d->topHeader = header;

    QObject::connect(d->topHeader,SIGNAL(sectionSizeChanged(int,int,int)),
                     this, SLOT(columnWidthChanged(int,int,int)));
    QObject::connect(d->topHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                     this, SLOT(columnIndexChanged(int,int,int)));
    QObject::connect(d->topHeader, SIGNAL(sectionClicked(int,Qt::ButtonState)),
                     this, SLOT(selectColumn(int,Qt::ButtonState)));
    QObject::connect(d->topHeader, SIGNAL(sectionCountChanged(int,int)),
                     this, SLOT(columnCountChanged(int,int)));
    QObject::connect(d->topHeader, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                     this, SLOT(resizeColumnToContents(int)));
}

/*!
    Sets the widget to use for the horizontal header to \a header.

    \sa leftHeader() setTopHeader()
*/
void QGenericTableView::setLeftHeader(QGenericHeader *header)
{
    if (d->leftHeader) {
        QObject::disconnect(d->leftHeader, SIGNAL(sectionSizeChanged(int,int,int)),
                            this, SLOT(rowHeightChanged(int,int,int)));
        QObject::disconnect(d->leftHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                            this, SLOT(rowIndexChanged(int,int,int)));
        QObject::disconnect(d->leftHeader, SIGNAL(sectionClicked(int,Qt::ButtonState)),
                            this, SLOT(selectRow(int,Qt::ButtonState)));
        QObject::disconnect(d->leftHeader, SIGNAL(sectionCountChanged(int,int)),
                            this, SLOT(rowCountChanged(int,int)));
        QObject::disconnect(d->leftHeader, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                            this, SLOT(resizeRowToContents(int)));
    }

    d->leftHeader = header;

    QObject::connect(d->leftHeader, SIGNAL(sectionSizeChanged(int,int,int)),
                     this, SLOT(rowHeightChanged(int,int,int)));
    QObject::connect(d->leftHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                     this, SLOT(rowIndexChanged(int,int,int)));
    QObject::connect(d->leftHeader, SIGNAL(sectionClicked(int,Qt::ButtonState)),
                     this, SLOT(selectRow(int,Qt::ButtonState)));
    QObject::connect(d->leftHeader, SIGNAL(sectionCountChanged(int,int)),
                     this, SLOT(rowCountChanged(int,int)));
    QObject::connect(d->leftHeader, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                     this, SLOT(resizeRowToContents(int)));
}

/*!
    \internal

    Scroll the contents of the table view by \a(dx, dy).
*/
void QGenericTableView::scrollContentsBy(int dx, int dy)
{
    if (dx) { // horizontal
        int value = horizontalScrollBar()->value();
        int section = d->topHeader->section(value / horizontalFactor());
        int left = (value % horizontalFactor()) * d->topHeader->sectionSize(section);
        int offset = (left / horizontalFactor()) + d->topHeader->sectionPosition(section);
        if (QApplication::reverseLayout()) {
            int delta = d->viewport->width() + d->topHeader->sectionSize(section)
                        + d->viewport->x() - 2;
            dx = offset + d->topHeader->offset();
            d->topHeader->setOffset(offset - delta);
        } else {
            dx = d->topHeader->offset() - offset;
            d->topHeader->setOffset(offset);
        }
        horizontalScrollBar()->repaint();
    }
    
    if (dy) { // vertical
        int value = verticalScrollBar()->value();
        int section = d->leftHeader->section(value / verticalFactor());
        int above = (value % verticalFactor()) * d->leftHeader->sectionSize(section);
        int offset = (above / verticalFactor()) + d->leftHeader->sectionPosition(section);
        dy = d->leftHeader->offset() - offset;
        d->leftHeader->setOffset(offset);
        verticalScrollBar()->repaint();
    }
    
    d->viewport->scroll(dx, dy);
}

/*!
    Paints the table on receipt of the given paint event \a e.
*/
void QGenericTableView::paintEvent(QPaintEvent *e)
{
    QStyleOptionViewItem option = viewOptions();
    QBrush base = option.palette.base();
    QRect area = e->rect();

    if (d->topHeader->count() == 0 || d->leftHeader->count() == 0) {
        QPainter painter(d->viewport);
        painter.fillRect(area, base);
        return;
    }

    QPainter painter(&d->backBuffer);

    int colfirst = columnAt(area.left());
    int collast = columnAt(area.right() - 1);

    if (colfirst == -1)
        colfirst = 0;
    if (collast == -1)
        collast = model()->columnCount(root()) - 1;
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
        rowlast = model()->rowCount(root()) - 1;
    if (rowfirst > rowlast) {
        int tmp = rowfirst;
        rowfirst = rowlast;
        rowlast = tmp;
    }

    bool showGrid = d->showGrid;
    int gridSize = showGrid ? 1 : 0;
    int gridHint = style().styleHint(QStyle::SH_Table_GridLineColor, this);
    QColor gridColor = gridHint != -1
                       ? static_cast<QRgb>(gridHint)
                       : palette().color(QPalette::Mid);
    QPen gridPen = QPen(gridColor, 0, d->gridStyle);

    QItemSelectionModel *sels = selectionModel();
    QGenericHeader *leftHeader = d->leftHeader;
    QGenericHeader *topHeader = d->topHeader;
    QModelIndex current = currentItem();
    bool focus = hasFocus() && current.isValid();
    QStyle::SFlags state = option.state;
    
    for (int r = rowfirst; r <= rowlast; ++r) {
        if (leftHeader->isSectionHidden(r))
            continue;
        int rowp = rowViewportPosition(r);
        int rowh = rowHeight(r) - gridSize;
        for (int c = colfirst; c <= collast; ++c) {
            if (topHeader->isSectionHidden(c))
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
    int x = d->topHeader->size();
    int y = d->leftHeader->size();
    QRect bottom(0, y, w, h - y);
    QRect left(x, 0, w - x, h);
    if (y < h && area.intersects(bottom))
        painter.fillRect(bottom, base);
    if (x < w && area.intersects(left))
        painter.fillRect(left, base);

    painter.end();
    painter.begin(d->viewport);
    painter.drawPixmap(area.topLeft(), d->backBuffer, area);
}

/*!
    Returns the index position of the model item corresponding to the
    table item at position (\a x, \a y) in contents coordinates.
*/
QModelIndex QGenericTableView::itemAt(int x, int y) const
{
    return model()->index(rowAt(y), columnAt(x), root());
}

/*!
    Returns the horizontal offset of the items in the table view.

    \sa verticalOffset()
*/
int QGenericTableView::horizontalOffset() const
{
    return d->topHeader->offset();
}

/*!
    Returns the vertical offset of the items in the table view.

    \sa horizontalOffset()
*/
int QGenericTableView::verticalOffset() const
{
    return d->leftHeader->offset();
}

/*!
    \fn QModelIndex QGenericTableView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state)

    Moves the cursor in accordance with the given \a cursorAction, using the
    information provided by the button \a state.

    \sa QAbstractItemView::CursorAction
*/
QModelIndex QGenericTableView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                          Qt::ButtonState)
{
    QModelIndex current = currentItem();
    QModelIndex bottomRight = model()->bottomRight(root());
    switch (cursorAction) {
    case QAbstractItemView::MoveUp:
        if (current.row() > 0)
            return model()->index(current.row() - 1, current.column(), root());
        break;
    case QAbstractItemView::MoveDown:
        if (current.row() < bottomRight.row())
            return model()->index(current.row() + 1, current.column(), root());
        break;
    case QAbstractItemView::MoveLeft:
        if (current.column() > 0)
            return model()->index(current.row(), current.column() - 1, root());
        break;
    case QAbstractItemView::MoveRight:
        if (current.column() < bottomRight.column())
            return model()->index(current.row(), current.column() + 1, root());
        break;
    case QAbstractItemView::MoveHome:
        return model()->index(0, current.column(), root());
    case QAbstractItemView::MoveEnd:
        return model()->index(bottomRight.row(), current.column(), root());
    case QAbstractItemView::MovePageUp: {
        int newRow = rowAt(itemViewportRect(current).top() - d->viewport->height());
        return model()->index(newRow <= bottomRight.row() ? newRow : 0, current.column(), root());
    }
    case QAbstractItemView::MovePageDown: {
        int newRow = rowAt(itemViewportRect(current).bottom() + d->viewport->height());
        return model()->index(newRow >= 0 ? newRow : bottomRight.row(), current.column(), root());
    }}
    return QModelIndex();
}

/*!
    \fn void QGenericTableView::setSelection(const QRect &rect,
    QItemSelectionModel::SelectionFlags flags)

    Selects the items within the given \a rect and in accordance with
    the specified selection \a flags.
*/
void QGenericTableView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
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
QRect QGenericTableView::selectionViewportRect(const QItemSelection &selection) const
{
    QModelIndex bottomRight = model()->bottomRight(root());
    int top = bottomRight.row();
    int left = bottomRight.column();
    int bottom = 0;
    int right = 0;
    int rangeTop, rangeLeft, rangeBottom, rangeRight;
    int i;
    for (i = 0; i < selection.count(); ++i) {
        QItemSelectionRange r = selection.at(i);
        if (r.parent().isValid())
            continue;
        rangeTop = d->leftHeader->index(r.top());
        rangeLeft = d->topHeader->index(r.left());
        rangeBottom = d->leftHeader->index(r.bottom());
        rangeRight = d->topHeader->index(r.right());
        if (rangeTop < top)
            top = rangeTop;
        if (rangeLeft < left)
            left = rangeLeft;
        if (rangeBottom > bottom)
            bottom = rangeBottom;
        if (rangeRight > right)
            right = rangeRight;
    }

    int leftCol = d->topHeader->section(left);
    int topRow = d->leftHeader->section(top);
    int rightCol = d->topHeader->section(right);
    int bottomRow = d->leftHeader->section(bottom);

    int leftPos = columnViewportPosition(leftCol);
    int topPos = rowViewportPosition(topRow);
    int rightPos = columnViewportPosition(rightCol) + columnWidth(rightCol);
    int bottomPos = rowViewportPosition(bottomRow) + rowHeight(bottomRow);

    return QRect(leftPos, topPos, rightPos - leftPos, bottomPos - topPos);
}

/*!
    This slot is called whenever rows are added or deleted. The
    previous number of rows is given by \a oldCount, and the new
    number of rows is given by \a newCount.
*/

void QGenericTableView::rowCountChanged(int, int)
{
    updateGeometries();
    d->viewport->update();
}

/*!
    This slot is called whenever columns are added or deleted. The
    previous number of columns is given by \a oldCount, and the new
    number of columns is given by \a newCount.
*/

void QGenericTableView::columnCountChanged(int, int)
{
    updateGeometries();
    d->viewport->update();
}

/*!
    \internal
*/

void QGenericTableView::updateGeometries()
{
    int width = d->leftHeader->sizeHint().width();
    QSize topHint = d->topHeader->sizeHint();

    bool reverse = QApplication::reverseLayout();
    setViewportMargins(reverse ? 0 : width, topHint.height(), reverse ? width : 0, 0);

    QRect vg = d->viewport->geometry();
    if (QApplication::reverseLayout())
        d->topHeader->setOffset(vg.width() - topHint.width());
    d->leftHeader->setGeometry(reverse ? vg.right() : (vg.left() - width), vg.top(),
                               width, vg.height());
    d->topHeader->setGeometry(vg.left(), vg.top() - topHint.height(),
                              vg.width(), topHint.height());

    if (!d->model)
        return;

    // update sliders    
    QStyleOptionViewItem option = viewOptions();

    int h = d->viewport->height();
    int row = d->model->rowCount();
    if (h <= 0 || row <= 0) // if we have no viewport or no rows, there is nothing to do
        return;
    QSize def = itemDelegate()->sizeHint(fontMetrics(), option, d->model, d->model->index(0, 0));
    verticalScrollBar()->setPageStep(h / def.height() * verticalFactor());
    while (h > 0 && row > 0)
        h -= d->leftHeader->sectionSize(--row);
    int max = row * verticalFactor();
    if (h < 0)
        max += 1 + (verticalFactor() * -h / d->leftHeader->sectionSize(row));
    verticalScrollBar()->setRange(0, max);

    int w = d->viewport->width();
    int col = model()->columnCount();
    int factor = horizontalFactor();
    if (def.width() && factor)
        horizontalScrollBar()->setPageStep(w / def.width() * factor);
    while (w > 0 && col > 0)
        w -= d->topHeader->sectionSize(--col);
    max = col * factor;
    if (w < 0)
        max += (factor * -w / d->topHeader->sectionSize(col));
    horizontalScrollBar()->setRange(0, max);
}

/*!
    Returns the size hint for the given \a row's height.

    \sa QWidget::sizeHint
*/
int QGenericTableView::rowSizeHint(int row) const
{
    int columnfirst = columnAt(0);
    int columnlast = columnAt(d->viewport->width());
    if (columnlast < 0)
        columnlast = d->topHeader->count() - 1;

    QStyleOptionViewItem option = viewOptions();

    int hint = 0;
    QModelIndex index;
    for (int column = columnfirst; column < columnlast; ++column) {
        index = d->model->index(row, column, root());
        hint = qMax(hint, itemDelegate()->sizeHint(fontMetrics(), option, d->model, index).height());
    }
    return hint + 1; // add space for the grid
}

/*!
    Returns the size hint for the given \a column's width.

    \sa QWidget::sizeHint
*/
int QGenericTableView::columnSizeHint(int column) const
{
    int rowfirst = rowAt(0);
    int rowlast = rowAt(d->viewport->height());
    if (rowlast < 0)
        rowlast = d->leftHeader->count() - 1;

    QStyleOptionViewItem option = viewOptions();

    int hint = 0;
    QModelIndex index;
    for (int row = rowfirst; row <= rowlast; ++row) {
        index = d->model->index(row, column, root());
        hint = qMax(hint, itemDelegate()->sizeHint(fontMetrics(), option, d->model, index).width());
    }
    return hint + 1; // add space for the grid
}

/*!
    Returns the x-coordinate in contents coordinates of the given \a
    row.
*/

int QGenericTableView::rowViewportPosition(int row) const
{
    return d->leftHeader->sectionPosition(row) - d->leftHeader->offset();
}

/*!
    Returns the height of the given \a row.
*/

int QGenericTableView::rowHeight(int row) const
{
    return d->leftHeader->sectionSize(row);
}

/*!
    Returns the row in which the given y-coordinate, \a y, in contents
    coordinates is located.
*/

int QGenericTableView::rowAt(int y) const
{
    return d->leftHeader->sectionAt(y + d->leftHeader->offset());
}

/*!
    Returns the y-coordinate in contents coordinates of the given \a
    column.
*/

int QGenericTableView::columnViewportPosition(int column) const
{
    int colp = d->topHeader->sectionPosition(column) - d->topHeader->offset();
    if (!QApplication::reverseLayout())
        return colp;
    return colp + (d->topHeader->x() - d->viewport->x());
}

/*!
    Returns the width of the given \a column.
*/

int QGenericTableView::columnWidth(int column) const
{
    return d->topHeader->sectionSize(column);
}

/*!
    Returns the column in which the given x-coordinate, \a x, in contents
    coordinates is located.
*/

int QGenericTableView::columnAt(int x) const
{
    int p = x + d->topHeader->offset();
    if (!QApplication::reverseLayout())
        return d->topHeader->sectionAt(p);
    return d->topHeader->sectionAt(p - (d->topHeader->x() - d->viewport->x()));
}

/*!
    Returns true if the given \a row is hidden; otherwise returns false.
*/

bool QGenericTableView::isRowHidden(int row) const
{
    return d->leftHeader->isSectionHidden(row);
}

/*!
    Returns true if the given \a column is hidden; otherwise returns false.
*/

bool QGenericTableView::isColumnHidden(int column) const
{
    return d->topHeader->isSectionHidden(column);
}

// ### DOC: What is the default?
/*!
    \property QGenericTableView::showGrid
    \brief whether the grid is shown

    If this property is true a grid is drawn for the table; if the
    property is false no grid is drawn.
*/

void QGenericTableView::setShowGrid(bool show)
{
    d->showGrid = show;
}

bool QGenericTableView::showGrid() const
{
    return d->showGrid;
}

/*!
    Sets the style used by the grid (see \l{showGrid}) to the given
    pen \a style.
*/

void QGenericTableView::setGridStyle(Qt::PenStyle style)
{
    d->gridStyle = style;
}

/*!
    \internal

    Returns the rectangle on the viewport occupied by the given \a
    item.
*/

QRect QGenericTableView::itemViewportRect(const QModelIndex &item) const
{
    if (!item.isValid() || model()->parent(item) != root())
        return QRect();
    return QRect(columnViewportPosition(item.column()), rowViewportPosition(item.row()),
                 columnWidth(item.column()), rowHeight(item.row()));
}

/*!
    \internal

    Makes sure that the given \a item is visible in the table view,
    scrolling if necessary.
*/

void QGenericTableView::ensureItemVisible(const QModelIndex &item)
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
    if (rect.top() < area.top()) { // above
        verticalScrollBar()->setValue(item.row() * verticalFactor());
    } else if (rect.bottom() > area.bottom()) { // below
        int r = item.row();
        int y = area.height();
        while (y > 0 && r > 0)
            y -= rowHeight(r--);
        int a = (-y * verticalFactor()) / rowHeight(r);
        verticalScrollBar()->setValue(++r * verticalFactor() + a);
    }

    // horizontal
    if (rect.left() < area.left()) { // left of
        horizontalScrollBar()->setValue(item.column() * horizontalFactor());
    } else if (rect.right() > area.right()) { // right of
        int c = item.column();
        int x = area.width();
        while (x > 0 && c > 0)
            x -= columnWidth(c--);
        int a = (-x * horizontalFactor()) / columnWidth(c);
        horizontalScrollBar()->setValue(++c * horizontalFactor() + a);
    }
}

/*!
    This slot is called to change the height of the given \a row. The
    old height is given by \a oldHeight, and the new height by \a
    newHeight.

    \sa columnWidthChanged()
*/

void QGenericTableView::rowHeightChanged(int row, int, int)
{
    int y = rowViewportPosition(row);
    d->viewport->update(QRect(0, y, d->viewport->width(), d->viewport->height() - y));
    updateGeometries();
    updateEditors();
}

/*!
    This slot is called to change the width of the given \a column.
    The old width is given by \a oldWidth, and the new width by \a
    newWidth.

    \sa rowHeightChanged()
*/

void QGenericTableView::columnWidthChanged(int column, int, int)
{
    bool reverse = QApplication::reverseLayout();
    int x = columnViewportPosition(column) - (reverse ? columnWidth(column) : 0);
    QRect rect(x, 0, d->viewport->width() - x, d->viewport->height());
    d->viewport->update(rect.normalize());
    updateGeometries();
    updateEditors();
}

/*!
    This slot is called to change the index of the given \a row in the
    table view. The old index is given by \a oldIndex, and the new
    index by \a newIndex.

    \sa columnIndexChanged()
*/

void QGenericTableView::rowIndexChanged(int, int oldIndex, int newIndex)
{
    int o = rowViewportPosition(d->leftHeader->section(oldIndex));
    int n = rowViewportPosition(d->leftHeader->section(newIndex));
    int top = (o < n ? o : n);
    int height = d->viewport->height() - (o > n ? o : n);
    updateGeometries();
    d->viewport->update(0, top, d->viewport->width(), height);
}

/*!
    This slot is called to change the index of the given \a column in
    the table view. The old index is given by \a oldIndex, and
    the new index by \a newIndex.

    \sa rowIndexChanged()
*/

void QGenericTableView::columnIndexChanged(int, int oldIndex, int newIndex)
{
    int o = columnViewportPosition(d->topHeader->section(oldIndex));
    int n = columnViewportPosition(d->topHeader->section(newIndex));
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

void QGenericTableView::selectRow(int row, Qt::ButtonState state)
{
    if (row >= 0 && row < model()->rowCount()) {
        QModelIndex tl = model()->index(row, 0, root());
        QModelIndex br = model()->index(row, model()->columnCount() - 1, root());
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

void QGenericTableView::selectColumn(int column, Qt::ButtonState state)
{
    if (column >= 0 && column < model()->columnCount()) {
        QModelIndex tl = model()->index(0, column, root());
        QModelIndex br = model()->index(model()->rowCount() - 1, column, root());
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

void QGenericTableView::hideRow(int row)
{
    d->leftHeader->hideSection(row);
}

/*!
    Hide the given \a column.

    \sa showColumn() hideRow()
*/

void QGenericTableView::hideColumn(int column)
{
    d->topHeader->hideSection(column);
}

/*!
    Show the given \a row.

    \sa hideRow() showColumn()
*/

void QGenericTableView::showRow(int row)
{
    d->leftHeader->showSection(row);
}

/*!
    Show the given \a column.

    \sa hideColumn() showRow()
*/

void QGenericTableView::showColumn(int column)
{
    d->topHeader->showSection(column);
}

/*!
    \internal
*/

void QGenericTableView::resizeRowToContents(int row, bool checkHeader)
{
    int content = rowSizeHint(row);
    int header = checkHeader ? d->leftHeader->sectionSizeHint(row) : 0;
    d->leftHeader->resizeSection(row, qMax(content, header));
}

/*!
    \internal
*/

void QGenericTableView::resizeColumnToContents(int column, bool checkHeader)
{
    int content = columnSizeHint(column);
    int header = checkHeader ? d->topHeader->sectionSizeHint(column) : 0;
    d->topHeader->resizeSection(column, qMax(content, header));
}

/*!
    \internal
*/

void QGenericTableView::verticalScrollbarAction(int action)
{
    int factor = d->verticalFactor;
    int value = verticalScrollBar()->value();
    int row = value / factor;
    int above = (value % factor) * d->leftHeader->sectionSize(row); // what's left; in "item units"
    int y = -(above / factor); // above the page

    if (action == QScrollBar::SliderPageStepAdd) {
        // go down to the bottom of the page
        int h = d->viewport->height();
        while (y < h && row < d->model->rowCount())
            y += d->leftHeader->sectionSize(row++);
        value = row * factor; // i is now the last item on the page
        if (y > h && row)
            value -= factor * (y - h) / d->leftHeader->sectionSize(row - 1);
        verticalScrollBar()->setSliderPosition(value);
    } else if (action == QScrollBar::SliderPageStepSub) {
        y += d->viewport->height();
        // go up to the top of the page
        while (y > 0 && row > 0)
            y -= d->leftHeader->sectionSize(--row);
        value = row * factor; // i is now the first item in the page
        if (y < 0)
            value += factor * -y / d->leftHeader->sectionSize(row);
        verticalScrollBar()->setSliderPosition(value);
    }
}

/*!
    \internal
*/

void QGenericTableView::horizontalScrollbarAction(int action)
{
    int factor = d->horizontalFactor;
    int value = horizontalScrollBar()->value();
    int column = value / factor;
    int above = (value % factor) * d->topHeader->sectionSize(column); // what's left; in "item units"
    int x = -(above / factor); // above the page

    if (action == QScrollBar::SliderPageStepAdd) {
        // go down to the right of the page
        int w = d->viewport->width();
        while (x < w && column < d->model->columnCount())
            x += d->topHeader->sectionSize(column++);
        value = column * factor; // i is now the last item on the page
        if (x > w && column)
            value -= factor * (x - w) / d->topHeader->sectionSize(column - 1);
        horizontalScrollBar()->setSliderPosition(value);

    } else if (action == QScrollBar::SliderPageStepSub) {
        x += d->viewport->width();
        // go up to the left of the page
        while (x > 0 && column > 0)
            x -= d->topHeader->sectionSize(--column);
        value = column * factor; // i is now the first item in the page
        if (x < 0)
            value += factor * -x / d->topHeader->sectionSize(column);
        horizontalScrollBar()->setSliderPosition(value);
    }
}
