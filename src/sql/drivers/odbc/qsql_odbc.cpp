/****************************************************************************
**
** Implementation of ODBC driver classes
**
** Created : 001103
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

#include "qsql_odbc.h"
#include <qsqlrecord.h>

#if defined (Q_OS_WIN32)
#include <qt_windows.h>
#include <qapplication.h>
#endif
#include <qdatetime.h>

// undefine this to prevent initial check of the ODBC driver 
#define ODBC_CHECK_DRIVER

class QODBCPrivate
{
public:
    QODBCPrivate()
    : hEnv(0), hDbc(0), hStmt(0)
    {
	sql_char_type = sql_varchar_type = sql_longvarchar_type = QVariant::CString;
	unicode = FALSE;
    }

    SQLHANDLE hEnv;
    SQLHANDLE hDbc;
    SQLHANDLE hStmt;

    bool unicode;
    QVariant::Type sql_char_type;
    QVariant::Type sql_varchar_type;
    QVariant::Type sql_longvarchar_type;

    QSqlRecordInfo rInf;

    bool checkDriver() const;
    void checkUnicode();
};

QString qWarnODBCHandle(int handleType, SQLHANDLE handle)
{
    SQLINTEGER nativeCode_;
    SQLSMALLINT msgLen;
    SQLRETURN r = SQL_ERROR;
    SQLTCHAR state_[SQL_SQLSTATE_SIZE+1];
    SQLTCHAR description_[SQL_MAX_MESSAGE_LENGTH];
    r = SQLGetDiagRec( handleType,
			 handle,
			 1,
			 (SQLTCHAR*)state_,
			 &nativeCode_,
			 (SQLTCHAR*)description_,
			 SQL_MAX_MESSAGE_LENGTH-1, /* in bytes, not in characters */
			 &msgLen);
    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO )
#ifdef UNICODE
	return QString( (const QChar*)description_, (uint)msgLen );
#else
	return QString( (const char*)description_ );
#endif
    return QString::null;
}

QString qODBCWarn( const QODBCPrivate* odbc)
{
    return ( qWarnODBCHandle( SQL_HANDLE_ENV, odbc->hEnv ) + " " \
	     + qWarnODBCHandle( SQL_HANDLE_DBC, odbc->hDbc ) + " " \
	     + qWarnODBCHandle( SQL_HANDLE_STMT, odbc->hStmt ) );
}

void qSqlWarning( const QString& message, const QODBCPrivate* odbc )
{
#ifdef QT_CHECK_RANGE
    qWarning( message + "\tError:" + qODBCWarn( odbc ) );
#endif
}

QSqlError qMakeError( const QString& err, int type, const QODBCPrivate* p )
{
    return QSqlError( "QODBC3: " + err, qODBCWarn(p), type );
}

QVariant::Type qDecodeODBCType( SQLSMALLINT sqltype, const QODBCPrivate* p )
{
    QVariant::Type type = QVariant::Invalid;
    switch ( sqltype ) {
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
	type = QVariant::Double;
	break;
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_BIGINT:
	type = QVariant::Int;
	break;
//     case SQL_BINARY:
//     case SQL_VARBINARY:
//     case SQL_LONGVARBINARY:
//	type = QVariant::ByteArray;
//	break;
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
	type = QVariant::String;
	break;
    case SQL_CHAR:
	type = p->sql_char_type;
	break;
    case SQL_VARCHAR:
	type = p->sql_varchar_type;
	break;
    case SQL_LONGVARCHAR:
	type = p->sql_longvarchar_type;
	break;
    default:
	type = QVariant::CString;
	break;
    }
    return type;
}

