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

  \brief Table view implementation

  This class implements a table representation of a QGenericItemView working
  on a QAbstractItemModel.
*/

QGenericTableView::QGenericTableView(QAbstractItemModel *model, QWidget *parent)
    : QAbstractItemView(*new QGenericTableViewPrivate, model, parent)
{
    setStartEditActions(startEditActions()|QAbstractItemDelegate::AnyKeyPressed);
    setLeftHeader(new QGenericHeader(model, Vertical, this));
    d->leftHeader->setClickable(true);
    setTopHeader(new QGenericHeader(model, Horizontal, this));
    d->topHeader->setClickable(true);
}

QGenericTableView::QGenericTableView(QGenericTableViewPrivate &dd, QAbstractItemModel *model, QWidget *parent)
    : QAbstractItemView(dd, model, parent)
{
    setStartEditActions(startEditActions()|QAbstractItemDelegate::AnyKeyPressed);
    setLeftHeader(new QGenericHeader(model, Vertical, this));
    d->leftHeader->setClickable(true);
    setTopHeader(new QGenericHeader(model, Horizontal, this));
    d->topHeader->setClickable(true);
}

QGenericTableView::~QGenericTableView()
{
}

QGenericHeader *QGenericTableView::topHeader() const
{
    return d->topHeader;
}

QGenericHeader *QGenericTableView::leftHeader() const
{
    return d->leftHeader;
}

void QGenericTableView::setTopHeader(QGenericHeader *header)
{
    if (d->topHeader) {
        QObject::disconnect(d->topHeader,SIGNAL(sectionSizeChanged(int,int,int)),
                            this, SLOT(columnWidthChanged(int,int,int)));
        QObject::disconnect(d->topHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                            this, SLOT(columnIndexChanged(int,int,int)));
        QObject::disconnect(d->topHeader, SIGNAL(sectionClicked(int,ButtonState)),
                            this, SLOT(selectColumn(int,ButtonState)));
        QObject::disconnect(d->topHeader, SIGNAL(sectionCountChanged(int,int)),
                            this, SLOT(columnCountChanged(int,int)));
        QObject::disconnect(d->topHeader, SIGNAL(sectionHandleDoubleClicked(int,ButtonState)),
                            this, SLOT(resizeColumnToContents(int)));
    }

    d->topHeader = header;

    QObject::connect(d->topHeader,SIGNAL(sectionSizeChanged(int,int,int)),
                     this, SLOT(columnWidthChanged(int,int,int)));
    QObject::connect(d->topHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                     this, SLOT(columnIndexChanged(int,int,int)));
    QObject::connect(d->topHeader, SIGNAL(sectionClicked(int,ButtonState)),
                     this, SLOT(selectColumn(int,ButtonState)));
    QObject::connect(d->topHeader, SIGNAL(sectionCountChanged(int,int)),
                     this, SLOT(columnCountChanged(int,int)));
    QObject::connect(d->topHeader, SIGNAL(sectionHandleDoubleClicked(int,ButtonState)),
                     this, SLOT(resizeColumnToContents(int)));

    // FIXME: this needs to be set in setSelectionModel too
    d->topHeader->setSelectionModel(selectionModel());
    //columnCountChanged(0, d->topHeader->count()); // FIXME
}

void QGenericTableView::setLeftHeader(QGenericHeader *header)
{
    if (d->leftHeader) {
        QObject::disconnect(d->leftHeader, SIGNAL(sectionSizeChanged(int,int,int)),
                            this, SLOT(rowHeightChanged(int,int,int)));
        QObject::disconnect(d->leftHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                            this, SLOT(rowIndexChanged(int,int,int)));
        QObject::disconnect(d->leftHeader, SIGNAL(sectionClicked(int,ButtonState)),
                            this, SLOT(selectRow(int,ButtonState)));
        QObject::disconnect(d->leftHeader, SIGNAL(sectionCountChanged(int,int)),
                            this, SLOT(rowCountChanged(int,int)));
        QObject::disconnect(d->leftHeader, SIGNAL(sectionHandleDoubleClicked(int,ButtonState)),
                            this, SLOT(resizeRowToContents(int)));
    }

    d->leftHeader = header;

    QObject::connect(d->leftHeader, SIGNAL(sectionSizeChanged(int,int,int)),
                     this, SLOT(rowHeightChanged(int,int,int)));
    QObject::connect(d->leftHeader, SIGNAL(sectionIndexChanged(int,int,int)),
                     this, SLOT(rowIndexChanged(int,int,int)));
    QObject::connect(d->leftHeader, SIGNAL(sectionClicked(int,ButtonState)),
                     this, SLOT(selectRow(int,ButtonState)));
    QObject::connect(d->leftHeader, SIGNAL(sectionCountChanged(int,int)),
                     this, SLOT(rowCountChanged(int,int)));
    QObject::connect(d->leftHeader, SIGNAL(sectionHandleDoubleClicked(int,ButtonState)),
                     this, SLOT(resizeRowToContents(int)));

    // FIXME: this needs to be set in setSelectionModel too
    d->leftHeader->setSelectionModel(selectionModel());
    //rowCountChanged(0, d->leftHeader->count());
}

