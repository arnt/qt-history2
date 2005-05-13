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

#include "qitemselectionmodel.h"
#include <private/qitemselectionmodel_p.h>

/*!
    \class QItemSelectionRange

    \brief The QItemSelectionRange class manages information about a
    range of selected items in a model.

    \ingroup model-view

    A QItemSelectionRange contains information about a range of
    selected items in a model. A range of items is a contiguous array
    of model items, extending to cover a number of adjacent rows and
    columns with a common parent item; this can be visualized as a
    two-dimensional block of cells in a table. A selection range has a
    top(), left() a bottom(), right() and a parent().

    The QItemSelectionRange class is one of the \l{Model/View Classes}
    and is part of Qt's \l{Model/View Programming}{model/view framework}.

    The model items contained in the selection range can be obtained
    by using the items() function. Use
    QItemSelectionModel::selectedIndexes() to get a list of all
    selected items for a view.

    You can determine whether a given model item lies within a
    particular range by using the contains() function. Ranges can also
    be compared using the overloaded operators for equality and
    inequality, and the intersects() function allows you to determine
    whether two ranges overlap.

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemModel QItemSelection QItemSelectionModel

*/

/*!
    \fn QItemSelectionRange::QItemSelectionRange()

    Constructs an empty selection range.
*/

/*!
    \fn QItemSelectionRange::QItemSelectionRange(const QItemSelectionRange &other)

    Copy constructor. Constructs a new selection range with the same contents
    as the \a other range given.

*/

/*!
    \fn QItemSelectionRange::QItemSelectionRange(const QModelIndex &parent, const QModelIndex &index)

    Constructs a new selection range containing only the model item specified
    by the \a parent and the model \a index.

*/

/*!
    \fn QItemSelectionRange::QItemSelectionRange(const QModelIndex &index)

    Constructs a new selection range containing only the model item specified
    by the model \a index.
*/

/*!
    \fn int QItemSelectionRange::top() const

    Returns the row index corresponding to the uppermost selected row in the
    selection range.

*/

/*!
    \fn int QItemSelectionRange::left() const

    Returns the column index corresponding to the leftmost selected column in the
    selection range.

*/

/*!
    \fn int QItemSelectionRange::bottom() const

    Returns the row index corresponding to the lowermost selected row in the
    selection range.

*/

/*!
    \fn int QItemSelectionRange::right() const

    Returns the column index corresponding to the rightmost selected column in
    the selection range.

*/

/*!
    \fn int QItemSelectionRange::width() const

    Returns the number of selected columns in the selection range.

*/

/*!
    \fn int QItemSelectionRange::height() const

    Returns the number of selected rows in the selection range.

*/

/*!
    \fn const QAbstractItemModel *QItemSelectionRange::model() const

    Returns the model that the items in the selection range belong to.
*/

/*!
    \fn QModelIndex QItemSelectionRange::parent() const

    Returns the parent model item index of the items in the selection range.

*/

/*!
    \fn bool QItemSelectionRange::contains(const QModelIndex &index) const

    Returns true if the model item specified by the \a index lies within the
    range of selected items; otherwise returns false.

*/

/*!
    \fn bool QItemSelectionRange::intersects(const QItemSelectionRange &other) const

    Returns true if this selection range intersects (overlaps with) the \a other
    range given; otherwise returns false.

*/
bool QItemSelectionRange::intersects(const QItemSelectionRange &other) const
{
    return (parent() == other.parent()
            && ((top() <= other.top() && bottom() >= other.top())
                || (top() >= other.top() && top() <= other.bottom()))
            && ((left() <= other.left() && right() >= other.left())
                || (left() >= other.left() && left() <= other.right())));
}

/*!
    \fn QItemSelectionRange QItemSelectionRange::intersect(const QItemSelectionRange &other) const

    Returns a new selection range containing only the items that are found in
    both the selection range and the \a other selection range.

*/