QString qGetStringData( SQLHANDLE hStmt, int column, int colSize, bool& isNull, bool unicode = FALSE )
{
    QString     fieldVal;
    SQLRETURN   r = SQL_ERROR;
    SQLINTEGER  lengthIndicator = 0;

    if ( colSize <= 0 ) {
	colSize = 255;
    }
    colSize++; // trailing zero
    if ( unicode ) {
	colSize *= 2;
    }
    char* buf = new char[ colSize ];
    while ( TRUE ) {
	r = SQLGetData( hStmt,
			column+1,
			unicode ? SQL_C_WCHAR : SQL_C_CHAR,
			(SQLPOINTER)buf,
			colSize,
			&lengthIndicator );
	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
	    if ( lengthIndicator == SQL_NULL_DATA || lengthIndicator == SQL_NO_TOTAL ) {
		fieldVal = QString::null;
		isNull = TRUE;
		break;
	    }
	    if ( unicode ) {
		fieldVal += QString( (QChar*)buf, lengthIndicator / 2 );
	    } else {
		fieldVal += buf;
	    }
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

int qGetIntData( SQLHANDLE hStmt, int column, bool& isNull  )
{
    SQLINTEGER intbuf;
    isNull = FALSE;
    SQLINTEGER lengthIndicator = 0;
    SQLRETURN r = SQLGetData( hStmt,
		    column+1,
		    SQL_C_SLONG,
		    (SQLPOINTER)&intbuf,
		    0,
		    &lengthIndicator );
    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
	if ( lengthIndicator == SQL_NULL_DATA )
	    isNull = TRUE;
    }
    return (int)intbuf;
}

// creates a QSqlFieldInfo from a valid hStmt generated
// by SQLColumns. The hStmt has to point to a valid position.
QSqlFieldInfo qMakeFieldInfo( const SQLHANDLE hStmt, const QODBCPrivate* p )
{
    bool isNull;
    QString fname = qGetStringData( hStmt, 3, -1, isNull, p->unicode );
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
    return QSqlFieldInfo( fname, qDecodeODBCType( type, p ), required, size, prec, QVariant(), type );
}

QSqlFieldInfo qMakeFieldInfo( const QODBCPrivate* p, int i  )
{
#define COLNAMESIZE 255

    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLUINTEGER colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;
    SQLTCHAR colName[ COLNAMESIZE ];
    r = SQLDescribeCol( p->hStmt,
			i+1,
			colName,
			(SQLSMALLINT)COLNAMESIZE,
			&colNameLen,
			&colType,
			&colSize,
			&colScale,
			&nullable);

    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( QString("qMakeField: Unable to describe column %1").arg(i), p );
#endif
	return QSqlFieldInfo();
    }
#ifdef UNICODE
    QString qColName( (const QChar*)colName, (uint)colNameLen );
#else
    QString qColName( (const char*)colName );
#endif
    // nullable can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    int required = -1;
    if ( nullable == SQL_NO_NULLS ) {
	required = 1;
    } else if ( nullable == SQL_NULLABLE ) {
	required = 0;
    }
    QVariant::Type type = qDecodeODBCType( colType, p );
    return QSqlFieldInfo( qColName,
    			  type,
    			  required,
    			  (int)colSize == 0 ? -1 : (int)colSize,
    			  (int)colScale == 0 ? -1 : (int)colScale,
    			  QVariant(),
    			  (int)colType );
#undef COLNAMESIZE
}

////////////////////////////////////////////////////////////////////////////

QODBCResult::QODBCResult( const QODBCDriver * db, QODBCPrivate* p )
: QSqlResult(db)
{
    d = new QODBCPrivate();
    (*d) = (*p);
}

QODBCResult::~QODBCResult()
{
    if ( d->hStmt && driver()->isOpen() ) {
	SQLRETURN r = SQLFreeHandle( SQL_HANDLE_STMT, d->hStmt );
#ifdef QT_CHECK_RANGE
	if ( r!= SQL_SUCCESS )
		qSqlWarning( "QODBCDriver: Unable to free statement handle" + QString::number(r), d );
#endif
    }

    delete d;
}

