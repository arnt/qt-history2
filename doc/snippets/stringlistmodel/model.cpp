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

  A simple model that uses a QStringList as its data source.
*/

#include "model.h"

/*!
    Returns the number of items in the string list as the number of rows
    in the model.
*/

int StringListModel::rowCount() const
{
    return stringList.count();
}

/*!
    Returns an appropriate value for the requested data.
    If the view requests an invalid index, an invalid variant is returned.
    If a header is requested then we just return the column or row number,
    depending on the orientation of the header.
    Any valid index that corresponds to a string in the list causes that
    string to be returned.
*/

QVariant StringListModel::data(const QModelIndex &index, int /* role */) const
{
    if (!index.isValid())
        return QVariant();
    if (index.type() == QModelIndex::HorizontalHeader)
        return index.column();
    if (index.type() == QModelIndex::VerticalHeader)
        return index.row();
    return stringList.at(index.row());
}

/*!
    Returns true so that all items in the string list can be edited.
*/

bool StringListModel::isEditable(const QModelIndex &/*index*/) const
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

bool StringListModel::setData(const QModelIndex &index, int role,
                              const QVariant &value)
{
    if (index.isValid() && index.type() == QModelIndex::View &&
        role == EditRole) {

        stringList.replace(index.row(), value.toString());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

/*!
    Inserts a number of rows into the model at the specified position.
*/

bool StringListModel::insertRows(int position, const QModelIndex &/*index*/,
                                 int rows)
{
    for (int row = 0; row < rows; ++row) {
        stringList.insert(position, "");
    }

    emit rowsInserted(QModelIndex(), position, position+rows-1);
    return true;
}

/*!
    Removes a number of rows from the model at the specified position.
*/

bool StringListModel::removeRows(int position, const QModelIndex &/*index*/,
                                 int rows)
{
    emit rowsRemoved(QModelIndex(), position, position+rows-1);

    for (int row = 0; row < rows; ++row) {
        stringList.removeAt(position);
    }

    return true;
}
