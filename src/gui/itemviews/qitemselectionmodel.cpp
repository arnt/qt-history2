#include "qitemselectionmodel.h"

#include <private/qitemselectionmodel_p.h>
#define d d_func()
#define q q_func()

QModelIndexList QItemSelectionRange::items(const QAbstractItemModel *model) const
{
    QModelIndex item = model->index(top(), left(), parent());
    QModelIndexList items;
    int row, column;
    while (item.isValid()) {
	items.append(item);
	row = item.row();
	column = item.column();
	if (row >= bottom())
	    break;
	if (column >= right())
	    item = model->index(row + 1, left(), parent());
	else
	    item = model->index(row, column + 1, parent());
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

static void split(QItemSelectionRange &range, const QItemSelectionRange &other, QItemSelection *result)
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

QItemSelection QItemSelectionModelPrivate::expandRows(const QItemSelection &selection) const
{
    if (selection.size() == 0)
	return QItemSelection();
    QModelIndex bottomRight = model->bottomRight(selection.first().parent());
    QItemSelection rows;
    QList<QItemSelectionRange>::const_iterator it = selection.begin();
    for (; it != selection.end(); ++it)
	rows.append(QItemSelectionRange((*it).parent(), (*it).top(), 0,
					       (*it).bottom(), bottomRight.column()));
    return rows;
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

void QItemSelectionModel::select(const QModelIndex &item,
				 SelectionUpdateMode updateMode,
				 SelectionBehavior behavior)
{
    QItemSelection selection(item, item, model());
    select(selection, updateMode, behavior);
}

void QItemSelectionModel::select(const QItemSelection &selection,
				 SelectionUpdateMode updateMode,
				 SelectionBehavior behavior)
{
    QItemSelection sel = selection;
    QItemSelection old;
    if (behavior == SelectRows)
	sel = d->expandRows(sel);
    switch (updateMode) {
    case NoUpdate:
	return;
    case Toggle:
	mergeCurrentSelection();
    case ToggleCurrent: {
	d->toggleState = true;
	old = d->currentSelection;
	d->currentSelection = sel;
	exchange(old, sel, false); // emits selectionChanged
	return; }
    case ClearAndSelect:
// 	if (d->ranges.size() || d->currentSelection.size())
// 	    old = QItemSelection;
	if (d->ranges.size()) {
	    old += d->ranges;
	    d->ranges.clear();
	}
	if (d->currentSelection.size()) {
	    old += d->currentSelection;
	    d->currentSelection.clear();
	}
    case Select:
	mergeCurrentSelection();
    case SelectCurrent:
	d->toggleState = false;
	if (d->currentSelection.size())
	    old += d->currentSelection;
	d->currentSelection = sel;
	exchange(old, sel, false); // emits selectionChanged
	return;
    case Remove:
	qDebug("Remove");
	exchange(sel, old);
	qWarning( "QItemSelectionModel::select Remove has not been implemented yet!" );
	return;
    }
}
/*!
  \internal

  merges the currentSelection with the ranges in the selection model, does not emit any signal
*/
void QItemSelectionModel::mergeCurrentSelection()
{
    if (d->currentSelection.size()) {
	if (d->toggleState)
	    toggle(d->currentSelection, false);
	else
	    d->ranges += d->currentSelection;
	d->currentSelection.clear();
    }
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
    QModelIndexList selectedItems;
    QList<QItemSelectionRange>::const_iterator it = d->ranges.begin();
    for (; it != d->ranges.end(); ++it)
	selectedItems += (*it).items(model());
    if (!d->toggleState && d->currentSelection.size())
	selectedItems += d->currentSelection.items(model());
    return selectedItems;
}

void QItemSelectionModel::exchange(QItemSelection &oldSelection,
				   const QItemSelection &newSelection,
				   bool alterRanges)
{
	if (oldSelection.size() && alterRanges)
	    d->remove(oldSelection);

	if (newSelection.size() && alterRanges)
	    d->ranges += newSelection;

	if (oldSelection.size() && newSelection.size()) {
	    // Find intersections between new and old selections
	    QItemSelection intersections;

	    for (int n = 0; n < newSelection.count(); ++n)
		for (int o = 0; o < oldSelection.count(); ++o)
		    if (newSelection.at(n).intersects(oldSelection.at(o)))
			intersections.append(oldSelection.at(o).intersect(newSelection.at(n)));

	    // Split old selections using the intersections
	    for (int i = 0; i < intersections.count(); ++i) {
		for (int o = 0; o < oldSelection.count();) {
		    if (oldSelection.at(o).intersects(intersections.at(i))) {
			split(oldSelection[o], intersections.at(i), &oldSelection);
			oldSelection.removeAt(o);
		    } else {
			++o;
		    }
		}
	    }
	}

// 	qDebug( "QItemSelectionModel::exchange old %d new %d",
// 		oldSelection ? oldSelection->refCount() : -1,//oldSelection->count() : -1,
// 		newSelection ? newSelection->refCount() : -1 );//newSelection->count() : -1 );

	// The result will be the deselected ranges
	emit selectionChanged(oldSelection, newSelection);
}

void QItemSelectionModel::toggle(const QItemSelection &selection, bool emitSelectionChanged)
{
    QItemSelection oldSelection;
    oldSelection += d->ranges;
    QItemSelection newSelection = selection;

    // Collect intersections
    QItemSelection intersections;
    for (int n = 0; n < newSelection.count(); ++n) {
	for (int o = 0; o < oldSelection.count(); ++o) {
	    if (newSelection.at(n).intersects(oldSelection.at(o)))
		intersections.append(oldSelection.at(o).intersect(newSelection.at(n)));
	}
    }

    //  Split the old and new ranges using the intersections
    for (int i = 0; i < intersections.count(); ++i) { // for each intersection
	for (int o = 0; o < oldSelection.count();) { // splitt each old range
	    if (oldSelection.at(o).intersects(intersections.at(i))) {
		split(oldSelection[o], intersections.at(i), &oldSelection);
		oldSelection.removeAt(o);
	    } else {
		++o;
	    }
	}
        for (int n = 0; n < newSelection.count();) { // splitt each new range
	    if (newSelection.at(n).intersects(intersections.at(i))) {
		split(newSelection[n], intersections.at(i), &newSelection);
		newSelection.removeAt(n);
	    } else {
		++n;
	    }
	}
    }

    // The result is the split old and the split new selections
    static_cast< QList<QItemSelectionRange> >(d->ranges) = oldSelection + newSelection;

    // The new selected areas will be the split newSelection
    // The deselected areas are the intersections
    if (emitSelectionChanged)
	emit selectionChanged(intersections, newSelection);
}
