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
#include <qdatastream.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qsize.h>
#include <qmimedata.h>
#include <qdebug.h>
#include <qvector.h>
#include <private/qabstractitemmodel_p.h>

QPersistentModelIndexData QPersistentModelIndexData::shared_null;

QPersistentModelIndexData *QPersistentModelIndexData::create(const QModelIndex &index)
{
    Q_ASSERT(index.isValid());
    QPersistentModelIndexData *d = &QPersistentModelIndexData::shared_null;
    QAbstractItemModel *model = const_cast<QAbstractItemModel*>(index.model());
    QList<QPersistentModelIndexData*> *persistentIndexes =
        &(model->d_func()->persistentIndexes);
    for (int i = 0; i < persistentIndexes->count(); ++i) {
        if (persistentIndexes->at(i)->index == index) {
            d = persistentIndexes->at(i);
            break;
        }
    }
    if (d == &QPersistentModelIndexData::shared_null) {
        d = new QPersistentModelIndexData();
        d->model = model;
        d->index = index;
        persistentIndexes->append(d);
    }
    return d;
}

void QPersistentModelIndexData::destroy(QPersistentModelIndexData *data)
{
    if (data && data != &QPersistentModelIndexData::shared_null) {
        QAbstractItemModel *model = const_cast<QAbstractItemModel*>(data->model);
        // a valid persistent model index with a null model pointer can only happen if the model was destroyed
        if (model)
            model->d_func()->persistentIndexes.removeAll(data);
        delete data;
    }
}

/*!
  \class QPersistentModelIndex

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
    d->ref.ref();
}

/*!
  \fn QPersistentModelIndex::QPersistentModelIndex(const QPersistentModelIndex &other)

  Creates a new QPersistentModelIndex that is a copy of the \a other persistent
  model index.
*/

QPersistentModelIndex::QPersistentModelIndex(const QPersistentModelIndex &other)
    : d(other.d)
{
    d->ref.ref();
}

/*!
    Creates a new QPersistentModelIndex that is a copy of the model \a index.
*/

QPersistentModelIndex::QPersistentModelIndex(const QModelIndex &index)
    : d(&QPersistentModelIndexData::shared_null)
{
    if (index.isValid())
        d = QPersistentModelIndexData::create(index);
    d->ref.ref();
}

/*!
    \fn QPersistentModelIndex::~QPersistentModelIndex()

    \internal
*/

QPersistentModelIndex::~QPersistentModelIndex()
{
    if (!d->ref.deref())
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
    if (!d->ref.deref())
        QPersistentModelIndexData::destroy(d);
    d = other.d;
    d->ref.ref();
}

/*!
    Sets the persistent model index to refer to the same item in a model
    as the \a other model index.
*/

