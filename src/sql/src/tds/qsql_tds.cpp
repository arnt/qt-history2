/****************************************************************************
**
** Implementation of TDS driver classes
**
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsql_tds.h"

#include <qglobal.h>
#include <qptrdict.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qregexp.h>
#ifdef Q_OS_WIN32    // We assume that MS SQL Server is used. Set Q_USE_SYBASE to force Sybase.
#include <windows.h>
#else
#define Q_USE_SYBASE
#endif

#ifdef Q_USE_SYBASE
#include <sybfront.h>
#include <sybdb.h>
#else
#define DBNTWIN32 // indicates 32bit windows dblib
#include <sqlfront.h>
#include <sqldb.h>
#define CS_PUBLIC
#endif //Q_OS_WIN32



#ifdef DBNTWIN32
#define QMSGHANDLE DBMSGHANDLE_PROC
#define QERRHANDLE DBERRHANDLE_PROC
#define QTDSCHAR SQLCHAR
#define QTDSDATETIME4 SQLDATETIM4
#define QTDSDATETIME SQLDATETIME
#define QTDSDATETIME_N SQLDATETIMN
#define QTDSDECIMAL SQLDECIMAL
#define QTDSFLT4 SQLFLT4
#define QTDSFLT8 SQLFLT8
#define QTDSFLT8_N SQLFLTN
#define QTDSINT1 SQLINT1
#define QTDSINT2 SQLINT2
#define QTDSINT4 SQLINT4
#define QTDSINT4_N SQLINTN
#define QTDSMONEY4 SQLMONEY4
#define QTDSMONEY SQLMONEY
#define QTDSMONEY_N SQLMONEYN
#define QTDSNUMERIC SQLNUMERIC
#define QTDSTEXT SQLTEXT
#define QTDSVARCHAR SQLVARCHAR
#else
#define QMSGHANDLE MHANDLEFUNC
#define QERRHANDLE EHANDLEFUNC
#define QTDSCHAR SYBCHAR
#define QTDSDATETIME4 SYBDATETIME4
#define QTDSDATETIME SYBDATETIME
#define QTDSDATETIME_N SYBDATETIMN
#define QTDSDECIMAL SYBDECIMAL
#define QTDSFLT8 SYBFLT8
#define QTDSFLT8_N SYBFLTN
#define QTDSFLT4 SYBREAL
#define QTDSINT1 SYBINT1
#define QTDSINT2 SYBINT2
#define QTDSINT4 SYBINT4
#define QTDSINT4_N SYBINTN
#define QTDSMONEY4 SYBMONEY4
#define QTDSMONEY SYBMONEY
#define QTDSMONEY_N SYBMONEYN
#define QTDSNUMERIC SYBNUMERIC
#define QTDSTEXT SYBTEXT
#define QTDSVARCHAR SYBVARCHAR
#endif  //DBNTWIN32

#define TDS_CURSOR_SIZE 50

//#define DEBUG_TDS

QSqlError qMakeError( const QString& err, int type, int errNo = -1 )
{
    return QSqlError( "QTDS: " + err, QString::null, type, errNo );
}

class QTDSClientData: public QSqlClientData
{
public:
    QVariant format( void* buf, int size, QVariant::Type type ) const
    {
	if ( type == QVariant::DateTime ) {
	    DBDATETIME *bdt = (DBDATETIME*) buf;
	    QDate date = QDate::fromString( "1900-01-01", Qt::ISODate );
	    QTime time = QTime::fromString( "00:00:00", Qt::ISODate );
	    return QVariant( QDateTime( date.addDays( bdt->dtdays ), time.addMSecs( int( bdt->dttime / 0.3 ) ) ) );
	} else {
	    return QSqlClientData::format( buf, size, type );
        }
    }

    QSqlClientData* clone() { return new QTDSClientData(); }
};

class QTDSClientNullData: public QSqlClientNullData
{
public:
    QTDSClientNullData(): QSqlClientNullData(), nullInfo( 0 ) {}
    QTDSClientNullData( QTDSClientNullData& other ): QSqlClientNullData(), nullInfo( other.nullInfo ) {}
    bool isNull() const
    {
#ifdef DEBUG_TDS
	qDebug( "QTDSClientNullData::isNull: %i", nullInfo );
#endif
	if ( nullInfo == -1 )
	    return TRUE;
	return FALSE;
    }
    void* binder() const
    {
	return (void*)&nullInfo;
    }
    QSqlClientNullData* clone()
    {
	return new QTDSClientNullData( *this );
    }
private:
    DBINT nullInfo;
};


class QTDSPrivate
{
public:
    QTDSPrivate():login(0), dbproc(0) {}
    QTDSPrivate( const QTDSPrivate& other):login(other.login), dbproc(other.dbproc), errorMsgs(other.errorMsgs) {}
    LOGINREC*    login;  // login information
    DBPROCESS*   dbproc; // connection from app to server
    QSqlError    lastError;
    void         addErrorMsg( QString& errMsg ) { errorMsgs.append( errMsg ); }
    QString      getErrorMsgs() { return errorMsgs.join("\n"); }
    void         clearErrorMsgs() { errorMsgs.clear(); }

private:
    QStringList  errorMsgs;
};

static QPtrDict<QTDSPrivate> errs;

extern "C" {
static int CS_PUBLIC qTdsMsgHandler ( DBPROCESS* dbproc,
			    DBINT msgno,
			    int msgstate,
			    int severity,
			    char* msgtext,
			    char* srvname,
			    char* /*procname*/,
			    int /*line*/)
{
#ifdef DEBUG_TDS
    qDebug( QString( "QTDSDriver warning (%1, severity %4): [%2] from server [%3]" ).arg( msgstate ).arg( msgtext ).arg( srvname ).arg( severity ) );
#endif
    QTDSPrivate* p = errs.find( dbproc );

    if ( !p ) {
#ifdef QT_RANGE_CHECK
	qDebug( QString( "QTDSDriver warning (%1): [%2] from server [%3]" ).arg( msgstate ).arg( msgtext ).arg( srvname ) );
#endif
	return INT_CANCEL;
    }

    if ( severity > 0 ) {
	QString errMsg = QString( "%1 (%2)" ).arg( msgtext ).arg( msgstate );
	p->addErrorMsg( errMsg );
    }

    return INT_CANCEL;
}

