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

#include "qstandarditemmodel.h"
#include <qpair.h>
#include <qvariant.h>
#include <qvector.h>

#include <private/qstandarditemmodel_p.h>
#include <qdebug.h>

/*!
    \class QStandardItemModel
    \brief The QStandardItemModel provides a generic model for storing custom data.

    QStandardItemModel provides a model that that can be used as a repository
    for standard Qt data types. Data is written to the model and read back using
    the standard QAbstractItemModel interface. The way each item of information
    is referenced depends on how the data is inserted into the model.

    For performance and flexibility, you may want to subclass QAbstractItemModel
    to provide support for different kinds of repositories. For example, the
    QDirModel provides a model interface to the underlying file system, and does
    not actually store file information internally.

    \sa \link model-view-programming.html Model/View Programming\endlink QAbstractItemModel

*/

/*!
    Creates an empty model that contains no rows or columns with the given
    \a parent.
*/
QStandardItemModel::QStandardItemModel(QObject *parent)
    : QAbstractItemModel(*new QStandardItemModelPrivate(0, 0), parent)
{
    // nothing
}

/*!
    Creates a model with \a rows number of rows and \a columns number of columns.
*/
QStandardItemModel::QStandardItemModel(int rows, int columns, QObject *parent)
    : QAbstractItemModel(*new QStandardItemModelPrivate(rows, columns), parent)
{
    // nothing
}


/*!
    Destroys the model.
*/
QStandardItemModel::~QStandardItemModel()
{
    Q_D(QStandardItemModel);
    for (int i=0; i<d->topLevelRows.count(); ++i)
        delete d->topLevelRows.at(i);
    d->topLevelRows.clear();
    for (int i=0; i<d->horizontalHeader.count(); ++i)
        delete d->horizontalHeader.at(i);
    d->horizontalHeader.clear();
    for (int i=0; i<d->verticalHeader.count(); ++i)
        delete d->verticalHeader.at(i);
    d->verticalHeader.clear();
}

/*!
    Returns a model index for the given \a row, \a column, and \a parent.
*/
QModelIndex QStandardItemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_D(const QStandardItemModel);
    if (hasIndex(row, column, parent)) {
        if (parent.isValid()) {
            QStdModelRow *parentRow = d->containedRow(parent, false);
            return createIndex(row, column, parentRow);
        } else {
            return createIndex(row, column, 0);
        }
    }
    return QModelIndex();
}

/*!
    Returns a model index for the parent of the \a child item.
*/
QModelIndex QStandardItemModel::parent(const QModelIndex &child) const
{
    Q_D(const QStandardItemModel);
    if (child.isValid() && child.internalPointer()) {
        QStdModelRow *parentRow = static_cast<QStdModelRow*>(child.internalPointer());
        QStdModelRow *grandParentRow = parentRow ? parentRow->p : 0;
        QVector<QStdModelRow*> grandParentChildren = d->topLevelRows;
        if (grandParentRow)
            grandParentChildren = grandParentRow->childrenRows;
        // ### slow, use ptr trick
        int row = grandParentChildren.indexOf(parentRow);
        return createIndex(row, 0, grandParentRow);
    }
    return QModelIndex();
}

/*!
    Returns the number of rows in the model that contain items with the given
    \a parent.

*/
int QStandardItemModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const QStandardItemModel);
    QStdModelRow *modelRow = d->containedRow(parent, false);
    if (modelRow)
        return modelRow->childrenRows.count();

    return d->topLevelRows.count();
}

/*!
    Returns the number of columns in the model that contain items with the given
    \a parent.
*/
int QStandardItemModel::columnCount(const QModelIndex &parent) const
{
    Q_D(const QStandardItemModel);
    QStdModelRow *modelRow = d->containedRow(parent, false);
    if (modelRow)
        return modelRow->childrenColumns;

    return d->topLevelColumns;
}

/*!
    Returns true if the \a parent model index has child items; otherwise returns
    false.
*/
bool QStandardItemModel::hasChildren(const QModelIndex &parent) const
{
    Q_D(const QStandardItemModel);
    if (parent.isValid()) {
        QStdModelRow *modelRow = d->containedRow(parent, false);
        if (modelRow)
            return modelRow->childrenRows.count() && modelRow->childrenColumns;
    }
    return false;
}