void QPersistentModelIndex::operator=(const QModelIndex &other)
{
    if (!d->ref.deref())
        QPersistentModelIndexData::destroy(d);
    if (other.isValid())
        d = QPersistentModelIndexData::create(other);
    else
        d = &QPersistentModelIndexData::shared_null;
    d->ref.ref();
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
  Returns the parent QModelIndex for this persistent index, or
  QModelIndex() if it has no parent.

  \sa child() sibling() model()
*/
QModelIndex QPersistentModelIndex::parent() const
{
    return d->index.parent();
}

/*!
  Returns the sibling at \a row and \a column or an invalid
  QModelIndex if there is no sibling at this position.

  \sa parent() child()
*/

QModelIndex QPersistentModelIndex::sibling(int row, int column) const
{
    return d->index.sibling(row, column);
}

/*!
  Returns the child of the model index that is stored in the given
  \a row and \a column.

  \sa parent() sibling()
*/

QModelIndex QPersistentModelIndex::child(int row, int column) const
{
    return d->index.child(row, column);
}

/*!
  Returns the model that the index belongs to.
*/
const QAbstractItemModel *QPersistentModelIndex::model() const
{
    return d->index.model();
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

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const QModelIndex &idx)
{
#ifndef Q_BROKEN_DEBUG_STREAM
    dbg.nospace() << "QModelIndex(" << idx.row() << "," << idx.column()
                  << "," << idx.data() << "," << idx.model() << ")";
    return dbg.space();
#else
    qWarning("This compiler doesn't support streaming QModelIndex to QDebug");
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

    Model indexes contain all the information required to specify the items
    they refer to in a model. Indexes are located in a row and a column, and
    they may have a parent index; use row(), column(), and parent() to obtain
    this information. Top-level items in a model are represented by model
    indexes that do not have a parent index - in this case, parent() will
    return an invalid model index that is equivalent to an index constructed
    with the zero argument form of the QModelIndex() constructor.

    To obtain a model index that refers to an item in a model, call
    \l{QAbstractItemModel::index()}{index()} with the required row and column
    values, and the parent model index. Supply the zero argument form of
    the QModelIndex() constructor as the parent index when referring to
    top-level items in a model.

    The model() function returns the model that the index references as a
    QAbstractItemModel.
    The child() function is used to examine the items held beneath the index
    in the model.
    The sibling() function allows you to traverse items in the model on the
    same level as the index.

    Model indexes can become invalid over time so they should be used
    immediately and then discarded. If you need to keep a model index
    over time use a QPersistentModelIndex.

    \sa \link model-view-programming.html Model/View Programming\endlink QPersistentModelIndex QAbstractItemModel
*/

/*!
    \fn QModelIndex::QModelIndex()

    Creates a new empty model index.
    This type of model index is used to indicate
    that the position in the model is invalid.

    \sa isValid() QAbstractItemModel
*/

/*!
    \fn QModelIndex::QModelIndex(int row, int column, void *data, const QAbstractItemModel *model)

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
    \fn const QAbstractItemModel *QModelIndex::model() const

    Returns a pointer to the model containing the item that this index
    refers to.
*/

/*!
    \fn QModelIndex QModelIndex::sibling(int row, int column) const

    Returns the sibling at \a row and \a column or an invalid
    QModelIndex if there is no sibling at this position.

    \sa parent() child()
*/

/*!
    \fn QModelIndex QModelIndex::child(int row, int column) const

    Returns the child of the model index that is stored in the given
    \a row and \a column.

    \sa parent() sibling()
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
  \fn QModelIndex QModelIndex::parent() const

  Return the parent of the model index, or QModelIndex() if it has no
  parent.

  \sa child() sibling() model()
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

    The QAbstractItemModel class is one of the \l{Model/View Classes}
    and is part of Qt's \l{Model/View Programming}{model/view framework}.

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
    Data is retrieved from an item with data() (for a single role), or
    with itemData() (for every role). Items can be queried with
    flags() (see {QAbstractTableModel::ItemFlag}) to see if they
    are selectable, dragable etc.  An item can be searched for using
    match().

    The model has a rowCount() and a columnCount() for each level of
    the hierarchy. Rows and columns can be inserted and removed with
    insertRows(), insertColumns(), removeRows(), and removeColumns().

    The model emits signals to indicate changes. For example,
    dataChanged() is emitted whenever the contents of the model are
    changed; rowsInserted(), columnsInserted(), rowsAboutToBeRemoved(),
    and columnsAboutToBeRemoved() are emitted when the model's dimensions
    are changed.

    If the model is sortable, it can be sorted with sort(). To
    customize sorting and searching, comparison functions can be
    reimplemented; for example, lessThan(), equal(), and
    greaterThan().

    When subclassing QAbstractItemModel, at the very least you must
    implement index(), parent(), rowCount(), columnCount(), hasChildren(),
    and data().  To enable editing in your model, you must
    also implement setData(), and reimplement flags() to ensure that
    \c ItemIsEditable is returned.

    You can also reimplement headerData() to control the way the headers
    for your model are presented.

    \sa \link model-view-programming.html Model/View Programming\endlink QModelIndex QAbstractItemView

*/

/*!
    \fn QModelIndex QAbstractItemModel::index(int row, int column, const QModelIndex &parent) const = 0

    Returns the index of the item in the model specified by the given \a row,
    \a column and \a parent index.
*/

/*!
    \fn bool QAbstractItemModel::insertColumn(int column, const QModelIndex &parent)

    Inserts a single column before the given \a column in the child items of
    the \a parent specified. Returns true if the column is inserted; otherwise
    returns false.

    \sa insertColumns() insertRow() removeColumn()
*/

/*!
    \fn bool QAbstractItemModel::insertRow(int row, const QModelIndex &parent)

    Inserts a single row before the given \a row in the child items of the
    \a parent specified. Returns true if the row is inserted; otherwise
    returns false.

    \sa insertRows() insertColumn() removeRow()
*/

/*!
    \fn QModelIndex QAbstractItemModel::parent(const QModelIndex &index) const = 0

    Returns the parent of the model item with the given \a index.
*/

/*!
    \fn bool QAbstractItemModel::removeColumn(int column, const QModelIndex &parent)

    Removes the given \a column from the child items of the \a parent specified.
    Returns true if the column is removed; otherwise returns false.

    \sa removeColumns() removeRow() insertColumn()
*/

/*!
    \fn bool QAbstractItemModel::removeRow(int row, const QModelIndex &parent)

    Removes the given \a row from the child items of the \a parent specified.
    Returns true if the row is removed; otherwise returns false.

    \sa removeRows() removeColumn() insertRow()
*/

/*!
    \fn void QAbstractItemModel::headerDataChanged(Qt::Orientation orientation, int first, int last)

    This signal is emitted whenever a header is changed. The \a orientation
    indicates whether the horizontal or vertical header has changed. The
    sections in the header from the \a first to the \a last need to be updated.

    \sa headerData() setHeaderData()
*/

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
    Q_D(QAbstractItemModel);
    QList<QPersistentModelIndexData*>::iterator it = d->persistentIndexes.begin();
    for (; it != d->persistentIndexes.end(); ++it) {
        Q_ASSERT((*it) != &QPersistentModelIndexData::shared_null);
        (*it)->index = QModelIndex();
        (*it)->model = 0;
    }
}

/*!
    \fn QModelIndex QAbstractItemModel::sibling(int row, int column, const QModelIndex &index) const

    Returns the sibling at \a row and \a column for the item at \a
    index or an invalid QModelIndex if there is no sibling.

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

    \sa rowsInserted() rowsAboutToBeRemoved() columnsInserted() columnsAboutToBeRemoved() setData()
*/

/*!
    \fn void QAbstractItemModel::rowsInserted(const QModelIndex &parent, int start, int end)

    This signal is emitted after rows have been inserted into the
    model. The new items are those between \a start and \a end
    inclusive, under the given \a parent item.

    \sa insertRows()
*/

/*!
    \fn void QAbstractItemModel::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)

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
    \fn void QAbstractItemModel::columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)

    This signal is emitted just before columns are removed
    from the model. The removed items are those between \a start and
    \a end inclusive, under the given \a parent item.

    \sa removeRows()
*/

/*!
    \fn void QAbstractItemModel::reset()

    This signal is emitted when the model has been reset. When a model
    is reset it means that any previous data reported from the model
    is now invalid and has to be queried for again.

    When a model radically changes its data it can sometimes be easier
    to just emit this signal rather than using rowsRemoved(),
    dataChanged() etc.
*/

/*!
    \enum QAbstractItemModel::Role

    Each item in the model has a set of data elements associated with
    it, each with its own role. The roles are used by the view to indicate
    to the model which type of data it needs.

    The general purpose roles are:

    \value DisplayRole    The key data to be rendered (usually text).
    \value DecorationRole The data to be rendered as a decoration (usually an icon).
    \value EditRole       The data in a form suitable for editing in an
                          editor.
    \value ToolTipRole    The data displayed in the item's tooltip.
    \value StatusTipRole  The data displayed in the status bar.
    \value WhatsThisRole  The data displayed for the item in "What's This?"
                          mode.

    Roles describing appearance and meta data:

    \value FontRole            The font used for items rendered with the default
                               delegate.
    \value TextAlignmentRole   The alignment of the text for items rendered with the
                               default delegate.
    \value BackgroundColorRole The background color used for items rendered with
                               the default delegate.
    \value TextColorRole       The text color used for items rendered with
                               the default delegate.
    \value CheckStateRole      This role is used to obtain the checked state of
                               an item (see \l Qt::CheckState).

    Accessibility roles:

    \value AccessibleTextRole        The text to be used by accessibility
                                     extensions and plugins, such as screen
                                     readers.
    \value AccessibleDescriptionRole A description of the item for accessibility
                                     purposes.

    User roles:

    \value UserRole       The first role that can be used for
                                     application-specific purposes.
*/

/*!
  Returns true if the model returns a valid QModelIndex for \a row and
  \a column with \a parent, otherwise returns false.
*/
bool QAbstractItemModel::hasIndex(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0)
        return false;
    return row < rowCount(parent) && column < columnCount(parent);
}


