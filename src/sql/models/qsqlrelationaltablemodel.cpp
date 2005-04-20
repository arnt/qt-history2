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
#include "qsqlfield.h"
#include "qsqlindex.h"
#include "qsqlquery.h"
#include "qsqlrecord.h"

#include "qsqltablemodel_p.h"

/*!
    \class QSqlRelation
    \brief The QSqlRelation class stores information about an SQL foreign key.

    QSqlRelation is a helper class for QSqlRelationalTableModel. See
    QSqlRelationalTableModel::setRelation() and
    QSqlRelationalTableModel::relation() for details.

    \sa QSqlRelationalTableModel, QSqlRelationalDelegate
*/

/*!
    \fn QSqlRelation::QSqlRelation()

    Constructs an invalid QSqlRelation object.

    For such an object, the tableName(), indexColumn(), and
    displayColumn() functions return an empty string.

    \sa isValid()
*/

/*!
    \fn QSqlRelation::QSqlRelation(const QString &tableName, const QString &indexColumn,
                                   const QString &displayColumn)

    Constructs a QSqlRelation object, where \a tableName is the SQL
    table name to which a foreign key refers, \a indexColumn is the
    foreign key, and \a displayColumn is the field that should be
    presented to the user.

    \sa tableName(), indexColumn(), displayColumn()
*/

/*!
    \fn QString QSqlRelation::tableName() const

    Returns the name of the table to which a foreign key refers.
*/

/*!
    \fn QString QSqlRelation::indexColumn() const

    Returns the index column from table tableName() to which a
    foreign key refers.
*/

/*!
    \fn QString QSqlRelation::displayColumn() const

    Returns the column from table tableName() that should be
    presented to the user instead of a foreign key.
*/

/*!
    \fn bool QSqlRelation::isValid() const

    Returns true if the QSqlRelation object is valid; otherwise
    returns false.
*/

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
    QSqlRelationalTableModelPrivate()
        : QSqlTableModelPrivate()
    {}

    mutable QVector<Relation> relations;
    QSqlRecord baseRec; // the record without relations
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

/*!
    \class QSqlRelationalTableModel
    \brief The QSqlRelationalTableModel class provides an editable
    data model for a single database table, with foreign key support.

    \ingroup database
    \module sql

    QSqlRelationalTableModel acts like QSqlTableModel, but allows
    columns to be set as foreign keys into other database tables.

    \table
    \row \o \inlineimage noforeignkeys.png
         \o \inlineimage foreignkeys.png
    \endtable

    The screenshot on the left shows a plain QSqlTableModel in a
    QTableView. Foreign keys (\c city and \c country) aren't resolved
    to human-readable values. The screenshot on the right shows a
    QSqlRelationalTableModel, with foreign keys resolved into
    human-readable text strings.

    The following code snippet shows how the QSqlRelationalTableModel
    was set up:

    \quotefromfile sql/relationaltablemodel/relationaltablemodel.cpp
    \skipto model->setTable
    \printline model->setTable
    \skipto setRelation
    \printline setRelation
    \printline setRelation

    The setRelation() function calls establish a relationship between
    two tables. The first call specifies that column 2 in table \c
    employee is a foreign key that maps with field \c id of table \c
    city, and that the view should present the \c{city}'s \c name
    field to the user. The second call does something similar with
    column 3.

    If you use a read-write QSqlRelationalTableModel, you probably
    want to use QSqlRelatinalDelegate on the view. Unlike the default
    delegate, QSqlRelationalDelegate provides a combobox for fields
    that are foreign keys into other tables. To use the class, simply
    call QAbstractItemView::setItemDelegate() on the view with an
    instance of QSqlRelationalDelegate:

    \quotefromfile sql/relationaltablemodel/relationaltablemodel.cpp
    \skipto QTableView *view = new
    \printuntil setItemDelegate

    The \l{sql/relationaltablemodel} example illustrates how to use
    QSqlRelationalTableModel in conjunction with
    QSqlRelationalDelegate to provide tables with foreigh key
    support.

    \image relationaltable.png

    \sa QSqlRelation, QSqlRelationalDelegate
*/


/*!
    Creates an empty QSqlRelationalTableModel and sets the parent to \a parent
    and the database connection to \a db. If \a db is not valid, the
    default database connection will be used.
*/
QSqlRelationalTableModel::QSqlRelationalTableModel(QObject *parent, QSqlDatabase db)
    : QSqlTableModel(*new QSqlRelationalTableModelPrivate, parent, db)
{
}

/*!
    Destroys the object and frees any allocated resources.
*/
QSqlRelationalTableModel::~QSqlRelationalTableModel()
{
}

