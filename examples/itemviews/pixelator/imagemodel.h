#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

#include <QAbstractTableModel>
#include <QImage>

class ImageModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ImageModel::ImageModel(const QImage &image, QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;

private:
    QImage modelImage;
};

#endif
