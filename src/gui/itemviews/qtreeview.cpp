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
#include <qsignal.h>

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

    \quotefromfile snippets/shareddirmodel/main.cpp
    \skipto QDirModel *model
    \printuntil QTreeView *tree
    \skipto tree->setModel(
    \printuntil tree->setModel(

    The model/view architecture ensures that the contents of the tree view
    are updated as the model changes.

    Items that have children can be in an expanded (children are
    visible) or collapsed (children are hidden) state. When this state
    changes a collapsed() or expanded() signal is emitted with the
    model index of the relevant item.

    The amount of indentation used to indicate levels of hierarchy is
    controlled by the \l indentation property.

    Headers in a tree view are constructed using the QHeaderView class.

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
    initialize();
}

/*!
  \internal
*/
QTreeView::QTreeView(QTreeViewPrivate &dd, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    initialize();
}

/*!
  Destroys the tree view.
*/
QTreeView::~QTreeView()
{
}

/*!
  \internal
*/
void QTreeView::initialize()
{
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);

    QHeaderView *header = new QHeaderView(Qt::Horizontal, this);
    header->setMovable(true);
    header->setStretchLastSection(true);
    setHeader(header);
}

/*!
  \reimp
*/
void QTreeView::setModel(QAbstractItemModel *model)
{
    if (d->selectionModel && d->model) // support row editing
        disconnect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex, QModelIndex)),
                   d->model, SLOT(submit()));

    d->viewItems.clear();
    d->openedIndexes.clear();
    d->hiddenIndexes.clear();
    d->header->setModel(model);
    QAbstractItemView::setModel(model);
}

/*!
  \reimp
*/
void QTreeView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    Q_ASSERT(selectionModel);
    if (d->model && d->selectionModel) // support row editing
        disconnect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                   d->model, SLOT(submit()));

    d->header->setSelectionModel(selectionModel);
    QAbstractItemView::setSelectionModel(selectionModel);

    if (d->model && d->selectionModel) // support row editing
        connect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                d->model, SLOT(submit()));
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
    Q_ASSERT(header);
    delete d->header;
    d->header = header;

    connect(d->header, SIGNAL(sectionResized(int,int,int)),
            this, SLOT(columnResized(int,int,int)),Qt::QueuedConnection);
    connect(d->header, SIGNAL(sectionMoved(int,int,int)),
            this, SLOT(columnMoved()),Qt::QueuedConnection);
    connect(d->header, SIGNAL(sectionCountChanged(int,int)),
            this, SLOT(columnCountChanged(int,int)),Qt::QueuedConnection);
    connect(d->header, SIGNAL(sectionHandleDoubleClicked(int)),
            this, SLOT(resizeColumnToContents(int)));
    connect(d->header, SIGNAL(sectionClicked(int)),
            this, SLOT(sortByColumn(int)));
    d->header->setFocusProxy(this);
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
  \property QTreeView::uniformRowHeights
  \brief whether all items in the treeview have the same height

  This property should only be set to true if it is guarantied that all items
  in the view has the same height. This enables the view to do some
  optimizations.
*/
bool QTreeView::uniformRowHeights() const
{
    return d->uniformRowHeights;
}

void QTreeView::setUniformRowHeights(bool uniform)
{
    d->uniformRowHeights = uniform;
}

/*!
  Returns the horizontal position of the \a column in the viewport.
*/
int QTreeView::columnViewportPosition(int column) const
{
    return d->header->sectionViewportPosition(column);
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
    return d->header->logicalIndexAt(x);
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
  If \a hide is true the \a column is hidden, otherwise the \a column is shown.
*/
void QTreeView::setColumnHidden(int column, bool hide)
{
    d->header->setSectionHidden(column, hide);
}

/*!
    Returns true if the item in the given \a row of the \a parent is hidden;
    otherwise returns false.

    \sa setRowHidden()
*/
bool QTreeView::isRowHidden(int row, const QModelIndex &parent) const
{
    if (d->hiddenIndexes.count() <= 0)
        return false;
    QModelIndex index = model()->index(row, 0, parent);
    QPersistentModelIndex persistent(index);
    return d->hiddenIndexes.contains(persistent);
}

/*!
  If \a hide is true the \a row with the given \a parent is hidden, otherwise the \a row is shown.

  \sa isRowHidden()
*/
void QTreeView::setRowHidden(int row, const QModelIndex &parent, bool hide)
{
    QModelIndex index = model()->index(row, 0, parent);
    QPersistentModelIndex persistent(index);
    if (hide) {
        d->hiddenIndexes.append(persistent);
    } else {
        int i = d->hiddenIndexes.indexOf(persistent);
        d->hiddenIndexes.remove(i);
    }

    d->relayout(parent);
}

/*!
  \reimp
*/
void QTreeView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    // set the height to be 0, so we get a new sizehint next time we ask for the row height
    QModelIndex top = model()->sibling(topLeft.row(), 0, topLeft);
    QModelIndex bottom = model()->sibling(bottomRight.row(), 0, bottomRight);
    int topViewIndex = d->viewIndex(top);
    int bottomViewIndex = d->viewIndex(bottom);
    if (topViewIndex >= 0)
        for (int i = topViewIndex; i <= bottomViewIndex; ++i)
            d->viewItems[i].height = 0;
    QAbstractItemView::dataChanged(topLeft, bottomRight);
}

