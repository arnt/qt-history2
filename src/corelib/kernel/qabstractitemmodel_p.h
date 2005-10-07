/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QABSTRACTITEMMODEL_P_H
#define QABSTRACTITEMMODEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of QAbstractItemModel*.  This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//
//

#include "private/qobject_p.h"
#include "QtCore/qstack.h"

class Q_CORE_EXPORT QPersistentModelIndexData
{
public:
    QPersistentModelIndexData() : model (0) {}
    QModelIndex index;
    QAtomic ref;
    const QAbstractItemModel *model;
    static QPersistentModelIndexData *create(const QModelIndex &index);
    static void destroy(QPersistentModelIndexData *data);
};

class Q_CORE_EXPORT QAbstractItemModelPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QAbstractItemModel)

public:
    ~QAbstractItemModelPrivate();

    void removePersistentIndexData(QPersistentModelIndexData *data);
    void invalidate(int position);
    void rowsAboutToBeInserted(const QModelIndex &parent, int first, int last);
    void rowsInserted(const QModelIndex &parent, int first, int last);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void rowsRemoved(const QModelIndex &parent, int first, int last);
    void columnsAboutToBeInserted(const QModelIndex &parent, int first, int last);
    void columnsInserted(const QModelIndex &parent, int first, int last);
    void columnsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void columnsRemoved(const QModelIndex &parent, int first, int last);
    void reset();

    struct Change {
        Change() : first(-1), last(-1) {}
        Change(const Change &c) : parent(c.parent), first(c.first), last(c.last) {}
        Change(const QModelIndex &p, int f, int l) : parent(p), first(f), last(l) {}
        QModelIndex parent;
        int first, last;
    };
    QStack<Change> changes;

    struct Persistent {
        QList<QPersistentModelIndexData*> indexes;
        QStack<QList<int> > moved;
        QStack<QList<int> > invalidated;
    } persistent;
};

#endif // QABSTRACTITEMMODEL_P_H
