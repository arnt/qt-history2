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

#ifndef QSQLTABLEMODEL_H
#define QSQLTABLEMODEL_H

#include "qsqldatabase.h"
#include "qsqlquerymodel.h"

class QSqlTableModelPrivate;
class QSqlRecord;
class QSqlField;
class QSqlIndex;

class Q_SQL_EXPORT QSqlTableModel: public QSqlQueryModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSqlTableModel)

public:
    enum EditStrategy {OnFieldChange, OnRowChange, OnManualSubmit};

    QSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    virtual ~QSqlTableModel();

    virtual bool select();

    void setTable(const QString &tableName);
    QString tableName() const;

    ItemFlags flags(const QModelIndex &index) const;

    QVariant data(const QModelIndex &idx, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);
#ifdef Q_NO_USING_KEYWORD
    inline bool setData(const QModelIndex &index, const QVariant &value)
    { return QAbstractItemModel::setData(index, value); }
#else
    using QAbstractItemModel::setData;
#endif

    QVariant headerData(int section, Qt::Orientation orientation, int role);

    bool isDirty(const QModelIndex &index) const;
    void clear();

    virtual void setEditStrategy(EditStrategy strategy);
    EditStrategy editStrategy() const;

    QSqlIndex primaryKey() const;
    QSqlDatabase database() const;
    int fieldIndex(const QString &fieldName) const;

    bool isSortable() const;
    void sort(int column, Qt::SortOrder order);
    virtual void setSort(int column, Qt::SortOrder order);

    QString filter() const;
    virtual void setFilter(const QString &filter);

    int rowCount() const;

    bool removeColumns(int column, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool removeRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);
    bool insertRows(int row, const QModelIndex &parent = QModelIndex::Null, int count = 1);

    bool insertRecord(int row, const QSqlRecord &record);
    bool setRecord(int row, const QSqlRecord &record);

public slots:
    virtual bool submitChanges();
    virtual void revertRow(int row);
    void revertAll();

signals:
    void primeInsert(int row, QSqlRecord &record);

    void beforeInsert(QSqlRecord &record);
    void beforeUpdate(int row, QSqlRecord &record);
    void beforeDelete(int row);

protected:
    QSqlTableModel(QSqlTableModelPrivate &dd, QObject *parent, QSqlDatabase db);

    virtual bool updateRowInTable(int row, const QSqlRecord &values);
    virtual bool insertRowIntoTable(const QSqlRecord &values);
    virtual bool deleteRowFromTable(int row);
    virtual QString orderByStatement() const;
    virtual QString selectStatement() const;

    void setPrimaryKey(const QSqlIndex &key);
    void setQuery(const QSqlQuery &query);
    QModelIndex dataIndex(const QModelIndex &item) const;
};

#endif
