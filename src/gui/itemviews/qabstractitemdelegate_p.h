#ifndef QABSTRACTITEMDELEGATE_P_H
#define QABSTRACTITEMDELEGATE_P_H

#include <private/qobject_p.h>

class QGenericItemModel;

class QAbstractItemDelegatePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractItemDelegate);

public:
    QAbstractItemModel *model;
};

#endif
