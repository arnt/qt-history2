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
#include <qsqlfield.h>
#include <qsqldriver.h>
#include "../shared/qsql_result.h"

#ifdef Q_OS_WIN32
#define DBNTWIN32 // indicates 32bit windows dblib
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

class QTDSPrivate;
class QTDSDriver;

class QTDSResult : public QSqlCachedResult
{
public:
    QTDSResult( const QTDSDriver* db, const QTDSPrivate* p );
    ~QTDSResult();
protected:
    void		cleanup();
    bool		reset ( const QString& query );
    int			size();
    int			numRowsAffected();
    bool		gotoNext();
    QSqlRecord record() const;

private:
    QTDSPrivate*	d;
};

class Q_EXPORT_SQLDRIVER_TDS QTDSDriver : public QSqlDriver
{
public:
    QTDSDriver(QObject* parent = 0);
    QTDSDriver(LOGINREC* rec, DBPROCESS* proc, const QString& host, QObject* parent = 0);
    ~QTDSDriver();
    bool		hasFeature( DriverFeature f ) const;
    bool		open( const QString & db,
			      const QString & user,
			      const QString & password,
			      const QString & host,
			      int port,
			      const QString& connOpts);
    void		close();
    QStringList         tables( const QString& user ) const;
    QSqlQuery		createQuery() const;
    QSqlRecord          record( const QString& tablename ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;

    QString		formatValue( const QSqlField* field,
				     bool trimStrings ) const;
    LOGINREC*    	loginrec();
    DBPROCESS*   	dbprocess();

protected:
    bool		beginTransaction();
    bool		commitTransaction();
    bool		rollbackTransaction();
private:
    void		init();
    QTDSPrivate*	d;
};

#endif