/*!
    Returns the data for the given \a index and \a role.
*/
QVariant QStandardItemModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QStandardItemModel);
    role = (role == Qt::EditRole ? Qt::DisplayRole : role);
    if (index.isValid()) {
        QStdModelRow *modelRow = d->containedRow(index, false);
        if (modelRow && modelRow->items.count() > index.column()) {
            QStdModelItem *item = modelRow->items.at(index.column());
            if (item)
                return item->value(role);
        }
    }
    return QVariant();
}

/*!
    Sets the data for the given \a index and \a role to the \a value specified.
*/
bool QStandardItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_D(const QStandardItemModel);
    role = (role == Qt::EditRole ? Qt::DisplayRole : role);
    if (index.isValid()) {
        QStdModelRow *modelRow = d->containedRow(index, true);
        int count = modelRow->items.count();
        // make room for enough items
        if (count <= index.column())
            modelRow->items.insert(count, index.column() + 1 - count, 0);
        // make sure we have a QStdModelItem at the position
        if (!modelRow->items.at(index.column()))
            modelRow->items[index.column()] = new QStdModelItem();
        modelRow->items.at(index.column())->setValue(role, value);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

/*!
  \reimp
*/
QVariant QStandardItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_D(const QStandardItemModel);
    if (section < 0
        || (orientation == Qt::Horizontal && section >= columnCount())
        || (orientation == Qt::Vertical && section >= rowCount()))
        return QVariant();

    const QStdModelItem *headerItem = 0;
    const QVector<QStdModelItem*> &header = (orientation == Qt::Horizontal
                                             ? d->horizontalHeader : d->verticalHeader);
    role = (role == Qt::EditRole ? Qt::DisplayRole : role);
    if (header.size() > section)
        headerItem = header.at(section);

    if (headerItem)
        return headerItem->value(role);

    return QAbstractItemModel::headerData(section, orientation, role);
}

/*!
  \reimp
*/
bool QStandardItemModel::setHeaderData(int section, Qt::Orientation orientation,
                                       const QVariant &value, int role)
{
    Q_D(QStandardItemModel);
    if (section < 0
        || (orientation == Qt::Horizontal && section >= columnCount())
        || (orientation == Qt::Vertical && section >= rowCount()))
        return false;

    QStdModelItem *headerItem = 0;
    QVector<QStdModelItem*> &header = (orientation == Qt::Horizontal
                                       ? d->horizontalHeader : d->verticalHeader);
    role = (role == Qt::EditRole ? Qt::DisplayRole : role);
    if (header.size() <= section)
        header.resize(section + 1);
    headerItem = header[section];
    if (!headerItem) {
        headerItem = new QStdModelItem();
        header.replace(section, headerItem);
    }

    headerItem->setValue(role, value);
    emit  headerDataChanged(orientation, section, section);
    return true;
}


/*!
Inserts \a count rows into the model, creating new items as children of
the given \a parent. The new rows are inserted before the \a row specified.

If \a row is 0, the rows are prepended to any existing rows in the parent.
If \a row is columnCount(), the rows are appended to any existing rows in
the parent.
If \a parent has no children, a single column with \a count rows is inserted.

Returns true if the rows were successfully inserted; otherwise returns false.
*/
bool QStandardItemModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_D(QStandardItemModel);
    if (count < 1 || row < 0 || row > rowCount(parent))
        return false;

    QVector<QStdModelRow*> &rows = (parent.isValid()) ? d->containedRow(parent, true)->childrenRows
                               : d->topLevelRows;

    if (!parent.isValid() && d->verticalHeader.size() > row)
        d->verticalHeader.insert(row, count, 0);

    emit rowsAboutToBeInserted(parent, row, row + count - 1);

    rows.insert(row, count, 0);

    emit rowsInserted(parent, row, row + count - 1);
    
    return true;
}

/*!
    Inserts \a count columns into the model, creating new items as children of
    the given \a parent. The new columns are inserted before the \a column
    specified.

    If \a column is 0, the columns are prepended to any existing columns in the
    parent.
    If \a column is columnCount(), the columns are appended to any existing
    columns in the parent.
    If \a parent has no children, a single row with \a count columns is inserted.

    Returns true if the columns were successfully inserted; otherwise returns
    false.
*/
bool QStandardItemModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    Q_D(QStandardItemModel);
    if (count < 0 || column < 0 || column > columnCount(parent))
        return false;

    emit columnsAboutToBeInserted(parent, column, column + count - 1);

    QVector<QStdModelRow*> *rows = &d->topLevelRows;
    // update the column counters
    if (parent.isValid()) {
        QStdModelRow *modelRow = d->containedRow(parent, true);
        modelRow->childrenColumns += count;
        rows = &modelRow->childrenRows;
    } else {
        d->topLevelColumns += count;
        if (d->horizontalHeader.size() > column)
            d->horizontalHeader.insert(column, count, 0);
    }

    // update any item vectors if needed
    for (int i=0; i<rows->count(); ++i) {
        QStdModelRow *modelRow = rows->at(i);
        // only insert if there is a QStdModelRow and QStdModelItems in it
        if (modelRow && modelRow->items.count()
              && modelRow->items.count() > column)
            modelRow->items.insert(column, count, 0);
    }

    emit columnsInserted(parent, column, column + count - 1);

    return true;
}

