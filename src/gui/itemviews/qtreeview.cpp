/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
    \class QTreeView
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

    Headers in a tree view are constructed using the QHeaderView class
    and can be hidden using header()->hide().

    \section1 Key Bindings

    QTreeView supports a set of key bindings that enable the user to
    navigate in the view and interact with the contents of items:

    \table
    \header \o Key \o Action
    \row \o UpArrow   \o Moves the cursor to the item in the same column on
         the previous row. If the parent of the current item has no more rows to
         navigate to, the cursor moves to the relevant item in the last row
         of the sibling that precedes the parent.
    \row \o DownArrow \o Moves the cursor to the item in the same column on
         the next row. If the parent of the current item has no more rows to
         navigate to, the cursor moves to the relevant item in the first row
         of the sibling that follows the parent.
    \row \o LeftArrow  \o Hides the children of the current item (if present)
         by collapsing a branch.
    \row \o RightArrow \o Reveals the children of the current item (if present)
         by expanding a branch.
    \row \o PageUp   \o Moves the cursor up one page.
    \row \o PageDown \o Moves the cursor down one page.
    \row \o Home \o Moves the cursor to an item in the same column of the first
         row of the first top-level item in the model.
    \row \o End  \o Moves the cursor to an item in the same column of the last
         row of the last top-level item in the model.
    \row \o F2   \o In editable models, this opens the current item for editing.
         The Escape key can be used to cancel the editing process and revert
         any changes to the data displayed.
    \endtable

    \omit
    Describe the expanding/collapsing concept if not covered elsewhere.
    \endomit

    \table 100%
    \row \o \inlineimage windowsxp-treeview.png Screenshot of a Windows XP style tree view
         \o \inlineimage macintosh-treeview.png Screenshot of a Macintosh style tree view
         \o \inlineimage plastique-treeview.png Screenshot of a Plastique style tree view
    \row \o A \l{Windows XP Style Widget Gallery}{Windows XP style} tree view.
         \o A \l{Macintosh Style Widget Gallery}{Macintosh style} tree view.
         \o A \l{Plastique Style Widget Gallery}{Plastique style} tree view.
    \endtable

    \sa QListView, QTreeWidget, {Model/View Programming}, QAbstractItemModel, QAbstractItemView,
        {Dir View Example}
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
    if (d->selectionModel && d->model) { // support row editing
        disconnect(d->selectionModel, SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
                   d->model, SLOT(submit()));
        disconnect(d->model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                   this, SLOT(rowsRemoved(const QModelIndex &, int, int)));
    }
    d->viewItems.clear();
    d->expandedIndexes.clear();
    d->hiddenIndexes.clear();
    d->header->setModel(model);
    QAbstractItemView::setModel(model);
    if (d->model) {
        // QAbstractItemView connects to a private slot
        disconnect(d->model, SIGNAL(rowsRemoved(QModelIndex, int, int)),
                   this, SLOT(rowsRemoved(QModelIndex, int, int)));
        // QTreeView has a public slot for this
        connect(d->model, SIGNAL(rowsRemoved(const QModelIndex &, int, int)),
                this, SLOT(rowsRemoved(const QModelIndex &, int, int)));
    }

    if (d->sortingEnabled)
        sortByColumn(header()->sortIndicatorSection());
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

  \sa QAbstractItemModel::headerData()
*/
QHeaderView *QTreeView::header() const
{
    Q_D(const QTreeView);
    return d->header;
}

/*!
    Sets the header for the tree view, to the given \a header.

    The view takes ownership over the given \a header and deletes it
    when a new header is set.

    \sa QAbstractItemModel::headerData()
*/
void QTreeView::setHeader(QHeaderView *header)
{
    Q_D(QTreeView);
    if (header == d->header || !header)
        return;
    if (d->header && d->header->parent() == this)
        delete d->header;
    d->header = header;
    d->header->setParent(this);

    if (!d->header->model())
        d->header->setModel(model());

    connect(d->header, SIGNAL(sectionResized(int,int,int)),
            this, SLOT(columnResized(int,int,int)));
    connect(d->header, SIGNAL(sectionMoved(int,int,int)),
            this, SLOT(columnMoved()));
    connect(d->header, SIGNAL(sectionCountChanged(int,int)),
            this, SLOT(columnCountChanged(int,int)));
    connect(d->header, SIGNAL(sectionHandleDoubleClicked(int)),
            this, SLOT(resizeColumnToContents(int)));
    connect(d->header, SIGNAL(geometriesChanged()),
            this, SLOT(updateGeometries()));
    d->header->setFocusProxy(this);

    setSortingEnabled(d->sortingEnabled);
}

/*!
  \property QTreeView::indentation
  \brief indentation of the items in the tree view.

  This property holds the indentation measured in pixels of the items for each
  level in the tree view. For top-level items, the indentation specifies the
  horizontal distance from the viewport edge to the items in the first column;
  for child items, it specifies their indentation from their parent items.
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

  This property should only be set to true if it is guaranteed that all items
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

  \sa resizeColumnToContents(), setColumnWidth()
*/
int QTreeView::columnWidth(int column) const
{
    Q_D(const QTreeView);
    return d->header->sectionSize(column);
}

/*!
  Sets the width of the given \a column to the \a width specified.

  \sa columnWidth(), resizeColumnToContents()
*/
void QTreeView::setColumnWidth(int column, int width)
{
    Q_D(QTreeView);
    d->header->resizeSection(column, width);
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

    \sa hideColumn(), isRowHidden()
*/
bool QTreeView::isColumnHidden(int column) const
{
    Q_D(const QTreeView);
    return d->header->isSectionHidden(column);
}

/*!
  If \a hide is true the \a column is hidden, otherwise the \a column is shown.

  \sa hideColumn(), setRowHidden()
*/
void QTreeView::setColumnHidden(int column, bool hide)
{
    Q_D(QTreeView);
    if (column < 0 || column >= d->header->count())
        return;
    d->header->setSectionHidden(column, hide);
}

/*!
    Returns true if the item in the given \a row of the \a parent is hidden;
    otherwise returns false.

    \sa setRowHidden(), isColumnHidden()
*/
bool QTreeView::isRowHidden(int row, const QModelIndex &parent) const
{
    Q_D(const QTreeView);
    if (!model() || d->hiddenIndexes.isEmpty())
        return false;
    QModelIndex index = d->model->index(row, 0, parent);
    for (int i = 0; i < d->hiddenIndexes.count(); ++i)
        if (d->hiddenIndexes.at(i) == index)
            return true;
    return false;
}

/*!
  If \a hide is true the \a row with the given \a parent is hidden, otherwise the \a row is shown.

  \sa isRowHidden(), setColumnHidden()
*/
void QTreeView::setRowHidden(int row, const QModelIndex &parent, bool hide)
{
    Q_D(QTreeView);
    if (!model())
        return;
    QModelIndex index = model()->index(row, 0, parent);
    if (!index.isValid())
        return;

    if (hide) {
        QPersistentModelIndex persistent(index);
        if (!d->hiddenIndexes.contains(persistent)) d->hiddenIndexes.append(persistent);
    } else {
        QPersistentModelIndex persistent(index);
        int i = d->hiddenIndexes.indexOf(persistent);
        if (i >= 0) d->hiddenIndexes.remove(i);
    }

    if (isVisible()) {
        int p = d->viewIndex(parent);
        if (p >= 0) {
            for (uint i = 0; i < d->viewItems.at(p).total; ++i) {
                if (d->viewItems.at(p + i).index == index) {
                    d->viewItems[p].total--;
                    d->viewItems.remove(i);
                    break;
                }
            }
            d->viewport->update();
        }
        else
            d->doDelayedItemsLayout();
    } else {
        d->doDelayedItemsLayout();
    }
}

/*!
  \reimp
*/
void QTreeView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_D(QTreeView);

    // if we are going to do a complete realyout anyway, there is no need to update
    if (!model() || d->delayedLayout.isActive())
        return;

    // refresh the height cache here; we don't really lose anything by getting the size hint,
    // since QAbstractItemView::dataChanged() will get the visualRect for the items anyway

    QModelIndex top = (topLeft.column() == 0) ? topLeft
                      : model()->sibling(topLeft.row(), 0, topLeft);
    int topViewIndex = d->viewIndex(top);
    bool sizeChanged = false;
    if (topViewIndex != -1) {
        if (topLeft == bottomRight) {
            int oldHeight = d->itemHeight(topViewIndex);
            d->invalidateHeightCache(topViewIndex);
            sizeChanged = (oldHeight != d->itemHeight(topViewIndex));
        } else {
            QModelIndex bottom = (bottomRight.column() == 0) ? bottomRight
                                 : model()->sibling(bottomRight.row(), 0, bottomRight);
            int bottomViewIndex = d->viewIndex(bottom);
            for (int i = topViewIndex; i <= bottomViewIndex; ++i) {
                int oldHeight = d->itemHeight(i);
                d->invalidateHeightCache(i);
                sizeChanged |= (oldHeight != d->itemHeight(i));
            }
        }
    }

    if (sizeChanged)
        d->viewport->update();
    QAbstractItemView::dataChanged(topLeft, bottomRight);
}

