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
  \class QAbstractItemModel qabstractitemmodel.h

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

/*!
  Constructs a itemmodel.
*/
QAbstractItemModel::QAbstractItemModel(QObject *parent)
    : QObject(parent)
{
}

/*!
  \internal
*/
QAbstractItemModel::QAbstractItemModel(QObjectPrivate &dp, QObject *parent)
    : QObject(dp, parent)
{
}

/*!
  Destroys the itemmodel.
*/
QAbstractItemModel::~QAbstractItemModel()
{
}

/*!
  \fn void QAbstractItemModel::contentsChanged()

  This signal is emitted when the data in exiting items changes.

  \sa setData()
*/

/*!
  \fn void QAbstractItemModel::contentsInserted()

  This signal is emitted when rows or columns are inserted in the model.

  \sa insertRow() insertColumn()
*/

/* \fn void QAbstractItemModel::contentsRemoved()

This signal is emitted when rows or columns are removed from the model.

\sa removeRow() removeColumn()
*/

/*!
  \enum QAbstractItemModel::Role

  Each item in the model can have a set of data with associated roles.
  The roles are used when visualizing and editing the items in the view.

  \value Display The data rendered as item text
  \value Decoration The data rendered as item icon
  \value Edit The data edited in the item editor.
  \value ToolTip The data displayed in the item tooltip
  \value StatusTip The data displayed in the status bar
  \value WhatsThis The data displayed in what's this mode
  \value User The first custom role defined by the user
*/

/*!
  Returns the QModelIndex object associated with the data in \a row \a column with \a parent.
 */
QModelIndex QAbstractItemModel::index(int row, int column, const QModelIndex &parent,
                                     QModelIndex::Type type) const
{
    if (row >= 0 && row < rowCount(parent) && column >= 0 && column < columnCount(parent))
        return QModelIndex(row, column, 0, type);
    return QModelIndex();
}

/*!
  Returns the parent QModelIndex object of \a child.
*/
QModelIndex QAbstractItemModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

/*!
  Returns true if \a parent has any children; otherwise returns false.
*/
bool QAbstractItemModel::hasChildren(const QModelIndex &parent) const
{
    return (rowCount(parent) > 0) && (columnCount(parent) > 0);
}

/*!
  FIXME
*/
void QAbstractItemModel::fetchMore()
{
    // do nothing
}

/*!
  Returns true if decode() would be able to decode \a src;
  otherwise returns false.
*/
bool QAbstractItemModel::canDecode(QMimeSource *src) const
{
    return QAbstractItemModelDrag::canDecode(src);
}

/*!
  Decodes data from \a e, inserting the data under \a parent (if possible).

  Returns true if the data was successfully decoded and inserted;
  otherwise returns false.
*/
bool QAbstractItemModel::decode(QDropEvent *e, const QModelIndex &parent)
{
    return QAbstractItemModelDrag::decode(e, this, parent);
}

/*!
  Returns a pointer to a QDragObject object containing the data associated with \a indices.

  \sa itemData
 */
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

/*!
  Sets the data in \a index, \a role to \a value.
  Returns true if successfull; otherwise returns false.

  \sa data
*/
bool QAbstractItemModel::setData(const QModelIndex &, int, const QVariant &)
{
    return false;
}

/*!
  FIXME
*/
bool QAbstractItemModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    bool b = true;
    for (QMap<int, QVariant>::ConstIterator it = roles.begin(); it != roles.end(); ++it)
        b = b && setData(index, it.key(), it.value());
    return b;
}

/*!
  Inserts a new row in the model after \a row. The row will be a child of \a parent.
  Returns true if the row was successfully inserted; otherwise returns false.
*/
bool QAbstractItemModel::insertRow(int, const QModelIndex &)
{
    return false;
}

/*!
  Inserts a new column in the model after \a column. The column will be a child of \a parent.
  Returns true if the column was successfully inserted; otherwise returns false.
*/
bool QAbstractItemModel::insertColumn(int, const QModelIndex &)
{
    return false;
}

/*!
  Removes row \a row in the model.
  Returns true if the row was successfully removed; otherwise returns false.
*/
bool QAbstractItemModel::removeRow(int, const QModelIndex &)
{
    return false;
}

/*!
  Removes column \a column in the model.
  Returns true if the column was successfully removed; otherwise returns false.
*/
bool QAbstractItemModel::removeColumn(int, const QModelIndex &)
{
    return false;
}

/*!
  Returns true if the item associated with \a index is selectable; otherwise returns false.
*/
bool QAbstractItemModel::isSelectable(const QModelIndex &) const
{
    return false;
}

/*!
  Returns true if the item associated with \a index is editable; otherwise returns false.
*/
bool QAbstractItemModel::isEditable(const QModelIndex &) const
{
    return false;
}

/*!
  Returns true if the item associated with \a index can be dragged; otherwise returns false.
*/
bool QAbstractItemModel::isDragEnabled(const QModelIndex &) const
{
    return false;
}

/*!
  Returns true if other items can be dropped on the item associated with \a index;
  otherwise returns false.
*/
bool QAbstractItemModel::isDropEnabled(const QModelIndex &) const
{
    return false;
}

/*!
  Returns true if the items in the model can be sorted; otherwise returns false.

  \sa sort()
*/
bool QAbstractItemModel::isSortable() const
{
    return false;
}

/*!
  Sorts the model if it is sortable.

  \sa isSortable()
*/
void QAbstractItemModel::sort(int, SortOrder)
{
    // do nothing
}

/*!
  Returns true if the data associated with \a left and \a right are equal; otherwise returns false.

  \sa greater()
 */
bool QAbstractItemModel::equal(const QModelIndex &left, const QModelIndex &right) const
{
    return left == right;
}

/*!
  Returns true if the data associated with \a left is greater than the data associated with \a right;
  otherwise returns false.

  \sa equal()
 */
bool QAbstractItemModel::greater(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.row() == right.row())
        return left.column() > right.column();
    return left.row() > right.row();
}

/*!
  Retuns the list of the QModelIndex objects associated with the data items matching \a value in role \a role.
  The search starts from \a start and continues the number of matching data items equals \a hits
  or the search reaches the last row.
 */
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