/*!
    \reimp
*/
QVariant QSqlRelationalTableModel::data(const QModelIndex &index, int role) const
{
    Q_D(const QSqlRelationalTableModel);
    if (role == Qt::DisplayRole && index.column() > 0 && index.column() < d->relations.count()) {
        const QVariant v = d->relations.at(index.column()).displayValues.value(index.row());
        if (v.isValid())
            return v;
    }

    return QSqlTableModel::data(index, role);
}

/*!
    Sets the data for the \a role in the item with the specified \a
    index to the \a value given. Depending on the edit strategy, the
    value might be applied to the database at once, or it may be
    cached in the model.

    Returns true if the value could be set, or false on error (for
    example, if \a index is out of bounds).

    For relational columns, \a value must be the index, not the
    display value.

    \sa editStrategy(), data(), submit(), revertRow()
*/
bool QSqlRelationalTableModel::setData(const QModelIndex &index, const QVariant &value,
                                       int role)
{
    Q_D(QSqlRelationalTableModel);
    if (role == Qt::DisplayRole && index.column() > 0 && index.column() < d->relations.count()) {
        d->relations[index.column()].displayValues[index.row()] = value;
        return true;
    }

    return QSqlTableModel::setData(index, value, role);
}

/*!
    Lets the specified \a column be a foreign index specified by \a relation.

    Example:

    \quotefromfile sql/relationaltablemodel/relationaltablemodel.cpp
    \skipto model->setTable
    \printline model->setTable
    \skipto setRelation
    \printline setRelation

    The setRelation() call specifies that column 2 in table \c
    employee is a foreign key that maps with field \c id of table \c
    city, and that the view should present the \c{city}'s \c name
    field to the user.

    \sa relation()
*/
void QSqlRelationalTableModel::setRelation(int column, const QSqlRelation &relation)
{
    Q_D(QSqlRelationalTableModel);
    if (column < 0)
        return;
    if (d->relations.size() <= column)
        d->relations.resize(column + 1);
    d->relations[column].rel = relation;
}

/*!
    Returns the relation for the column \a column, or an invalid
    relation if no relation is set.

    \sa setRelation(), QSqlRelation::isValid()
*/
QSqlRelation QSqlRelationalTableModel::relation(int column) const
{
    Q_D(const QSqlRelationalTableModel);
    return d->relations.value(column).rel;
}

/*!
    \reimp
*/
QString QSqlRelationalTableModel::selectStatement() const
{
    Q_D(const QSqlRelationalTableModel);
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

    return query;
}

/*!
    Returns a QSqlTableModel object for accessing the table for which
    \a column is a foreign key, or 0 if there is no relation for the
    given \a column.

    The returned object is owned by the QSqlRelationalTableModel.

    \sa setRelation(), relation()
*/
QSqlTableModel *QSqlRelationalTableModel::relationModel(int column) const
{
    Q_D(const QSqlRelationalTableModel);
    Relation relation = d->relations.value(column);
    if (!relation.rel.isValid())
        return 0;

    QSqlTableModel *childModel = relation.model;
    if (!childModel) {
        childModel = new QSqlTableModel(const_cast<QSqlRelationalTableModel *>(this), database());
        childModel->setTable(relation.rel.tableName());
        childModel->select();
        d->relations[column].model = childModel;
    }
    return childModel;
}

/*!
    \reimp
*/
void QSqlRelationalTableModel::revertRow(int row)
{
    Q_D(QSqlRelationalTableModel);
    for (int i = 0; i < d->relations.count(); ++i)
        d->relations[i].displayValues.remove(row);
    QSqlTableModel::revertRow(row);
}

/*!
    \reimp
*/
void QSqlRelationalTableModel::clear()
{
    Q_D(QSqlRelationalTableModel);
    d->clearChanges();
    d->relations.clear();
    QSqlTableModel::clear();
}

/*!
    \reimp
*/
bool QSqlRelationalTableModel::select()
{
    Q_D(QSqlRelationalTableModel);
    d->clearChanges();
    return QSqlTableModel::select();
}

/*!
    \reimp
*/
void QSqlRelationalTableModel::setTable(const QString &table)
{
    Q_D(QSqlRelationalTableModel);

    // memorize the table before applying the relations
    d->baseRec = d->db.record(table);

    QSqlTableModel::setTable(table);
}

/*!
    \reimp
*/
bool QSqlRelationalTableModel::updateRowInTable(int row, const QSqlRecord &values)
{
    Q_D(QSqlRelationalTableModel);

    QSqlRecord rec = values;

    // translate the field names
    for (int i = 0; i < values.count(); ++i) {
        int realCol = indexInQuery(createIndex(row, i)).column();
        if (realCol != -1 && d->relations.value(realCol).rel.isValid()) {
            QVariant v = values.value(i);
            rec.replace(i, d->baseRec.field(realCol));
            rec.setValue(i, v);
        }
    }
    return QSqlTableModel::updateRowInTable(row, rec);
}

