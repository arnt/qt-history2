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
#include "qtreeview.h"
#include <qheaderview.h>
#include <qitemdelegate.h>
#include <qapplication.h>
#include <qscrollbar.h>
#include <qpainter.h>
#include <qstack.h>
#include <qstyle.h>
#include <qstyleoption.h>
#include <qevent.h>
#include <qpen.h>
#include <qdebug.h>

#include <private/qtreeview_p.h>
#define d d_func()
#define q q_func()

/*!
    \class QTreeView qtreeview.h
    \brief The QTreeView class provides a default model/view implementation of a tree view.

    \ingroup model-view

    A QTreeView implements a tree representation of items from a
    model. This class is used to provide standard hierarchical lists that
    were previously provided by the \c QListView class, but using the more
    flexible approach provided by Qt's model/view architecture.

    QTreeView implements the interfaces defined by the
    QAbstractItemView class to allow it to display data provided by
    models derived from the QAbstractItemModel class.

    It is simple to construct a tree view displaying data from a
    model. In the following example, the contents of a directory are
    supplied by a QDirModel and displayed as a tree:

    \code
        QDirModel *model = new QDirModel(QDir(), parent);
        QTreeView *tree = new QTreeView(parent);
        tree->setModel(model);
    \endcode

    The model/view architecture ensures that the contents of the tree view
    are updated as the model changes.

    Items that have children can be in an expanded (children are
    visible) or collapsed (children are hidden) state. When this state
    changes a collapsed() or expanded() signal is emitted with the
    model index of the relevant item.

    The amount of indentation used to indicate levels of hierarchy is
    controlled by the \l indentation property.

    \omit
    Describe the opening/closing concept if not covered elsewhere.
    \endomit

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemModel QAbstractItemView
*/


/*!
  \fn void QTreeView::expanded(const QModelIndex &index)

  This signal is emitted when the item specified by \a index is expanded.
*/


/*!
  \fn void QTreeView::collapsed(const QModelIndex &index)

  This signal is emitted when the item specified by \a index is collapsed.
*/

/*!
    Constructs a table view with a \a parent to represent a model's
    data. Use setModel() to set the model.

    \sa QAbstractItemModel
*/

QTreeView::QTreeView(QWidget *parent)
    : QAbstractItemView(*new QTreeViewPrivate, parent)
{
    setHeader(new QHeaderView(Qt::Horizontal, this));
    d->header->setModel(model());
    d->header->setSelectionModel(selectionModel());
    d->header->setMovable(true);
    d->rootDecoration = true;
    setSelectionBehavior(QAbstractItemView::SelectRows);
}

/*!
  \internal
*/

QTreeView::QTreeView(QTreeViewPrivate &dd, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    d->rootDecoration = true;
    setSelectionBehavior(QAbstractItemView::SelectRows);

    QHeaderView *header = new QHeaderView(Qt::Horizontal, this);
    header->setModel(model());
    header->setSelectionModel(selectionModel());
    header->setMovable(true);
    setHeader(header);
}

/*!
  Destroys the tree view.
*/

QTreeView::~QTreeView()
{
}

/*!
  \reimp
*/
void QTreeView::setModel(QAbstractItemModel *model)
{
    reset();
    d->header->setModel(model);
    QAbstractItemView::setModel(model);
}

/*!
  \reimp
*/
void QTreeView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    d->header->setSelectionModel(selectionModel);
    QAbstractItemView::setSelectionModel(selectionModel);
}

/*!
  Returns the header for the tree view.
*/

QHeaderView *QTreeView::header() const
{
    return d->header;
}

/*!
  Sets the \a header for the tree view.
*/

void QTreeView::setHeader(QHeaderView *header)
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

//    setViewportMargins(0, d->header->sizeHint().height(), 0, 0);

    QObject::connect(d->header, SIGNAL(sectionSizeChanged(int,int,int)),
                     this, SLOT(columnWidthChanged(int,int,int)));
    QObject::connect(d->header, SIGNAL(sectionIndexChanged(int,int,int)),
                     this, SLOT(dataChanged()));
    QObject::connect(d->header, SIGNAL(sectionCountChanged(int,int)),
                     this, SLOT(columnCountChanged(int,int)));
    QObject::connect(d->header, SIGNAL(sectionHandleDoubleClicked(int,Qt::ButtonState)),
                     this, SLOT(resizeColumnToContents(int)));

