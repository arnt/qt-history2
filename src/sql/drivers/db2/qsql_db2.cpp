/****************************************************************************
**
** Implementation of IBM DB2 driver classes
**
** Created : 021121
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

// ### UNICODE???

#include "qsql_db2.h"
#include <qsqlrecord.h>
#include <qdatetime.h>
#include <qptrvector.h>
#include <private/qsqlextension_p.h>

#include <sqlcli.h>
#include <sqlcli1.h>

static const int COLNAMESIZE = 255;

class QDB2DriverPrivate
{
public:
    QDB2DriverPrivate(): hEnv( 0 ), hDbc( 0 ) {}
    SQLHANDLE hEnv;
    SQLHANDLE hDbc;
    QString user;
};

class QDB2PreparedExtension : public QSqlExtension
{
public:
    QDB2PreparedExtension( QDB2Result * r )
	: result( r ) {}

    bool prepare( const QString& query )
    {
	return result->prepare( query );
    }

    bool exec()
    {
	return result->exec();
    }
    
    QDB2Result * result;
};

class QDB2ResultPrivate
{
public:
    QDB2ResultPrivate( const QDB2DriverPrivate* d ): dp( d ), hStmt( 0 )
    {
	valueCache.setAutoDelete( TRUE );
    }
    
    const QDB2DriverPrivate* dp;
    SQLHANDLE hStmt;
    QSqlRecordInfo recInf;
    QPtrVector<QVariant> valueCache;
};

QString qWarnDB2Handle( int handleType, SQLHANDLE handle )
{
    SQLINTEGER nativeCode;
    SQLSMALLINT msgLen;
    SQLRETURN r = SQL_ERROR;
    SQLTCHAR state[ SQL_SQLSTATE_SIZE + 1 ];
    SQLTCHAR description[ SQL_MAX_MESSAGE_LENGTH ];
    r = SQLGetDiagRec( handleType,
		       handle,
		       1,
		       (SQLTCHAR*) state,
		       &nativeCode,
		       (SQLTCHAR*) description,
		       SQL_MAX_MESSAGE_LENGTH - 1, /* in bytes, not in characters */
		       &msgLen );
    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO )
	return QString( (const char*)description );
    return QString::null;
}

QString qDB2Warn( const QDB2DriverPrivate* d )
{
    return ( qWarnDB2Handle( SQL_HANDLE_ENV, d->hEnv ) + " "
	     + qWarnDB2Handle( SQL_HANDLE_DBC, d->hDbc ) );
}

QString qDB2Warn( const QDB2ResultPrivate* d )
{
    return ( qWarnDB2Handle( SQL_HANDLE_ENV, d->dp->hEnv ) + " "
	     + qWarnDB2Handle( SQL_HANDLE_DBC, d->dp->hDbc )
	     + qWarnDB2Handle( SQL_HANDLE_STMT, d->hStmt ) );
}

void qSqlWarning( const QString& message, const QDB2DriverPrivate* d )
{
#ifdef QT_CHECK_RANGE
    qWarning( message + "\tError:" + qDB2Warn( d ) );
#else
    Q_UNUSED( message );
    Q_UNUSED( d );
#endif
}

void qSqlWarning( const QString& message, const QDB2ResultPrivate* d )
{
#ifdef QT_CHECK_RANGE
    qWarning( message + "\tError:" + qDB2Warn( d ) );
#else
    Q_UNUSED( message );
    Q_UNUSED( d );
#endif
}

QSqlError qMakeError( const QString& err, int type, const QDB2DriverPrivate* p )
{
    return QSqlError( "QDB2: " + err, qDB2Warn(p), type );
}

QSqlError qMakeError( const QString& err, int type, const QDB2ResultPrivate* p )
{
    return QSqlError( "QDB2: " + err, qDB2Warn(p), type );
}

