#ifndef QITEMSELECTIONMODEL_P_H
#define QITEMSELECTIONMODEL_P_H

#include <private/qobject_p.h>

class QItemSelectionModelPrivate: public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QItemSelectionModel);
public:
    QItemSelectionModelPrivate()
        : selectionMode(QItemSelectionModel::Multi), toggleState(false) {}

    inline void remove(QList<QItemSelectionRange> &r)
    {
        QList<QItemSelectionRange>::const_iterator it = r.constBegin();
        for (; it != r.constEnd(); ++it)
            ranges.removeAll(*it);
    }

    QItemSelection expandRows(const QItemSelection &selection) const;

    QAbstractItemModel *model;
    QItemSelectionModel::SelectionMode selectionMode;
    QItemSelection ranges;
    QItemSelection currentSelection;
    QModelIndex currentItem;
    bool toggleState;
};

#endif