void QGenericTableView::scrollContentsBy(int dx, int dy)
{
    int hscroll = 0;
    int vscroll = 0;
    if (dx) { // horizontal
        int value = horizontalScrollBar()->value();
        int section = d->topHeader->section(value / horizontalFactor());
        int left = (value % horizontalFactor()) * d->topHeader->sectionSize(section);
        int offset = (left / horizontalFactor()) + d->topHeader->sectionPosition(section);
        if (QApplication::reverseLayout()) {
            int delta = d->viewport->width() + d->topHeader->sectionSize(section) + d->viewport->x() - 2;
            hscroll = offset + d->topHeader->offset();
            d->topHeader->setOffset(offset - delta);
        } else {
            hscroll = d->topHeader->offset() - offset;
            d->topHeader->setOffset(offset);
        }
    }
    if (dy) { // vetical
        int value = verticalScrollBar()->value();
        int section = d->leftHeader->section(value / verticalFactor());
        int above = (value % verticalFactor()) * d->leftHeader->sectionSize(section);
        int offset = (above / verticalFactor()) + d->leftHeader->sectionPosition(section);
        vscroll = d->leftHeader->offset() - offset;
        d->leftHeader->setOffset(offset);
    }
    //d->viewport->scroll(hscroll, vscroll);
    d->viewport->update();
}

void QGenericTableView::drawGrid(QPainter *p, int x, int y, int w, int h) const
{
    QPen old = p->pen();
    p->setPen(lightGray);
    int v = QApplication::reverseLayout() ? x : x + w;
    p->drawLine(v, y, v, y + h);
    p->drawLine(x, y + h, x + w, y + h);
    p->setPen(old);
}

void QGenericTableView::paintEvent(QPaintEvent *e)
{
    QPainter painter(d->viewport);
    QRect area = e->rect();

    int colfirst = columnAt(area.left());
    int collast = columnAt(area.right());
//    qDebug("%d %d : %d", colfirst, collast, d->topHeader->offset());
    if (colfirst > collast) {
        int tmp = colfirst;
        colfirst = collast;
        collast = tmp;
    }
    int rowfirst = rowAt(area.top());
    int rowlast = rowAt(area.bottom());
    bool showGrid = d->showGrid;

    QModelIndex bottomRight = model()->bottomRight(root());

    if (rowfirst == -1 || colfirst == -1)
        return;
    if (rowlast == -1)
        rowlast = bottomRight.row();
    if (collast == -1)
        collast = bottomRight.column();

    QItemOptions options;
    getViewOptions(&options);
    QItemSelectionModel *sels = selectionModel();
    QGenericHeader *leftHeader = d->leftHeader;
    QGenericHeader *topHeader = d->topHeader;
    QModelIndex current = currentItem();
    bool focus = hasFocus() && current.isValid();

    for (int r = rowfirst; r <= rowlast; ++r) {
        if (leftHeader->isSectionHidden(r))
            continue;
        int rowp = rowViewportPosition(r);
        int rowh = rowHeight(r);
        for (int c = colfirst; c <= collast; ++c) {
            if (topHeader->isSectionHidden(c))
                continue;
            int colp = columnViewportPosition(c);
            int colw = columnWidth(c);
            QModelIndex item = model()->index(r, c, root());
            if (item.isValid()) {
                options.itemRect = QRect(colp, rowp, colw - 1, rowh - 1);
                options.selected = sels ? sels->isSelected(item) : 0;
                options.focus = (focus && item == current);
                painter.fillRect(colp, rowp, colw, rowh,
                                 (options.selected ? options.palette.highlight() :
                                  options.palette.base()));
                itemDelegate()->paint(&painter, options, item);
            }
            if (showGrid)
                drawGrid(&painter, colp, rowp, colw - 1, rowh - 1);
        }
    }
}

bool QGenericTableView::event(QEvent *e)
{
    if (e->type() == QEvent::Show) {
        emit needMore();
    }
    return QAbstractItemView::event(e);
}

QModelIndex QGenericTableView::itemAt(int x, int y) const
{
    return model()->index(rowAt(y), columnAt(x), root());
}

int QGenericTableView::horizontalOffset() const
{
    return d->topHeader->offset();
}

