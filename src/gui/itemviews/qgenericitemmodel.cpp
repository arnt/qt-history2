#include "qgenericitemmodel.h"
#include <qdatastream.h>

/*!
  \class QModelIndex qgenericitemmodel.h

  \brief class used to access data in the data model.

  This class is used as an index into QGenericItemModel derived data models.
  The index is used by itemviews, delegates and selection models to represent an item in the model.
  QModelIndex objects are created by the model.
*/

/*!
  \class QGenericItemModel qgenericitemmodel.h

  \brief Abstract class used to provide abitrary data.

  This class is an abstract class which provides data.  The data model is a hierarchy of rows. The data is accessed
  by calling data() with a QModelIndex object.  Functions rowCunt() and columnCount() returns
  the size of the data source which is provided by the model on each leve in the heierarchy.

  fetchMore() can be reimplemented by models where it is hard  to determine the final size of the data (e.g. data streams ).
  The model is also responsible for sorting its data through the virtual functions
  sort() and isSortable().

  To notify about changes (e.g. an item changed, new data was added, etc.), the QGenericItemModel emits the contentsChanged,
  contentsInserted and/or contentsRemoved signals.
*/

QGenericItemModel::QGenericItemModel(QObject *parent)
    : QObject(parent)
{
}

QGenericItemModel::QGenericItemModel(QObjectPrivate &dp, QObject *parent)
    : QObject(dp, parent)
{
}

QGenericItemModel::~QGenericItemModel()
{
}

QModelIndex QGenericItemModel::index(int row, int column, const QModelIndex &parent,
                                     QModelIndex::Type type) const
{
    if (row >= 0 && row < rowCount(parent) && column >= 0 && column < columnCount(parent))
	return QModelIndex(row, column, 0, type);
    return QModelIndex();
}

QModelIndex QGenericItemModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

bool QGenericItemModel::hasChildren(const QModelIndex &parent) const
{
    return (rowCount(parent) > 0) && (columnCount(parent) > 0);
}

void QGenericItemModel::fetchMore()
{
    // do nothing
}

const char *QGenericItemModel::format(int i) const
{
    if (i == 0)
	return "application/x-qgenericmodeldatalist";
    return 0;
}

QByteArray QGenericItemModel::encodedData(const char *mime, const QModelIndexList &indices) const
{
    if (indices.count() <= 0 || QString(mime) != format(0))
	return QByteArray();

    QByteArray encoded;
    QDataStream stream(encoded, IO_WriteOnly);
    QModelIndexList::ConstIterator it = indices.begin();
    for (; it != indices.end(); ++it) {
	QMap<int, QVariant> roles = itemData((*it));
	for (QMap<int, QVariant>::ConstIterator r = roles.begin(); r != roles.end(); ++r)
	    stream << r.key() << r.value();
	stream << -1;
    }
    return encoded;
}

bool QGenericItemModel::canDecode(QMimeSource *src) const
{
    return src->provides(format(0));
}

bool QGenericItemModel::decode(QMimeSource *src)
{
    if (!canDecode(src))
	return false;

    QByteArray encoded = src->encodedData(format(0));
    QDataStream stream(encoded, IO_ReadOnly);
    bool newItem = true;
    int role;
    QVariant variantData;
    QModelIndex insertedItem;
    while (!stream.atEnd()) {
	stream >> role;
	if (role > -1) {
	    if (newItem) {
		insertedItem = insertItem();
		newItem = false;
	    }
	    stream >> variantData;
	    setData(insertedItem, role, variantData);
	} else {
	    newItem = true;
	}
    }
    return true;
}

/*!
  Returns a map with values for all predefined roles in the model. Must be reimplemented if you extend the model
  with customized roles.

  \sa Role
*/

QMap<int, QVariant> QGenericItemModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> roles;
    for (int i=0; i<User; ++i) {
	QVariant variantData = data(index, i);
	if (variantData != QVariant::Invalid)
	    roles.insert(i, variantData);
    }
    return roles;
}

void QGenericItemModel::setData(const QModelIndex &, int, const QVariant &)
{
    // do nothing - read only
}

void QGenericItemModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    for (QMap<int, QVariant>::ConstIterator it = roles.begin(); it != roles.end(); ++it)
	setData(index, it.key(), it.value());
}

/*!
  Inserts a new item at QModelIndex, or if QModelIndex is invalid or out
  of bounds appends at the end of the model.  Returns and invalid
  QModelIndex on failure or the QModelIndex of the inserted item.
*/

QModelIndex QGenericItemModel::insertItem(const QModelIndex &)
{
    // read-only does nothing
    return QModelIndex();
}

bool QGenericItemModel::isSelectable(const QModelIndex &) const
{
    return false;
}

bool QGenericItemModel::isEditable(const QModelIndex &) const
{
    return false;
}

bool QGenericItemModel::isDragEnabled(const QModelIndex &) const
{
    return false;
}

bool QGenericItemModel::isDropEnabled(const QModelIndex &) const
{
    return false;
}

bool QGenericItemModel::isSortable() const
{
    return false;
}

void QGenericItemModel::sort(int, SortOrder)
{
    // do nothing
}

bool QGenericItemModel::equal(const QModelIndex &left, const QModelIndex &right) const
{
    return left == right;
}

bool QGenericItemModel::greater(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.row() == right.row())
	return left.column() > right.column();
    return left.row() > right.row();
}
