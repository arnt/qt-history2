/****************************************************************************
**
** Definition of TDS driver classes.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQL_TDS_H
#define QSQL_TDS_H

#include <qsqlresult.h>
#include <qsqldriver.h>
#include "../cache/qsqlcachedresult.h"

#ifdef Q_OS_WIN32
#define DBNTWIN32 // indicates 32bit windows dblib
#include "qt_windows.h
#include <sqlfront.h>
#include <sqldb.h>
#define CS_PUBLIC
#else
#include <sybfront.h>
#include <sybdb.h>
#endif //Q_OS_WIN32

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_TDS
#else
#define Q_EXPORT_SQLDRIVER_TDS Q_EXPORT
#endif

class QTDSDriverPrivate;
class QTDSResultPrivate;
class QTDSDriver;

class QTDSResult : public QtSqlCachedResult
{
public:
    QTDSResult(const QTDSDriver* db);
    ~QTDSResult();
    DBPROCESS *dbprocess() const;

protected:
    void cleanup();
    bool reset ( const QString& query );
    int size();
    int numRowsAffected();
    bool gotoNext(QtSqlCachedResult::ValueCache &values, int index);
    QSqlRecord record() const;

private:
    QTDSResultPrivate* d;
};

class Q_EXPORT_SQLDRIVER_TDS QTDSDriver : public QSqlDriver
{
    friend class QTDSResult;
public:
    QTDSDriver(QObject* parent = 0);
    QTDSDriver(LOGINREC* rec, const QString& host, const QString &db, QObject* parent = 0);
    ~QTDSDriver();
    bool hasFeature( DriverFeature f ) const;
    bool open( const QString & db,
	       const QString & user,
	       const QString & password,
	       const QString & host,
	       int port,
	       const QString& connOpts);
    void close();
    QStringList tables( const QString& user ) const;
    QSqlQuery createQuery() const;
    QSqlRecord record( const QString& tablename ) const;
    QSqlIndex primaryIndex( const QString& tablename ) const;

    QString formatValue( const QSqlField* field,
			 bool trimStrings ) const;
    LOGINREC* loginrec() const;

protected:
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();
private:
    void init();
    QTDSDriverPrivate *d;
};

#endif
