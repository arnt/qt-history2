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
#define d d_func()
#define q q_func()

/*!
    \class QItemSelectionRange

    \brief The QItemSelectionRange class manages information about ranges of selected items in a model.

    \ingroup model-view

    A QItemSelectionRange contains information about ranges of selected items
    in a model. A range of items is a contiguous array of model items, extending
    to cover a number of adjacent rows and columns with a common parent item;
    this can be visualized as a two-dimensional block of cells in a table.

    Selection ranges perform most of the management functions associated with
    item selections, with the QItemSelection class providing a higher-level
    interface for manipulating selections.

    The model items contained in the selection range can be obtained by using
    the items() function.
    You can determine whether a given model item lies within a particular
    range by using the contains() function. Ranges can also be compared using
    the overloaded operators for equality and inequality, and the intersects()
    function allows you to determine whether two ranges overlap.

    \sa \link model-view-programming.html Model/View Programming\endlink
        QAbstractItemModel QItemSelection QItemSelectionModel

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
    \fn QItemSelectionRange::QItemSelectionRange(const QModelIndex &parent, int top, int left, int bottom, int right)

    Constructs a selection range containing the model items specified by the
    \a parent and the extents of the range: \a top, \a left, \a bottom, and
    \a right.

*/

/*!
    \fn QItemSelectionRange::QItemSelectionRange(const QModelIndex &parent, const QModelIndex &index)

    Constructs a new selection range containing only the model item specified
    by the \a parent and the model item \a index.

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
    \fn QModelIndex QItemSelectionRange::parent() const

    Returns the parent model item index of the items in the selection range.

*/

/*!
    \fn bool QItemSelectionRange::contains(const QModelIndex &index, const QAbstractItemModel *model) const

    Returns true if the model item specified by the \a index lies within the
    range of selected items for the given \a model; otherwise returns false.

*/

/*!
    \fn bool QItemSelectionRange::intersects(const QItemSelectionRange &other) const

    Returns true if this selection range intersects (overlaps with) the \a other
    range given; otherwise returns false.

*/

/*!
    \fn QItemSelectionRange QItemSelectionRange::intersect(const QItemSelectionRange &other) const

    Returns a new selection range containing only the items that are found in
    both the selection range and the \a other selection range.

*/

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
    Returns the list of model index items stored for the given \a model.
*/

QModelIndexList QItemSelectionRange::items(const QAbstractItemModel *model) const
{
    QModelIndex index;
    QModelIndexList indexes;
    if (isValid()) {
        for (int column = l; column <= r; ++column) {
            for (int row = t; row <= b; ++row) {
                index = model->index(row, column, parent());
                if (model->isSelectable(index))
                    indexes.append(index); //###does not specify Type
            }
        }
    }
    return indexes;
}

/*!
  \class QItemSelection

  \brief The QItemSelection class manages information about selected items in a model.

  \ingroup model-view

  A QItemSelection describes the items in a model that have been selected
  by the user. It provides functions for creating and manipulating
  selections, and selecting a range of items from a model.

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

  Use merge() to merge two item selections, split() to 
   that include merging (), splitting (split()) (select())

  \sa \link model-view-programming.html Model/View Programming\endlink
      QAbstractItemModel

*/

/*!
    \fn QItemSelection::QItemSelection()

    Constructs an empty selection.
*/

/*!
    Constructs an item selection for the \a model that extends from the
    top-left model item, specified by the \a topLeft index, to the
    bottom-right item, specified by \a bottomRight.*/

QItemSelection::QItemSelection(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                               const QAbstractItemModel *model)
{
    select(topLeft, bottomRight, model);
}

/*!
    Selects the range in the \a model that extends from the top-left model
    item, specified by the \a topLeft index, to the bottom-right item,
    specified by \a bottomRight.*/

void QItemSelection::select(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                            const QAbstractItemModel *model)
{
    if (model->parent(topLeft) != model->parent(bottomRight) ||
        !topLeft.isValid() || !bottomRight.isValid())
        return;
    append(QItemSelectionRange(
               model->parent(bottomRight),
               topLeft.row(), topLeft.column(),
               bottomRight.row(), bottomRight.column()));
}

/*!
    Returns true if the \a model contains the specified \a index; otherwise
    returns false.
*/