/*!
  Returns true if \a parent has any children; otherwise returns false.

  \sa parent() index()
*/
bool QAbstractItemModel::hasChildren(const QModelIndex &parent) const
{
    return (rowCount(parent) > 0) && (columnCount(parent) > 0);
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
        if (variantData.type() != QVariant::Invalid)
            roles.insert(i, variantData);
    }
    return roles;
}

/*!
    Sets the \a role data for the item at \a index to \a value.
    Returns true if successful; otherwise returns false.

    The base class implementation returns false. This function and
    data() must be reimplemented for editable models.

    \sa data() itemData()
*/
bool QAbstractItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(index);
    Q_UNUSED(value);
    Q_UNUSED(role);
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
        b = b && setData(index, it.value(), it.key());
    return b;
}

/*!
    Returns a list of MIME types that can be used to describe a list of
    model indexes.

    \sa mimeData()
*/
QStringList QAbstractItemModel::mimeTypes() const
{
    QStringList types;
    types << "application/x-qabstractitemmodeldatalist";
    return types;
}

/*!
    Returns an object that contains a serialized description of the specified
    \a indexes. The format used to describe the items corresponding to the
    indexes is obtained from the mimeTypes() function.

    If the list of indexes is empty, 0 is returned rather than a serialized
    empty list.
*/
QMimeData *QAbstractItemModel::mimeData(const QModelIndexList &indexes) const
{
    if (indexes.count() <= 0)
        return 0;
    QMimeData *data = new QMimeData();
    QString format = mimeTypes().at(0);
    QByteArray encoded;
    QDataStream stream(&encoded, QIODevice::WriteOnly);
    encodeData(indexes, stream);
    data->setData(format, encoded);
    return data;
}