bool QODBCResult::reset ( const QString& query )
{
    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    SQLRETURN r;

    d->rInf.clear();
    // If a statement handle exists - reuse it
    if ( d->hStmt ) {
	r = SQLFreeStmt( d->hStmt, SQL_CLOSE );
	if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	    qSqlWarning( "QODBCResult::reset: Unable to close statement", d );
#endif
	    return FALSE;
	}
    } else {
	r  = SQLAllocHandle( SQL_HANDLE_STMT,
	    		     d->hDbc,
			     &d->hStmt );
	if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	    qSqlWarning( "QODBCResult::reset: Unable to allocate statement handle", d );
#endif
	    return FALSE;
	}
    }

    if ( isForwardOnly() ) {
	r = SQLSetStmtAttr( d->hStmt,
	    		    SQL_ATTR_CURSOR_TYPE,
			    (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			    SQL_IS_UINTEGER );
    } else {
	r = SQLSetStmtAttr( d->hStmt,
			    SQL_ATTR_CURSOR_TYPE,
			    (SQLPOINTER)SQL_CURSOR_STATIC,
			    SQL_IS_UINTEGER );
    }
    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCResult::reset: Unable to set statement attribute", d );
#endif
	return FALSE;
    }

    r = SQLExecDirect( d->hStmt,
#ifdef UNICODE
		       (SQLWCHAR*)query.unicode(),
#else
		       // the ODBC driver manager will translate charsets for us
		       (SQLCHAR*)query.local8Bit().data(),
#endif
		       (SQLINTEGER)query.length() /* count of characters, not bytes */ );
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to execute statement", QSqlError::Statement, d ) );
	return FALSE;
    }
    SQLSMALLINT count;
    r = SQLNumResultCols( d->hStmt, &count );
    if ( count ) {
	setSelect( TRUE );
	for ( int i = 0; i < count; ++i ) {
	    d->rInf.append( qMakeFieldInfo( d, i ) );
	}
    } else {
	setSelect( FALSE );
    }
    setActive( TRUE );
    return TRUE;
}

bool QODBCResult::fetch(int i)
{
    if ( isForwardOnly() && i < at() )
	return FALSE;
    if ( i == at() )
	return TRUE;
    fieldCache.clear();
    nullCache.clear();
    int actualIdx = i + 1;
    if ( actualIdx <= 0 ) {
	setAt( QSql::BeforeFirst );
	return FALSE;
    }
    SQLRETURN r;
    r = SQLFetchScroll( d->hStmt,
			SQL_FETCH_ABSOLUTE,
			actualIdx );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( i );
    return TRUE;
}

bool QODBCResult::fetchFirst()
{
    if ( isForwardOnly() && at() != QSql::BeforeFirst )
	return FALSE;
    SQLRETURN r;
    fieldCache.clear();
    nullCache.clear();
    r = SQLFetchScroll( d->hStmt,
		       SQL_FETCH_FIRST,
		       0 );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( 0 );
    return TRUE;
}

bool QODBCResult::fetchNext()
{
    SQLRETURN r;
    fieldCache.clear();
    nullCache.clear();
    r = SQLFetchScroll( d->hStmt,
		       SQL_FETCH_NEXT,
		       0 );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( at() + 1 );
    return TRUE;
}

bool QODBCResult::fetchPrior()
{
    if ( isForwardOnly() )
	return FALSE;
    SQLRETURN r;
    fieldCache.clear();
    nullCache.clear();
    r = SQLFetchScroll( d->hStmt,
		       SQL_FETCH_PRIOR,
		       0 );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( at() - 1 );
    return TRUE;
}

bool QODBCResult::fetchLast()
{
    SQLRETURN r;
    fieldCache.clear();
    nullCache.clear();
    r = SQLFetchScroll( d->hStmt,
		       SQL_FETCH_LAST,
		       0 );
    if ( r != SQL_SUCCESS ) {
	return FALSE;
    }
    SQLINTEGER currRow;
    r = SQLGetStmtAttr( d->hStmt,
			SQL_ROW_NUMBER,
			&currRow,
			SQL_IS_INTEGER,
			0 );
    if ( r != SQL_SUCCESS )
	return FALSE;
    setAt( currRow-1 );
    return TRUE;
}

