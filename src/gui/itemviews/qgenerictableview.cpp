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

  \brief The QGenericTableView class provides a default model/view implementation of a table view.

  \ingroup model-view

  This class implements a table representation of a QGenericItemView working
  on a QAbstractItemModel.

  \sa \link model-view-programming.html Model/View Programming\endlink.

*/

/*!
  Constructs a table view with a \a parent to represent the data in
  the given \a model.

  \sa QAbstractItemModel
*/

QGenericTableView::QGenericTableView(QAbstractItemModel *model, QWidget *parent)
    : QAbstractItemView(*new QGenericTableViewPrivate, model, parent)
{
    setBeginEditActions(beginEditActions()|QAbstractItemDelegate::AnyKeyPressed);
    setLeftHeader(new QGenericHeader(model, Qt::Vertical, this));
    d->leftHeader->setClickable(true);
    setTopHeader(new QGenericHeader(model, Qt::Horizontal, this));
    d->topHeader->setClickable(true);
}

/*!
  \internal
*/
QGenericTableView::QGenericTableView(QGenericTableViewPrivate &dd, QAbstractItemModel *model, QWidget *parent)
    : QAbstractItemView(dd, model, parent)
{
    setBeginEditActions(beginEditActions()|QAbstractItemDelegate::AnyKeyPressed);
    setLeftHeader(new QGenericHeader(model, Qt::Vertical, this));
    d->leftHeader->setClickable(true);
    setTopHeader(new QGenericHeader(model, Qt::Horizontal, this));
    d->topHeader->setClickable(true);
}

/*!
  Destroys the table view.
*/
QGenericTableView::~QGenericTableView()
{
}

/*!
  Returns the header to the top of the table view.
*/

QGenericHeader *QGenericTableView::topHeader() const
{
    return d->topHeader;
}

/*!
  Returns the header to the left of the table view.
*/
QGenericHeader *QGenericTableView::leftHeader() const
{
    return d->leftHeader;
}

/*!
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

    d->topHeader->setSelectionModel(selectionModel());
}

/*!
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

    d->leftHeader->setSelectionModel(selectionModel());
}

/*!
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
            QModelIndex item = model()->index(r, c, root());
            if (item.isValid()) {
                option.rect = QRect(colp, rowp, colw, rowh);
                option.state = state;
                option.state |= (sels->isSelected(item)
                                 ? QStyle::Style_Selected : QStyle::Style_Default);
                option.state |= (focus && item == current
                                 ? QStyle::Style_HasFocus : QStyle::Style_Default);
                painter.fillRect(colp, rowp, colw, rowh,
                                 (option.state & QStyle::Style_Selected
                                  ? option.palette.highlight() : base));
                itemDelegate()->paint(&painter, option, item);
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
  Returns the model item index corresponding to the item at \a(x, y).
*/
QModelIndex QGenericTableView::itemAt(int x, int y) const
{
    return model()->index(rowAt(y), columnAt(x), root());
}

/*!
  Returns the horizontal offset of the items in the table view.
*/
int QGenericTableView::horizontalOffset() const
{
    return d->topHeader->offset();
}

/*!
  Returns the vertical offset of the items in the table view.
*/
int QGenericTableView::verticalOffset() const
{
    return d->leftHeader->offset();
}

/*!
\fn QModelIndex QGenericTableView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state)

Move the cursor in the way described by \a cursorAction, using the
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
*/

void QGenericTableView::rowCountChanged(int, int)
{
    updateGeometries();
    d->viewport->update();
}

/*!
  \fn void QGenericTableView::columnCountChanged(int start, int end)

  Informs the table view that the columns from \a start to \a end
  inclusive have been changed.
*/

void QGenericTableView::columnCountChanged(int, int)
{
    updateGeometries();
    d->viewport->update();
}

/*!
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

    // update sliders
    QStyleOptionViewItem option = viewOptions();

    int h = d->viewport->height();
    int row = model()->rowCount();
    if (h <= 0 || row <= 0) // if we have no viewport or no rows, there is nothing to do
        return;
    QSize def = itemDelegate()->sizeHint(fontMetrics(), option, model()->index(0, 0));
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
  Returns the size hint for the \a row's height.

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
        index = model()->index(row, column, root());
        hint = qMax(hint, itemDelegate()->sizeHint(fontMetrics(), option, index).height());
    }
    return hint + 1; // add space for the grid
}

/*!
  Returns the size hint for the \a column's width.

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
        index = model()->index(row, column, root());
        hint = qMax(hint, itemDelegate()->sizeHint(fontMetrics(), option, index).width());
    }
    return hint + 1; // add space for the grid
}

/*!
*/

