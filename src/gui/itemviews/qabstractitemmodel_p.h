#ifndef QABSTRACTITEMMODEL_P_H
#define QABSTRACTITEMMODEL_P_H

#include <private/qobject_p.h>

class QPersistentModelIndexData
{
public:
    QPersistentModelIndexData() : model(0) {}
    QAbstractItemModel *model;
    QModelIndex index;
    QAtomic ref;
    static QPersistentModelIndexData shared_null;
};

class QAbstractItemModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractItemModel)

public:
    
    QList<QPersistentModelIndexData*> persistentIndices;
};

#endif // QABSTRACTITEMMODEL_P_H