QVariant QODBCResult::data( int field )
{
    if ( fieldCache.contains( field ) )
	return fieldCache[ field ];
    SQLRETURN r(0);
    SQLINTEGER lengthIndicator = 0;
    bool isNull = FALSE;
    int current = fieldCache.count();
    for ( ; current < (field + 1); ++current ) {
	const QSqlFieldInfo info = d->rInf[ current ];
	switch ( info.type() ) {
	case QVariant::Int:
	    isNull = FALSE;
	    fieldCache[ current ] = QVariant( qGetIntData( d->hStmt, current, isNull ) );
	    nullCache[ current ] = isNull;
	    break;
	case QVariant::Double:
	    SQLDOUBLE dblbuf;
	    r = SQLGetData( d->hStmt,
			    current+1,
			    SQL_C_DOUBLE,
			    (SQLPOINTER)&dblbuf,
			    0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) ) {
		fieldCache[ current ] = QVariant( dblbuf );
		nullCache[ current ] = FALSE;	
	    } else {
		fieldCache[ current ] = QVariant();
		nullCache[ current ] = TRUE;	
	    }
	    break;
	case QVariant::Date:
	    DATE_STRUCT dbuf;
	    r = SQLGetData( d->hStmt,
			    current+1,
				SQL_C_DATE,
				(SQLPOINTER)&dbuf,
				0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) ) {
		fieldCache[ current ] = QVariant( QDate( dbuf.year, dbuf.month, dbuf.day ) );
		nullCache[ current ] = FALSE;
	    } else {
		fieldCache[ current ] = QVariant( QDate() );
		nullCache[ current ] = TRUE;
	    }
	    break;
	case QVariant::Time:
	    TIME_STRUCT tbuf;
	    r = SQLGetData( d->hStmt,
			    current+1,
			    SQL_C_TIME,
			    (SQLPOINTER)&tbuf,
			    0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) ) {
		fieldCache[ current ] = QVariant( QTime( tbuf.hour, tbuf.minute, tbuf.second ) );
		nullCache[ current ] = FALSE;
	    } else {
		fieldCache[ current ] = QVariant( QTime() );
		nullCache[ current ] = TRUE;
	    }
	    break;
	case QVariant::DateTime:
	    TIMESTAMP_STRUCT dtbuf;
	    r = SQLGetData( d->hStmt,
			    current+1,
			    SQL_C_TIMESTAMP,
			    (SQLPOINTER)&dtbuf,
			    0,
			    &lengthIndicator );
	    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( lengthIndicator != SQL_NULL_DATA ) ) {
		fieldCache[ current ] = QVariant( QDateTime( QDate( dtbuf.year, dtbuf.month, dtbuf.day ), QTime( dtbuf.hour, dtbuf.minute, dtbuf.second ) ) );
		nullCache[ current ] = FALSE;
	    } else {
		fieldCache[ current ] = QVariant( QDateTime() );
		nullCache[ current ] = TRUE;
	    }
	    break;
	case QVariant::String:
	    isNull = FALSE;
	    fieldCache[ current ] = QVariant( qGetStringData( d->hStmt, current,
					      info.length(), isNull, TRUE ) );
	    nullCache[ current ] = isNull;
	    break;
	default:
	case QVariant::CString:
	    isNull = FALSE;
	    fieldCache[ current ] = QVariant( qGetStringData( d->hStmt, current, 
					      info.length(), isNull, FALSE ) );
	    nullCache[ current ] = isNull;
	    break;
	}
    }
    return fieldCache[ --current ];
}

bool QODBCResult::isNull( int field )
{
    if ( !fieldCache.contains( field ) ) {
	// since there is no good way to find out whether the value is NULL
	// without fetching the field we'll fetch it here.
	// (data() also sets the NULL flag)
	data( field );
    }
    return nullCache[ field ];
}

