/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
    model.cpp

    Provides a table model for use in various examples.
*/

#include <QtGui>

#include "model.h"

/*!
    Constructs a table model with at least one row and one column.
*/

TableModel::TableModel(int rows, int columns, QObject *parent)
    : QAbstractTableModel(parent)
{
    QVector<int> newRow(qMax(1, columns), 0);

    for (int row = 0; row < qMax(1, rows); ++row) {
        rowList.append(newRow);
    }
}


/*!
    Returns the number of items in the row list as the number of rows
    in the model.
*/

int TableModel::rowCount(const QModelIndex &/*parent*/) const
{
    return rowList.size();
}

/*!
    Returns the number of items in the first list item as the number of
    columns in the model. All rows should have the same number of columns.
*/

int TableModel::columnCount(const QModelIndex &/*parent*/) const
{
    return rowList[0].size();
}

/*!
    Returns an appropriate value for the requested data.
    If the view requests an invalid index, an invalid variant is returned.
    Any valid index that corresponds to a string in the list causes that
    string to be returned for the display role; otherwise an invalid variant
    is returned.
*/

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == DisplayRole)
        return rowList[index.row()][index.column()];
    else
        return QVariant();
}

/*!
    Returns the appropriate header string depending on the orientation of
    the header and the section. If anything other than the display role is
    requested, we return an invalid variant.
*/

QVariant TableModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
    if (role != DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
        return QString("Column %1").arg(section);
    else
        return QString("Row %1").arg(section);
}

/*!
    Returns an appropriate value for the item's flags. Valid items are
    enabled, selectable, and editable.
*/

QAbstractItemModel::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return ItemIsEnabled;

    return ItemIsEnabled | ItemIsSelectable | ItemIsEditable;
}

/*!
    Changes an item in the model, but only if the following conditions
    are met:

    * The index supplied is valid.
    * The role associated with editing text is specified.

    The dataChanged() signal is emitted if the item is changed.
*/

bool TableModel::setData(const QModelIndex &index,
                         const QVariant &value, int role)
{
    if (!index.isValid() || role != EditRole)
        return false;

    rowList[index.row()][index.column()] = value.toInt();
    emit dataChanged(index, index);
    return true;
}

/*!
    Inserts a number of rows into the model at the specified position.
*/

bool TableModel::insertRows(int position, int rows, const QModelIndex &/*parent*/)
{
    int columns = columnCount();

    for (int row = 0; row < rows; ++row) {
        QVector<int> newRow(columns, 0);
        rowList.insert(position, newRow);
    }

    emit rowsInserted(QModelIndex(), position, position+rows-1);
    return true;
}

/*!
    Inserts a number of columns into the model at the specified position.
    Each entry in the list is extended in turn with the required number of
    default valued integers.
*/

bool TableModel::insertColumns(int position, int columns, const QModelIndex &/*parent*/)
{
    int rows = rowCount();

    for (int row = 0; row < rows; ++row) {
        rowList[row].insert(position, qMax(0, columns), 0);
    }

    emit columnsInserted(QModelIndex(), position, position+columns-1);
    return true;
}

/*!
    Removes a number of rows from the model at the specified position.
*/

bool TableModel::removeRows(int position, int rows, const QModelIndex &/*parent*/)
{
    emit rowsAboutToBeRemoved(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        rowList.removeAt(position);
    }

    return true;
}

/*!
    Removes a number of columns from the model at the specified position.
    Each row is shortened by the number of columns specified.
*/

bool TableModel::removeColumns(int position, int columns, const QModelIndex &/*parent*/)
{
    int rows = rowCount();
    emit columnsAboutToBeRemoved(QModelIndex(), position, position+columns-1);

    for (int row = 0; row < rows; ++row) {
        rowList[row].remove(position, qMax(0, columns));
    }

    return true;
}
