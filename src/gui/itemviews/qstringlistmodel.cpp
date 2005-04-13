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
  \brief The QStringListModel provides a model that supplies strings to views.

  \ingroup model-view
  \mainclass

  QStringListModel is an editable model that can be used for simple cases
  where you need to display a number of strings in a view widget, such as
  QListView or QComboBox.

  The model can be constructed

  \sa QAbstractListModel QAbstractItemModel
*/

/*!
  Constructs a string list model containing the specified \a strings with
  the given \a parent.
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
    Returns the number of rows in the model. This value corresponds to the
    number of items in the model's internal string list.

    \sa insertRows() removeRows()
*/

int QStringListModel::rowCount(const QModelIndex &parent) const
{
    return lst.count();
}

/*!
    Returns data from the item with the given \a index for the specified \a role.

    If the view requests an invalid index, an invalid variant is returned.

    \sa setData()
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
    Returns the flags for the item that corresponds to the given \a index.

    Valid items are enabled, selectable, and editable.
*/

Qt::ItemFlags QStringListModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

/*!
    Sets the data for the item corresponding to the given \a index in the
    model to the \a value for the specifed \a role.

    The dataChanged() signal is emitted if the item is changed.

    \sa data()
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
    Inserts \a count rows into the model beginning at the given \a row.

    The \a parent index of the rows is optional and is only used for
    consistency with QAbstractItemModel. By default, a null index is
    specified, indicating that the rows are inserted in the top level of
    the model.

    \sa QAbstractItemModel::insertRows()
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
    Removes \a count rows from the model beginning at the given \a row.

    The \a parent index of the rows is optional and is only used for
    consistency with QAbstractItemModel. By default, a null index is
    specified, indicating that the rows are removed in the top level of
    the model.

    \sa QAbstractItemModel::removeRows()
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