bool QItemSelection::contains(const QModelIndex &index, const QAbstractItemModel *model) const
{
    QList<QItemSelectionRange>::const_iterator it = begin();
    for (; it != end(); ++it)
        if ((*it).contains(index, model))
            return true;
    return false;
}

/*!
    Returns the list of selected model index items for the given \a model.
*/

QModelIndexList QItemSelection::items(QAbstractItemModel *model) const
{
    QModelIndexList items;
    QList<QItemSelectionRange>::const_iterator it = begin();
    for (; it != end(); ++it)
        items += (*it).items(model);
    return items;
}

/*!
  Merges the \a other selection with this QItemSelection using the
  \a command given. This method guarantees that no ranges are overlapping.

  Note that only QItemSelectionModel::Select,
  QItemSelectionModel::Deselect, and QItemSelectionModel::Toggle are
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
        for (int t = 0; t < count(); ++t) {
            if (newSelection.at(n).intersects(at(t)))
                intersections.append(at(t).intersect(newSelection.at(n)));
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
    if (other_top > top)
        result->append(QItemSelectionRange(parent, top, left, other_top - 1,right));
    if (other_bottom < bottom)
        result->append(QItemSelectionRange(parent, other_bottom + 1, left, bottom, right));
    if (other_left > left)
        result->append(QItemSelectionRange(parent, top, left, bottom, other_left - 1));
    if (other_right < right)
        result->append(QItemSelectionRange(parent, top, other_right + 1, bottom, right));
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
            expanded.append(QItemSelectionRange(selection.at(i).parent(),
                                                selection.at(i).top(),
                                                0,
                                                selection.at(i).bottom(),
                                                colCount - 1));
        }
    }
    if (command & QItemSelectionModel::Columns) {
        for (int i = 0; i < selection.count(); ++i) {
            QModelIndex parent = selection.at(i).parent();
            int rowCount = model->rowCount(parent);
            expanded.append(QItemSelectionRange(selection.at(i).parent(),
                                                0,
                                                selection.at(i).left(),
                                                rowCount - 1,
                                                selection.at(i).right()));
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

  \sa \link model-view-programming.html Model/View Programming\endlink
*/

/*!
  Constructs a selection model with the given \a parent that operates on
  the specified item \a model.
*/
QItemSelectionModel::QItemSelectionModel(QAbstractItemModel *model, QObject *parent)
    : QObject(*new QItemSelectionModelPrivate, parent)
{
    d->model = model;
    d->init();
}

/*!
  \internal
*/
QItemSelectionModel::QItemSelectionModel(QItemSelectionModelPrivate &dd, QAbstractItemModel *model,
                                         QObject *parent)
    : QObject(dd, parent)
{
    d->model = model;
    d->init();
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
    QItemSelection selection(index, index, model());
    select(selection, command);
}

/*!
   \fn void QItemSelectionModel::currentChanged(const QModelIndex &old, const QModelIndex &current)

   This signal is emitted whenever the current item changes. The \a old
   model item index is replaced by the \a current index as the selection's
   current item.

   \sa currentItem() setCurrentItem()
*/

/*!
    \fn void QItemSelectionModel::selectionChanged(const QItemSelection &deselected, const QItemSelection &selected)

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
  \value Select         All specified indices will be selected.
  \value Deselect       All specified indices will be deselected.
  \value Toggle         All specified indicies will be selected or
                        deselected depending on their current state.
  \value Current        The current selection will be updated.
  \value Rows           All indices will be expanded to span rows.
  \value Columns        All indices will be expanded to span columns.
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

  \sa QItemSelectionModel::SelectionCommand
*/
void QItemSelectionModel::select(const QItemSelection &selection, SelectionFlags command)
{
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
    emitSelectionChanged(old, newSelection);
    return;
}