int QODBCResult::size()
{
    return -1;
//     int size(-1);
//     int at(0);
//     SQLINTEGER currRow(0);
//     SQLRETURN r = SQLGetStmtAttr( d->hStmt,
//			SQL_ROW_NUMBER,
//			&currRow,
//			SQL_IS_INTEGER,
//			0);
//     at = currRow;
//     r = SQLFetchScroll( d->hStmt,
//                         SQL_FETCH_LAST,
//                         0);
//     if ( r == SQL_SUCCESS ) {
//	r = SQLGetStmtAttr( d->hStmt,
//			SQL_ROW_NUMBER,
//			&currRow,
//			SQL_IS_INTEGER,
//			0);
//	if ( r == SQL_SUCCESS )
//	    size = currRow;
//	r = SQLFetchScroll( d->hStmt,
//                         SQL_FETCH_ABSOLUTE,
//                         currRow);
//	if ( r != SQL_SUCCESS )
//	    qSqlWarning("QODBCResult::size: Unable to restore position", d );
//     }
//     return size;
}

int QODBCResult::numRowsAffected()
{
    SQLINTEGER affectedRowCount(0);
    SQLRETURN r = SQLRowCount( d->hStmt, &affectedRowCount);
    if ( r == SQL_SUCCESS )
	return affectedRowCount;
#ifdef QT_CHECK_RANGE
    else
	qSqlWarning( "QODBCResult::numRowsAffected: Unable to count affected rows", d );
#endif
    return -1;
}



////////////////////////////////////////


QODBCDriver::QODBCDriver( QObject * parent, const char * name )
: QSqlDriver(parent,name ? name : "QODBC")
{
    init();
}

void QODBCDriver::init()
{
    d = new QODBCPrivate();
}

QODBCDriver::~QODBCDriver()
{
    cleanup();
    delete d;
}

bool QODBCDriver::hasFeature( DriverFeature f ) const
{
    switch ( f ) {
    case Transactions: {
	if ( !d->hDbc )
	    return FALSE;
	SQLUSMALLINT txn;
	SQLSMALLINT t;
	int r = SQLGetInfo( d->hDbc,
			(SQLUSMALLINT)SQL_TXN_CAPABLE,
			&txn,
			sizeof(txn),
			&t);
	if ( r != SQL_SUCCESS || txn == SQL_TC_NONE )
	    return FALSE;
	else
	    return TRUE;
    }
    case QuerySize:
	return FALSE;
    case BLOB:
	return FALSE;
    case Unicode:
	return d->unicode;
    default:
	return FALSE;
    }
}

bool QODBCDriver::open( const QString & db,
			const QString & user,
			const QString & password,
			const QString &,
			int )
{
    if ( isOpen() )
		close();
    SQLRETURN r;
    r = SQLAllocHandle( SQL_HANDLE_ENV,
			SQL_NULL_HANDLE,
			&d->hEnv);
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCDriver::open: Unable to allocate environment", d );
#endif
	setOpenError( TRUE );
	return FALSE;
    }
    r = SQLSetEnvAttr( d->hEnv,
			SQL_ATTR_ODBC_VERSION,
			(SQLPOINTER)SQL_OV_ODBC2,
			SQL_IS_UINTEGER );
    r = SQLAllocHandle(SQL_HANDLE_DBC,
			d->hEnv,
			&d->hDbc);
    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
		qSqlWarning( "QODBCDriver::open: Unable to allocate connection", d );
#endif
		setOpenError( TRUE );
		return FALSE;
    }
    QString      connQStr = "DSN=" + db + ";UID=" + user + ";PWD=" + password + ";";
    SQLSMALLINT  cb;
    SQLTCHAR     connOut[1024]; // The doc says at least 1024 characters
    r = SQLDriverConnect( d->hDbc,
			    NULL,
#ifdef UNICODE
			    (SQLWCHAR*)connQStr.unicode(),
#else
			    (SQLCHAR*)connQStr.latin1(),
#endif
			    (SQLSMALLINT)connQStr.length(),
			    connOut,
			    1024,
			    &cb,
			    SQL_DRIVER_NOPROMPT);
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	setLastError( qMakeError( "Unable to connect", QSqlError::Connection, d ) );
	setOpenError( TRUE );
	return FALSE;
    }

    if ( !d->checkDriver() ) {
#ifdef QT_CHECK_RANGE
	qWarning ( "QODBCDriver::open: Warning - Driver doesn't support all needed functionality. "
		   "Please look at the Qt SQL Module Driver documentation for more information." );
#endif
	setLastError( qMakeError( "Unable to connect - Driver doesn't support all needed functionality", QSqlError::Connection, d ) );
	setOpenError( TRUE );
	return FALSE;
    }

    d->checkUnicode();

    setOpen( TRUE );
    return TRUE;
}