/*!
    Handles the \a data supplied by a drag and drop operation that ended with
    the given \a action over the row in the model specified by the \a row
    and the \a parent index.

    \sa supportedDropActions()
*/
bool QAbstractItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                      int row, const QModelIndex &parent)
{
    // check if the action is supported
    if (action != Qt::CopyAction)
        return false;
    // check if the format is supported
    QString format = mimeTypes().at(0);
    if (!data->hasFormat(format))
        return false;
    // check if we have a valid table
    if (columnCount(parent) <= 0)
        return false;
    // decode and insert
    QByteArray encoded = data->data(format);
    QDataStream stream(&encoded, QIODevice::ReadOnly);
    return decodeData(row, parent, stream);
}

/*!
  Returns the drop actions supported by this model.

  \sa Qt::DropActions
*/
Qt::DropActions QAbstractItemModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

/*!
  Inserts \a count rows into the model before the given \a row.
  The items in the new row will be children of the item represented by the
  \a parent model index.

  If \a row is 0, the rows are prepended to any existing rows in the parent.
  If \a row is rowCount(), the rows are appended to any existing rows in the
  parent.
  If \a parent has no children, a single column with \a count rows is inserted.

  Returns true if the rows were successfully inserted; otherwise returns
  false.

  The base class implementation does nothing and returns false. If
  you want to be able to insert rows you must reimplement this
  function.
*/
bool QAbstractItemModel::insertRows(int, int, const QModelIndex &)
{
    return false;
}

/*!
  Inserts \a count new columns into the model before the given \a column.
  The items in each new column will be children of the item represented by the
  \a parent model index.

  If \a column is 0, the columns are prepended to any existing columns.
  If \a column is columnCount(), the columns are appended to any existing
  columns.
  If \a parent has no children, a single row with \a count columns is inserted.

  Returns true if the columns were successfully inserted; otherwise returns
  false.

  The base class implementation does nothing and returns false. If
  you want to be able to insert columns you must reimplement this
  function.
*/
bool QAbstractItemModel::insertColumns(int, int, const QModelIndex &)
{
    return false;
}

/*!
    Removes \a count rows starting with the given \a row under parent
    \a parent from the model. Returns true if the rows were successfully
    removed; otherwise returns false.

    The base class implementation does nothing and returns false.
*/
bool QAbstractItemModel::removeRows(int, int, const QModelIndex &)
{
    return false;
}

/*!
    Removes \a count columns starting with the given \a column under
    parent \a parent from the model.  Returns true if the columns were
    successfully removed; otherwise returns false.

    The base class implementation does nothing and returns false.
*/
bool QAbstractItemModel::removeColumns(int, int, const QModelIndex &)
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
    \value ItemIsUserCheckable It can be checked or unchecked by the user.
    \value ItemIsEnabled The user can interact with the item.
    \value ItemIsTristate The item is checkable with three separate states.
*/

/*!
    Returns the item flags for the given \a index.

    The base class implementation returns a combination of flags that
    enables the item (\c ItemIsEnabled) and allows it to be
    selected (\c ItemIsSelectable).

    \sa ItemFlag
*/
QAbstractItemModel::ItemFlags QAbstractItemModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return ItemIsSelectable | ItemIsEnabled;
}