QVariant::Type qDecodeDB2Type( SQLSMALLINT sqltype )
{
    QVariant::Type type = QVariant::Invalid;
    switch ( sqltype ) {
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_BIGINT: //use high precision binding
	type = QVariant::Double;
	break;
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIT:
    case SQL_TINYINT:
	type = QVariant::Int;
	break;
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
	type = QVariant::ByteArray;
	break;
    case SQL_DATE:
    case SQL_TYPE_DATE:
	type = QVariant::Date;
	break;
    case SQL_TIME:
    case SQL_TYPE_TIME:
	type = QVariant::Time;
	break;
    case SQL_TIMESTAMP:
    case SQL_TYPE_TIMESTAMP:
	type = QVariant::DateTime;
	break;
    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_WLONGVARCHAR:
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
	type = QVariant::String;
	break;
    default:
	type = QVariant::CString;
	break;
    }
    return type;
}

QSqlFieldInfo qMakeFieldInfo( const QDB2ResultPrivate* d, int i )
{
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLUINTEGER colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;
    SQLCHAR colName[ COLNAMESIZE ];
    r = SQLDescribeCol( d->hStmt,
			i+1,
			colName,
			(SQLSMALLINT) COLNAMESIZE,
			&colNameLen,
			&colType,
			&colSize,
			&colScale,
			&nullable );

    if ( r != SQL_SUCCESS ) {
	qSqlWarning( QString("qMakeFieldInfo: Unable to describe column %1").arg(i), d );
	return QSqlFieldInfo();
    }
    QString qColName( (const char*) colName );
    // nullable can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    int required = -1;
    if ( nullable == SQL_NO_NULLS ) {
	required = 1;
    } else if ( nullable == SQL_NULLABLE ) {
	required = 0;
    }
    QVariant::Type type = qDecodeDB2Type( colType );
    return QSqlFieldInfo( qColName,
    			  type,
    			  required,
    			  (int) colSize == 0 ? -1 : (int) colSize,
    			  (int) colScale == 0 ? -1 : (int) colScale,
    			  QVariant(),
    			  (int) colType );
}

int qGetIntData( SQLHANDLE hStmt, int column, bool& isNull )
{
    SQLINTEGER intbuf;
    isNull = FALSE;
    SQLINTEGER lengthIndicator = 0;
    SQLRETURN r = SQLGetData( hStmt,
			      column + 1,
			      SQL_C_SLONG,
			      (SQLPOINTER) &intbuf,
			      0,
			      &lengthIndicator );
    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) &&  lengthIndicator == SQL_NULL_DATA ) {
	isNull = TRUE;
	return 0;
    }
    return (int) intbuf;
}

QString qGetStringData( SQLHANDLE hStmt, int column, int colSize, bool& isNull )
{
    QString     fieldVal;
    SQLRETURN   r = SQL_ERROR;
    SQLINTEGER  lengthIndicator = 0;

    if ( colSize <= 0 )
	colSize = 255;
    else if ( colSize > 65536 ) // limit buffer size to 64 KB 
	colSize = 65536;
    else
	colSize++; // make sure there is room for more than the 0 termination
    char* buf = new char[ colSize ];
    while ( TRUE ) {
	r = SQLGetData( hStmt,
			column+1,
			SQL_C_CHAR,
			(SQLPOINTER)buf,
			colSize,
			&lengthIndicator );
	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
	    if ( lengthIndicator == SQL_NULL_DATA || lengthIndicator == SQL_NO_TOTAL ) {
		fieldVal = QString::null;
		isNull = TRUE;
		break;
	    }
	    // if SQL_SUCCESS_WITH_INFO is returned, indicating that
	    // more data can be fetched, the length indicator does NOT
	    // contain the number of bytes returned - it contains the
	    // total number of bytes that CAN be fetched
	    // colSize-1: remove 0 termination when there is more data to fetch
	    int rSize = ( r == SQL_SUCCESS_WITH_INFO ) ? colSize - 1 : lengthIndicator;
	    buf[ rSize ] = 0;
	    fieldVal += buf;
	    if ( lengthIndicator < colSize ) {
		// workaround for Drivermanagers that don't return SQL_NO_DATA
		break;
	    }
	} else if ( r == SQL_NO_DATA ) {
	    break;
	} else {
#ifdef QT_CHECK_RANGE
	    qWarning( "qGetStringData: Error while fetching data (%d)", r );
#endif
	    fieldVal = QString::null;
	    break;
	}
    }
    delete[] buf;
    return fieldVal;
}