void QODBCDriver::close()
{
    cleanup();
    setOpen( FALSE );
    setOpenError( FALSE );
}

void QODBCDriver::cleanup()
{
    SQLRETURN r;
    if ( (isOpen() || isOpenError()) && (d != 0)) {
	if( d->hDbc ) {
	    // Open statements/descriptors handles are automatically cleaned up by SQLDisconnect
	    r = SQLDisconnect( d->hDbc );
#ifdef QT_CHECK_RANGE
	    if ( r != SQL_SUCCESS )
		qSqlWarning( "QODBCDriver::disconnect: Unable to disconnect datasource", d );
#endif
		r = SQLFreeHandle( SQL_HANDLE_DBC, d->hDbc );
#ifdef QT_CHECK_RANGE
		if ( r != SQL_SUCCESS )
		    qSqlWarning( "QODBCDriver::cleanup: Unable to free connection handle", d );
#endif
		    d->hDbc = 0;
		}
		if ( d->hEnv ) {
		    r = SQLFreeHandle( SQL_HANDLE_ENV, d->hEnv );
#ifdef QT_CHECK_RANGE
		    if ( r != SQL_SUCCESS )
			qSqlWarning( "QODBCDriver::cleanup: Unable to free environment handle", d );
#endif
		    d->hEnv = 0;
		}
    }
}

// checks whether the server can return char, varchar and longvarchar
// as two byte unicode characters
void QODBCPrivate::checkUnicode()
{
    SQLRETURN   r;
    SQLUINTEGER fFunc;

    unicode = TRUE;
    r = SQLGetInfo( hDbc, 
		    SQL_CONVERT_CHAR,
		    (SQLPOINTER)&fFunc,
		    sizeof(fFunc),
		    NULL );
    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( fFunc & SQL_CVT_WCHAR ) ) {
	sql_char_type = QVariant::String;
    } else {
	unicode = FALSE;
    }

    r = SQLGetInfo( hDbc, 
		    SQL_CONVERT_VARCHAR,
		    (SQLPOINTER)&fFunc,
		    sizeof(fFunc),
		    NULL );
    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( fFunc & SQL_CVT_WVARCHAR ) ) {
	sql_varchar_type = QVariant::String;
    } else {
	unicode = FALSE;
    }

    r = SQLGetInfo( hDbc,
		    SQL_CONVERT_LONGVARCHAR,
		    (SQLPOINTER)&fFunc,
		    sizeof(fFunc),
		    NULL );
    if ( ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) && ( fFunc & SQL_CVT_WLONGVARCHAR ) ) {
	sql_longvarchar_type = QVariant::String;
    } else {
	unicode = FALSE;
    }

}

