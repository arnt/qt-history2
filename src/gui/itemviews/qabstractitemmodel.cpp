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
        if (data->model) {
            data->model->d_func()->persistentIndexes.removeAll(data);
        }
        delete data;
    }
}

QString QAbstractItemModelPrivate::i2s(QChar *buf, int size, int num)
{
    static ushort unicode_zero = QChar('0').unicode();
    static ushort unicode_dash = QChar('-').unicode();
    bool neg = num < 0;
    int len = 0;
    while (num != 0 && size > 0) {
        buf[--size] = unicode_zero + (num % 10);
        num /= 10;
        ++len;
    }
    if (len == 0) {
        buf[--size] = unicode_zero;
        ++len;
    }
    if (neg) {
        buf[--size] = unicode_dash;
        ++len;
    }
    return QString(&buf[size], len);
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

  It is good practice to check that persistent model indexes are valid
  before using them.

  \sa \link model-view-programming.html Model/View Programming\endlink QModelIndex QAbstractItemModel
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

  Creates a new QPersistentModelIndex that is a copy of the \a other persistent
  model index.
*/

QPersistentModelIndex::QPersistentModelIndex(const QPersistentModelIndex &other)
    : d(other.d)
{
    ++d->ref;
}

/*!
    Creates a new QPersistentModelIndex that is a copy of the model \a index
    in the given \a model.
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
  \fn bool QPersistentModelIndex::operator==(const QPersistentModelIndex &other) const

  Returns true if this persistent model index is equal to the \a other
  persistent model index, otherwist returns false.
*/

bool QPersistentModelIndex::operator==(const QPersistentModelIndex &other) const
{
    return d->index == other.d->index;
}

/*!
  \fn bool QPersistentModelIndex::operator<(const QPersistentModelIndex &other) const

  Returns true if this persistent model index is smaller than the \a other
  persistent model index; otherwise returns false.
*/

bool QPersistentModelIndex::operator<(const QPersistentModelIndex &other) const
{
    return d < other.d;
}

/*!
    Sets the persistent model index to refer to the same item in a model
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

    Returns true if this persistent model index does not refer to the same
    location as the \a other model index; otherwise returns false.
*/

bool QPersistentModelIndex::operator!=(const QModelIndex &other) const
{
    return d->index != other;
}

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QModelIndex &idx)
{
#ifndef Q_NO_STREAMING_DEBUG
    dbg.nospace() << "QModelIndex(" << idx.row() << "," << idx.column() << "," << idx.data() << ")";
    return dbg.space();
#else
    qWarning("This compiler doesn't support the streaming of QDebug");
    return dbg;
    Q_UNUSED(idx);
#endif
}

QDebug operator<<(QDebug dbg, const QPersistentModelIndex &idx)
{
    dbg << idx.d->index;
    return dbg;
}
#endif

/*!
    \class QModelIndex qabstractitemmodel.h

    \brief The QModelIndex class is used to locate data in a data model.

    \ingroup model-view
    \mainclass

    This class is used as an index into item models derived from
    QAbstractItemModel. The index is used by item views, delegates, and
    selection models to locate an item in the model. QModelIndex objects are
    created by the model.

    An invalid model index can be constructed with the zero argument form of
    the QModelIndex() constructor. This is useful when referring to top-level
    items in a model.

    Model indexes can become invalid over time so they should be used
    immediately and then discarded. If you need to keep a model index
    over time use a QPersistentModelIndex.

    A model index has a row() and a column()

    \sa \link model-view-programming.html Model/View Programming\endlink QPersistentModelIndex QAbstractItemModel
*/

/*!
    \enum QModelIndex::SpecialValue

    This enum describes a special type of model index:

    \value Null A model index to represent the parent of top-level table items.
*/

/*!
    \fn QModelIndex::QModelIndex(SpecialValue type = Null)

    Creates a new empty model index of the given \a type. By default, a
    \c Null model index is created. This type of model index is used as the
    parent index for top-level items in a model.

    \sa QAbstractItemModel
*/

