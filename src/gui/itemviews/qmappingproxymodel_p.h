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

#include "QtCore/qhash.h"
#include "private/qabstractproxymodel_p.h"

class QMappingProxyModelPrivate : public QAbstractProxyModelPrivate
{
    Q_DECLARE_PUBLIC(QMappingProxyModel)
public:
    QMappingProxyModelPrivate() : QAbstractProxyModelPrivate() {}

    inline void insert_mapping(const QModelIndex &proxy_index,
                               const QModelIndex &source_index) const
    {
        proxy_to_source.insert(proxy_index, source_index);
        source_to_proxy.insert(source_index, proxy_index);
    }

    inline void remove_mapping(const QModelIndex &proxy_index,
                               const QModelIndex &source_index) const
    {
        proxy_to_source.remove(proxy_index);
        source_to_proxy.remove(source_index);
    }

    inline QModelIndex map_proxy_to_source(const QModelIndex &proxy_index) const
    {
        return proxy_to_source.value(proxy_index);
    }

    inline QModelIndex map_source_to_proxy(const QModelIndex &source_index) const
    {
        return source_to_proxy.value(source_index);
    }

    inline void* proxy_internal_pointer(const QModelIndex &proxy_parent) const
    {
        return proxy_parent.isValid() ? static_cast<void*>(proxy_to_source.find(proxy_parent)) : 0;
    }

    inline bool is_mapped(const QModelIndex &proxy_index) const
    {
        return proxy_to_source.contains(proxy_index);
    }

    inline void clear_mapping()
    {
        proxy_to_source.clear();
        source_to_proxy.clear();
    }

    mutable QHash<QModelIndex, QModelIndex> proxy_to_source;
    mutable QHash<QModelIndex, QModelIndex> source_to_proxy;
};

#endif // QMAPPINGPROXYMODEL_P_H