static int CS_PUBLIC qTdsErrHandler( DBPROCESS* dbproc,
				int /*severity*/,
				int dberr,
				int oserr,
				char* dberrstr,
				char* oserrstr)
{
#ifdef DEBUG_TDS
    qDebug( QString( "QTDSDriver error (%1): [%2] [%3]" ).arg( dberr ).arg( dberrstr ).arg( oserrstr ) );
#endif
    QTDSPrivate* p = errs.find( dbproc );
    if ( !p ) {
#ifdef QT_RANGE_CHECK
	qDebug( QString( "QTDSDriver error (%1): [%2] [%3]" ).arg( dberr ).arg( dberrstr ).arg( oserrstr ) );
#endif
	return INT_CANCEL;
    }
    /*
     * If the process is dead or NULL and
     * we are not in the middle of logging in...
     */
    if( (dbproc == NULL || DBDEAD( dbproc )) ) {
#ifdef QT_RANGE_CHECK	
	qDebug( QString( "QTDSDriver error (%1): [%2] [%3]" ).arg( dberr ).arg( dberrstr ).arg( oserrstr ) );
#endif
	return INT_CANCEL;
    }


    QString errMsg = QString( "%1 %2\n" ).arg( dberrstr ).arg( oserrstr );
    errMsg += p->getErrorMsgs();
    p->lastError = qMakeError( errMsg, QSqlError::Unknown, dberr );
    p->clearErrorMsgs();

    return INT_CANCEL ;
}

} //extern "C"


QVariant::Type qDecodeTDSType( int type )
{
    QVariant::Type t = QVariant::Invalid;
    switch ( type ) {
    case QTDSCHAR:
    case QTDSTEXT:
    case QTDSVARCHAR:
	t = QVariant::String;
	break;
    case QTDSINT1:
    case QTDSINT2:
    case QTDSINT4:
    case QTDSINT4_N:
	t = QVariant::Int;
	break;
    case QTDSFLT4:
    case QTDSFLT8:
    case QTDSFLT8_N:
    case QTDSMONEY4:
    case QTDSMONEY:
    case QTDSDECIMAL:
    case QTDSNUMERIC:
    case QTDSMONEY_N:
	t = QVariant::Double;
	break;
    case QTDSDATETIME4:
    case QTDSDATETIME:
    case QTDSDATETIME_N:
	t = QVariant::DateTime;
    break;
    default:
	t = QVariant::Invalid;
	break;
    }
    return t;
}

