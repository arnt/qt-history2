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
    if (model->parent(topLeft) != model->parent(bottomRight))
        return;
    append(QItemSelectionRange(model->parent(bottomRight),
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

  Note: Only QItemSelectionModel::Toggle and
  QItemSelectionModel::Select are supported.
*/
void QItemSelection::merge(const QItemSelection &other, int update)
{
    if (!(update & QItemSelectionModel::Toggle || update & QItemSelectionModel::Select) ||
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
        for (int n = 0; (update & QItemSelectionModel::Toggle) && n < newSelection.count();) {
            if (newSelection.at(n).intersects(intersections.at(i))) {
                split(newSelection.at(n), intersections.at(i), &newSelection);
                newSelection.removeAt(n);
            } else {
                ++n;
            }
        }
    }
    operator+=(newSelection);
}

/*!
  \internal

  returns a QItemSelection where all ranges have been expanded to:
  SelectRows: left: 0 and right: columnCount()-1
  SelectColumns: top: 0 and bottom: rowCount()-1
*/

QItemSelection QItemSelectionModelPrivate::expandSelection(
    const QItemSelection &selection,
    QItemSelectionModel::SelectionBehavior behavior) const
{
    if (selection.count() == 0)
        return selection;

    QItemSelection expanded;
    switch (behavior) {
    case QItemSelectionModel::SelectRows:
        for (int i=0; i<selection.count(); ++i)
            expanded.append(QItemSelectionRange(selection.at(i).parent(),
                                                selection.at(i).top(),
                                                0,
                                                selection.at(i).bottom(),
                                                model->columnCount(selection.at(i).parent())-1));
        break;
    case QItemSelectionModel::SelectColumns:
        for (int i=0; i<selection.count(); ++i)
            expanded.append(QItemSelectionRange(selection.at(i).parent(),
                                                0,
                                                selection.at(i).left(),
                                                model->rowCount(selection.at(i).parent())-1,
                                                selection.at(i).right()));
        break;
    default:
        expanded = selection;
        break;
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

void QItemSelectionModel::select(const QModelIndex &item, int updateMode,
                                 SelectionBehavior behavior)
{
    QItemSelection selection(item, item, model());
    select(selection, updateMode, behavior);
}

void QItemSelectionModel::select(const QItemSelection &selection, int updateMode,
                                 SelectionBehavior behavior)
{
    if (updateMode == NoUpdate)
        return;

    QItemSelection sel = selection;
    QItemSelection old = d->ranges;
    old.merge(d->currentSelection, d->toggleState ? Toggle : Select);

    if (d->selectionMode == Single) {
        if (!sel.isEmpty()) {
            QModelIndex singleIndex = model()->index(sel.at(sel.count()-1).bottom(),
                                                     sel.at(sel.count()-1).right(),
                                                     sel.at(sel.count()-1).parent());
            sel = QItemSelection(singleIndex, singleIndex, model());
        }
        updateMode |= Clear;
    }

    if (behavior != SelectItems)
        sel = d->expandSelection(sel, behavior);

    // clear ranges and currentSelection
    if (updateMode & Clear) {
        d->ranges.clear();
        d->currentSelection.clear();
    }

    // merge and clear currentSelection if Current was not set (ie. start new currentSelection)
    if (!(updateMode & Current)) {
        d->ranges.merge(d->currentSelection, d->toggleState ? Toggle : Select);
        d->currentSelection.clear();
    }

    // update currentSelection
    if (updateMode & Toggle || updateMode & Select) {
        d->toggleState = (updateMode & Toggle);
        d->currentSelection = sel;
    }

    // generate new selection, compare with old and emit selectionChanged()
    QItemSelection newSelection = d->ranges;
    newSelection.merge(d->currentSelection, d->toggleState ? Toggle : Select);
    emitSelectionChanged(old, newSelection);
    return;
}

void QItemSelectionModel::clear()
{
    if (d->ranges.size() == 0 && d->currentSelection.size() == 0)
        return;
    QItemSelection selection;
    if (d->ranges.size()) {
        selection = d->ranges;
        d->ranges.clear();
    }
    selection += d->currentSelection;
    d->currentSelection.clear();
    emit selectionChanged(selection, QItemSelection());
}

void QItemSelectionModel::setCurrentItem(const QModelIndex &item,
                                         SelectionUpdateMode mode,
                                         SelectionBehavior behavior)
{
    if (mode != NoUpdate)
        select(item, mode, behavior); // select item
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

void QItemSelectionModel::setSelectionMode(SelectionMode mode)
{
    d->selectionMode = mode;
}

QItemSelectionModel::SelectionMode QItemSelectionModel::selectionMode() const
{
    return d->selectionMode;
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
    if (d->currentSelection.size()) {
        if (d->toggleState)
            selected ^= d->currentSelection.contains(item, model());
        else if (!selected)
            selected = d->currentSelection.contains(item, model());
    }
    return selected;
}

bool QItemSelectionModel::isRowSelected(int row, const QModelIndex &parent) const
{
    QList<QItemSelectionRange> joined = d->ranges;
    if (d->toggleState && d->currentSelection.size()) {
        // return false if ranges in both currentSelection and the selection model
        // intersect and have the same row contained
        QList<QItemSelectionRange> toggle = d->currentSelection;
        for (int i=0; i<toggle.count(); ++i)
            if (toggle.at(i).top() <= row && toggle.at(i).bottom() >= row)
                for (int j=0; j<joined.count(); ++j)
                    if (joined.at(j).top() <= row && joined.at(j).bottom() >= row
                        && toggle.at(i).intersect(joined.at(j)).isValid())
                        return false;
    }
    QModelIndex item;
    QList<QItemSelectionRange>::const_iterator it;
    if (d->currentSelection.size())
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
    QList<QItemSelectionRange> joined = d->ranges;
    if (d->toggleState && d->currentSelection.size()) {
        // return false if ranges in both currentSelection and the selection model
        // intersect and have the same column contained
        QList<QItemSelectionRange> toggle = d->currentSelection;
        for (int i=0; i<toggle.count(); ++i) {
            if (toggle.at(i).left() <= column && toggle.at(i).right() >= column) {
                for (int j=0; j<joined.count(); ++j) {
                    if (joined.at(j).left() <= column && joined.at(j).right() >= column
                        && toggle.at(i).intersect(joined.at(j)).isValid()) {
                        return false;
                    }
                }
            }
        }
    }
    QModelIndex item;
    QList<QItemSelectionRange>::const_iterator it;
    if (d->currentSelection.size())
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
    selected.merge(d->currentSelection, d->toggleState ? Toggle : Select);
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
                split(deselected.at(o), intersections.at(i), &deselected);
                deselected.removeAt(o);
            } else {
                ++o;
            }
        }
        // split selected
        for (int s = 0; s < selected.count();) {
            if (selected.at(s).intersects(intersections.at(i))) {
                split(selected.at(s), intersections.at(i), &selected);
                selected.removeAt(s);
            } else {
                ++s;
            }
        }
    }

    emit selectionChanged(deselected, selected);
}
