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
    QList<QPersistentModelIndexData*> *persistentIndexes = &(model->d_func()->persistent.indexes);
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
        if (model) {
            QAbstractItemModelPrivate *p = model->d_func();
            int position = p->persistent.indexes.indexOf(data);
            p->persistent.changed.removeAll(position);
            p->persistent.invalidated.removeAll(position);
            p->persistent.indexes.removeAll(data);
        }
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
    \fn void *QPersistentModelIndex::internalPointer() const

    \internal

    Returns a \c{void} \c{*} pointer used by the model to associate the index with
    the internal data structure.
*/

void *QPersistentModelIndex::internalPointer() const
{
    return d->index.internalPointer();
}

/*!
    \fn void *QPersistentModelIndex::internalId() const

    \internal

    Returns a \c{qint64} used by the model to associate the index with
    the internal data structure.
*/

qint64 QPersistentModelIndex::internalId() const
{
    return d->index.internalId();
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
                  << "," << idx.internalId() << "," << idx.model() << ")";
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

QAbstractItemModelPrivate::~QAbstractItemModelPrivate()
{
    QList<QPersistentModelIndexData*>::iterator it = persistent.indexes.begin();
    for (; it != persistent.indexes.end(); ++it) {
        Q_ASSERT((*it) != &QPersistentModelIndexData::shared_null);
        (*it)->index = QModelIndex();
        (*it)->model = 0;
    }
}

void QAbstractItemModelPrivate::invalidate(int position)
{
    QModelIndex parent = persistent.indexes.at(position)->index;
    for (int i = 0; i < persistent.indexes.count(); ++i) {
        if (persistent.indexes.at(i)->index.parent() == parent) {
            Q_ASSERT((persistent.indexes.at(i)) != &QPersistentModelIndexData::shared_null);
            invalidate(i); // recursive
        }
    }
    persistent.indexes[position]->index = QModelIndex();
}

void QAbstractItemModelPrivate::rowsAboutToBeInserted(const QModelIndex &parent,
                                                         int first, int last)
{
    Q_UNUSED(last);
    persistent.changed.clear();
    for (int position = 0; position < persistent.indexes.count(); ++position) {
        QModelIndex index = persistent.indexes.at(position)->index;
        if (index.isValid() && index.parent() == parent && index.row() >= first)
            persistent.changed.append(position);
    }
}

void QAbstractItemModelPrivate::rowsInserted(const QModelIndex &parent,
                                                int first, int last)
{
    int count = (last - first) + 1;
    for (int i = 0; i < persistent.changed.count(); ++i) {
        int position = persistent.changed.at(i);
        QModelIndex old = persistent.indexes.at(position)->index;
        persistent.indexes[position]->index =
            q_func()->index(old.row() + count, old.column(), parent);
    }
    persistent.changed.clear();
}

void QAbstractItemModelPrivate::rowsAboutToBeRemoved(const QModelIndex &parent,
                                                        int first, int last)
{
    persistent.changed.clear();
    persistent.invalidated.clear();
    for (int position = 0; position < persistent.indexes.count(); ++position) {
        QModelIndex index = persistent.indexes.at(position)->index;
        if (index.isValid() && index.parent() == parent) {
            if (index.row() > last) // below the removed rows
                persistent.changed.append(position);
            else if (index.row() >= first) // about to be removed
                persistent.invalidated.append(position);
        }
    }
}

void QAbstractItemModelPrivate::rowsRemoved(const QModelIndex &parent,
                                               int first, int last)
{
    int count = (last - first) + 1;
    for (int i = 0; i < persistent.changed.count(); ++i) {
        int position = persistent.changed.at(i);
        QModelIndex old = persistent.indexes.at(position)->index;
        persistent.indexes[position]->index =
            q_func()->index(old.row() - count, old.column(), parent);
    }
    persistent.changed.clear();
    for (int j = 0; j < persistent.invalidated.count(); ++j)
        invalidate(persistent.invalidated.at(j));
    persistent.invalidated.clear();
}

void QAbstractItemModelPrivate::columnsAboutToBeInserted(const QModelIndex &parent,
                                                            int first, int last)
{
    Q_UNUSED(last);
    persistent.changed.clear();
    for (int position = 0; position < persistent.indexes.count(); ++position) {
        QModelIndex index = persistent.indexes.at(position)->index;
        if (index.isValid() && index.parent() == parent && index.column() >= first)
            persistent.changed.append(position);
    }
}

void QAbstractItemModelPrivate::columnsInserted(const QModelIndex &parent,
                                                   int first, int last)
{
    int count = (last - first) + 1;
    for (int i = 0; i < persistent.changed.count(); ++i) {
        int position = persistent.changed.at(i);
        QModelIndex old = persistent.indexes.at(position)->index;
        persistent.indexes[position]->index =
            q_func()->index(old.row(), old.column() + count, parent);
    }
    persistent.changed.clear();
}

void QAbstractItemModelPrivate::columnsAboutToBeRemoved(const QModelIndex &parent,
                                                           int first, int last)
{
    persistent.changed.clear();
    persistent.invalidated.clear();
    for (int position = 0; position < persistent.indexes.count(); ++position) {
        QModelIndex index = persistent.indexes.at(position)->index;
        if (index.isValid() && index.parent() == parent) {
            if (index.column() > last) // after the removed columns
                persistent.changed.append(position);
            else if (index.column() >= first) // about to be removed
                persistent.invalidated.append(position);
        }
    }
}

void QAbstractItemModelPrivate::columnsRemoved(const QModelIndex &parent,
                                                  int first, int last)
{
    int count = (last - first) + 1;
    for (int i = 0; i < persistent.changed.count(); ++i) {
        int position = persistent.changed.at(i);
        QModelIndex old = persistent.indexes.at(position)->index;
        persistent.indexes[position]->index =
            q_func()->index(old.row(), old.column() - count, parent);
    }
    persistent.changed.clear();
    for (int j = 0; j < persistent.invalidated.count(); ++j)
        invalidate(persistent.invalidated.at(j));
    persistent.invalidated.clear();
}

void QAbstractItemModelPrivate::reset()
{
    for (int i = 0; i < persistent.indexes.count(); ++i)
        persistent.indexes[i]->index = QModelIndex();
}

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
    QAbstractItemModel::index() with the required row and column
    values, and the model index of the parent. When referring to
    top-level items in a model, supply QModelIndex() as the parent index.

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
    \fn void *QModelIndex::internalPointer() const

    Returns a \c{void} \c{*} pointer used by the model to associate
    the index with the internal data structure.
*/

/*!
    \fn void *QModelIndex::internalId() const

    Returns a \c{qint64} used by the model to associate
    the index with the internal data structure.
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
    \mainclass

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

    Every item of data that can be accessed via a model has an associated
    model index that is obtained using the index() function. Each index
    may have a sibling() index; child items have a parent() index.

    Each item has a number of data elements associated with it, and each
    of these can be retrieved by specifying a role (see \l Qt::ItemDataRole)
    to the model's data() function. Data for all available roles can be
    obtained at the same time using the itemData() function.

    Data for each role is set using a particular \l Qt::ItemDataRole.
    Data for individual roles are set individually with setData(), or they
    can be set for all roles with setItemData().

    Items can be queried with flags() (see \l Qt::ItemFlag) to see if they
    can be selected, dragged, or manipulated in other ways.

    If an item has child objects, hasChildren() returns true for the
    corresponding index.

    The model has a rowCount() and a columnCount() for each level of
    the hierarchy. Rows and columns can be inserted and removed with
    insertRows(), insertColumns(), removeRows(), and removeColumns().

    The model emits signals to indicate changes. For example,
    dataChanged() is emitted whenever the contents of the model are
    changed. Other components are made aware of changes to the number of
    rows in the model via the rowsAboutToBeInserted(), rowsInserted(),
    rowsAboutToBeRemoved(), and rowsRemoved() signals. Changes to the
    number of columns are sent via the columnsAboutToBeInserted(),
    columnsInserted(), columnsAboutToBeRemoved(), and columnsRemoved()
    signals.

    The items available through the model can be searched for particular
    data using the match() function.

    If the model is sortable, it can be sorted with sort(). To
    customize sorting and searching, comparison functions can be
    reimplemented; for example, lessThan(), equal(), and
    greaterThan().

    \section1 Subclassing

    When subclassing QAbstractItemModel, at the very least you must
    implement index(), parent(), rowCount(), columnCount(), hasChildren(),
    and data().  To enable editing in your model, you must
    also implement setData(), and reimplement flags() to ensure that
    \c ItemIsEditable is returned.

    You can also reimplement headerData() and setHeaderData() to control
    the way the headers for your model are presented.

    Models that provide interfaces to resizable data structures can
    provide implementations of insertRows(), removeRows(), insertColumns(),
    and removeColumns(). When implementing these functions, it is
    important to notify any connected views about changes to the model's
    dimensions both \e before and \e after they occur:

    \list
    \o An insertRows() implementation must call beginInsertRows()
       \e before inserting new rows into the data structure, and it must
       call endInsertRows() \e{immediately afterwards}.
    \o An insertColumns() implementation must call beginInsertColumns()
       \e before inserting new columns into the data structure, and it must
       call endInsertColumns() \e{immediately afterwards}.
    \o A removeRows() implementation must call beginRemoveRows() 
       \e before the rows are removed from the data structure, and it must
       call endRemoveRows() \e{immediately afterwards}.
    \o A removeColumns() implementation must call beginRemoveColumns()
       \e before the columns are removed from the data structure, and it must
       call endRemoveColumns() \e{immediately afterwards}.
    \endlist

    The signals that these functions emit give attached components the chance
    to take action before any data becomes unavailable. The encapsulation of
    the insert and remove operations with these begin and end functions also
    enable the model to manage
    \l{QPersistentModelIndex}{persistent model indexes} correctly.
    \bold{If you want selections to be handled properly, you must ensure that
    you call these functions.}

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
    \fn void QAbstractItemModel::rowsAboutToBeInserted(const QModelIndex &parent, int start, int end)

    This signal is emitted just before rows are inserted into the
    model. The new items will be positioned between \a start and \a end
    inclusive, under the given \a parent item.

    \sa insertRows()
*/

/*!
    \fn void QAbstractItemModel::rowsRemoved(const QModelIndex &parent, int start, int end)

    This signal is emitted after rows have been removed from the
    model. The removed items are those between \a start and \a end
    inclusive, under the given \a parent item.

    \sa removeRows()
*/

/*!
    \fn void QAbstractItemModel::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)

    This signal is emitted just before rows are removed from the
    model. The items that will be removed are those between \a start and \a end
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
    \fn void QAbstractItemModel::columnsAboutToBeInserted(const QModelIndex &parent, int start, int end)

    This signal is emitted just before columns are inserted into the
    model. The new items will be positioned between \a start and \a end
    inclusive, under the given \a parent item.

    \sa insertColumns()
*/

/*!
    \fn void QAbstractItemModel::columnsRemoved(const QModelIndex &parent, int start, int end)

    This signal is emitted after columns have been removed from the
    model. The removed items are those between \a start and \a end
    inclusive, under the given \a parent item.

    \sa removeColumns()
*/

/*!
    \fn void QAbstractItemModel::columnsAboutToBeRemoved(const QModelIndex &parent, int start, int end)

    This signal is emitted just before columns are removed
    from the model. The items to be removed are those between \a start and
    \a end inclusive, under the given \a parent item.

    \sa removeColumns()
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

    \sa Qt::ItemDataRole data()
*/
QMap<int, QVariant> QAbstractItemModel::itemData(const QModelIndex &index) const
{
    QMap<int, QVariant> roles;
    for (int i = 0; i < Qt::UserRole; ++i) {
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

    \sa Qt::ItemDataRole
*/

/*!
    For every \c Qt::ItemDataRole in \a roles, sets the role data for the item at
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
    the given \a action over the row in the model specified by the \a row,
    \a column, and the \a parent index.

    \sa supportedDropActions()
*/
bool QAbstractItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
                                      int row, int column, const QModelIndex &parent)
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
    return decodeData(row, column, parent, stream);
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

  The base class implementation does nothing and returns false.

  If you want to be able to insert rows in a subclass, you must reimplement
  this function, calling the beginInsertRows() function \e before inserting
  new rows into your underlying data store, and call the endInsertRows()
  function \e afterwards. Return true to indicate success; otherwise return
  false.
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

  The base class implementation does nothing and returns false.

  If you want to be able to insert columns in a subclass, you must reimplement
  this function, calling the beginInsertColumns() function \e before inserting
  new columns into your underlying data store, and call the endInsertColumns()
  function \e afterwards. Return true to indicate success; otherwise return
  false.
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

    If you want to be able to remove rows in a subclass, you must reimplement
    this function, calling the beginRemoveRows() function \e before removing
    new rows into your underlying data store, and call the endRemoveRows()
    function \e afterwards. Return true to indicate success; otherwise return
    false.
*/
bool QAbstractItemModel::removeRows(int, int, const QModelIndex &)
{
    return false;
}

/*!
    Removes \a count columns starting with the given \a column under
    parent \a parent from the model. Returns true if the columns were
    successfully removed; otherwise returns false.

    The base class implementation does nothing and returns false.

    If you want to be able to remove columns in a subclass, you must reimplement
    this function, calling the beginRemoveColumns() function \e before removing
    new columns into your underlying data store, and call the endRemoveColumns()
    function \e afterwards. Return true to indicate success; otherwise return
    false.
*/
bool QAbstractItemModel::removeColumns(int, int, const QModelIndex &)
{
    return false;
}

/*!
  Fetches any available data for the items with the parent specified by the
  \a parent index.

  Reimplement this if you have incremental data.

  The default implementation does nothing.

  \sa canFetchMore()
*/
void QAbstractItemModel::fetchMore(const QModelIndex &)
{
    // do nothing
}

/*!
  Returns true if there is more data available for \a parent,
  otherwise false.

  The default implementation always returns false.

  \sa fetchMore()
*/
bool QAbstractItemModel::canFetchMore(const QModelIndex &) const
{
    return false;
}

/*!
    Returns the item flags for the given \a index.

    The base class implementation returns a combination of flags that
    enables the item (\c ItemIsEnabled) and allows it to be
    selected (\c ItemIsSelectable).

    \sa Qt::ItemFlags
*/
Qt::ItemFlags QAbstractItemModel::flags(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return Qt::ItemIsSelectable|Qt::ItemIsEnabled;
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

  \sa Qt::ItemDataRole
*/

QVariant QAbstractItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if ((orientation == Qt::Horizontal && section < columnCount())
            || (orientation == Qt::Vertical && section < rowCount()))
            return QString::number(section + 1);
    } else if (role == Qt::TextAlignmentRole)
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
    \fn QModelIndex QAbstractItemModel::createIndex(int row, int column, void *ptr) const

    Creates a model index for the given \a row and \a column with the internal pointer \a ptr.

    This function provides a consistent interface that model subclasses must
    use to create model indexes.
*/

/*!
    \fn QModelIndex QAbstractItemModel::createIndex(int row, int column, int id) const

    Creates a model index for the given \a row and \a column with the internal identifier \a id.

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
//        if (hasChildren(*it)) { // encode children FIXME: disabled for now
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
            if (hasChildren(idx)) { // encode children
                stream << rowCount(idx);
                stream << columnCount(idx);
                encodeData(idx, stream);
            } else { // no children
                stream << 0;
                stream << 0;
            }
        }
    }
}

/*!
  \internal
*/
bool QAbstractItemModel::decodeData(int row, int column, const QModelIndex &parent,
                                    QDataStream &stream)
{
    int count, role, rows, columns;
    QVariant value;
    QModelIndex idx;
    QVector<QModelIndex> parents;
    while (!stream.atEnd()) {
        insertRows(row, 1, parent);
        column = 0; // NOTE: the argument is ignored; we insert a new row per data item
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
                decodeData(0, 0, idx, stream);
            ++column;
        }
        ++row;
    }
    return true;
}

/*!
    Begins a row insertion operation.

    When reimplementing insertRows() in a subclass, you must call this
    function \e before inserting data into the model's underlying data
    store.

    The \a parent index corresponds to the parent into which the new
    rows are inserted; \a first and \a last are the row numbers of the new
    rows to be inserted.

    \sa endInsertRows()
*/
void QAbstractItemModel::beginInsertRows(const QModelIndex &parent, int first, int last)
{
    Q_D(QAbstractItemModel);
    d->change.parent = parent;
    d->change.first = first;
    d->change.last = last;
    emit rowsAboutToBeInserted(parent, first, last);
    d->rowsAboutToBeInserted(parent, first, last);
}

/*!
    Ends a row insertion operation.

    When reimplementing insertRows() in a subclass, you must call this
    function \e after inserting data into the model's underlying data
    store.

    \sa beginInsertRows()
*/
void QAbstractItemModel::endInsertRows()
{
    Q_D(QAbstractItemModel);
    d->rowsInserted(d->change.parent, d->change.first, d->change.last);
    emit rowsInserted(d->change.parent, d->change.first, d->change.last);
}

/*!
    Begins a row removal operation.

    When reimplementing removeRows() in a subclass, you must call this
    function \e before removing data from the model's underlying data
    store.

    The \a parent index corresponds to the parent from which the new
    rows are removed; \a first and \a last are the row numbers of the
    rows to be removed.

    \sa endRemoveRows()
*/
void QAbstractItemModel::beginRemoveRows(const QModelIndex &parent, int first, int last)
{
    Q_D(QAbstractItemModel);
    d->change.parent = parent;
    d->change.first = first;
    d->change.last = last;
    emit rowsAboutToBeRemoved(parent, first, last);
    d->rowsAboutToBeRemoved(parent, first, last);
}

/*!
    Ends a row removal operation.

    When reimplementing removeRows() in a subclass, you must call this
    function \e after removing data from the model's underlying data
    store.

    \sa beginRemoveRows()
*/
void QAbstractItemModel::endRemoveRows()
{
    Q_D(QAbstractItemModel);
    d->rowsRemoved(d->change.parent, d->change.first, d->change.last);
    emit rowsRemoved(d->change.parent, d->change.first, d->change.last);
}

/*!
    Begins a column insertion operation.

    When reimplementing insertColumns() in a subclass, you must call this
    function \e before inserting data into the model's underlying data
    store.

    The \a parent index corresponds to the parent into which the new
    columns are inserted; \a first and \a last are the column numbers of
    the new columns to be inserted.

    \sa endInsertColumns()
*/
void QAbstractItemModel::beginInsertColumns(const QModelIndex &parent, int first, int last)
{
    Q_D(QAbstractItemModel);
    d->change.parent = parent;
    d->change.first = first;
    d->change.last = last;
    emit columnsAboutToBeInserted(parent, first, last);
    d->columnsAboutToBeInserted(parent, first, last);
}

/*!
    Ends a column insertion operation.

    When reimplementing insertColumns() in a subclass, you must call this
    function \e after inserting data into the model's underlying data
    store.

    \sa beginInsertColumns()
*/
void QAbstractItemModel::endInsertColumns()
{
    Q_D(QAbstractItemModel);
    d->columnsInserted(d->change.parent, d->change.first, d->change.last);
    emit columnsInserted(d->change.parent, d->change.first, d->change.last);
}

/*!
    Begins a column removal operation.

    When reimplementing removeColumns() in a subclass, you must call this
    function \e before removing data from the model's underlying data
    store.

    The \a parent index corresponds to the parent from which the new
    columns are removed; \a first and \a last are the column numbers of the
    columns to be removed.

    \sa endRemoveColumns()
*/
void QAbstractItemModel::beginRemoveColumns(const QModelIndex &parent, int first, int last)
{
    Q_D(QAbstractItemModel);
    d->change.parent = parent;
    d->change.first = first;
    d->change.last = last;
    emit columnsAboutToBeRemoved(parent, first, last);
    d->columnsAboutToBeRemoved(parent, first, last);
}

/*!
    Ends a column removal operation.

    When reimplementing removeColumns() in a subclass, you must call this
    function \e after removing data from the model's underlying data
    store.

    \sa beginRemoveColumns()
*/
void QAbstractItemModel::endRemoveColumns()
{
    Q_D(QAbstractItemModel);
    d->columnsRemoved(d->change.parent, d->change.first, d->change.last);
    emit columnsRemoved(d->change.parent, d->change.first, d->change.last);
}

/*!
    Resets the model to its original state in any attached views.

    When a model is reset it means that any previous data reported from
    the model is now invalid and has to be queried for again.

    When a model radically changes its data it can sometimes be easier
    to just emit this signal rather than using rowsRemoved(),
    dataChanged(), etc.
*/
void QAbstractItemModel::reset()
{
    Q_D(QAbstractItemModel);
    emit modelReset();
    d->reset();
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

    \section1 Subclassing

    When subclassing QAbstractTableModel, you must implement rowCount(),
    columnCount(), and data(). Default implementations of the index() and
    parent() functions are provided by QAbstractTableModel.
    Well behaved models will also implement headerData().

    Editable models need to implement setData(), and implement flags() to
    return a value containing
    \l{Qt::ItemFlags}{Qt::ItemIsEditable}.

    Models that provide interfaces to resizable data structures can
    provide implementations of insertRows(), removeRows(), insertColumns(),
    and removeColumns(). When implementing these functions, it is
    important to emit the appropriate signals so that all connected views
    are aware of any changes:

    \list
    \o An insertRows() implementation must emit rowsInserted() after the
       new rows have been inserted into the data structure.
    \o An insertColumns() implementation must emit columnsInserted() after
       the new columns have been inserted into the data structure.
    \o A removeRows() implementation must emit rowsAboutToBeRemoved()
       \e before the rows are removed from the data structure. This gives
       attached components the chance to take action before any data
       becomes unavailable.
    \o A removeColumns() implementation must emit columnsAboutToBeRemoved().
       \e before the columns are removed from the data structure. This gives
       attached components the chance to take action before any data
       becomes unavailable.
    \endlist

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

bool QAbstractTableModel::hasChildren(const QModelIndex &parent) const
{
    return !parent.isValid();
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

    \section1 Subclassing

    When subclassing QAbstractListModel, you must provide implementations
    of the rowCount() and data() functions. Well behaved models also provide
    a headerData() implementation.

    For editable list models, you must also provide an implementation of
    setData(), implement the flags() function so that it returns a value
    containing \l{Qt::ItemFlags}{Qt::ItemIsEditable}.

    Note that QAbstractListModel provides a default implementation of
    columnCount() that informs views that there is only a single column
    of items in this model.

    Models that provide interfaces to resizable list-like data structures
    can provide implementations of insertRows() and removeRows(). When
    implementing these functions, it is important to emit the appropriate
    signals so that all connected views are aware of any changes:

    \list
    \o An insertRows() implementation must emit rowsInserted() after the
       new rows have been inserted into the data structure.
    \o A removeRows() implementation must emit rowsAboutToBeRemoved()
       \e before the rows are removed from the data structure. This gives
       attached components the chance to take action before any data
       becomes unavailable.
    \endlist

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

bool QAbstractListModel::hasChildren(const QModelIndex &parent) const
{
    return !parent.isValid();
}
