#include "qabstractitemmodel.h"
#include <qdragobject.h>
#include <qdatastream.h>
#include <qdebug.h>

class QAbstractItemModelDrag : public QDragObject
{
public:
    QAbstractItemModelDrag(const QModelIndexList &indices, QAbstractItemModel *model, QWidget *dragSource);
    ~QAbstractItemModelDrag();

    const char *format(int i) const;
    bool provides(const char *mime) const;
    QByteArray encodedData(const char *mime) const;

    static bool canDecode(QMimeSource *src);
    static bool decode(QMimeSource *src, QAbstractItemModel *model, const QModelIndex &parent);

    static const char *format();

    QAbstractItemModel *model;
    QModelIndexList indices;
};

QAbstractItemModelDrag::QAbstractItemModelDrag(const QModelIndexList &indices,
                                               QAbstractItemModel *model,
                                               QWidget *dragSource)
    : QDragObject(dragSource), model(model), indices(indices)
{
}

QAbstractItemModelDrag::~QAbstractItemModelDrag()
{
}

const char *QAbstractItemModelDrag::format(int i) const
{
    if (i == 0)
        return format();
    return 0;
}

bool QAbstractItemModelDrag::provides(const char *mime) const
{
    return QString(mime) == QString(format());
}

QByteArray QAbstractItemModelDrag::encodedData(const char *mime) const
{
    if (indices.count() <= 0 || !provides(mime))
        return QByteArray();

    QByteArray encoded;
    QDataStream stream(&encoded, IO_WriteOnly);
    QModelIndexList::ConstIterator it = indices.begin();
    for (; it != indices.end(); ++it) {
        QMap<int, QVariant> data = model->itemData((*it));
        stream << data.count();
        QMap<int, QVariant>::ConstIterator it2 = data.begin();
        for (; it2 != data.end(); ++it2) {
            stream << it2.key();
            stream << it2.value();
        }
    }
    return encoded;
}

bool QAbstractItemModelDrag::canDecode(QMimeSource *src)
{
    return src->provides(format());
}

bool QAbstractItemModelDrag::decode(QMimeSource *src,
                                    QAbstractItemModel *model,
                                    const QModelIndex &parent)
{
    if (!canDecode(src))
        return false;
    return false;

    QByteArray encoded = src->encodedData(format());
    QDataStream stream(&encoded, IO_ReadOnly);
    int row = model->rowCount(parent);
    int count, role;
    QVariant data;
    QModelIndex index;
    while (!stream.atEnd()) {
        model->insertRow(row, parent); // append row
        index = model->index(row, 0, parent); // only insert in col 0
        stream >> count;
        for (int i = 0; i < count; ++i) {
            stream >> role;
            stream >> data;
            model->setData(index, role, data);
        }
    }
    return true;
}

const char *QAbstractItemModelDrag::format()
{
    return "application/x-qabstractitemmodeldatalist";
}

/*!
  \class QModelIndex qgenericitemmodel.h

  \brief class used to access data in the data model.

  This class is used as an index into QAbstractItemModel derived data models.
  The index is used by itemviews, delegates and selection models to represent an item in the model.
  QModelIndex objects are created by the model.
*/

/*!
  \class QAbstractItemModel qgenericitemmodel.h

  \brief Abstract class used to provide abitrary data.

  This class is an abstract class which provides data.  The data model is a hierarchy of rows. The data is accessed
  by calling data() with a QModelIndex object.  Functions rowCunt() and columnCount() returns
  the size of the data source which is provided by the model on each leve in the heierarchy.

  fetchMore() can be reimplemented by models where it is hard  to determine the final size of the data (e.g. data streams).
  The model is also responsible for sorting its data through the virtual functions
  sort() and isSortable().

  To notify about changes (e.g. an item changed, new data was added, etc.), the QAbstractItemModel emits the contentsChanged,
  contentsInserted and/or contentsRemoved signals.
*/

QAbstractItemModel::QAbstractItemModel(QObject *parent)
    : QObject(parent)
{
}

QAbstractItemModel::QAbstractItemModel(QObjectPrivate &dp, QObject *parent)
    : QObject(dp, parent)
{
}

