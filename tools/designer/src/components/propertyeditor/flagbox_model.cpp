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
#include <QtCore/qdebug.h>

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

QVariant FlagBoxModel::data(const QModelIndex &index, int role) const
{
    Q_ASSERT(index.row() != -1);

    const FlagBoxModelItem &item = m_items.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
    case Qt::EditRole:
        return item.name();

    case Qt::CheckStateRole:
        return item.isChecked();

    default:
        return QVariant();
    } // end switch
}

bool FlagBoxModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_ASSERT(index.row() != -1);

    FlagBoxModelItem &item = m_items[index.row()];

    switch (role) {
    case Qt::EditRole:
    case Qt::DisplayRole: {
        item.setName(value.toString());
    } return true;

    case Qt::CheckStateRole: {
        item.setChecked(value.toBool());
        emit dataChanged(index, index);
    } return true;

    default: break;
    } // end switch

    return false;
}
