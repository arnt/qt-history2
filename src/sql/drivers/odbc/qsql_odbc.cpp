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
    {}

    SQLHANDLE hEnv;
    SQLHANDLE hDbc;
    SQLHANDLE hStmt;

    bool checkDriver() const;
};

QString qWarnODBCHandle(int handleType, SQLHANDLE handle)
{
    SQLINTEGER nativeCode_;
    SQLSMALLINT tmp;
    SQLRETURN r = SQL_ERROR;
    SQLCHAR state_[SQL_SQLSTATE_SIZE+1];
    SQLCHAR description_[SQL_MAX_MESSAGE_LENGTH];
    r = SQLGetDiagRec( handleType,
			 handle,
			 1,
			 (SQLCHAR*)state_,
			 &nativeCode_,
			 (SQLCHAR*)description_,
			 SQL_MAX_MESSAGE_LENGTH-1,
			 &tmp);
    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO )
	return QString( (const char*)description_ );
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

QVariant::Type qDecodeODBCType( SQLSMALLINT sqltype )
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
//	type = QSqlFieldInfo::Binary;
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
    default:
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
	type = QVariant::String;
	break;
    }
    return type;
}

QSqlFieldInfo qMakeFieldInfo( const QODBCPrivate* p, int i  )
{
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLUINTEGER colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;
    QString qColName;
    SQLCHAR colName[255];
    r = SQLDescribeCol( p->hStmt,
			i+1,
			colName,
			sizeof(colName),
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
    qColName = qstrdup((const char*)colName);
    // nullable can be SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
    int required = -1;
    if ( nullable == SQL_NO_NULLS ) {
	required = 1;
    } else if ( nullable == SQL_NULLABLE ) {
	required = 0;
    }
    QVariant::Type type = qDecodeODBCType( colType );
    return QSqlFieldInfo( qColName,
    			  type,
    			  required,
    			  (int)colSize == 0 ? -1 : (int)colSize,
    			  (int)colScale == 0 ? -1 : (int)colScale,
    			  QVariant(),
    			  (int)colType );
}

QString qGetStringData( SQLHANDLE hStmt, int column, SQLINTEGER& lengthIndicator, bool& isNull )
{
    QString fieldVal;
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLUINTEGER colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;
    QString qColName;

    SQLCHAR colName[255];
    r = SQLDescribeCol( hStmt,
			column+1,
			colName,
			sizeof(colName),
			&colNameLen,
			&colType,
			&colSize,
			&colScale,
			&nullable);
    qColName = qstrdup( (const char*)colName );
#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qWarning( QString("qGetStringData: Unable to describe column %1").arg(column) );
#endif
    // SQLDescribeCol may return 0 if size cannot be determined
    if (!colSize) {
	colSize = 255;
    }
    SQLCHAR* buf = new SQLTCHAR[ colSize + 1 ];
    while ( TRUE ) {
	r = SQLGetData( hStmt,
			column+1,
			SQL_C_CHAR,
			(SQLPOINTER)buf,
			colSize + 1,
			&lengthIndicator );
	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
	    if ( lengthIndicator == SQL_NO_TOTAL ){
		fieldVal += QString( (char*)buf );  // keep going
	    }
	    else if ( lengthIndicator == SQL_NULL_DATA ) {
			fieldVal = QString::null;
			isNull = TRUE;
			break;
	    } else {
		if ( r == SQL_SUCCESS ) {
		    fieldVal += QString( (char*)buf );
		    break;
		} else {
		    if( (int)fieldVal.length() >= lengthIndicator ) // ### HACK - remove asap
			break;
		    fieldVal += QString( (char*)buf );
		}
	    }
	} else {
	    fieldVal += QString::null;
	    break;
	}
    }
    delete buf;
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

QSqlFieldInfo qMakeFieldInfo( const QODBCPrivate* d, const QString& tablename, const QString& fieldname )
{
    QSqlFieldInfo fi;
    SQLHANDLE hStmt;
    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
        qSqlWarning( "qMakeField: Unable to alloc handle", d );
#endif
	return fi;
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
		     (SQLCHAR*)tablename.local8Bit().data(),
		     tablename.length(),
		     (SQLCHAR*)fieldname.local8Bit().data(),
		     fieldname.length() );

    if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
	qSqlWarning( "qMakeField: Unable to execute column list", d );
#endif
	return fi;
    }
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0);
    if ( r == SQL_SUCCESS ) {
	bool isNull;
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
	fi = QSqlFieldInfo( fieldname, qDecodeODBCType( type ), required, size, prec, QVariant(), type );
    }
    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
        qSqlWarning( "QODBCDriver: Unable to free statement handle " + QString::number(r), d );