/*!
    \fn QModelIndex::QModelIndex(int row, int column, void *data)

    \internal

    Creates a new model index at the given \a row and \a column,
    pointing to some \a data.
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

    \brief The QAbstractItemModel class provides the abstract interface for
    item model classes.

    \ingroup model-view

    The QAbstractItemModel class defines the standard interface that
    item models must use to be able to interoperate with other
    components in the model/view architecture. It is not supposed to
    be instantiated directly. Instead, you should subclass it to create
    new models.

    If you need a model to use with a QListView or a QTableView, you
    should consider subclassing QAbstractListModel or QAbstractTableModel
    instead of this class.

    The underlying data model is exposed to views and delegates as
    a hierarchy of tables. If you don't make use of the hierarchy,
    then the model is a simple table of rows and columns. Each item
    has a unique index specified by a QModelIndex.

    \img modelindex-no-parent.png

    Every item has an index(), and possibly a sibling() index; child
    items have a parent() index. hasChildren() is true for items that
    have children. Each item has a number of data elements associated
    with them, each with a particular \c Role. Data elements are set
    individually with setData(), or for all roles with setItemData().
    Data is retrieved from an item with data() (for a single role),
    or with itemData() (for every role). Items can be queried with
    isSelectable() and isEditable(). An item can be searched for using
    match().

    The model has a rowCount() and a columnCount() for each level of
    the hierarchy.
    Rows and columns can be inserted and removed with insertRow(),
    insertColumn(), removeRow(), and removeColumn().

    The model emits signals to indicate changes. For example,
    dataChanged() is emitted whenever the contents of the model are
    changed; rowsInserted(), columnsInserted(), rowsRemoved(),
    and columnsRemoved() are emitted when the model's dimensions
    are changed.

    If the model isSortable(), it can be sorted with sort(). To
    customize sorting and searching, comparison functions can be
    reimplemented; for example, lessThan(), equal(), and
    greaterThan().

    When subclassing QAbstractItemModel, at the very least you must
    implement index(), parent(), rowCount(), columnCount(), and data().
    To enable editing in your model, you must also implement isEditable()
    and setData().

    \sa \link model-view-programming.html Model/View Programming\endlink QModelIndex QAbstractItemView

*/

/*!
    \fn QModelIndex QAbstractItemModel::index(int row, int column, const QModelIndex &parent) const = 0

    Returns the index of the item in the model specified by the given \a row,
    \a column and \a parent index.
*/

/*!
    \fn QModelIndex QAbstractItemModel::parent(const QModelIndex &index) const = 0

    Returns the parent of the model item with the given \a index.
*/

/*!
    \fn bool QAbstractItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value)

    \overload

    Sets the title for the \a section in the header with the given
    \a orientation to the \a value specified.

    \sa headerData()
*/

/*!
    \fn void QAbstractItemModel::headerDataChanged(Qt::Orientation orientation, int first, int last)

    This signal is emitted whenever a header is changed. The \a orientation
    indicates whether the horizontal or vertical header has changed. The
    sections in the header from the \a first to the \a last need to be updated.

    \sa headerData() setHeaderData()
*/

#define d d_func()
#define q q_func()

/*!
    Constructs an abstract item model with the given \a parent.
*/
QAbstractItemModel::QAbstractItemModel(QObject *parent)
    : QObject(*new QAbstractItemModelPrivate, parent)
{
    QObject::connect(this, SIGNAL(reset()), this, SLOT(resetPersistentIndexes()));
}

/*!
  \internal
*/
QAbstractItemModel::QAbstractItemModel(QAbstractItemModelPrivate &dd, QObject *parent)
    : QObject(dd, parent)
{
    QObject::connect(this, SIGNAL(reset()), this, SLOT(resetPersistentIndexes()));
}

/*!
    Destroys the abstract item model.
*/
QAbstractItemModel::~QAbstractItemModel()
{
    for (int i = 0; i < d->persistentIndexes.count(); ++i) {
        Q_ASSERT(d->persistentIndexes.at(i) != &QPersistentModelIndexData::shared_null);
        d->persistentIndexes.at(i)->index = QModelIndex::Null;
        d->persistentIndexes.at(i)->model = 0;
    }
}

/*!
    \fn QModelIndex QAbstractItemModel::sibling(int row, int column, const QModelIndex &index) const

    Returns the model index of the sibling of the item specified by the given
    \a row, \a column, and \a index.
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
    \fn void QAbstractItemModel::reset()

    \internal
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

    This signal is emitted after rows have been inserted into the
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

    This signal is emitted after columns have been inserted into the
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
    it, each with its own role. The roles are used by the view to indicate
    to the model which type of data it needs.

    The general purpose roles are:

    \value DisplayRole    The data to be rendered as text.
    \value DecorationRole The data to be rendered as an icon.
    \value EditRole       The data in a form suitable for editing in an
                          editor.
    \value ToolTipRole    The data displayed in the item's tooltip.
    \value StatusTipRole  The data displayed in the status bar.
    \value WhatsThisRole  The data displayed for the item in "What's This?"
                          mode.

    Roles describing appearance and meta data:

    \value FontRole            The font used for items rendered with the default
                               delegate.
    \value BackgroundColorRole The background color used for items rendered with
                               the default delegate.
    \value TextColorRole       The text color used for items rendered with
                               the default delegate.
    \value CheckStateRole      Whether the item can be checked.

    Accessibility roles:

    \value AccessibleTextRole        The text to be used by accessibility
                                     extensions and plugins, such as screen
                                     readers.
    \value AccessibleDescriptionRole A description of the item for accessibility
                                     purposes.

    \value UserRole       The first role that can be used for
                          application-specific purposes.
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
    \fn bool QAbstractItemModel::decode(QDropEvent *event, const QModelIndex &parent)

    Decodes data contained in the \a event object, inserting it under the
    \a parent index if possible.

    Returns true if the data was successfully decoded and inserted;
    otherwise returns false.
*/
bool QAbstractItemModel::decode(QDropEvent *e, const QModelIndex &parent)
{
    return QAbstractItemModelDrag::decode(e, this, parent);
}

