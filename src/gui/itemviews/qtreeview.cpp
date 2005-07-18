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

#ifndef QT_NO_TREEVIEW
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

/*!
    \class QTreeView qtreeview.h
    \brief The QTreeView class provides a default model/view implementation of a tree view.

    \ingroup model-view
    \mainclass

    A QTreeView implements a tree representation of items from a
    model. This class is used to provide standard hierarchical lists that
    were previously provided by the \c QListView class, but using the more
    flexible approach provided by Qt's model/view architecture.

    The QTreeView class is one of the \l{Model/View Classes} and is part of
    Qt's \l{Model/View Programming}{model/view framework}.

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
    Describe the expanding/collapsing concept if not covered elsewhere.
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
    Q_D(QTreeView);
    d->initialize();
}

/*!
  \internal
*/
QTreeView::QTreeView(QTreeViewPrivate &dd, QWidget *parent)
    : QAbstractItemView(dd, parent)
{
    Q_D(QTreeView);
    d->initialize();
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
    Q_D(QTreeView);
    if (d->selectionModel && d->model) // support row editing
        disconnect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                   d->model, SLOT(submit()));

    d->viewItems.clear();
    d->expandedIndexes.clear();
    d->hiddenIndexes.clear();
    d->header->setModel(model);
    QAbstractItemView::setModel(model);
}

/*!
  \reimp
*/
void QTreeView::setRootIndex(const QModelIndex &index)
{
    Q_D(QTreeView);
    d->header->setRootIndex(index);
    QAbstractItemView::setRootIndex(index);
}

/*!
  \reimp
*/
void QTreeView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    Q_D(QTreeView);
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
    Q_D(const QTreeView);
    return d->header;
}

/*!
  Sets the \a header for the tree view.
*/
void QTreeView::setHeader(QHeaderView *header)
{
    Q_ASSERT(header);
    Q_D(QTreeView);
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
*/
int QTreeView::indentation() const
{
    Q_D(const QTreeView);
    return d->indent;
}

void QTreeView::setIndentation(int i)
{
    Q_D(QTreeView);
    d->indent = i;
}

/*!
  \property QTreeView::rootIsDecorated
  \brief whether to show controls for expanding and collapsing items

  This property holds whether root items are displayed with controls for
  expanding and collapsing them.
*/
bool QTreeView::rootIsDecorated() const
{
    Q_D(const QTreeView);
    return d->rootDecoration;
}

void QTreeView::setRootIsDecorated(bool show)
{
    Q_D(QTreeView);
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
    Q_D(const QTreeView);
    return d->uniformRowHeights;
}

void QTreeView::setUniformRowHeights(bool uniform)
{
    Q_D(QTreeView);
    d->uniformRowHeights = uniform;
}

/*!
  \property QTreeView::itemsExpandable
  \brief whether the items are expandable by the user.

  This property holds whether the user can expand and collapse items
  interactively.

*/
bool QTreeView::itemsExpandable() const
{
    Q_D(const QTreeView);
    return d->itemsExpandable;
}

void QTreeView::setItemsExpandable(bool enable)
{
    Q_D(QTreeView);
    d->itemsExpandable = enable;
}

/*!
  Returns the horizontal position of the \a column in the viewport.
*/
int QTreeView::columnViewportPosition(int column) const
{
    Q_D(const QTreeView);
    return d->header->sectionViewportPosition(column);
}

/*!
  Returns the width of the \a column.

  \sa resizeColumnToContents()
*/
int QTreeView::columnWidth(int column) const
{
    Q_D(const QTreeView);
    return d->header->sectionSize(column);
}

/*!
  Returns the column in the tree view whose header covers the \a x
  coordinate given.
*/
int QTreeView::columnAt(int x) const
{
    Q_D(const QTreeView);
    return d->header->logicalIndexAt(x);
}

/*!
    Returns true if the \a column is hidden; otherwise returns false.

    \sa hideColumn()
*/
bool QTreeView::isColumnHidden(int column) const
{
    Q_D(const QTreeView);
    return d->header->isSectionHidden(column);
}

/*!
  If \a hide is true the \a column is hidden, otherwise the \a column is shown.
*/
void QTreeView::setColumnHidden(int column, bool hide)
{
    Q_D(QTreeView);
    d->header->setSectionHidden(column, hide);
}

/*!
    Returns true if the item in the given \a row of the \a parent is hidden;
    otherwise returns false.

    \sa setRowHidden()
*/
bool QTreeView::isRowHidden(int row, const QModelIndex &parent) const
{
    Q_D(const QTreeView);
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
    Q_D(QTreeView);
    QModelIndex index = model()->index(row, 0, parent);
    QPersistentModelIndex persistent(index);
    if (hide) {
        if (!d->hiddenIndexes.contains(persistent)) d->hiddenIndexes.append(persistent);
    } else {
        int i = d->hiddenIndexes.indexOf(persistent);
        if (i >= 0) d->hiddenIndexes.remove(i);
    }

    if (isVisible())
        d->relayout(parent);
    else
        d->doDelayedItemsLayout();

}

/*!
  \reimp
*/
void QTreeView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_D(QTreeView);
    if (!model())
        return;
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
    Q_D(QTreeView);
    d->header->hideSection(column);
}