//    updateGeometries();
}

/*!
  \property QTreeView::indentation
  \brief indentation of the items in the tree view.

  This property holds the indentation of the items for each level in the tree view.
  \sa setIndentation()
*/

int QTreeView::indentation() const
{
    return d->indent;
}

void QTreeView::setIndentation(int i)
{
    d->indent = i;
}

/*!
  \property QTreeView::rootIsDecorated
  \brief whether to show controls for opening and closing items

  This property holds whether root items are displayed with controls for opening and
  closing them.
*/

bool QTreeView::rootIsDecorated() const
{
    return d->rootDecoration;
}

void QTreeView::setRootIsDecorated(bool show)
{

    d->rootDecoration = show;
    d->viewport->update();
}

/*!
  Returns the horizontal position of the \a column in the viewport.
*/

int QTreeView::columnViewportPosition(int column) const
{
    int colp = d->header->sectionPosition(column) - d->header->offset();
    if (!QApplication::reverseLayout())
        return colp;
    return colp + (d->header->x() - d->viewport->x());
}

/*!
  Returns the width of the \a column.
*/

int QTreeView::columnWidth(int column) const
{
    return d->header->sectionSize(column);
}

/*!
  Returns the column in the tree view whose header covers the \a x
  coordinate given.
*/

int QTreeView::columnAt(int x) const
{
    int p = x + d->header->offset();
    if (!QApplication::reverseLayout())
        return d->header->sectionAt(p);
    return d->header->sectionAt(p - (d->header->x() - d->viewport->x()));
}

/*!
    Returns true if the \a column is hidden; otherwise returns false.

    \sa hideColumn()
*/

bool QTreeView::isColumnHidden(int column) const
{
    return d->header->isSectionHidden(column);
}

/*!
  Hides the \a column given.

  \sa showColumn() */

void QTreeView::hideColumn(int column)
{
    d->header->hideSection(column);
}

/*!
  Shows the given \a column in the tree view.

  \sa hideColumn() */

void QTreeView::showColumn(int column)
{
    d->header->showSection(column);
}

/*!
  \fn void QTreeView::open(const QModelIndex &index)

  Opens the model item specified by the \a index.
*/

void QTreeView::open(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    int idx = d->viewIndex(index);
    if (idx > -1) // is visible
        d->open(idx, true);
    else
        d->opened.append(index);
}

/*!
  \fn void QTreeView::close(const QModelIndex &index)

  Closes the model item specified by the \a index.
*/

void QTreeView::close(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    int idx = d->viewIndex(index);
    if (idx > -1) { // is visible
        d->close(idx, true);
    } else {
        idx = d->opened.indexOf(index);
        if (idx > -1)
            d->opened.remove(idx);
    }
}

/*!
  \fn bool QTreeView::isOpen(const QModelIndex &index) const

  Returns true if the model item \a index is open; otherwise returns
  false.
*/

bool QTreeView::isOpen(const QModelIndex &index) const
{
    return d->opened.contains(index);
}

/*!
  Returns the rectangle on the viewport occupied by the item at \a
  index.*/

QRect QTreeView::itemViewportRect(const QModelIndex &index) const
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
    QStyleOptionViewItem option = viewOptions();
    int h = itemDelegate()->sizeHint(fontMetrics(), option, d->model, d->modelIndex(vi)).height();
    return QRect(x, y, w, h);
}

/*!
  Scroll the contents of the tree view until the given model item \a index
  is visible.
*/