int QGenericTableView::rowViewportPosition(int row) const
{
    return d->leftHeader->sectionPosition(row) - d->leftHeader->offset();
}

/*!
  Returns the height of the \a row.
*/

int QGenericTableView::rowHeight(int row) const
{
    return d->leftHeader->sectionSize(row);
}

/*!
  Returns the row in the tree view whose header covers the \a y
  coordinate given.
*/

int QGenericTableView::rowAt(int y) const
{
    return d->leftHeader->sectionAt(y + d->leftHeader->offset());
}

/*!
  Returns the horizontal position of the \a column in the viewport.*/

int QGenericTableView::columnViewportPosition(int column) const
{
    int colp = d->topHeader->sectionPosition(column) - d->topHeader->offset();
    if (!QApplication::reverseLayout())
        return colp;
    return colp + (d->topHeader->x() - d->viewport->x());
}

/*!
  Returns the widget of the \a column.
*/

int QGenericTableView::columnWidth(int column) const
{
    return d->topHeader->sectionSize(column);
}

/*!
  Returns the column in the tree view whose header covers the \a x
  coordinate given.
*/

int QGenericTableView::columnAt(int x) const
{
    int p = x + d->topHeader->offset();
    if (!QApplication::reverseLayout())
        return d->topHeader->sectionAt(p);
    return d->topHeader->sectionAt(p - (d->topHeader->x() - d->viewport->x()));
}

/*!
  Returns true if the \a row is hidden; otherwise returns false.
*/

bool QGenericTableView::isRowHidden(int row) const
{
    return d->leftHeader->isSectionHidden(row);
}

/*!
  Returns true if the \a column is hidden; otherwise returns false.
*/

bool QGenericTableView::isColumnHidden(int column) const
{
    return d->topHeader->isSectionHidden(column);
}

/*!
  \property QGenericTableView::showGrid
  \brief whether or not to show the grid.

  This property is true if the table grid should be drawn
  and false if it should not be drawn.
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
*/

void QGenericTableView::setGridStyle(Qt::PenStyle style)
{
    d->gridStyle = style;
}

/*!
  \fn QRect QGenericTableView::itemViewportRect(const QModelIndex &index) const

  Returns the rectangle on the viewport occupied by the item at
  \a index.
*/

QRect QGenericTableView::itemViewportRect(const QModelIndex &item) const
{
    if (!item.isValid() || model()->parent(item) != root())
        return QRect();
    return QRect(columnViewportPosition(item.column()), rowViewportPosition(item.row()),
                 columnWidth(item.column()), rowHeight(item.row()));
}

/*!
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
*/

void QGenericTableView::rowHeightChanged(int row, int, int)
{
    int y = rowViewportPosition(row);
    d->viewport->update(QRect(0, y, d->viewport->width(), d->viewport->height() - y));
    updateGeometries();
    updateEditors();
}

/*!
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
*/

void QGenericTableView::hideRow(int row)
{
    d->leftHeader->hideSection(row);
}

/*!
*/

void QGenericTableView::hideColumn(int column)
{
    d->topHeader->hideSection(column);
}

/*!
*/

void QGenericTableView::showRow(int row)
{
    d->leftHeader->showSection(row);
}

/*!
*/

void QGenericTableView::showColumn(int column)
{
    d->topHeader->showSection(column);
}

/*!
*/

void QGenericTableView::resizeRowToContents(int row, bool checkHeader)
{
    int content = rowSizeHint(row);
    int header = checkHeader ? d->leftHeader->sectionSizeHint(row) : 0;
    d->leftHeader->resizeSection(row, qMax(content, header));
}

/*!
*/

void QGenericTableView::resizeColumnToContents(int column, bool checkHeader)
{
    int content = columnSizeHint(column);
    int header = checkHeader ? d->topHeader->sectionSizeHint(column) : 0;
    d->topHeader->resizeSection(column, qMax(content, header));
}

/*!
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
