/****************************************************************************
**
** Implementation of IBM DB2 driver classes.
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

#include "qsql_db2.h"
#include <qsqlrecord.h>
#include <qdatetime.h>
#include <qvector.h>
#include <qmap.h>
#include <private/qinternal_p.h>

#ifndef UNICODE
#define UNICODE
#endif

#if defined(Q_CC_BOR)
// DB2's sqlsystm.h (included through sqlcli1.h) defines the SQL_BIGINT_TYPE
// and SQL_BIGUINT_TYPE to wrong the types for Borland; so do the defines to
// the right type before including the header
#define SQL_BIGINT_TYPE Q_INT64
#define SQL_BIGUINT_TYPE Q_UINT64
#endif

#include <sqlcli1.h>

static const int COLNAMESIZE = 255;
static const SQLSMALLINT qParamType[ 4 ] = { SQL_PARAM_INPUT, SQL_PARAM_INPUT, SQL_PARAM_OUTPUT, SQL_PARAM_INPUT_OUTPUT };

class QDB2DriverPrivate
{
public:
    QDB2DriverPrivate(): hEnv( 0 ), hDbc( 0 ) {}
    SQLHANDLE hEnv;
    SQLHANDLE hDbc;
    QString user;
};

class QDB2ResultPrivate
{
public:
    QDB2ResultPrivate( const QDB2DriverPrivate* d ): dp( d ), hStmt( 0 )
    {}
    ~QDB2ResultPrivate()
    {
	for ( int i = 0; i < valueCache.count(); ++i )
	    delete valueCache[ i ];
    }
    
    const QDB2DriverPrivate* dp;
    SQLHANDLE hStmt;
    QSqlRecord recInf;
    QVector<QVariant*> valueCache;
};

static QString qFromTChar( SQLTCHAR* str )
{
#ifdef UNICODE    
    return QString::fromUcs2( str );
#else
    return QString::fromLocal8Bit( (const char*) str );
#endif    
}

// dangerous!! (but fast). Don't use in functions that
// require out parameters!
static SQLTCHAR* qToTChar( const QString& str )
{
#ifdef UNICODE
    return (SQLTCHAR*)str.ucs2();
#else
    return (unsigned char*) str.ascii();
#endif
}

static QString qWarnDB2Handle( int handleType, SQLHANDLE handle )
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
	return QString( qFromTChar( description ) );
    return QString();
}

static QString qDB2Warn( const QDB2DriverPrivate* d )
{
    return ( qWarnDB2Handle( SQL_HANDLE_ENV, d->hEnv ) + " "
	     + qWarnDB2Handle( SQL_HANDLE_DBC, d->hDbc ) );
}

static QString qDB2Warn( const QDB2ResultPrivate* d )
{
    return ( qWarnDB2Handle( SQL_HANDLE_ENV, d->dp->hEnv ) + " "
	     + qWarnDB2Handle( SQL_HANDLE_DBC, d->dp->hDbc )
	     + qWarnDB2Handle( SQL_HANDLE_STMT, d->hStmt ) );
}

static void qSqlWarning( const QString& message, const QDB2DriverPrivate* d )
{
    qWarning( message + "\tError:" + qDB2Warn( d ) );
}

static void qSqlWarning( const QString& message, const QDB2ResultPrivate* d )
{
    qWarning( message + "\tError:" + qDB2Warn( d ) );
}

static QSqlError qMakeError( const QString& err, int type, const QDB2DriverPrivate* p )
{
    return QSqlError( "QDB2: " + err, qDB2Warn(p), type );
}

static QSqlError qMakeError( const QString& err, int type, const QDB2ResultPrivate* p )
{
    return QSqlError( "QDB2: " + err, qDB2Warn(p), type );
}

static QVariant::Type qDecodeDB2Type( SQLSMALLINT sqltype )
{
    QVariant::Type type = QVariant::Invalid;
    switch ( sqltype ) {
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
    case SQL_DECIMAL:
    case SQL_NUMERIC:
	type = QVariant::Double;
	break;
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIT:
    case SQL_TINYINT:
	type = QVariant::Int;
	break;
    case SQL_BIGINT:
	type = QVariant::LongLong;
	break;
    case SQL_BLOB:
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

static QSqlField qMakeFieldInfo( const QDB2ResultPrivate* d, int i )
{
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLUINTEGER colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;
    SQLTCHAR colName[ COLNAMESIZE ];
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
	return QSqlField();
    }
    QString qColName( qFromTChar( colName ) );
    // nullable can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    int required = -1;
    if ( nullable == SQL_NO_NULLS ) {
	required = 1;
    } else if ( nullable == SQL_NULLABLE ) {
	required = 0;
    }
    QVariant::Type type = qDecodeDB2Type( colType );
    return QSqlField( qColName,
		      type,
		      required,
		      (int) colSize == 0 ? -1 : (int) colSize,
		      (int) colScale == 0 ? -1 : (int) colScale,
		      QVariant(),
		      (int) colType );
}

static int qGetIntData( SQLHANDLE hStmt, int column, bool& isNull )
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
    if ( ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) || lengthIndicator == SQL_NULL_DATA ) {
	isNull = TRUE;
	return 0;
    }
    return (int) intbuf;
}

static double qGetDoubleData( SQLHANDLE hStmt, int column, bool& isNull )
{
    SQLDOUBLE dblbuf;
    isNull = FALSE;
    SQLINTEGER lengthIndicator = 0;
    SQLRETURN r = SQLGetData( hStmt,
			      column+1,
			      SQL_C_DOUBLE,
			      (SQLPOINTER) &dblbuf,
			      0,
			      &lengthIndicator );
    if ( ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) || lengthIndicator == SQL_NULL_DATA ) {
	isNull = TRUE;
	return 0.0;
    }

    return (double) dblbuf;
}

static SQLBIGINT qGetBigIntData( SQLHANDLE hStmt, int column, bool& isNull )
{
    SQLBIGINT lngbuf = Q_INT64_C( 0 );
    isNull = FALSE;
    SQLINTEGER lengthIndicator = 0;
    SQLRETURN r = SQLGetData( hStmt,
			      column+1,
			      SQL_C_SBIGINT,
			      (SQLPOINTER) &lngbuf,
			      0,
			      &lengthIndicator );
    if ( ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) || lengthIndicator == SQL_NULL_DATA )
	isNull = TRUE;

    return lngbuf;
}

static QString qGetStringData( SQLHANDLE hStmt, int column, int colSize, bool& isNull )
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
    SQLTCHAR* buf = new SQLTCHAR[ colSize ];

    while ( TRUE ) {
	r = SQLGetData( hStmt,
			column+1,
#ifdef UNICODE
			SQL_C_WCHAR,
#else
			SQL_C_CHAR,
#endif
			(SQLPOINTER)buf,
			colSize * sizeof( SQLTCHAR ),
			&lengthIndicator );
	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
	    if ( lengthIndicator == SQL_NULL_DATA || lengthIndicator == SQL_NO_TOTAL ) {
		fieldVal = QString();
		isNull = TRUE;
		break;
	    }
	    fieldVal += qFromTChar( buf );
	} else if ( r == SQL_NO_DATA ) {
	    break;
	} else {
	    qWarning( "qGetStringData: Error while fetching data (%d)", r );
	    fieldVal = QString();
	    break;
	}
    }
    delete[] buf;
    return fieldVal;
}

static QByteArray qGetBinaryData( SQLHANDLE hStmt, int column, SQLINTEGER& lengthIndicator, bool& isNull )
{
    QByteArray fieldVal;
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLUINTEGER colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;

    SQLTCHAR colName[ COLNAMESIZE ];
    r = SQLDescribeCol( hStmt,
			column+1,
			colName,
			COLNAMESIZE,
			&colNameLen,
			&colType,
			&colSize,
			&colScale,
			&nullable );
    if ( r != SQL_SUCCESS )
	qWarning( QString( "qGetBinaryData: Unable to describe column %1" ).arg( column ) );
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
		fieldVal.append( QByteArray( buf, rSize ) );
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

static void qSplitTableQualifier( const QString & qualifier, QString * catalog,
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

// creates a QSqlField from a valid hStmt generated
// by SQLColumns. The hStmt has to point to a valid position.
static QSqlField qMakeFieldInfo( const SQLHANDLE hStmt )
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
    return QSqlField( fname, qDecodeDB2Type( type ), required, size, prec, QVariant(), type );
}

static bool qMakeStatement( QDB2ResultPrivate* d, bool forwardOnly, bool setForwardOnly = TRUE )
{
    SQLRETURN r;
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
    
    if ( !setForwardOnly )
	return TRUE;
    
    if ( forwardOnly ) {
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
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	qSqlWarning( QString().sprintf("QDB2Result::reset: Unable to set %s attribute.", forwardOnly ? "SQL_CURSOR_FORWARD_ONLY" : "SQL_CURSOR_STATIC" ), d );
	return FALSE;
    }
    return TRUE;
}

/************************************/

