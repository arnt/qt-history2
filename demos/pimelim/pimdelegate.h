#ifndef PIMDELEGATE_H
#define PIMDELEGATE_H

#include <qabstractitemdelegate.h>

class PimDelegate : public QAbstractItemDelegate
{
public:
    PimDelegate(QObject *parent = 0);
    ~PimDelegate();

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
};

#endif // PIMDELEGATE_H
