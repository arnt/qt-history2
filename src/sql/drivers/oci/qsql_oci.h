/****************************************************************************
**
** Definition of OCI driver classes.
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

#ifndef QSQL_OCI_H
#define QSQL_OCI_H

#include <qsqlresult.h>
#include <qsqlfield.h>
#include <qsqldriver.h>
#include <qstring.h>

#include <oci.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_OCI
#else
#define Q_EXPORT_SQLDRIVER_OCI Q_EXPORT
#endif

// Check if OCI supports scrollable cursors (Oracle version >= 9)
#ifdef OCI_STMT_SCROLLABLE_READONLY
#define QOCI_USES_VERSION_9
#endif

class QOCIPrivate;
class QOCIResultPrivate;
class QOCIDriver;

class QOCIResult : public QSqlResult
{
    friend class QOCIDriver;
    friend class QOCIPrivate;
public:
    QOCIResult( const QOCIDriver * db, QOCIPrivate* p );
    ~QOCIResult();
    OCIStmt*    statement();
    bool 	prepare( const QString& query );
    bool 	exec();
    
protected:
    bool	fetchNext();
    bool	fetchFirst();
    bool	fetchLast();
    bool	fetch(int i);
    bool	reset ( const QString& query );
    QVariant	data( int field );
    bool	isNull( int field );
    int         size();
    int         numRowsAffected();

private:
    QOCIPrivate*	d;
    QOCIResultPrivate*  cols;
    QSqlRecord          fs;
    bool                cached;
    bool                cacheNext();
};

#ifdef QOCI_USES_VERSION_9
class QOCI9Result : public QSqlResult
{
    friend class QOCIPrivate;
    friend class QOCIDriver;
public:
    QOCI9Result( const QOCIDriver * db, QOCIPrivate* p );
    ~QOCI9Result();
    OCIStmt*    statement();
    bool 	prepare( const QString& query );
    bool 	exec();

protected:
    bool	fetchNext();
    bool	fetchPrev();
    bool	fetchFirst();
    bool	fetchLast();
    bool	fetch(int i);
    bool	reset ( const QString& query );
    QVariant	data( int field );
    bool	isNull( int field );
    int         size();
    int         numRowsAffected();

private:
    QOCIPrivate*	d;
    QOCIResultPrivate*  cols;
    QSqlRecord          fs;
    bool                cacheNext( int r );
};
#endif //QOCI_USES_VERSION_9

class Q_EXPORT_SQLDRIVER_OCI QOCIDriver : public QSqlDriver
{
public:
    QOCIDriver( QObject* parent = 0, const char* name = 0 );
    QOCIDriver( OCIEnv* env, OCIError* err, OCISvcCtx* ctx, QObject* parent = 0, const char* name = 0 );
    ~QOCIDriver();
    bool		hasFeature( DriverFeature f ) const;
    bool                open( const QString & db,
			      const QString & user = QString::null,
			      const QString & password = QString::null,
			      const QString & host = QString::null,
			      int port = -1,
			      const QString& connOpts = QString::null );
    void	        close();
    QSqlQuery	        createQuery() const;
    QStringList         tables( const QString& user ) const;
    QSqlRecord          record( const QString& tablename ) const;
    QSqlRecord          record( const QSqlQuery& query ) const;
    QSqlRecordInfo      recordInfo( const QString& tablename ) const;
    QSqlRecordInfo      recordInfo( const QSqlQuery& query ) const;
    QSqlIndex           primaryIndex( const QString& tablename ) const;
    QString             formatValue( const QSqlField* field,
				     bool trimStrings ) const;
    OCIEnv*             environment();
    OCISvcCtx*          serviceContext();

protected:
    bool	        beginTransaction();
    bool	        commitTransaction();
    bool	        rollbackTransaction();
private:
    void	        init();
    void	        cleanup();
    QOCIPrivate*        d;
};

#endif