int QGenericTableView::verticalOffset() const
{
    return d->leftHeader->offset();
}

QModelIndex QGenericTableView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                          ButtonState /*state*/)
{
    QModelIndex current = currentItem();
    QModelIndex bottomRight = model()->bottomRight(root());
    switch (cursorAction) {
        case QAbstractItemView::MoveUp:
            return model()->index(current.row() - 1, current.column(), root());
        case QAbstractItemView::MoveDown:
            return model()->index(current.row() + 1, current.column(), root());
        case QAbstractItemView::MoveLeft:
            return model()->index(current.row(), current.column() - 1, root());
        case QAbstractItemView::MoveRight:
            return model()->index(current.row(), current.column() + 1, root());
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
        }
    }
    return QModelIndex();
}

void QGenericTableView::setSelection(const QRect &rect, int command)
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

void QGenericTableView::rowCountChanged(int, int)
{
    updateGeometries();
    d->viewport->update();
    if (d->viewport->height() >= d->topHeader->size())
        emit needMore();
}

void QGenericTableView::columnCountChanged(int, int)
{
    updateGeometries();
    d->viewport->update();
}

void QGenericTableView::updateGeometries()
{
    int width = d->leftHeader->sizeHint().width();
    QSize topHint = d->topHeader->sizeHint();

    bool reverse = QApplication::reverseLayout();
    setViewportMargins(reverse ? 0 : width, topHint.height(), reverse ? width : 0, 0);

    QRect vg = d->viewport->geometry();
    if (QApplication::reverseLayout())
        d->topHeader->setOffset(vg.width() - topHint.width());
    d->leftHeader->setGeometry(reverse ? vg.right() + 1 : (vg.left() - 1 - width), vg.top(), width, vg.height());
    d->topHeader->setGeometry(vg.left(), vg.top() - topHint.height() - 1, vg.width(), topHint.height());

    // update sliders
    QItemOptions options;
    getViewOptions(&options);

    int h = d->viewport->height();
    int row = model()->rowCount(0);
    if (h <= 0 || row <= 0) // if we have no viewport or no rows, there is nothing to do
        return;
    QSize def = itemDelegate()->sizeHint(fontMetrics(), options, model()->index(0, 0, 0));
    verticalScrollBar()->setPageStep(h / def.height() * verticalFactor());
    while (h > 0 && row > 0)
        h -= d->leftHeader->sectionSize(--row);
    int max = row * verticalFactor();
    if (h < 0)
         max += 1 + (verticalFactor() * -h / d->leftHeader->sectionSize(row));
    verticalScrollBar()->setRange(0, max);

    int w = d->viewport->width();
    int col = model()->columnCount(0);
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

int QGenericTableView::rowSizeHint(int row) const
{
    int columnfirst = columnAt(0);
    int columnlast = columnAt(d->viewport->width());
    if (columnlast < 0)
        columnlast = d->topHeader->count() - 1;

    QItemOptions options;
    getViewOptions(&options);

    int hint = 0;
    QModelIndex index;
    for (int column = columnfirst; column < columnlast; ++column) {
        index = model()->index(row, column, root());
        hint = qMax(hint, itemDelegate()->sizeHint(fontMetrics(), options, index).height());
    }
    return hint + 1; // add space for the grid
}

int QGenericTableView::columnSizeHint(int column) const
{
    int rowfirst = rowAt(0);
    int rowlast = rowAt(d->viewport->height());
    if (rowlast < 0)
        rowlast = d->leftHeader->count() - 1;

    QItemOptions options;
    getViewOptions(&options);

    int hint = 0;
    QModelIndex index;
    for (int row = rowfirst; row < rowlast; ++row) {
        index = model()->index(row, column, root());
        hint = qMax(hint, itemDelegate()->sizeHint(fontMetrics(), options, index).width());
    }
    return hint + 1; // add space for the grid
}

int QGenericTableView::rowViewportPosition(int row) const
{
    return d->leftHeader->sectionPosition(row) - d->leftHeader->offset();
}

int QGenericTableView::rowHeight(int row) const
{
    return d->leftHeader->sectionSize(row);
}

int QGenericTableView::rowAt(int y) const
{
    return d->leftHeader->sectionAt(y + d->leftHeader->offset());
}

int QGenericTableView::columnViewportPosition(int column) const
{
    int colp = d->topHeader->sectionPosition(column) - d->topHeader->offset();
    if (!QApplication::reverseLayout())
        return colp;
    return colp + (d->topHeader->x() - d->viewport->x());
}

int QGenericTableView::columnWidth(int column) const
{
    return d->topHeader->sectionSize(column);
}

int QGenericTableView::columnAt(int x) const
{
    int p = x + d->topHeader->offset();
    if (!QApplication::reverseLayout())
        return d->topHeader->sectionAt(p);
    return d->topHeader->sectionAt(p - (d->topHeader->x() - d->viewport->x()));
}

bool QGenericTableView::isRowHidden(int row) const
{
    return d->leftHeader->isSectionHidden(row);
}

bool QGenericTableView::isColumnHidden(int column) const
{
    return d->topHeader->isSectionHidden(column);
}

void QGenericTableView::setShowGrid(bool show)
{
    d->showGrid = show;
}

bool QGenericTableView::showGrid() const
{
    return d->showGrid;
}

QRect QGenericTableView::itemViewportRect(const QModelIndex &item) const
{
    if (!item.isValid() || model()->parent(item) != root())
        return QRect();
    return QRect(columnViewportPosition(item.column()), rowViewportPosition(item.row()),
                 columnWidth(item.column()), rowHeight(item.row()));
}

void QGenericTableView::ensureItemVisible(const QModelIndex &item)
{
    QRect area = d->viewport->geometry();
    QRect rect = itemViewportRect(item);

    if (area.contains(rect) || model()->parent(item) != root())
        return;

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

void QGenericTableView::rowHeightChanged(int row, int, int)
{
    int y = rowViewportPosition(row);
    d->viewport->update(QRect(0, y, d->viewport->width(), d->viewport->height() - y));
    updateGeometries();
    updateCurrentEditor();
}

void QGenericTableView::columnWidthChanged(int column, int, int)
{
    bool reverse = QApplication::reverseLayout();
    int x = columnViewportPosition(column) - (reverse ? columnWidth(column) : 0);
    QRect rect(x, 0, d->viewport->width() - x, d->viewport->height());
    d->viewport->update(rect.normalize());
    updateGeometries();
    updateCurrentEditor();
}

void QGenericTableView::rowIndexChanged(int, int oldIndex, int newIndex)
{
    int o = rowViewportPosition(d->leftHeader->section(oldIndex));
    int n = rowViewportPosition(d->leftHeader->section(newIndex));
    int top = (o < n ? o : n);
    int height = d->viewport->height() - (o > n ? o : n);
    updateGeometries();
    d->viewport->update(0, top, d->viewport->width(), height);
}

void QGenericTableView::columnIndexChanged(int, int oldIndex, int newIndex)
{
    int o = columnViewportPosition(d->topHeader->section(oldIndex));
    int n = columnViewportPosition(d->topHeader->section(newIndex));
    int left = (o < n ? o : n);
    int width = d->viewport->width() - (o > n ? o : n);
    updateGeometries();
    d->viewport->update(left, 0, width, d->viewport->height());
}

void QGenericTableView::selectRow(int row, ButtonState state)
{
    if (row >= 0 && row < model()->rowCount()) {
        QModelIndex tl = model()->index(row, 0, root());
        QModelIndex br = model()->index(row, model()->columnCount() - 1, root());
        selectionModel()->setCurrentItem(tl, QItemSelectionModel::NoUpdate);
        if (d->selectionMode == Single)
            selectionModel()->select(tl, selectionCommand(state, tl));
        else
           selectionModel()->select(QItemSelection(tl, br, model()), selectionCommand(state, tl));
    }
}

void QGenericTableView::selectColumn(int column, ButtonState state)
{
    if (column >= 0 && column < model()->columnCount()) {
        QModelIndex tl = model()->index(0, column, root());
        QModelIndex br = model()->index(model()->rowCount() - 1, column, root());
        selectionModel()->setCurrentItem(tl, QItemSelectionModel::NoUpdate);
        if (d->selectionMode == Single)
            selectionModel()->select(tl, selectionCommand(state, tl));
        else
           selectionModel()->select(QItemSelection(tl, br, model()), selectionCommand(state, tl));
    }
}

void QGenericTableView::hideRow(int row)
{
    d->leftHeader->hideSection(row);
}

void QGenericTableView::hideColumn(int column)
{
    d->topHeader->hideSection(column);
}

void QGenericTableView::showRow(int row)
{
    d->leftHeader->showSection(row);
}

void QGenericTableView::showColumn(int column)
{
    d->topHeader->showSection(column);
}

void QGenericTableView::resizeRowToContents(int row)
{
    int size = rowSizeHint(row);
    d->leftHeader->resizeSection(row, size);
}

void QGenericTableView::resizeColumnToContents(int column)
{
    int size = columnSizeHint(column);
    d->topHeader->resizeSection(column, size);
}

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
        while (y < h && row < d->model->rowCount(0))
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
        while (x < w && column < d->model->columnCount(0))
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