/*!
  Hides the \a column given.

  \sa showColumn()
*/
void QTreeView::hideColumn(int column)
{
    d->header->hideSection(column);
}

/*!
  Shows the given \a column in the tree view.

  \sa hideColumn()
*/
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
    if (idx > -1) { // is visible
        d->open(idx);
        updateGeometries();
        viewport()->update();
    } else {
        d->openedIndexes.append(index);
    }
}

/*!
  \fn void QTreeView::close(const QModelIndex &index)

  Closes the model item specified by the \a index.
*/
void QTreeView::close(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    int i = d->viewIndex(index);
    if (i > -1) { // is visible
        d->close(i);
        updateGeometries();
        viewport()->update();
    } else {
        i = d->openedIndexes.indexOf(index);
        if (i > -1)
            d->openedIndexes.remove(i);
    }
}

/*!
  \fn bool QTreeView::isOpen(const QModelIndex &index) const

  Returns true if the model item \a index is open; otherwise returns
  false.
*/
bool QTreeView::isOpen(const QModelIndex &index) const
{
    int i = d->viewIndex(index);
    if (i > -1) // is visible - FIXME: this is a workaround for a bug!
        return d->viewItems.at(i).open;
    return d->openedIndexes.contains(index);
}

/*!
  Sets the item referred to by \a index to either closed or opened,
  depending on the value of \a open.

  \sa open, close
*/
void QTreeView::setOpen(const QModelIndex &index, bool open)
{
    if (open)
        this->open(index);
    else
        this->close(index);
}

/*!
  Returns the rectangle on the viewport occupied by the item at \a
  index.
*/
QRect QTreeView::viewportRectForIndex(const QModelIndex &index) const
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
    int h = d->height(vi);
    return QRect(x, y, w, h);
}