QByteArray qGetBinaryData( SQLHANDLE hStmt, int column, SQLINTEGER& lengthIndicator, bool& isNull )
{
    QByteArray fieldVal;
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLUINTEGER colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;

    SQLCHAR colName[ COLNAMESIZE ];
    r = SQLDescribeCol( hStmt,
			column+1,
			colName,
			COLNAMESIZE,
			&colNameLen,
			&colType,
			&colSize,
			&colScale,
			&nullable );
#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qWarning( QString( "qGetBinaryData: Unable to describe column %1" ).arg( column ) );
#endif
    // SQLDescribeCol may return 0 if size cannot be determined
    if ( !colSize )
	colSize = 255;
    else if ( colSize > 65536 ) // read the field in 64 KB chunks
	colSize = 65536;
    char * buf = new char[ colSize ];
    while ( TRUE ) {
	r = SQLGetData( hStmt,
			column+1,
			SQL_C_BINARY,
			(SQLPOINTER) buf,
			colSize,
			&lengthIndicator );
	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
	    if ( lengthIndicator == SQL_NULL_DATA ) {
		isNull = TRUE;
		break;
	    } else {
		int rSize;
		r == SQL_SUCCESS ? rSize = lengthIndicator : rSize = colSize;
		if ( lengthIndicator == SQL_NO_TOTAL ) // size cannot be determined
		    rSize = colSize;
		// NB! This is not a memleak - the mem will be deleted by QByteArray when
		// no longer ref'd
		char * tmp = (char *) malloc( rSize + fieldVal.size() );
		if ( fieldVal.size() )
		    memcpy( tmp, fieldVal.data(), fieldVal.size() );
		memcpy( tmp + fieldVal.size(), buf, rSize );
		fieldVal = fieldVal.assign( tmp, fieldVal.size() + rSize );

		if ( r == SQL_SUCCESS ) // the whole field was read in one chunk
		    break;
	    }
	} else {
	    break;
	}
    }
    delete [] buf;
    return fieldVal;
}

void qSplitTableQualifier( const QString & qualifier, QString * catalog,
			   QString * schema, QString * table )
{
    if ( !catalog || !schema || !table )
	return;
    QStringList l = QStringList::split( ".", qualifier, TRUE );
    if ( l.count() > 3 )
	return; // can't possibly be a valid table qualifier
    int i = 0, n = l.count();
    if ( n == 1 ) {
	*table = qualifier;
    } else {
	for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
	    if ( n == 3 ) {
		if ( i == 0 )
		    *catalog = *it;
		else if ( i == 1 )
		    *schema = *it;
		else if ( i == 2 )
		    *table = *it;
	    } else if ( n == 2 ) {
		if ( i == 0 )
		    *schema = *it;
		else if ( i == 1 )
		    *table = *it;
	    }
	    i++;
	}
    }
}

// creates a QSqlFieldInfo from a valid hStmt generated
// by SQLColumns. The hStmt has to point to a valid position.
QSqlFieldInfo qMakeFieldInfo( const SQLHANDLE hStmt )
{
    bool isNull;
    QString fname = qGetStringData( hStmt, 3, -1, isNull );
    int type = qGetIntData( hStmt, 4, isNull ); // column type
    int required = qGetIntData( hStmt, 10, isNull ); // nullable-flag
    // required can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    if ( required == SQL_NO_NULLS ) {
	required = 1;
    } else if ( required == SQL_NULLABLE ) {
	required = 0;
    } else {
	required = -1;
    }
    int size = qGetIntData( hStmt, 6, isNull ); // column size
    int prec = qGetIntData( hStmt, 8, isNull ); // precision
    return QSqlFieldInfo( fname, qDecodeDB2Type( type ), required, size, prec, QVariant(), type );
}