/*!
  Hides the \a column given.

  \sa showColumn(), setColumnHidden()
*/
void QTreeView::hideColumn(int column)
{
    Q_D(QTreeView);
    d->header->hideSection(column);
}

/*!
  Shows the given \a column in the tree view.

  \sa hideColumn(), setColumnHidden()
*/
void QTreeView::showColumn(int column)
{
    Q_D(QTreeView);
    d->header->showSection(column);
}

/*!
  \fn void QTreeView::expand(const QModelIndex &index)

  Expands the model item specified by the \a index.

  \sa expanded()
*/
void QTreeView::expand(const QModelIndex &index)
{
    Q_D(QTreeView);
    if (!d->indexValid(index))
        return;
    int i = d->viewIndex(index);
    if (i != -1) { // is visible
        d->expand(i, true);
        if (!d->isAnimating()) {
            updateGeometries();
            d->viewport->update();
        }
    } else {
        d->expandedIndexes.append(index);
        emit expanded(index);
    }
}

/*!
  \fn void QTreeView::collapse(const QModelIndex &index)

  Collapses the model item specified by the \a index.

  \sa collapsed()
*/
void QTreeView::collapse(const QModelIndex &index)
{
    Q_D(QTreeView);
    if (!d->indexValid(index))
        return;
    int i = d->viewIndex(index);
    if (i != -1) { // is visible
        d->collapse(i, true);
        if (!d->isAnimating()) {
            updateGeometries();
            viewport()->update();
        }
    } else {
        int i = d->expandedIndexes.indexOf(index);
        if (i != -1) {
            d->expandedIndexes.remove(i);
            emit collapsed(index);
        }
    }
}

/*!
  \fn bool QTreeView::isExpanded(const QModelIndex &index) const

  Returns true if the model item \a index is expanded; otherwise returns
  false.

  \sa expand(), expanded()
*/
bool QTreeView::isExpanded(const QModelIndex &index) const
{
    Q_D(const QTreeView);
    int i = d->viewIndex(index);
    if (i != -1) // is visible
        return d->viewItems.at(i).expanded;
    return d->expandedIndexes.contains(index);
}

/*!
  Sets the item referred to by \a index to either collapse or expanded,
  depending on the value of \a expanded.

  \sa expanded(), expand()
*/
void QTreeView::setExpanded(const QModelIndex &index, bool expanded)
{
    if (expanded)
        this->expand(index);
    else
        this->collapse(index);
}

/*!
    \since Qt 4.2
    \property QTreeView::sortingEnabled
    \brief whether sorting is enabled

    If this property is true sorting is enabled for the table; if the
    property is false, sorting is not enabled. The default value is false.
*/

void QTreeView::setSortingEnabled(bool enable)
{
    Q_D(QTreeView);
    d->sortingEnabled = enable;
    header()->setSortIndicatorShown(enable);
    header()->setClickable(enable);
    if (enable)
        connect(header(), SIGNAL(sectionClicked(int)), this, SLOT(sortByColumn(int)));
    else
        disconnect(header(), SIGNAL(sectionClicked(int)), this, SLOT(sortByColumn(int)));
    sortByColumn(header()->sortIndicatorSection());
}

bool QTreeView::isSortingEnabled() const
{
    Q_D(const QTreeView);
    return d->sortingEnabled;
}

/*!
    \since Qt 4.2
    \property QTreeView::animationsEnabled
    \brief whether animations are enabled

    If this property is true the treeview will animate expandsion
    and collasping of branches. If this property is false, the treeview
    will expand or collapse branches immediately without showing
    the animation.
*/

void QTreeView::setAnimationsEnabled(bool enable)
{
    Q_D(QTreeView);
    d->animationsEnabled = enable;
}

bool QTreeView::isAnimationsEnabled() const
{
    Q_D(const QTreeView);
    return d->animationsEnabled;
}

/*!
  \reimp
 */
void QTreeView::keyboardSearch(const QString &search)
{
    Q_D(QTreeView);
    if (!model() || !model()->rowCount(rootIndex()) || !model()->columnCount(rootIndex()))
        return;

    QModelIndex start;
    if (currentIndex().isValid())
        start = currentIndex();
    else
        start = model()->index(0, 0, rootIndex());

    QTime now(QTime::currentTime());
    bool skipRow = false;
    if (d->keyboardInputTime.msecsTo(now) > QApplication::keyboardInputInterval()) {
        d->keyboardInput = search;
        skipRow = true;
    } else {
        d->keyboardInput += search;
    }
    d->keyboardInputTime = now;

    // special case for searches with same key like 'aaaaa'
    bool sameKey = false;
    if (d->keyboardInput.length() > 1) {
        int c = d->keyboardInput.count(d->keyboardInput.at(d->keyboardInput.length() - 1));
        sameKey = (c == d->keyboardInput.length());
        if (sameKey)
            skipRow = true;
    }

    // skip if we are searching for the same key or a new search started
    if (skipRow) {
        if (indexBelow(start).isValid())
            start = indexBelow(start);
        else
            start = model()->index(0, start.column(), rootIndex());
    }

    int startIndex = d->viewIndex(start);
    if (startIndex <= -1)
        return;

    int previousLevel = -1;
    int bestAbove = -1;
    int bestBelow = -1;
    QString searchString = sameKey ? QString(d->keyboardInput.at(0)) : d->keyboardInput;
    for (int i = 0; i < d->viewItems.count(); ++i) {
        if ((int)d->viewItems.at(i).level > previousLevel) {
            QModelIndex searchFrom = d->viewItems.at(i).index;
            if (searchFrom.parent() == start.parent())
                searchFrom = start;
            QModelIndexList match = model()->match(searchFrom, Qt::DisplayRole, searchString);
            if (match.count()) {
                int hitIndex = d->viewIndex(match.at(0));
                if (hitIndex >= 0 && hitIndex < startIndex)
                    bestAbove = bestAbove == -1 ? hitIndex : qMin(hitIndex, bestAbove);
                else if (hitIndex >= startIndex)
                    bestBelow = bestBelow == -1 ? hitIndex : qMin(hitIndex, bestBelow);
            }
        }
        previousLevel = d->viewItems.at(i).level;
    }

    QModelIndex index;
    if (bestBelow > -1)
        index = d->viewItems.at(bestBelow).index;
    else if (bestAbove > -1)
        index = d->viewItems.at(bestAbove).index;

    if (index.isValid()) {
        QItemSelectionModel::SelectionFlags flags = (d->selectionMode == SingleSelection
                                                     ? QItemSelectionModel::SelectionFlags(
                                                         QItemSelectionModel::ClearAndSelect
                                                         |d->selectionBehaviorFlags())
                                                     : QItemSelectionModel::SelectionFlags(
                                                         QItemSelectionModel::NoUpdate));
        selectionModel()->setCurrentIndex(index, flags);
    }
}

