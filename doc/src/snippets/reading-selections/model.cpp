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

#include "model.h"

/*!
    Constructs a table model with at least one row and one column.
*/

TableModel::TableModel(int rows, int columns, QObject *parent)
    : QAbstractTableModel(parent)
{
    QStringList newList;

    for (int column = 0; column < qMax(1, columns); ++column) {
        newList.append("");
    }

    for (int row = 0; row < qMax(1, rows); ++row) {
        rowList.append(newList);
    }
}


/*!
    Returns the number of items in the row list as the number of rows
    in the model.
*/

int TableModel::rowCount() const
{
    return rowList.size();
}

/*!
    Returns the number of items in the first list item as the number of
    columns in the model. All rows should have the same number of columns.
*/

int TableModel::columnCount() const
{
    return rowList[0].size();
}

/*!
    Returns an appropriate value for the requested data.
    If the view requests an invalid index, an invalid variant is returned.
    If a header is requested then we just return the column or row number,
    depending on the orientation of the header.
    Any valid index that corresponds to a string in the list causes that
    string to be returned.
*/

QVariant TableModel::data(const QModelIndex &index, int /* role */) const
{
    if (!index.isValid())
        return QVariant();
    if (index.type() == QModelIndex::HorizontalHeader)
        return QString("Column %1").arg(index.column());
    if (index.type() == QModelIndex::VerticalHeader)
        return QString("Row %1").arg(index.row());

    return rowList[index.row()][index.column()];
}

/*!
    Returns true so that all items in the string list can be edited.
*/

bool TableModel::isEditable(const QModelIndex &/*index*/) const
{
    return true; // all items in the model are editable
}

/*!
    Changes an item in the string list, but only if the following conditions
    are met:

    * The index supplied is valid.
    * The index corresponds to an item to be shown in a view.
    * The role associated with editing text is specified.

    The dataChanged() signal is emitted if the item is changed.
*/

bool TableModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (!index.isValid() || index.type() != QModelIndex::View || role != EditRole)
        return false;

    rowList[index.row()][index.column()] = value.toString();
    emit dataChanged(index, index);
    return true;
}

/*!
    Inserts a number of rows into the model at the specified position.
*/

bool TableModel::insertRows(int position, const QModelIndex &/*index*/, int rows)
{
    int columns = columnCount();

    for (int row = 0; row < rows; ++row) {
        QStringList items;
        for (int column = 0; column < columns; ++column)
            items.append("");
        rowList.insert(position, items);
    }

    emit rowsInserted(QModelIndex(), position, position+rows-1);
    return true;
}

/*!
    Inserts a number of columns into the model at the specified position.
    Each entry in the list is extended in turn with the required number of
    empty strings.
*/

bool TableModel::insertColumns(int position, const QModelIndex &/*index*/, int columns)
{
    int rows = rowCount();

    for (int row = 0; row < rows; ++row) {
        for (int column = position; column < columns; ++column) {
            rowList[row].insert(position, "");
        }
    }

    emit columnsInserted(QModelIndex(), position, position+columns-1);
    return true;
}

/*!
    Removes a number of rows from the model at the specified position.
*/

bool TableModel::removeRows(int position, const QModelIndex &/*index*/, int rows)
{
    emit rowsRemoved(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        rowList.removeAt(position);
    }

    return true;
}

/*!
    Removes a number of columns from the model at the specified position.
    Each row is shortened by the number of columns specified.
*/

bool TableModel::removeColumns(int position, const QModelIndex &/*index*/, int columns)
{
    int rows = rowCount();
    emit columnsRemoved(QModelIndex(), position, position+columns-1);

    for (int row = 0; row < rows; ++row) {
        for (int column = 0; column < columns; ++column) {
            rowList[row].removeAt(position);
        }
    }

    return true;
}
