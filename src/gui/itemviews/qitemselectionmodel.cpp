#include "qitemselectionmodel.h"

QModelIndexList QItemSelectionRange::items(const QGenericItemModel *model) const
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
			       const QGenericItemModel *model)
{
    select(topLeft, bottomRight, model);
}

void QItemSelection::select(const QModelIndex &topLeft, const QModelIndex &bottomRight,
			    const QGenericItemModel *model)
{
    if (model->parent(topLeft) != model->parent(bottomRight))
  	return;
    ranges.append(QItemSelectionRange(model->parent(bottomRight),
				      topLeft.row(), topLeft.column(),
				      bottomRight.row(), bottomRight.column()));
}

bool QItemSelection::contains(const QModelIndex &item, const QGenericItemModel *model) const
{
    QList<QItemSelectionRange>::const_iterator it = ranges.begin();
    for (; it != ranges.end(); ++it)
	if ((*it).contains(item, model))
	    return true;
    return false;
}

QModelIndexList QItemSelection::items(QGenericItemModel *model) const
{
    QModelIndexList items;
    QList<QItemSelectionRange>::const_iterator it = ranges.begin();
    for (; it != ranges.end(); ++it)
	items += (*it).items(model);
    return items;
}

bool QItemSelection::operator==(const QItemSelection &other) const
{
    if (ranges.count() != other.ranges.count())
 	return false;
    QList<QItemSelectionRange>::const_iterator it = ranges.begin();
    QList<QItemSelectionRange>::const_iterator it2 = other.ranges.begin();
    for (; it != ranges.end() && (*it) == (*it2); ++it, ++it2);
    return (it == ranges.end() && it2 == other.ranges.end());
}

class QItemSelectionModelPrivate
{
public:
    QItemSelectionModelPrivate(QItemSelectionModel *owner, QGenericItemModel *model)
	: q(owner), model(model), selectionMode(QItemSelectionModel::Multi), toggleState(false) {}

    inline void remove(QList<QItemSelectionRange> &r)
    {
	QList<QItemSelectionRange>::const_iterator it = r.begin();
	for (; it != r.end(); ++it)
	    ranges.remove(*it);
    }

    inline void split(QItemSelectionRange &range, const QItemSelectionRange &other, QItemSelection *result)
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
	    result->ranges.append(QItemSelectionRange(parent, top, left, other_top - 1,right));
	if (other_bottom < bottom)
	    result->ranges.append(QItemSelectionRange(parent, other_bottom + 1, left, bottom, right));
	if (other_left > left)
	    result->ranges.append(QItemSelectionRange(parent, top, left, bottom, other_left - 1));
	if (other_right < right)
	    result->ranges.append(QItemSelectionRange(parent, top, other_right + 1, bottom, right));
    }

    inline QItemSelectionPointer expandRows(const QItemSelection *selection) const
    {
	if (!selection->ranges)
	    return QItemSelectionPointer();
	QModelIndex bottomRight = model->bottomRight(selection->ranges.first().parent());
	QItemSelectionPointer rows(new QItemSelection);
	QList<QItemSelectionRange>::const_iterator it = selection->ranges.begin();
	for (; it != selection->ranges.end(); ++it)
	    rows->ranges.append(QItemSelectionRange((*it).parent(), (*it).top(), 0,
						    (*it).bottom(), bottomRight.column()));
	return rows;
    }

    QItemSelectionModel *q;
    QGenericItemModel *model;
    QItemSelectionModel::SelectionMode selectionMode;
    QItemSelectionPointer currentSelection;
    QModelIndex currentItem;
    QList<QItemSelectionRange> ranges;
    bool toggleState;

};

/*!
  \class QItemSelectionModel

  \brief QItemSelectionModel keeps a list of QItemSelection objects.
*/

QItemSelectionModel::QItemSelectionModel(QGenericItemModel *model)
{
    d = new QItemSelectionModelPrivate(this, model);
}

QItemSelectionModel::~QItemSelectionModel()
{
    delete d;
}

void QItemSelectionModel::select(const QModelIndex &item,
				 SelectionUpdateMode updateMode,
				 SelectionBehavior behavior)
{
    QItemSelectionPointer selection(new QItemSelection(item, item, model()));
    select(selection, updateMode, behavior);
}