/*!
    \fn QDragObject *QAbstractItemModel::dragObject(const QModelIndexList &indexes, QWidget *dragSource)

    Returns a pointer to a QDragObject object containing the data
    associated with the \a indexes from the \a dragSource.

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
    custom roles.

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

    Sets the \c EditRole role data for the item at \a index to \a value.
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

    Returns the data stored under the given \a role for the item referred to
    by the \a index.
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
  Fetches any available data for the items with the parent specified by the
  \a parent index.
*/
void QAbstractItemModel::fetchMore(const QModelIndex &)
{
    // do nothing
}

/*!
    \enum QAbstractItemModel::ItemFlag

    This enum describes the properties of an item:

    \value ItemIsSelectable It can be selected.
    \value ItemIsEditable It can be edited.
    \value ItemIsDragEnabled It can be dragged.
    \value ItemIsDropEnabled It can be used as a drop target.
    \value ItemIsCheckable It can be checked.
    \value ItemIsEnabled The user can interact with the item.
*/

/*!
    Returns the item flags for the given \a index.

    The base class implementation returns a combination of flags that
    enables the item (\c ItemIsEnabled) and allows it to be
    selected (\c ItemIsSelectable).

    \sa ItemFlag
*/
QAbstractItemModel::ItemFlags QAbstractItemModel::flags(const QModelIndex &) const
{
    return ItemIsSelectable | ItemIsEnabled;
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
    Sorts the model by \a column in the given \a order for all the items with
    the specified \a parent. Sorting will only occur if the model is sortable.

    The base class implementation does nothing.

    \sa isSortable()
*/
void QAbstractItemModel::sort(int, const QModelIndex &, Qt::SortOrder)
{
    // do nothing
}

/*!
    Returns true if the data referred to by indexes \a left and \a right is
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
  Returns the buddy of the item represented by \a index.
  When the used wants to edit an item that is not editable,
  the delegate may ask for the item's buddy, and edit that
  item instead.

  \sa isEditable()
*/
QModelIndex QAbstractItemModel::buddy(const QModelIndex &) const
{
    return QModelIndex::Null;
}

/*!
    \enum QAbstractItemModel::MatchFlag

    \value MatchContains  The value is contained in the item.
    \value MatchFromStart The value matches the start of the item.
    \value MatchFromEnd   The value matches the end of the item.
    \value MatchExactly   The value matches the item exactly.
    \value MatchCase      The search is case sensitive.
    \value MatchWrap      The search wraps around.
    \value MatchDefault   The default match, which is MatchFromStart|MatchWrap.
*/

/*!
    Returns a list of indexes for the items where the data stored under
    the given \a role matches the specified \a value. The way the search
    is performed is defined by the \a flags given. The list that is
    returned may be empty.

    The search starts from the \a start index, and continues until the
    number of matching data items equals \a hits, the search reaches
    the last row, or the search reaches \a start again, depending on
    whether \c MatchWrap is specified in \a flags or not.

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
  Returns the row and column span of the item represented by \a index.
*/

QSize QAbstractItemModel::span(const QModelIndex &) const
{
    return QSize(1, 1);
}

/*!
  Returns the data for the given \a role and \a section in the header
  with the specified \a orientation.
*/

QVariant QAbstractItemModel::headerData(int section, Qt::Orientation, int role) const
{
    if (role == DisplayRole)
        return QString::number(section + 1);
    if (role == TextAlignmentRole)
        return Qt::AlignVCenter;
    return QVariant();
}

/*!
  Sets the title for the \a section in the header with the given
  \a orientation to the \a value for the specified \a role.

  \sa headerData()
*/

bool QAbstractItemModel::setHeaderData(int section, Qt::Orientation orientation, int role,
                                       const QVariant &value)
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);
    Q_UNUSED(value);
    return false;
}

/*!
    \fn QModelIndex QAbstractItemModel::createIndex(int row, int column, void *data) const

    Creates a model index for the given \a row and \a column that
    points to the given \a data.

    This function provides a consistent interface that model subclasses must
    use to create model indexes.
*/

