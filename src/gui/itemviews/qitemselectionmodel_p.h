#ifndef QITEMSELECTIONMODEL_P_H
#define QITEMSELECTIONMODEL_P_H

#include <private/qobject_p.h>

class QItemSelectionModelPrivate: public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QItemSelectionModel)
public:
    QItemSelectionModelPrivate()
        : model(0), currentCommand(QItemSelectionModel::NoUpdate) {}

    void init();
    QItemSelection expandSelection(const QItemSelection &selection, int selectionCommand) const;
    
    inline void remove(QList<QItemSelectionRange> &r)
    {
        QList<QItemSelectionRange>::const_iterator it = r.constBegin();
        for (; it != r.constEnd(); ++it)
            ranges.removeAll(*it);
    }

    QAbstractItemModel *model;
    QItemSelection ranges;
    QItemSelection currentSelection;
    QPersistentModelIndex currentItem;
    int currentCommand;
};

#endif
