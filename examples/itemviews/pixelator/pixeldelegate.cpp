#include <QtGui>

#include "pixeldelegate.h"

void PixelDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const
{
    painter->setBrush(QBrush(Qt::white));
    painter->setPen(Qt::NoPen);
    painter->drawRect(option.rect);
    painter->setBrush(QBrush(Qt::black));

    int size = qMin(option.rect.width(), option.rect.height());
    int brightness = index.model()->data(index,
        QAbstractItemModel::DisplayRole).toInt();
    double radius = size/2.0 - (brightness/255.0 * size/2.0);

    painter->save();
    painter->translate(option.rect.x() + option.rect.width()/2 - radius,
        option.rect.y() + option.rect.height()/2 - radius);
    painter->scale(2*radius/100.0, 2*radius/100.0);
    painter->drawEllipse(0, 0, 100, 100);
    painter->restore();
}

QSize PixelDelegate::sizeHint(const QStyleOptionViewItem & /* option */,
                              const QModelIndex & /* index */) const
{
    return QSize(PixelSize, PixelSize);
}
