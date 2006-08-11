/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>
#include "imagemodel.h"

ImageModel::ImageModel(QObject *parent) : QAbstractTableModel(parent)
{
}

void ImageModel::setImage(const QImage &image)
{
    modelImage = QImage(image);
    reset();
}

int ImageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return modelImage.height();
}

int ImageModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return modelImage.width();
}

QVariant ImageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();
    return qGray(modelImage.pixel(index.column(), index.row()));
}

QVariant ImageModel::headerData(int section, Qt::Orientation orientation, int role) const {
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    if (role == Qt::SizeHintRole)
        return QSize(1, 1);
    return QVariant();
}