bool QODBCPrivate::checkDriver() const
{
#ifdef ODBC_CHECK_DRIVER


    // do not query for SQL_API_SQLFETCHSCROLL because it can't be used at this time
    static const SQLUSMALLINT reqFunc[] = {
		SQL_API_SQLDESCRIBECOL, SQL_API_SQLGETDATA, SQL_API_SQLCOLUMNS, 
		SQL_API_SQLGETSTMTATTR, SQL_API_SQLGETDIAGREC, SQL_API_SQLEXECDIRECT,
		SQL_API_SQLGETINFO, SQL_API_SQLTABLES, SQL_API_SQLPRIMARYKEYS, 0
    };

    // these functions are optional
    static const SQLUSMALLINT optFunc[] = {
    		SQL_API_SQLNUMRESULTCOLS, SQL_API_SQLROWCOUNT, 0
    };

    SQLRETURN r;
    SQLUSMALLINT sup;

    
    int i;
    // check the required functions
    for ( i = 0; reqFunc[ i ] != 0; ++i ) {

	r = SQLGetFunctions( hDbc, reqFunc[ i ], &sup );

#ifdef QT_CHECK_RANGE
        if ( r != SQL_SUCCESS ) {
	    qSqlWarning( "QODBCDriver::checkDriver: Cannot get list of supported functions", this );
	    return FALSE;
	}
#endif
	if ( sup == SQL_FALSE ) {
	    return FALSE;
	}
    }

    // these functions are optional and just generate a warning
    for ( i = 0; optFunc[ i ] != 0; ++i ) {

	r = SQLGetFunctions( hDbc, optFunc[ i ], &sup );

#ifdef QT_CHECK_RANGE
        if ( r != SQL_SUCCESS ) {
	    qSqlWarning( "QODBCDriver::checkDriver: Cannot get list of supported functions", this );
	    return FALSE;
	}
#endif
	if ( sup == SQL_FALSE ) {
#ifdef QT_CHECK_RANGE
	    qWarning( "QODBCDriver::checkDriver: Warning - Driver doesn't support some non-critical functions" );
#endif
	    return TRUE;
	}
    }

#endif //ODBC_CHECK_DRIVER

    return TRUE;
}

QSqlQuery QODBCDriver::createQuery() const
{
    return QSqlQuery( new QODBCResult( this, d ) );
}

bool QODBCDriver::beginTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning(" QODBCDriver::beginTransaction: Database not open" );
#endif
	return FALSE;
    }
    SQLUINTEGER ac(SQL_AUTOCOMMIT_OFF);
    SQLRETURN r  = SQLSetConnectAttr( d->hDbc,
			    SQL_ATTR_AUTOCOMMIT,
			    (SQLPOINTER)ac,
			    sizeof(ac));
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to disable autocommit", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return TRUE;
}

bool QODBCDriver::commitTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning(" QODBCDriver::commitTransaction: Database not open" );
#endif
	return FALSE;
    }
    SQLRETURN r = SQLEndTran( SQL_HANDLE_ENV,
				d->hEnv,
				SQL_COMMIT);
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError("Unable to commit transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return endTrans();
}

bool QODBCDriver::rollbackTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning(" QODBCDriver::rollbackTransaction: Database not open" );
#endif
	return FALSE;
    }
    SQLRETURN r = SQLEndTran( SQL_HANDLE_ENV,
				d->hEnv,
				SQL_ROLLBACK);
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to rollback transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return endTrans();
}

bool QODBCDriver::endTrans()
{
    SQLUINTEGER ac(SQL_AUTOCOMMIT_ON);
    SQLRETURN r  = SQLSetConnectAttr( d->hDbc,
			    SQL_ATTR_AUTOCOMMIT,
			    (SQLPOINTER)ac,
			    sizeof(ac));
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to enable autocommit", QSqlError::Transaction, d ) );
	return FALSE;
    }
    return TRUE;
}

QStringList QODBCDriver::tables( const QString& /* user */ ) const
{
    QStringList tl;
    if ( !isOpen() )
	return tl;
    SQLHANDLE hStmt;

    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCDriver::tables: Unable to allocate handle", d );
#endif
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
#ifdef UNICODE
		   (SQLWCHAR*)tableType.unicode(),
#else
		   (SQLCHAR*)tableType.latin1(),
#endif
		   tableType.length() /* characters, not bytes */ );

#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QODBCDriver::tables Unable to execute table list", d );
#endif
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0);
    while ( r == SQL_SUCCESS ) {
	bool isNull;
	QString fieldVal = qGetStringData( hStmt, 2, -1, isNull, d->unicode );
	tl.append( fieldVal );
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0);
    }

    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r!= SQL_SUCCESS )
	qSqlWarning( "QODBCDriver: Unable to free statement handle" + QString::number(r), d );
    return tl;
}