/*!
    Sorts the model by \a column in the given \a order.

    The base class implementation does nothing.
*/
void QAbstractItemModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column);
    Q_UNUSED(order);
    // do nothing
}

/*!
  Returns a model index for the buddy of the item represented by \a index.
  When the user wants to edit an item, the view will call this function to
  check whether another item in the model should be edited instead, and
  construct a delegate using the model index returned by the buddy item.

  In the default implementation each item is its own buddy.
*/
QModelIndex QAbstractItemModel::buddy(const QModelIndex &index) const
{
    return index;
}

/*!
    \enum QAbstractItemModel::MatchFlag

    This enum describes the type of matches that can be used when searching
    for items in a model.

    \value MatchContains  The value is contained in the item.
    \value MatchFromStart The value matches the start of the item.
    \value MatchFromEnd   The value matches the end of the item.
    \value MatchExactly   The value matches the item exactly.
    \value MatchCase      The search is case sensitive.
    \value MatchWrap      The search wraps around.
*/

/*!
    Returns a list of indexes for the items where the data stored under
    the given \a role matches the specified \a value. The way the search
    is performed is defined by the \a flags given. The list that is
    returned may be empty.

    The search starts from the \a start index, and continues until the
    number of matching data items equals \a hits, the search reaches
    the last row, or the search reaches \a start again, depending on
    whether \c MatchWrap is specified in \a flags.

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
  Called to let the model know that it should submit whatever it has cached
  to the permanent storage. Typically used for row editing.

  Returns false on error, otherwise true.
*/

bool QAbstractItemModel::submit()
{
    return true;
}

/*!
  Called to let the model know that it should discart whatever it has cached.
  Typically used for row editing.
*/

void QAbstractItemModel::revert()
{
    // do nothing
}

/*!
  Returns the data for the given \a role and \a section in the header
  with the specified \a orientation.
*/

QVariant QAbstractItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == DisplayRole) {
        if ((orientation == Qt::Horizontal && section < columnCount())
            || (orientation == Qt::Vertical && section < rowCount()))
            return QString::number(section + 1);
    } else if (role == TextAlignmentRole)
        return Qt::AlignVCenter;
    return QVariant();
}

/*!
  Sets the \a role for the header \a section to \a value.
  The \a orientation gives the orientation of the header.

  \sa headerData()
*/

bool QAbstractItemModel::setHeaderData(int section, Qt::Orientation orientation,
                                       const QVariant &value, int role)
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(value);
    Q_UNUSED(role);
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
void QAbstractItemModel::encodeData(const QModelIndexList &indexes, QDataStream &stream) const
{
    QModelIndexList::ConstIterator it = indexes.begin();
    for (; it != indexes.end(); ++it) {
        QMap<int, QVariant> data = itemData(*it);
        stream << data.count(); // roles
        QMap<int, QVariant>::ConstIterator it2 = data.begin();
        for (; it2 != data.end(); ++it2) {
            stream << it2.key();
            stream << it2.value();
        }
//         if (hasChildren(*it)) { // encode children FIXME: disabled for now
//             stream << rowCount(*it);
//             stream << columnCount(*it);
//             encodeData(*it, stream);
//         } else { // no children
            stream << 0;
            stream << 0;
//        }
    }
}

/*!
  \internal
*/
void QAbstractItemModel::encodeData(const QModelIndex &parent, QDataStream &stream) const
{
    for (int row = 0; row < rowCount(parent); ++row) {
        for (int column = 0; column < columnCount(parent); ++column) {
            QModelIndex idx = index(row, column, parent);
            QMap<int, QVariant> data = itemData(idx);
            stream << data.count(); // roles
            QMap<int, QVariant>::ConstIterator it2 = data.begin();
            for (; it2 != data.end(); ++it2) {
                stream << it2.key();
                stream << it2.value();
            }
//             if (hasChildren(idx)) { // encode children FIXME: disabled for now
//                 stream << rowCount(idx);
//                 stream << columnCount(idx);
//                 encodeData(idx, stream);
//             } else { // no children
                stream << 0;
                stream << 0;
//            }
        }
    }
}