/*!
  Scroll the contents of the tree view until the given model item \a index
  is visible.
*/
void QTreeView::ensureVisible(const QModelIndex &index)
{
    // check if we really need to do anything
    if (isIndexHidden(index))
        return;
    QRect rect = viewportRectForIndex(index);
    if (rect.isEmpty())
        return;
    QRect area = d->viewport->rect();
    if (area.contains(rect)) {
        d->viewport->repaint(rect);
        return;
    }

    // vertical
    if (rect.top() < area.top()) { // above
        int i = d->viewIndex(index);
        verticalScrollBar()->setValue(i * verticalFactor());
    } else if (rect.bottom() > area.bottom()) { // below
        int i = d->viewIndex(index);
        if (i < 0) {
            qWarning("ensureVisible: item index was illegal: %d", i);
            return;
        }
        int y = area.height();
        while (y > 0 && i > 0)
            y -= d->height(i--);
        int h = d->height(i);
        int a = (-y * verticalFactor()) / (h ? h : 1);
        verticalScrollBar()->setValue(++i * verticalFactor() + a);
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
}

/*!
  \reimp
*/
void QTreeView::paintEvent(QPaintEvent *e)
{
    QStyleOptionViewItem option = viewOptions();
    const QBrush base = option.palette.base();
    const QRect area = e->rect();

    if (d->viewItems.isEmpty() || d->header->count() == 0) {
        QPainter painter(d->viewport);
        painter.fillRect(area, base);
        return;
    }

    QPainter painter(d->viewport);

    d->left = d->header->visualIndexAt(area.left());
    d->right = d->header->visualIndexAt(area.right());

    if (isRightToLeft()) {
        d->left = (d->left == -1 ? d->header->count() - 1 : d->left);
        d->right = (d->right == -1 ? 0 : d->right);
    } else {
        d->left = (d->left == -1 ? 0 : d->left);
        d->right = (d->right == -1 ? d->header->count() - 1 : d->right);
    }

    int tmp = d->left;
    d->left = qMin(d->left, d->right);
    d->right = qMax(tmp, d->right);

    const QStyle::State state = option.state;
    const bool alternate = d->alternatingColors;
    const QColor oddColor = d->oddRowColor();
    const QColor evenColor = d->evenRowColor();
    const int t = area.top();
    const int b = area.bottom() + 1;
    const int v = verticalScrollBar()->value();
    const int c = d->viewItems.count();
    const QVector<QTreeViewItem> viewItems = d->viewItems;

    int i = d->itemAt(v); // first item
    int y = d->topItemDelta(v, d->height(i));

    while (y < b && i < c) {
        int h = d->height(i); // actual height
        if (y + h >= t) { // we are in the update area
            option.rect.setRect(0, y, 0, h);
            option.state = state | (viewItems.at(i).open ? QStyle::State_Open : QStyle::State_None);
            if (alternate)
                option.palette.setColor(QPalette::Base, i & 1 ? oddColor : evenColor);
            d->current = i;
            drawRow(&painter, option, viewItems.at(i).index);
        }
        y += h;
        ++i;
    }

    int w = d->viewport->width();
    int x = d->header->length();
    QRect bottom(0, y, w, b - y);
    if (y < b && area.intersects(bottom))
        painter.fillRect(bottom, base);
    if (isRightToLeft()) {
        QRect right(0, 0, w - x, b);
        if (x > 0 && area.intersects(right))
            painter.fillRect(right, base);
    } else {
        QRect left(x, 0, w - x, b);
        if (x < w && area.intersects(left))
            painter.fillRect(left, base);
    }

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
    const QBrush base = option.palette.base();
    const int y = option.rect.y();
    const QModelIndex parent = index.parent();
    const QHeaderView *header = d->header;
    const QModelIndex current = selectionModel()->currentIndex();
    const bool focus = hasFocus() && current.isValid();
    const bool reverse = isRightToLeft();
    const QStyle::State state = opt.state;

    int width, height = option.rect.height();

    int position;
    int headerSection;
    QModelIndex modelIndex;

    for (int headerIndex = d->left; headerIndex <= d->right; ++headerIndex) {
        headerSection = d->header->logicalIndex(headerIndex);
        if (header->isSectionHidden(headerSection))
            continue;
        position = columnViewportPosition(headerSection);
        width = header->sectionSize(headerSection);
        modelIndex = d->model->index(index.row(), headerSection, parent);
        if (!modelIndex.isValid())
            continue;
        opt.state = state;
        opt.state |= (focus && current == modelIndex ? QStyle::State_HasFocus : QStyle::State_None);
        opt.state |= (selectionModel()->isSelected(modelIndex)
                     ? QStyle::State_Selected : QStyle::State_None);
        if ((model()->flags(index) & QAbstractItemModel::ItemIsEnabled) == 0)
            opt.state &= ~QStyle::State_Enabled;
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
        itemDelegate()->paint(painter, opt, modelIndex);
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
    const bool reverse = isRightToLeft();
    const int indent = d->indent;
    const int outer = d->rootDecoration ? 0 : 1;
    int level = d->viewItems.at(d->current).level;
    QRect primitive(reverse ? rect.left() : rect.right(), rect.top(), indent, rect.height());

    QModelIndex parent = index.parent();
    QModelIndex current = parent;
    QModelIndex ancestor = current.parent();

    QStyleOption opt(0);
    opt.palette = palette();
    if (level >= outer) {
        // start with the innermost branch
        primitive.moveLeft(reverse ? primitive.left() : primitive.left() - indent);
        opt.rect = primitive;
        opt.state = QStyle::State_Item
                    | (d->model->rowCount(parent) - 1 > index.row()
                      ? QStyle::State_Sibling : QStyle::State_None)
                    | (model()->hasChildren(index) ? QStyle::State_Children : QStyle::State_None)
                    | (d->viewItems.at(d->current).open ? QStyle::State_Open : QStyle::State_None);
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
    }
    // then go out level by level
    for (--level; level >= outer; --level) { // we have already drawn the innermost branch
        primitive.moveLeft(reverse ? primitive.left() + indent : primitive.left() - indent);
        opt.rect = primitive;
        opt.state = (d->model->rowCount(ancestor) - 1 > current.row())
                    ? QStyle::State_Sibling : QStyle::State_None;
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
        current = ancestor;
        ancestor = current.parent();
    }
}

/*!
  \reimp
*/
void QTreeView::mousePressEvent(QMouseEvent *e)
{
    int i = d->itemDecorationAt(e->pos());
    if (i == -1) {
        QAbstractItemView::mousePressEvent(e);
    } else {
        if (d->viewItems.at(i).open) {
            setState(QAbstractItemView::OpeningState);
            d->close(i);
        } else {
            setState(QAbstractItemView::ClosingState);
            d->open(i);
        }
        updateGeometries();
        viewport()->update();
    }
}

/*!
  \reimp
*/
void QTreeView::mouseDoubleClickEvent(QMouseEvent *e)
{
    int i = d->itemDecorationAt(e->pos());
    if (i == -1)
        QAbstractItemView::mouseDoubleClickEvent(e);
}

/*!
  \fn QModelIndex QTreeView::indexAt(int x, int y) const

  Returns the model index of the item at point (\a x, \a y).
*/
QModelIndex QTreeView::indexAt(int x, int y) const
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
QModelIndex QTreeView::indexAbove(const QModelIndex &index) const
{
    int above = d->above(d->viewIndex(index));
    return d->modelIndex(above);
}

/*!
  Returns the model index of the item below \a index.
*/
QModelIndex QTreeView::indexBelow(const QModelIndex &index) const
{
    int below = d->below(d->viewIndex(index));
    return d->modelIndex(below);
}

/*!
  Lays out the items in the tree view.
*/
void QTreeView::doItemsLayout()
{
    d->viewItems.clear(); // prepare for new layout
    QStyleOptionViewItem option = viewOptions();
    QModelIndex parent = rootIndex();
    if (model() && model()->rowCount(parent) > 0 && model()->columnCount(parent) > 0) {
        QModelIndex index = model()->index(0, 0, parent);
        d->itemHeight = itemDelegate()->sizeHint(option, index).height();
        d->layout(-1);
        d->reopenChildren(parent);
    }
    QAbstractItemView::doItemsLayout();
}

/*!
  \reimp
*/
void QTreeView::reset()
{
    d->openedIndexes.clear();
    d->hiddenIndexes.clear();
    d->viewItems.clear();
    QAbstractItemView::reset();
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
    int item = verticalScrollBar()->value() / d->verticalFactor;
    if (model()->rowCount(rootIndex()) > 0 && model()->columnCount(rootIndex()) > 0)
        return item * d->itemHeight;
    return item * 30;
}

/*!
  Move the cursor in the way described by \a cursorAction, using the
  information provided by the button \a state.

  \sa QAbstractItemView::CursorAction
*/
QModelIndex QTreeView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                  Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);

    QModelIndex current = currentIndex();
    if (!current.isValid())
        return current;
    int vi = d->viewIndex(current);
    if (vi < 0)
        return current;
    switch (cursorAction) {
    case MoveNext:
    case MoveDown:
        return d->modelIndex(d->below(vi));
    case MovePrevious:
    case MoveUp:
        return d->modelIndex(d->above(vi));
    case MoveLeft:
        if (d->viewItems.at(vi).open)
            d->close(vi);
        updateGeometries();
        viewport()->update();
        break;
    case MoveRight:
        if (!d->viewItems.at(vi).open)
            d->open(vi);
        updateGeometries();
        viewport()->update();
        break;
    case MovePageUp:
        return d->modelIndex(d->pageUp(vi));
    case MovePageDown:
        return d->modelIndex(d->pageDown(vi));
    case MoveHome:
        return model()->index(0, 0, rootIndex());
    case MoveEnd:
        return d->modelIndex(d->viewItems.count() - 1);
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
    if (!selectionModel())
        return;
    int start = d->viewIndex(indexAt(isRightToLeft() ? rect.right() : rect.left(), rect.top()));
    int stop = d->viewIndex(indexAt(isRightToLeft() ? rect.left() : rect.right(), rect.bottom()));
    d->select(start, stop, command);
}

/*!
  Returns the rectangle from the viewport of the items in the given
  \a selection.
*/
QRect QTreeView::selectionViewportRect(const QItemSelection &selection) const
{
    if (selection.count() <= 0 || d->viewItems.count() <= 0)
        return QRect();

    int top = d->viewItems.count();
    int left = d->header->count();
    int bottom = 0;
    int right = 0;
    QItemSelectionRange r;

    for (int i = 0; i < selection.count(); ++i) {
        r = selection.at(i);
        top = qMin(d->viewIndex(r.topLeft()), top);
        bottom = qMax(d->viewIndex(r.bottomRight()), bottom);
        left = qMin(r.left(), left);
        right = qMax(r.right(), right);
    }

    QModelIndex tl = d->modelIndex(top);
    QModelIndex br = d->modelIndex(bottom);
    if (!(tl.isValid() && br.isValid()))
        return QRect();

    int bottomHeight = d->height(bottom);
    int bottomPos = d->coordinate(bottom) + bottomHeight;
    int topPos = d->coordinate(top);

    QRect rect(0, topPos, d->viewport->width(), bottomPos - topPos); // always the width of a row
    return rect.normalize();
}

/*!
  \reimp
*/
QModelIndexList QTreeView::selectedIndexes() const
{
    QModelIndexList viewSelected;
    QModelIndexList modelSelected = selectionModel()->selectedIndexes();
    for (int i = 0; i < modelSelected.count(); ++i) {
        // check that neither the parents nor the index is hidden before we add
        QModelIndex index = modelSelected.at(i);
        while (index.isValid() && !isIndexHidden(index))
            index = index.parent();
        if (index.isValid())
            continue;
        viewSelected.append(modelSelected.at(i));
    }
    return viewSelected;
}

/*!
  Scrolls the contents of the tree view by (\a dx, \a dy).
*/
void QTreeView::scrollContentsBy(int dx, int dy)
{
    // guestimate the number of items in the viewport
    int viewCount = d->viewport->height() / d->itemHeight;
    int maxDeltaY = verticalFactor() * qMin(d->viewItems.count(), viewCount);

    // no need to do a lot of work if we are going to redraw the whole thing anyway
    if (qAbs(dy) > maxDeltaY) {
        verticalScrollBar()->repaint();
        d->viewport->update();
        return;
    }

    if (dx) {
        int scrollbarValue = horizontalScrollBar()->value();
        int column = d->header->logicalIndex(scrollbarValue / d->horizontalFactor);
        int left = (scrollbarValue % d->horizontalFactor) * d->header->sectionSize(column);
        int offset = (left / d->horizontalFactor) + d->header->sectionPosition(column);
        if (isRightToLeft())
            dx = offset - d->header->offset();
        else
            dx = d->header->offset() - offset;
        d->header->setOffset(offset);
    }

    if (dy) {
        int currentScrollbarValue = verticalScrollBar()->value();
        int previousScrollbarValue = currentScrollbarValue + dy; // -(-dy)
        int currentViewIndex = currentScrollbarValue / d->verticalFactor; // the first visible item
        int previousViewIndex = previousScrollbarValue / d->verticalFactor;

        const QVector<QTreeViewItem> viewItems = d->viewItems;

        int currentY = d->topItemDelta(currentScrollbarValue, d->height(currentViewIndex));
        int previousY = d->topItemDelta(previousScrollbarValue, d->height(previousViewIndex));

        dy = currentY - previousY;
        if (previousViewIndex < currentViewIndex) { // scrolling down
            for (int i = previousViewIndex; i < currentViewIndex; ++i)
                dy -= d->height(i);
        } else if (previousViewIndex > currentViewIndex) { // scrolling up
            for (int i = previousViewIndex - 1; i >= currentViewIndex; --i)
                dy += d->height(i);
        }
    }

    d->viewport->scroll(dx, dy);
}

/*!
  This slot is called whenever a column has been moved.
*/
void QTreeView::columnMoved()
{
    QAbstractItemView::dataChanged(QModelIndex(), QModelIndex());
}

/*!
  \internal
*/
void QTreeView::reopen()
{
    if (d->reopen == -1)
        return;
    d->open(d->reopen);
    d->reopen = -1;
    viewport()->update();
}

/*!
  Informs the view that the rows from the \a start row to the \a end row
  inclusive have been inserted into the \a parent model item.
*/
void QTreeView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    d->relayout(parent);
    QAbstractItemView::rowsInserted(parent, start, end);
}

