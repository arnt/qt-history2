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

#include "qsqltablemodel_p.h"

struct Relation
{
    Relation(): model(0) {}
    QSqlRelation rel;
    QSqlTableModel *model;
    QHash<int, QVariant> displayValues;
};

class QSqlRelationalTableModelPrivate: public QSqlTableModelPrivate
{
public:
    QSqlRelationalTableModelPrivate(QSqlRelationalTableModel *qq)
        : QSqlTableModelPrivate(qq)
    {}

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
    for (int i = 0; i < relations.count(); ++i) {
        Relation &rel = relations[i];
        delete rel.model;
        rel.displayValues.clear();
    }
}

#define d d_func()

/*!
  \class QSqlRelationalTableModel
  \brief The QSqlRelationalTableModel class provides an editable data model
  for a single database table. In addition, columns can be set as foreign
  keys into another table.

  \ingroup database
  \module sql

  QSqlRelationalTableModel acts like QSqlTableModel, but allows columns to
  be set as foreign keys into other database tables.

  The following example assumes that there are two tables called \c people and
  \c titles. The \c titles table has two columns called \c id and \c title.
  The \c people table's 4th column is a foreign index into the \c titles table:

  \code
    QSqlTableModel model;
    model.setTable("people");
    model.setRelation(3, QSqlRelation("titles", "id", "title"));
    model.select();
  \endcode

  Instead of displaying the id of the title, the model will display the value
  stored in the \c title column of the \c titles table.
 */


/*!
  Creates an empty QSqlRelationalTableModel and sets the parent to \a parent
  and the database connection to \a db. If \a db is not valid, the
  default database connection will be used.
 */
QSqlRelationalTableModel::QSqlRelationalTableModel(QObject *parent, QSqlDatabase db)
    : QSqlTableModel(*new QSqlRelationalTableModelPrivate(this), parent, db)
{
}

/*!
  Destroys the object and frees any allocated resources.
 */
QSqlRelationalTableModel::~QSqlRelationalTableModel()
{
}

/*!
  \fn QVariant QSqlRelationalTableModel::data(const QModelIndex &index, int role) const

  Returns the data stored under the given \a role for the item referred to
  by \a index.
  Returns an invalid variant if \a index is out of bounds.
 */
QVariant QSqlRelationalTableModel::data(const QModelIndex &item, int role) const
{
    if (role == DisplayRole && item.column() > 0 && item.column() < d->relations.count()) {
        const QVariant v = d->relations.at(item.column()).displayValues.value(item.row());
        if (v.isValid())
            return v;
    }

    return QSqlTableModel::data(item, role);
}

/*!
    \fn bool QSqlRelationalTableModel::setData(const QModelIndex &index, int role, const QVariant &value)

    Sets the data for the \a role in the item with the specified \a index
    to the \a value given.
    Depending on the edit strategy, the value might be applied to the database at
    once, or it may be cached in the model.

    Returns true if the value could be set, or false on error (for example, if
    \a index is out of bounds).

    For relational columns, \a value has to be the index, not the display value.

    \sa editStrategy(), data(), submitChanges(), revertRow()
 */
bool QSqlRelationalTableModel::setData(const QModelIndex &item, int role, const QVariant &value)
{
    if (role == DisplayRole && item.column() > 0 && item.column() < d->relations.count()) {
        d->relations[item.column()].displayValues[item.row()] = value;
        return true;
    }

    return QSqlTableModel::setData(item, role, value);
}

/*!
    Lets the specified \a column be a foreign index specified by \a relation.
 */
void QSqlRelationalTableModel::setRelation(int column, const QSqlRelation &relation)
{
    if (d->relations.size() <= column)
        d->relations.resize(column + 1);
    d->relations[column].rel = relation;
}

/*!
    Returns the relation for the column \a column.
 */
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

void QSqlRelationalTableModel::revertRow(int row)
{
    for (int i = 0; i < d->relations.count(); ++i)
        d->relations[i].displayValues.remove(row);
    QSqlTableModel::revertRow(row);
}

void QSqlRelationalTableModel::clear()
{
    d->clearChanges();
    d->relations.clear();
    QSqlTableModel::clear();
}

bool QSqlRelationalTableModel::select()
{
    d->clearChanges();
    return QSqlTableModel::select();
}
