//depot/qt/main/examples/itemviews/pixelator/imagemodel.cpp#1 - branch change 154710 (text)
#include <QtGui>

#include "imagemodel.h"

ImageModel::ImageModel(const QImage &image, QObject *parent)
    : QAbstractTableModel(parent)
{
    modelImage = QImage(image);
}

int ImageModel::rowCount() const
{
    return modelImage.height();
}

int ImageModel::columnCount() const
{
    return modelImage.width();
}

QVariant ImageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    else if (role == QAbstractItemModel::ToolTipRole)
        return QVariant();

    return qGray(modelImage.pixel(index.column(), index.row()));
}
