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

#include "qitemselectionmodel.h"
#include <private/qitemselectionmodel_p.h>
#define d d_func()
#define q q_func()

QModelIndexList QItemSelectionRange::items(const QAbstractItemModel *model) const
{
    QModelIndexList items;
    if (isValid()) {
        for (int column=l; column<=r; ++column)
            for (int row=t; row<=b; ++row)
                items.append(model->index(row, column, parent())); //###does not specify Type
    }
    return items;
}

/*!
  \class QItemSelection

  \brief QItemSelection stores the top-left and bottom-right QModelIndexs in a selection range

  \ingroup model-view


  \sa \link model-view-programming.html Model/View Programming\endlink.
*/

QItemSelection::QItemSelection(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                               const QAbstractItemModel *model)
{
    select(topLeft, bottomRight, model);
}

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

bool QItemSelection::contains(const QModelIndex &item, const QAbstractItemModel *model) const
{
    QList<QItemSelectionRange>::const_iterator it = begin();
    for (; it != end(); ++it)
        if ((*it).contains(item, model))
            return true;
    return false;
}

QModelIndexList QItemSelection::items(QAbstractItemModel *model) const
{
    QModelIndexList items;
    QList<QItemSelectionRange>::const_iterator it = begin();
    for (; it != end(); ++it)
        items += (*it).items(model);
    return items;
}

/*!
  Merges selection \a other with this QItemSelection using the \a selectionCommand.
  This method guarantees that no ranges are overlapping.

  Note: Only QItemSelectionModel::Select,
  QItemSelectionModel::Deselect and QItemSelectionModel::Toggle are
  supported.
*/
void QItemSelection::merge(const QItemSelection &other, int selectionCommand)
{
    if (!(selectionCommand & QItemSelectionModel::Select ||
          selectionCommand & QItemSelectionModel::Deselect ||
          selectionCommand & QItemSelectionModel::Toggle) ||
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
        for (int n = 0; (selectionCommand & QItemSelectionModel::Toggle) && n < newSelection.count();) {
            if (newSelection.at(n).intersects(intersections.at(i))) {
                split(newSelection.at(n), intersections.at(i), &newSelection);
                newSelection.removeAt(n);
            } else {
                ++n;
            }
        }
    }
    // do not add newSelection for Deselect
    if (!(selectionCommand & QItemSelectionModel::Deselect))
        operator+=(newSelection);
}

/*!
  Splits selection range \a range using the selection range \a other, and puts the resulting selection in \a result.
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
                                                           int selectionCommand) const
{
    if (selection.isEmpty() && !((selectionCommand & QItemSelectionModel::Rows) ||
                                 (selectionCommand & QItemSelectionModel::Columns)))
        return selection;

    QItemSelection expanded;
    if (selectionCommand & QItemSelectionModel::Rows) {
        for (int i=0; i<selection.count(); ++i)
            expanded.append(QItemSelectionRange(selection.at(i).parent(),
                                                selection.at(i).top(),
                                                0,
                                                selection.at(i).bottom(),
                                                model->columnCount(selection.at(i).parent())-1));
    }
    if (selectionCommand & QItemSelectionModel::Columns) {
        for (int i=0; i<selection.count(); ++i)
            expanded.append(QItemSelectionRange(selection.at(i).parent(),
                                                0,
                                                selection.at(i).left(),
                                                model->rowCount(selection.at(i).parent())-1,
                                                selection.at(i).right()));
    }
    return expanded;
}

/*!
  \class QItemSelectionModel

  \brief QItemSelectionModel keeps track of a view's selected items and
  it's current item.

  \ingroup model-view

  QItemSelectionModel keeps track of a view, or several views,
  selected items. It also keeps track of a views current item.

  The selected items are stored using ranges. Whenever you want to
  modify the selected items use select() and provide either a
  QItemSelection or a QModelIndex and a
  QItemSelectionModel::SelectionCommand.

  The QItemSelectionModel has a two layer approach internally, the
  commited selected items, and the current selected items. The current
  selected items are the items part of the current interactive
  selection (for example with rubber-band selection or keyboard-shift
  selections). To update the current selected items use the
  QItemSelectionModel::Current selectionCommand or'ed with any of the
  other SelectionCommands. If you omit the
  QItemSelectionModel::Current command, a new current selection will
  be started and the previous one added to the commited selected
  items. All functions operate on both layers, so for instance
  selectedItems() will return items from both layers.

  \sa \link model-view-programming.html Model/View Programming\endlink.
*/

/*!
  Constructs a selection model that operates on the item \a model.
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
QItemSelectionModel::QItemSelectionModel(QItemSelectionModelPrivate &dd, QAbstractItemModel *model, QObject *parent)
    : QObject(dd, parent)
{
    d->model = model;
    d->init();
}

/*!
  Destroys the selectionmodel.
*/
QItemSelectionModel::~QItemSelectionModel()
{
}

/*!
  Selects the index \a item using \a selectionCommand and emits selectionChanged().

  \sa QItemSelectionModel::SelectionCommand
*/
void QItemSelectionModel::select(const QModelIndex &item, int selectionCommand)
{
    QItemSelection selection(item, item, model());
    select(selection, selectionCommand);
}

/*!
   \fn void QItemSelectionModel::currentChanged()

   This signal is emitted whenever the current item changes.

    \sa currentItem() setCurrentItem()
*/

/*!
   \fn void QItemSelectionModel::selectionChanged()

   This signal is emitted whenever the selection changes.

    \sa select()
*/