/*!
  Informs the view that the rows from the \a start row to the \a end row
  inclusive have been removed from the given \a parent model item.
*/
void QTreeView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    if (d->viewItems.isEmpty())
        return;

    // close all children
    for (int i = start; i <= end; ++i)
        close(model()->index(i, 0, parent));

    // close parent
    int p = d->viewIndex(parent);
    if (p > 0) {
        d->close(p);
        // reopen parent using a delayed function call
        d->reopen = p; // p is safe because all the changes happens after this one
        int slot = metaObject()->indexOfSlot("reopen()");
        QApplication::postEvent(this, new QMetaCallEvent(slot));
    } else {
        d->viewItems.clear();
        d->doDelayedItemsLayout();
    }
}

/*!
  Informs the tree view that the number of columns in the tree view has
  changed from \a oldCount to \a newCount.
*/
void QTreeView::columnCountChanged(int, int)
{
    if (isVisible())
        updateGeometries();
}

/*!
  Resizes the \a column given to the size of its contents.
*/
void QTreeView::resizeColumnToContents(int column)
{
    int contents = sizeHintForColumn(column);
    int header = d->header->isHidden() ? 0 : d->header->sectionSizeHint(column);
    d->header->resizeSection(column, qMax(contents, header));
}

/*!
  Sorts the model by the values in the given \a column.
 */
