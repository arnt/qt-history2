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

#ifndef QSQL_SQLITE_H
#define QSQL_SQLITE_H

#include <qsqldriver.h>
#include <qsqlresult.h>
#include <qsqlrecord.h>
#include <qsqlindex.h>
#include <qsqlcachedresult.h>

#if (QT_VERSION-0 >= 0x030000)
typedef QCoreVariant QSqlVariant;
#endif

#if defined (Q_OS_WIN32)
# include <qt_windows.h>
#endif

class QSQLiteDriverPrivate;
class QSQLiteResultPrivate;
class QSQLiteDriver;
struct sqlite;

class QSQLiteResult : public QSqlCachedResult
{
    friend class QSQLiteDriver;
    friend class QSQLiteResultPrivate;
public:
    QSQLiteResult(const QSQLiteDriver* db);
    ~QSQLiteResult();

protected:
    bool gotoNext(QSqlCachedResult::ValueCache& row, int idx);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();
    QSqlRecord record() const;

private:
    QSQLiteResultPrivate* d;
};

class QSQLiteDriver : public QSqlDriver
{
    friend class QSQLiteResult;
public:
    QSQLiteDriver(QObject *parent = 0);
    QSQLiteDriver(sqlite *connection, QObject *parent = 0);
    ~QSQLiteDriver();
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

private:
    QSQLiteDriverPrivate* d;
};
#endif
