/****************************************************************************
 **
 ** Definition of QSqlRelationalTableModel class.
 **
 ** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
 **
 ** This file is part of the sql module of the Qt GUI Toolkit.
 ** EDITIONS: FREE, ENTERPRISE
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ****************************************************************************/

#ifndef QSQLRELATIONALTABLEMODEL_H
#define QSQLRELATIONALTABLEMODEL_H

#include <qsqltablemodel.h>

class Q_SQL_EXPORT QSqlRelation
{
public:
    QSqlRelation() {}
    QSqlRelation(const QString &tableName, const QString &indexColumn,
               const QString &displayColumn)
        : tName(tableName), iColumn(indexColumn), dColumn(displayColumn) {}
    inline QString tableName() const
    { return tName; }
    inline QString indexColumn() const
    { return iColumn; }
    inline QString displayColumn() const
    { return dColumn; }
    inline bool isValid() const
    { return !(tName.isEmpty() || iColumn.isEmpty() || dColumn.isEmpty()); }
private:
    QString tName, iColumn, dColumn;
};

class QSqlRelationalTableModelPrivate;

class Q_SQL_EXPORT QSqlRelationalTableModel: public QSqlTableModel
{
//    Q_DECLARE_PRIVATE(DBCachedTableModel);

public:
    QSqlRelationalTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    virtual ~QSqlRelationalTableModel();

    QVariant data(const QModelIndex &item, int role = QAbstractItemModel::Display) const;
    bool setData(const QModelIndex &item, int role, const QVariant &value);

    virtual void setRelation(int column, const QSqlRelation &relation);
    QSqlRelation relation(int column) const;
    virtual QSqlTableModel *relationModel(int column);

public slots:
    bool submitChanges();
    void cancelChanges();

protected:
    QString selectStatement() const;

private:
    QSqlRelationalTableModelPrivate *d; //TODO - make me a DBTableModelPrivate
};

#endif
