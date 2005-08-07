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

#ifndef QSORTINGPROXYMODEL_P_H
#define QSORTINGPROXYMODEL_P_H

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

#include <qnamespace.h>
#include <private/qmappingproxymodel_p.h>

class QSortingProxyModelPrivate : private QMappingProxyModelPrivate
{
    Q_DECLARE_PUBLIC(QSortingProxyModel)
public:
    QSortingProxyModelPrivate()
        : QMappingProxyModelPrivate(),
          sort_column(-1),
          sort_order(Qt::AscendingOrder) {}
    int sort_column;
    Qt::SortOrder sort_order;
};

#endif // QSORTINGPROXYMODEL_P_H