void QItemSelectionModel::select(QItemSelection *selection,
				 SelectionUpdateMode updateMode,
				 SelectionBehavior behavior )
{
    QItemSelectionPointer sel(selection);
    QItemSelectionPointer old;
    if (behavior == SelectRows)
	sel = d->expandRows(sel);
    switch (updateMode) {
	case NoUpdate:
	    return;
	case Toggle:
	    d->toggleState = true;
	    old = d->currentSelection;
	    d->currentSelection = sel;
	    exchange( old, sel, false ); //emits selectionChanged
	    return;
	case ClearAndSelect:
	    if (!!d->ranges || d->currentSelection)
		old = new QItemSelection;
	    if (!!d->ranges) {
		old->ranges += d->ranges;
		d->ranges.clear();
	    }
	    if (d->currentSelection) {
		old->ranges += d->currentSelection->ranges;
		d->currentSelection = 0;
	    }
	case Select:
	    d->toggleState = false;
	    if (d->currentSelection) {
		if (!old)
		    old = new QItemSelection;
		old->ranges += d->currentSelection->ranges;
	    }
	    d->currentSelection = sel;
	    exchange(old, sel, false); //emits selectionChanged
	    return;
        case Remove:
	    qDebug("Remove");
 	    exchange(sel, old);
	    qWarning( "QItemSelectionModel::select Remove has not been implemented yet!" );
 	    return;
    }
}

void QItemSelectionModel::clear()
{
    if (!d->ranges && !d->currentSelection)
	return;
    QItemSelectionPointer selection(new QItemSelection);
    if (!!d->ranges) {
	selection->ranges = d->ranges;
	d->ranges.clear();
    }
    if ( d->currentSelection ) {
	selection->ranges += d->currentSelection->ranges;
	d->currentSelection = 0;
    }
    QItemSelectionPointer nothing;
    emit selectionChanged(selection, nothing);
}

void QItemSelectionModel::setCurrentItem(const QModelIndex &item,
					 SelectionUpdateMode mode,
					 SelectionBehavior behavior)
{
    if (d->currentSelection && mode != NoUpdate) {
	if (d->toggleState)
	    toggle(d->currentSelection, false);
	else
	    d->ranges += d->currentSelection->ranges;
  	d->currentSelection = 0;
    }
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
    if (d->currentSelection) {
	if (d->toggleState)
	    selected ^= d->currentSelection->contains(item, model());
	else if (!selected)
	    selected = d->currentSelection->contains(item, model());
    }
    return selected;
}

bool QItemSelectionModel::isRowSelected(int row, const QModelIndex &parent) const
{
    QList<QItemSelectionRange> joined = d->ranges;
    if (d->toggleState && d->currentSelection) {
	// return false if ranges in both currentSelection and the selection model
	// intersect and have the same row contained
	QList<QItemSelectionRange> toggle = d->currentSelection->ranges;
	for (int i=0; i<toggle.count(); ++i)
	    if (toggle[i].top() <= row && toggle[i].bottom() >= row)
		for (int j=0; j<joined.count(); ++j)
		    if (joined[j].top() <= row && joined[j].bottom() >= row &&
			toggle[i].intersect(joined[j]).isValid())
			return false;
    }
    QModelIndex item;
    QList<QItemSelectionRange>::const_iterator it;
    if (d->currentSelection)
	joined += d->currentSelection->ranges;
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
    if (d->toggleState && d->currentSelection) {
	// return false if ranges in both currentSelection and the selection model
	// intersect and have the same column contained
	QList<QItemSelectionRange> toggle = d->currentSelection->ranges;
	for (int i=0; i<toggle.count(); ++i)
	    if (toggle[i].left() <= column && toggle[i].right() >= column)
		for (int j=0; j<joined.count(); ++j)
		    if (joined[j].left() <= column && joined[j].right() >= column &&
			toggle[i].intersect(joined[j]).isValid())
			return false;
    }
    QModelIndex item;
    QList<QItemSelectionRange>::const_iterator it;
    if (d->currentSelection)
	joined += d->currentSelection->ranges;
    for (int i = 0; i < model()->rowCount(parent); ++i) {
	 item = model()->index(i, column, parent);
	 for (it = joined.begin(); it != joined.end(); ++it)
	     if ((*it).contains(item, model())) {
		 i = (*it).bottom();
		 break;
	     }
	 if (it == joined.end())
	     return false;
     }
     return true;
}