void QTreeView::ensureItemVisible(const QModelIndex &index)
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
        QStyleOptionViewItem option = viewOptions();
        QFontMetrics fontMetrics(this->fontMetrics());
        QAbstractItemDelegate *delegate = itemDelegate();
        int i = d->viewIndex(index);
        if (i < 0) {
            qWarning("ensureItemVisible: item index was illegal: %d", i);
            return;
        }
        int y = area.height();
        while (y > 0 && i > 0)
            y -= delegate->sizeHint(fontMetrics, option, d->model, d->items.at(i--).index).height();
        int a = (-y * verticalFactor()) / delegate->sizeHint(fontMetrics, option, d->model,
                                                             d->items.at(i).index).height();
        verticalScrollBar()->setValue(++i * verticalFactor() + a);
    }

    // horizontal
    if (rect.right() < area.left()) { // left of
        horizontalScrollBar()->setValue(index.column() * horizontalFactor());
    } else if (rect.left() > area.right()) { // right of
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

void QTreeView::paintEvent(QPaintEvent *e)
{
    QStyleOptionViewItem option = viewOptions();
    QBrush base = option.palette.base();
    QRect area = e->rect();

    if (d->items.isEmpty() || d->header->count() == 0) {
        QPainter painter(d->viewport);
        painter.fillRect(area, base);
        return;
    }

    QPainter painter(d->viewport);

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

    const QTreeViewItem *items = d->items.constData();

    QFontMetrics fontMetrics(this->fontMetrics());
    QAbstractItemDelegate *delegate = itemDelegate();
    QModelIndex index;
    QModelIndex current = selectionModel()->currentItem();
    QStyle::SFlags state = option.state;

    int t = area.top();
    int h = area.bottom() + 1;
    int v = verticalScrollBar()->value();
    int c = d->items.count();
    int i = d->itemAt(v);
    int s = delegate->sizeHint(fontMetrics, option, model(), items[i].index).height();
    int y = d->coordinateAt(v, s);

    while (y < h && i < c) {
        index = items[i].index;
        s = delegate->sizeHint(fontMetrics, option, d->model, index).height();
        if (y + s >= t) {
            option.rect.setRect(0, y, 0, s);
            option.state = state|(d->items[i].open ? QStyle::Style_Open : QStyle::Style_Default);
            d->current = i;
            drawRow(&painter, option, index);
        }
        y += s;
        ++i;
    }

    int w = d->viewport->width();
    int x = d->header->size();
    QRect bottom(0, y, w, h - y);
    QRect left(x, 0, w - x, h);
    if (y < h && area.intersects(bottom))
        painter.fillRect(bottom, base);
    if (x < w && area.intersects(left))
        painter.fillRect(left, base);
}

/*!
  Draws the row in the tree view that contains the model item \a index,
  using the \a painter given. The \a option control how the item is
  displayed.

  \sa QStyleOptionViewItem
*/

void QTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &option,
                               const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    QBrush base = option.palette.base();
    int y = option.rect.y();
    int width, height = option.rect.height();

    QModelIndex parent = model()->parent(index);
    QHeaderView *header = d->header;
    QModelIndex current = selectionModel()->currentItem();
    bool focus = hasFocus() && current.isValid();
    bool reverse = QApplication::reverseLayout();
    QStyle::SFlags state = opt.state;

    int position;
    int headerSection;
    QModelIndex modelIndex;

    for (int headerIndex = d->left; headerIndex <= d->right; ++headerIndex) {
        headerSection = d->header->section(headerIndex);
        if (header->isSectionHidden(headerSection))
            continue;
        position = columnViewportPosition(headerSection);
        width = header->sectionSize(headerSection);
        modelIndex = d->model->index(index.row(), headerSection, parent);
        opt.state = state;
        opt.state |= (focus && current == modelIndex
                     ? QStyle::Style_HasFocus : QStyle::Style_Default);
        opt.state |= (selectionModel()->isSelected(modelIndex)
                     ? QStyle::Style_Selected : QStyle::Style_Default);
        if (headerSection == 0) {
            int i = d->indentation(d->current);
            opt.rect.setRect(reverse ? position : i + position, y, width - i, height);
            painter->fillRect(position, y, width, height, base);
            drawBranches(painter, QRect(reverse ? position + width - i :
                                        position, y, i, option.rect.height()), index);
        } else {
            opt.rect.setRect(position, y, width, height);
            painter->fillRect(position, y, width, height, base);
        }
        itemDelegate()->paint(painter, opt, d->model, modelIndex);
    }
}

/*!
  Draws the branches in the tree view on the same row as the model item
  \a index, using the \a painter given. Only the branches within the
  rectangle specified by \a rect are drawn.

*/