QVariant::Type qFieldType( QTDSPrivate* d, int i )
{
    QVariant::Type type = qDecodeTDSType( dbcoltype( d->dbproc, i+1 ) );
    return type;
}


QTDSResult::QTDSResult( const QTDSDriver* db, const QTDSPrivate* p )
    : QSqlResult( db ), forwardOnly( FALSE )
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::QTDSResult" );
#endif
    d =   new QTDSPrivate( *p );
    // insert d in error handler dict
    errs.insert( (void*)d->dbproc, d );
    set = new QSqlClientResultSet();
    buf = new QSqlClientResultBuffer();
    QTDSClientData* tcd = new QTDSClientData();
    buf->installDataFormat( tcd );
}

QTDSResult::~QTDSResult()
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::~QTDSResult" );
#endif
    cleanup();
    dbclose( d->dbproc );
    errs.remove( d->dbproc );
    delete set;
    delete buf;
    delete d;
}

void QTDSResult::cleanup()
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::cleanup()" );
#endif
    setAt( -1 );
    setActive( FALSE );
    set->clear();
    buf->clear();
    d->clearErrorMsgs();
}

bool QTDSResult::fetch( int i )
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::fetch(%i), at: %i", i, at() );
#endif
    if ( ( !isActive() ) || ( i < 0 ) ) {
#ifdef DEBUG_TDS
	qDebug( "QTDSResult::fetch(%i): not active or < 0", i );
#endif
	return FALSE;
    }
    if ( at() == i )
	return TRUE;
    if ( set->seek( i ) ) {
	setAt( i );
	return TRUE;
    }
    setAt( set->size() - 1 );
    while ( at() < i ) {
	if ( !cacheNext() )
	    return FALSE;
	setAt( at() + 1 );
    }
    return TRUE;
}

bool QTDSResult::fetchFirst()
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::fetchFirst()" );
#endif
    if ( forwardOnly && at() != QSql::BeforeFirst ) {
	return FALSE;
    }
    if ( !forwardOnly && set->seek( 0 ) ) {
	setAt( 0 );
	return TRUE;
    }
    if ( cacheNext() ) {
	setAt( 0 );
	return TRUE;
    }
    return FALSE;
}

bool QTDSResult::fetchNext()
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::fetchNext()" );
#endif
    if ( !forwardOnly && set->seek( at() + 1 ) ) {
	setAt( at() + 1 );
	return TRUE;
    }
    if ( cacheNext() ) {
	setAt( at() + 1 );
	return TRUE;
    }
    return FALSE;
}


bool QTDSResult::fetchLast()
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::fetchLast()" );
#endif
    if ( !forwardOnly && at() == QSql::AfterLast && set->size() > 0 ) {
	setAt( set->size() - 1 );
	return TRUE;
    }
    if ( at() >= QSql::BeforeFirst ) {
	while ( fetchNext() )
	    ; /* brute force */
	if ( forwardOnly && at() == QSql::AfterLast ) {
	    setAt( at() - 1 );
	    return TRUE;
	} else
	    return fetch( set->size() - 1 );
    }
    return FALSE;
}

QVariant QTDSResult::data( int i )
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::data(%i)", i );
#endif
    return set->buffer()->data( i );
}

bool QTDSResult::cacheNext()
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::cacheNext: filling set at %i", at() );
#endif
    STATUS stat = dbnextrow( d->dbproc );
    if ( stat == NO_MORE_ROWS ) {
	setAt( QSql::AfterLast );
#ifdef DEBUG_TDS
	qDebug( "QTDSResult::cacheNext: got NO_MORE_ROWS at %i", at() );
#endif
	return FALSE;
    }
    if ( ( stat == FAIL ) || ( stat == BUF_FULL ) ) {
#ifdef DEBUG_TDS
	qDebug( "QTDSResult::cacheNext: FAIL or BUF_FULL" );
#endif
	setLastError( d->lastError );
	return FALSE;
    }
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::cacheNext: appending row" );
    QString data;
    for ( int i = 0; i < buf->count(); ++i ) {
	data += " \"" + buf->data( i ).toString() + "\"";
    }
    qDebug( "QTDSResult::cacheNext:" + data );
