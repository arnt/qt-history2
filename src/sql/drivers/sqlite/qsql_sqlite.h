/****************************************************************************
**
** Definition of SQLite driver classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
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

#if defined (Q_OS_WIN32)
# include <qt_windows.h>
#endif

class QSQLiteDriverPrivate;
class QSQLiteResultPrivate;
class QSQLiteDriver;

class QSQLiteResult : public QSqlResult
{
    friend class QSQLiteDriver;
    friend class QSQLiteResultPrivate;
public:
    QSQLiteResult(const QSQLiteDriver* db);
    ~QSQLiteResult();

protected:
    void cleanup();
    bool fetch(int i);
    bool fetchNext();
    bool fetchFirst();
    bool fetchLast();
    QVariant data(int field);
    bool isNull(int field);
    bool reset (const QString& query);
    int size();
    int numRowsAffected();

private:
    QSQLiteResultPrivate* d;
};

class QSQLiteDriver : public QSqlDriver
{
    friend class QSQLiteResult;
public:
    QSQLiteDriver(QObject * parent=0, const char * name=0);
    ~QSQLiteDriver();
    bool hasFeature(DriverFeature f) const;
    bool open(const QString & db,
                   const QString & user = QString(),
                   const QString & password = QString(),
                   const QString & host = QString(),
                   int port = -1,
                   const QString & connOpts = QString() );
    void close();
    QSqlQuery createQuery() const;
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
    QStringList tables(const QString& user) const;

    QSqlRecord record(const QString& tablename) const;
    QSqlRecordInfo recordInfo(const QString& tablename) const;
    QSqlIndex primaryIndex(const QString &table) const;
    QSqlRecord record(const QSqlQuery& query) const;
    QSqlRecordInfo recordInfo(const QSqlQuery& query) const;

private:
    QSQLiteDriverPrivate* d;
};
#endif