/*!
  \enum QItemSelectionModel::SelectionCommand

  This enum describes the way the selection model will be updated.

  \value NoUpdate No selection will happen.
  \value Clear The complete selection will be cleared.
  \value Select All specified indices will be selected.
  \value  Deselect All specified indices will be deselected.
  \value Toggle All specified indicies will be selected or deselected depending on their current state.
  \value Current The current selection will be updated.
  \value Rows All indices will be expanded to span rows.
  \value Columns All indices will be expanded to span columns.
  \value SelectCurrent Convenience combination of Select and Current.
  \value ToggleCurrent  Convenience combination of Toggle and Current.
  \value ClearAndSelect Convenience combination of Clear and Select.
*/

/*!
  Selects the itemselection \a selection using \a selectionCommand and emits selectionChanged().

  \sa QItemSelectionModel::SelectionCommand
*/
void QItemSelectionModel::select(const QItemSelection &selection, int selectionCommand)
{
    if (selectionCommand == NoUpdate)
        return;

    // store old selection
    QItemSelection sel = selection;
    QItemSelection old = d->ranges;
    old.merge(d->currentSelection, d->currentCommand);

    // expand selection according to SelectionBehavior
    if (selectionCommand & Rows || selectionCommand & Columns)
        sel = d->expandSelection(sel, selectionCommand);

    // clear ranges and currentSelection
    if (selectionCommand & Clear) {
        d->ranges.clear();
        d->currentSelection.clear();
    }

    // merge and clear currentSelection if Current was not set (ie. start new currentSelection)
    if (!(selectionCommand & Current)) {
        d->ranges.merge(d->currentSelection, d->currentCommand);
        d->currentSelection.clear();
    }

    // update currentSelection
    if (selectionCommand & Toggle || selectionCommand & Select || selectionCommand & Deselect) {
        d->currentCommand = selectionCommand;
        d->currentSelection = sel;
    }

    // generate new selection, compare with old and emit selectionChanged()
    QItemSelection newSelection = d->ranges;
    newSelection.merge(d->currentSelection, d->currentCommand);
    emitSelectionChanged(old, newSelection);
    return;
}

/*!
  Clears the selectionmodel. Emits selectionChanged() and currentChanged().
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
  Sets \a item to be the current item and emits currentChanged().The
  current item is used for keyboard navigation and focus indication;
  it is independent of any selected items, although a selected item
  can also be the current item.

  Depending on the \a selectionCommand a selection can also be performed.
  \sa select()
*/
void QItemSelectionModel::setCurrentItem(const QModelIndex &item, int selectionCommand)
{
    if (selectionCommand != NoUpdate)
        select(item, selectionCommand); // select item
    if (item == d->currentItem)
        return;
    QModelIndex old = d->currentItem;
    d->currentItem = QPersistentModelIndex(item, d->model); // set current
    emit currentChanged(old, item);
}

/*!
  Returns the index for the current item, or an invalid index if there is none.
*/
QModelIndex QItemSelectionModel::currentItem() const
{
    return d->currentItem;
}

/*!
  Returns true if \a item is selected.
*/
bool QItemSelectionModel::isSelected(const QModelIndex &item) const
{
    bool selected = false;
    QList<QItemSelectionRange>::const_iterator it = d->ranges.begin();
    //  search model ranges
    for (; !selected && it != d->ranges.end(); ++it)
        if ((*it).contains(item, model()))
            selected = true;
    // check  currentSelection
    if (d->currentSelection.count()) {
        if (d->currentCommand & Deselect && selected)
            selected = !d->currentSelection.contains(item, model());
        else if (d->currentCommand & Toggle)
            selected ^= d->currentSelection.contains(item, model());
        else if (d->currentCommand & Select && !selected)
            selected = d->currentSelection.contains(item, model());
    }
    return selected;
}

/*!
  Returns true if all items in \a row with parent \a parent are selected.

  Note: This function is usually faster then calling isSelected() on all items in the same row.
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
    QModelIndex item;
    QList<QItemSelectionRange>::const_iterator it;
    QList<QItemSelectionRange> joined = d->ranges;
    if (d->currentSelection.count())
        joined += d->currentSelection;
    for (int i = 0; i < model()->columnCount(parent); ++i) {
        item = model()->index(row, i, parent);
        for (it = joined.begin(); it != joined.end(); ++it)
            if ((*it).contains(item, model())) {
                i = (*it).right();
                break;
            }
        if (it == joined.end())
            return false;
    }
    return true;
}

/*!
  Returns true if all items in \a column with parent \a parent are selected.

  Note: This function is usually faster then calling isSelected() on all items in the same column.
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
    QModelIndex item;
    QList<QItemSelectionRange>::const_iterator it;
    QList<QItemSelectionRange> joined = d->ranges;
    if (d->currentSelection.count())
        joined += d->currentSelection;
    for (int i = 0; i < model()->rowCount(parent); ++i) {
         item = model()->index(i, column, parent);
         for (it = joined.begin(); it != joined.end(); ++it) {
             if ((*it).contains(item, model())) {
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
  Returns a list of all selected items. The list contains no duplicates and is not sorted.
*/
QModelIndexList QItemSelectionModel::selectedItems() const
{
    QItemSelection selected = d->ranges;
    selected.merge(d->currentSelection, d->currentCommand);
    return selected.items(model());
}

/*!
  Compares the two selections \a oldSelection and \a newSelection
  and emits selectionChanged with the deselected and selected items.
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
    QObject::connect(model, SIGNAL(contentsInserted(const QModelIndex &, const QModelIndex&)),
                     q, SLOT(clear()));
    QObject::connect(model, SIGNAL(contentsRemoved(const QModelIndex&, const QModelIndex&)),
                     q, SLOT(clear()));
}
