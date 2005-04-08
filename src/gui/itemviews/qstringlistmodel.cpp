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

/*
  A simple model that uses a QStringList as its data source.
*/

#include "qstringlistmodel.h"

/*!
  \class QStringListModel
  \brief The QStringListModel provides a model for showing strings in a list.

*/

QStringListModel::QStringListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

QStringListModel::QStringListModel(const QStringList &strings, QObject *parent)
    : QAbstractListModel(parent), lst(strings)
{
}

/*!
    Returns the number of items in the string list as the number of rows
    in the model.
*/

int QStringListModel::rowCount(const QModelIndex &parent) const
{
    return lst.count();
}

/*!
    Returns an appropriate value for the requested data.
    If the view requests an invalid index, an invalid variant is returned.
    If a header is requested then we just return the column or row number,
    depending on the orientation of the header.
    Any valid index that corresponds to a string in the list causes that
    string to be returned.
*/

QVariant QStringListModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= lst.size())
        return QVariant();

    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return lst.at(index.row());

    return QVariant();
}

/*!
    Returns an appropriate value for the item's flags. Valid items are
    enabled, selectable, and editable.
*/

Qt::ItemFlags QStringListModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

/*!
    Changes an item in the string list, but only if the following conditions
    are met:

    * The index supplied is valid.
    * The index corresponds to an item to be shown in a view.
    * The role associated with editing text is specified.

    The dataChanged() signal is emitted if the item is changed.
*/

bool QStringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole) {
        lst.replace(index.row(), value.toString());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

/*!
    Inserts a number of rows into the model at the specified position.
*/

bool QStringListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    emit rowsAboutToBeInserted(QModelIndex(), row, row + count - 1);
    
    for (int r = 0; r < count; ++r)
        lst.insert(row, QString());

    emit rowsInserted(QModelIndex(), row, row + count - 1);

    return true;
}

/*!
    Removes a number of rows from the model at the specified position.
*/

bool QStringListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);

    emit rowsAboutToBeRemoved(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        lst.removeAt(row);

    emit rowsRemoved(QModelIndex(), row, row + count - 1);
        
    return true;
}

QStringList QStringListModel::stringList() const
{
    return lst;
}

void QStringListModel::setStringList(const QStringList &strings)
{
    lst = strings;
    emit reset();
}