void QTreeView::drawBranches(QPainter *painter, const QRect &rect,
                                    const QModelIndex &index) const
{
    QModelIndex parent = d->model->parent(index);
    QModelIndex current = parent;
    QModelIndex ancestor = d->model->parent(current);
    bool reverse = QApplication::reverseLayout();
    int indent = d->indent;
    int level = d->items.at(d->current).level;
    int outer = d->rootDecoration ? 0 : 1;
    QRect primitive(reverse ? rect.left() : rect.right(), rect.top(), indent, rect.height());

    QStyleOption opt(0);
    opt.palette = palette();
    if (level >= outer) {
        // start with the innermost branch
        primitive.moveLeft(reverse ? primitive.left() : primitive.left() - indent);
        opt.rect = primitive;
        opt.state = QStyle::Style_Item
                    | (d->model->rowCount(parent) - 1 > index.row()
                       ? QStyle::Style_Sibling : 0)
                    | (model()->hasChildren(index) ? QStyle::Style_Children : 0)
                    | (d->items.at(d->current).open ? QStyle::Style_Open : 0);
        style().drawPrimitive(QStyle::PE_TreeBranch, &opt, painter, this);
    }
    // then go out level by level
    for (--level; level >= outer; --level) { // we have already drawn the innermost branch
        primitive.moveLeft(reverse ? primitive.left() + indent : primitive.left() - indent);
        opt.rect = primitive;
        opt.state = (d->model->rowCount(ancestor) - 1 > current.row())
                    ? QStyle::Style_Sibling : QStyle::Style_Default;
        style().drawPrimitive(QStyle::PE_TreeBranch, &opt, painter, this);
        current = ancestor;
        ancestor = d->model->parent(current);
    }
}

/*!
  \reimp
*/

void QTreeView::mousePressEvent(QMouseEvent *e)
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
        int indent = d->indentation(vi);
        if (column == 0 && cx < (indent - d->indent))
            return; // we are in the empty area in front of the tree - do nothing
        if (column != 0 || cx > indent) {
            QAbstractItemView::mousePressEvent(e);
            return; // we are on an item - select it
        }
        if (d->items.at(vi).open)
            d->close(vi, true);
        else
            d->open(vi, true);
    }
}

/*!
  \fn QModelIndex QTreeView::itemAt(int x, int y) const

  Returns the model index of the item at point (\a x, \a y).
*/

QModelIndex QTreeView::itemAt(int x, int y) const
{
    int vi = d->item(y);
    QModelIndex mi = d->modelIndex(vi);
    int c = d->columnAt(x);
    if (mi.isValid() && c >= 0)
        return model()->sibling(mi.row(), c, mi);
    return QModelIndex();
}

/*!
  Returns the model index of the item above \a index.
*/

QModelIndex QTreeView::itemAbove(const QModelIndex &index) const
{
    int vi = d->viewIndex(index);
    return d->modelIndex(d->above(vi));
}

/*!
  Returns the model index of the item below \a index.
*/

QModelIndex QTreeView::itemBelow(const QModelIndex &index) const
{
    int vi = d->viewIndex(index);
    return d->modelIndex(d->below(vi));
}

/*!
  Lays out the items in the tree view.
*/

void QTreeView::doItemsLayout()
{
    QAbstractItemView::doItemsLayout();
    QStyleOptionViewItem option = viewOptions();
    if (model()->rowCount(root()) > 0) {
        QModelIndex index = model()->index(0, 0, root());
        d->itemHeight = itemDelegate()->sizeHint(fontMetrics(), option, model(), index).height();
        d->layout(-1);
        d->reopenChildren(root(), false);
    }
    updateGeometries();
}

/*!
  Returns the horizontal offset.
*/

int QTreeView::horizontalOffset() const
{
    return d->header->offset();
}

/*!
  Returns the vertical offset of the items in the tree view.
*/

int QTreeView::verticalOffset() const
{
    // gives an estimate
    QStyleOptionViewItem option = viewOptions();
    QModelIndex index = model()->index(0, 0);
    int iheight = itemDelegate()->sizeHint(fontMetrics(), option, model(), index).height();
    int item = verticalScrollBar()->value() / d->verticalFactor;
    return item * iheight;
}

/*!
  \fn QModelIndex QTreeView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState state)

  Move the cursor in the way described by \a cursorAction, using the
  information provided by the button \a state.

  \sa QAbstractItemView::CursorAction
*/

