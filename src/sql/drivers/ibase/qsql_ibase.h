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

#ifndef QSQL_IBASE_H
#define QSQL_IBASE_H

#include <QtSql/qsqlresult.h>
#include <QtSql/qsqldriver.h>
#include <QtSql/private/qsqlcachedresult_p.h>

class QIBaseDriverPrivate;
class QIBaseResultPrivate;
class QIBaseDriver;

class QIBaseResult : public QSqlCachedResult
{
    friend class QIBaseResultPrivate;

public:
    explicit QIBaseResult(const QIBaseDriver* db);
    virtual ~QIBaseResult();

    bool prepare(const QString& query);
    bool exec();
    QVariant handle() const;

protected:
    bool gotoNext(QSqlCachedResult::ValueCache& row, int rowIdx);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;

private:
    QIBaseResultPrivate* d;
};

class QIBaseDriver : public QSqlDriver
{
    friend class QIBaseDriverPrivate;
    friend class QIBaseResultPrivate;
public:
    explicit QIBaseDriver(QObject *parent = 0);
    explicit QIBaseDriver(void *connection, QObject *parent = 0);
    virtual ~QIBaseDriver();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
                   const QString & user,
                   const QString & password,
                   const QString & host,
                   int port,
                   const QString & connOpts);
    bool open(const QString & db,
            const QString & user,
            const QString & password,
            const QString & host,
            int port) { return open (db, user, password, host, port, QString()); }
    void close();
    QSqlResult *createResult() const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    QStringList tables(QSql::TableType) const;

    QSqlRecord record(const QString& tablename) const;
    QSqlIndex primaryIndex(const QString &table) const;

    QString formatValue(const QSqlField &field, bool trimStrings) const;
    QVariant handle() const;

private:
    QIBaseDriverPrivate* d;
};

#endif // QSQL_IBASE_H
