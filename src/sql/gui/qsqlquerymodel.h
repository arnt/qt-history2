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

#ifndef QSQLQUERYMODEL_H
#define QSQLQUERYMODEL_H

#include <qabstractitemmodel.h>
#include <qsqldatabase.h>

class QSqlQueryModelPrivate;
class QSqlError;
class QSqlRecord;
class QSqlQuery;

class Q_SQL_EXPORT QSqlQueryModel: public QAbstractTableModel
{
    Q_OBJECT
    friend class QSqlQueryModelPrivate;

public:
    QSqlQueryModel(QObject *parent = 0);
    virtual ~QSqlQueryModel();

    int rowCount() const;
    int columnCount() const;
    QSqlRecord record(int row = -1) const;

    QVariant data(const QModelIndex &item, int role = QAbstractItemModel::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = QAbstractItemModel::DisplayRole) const;
    bool setHeaderData(int section, Qt::Orientation orientation, int role, const QVariant &value);

    bool insertColumns(int column, const QModelIndex &parent, int count);
#ifdef Q_NO_USING_KEYWORD
    inline bool insertColumns(int column, int count)
    { return QAbstractTableModel::insertColumns(column, count); }
#else
    using QAbstractTableModel::insertColumns;
#endif
    bool removeColumns(int column, const QModelIndex &parent, int count);
#ifdef Q_NO_USING_KEYWORD
    inline bool removeColumns(int column, int count)
    { return QAbstractTableModel::removeColumns(column, count); }
#else
    using QAbstractTableModel::removeColumns;
#endif

    virtual void setQuery(const QSqlQuery &query);
    void setQuery(const QString &query, const QSqlDatabase &db = QSqlDatabase());
    const QSqlQuery query() const;
    virtual void clear();

    QSqlError lastError() const;

    void fetchMore(const QModelIndex &parent = QModelIndex::Null);

protected:
    QModelIndex dataIndex(const QModelIndex &item) const;
    void setLastError(const QSqlError &error);
    QSqlQueryModel(QSqlQueryModelPrivate &d, QObject *parent);

    inline QSqlQueryModelPrivate *d_func() const { return d_ptr; }

    QSqlQueryModelPrivate *d_ptr;
};

#endif
