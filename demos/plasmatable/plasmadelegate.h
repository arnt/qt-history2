#ifndef PLASMADELEGATE_H
#define PLASMADELEGATE_H

#include <qabstractitemdelegate.h>

class PlasmaDelegate : public QAbstractItemDelegate
{
public:
    PlasmaDelegate(QObject *parent = 0);
    ~PlasmaDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QAbstractItemModel *model, const QModelIndex &index) const;
    QSize sizeHint(const QFontMetrics &fontMetrics, const QStyleOptionViewItem &option,
                   const QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif // PLASMADELEGATE_H