void QTreeView::sortByColumn(int column)
{
    if (!d->model)
        return;
    bool ascending = (header()->sortIndicatorSection() == column
                      && header()->sortIndicatorOrder() == Qt::DescendingOrder);
    Qt::SortOrder order = ascending ? Qt::AscendingOrder : Qt::DescendingOrder;
    header()->setSortIndicator(column, order);
    d->model->sort(column, order);
    if (!header()->isSortIndicatorShown())
        header()->setSortIndicatorShown(true);
}

/*!
    Selects all the items in the underlying model.
*/
void QTreeView::selectAll()
{
    d->select(0, d->viewItems.count() - 1,
              QItemSelectionModel::ClearAndSelect
              |QItemSelectionModel::Rows);
}

/*!
  This column is called whenever the column size is changed in the header.
*/
void QTreeView::columnResized(int column, int, int)
{
    int x = columnViewportPosition(column);
    QRect rect;
    if (isRightToLeft())
        rect.setRect(0, 0, x + d->header->sectionSize(column), d->viewport->height());
    else
        rect.setRect(x, 0, d->viewport->width() - x, d->viewport->height());
    d->viewport->update(rect.normalize());
    updateGeometries();
}

/*!
  Updates the items in the tree view.
  \internal
*/
void QTreeView::updateGeometries()
{
    QSize hint = d->header->isHidden() ? QSize(0, 0) : d->header->sizeHint();
    setViewportMargins(0, hint.height(), 0, 0);

    QRect vg = d->viewport->geometry();
    QRect geometryRect(vg.left(), vg.top() - hint.height(), vg.width(), hint.height());
    d->header->setGeometry(geometryRect);

    // make sure that the header sections are resized, even if the header is hidden
    if (d->header->isHidden() && d->header->stretchSectionCount()) {
        d->header->viewport()->setGeometry(geometryRect);
        qInvokeMetaMember(d->header, "resizeSections");
    }

    // update scrollbars
    if (model() && model()->rowCount(rootIndex()) > 0 && model()->columnCount(rootIndex()) > 0) {
        d->updateVerticalScrollbar();
        d->updateHorizontalScrollbar();
    }

    QAbstractItemView::updateGeometries();
}