QItemSelectionRange QItemSelectionRange::intersect(const QItemSelectionRange &other) const
{
    if (model()) {
        QModelIndex topLeft = model()->index(qMax(top(), other.top()),
                                             qMax(left(), other.left()),
                                             other.parent());
        QModelIndex bottomRight = model()->index(qMin(bottom(), other.bottom()),
                                                 qMin(right(), other.right()),
                                                 other.parent());
        return QItemSelectionRange(topLeft, bottomRight);
    }
    return QItemSelectionRange();
}

/*!
    \fn bool QItemSelectionRange::operator==(const QItemSelectionRange &other) const

    Returns true if the selection range is exactly the same as the \a other
    range given; otherwise returns false.

*/

/*!
    \fn bool QItemSelectionRange::operator!=(const QItemSelectionRange &other) const

    Returns true if the selection range differs from the \a other range given;
    otherwise returns false.

*/

/*!
    \fn bool QItemSelectionRange::isValid() const

    Returns true if the selection range is valid; otherwise returns false.

*/

/*!
    Returns the list of model index items stored in the selection.
*/

QModelIndexList QItemSelectionRange::indexes() const
{
    QModelIndex index;
    QModelIndexList result;
    if (isValid()) {
        for (int column = left(); column <= right(); ++column) {
            for (int row = top(); row <= bottom(); ++row) {
                index = model()->index(row, column, parent());
                if (model()->flags(index) & Qt::ItemIsSelectable)
                    result.append(index);
            }
        }
    }
    return result;
}

/*!
  \class QItemSelection

  \brief The QItemSelection class manages information about selected items in a model.

  \ingroup model-view

  A QItemSelection describes the items in a model that have been
  selected by the user. A QItemSelection is basically a list of
  selection ranges, see QItemSelectionRange. It provides functions for
  creating and manipulating selections, and selecting a range of items
  from a model.

  The QItemSelection class is one of the \l{Model/View Classes}
  and is part of Qt's \l{Model/View Programming}{model/view framework}.

  An item selection can be constructed and initialized to contain a
  range of items from an existing model. The following example constructs
  a selection that contains a range of items from the given \c model,
  beginning at the \c topLeft, and ending at the \c bottomRight.

  \code
    QItemSelection *selection = new QItemSelection(topLeft,
            bottomRight, model);
  \endcode

  An empty item selection can be constructed, and later populated as
  required. So, if the model is going to be unavailable when we construct
  the item selection, we can rewrite the above code in the following way:

  \code
    QItemSelection *selection = new QItemSelection();
    ...
    selection->select(topLeft, bottomRight, model);
  \endcode

  QItemSelection saves memory, and avoids unnecessary work, by working with
  selection ranges rather than recording the model item index for each
  item in the selection. Generally, an instance of this class will contain
  a list of non-overlapping selection ranges.

  Use merge() to merge one item selection into another without making
  overlapping ranges. Use split() to split one selection range into
  smaller ranges based on a another selection range.

  \sa \link model-view-programming.html Model/View Programming\endlink QItemSelectionModel

*/

/*!
    \fn QItemSelection::QItemSelection()

    Constructs an empty selection.
*/

/*!
    Constructs an item selection that extends from the top-left model item,
    specified by the \a topLeft index, to the bottom-right item, specified
    by \a bottomRight.
*/
QItemSelection::QItemSelection(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    select(topLeft, bottomRight);
}

