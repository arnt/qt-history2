#include <QtGui>

#include "imagemodel.h"

ImageModel::ImageModel(const QImage &image, QObject *parent)
    : QAbstractTableModel(parent)
{
    modelImage = QImage(image);
}

int ImageModel::rowCount(const QModelIndex & /* parent */) const
{
    return modelImage.height();
}

int ImageModel::columnCount(const QModelIndex & /* parent */) const
{
    return modelImage.width();
}

QVariant ImageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    else if (role == Qt::ToolTipRole)
        return QVariant();

    return qGray(modelImage.pixel(index.column(), index.row()));
}
