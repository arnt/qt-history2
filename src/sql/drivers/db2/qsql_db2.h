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

#ifndef QSQL_DB2_H
#define QSQL_DB2_H

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_DB2
#else
#define Q_EXPORT_SQLDRIVER_DB2 Q_SQL_EXPORT
#endif

#include "QtSql/qsqlresult.h"
#include "QtSql/qsqldriver.h"

class QDB2Driver;
class QDB2DriverPrivate;
class QDB2ResultPrivate;
class QSqlRecord;

class QDB2Result : public QSqlResult
{
public:
    QDB2Result(const QDB2Driver* dr, const QDB2DriverPrivate* dp);
    ~QDB2Result();
    bool prepare(const QString& query);
    bool exec();
    QVariant handle() const;

protected:
    QVariant data(int field);
    bool reset (const QString& query);
    bool fetch(int i);
    bool fetchNext();
    bool fetchFirst();
    bool fetchLast();
    bool isNull(int i);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;

private:
    QDB2ResultPrivate* d;
};

class Q_EXPORT_SQLDRIVER_DB2 QDB2Driver : public QSqlDriver
{
public:
    explicit QDB2Driver(QObject* parent = 0);
    QDB2Driver(Qt::HANDLE env, Qt::HANDLE con, QObject* parent = 0);
    ~QDB2Driver();
    bool hasFeature(DriverFeature) const;
    void close();
    QSqlRecord record(const QString& tableName) const;
    QStringList tables(QSql::TableType type) const;
    QSqlResult *createResult() const;
    QSqlIndex primaryIndex(const QString& tablename) const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    QString formatValue(const QSqlField &field, bool trimStrings) const;
    QVariant handle() const;
    bool open(const QString& db,
               const QString& user,
               const QString& password,
               const QString& host,
               int port,
               const QString& connOpts);
private:
    bool setAutoCommit(bool autoCommit);
    QDB2DriverPrivate* d;
};

#endif // QSQL_DB2_H