/*!
    Adds the items in the range that extends from the top-left model
    item, specified by the \a topLeft index, to the bottom-right item,
    specified by \a bottomRight to the list.
*/
void QItemSelection::select(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    if (!topLeft.isValid() || !bottomRight.isValid())
        return;

    if ((topLeft.model() != bottomRight.model())
        || topLeft.parent() != bottomRight.parent()) {
        qWarning("Can't select indexes from different model or with different parents");
        return;
    }
    if (topLeft.row() > bottomRight.row() || topLeft.column() > bottomRight.column()) {
        int top = qMin(topLeft.row(), bottomRight.row());
        int bottom = qMax(topLeft.row(), bottomRight.row());
        int left = qMin(topLeft.column(), bottomRight.column());
        int right = qMax(topLeft.column(), bottomRight.column());
        QModelIndex tl = topLeft.sibling(top, left);
        QModelIndex br = bottomRight.sibling(bottom, right);
        append(QItemSelectionRange(tl, br));
        return;
    }
    append(QItemSelectionRange(topLeft, bottomRight));
}

/*!
    Returns true if the selection contains the given \a index; otherwise
    returns false.
*/

bool QItemSelection::contains(const QModelIndex &index) const
{
    QList<QItemSelectionRange>::const_iterator it = begin();
    for (; it != end(); ++it)
        if ((*it).contains(index))
            return true;
    return false;
}

/*!
    Returns a list of model indexes that correspond to the selected items.
*/

QModelIndexList QItemSelection::indexes() const
{
    QModelIndexList result;
    QList<QItemSelectionRange>::const_iterator it = begin();
    for (; it != end(); ++it)
        result += (*it).indexes();
    return result;
}

/*!
  Merges the \a other selection with this QItemSelection using the
  \a command given. This method guarantees that no ranges are overlapping.

  Note that only \c QItemSelectionModel::Select,
  \c QItemSelectionModel::Deselect, and \c QItemSelectionModel::Toggle are
  supported.

  \sa split()
*/
void QItemSelection::merge(const QItemSelection &other, QItemSelectionModel::SelectionFlags command)
{
    if (!(command & QItemSelectionModel::Select ||
          command & QItemSelectionModel::Deselect ||
          command & QItemSelectionModel::Toggle) ||
        other.isEmpty())
        return;

    QItemSelection newSelection = other;
    // Collect intersections
    QItemSelection intersections;
    for (int n = 0; n < newSelection.count(); ++n) {
        if (newSelection.at(n).isValid()) { // nothing intersects an invalid selection range
            for (int t = 0; t < count(); ++t) {
                if (newSelection.at(n).intersects(at(t)))
                    intersections.append(at(t).intersect(newSelection.at(n)));
            }
        }
    }

    //  Split the old (and new) ranges using the intersections
    for (int i = 0; i < intersections.count(); ++i) { // for each intersection
        for (int t = 0; t < count();) { // splitt each old range
            if (at(t).intersects(intersections.at(i))) {
                split(at(t), intersections.at(i), this);
                removeAt(t);
            } else {
                ++t;
            }
        }
        // only split newSelection if Toggle is specified
        for (int n = 0; (command & QItemSelectionModel::Toggle) && n < newSelection.count();) {
            if (newSelection.at(n).intersects(intersections.at(i))) {
                split(newSelection.at(n), intersections.at(i), &newSelection);
                newSelection.removeAt(n);
            } else {
                ++n;
            }
        }
    }
    // do not add newSelection for Deselect
    if (!(command & QItemSelectionModel::Deselect))
        operator+=(newSelection);
}

/*!
  Splits the selection \a range using the selection \a other range, and puts
  the resulting selection in \a result.

  \sa merge()
*/