/*!
  Shows the given \a column in the tree view.

  \sa hideColumn()
*/
void QTreeView::showColumn(int column)
{
    Q_D(QTreeView);
    d->header->showSection(column);
}

/*!
  \fn void QTreeView::expand(const QModelIndex &index)

  Expands the model item specified by the \a index.
*/
void QTreeView::expand(const QModelIndex &index)
{
    Q_D(QTreeView);
    if (!index.isValid())
        return;
    int idx = d->viewIndex(index);
    if (idx > -1) { // is visible
        d->expand(idx);
        updateGeometries();
        viewport()->update();
    } else {
        d->expandedIndexes.append(index);
    }
}

/*!
  \fn void QTreeView::collapse(const QModelIndex &index)

  Collapses the model item specified by the \a index.
*/
void QTreeView::collapse(const QModelIndex &index)
{
    Q_D(QTreeView);
    if (!index.isValid())
        return;
    int i = d->viewIndex(index);
    if (i > -1) { // is visible
        d->collapse(i);
        updateGeometries();
        viewport()->update();
    } else {
        i = d->expandedIndexes.indexOf(index);
        if (i > -1)
            d->expandedIndexes.remove(i);
    }
}

/*!
  \fn bool QTreeView::isExpanded(const QModelIndex &index) const

  Returns true if the model item \a index is expanded; otherwise returns
  false.
*/
bool QTreeView::isExpanded(const QModelIndex &index) const
{
    Q_D(const QTreeView);
    int i = d->viewIndex(index);
    if (i > -1) // is visible - FIXME: this is a workaround for a bug!
        return d->viewItems.at(i).expanded;
    return d->expandedIndexes.contains(index);
}

/*!
  Sets the item referred to by \a index to either collapse or expanded,
  depending on the value of \a expanded.

  \sa expanded()
*/
void QTreeView::setExpanded(const QModelIndex &index, bool expanded)
{
    if (expanded)
        this->expand(index);
    else
        this->collapse(index);
}

