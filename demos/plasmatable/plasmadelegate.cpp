#include "plasmadelegate.h"
#include <qabstractitemmodel.h>
#include <qpainter.h>

PlasmaDelegate::PlasmaDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

PlasmaDelegate::~PlasmaDelegate()
{

}

void PlasmaDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QAbstractItemModel *model, const QModelIndex &index) const
{
    unsigned char s = option.state & QStyle::Style_Selected ? 63 : 0;
    unsigned int color = model->data(index, QAbstractItemModel::Role_Display).toInt();
    unsigned char r = ((color & 0x00FF0000) >> 16) + s;
    unsigned char g = ((color & 0x0000FF00) >> 8) + s;
    unsigned char b = (color & 0x000000FF) + s;

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(r, g, b));
    painter->drawRect(option.rect);
}

QSize PlasmaDelegate::sizeHint(const QFontMetrics &, const QStyleOptionViewItem &,
                               const QAbstractItemModel *, const QModelIndex &) const
{
    return QSize(4, 4);
}