#endif
    set->append( *buf );
    set->seek( set->size() - 1 );

    return TRUE;
}

bool QTDSResult::isNull( int field )
{
#ifdef DEBUG_TDS
    qDebug( QString( "QTDSResult::isNull for field %1: %2" ).arg( field ).arg( set->buffer()->isNull( field ) ) );
#endif
    return set->buffer()->isNull( field );
}

bool QTDSResult::reset ( const QString& query )
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::reset: \"" + query + "\"" );
#endif
    cleanup();
    if ( !driver() )
	return FALSE;
    if ( !driver()-> isOpen() || driver()->isOpenError() )
	return FALSE;
    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    dbfreebuf( d->dbproc );
    if ( dbcmd( d->dbproc , query.local8Bit().data() ) == FAIL ) {
	setLastError( d->lastError );
	return FALSE;
    }

    if ( dbsqlexec( d->dbproc ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    if ( dbresults( d->dbproc ) != SUCCEED ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }

    setSelect( (DBCMDROW( d->dbproc ) == SUCCEED) );
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::reset: creating buffer, num cols:" + QString::number( dbnumcols( d->dbproc ) ) );
#endif
    for ( int i = 0; i < dbnumcols( d->dbproc ) ; ++i ) {
#ifdef DEBUG_TDS
	qDebug( "QTDSResult::reset: appending field: " + QString::number( i ) );
	qDebug( "QTDSResult::reset: col %d is %s (type %i: %s; colLen: %i)", i, dbcolname( d->dbproc, i+1 ),
								    dbcoltype( d->dbproc, i+1 ),
								    QVariant::typeToName( qDecodeTDSType( dbcoltype( d->dbproc, i+1 ) ) ),
								    dbcollen( d->dbproc, i+1 ) );
#endif
	QVariant::Type vType = qDecodeTDSType( dbcoltype( d->dbproc, i+1 ) );
	RETCODE ret = -1;
	void* p = 0;
	QTDSClientNullData* nd = new QTDSClientNullData();
	switch ( vType ) {
	case QVariant::Int:	
	    p = buf->append( 4, vType, nd );
	    if ( p )
		ret = dbbind( d->dbproc, i+1, INTBIND, (DBINT) 4, (unsigned char *)p );
	    break;
	case QVariant::String:
	    p = buf->append( dbcollen( d->dbproc, i+1 ) + 1, vType, nd );
	    if ( p )
		ret = dbbind( d->dbproc, i+1, STRINGBIND, DBINT(dbcollen( d->dbproc, i+1 ) + 1), (unsigned char *)p );
	    break;
	case QVariant::Double:
	    p = buf->append( 8, vType, nd );
	    if ( p )
		ret = dbbind( d->dbproc, i+1, FLT8BIND, (DBINT) 8, (unsigned char *)p );
	    break;
	case QVariant::DateTime:
	    p = buf->append( 8, vType, nd );
	    if ( p )
		ret = dbbind( d->dbproc, i+1, DATETIMEBIND, (DBINT) 8, (unsigned char *)p );
	    break;
	default: //don't bind the field since we do not support it
	    delete nd;
	    nd = 0;
#ifdef QT_CHECK_RANGE
	    qDebug( "QTDSResult::reset: Unsupported type for field \"%s\"", dbcolname( d->dbproc, i+1 ) );
#endif
	    break;
	}
	if ( ret == SUCCEED ) {
	    ret = dbnullbind( d->dbproc, i+1, (DBINT*)buf->nullData( i )->binder() );
	}
	if ( ( ret != SUCCEED ) && ( ret != -1 ) ) {
	    setLastError( d->lastError );
#ifdef DEBUG_TDS
	    qDebug( "QTDSResult::reset: some bind error!" );
#endif
	    return FALSE;
	}
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::reset: done, active" );
#endif	
    }

    setActive( TRUE );
    return TRUE;
}

int QTDSResult::size()
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::size()" );
#endif
    return -1;
}

int QTDSResult::numRowsAffected()
{
#ifdef DEBUG_TDS
    qDebug( "QTDSResult::numRowsAffected()" );
#endif
    return DBCOUNT( d->dbproc );
}

///////////////////////////////////////////////////////////////////

