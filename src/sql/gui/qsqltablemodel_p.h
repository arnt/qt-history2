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

#ifndef QSQLTABLEMODEL_P_H
#define QSQLTABLEMODEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qsql*model.h .  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//
//

#include "private/qsqlquerymodel_p.h"
#include "qmap.h"

class QSqlTableModelPrivate: public QSqlQueryModelPrivate
{
    Q_DECLARE_PUBLIC(QSqlTableModel)

public:
    QSqlTableModelPrivate(QSqlTableModel *qq)
        : QSqlQueryModelPrivate(qq),
          editIndex(-1), insertIndex(-1), sortColumn(-1),
          sortOrder(Qt::AscendingOrder),
          strategy(QSqlTableModel::OnFieldChange)
    {}
    void clear();
    QSqlRecord primaryValues(int index);
    void clearEditBuffer();
    QSqlRecord record(const QVector<QVariant> &values) const;

    bool exec(const QString &stmt, bool prepStatement,
              const QSqlRecord &rec, const QSqlRecord &whereValues = QSqlRecord());
    void revertCachedRow(int row);
    QSqlDatabase db;
    int editIndex;
    int insertIndex;

    int sortColumn;
    Qt::SortOrder sortOrder;

    QSqlTableModel::EditStrategy strategy;

    QSqlQuery editQuery;
    QSqlIndex primaryIndex;
    QString tableName;
    QString filter;

    struct ModifiedRow
    {
        ModifiedRow(QSql::Op o = QSql::None, const QSqlRecord &r = QSqlRecord()): op(o), rec(r) {}
        ModifiedRow(const ModifiedRow &other): op(other.op), rec(other.rec) {}
        QSql::Op op;
        QSqlRecord rec;
    };

    QSqlRecord editBuffer;

    typedef QMap<int, ModifiedRow> CacheMap;
    CacheMap cache;
};

#endif

