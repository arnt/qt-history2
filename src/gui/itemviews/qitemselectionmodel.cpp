#include "qitemselectionmodel.h"

#include <private/qitemselectionmodel_p.h>
#define d d_func()
#define q q_func()

static void split(const QItemSelectionRange &range, const QItemSelectionRange &other, QItemSelection *result)
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
  Merges selection \a other with this QItemSelection using the \a update mode.
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
                remove(t);
            } else {
                ++t;
            }
        }
        // only split newSelection if Toggle is specified
        for (int n = 0; (selectionCommand & QItemSelectionModel::Toggle) && n < newSelection.count();) {
            if (newSelection.at(n).intersects(intersections.at(i))) {
                split(newSelection.at(n), intersections.at(i), &newSelection);
                newSelection.remove(n);
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

  \brief QItemSelectionModel keeps a list of QItemSelection objects.
*/

QItemSelectionModel::QItemSelectionModel(QAbstractItemModel *model, QObject *parent)
    : QObject(*new QItemSelectionModelPrivate, parent)
{
    d->model = model;
}

QItemSelectionModel::QItemSelectionModel(QItemSelectionModelPrivate &dd, QAbstractItemModel *model, QObject *parent)
    : QObject(dd, parent)
{
    d->model = model;
}

QItemSelectionModel::~QItemSelectionModel()
{
}

void QItemSelectionModel::select(const QModelIndex &item, int selectionCommand)
{
    QItemSelection selection(item, item, model());
    select(selection, selectionCommand);
}

/*!
  \enum QItemSelectionModel::SelectionCommand

  This enum type is used to describe in what way the selectionmodel will be updated.

  The states are

  \value NoUpdate No selection will happen
  \value Clear The complete selection will be cleared
  \value Select All specified indices will be selected
  \value  Deselect All specified indices will be deselected
  \value Toggle All specified indicies will be selected or deselected depending on their current state
  \value Current The current selection will be updated
  \value Rows All indices will be expanded to span rows
  \value Columns All indices will be expanded to span columns
  \value SelectCurrent Convenience combination of Select and Current
  \value ToggleCurrent  Convenience combination of Toggle and Current
  \value ClearAndSelect Convenience combination of Clear and Select
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

void QItemSelectionModel::clear()
{
    if (d->ranges.count() == 0 && d->currentSelection.count() == 0)
        return;
    QItemSelection selection = d->ranges;
    selection.merge(d->currentSelection, d->currentCommand);
    d->ranges.clear();
    d->currentSelection.clear();
    emit selectionChanged(selection, QItemSelection());
}

void QItemSelectionModel::setCurrentItem(const QModelIndex &item, int selectionCommand)
{
    if (selectionCommand != NoUpdate)
        select(item, selectionCommand); // select item
    if (item == d->currentItem)
        return;
    QModelIndex old = d->currentItem;
    d->currentItem = item; // set current
    emit currentChanged(old, item);
}

QModelIndex QItemSelectionModel::currentItem() const
{
    return d->currentItem;
}

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
            selected != d->currentSelection.contains(item, model());
        else if (d->currentCommand & Toggle)
            selected ^= d->currentSelection.contains(item, model());
        else if (d->currentCommand & Select && !selected)
            selected = d->currentSelection.contains(item, model());
    }
    return selected;
}

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

QAbstractItemModel *QItemSelectionModel::model() const
{
    return d->model;
}

QModelIndexList QItemSelectionModel::selectedItems() const
{
    QItemSelection selected = d->ranges;
    selected.merge(d->currentSelection, d->currentCommand);
    return selected.items(model());
}

/*!
  compares the two selections \a oldSelection and \a newSelection and
  emits selectionChanged with the deselected and selected items.
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
                deselected.remove(o);
                selected.remove(s);
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
                split(deselected.at(o), intersections.at(i), &deselected);
                deselected.remove(o);
            } else {
                ++o;
            }
        }
        // split selected
        for (int s = 0; s < selected.count();) {
            if (selected.at(s).intersects(intersections.at(i))) {
                split(selected.at(s), intersections.at(i), &selected);
                selected.remove(s);
            } else {
                ++s;
            }
        }
    }

    emit selectionChanged(deselected, selected);
}
