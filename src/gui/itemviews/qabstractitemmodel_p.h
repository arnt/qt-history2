#ifndef QABSTRACTITEMMODEL_P_H
#define QABSTRACTITEMMODEL_P_H

#include <private/qobject_p.h>

class QAbstractItemModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractItemModel)
public:
    
    QList<QPersistentModelIndexPrivate*> persistentIndices;
};

#endif // QABSTRACTITEMMODEL_P_H