QAbstractItemModel::~QAbstractItemModel()
{
}

QModelIndex QAbstractItemModel::index(int row, int column, const QModelIndex &parent,
                                     QModelIndex::Type type) const
{
    if (row >= 0 && row < rowCount(parent) && column >= 0 && column < columnCount(parent))
        return QModelIndex(row, column, 0, type);
    return QModelIndex();
}

QModelIndex QAbstractItemModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

bool QAbstractItemModel::hasChildren(const QModelIndex &parent) const
{
    return (rowCount(parent) > 0) && (columnCount(parent) > 0);
}

void QAbstractItemModel::fetchMore()
{
    // do nothing
}

bool QAbstractItemModel::canDecode(QMimeSource *src) const
{
    return QAbstractItemModelDrag::canDecode(src);
}

bool QAbstractItemModel::decode(QDropEvent *e, const QModelIndex &parent)
{
    return QAbstractItemModelDrag::decode(e, this, parent);
}

QDragObject *QAbstractItemModel::dragObject(const QModelIndexList &indices, QWidget *dragSource)
{
    return new QAbstractItemModelDrag(indices, this, dragSource);
}

/*!
  Returns a map with values for all predefined roles in the model. Must be reimplemented if you extend the model
  with customized roles.

  \sa Role
*/

QMap<int, QVariant> QAbstractItemModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> roles;
    for (int i=0; i<User; ++i) {
        QVariant variantData = data(index, i);
        if (variantData != QVariant::Invalid)
            roles.insert(i, variantData);
    }
    return roles;
}

bool QAbstractItemModel::setData(const QModelIndex &, int, const QVariant &)
{
    return false;
}

bool QAbstractItemModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    bool b = true;
    for (QMap<int, QVariant>::ConstIterator it = roles.begin(); it != roles.end(); ++it)
        b = b && setData(index, it.key(), it.value());
    return b;
}

bool QAbstractItemModel::insertRow(int, const QModelIndex &)
{
    return false;
}

bool QAbstractItemModel::insertColumn(int, const QModelIndex &)
{
    return false;
}

bool QAbstractItemModel::removeRow(int, const QModelIndex &)
{
    return false;
}

bool QAbstractItemModel::removeColumn(int, const QModelIndex &)
{
    return false;
}

bool QAbstractItemModel::isSelectable(const QModelIndex &) const
{
    return false;
}

bool QAbstractItemModel::isEditable(const QModelIndex &) const
{
    return false;
}

bool QAbstractItemModel::isDragEnabled(const QModelIndex &) const
{
    return false;
}

bool QAbstractItemModel::isDropEnabled(const QModelIndex &) const
{
    return false;
}

bool QAbstractItemModel::isSortable() const
{
    return false;
}

void QAbstractItemModel::sort(int, SortOrder)
{
    // do nothing
}

bool QAbstractItemModel::equal(const QModelIndex &left, const QModelIndex &right) const
{
    return left == right;
}

bool QAbstractItemModel::greater(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.row() == right.row())
        return left.column() > right.column();
    return left.row() > right.row();
}

QModelIndexList QAbstractItemModel::match(const QModelIndex &start, int role, const QVariant &value, int hits) const
{
    QModelIndexList result;
    QString val = value.toString().toLower();
    QModelIndex idx;
    QModelIndex par = parent(start);
    int hit = 0;
    int col = start.column();
    for (int row = start.row(); row < rowCount(par) && hit < hits; ++row) {
        idx = index(row, col, par);
        if (data(idx, role).toString().toLower().startsWith(val)) {
            result.append(idx);
            ++hit;
        }
    }
    return result;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QModelIndex &idx)
{
    dbg.nospace() << "QModelIndex(" << idx.row() << "," << idx.column() << ",";
    switch (idx.type()) {
    case QModelIndex::View:
        dbg.nospace() << "View";
        break;
    case QModelIndex::HorizontalHeader:
        dbg.nospace() << "HorizontalHeader";
        break;
    case QModelIndex::VerticalHeader:
        dbg.nospace() << "VerticalHeader";
        break;
    }
    dbg.nospace() << ")";
    return dbg.space();
}
#endif