/*!
  Returns the rectangle on the viewport occupied by the item at \a index.
  If the index is not visible or explicitly hidden, the returned rectangle is invalid.
*/
QRect QTreeView::visualRect(const QModelIndex &index) const
{
    Q_D(const QTreeView);

    if (!d->indexValid(index) || isIndexHidden(index))
        return QRect();

    d->executePostedLayout();

    int vi = d->viewIndex(index);
    if (vi < 0)
        return QRect();

    int x = columnViewportPosition(index.column());
    int w = columnWidth(index.column());

    if (index.column() == 0) {
        int i = d->indentationForItem(vi);
        x += i;
        w -= i;
    }
    int y = d->coordinateForItem(vi);
    int h = d->itemHeight(vi);
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

    if (!d->indexValid(index))
        return;

    d->executePostedLayout();

    // Expand all parents if the parent(s) of the node are not expanded.
    QModelIndex parent = index.parent();
    while (parent.isValid() && state() == NoState && d->itemsExpandable) {
        if (!isExpanded(parent))
            expand(parent);
        parent = model()->parent(parent);
    }

    QRect rect = visualRect(index);
    if (rect.isEmpty())
        return;

    // check if we really need to do anything
    QRect area = d->viewport->rect();
    if (hint == EnsureVisible && area.contains(rect)) {
        d->setDirtyRegion(rect);
        return;
    }

    // vertical
    bool above = (hint == EnsureVisible && rect.top() < area.top());
    bool below = (hint == EnsureVisible && rect.bottom() > area.bottom());
    if (hint == PositionAtTop || above) {
        int i = d->viewIndex(index);
        if (verticalScrollMode() == QAbstractItemView::ScrollPerItem)
            verticalScrollBar()->setValue(i);
        else
            verticalScrollBar()->setValue(d->coordinateForItem(i));
    } else if (hint == PositionAtCenter || hint == PositionAtBottom || below) {
        int i = d->viewIndex(index);
        if (i < 0) {
            qWarning("scrollTo: item index was illegal: %d", i);
            return;
        }
        int y = area.height();
        if (hint == PositionAtCenter)
            y = y / 2;
        while (y > 0 && i > 0)
            y -= d->itemHeight(i--);
        int h = d->itemHeight(i);
        int a = (-y) / (h ? h : 1);
        verticalScrollBar()->setValue(++i + a); // ### FIXME
    }

    // horizontal
    int viewportWidth = d->viewport->width();
    int horizontalOffset = d->header->offset();
    int horizontalPosition = d->header->sectionPosition(index.column());
    int cellWidth = d->header->sectionSize(index.column());

    if (hint == PositionAtCenter) {
        horizontalScrollBar()->setValue(horizontalPosition - ((viewportWidth - cellWidth) / 2));
    } else {
        if (horizontalPosition - horizontalOffset < 0 || cellWidth > viewportWidth)
            horizontalScrollBar()->setValue(horizontalPosition);
        else if (horizontalPosition - horizontalOffset + cellWidth > viewportWidth)
            horizontalScrollBar()->setValue(horizontalPosition - viewportWidth + cellWidth);
    }
}

/*!
  \reimp
*/
void QTreeView::timerEvent(QTimerEvent *event)
{
    Q_D(QTreeView);
    if (event->timerId() == d->columnResizeTimerID) {
        updateGeometries();
        killTimer(d->columnResizeTimerID);
        d->columnResizeTimerID = 0;
        QRect rect;
        int viewportHeight = d->viewport->height();
        int viewportWidth = d->viewport->width();
        for (int i = d->columnsToUpdate.size() - 1; i >= 0; --i) {
            int column = d->columnsToUpdate.at(i);
            int x = columnViewportPosition(column);
            if (isRightToLeft())
                rect |= QRect(0, 0, x + columnWidth(column), viewportHeight);
            else
                rect |= QRect(x, 0, viewportWidth - x, viewportHeight);
        }
        d->viewport->update(rect.normalized());
        d->columnsToUpdate.clear();
    }
    QAbstractItemView::timerEvent(event);
}

/*!
  \reimp
*/
void QTreeView::paintEvent(QPaintEvent *event)
{
    Q_D(QTreeView);
    QPainter painter(viewport());
    if (d->isAnimating()) {
        drawTree(&painter, event->region() - d->animationRect());
        d->drawAnimatedOperation(&painter);
    } else {
        drawTree(&painter, event->region());
#ifndef QT_NO_DRAGANDDROP
        d->paintDropIndicator(&painter);
#endif
    }
}

/*!
  \since 4.2
  Draws the part of the tree intersecting the given \a region using the specified
  \a painter.

  \sa paintEvent()
*/
void QTreeView::drawTree(QPainter *painter, const QRegion &region) const
{
    Q_D(const QTreeView);
    const QVector<QTreeViewItem> viewItems = d->viewItems;

    if (viewItems.count() == 0 || d->header->count() == 0) {
        painter->fillRect(region.boundingRect(), palette().brush(QPalette::Base));
        return;
    }

    QStyleOptionViewItem option = viewOptions();
    const QStyle::State state = option.state;
    const int deviceWidth = painter->device()->width();
    const int headerLength = d->header->length();

    int firstVisibleItemOffset = 0;
    const int firstVisibleItem = d->firstVisibleItem(&firstVisibleItemOffset);
    if (firstVisibleItem < 0)
        return;

    QVector<QRect> rects = region.rects();
    for (int a = 0; a < rects.size(); ++a) {

        const QRect area = rects.at(a);
        d->leftAndRight = d->startAndEndColumns(area);

        int i = firstVisibleItem; // the first item at the top of the viewport
        int y = firstVisibleItemOffset; // we may only see part of the first item

        // start at the top of the viewport  and iterate down to the update area
        for (; i < viewItems.count(); ++i) {
            const int itemHeight = d->itemHeight(i);
            if (y + itemHeight >= area.top())
                break;
            y += itemHeight;
        }

        // paint the visible rows
        for (; i < viewItems.count() && y <= area.bottom(); ++i) {
            const int itemHeight = d->itemHeight(i);
            option.rect.setRect(0, y, 0, itemHeight);
            option.state = state | (viewItems.at(i).expanded
                                    ? QStyle::State_Open : QStyle::State_None);
            d->current = i;
            drawRow(painter, option, viewItems.at(i).index);
            y += itemHeight;
        }

        if (y <= area.bottom()) {
            QRect bottomArea(0, y, deviceWidth, area.bottom() - y + 1);
            if (area.intersects(bottomArea))
                painter->fillRect(bottomArea, palette().brush(QPalette::Base));
        }
        if (isRightToLeft()) {
            QRect rightArea(0, 0, deviceWidth - headerLength, area.height());
            if (headerLength < deviceWidth && area.intersects(rightArea))
                painter->fillRect(rightArea, palette().brush(QPalette::Base));
        } else {
            QRect leftArea(headerLength, 0, deviceWidth - headerLength, area.height());
            if (headerLength < deviceWidth && area.intersects(leftArea))
                painter->fillRect(leftArea, palette().brush(QPalette::Base));
        }
    }
}

