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
#include "qgenerictreeview.h"
#include "qgenericheader.h"
#include <qitemdelegate.h>
#include <qapplication.h>
#include <qscrollbar.h>
#include <qpainter.h>
#include <qstack.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qpen.h>

#include <private/qgenerictreeview_p.h>
#define d d_func()
#define q q_func()

/*!
  \class QGenericTreeViewItem qgenerictreeview.cpp

  \brief This class implements a QViewItem working on a QGenericTreeView.

  \ingroup model-view


    \sa \link model-view-programming.html Model/View Programming\endlink.
*/

/*!
    \fn QGenericTreeViewItem::QGenericTreeViewItem()
    \internal
*/

/*!
  \class QGenericTreeView qgenerictreeview.h

  \brief The QGenericTreeView class provides a default model/view implementation of a tree view.

  \ingroup model-view

  This class implements a tree representation of a QGenericItemView working
  on a QAbstractItemModel.

  \omit
  Describe the opening/closing concept if not covered elsewhere.
  \endomit

  \sa \link model-view-programming.html Model/View Programming\endlink.
*/

QGenericTreeView::QGenericTreeView(QAbstractItemModel *model, QWidget *parent)
    : QAbstractItemView(*new QGenericTreeViewPrivate, model, parent)
{
    setHeader(new QGenericHeader(model, Qt::Horizontal, this));
    d->header->setMovable(true);
    d->rootDecoration = true;
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

/*!
  \internal
*/

QGenericTreeView::QGenericTreeView(QGenericTreeViewPrivate &dd, QAbstractItemModel *model,
                                   QWidget *parent)
    : QAbstractItemView(dd, model, parent)
{
    setHeader(new QGenericHeader(model, Qt::Horizontal, this));
    d->header->setMovable(true);
    d->rootDecoration = true;
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

/*!
  Destroys the tree view.
*/

QGenericTreeView::~QGenericTreeView()
{
}

/*!
  Returns the header for the tree view.
*/

QGenericHeader *QGenericTreeView::header() const
{
    return d->header;
}

/*!
  Sets the \a header for the tree view.
*/

void QGenericTreeView::setHeader(QGenericHeader *header)
{
    if (d->header) {
        QObject::disconnect(d->header, SIGNAL(sectionSizeChanged(int,int,int)),
                            this, SLOT(columnWidthChanged(int,int,int)));
        QObject::disconnect(d->header, SIGNAL(sectionIndexChanged(int,int,int)),
                            this, SLOT(dataChanged()));
        QObject::disconnect(d->header, SIGNAL(sectionCountChanged(int,int)),
                            this, SLOT(columnCountChanged(int,int)));
        QObject::disconnect(d->header, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                            this, SLOT(resizeColumnToContents(int)));
        delete d->header;
    }
    d->header = header;
    setViewportMargins(0, d->header->sizeHint().height(), 0, 0);
    QObject::connect(d->header, SIGNAL(sectionSizeChanged(int,int,int)),
                     this, SLOT(columnWidthChanged(int,int,int)));
    QObject::connect(d->header, SIGNAL(sectionIndexChanged(int,int,int)),
                     this, SLOT(dataChanged()));
    QObject::connect(d->header, SIGNAL(sectionCountChanged(int,int)),
                     this, SLOT(columnCountChanged(int,int)));
    QObject::connect(d->header, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                     this, SLOT(resizeColumnToContents(int)));
    d->header->setSelectionModel(selectionModel());

    updateGeometries();
}

/*!
  Returns the indentation of the root items in the tree view.

  \sa setIndentation()
*/

int QGenericTreeView::indentation() const
{
    return d->indent;
}

/*!
  \fn void QGenericTreeView::setIndentation(int amount)

  Sets the indentation of items in the leftmost column to the \a amount
  given.
*/

void QGenericTreeView::setIndentation(int i)
{
    d->indent = i;
}

/*!
  Returns the column that is currently being edited.

  \sa setEditColumn()
*/

int QGenericTreeView::editColumn() const
{
    return d->editColumn;
}

/*!
  Sets the \a column that is currently being edited.
*/

void QGenericTreeView::setEditColumn(int column)
{
    d->editColumn = column;
}

/*!
  Returns true if root items are displayed with controls for opening and
  closing them; otherwise returns false.

  \sa setShowRootDecoration()
*/

bool QGenericTreeView::showRootDecoration() const
{
    return d->rootDecoration;
}

/*!
  Controls whether to \a show controls for opening and closing items
  alongside the items in the leftmost column.
*/

void QGenericTreeView::setShowRootDecoration(bool show)
{
    d->rootDecoration = show;
    d->viewport->update();
}

/*!
*/

int QGenericTreeView::columnViewportPosition(int column) const
{
    int colp = d->header->sectionPosition(column) - d->header->offset();
    if (!QApplication::reverseLayout())
        return colp;
    return colp + (d->header->x() - d->viewport->x());
}

/*!
*/

int QGenericTreeView::columnWidth(int column) const
{
    return d->header->sectionSize(column);
}

/*!
  Returns the column in the tree view whose header covers the \a x
  coordinate given.
*/

int QGenericTreeView::columnAt(int x) const
{
    int p = x + d->header->offset();
    if (!QApplication::reverseLayout())
        return d->header->sectionAt(p);
    return d->header->sectionAt(p - (d->header->x() - d->viewport->x()));
}

/*!
  Returns true if the \a column is hidden; otherwise returns false.

  \sa hideColumn
*/

bool QGenericTreeView::isColumnHidden(int column) const
{
    return d->header->isSectionHidden(column);
}

/*!
  Hides the \a column given.

  \sa */

void QGenericTreeView::hideColumn(int column)
{
    d->header->hideSection(column);
}

/*!
  \fn void QGenericTreeView::open(const QModelIndex &index)

  Opens the model item given by the \a index.
*/

void QGenericTreeView::open(const QModelIndex &item)
{
    if (!item.isValid())
        return;
    int idx = d->viewIndex(item);
    if (idx > -1) // is visible
        d->open(idx);
    else
        d->opened.append(item);
}

/*!
*/

void QGenericTreeView::close(const QModelIndex &item)
{
    if (!item.isValid())
        return;
    int idx = d->viewIndex(item);
    if (idx > -1) // is visible
        d->close(idx);
    else
        d->opened.remove(d->opened.indexOf(item));
}

/*!
  \fn bool QGenericTreeView::isOpen(const QModelIndex &index) const

  Returns true if the model item \a index is open; otherwise returns
  false.
*/

bool QGenericTreeView::isOpen(const QModelIndex &item) const
{
    return d->opened.contains(item);
}

/*!
*/

QRect QGenericTreeView::itemViewportRect(const QModelIndex &index) const
{
    if (!index.isValid())
        return QRect();

    int x = columnViewportPosition(index.column());
    int w = columnWidth(index.column());
    int vi = d->viewIndex(index);
    if (vi < 0)
        return QRect();
    if (index.column() == 0) {
        int i = d->indentation(vi);
        x += i;
        w -= i;
    }
    int y = d->coordinate(vi);
    QItemOptions options = viewOptions();
    int h = itemDelegate()->sizeHint(fontMetrics(), options, d->modelIndex(vi)).height();
    return QRect(x, y, w, h);
}

/*!
  Scroll the contents of the tree view until the model item \a index is
  visible.
*/

void QGenericTreeView::ensureItemVisible(const QModelIndex &index)
{
    QRect area = d->viewport->rect();
    QRect rect = itemViewportRect(index);

    if (rect.isEmpty())
        return;
    if (area.contains(rect)) {
        d->viewport->repaint(rect);
        return;
    }

    // vertical
    if (rect.top() < area.top()) { // above
        int i = d->viewIndex(index);
        verticalScrollBar()->setValue(i * verticalFactor());
    } else if (rect.bottom() > area.bottom()) { // below
        QItemOptions options = viewOptions();
        QFontMetrics fontMetrics(this->fontMetrics());
        QAbstractItemDelegate *delegate = itemDelegate();
        int i = d->viewIndex(index);
        if (i < 0) {
            qWarning("ensureItemVisible: item index was illegal: %d", i);
            return;
        }
        int y = area.height();
        while (y > 0 && i > 0)
            y -= delegate->sizeHint(fontMetrics, options, d->items.at(i--).index).height();
        int a = (-y * verticalFactor()) / delegate->sizeHint(fontMetrics, options,
                                                             d->items.at(i).index).height();
        verticalScrollBar()->setValue(++i * verticalFactor() + a);
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
  \reimp
*/

void QGenericTreeView::paintEvent(QPaintEvent *e)
{
    QItemOptions options = viewOptions();

    QPainter painter(&d->backBuffer);
    QRect area = e->rect();

    if (d->items.isEmpty())
        return;

    d->left = d->header->indexAt(d->header->offset() + area.left());
    d->right = d->header->indexAt(d->header->offset() + area.right() - 1);

    if (d->left == -1)
        d->left = 0;
    if (d->right == -1)
        d->right = d->header->count() - 1;
    if (d->left > d->right) {
        int tmp = d->left;
        d->left = d->right;
        d->right = tmp;
    }

    const QGenericTreeViewItem *items = d->items.constData();

    QFontMetrics fontMetrics(this->fontMetrics());
    QAbstractItemDelegate *delegate = itemDelegate();
    QModelIndex index;
    QModelIndex current = selectionModel()->currentItem();

    int h = d->viewport->height();
    int v = verticalScrollBar()->value();
    int c = d->items.count();
    int i = d->itemAt(v);
    int y = d->coordinateAt(v, delegate->sizeHint(fontMetrics, options, items[i].index).height());
    while (y < h && i < c) {
        // prepare
        index = items[i].index;
        options.open = d->items[i].open;
        options.itemRect.setRect(0, y, 0, delegate->sizeHint(fontMetrics, options, index).height());
        options.focus = (index == current);
        d->current = i;
        // draw row
        drawRow(&painter, &options, index);
        // next row
        y += options.itemRect.height();
        ++i;
    }

    int w = d->viewport->width();
    int x = d->header->size();
    QRect bottom(0, y, w, h - y);
    QRect left(x, 0, w - x, h);
    if (y < h && area.intersects(bottom))
        painter.fillRect(bottom, options.palette.base());
    if (x < w && area.intersects(left))
        painter.fillRect(left, options.palette.base());

    painter.end();
    painter.begin(d->viewport);
    painter.drawPixmap(area.topLeft(), d->backBuffer, area);
}

/*!
  Draws the row in the tree view that contains the model item \a index,
  using the \a painter given. The \a options control how the item is
  displayed.

  \sa QItemOptions
*/

void QGenericTreeView::drawRow(QPainter *painter, QItemOptions *options, const QModelIndex &index) const
{
    int y = options->itemRect.y();
    int width, height = options->itemRect.height();
    QBrush base = options->palette.base();

    QModelIndex parent = model()->parent(index);
    QGenericHeader *header = d->header;
    QModelIndex current = selectionModel()->currentItem();
    bool focus = hasFocus() && current.isValid();
    bool reverse = QApplication::reverseLayout();

    int position;
    int headerSection;
    QModelIndex modelIndex;
    for (int headerIndex = d->left; headerIndex <= d->right; ++headerIndex) {
        headerSection = d->header->section(headerIndex);
        if (header->isSectionHidden(headerSection))
            continue;
        position = columnViewportPosition(headerSection);
        width = header->sectionSize(headerSection);
        modelIndex = model()->index(index.row(), headerSection, parent);
        options->focus = (focus && current == modelIndex);
        options->selected = selectionModel()->isSelected(modelIndex);
        if (headerSection == 0) {
            int i = d->indentation(d->current);
            options->itemRect.setRect(reverse ? position : i + position, y, width - i, height);
            painter->fillRect(position, y, width, height, base);
            drawBranches(painter, QRect(reverse ? position + width - i :
                                        position, y, i, options->itemRect.height()), index);
        } else {
            options->itemRect.setRect(position, y, width, height);
            painter->fillRect(position, y, width, height, base);
        }
        itemDelegate()->paint(painter, *options, modelIndex);
    }
}

/*!
  Draws the branches in the tree view on the same row as the model item
  \a index, using the \a painter given. Only the branches within the
  rectangle given by \a rect are drawn.

*/

void QGenericTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    QModelIndex parent = model()->parent(index);
    QModelIndex current = parent;
    QModelIndex ancestor = model()->parent(current);
    bool reverse = QApplication::reverseLayout();
    int indent = d->indent;
    int level = d->items.at(d->current).level;
    int outer = d->rootDecoration ? 0 : 1;
    QRect primitive(reverse ? rect.left() : rect.right(), rect.top(), indent, rect.height());

    Q4StyleOption opt(0, Q4StyleOption::Default);
    opt.palette = palette();
    if (level >= outer) {
        // start with the innermost branch
        primitive.moveLeft(reverse ? primitive.left() : primitive.left() - indent);
        opt.rect = primitive;
        opt.state = QStyle::Style_Item
                               | (model()->rowCount(parent) - 1 > index.row()
                                  ? QStyle::Style_Sibling : 0)
                               | (model()->hasChildren(index) ? QStyle::Style_Children : 0)
                               | (d->items.at(d->current).open ? QStyle::Style_Open : 0);
        style().drawPrimitive(QStyle::PE_TreeBranch, &opt, painter, this);
    }
    // then go out level by level
    for (--level; level >= outer; --level) { // we have already drawn the innermost branch
        primitive.moveLeft(reverse ? primitive.left() + indent : primitive.left() - indent);
        opt.rect = primitive;
        opt.state = model()->rowCount(ancestor) - 1 > current.row() ? QStyle::Style_Sibling
                                                                    : QStyle::Style_Default;
        style().drawPrimitive(QStyle::PE_TreeBranch, &opt, painter, this);
        current = ancestor;
        ancestor = model()->parent(current);
    }
}

/*!
  \reimp
*/

void QGenericTreeView::mousePressEvent(QMouseEvent *e)
{
    bool reverse = QApplication::reverseLayout();
    int scrollbar = reverse && verticalScrollBar()->isVisible() ? verticalScrollBar()->width() : 0;
    int x = e->x() - d->header->x() + d->header->offset() + scrollbar;
    int column = d->header->sectionAt(x);
    int position = d->header->sectionPosition(column);
    int cx = reverse ? position + d->header->sectionSize(column) - x : x - position;
    int vi = d->item(e->y());
    QModelIndex mi = d->modelIndex(vi);

    if (mi.isValid()) {
        int indent = d->indentation(vi) - (reverse ? 0 : d->header->offset());
        if (column == 0 && cx < (indent - d->indent))
            return; // we are in the empty area in front of the tree - do nothing
        if (column != 0 || cx > indent) {
            QAbstractItemView::mousePressEvent(e);
            return; // we are on an item - select it
        }
        if (d->items.at(vi).open)
            d->close(vi);
        else
            d->open(vi);
    }
}

/*!
  \fn QModelIndex QGenericTreeView::itemAt(int x, int y) const

  Returns the model index of the item at point \a(x, y).
*/

QModelIndex QGenericTreeView::itemAt(int, int y) const
{
    QModelIndex mi = d->modelIndex(d->item(y));
    return model()->sibling(mi.row(), d->editColumn, mi);
}

/*!
  Layout the items in the tree view.
*/

void QGenericTreeView::doItemsLayout()
{
    QItemOptions options = viewOptions();
    QModelIndex index = model()->index(0, 0, root());
    d->itemHeight = itemDelegate()->sizeHint(fontMetrics(), options, index).height();
    d->layout(-1);
    updateGeometries();
    d->viewport->update();
}

/*!
  Returns the horizontal offset.
*/

int QGenericTreeView::horizontalOffset() const
{
    return d->header->offset();
}

/*!
*/

int QGenericTreeView::verticalOffset() const
{
    // gives an estimate
    QItemOptions options = viewOptions();
    int iheight = d->delegate->sizeHint(fontMetrics(), options, model()->index(0, 0)).height();
    int item = verticalScrollBar()->value() / d->verticalFactor;
    return item * iheight;
}

/*!                                                                                       \fn QModelIndex QGenericTreeView::moveCursor(QAbstractItemView::CursorAction cursorAction, ButtonState buttons)                                                                                                                                                         Move the cursor to the                                                                                                                                                        */

QModelIndex QGenericTreeView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState)
{
    QModelIndex current = currentItem();
    int vi = d->viewIndex(current);

    switch (cursorAction) {
    case QAbstractItemView::MoveDown:
        return d->modelIndex(d->below(vi));
    case QAbstractItemView::MoveUp:
        return d->modelIndex(d->above(vi));
    case QAbstractItemView::MoveLeft:
        if (d->items.at(vi).open)
            d->close(vi);
        break;
    case QAbstractItemView::MoveRight:
        if (!d->items.at(vi).open)
            d->open(vi);
        break;
    case QAbstractItemView::MovePageUp:
        return d->modelIndex(d->pageUp(vi));
    case QAbstractItemView::MovePageDown:
        return d->modelIndex(d->pageDown(vi));
    case QAbstractItemView::MoveHome:
        return model()->index(0, 0);
    case QAbstractItemView::MoveEnd:
        return d->modelIndex(d->last());
    }
    return current;
}

/*!
  Applies the selection \a command to the items in or touched by the
  rectangle, \a rect.

  \sa selectionCommand()
*/

void QGenericTreeView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    int start = d->viewIndex(itemAt(rect.left(), rect.top()));
    int stop = d->viewIndex(itemAt(rect.right(), rect.bottom()));

    QModelIndex prevItem;
    QItemSelectionRange currentRange;
    QStack<QItemSelectionRange> rangeStack;
    QItemSelection selection;
    for (int i=start; i<=stop; ++i) {
        QModelIndex item = d->modelIndex(i);
        if (prevItem.isValid() &&
            model()->parent(item) == model()->parent(prevItem)) {
            // same parent
            currentRange = QItemSelectionRange(currentRange.parent(),
                                               currentRange.top(),
                                               currentRange.left(),
                                               item.row(),
                                               item.column());
        } else if (prevItem.isValid() &&
                   model()->parent(item) ==
                   model()->sibling(prevItem.row(), 0, prevItem)) {
            // item is child of prevItem
            rangeStack.push(currentRange);
            currentRange = QItemSelectionRange(model()->parent(item), item);
        } else {
            if (currentRange.isValid())
                selection.append(currentRange);
            if (rangeStack.isEmpty()) {
                currentRange = QItemSelectionRange(model()->parent(item), item);
            } else {
                currentRange = rangeStack.pop();
                if (model()->parent(item) == currentRange.parent()) {
                    currentRange = QItemSelectionRange(currentRange.parent(),
                                                       currentRange.top(),
                                                       currentRange.left(),
                                                       item.row(),
                                                       item.column());
                } else {
                    selection.append(currentRange);
                    currentRange = QItemSelectionRange(model()->parent(item), item);
                }
            }
        }
        prevItem = item;
    }
    if (currentRange.isValid())
        selection.append(currentRange);
    for (int i=0; i<rangeStack.count(); ++i)
        selection.append(rangeStack.at(i));
    selectionModel()->select(selection, command);
}

/*!
*/

QRect QGenericTreeView::selectionViewportRect(const QItemSelection &selection) const
{
    if (selection.count() <= 0 || d->items.count() <= 0)
        return QRect();

    QModelIndex bottomRight = model()->bottomRight(root());
    int top = d->items.count();
    int bottom = 0;
    QItemSelectionRange r;
    QModelIndex topIndex, bottomIndex;
    for (int i = 0; i < selection.count(); ++i) {
        r = selection.at(i);
        topIndex = model()->index(r.top(), r.left(), r.parent());
        top = qMin(d->viewIndex(topIndex), top);
        bottomIndex = model()->index(r.bottom(), r.left(), r.parent());
        bottom = qMax(d->viewIndex(bottomIndex), bottom);
    }

    QItemOptions options = viewOptions();
    int bottomHeight = itemDelegate()->sizeHint(fontMetrics(), options, bottomIndex).height();
    int bottomPos = d->coordinate(bottom) + bottomHeight;
    int topPos = d->coordinate(top);

    return QRect(0, topPos, d->viewport->width(), bottomPos - topPos); // always the width of a row
}

/*!
  Scrolls the contents of the tree view by \a (dx, dy).
*/

void QGenericTreeView::scrollContentsBy(int dx, int dy)
{
    int items = qMin(d->items.count(), d->viewport->height() / fontMetrics().height());
    int max_dx = horizontalFactor() * items;
    int max_dy = verticalFactor() * items;

    // no need to do a lot of work if we are going to redraw the whole thing anyway
    if (QABS(dx) > max_dx || QABS(dy) > max_dy) {
        horizontalScrollBar()->repaint();
        verticalScrollBar()->repaint();
        d->viewport->update();
        return;
    }

    if (dx) {
        int value = horizontalScrollBar()->value();
        int section = d->header->section(value / d->horizontalFactor);
        int left = (value % d->horizontalFactor) * d->header->sectionSize(section);
        int offset = (left / d->horizontalFactor) + d->header->sectionPosition(section);
        if (QApplication::reverseLayout()) {
            dx = offset + d->header->offset();
            d->header->setOffset(offset - d->header->size() + d->viewport->x());
        } else {
            dx = d->header->offset() - offset;
            d->header->setOffset(offset);
        }
        horizontalScrollBar()->repaint();
    }

    if (dy) {
        int current_value = verticalScrollBar()->value();
        int previous_value = current_value + dy; // -(-dy)
        int current_item = current_value / d->verticalFactor; // the first visible  item on the page
        int previous_item = previous_value / d->verticalFactor;

        QItemOptions options = viewOptions();
        QFontMetrics fontMetrics(this->fontMetrics());
        QAbstractItemDelegate *delegate = itemDelegate();
        const QGenericTreeViewItem *items = d->items.constData();
        QModelIndex current_index = items[current_item].index;
        QModelIndex previous_index = items[previous_item].index;

        int current_height = delegate->sizeHint(fontMetrics, options, current_index).height();
        int previous_height = delegate->sizeHint(fontMetrics, options, previous_index).height();
        int current_y = d->coordinateAt(current_value, current_height);
        int previous_y = d->coordinateAt(previous_value, previous_height);

        dy = current_y - previous_y;
        if (current_item > previous_item)
            for (int i = previous_item; i < current_item; ++i)
                dy -= delegate->sizeHint(fontMetrics, options, items[i].index).height();
        else if (current_item < previous_item)
            for (int i = previous_item; i > current_item; --i)
                dy += delegate->sizeHint(fontMetrics, options, items[i].index).height();

        verticalScrollBar()->repaint();
    }

    d->viewport->scroll(dx, dy);
}

/*!
  This signal is emitted whenever the contents of the tree view are changed.
  \omit
  ### This should be a signal.
  \endomit
*/

void QGenericTreeView::dataChanged()
{
    QAbstractItemView::dataChanged(QModelIndex(), QModelIndex());
}

/*!
*/

void QGenericTreeView::rowsInserted(const QModelIndex &parent, int, int)
{
    d->relayout(parent);
}

/*!
*/
void QGenericTreeView::rowsRemoved(const QModelIndex &parent, int, int)
{
    d->relayout(parent);
}

/*!
*/

void QGenericTreeView::columnCountChanged(int, int)
{
    if (isVisible())
        updateGeometries();
}

/*!
  Resizes the \a column given to the size of its contents.
*/

void QGenericTreeView::resizeColumnToContents(int column)
{
    int size = columnSizeHint(column);
    d->header->resizeSection(column, size);
}

/*!
*/

void QGenericTreeView::columnWidthChanged(int column, int, int)
{
    bool reverse = QApplication::reverseLayout();
    int x = d->header->sectionPosition(column) - d->header->offset()
            - (reverse ? d->header->sectionSize(column) : 0);
    QRect rect(x, 0, d->viewport->width() - x, d->viewport->height());
    d->viewport->update(rect.normalize());
    updateGeometries();
    updateEditors();
}

/*!
  Updates the items in the tree view.
  \internal
*/

void QGenericTreeView::updateGeometries()
{
    QSize hint = d->header->sizeHint();
    setViewportMargins(0, hint.height(), 0, 0);

    QRect vg = d->viewport->geometry();
    if (QApplication::reverseLayout())
        d->header->setOffset(vg.width() - hint.width());
    d->header->setGeometry(vg.left(), vg.top() - hint.height(), vg.width(), hint.height());

    // update sliders
    QItemOptions options = viewOptions();

    // vertical
    int h = viewport()->height();
    int item = d->items.count();
    if (h <= 0 || item <= 0) // if we have no viewport or no rows, there is nothing to do
        return;
    QModelIndex index = model()->index(0, 0);
    QSize def = itemDelegate()->sizeHint(fontMetrics(), options, model()->index(0, 0));
    verticalScrollBar()->setPageStep(h / def.height() * verticalFactor());
    while (h > 0 && item > 0)
        h -= itemDelegate()->sizeHint(fontMetrics(), options, d->modelIndex(--item)).height();
    int max = item * verticalFactor();
    if (h < 0)
         max += 1 + (verticalFactor() * -h /
                     itemDelegate()->sizeHint(fontMetrics(), options, d->modelIndex(item)).height());
    verticalScrollBar()->setRange(0, max);

    int w = viewport()->width();
    int col = model()->columnCount();
    if (w <= 0 || col <= 0 || def.isEmpty()) // if we have no viewport or no columns, there is nothing to do
        return;
    horizontalScrollBar()->setPageStep(w / def.width() * horizontalFactor());
    while (w > 0 && col > 0)
        w -= d->header->sectionSize(--col);
    max = col * horizontalFactor();
    if (w < 0)
        max += (horizontalFactor() * -w / d->header->sectionSize(col));
    horizontalScrollBar()->setRange(0, max);
}

/*!
  Moves the vertical scroll bar in the way described by the \a action.

  \sa QScrollBar::SliderAction

*/

void QGenericTreeView::verticalScrollbarAction(int action)
{
    QItemOptions options = viewOptions();

    int factor = d->verticalFactor;
    int value = verticalScrollBar()->value();
    int item = value / factor;
    int iheight = d->delegate->sizeHint(fontMetrics(), options, d->modelIndex(item)).height();
    int above = (value % factor) * iheight;
    int y = -(above / factor); // above the page

    if (action == QScrollBar::SliderPageStepAdd) {

        // go down to the bottom of the page
        int h = d->viewport->height();
        while (y < h && item < d->items.count())
            y += d->delegate->sizeHint(fontMetrics(), options, d->modelIndex(item++)).height();
        value = item * factor; // i is now the last item on the page
        if (y > h && item)
            value -= factor * (y - h) / d->delegate->sizeHint(fontMetrics(), options,
                                                              d->modelIndex(item - 1)).height();
        verticalScrollBar()->setSliderPosition(value);

    } else if (action == QScrollBar::SliderPageStepSub) {

        y += d->viewport->height();

        // go up to the top of the page
        while (y > 0 && item > 0)
            y -= d->delegate->sizeHint(fontMetrics(),
                                       options,
                                       d->modelIndex(--item)).height();
        value = item * factor; // i is now the first item in the page

        if (y < 0)
            value += factor * -y / d->delegate->sizeHint(fontMetrics(),
                                                         options,
                                                         d->modelIndex(item)).height();
        verticalScrollBar()->setSliderPosition(value);
    }
}

/*!
Moves the horizontal scroll bar in the way described by the \a action.

\sa QScrollBar::SliderAction
*/

void QGenericTreeView::horizontalScrollbarAction(int action)
{
    // horizontal
    int factor = d->horizontalFactor;
    int value = horizontalScrollBar()->value();
    int column = value / factor;
    int above = (value % factor) * d->header->sectionSize(column); // what's left; in "item units"
    int x = -(above / factor); // left of the page

    if (action == QScrollBar::SliderPageStepAdd) {

        // go down to the right of the page
        int w = d->viewport->width();
        while (x < w && column < d->model->columnCount())
            x += d->header->sectionSize(column++);
        value = column * factor; // i is now the last item on the page
        if (x > w && column)
            value -= factor * (x - w) / d->header->sectionSize(column - 1);
        horizontalScrollBar()->setSliderPosition(value);

    } else if (action == QScrollBar::SliderPageStepSub) {

        x += d->viewport->width();

        // go up to the left of the page
        while (x > 0 && column > 0)
            x -= d->header->sectionSize(--column);
        value = column * factor; // i is now the first item in the page

        if (x < 0)
            value += factor * -x / d->header->sectionSize(column);
        horizontalScrollBar()->setSliderPosition(value);
    }
}

/*!
*/

int QGenericTreeView::columnSizeHint(int column) const
{
    QItemOptions options = viewOptions();
    QFontMetrics fontMetrics(this->fontMetrics());
    QAbstractItemDelegate *delegate = itemDelegate();
    QModelIndex index;

    const QGenericTreeViewItem *items = d->items.constData();
    int v = verticalScrollBar()->value();
    int h = d->viewport->height();
    int c = d->items.count();
    int i = d->itemAt(v);
    int y = d->coordinateAt(v, delegate->sizeHint(fontMetrics, options, items[i].index).height());
    int w = 0;
    QSize size;

    while (y < h && i < c) {
        index = items[i].index;
        index = model()->sibling(index.row(), column, index);
        size = delegate->sizeHint(fontMetrics, options, index);
        w = qMax(w, size.width() + (column == 0 ? d->indentation(i) : 0));
        y += size.height();
        ++i;
    }

    return w;
}

void QGenericTreeViewPrivate::open(int i)
{
    QModelIndex index = items.at(i).index;
    opened.append(index);
    items[i].open = true;
    if (q->model()->rowCount(index) <= 0)
        return;

    layout(i);

    q->updateGeometries();
    viewport->update();
    qApp->processEvents();

    // make sure we open children that are already open
    // FIXME: this is slow: optimize
    QVector<QModelIndex> o = opened;
    for (int j = 0; j < o.count(); ++j) {
        if (q->model()->parent(o.at(j)) == index) {
            int k = opened.indexOf(o.at(j));
            opened.remove(k);
            int v = viewIndex(o.at(j));
            open(v);
        }
    }

    emit q->expanded(index);
}

void QGenericTreeViewPrivate::close(int i)
{
    QAbstractItemModel *model = q->model();
    int total = items.at(i).total;
    QModelIndex index = items.at(i).index;
    opened.remove(opened.indexOf(index));
    items[i].open = false;

    int idx = i;
    QModelIndex tmp = index;
    while (tmp.isValid()) {
        items[idx].total -= total;
        tmp = model->parent(tmp);
        idx = viewIndex(tmp);
    }
    qCollapse<QGenericTreeViewItem>(items, i, total);
    q->updateGeometries();
    viewport->update();

    emit q->collapsed(index);
}

void QGenericTreeViewPrivate::layout(int i)
{
    QModelIndex current;
    QModelIndex parent = modelIndex(i);
    int count = q->model()->rowCount(parent);

    if (i == -1)
        items.resize(count);
    else
        qExpand<QGenericTreeViewItem>(items, i, count);

    int level = i >= 0 ? items.at(i).level + 1 : 0;
    int first = i + 1;
    for (int j = first; j < first + count; ++j) {
        current = q->model()->index(j - first, 0, parent);
        items[j].index = current;
        items[j].level = level;
    }

    int k = i;
    QModelIndex root = q->root();
    while (parent != root) {
        items[k].total += count;
        parent = q->model()->parent(parent);
        k = viewIndex(parent);
    }
}

int QGenericTreeViewPrivate::pageUp(int i) const
{
    int idx = item(coordinate(i) - viewport->height());
    return idx == -1 ? first() : idx;
}

int QGenericTreeViewPrivate::pageDown(int i) const
{
    int idx = item(coordinate(i) + viewport->height());
    return idx == -1 ? last() : idx;
}

int QGenericTreeViewPrivate::above(int i) const
{
    int idx = i;
    while (--idx >= 0 && items.at(idx).hidden);
    return idx >= 0 ? idx : i;
}

int QGenericTreeViewPrivate::below(int i) const
{
    int idx = i;
    while (++idx < items.count() && items.at(idx).hidden);
    return idx < items.count() ? idx : i;
}

int QGenericTreeViewPrivate::first() const
{
    int i = -1;
    while (++i < items.count() && items.at(i).hidden);
    return i < items.count() ? i : -1;
}

int QGenericTreeViewPrivate::last() const
{
    int i = items.count();
    while (--i >= 0 && items.at(i).hidden);
    return i >= 0 ? i : -1;
}

int QGenericTreeViewPrivate::indentation(int i) const
{
    if (i < 0 || i >= items.count())
        return 0;
    int level = items.at(i).level;
    if (rootDecoration)
        level++;
    return level * indent;
}

int QGenericTreeViewPrivate::coordinate(int item) const
{
    QItemOptions options = q->viewOptions();
    QFontMetrics fontMetrics(q->fontMetrics());
    QAbstractItemDelegate *delegate = q->itemDelegate();
    int v = q->verticalScrollBar()->value();
    int i = itemAt(v); // first item (may start above the page)
    int ih = delegate->sizeHint(fontMetrics, options, items.at(i).index).height();
    int y = coordinateAt(v, ih); // the part of the item above the page
    int h = q->viewport()->height();
    if (i <= item) {
        while (y < h && i < items.count()) {
            if (i == item)
                return y; // item is visible - actual y in viewport
            y += delegate->sizeHint(fontMetrics, options, items.at(i).index).height();
            ++i;
        }
        // item is below the viewport - estimated y
        return y + (itemHeight * (item - itemAt(v)));
    }
    // item is above the viewport - estimated y
    return y - (itemHeight * item);
}

int QGenericTreeViewPrivate::item(int coordinate) const
{
    QItemOptions options = q->viewOptions();
    QFontMetrics fontMetrics(q->fontMetrics());
    QAbstractItemDelegate *delegate = q->itemDelegate();

    int v = q->verticalScrollBar()->value();
    int i = itemAt(v);
    if (i >= items.count())
        return -1;

    int y = coordinateAt(v, delegate->sizeHint(fontMetrics, options, items.at(i).index).height());
    int h = q->viewport()->height();
    if (coordinate >= y) {
        // search for item in viewport
        while (y < h && i < items.count()) {
            y += delegate->sizeHint(fontMetrics, options, items.at(i).index).height();
            if (coordinate < y)
                return i;
            ++i;
        }
        // item is below viewport - give estimated coordinates
    }
    // item is above the viewport - give estimated coordinates
    int idx = i + ((coordinate - y) / itemHeight);
    return idx < 0 || idx >= items.count() ? -1 : idx;
}

int QGenericTreeViewPrivate::viewIndex(const QModelIndex &index) const
{
    // NOTE: this function is slow if the item is outside the visible area
    // search in visible items first, then below
    int t = itemAt(q->verticalScrollBar()->value());
    t = t > 100 ? t - 100 : 0; // start 100 items above the visible area
    for (int i = t; i < items.count(); ++i)
        if (items.at(i).index.row() == index.row() &&
            items.at(i).index.data() == index.data()) // ignore column
            return i;
    // search above
    for (int j = 0; j < t; ++j)
        if (items.at(j).index.row() == index.row() &&
            items.at(j).index.data() == index.data()) // ignore column
            return j;
    return -1;
}

QModelIndex QGenericTreeViewPrivate::modelIndex(int i) const
{
    if (i < 0 || i >= items.count())
        return q->root();
    QModelIndex index = items.at(i).index;
    if (index.column() != editColumn)
        return index = q->model()->sibling(index.row(), editColumn, index);
    return items.at(i).index;
}

int QGenericTreeViewPrivate::itemAt(int value) const
{
    return value / q->verticalFactor();
}

int QGenericTreeViewPrivate::coordinateAt(int value, int iheight) const
{
    int factor = q->verticalFactor();
    int above = (value % factor) * iheight; // what's left; in "item units"
    return -(above / factor); // above the page
}

void QGenericTreeViewPrivate::relayout(const QModelIndex &parent)
{
    if (!q->isVisible()) {
        items.clear();
        return;
    }
    // do a local relayout of the items
    if (parent.isValid()) {
        int p = viewIndex(parent);
        if (p > -1 && items.at(p).open) {
            close(p);
            open(p);
        }
    } else {
        items.clear();
        q->doItemsLayout();
    }
}