/*!
  Returns the rectangle on the viewport occupied by the item at \a index.
  If the index is not visible or explicitly hidden, the returned rectangle is invalid.
*/
QRect QTreeView::visualRect(const QModelIndex &index) const
{
    Q_D(const QTreeView);

    if (!index.isValid() || !isVisible() || isIndexHidden(index))
        return QRect();

    d->executePostedLayout();

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
    Scroll the contents of the tree view until the given model item
    \a index is visible. The \a hint parameter specifies more
    precisely where the item should be located after the
    operation.
    If any of the parents of the model item are collapsed, they will 
    be expanded to ensure that the model item is visible.
*/
void QTreeView::scrollTo(const QModelIndex &index, ScrollHint hint)
{
    Q_D(QTreeView);
    
    // Expand all parents if the parent(s) of the node are not expanded.
    QModelIndex parent = index.parent();
    while (parent.isValid()) {
        if (!isExpanded(parent)) expand(parent);
        parent = parent.parent();
    }    
    
    // check if we really need to do anything
    QRect rect = visualRect(index);
    if (rect.isEmpty())
        return;
    QRect area = d->viewport->rect();
    if (hint == EnsureVisible && area.contains(rect)) {
        d->setDirtyRegion(rect);
        return;
    }

    // vertical
    int verticalSteps = verticalStepsPerItem();
    bool above = (hint == EnsureVisible && rect.top() < area.top());
    bool below = (hint == EnsureVisible && rect.bottom() > area.bottom());
    if (hint == PositionAtTop || above) {
        int i = d->viewIndex(index);
        verticalScrollBar()->setValue(i * verticalSteps);
    } else if (hint == PositionAtBottom || below) {
        int i = d->viewIndex(index);
        if (i < 0) {
            qWarning("scrollTo: item index was illegal: %d", i);
            return;
        }
        int y = area.height();
        while (y > 0 && i > 0)
            y -= d->height(i--);
        int h = d->height(i);
        int a = (-y * verticalSteps) / (h ? h : 1);
        verticalScrollBar()->setValue(++i * verticalSteps + a);
    }

    // horizontal
    bool leftOf = isRightToLeft()
                  ? rect.right() > area.right()
                  : rect.left() < area.left();
    bool rightOf = isRightToLeft()
                   ? rect.left() < area.left()
                   : rect.right() > area.right();
    int horizontalSteps = horizontalStepsPerItem();
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
}

/*!
  \reimp
*/
void QTreeView::paintEvent(QPaintEvent *e)
{
    Q_D(QTreeView);
    QStyleOptionViewItem option = viewOptions();
    const QStyle::State state = option.state;
    const bool alternate = d->alternatingColors;
    const QBrush baseBrush = palette().brush(QPalette::Base);
    const QBrush alternateBrush = palette().brush(QPalette::AlternateBase);
    const int v = verticalScrollBar()->value();
    const int c = d->viewItems.count();
    const QVector<QTreeViewItem> viewItems = d->viewItems;
    const QPoint offset = d->scrollDelayOffset;

    QPainter painter(d->viewport);
    if (c == 0 || d->header->count() == 0) {
        painter.fillRect(e->rect(), baseBrush);
        return;
    }

    QVector<QRect> rects = e->region().rects();
    for (int ii = 0; ii < rects.size(); ++ii) {

        QRect area = rects.at(ii);
        area.translate(offset);

        const int t = area.top();
        const int b = area.bottom() + 1;

        d->left = d->header->visualIndexAt(area.left());
        d->right = d->header->visualIndexAt(d->header->length());
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
        
        int i = d->itemAt(v); // first item
        if (i < 0) // couldn't find the first item
            return;
        int y = d->topItemDelta(v, d->height(i));
        int w = d->viewport->width();

        while (y < b && i < c) {
            int h = d->height(i); // actual height
            if (y + h >= t) { // we are in the update area
                option.rect.setRect(0, y, 0, h);
                option.state = state | (viewItems.at(i).expanded
                                        ? QStyle::State_Open : QStyle::State_None);
                if (alternate)
                    option.palette.setBrush(QPalette::Base, i & 1 ? baseBrush : alternateBrush);
                d->current = i;
                drawRow(&painter, option, viewItems.at(i).index);
            }
            y += h;
            ++i;
        }

        int x = d->header->length();
        QRect bottom(0, y, w, b - y);
        if (y < b && area.intersects(bottom))
            painter.fillRect(bottom, baseBrush);
        if (isRightToLeft()) {
            QRect right(0, 0, w - x, b);
            if (x > 0 && area.intersects(right))
                painter.fillRect(right, baseBrush);
        } else {
            QRect left(x, 0, w - x, b);
            if (x < w && area.intersects(left))
                painter.fillRect(left, baseBrush);
        }
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
    Q_D(const QTreeView);
    const QPoint offset = d->scrollDelayOffset;
    QStyleOptionViewItem opt = option;
    const int y = option.rect.y() + offset.y();
    const QModelIndex parent = index.parent();
    const QHeaderView *header = d->header;
    const QModelIndex current = currentIndex();
    const bool focus = (hasFocus() || d->viewport->hasFocus()) && current.isValid();
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
        position = columnViewportPosition(headerSection) + offset.x();
        width = header->sectionSize(headerSection);
        modelIndex = d->model->index(index.row(), headerSection, parent);
        if (!modelIndex.isValid())
            continue;
        opt.state = state;
        if (selectionModel()->isSelected(modelIndex))
            opt.state |= QStyle::State_Selected;
        if (focus && current == modelIndex)
            opt.state |= QStyle::State_HasFocus;
        QPalette::ColorGroup cg;
        if ((model()->flags(index) & Qt::ItemIsEnabled) == 0) {
            opt.state &= ~QStyle::State_Enabled;
            cg = QPalette::Disabled;
        } else {
            cg = QPalette::Normal;
        }
        if (headerSection == 0) {
            int i = d->indentation(d->current);
            opt.rect.setRect(reverse ? position : i + position, y, width - i, height);
            painter->fillRect(opt.rect, option.palette.brush(QPalette::Base));
            QRect branches(reverse ? position + width - i : position, y, i, height);
            if ((opt.state & QStyle::State_Selected) && option.showDecorationSelected)
                painter->fillRect(branches, option.palette.brush(cg, QPalette::Highlight));
            else
                painter->fillRect(branches, option.palette.brush(QPalette::Base));
            drawBranches(painter, branches, index);
        } else {
            opt.rect.setRect(position, y, width, height);
            painter->fillRect(opt.rect, option.palette.brush(QPalette::Base));
        }
        itemDelegate()->paint(painter, opt, modelIndex);
    }
}

/*!
  Draws the branches in the tree view on the same row as the model item
  \a index, using the \a painter given. The branches are drawn in the
  rectangle specified by \a rect.
*/
void QTreeView::drawBranches(QPainter *painter, const QRect &rect,
                             const QModelIndex &index) const
{
    Q_D(const QTreeView);
    const bool reverse = isRightToLeft();
    const int indent = d->indent;
    const int outer = d->rootDecoration ? 0 : 1;
    const int item = d->current;
    int level = d->viewItems.at(item).level;
    QRect primitive(reverse ? rect.left() : rect.right(), rect.top(), indent, rect.height());

    QModelIndex parent = index.parent();
    QModelIndex current = parent;
    QModelIndex ancestor = current.parent();

    QStyleOption opt(0);
    opt.palette = palette();
    QStyle::State extraFlags = QStyle::State_None;
    if (isEnabled())
        extraFlags |= QStyle::State_Enabled;
    if (window()->isActiveWindow())
        extraFlags |= QStyle::State_Active;
    if (level >= outer) {
        // start with the innermost branch
        primitive.moveLeft(reverse ? primitive.left() : primitive.left() - indent);
        opt.rect = primitive;

        const bool expanded = d->viewItems.at(item).expanded;
        const bool children = (expanded // already layed out
                               ? d->viewItems.at(item).total // this also covers the hidden items
                               : d->model->hasChildren(index)); // not layed out yet, so we don't know
        opt.state = QStyle::State_Item | extraFlags
                    | (d->model->rowCount(parent) - 1 > index.row()
                      ? QStyle::State_Sibling : QStyle::State_None)
                    | (children ? QStyle::State_Children : QStyle::State_None)
                    | (expanded ? QStyle::State_Open : QStyle::State_None);
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
    }
    // then go out level by level
    for (--level; level >= outer; --level) { // we have already drawn the innermost branch
        primitive.moveLeft(reverse ? primitive.left() + indent : primitive.left() - indent);
        opt.rect = primitive;
        opt.state = extraFlags;
        if (d->model->rowCount(ancestor) - 1 > current.row())
            opt.state |= QStyle::State_Sibling;
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
    Q_D(QTreeView);
    int i = d->itemDecorationAt(e->pos());
    if (i == -1) {
        QAbstractItemView::mousePressEvent(e);
    } else if (model()->hasChildren(d->viewItems.at(i).index)) {
        if (d->viewItems.at(i).expanded) {
            setState(ExpandingState);
            d->collapse(i);
        } else {
            setState(CollapsingState);
            d->expand(i);
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
    Q_D(QTreeView);
    int i = d->itemDecorationAt(e->pos());
    if (i == -1) {
        QAbstractItemView::mouseDoubleClickEvent(e);
        i = d->item(e->y());
        if (i == -1 || state() != NoState || !d->itemsExpandable)
            return; // the double click triggered editing or we clicked outside the items
        if (model()->hasChildren(d->viewItems.at(i).index)) {
            if (d->viewItems.at(i).expanded) {
                setState(ExpandingState);
                d->collapse(i);
            } else {
                setState(CollapsingState);
                d->expand(i);
            }
            updateGeometries();
            viewport()->update();
        }
    }
}

/*!
  \reimp
*/
QModelIndex QTreeView::indexAt(const QPoint &p) const
{
    Q_D(const QTreeView);
    d->executePostedLayout();
    int vi = d->item(p.y());
    QModelIndex mi = d->modelIndex(vi);
    int c = d->columnAt(p.x());
    if (mi.isValid() && c >= 0)
        return model()->sibling(mi.row(), c, mi);
    return QModelIndex();
}

/*!
  Returns the model index of the item above \a index.
*/
QModelIndex QTreeView::indexAbove(const QModelIndex &index) const
{
    Q_D(const QTreeView);
    d->executePostedLayout();
    int above = d->above(d->viewIndex(index));
    return d->modelIndex(above);
}

/*!
  Returns the model index of the item below \a index.
*/
QModelIndex QTreeView::indexBelow(const QModelIndex &index) const
{
    Q_D(const QTreeView);
    d->executePostedLayout();
    int below = d->below(d->viewIndex(index));
    return d->modelIndex(below);
}

/*!
  Lays out the items in the tree view.
*/
void QTreeView::doItemsLayout()
{
    Q_D(QTreeView);
    d->viewItems.clear(); // prepare for new layout
    QStyleOptionViewItem option = viewOptions();
    QModelIndex parent = rootIndex();
    if (model() && model()->hasChildren(parent)) {
        QModelIndex index = model()->index(0, 0, parent);
        d->itemHeight = indexRowSizeHint(index);
        d->layout(-1);
        d->reexpandChildren(parent);
    }
    QAbstractItemView::doItemsLayout();
}

/*!
  \reimp
*/
void QTreeView::reset()
{
    Q_D(QTreeView);
    d->expandedIndexes.clear();
    d->hiddenIndexes.clear();
    d->viewItems.clear();
    QAbstractItemView::reset();
}

/*!
  Returns the horizontal offset.
*/
int QTreeView::horizontalOffset() const
{
    Q_D(const QTreeView);
    return d->header->offset();
}

/*!
  Returns the vertical offset of the items in the tree view.
*/
int QTreeView::verticalOffset() const
{
    Q_D(const QTreeView);
    // gives an estimate
    int item = verticalScrollBar()->value() / verticalStepsPerItem();
    if (model() && model()->rowCount(rootIndex()) > 0 && model()->columnCount(rootIndex()) > 0)
        return item * d->itemHeight;
    return item * 30;
}

/*!
    Move the cursor in the way described by \a cursorAction, using the
    information provided by the button \a modifiers.
*/
QModelIndex QTreeView::moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers)
{
    Q_D(QTreeView);
    Q_UNUSED(modifiers);

    QModelIndex current = currentIndex();
    if (!current.isValid()) {
        int i = 0;
        while (i < d->viewItems.count() && d->hiddenIndexes.contains(d->viewItems.at(i).index))
            ++i;
        return d->viewItems.value(i).index;
    }
    int vi = qMax(0, d->viewIndex(current));
    switch (cursorAction) {
    case MoveNext:
    case MoveDown:
        return d->modelIndex(d->below(vi));
    case MovePrevious:
    case MoveUp:
        return d->modelIndex(d->above(vi));
    case MoveLeft:
        if (d->viewItems.at(vi).expanded && d->itemsExpandable)
            d->collapse(vi);
        updateGeometries();
        viewport()->update();
        break;
    case MoveRight:
        if (!d->viewItems.at(vi).expanded && d->itemsExpandable)
            d->expand(vi);
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
    Q_D(QTreeView);
    if (!selectionModel())
        return;
    QPoint tl(isRightToLeft() ? rect.right() : rect.left(), rect.top());
    QPoint br(isRightToLeft() ? rect.left() : rect.right(), rect.bottom());
    QModelIndex topLeft = indexAt(tl);
    QModelIndex bottomRight = indexAt(br);
    if (selectionBehavior() != SelectRows) {
        QItemSelection selection;
        selection.append(QItemSelectionRange(topLeft, bottomRight));
        selectionModel()->select(selection, command);
    } else {
        d->select(d->viewIndex(topLeft), d->viewIndex(bottomRight), command);
    }
}

/*!
  Returns the rectangle from the viewport of the items in the given
  \a selection.
*/
QRegion QTreeView::visualRegionForSelection(const QItemSelection &selection) const
{
    if (selection.isEmpty())
        return QRegion();

    QRegion selectionRegion;
    for (int i = 0; i < selection.count(); ++i) {
        QItemSelectionRange range = selection.at(i);
           
        QModelIndex leftIndex = range.topLeft();
        while(isIndexHidden(leftIndex))
            leftIndex = leftIndex.sibling(leftIndex.row(), leftIndex.column()+1);
        int top = visualRect(leftIndex).top();
         
        QModelIndex rightIndex = range.topLeft();
        while(isIndexHidden(rightIndex))
            rightIndex = rightIndex.sibling(rightIndex.row(), rightIndex.column()-1);
        int bottom = visualRect(rightIndex).bottom();
        if (top > bottom)
            qSwap<int>(top, bottom);
        int height = bottom - top+1;
        for (int c = range.left(); c <= range.right(); ++c)
            selectionRegion += QRegion(QRect(columnViewportPosition(c), top,
                                             columnWidth(c), height));
    }
    return selectionRegion;
}

/*!
  \reimp
*/
QModelIndexList QTreeView::selectedIndexes() const
{
    QModelIndexList viewSelected;
    QModelIndexList modelSelected;
    if (selectionModel())
        modelSelected = selectionModel()->selectedIndexes();
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
    Q_D(QTreeView);

    // guestimate the number of items in the viewport
    int viewCount = d->viewport->height() / d->itemHeight;
    int maxDeltaY = verticalStepsPerItem() * qMin(d->viewItems.count(), viewCount);

    // no need to do a lot of work if we are going to redraw the whole thing anyway
    if (qAbs(dy) > qAbs(maxDeltaY)) {
        verticalScrollBar()->repaint();
        d->viewport->update();
        return;
    }

    if (dx) {
        int steps = horizontalStepsPerItem();
        int scrollbarValue = horizontalScrollBar()->value();
        int column = d->header->logicalIndex(scrollbarValue / steps);
        int left = (scrollbarValue % steps) * d->header->sectionSize(column);
        int offset = (left / steps) + d->header->sectionPosition(column);
        if (isRightToLeft())
            dx = offset - d->header->offset();
        else
            dx = d->header->offset() - offset;
        d->header->setOffset(offset);
    }

    if (dy) {
        int steps = verticalStepsPerItem();
        int currentScrollbarValue = verticalScrollBar()->value();
        int previousScrollbarValue = currentScrollbarValue + dy; // -(-dy)
        int currentViewIndex = currentScrollbarValue / steps; // the first visible item
        int previousViewIndex = previousScrollbarValue / steps;

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

    d->scrollContentsBy(dx, dy);
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
void QTreeView::reexpand()
{
    Q_D(QTreeView);
    if (d->reexpand == -1)
        return;
    d->expand(d->reexpand);
    d->reexpand = -1;
    viewport()->update();
}

/*!
  Informs the view that the rows from the \a start row to the \a end row
  inclusive have been inserted into the \a parent model item.
*/
void QTreeView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(QTreeView);

    if (isVisible()) {
        d->relayout(parent);
        if (parent.isValid() && parent.column() == 0) {
            QRect rect = visualRect(parent);
            rect.setLeft(columnViewportPosition(parent.column()));
            d->viewport->update(rect);
        }
    } else {
        d->doDelayedItemsLayout();
    }

    QAbstractItemView::rowsInserted(parent, start, end);
}

/*!
  Informs the view that the rows from the \a start row to the \a end row
  inclusive have been removed from the given \a parent model item.
*/
void QTreeView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(QTreeView);
    if (d->viewItems.isEmpty())
        return;

    // collapse all children
    for (int i = start; i <= end; ++i)
        collapse(model()->index(i, 0, parent));

    // collapse parent
    int p = d->viewIndex(parent);
    if (p > 0) {
        d->collapse(p);
        // reexpamd parent using a delayed function call
        d->reexpand = p; // p is safe because all the changes happens after this one
        int slot = metaObject()->indexOfSlot("reexpand()");
        QApplication::postEvent(this, new QMetaCallEvent(slot));
    } else {
        d->reexpand = -1;
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
    Q_D(QTreeView);
    d->executePostedLayout();
    int contents = sizeHintForColumn(column);
    int header = d->header->isHidden() ? 0 : d->header->sectionSizeHint(column);
    d->header->resizeSection(column, qMax(contents, header));
}

/*!
  Sorts the model by the values in the given \a column.
 */
void QTreeView::sortByColumn(int column)
{
    Q_D(QTreeView);
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
    Q_D(QTreeView);
    if (!selectionModel())
        return;
    d->select(0, d->viewItems.count() - 1,
              QItemSelectionModel::ClearAndSelect
              |QItemSelectionModel::Rows);
}

/*!
    This function is called whenever \a{column}'s size is changed in
    the header. \a oldSize and \a newSize give the previous size and
    the new size in pixels.
*/
void QTreeView::columnResized(int column, int /* oldSize */, int /* newSize */)
{
    Q_D(QTreeView);
    int x = columnViewportPosition(column);
    QRect rect;
    if (isRightToLeft())
        rect.setRect(0, 0, x + d->header->sectionSize(column), d->viewport->height());
    else
        rect.setRect(x, 0, d->viewport->width() - x, d->viewport->height());
    d->viewport->update(rect.normalized());
    updateGeometries();
}

/*!
  Updates the items in the tree view.
  \internal
*/
void QTreeView::updateGeometries()
{
    Q_D(QTreeView);
    QSize hint = d->header->isHidden() ? QSize(0, 0) : d->header->sizeHint();
    setViewportMargins(0, hint.height(), 0, 0);

    QRect vg = d->viewport->geometry();
    QRect geometryRect(vg.left(), vg.top() - hint.height(), vg.width(), hint.height());
    d->header->setGeometry(geometryRect);

    // make sure that the header sections are resized, even if the header is hidden
    if (d->header->isHidden()
        && (d->header->stretchSectionCount() || d->header->stretchLastSection())) {
        d->header->viewport()->setGeometry(geometryRect);
        QMetaObject::invokeMethod(d->header, "resizeSections");
    }

    // update scrollbars
    if (model() && model()->rowCount(rootIndex()) > 0)
        d->updateVerticalScrollbar();
    else
        verticalScrollBar()->setRange(0, 0);
    if (model() && model()->columnCount(rootIndex()) > 0)
        d->updateHorizontalScrollbar();
    else
        horizontalScrollBar()->setRange(0, 0);

    QAbstractItemView::updateGeometries();
}

/*!
  Returns the size hint for the \a column's width or -1 if there is no
  model.

  If you need to set the width of a given column to a fixed value, call
  QHeaderView::resizeSection() on the view's header.

  If you reimplement this function in a subclass, note that the value you
  return is only used when resizeColumnToContents() is called. In that case,
  if a larger column width is required by either the view's header or
  the item delegate, that width will be used instead.

  \sa QWidget::sizeHint, header()
*/
int QTreeView::sizeHintForColumn(int column) const
{
    Q_D(const QTreeView);
    if (d->viewItems.count() <= 0)
        return -1;

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
        Q_ASSERT(i != -1);
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
  Returns the size hint for the row indicated by \a index.

  \sa sizeHintForColumn()
*/
int QTreeView::indexRowSizeHint(const QModelIndex &index) const
{
    Q_D(const QTreeView);
    if (!index.isValid() || !model())
        return -1;

    int start = -1;
    int end = -1;
    int count = d->header->count();
    if (count) {
        // If the sections have moved, we end up checking too many or too few
        start = d->header->logicalIndexAt(0);
        end = d->header->logicalIndexAt(viewport()->width());
    } else {
        // If the header has not been layed out yet, we use the model directly
        count = model()->columnCount(index.parent());
    }

    if (isRightToLeft()) {
        start = (start == -1 ? count - 1 : start);
        end = (end == -1 ? 0 : end);
    } else {
        start = (start == -1 ? 0 : start);
        end = (end == -1 ? count - 1 : end);
    }

    int tmp = start;
    start = qMin(start, end);
    end = qMax(tmp, end);

    int height = -1;
    QStyleOptionViewItem option = viewOptions();
    QAbstractItemDelegate *delegate = itemDelegate();
    for (int column = start; column <= end; ++column) {
        QModelIndex idx = index.sibling(index.row(), column);
        if (idx.isValid() ) 
            height = qMax(height, delegate->sizeHint(option, idx).height());
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
void QTreeViewPrivate::initialize()
{
    Q_Q(QTreeView);
    q->setSelectionBehavior(QAbstractItemView::SelectRows);
    q->setSelectionMode(QAbstractItemView::SingleSelection);

    QHeaderView *header = new QHeaderView(Qt::Horizontal, q);
    header->setMovable(true);
    header->setStretchLastSection(true);
    q->setHeader(header);
}

void QTreeViewPrivate::expand(int i)
{
    Q_Q(QTreeView);

    if (!model || i < 0)
        return;

    QModelIndex index = viewItems.at(i).index;

    if (viewItems.at(i).expanded)
        return;

    expandedIndexes.append(index);

    viewItems[i].expanded = true;
    layout(i);

    // make sure we expand children that are already expanded
    if (model->hasChildren(index))
        reexpandChildren(index);

    emit q->expanded(index);
}

void QTreeViewPrivate::collapse(int i)
{
    Q_Q(QTreeView);
    if (!model || i < 0 || expandedIndexes.isEmpty())
        return;
    int total = viewItems.at(i).total;
    QModelIndex index = viewItems.at(i).index;
    int idx = expandedIndexes.indexOf(index);
    if (idx >= 0)
        expandedIndexes.remove(idx);
    viewItems[i].expanded = false;

    idx = i;
    QModelIndex tmp = index;
    while (tmp.isValid() && tmp != root) {
        Q_ASSERT(idx > -1);
        viewItems[idx].total -= total;
        tmp = tmp.parent();
        idx = viewIndex(tmp);
    }
    viewItems.remove(i + 1, total); // collapse

    emit q->collapsed(index);
}

void QTreeViewPrivate::layout(int i)
{
    Q_Q(QTreeView);
    QModelIndex current;
    QModelIndex parent = modelIndex(i);

    int count = 0;
    if (model->hasChildren(parent))
        count = model->rowCount(parent);

    if (i == -1)
        viewItems.resize(count);
    else
        viewItems.insert(i + 1, count, QTreeViewItem()); // expand

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
        viewItems.remove(last + 1, hidden); // collapse

    QModelIndex root = q->rootIndex();
    while (parent != root) {
        Q_ASSERT(i > -1);
        viewItems[i].total += count - hidden;
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
    return idx == -1 ? viewItems.count() : idx;
}

int QTreeViewPrivate::indentation(int i) const
{
    if (i < 0 || i >= viewItems.count())
        return 0;
    int level = viewItems.at(i).level;
    if (rootDecoration)
        ++level;
    return level * indent;
}

int QTreeViewPrivate::coordinate(int item) const
{
    Q_Q(const QTreeView);
    int scrollbarValue = q->verticalScrollBar()->value();
    int viewItemIndex = itemAt(scrollbarValue); // first item (may start above the page)
    Q_ASSERT(viewItemIndex != -1);
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
        int i = itemAt(scrollbarValue);
        Q_ASSERT(i != -1);
        return y + (itemHeight * (item - i));
    }
    // item is above the viewport - estimated y
    return y - (itemHeight * (viewItemIndex - item));
}

int QTreeViewPrivate::item(int coordinate) const
{
    Q_Q(const QTreeView);
    int scrollbarValue = q->verticalScrollBar()->value();
    int viewItemIndex = itemAt(scrollbarValue);
    if (viewItemIndex < 0) // couldn't find first visible item
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
    Q_Q(const QTreeView);
    // NOTE: this function is slow if the item is outside the visible area
    // search in visible items first, then below
    int t = itemAt(q->verticalScrollBar()->value());
    t = t > 100 ? t - 100 : 0; // start 100 items above the visible area
    for (int i = t; i < viewItems.count(); ++i)
        if (viewItems.at(i).index.row() == index.row() &&
            viewItems.at(i).index.internalId() == index.internalId()) // ignore column
            return i;
    // search above
    for (int j = 0; j < t; ++j)
        if (viewItems.at(j).index.row() == index.row() &&
            viewItems.at(j).index.internalId() == index.internalId()) // ignore column
            return j;
    // nothing found
    return -1;
}

QModelIndex QTreeViewPrivate::modelIndex(int i) const
{
    return ((i < 0 || i >= viewItems.count())
            ? (QModelIndex)root : viewItems.at(i).index);
}

int QTreeViewPrivate::itemAt(int value) const
{
    int i = value / verticalStepsPerItem;
    return (i < 0 || i >= viewItems.count()) ? -1 : i;
}

int QTreeViewPrivate::topItemDelta(int value, int iheight) const
{
    int above = (value % verticalStepsPerItem) * iheight; // what's left; in "item units"
    return -(above / verticalStepsPerItem); // above the page
}

int QTreeViewPrivate::columnAt(int x) const
{
    return header->logicalIndexAt(x);
}

void QTreeViewPrivate::relayout(const QModelIndex &parent)
{
    Q_Q(QTreeView);
    // do a local relayout of the items
    if (parent.isValid()) {
        int parentViewIndex = viewIndex(parent);
        if (parentViewIndex > -1 && viewItems.at(parentViewIndex).expanded) {
            collapse(parentViewIndex);
            expand(parentViewIndex);
            q->updateGeometries();
            viewport->update();
        }
    } else {
        viewItems.clear();
        q->doItemsLayout();
    }
}

void QTreeViewPrivate::reexpandChildren(const QModelIndex &parent)
{
    // FIXME: this is slow: optimize
    QVector<QPersistentModelIndex> o = expandedIndexes;
    for (int j = 0; j < o.count(); ++j) {
        QModelIndex index = o.at(j);
        if (index.parent() == parent) {
            int v = viewIndex(index);
            if (v < 0)
                continue;
            int k = expandedIndexes.indexOf(index);
            expandedIndexes.remove(k);
            expand(v);
        }
    }
}

void QTreeViewPrivate::updateVerticalScrollbar()
{
    Q_Q(QTreeView);
    int viewHeight = viewport->height();
    int itemCount = viewItems.count();

    // set page step size
    int verticalScrollBarValue = q->verticalScrollBar()->value();
    int itemsInViewport = 0;
    if (uniformRowHeights) {
        itemsInViewport = viewHeight / itemHeight;
    } else {
        int topItemInViewport = itemAt(verticalScrollBarValue);
        if (topItemInViewport < 0) {
            // if itemAt can't find the top item, there are no visible items in the view
            q->verticalScrollBar()->setRange(0, 0);
            q->verticalScrollBar()->setPageStep(0);
            return;
        }
        int h = height(topItemInViewport);
        int y = topItemDelta(verticalScrollBarValue, h);
        int i = topItemInViewport;
        for (; y < viewHeight && i < itemCount; ++i)
            y += height(i);
        itemsInViewport = i - topItemInViewport;
    }
    q->verticalScrollBar()->setPageStep(itemsInViewport * verticalStepsPerItem);

    // set the scroller range
    int y = viewHeight;
    int i = itemCount; // FIXME: wrong
    while (y > 0 && i > 0)
        y -= height(--i);
    int max = i * verticalStepsPerItem;

    if (y < 0) { // if the first item starts above the viewport, we have to backtrack
        int backtracking = verticalStepsPerItem * -y;
        int itemSize = height(i);
        if (itemSize > 0) // avoid division by zero
            max += (backtracking / itemSize) + 1;
    }

    q->verticalScrollBar()->setRange(0, max);
}

void QTreeViewPrivate::updateHorizontalScrollbar()
{
    Q_Q(QTreeView);
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
    int max = count * horizontalStepsPerItem;

    // set page step size
    int visibleCount = header->count() - count - 1;
    q->horizontalScrollBar()->setPageStep(visibleCount * horizontalStepsPerItem);

    if (x < 0) { // if the first item starts left of the viewport, we have to backtrack
        int sectionSize = header->sectionSize(count);
        if (sectionSize > 0) // avoid division by zero
            max += ((-x * horizontalStepsPerItem) / sectionSize) + 1;
    }

    q->horizontalScrollBar()->setRange(0, max);
}

int QTreeViewPrivate::itemDecorationAt(const QPoint &pos) const
{
    Q_Q(const QTreeView);
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

void QTreeViewPrivate::select(int top, int bottom,
                              QItemSelectionModel::SelectionFlags command)
{
    Q_Q(QTreeView);
    QModelIndex previous;
    QItemSelectionRange currentRange;
    QStack<QItemSelectionRange> rangeStack;
    QItemSelection selection;
    for (int i = top; i <= bottom; ++i) {
        QModelIndex index = modelIndex(i);
        QModelIndex parent = index.parent();
        if (previous.isValid() && parent == previous.parent()) {
            // same parent
            QModelIndex tl = model->index(currentRange.top(), currentRange.left(),
                                          currentRange.parent());
            currentRange = QItemSelectionRange(tl, index);
        } else if (previous.isValid() && parent == model->sibling(previous.row(), 0, previous)) {
            // item is child of previous
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

#endif // QT_NO_TREEVIEW
