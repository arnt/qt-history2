/****************************************************************************
 **
 ** Definition of QSqlModel class.
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

#ifndef QSQLMODEL_H
#define QSQLMODEL_H

#include <qabstractitemmodel.h>

class QSqlModelPrivate;
class QSqlError;
class QSqlRecord;
class QSqlQuery;

class Q_SQL_EXPORT QSqlModel: public QAbstractItemModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSqlModel);

public:
    QSqlModel(QObject *parent = 0);
    virtual ~QSqlModel();

    int rowCount() const;
    int columnCount() const;
    QSqlRecord record() const;

    QVariant data(const QModelIndex &item, int role = QAbstractItemModel::DisplayRole) const;
    bool setData(const QModelIndex &index, int role, const QVariant &value);

    bool insertColumn(int column, const QModelIndex &parent = QModelIndex(), int count = 1);
    bool removeColumn(int column, const QModelIndex &parent = QModelIndex(), int count = 1);

    virtual void setQuery(const QSqlQuery &query);
    const QSqlQuery query() const;
    virtual void clear();

    QSqlError lastError() const;

public slots:
    void fetchMore();

protected:
    QModelIndex dataIndex(const QModelIndex &item) const;
    void setLastError(const QSqlError &error);
    QSqlModel(QSqlModelPrivate &d, QObject *parent);
};

#endif