void QItemSelection::split(const QItemSelectionRange &range,
                           const QItemSelectionRange &other, QItemSelection *result)
{
    QModelIndex parent = other.parent();
    int top = range.top();
    int left = range.left();
    int bottom = range.bottom();
    int right = range.right();
    int other_top = other.top();
    int other_left = other.left();
    int other_bottom = other.bottom();
    int other_right = other.right();
    const QAbstractItemModel *model = range.model();
    if (other_top > top) {
        QModelIndex tl = model->index(top, left, parent);
        QModelIndex br = model->index(other_top - 1, right, parent);
        result->append(QItemSelectionRange(tl, br));
        top = other_top;
    }
    if (other_bottom < bottom) {
        QModelIndex tl = model->index(other_bottom + 1, left, parent);
        QModelIndex br = model->index(bottom, right, parent);
        result->append(QItemSelectionRange(tl, br));
        bottom = other_bottom;
    }
    if (other_left > left) {
        QModelIndex tl = model->index(top, left, parent);
        QModelIndex br = model->index(bottom, other_left - 1, parent);
        result->append(QItemSelectionRange(tl, br));
        left = other_left;
    }
    if (other_right < right) {
        QModelIndex tl = model->index(top, other_right + 1, parent);
        QModelIndex br = model->index(bottom, right, parent);
        result->append(QItemSelectionRange(tl, br));
        right = other_right;
    }
}

/*!
  \internal

  returns a QItemSelection where all ranges have been expanded to:
  Rows: left: 0 and right: columnCount()-1
  Columns: top: 0 and bottom: rowCount()-1
*/

QItemSelection QItemSelectionModelPrivate::expandSelection(const QItemSelection &selection,
                                                           QItemSelectionModel::SelectionFlags command) const
{
    if (selection.isEmpty() && !((command & QItemSelectionModel::Rows) ||
                                 (command & QItemSelectionModel::Columns)))
        return selection;

    QItemSelection expanded;
    if (command & QItemSelectionModel::Rows) {
        for (int i = 0; i < selection.count(); ++i) {
            QModelIndex parent = selection.at(i).parent();
            int colCount = model->columnCount(parent);
            QModelIndex tl = model->index(selection.at(i).top(), 0, parent);
            QModelIndex br = model->index(selection.at(i).bottom(), colCount - 1, parent);
            expanded.append(QItemSelectionRange(tl, br));
        }
    }
    if (command & QItemSelectionModel::Columns) {
        for (int i = 0; i < selection.count(); ++i) {
            QModelIndex parent = selection.at(i).parent();
            int rowCount = model->rowCount(parent);
            QModelIndex tl = model->index(0, selection.at(i).left(), parent);
            QModelIndex br = model->index(rowCount - 1, selection.at(i).right(), parent);
            expanded.append(QItemSelectionRange(tl, br));
        }
    }
    return expanded;
}

/*!
  \class QItemSelectionModel

  \brief The QItemSelectionModel class keeps track of a view's selected items.

  \ingroup model-view

  A QItemSelectionModel keeps track of the selected items in a view, or
  in several views onto the same model. It also keeps track of the
  currently selected item in a view.

  The QItemSelectionModel class is one of the \l{Model/View Classes}
  and is part of Qt's \l{Model/View Programming}{model/view framework}.

  The selected items are stored using ranges. Whenever you want to
  modify the selected items use select() and provide either a
  QItemSelection, or a QModelIndex and a QItemSelectionModel::SelectionFlag.

  The QItemSelectionModel takes a two layer approach to selection
  management, dealing with both selected items that have been committed
  and items that are part of the current selection. The current
  selected items are part of the current interactive selection (for
  example with rubber-band selection or keyboard-shift selections).

  To update the currently selected items, use the bitwise OR of
  QItemSelectionModel::Current and any of the other SelectionFlags.
  If you omit the QItemSelectionModel::Current command, a new current
  selection will be created, and the previous one added to the committed
  selection. All functions operate on both layers; for example,
  selectedItems() will return items from both layers.

  \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemModel
*/

/*!
  Constructs a selection model that operates on the specified item \a model.
*/
QItemSelectionModel::QItemSelectionModel(QAbstractItemModel *model)
    : QObject(*new QItemSelectionModelPrivate, model)
{
    d_func()->model = model;
}

/*!
  \internal
*/
QItemSelectionModel::QItemSelectionModel(QItemSelectionModelPrivate &dd, QAbstractItemModel *model)
    : QObject(dd, model)
{
    d_func()->model = model;
}

/*!
  Destroys the selection model.
*/
QItemSelectionModel::~QItemSelectionModel()
{
}