/*!
  Clears the selection model. Emits selectionChanged() and currentChanged().
*/
void QItemSelectionModel::clear()
{
    if (d->ranges.count() == 0 && d->currentSelection.count() == 0)
        return;
    QItemSelection selection = d->ranges;
    selection.merge(d->currentSelection, d->currentCommand);
    d->ranges.clear();
    d->currentSelection.clear();
    emit selectionChanged(selection, QItemSelection());
    QModelIndex old = d->currentItem;
    d->currentItem = QPersistentModelIndex();
    emit currentChanged(old, d->currentItem);
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
void QItemSelectionModel::setCurrentItem(const QModelIndex &index, SelectionFlags command)
{
    if (command != NoUpdate)
        select(index, command); // select item
    if (index == d->currentItem)
        return;
    QModelIndex old = d->currentItem;
    d->currentItem = QPersistentModelIndex(index, d->model); // set current
    emit currentChanged(old, index);
}

/*!
  Returns the model item index for the current item, or an invalid index
  if there is no current item.
*/
QModelIndex QItemSelectionModel::currentItem() const
{
    return (QModelIndex)d->currentItem;
}

/*!
  Returns true if the given model item \a index is selected.
*/
bool QItemSelectionModel::isSelected(const QModelIndex &index) const
{
    if (!model()->isSelectable(index))
        return false;
    bool selected = false;
    QList<QItemSelectionRange>::const_iterator it = d->ranges.begin();
    //  search model ranges
    for (; !selected && it != d->ranges.end(); ++it)
        if ((*it).contains(index, model()))
            selected = true;
    // check  currentSelection
    if (d->currentSelection.count()) {
        if (d->currentCommand & Deselect && selected)
            selected = !d->currentSelection.contains(index, model());
        else if (d->currentCommand & Toggle)
            selected ^= d->currentSelection.contains(index, model());
        else if (d->currentCommand & Select && !selected)
            selected = d->currentSelection.contains(index, model());
    }
    return selected;
}

/*!
  Returns true if all items are selected in the \a row with the given
  \a parent.

  Note that this function is usually faster then calling isSelected() on
  all items in the same row.
*/
bool QItemSelectionModel::isRowSelected(int row, const QModelIndex &parent) const
{
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
            if ((*it).contains(index, model())) {
                i = (*it).right();
                break;
            }
        if (it == joined.end())
            return false;
    }
    return true;
}

/*!
  Returns true if all items are selected in the \a column with the given
  \a parent.

  Note that this function is usually faster then calling isSelected() on
  all items in the same column.
*/
bool QItemSelectionModel::isColumnSelected(int column, const QModelIndex &parent) const
{
    // return false if column exist in currentSelection (Deselect)
    if (d->currentCommand & Deselect && d->currentSelection.count()) {
        for (int i=0; i<d->currentSelection.count(); ++i) {
            if (d->currentSelection.at(i).parent() == parent &&
                column >= d->currentSelection.at(i).left() &&
                column <= d->currentSelection.at(i).right())
                return false;
        }
    }
    // return false if ranges in both currentSelection and the selection model
    // intersect and have the same column contained
    if (d->currentCommand & Toggle && d->currentSelection.count()) {
        for (int i=0; i<d->currentSelection.count(); ++i) {
            if (d->currentSelection.at(i).left() <= column &&
                d->currentSelection.at(i).right() >= column) {
                for (int j=0; j<d->ranges.count(); ++j) {
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
             if ((*it).contains(index, model())) {
                 i = (*it).bottom();
                 break;
             }
         }
         if (it == joined.end())
             return false;
     }
     return true;
}

/*!
  Returns the item model operated on by the selection model.
*/
QAbstractItemModel *QItemSelectionModel::model() const
{
    return d->model;
}

/*!
  Returns a list of all selected model item indices. The list contains no
  duplicates, and is not sorted.
*/
QModelIndexList QItemSelectionModel::selectedItems() const
{
    QItemSelection selected = d->ranges;
    selected.merge(d->currentSelection, d->currentCommand);
    return selected.items(model());
}

/*!
  Compares the two selections \a oldSelection and \a newSelection
  and emits selectionChanged() with the deselected and selected items.
*/
void QItemSelectionModel::emitSelectionChanged(const QItemSelection &oldSelection,
                                               const QItemSelection &newSelection )
{
    // if both selections are empty or equal we return
    if ((oldSelection.isEmpty() && newSelection.isEmpty()) ||
        oldSelection == newSelection)
        return;

    // if either selection is empty we do not need to compare
    if (oldSelection.isEmpty() || newSelection.isEmpty()) {
        emit selectionChanged(oldSelection, newSelection);
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

    emit selectionChanged(deselected, selected);
}

void QItemSelectionModelPrivate::init()
{
    // ### This is only necessary until we move the selection model over to use QPersistentModelIndex
    QObject::connect(model, SIGNAL(rowsRemoved(const QModelIndex&,int,int)), q, SLOT(clear()));
    QObject::connect(model, SIGNAL(columnsRemoved(const QModelIndex&,int,int)), q, SLOT(clear()));
}