QTDSDriver::QTDSDriver( QObject * parent, const char * name )
    : QSqlDriver(parent,name ? name : "QTDS")
{
    init();
    dberrhandle( (QERRHANDLE)qTdsErrHandler );
    dbmsghandle( (QMSGHANDLE)qTdsMsgHandler );
}

void QTDSDriver::init()
{
    d = new QTDSPrivate();
}

QTDSDriver::~QTDSDriver()
{
    dberrhandle( 0 );
    dbmsghandle( 0 );
    // dbexit also calls dbclose if neccessary
    dbexit();
    if ( ( d->dbproc ) && ( errs.find( d->dbproc ) ) ) {
	// remove dbproc from error handling dict
	errs.remove( d->dbproc );
    }
    delete d;
}

bool QTDSDriver::hasTransactionSupport() const
{
    return TRUE;
}

bool QTDSDriver::hasQuerySizeSupport() const
{
    // unfortunately no QuerySize support...
    return FALSE;
}

bool QTDSDriver::canEditBinaryFields() const
{
    // not supported yet.
    return FALSE;
}

bool QTDSDriver::open( const QString & db,
		       const QString & user,
		       const QString & password,
		       const QString & host,
		       int /*port*/ )
{
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::open()" );
#endif
    if ( isOpen() )
	close();
    if ( !dbinit() ) {
	setLastError( d->lastError );
	setOpenError( TRUE );
	return FALSE;
    }
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::open: inited" );
#endif
    d->login = dblogin();
    if ( !d->login ) {
	setLastError( d->lastError );
	setOpenError( TRUE );
	return FALSE;
    }
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::open: logged in as %s %s", user.local8Bit().data(), password.local8Bit().data() );
#endif
    DBSETLPWD( d->login, password.local8Bit().data() );
    DBSETLUSER( d->login, user.local8Bit().data() );
//    DBSETLAPP( d->login, "QTDS7"); //Well, we can set the name of the app here...

    d->dbproc = dbopen( d->login, host.local8Bit().data() );
    if ( !d->dbproc ) {
	// we have to manually set the error here because the error handler won't fire when no dbproc exists
	setLastError( QSqlError( QString::null, tr( "Could not open database connection" ), QSqlError::Connection ) );
	setOpenError( TRUE );
	return FALSE;
    }
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::open: opened" );
#endif
    if ( dbuse( d->dbproc, db.local8Bit().data() ) == FAIL ) {
	setLastError( QSqlError( QString::null, tr( "Could not open database" ), QSqlError::Connection ) );
	setOpenError( TRUE );
	return FALSE;
    }
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::open: database used" );
#endif
    setOpen( TRUE );
    dbName = db;
    hostName = host;
    return TRUE;
}

void QTDSDriver::close()
{
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::close()" );
#endif
    if ( isOpen() ) {
	dbclose( d->dbproc );
	errs.remove ( d->dbproc );
	d->dbproc = 0;
	setOpen( FALSE );
	setOpenError( FALSE );
    }
}

QSqlQuery QTDSDriver::createQuery() const
{
#ifdef DEBUG_TDS
    qDebug("QTDSDriver::createQuery()");
#endif
    QTDSPrivate d2;
    d2.login = d->login;

    d2.dbproc = dbopen( d2.login, hostName.local8Bit().data() );
    if ( !d2.dbproc ) {
	return QSqlQuery();
    }
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::createQuery: opened" );
#endif
    if ( dbuse( d2.dbproc, dbName.local8Bit().data() ) == FAIL ) {
	return QSqlQuery();
    }
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::createQuery: database used" );
#endif

    return QSqlQuery( new QTDSResult( this, &d2 ) );
}

