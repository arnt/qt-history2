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
#include <qdatetime.h>

class QSortingProxyModelPrivate : private QMappingProxyModelPrivate
{
    Q_DECLARE_PUBLIC(QSortingProxyModel)

public:

    static bool defaultLessThan(const QModelIndex &left, const QModelIndex &right)
    {
        QVariant l = left.model()->data(left, Qt::DisplayRole);
        QVariant r = right.model()->data(right, Qt::DisplayRole);
        Q_ASSERT(l.type() == r.type());
        switch (l.type()) {
        case QVariant::Int:
            return l.toInt() < r.toInt();
        case QVariant::UInt:
            return l.toUInt() < r.toUInt();
        case QVariant::LongLong:
            return l.toLongLong() < r.toLongLong();
        case QVariant::ULongLong:
            return l.toULongLong() < r.toULongLong();
        case QVariant::Double:
            return l.toDouble() < r.toDouble();
        case QVariant::Char:
            return l.toChar() < r.toChar();
        case QVariant::Date:
            return l.toDate() < r.toDate();
        case QVariant::Time:
            return l.toTime() < r.toTime();
        case QVariant::DateTime:
            return l.toDateTime() < r.toDateTime();
        case QVariant::String:
        default:
            return l.toString() < r.toString();
        }

        return false;
    }

    static bool defaultGreaterThan(const QModelIndex &left, const QModelIndex &right)
    {
        QVariant l = left.model()->data(left, Qt::DisplayRole);
        QVariant r = right.model()->data(right, Qt::DisplayRole);
        Q_ASSERT(l.type() == r.type());
        switch (l.type()) {
        case QVariant::Int:
            return l.toInt() > r.toInt();
        case QVariant::UInt:
            return l.toUInt() > r.toUInt();
        case QVariant::LongLong:
            return l.toLongLong() > r.toLongLong();
        case QVariant::ULongLong:
            return l.toULongLong() > r.toULongLong();
        case QVariant::Double:
            return l.toDouble() > r.toDouble();
        case QVariant::Char:
            return l.toChar() > r.toChar();
        case QVariant::Date:
            return l.toDate() > r.toDate();
        case QVariant::Time:
            return l.toTime() > r.toTime();
        case QVariant::DateTime:
            return l.toDateTime() > r.toDateTime();
        case QVariant::String:
        default:
            return l.toString() > r.toString();
        }

        return false;
    }
    
    QSortingProxyModelPrivate()
        : QMappingProxyModelPrivate(),
          sort_column(-1),
          sort_order(Qt::AscendingOrder),
          less(&defaultLessThan),
          greater(&defaultGreaterThan) {}
 
    int sort_column;
    Qt::SortOrder sort_order;

    QSortingProxyModel::Compare *less;
    QSortingProxyModel::Compare *greater;
};

#endif // QSORTINGPROXYMODEL_P_H
