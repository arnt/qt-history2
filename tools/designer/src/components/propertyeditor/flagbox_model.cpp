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

#include "flagbox_model_p.h"

FlagBoxModel::FlagBoxModel(QObject *parent)
    : QAbstractItemModel(parent)
{
}

FlagBoxModel::~FlagBoxModel()
{
}

void FlagBoxModel::setItems(const QList<FlagBoxModelItem> &items)
{
    m_items = items;
    emit reset();
}

int FlagBoxModel::rowCount(const QModelIndex &parent) const
{
    return !parent.isValid() ? m_items.count() : 0;
}

int FlagBoxModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QModelIndex FlagBoxModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index);
    return QModelIndex();
}

QModelIndex FlagBoxModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return createIndex(row, column, 0);
}

QCoreVariant FlagBoxModel::data(const QModelIndex &index, int role) const
{
    const FlagBoxModelItem &item = m_items.at(index.row());

    switch (role) {
    case DisplayRole:
    case EditRole:
        return item.name();

    case DecorationRole:
        return item.isChecked();

    default:
        return QCoreVariant();
    } // end switch
}

bool FlagBoxModel::setData(const QModelIndex &index, const QCoreVariant &value, int role)
{
    FlagBoxModelItem &item = m_items[index.row()];

    switch (role) {
    case EditRole:
    case DisplayRole:
        item.setName(value.toString());
        return true;

    case DecorationRole:
        item.setChecked(value.toBool());
        emit dataChanged(index, index);
        return true;

    default:
        return false;
    } // end switch
}