/************************************/

QDB2Result::QDB2Result( const QDB2Driver* dr, const QDB2DriverPrivate* dp )
    : QSqlResult( dr )
{
    d = new QDB2ResultPrivate( dp );
    setExtension( new QDB2PreparedExtension( this ) );
}

QDB2Result::~QDB2Result()
{
    if ( d->hStmt ) {
	SQLRETURN r = SQLFreeHandle( SQL_HANDLE_STMT, d->hStmt );
	if ( r != SQL_SUCCESS )
	    qSqlWarning( "QDB2Driver: Unable to free statement handle " + QString::number(r), d );
    }    
    delete d;
}

bool QDB2Result::reset ( const QString& query )
{
    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    SQLRETURN r;

    d->recInf.clear();
    d->valueCache.clear();
    if ( !d->hStmt ) {
	r = SQLAllocHandle( SQL_HANDLE_STMT,
			    d->dp->hDbc,
			    &d->hStmt );
	if ( r != SQL_SUCCESS ) {
	    qSqlWarning( "QDB2Result::reset: Unable to allocate statement handle", d );
	    return FALSE;
	}
    } else {
	r = SQLFreeStmt( d->hStmt, SQL_CLOSE );
	if ( r != SQL_SUCCESS ) {
	    qSqlWarning( "QDB2Result::reset: Unable to close statement handle", d );
	    return FALSE;
	}
    }
    
    if ( isForwardOnly() ) {
	r = SQLSetStmtAttr( d->hStmt,
			    SQL_ATTR_CURSOR_TYPE,
			    (SQLPOINTER) SQL_CURSOR_FORWARD_ONLY,
			    SQL_IS_UINTEGER );
    } else {
	r = SQLSetStmtAttr( d->hStmt,
			    SQL_ATTR_CURSOR_TYPE,
			    (SQLPOINTER) SQL_CURSOR_STATIC,
			    SQL_IS_UINTEGER );
    }
    if ( r != SQL_SUCCESS ) {
	qSqlWarning( QString().sprintf("QDB2Result::reset: Unable to set %s attribute.", isForwardOnly() ? "SQL_CURSOR_FORWARD_ONLY" : "SQL_CURSOR_STATIC" ), d );
	return FALSE;
    }
    
    r = SQLExecDirect( d->hStmt,
		       (SQLCHAR*) query.local8Bit().data(),
		       (SQLINTEGER) query.length() );
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	setLastError( qMakeError( "Unable to execute statement", QSqlError::Statement, d ) );
	return FALSE;
    }
    SQLSMALLINT count;
    r = SQLNumResultCols( d->hStmt, &count );
    if ( count ) {
	setSelect( TRUE );
	for ( int i = 0; i < count; ++i ) {
	    d->recInf.append( qMakeFieldInfo( d, i ) );
	}
    } else {
	setSelect( FALSE );
    }
    d->valueCache.resize( count );
    setActive( TRUE );
    return TRUE;
}

bool QDB2Result::prepare( const QString& query )
{
    return FALSE;
}

bool QDB2Result::exec()
{
    return FALSE;
}

bool QDB2Result::fetch( int i )
{ 
    if ( isForwardOnly() && i < at() )
	return FALSE;
    if ( i == at() )
	return TRUE;
    d->valueCache.fill( 0 );
    int actualIdx = i + 1;
    if ( actualIdx <= 0 ) {
	setAt( QSql::BeforeFirst );
	return FALSE;
    }
    SQLRETURN r;
    if ( isForwardOnly() ) {
	bool ok = TRUE;
	while ( ok && i > at() )
	    ok = fetchNext();
	return ok;
    } else {
	r = SQLFetchScroll( d->hStmt,
			    SQL_FETCH_ABSOLUTE,
			    actualIdx );
    }
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( i );
    return TRUE;
}

bool QDB2Result::fetchNext()
{
    SQLRETURN r;
    d->valueCache.fill( 0 );
    r = SQLFetchScroll( d->hStmt,
		       SQL_FETCH_NEXT,
		       0 );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( at() + 1 );
    return TRUE;
}

