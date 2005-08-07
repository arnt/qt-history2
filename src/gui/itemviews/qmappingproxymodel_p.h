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

#ifndef QMAPPINGPROXYMODEL_P_H
#define QMAPPINGPROXYMODEL_P_H

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


// FIXME: the empty model should maybe be public (it's usefull)

#include <qabstractitemmodel.h>
#include <private/qabstractitemmodel_p.h>

class QEmptyModel : public QAbstractItemModel
{
public:
    explicit QEmptyModel(QObject *parent = 0) : QAbstractItemModel(parent) {}
    QModelIndex index(int, int, const QModelIndex &) const { return QModelIndex(); }
    QModelIndex parent(const QModelIndex &) const { return QModelIndex(); }
    int rowCount(const QModelIndex &) const { return 0; }
    int columnCount(const QModelIndex &) const { return 0; }
    bool hasChildren(const QModelIndex &) const { return false; }
    QVariant data(const QModelIndex &, int) const { return QVariant(); }
};

class QMappingProxyModelPrivate : public QAbstractItemModelPrivate
{
    Q_DECLARE_PUBLIC(QMappingProxyModel)
public:
    QMappingProxyModelPrivate() : QAbstractItemModelPrivate(), model(0) {}
    QAbstractItemModel *model;
    QEmptyModel empty;
};

#endif // QMAPPINGPROXYMODEL_P_H