/*!
    Removes \a count rows from the model, starting with the given
    \a row.
    The items removed are children of the item represented by the \a parent
    model index.

    Returns true if the rows were successfully removed; otherwise returns
    false.
*/
bool QStandardItemModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_D(QStandardItemModel);
    if (count < 1 || row < 0 || (row + count) > rowCount(parent))
        return false;

    emit rowsAboutToBeRemoved(parent, row, row + count - 1);

    QVector<QStdModelRow*> &rows = (parent.isValid()) ? d->containedRow(parent, false)->childrenRows
                               : d->topLevelRows;

    if (!parent.isValid() && d->verticalHeader.count() > row) {
        int headerCount = qMin(d->verticalHeader.count() - row, count);
        for (int i=row; i < (row+headerCount); ++i)
            delete d->verticalHeader.at(i);
        d->verticalHeader.remove(row, headerCount);
    }

    // delete QStdModelRows
    for (int i=row; i<(row+count); ++i)
        delete rows.at(i);
    // resize row vector
    rows.remove(row, count);

    emit rowsRemoved(parent, row, row + count - 1);

    return true;
}

/*!
    Removes \a count columns from the model, starting with the given
    \a column.
    The items removed are children of the item represented by the \a parent
    model index.

    Returns true if the columns were successfully removed; otherwise returns
    false.
*/
bool QStandardItemModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    Q_D(QStandardItemModel);
    if (count < 1 || column < 0 || (column + count) > columnCount(parent))
        return false;

    emit columnsAboutToBeRemoved(parent, column, column + count - 1);

    QVector<QStdModelRow*> *rows = &d->topLevelRows;
    // update the column counters
    if (parent.isValid()) {
        QStdModelRow *modelRow = d->containedRow(parent, true);
        modelRow->childrenColumns -= count;
        rows = &modelRow->childrenRows;
    } else {
        d->topLevelColumns -= count;
        if (d->horizontalHeader.count() > column) {
            int headerCount = qMin(d->horizontalHeader.size() - column, count);
            for (int i=column; i < (column+headerCount); ++i)
                delete d->horizontalHeader.at(i);
            d->horizontalHeader.remove(column, headerCount);
        }
    }

    // iterate over all child rows and remove if needed
    for (int i=0; i<rows->count(); ++i) {
        QStdModelRow *modelRow = rows->at(i);
        if (modelRow && column < modelRow->items.count()) {
            // delete the QStdModelItem if any
            for (int j=column; j<(column+count) && j<modelRow->items.count(); ++j)
                delete modelRow->items.at(j);
            // resize item vector
            modelRow->items.remove(column, qMin(count, modelRow->items.count() - column));
        }
    }

    emit columnsRemoved(parent, column, column + count - 1);

    return true;
}

/*!
    Returns the item flags for the given \a index.

    This model returns returns a combination of flags that
    enables the item (\c ItemIsEnabled), allows it to be
    selected (\c ItemIsSelectable) and edited (\c ItemIsEditable).

    \sa ItemFlag
*/
Qt::ItemFlags QStandardItemModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

/*!
    \internal
*/
QStdModelRow *QStandardItemModelPrivate::containedRow(const QModelIndex &index,
                                                  bool createIfMissing) const
{
    if (!index.isValid())
        return 0;

    QStdModelRow *parentRow = static_cast<QStdModelRow*>(index.internalPointer());
    QVector<QStdModelRow*> *rowList = const_cast<QVector<QStdModelRow*> *>(&topLevelRows);
    if (parentRow) {
        rowList = const_cast<QVector<QStdModelRow*> *>(&parentRow->childrenRows);
    }

    if (createIfMissing && !rowList->at(index.row()))
        rowList->replace(index.row() , new QStdModelRow(parentRow));
    return rowList->at(index.row());
}