/*!
  Selects the model item \a index using the specified \a command, and emits
  selectionChanged().

  \sa QItemSelectionModel::SelectionFlags
*/
void QItemSelectionModel::select(const QModelIndex &index, SelectionFlags command)
{
    if (index.isValid()) {
        QItemSelection selection(index, index);
        select(selection, command);
    }
}

/*!
   \fn void QItemSelectionModel::currentChanged(const QModelIndex &current, const QModelIndex &previous)

   This signal is emitted whenever the current item changes. The \a previous
   model item index is replaced by the \a current index as the selection's
   current item.

   \sa currentIndex() setCurrentItem()
*/

/*!
   \fn void QItemSelectionModel::currentColumnChanged(const QModelIndex &current, const QModelIndex &previous)

   This signal is emitted if the \a current item changes and its column is
   different to the column of the \a previous current item.

   \sa currentChanged() currentRowChanged() currentIndex() setCurrentItem()
*/

/*!
   \fn void QItemSelectionModel::currentRowChanged(const QModelIndex &current, const QModelIndex &previous)

   This signal is emitted if the \a current item changes and its row is
   different to the row of the \a previous current item.

   \sa currentChanged() currentColumnChanged() currentIndex() setCurrentItem()
*/

/*!
    \fn void QItemSelectionModel::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)

    This signal is emitted whenever the selection changes. The change in the
    selection is represented as an item selection of \a deselected items and
    an item selection of \a selected items.

    \sa select()
*/

/*!
  \enum QItemSelectionModel::SelectionFlag

  This enum describes the way the selection model will be updated.

  \value NoUpdate       No selection will be made.
  \value Clear          The complete selection will be cleared.
  \value Select         All specified indexes will be selected.
  \value Deselect       All specified indexes will be deselected.
  \value Toggle         All specified indexes will be selected or
                        deselected depending on their current state.
  \value Current        The current selection will be updated.
  \value Rows           All indexes will be expanded to span rows.
  \value Columns        All indexes will be expanded to span columns.
  \value SelectCurrent  A combination of Select and Current, provided for
                        convenience.
  \value ToggleCurrent  A combination of Toggle and Current, provided for
                        convenience.
  \value ClearAndSelect A combination of Clear and Select, provided for
                        convenience.
*/

/*!
  Selects the item \a selection using the specified \a command, and emits
  selectionChanged().

  \sa QItemSelectionModel::SelectionFlag
*/
void QItemSelectionModel::select(const QItemSelection &selection, SelectionFlags command)
{
    Q_D(QItemSelectionModel);
    if (command == NoUpdate)
        return;

    // store old selection
    QItemSelection sel = selection;
    QItemSelection old = d->ranges;
    old.merge(d->currentSelection, d->currentCommand);

    // expand selection according to SelectionBehavior
    if (command & Rows || command & Columns)
        sel = d->expandSelection(sel, command);

    // clear ranges and currentSelection
    if (command & Clear) {
        d->ranges.clear();
        d->currentSelection.clear();
    }

    // merge and clear currentSelection if Current was not set (ie. start new currentSelection)
    if (!(command & Current)) {
        d->ranges.merge(d->currentSelection, d->currentCommand);
        d->currentSelection.clear();
    }

    // update currentSelection
    if (command & Toggle || command & Select || command & Deselect) {
        d->currentCommand = command;
        d->currentSelection = sel;
    }

    // generate new selection, compare with old and emit selectionChanged()
    QItemSelection newSelection = d->ranges;
    newSelection.merge(d->currentSelection, d->currentCommand);
    emitSelectionChanged(newSelection, old);
    return;
}