bool QTDSDriver::beginTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QTDSDriver::beginTransaction: Database not open" );
#endif
	return FALSE;
    }
    if ( dbcmd( d->dbproc, "BEGIN TRANSACTION" ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    if ( dbsqlexec( d->dbproc ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    while( dbresults( d->dbproc ) == NO_MORE_RESULTS ) {}
    dbfreebuf( d->dbproc );
    return TRUE;
}

bool QTDSDriver::commitTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QTDSDriver::commitTransaction: Database not open" );
#endif
	return FALSE;
    }
    if ( dbcmd( d->dbproc, "COMMIT TRANSACTION" ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    if ( dbsqlexec( d->dbproc ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    while( dbresults( d->dbproc ) == NO_MORE_RESULTS ) {}
    dbfreebuf( d->dbproc );
    return TRUE;
}

bool QTDSDriver::rollbackTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QTDSDriver::rollbackTransaction: Database not open" );
#endif
	return FALSE;
    }
    if ( dbcmd( d->dbproc, "ROLLBACK TRANSACTION" ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    if ( dbsqlexec( d->dbproc ) == FAIL ) {
	setLastError( d->lastError );
	dbfreebuf( d->dbproc );
	return FALSE;
    }
    while( dbresults( d->dbproc ) == NO_MORE_RESULTS ) {}
    dbfreebuf( d->dbproc );
    return TRUE;
}

QSqlRecord QTDSDriver::record( const QSqlQuery& query ) const
{
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::record: " + query.lastQuery() );
#endif
    QSqlRecord fil;
    if ( !isOpen() )
	return fil;
    if ( query.isActive() && query.driver() == this ) {
	QTDSResult* result = (QTDSResult*)query.result();
	int count = dbnumcols ( result->d->dbproc );
	for ( int i = 0; i < count; ++i ) {
	    QString name = dbcolname( result->d->dbproc, i+1 );
	    QVariant::Type type = qDecodeTDSType( dbcoltype( result->d->dbproc, i+1 ) );
	    QSqlField rf( name, type );
	    fil.append( rf );
	}
    }
    return fil;
}

QSqlRecord QTDSDriver::record( const QString& tablename ) const
{
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::record: " + tablename );
#endif
    QSqlRecord fil;
    if ( !isOpen() )
	return fil;
    QSqlQuery t = createQuery();
    QString stmt ( "select name,type from syscolumns where id = (select id from sysobjects where name = '%1')" );
    t.exec( stmt.arg( tablename ) );
    while ( t.next() ) {
	QVariant::Type ty = qDecodeTDSType( t.value(1).toInt() );
	QSqlField f( t.value(0).toString().stripWhiteSpace(), ty );
	fil.append( f );
    }
    return fil;
}

QStringList QTDSDriver::tables( const QString& /*user*/ ) const
{
#ifdef DEBUG_TDS
    qDebug( "QTDSDriver::tables" );
#endif
    QStringList list;

    if ( !isOpen() )
	return list;

    QSqlQuery t = createQuery();
    t.exec( "select name from sysobjects where type='U'" );
    while ( t.next() ) {
	list.append( t.value(0).toString().stripWhiteSpace() );
    }

    return list;
}

QString QTDSDriver::formatValue( const QSqlField* field,
				  bool ) const
{
#ifdef DEBUG_TDS
    qDebug("QTDSDriver::formatValue: " + field->name() );
#endif
    QString r;
    if ( field->isNull() )
	r = nullText();
    else if ( field->type() == QVariant::DateTime ) {
	if ( field->value().toDateTime().isValid() ){
	    r = field->value().toDateTime().toString( "'yyyyMMdd hh:mm:ss'" );
	} else
	    r = nullText();
    } else {
	r = QSqlDriver::formatValue( field );
    }
    return r;
}

QSqlIndex QTDSDriver::primaryIndex( const QString& tablename ) const
{
    QSqlRecord rec = record( tablename );

    QSqlIndex idx( tablename );
    if ( ( !isOpen() ) || ( tablename.isEmpty() ) )
	return idx;

    QSqlQuery t = createQuery();
    t.exec( QString( "sp_helpindex '%1'" ).arg( tablename ) );
    if ( t.next() ) {
	QStringList fNames = QStringList::split( ',', t.value(2).toString().stripWhiteSpace(), FALSE );
	QRegExp regx("\\s*(\\S+)(?:\\s+(DESC|desc))?\\s*");
	for( QStringList::Iterator it = fNames.begin(); it != fNames.end(); ++it ) {
	    regx.search( *it );
	    QSqlField f( regx.cap( 1 ), rec.field( regx.cap( 1 ) )->type() );
	    if ( regx.cap( 2 ).lower() == "desc" ) {
		idx.append( f, TRUE );
	    } else {
		idx.append( f, FALSE );
	    }
	}
	idx.setName( t.value(0).toString().stripWhiteSpace() );
    }
    return idx;
    return QSqlIndex();
}