QModelIndex QTreeView::moveCursor(QAbstractItemView::CursorAction cursorAction, Qt::ButtonState)
{
    QModelIndex current = currentItem();
    if (!current.isValid())
        return current;
    int vi = d->viewIndex(current);
    if (vi < 0)
        return current;
    switch (cursorAction) {
    case QAbstractItemView::MoveDown:
        return d->modelIndex(d->below(vi));
    case QAbstractItemView::MoveUp:
        return d->modelIndex(d->above(vi));
    case QAbstractItemView::MoveLeft:
        if (d->items.at(vi).open)
            d->close(vi, true);
        break;
    case QAbstractItemView::MoveRight:
        if (!d->items.at(vi).open)
            d->open(vi, true);
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

void QTreeView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
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
  Returns the rectangle from the viewport of the items in the given
  \a selection.
*/

QRect QTreeView::selectionViewportRect(const QItemSelection &selection) const
{
    if (selection.count() <= 0 || d->items.count() <= 0)
        return QRect();

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

    QStyleOptionViewItem option = viewOptions();
    int bottomHeight = itemDelegate()->sizeHint(fontMetrics(), option, model(), bottomIndex).height();
    int bottomPos = d->coordinate(bottom) + bottomHeight;
    int topPos = d->coordinate(top);

    return QRect(0, topPos, d->viewport->width(), bottomPos - topPos); // always the width of a row
}

/*!
  \internal
*/

void QTreeView::reset()
{
    d->opened.clear();
    d->items.clear();
}

/*!
  Scrolls the contents of the tree view by (\a dx, \a dy).
*/

void QTreeView::scrollContentsBy(int dx, int dy)
{
    int items = qMin(d->items.count(), d->viewport->height() / fontMetrics().height());
    int max_dy = verticalFactor() * items;

    // no need to do a lot of work if we are going to redraw the whole thing anyway
    if (QABS(dy) > max_dy) {
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

        QStyleOptionViewItem option = viewOptions();
        QFontMetrics fontMetrics(this->fontMetrics());
        QAbstractItemDelegate *delegate = itemDelegate();
        const QTreeViewItem *items = d->items.constData();
        QModelIndex current_index = items[current_item].index;
        QModelIndex previous_index = items[previous_item].index;

        int current_height = delegate->sizeHint(fontMetrics, option,
                                                d->model, current_index).height();
        int previous_height = delegate->sizeHint(fontMetrics, option,
                                                 d->model, previous_index).height();
        int current_y = d->coordinateAt(current_value, current_height);
        int previous_y = d->coordinateAt(previous_value, previous_height);

        dy = current_y - previous_y;
        if (current_item > previous_item)
            for (int i = previous_item; i < current_item; ++i)
                dy -= delegate->sizeHint(fontMetrics, option, d->model, items[i].index).height();
        else if (current_item < previous_item)
            for (int i = previous_item; i > current_item; --i)
                dy += delegate->sizeHint(fontMetrics, option, d->model, items[i].index).height();

        verticalScrollBar()->repaint();
    }

    d->viewport->scroll(dx, dy);
}

/*!
  This slot is called whenever the items in the tree view are changed.

*/

void QTreeView::dataChanged()
{
    QAbstractItemView::dataChanged(QModelIndex(), QModelIndex());
}

/*!
  \fn void QTreeView::rowsInserted(const QModelIndex &parent, int first, int last)

  Informs the view that the rows from the \a first to the \a last
  inclusive have been inserted into the \a parent model item.
*/

void QTreeView::rowsInserted(const QModelIndex &parent, int, int)
{
    d->relayout(parent);
}

/*!
  \fn void QTreeView::rowsRemoved(const QModelIndex &parent, int first, int last)

  Informs the view that the rows from the \a first to the \a last
  inclusive have been removed from the given \a parent model item.*/
void QTreeView::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    if (d->items.isEmpty())
        return;

    for (int i = start; i <= end; ++i) {
        QModelIndex idx = model()->index(i, 0, parent);
        close(model()->index(i, 0, parent));
    }
    int offset = parent.isValid() ? d->viewIndex(parent) : 0;
    qCollapse<QTreeViewItem>(d->items, offset + start, end - start + 1);

    d->opened.clear(); // ### FIXME: do not collapse everything
}

/*!
  \fn void QTreeView::columnCountChanged(int first, int last)

  Informs the tree view that the columns from the \a first to the
  \a last inclusive as changed.
*/

void QTreeView::columnCountChanged(int, int)
{
    if (isVisible())
        updateGeometries();
}

/*!
  Resizes the \a column given to the size of its contents.
  If \a checkHeader is true, the contents of the header section will be
  taken into consideration.
*/

void QTreeView::resizeColumnToContents(int column, bool checkHeader)
{
    int contents = columnSizeHint(column);
    int header = checkHeader ? d->header->sectionSizeHint(column) : 0;
    d->header->resizeSection(column, qMax(contents, header));
}

/*!
    \fn void QTreeView::columnWidthChanged(int column, int oldSize, int newSize)

  Changes the \a column's width from the size specified by \a oldSize to
  the size specified by \a newSize.
*/

void QTreeView::columnWidthChanged(int column, int, int)
{
    bool reverse = QApplication::reverseLayout();
    int x = d->header->sectionPosition(column) - d->header->offset()
            - (reverse ? d->header->sectionSize(column) : 0);
    QRect rect(x, 0, d->viewport->width() - x, d->viewport->height());
    d->viewport->update(rect.normalize());
    updateGeometries();
}

/*!
  Updates the items in the tree view.
  \internal
*/

void QTreeView::updateGeometries()
{
    QSize hint = d->header->sizeHint();
    setViewportMargins(0, hint.height(), 0, 0);

    QRect vg = d->viewport->geometry();
    if (QApplication::reverseLayout())
        d->header->setOffset(vg.width() - hint.width());
    d->header->setGeometry(vg.left(), vg.top() - hint.height(), vg.width(), hint.height());

    if (!d->model)
        return;

    // update sliders
    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    QAbstractItemModel *model = d->model;

    // vertical
    int h = d->viewport->height();
    int item = d->items.count();
    if (h <= 0 || item <= 0) // if we have no viewport or no rows, there is nothing to do
        return;
    QModelIndex index = model->index(0, 0);
    QSize def = delegate->sizeHint(fontMetrics(), option, model, index);
    verticalScrollBar()->setPageStep(h / def.height() * verticalFactor());
    while (h > 0 && item > 0)
        h -= delegate->sizeHint(fontMetrics(), option, model, d->modelIndex(--item)).height();
    int max = item * verticalFactor();
    if (h < 0)
         max += 1 + (verticalFactor() * -h /
                     delegate->sizeHint(fontMetrics(), option, model, d->modelIndex(item)).height());
    verticalScrollBar()->setRange(0, max);

    int w = d->viewport->width();
    int col = d->model->columnCount(root());
    if (w <= 0 || col <= 0 || def.isEmpty()) // if we have no viewport or no columns, there is nothing to do
        return;
    horizontalScrollBar()->setPageStep(w / def.width() * horizontalFactor());
    while (w > 0 && col > 0)
        w -= d->header->sectionSize(--col);
    max = col * horizontalFactor();
    if (w < 0)
        max += (horizontalFactor() * -w / d->header->sectionSize(col));
    horizontalScrollBar()->setRange(0, max);

    QAbstractItemView::updateGeometries();
}

/*!
  Moves the vertical scroll bar in the way described by the \a action.

  \sa QScrollBar::SliderAction

*/

void QTreeView::verticalScrollbarAction(int action)
{
    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    QAbstractItemModel *model = d->model;

    int factor = d->verticalFactor;
    int value = verticalScrollBar()->value();
    int item = value / factor;
    int iheight = delegate->sizeHint(fontMetrics(), option, model, d->modelIndex(item)).height();
    int above = (value % factor) * iheight;
    int y = -(above / factor); // above the page

    if (action == QScrollBar::SliderPageStepAdd) {

        // go down to the bottom of the page
        int h = d->viewport->height();
        while (y < h && item < d->items.count())
            y += delegate->sizeHint(fontMetrics(), option, model, d->modelIndex(item++)).height();
        value = item * factor; // i is now the last item on the page
        if (y > h && item)
            value -= factor * (y - h) / delegate->sizeHint(fontMetrics(), option, model,
                                                           d->modelIndex(item - 1)).height();
        verticalScrollBar()->setSliderPosition(value);

    } else if (action == QScrollBar::SliderPageStepSub) {

        y += d->viewport->height();

        // go up to the top of the page
        while (y > 0 && item > 0)
            y -= delegate->sizeHint(fontMetrics(), option, model, d->modelIndex(--item)).height();
        value = item * factor; // i is now the first item in the page

        if (y < 0)
            value += factor * -y / delegate->sizeHint(fontMetrics(), option, model,
                                                      d->modelIndex(item)).height();
        verticalScrollBar()->setSliderPosition(value);
    }
}

/*!
Moves the horizontal scroll bar in the way described by the \a action.

\sa QScrollBar::SliderAction
*/

void QTreeView::horizontalScrollbarAction(int action)
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
        while (x < w && column < d->model->columnCount(root()))
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
  Returns the size hint for the \a column's width.

  \sa QWidget::sizeHint
*/

int QTreeView::columnSizeHint(int column) const
{
    if (d->items.count() <= 0)
        return 0;

    QStyleOptionViewItem option = viewOptions();
    QFontMetrics fontMetrics(this->fontMetrics());
    QAbstractItemDelegate *delegate = itemDelegate();
    QModelIndex index;

    const QTreeViewItem *items = d->items.constData();
    int v = verticalScrollBar()->value();
    int h = d->viewport->height();
    int c = d->items.count();
    int i = d->itemAt(v);
    int s = delegate->sizeHint(fontMetrics, option, d->model, items[i].index).height();
    int y = d->coordinateAt(v, s);
    int w = 0;
    QSize size;

    while (y < h && i < c) {
        index = items[i].index;
        index = d->model->sibling(index.row(), column, index);
        size = delegate->sizeHint(fontMetrics, option, d->model, index);
        w = qMax(w, size.width() + (column == 0 ? d->indentation(i) : 0));
        y += size.height();
        ++i;
    }

    return w;
}

/*
  private implementation
*/

void QTreeViewPrivate::open(int i, bool update)
{
    QModelIndex index = items.at(i).index;

    if (!model->hasChildren(index) || d->items.at(i).open)
        return;

    opened.append(index);

    items[i].open = true;
    layout(i);

    if (update) {
        q->updateGeometries();
        viewport->update();
        qApp->processEvents();
    }

    // make sure we open children that are already open
    reopenChildren(index, update);

    if (update)
        emit q->expanded(index);
}

void QTreeViewPrivate::close(int i, bool update)
{
    int total = items.at(i).total;
    QModelIndex index = items.at(i).index;
    int idx = opened.indexOf(index);
    if (idx >= 0)
        opened.remove(idx);
    items[i].open = false;

    idx = i;
    QModelIndex tmp = index;
    while (tmp.isValid() && tmp != d->root) {
        items[idx].total -= total;
        tmp = model->parent(tmp);
        idx = viewIndex(tmp);
    }
    qCollapse<QTreeViewItem>(items, i, total);

    if (update) {
        q->updateGeometries();
        viewport->update();
        emit q->collapsed(index);
    }
}

void QTreeViewPrivate::layout(int i)
{
    QModelIndex current;
    QModelIndex parent = modelIndex(i);
    int count = model->rowCount(parent);

    if (i == -1)
        items.resize(count);
    else
        qExpand<QTreeViewItem>(items, i, count);

    int level = i >= 0 ? items.at(i).level + 1 : 0;
    int first = i + 1;
    for (int j = first; j < first + count; ++j) {
        current = model->index(j - first, 0, parent);
        items[j].index = current;
        items[j].level = level;
    }

    int k = i;
    QModelIndex root = q->root();
    while (parent != root) {
        items[k].total += count;
        parent = model->parent(parent);
        k = viewIndex(parent);
    }
}

int QTreeViewPrivate::pageUp(int i) const
{
    int idx = item(coordinate(i) - viewport->height());
    return idx == -1 ? first() : idx;
}

int QTreeViewPrivate::pageDown(int i) const
{
    int idx = item(coordinate(i) + viewport->height());
    return idx == -1 ? last() : idx;
}

int QTreeViewPrivate::above(int i) const
{
    int idx = i;
    while (--idx >= 0 && items.at(idx).hidden);
    return idx >= 0 ? idx : i;
}

int QTreeViewPrivate::below(int i) const
{
    int idx = i;
    while (++idx < items.count() && items.at(idx).hidden);
    return idx < items.count() ? idx : i;
}

int QTreeViewPrivate::first() const
{
    int i = -1;
    while (++i < items.count() && items.at(i).hidden);
    return i < items.count() ? i : -1;
}

int QTreeViewPrivate::last() const
{
    int i = items.count();
    while (--i >= 0 && items.at(i).hidden);
    return i >= 0 ? i : -1;
}

int QTreeViewPrivate::indentation(int i) const
{
    if (i < 0 || i >= items.count())
        return 0;
    int level = items.at(i).level;
    if (rootDecoration)
        level++;
    return level * indent;
}

int QTreeViewPrivate::coordinate(int item) const
{
    QStyleOptionViewItem option = q->viewOptions();
    QFontMetrics fontMetrics(q->fontMetrics());
    int v = q->verticalScrollBar()->value();
    int i = itemAt(v); // first item (may start above the page)
    int ih = delegate->sizeHint(fontMetrics, option, model, items.at(i).index).height();
    int y = coordinateAt(v, ih); // the part of the item above the page
    int h = q->viewport()->height();
    if (i <= item) {
        while (y < h && i < items.count()) {
            if (i == item)
                return y; // item is visible - actual y in viewport
            y += delegate->sizeHint(fontMetrics, option, model, items.at(i).index).height();
            ++i;
        }
        // item is below the viewport - estimated y
        return y + (itemHeight * (item - itemAt(v)));
    }
    // item is above the viewport - estimated y
    return y - (itemHeight * (i - item));
}

int QTreeViewPrivate::item(int coordinate) const
{
    QStyleOptionViewItem option = q->viewOptions();
    QFontMetrics fontMetrics(q->fontMetrics());

    int v = q->verticalScrollBar()->value();
    int i = itemAt(v);
    if (i >= items.count())
        return -1;

    int s = delegate->sizeHint(fontMetrics, option, model, items.at(i).index).height();
    int y = coordinateAt(v, s);
    int h = q->viewport()->height();
    if (coordinate >= y) {
        // search for item in viewport
        while (y < h && i < items.count()) {
            y += delegate->sizeHint(fontMetrics, option, model, items.at(i).index).height();
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

int QTreeViewPrivate::viewIndex(const QModelIndex &index) const
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

QModelIndex QTreeViewPrivate::modelIndex(int i) const
{
    if (i < 0 || i >= items.count())
        return q->root();
    return items.at(i).index;
}

int QTreeViewPrivate::itemAt(int value) const
{
    return value / q->verticalFactor();
}

int QTreeViewPrivate::coordinateAt(int value, int iheight) const
{
    int factor = q->verticalFactor();
    int above = (value % factor) * iheight; // what's left; in "item units"
    return -(above / factor); // above the page
}

int QTreeViewPrivate::columnAt(int x) const
{
    int hx = x + header->offset() - header->x();
    if (QApplication::reverseLayout() && q->verticalScrollBar()->isVisible())
        hx += q->verticalScrollBar()->width();
    return header->sectionAt(hx);
}

void QTreeViewPrivate::relayout(const QModelIndex &parent)
{
    if (!q->isVisible()) {
        items.clear();
        return;
    }
    // do a local relayout of the items
    if (parent.isValid()) {
        int p = viewIndex(parent);
        if (p > -1 && items.at(p).open) {
            close(p, false);
            open(p, false);
            q->updateGeometries();
            viewport->update();
        }

    } else {
        items.clear();
        q->doItemsLayout();
    }
}

void QTreeViewPrivate::reopenChildren(const QModelIndex &parent, bool update)
{
    // FIXME: this is slow: optimize
    QVector<QModelIndex> o = opened;
    for (int j = 0; j < o.count(); ++j) {
        if (model->parent(o.at(j)) == parent) {
            int k = opened.indexOf(o.at(j));
            opened.remove(k);
            int v = viewIndex(o.at(j));
            open(v, update);
        }
    }
}
