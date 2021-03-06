/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
    A simple model that uses a QStringList as its data source.
*/

#include "qstringlistmodel.h"

#ifndef QT_NO_STRINGLISTMODEL

QT_BEGIN_NAMESPACE

/*!
    \class QStringListModel
    \brief The QStringListModel class provides a model that supplies strings to views.

    \ingroup model-view
    \mainclass

    QStringListModel is an editable model that can be used for simple
    cases where you need to display a number of strings in a view
    widget, such as a QListView or a QComboBox.

    The model provides all the standard functions of an editable
    model, representing the data in the string list as a model with
    one column and a number of rows equal to the number of items in
    the list.

    Model indexes corresponding to items are obtained with the
    \l{QAbstractListModel::index()}{index()} function, and item flags
    are obtained with flags().  Item data is read with the data()
    function and written with setData().  The number of rows (and
    number of items in the string list) can be found with the
    rowCount() function.

    The model can be constructed with an existing string list, or
    strings can be set later with the setStringList() convenience
    function. Strings can also be inserted in the usual way with the
    insertRows() function, and removed with removeRows(). The contents
    of the string list can be retrieved with the stringList()
    convenience function.

    An example usage of QStringListModel:

    \quotefromfile snippets/qstringlistmodel/main.cpp
    \skipto QStringListModel
    \printuntil model->setStringList(list)

    \sa QAbstractListModel, QAbstractItemModel, {Model Classes}
*/

/*!
    Constructs a string list model with the given \a parent.
*/

QStringListModel::QStringListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

/*!
    Constructs a string list model containing the specified \a strings
    with the given \a parent.
*/

QStringListModel::QStringListModel(const QStringList &strings, QObject *parent)
    : QAbstractListModel(parent), lst(strings)
{
}

/*!
    Returns the number of rows in the model. This value corresponds to the
    number of items in the model's internal string list.

    The optional \a parent argument is in most models used to specify
    the parent of the rows to be counted. Because this is a list if a
    valid parent is specified, the result will always be 0.

    \sa insertRows(), removeRows(), QAbstractItemModel::rowCount()
*/

int QStringListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return lst.count();
}

/*!
    Returns data for the specified \a role, from the item with the
    given \a index.

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
    Returns the flags for the item with the given \a index.

    Valid items are enabled, selectable, and editable.

    \sa QAbstractItemModel::flags()
*/

Qt::ItemFlags QStringListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;

    return QAbstractItemModel::flags(index) | Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

/*!
    Sets the data for the specified \a role in the item with the given
    \a index in the model, to the provided \a value.

    The dataChanged() signal is emitted if the item is changed.

    \sa Qt::ItemDataRole, data()
*/

bool QStringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.row() >= 0 && index.row() < lst.size()
        && (role == Qt::EditRole || role == Qt::DisplayRole)) {
        lst.replace(index.row(), value.toString());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

/*!
    Inserts \a count rows into the model, beginning at the given \a row.

    The \a parent index of the rows is optional and is only used for
    consistency with QAbstractItemModel. By default, a null index is
    specified, indicating that the rows are inserted in the top level of
    the model.

    \sa QAbstractItemModel::insertRows()
*/

bool QStringListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (count < 1 || row < 0 || row > rowCount(parent))
        return false;

    beginInsertRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        lst.insert(row, QString());

    endInsertRows();

    return true;
}

/*!
    Removes \a count rows from the model, beginning at the given \a row.

    The \a parent index of the rows is optional and is only used for
    consistency with QAbstractItemModel. By default, a null index is
    specified, indicating that the rows are removed in the top level of
    the model.

    \sa QAbstractItemModel::removeRows()
*/

bool QStringListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    if (count <= 0 || row < 0 || (row + count) > rowCount(parent))
        return false;

    beginRemoveRows(QModelIndex(), row, row + count - 1);

    for (int r = 0; r < count; ++r)
        lst.removeAt(row);

    endRemoveRows();

    return true;
}

/*!
  \reimp
*/
void QStringListModel::sort(int, Qt::SortOrder order)
{
    emit layoutAboutToBeChanged();
    if (order == Qt::AscendingOrder)
        qSort(lst.begin(), lst.end(), qLess<QString>());
    else
        qSort(lst.begin(), lst.end(), qGreater<QString>());
    emit layoutChanged();
}

/*!
    Returns the string list used by the model to store data.
*/
QStringList QStringListModel::stringList() const
{
    return lst;
}

/*!
    Sets the model's internal string list to \a strings. The model will
    notify any attached views that its underlying data has changed.

    \sa dataChanged()
*/
void QStringListModel::setStringList(const QStringList &strings)
{
    lst = strings;
    reset();
}

/*!
  \reimp
*/
Qt::DropActions QStringListModel::supportedDropActions() const
{
    return QAbstractItemModel::supportedDropActions() | Qt::MoveAction;
}

QT_END_NAMESPACE

#endif // QT_NO_STRINGLISTMODEL