/*!
  \internal
*/
bool QAbstractItemModel::decodeData(int row, const QModelIndex &parent, QDataStream &stream)
{
    int count, role, rows, columns;
    QVariant value;
    QModelIndex idx;
    QVector<QModelIndex> parents;
    while (!stream.atEnd()) {
        insertRows(row, 1, parent);
        int column = 0;
        while (!stream.atEnd() && column < columnCount(parent)) {
            idx = index(row, column, parent); // only insert in col 0
            stream >> count; // roles
            for (int i = 0; i < count; ++i) {
                stream >> role;
                stream >> value;
                setData(idx, value, role);
            }
            stream >> rows;
            stream >> columns;
            if (rows && columns) // decode children
                decodeData(0, idx, stream);
            ++column;
        }
        ++row;
    }
    return true;
}

/*!
  \internal
*/
void QAbstractItemModel::resetPersistentIndexes()
{
    invalidatePersistentIndexes();
}

/*!
  \internal
*/
void QAbstractItemModel::invalidatePersistentIndex(const QModelIndex &index)
{
    Q_D(QAbstractItemModel);
    // FIXME: make this a QMap<QModelIndex, QPresistentModelIndexData*> or something similar
    QList<QPersistentModelIndexData*>::iterator it = d->persistentIndexes.begin();
    for (; it != d->persistentIndexes.end(); ++it) {
        if ((*it)->index == index) {
            (*it)->index = QModelIndex();
            if (hasChildren(index))
                invalidatePersistentIndexes(index);
            return;
        }
    }
}

/*!
    \internal

    Invalidates the persistent indexes by setting them to invalid
    model indexes. Affects the given \a parent index, or if the \a
    parent is invalid affects all indexes.

    This function is used in model subclasses that can manage persistent
    model indexes.

    NOTE: this function is recursive
*/
void QAbstractItemModel::invalidatePersistentIndexes(const QModelIndex &parent)
{
    Q_D(QAbstractItemModel);
    bool all = !parent.isValid();
    QList<QPersistentModelIndexData*>::iterator it = d->persistentIndexes.begin();
    for (; it != d->persistentIndexes.end(); ++it) {
        if (all || (*it)->index.parent() == parent) {
            Q_ASSERT((*it) != &QPersistentModelIndexData::shared_null);
            if (!all) invalidatePersistentIndexes((*it)->index); // recursive
            (*it)->index = QModelIndex();
        }
    }
}



/*!
    \internal

    Returns the number of persistent indexes held by this model.
*/
int QAbstractItemModel::persistentIndexesCount() const
{
    Q_D(const QAbstractItemModel);
    return d->persistentIndexes.count();
}

/*!
    \internal

    Returns the persistent index at the given \a position.
*/
QModelIndex QAbstractItemModel::persistentIndexAt(int position) const
{
    Q_D(const QAbstractItemModel);
    return d->persistentIndexes.at(position)->index;
}

/*!
    \internal

    Sets the model's persistent index at position \a position to \a
    index.
*/
void QAbstractItemModel::setPersistentIndex(int position, const QModelIndex &index)
{
    Q_D(QAbstractItemModel);
    d->persistentIndexes[position]->index = index;
}

/*!
  \internal

  Returns the position of \a index in the list of persistent indexes.
*/
int QAbstractItemModel::persistentIndexPosition(const QModelIndex &index, int from) const
{
    Q_D(const QAbstractItemModel);
    for (int i = from; i < d->persistentIndexes.count(); ++i)
        if (d->persistentIndexes.at(i)->index == index)
            return i;
    return -1;
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
    \fn QModelIndex QAbstractTableModel::index(int row, int column, const QModelIndex &parent = QModelIndex()) const

    Returns the index of the data in \a row and \a column with \a parent.

    \sa parent()
*/

QModelIndex QAbstractTableModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column, 0) : QModelIndex();
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

bool QAbstractTableModel::hasChildren(const QModelIndex &) const
{
    return false;
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

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemView QAbstractTableModel

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
    \fn QModelIndex QAbstractListModel::index(int row, int column, const QModelIndex &parent = QModelIndex()) const

    Returns the index of the data in \a row and \a column with \a parent.

    \sa parent()
*/

QModelIndex QAbstractListModel::index(int row, int column, const QModelIndex &parent) const
{
    return hasIndex(row, column, parent) ? createIndex(row, column, 0) : QModelIndex();
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

    Returns the number of columns in the list with the given \a parent.

    \sa rowCount()
*/

int QAbstractListModel::columnCount(const QModelIndex &) const
{
    return 1;
}

bool QAbstractListModel::hasChildren(const QModelIndex &) const
{
    return false;
}