bool QDB2Result::fetchFirst()
{
    if ( isForwardOnly() && at() != QSql::BeforeFirst )
	return FALSE;
    if ( isForwardOnly() )
	return fetchNext();
    d->valueCache.fill( 0 );
    SQLRETURN r;
    r = SQLFetchScroll( d->hStmt,
			SQL_FETCH_FIRST,
			0 );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( 0 );
    return TRUE;
}

bool QDB2Result::fetchLast()
{ 
    d->valueCache.fill( 0 );
    
    int i = at();
    if ( i == QSql::AfterLast ) {
	if ( isForwardOnly() ) {
	    return FALSE;
	} else {
	    if ( !fetch( 0 ) )
		return FALSE;
	    i = at();
	}
    }

    while ( fetchNext() )
	++i;
    
    if ( i == QSql::BeforeFirst ) {
	setAt( QSql::AfterLast );
	return FALSE;
    }

    if ( !isForwardOnly() )
	return fetch( i );

    setAt( i );
    return TRUE;
}


QVariant QDB2Result::data( int field )
{
    SQLRETURN r = 0;
    SQLINTEGER lengthIndicator = 0;
    const QSqlFieldInfo info = d->recInf[ field ];
    bool isNull = FALSE;
    
    if ( field >= (int)d->valueCache.size() )
	return QVariant();
    
    if ( d->valueCache[ field ] )
	return *d->valueCache[ field ];


    QVariant* v = 0;
    switch ( info.type() ) {
	case QVariant::Int: {
	    int val = qGetIntData( d->hStmt, field, isNull );
	    if ( isNull ) {
		v = new QVariant();
		v->cast( QVariant::Int );
	    } else {
		v = new QVariant( val );
	    }
	    break; }
	case QVariant::Date: {
	    DATE_STRUCT dbuf;
	    r = SQLGetData( d->hStmt,
			    field + 1,
			    SQL_C_DATE,
			    (SQLPOINTER) &dbuf,
			    0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) )
		v = new QVariant( QDate( dbuf.year, dbuf.month, dbuf.day ) );
	    else
		v = new QVariant( QDate() );
	break; }
	case QVariant::Time: {
	    TIME_STRUCT tbuf;
	    r = SQLGetData( d->hStmt,
			    field + 1,
			    SQL_C_TIME,
			    (SQLPOINTER) &tbuf,
			    0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) )
		v = new QVariant( QTime( tbuf.hour, tbuf.minute, tbuf.second ) );
	    else
		v = new QVariant( QTime() );
	break; }
	case QVariant::DateTime: {
	    TIMESTAMP_STRUCT dtbuf;
	    r = SQLGetData( d->hStmt,
			    field + 1,
			    SQL_C_TIMESTAMP,
			    (SQLPOINTER) &dtbuf,
			    0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) )
		v = new QVariant( QDateTime( QDate( dtbuf.year, dtbuf.month, dtbuf.day ), QTime( dtbuf.hour, dtbuf.minute, dtbuf.second ) ) );
	    else
		v = new QVariant( QDateTime() );
	break; }
        case QVariant::ByteArray:
	    v = new QVariant( qGetBinaryData( d->hStmt, field, lengthIndicator, isNull ) );
	    break;
	case QVariant::Double:
	    // length + 1 for the comma
	    v = new QVariant( qGetStringData( d->hStmt, field, info.length() + 1, isNull ) );
	    break;
	case QVariant::String:
	case QVariant::CString:
	default:
	    v = new QVariant( qGetStringData( d->hStmt, field, info.length(), isNull ) );
	    break;
    }
    d->valueCache.insert( field, v );
    return *v;
}

bool QDB2Result::isNull( int i )
{
    if ( i >= (int)d->valueCache.size() )
	return TRUE;
    
    if ( d->valueCache[ i ] )
	return d->valueCache[ i ]->isNull();
    return data( i ).isNull();
}

