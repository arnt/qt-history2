#ifndef PIXELDELEGATE_H
#define PIXELDELEGATE_H

#include <QAbstractItemDelegate>
#include <QFontMetrics>
#include <QModelIndex>
#include <QSize>

class QAbstractItemModel;
class QObject;
class QPainter;

static const int ItemSize = 256;
static const int PixelSize = 12;

class PixelDelegate : public QAbstractItemDelegate
{
    Q_OBJECT

public:
    PixelDelegate::PixelDelegate(QObject *parent = 0)
        : QAbstractItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
        const QAbstractItemModel *model, const QModelIndex &index) const;

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QAbstractItemModel *model,
                   const QModelIndex &index ) const;
};

#endif
