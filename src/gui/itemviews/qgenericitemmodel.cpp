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

QGenericItemModel::QGenericItemModel(QObject *parent, const char *name)
    : QObject(parent, name)
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
    QVariantList variants;
    QModelIndexList::ConstIterator it = indices.begin();
    for (; it != indices.end(); ++it)
	variants << QVariant(data(*it)); // data returns a QVariantList
    QByteArray encoded;
    QDataStream stream(encoded, IO_WriteOnly);
    stream << QVariant(variants);
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
    QVariant variant;
    stream >> variant;
    if (variant.type() != QVariant::List)
	return false;
    QCoreVariantList variants = variant.toList();
    QCoreVariantList::ConstIterator it = variants.begin();
    for (int i = 0; it != variants.end(); ++it, ++i)
	appendData(*reinterpret_cast< QList<QVariant> *>(&(*it).toList()));
    return true;
}

QVariantList QGenericItemModel::data(const QModelIndex &index) const
{
    QVariantList elements;
    for (int e = 0; e < elementCount(index); ++e)
	elements << data(index, e);
    return elements;
}

void QGenericItemModel::setData(const QModelIndex &, int, const QVariant &)
{
    // do nothing - read only
}

void QGenericItemModel::setData(const QModelIndex &index, const QVariantList &elements)
{
    QVariantList::ConstIterator it = elements.begin();
    for (int e = 0; it != elements.end() && e < elementCount(index); ++it)
	setData(index, e++, *it);
}

void QGenericItemModel::insertData(const QModelIndex &, const QVariantList &)
{
    // do nothing - read only
}

void QGenericItemModel::appendData(const QVariantList &)
{
    // do nothing - read only
}

int QGenericItemModel::elementCount(const QModelIndex &index) const
{
    int element = 0;
    while (type(index, element++) != QVariant::Invalid);
    return element;
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