int QDB2Result::numRowsAffected()
{
    SQLINTEGER affectedRowCount = 0;
    SQLRETURN r = SQLRowCount( d->hStmt, &affectedRowCount );
    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO )
	return affectedRowCount;
    else
	qSqlWarning( "QDB2Result::numRowsAffected: Unable to count affected rows", d );
    return -1;
}

int QDB2Result::size()
{ 
    return -1;
}

/************************************/

QDB2Driver::QDB2Driver()
    : QSqlDriver()
{
    d = new QDB2DriverPrivate;
}

QDB2Driver::~QDB2Driver()
{
    close();
    delete d;
}


bool QDB2Driver::open( const QString& db, const QString& user, const QString& password, const QString&, int )
{
    if ( isOpen() )
      close();
    SQLRETURN r;
    r = SQLAllocHandle( SQL_HANDLE_ENV,
			SQL_NULL_HANDLE,
			&d->hEnv );
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	qSqlWarning( "QDB2Driver::open: Unable to allocate environment", d );
	setOpenError( TRUE );
	return FALSE;
    }
        
    r = SQLAllocHandle( SQL_HANDLE_DBC,
			d->hEnv,
			&d->hDbc );
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	qSqlWarning( "QODBCDriver::open: Unable to allocate connection", d );
	setOpenError( TRUE );
	return FALSE;
    }
        
    QString connQStr;
    connQStr = "DSN=" + db + ";UID=" + user + ";PWD=" + password + ";";
    SQLTCHAR connOut[ SQL_MAX_OPTION_STRING_LENGTH ];
    SQLSMALLINT cb;

    r = SQLDriverConnect( d->hDbc,
			  NULL,
			  (SQLCHAR*) connQStr.latin1(),
			  (SQLSMALLINT) connQStr.length(),
			  connOut,
			  SQL_MAX_OPTION_STRING_LENGTH,
			  &cb,
			  SQL_DRIVER_NOPROMPT );
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	setLastError( qMakeError( "Unable to connect", QSqlError::Connection, d ) );
	setOpenError( TRUE );
	return FALSE;
    }

    d->user = user.upper();
    setOpen( TRUE );
    return TRUE;
}

void QDB2Driver::close()
{
    SQLRETURN r;
    if ( d->hDbc ) {
	// Open statements/descriptors handles are automatically cleaned up by SQLDisconnect
	if ( isOpen() ) {
	    r = SQLDisconnect( d->hDbc );
	    if ( r != SQL_SUCCESS )
		qSqlWarning( "QDB2Driver::close: Unable to disconnect datasource", d );
	}
	r = SQLFreeHandle( SQL_HANDLE_DBC, d->hDbc );
	if ( r != SQL_SUCCESS )
	    qSqlWarning( "QDB2Driver::close: Unable to free connection handle", d );
	d->hDbc = 0;
    }

    if ( d->hEnv ) {
	r = SQLFreeHandle( SQL_HANDLE_ENV, d->hEnv );
	if ( r != SQL_SUCCESS )
	    qSqlWarning( "QDB2Driver::close: Unable to free environment handle", d );
	d->hEnv = 0;
    }
    setOpen( FALSE );
    setOpenError( FALSE );    
}

QSqlQuery QDB2Driver::createQuery() const 
{ 
    return QSqlQuery( new QDB2Result( this, d ) );
}

QSqlRecord QDB2Driver::record( const QString& tableName ) const
{
    return recordInfo( tableName ).toRecord();
}

QSqlRecord QDB2Driver::record( const QSqlQuery& query ) const
{ 
    return recordInfo( query ).toRecord();
}