/*!
  Returns the size hint for the \a column's width.

  \sa QWidget::sizeHint
*/
int QTreeView::sizeHintForColumn(int column) const
{
    if (d->viewItems.count() <= 0)
        return 0;

    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    QModelIndex index;

    const QVector<QTreeViewItem> viewItems = d->viewItems;
    int v = verticalScrollBar()->value();
    int h = viewport()->height();
    int i = d->itemAt(v);
    int c = viewItems.count();
    int s = d->height(i);
    int y = d->topItemDelta(v, s);
    int w = 0;
    QSize size;

    while (y < h && i < c) {
        index = viewItems.at(i).index;
        index = model()->sibling(index.row(), column, index);
        size = delegate->sizeHint(option, index);
        w = qMax(w, size.width() + (column == 0 ? d->indentation(i) : 0));
        y += size.height();
        ++i;
    }

    return w;
}

/*!
  Returns the size hint for the \a column's width.

  \sa QWidget::sizeHint
*/
int QTreeView::indexRowSizeHint(const QModelIndex &left) const
{
    if (d->viewItems.count() <= 0)
        return 0;

    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    int width = viewport()->width();
    int height = 0;
    // only check the visible columns - FIXME: is this ok ?
    int start = d->header->visualIndexAt(0);
    int end = d->header->visualIndexAt(width);

    if (isRightToLeft()) {
        start = start == -1 ? d->header->count() - 1 : start;
        end = end == -1 ? 0 : end;
    } else {
        start = start == -1 ? 0 : start;
        end = end == -1 ? d->header->count() - 1 : end;
    }

    int tmp = start;
    start = qMin(start, end);
    end = qMax(tmp, end);

    QModelIndex parent = left.parent();
    const QVector<QTreeViewItem> viewItems = d->viewItems;
    for (int column = start; column <= end; ++column) {
        QModelIndex index = d->model->index(left.row(), column, parent);
        height = qMax(height, delegate->sizeHint(option, index).height());
    }

    return height;
}

/*!
  \reimp
*/
bool QTreeView::isIndexHidden(const QModelIndex &index) const
{
    return (isColumnHidden(index.column()) || isRowHidden(index.row(), index.parent()));
}

/*
  private implementation
*/
void QTreeViewPrivate::open(int i)
{
    QModelIndex index = viewItems.at(i).index;

    if (!model->hasChildren(index) || viewItems.at(i).open)
        return;

    openedIndexes.append(index);

    viewItems[i].open = true;
    layout(i);

    // make sure we open children that are already open
    reopenChildren(index);

    emit q->expanded(index);
}

void QTreeViewPrivate::close(int i)
{
    if (i < 0 || openedIndexes.isEmpty())
        return;
    int total = viewItems.at(i).total;
    QModelIndex index = viewItems.at(i).index;
    int idx = openedIndexes.indexOf(index);
    if (idx >= 0)
        openedIndexes.remove(idx);
    viewItems[i].open = false;

    idx = i;
    QModelIndex tmp = index;
    while (tmp.isValid() && tmp != d->root) {
        Q_ASSERT(idx > -1);
        viewItems[idx].total -= total;
        tmp = tmp.parent();
        idx = viewIndex(tmp);
    }
    qCollapse<QTreeViewItem>(viewItems, i, total);

    emit q->collapsed(index);
}

