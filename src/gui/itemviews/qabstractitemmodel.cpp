/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qabstractitemmodel.h"
#include <qevent.h>
#include <qdragobject.h>
#include <qdatastream.h>
#include <qmap.h>
#include <qmime.h>
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

    QByteArray encoded = src->encodedData(format());
    QDataStream stream(&encoded, IO_ReadOnly);
    int row = model->rowCount(parent);
    int count, role;
    QVariant data;
    QModelIndex index;
    while (!stream.atEnd()) {
        model->insertRows(row, parent); // append row
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

QPersistentModelIndexData *QPersistentModelIndexData::create(const QModelIndex &index,
                                                             QAbstractItemModel *model)
{
    // FIXME: this is slow
    QPersistentModelIndexData *d = &QPersistentModelIndexData::shared_null;
    QList<QPersistentModelIndexData*> *persistentIndexes = &(model->d_func()->persistentIndexes);
    for (int i = 0; i < persistentIndexes->count(); ++i) {
        if (persistentIndexes->at(i)->index == index) {
            d = persistentIndexes->at(i);
            break;
        }
    }
    if (d == &QPersistentModelIndexData::shared_null) {
        d = new QPersistentModelIndexData;
        d->model = model;
        d->index = index;
        persistentIndexes->append(d);
    }
    return d;
}

void QPersistentModelIndexData::destroy(QPersistentModelIndexData *data)
{
    if (data && data != &QPersistentModelIndexData::shared_null) {
        if (data->model)
            data->model->d_func()->persistentIndexes.removeAll(data);
        delete data;
    }
}


/*!
  \class QPersistentModelIndex qabstractitemmodel.h

  \brief The QPersistentModelIndex class is used to locate data in a data model.

  \ingroup model-view

  A QPersistentModelIndex is a model index that can be stored by an
  application, and later used to access information in a model.
  Unlike the QModelIndex class, it is safe to store a
  QPersistentModelIndex since the model will ensure that references
  to data will continue to be valid as long as that data exists within
  the model.

  \sa QModelIndex
      \link model-view-programming.html Model/View Programming\endlink
*/


/*!
  \fn QPersistentModelIndex::QPersistentModelIndex()

  \internal
*/

QPersistentModelIndex::QPersistentModelIndex()
    : d(&QPersistentModelIndexData::shared_null)
{
    ++d->ref;
}

/*!
  \fn QPersistentModelIndex::QPersistentModelIndex(const QPersistentModelIndex &other)

  Creates a new QPersistentModelIndex that is a copy of the \a other persistent  model index.
*/

QPersistentModelIndex::QPersistentModelIndex(const QPersistentModelIndex &other)
    : d(other.d)
{
    ++d->ref;
}

/*!
    Creates a new QPersistentModelIndex that is a copy of the \a index
    model index in the given \a model.
*/

QPersistentModelIndex::QPersistentModelIndex(const QModelIndex &index, QAbstractItemModel *model)
    : d(&QPersistentModelIndexData::shared_null)
{
    if (!index.isValid()) {
        ++d->ref;
        return;
    }
    d = QPersistentModelIndexData::create(index, model);
    ++d->ref;
}

/*!
    \fn QPersistentModelIndex::~QPersistentModelIndex()

    \internal
*/

QPersistentModelIndex::~QPersistentModelIndex()
{
    if (!--d->ref)
        QPersistentModelIndexData::destroy(d);
    d = 0;
}

/*!
  \fn bool QPersistentModelIndex::operator<(const QPersistentModelIndex &other) const

  Returns true if this persistent model index is smaller than the \a other persistent model index; otherwise false.
*/

bool QPersistentModelIndex::operator<(const QPersistentModelIndex &other) const
{
    return d < other.d;
}

/*!
    Sets the persistent model index to refer to the same model index
    as the \a other persistent model index.
*/

void QPersistentModelIndex::operator=(const QPersistentModelIndex &other)
{
    if (!--d->ref)
        QPersistentModelIndexData::destroy(d);
    d = other.d;
    ++d->ref;
}

/*!
  \fn QPersistentModelIndex::operator const QModelIndex&() const

  Cast operator that returns a const QModelIndex&.
*/

QPersistentModelIndex::operator const QModelIndex&() const
{
    return d->index;
}


/*!
    \fn int QPersistentModelIndex::row() const

    Returns the row this persistent model index refers to.
*/

int QPersistentModelIndex::row() const
{
    return d->index.row();
}

/*!
    \fn int QPersistentModelIndex::column() const

    Returns the column this persistent model index refers to.
*/

int QPersistentModelIndex::column() const
{
    return d->index.column();
}

/*!
    \fn void *QPersistentModelIndex::data() const

    \internal

    Returns a \c{void} \c{*} pointer to the data located at this persistent model
    index position.
*/

void *QPersistentModelIndex::data() const
{
    return d->index.data();
}


/*!
    \fn Type QPersistentModelIndex::type() const

    Returns the \c Type of this persistent model index.
*/

QModelIndex::Type QPersistentModelIndex::type() const
{
    return d->index.type();
}


/*!
    \fn bool QPersistentModelIndex::isValid() const

    Returns true if this persistent model index is valid; otherwise returns
    false.
*/

bool QPersistentModelIndex::isValid() const
{
    return d->index.isValid();
}


/*!
    \fn bool QPersistentModelIndex::operator==(const QModelIndex &other) const

    Returns true if this persistent model index refers to the same location as
    the \a other model index; otherwise returns false.
*/

bool QPersistentModelIndex::operator==(const QModelIndex &other) const
{
    return d->index == other;
}

/*!
    \fn bool QPersistentModelIndex::operator!=(const QModelIndex &other) const

    Returns true if this persistent  model index does not refer to the same
    location as the \a other model index; otherwise returns false.
*/

bool QPersistentModelIndex::operator!=(const QModelIndex &other) const
{
    return d->index != other;
}

/*!
    \class QModelIndex qabstractitemmodel.h

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

    \sa \link model-view-programming.html Model/View Programming\endlink
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
    \fn QModelIndex::QModelIndex()

    Creates a new invalid model index.
*/

/*!
    \fn QModelIndex::QModelIndex(int row, int column, void *data, Type type)

    Creates a new model index at the given \a row and \a column,
    pointing to data \a data, and of type \a type.
*/

/*!
    \fn QModelIndex::QModelIndex(const QModelIndex &other)

    Creates a new model index that is a copy of the \a other model
    index.
*/

/*!
    \fn QModelIndex::~QModelIndex()

    Destroys the model index.
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

    The underlying data model is a hierarchy of tables. If you don't
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
    dataChanged(), rowsInserted(), columnsInserted(), rowsRemoved()
    and columnsRemoved().

    \sa \link model-view-programming.html Model/View Programming\endlink
*/

/*!
    \fn QModelIndex QAbstractItemModel::index(int row, int column, const QModelIndex &parent, QModelIndex::Type type) const = 0

    Returns the index of the item in the model specified by the given \a row,
    \a column, \a parent index, and \a type.
*/

/*!
    \fn QModelIndex QAbstractItemModel::parent(const QModelIndex &index) const = 0

    Returns the parent of the model item with the given \a index.
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
    invalidatePersistentIndexes();
}

/*!
    \fn QModelIndex QAbstractItemModel::sibling(int row, int column, const QModelIndex &idx) const

    Returns the index of the sibling of the item at the given \a row,
    \a column, and index \a idx.
*/


/*!
    \fn int QAbstractItemModel::rowCount(const QModelIndex &parent) const = 0

    Returns the number of rows under the given \a parent.
*/


/*!
    \fn int QAbstractItemModel::columnCount(const QModelIndex &parent) const = 0;
    Returns the number of columns for the given \a parent.
*/


/*!
    \fn void QAbstractItemModel::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)

    This signal is emitted whenever the data in an existing item
    changes. The affected items are those between \a topLeft and \a
    bottomRight inclusive.

    \sa rowsInserted() rowsRemoved() columnsInserted() columnsRemoved() setData()
*/

/*!
    \fn void QAbstractItemModel::rowsInserted(const QModelIndex &parent, int start, int end)

    This signal is emitted when rows have been inserted into the
    model. The new items are those between \a start and \a end
    inclusive, under the given \a parent item.

    \sa insertRows()
*/

/*!
    \fn void QAbstractItemModel::rowsRemoved(const QModelIndex &parent, int start, int end)

    This signal is emitted just before rows are removed from the
    model. The removed items are those between \a start and \a end
    inclusive, under the given \a parent item.

    \sa removeRows()
*/

/*!
    \fn void QAbstractItemModel::columnsInserted(const QModelIndex &parent, int start, int end)

    This signal is emitted when columns have been inserted into the
    model. The new items are those between \a start and \a end
    inclusive, under the given \a parent item.

    \sa insertColumns()
*/

/*!
    \fn void QAbstractItemModel::columnsRemoved(const QModelIndex &parent, int start, int end)

    This signal is emitted just before columns are removed
    from the model. The removed items are those between \a start and
    \a end inclusive, under the given \a parent item.

    \sa removeRows()
*/

/*!
    \enum QAbstractItemModel::Role

    Each item in the model has a set of data elements associated with
    it, each with its own role. The roles are used when visualizing
    and editing the items in the views.

    \value DisplayRole The data rendered as text.
    \value DecorationRole The data rendered as an icon.
    \value EditRole The data in a form suitable for editing in an item editor.
    \value ToolTipRole The data displayed in the item's tooltip.
    \value StatusTipRole The data displayed in the status bar.
    \value WhatsThisRole The data displayed for the item in "What's This?"
    mode.
    \value UserRole The first custom role defined by the user.
*/

/*!
    Returns true if \a parent has any children; otherwise returns false.

    \sa parent() index()
*/
bool QAbstractItemModel::hasChildren(const QModelIndex &parent) const
{
    return (rowCount(parent) > 0) && (columnCount(parent) > 0);
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
    for (int i = 0; i < UserRole; ++i) {
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
    \fn QVariant QAbstractItemModel::data(const QModelIndex &index, int role) const = 0

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
  Inserts \a count rows in the model before position \a row. If \a row is 0 the
  rows are prepended to the model, if \a row is rowCount() the rows are
  appended to the model. The row will be a child of \a parent.
  If \a parent has no children \a count rows with at least one column is inserted.
  Returns true if the rows were successfully inserted; otherwise returns
  false.

  The base class implementation does nothing and returns false. If
  you want to be able to insert rows you must reimplement this
  function.
*/
bool QAbstractItemModel::insertRows(int, const QModelIndex &, int)
{
    return false;
}

/*!
  Inserts \a count new columns in the model before position \a column. If \a
  column is 0 the columns are prepended to the model, if \a column is
  columnCount() the columns are appended to the model. The column will
  be a child of \a parent.   If \a parent has no children \a count columns with at least one row is inserted.
  Returns true if the columns were successfully inserted; otherwise
  returns false.

  The base class implementation does nothing and returns false. If
  you want to be able to insert columns you must reimplement this
  function.
*/
bool QAbstractItemModel::insertColumns(int, const QModelIndex &, int)
{
    return false;
}

/*!
    Removes \a count rows starting with row \a row under parent \a
    parent from the model. Returns true if the rows were successfully
    removed; otherwise returns false.

    The base class implementation does nothing and returns false.
*/
bool QAbstractItemModel::removeRows(int, const QModelIndex &, int)
{
    return false;
}

/*!
    Removes \a count columns starting with column \a column under parent
    \a parent from the model.  Returns true if the columns were
    successfully removed; otherwise returns false.

    The base class implementation does nothing and returns false.
*/
bool QAbstractItemModel::removeColumns(int, const QModelIndex &, int)
{
    return false;
}

/*!
    Returns true if the item at \a index is selectable; otherwise
    returns false.

    The base class implementation returns true.
*/
bool QAbstractItemModel::isSelectable(const QModelIndex &) const
{
    return true;
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
void QAbstractItemModel::sort(int, Qt::SortOrder)
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
  Returns the 'buddy' of the item represented by \a index.
  When the used wants to edit an item that is not editable,
  the delegate may ask for the item's buddy, and edit that
  item instead.

  \sa isEditable()
*/
QModelIndex QAbstractItemModel::buddy(const QModelIndex &) const
{
    return QModelIndex();
}

/*!
    \enum QAbstractItemModel::MatchFlag

    \value MatchContains The value is contained in the item.
    \value MatchFromStart The value matches the start of the item.
    \value MatchFromEnd The value matches the end of the item.
    \value MatchExactly The value matches the item exactly.
    \value MatchCase The search is case sensitive.
    \value MatchWrap The search wraps around.
    \value MatchDefault The default match, which is MatchFromStart|MatchWrap.
*/

/*!
    Retuns a list of indexes for the items matching \a value in role
    \a role based on the \a flags. The list may be empty. The search
    starts from index \a start and continues until the number of
    matching data items equals \a hits or the search reaches the last
    row or \a start, depending on whether \c MatchWrap is specified
    in \a flags or not.

    \sa QAbstractItemModel::MatchFlag
*/
QModelIndexList QAbstractItemModel::match(const QModelIndex &start, int role,
                                          const QVariant &value, int hits,
                                          MatchFlags flags) const
{
    QString val = value.toString();

    if (!(flags & MatchCase))
        val = val.toLower();

    QModelIndexList result;
    QModelIndex idx;
    QModelIndex par = parent(start);
    QString itemText;
    int col = start.column();
    int matchType = flags & MatchExactly;
    int rc = rowCount(par);

    // iterates twice if wrapping
    for (int i = 0; i < 2 && result.count() < hits; ++i) {
        if (!(flags & MatchWrap) && i == 1)
            break;
        int rowStart = (i == 0) ? start.row() : 0;
        int rowEnd = (i == 0) ? rc : start.row();

        for (int row = rowStart; row < rowEnd && result.count() < hits; ++row) {
            idx = index(row, col, par);
            itemText = data(idx, role).toString();
            if (!(flags & MatchCase))
                itemText = itemText.toLower();

            switch (matchType) {
            case MatchExactly:
                if (itemText == val)
                    result.append(idx);
                break;
            case MatchFromStart:
                if (itemText.startsWith(val))
                    result.append(idx);
                break;
            case MatchFromEnd:
                if (itemText.endsWith(val))
                    result.append(idx);
                break;
            case MatchContains:
            default:
                if (itemText.contains(val))
                    result.append(idx);
            }
        }
    }
    return result;
}

/*!
    \fn QModelIndex QAbstractItemModel::createIndex(int row, int column, void *data, QModelIndex::Type type) const

    \internal

    Creates a model index for the given \a row and \c column that
    points to the given \a data and is of the given \a type.
*/


/*!
  \internal

  Returns true if \a row and \a column is valid in the child table \a parent,
  otherwise returns false.
*/
bool QAbstractItemModel::isValid(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0)
        return false;
    return row < rowCount(parent) && column < columnCount(parent);
}


/*!
    \internal

    Invalidates the persistent indices by setting them to invalid
    model indexes. Affects the given \a parent index, or if the \a
    parent is invalid affects all indexes.
*/
void QAbstractItemModel::invalidatePersistentIndexes(const QModelIndex &parent)
{
    bool all = !parent.isValid();
    for (int i = 0; i < d->persistentIndexes.count(); ++i) {
        if (all || this->parent(d->persistentIndexes.at(i)->index) == parent) {
            d->persistentIndexes[i]->index = QModelIndex();
            d->persistentIndexes[i]->model = 0;
        }
    }
}

/*!
    \internal

    Returns the number of persistent indexes held by this model.
*/
int QAbstractItemModel::persistentIndexesCount() const
{
    return d->persistentIndexes.count();
}

/*!
    \internal

    Returns the persistent index at the given \a position.
*/
QModelIndex QAbstractItemModel::persistentIndexAt(int position) const
{
    return d->persistentIndexes.at(position)->index;
}

/*!
    \internal

    Sets the model's persistent index at position \a position to \a
    index.
*/
void QAbstractItemModel::setPersistentIndex(int position, const QModelIndex &index)
{
    d->persistentIndexes[position]->index = index;
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

/*!
    \class QAbstractTableModel
    \brief The QAbstractTableModel class provides a default model to use with
    the QGenericTableView class.

    \ingroup model-view

    A QAbstractTableModel is a ready-made model for use with the generic view
    widgets. In particular, it is intended for use with QGenericTableView.
    The model provides a two-dimensional model in which data can be stored,
    containing a number of rows and columns.

    If you need to represent a simple list of items, and only need a model to
    contain a single column of data, the QAbstractListModel may be more
    appropriate. For hierarchical lists, for use with QGenericTreeView, it is
    necessary to subclass QAbstractItemModel.

    The rowCount() and columnCount() functions return the dimensions of the
    table. To retrieve a model index corresponding to an item in the mode, use
    index() and provide only the row and column numbers.

    \sa \link model-view-programming.html Model/View Programming\endlink
        QAbstractItemModel QAbstractListModel

*/

/*!
    \fn int QAbstractTableModel::rowCount() const = 0

    Returns the number of rows in the model.
*/

/*!
    \fn int QAbstractTableModel::columnCount() const = 0

    Returns the number of columns in the model.
*/

/*!
    Constructs an abstract table model for the given \a parent.
*/

QAbstractTableModel::QAbstractTableModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

/*!
    \internal

    Constructs an abstract table model with \a dd and the given \a parent.
*/

QAbstractTableModel::QAbstractTableModel(QAbstractItemModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{

}

/*!
    Destroys the abstract table model.
*/

QAbstractTableModel::~QAbstractTableModel()
{

}

/*!
    \fn QModelIndex QAbstractTableModel::index(int row, int column, const QModelIndex &parent = QModelIndex(), QModelIndex::Type type = QModelIndex::View) const

    Returns the index of the data in \a row and \a column with \a
    parent, of the given \a type.

    \sa parent()
*/

QModelIndex QAbstractTableModel::index(int row, int column, const QModelIndex &parent,
                                     QModelIndex::Type type) const
{
    return isValid(row, column, parent) ? createIndex(row, column, 0, type) : QModelIndex();
}

/*!
    \fn QModelIndex QAbstractTableModel::parent(const QModelIndex &index) const

    Returns the parent of the model item with the given \a index.

    \sa index() hasChildren()
*/

QModelIndex QAbstractTableModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

/*!
    \internal

    Returns the number of rows in the table with the given \a parent.

    \sa columnCount()
*/

int QAbstractTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return rowCount();
}

/*!
    \internal

    Returns the number of columns in the table with the given \a parent.

    \sa rowCount()
*/

int QAbstractTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return columnCount();
}

/*!
    \class QAbstractListModel
    \brief The QAbstractListModel class provides a default model to use with the
    QGenericListView class.

    \ingroup model-view

    A QAbstractListModel is used to store a list of items, typically for
    display in a list view. Since the model represents a simple non-hierarchical
    sequence of items, it is not suitable for use with a tree view; you will
    need to subclass QAbstractItemModel for that purpose.

    If you need to use a number of list models to manage data, it may be more
    appropriate to use the QAbstractTableModel class.

    QAbstractListModel is not used directly, but must be subclassed. For example,
    we can implement a simple QStringList-based model that provides a list of
    strings to a QGenericListView widget. In such a case, we only need to
    implement the rowCount() function to return the number of items in the list,
    and the data() function to retrieve items from the list.


    Since the model represents a one-dimensional structure, the rowCount()
    function returns the number of items in the model. The columnCount() function
    is implemented for interoperability with all kinds of generic views, but
    by default informs views that the model contains only one column.

    \sa \link model-view-programming.html Model/View Programming\endlink
        QAbstractItemView QAbstractTableView

*/

/*!
    \fn int QAbstractListModel::rowCount() const

    Returns the number of rows in the model.
    The number of rows is equal to the number of items stored in the model.
*/

/*!
    \fn int QAbstractListModel::columnCount() const

    Returns the number of columns in the model.
    This list model only contains one column of items.
*/

/*!
    Constructs an abstract list model with the given \a parent.
*/

QAbstractListModel::QAbstractListModel(QObject *parent)
    : QAbstractItemModel(parent)
{

}

/*!
    \internal

    Constructs an abstract list model with \a dd and the given \a parent.
*/

QAbstractListModel::QAbstractListModel(QAbstractItemModelPrivate &dd, QObject *parent)
    : QAbstractItemModel(dd, parent)
{

}

/*!
    Destroys the abstract list model.
*/

QAbstractListModel::~QAbstractListModel()
{

}

/*!
    \fn QModelIndex QAbstractListModel::index(int row, int column, const QModelIndex &parent = QModelIndex(), QModelIndex::Type type = QModelIndex::View) const

    Returns the index of the data in \a row and \a column with \a
    parent, of the given \a type.

    \sa parent()
*/

QModelIndex QAbstractListModel::index(int row, int column, const QModelIndex &parent,
                                     QModelIndex::Type type) const
{
    return isValid(row, column, parent) ? createIndex(row, column, 0, type) : QModelIndex();
}

/*!
    \fn QModelIndex QAbstractListModel::parent(const QModelIndex &index) const

    Returns the parent of the model item with the given \a index.

    \sa index() hasChildren()
*/

QModelIndex QAbstractListModel::parent(const QModelIndex &) const
{
    return QModelIndex();
}

/*!
    \internal

    Returns the number of rows in the table with the given \a parent.

    \sa columnCount()
*/

int QAbstractListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return rowCount();
}

/*!
    \internal

    Returns the number of columns in the table with the given \a parent.

    \sa rowCount()
*/

int QAbstractListModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return columnCount();
}
