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

  A simple model that uses a QVector as its data source.
*/

#include "model.h"

/*!
    Returns the number of items in the string list as the number of rows
    in the model.
*/

int LinearModel::rowCount(const QModelIndex &/*parent*/) const
{
    return values.count();
}

/*
    Returns an appropriate value for the requested data.
    If the view requests an invalid index, an invalid variant is returned.
    If a header is requested then we just return the column or row number,
    depending on the orientation of the header.
    Any valid index that corresponds to a string in the list causes that
    string to be returned.
*/

/*!
    Returns a model index for other component to use when referencing the
    item specified by the given row, column, and type. The parent index
    is ignored.
*/

QModelIndex LinearModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent == QModelIndex() && row >= 0 && row < rowCount()
        && column == 0)
        return createIndex(row, column, 0);
    else
        return QModelIndex();
}

QVariant LinearModel::data(const QModelIndex &index, int role) const
{
    Q_UNUSED(role);

    if (!index.isValid())
        return QVariant();

    return values.at(index.row());
}

/*!
    Returns ItemIsEditable so that all items in the vector can be edited.
*/

QAbstractItemModel::ItemFlags LinearModel::flags(const QModelIndex &index) const
{
    return QAbstractListModel::flags(index)|QAbstractListModel::ItemIsEditable; // all items in the model are editable
}

/*!
    Changes an item in the string list, but only if the following conditions
    are met:

    * The index supplied is valid.
    * The index corresponds to an item to be shown in a view.
    * The role associated with editing text is specified.

    The dataChanged() signal is emitted if the item is changed.
*/

bool LinearModel::setData(const QModelIndex &index,
                          const QVariant &value, int role)
{
    if (!index.isValid() || role != EditRole)
        return false;
    values.replace(index.row(), value.toInt());
    emit dataChanged(index, index);
    return true;
}

/*!
    Inserts a number of rows into the model at the specified position.
*/

bool LinearModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    values.insert(position, rows, 0);

    emit rowsInserted(QModelIndex(), position, position+rows-1);
    return true;
}

/*!
    Removes a number of rows from the model at the specified position.
*/

bool LinearModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    emit rowsAboutToBeRemoved(QModelIndex(), position, position+rows-1);
    values.remove(position, rows);

    return true;
}