void QTreeViewPrivate::layout(int i)
{
    QModelIndex current;
    QModelIndex parent = modelIndex(i);
    int count = model->rowCount(parent);

    if (i == -1)
        viewItems.resize(count);
    else
        qExpand<QTreeViewItem>(viewItems, i, count);

    int first = i + 1;
    int level = (i >= 0 ? viewItems.at(i).level + 1 : 0);
    int hidden = 0;
    int last = 0;
    for (int j = first; j < first + count; ++j) {
        current = model->index(j - first, 0, parent);
        if (q->isRowHidden(current.row(), parent)) { // slow with lots of hidden rows
            ++hidden;
            last = j - hidden;
        } else {
            last = j - hidden;
            viewItems[last].index = current;
            viewItems[last].level = level;
        }
    }

    // remove hidden items
    if (hidden > 0)
        qCollapse<QTreeViewItem>(viewItems, last, hidden);

    QModelIndex root = q->rootIndex();
    while (parent != root) {
        Q_ASSERT(i > -1);
        viewItems[i].total += count;
        parent = parent.parent();
        i = viewIndex(parent);
    }
}

int QTreeViewPrivate::pageUp(int i) const
{
    int idx = item(coordinate(i) - viewport->height());
    return idx == -1 ? 0 : idx;
}

int QTreeViewPrivate::pageDown(int i) const
{
    int idx = item(coordinate(i) + viewport->height());
    return idx == -1 ? d->viewItems.count() : idx;
}

int QTreeViewPrivate::indentation(int i) const
{
    if (i < 0 || i >= viewItems.count())
        return 0;
    int level = viewItems.at(i).level;
    if (rootDecoration)
        level++;
    return level * indent;
}

int QTreeViewPrivate::coordinate(int item) const
{
    int scrollbarValue = q->verticalScrollBar()->value();
    int viewItemIndex = itemAt(scrollbarValue); // first item (may start above the page)
    int viewItemHeight = height(viewItemIndex);
    int viewportHeight = viewport->height();
    int y = topItemDelta(scrollbarValue, viewItemHeight); // the part of the item above the page
    if (viewItemIndex <= item) {
        while (y < viewportHeight && viewItemIndex < viewItems.count()) {
            if (viewItemIndex == item)
                return y; // item is visible - actual y in viewport
            y += height(viewItemIndex);
            ++viewItemIndex;
        }
        // item is below the viewport - estimated y
        return y + (itemHeight * (item - itemAt(scrollbarValue)));
    }
    // item is above the viewport - estimated y
    return y - (itemHeight * (viewItemIndex - item));
}

int QTreeViewPrivate::item(int coordinate) const
{
    int scrollbarValue = q->verticalScrollBar()->value();
    int viewItemIndex = itemAt(scrollbarValue);
    if (viewItemIndex >= viewItems.count())
        return -1;
    int viewItemHeight = height(viewItemIndex);
    int viewportHeight = viewport->height();
    int y = topItemDelta(scrollbarValue, viewItemHeight);
    if (coordinate >= y) {
        // search for item in viewport
        while (y < viewportHeight && viewItemIndex < viewItems.count()) {
            y += height(viewItemIndex);
            if (coordinate < y)
                return viewItemIndex;
            ++viewItemIndex;
        }
        // item is below viewport - give estimated coordinate
    }
    // item is above the viewport - give estimated coordinate
    int i = viewItemIndex + ((coordinate - y) / itemHeight);
    return i < 0 || i >= viewItems.count() ? -1 : i;
}

int QTreeViewPrivate::viewIndex(const QModelIndex &index) const
{
    // NOTE: this function is slow if the item is outside the visible area
    // search in visible items first, then below
    int t = itemAt(q->verticalScrollBar()->value());
    t = t > 100 ? t - 100 : 0; // start 100 items above the visible area
    for (int i = t; i < viewItems.count(); ++i)
        if (viewItems.at(i).index.row() == index.row() &&
            viewItems.at(i).index.data() == index.data()) // ignore column
            return i;
    // search above
    for (int j = 0; j < t; ++j)
        if (viewItems.at(j).index.row() == index.row() &&
            viewItems.at(j).index.data() == index.data()) // ignore column
            return j;
    // nothing found
    return -1;
}

QModelIndex QTreeViewPrivate::modelIndex(int i) const
{
    if (i < 0 || i >= viewItems.count())
        return (QModelIndex)root;
    return viewItems.at(i).index;
}

int QTreeViewPrivate::itemAt(int value) const
{
    return value / verticalFactor;
}

int QTreeViewPrivate::topItemDelta(int value, int iheight) const
{
    int above = (value % verticalFactor) * iheight; // what's left; in "item units"
    return -(above / verticalFactor); // above the page
}

int QTreeViewPrivate::columnAt(int x) const
{
    return header->logicalIndexAt(x);
}

void QTreeViewPrivate::relayout(const QModelIndex &parent)
{
    if (!q->isVisible()) {
        viewItems.clear();
        return;
    }
    // do a local relayout of the items
    if (parent.isValid()) {
        int parentViewIndex = viewIndex(parent);
        if (parentViewIndex > -1 && viewItems.at(parentViewIndex).open) {
            close(parentViewIndex);
            open(parentViewIndex);
            q->updateGeometries();
            viewport->update();
        }
    } else {
        viewItems.clear();
        q->doItemsLayout();
    }
}

