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

#ifndef QPERSISTENTMODELINDEX_P_H
#define QPERSISTENTMODELINDEX_P_H

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

#include <QtCore/qobject.h>
#include <QtCore/qabstractitemmodel.h>

class Q_CORE_EXPORT QPersistentModelIndexData
{
public:
    QPersistentModelIndexData() : model (0) { ref = 0; }
    QModelIndex index;
    QAtomic ref;
    const QAbstractItemModel *model;
    static QPersistentModelIndexData shared_null;
    static QPersistentModelIndexData *create(const QModelIndex &index);
    static void destroy(QPersistentModelIndexData *data);
};

class Q_CORE_EXPORT QPersistentModelIndexManager : public QObject
{
    Q_OBJECT

public:    
    QPersistentModelIndexManager(QAbstractItemModel *parent);
    ~QPersistentModelIndexManager();

    void invalidateIndex(const QModelIndex &index);
    void invalidateChildren(const QModelIndex &parent);

public slots:
    void rowsAboutToBeInserted(const QModelIndex &parent, int first, int last);
    void rowsInserted(const QModelIndex &parent, int first, int last);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void rowsRemoved(const QModelIndex &parent, int first, int last);
    void columnsAboutToBeInserted(const QModelIndex &parent, int first, int last);
    void columnsInserted(const QModelIndex &parent, int first, int last);
    void columnsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
    void columnsRemoved(const QModelIndex &parent, int first, int last);
    void reset();
    
public:
    QList<QPersistentModelIndexData*> indexes;
    QList<int> affected;
    QAbstractItemModel *model;
};

#endif // QPERSISTENTMODELINDEX_P_H