QSqlRecordInfo QDB2Driver::recordInfo( const QString& tableName ) const
{
    QSqlRecordInfo fil;
    if ( !isOpen() )
	return fil;

    SQLHANDLE hStmt;
    QString catalog, schema, table;
    qSplitTableQualifier( tableName.upper(), &catalog, &schema, &table );

    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
	qSqlWarning( "QDB2Driver::record: Unable to allocate handle", d );
	return fil;
    }
    
    r = SQLSetStmtAttr( hStmt,
			SQL_ATTR_CURSOR_TYPE,
			(SQLPOINTER) SQL_CURSOR_FORWARD_ONLY,
			SQL_IS_UINTEGER );

    r =  SQLColumns( hStmt,
		     NULL,
		     0,
		     (SQLCHAR*) schema.latin1(),
		     schema.length(),
		     (SQLCHAR*) table.latin1(),
		     table.length(),
		     NULL,
		     0 );

    if ( r != SQL_SUCCESS )
	qSqlWarning( "QDB2Driver::record: Unable to execute column list", d );
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0 );
    while ( r == SQL_SUCCESS ) {
	fil.append( qMakeFieldInfo( hStmt ) );
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0 );
    }

    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QDB2Driver: Unable to free statement handle " + QString::number(r), d );

    return fil;
}

QSqlRecordInfo QDB2Driver::recordInfo( const QSqlQuery& query ) const
{
    if ( !isOpen() )
	return QSqlRecord();
    if ( query.isActive() && query.driver() == this ) {
	QDB2Result* result = (QDB2Result*) query.result();
	return result->d->recInf;
    }
    return QSqlRecord();
}

QStringList QDB2Driver::tables( const QString& /* user */ ) const
{
    QStringList tl;
    if ( !isOpen() )
	return tl;
	
    SQLHANDLE hStmt;

    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	qSqlWarning( "QDB2Driver::tables: Unable to allocate handle", d );
	return tl;
    }
    r = SQLSetStmtAttr( hStmt,
			SQL_ATTR_CURSOR_TYPE,
			(SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			SQL_IS_UINTEGER );

    // Prevent SQLTables to display all the system tables
    QString tableType = "TABLE";
    r = SQLTables( hStmt,
		   NULL,
		   0,
		   NULL,
		   0,
		   NULL,
		   0,
		   (SQLCHAR*) tableType.latin1(),
		   tableType.length() );

    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
	qSqlWarning( "QDB2Driver::tables: Unable to execute table list", d );
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0 );
    while ( r == SQL_SUCCESS ) {
	bool isNull;
	QString fieldVal = qGetStringData( hStmt, 2, -1, isNull );
	QString userVal = qGetStringData( hStmt, 1, -1, isNull );
	if ( userVal != d->user )
	    fieldVal = userVal + "." + fieldVal;
	tl.append( fieldVal );
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0 );
    }

    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QDB2Driver::tables: Unable to free statement handle " + QString::number(r), d );
    return tl;
}

QSqlIndex QDB2Driver::primaryIndex( const QString& tablename ) const
{
    QSqlIndex index( tablename );
    if ( !isOpen() )
	return index;
    QSqlRecord rec = record( tablename );

    SQLHANDLE hStmt;
    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
	qSqlWarning( "QDB2Driver::primaryIndex: Unable to list primary key", d );
	return index;
    }
    QString catalog, schema, table;
    qSplitTableQualifier( tablename.upper(), &catalog, &schema, &table );
    r = SQLSetStmtAttr( hStmt,
			SQL_ATTR_CURSOR_TYPE,
			(SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			SQL_IS_UINTEGER );

    r = SQLPrimaryKeys( hStmt,
			NULL,
			0,
			(SQLCHAR*) schema.latin1(),
			schema.length(),
			(SQLCHAR*) table.latin1(),
			table.length() );
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0 );

    bool isNull;
    QString cName, idxName;
    // Store all fields in a StringList because the driver can't detail fields in this FETCH loop
    while ( r == SQL_SUCCESS ) {
	cName = qGetStringData( hStmt, 3, -1, isNull ); // column name
	idxName = qGetStringData( hStmt, 5, -1, isNull ); // pk index name
	index.append( *(rec.field( cName )) );
	index.setName( idxName );
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0 );
    }
    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r!= SQL_SUCCESS )
	qSqlWarning( "QDB2Driver: Unable to free statement handle " + QString::number(r), d );
    return index;
}
