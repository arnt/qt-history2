/****************************************************************************
 **
 ** Definition of QSqlTableModel class.
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

#ifndef QSQLTABLEMODEL_H
#define QSQLTABLEMODEL_H

#include "qsqldatabase.h"
#include "qsqlmodel.h"

class QSqlTableModelPrivate;
class QSqlRecord;
class QSqlField;
class QSqlIndex;

class QSqlTableModel: public QSqlModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSqlTableModel);

public:
    enum EditStrategy {OnFieldChange, OnRowChange, OnManualSubmit};

    QSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    virtual ~QSqlTableModel();

    virtual bool select();

    void setTable(const QString &tableName);
    QString tableName() const;

    bool isModelReadOnly() const { return false; }
    bool isEditable(const QModelIndex &) const;

    QVariant data(const QModelIndex &idx, int role = QAbstractItemModel::Display) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    virtual void setEditStrategy(EditStrategy strategy);
    EditStrategy editStrategy() const;

    QSqlIndex primaryKey() const;
    QSqlDatabase database() const;
    int fieldIndex(const QString &fieldName) const;

    bool isSortable() const;
    void sort(int column, SortOrder order);
    virtual void setSort(int column, SortOrder order);

    QString filter() const;
    virtual void setFilter(const QString &filter);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    bool removeColumn(int column, const QModelIndex &parent = QModelIndex());
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    bool insertRow(int row, const QModelIndex &parent = QModelIndex(), int count = 1);

public slots:
    virtual bool submitChanges();
    virtual void cancelChanges();

signals:
    void primeInsert(int row, QSqlRecord &record);

    void beforeInsert(int row, QSqlRecord &record);
    void beforeUpdate(int row, QSqlRecord &record);
    void beforeDelete(int row, QSqlRecord &record);

protected:
    virtual bool update(int row, const QSqlRecord &values);
    virtual bool insert(const QSqlRecord &values);
    virtual QString updateStatement(const QSqlRecord &values) const;
    virtual QString orderByStatement() const;
    virtual QString selectStatement() const;
    virtual QString deleteStatement() const;
    virtual QString insertStatement() const;

    void setPrimaryKey(const QSqlIndex &key);
    void setQuery(const QSqlQuery &query);
    QModelIndex dataIndex(const QModelIndex &item) const;
};

#endif
