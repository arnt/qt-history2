#include "qgenericitemmodel.h"
#include <qdatastream.h>

/*!
  \class QGenericModelItem qgenericitemmodel.h

  \brief Encapsulates the data of an item in a QGenericItemModel

  This class is used to encapsulate a specific data item in a
  model. To retrieve one, QGenericItemModel::item() is used.

  A QGenericModelItem object has a short life, meaning for every time
  retrieving an item you get a new QGenericModelItem object. To ensure
  that the data item is not deleted too early, it is reference
  counted. Too ease the use of it, in the API QGenericModelItemPtr
  objects are passed around, which do automatic reference counting in
  the QGenericModelItem.

  description() returns what type of data this item provides. This can
  be Text, Pixmap or a user defined data field (or a mixture). Using
  text(), iconSet(), data() and dataPtr() the data can be
  retrieved. Using setText(), setIconSet(), setData() and
  setDataPtr(), data can be set on the item. The default
  implementation of those functions do nothing, they need to be
  reimplemented to work on the specific data of the data model.

  Data items in a model are structured in a table. row() and column()
  return the position of that item in the table. Using nextRow(),
  nextColumn(), prevRow() and prevColumn() it is possible to traverse
  the table. A data item can also span over multiple rows or columns
  (rowSpan() and colSpan()).

  An item can have children. Using hasChildren() and numChildren(),
  firstChild() and parent() the relevant information can be found
  out. A child level starts a new sub-table, meaning the row() and
  column() of a first child of an item is (0,0).

  All the above functions except parent() need to be reimplemented to
  work on the specific data of a model.

  An item can also define, whether it can be edited and seleced by a
  view (isEditable() and isSelectable()).

  The model of an item can be accessed using model().
*/

/*!
  \class QGenericItemModel qgenericitemmodel.h

  \brief Abstract class used to provide abitrary data to a view

  This class is an abstract class which provides item data which can
  be displayed in different item views. Using item() the data of a
  specific item can be retrieved, numRows() and numColumns() returns
  the size of the data source which is provided by the model. All
  those three functions need to be reimplemented.
  fetchMore() can be reimplemented by models where it is hard  to determine the
  final size of the data (e.g. data streams ).
  The model is also responsible for sorting its data through the virtual functions
  sort() and isSortable().

  The data which the model provides is encapsulatd in QGenericModelItem.

  To notify about changes (e.g. an item changed,
  new data was added, etc.), the QGenericItemModel emits the itemChanged,
  numRowsChanged, numColumnChanged and/or contentsChanged signals.
*/

QGenericItemModel::QGenericItemModel(QObject *parent, const char *name)
    : QObject(parent, name)
{
}

QGenericItemModel::~QGenericItemModel()
{
}

QModelIndex QGenericItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row >= 0 && row < rowCount(parent) && column >= 0 && column < columnCount(parent))
	return QModelIndex(row, column, 0);
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
	variants << dataList(*it);
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
	appendDataList(*it);
    return true;
}

QVariant QGenericItemModel::dataList(const QModelIndex &index) const
{
    QVariantList elementList;
    for (int e = 0; e < elementCount(index); ++e)
	elementList << data(index, e);
    return QVariant(elementList);
}

void QGenericItemModel::setData(const QModelIndex &, int, const QVariant &)
{
    // do nothing - read only
}

void QGenericItemModel::setDataList(const QModelIndex &index, const QVariant &values)
{
    if (values.type() != QVariant::List)
	return;
    QCoreVariantList elementList = values.toList();
    QCoreVariantList::ConstIterator it = elementList.begin();
    for (int e = 0; it != elementList.end() && e < elementCount(index); ++it)
	setData(index, e++, *it);
}

void QGenericItemModel::insertDataList(const QModelIndex &, const QVariant &)
{
    // do nothing - read only
}

void QGenericItemModel::appendDataList(const QVariant &)
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
