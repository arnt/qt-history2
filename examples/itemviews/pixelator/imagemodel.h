//depot/qt/main/examples/itemviews/pixelator/imagemodel.h#1 - branch change 154710 (text)
#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

#include <QAbstractTableModel>
#include <QImage>

class ImageModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ImageModel::ImageModel(const QImage &image, QObject *parent = 0);

    int rowCount() const;
    int columnCount() const;

    QVariant data(const QModelIndex &index, int role) const;

private:
    QImage modelImage;
};

#endif