#endif
    return fi;
}

QSqlField qMakeField( const QODBCPrivate* d, const QString& tablename, const QString& fieldname )
{
    QSqlFieldInfo info = qMakeFieldInfo( d, tablename, fieldname );
    return QSqlField( info.name(), info.type() );
}

QSqlField qMakeField( const QODBCPrivate* p, int i  )
{
    QSqlFieldInfo info = qMakeFieldInfo( p, i );
    return QSqlField( info.name(), info.type() );
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
		r = SQLSetStmtAttr( d->hStmt,
				SQL_ATTR_CURSOR_TYPE,
				(SQLPOINTER)SQL_CURSOR_STATIC,
				SQL_IS_UINTEGER );
		if ( r != SQL_SUCCESS ) {
#ifdef QT_CHECK_RANGE
			qSqlWarning( "QODBCResult::reset: Unable to set statement attribute", d );
#endif
			return FALSE;
		}
    }
    r = SQLExecDirect( d->hStmt,
			(SQLCHAR*)query.local8Bit().data(),
			SQL_NTS );
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to execute statement", QSqlError::Statement, d ) );
	return FALSE;
    }
    SQLSMALLINT count;
    r = SQLNumResultCols( d->hStmt, &count );
    setSelect( count != 0 );
    setActive( TRUE );
    return TRUE;
}

bool QODBCResult::fetch(int i)
{
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
	QSqlField info = qMakeField( d, current );
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
	default:
	case QVariant::String:
	    isNull = FALSE;
	    QString fieldVal = qGetStringData( d->hStmt, current, lengthIndicator, isNull );
	    fieldCache[ current ] = QVariant( fieldVal );
	    nullCache[ current ] = isNull;
	    break;
	}
    }
    return fieldCache[ --current ];
}

bool QODBCResult::isNull( int field )
{
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
    QString connQStr = "DSN=" + db + ";UID=" + user + ";PWD=" + password + ";";
    SWORD       cb;
    SQLCHAR connOut[255];
    r = SQLDriverConnect( d->hDbc,
			    NULL,
			    (SQLCHAR*)connQStr.local8Bit().data(),
			    SQL_NTS,
			    connOut,
			    255,
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
		   (SQLCHAR*)tableType.local8Bit().data(),
		   tableType.length() );

#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QODBCDriver::tables Unable to execute table list", d );
#endif
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0);
    while ( r == SQL_SUCCESS ) {
	SQLINTEGER lengthIndicator = 0;
	bool isNull;
	QString fieldVal = qGetStringData( hStmt, 2, lengthIndicator, isNull ); // table name
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
    typedef QMap<QString,QString> FieldMap;
    FieldMap fMap;

    QSqlIndex index( tablename );
    if ( !isOpen() )
	return index;
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
			(SQLCHAR*)tablename.local8Bit().data(),
			tablename.length());
#ifdef QT_CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QODBCDriver::primaryIndex: Unable to execute primary key list", d );
#endif
    r = SQLFetchScroll( hStmt,
			SQL_FETCH_NEXT,
			0);
    // Store all fields in a StringList because some drivers can't detail fields in this FETCH loop
    while ( r == SQL_SUCCESS ) {
	SQLINTEGER lengthIndicator = 0;
	bool isNull;
	QString cName = qGetStringData( hStmt, 3, lengthIndicator, isNull ); // column name
	fMap[cName] = qGetStringData( hStmt, 5, lengthIndicator, isNull ); // pk index name
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0);
    }

    FieldMap::Iterator i;
    for (i = fMap.begin();  i != fMap.end(); i++) {
    	QSqlField f = qMakeField( d, tablename, ( i.key() ) );
	index.append( f );
	index.setName( i.data() ); // pk index name
    }

    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r!= SQL_SUCCESS )
	qSqlWarning( "QODBCDriver: Unable to free statement handle" + QString::number(r), d );
    return index;
}