QGenericItemModel *QItemSelectionModel::model() const
{
    return d->model;
}

QModelIndexList QItemSelectionModel::selectedItems() const
{
    QModelIndexList selectedItems;
    QList<QItemSelectionRange>::const_iterator it = d->ranges.begin();
    for (; it != d->ranges.end(); ++it)
	selectedItems += (*it).items(model());
    return selectedItems;
}

void QItemSelectionModel::exchange(QItemSelectionPointer &oldSelection,
				   const QItemSelectionPointer &newSelection,
				   bool alterRanges)
{
	if (!!oldSelection && alterRanges)
	    d->remove(oldSelection->ranges);

	if (!!newSelection && alterRanges)
	    d->ranges += newSelection->ranges;

	if (!!oldSelection && !!newSelection) {
	    // Find intersections between new and old selections
	    QItemSelectionPointer intersections(new QItemSelection);
	    const QList<QItemSelectionRange> &newRange = newSelection->ranges;
	    QList<QItemSelectionRange> &oldRange = oldSelection->ranges;
	    QList<QItemSelectionRange> &intRange = intersections->ranges;

	    for (int n = 0; n < newRange.count(); ++n)
		for (int o = 0; o < oldRange.count(); ++o)
		    if (newRange[n].intersects(oldRange[o]))
			intersections->ranges.append(oldRange[o].intersect(newRange[n]));

	    // Split old selections using the intersections
	    for (int i = 0; i < intRange.count(); ++i) {
		for (int o = 0; o < oldRange.count();) {
		    if (oldRange[o].intersects(intRange[i])) {
			d->split(oldRange[o], intRange[i], oldSelection);
			oldRange.removeAt(o);
		    } else {
			++o;
		    }
		}
	    }
	}

// 	qDebug( "QItemSelectionModel::exchange old %d new %d",
// 		oldSelection ? oldSelection->refCount() : -1,//oldSelection->ranges.count() : -1,
// 		newSelection ? newSelection->refCount() : -1 );//newSelection->ranges.count() : -1 );

	// The result will be the deselected ranges
	emit selectionChanged(oldSelection, newSelection);
}

void QItemSelectionModel::toggle(const QItemSelectionPointer &selection, bool emitSelectionChanged)
{
    QItemSelectionPointer oldSelection(new QItemSelection);
    oldSelection->ranges = d->ranges;
    QItemSelectionPointer newSelection(new QItemSelection);
    newSelection->ranges = selection->ranges;

    // Collect intersections
    QItemSelectionPointer intersections(new QItemSelection);
    QList<QItemSelectionRange> &newRange = newSelection->ranges;
    QList<QItemSelectionRange> &oldRange = oldSelection->ranges;
    QList<QItemSelectionRange> &intRange = intersections->ranges;
    for (int n = 0; n < newRange.count(); ++n) {
	for (int o = 0; o < oldRange.count(); ++o) {
	    if (newRange[n].intersects(oldRange[o]))
		intRange.append(oldRange[o].intersect(newRange[n]));
	}
    }

    //  Split the old and new ranges using the intersections
    for (int i = 0; i < intRange.count(); ++i) { // for each intersection
	for (int o = 0; o < oldRange.count();) { // splitt each old range
	    if ( oldRange[o].intersects(intRange[i])) {
		d->split(oldRange[o], intRange[i], oldSelection);
		oldRange.removeAt(o);
	    } else {
		++o;
	    }
	}
        for (int n = 0; n < newRange.count();) { // splitt each new range
	    if (newRange[n].intersects(intRange[i])) {
		d->split(newRange[n], intRange[i], newSelection);
		newRange.removeAt(n);
	    } else {
		++n;
	    }
	}
    }

    // The result is the split old and the split new selections
    d->ranges = oldRange + newRange;

    // The new selected areas will be the split newSelection
    // The deselected areas are the intersections
    if (emitSelectionChanged)
	emit selectionChanged(intersections, newSelection);
}
