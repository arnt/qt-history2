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

#include "qsqlrelationaltablemodel.h"

#include "qhash.h"
#include "qstringlist.h"
#include "qsqldatabase.h"
#include "qsqlerror.h"
#include "qsqlindex.h"
#include "qsqlquery.h"
#include "qsqlrecord.h"

struct Relation
{
    Relation(): model(0) {}
    QSqlRelation rel;
    QSqlTableModel *model;
    QHash<int, QVariant> displayValues;
};

class QSqlRelationalTableModelPrivate
{
public:
    QVector<Relation> relations;
    void clearChanges();
};

static void qAppendWhereClause(QString &query, const QString &clause1, const QString &clause2)
{
    if (clause1.isEmpty() && clause2.isEmpty())
        return;
    if (clause1.isEmpty() || clause2.isEmpty())
        query.append(QLatin1String(" WHERE (")).append(clause1).append(clause2);
    else
        query.append(QLatin1String(" WHERE (")).append(clause1).append(
                        QLatin1String(") AND (")).append(clause2);
    query.append(QLatin1String(") "));
}

void QSqlRelationalTableModelPrivate::clearChanges()
{
    for (int i = 0; i < relations.count(); ++i)
        relations[i].displayValues.clear();
}

QSqlRelationalTableModel::QSqlRelationalTableModel(QObject *parent, QSqlDatabase db)
    : QSqlTableModel(parent, db)
{
    d = new QSqlRelationalTableModelPrivate;
}

QSqlRelationalTableModel::~QSqlRelationalTableModel()
{
    delete d;
}

QVariant QSqlRelationalTableModel::data(const QModelIndex &item, int role) const
{
    if (role == DisplayRole && item.column() > 0 && item.column() < d->relations.count()) {
        const QVariant v = d->relations.at(item.column()).displayValues.value(item.row());
        if (v.isValid())
            return v;
    }

    return QSqlTableModel::data(item, role);
}

bool QSqlRelationalTableModel::setData(const QModelIndex &item, int role, const QVariant &value)
{
    if (role == DisplayRole && item.column() > 0 && item.column() < d->relations.count()) {
        d->relations[item.column()].displayValues[item.row()] = value;
        return true;
    }

    return QSqlTableModel::setData(item, role, value);
}

void QSqlRelationalTableModel::setRelation(int column, const QSqlRelation &relation)
{
    if (d->relations.size() <= column)
        d->relations.resize(column + 1);
    d->relations[column].rel = relation;
}

QSqlRelation QSqlRelationalTableModel::relation(int column) const
{
    return d->relations.value(column).rel;
}

QString QSqlRelationalTableModel::selectStatement() const
{
    QString query;

    if (tableName().isEmpty())
        return query;
    if (d->relations.isEmpty())
        return QSqlTableModel::selectStatement();

    QString tList;
    QString fList;
    QString where;

    QSqlRecord rec = database().record(tableName());
    QStringList tables;
    const Relation nullRelation;
    for (int i = 0; i < rec.count(); ++i) {
        QSqlRelation relation = d->relations.value(i, nullRelation).rel;
        if (relation.isValid()) {
            fList.append(relation.tableName()).append(QLatin1Char('.'));
            fList.append(relation.displayColumn()).append(QLatin1Char(','));
            if (!tables.contains(relation.tableName()))
                tables.append(relation.tableName());
            where.append(tableName()).append(QLatin1Char('.')).append(rec.fieldName(i));
            where.append(QLatin1Char('=')).append(relation.tableName()).append(QLatin1Char('.'));
            where.append(relation.indexColumn()).append(QLatin1String(" and "));
        } else {
            fList.append(tableName()).append(QLatin1Char('.')).append(rec.fieldName(i)).append(
                            QLatin1Char(','));
        }
    }
    if (!tables.isEmpty())
        tList.append(tables.join(QLatin1String(","))).append(QLatin1String(","));
    if (fList.isEmpty())
        return query;
    tList.prepend(QLatin1Char(',')).prepend(tableName());
    // truncate tailing comma
    tList.chop(1);
    fList.chop(1);
    query.append(QLatin1String("SELECT "));
    query.append(fList).append(QLatin1String(" FROM ")).append(tList);
    if (!where.isEmpty())
        where.chop(5);
    qAppendWhereClause(query, where, filter());

//    qDebug("query: %s", query.ascii());
    return query;
}

QSqlTableModel *QSqlRelationalTableModel::relationModel(int column)
{
    Relation relation = d->relations.value(column);
    if (!relation.rel.isValid())
        return 0;

    QSqlTableModel *childModel = relation.model;
    if (!childModel) {
        childModel = new QSqlTableModel(this, database());
        childModel->setTable(relation.rel.tableName());
        if (!childModel->select())
            qDebug("error looking up child relation: %s", childModel->lastError().text().ascii());
            // TODO ### error-handling
        childModel->fetchMore(); // ### HACK
        d->relations[column].model = childModel;
    }
    return childModel;
}

bool QSqlRelationalTableModel::submitChanges()
{
    if (QSqlTableModel::submitChanges()) {
        d->clearChanges();
        return true;
    }
    return false;
}

void QSqlRelationalTableModel::cancelChanges()
{
    QSqlTableModel::cancelChanges();
    d->clearChanges();
}

void QSqlRelationalTableModel::clear()
{
    for (int i = 0; i < d->relations.count(); ++i)
        delete d->relations.at(i).model;
    d->relations.clear();
    QSqlTableModel::clear();
}