QSqlIndex QODBCDriver::primaryIndex( const QString& tablename ) const
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
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCDriver::primaryIndex: Unable to list primary key", d );
#endif
	return index;
    }
    r = SQLSetStmtAttr( hStmt,
			SQL_ATTR_CURSOR_TYPE,
			(SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			SQL_IS_UINTEGER );
    r = SQLPrimaryKeys( hStmt,
			NULL,
			0,
			NULL,
			0,
#ifdef UNICODE
			(SQLWCHAR*)tablename.unicode(),
#else
			(SQLCHAR*)tablename.latin1(),
#endif
			tablename.length() /* in characters, not in bytes */);
#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QODBCDriver::primaryIndex: Unable to execute primary key list", d );
#endif
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0);
    while ( r == SQL_SUCCESS ) {
	bool isNull;
	QString cName = qGetStringData( hStmt, 3, -1, isNull, d->unicode ); // column name
	index.append( *(rec.field( cName )) );
	QString idxName = qGetStringData( hStmt, 5, -1, isNull, d->unicode ); // pk index name
	index.setName( idxName );
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0);
    }

    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r!= SQL_SUCCESS )
	qSqlWarning( "QODBCDriver: Unable to free statement handle" + QString::number(r), d );
    return index;
}

QSqlRecord QODBCDriver::record( const QString& tablename ) const
{
    return recordInfo( tablename ).toRecord();
}

QSqlRecord QODBCDriver::record( const QSqlQuery& query ) const
{
    return recordInfo( query ).toRecord();
}

QSqlRecordInfo QODBCDriver::recordInfo( const QString& tablename ) const
{
    QSqlRecordInfo fil;
    if ( !isOpen() )
	return fil;

    SQLHANDLE hStmt;
    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "QODBCDriver::record: Unable to allocate handle", d );
#endif
	return fil;
    }
    r = SQLSetStmtAttr( hStmt,
			SQL_ATTR_CURSOR_TYPE,
			(SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
			SQL_IS_UINTEGER );
    r =  SQLColumns( hStmt,
		     NULL,
		     0,
		     NULL,
		     0,
#ifdef UNICODE
		     (SQLWCHAR*)tablename.unicode(),
#else
		     (SQLCHAR*)tablename.latin1(),
#endif
		     tablename.length(),
		     NULL,
		     0 );
#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QODBCDriver::record: Unable to execute column list", d );
#endif
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0);
    // Store all fields in a StringList because some drivers can't detail fields in this FETCH loop
    while ( r == SQL_SUCCESS ) {

	fil.append( qMakeFieldInfo( hStmt, d ) );

	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0);
    }

    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r!= SQL_SUCCESS )
	qSqlWarning( "QODBCDriver: Unable to free statement handle " + QString::number(r), d );

    return fil;
}

QSqlRecordInfo QODBCDriver::recordInfo( const QSqlQuery& query ) const
{
    QSqlRecordInfo fil;
    if ( !isOpen() )
	return fil;
    if ( query.isActive() && query.driver() == this ) {
	QODBCResult* result = (QODBCResult*)query.result();
	fil = result->d->rInf;
    }
    return fil;
}

SQLHANDLE QODBCDriver::environment()
{
    return d->hEnv;
}

SQLHANDLE QODBCDriver::connection()
{
    return d->hDbc;
}

QString QODBCDriver::formatValue( const QSqlField* field,
				  bool trimStrings ) const
{
    QString r;
    if ( field->isNull() )
	r = nullText();
    else if ( field->type() == QVariant::DateTime ) {
	// Use an escape sequence for the datetime fields
	if ( field->value().toDateTime().isValid() ){
	    QDate dt = field->value().toDateTime().date();
	    QTime tm = field->value().toDateTime().time();	
	    // Dateformat has to be "yyyy-MM-dd hh:mm:ss", with leading zeroes if month or day < 10
	    r = "{ ts '" +
		QString::number(dt.year()) + "-" +
		QString::number(dt.month()).rightJustify( 2, '0', TRUE ) + "-" +
		QString::number(dt.day()).rightJustify( 2, '0', TRUE ) + " " +
		tm.toString() +
		"' }";
	} else
	    r = nullText();
    } else {
	r = QSqlDriver::formatValue( field, trimStrings );
    }
    return r;
}
