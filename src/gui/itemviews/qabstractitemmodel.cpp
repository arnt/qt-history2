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
#include <qatomic.h>
#include <qdebug.h>
#include <private/qabstractitemmodel_p.h>

class QAbstractItemModelDrag : public QDragObject
{
public:
    QAbstractItemModelDrag(const QModelIndexList &indices, QAbstractItemModel *model,
                           QWidget *dragSource);
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

QPersistentModelIndexData QPersistentModelIndexData::shared_null;

QPersistentModelIndex::QPersistentModelIndex()
    : d(&QPersistentModelIndexData::shared_null)
{
    ++d->ref;
}

QPersistentModelIndex::QPersistentModelIndex(const QPersistentModelIndex &other)
    : d(other.d)
{
    ++d->ref;
}

QPersistentModelIndex::QPersistentModelIndex(const QModelIndex &index, QAbstractItemModel *model)
    : d(&QPersistentModelIndexData::shared_null)
{
    if (!index.isValid()) {
        ++d->ref;
        return;
    }
    // FIXME: this is slow
    QList<QPersistentModelIndexData*> *persistentIndices = &(model->d_func()->persistentIndices);
    for (int i = 0; i < persistentIndices->count(); ++i) {
        if (persistentIndices->at(i)->index == index) {
            d = persistentIndices->at(i);
            break;
        }
    }
    if (d == &QPersistentModelIndexData::shared_null) {
        d = new QPersistentModelIndexData;
        d->model = model;
        d->index = index;
        persistentIndices->append(d);
    }
    ++d->ref;
}

QPersistentModelIndex::~QPersistentModelIndex()
{
    if (!--d->ref && d != &QPersistentModelIndexData::shared_null) {
        d->model->d_func()->persistentIndices.removeAll(d);
        delete d;
    }
    d = 0;
}

bool QPersistentModelIndex::operator<(const QPersistentModelIndex &other) const
{
    return d < other.d;
}

void QPersistentModelIndex::operator=(const QPersistentModelIndex &other)
{
    if (!--d->ref && d != &QPersistentModelIndexData::shared_null) {
        d->model->d_func()->persistentIndices.removeAll(d);
        delete d;
    }
    d = other.d;
    ++d->ref;
}

QPersistentModelIndex::operator const QModelIndex&() const
{
    return d->index;
}

int QPersistentModelIndex::row() const
{
    return d->index.row();
}

int QPersistentModelIndex::column() const
{
    return d->index.column();
}

void *QPersistentModelIndex::data() const
{
    return d->index.data();
}

bool QPersistentModelIndex::isValid() const
{
    return d->index.isValid();
}

bool QPersistentModelIndex::operator==(const QModelIndex &other) const
{
    return d->index == other;
}

bool QPersistentModelIndex::operator!=(const QModelIndex &other) const
{
    return d->index != other;
}

/*!
    \class QModelIndex qgenericitemmodel.h

    \brief The QModelIndex class is used to locate data in a data model.

    \ingroup model-view

    This class is used as an index into QAbstractItemModel derived
    data models. The index is used by item views, delegates and
    selection models to locate an item in the model. QModelIndex
    objects are created by the model.

    A model index has a row(), a column(), and a type().
    \omit
    It can also provide a \c{void} \c{*} pointer to the data() located
    at the index position.
    \endomit
*/

/*!
    \enum QModelIndex::Type

    A model index locates an item in a view or in a view's horizontal
    or vertical header.

    \value View
    \value HorizontalHeader
    \value VerticalHeader
*/

/*!
    \fn QModelIndex::QModelIndex(int row, int column, void *data, Type type)

    \internal

    Creates a new model index at the given \a row and \a column,
    pointing to data \a data, and of type \a type.
*/

/*!
    \fn QModelIndex::QModelIndex(const QModelIndex &other)

    \internal

    Creates a new model index that is a copy of the \a other model
    index.
*/

/*!
    \fn QModelIndex::~QModelIndex()

    \internal
*/

/*!
    \fn int QModelIndex::row() const

    Returns the row this model index refers to.
*/


/*!
    \fn int QModelIndex::column() const

    Returns the column this model index refers to.
*/


/*!
    \fn void *QModelIndex::data() const

    \internal

    Returns a \c{void} \c{*} pointer to the data located at this model
    index position.
*/


/*!
    \fn Type QModelIndex::type() const

    Returns the \c Type of this model index.
*/


/*!
    \fn bool QModelIndex::isValid() const

    Returns true if this model index is valid; otherwise returns
    false.
*/


/*!
    \fn bool QModelIndex::operator==(const QModelIndex &other) const

    Returns true if this model index refers to the same location as
    the \a other model index; otherwise returns false.
*/


/*!
    \fn bool QModelIndex::operator!=(const QModelIndex &other) const

    Returns true if this model index does not refer to the same
    location as the \a other model index; otherwise returns false.
*/


/*!
    \class QAbstractItemModel qabstractitemmodel.h

    \brief The QAbstractItemModel class provides an abstract model of
    a set of arbitrary data.

    \ingroup model-view

    The underlying data model is a hierarchy of rows. If you don't
    make use of the hierarchy, then the model is a simple table of
    rows and columns. Each item has a unique index specified by a
    QModelIndex.

    Every item has an index(), and possibly a sibling() index; child
    items have a parent() index. hasChildren() is true for items that
    have children. Each item has a number of data elements associated
    with them, each with a particular \c Role. Data elements are set
    individually with setData(), or for all roles with setItemData().
    Data is retrieved with data() (for an element with a given role),
    or with itemData() (for every role's element data). Items can be
    queried with isSelectable() and isEditable(). An item can be
    searched for using match().

    To customize sorting and searching, comparison functions can be
    reimplemented, for example, lessThan(), equal(), and
    greaterThan().

    The model has a rowCount() and a columnCount() for each level of
    the hierarchy, and indexes for the topLeft() and bottomRight().
    Sometimes it is difficult to determine the size of a model (for
    example, if it is being delivered via a data stream), in which
    case it may be necessary to implement fetchMore(). If the model
    isSortable(), it can be sorted with sort().

    Rows and columns can be inserted and removed with insertRow(),
    insertColumn(), removeRow(), and removeColumn().

    The model emits signals to indicate changes, for example,
    contentsChanged(), contentsInserted(), and contentsRemoved().

*/

#define d d_func()
#define q q_func()

/*!
    Constructs an abstract item model with parent \a parent.
*/
QAbstractItemModel::QAbstractItemModel(QObject *parent)
    : QObject(*new QAbstractItemModelPrivate, parent)
{
}

/*!
  \internal
*/
QAbstractItemModel::QAbstractItemModel(QAbstractItemModelPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
}

/*!
    Destroys the abstract item model.
*/
QAbstractItemModel::~QAbstractItemModel()
{
    invalidatePersistentIndices();
}

/*!
    \fn QModelIndex QAbstractItemModel::topLeft(const QModelIndex &parent = 0) const

    Returns the index of the top left item for the given \a parent.
*/


/*!
    \fn QModelIndex QAbstractItemModel::bottomRight(const QModelIndex &parent = 0) const

    Returns the index of the bottom right item for the given \a parent.
*/


/*!
    \fn QModelIndex QAbstractItemModel::sibling(int row, int column, const QModelIndex &idx) const

    Returns the index of the sibling of the item at the given \a row,
    \a column, and index \a idx.
*/


/*!
    \fn int QAbstractItemModel::rowCount(const QModelIndex &parent = 0) const = 0

    Returns the number of rows under the given \a parent.
*/


/*!
    \fn int QAbstractItemModel::columnCount(const QModelIndex &parent = 0) const = 0;
    Returns the number of columns for the given \a parent.
*/


/*!
    \fn void QAbstractItemModel::contentsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)

    This signal is emitted whenever the data in an existing item
    changes. The affected items are those between \a topLeft and \a
    bottomRight inclusive.

    \sa contentsInserted() contentsRemoved() setData()
*/

/*!
    \fn void QAbstractItemModel::contentsInserted(const QModelIndex &topLeft, const QModelIndex &bottomRight)

    This signal is emitted when rows or columns are inserted into the
    model. The new items are those between \a topLeft and \a
    bottomRight inclusive.

    \sa insertRow() insertColumn()
*/

/*!
    \fn void QAbstractItemModel::contentsRemoved(const QModelIndex &topLeft, const QModelIndex &bottomRight)

    This signal is emitted just before rows or columns are removed
    from the model. The removed items are those between \a topLeft and
    \a bottomRight inclusive.

    \sa removeRow() removeColumn()
*/

/*!
    \enum QAbstractItemModel::Role

    Each item in the model has a set of data elements associated with
    it, each with its own role. The roles are used when visualizing
    and editing the items in the views.

    \value Display The data rendered as text.
    \value Decoration The data rendered as an icon.
    \value Edit The data in a form suitable for editing in an item editor.
    \value ToolTip The data displayed in the item's tooltip.
    \value StatusTip The data displayed in the status bar.
    \value WhatsThis The data displayed for the item in "What's This?"
    mode.
    \value User The first custom role defined by the user.
*/

/*!
    Returns the index of the data in \a row and \a column with \a
    parent, of type \a type.

    \sa parent()
 */
QModelIndex QAbstractItemModel::index(int row, int column, const QModelIndex &parent,
                                     QModelIndex::Type type) const
{
    if (row >= 0 && row < rowCount(parent) && column >= 0 && column < columnCount(parent))
        return QModelIndex(row, column, 0, type);
    return QModelIndex();
}

/*!
    Returns the index of the parent of \a child.

    \sa index() hasChildren()
*/
QModelIndex QAbstractItemModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

/*!
    Returns true if \a parent has any children; otherwise returns false.

    \sa parent() index()
*/
bool QAbstractItemModel::hasChildren(const QModelIndex &parent) const
{
    return (rowCount(parent) > 0) && (columnCount(parent) > 0);
}

// ### DOC: write something or make it \internal!
void QAbstractItemModel::fetchMore()
{
    // do nothing
}

/*!
    Returns true if decode() would be able to decode \a src; otherwise
    returns false.
*/
bool QAbstractItemModel::canDecode(QMimeSource *src) const
{
    return QAbstractItemModelDrag::canDecode(src);
}

/*!
    Decodes data from \a e, inserting the data under \a parent (if
    possible).

    Returns true if the data was successfully decoded and inserted;
    otherwise returns false.
*/
bool QAbstractItemModel::decode(QDropEvent *e, const QModelIndex &parent)
{
    return QAbstractItemModelDrag::decode(e, this, parent);
}

/*!
    Returns a pointer to a QDragObject object containing the data
    associated with the \a indices from the \a dragSource.

    \sa itemData()
*/
QDragObject *QAbstractItemModel::dragObject(const QModelIndexList &indices, QWidget *dragSource)
{
    return new QAbstractItemModelDrag(indices, this, dragSource);
}

/*!
    Returns a map with values for all predefined roles in the model
    for the item at the given \a index.

    This must be reimplemented if you want to extend the model with
    customized roles.

    \sa Role data()
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
    \fn bool QAbstractItemModel::setData(const QModelIndex &index, const QVariant &value)

    \overload

    Sets the \c Edit role data for the item at \a index to \a value.
    Returns true if successful; otherwise returns false.

    \sa data() itemData()
*/

/*!
    Sets the \a role data for the item at \a index to \a value.
    Returns true if successful; otherwise returns false.

    The base class implementation returns false. This function (and
    data()) must be reimplemented.

    \sa data() itemData()
*/
bool QAbstractItemModel::setData(const QModelIndex &, int, const QVariant &)
{
    return false;
}

/*!
    \fn QVariant QAbstractItemModel::data(const QModelIndex &index, int role = Display) const = 0

    Returns the data of role \a role for the item at \a index.
*/

/*!
    For every \c Role in \a roles, sets the role data for the item at
    \a index to the associated value in \a roles. Returns true if
    successful; otherwise returns false.

    \sa setData() data() itemData()
*/
bool QAbstractItemModel::setItemData(const QModelIndex &index, const QMap<int, QVariant> &roles)
{
    bool b = true;
    for (QMap<int, QVariant>::ConstIterator it = roles.begin(); it != roles.end(); ++it)
        b = b && setData(index, it.key(), it.value());
    return b;
}

/*!
  Inserts \a count rows in the model at position \a row. If \a row is 0 the
  rows are prepended to the model, if \a row is rowCount() the rows are
  appended to the model. The row will be a child of \a parent.
  If \a parent has no children \a count rows with at least one column is inserted.
  Returns true if the rows were successfully inserted; otherwise returns
  false.

  The base class implementation does nothing and returns false. If
  you want to be able to insert rows you must reimplement this
  function.
*/
bool QAbstractItemModel::insertRow(int, const QModelIndex &, int)
{
    return false;
}

/*!
  Inserts \a count new columns in the model at position \a column. If \a
  column is 0 the columns are prepended to the model, if \a column is
  columnCount() the columns are appended to the model. The column will
  be a child of \a parent.   If \a parent has no children \a count columns with at least one row is inserted.
  Returns true if the columns were successfully inserted; otherwise
  returns false.

  The base class implementation does nothing and returns false. If
  you want to be able to insert columns you must reimplement this
  function.
*/
bool QAbstractItemModel::insertColumn(int, const QModelIndex &, int)
{
    return false;
}

/*!
    Removes \a count rows starting with row \a row under parent \a
    parent from the model. Returns true if the rows were successfully
    removed; otherwise returns false.

    The base class implementation does nothing and returns false.
*/
bool QAbstractItemModel::removeRow(int, const QModelIndex &, int)
{
    return false;
}

/*!
    Removes \a count columns starting with column \a column under parent
    \a parent from the model.  Returns true if the columns were
    successfully removed; otherwise returns false.

    The base class implementation does nothing and returns false.
*/
bool QAbstractItemModel::removeColumn(int, const QModelIndex &, int)
{
    return false;
}

/*!
    Returns true if the item at \a index is selectable; otherwise
    returns false.

    The base class implementation returns false.
*/
bool QAbstractItemModel::isSelectable(const QModelIndex &) const
{
    return false;
}

/*!
    Returns true if the item at \a index is editable; otherwise
    returns false.

    The base class implementation returns false.
*/
bool QAbstractItemModel::isEditable(const QModelIndex &) const
{
    return false;
}

/*!
    Returns true if the item at \a index can be dragged; otherwise
    returns false.

    The base class implementation returns false.
*/
bool QAbstractItemModel::isDragEnabled(const QModelIndex &) const
{
    return false;
}

/*!
    Returns true if other items can be dropped on the item at \a
    index; otherwise returns false.

    The base class implementation returns false.
*/
bool QAbstractItemModel::isDropEnabled(const QModelIndex &) const
{
    return false;
}

/*!
    Returns true if the items in the model can be sorted; otherwise
    returns false.

    The base class implementation returns false.

    \sa sort()
*/
bool QAbstractItemModel::isSortable() const
{
    return false;
}

/*!
    Sorts the model, if it is sortable, by column \a column in the
    given \a order.

    The base class implementation does nothing.

    \sa isSortable()
*/
void QAbstractItemModel::sort(int, SortOrder)
{
    // do nothing
}

/*!
    Returns true if the data at indexes \a left and \a right are
    equal; otherwise returns false.

    \sa greaterThan() lessThan()
*/
bool QAbstractItemModel::equal(const QModelIndex &left, const QModelIndex &right) const
{
    return left == right;
}

/*!
    \fn bool QAbstractItemModel::greaterThan(const QModelIndex &left, const QModelIndex &right) const

    Returns true if the data at index \a left is greater than the
    data at index \a right; otherwise returns false.

    \sa equal() lessThan()
*/

/*!
    Returns true if the data at index \a left is less than the
    data at index \a right; otherwise returns false.

    \sa equal() greaterThan()
*/
bool QAbstractItemModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    if (left.row() == right.row())
        return left.column() < right.column();
    return left.row() < right.row();
}

/*!
    Retuns a list of indexes for the items matching \a value in role
    \a role. The list may be empty. The search starts from index \a
    start and continues until the number of matching data items equals
    \a hits or the search reaches the last row or \a start, depending
    on whether \a wrap is true (search from \a start and work
    forwards), or false (search from \a start and work backwards).
*/
QModelIndexList QAbstractItemModel::match(const QModelIndex &start, int role,
                                          const QVariant &value, int hits, bool wrap) const
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
    if (wrap) {
        for (int row = 0; row < start.row() && hit < hits; ++row) {
            idx = index(row, col, par);
            if (data(idx, role).toString().toLower().startsWith(val)) {
                result.append(idx);
                ++hit;
            }
        }
    }
    return result;
}

void QAbstractItemModel::invalidatePersistentIndices(const QModelIndex &parent)
{
    bool all = !parent.isValid();
    for (int i = 0; i < d->persistentIndices.count(); ++i)
        if (all || this->parent(d->persistentIndices.at(i)->index) == parent)
            d->persistentIndices[i]->index = QModelIndex();
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