/*!
  Clears the selection model. Emits selectionChanged() and currentChanged().
*/
void QItemSelectionModel::clear()
{
    Q_D(QItemSelectionModel);
    if (d->ranges.count() == 0 && d->currentSelection.count() == 0)
        return;
    QItemSelection selection = d->ranges;
    selection.merge(d->currentSelection, d->currentCommand);
    d->ranges.clear();
    d->currentSelection.clear();
    emit selectionChanged(QItemSelection(), selection);
    QModelIndex previous = d->currentIndex;
    d->currentIndex = QModelIndex();
    if (previous.isValid()) {
        emit currentChanged(d->currentIndex, previous);
        emit currentRowChanged(d->currentIndex, previous);
        emit currentColumnChanged(d->currentIndex, previous);
    }
}

/*!
  Clears the selection model. Does not emit any signals.
*/
void QItemSelectionModel::reset()
{
    bool block = blockSignals(true);
    clear();
    blockSignals(block);
}


/*!
  Sets the model item \a index to be the current item, and emits
  currentChanged(). The current item is used for keyboard navigation and
  focus indication; it is independent of any selected items, although a
  selected item can also be the current item.

  Depending on the specified \a command, the \a index can also become part
  of the current selection.
  \sa select()
*/
void QItemSelectionModel::setCurrentIndex(const QModelIndex &index, SelectionFlags command)
{
    Q_D(QItemSelectionModel);
    if (index == d->currentIndex) {
        if (command != NoUpdate)
            select(index, command); // select item
        return;
    }
    QModelIndex previous = d->currentIndex;
    d->currentIndex = index; // set current before emitting selection changed below
    if (command != NoUpdate)
        select(index, command); // select item
    emit currentChanged(index, previous);
    if (d->currentIndex.row() != previous.row())
        emit currentRowChanged(d->currentIndex, previous);
    if (d->currentIndex.column() != previous.column())
        emit currentColumnChanged(d->currentIndex, previous);
}

/*!
  Returns the model item index for the current item, or an invalid index
  if there is no current item.
*/
QModelIndex QItemSelectionModel::currentIndex() const
{
    return static_cast<QModelIndex>(d_func()->currentIndex);
}

/*!
  Returns true if the given model item \a index is selected.
*/
bool QItemSelectionModel::isSelected(const QModelIndex &index) const
{
    Q_D(const QItemSelectionModel);
    if (model() != index.model()
        || (model()->flags(index) & Qt::ItemIsSelectable) == 0)
        return false;
    bool selected = false;
    QList<QItemSelectionRange>::const_iterator it = d->ranges.begin();
    //  search model ranges
    for (; !selected && it != d->ranges.end(); ++it)
        if ((*it).contains(index))
            selected = true;
    // check  currentSelection
    if (d->currentSelection.count()) {
        if (d->currentCommand & Deselect && selected)
            selected = !d->currentSelection.contains(index);
        else if (d->currentCommand & Toggle)
            selected ^= d->currentSelection.contains(index);
        else if (d->currentCommand & Select && !selected)
            selected = d->currentSelection.contains(index);
    }
    return selected;
}