QDB2Result::QDB2Result( const QDB2Driver* dr, const QDB2DriverPrivate* dp )
    : QSqlResult( dr )
{
    d = new QDB2ResultPrivate( dp );
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

    if ( !qMakeStatement( d, isForwardOnly() ) )
	return FALSE;
        
    r = SQLExecDirect( d->hStmt,
		       qToTChar( query ),
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
    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    SQLRETURN r;

    d->recInf.clear();
    d->valueCache.clear();
    
    if ( !qMakeStatement( d, isForwardOnly() ) )
	return FALSE;
    
    r = SQLPrepare( d->hStmt,
		    qToTChar( query ),
		    (SQLINTEGER) query.length() );

    if ( r != SQL_SUCCESS ) {
	qSqlWarning( "QDB2Result::prepare: Unable to prepare statement", d );
	return FALSE;
    }    
    return TRUE;
}

bool QDB2Result::exec()
{
    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    SQLRETURN r;

    d->recInf.clear();
    d->valueCache.clear();
    
    if ( !qMakeStatement( d, isForwardOnly(), FALSE ) )
	return FALSE;
        
    QList<QVirtualDestructor*> tmpStorage; // holds temporary ptrs. which will be deleted on fu exit
    tmpStorage.setAutoDelete( TRUE );
    
    QVector<QVariant>& values = boundValues();
    int i;
    for ( i = 0; i < values.count(); ++i ) {
	// bind parameters - only positional binding allowed
	QVariant val = values[i];
	SQLINTEGER * ind = new SQLINTEGER( SQL_NTS );
	tmpStorage.append( qAutoDeleter(ind) );
	if ( val.isNull() )
	    *ind = SQL_NULL_DATA;
    
	switch ( val.type() ) {
	    case QVariant::Date: {
		DATE_STRUCT * dt = new DATE_STRUCT;
		tmpStorage.append( qAutoDeleter(dt) );
		QDate qdt = val.toDate();
		dt->year = qdt.year();
		dt->month = qdt.month();
		dt->day = qdt.day();
		r = SQLBindParameter( d->hStmt,
				      i + 1,
				      qParamType[ (QFlag)(bindValueType(i)) & 3 ],
				      SQL_C_DATE,
				      SQL_DATE,
				      0,
				      0,
				      (void *) dt,
				      0,
				      *ind == SQL_NULL_DATA ? ind : NULL );
		break; }
	    case QVariant::Time: {
		TIME_STRUCT * dt = new TIME_STRUCT;
		tmpStorage.append( qAutoDeleter(dt) );
		QTime qdt = val.toTime();
		dt->hour = qdt.hour();
		dt->minute = qdt.minute();
		dt->second = qdt.second();
		r = SQLBindParameter( d->hStmt,
				      i + 1,
				      qParamType[ (QFlag)(bindValueType(i)) & 3 ],
				      SQL_C_TIME,
				      SQL_TIME,
				      0,
				      0,
				      (void *) dt,
				      0,
				      *ind == SQL_NULL_DATA ? ind : NULL );
		break; }
	    case QVariant::DateTime: {
		TIMESTAMP_STRUCT * dt = new TIMESTAMP_STRUCT;
		tmpStorage.append( qAutoDeleter(dt) );
		QDateTime qdt = val.toDateTime();
		dt->year = qdt.date().year();
		dt->month = qdt.date().month();
		dt->day = qdt.date().day();
		dt->hour = qdt.time().hour();
		dt->minute = qdt.time().minute();
		dt->second = qdt.time().second();
		dt->fraction = qdt.time().msec() * 1000000;
		r = SQLBindParameter( d->hStmt,
				      i + 1,
				      qParamType[ (QFlag)(bindValueType(i)) & 3 ],
				      SQL_C_TIMESTAMP,
				      SQL_TIMESTAMP,
				      0,
				      0,
				      (void *) dt,
				      0,
				      *ind == SQL_NULL_DATA ? ind : NULL );
		break; }
	    case QVariant::Int: {
		int * v = new int( val.toInt() );
		tmpStorage.append( qAutoDeleter(v) );
		r = SQLBindParameter( d->hStmt,
				      i + 1,
				      qParamType[ (QFlag)(bindValueType(i)) & 3 ],
				      SQL_C_SLONG,
				      SQL_INTEGER,
				      0,
				      0,
				      (void *) v,
				      0,
				      *ind == SQL_NULL_DATA ? ind : NULL );
		break; }
	    case QVariant::Double: {
		double * v = new double( val.toDouble() );
		tmpStorage.append( qAutoDeleter(v) );
		r = SQLBindParameter( d->hStmt,
				      i + 1,
				      qParamType[ (QFlag)(bindValueType(i)) & 3 ],
				      SQL_C_DOUBLE,
				      SQL_DOUBLE,
				      0,
				      0,
				      (void *) v,
				      0,
				      *ind == SQL_NULL_DATA ? ind : NULL );
		break; }
	    case QVariant::ByteArray: {
		if ( *ind != SQL_NULL_DATA ) {
		    *ind = val.toByteArray().size();
		}
		r = SQLBindParameter( d->hStmt,
				      i + 1,
				      qParamType[ (QFlag)(bindValueType(i)) & 3 ],
				      SQL_C_BINARY,
				      SQL_LONGVARBINARY,
				      val.toByteArray().size(),
				      0,
				      (void *) val.toByteArray().data(),
				      val.toByteArray().size(),
				      ind );
		break; }
	    case QVariant::String:
#ifdef UNICODE
	    {
		QString * str = new QString( val.toString() );
		//str->ucs2();
		int len = str->length()*2;
		tmpStorage.append( qAutoDeleter(str) );
		r = SQLBindParameter( d->hStmt,
				      i + 1,
				      qParamType[ (QFlag)(bindValueType(i)) & 3 ],
				      SQL_C_WCHAR,
				      SQL_WVARCHAR,
				      str->length(),
				      0,
				      (void *) str->unicode(),
				      len,
				      ind );
		break;
	    }
#endif
	    // fall through
	    default: {
		QByteArray * str = new QByteArray( val.toString().local8Bit() );
		tmpStorage.append( qAutoDeleter(str) );
		r = SQLBindParameter( d->hStmt,
				      i + 1,
				      qParamType[ (QFlag)(bindValueType(i)) & 3 ],
				      SQL_C_CHAR,
				      SQL_VARCHAR,
				      str->length() + 1,
				      0,
				      (void *) str->data(),
				      str->length() + 1,
				      ind );
		break; }
	}
	if ( r != SQL_SUCCESS ) {
	    qWarning( "QDB2Result::exec: unable to bind variable: " + qDB2Warn( d ) );
	    setLastError( qMakeError( "Unable to bind variable", QSqlError::Statement, d ) );
	    return FALSE;
	}
    }

    r = SQLExecute( d->hStmt );
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	qWarning( "QDB2Result::exec: Unable to execute statement: " + qDB2Warn( d ) );
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
    setActive( TRUE );
    d->valueCache.resize( count );

    //get out parameters
    if ( hasOutValues() ) {
	for ( i = 0; i < values.count(); ++i ) {    
	    SQLINTEGER* indPtr = qAutoDeleterData( (QAutoDeleter<SQLINTEGER>*)tmpStorage.first() );
	    if ( !indPtr )
		return FALSE;
	    bool isNull = (*indPtr == SQL_NULL_DATA);
	    tmpStorage.removeFirst();

	    if ( isNull ) {
		values[ i ] = QVariant( values[ i ].type() );
		if ( values[ i ].type() != QVariant::ByteArray )
		    tmpStorage.removeFirst();
		continue;
	    }

	    switch ( values[ i ].type() ) {
		case QVariant::Date: {
		    DATE_STRUCT * ds = qAutoDeleterData( (QAutoDeleter<DATE_STRUCT>*)tmpStorage.first() );
		    values[ i ] = QVariant( QDate( ds->year, ds->month, ds->day ) );
		    break; }
		case QVariant::Time: {
		    TIME_STRUCT * dt = qAutoDeleterData( (QAutoDeleter<TIME_STRUCT>*)tmpStorage.first() );
		     values[ i ] = QVariant( QTime( dt->hour, dt->minute, dt->second ) );
		    break; }
		case QVariant::DateTime: {
		    TIMESTAMP_STRUCT * dt = qAutoDeleterData( (QAutoDeleter<TIMESTAMP_STRUCT>*)tmpStorage.first() );
		    values[ i ] = QVariant( QDateTime( QDate( dt->year, dt->month, dt->day ), 
								       QTime( dt->hour, dt->minute, dt->second ) ) );
		    break; }
	        case QVariant::Int: {
		    int * v = qAutoDeleterData( (QAutoDeleter<int>*)tmpStorage.first() );
		    values[ i ] = QVariant( *v );
		    break; }
	        case QVariant::Double: {
		    double * v = qAutoDeleterData( (QAutoDeleter<double>*)tmpStorage.first() );
		    values[ i ] = QVariant( *v );
		    break; }
	        case QVariant::ByteArray:
		    break;
	        case QVariant::String:
#ifdef UNICODE
		    {
			QString * str = qAutoDeleterData( (QAutoDeleter<QString>*)tmpStorage.first() );
			values[ i ] = QVariant( *str );
			break;
		    }
#endif
		    // fall through
	        default: {
		    QByteArray * str = qAutoDeleterData( (QAutoDeleter<QByteArray>*)tmpStorage.first() );
		    values[ i ] = QVariant( *str );
		    break; }
	    }
	    if ( values[ i ].type() != QVariant::ByteArray )
		tmpStorage.removeFirst();
	}
    }
    return TRUE;
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
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( QString ( "Unable to fetch record %1" ).arg( i ), QSqlError::Statement, d ) );
	return FALSE;
    }
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
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to fetch next", QSqlError::Statement, d ) );
	return FALSE;
    }
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
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to fetch first", QSqlError::Statement, d ) );
	return FALSE;
    }
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
    if ( field >= d->recInf.count() ) {
	qWarning( "QDB2Result::data: column %d out of range", field );
	return QVariant();
    }
    SQLRETURN r = 0;
    SQLINTEGER lengthIndicator = 0;
    bool isNull = FALSE;
    const QSqlField* info = d->recInf.field(field);
    
    if ( !info || field >= (int)d->valueCache.size() )
	return QVariant();
    
    if ( d->valueCache[ field ] )
	return *d->valueCache[ field ];


    QVariant* v = 0;
    switch ( info->type() ) {
	case QVariant::LongLong:
	    v = new QVariant( (Q_LLONG) qGetBigIntData( d->hStmt, field, isNull ) );
	    break;
	case QVariant::Int:
	    v = new QVariant( qGetIntData( d->hStmt, field, isNull ) );
	    break;
	case QVariant::Date: {
	    DATE_STRUCT dbuf;
	    r = SQLGetData( d->hStmt,
			    field + 1,
			    SQL_C_DATE,
			    (SQLPOINTER) &dbuf,
			    0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) ) {
		v = new QVariant( QDate( dbuf.year, dbuf.month, dbuf.day ) );
	    } else {
		v = new QVariant( QDate() );
		isNull = TRUE;
	    }
	    break; }
	case QVariant::Time: {
	    TIME_STRUCT tbuf;
	    r = SQLGetData( d->hStmt,
			    field + 1,
			    SQL_C_TIME,
			    (SQLPOINTER) &tbuf,
			    0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) ) {
		v = new QVariant( QTime( tbuf.hour, tbuf.minute, tbuf.second ) );
	    } else {
		v = new QVariant( QTime() );
		isNull = TRUE;
	    }
	    break; }
	case QVariant::DateTime: {
	    TIMESTAMP_STRUCT dtbuf;
	    r = SQLGetData( d->hStmt,
			    field + 1,
			    SQL_C_TIMESTAMP,
			    (SQLPOINTER) &dtbuf,
			    0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) ) {
		v = new QVariant( QDateTime( QDate( dtbuf.year, dtbuf.month, dtbuf.day ),
					     QTime( dtbuf.hour, dtbuf.minute, dtbuf.second, dtbuf.fraction / 1000000 ) ) );
	    } else {
		v = new QVariant( QDateTime() );
		isNull = TRUE;
	    }
	    break; }
        case QVariant::ByteArray:
	    v = new QVariant( qGetBinaryData( d->hStmt, field, lengthIndicator, isNull ) );
	    break;
	case QVariant::Double:
	    if ( info->typeID() == SQL_DECIMAL || info->typeID() == SQL_NUMERIC )
		// length + 1 for the comma
		v = new QVariant( qGetStringData( d->hStmt, field, info->length() + 1, isNull ) );
	    else
		v = new QVariant( qGetDoubleData( d->hStmt, field, isNull ) );
	    break;
	case QVariant::String:
	default:
	    v = new QVariant( qGetStringData( d->hStmt, field, info->length(), isNull ) );
	    break;
    }
    if ( isNull )
	*v = QVariant(info->type());
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