/*!
  \internal
*/
void QAbstractItemModel::resetPersistentIndexes()
{
    invalidatePersistentIndexes();
}

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

    Invalidates the persistent indexes by setting them to invalid
    model indexes. Affects the given \a parent index, or if the \a
    parent is invalid affects all indexes.

    This function is used in model subclasses that can manage persistent
    model indexes.

*/
void QAbstractItemModel::invalidatePersistentIndexes(const QModelIndex &parent)
{
    bool all = !parent.isValid();
    for (int i = 0; i < d->persistentIndexes.count(); ++i) {
        if (all || this->parent(d->persistentIndexes.at(i)->index) == parent) {
            Q_ASSERT(d->persistentIndexes.at(i) != &QPersistentModelIndexData::shared_null);
            d->persistentIndexes.at(i)->index = QModelIndex::Null;
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

/*!
    \class QAbstractTableModel
    \brief The QAbstractTableModel class provides an abstract model that can be
    subclassed to create table models.

    \ingroup model-view

    QAbstractTableModel provides a standard interface for models that represent
    their data as a two-dimensional array of items. It is not used directly,
    but must be subclassed.

    Since the model provides a more specialized interface than
    QAbstractItemModel, it is not suitable for use with tree views, although
    it can be used to provide data to a QListView. If you need to represent
    a simple list of items, and only need a model to contain a single column
    of data, subclassing the QAbstractListModel may be more appropriate.

    The rowCount() and columnCount() functions return the dimensions of the
    table. To retrieve a model index corresponding to an item in the model, use
    index() and provide only the row and column numbers.

    When subclassing QAbstractTableModel, you must implement rowCount(),
    columnCount(), and data(). Default implementations of the index() and
    parent() functions are provided.

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemModel QAbstractListModel

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
    \fn QModelIndex QAbstractTableModel::index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const

    Returns the index of the data in \a row and \a column with \a parent.

    \sa parent()
*/

QModelIndex QAbstractTableModel::index(int row, int column, const QModelIndex &parent) const
{
    return isValid(row, column, parent) ? createIndex(row, column, 0) : QModelIndex::Null;
}

/*!
    \fn QModelIndex QAbstractTableModel::parent(const QModelIndex &index) const

    Returns the parent of the model item with the given \a index.

    \sa index() hasChildren()
*/

QModelIndex QAbstractTableModel::parent(const QModelIndex &) const
{
    return QModelIndex::Null;
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
    \brief The QAbstractListModel class provides an abstract model that can be
    subclassed to create one-dimensional list models.

    \ingroup model-view

    QAbstractListModel provides a standard interface for models that represent
    their data as a simple non-hierarchical sequence of items. It is not used
    directly, but must be subclassed.

    Since the model provides a more specialized interface than
    QAbstractItemModel, it is not suitable for use with tree views; you will
    need to subclass QAbstractItemModel if you want to provide a model for
    that purpose. If you need to use a number of list models to manage data,
    it may be more appropriate to subclass QAbstractTableModel class instead.

    Simple models can be created by subclassing this class and implementing
    the minimum number of required functions. For example, we could implement
    a simple read-only QStringList-based model that provides a list of strings
    to a QListView widget. In such a case, we only need to implement the
    rowCount() function to return the number of items in the list, and the
    data() function to retrieve items from the list.

    Since the model represents a one-dimensional structure, the rowCount()
    function returns the total number of items in the model. The columnCount()
    function is implemented for interoperability with all kinds of views, but
    by default informs views that the model contains only one column.

    When subclassing QAbstractListModel, you must provide implementations
    of the rowCount() and data() functions. A default implementation of
    columnCount() is provided that informs views that there is only a single
    column of items in this model.

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemView QAbstractTableView

*/

/*!
    \fn int QAbstractListModel::rowCount() const

    Returns the number of rows in the model.
    The number of rows is equal to the number of items stored in the model.
*/

/*!
    \fn int QAbstractListModel::columnCount() const

    Returns the number of columns in the model.
    List models only contain one column of items.
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
    \fn QModelIndex QAbstractListModel::index(int row, int column, const QModelIndex &parent = QModelIndex::Null) const

    Returns the index of the data in \a row and \a column with \a parent.

    \sa parent()
*/

QModelIndex QAbstractListModel::index(int row, int column, const QModelIndex &parent) const
{
    return isValid(row, column, parent) ? createIndex(row, column, 0) : QModelIndex::Null;
}

/*!
    \fn QModelIndex QAbstractListModel::parent(const QModelIndex &index) const

    Returns the parent of the model item with the given \a index.

    \sa index() hasChildren()
*/

QModelIndex QAbstractListModel::parent(const QModelIndex &) const
{
    return QModelIndex::Null;
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