void QTreeViewPrivate::reopenChildren(const QModelIndex &parent)
{
    // FIXME: this is slow: optimize
    QVector<QPersistentModelIndex> o = openedIndexes;
    for (int j = 0; j < o.count(); ++j) {
        QModelIndex index = o.at(j);
        if (index.parent() == parent) {
            int v = viewIndex(index);
            if (v < 0)
                continue;
            int k = openedIndexes.indexOf(index);
            openedIndexes.remove(k);
            open(v);
        }
    }
}

void QTreeViewPrivate::updateVerticalScrollbar()
{
    int viewHeight = viewport->height();
    int itemCount = viewItems.count();

    // if we have no viewport or no items, there is nothing to do
    if (viewHeight <= 0 || itemCount <= 0) {
        q->verticalScrollBar()->setRange(0, 0);
        return;
    }

    // set page step size
    q->verticalScrollBar()->setPageStep(viewHeight); // FIXME: wrong

    // set the scroller range
    int y = viewHeight;
    int i = itemCount; // FIXME: wrong
    while (y > 0 && i > 0)
        y -= height(--i);
    int max = i * verticalFactor;

    if (y < 0) { // if the first item starts above the viewport, we have to backtrack
        int backtracking = verticalFactor * -y;
        int itemSize = height(i);
        if (itemSize > 0) // avoid division by zero
            max += (backtracking / itemSize) + 1;
    }

    q->verticalScrollBar()->setRange(0, max);
}

void QTreeViewPrivate::updateHorizontalScrollbar()
{
    int width = viewport->width();
    int count = header->count();

    // if we have no viewport or no columns, there is nothing to do
    if (width <= 0 || count <= 0) {
        q->horizontalScrollBar()->setRange(0, 0);
        return;
    }

    // set the scroller range
    int x = width;
    while (x > 0 && count > 0)
        x -= header->sectionSize(--count);
    int max = count * horizontalFactor;

    // set page step size
    int visibleCount = header->count() - count - 1;
    q->horizontalScrollBar()->setPageStep(visibleCount * horizontalFactor);

    if (x < 0) { // if the first item starts left of the viewport, we have to backtrack
        int sectionSize = header->sectionSize(count);
        if (sectionSize > 0) // avoid division by zero
            max += ((-x * horizontalFactor) / sectionSize) + 1;
    }

    q->horizontalScrollBar()->setRange(0, max);
}

int QTreeViewPrivate::itemDecorationAt(const QPoint &pos) const
{
    int x = pos.x();
    int column = header->logicalIndexAt(x);
    int position = header->sectionViewportPosition(column);
    int size = header->sectionSize(column);
    int cx = (q->isRightToLeft() ? size - x + position : x - position);
    int viewItemIndex = item(pos.y());
    int itemIndentation = indentation(viewItemIndex);
    QModelIndex index = modelIndex(viewItemIndex);
    if (!index.isValid() || column != 0
        || cx < (itemIndentation - indent) || cx > itemIndentation)
        return -1; // pos is outside the decoration rect
    return viewItemIndex;
}

void QTreeViewPrivate::select(int start, int stop, QItemSelectionModel::SelectionFlags command)
{
    QModelIndex previous;
    QItemSelectionRange currentRange;
    QStack<QItemSelectionRange> rangeStack;
    QItemSelection selection;
    for (int i = start; i <= stop; ++i) {
        QModelIndex index = d->modelIndex(i);
        QModelIndex parent = index.parent();
        if (previous.isValid() && parent == previous.parent()) {
            // same parent
            QModelIndex tl = model->index(currentRange.top(),
                                          currentRange.left(),
                                          currentRange.parent());
            currentRange = QItemSelectionRange(tl, index);
        } else if (previous.isValid()
                   && parent == model->sibling(previous.row(), 0, previous)) {
            // item is child of prevItem
            rangeStack.push(currentRange);
            currentRange = QItemSelectionRange(index);
        } else {
            if (currentRange.isValid())
                selection.append(currentRange);
            if (rangeStack.isEmpty()) {
                currentRange = QItemSelectionRange(index);
            } else {
                currentRange = rangeStack.pop();
                if (parent == currentRange.parent()) {
                    QModelIndex tl = model->index(currentRange.top(),
                                                  currentRange.left(),
                                                  currentRange.parent());
                    currentRange = QItemSelectionRange(tl, index);
                } else {
                    selection.append(currentRange);
                    currentRange = QItemSelectionRange(index);
                }
            }
        }
        previous = index;
    }
    if (currentRange.isValid())
        selection.append(currentRange);
    for (int i = 0; i < rangeStack.count(); ++i)
        selection.append(rangeStack.at(i));
    q->selectionModel()->select(selection, command);
}
