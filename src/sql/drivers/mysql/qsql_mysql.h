/****************************************************************************
**
** Definition of MySQL driver classes.
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

#ifndef QSQL_MYSQL_H
#define QSQL_MYSQL_H

#include <qsqldriver.h>
#include <qsqlresult.h>
#include <qsqlfield.h>
#include <qsqlindex.h>

#if defined (Q_OS_WIN32)
#include <qt_windows.h>
#endif

#include <mysql.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_MYSQL
#else
#define Q_EXPORT_SQLDRIVER_MYSQL Q_EXPORT
#endif

class QMYSQLDriverPrivate;
class QMYSQLResultPrivate;
class QMYSQLDriver;
class QSqlRecordInfo;

class QMYSQLResult : public QSqlResult
{
    friend class QMYSQLDriver;
public:
    QMYSQLResult( const QMYSQLDriver* db );
    ~QMYSQLResult();

    MYSQL_RES* result();
protected:
    void		cleanup();
    bool		fetch( int i );
    bool		fetchNext();
    bool		fetchLast();
    bool		fetchFirst();
    QVariant		data( int field );
    bool		isNull( int field );
    bool		reset ( const QString& query );
    int			size();
    int			numRowsAffected();
private:
    QMYSQLResultPrivate* d;
};

class Q_EXPORT_SQLDRIVER_MYSQL QMYSQLDriver : public QSqlDriver
{
    friend class QMYSQLResult;
public:
    QMYSQLDriver( QObject * parent=0, const char * name=0 );
    QMYSQLDriver( MYSQL * con, QObject * parent=0, const char * name=0 );
    ~QMYSQLDriver();
    bool		hasFeature( DriverFeature f ) const;
    bool		open( const QString & db,
			      const QString & user = QString::null,
			      const QString & password = QString::null,
			      const QString & host = QString::null,
			      int port = -1,
			      const QString& connOpts = QString::null );
    void		close();
    QSqlQuery		createQuery() const;
    QStringList		tables( const QString& user ) const;
    QSqlIndex		primaryIndex( const QString& tablename ) const;
    QSqlRecord		record( const QString& tablename ) const;
    QSqlRecord		record( const QSqlQuery& query ) const;
    QSqlRecordInfo	recordInfo( const QString& tablename ) const;
    QSqlRecordInfo	recordInfo( const QSqlQuery& query ) const;
    QString		formatValue( const QSqlField* field,
				     bool trimStrings ) const;
    MYSQL*		mysql();
    
protected:
    bool		beginTransaction();
    bool		commitTransaction();
    bool		rollbackTransaction();
private:
    void		init();
    QMYSQLDriverPrivate* d;
};


#endif
