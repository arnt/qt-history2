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

#include "imagemodel.h"

ImageModel::ImageModel(const QString &fileName, QObject *parent)
    : QAbstractTableModel(parent)
{
    image.load(fileName);
}

ImageModel::~ImageModel()
{

}

int ImageModel::rowCount() const
{
    return image.height();
}

int ImageModel::columnCount() const
{
    return image.width();
}

QVariant ImageModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && role == QAbstractItemModel::DisplayRole)
        return image.pixel(index.column(), index.row());
    return QVariant();
}

bool ImageModel::setData(const QModelIndex &index, int role, const QVariant &value)
{
    if (index.isValid() && role == QAbstractItemModel::EditRole) {
        image.setPixel(index.column(), index.row(), value.toInt());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

QAbstractItemModel::ItemFlags ImageModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | ItemIsEditable;
}