/*!
  Returns true if all items are selected in the \a row with the given
  \a parent.

  Note that this function is usually faster than calling isSelected()
  on all items in the same row and that unselectable items are
  ignored.
*/
bool QItemSelectionModel::isRowSelected(int row, const QModelIndex &parent) const
{
    Q_D(const QItemSelectionModel);
    if (parent.isValid() && model() != parent.model())
        return false;

    // return false if row exist in currentSelection (Deselect)
    if (d->currentCommand & Deselect && d->currentSelection.count()) {
        for (int i=0; i<d->currentSelection.count(); ++i) {
            if (d->currentSelection.at(i).parent() == parent &&
                row >= d->currentSelection.at(i).top() &&
                row <= d->currentSelection.at(i).bottom())
                return false;
        }
    }
    // return false if ranges in both currentSelection and ranges
    // intersect and have the same row contained
    if (d->currentCommand & Toggle && d->currentSelection.count()) {
        for (int i=0; i<d->currentSelection.count(); ++i)
            if (d->currentSelection.at(i).top() <= row &&
                d->currentSelection.at(i).bottom() >= row)
                for (int j=0; j<d->ranges.count(); ++j)
                    if (d->ranges.at(j).top() <= row && d->ranges.at(j).bottom() >= row
                        && d->currentSelection.at(i).intersect(d->ranges.at(j)).isValid())
                        return false;
    }
    // add ranges and currentSelection and check through them all
    QModelIndex index;
    QList<QItemSelectionRange>::const_iterator it;
    QList<QItemSelectionRange> joined = d->ranges;
    if (d->currentSelection.count())
        joined += d->currentSelection;
    int colCount = model()->columnCount(parent);
    for (int i = 0; i < colCount; ++i) {
        index = model()->index(row, i, parent);
        for (it = joined.begin(); it != joined.end(); ++it)
            if ((*it).contains(index)) {
                i = (*it).right();
                break;
            }
        if (it == joined.end())
            return false;
    }
    return colCount > 0; // no columns means no selected items
}

/*!
  Returns true if all items are selected in the \a column with the given
  \a parent.

  Note that this function is usually faster than calling isSelected()
  on all items in the same column and that unselectable items are
  ignored.
*/
bool QItemSelectionModel::isColumnSelected(int column, const QModelIndex &parent) const
{
    Q_D(const QItemSelectionModel);
    if (parent.isValid() && model() != parent.model())
        return false;

    // return false if column exist in currentSelection (Deselect)
    if (d->currentCommand & Deselect && d->currentSelection.count()) {
        for (int i = 0; i < d->currentSelection.count(); ++i) {
            if (d->currentSelection.at(i).parent() == parent &&
                column >= d->currentSelection.at(i).left() &&
                column <= d->currentSelection.at(i).right())
                return false;
        }
    }
    // return false if ranges in both currentSelection and the selection model
    // intersect and have the same column contained
    if (d->currentCommand & Toggle && d->currentSelection.count()) {
        for (int i = 0; i < d->currentSelection.count(); ++i) {
            if (d->currentSelection.at(i).left() <= column &&
                d->currentSelection.at(i).right() >= column) {
                for (int j = 0; j < d->ranges.count(); ++j) {
                    if (d->ranges.at(j).left() <= column && d->ranges.at(j).right() >= column
                        && d->currentSelection.at(i).intersect(d->ranges.at(j)).isValid()) {
                        return false;
                    }
                }
            }
        }
    }
    // add ranges and currentSelection and check through them all
    QModelIndex index;
    QList<QItemSelectionRange>::const_iterator it;
    QList<QItemSelectionRange> joined = d->ranges;
    if (d->currentSelection.count())
        joined += d->currentSelection;
    int rowCount = model()->rowCount(parent);
    for (int i = 0; i < rowCount; ++i) {
         index = model()->index(i, column, parent);
         for (it = joined.begin(); it != joined.end(); ++it) {
             if ((*it).contains(index)) {
                 i = (*it).bottom();
                 break;
             }
         }
         if (it == joined.end())
             return false;
    }
    return rowCount > 0; // no rows means no selected items
}

/*!
  Returns true if there are any  items selected in the \a row with the given
  \a parent.
*/
bool QItemSelectionModel::rowIntersectsSelection(int row, const QModelIndex &parent) const
{
    Q_D(const QItemSelectionModel);
    if (parent.isValid() && model() != parent.model())
         return false;
    // check current selection
    for (int i = 0; i < d->currentSelection.count(); ++i)
        if (d->currentSelection.at(i).top() <= row
            && d->currentSelection.at(i).bottom() >= row)
            return true;
    // check the ranges
    for (int i = 0; i < d->ranges.count(); ++i)
        if (d->ranges.at(i).top() <= row
            && d->ranges.at(i).bottom() >= row)
            return true;
    return false;
}