QSqlRecord QODBCDriver::record( const QString& tablename ) const
{
    QSqlRecord fil;
    QStringList fList;
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
		     (SQLCHAR*)tablename.local8Bit().data(),
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
	bool isNull;
	SQLINTEGER lengthIndicator(0);
	fList += qGetStringData( hStmt, 3, lengthIndicator, isNull );
	
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0);
    }

    QStringList::Iterator i;
    for (i = fList.begin();  i != fList.end(); i++) {
	QSqlField f = qMakeField( d, tablename, (*i) );
	fil.append( f );
    }

    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r!= SQL_SUCCESS )
	qSqlWarning( "QODBCDriver: Unable to free statement handle" + QString::number(r), d );

//     QSqlIndex priIdx = primaryIndex( tablename );
//     for ( uint i = 0; i < priIdx.count(); ++i )
//	fil.field( priIdx.field(i)->name() )->setPrimaryIndex( TRUE );
    return fil;
}

QSqlRecord QODBCDriver::record( const QSqlQuery& query ) const
{
    QSqlRecord fil;
    if ( !isOpen() )
	return fil;
    if ( query.isActive() && query.driver() == this ) {
	QODBCResult* result = (QODBCResult*)query.result();
	SQLRETURN r;
	SQLSMALLINT count;
	r = SQLNumResultCols( result->d->hStmt, &count );
#ifdef QT_CHECK_RANGE
	if ( r != SQL_SUCCESS )
	    qSqlWarning( "QODBCDriver::record: Unable to count result columns", d );
#endif
	if ( count > 0 && r == SQL_SUCCESS ) {
	    for ( int i = 0; i < count; ++i ) {
		QSqlField fi = qMakeField( result->d, i );
		fil.append( fi );
	    }
	}
    }
    return fil;
}

QSqlRecordInfo QODBCDriver::recordInfo( const QString& tablename ) const
{
    QSqlRecordInfo fil;
    QStringList fList;
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
		     (SQLCHAR*)tablename.local8Bit().data(),
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
	bool isNull;
	SQLINTEGER lengthIndicator(0);
	fList += qGetStringData( hStmt, 3, lengthIndicator, isNull );
	
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0);
    }

    QStringList::Iterator i;
    for (i = fList.begin();  i != fList.end(); i++) {
	QSqlFieldInfo f = qMakeFieldInfo( d, tablename, (*i) );
	fil.append( f );
    }

    r = SQLFreeHandle( SQL_HANDLE_STMT, hStmt );
    if ( r!= SQL_SUCCESS )
	qSqlWarning( "QODBCDriver: Unable to free statement handle" + QString::number(r), d );

//     QSqlIndex priIdx = primaryIndex( tablename );
//     for ( uint i = 0; i < priIdx.count(); ++i )
//	fil.field( priIdx.field(i)->name() )->setPrimaryIndex( TRUE );
    return fil;
}

QSqlRecordInfo QODBCDriver::recordInfo( const QSqlQuery& query ) const
{
    QSqlRecordInfo fil;
    if ( !isOpen() )
	return fil;
    if ( query.isActive() && query.driver() == this ) {
	QODBCResult* result = (QODBCResult*)query.result();
	SQLRETURN r;
	SQLSMALLINT count;
	r = SQLNumResultCols( result->d->hStmt, &count );
#ifdef QT_CHECK_RANGE
	if ( r != SQL_SUCCESS )
	    qSqlWarning( "QODBCDriver::record: Unable to count result columns", d );
#endif
	if ( count > 0 && r == SQL_SUCCESS ) {
	    for ( int i = 0; i < count; ++i ) {
		QSqlFieldInfo fi = qMakeFieldInfo( result->d, i );
		fil.append( fi );
	    }
	}
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
