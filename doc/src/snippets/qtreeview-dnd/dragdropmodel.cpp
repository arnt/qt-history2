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

#include <QtGui>

#include "dragdropmodel.h"

DragDropModel::DragDropModel(const QStringList &strings, QObject *parent)
    : TreeModel(strings, parent)
{
}

bool DragDropModel::dropMimeData(const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    qDebug() << data->formats();
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("text/plain"))
        return false;

    if (column > 0)
        return false;

    int beginRow;

    if (!parent.isValid())
        beginRow = row;
    else
        beginRow = parent.row();

    QByteArray encodedData = data->data("text/plain");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;
    int rows = 0;

    while (!stream.atEnd()) {
        QString text;
        stream >> text;
        newItems << text;
        ++rows;
    }

    insertRows(beginRow, rows, QModelIndex());
    foreach (QString text, newItems) {
        QModelIndex idx = index(beginRow, 0, QModelIndex());
        setData(idx, text);
    }

    return true;
}

Qt::ItemFlags DragDropModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = TreeModel::flags(index);

    if (index.isValid())
        return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
    else
        return Qt::ItemIsDropEnabled | defaultFlags;
}

QMimeData *DragDropModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes) {
        if (index.isValid()) {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << text;
        }
    }

    mimeData->setData("text/plain", encodedData);
    return mimeData;
}

QStringList DragDropModel::mimeTypes() const
{
    QStringList types;
    types << "text/plain";
    return types;
}

Qt::DropActions DragDropModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}