/*!
  Draws the row in the tree view that contains the model item \a index,
  using the \a painter given. The \a option control how the item is
  displayed.

  \sa QStyleOptionViewItem(), setAlternatingRowColors()
*/
void QTreeView::drawRow(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const
{
    Q_D(const QTreeView);
    QStyleOptionViewItem opt = option;
    const QPoint offset = d->scrollDelayOffset;
    const int y = option.rect.y() + offset.y();
    const QModelIndex parent = index.parent();
    const QHeaderView *header = d->header;
    const QModelIndex current = currentIndex();
    const QModelIndex hover = d->hover;
    const bool focus = (hasFocus() || d->viewport->hasFocus()) && current.isValid();
    const bool reverse = isRightToLeft();
    const QStyle::State state = opt.state;
    const int left = d->leftAndRight.first;
    const int right = d->leftAndRight.second;
    const bool alternate = d->alternatingColors;
    const bool enabled = (state & QStyle::State_Enabled) != 0;

    // ### special case: treeviews with multiple columns draw
    // the selections differently than with only one column
    opt.showDecorationSelected = (d->selectionBehavior & SelectRows)
                                 || option.showDecorationSelected;

    int width, height = option.rect.height();
    int position;
    int headerSection;
    QModelIndex modelIndex;

    for (int headerIndex = left; headerIndex <= right; ++headerIndex) {
        headerSection = d->header->logicalIndex(headerIndex);
        if (header->isSectionHidden(headerSection))
            continue;
        position = columnViewportPosition(headerSection) + offset.x();
        width = header->sectionSize(headerSection);
        modelIndex = d->model->index(index.row(), headerSection, parent);
        opt.state = state;
        if (!modelIndex.isValid()) {
            opt.rect.setRect(position, y, width, height);
            painter->fillRect(opt.rect, palette().brush(QPalette::Base));
            continue;
        }
        if (selectionModel()->isSelected(modelIndex))
            opt.state |= QStyle::State_Selected;
        if (focus && current == modelIndex)
            opt.state |= QStyle::State_HasFocus;
        if (modelIndex == hover)
            opt.state |= QStyle::State_MouseOver;
        else
            opt.state &= ~QStyle::State_MouseOver;
        if (enabled) {
            QPalette::ColorGroup cg;
            if ((model()->flags(index) & Qt::ItemIsEnabled) == 0) {
                opt.state &= ~QStyle::State_Enabled;
                cg = QPalette::Disabled;
            } else {
                cg = QPalette::Normal;
            }
            opt.palette.setCurrentColorGroup(cg);
        }
        QBrush fill;
        if (alternate) {
            if (d->current & 1) {
                opt.state |= QStyle::State_Alternate;
                fill = opt.palette.brush(QPalette::AlternateBase);
            } else {
                opt.state &= ~QStyle::State_Alternate;
                fill = opt.palette.brush(QPalette::Base);
            }
        } else {
            fill = opt.palette.brush(QPalette::Base);
        }
        if (headerSection == 0) {
            int i = d->indentationForItem(d->current);
            opt.rect.setRect(reverse ? position : i + position, y, width - i, height);
            painter->fillRect(opt.rect, fill);
            QRect branches(reverse ? position + width - i : position, y, i, height);
            if ((opt.state & QStyle::State_Selected) && option.showDecorationSelected)
                painter->fillRect(branches, opt.palette.brush(QPalette::Highlight));
            else
                painter->fillRect(branches, fill);
            drawBranches(painter, branches, index);
        } else {
            opt.rect.setRect(position, y, width, height);
            painter->fillRect(opt.rect, fill);
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

    QStyleOption opt;
    opt.initFrom(this);
    QStyle::State extraFlags = QStyle::State_None;
    if (isEnabled())
        extraFlags |= QStyle::State_Enabled;
    if (window()->isActiveWindow())
        extraFlags |= QStyle::State_Active;

    QPoint oldBO = painter->brushOrigin();
    if (verticalScrollMode() == QAbstractItemView::ScrollPerPixel)
        painter->setBrushOrigin(QPoint(0, verticalOffset()));

    if (level >= outer) {
        // start with the innermost branch
        primitive.moveLeft(reverse ? primitive.left() : primitive.left() - indent);
        opt.rect = primitive;

        const bool expanded = d->viewItems.at(item).expanded;
        const bool children = (((expanded && d->viewItems.at(item).total > 0)) // already layed out and has children
                                || d->model->hasChildren(index)); // not layed out yet, so we don't know
        bool moreSiblings = false;
        if (d->hiddenIndexes.isEmpty())
            moreSiblings = (d->model->rowCount(parent) - 1 > index.row());
        else
            moreSiblings = ((d->viewItems.size() > item +1)
                            && (d->viewItems.at(item + 1).index.parent() == parent));

        opt.state = QStyle::State_Item | extraFlags
                    | (moreSiblings ? QStyle::State_Sibling : QStyle::State_None)
                    | (children ? QStyle::State_Children : QStyle::State_None)
                    | (expanded ? QStyle::State_Open : QStyle::State_None);
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
    }
    // then go out level by level
    for (--level; level >= outer; --level) { // we have already drawn the innermost branch
        primitive.moveLeft(reverse ? primitive.left() + indent : primitive.left() - indent);
        opt.rect = primitive;
        opt.state = extraFlags;
        bool moreSiblings = false;
        if (d->hiddenIndexes.isEmpty()) {
            moreSiblings = (d->model->rowCount(ancestor) - 1 > current.row());
        } else {
            int successor = item + d->viewItems.at(item).total + 1;
            while (successor < d->viewItems.size()
                   && d->viewItems.at(successor).level >= uint(level)) {
                if (d->viewItems.at(successor).level == uint(level)) {
                    moreSiblings = true;
                    break;
                }
                successor += d->viewItems.at(successor).total + 1;
            }
        }
        if (moreSiblings)
            opt.state |= QStyle::State_Sibling;
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
        current = ancestor;
        ancestor = current.parent();
    }
    painter->setBrushOrigin(oldBO);
}

/*!
  \reimp
*/
void QTreeView::mousePressEvent(QMouseEvent *event)
{
    Q_D(QTreeView);
    if (state() != NoState || !d->viewport->rect().contains(event->pos()))
        return;
    int i = d->itemDecorationAt(event->pos());
    if (i == -1) {
        QAbstractItemView::mousePressEvent(event);
    } else if (itemsExpandable() && model()->hasChildren(d->viewItems.at(i).index)) {
        if (d->viewItems.at(i).expanded)
            d->collapse(i, true);
        else
            d->expand(i, true);
        if (!d->isAnimating()) {
            updateGeometries();
            viewport()->update();
        }
    }
}

/*!
  \reimp
*/
void QTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    Q_D(QTreeView);
    if (d->itemDecorationAt(event->pos()) == -1) // ### what about expanding/collapsing state ?
        QAbstractItemView::mouseReleaseEvent(event);
}

/*!
  \reimp
*/
void QTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_D(QTreeView);
    if (state() != NoState || !d->viewport->rect().contains(event->pos()))
        return;

    int i = d->itemDecorationAt(event->pos());
    if (i == -1) {
        i = d->itemAtCoordinate(event->y());
        if (i == -1)
            return; // user clicked outside the items

        // signal handlers may change the model
        QModelIndex index = d->viewItems.at(i).index;
        int column = d->header->logicalIndexAt(event->x());
        QPersistentModelIndex persistent = index.sibling(index.row(), column);
        emit doubleClicked(persistent);

        if (!persistent.isValid())
            return;

        if (edit(persistent, DoubleClicked, event) || state() != NoState)
            return; // the double click triggered editing

        if (!style()->styleHint(QStyle::SH_ItemView_ActivateItemOnSingleClick))
            emit activated(persistent);

        d->executePostedLayout(); // we need to make sure viewItems is updated
        if (d->itemsExpandable && model()->hasChildren(persistent)) {
            if (!((i < d->viewItems.count()) && (d->viewItems.at(i).index == persistent))) {
                // find the new index of the item
                for (i = 0; i < d->viewItems.count(); ++i) {
                    if (d->viewItems.at(i).index == persistent)
                        break;
                }
                if (i == d->viewItems.count())
                    return;
            }
            if (d->viewItems.at(i).expanded)
                d->collapse(i, true);
            else
                d->expand(i, true);
            updateGeometries();
            viewport()->update();
        }
    }
}

/*!
  \reimp
*/
void QTreeView::mouseMoveEvent(QMouseEvent *event)
{
    Q_D(QTreeView);
    if (d->itemDecorationAt(event->pos()) == -1) // ### what about expanding/collapsing state ?
        QAbstractItemView::mouseMoveEvent(event);
}

/*!
  \reimp
*/
QModelIndex QTreeView::indexAt(const QPoint &point) const
{
    Q_D(const QTreeView);
    d->executePostedLayout();

    int visualIndex = d->itemAtCoordinate(point.y());
    QModelIndex idx = d->modelIndex(visualIndex);
    int column = d->columnAt(point.x());
    if (idx.isValid() && column >= 0)
        return model()->sibling(idx.row(), column, idx);
    return idx;
}

/*!
  Returns the model index of the item above \a index.
*/
QModelIndex QTreeView::indexAbove(const QModelIndex &index) const
{
    Q_D(const QTreeView);
    if (!d->indexValid(index))
        return QModelIndex();
    d->executePostedLayout();
    int i = d->viewIndex(index);
    if (--i < 0)
        return QModelIndex();
    return d->viewItems.at(i).index;
}

/*!
  Returns the model index of the item below \a index.
*/
QModelIndex QTreeView::indexBelow(const QModelIndex &index) const
{
    Q_D(const QTreeView);
    if (!d->indexValid(index))
        return QModelIndex();
    d->executePostedLayout();
    int i = d->viewIndex(index);
    if (++i >= d->viewItems.count())
        return QModelIndex();
    return d->viewItems.at(i).index;
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
        d->defaultItemHeight = indexRowSizeHint(index);
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
  Returns the horizontal offset of the items in the treeview.

  Note that the tree view uses the horizontal header section
  positions to determine the positions of columns in the view.

  \sa verticalOffset()
*/
int QTreeView::horizontalOffset() const
{
    Q_D(const QTreeView);
    return d->header->offset();
}

/*!
  Returns the vertical offset of the items in the tree view.

  \sa horizontalOffset()
*/
int QTreeView::verticalOffset() const
{
    Q_D(const QTreeView);
    if (verticalScrollMode() == QAbstractItemView::ScrollPerItem) {
        if (uniformRowHeights())
            return verticalScrollBar()->value() * d->defaultItemHeight;
        // If we are scrolling per item and have non-uniform row heights,
        // finding the vertical offset in pixels is going to be relatively slow.
        // ### find a faster way to do this
        int offset = 0;
        for (int i = 0; i < d->viewItems.count(); ++i) {
            if (i == verticalScrollBar()->value())
                return offset;
            offset += d->itemHeight(i);
        }
        return 0;
    }
    // scroll per pixel
    return verticalScrollBar()->value();
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
#ifdef QT_KEYPAD_NAVIGATION
        if (vi == d->viewItems.count()-1 && QApplication::keypadNavigationEnabled())
            return model()->index(0, 0, rootIndex());
#endif
        return d->modelIndex(d->below(vi));
    case MovePrevious:
    case MoveUp:
#ifdef QT_KEYPAD_NAVIGATION
        if (vi == 0 && QApplication::keypadNavigationEnabled())
            return d->modelIndex(d->viewItems.count() - 1);
#endif
        return d->modelIndex(d->above(vi));
    case MoveLeft:
        if (d->viewItems.at(vi).expanded && d->itemsExpandable)
            d->collapse(vi, true);
        updateGeometries();
        viewport()->update();
        break;
    case MoveRight:
        if (!d->viewItems.at(vi).expanded && d->itemsExpandable)
            d->expand(vi, true);
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
        if (topLeft.isValid() && bottomRight.isValid()) {
            selection.append(QItemSelectionRange(topLeft, bottomRight));
            selectionModel()->select(selection, command);
        }
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
        if (!range.isValid())
            continue;
        QModelIndex parent = range.parent();
        QModelIndex leftIndex = range.topLeft();
        int columnCount = model()->columnCount(parent);
        while (leftIndex.isValid() && isIndexHidden(leftIndex)) {
            if (leftIndex.column() + 1 < columnCount)
                leftIndex = model()->index(leftIndex.row(), leftIndex.column() + 1, parent);
            else
                leftIndex = QModelIndex();
        }
        if (!leftIndex.isValid())
            continue;
        int top = visualRect(leftIndex).top();
        QModelIndex rightIndex = range.bottomRight();
        while (rightIndex.isValid() && isIndexHidden(rightIndex)) {
            if (rightIndex.column() - 1 >= 0)
                rightIndex = model()->index(rightIndex.row(), rightIndex.column() - 1, parent);
            else
                rightIndex = QModelIndex();
        }
        if (!rightIndex.isValid())
            continue;
        int bottom = visualRect(rightIndex).bottom();
        if (top > bottom)
            qSwap<int>(top, bottom);
        int height = bottom - top + 1;
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
    dx = isRightToLeft() ? -dx : dx;
    if (dx) {
        if (horizontalScrollMode() == QAbstractItemView::ScrollPerItem) {
            int currentScrollbarValue = horizontalScrollBar()->value();
            int previousScrollbarValue = currentScrollbarValue + dx; // -(-dx)
            d->header->setOffsetToSectionPosition(currentScrollbarValue);
            dx = 0;
            if (previousScrollbarValue < currentScrollbarValue) { // scrolling right
                for (int c = previousScrollbarValue; c < currentScrollbarValue; ++c) {
                    int l = d->header->logicalIndex(c);
                    dx -= d->header->sectionSize(l);
                }
            } else if (previousScrollbarValue > currentScrollbarValue) { // scrolling left
                for (int c = previousScrollbarValue; c >= currentScrollbarValue; --c) {
                    int l = d->header->logicalIndex(c);
                    dx += d->header->sectionSize(l);
                }
            }
        } else {
            d->header->setOffset(horizontalScrollBar()->value());
        }
    }

    if (d->viewItems.isEmpty())
        return;

    // guestimate the number of items in the viewport
    int viewCount = d->viewport->height() / d->defaultItemHeight;
    int maxDeltaY = qMin(d->viewItems.count(), viewCount);

    // no need to do a lot of work if we are going to redraw the whole thing anyway
    if (qAbs(dy) > qAbs(maxDeltaY) && d->editors.isEmpty()) {
        verticalScrollBar()->repaint();
        d->viewport->update();
        return;
    }

    if (dy && verticalScrollMode() == QAbstractItemView::ScrollPerItem) {
        int currentScrollbarValue = verticalScrollBar()->value();
        int previousScrollbarValue = currentScrollbarValue + dy; // -(-dy)
        int currentViewIndex = currentScrollbarValue; // the first visible item
        int previousViewIndex = previousScrollbarValue;
        const QVector<QTreeViewItem> viewItems = d->viewItems;
        dy = 0;
        if (previousViewIndex < currentViewIndex) { // scrolling down
            for (int i = previousViewIndex; i < currentViewIndex; ++i)
                dy -= d->itemHeight(i);
        } else if (previousViewIndex > currentViewIndex) { // scrolling up
            for (int i = previousViewIndex - 1; i >= currentViewIndex; --i)
                dy += d->itemHeight(i);
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
    // do nothing
}

/*!
  Informs the view that the rows from the \a start row to the \a end row
  inclusive have been inserted into the \a parent model item.
*/
void QTreeView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    Q_D(QTreeView);
    d->doDelayedItemsLayout();
    QAbstractItemView::rowsInserted(parent, start, end);
}

/*!
  Informs the view that the rows from the \a start row to the \a end row
  inclusive are about to removed from the given \a parent model item.
*/
void QTreeView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    Q_D(QTreeView);
    if (d->viewItems.isEmpty())
        return;

    setState(CollapsingState);

    if (parent == rootIndex()) {
        d->viewItems.clear();
        d->doDelayedItemsLayout();
        return;
    }

    // collapse the parent
    bool expanded = isExpanded(parent);
    d->expandParent.push(expanded);
    if (expanded) {
        int p = d->viewIndex(parent);
        if (p != -1)
            d->collapse(p, false);
    }

    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

/*!
    \since 4.1

    Informs the view that the rows from the \a start row to the \a end row
    inclusive have been removed from the given \a parent model item.
*/
void QTreeView::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    Q_UNUSED(start);
    Q_UNUSED(end);
    Q_D(QTreeView);

    if (d->viewItems.isEmpty())
        return;

    if (parent == rootIndex()) {
        d->viewItems.clear();
        d->doDelayedItemsLayout();
        return;
    }

    bool expanded = d->expandParent.pop();
    if (expanded) {
        int p = d->viewIndex(parent);
        if (p != -1) { // item is visible
            d->expand(p, false);
            d->viewport->update();
        } else if (!d->expandedIndexes.contains(parent)) {
            d->expandedIndexes.append(parent);
        }
    }

    setState(NoState);
    d->updateScrollBars();
}

/*!
  Informs the tree view that the number of columns in the tree view has
  changed from \a oldCount to \a newCount.
*/
void QTreeView::columnCountChanged(int, int)
{
    if (isVisible())
        updateGeometries();
	viewport()->update();
}

/*!
  Resizes the \a column given to the size of its contents.

  \sa columnWidth(), setColumnWidth()
*/
void QTreeView::resizeColumnToContents(int column)
{
    Q_D(QTreeView);
    d->executePostedLayout();
    if (column < 0 || column >= d->header->count())
        return;
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
    bool ascending = (header()->sortIndicatorSection() != column
                      || header()->sortIndicatorOrder() == Qt::DescendingOrder);
    Qt::SortOrder order = ascending ? Qt::AscendingOrder : Qt::DescendingOrder;
    header()->setSortIndicator(column, order);
    d->model->sort(column, order);
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
  \since 4.2
  Expands all expandable items.

  \sa collapseAll() expand()  collapse() setExpanded()
*/
void QTreeView::expandAll()
{
    Q_D(QTreeView);
    d->executePostedLayout();
    d->expandedIndexes.clear();
    for (int i = 0; i < d->viewItems.count(); ++i)
        if (!d->viewItems.at(i).expanded)
            d->expand(i, false);
    updateGeometries();
    d->viewport->update();
}

/*!
  \since 4.2
  Collpses all expanded items.

  \sa expandAll() expand()  collapse() setExpanded()
*/
void QTreeView::collapseAll()
{
    Q_D(QTreeView);
    d->expandedIndexes.clear();
    doItemsLayout();
}

/*!
    This function is called whenever \a{column}'s size is changed in
    the header. \a oldSize and \a newSize give the previous size and
    the new size in pixels.

    \sa setColumnWidth()
*/
void QTreeView::columnResized(int column, int /* oldSize */, int /* newSize */)
{
    Q_D(QTreeView);
    d->columnsToUpdate.append(column);
    if (d->columnResizeTimerID == 0)
        d->columnResizeTimerID = startTimer(0);
}

/*!
  Updates the items in the tree view.
  \internal
*/
void QTreeView::updateGeometries()
{
    Q_D(QTreeView);
    if (d->header) {
        QSize hint = d->header->isHidden() ? QSize(0, 0) : d->header->sizeHint();
        setViewportMargins(0, hint.height(), 0, 0);
        QRect vg = d->viewport->geometry();
        QRect geometryRect(vg.left(), vg.top() - hint.height(), vg.width(), hint.height());
        d->header->setGeometry(geometryRect);
        d->header->setOffset(horizontalScrollBar()->value());
        if (d->header->isHidden())
            QMetaObject::invokeMethod(d->header, "updateGeometries");
        d->updateScrollBars();
    }
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
    if (d->viewItems.isEmpty())
        return -1;
    int w = 0;
    QStyleOptionViewItem option = viewOptions();
    const QVector<QTreeViewItem> viewItems = d->viewItems;
    for (int i = 0; i < viewItems.count(); ++i) {
        QModelIndex index = viewItems.at(i).index;
        if (index.column() != column)
            index = index.sibling(index.row(), column);
        int width = d->delegateForIndex(index)->sizeHint(option, index).width();
        w = qMax(w, width + (column == 0 ? d->indentationForItem(i) : 0));
    }
    return w;
}

/*!
  Returns the size hint for the row indicated by \a index.

  \sa sizeHintForColumn(), uniformRowHeights()
*/
int QTreeView::indexRowSizeHint(const QModelIndex &index) const
{
    Q_D(const QTreeView);
    if (!d->indexValid(index))
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
    // ### Temporary hack to speed up the function
    option.rect.setWidth(-1);
    for (int column = start; column <= end; ++column) {
        QModelIndex idx = index.sibling(index.row(), column);
        if (idx.isValid()) {
            if (QWidget *editor = d->editorForIndex(idx))
                height = qMax(height, editor->size().height());
            int hint = d->delegateForIndex(idx)->sizeHint(option, idx).height();
            height = qMax(height, hint);
        }
    }

    return height;
}

/*!
  \reimp
*/
void QTreeView::horizontalScrollbarAction(int action)
{
    QAbstractItemView::horizontalScrollbarAction(action);
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
    q->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    QHeaderView *header = new QHeaderView(Qt::Horizontal, q);
    header->setMovable(true);
    header->setStretchLastSection(true);
    q->setHeader(header);

    // animation
    QObject::connect(&timeline, SIGNAL(frameChanged(int)), viewport, SLOT(update()));
    QObject::connect(&timeline, SIGNAL(finished()), q, SLOT(_q_endAnimatedOperation()));
}

void QTreeViewPrivate::expand(int item, bool emitSignal)
{
    Q_Q(QTreeView);

    if (!model || item == -1 || viewItems.at(item).expanded)
        return;

    if (emitSignal && animationsEnabled)
        prepareAnimatedOperation(item, AnimatedOperation::Expand);

    q->setState(QAbstractItemView::ExpandingState);
    QModelIndex index = viewItems.at(item).index;
        expandedIndexes.append(index);
    viewItems[item].expanded = true;
    layout(item);
    if (model->hasChildren(index))
        reexpandChildren(index); // will call expand with emitSignal == false
    q->setState(QAbstractItemView::NoState);

    if (emitSignal) {
        if (animationsEnabled)
            beginAnimatedOperation();
        else
            emit q->expanded(index);
    }
}

void QTreeViewPrivate::collapse(int item, bool emitSignal)
{
    Q_Q(QTreeView);

    if (!model || item == -1 || expandedIndexes.isEmpty())
        return;

    int total = viewItems.at(item).total;
    QModelIndex modelIndex = viewItems.at(item).index;
    int index = expandedIndexes.indexOf(modelIndex);
    if (index == -1 || viewItems.at(item).expanded == false)
        return; // nothing to do

    if (emitSignal && animationsEnabled)
        prepareAnimatedOperation(item, AnimatedOperation::Collapse);

    q->setState(QAbstractItemView::CollapsingState);
    expandedIndexes.remove(index);
    viewItems[item].expanded = false;
    index = item;
    QModelIndex parent = modelIndex;
    while (parent.isValid() && parent != root) {
        Q_ASSERT(index > -1);
        viewItems[index].total -= total;
        parent = parent.parent();
        index = viewIndex(parent);
    }
    viewItems.remove(item + 1, total); // collapse
    q->setState(QAbstractItemView::NoState);

    if (emitSignal) {
        if (animationsEnabled)
            beginAnimatedOperation();
        else
            emit q->collapsed(modelIndex);
    }
}

void QTreeViewPrivate::prepareAnimatedOperation(int item, AnimatedOperation::Type type)
{
    animatedOperation.item = item;
    animatedOperation.type = type;

    int top = coordinateForItem(item) + itemHeight(item);
    QRect rect = viewport->rect();
    if (type == AnimatedOperation::Collapse) {
        int h = 0;
        int c = item + viewItems.at(item).total + 1;
        for (int i = item + 1; i < c; ++i)
            h += itemHeight(i);
        rect.setHeight(h);
        animatedOperation.duration = h;
    }
    rect.moveTop(top);
    animatedOperation.top = top;
    animatedOperation.before = renderTreeToPixmap(rect);
}

void QTreeViewPrivate::beginAnimatedOperation()
{
    Q_Q(QTreeView);

    QRect rect = viewport->rect();
    if (animatedOperation.type == AnimatedOperation::Expand) {
        int h = 0;
        int c = animatedOperation.item + viewItems.at(animatedOperation.item).total + 1;
        for (int i = animatedOperation.item + 1; i < c; ++i)
            h += itemHeight(i);
        rect.setHeight(h);
        animatedOperation.duration = h;
    }
    rect.moveTop(animatedOperation.top);

    animatedOperation.after = renderTreeToPixmap(rect);

    timeline.stop();
    timeline.setDuration(1000);
    timeline.setFrameRange(animatedOperation.top, animatedOperation.top + animatedOperation.duration);
    timeline.start();

    q->setState(QAbstractItemView::AnimatingState);
}

void QTreeViewPrivate::_q_endAnimatedOperation()
{
    Q_Q(QTreeView);
    animatedOperation.before = QPixmap();
    animatedOperation.after = QPixmap();
    q->setState(QAbstractItemView::NoState);
    if (animatedOperation.type == AnimatedOperation::Expand)
        emit q->expanded(viewItems.at(animatedOperation.item).index);
    else // operation == AnimatedOperation::Collapse
        emit q->collapse(viewItems.at(animatedOperation.item).index);
    q->updateGeometries();
    viewport->update();
}

void QTreeViewPrivate::drawAnimatedOperation(QPainter *painter) const
{
    int start = timeline.startFrame();
    int end = timeline.endFrame();
    bool collapsing = animatedOperation.type == AnimatedOperation::Collapse;
    int current = collapsing ? end - timeline.currentFrame() + start : timeline.currentFrame();
    const QPixmap top = collapsing ? animatedOperation.before : animatedOperation.after;
    painter->drawPixmap(0, start, top, 0, end - current - 1, top.width(), top.height());
    const QPixmap bottom = collapsing ? animatedOperation.after : animatedOperation.before;
    painter->drawPixmap(0, current, bottom);
}

QPixmap QTreeViewPrivate::renderTreeToPixmap(const QRect &rect) const
{
    Q_Q(const QTreeView);
    QPixmap pixmap(rect.size());
    QPainter painter(&pixmap);
    painter.translate(0, -rect.top());
    q->drawTree(&painter, QRegion(rect));
    painter.end();
    return pixmap;
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

    int firstColumn = 0;
    while (q->isColumnHidden(firstColumn) && firstColumn < q->header()->count())
        ++firstColumn;

    for (int j = first; j < first + count; ++j) {
        current = model->index(j - first, firstColumn, parent);
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
    int index = itemAtCoordinate(coordinateForItem(i) - viewport->height());
    return index == -1 ? 0 : index;
}

int QTreeViewPrivate::pageDown(int i) const
{
    int index = itemAtCoordinate(coordinateForItem(i) + viewport->height());
    return index == -1 ? viewItems.count() - 1 : index;
}

int QTreeViewPrivate::indentationForItem(int item) const
{
    if (item < 0 || item >= viewItems.count())
        return 0;
    int level = viewItems.at(item).level;
    if (rootDecoration)
        ++level;
    return level * indent;
}

int QTreeViewPrivate::itemHeight(int item) const
{
    if (uniformRowHeights)
        return defaultItemHeight;
    if (viewItems.isEmpty())
        return 0;
    const QModelIndex index = viewItems.at(item).index;
    int height = viewItems.at(item).height;
    if (height <= 0 && index.isValid()) {
        height = q_func()->indexRowSizeHint(index);
        viewItems[item].height = height;
    }
    if (!index.isValid() || height < 0)
        return 0;
    return height;
}


/*!
  \internal
  Returns the viewport y coordinate for \a item.
*/
int QTreeViewPrivate::coordinateForItem(int item) const
{
    Q_Q(const QTreeView);
    if (verticalScrollMode == QAbstractItemView::ScrollPerPixel) {
        if (uniformRowHeights)
            return (item * defaultItemHeight) - q->verticalScrollBar()->value();
        // ### optimize (spans or caching)
        int y = 0;
        for (int i = 0; i < viewItems.count(); ++i) {
            if (i == item)
                return y - q->verticalScrollBar()->value();
            y += itemHeight(i);
        }
    } else { // ScrollPerItem
        int topViewItemIndex = q->verticalScrollBar()->value();
//         if (topViewItemIndex == -1) {
//             const_cast<QTreeViewPrivate*>(this)->updateScrollBars();
//             topViewItemIndex = q->verticalScrollBar()->value();
//             Q_ASSERT(topViewItemIndex != -1); // the scrollbars were not updated correctly
//         }
        if (uniformRowHeights)
            return defaultItemHeight * (item - topViewItemIndex);
        if (item >= topViewItemIndex) {
            // search in the visible area first and continue down
            int viewItemCoordinate = 0;
            int viewItemIndex = topViewItemIndex;
            while (viewItemIndex < viewItems.count()) {
                if (viewItemIndex == item)
                    return viewItemCoordinate;
                viewItemCoordinate += itemHeight(viewItemIndex);
                ++viewItemIndex;
            }
            // below the last item in the view
            Q_ASSERT(false);
            return viewItemCoordinate;
        } else {
            // search the area above the viewport
            int viewItemCoordinate = 0;
            for (int viewItemIndex = topViewItemIndex; viewItemIndex >= 0; --viewItemIndex) {
                if (viewItemIndex == item)
                    return viewItemCoordinate;
                viewItemCoordinate -= itemHeight(viewItemIndex);
            }
            // above the first item in the view
            Q_ASSERT(false);
            return viewItemCoordinate;
        }
    }
    return 0;
}

/*!
  \internal
  Returns the index of the view item at the
  given viewport \a coordinate.

  \sa modelIndex()
*/
int QTreeViewPrivate::itemAtCoordinate(int coordinate) const
{
    Q_Q(const QTreeView);
    if (verticalScrollMode == QAbstractItemView::ScrollPerPixel) {
        if (uniformRowHeights) {
            Q_ASSERT(defaultItemHeight != 0);
            return (coordinate + q->verticalScrollBar()->value()) / defaultItemHeight;
        }
        // ### optimize
        int viewItemCoordinate = 0;
        const int contentsCoordinate = coordinate + q->verticalScrollBar()->value();
        for (int viewItemIndex = 0; viewItemIndex < viewItems.count(); ++viewItemIndex) {
            viewItemCoordinate += itemHeight(viewItemIndex);
            if (viewItemCoordinate >= contentsCoordinate)
                return viewItemIndex;
        }
    } else { // ScrollPerItem
        int topViewItemIndex = q->verticalScrollBar()->value();
        if (uniformRowHeights) {
            Q_ASSERT(defaultItemHeight != 0);
            return topViewItemIndex + (coordinate / defaultItemHeight);
        }
        if (coordinate >= 0) {
            // the coordinate is in or below the viewport
            int viewItemCoordinate = 0;
            for (int viewItemIndex = topViewItemIndex; viewItemIndex < viewItems.count(); ++viewItemIndex) {
                viewItemCoordinate += itemHeight(viewItemIndex);
                if (viewItemCoordinate > coordinate)
                    return viewItemIndex;
            }
        } else {
            // the coordinate is above the viewport
            int viewItemCoordinate = 0;
            for (int viewItemIndex = topViewItemIndex; viewItemIndex >= 0; --viewItemIndex) {
                if (viewItemCoordinate <= coordinate)
                    return viewItemIndex;
                viewItemCoordinate -= itemHeight(viewItemIndex);
            }
        }
    }
    return -1;
}

int QTreeViewPrivate::viewIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return -1;

    int totalCount = viewItems.count();
    QModelIndex parent = index.parent();

    // A quick check near the last item to see if we are just incrementing
    int start = lastViewedItem > 2 ? lastViewedItem - 2 : 0;
    int end = lastViewedItem < totalCount - 2 ? lastViewedItem + 2 : totalCount;
    for (int i = start; i < end; ++i) {
        const QModelIndex idx = viewItems.at(i).index;
        if (idx.row() == index.row()) {
            if (idx.internalId() == index.internalId() || idx.parent() == parent) {// ignore column
                lastViewedItem = i;
                return i;
            }
        }
    }

    // NOTE: this function is slow if the item is outside the visible area
    // search in visible items first and below
    int t = firstVisibleItem();
    t = t > 100 ? t - 100 : 0; // start 100 items above the visible area

    for (int i = t; i < totalCount; ++i) {
        const QModelIndex idx = viewItems.at(i).index;
        if (idx.row() == index.row()) {
            if (idx.internalId() == index.internalId() || idx.parent() == parent) {// ignore column
                lastViewedItem = i;
                return i;
            }
        }
    }
    // search from top to first visible
    for (int j = 0; j < t; ++j) {
        const QModelIndex idx = viewItems.at(j).index;
        if (idx.row() == index.row()) {
            if (idx.internalId() == index.internalId() || idx.parent() == parent) { // ignore column
                lastViewedItem = j;
                return j;
            }
        }
    }
    // nothing found
    return -1;
}

QModelIndex QTreeViewPrivate::modelIndex(int i) const
{
    return ((i < 0 || i >= viewItems.count())
            ? (QModelIndex)root : viewItems.at(i).index);
}

int QTreeViewPrivate::firstVisibleItem(int *offset) const
{
    Q_Q(const QTreeView);
    const int value = q->verticalScrollBar()->value();
    if (verticalScrollMode == QAbstractItemView::ScrollPerItem) {
        if (offset)
            *offset = 0;
        return (value < 0 || value >= viewItems.count()) ? -1 : value;
    }
    // ScrollMode == ScrollPerPixel
    if (uniformRowHeights) {
        if (offset)
            *offset = -(value % defaultItemHeight);
        return value / defaultItemHeight;
    }
    int y = 0; // ### optimize (use spans ?)
    for (int i = 0; i < viewItems.count(); ++i) {
        y += itemHeight(i); // the height value is cached
        if (y > value) {
            if (offset)
                *offset = y - value - itemHeight(i);
            return i;
        }
    }
    return -1;
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
            collapse(parentViewIndex, false); // remove the current layout
            expand(parentViewIndex, false); // do the relayout
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
    if (!model)
        return;
    // ### optimize
    QVector<QPersistentModelIndex> o = expandedIndexes;
    for (int j = 0; j < o.count(); ++j) {
        QModelIndex index = o.at(j);
        if (!index.isValid()){
            int k = expandedIndexes.indexOf(index);
            if (k >= 0)
                expandedIndexes.remove(k);
        } else if (model->parent(index) == parent) {
            int v = viewIndex(index);
            if (v < 0)
                continue;
            int k = expandedIndexes.indexOf(index);
            expandedIndexes.remove(k);
            expand(v, false);
        }
    }
}

void QTreeViewPrivate::updateScrollBars()
{
    Q_Q(QTreeView);

    QSize viewportSize = viewport->size();

    // ### asserts if removed
    if (!viewport->isVisible()) {
        q->verticalScrollBar()->setRange(0, 0);
        q->verticalScrollBar()->setPageStep(0);
        return;
    }

    if (verticalScrollMode == QAbstractItemView::ScrollPerItem) {
        int itemsInViewport = 0;
        if (uniformRowHeights) {
            itemsInViewport = viewportSize.height() / defaultItemHeight;
        } else {
            int y = viewportSize.height();
            int i = viewItems.count();
            while (y > 0 && i > 0)
                y -= itemHeight(--i);
            itemsInViewport = (i > 0 ? (viewItems.count() - i - 1) : viewItems.count());
        }
        q->verticalScrollBar()->setRange(0, viewItems.count() - itemsInViewport);
        q->verticalScrollBar()->setPageStep(itemsInViewport);
    } else { // scroll per pixel
        int contentsHeight = 0;
        if (uniformRowHeights) {
            contentsHeight = defaultItemHeight * viewItems.count();
        } else { // ### optimize (spans or caching)
            for (int i = 0; i < viewItems.count(); ++i)
                contentsHeight += itemHeight(i);
        }
        q->verticalScrollBar()->setRange(0, contentsHeight - viewportSize.height());
        q->verticalScrollBar()->setPageStep(viewportSize.height());
    }

    if (horizontalScrollMode == QAbstractItemView::ScrollPerItem) {
        const int columnCount = header->count();
        int x = viewportSize.width();
        int c = columnCount;
        while (x > 0 && c > 0) {
            int l = header->logicalIndex(--c);
            x -= header->sectionSize(l);
        }
        int columnsInViewport = (c > 0 ? (columnCount - c - 1) : columnCount);
        q->horizontalScrollBar()->setRange(0, columnCount - columnsInViewport);
        q->horizontalScrollBar()->setPageStep(columnsInViewport);
    } else { // scroll per pixel
        const int horizontalLength = header->length();
        const QSize maxSize = q->maximumViewportSize();
        if (maxSize.width() >= horizontalLength && q->verticalScrollBar()->maximum() <= 0)
            viewportSize = maxSize;
        q->horizontalScrollBar()->setPageStep(viewportSize.width());
        q->horizontalScrollBar()->setRange(0, horizontalLength - viewportSize.width());
    }
}

int QTreeViewPrivate::itemDecorationAt(const QPoint &pos) const
{
    Q_Q(const QTreeView);
    int x = pos.x();
    int column = header->logicalIndexAt(x);
    if (column == -1)
        return -1; // no logical index at x
    int position = header->sectionViewportPosition(column);
    int size = header->sectionSize(column);
    int cx = (q->isRightToLeft() ? size - x + position : x - position);
    int viewItemIndex = itemAtCoordinate(pos.y());
    int itemIndentation = indentationForItem(viewItemIndex);
    QModelIndex index = modelIndex(viewItemIndex);

    if (!index.isValid() || column != 0
        || cx < (itemIndentation - indent) || cx > itemIndentation)
        return -1; // pos is outside the decoration rect

    if (!rootDecoration && index.parent() == q->rootIndex())
        return -1; // no decoration at root

    QRect rect;
    if (q->isRightToLeft())
        rect = QRect(position + size - itemIndentation, coordinateForItem(viewItemIndex),
                     indent, itemHeight(viewItemIndex));
    else
        rect = QRect(position + itemIndentation - indent, coordinateForItem(viewItemIndex),
                     indent, itemHeight(viewItemIndex));
    QStyleOption opt;
    opt.initFrom(q);
    opt.rect = rect;
    QRect returning = q->style()->subElementRect(QStyle::SE_TreeViewDisclosureItem, &opt, q);
    if (!returning.contains(pos))
        return -1;

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

QPair<int,int> QTreeViewPrivate::startAndEndColumns(const QRect &rect) const
{
    Q_Q(const QTreeView);
    int start = header->visualIndexAt(rect.left());
    int end = header->visualIndexAt(rect.right());
    if (q->isRightToLeft()) {
        start = (start == -1 ? header->count() - 1 : start);
        end = (end == -1 ? 0 : end);
    } else {
        start = (start == -1 ? 0 : start);
        end = (end == -1 ? header->count() - 1 : end);
    }
    return qMakePair<int,int>(qMin(start, end), qMax(start, end));
}

#include "moc_qtreeview.cpp"

#endif // QT_NO_TREEVIEW