QSqlRecord QDB2Result::record() const
{ 
    if (isActive())
	return d->recInf;
    return QSqlRecord();
}

/************************************/

QDB2Driver::QDB2Driver(QObject* parent)
    : QSqlDriver(parent)
{
    d = new QDB2DriverPrivate;
}

QDB2Driver::QDB2Driver( Qt::HANDLE env, Qt::HANDLE con, QObject* parent)
    : QSqlDriver(parent)
{
    d = new QDB2DriverPrivate;
    d->hEnv = (SQLHANDLE)env;
    d->hDbc = (SQLHANDLE)con;
    if ( env && con ) {
	setOpen( TRUE );
	setOpenError( FALSE );
    }
}

QDB2Driver::~QDB2Driver()
{
    close();
    delete d;
}

bool QDB2Driver::open( const QString& db, const QString& user, const QString& password, const QString&, int, 
		       const QString& connOpts )
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
	qSqlWarning( "QDB2Driver::open: Unable to allocate connection", d );
	setOpenError( TRUE );
	return FALSE;
    }
    // Set connection attributes
    QStringList raw = QStringList::split( ';', connOpts );
    QStringList opts;
    QMap<QString, QString> connMap;
    for ( QStringList::ConstIterator it = raw.begin(); it != raw.end(); ++it ) {
	QString tmp( *it );
	int idx;
	if ( (idx = tmp.indexOf( '=' )) != -1 )
	    connMap[ tmp.left( idx ) ] = tmp.mid( idx + 1 ).simplified();
	else
	    qWarning( "QDB2Driver::open: Illegal connect option value '%s'", tmp.latin1() );
    }
    if ( connMap.size() ) {
	QMap<QString, QString>::ConstIterator it;
	QString opt, val;
	SQLUINTEGER v = 0;
	for ( it = connMap.constBegin(); it != connMap.constEnd(); ++it ) {
	    opt = it.key().toUpper();
	    val = it.value().toUpper();
	    r = SQL_SUCCESS;
	    if ( opt == "SQL_ATTR_ACCESS_MODE" ) {
		if ( val == "SQL_MODE_READ_ONLY" ) {
		    v = SQL_MODE_READ_ONLY;
		} else if ( val == "SQL_MODE_READ_WRITE" ) {
		    v = SQL_MODE_READ_WRITE;
		} else {
		    qWarning( "QDB2Driver::open: Unknown option value '%s'", (*it).latin1() );
		    continue;
		}
		r = SQLSetConnectAttr( d->hDbc, SQL_ATTR_ACCESS_MODE, (SQLPOINTER) v, 0 );
	    } else if ( opt == "SQL_ATTR_LOGIN_TIMEOUT" ) {
		v = val.toUInt();
		r = SQLSetConnectAttr( d->hDbc, SQL_ATTR_LOGIN_TIMEOUT, (SQLPOINTER) v, 0 );
	    }
	    else {
		  qWarning( "QDB2Driver::open: Unknown connection attribute '%s'",
			    it.key().latin1() );
	    }
	    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO )
		qSqlWarning( QString("QDB2Driver::open: Unable to set connection attribute '%1'"
				    ).arg( opt ), d );
	}
    }
        
    QString connQStr;
    connQStr = "DSN=" + db + ";UID=" + user + ";PWD=" + password;
    SQLTCHAR connOut[ SQL_MAX_OPTION_STRING_LENGTH ];
    SQLSMALLINT cb;

    r = SQLDriverConnect( d->hDbc,
			  NULL,
			  qToTChar( connQStr ),
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

    d->user = user.toUpper();
    setOpen( TRUE );
    setOpenError( FALSE );
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
    QSqlRecord fil;
    if ( !isOpen() )
	return fil;

    SQLHANDLE hStmt;
    QString catalog, schema, table;
    qSplitTableQualifier( tableName.toUpper(), &catalog, &schema, &table );
    if ( schema.isEmpty() )
	schema = d->user;

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
		     qToTChar( schema ),
		     schema.length(),
		     qToTChar( table ),
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

QStringList QDB2Driver::tables( const QString& typeName ) const
{
    QStringList tl;
    if ( !isOpen() )
	return tl;

    int type = typeName.toInt();	
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

    QString tableType;
    if ( typeName.isEmpty() || ((type & (int)QSql::Tables) == (int)QSql::Tables) )
	tableType += "TABLE,";
    if ( (type & (int)QSql::Views) == (int)QSql::Views )
	tableType += "VIEW,";
    if ( (type & (int)QSql::SystemTables) == (int)QSql::SystemTables )
	tableType += "SYSTEM TABLE,";
    if ( tableType.isEmpty() )
	return tl;
    tableType.truncate( tableType.length() - 1 );

    r = SQLTables( hStmt,
		   NULL,
		   0,
		   NULL,
		   0,
		   NULL,
		   0,
		   qToTChar( tableType ),
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
    qSplitTableQualifier( tablename.toUpper(), &catalog, &schema, &table );
    r = SQLSetStmtAttr( hStmt,
			SQL_ATTR_CURSOR_TYPE,
			(SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			SQL_IS_UINTEGER );

    r = SQLPrimaryKeys( hStmt,
			NULL,
			0,
			qToTChar( schema ),
			schema.length(),
			qToTChar( table ),
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

bool QDB2Driver::hasFeature( DriverFeature f ) const
{
    switch ( f ) {
	case Transactions:
	    return TRUE;
	case QuerySize:
	    return FALSE;
	case BLOB:
	    return TRUE;
	case Unicode:
	// this is the query that shows the codepage for the types:
	// select typename, codepage from syscat.datatypes
#ifdef UNICODE
	    return TRUE;
#else
	    return FALSE;
#endif
	case PreparedQueries:
	    return TRUE;
	case PositionalPlaceholders:
	    return TRUE;
	case NamedPlaceholders:
	    return FALSE;
	default:
	    return FALSE;
    }
}

bool QDB2Driver::beginTransaction()
{
    if ( !isOpen() ) {
	qWarning(" QDB2Driver::beginTransaction: Database not open" );
	return FALSE;
    }
    return setAutoCommit( FALSE );
}

bool QDB2Driver::commitTransaction()
{
    if ( !isOpen() ) {
	qWarning(" QDB2Driver::commitTransaction: Database not open" );
	return FALSE;
    }
    SQLRETURN r = SQLEndTran( SQL_HANDLE_DBC,
			      d->hDbc,
			      SQL_COMMIT );
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to commit transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return setAutoCommit( TRUE );
}

bool QDB2Driver::rollbackTransaction()
{
    if ( !isOpen() ) {
	qWarning(" QDB2Driver::rollbackTransaction: Database not open" );
	return FALSE;
    }
    SQLRETURN r = SQLEndTran( SQL_HANDLE_DBC,
			      d->hDbc,
			      SQL_ROLLBACK );
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to rollback transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return setAutoCommit( TRUE );
}

bool QDB2Driver::setAutoCommit( bool autoCommit )
{
    SQLUINTEGER ac = autoCommit ? SQL_AUTOCOMMIT_ON : SQL_AUTOCOMMIT_OFF;
    SQLRETURN r  = SQLSetConnectAttr( d->hDbc,
				      SQL_ATTR_AUTOCOMMIT,
				      (SQLPOINTER)ac,
				      sizeof(ac) );
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to set autocommit", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return TRUE;
}

QString QDB2Driver::formatValue( const QSqlField* field, bool trimStrings ) const
{
    if ( field->isNull() )
	return nullText();

    switch ( field->type() ) {
	case QVariant::DateTime: {
	    // Use an escape sequence for the datetime fields
	    if ( field->value().toDateTime().isValid() ) {
		QDate dt = field->value().toDateTime().date();
		QTime tm = field->value().toDateTime().time();	
		// Dateformat has to be "yyyy-MM-dd hh:mm:ss", with leading zeroes if month or day < 10
		return "'" + QString::number( dt.year() ) + "-" +
		       QString::number( dt.month() ) + "-" +
		       QString::number( dt.day() ) + "-" +
		       QString::number( tm.hour() ) + "." +
		       QString::number( tm.minute() ).rightJustified( 2, '0', TRUE ) + "." +
		       QString::number( tm.second() ).rightJustified( 2, '0', TRUE ) + "." +
		       QString::number( tm.msec() * 1000 ).rightJustified( 6, '0', TRUE ) + "'";
		} else {
		    return nullText();
		}
	}
	case QVariant::ByteArray: {
	    QByteArray ba = field->value().toByteArray();
	    QString res( "BLOB(X'" );
	    static const char hexchars[] = "0123456789abcdef";
	    for ( int i = 0; i < ba.size(); ++i ) {
		uchar s = (uchar) ba[i];
		res += hexchars[s >> 4];
		res += hexchars[s & 0x0f];
	    }
	    res += "')";
	    return res;
	}
	default:
	    return QSqlDriver::formatValue( field, trimStrings );
    }
}

Qt::HANDLE QDB2Driver::environment()
{
    return (Qt::HANDLE)d->hEnv;
}

Qt::HANDLE QDB2Driver::connection()
{
    return (Qt::HANDLE)d->hDbc;
}