/*!
  Returns true if there are any  items selected in the \a column with the given
  \a parent.
*/
bool QItemSelectionModel::columnIntersectsSelection(int column, const QModelIndex &parent) const
{
    Q_D(const QItemSelectionModel);
    if (parent.isValid() && model() != parent.model())
        return false;
    // check current selection
    for (int i = 0; i < d->currentSelection.count(); ++i)
        if (d->currentSelection.at(i).left() <= column
            && d->currentSelection.at(i).right() >= column)
            return true;
    // check the ranges
    for (int i = 0; i < d->ranges.count(); ++i)
        if (d->ranges.at(i).left() <= column
            && d->ranges.at(i).right() >= column)
            return true;
    return false;
}

/*!
  Returns a list of all selected model item indexes. The list contains no
  duplicates, and is not sorted.
*/
QModelIndexList QItemSelectionModel::selectedIndexes() const
{
    Q_D(const QItemSelectionModel);
    QItemSelection selected = d->ranges;
    selected.merge(d->currentSelection, d->currentCommand);
    return selected.indexes();
}

/*!
  Returns the selection ranges stored in the selection model.
*/
const QItemSelection QItemSelectionModel::selection() const
{
    Q_D(const QItemSelectionModel);
    QItemSelection selected = d->ranges;
    selected.merge(d->currentSelection, d->currentCommand);
    int i = 0;
    // make sure we have no invalid ranges
    // ###  should probably be handled more generic somewhere else
    while (i<selected.count()) {
        if (selected.at(i).isValid())
            ++i;
        else
            (selected.removeAt(i));
    }
    return selected;
}

/*!
  Returns the item model operated on by the selection model.
*/
const QAbstractItemModel *QItemSelectionModel::model() const
{
    return d_func()->model;
}

/*!
  Compares the two selections \a newSelection and \a oldSelection
  and emits selectionChanged() with the deselected and selected items.
*/
void QItemSelectionModel::emitSelectionChanged(const QItemSelection &newSelection,
                                               const QItemSelection &oldSelection)
{
    // if both selections are empty or equal we return
    if ((oldSelection.isEmpty() && newSelection.isEmpty()) ||
        oldSelection == newSelection)
        return;

    // if either selection is empty we do not need to compare
    if (oldSelection.isEmpty() || newSelection.isEmpty()) {
        emit selectionChanged(newSelection, oldSelection);
        return;
    }

    QItemSelection deselected = oldSelection;
    QItemSelection selected = newSelection;

    // remove equal ranges
    bool advance;
    for (int o = 0; o < deselected.count(); ++o) {
        advance = true;
        for (int s = 0; s < selected.count() && o < deselected.count();) {
            if (deselected.at(o) == selected.at(s)) {
                deselected.removeAt(o);
                selected.removeAt(s);
                advance = false;
            } else {
                ++s;
            }
        }
        if (advance)
            ++o;
    }

    // find intersections
    QItemSelection intersections;
    for (int o = 0; o < deselected.count(); ++o) {
        for (int s = 0; s < selected.count(); ++s) {
            if (deselected.at(o).intersects(selected.at(s)))
                intersections.append(deselected.at(o).intersect(selected.at(s)));
        }
    }

    // compare remaining ranges with intersections and split them to find deselected and selected
    for (int i = 0; i < intersections.count(); ++i) {
        // split deselected
        for (int o = 0; o < deselected.count();) {
            if (deselected.at(o).intersects(intersections.at(i))) {
                QItemSelection::split(deselected.at(o), intersections.at(i), &deselected);
                deselected.removeAt(o);
            } else {
                ++o;
            }
        }
        // split selected
        for (int s = 0; s < selected.count();) {
            if (selected.at(s).intersects(intersections.at(i))) {
                QItemSelection::split(selected.at(s), intersections.at(i), &selected);
                selected.removeAt(s);
            } else {
                ++s;
            }
        }
    }

    emit selectionChanged(selected, deselected);
}
