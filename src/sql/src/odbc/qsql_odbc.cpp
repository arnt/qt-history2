#include "qsql_odbc.h"

#if defined (_OS_WIN32_)
#include <qt_windows.h>
#include <qapplication.h>
#endif
#include <qdatetime.h>

#include <sql.h>
#include <sqlext.h>

class QODBCPrivate
{
public:
    QODBCPrivate()
    : hEnv(0), hDbc(0), hStmt(0)
    {}
    SQLHANDLE hEnv;
    SQLHANDLE hDbc;
    SQLHANDLE hStmt;
};

QString qWarnODBCHandle(int handleType, SQLHANDLE handle)
{
    SQLINTEGER nativeCode_;
    SQLSMALLINT tmp;
    SQLRETURN r = SQL_ERROR;
#if defined(_OS_WIN32_)
    if ( qApp->winVersion() & Qt::WV_NT_based ) {
	SQLTCHAR state_[SQL_SQLSTATE_SIZE+1];
	SQLTCHAR description_[SQL_MAX_MESSAGE_LENGTH];
	r = SQLGetDiagRec( handleType,
    				     handle,
				     1,
				     (SQLTCHAR*)&state_,
				     &nativeCode_,
			    	     (SQLTCHAR*)&description_,
				     SQL_MAX_MESSAGE_LENGTH-1,
				     &tmp);
	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO )
	    return QString( qt_winQString( (SQLTCHAR*)description_ ) );
    } else {
	SQLCHAR state_[SQL_SQLSTATE_SIZE+1];
	SQLCHAR description_[SQL_MAX_MESSAGE_LENGTH];
	r = SQLGetDiagRecA( handleType,
    				     handle,
				     1,
				     (SQLCHAR*)&state_,
				     &nativeCode_,
			    	     (SQLCHAR*)&description_,
				     SQL_MAX_MESSAGE_LENGTH-1,
				     &tmp);
	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO )
	    return QString( (const char*)description_ );
    }
#else
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
#ifdef CHECK_RANGE
    qWarning( message + "\tError:" + qODBCWarn( odbc ) );
#endif
}

QSqlError qMakeError( const QString& err, int type, const QODBCPrivate* p )
{
    return QSqlError( "QODBC: " + err, qODBCWarn(p), type );
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
// 	type = QSqlFieldInfo::Binary;
// 	break;
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

QSqlField qMakeField( const QODBCPrivate* p, int i  )
{
    SQLSMALLINT colNameLen;
    SQLSMALLINT colType;
    SQLUINTEGER colSize;
    SQLSMALLINT colScale;
    SQLSMALLINT nullable;
    SQLRETURN r = SQL_ERROR;
    QString qColName;
#if defined(_OS_WIN32_)
    if ( qApp->winVersion() & Qt::WV_NT_based ) {
        SQLTCHAR colName[255];
	r = SQLDescribeCol( p->hStmt,
			    i+1,
			    (SQLTCHAR*)colName,
			    sizeof(colName),
			    &colNameLen,
			    &colType,
			    &colSize,
			    &colScale,
			    &nullable);
	qColName = qt_winQString( colName );
    } else {
	SQLCHAR colName[255];
	r = SQLDescribeColA( p->hStmt,
		    i+1,
		    colName,
		    sizeof(colName),
		    &colNameLen,
		    &colType,
		    &colSize,
		    &colScale,
		    &nullable);
	qColName = qstrdup((const char*)colName);
    }
#else
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
    qColName = qstrdup((const char*)colName);
#endif
#ifdef CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( QString("qMakeField: Unable to describe column %1").arg(i), p );
#endif
    QVariant::Type type = qDecodeODBCType( colType );
    return QSqlField( qColName, i, type );
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
#if defined(_OS_WIN32_)
    if ( qApp->winVersion() & Qt::WV_NT_based ) {
	SQLTCHAR colName[255];
	r = SQLDescribeCol( hStmt,
			    column+1,
			    (SQLTCHAR*)colName,
			    sizeof(colName),
			    &colNameLen,
			    &colType,
			    &colSize,
			    &colScale,
			    &nullable);
	qColName = qt_winQString( colName );
    } else {
	SQLCHAR colName[255];
	r = SQLDescribeColA( hStmt,
			    column+1,
			    colName,
			    sizeof(colName),
			    &colNameLen,
			    &colType,
			    &colSize,
			    &colScale,
			    &nullable);
	qColName = qstrdup( (const char*)colName );
    }
#else
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
#endif
#ifdef CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qWarning( QString("qGetStringData: Unable to describe column %1").arg(column) );
#endif
#if defined(_OS_WIN32_)
    if ( qApp->winVersion() & Qt::WV_NT_based ) {
	SQLTCHAR* buf = new SQLTCHAR[ colSize ];
	while ( TRUE ) {
	    r = SQLGetData( hStmt,
			    column+1,
			    SQL_C_CHAR,
			    (SQLPOINTER)buf,
			    sizeof(buf),
			    &lengthIndicator );
	    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
		if ( lengthIndicator == SQL_NO_TOTAL )
		    fieldVal += (char*)buf;  // keep going
		else if ( lengthIndicator == SQL_NULL_DATA ) {
		    fieldVal = QString::null;
		    isNull = TRUE;
		    break;
		} else {
		    if ( r == SQL_SUCCESS ) {
			fieldVal += (char*)buf;
			break;
		    } else
			fieldVal += (char*)buf;
		}
	    } else {
		fieldVal += QString::null;
		break;
	    }
	}
	delete buf;
    } else {
	SQLCHAR* buf = new SQLCHAR[ colSize ];
	while ( TRUE ) {
	    r = SQLGetData( hStmt,
			    column+1,
			    SQL_C_CHAR,
			    (SQLPOINTER)buf,
			    sizeof(buf),
			    &lengthIndicator );
	    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
		if ( lengthIndicator == SQL_NO_TOTAL )
		    fieldVal += QString( (char*)buf );  // keep going
		else if ( lengthIndicator == SQL_NULL_DATA ) {
		    fieldVal = QString::null;
		    isNull = TRUE;
		    break;
		} else {
		    if ( r == SQL_SUCCESS ) {
			fieldVal += QString( (char*)buf );
			break;
		    } else
			fieldVal += QString( (char*)buf );
		}
	    } else {
		fieldVal += QString::null;
		break;
	    }
	}
	delete buf;
    }
#else
    SQLCHAR* buf = new SQLTCHAR[ colSize ];
    while ( TRUE ) {
	r = SQLGetData( hStmt,
			column+1,
			SQL_C_CHAR,
			(SQLPOINTER)buf,
			sizeof(buf),
			&lengthIndicator );
	if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
	    if ( lengthIndicator == SQL_NO_TOTAL )
		fieldVal += QString( (char*)buf );  // keep going
	    else if ( lengthIndicator == SQL_NULL_DATA ) {
		fieldVal = QString::null;
		isNull = TRUE;
		break;
	    } else {
		if ( r == SQL_SUCCESS ) {
		    fieldVal += QString( (char*)buf );
		    break;
		} else
		    fieldVal += QString( (char*)buf );
	    }
	} else {
	    fieldVal += QString::null;
	    break;
	}
    }
    delete buf;
#endif
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

QSqlField qMakeField( const QODBCPrivate* d, const QString& tablename, const QString& fieldname )
{
    QSqlField fi;
    SQLHANDLE hStmt;
    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef CHECK_RANGE
	qSqlWarning( "qMakeField: Unable to alloc handle", d );
#endif
	return fi;
    }
    r = SQLSetStmtAttr( hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                        SQL_IS_UINTEGER );
#if defined(_OS_WIN32_)
    if ( qApp->winVersion() & Qt::WV_NT_based )
	r =  SQLColumns( hStmt,
			 NULL,
			 0,
			 NULL,
			 0,
			 (SQLTCHAR*)qt_winTchar( tablename, TRUE ),
			 tablename.length(),
			 (SQLTCHAR*)qt_winTchar( fieldname, TRUE ),
			 fieldname.length() );
    else
	r =  SQLColumnsA( hStmt,
			 NULL,
			 0,
			 NULL,
			 0,
			 (SQLCHAR*)tablename.local8Bit().data(),
			 tablename.length(),
			 (SQLCHAR*)fieldname.local8Bit().data(),
			 fieldname.length() );
#else
    r =  SQLColumns( hStmt,
		     NULL,
		     0,
		     NULL,
		     0,
		     (SQLCHAR*)tablename.local8Bit().data(),
		     tablename.length(),
		     (SQLCHAR*)fieldname.local8Bit().data(),
		     fieldname.length() );
#endif

#ifdef CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( "qMakeField: Unable to execute column list", d );
#endif
    r = SQLFetchScroll( hStmt,
                        SQL_FETCH_NEXT,
                        0);
    if ( r == SQL_SUCCESS ) {
	bool isNull;
	int type = qGetIntData( hStmt, 4, isNull ); // column type
	QSqlField f( fieldname, 0, qDecodeODBCType( type ) );
	fi = f;
    }
    r = SQLFreeStmt( hStmt, SQL_CLOSE );
    return fi;
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
    if ( d->hStmt ) {
	SQLRETURN r = SQLFreeHandle( SQL_HANDLE_STMT, d->hStmt );
#ifdef CHECK_RANGE
	if ( r!= SQL_SUCCESS )
	    qSqlWarning( "QODBCDriver: Unable to free statement handle" + QString::number(r), d );
#endif
    }
    delete d;
}

bool QODBCResult::reset ( const QString& query )
{
    setActive( FALSE );
    setAt( BeforeFirst );
    SQLRETURN r;
    if ( d->hStmt ) {
//    	r = SQLFreeHandle( SQL_HANDLE_STMT, d->hStmt );
//#ifdef CHECK_RANGE
//	if ( r!= SQL_SUCCESS )
//	    qSqlWarning( "Unable to free statement handle", d );
//#endif
    	r = SQLFreeStmt( d->hStmt, SQL_CLOSE );
    	if ( r != SQL_SUCCESS ) {
#ifdef CHECK_RANGE
		qSqlWarning( "QODBCDriver::reset: Unable to close statement", d );
#endif
		return FALSE;
    	}
    }
    r  = SQLAllocHandle( SQL_HANDLE_STMT,
    			d->hDbc,
			&d->hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef CHECK_RANGE
	qSqlWarning( "QODBCDriver::reset: Unable to allocate statement handle", d );
#endif
	return FALSE;
    }
    r = SQLSetStmtAttr( d->hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_STATIC,
                        SQL_IS_UINTEGER );
    if ( r != SQL_SUCCESS ) {
#ifdef CHECK_RANGE
	qSqlWarning( "QODBCDriver::reset: Unable to set statement attribute", d );
#endif
	return FALSE;
    }
#if defined(_OS_WIN32_)
    if ( qApp->winVersion() & Qt::WV_NT_based )
	r = SQLExecDirect( d->hStmt,
			    (SQLTCHAR*)qt_winTchar(query, TRUE),
 			    SQL_NTS );
    else
	r = SQLExecDirectA( d->hStmt,
			    (SQLCHAR*)query.local8Bit().data(),
 			    SQL_NTS );
#else
    r = SQLExecDirect( d->hStmt,
			(SQLCHAR*)query.local8Bit().data(),
 			SQL_NTS );
#endif
    if ( r != SQL_SUCCESS ) {
	setLastError( qMakeError( "Unable to execute statement", QSqlError::Statement, d ) );
	return FALSE;
    }
    setActive( TRUE) ;
    return FALSE;
}

bool QODBCResult::fetch(int i)
{
    if ( i == at() )
        return TRUE;
    fieldCache.clear();
    nullCache.clear();
    int actualIdx = i + 1;
    if ( actualIdx <= 0 ) {
	setAt( BeforeFirst );
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
	QSqlField info = qMakeField( d, field );
	switch ( info.type() ) {
	case QVariant::Int:
	    isNull = FALSE;
	    fieldCache[ current ] = QVariant( qGetIntData( d->hStmt, current, isNull ) );
	    nullCache[ current ] = isNull;
	    break;
	case QVariant::Double:
	    SQLDOUBLE dblbuf;
	    isNull = FALSE;
	    r = SQLGetData( d->hStmt,
			    current+1,
			    SQL_C_DOUBLE,
			    (SQLPOINTER)&dblbuf,
			    0,
			    &lengthIndicator );
	    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
		if ( lengthIndicator == SQL_NULL_DATA )
		    isNull = TRUE;
	    }
	    fieldCache[ current ] = QVariant( dblbuf );
	    nullCache[ current ] = isNull;
	    break;
	case QVariant::Date:
	    DATE_STRUCT dbuf;
	    isNull = FALSE;
	    r = SQLGetData( d->hStmt,
			    current+1,
			    SQL_C_TYPE_DATE,
			    (SQLPOINTER)&dbuf,
			    0,
			    &lengthIndicator );
	    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
		if ( lengthIndicator == SQL_NULL_DATA )
		    isNull = TRUE;
	    }
	    fieldCache[ current ] = QVariant( QDate( dbuf.year, dbuf.month, dbuf.day ) );
	    nullCache[ current ] = isNull;
	    break;
	case QVariant::Time:
	    TIME_STRUCT tbuf;
	    isNull = FALSE;
	    r = SQLGetData( d->hStmt,
			    current+1,
			    SQL_C_TYPE_TIME,
			    (SQLPOINTER)&tbuf,
			    0,
			    &lengthIndicator );
	    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
		if ( lengthIndicator == SQL_NULL_DATA )
		    isNull = TRUE;
	    }
	    fieldCache[ current ] = QVariant( QTime( tbuf.hour, tbuf.minute, tbuf.second ) );
	    nullCache[ current ] = isNull;
	    break;
	case QVariant::DateTime:
	    TIMESTAMP_STRUCT dtbuf;
	    isNull = FALSE;
	    r = SQLGetData( d->hStmt,
			    current+1,
			    SQL_C_TYPE_TIMESTAMP,
			    (SQLPOINTER)&dtbuf,
			    0,
			    &lengthIndicator );
	    if ( r == SQL_SUCCESS || r == SQL_SUCCESS_WITH_INFO ) {
		if ( lengthIndicator == SQL_NULL_DATA )
		    isNull = TRUE;
	    }
	    fieldCache[ current ] = QVariant( QDateTime( QDate( dtbuf.year, dtbuf.month, dtbuf.day ), QTime( dtbuf.hour, dtbuf.minute, dtbuf.second ) ) );
	    nullCache[ current ] = isNull;
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
//     			SQL_ROW_NUMBER,
// 			&currRow,
// 			SQL_IS_INTEGER,
// 			0);
//     at = currRow;
//     r = SQLFetchScroll( d->hStmt,
//                         SQL_FETCH_LAST,
//                         0);
//     if ( r == SQL_SUCCESS ) {
// 	r = SQLGetStmtAttr( d->hStmt,
//     			SQL_ROW_NUMBER,
// 			&currRow,
// 			SQL_IS_INTEGER,
// 			0);
// 	if ( r == SQL_SUCCESS )
// 	    size = currRow;
// 	r = SQLFetchScroll( d->hStmt,
//                         SQL_FETCH_ABSOLUTE,
//                         currRow);
// 	if ( r != SQL_SUCCESS )
// 	    qSqlWarning("QODBCDriver::size: Unable to restore position", d );
//     }
//     return size;
}

int QODBCResult::numRowsAffected()
{
    SQLINTEGER affectedRowCount(0);
    SQLRETURN r = SQLRowCount( d->hStmt, &affectedRowCount);
    if ( r == SQL_SUCCESS )
	return affectedRowCount;
#ifdef CHECK_RANGE
    else
	qSqlWarning( "QODBCDriver::afectedRows: Unable to count affected rows", d );
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
    // transaction support set in open()
    setQuerySizeSupport( FALSE );
    d = new QODBCPrivate();
}

QODBCDriver::~QODBCDriver()
{
    cleanup();
    delete d;
}

bool QODBCDriver::open( const QString & db,
    			const QString & user,
			const QString & password,
			const QString & )
{
    if ( isOpen() )
	close();
    SQLRETURN r;
    r = SQLAllocHandle( SQL_HANDLE_ENV,
    			SQL_NULL_HANDLE,
			&d->hEnv);
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
#ifdef CHECK_RANGE
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
#ifdef CHECK_RANGE
	qSqlWarning("QODBCDriver::open: Unable to allocate connection", d );
#endif
	setOpenError( TRUE );
        return FALSE;
    }
    QString connQStr = "DSN=" + db + ";UID=" + user + ";PWD=" + password + ";";
    SWORD       cb;
#if defined(_OS_WIN32_)
    if ( qApp->winVersion() & Qt::WV_NT_based ) {
	SQLTCHAR connOut[255];
	r = SQLDriverConnect( d->hDbc,
				NULL,
				(SQLTCHAR*)qt_winTchar(connQStr, TRUE),
				SQL_NTS,
				(SQLTCHAR*)&connOut,
				255,
				&cb,
				SQL_DRIVER_NOPROMPT);
    } else {
	SQLCHAR connOut[255];
	r = SQLDriverConnectA( d->hDbc,
				NULL,
				(SQLCHAR*)connQStr.local8Bit().data(),
				SQL_NTS,
				connOut,
				255,
				&cb,
				SQL_DRIVER_NOPROMPT);
    }
#else
    SQLCHAR connOut[255];
    r = SQLDriverConnect( d->hDbc,
			    NULL,
			    (SQLCHAR*)connQStr.local8Bit().data(),
			    SQL_NTS,
			    connOut,
			    255,
			    &cb,
			    SQL_DRIVER_NOPROMPT);
#endif
    if ( r != SQL_SUCCESS && r != SQL_SUCCESS_WITH_INFO ) {
	setLastError( qMakeError( "Unable to connect", QSqlError::Connection, d ) );
	setOpenError( TRUE );
	return FALSE;
    }
    SQLUSMALLINT txn;
    SQLSMALLINT t;
    r = SQLGetInfo( d->hDbc,
		   	(SQLUSMALLINT)SQL_TXN_CAPABLE,
			&txn,
			sizeof(txn),
			&t);
    if ( txn == SQL_TC_NONE )
	setTransactionSupport( FALSE );
    else
	setTransactionSupport( TRUE );
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
    if ( isOpen() || isOpenError() ) {
    	if ( d->hDbc ) {
		SQLRETURN r = SQLDisconnect( d->hDbc );
		r = SQLFreeHandle( SQL_HANDLE_DBC, d->hDbc );
#ifdef CHECK_RANGE
		if ( r != SQL_SUCCESS )
		    qSqlWarning( "QODBCDriver::cleanup: Unable to free connection handle", d );
#endif
		d->hDbc = 0;
    	}
    	if ( d->hEnv ) {
		SQLRETURN r = SQLFreeHandle( SQL_HANDLE_ENV, d->hEnv );
#ifdef CHECK_RANGE
		if ( r != SQL_SUCCESS )
		    qSqlWarning( "QODBCDriver::cleanup: Unable to free environment handle", d );
#endif
		d->hEnv = 0;
    	}
    }
}

QSqlQuery QODBCDriver::createQuery() const
{
    return QSqlQuery( new QODBCResult( this, d ) );
}

bool QODBCDriver::beginTransaction()
{
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

QStringList QODBCDriver::tables( const QString& user ) const
{
    QStringList tl;
    SQLHANDLE hStmt;
    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef CHECK_RANGE
	qSqlWarning( "QODBCDriver::tables: Unable to allocate handle", d );
#endif
	return tl;
    }
    r = SQLSetStmtAttr( hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                        SQL_IS_UINTEGER );
#if defined(_OS_WIN32_)
    if ( qApp->winVersion() & Qt::WV_NT_based )
    	r = SQLTables( hStmt,
		       NULL,
		       0,
		       (SQLTCHAR*)qt_winTchar( user, FALSE ),
		       user.length(),
		       NULL,
		       0,
		       NULL,
		       0);
    else
	r = SQLTablesA( hStmt,
		       NULL,
		       0,
		       (SQLCHAR*)user.local8Bit().data(),
		       user.length(),
		       NULL,
		       0,
		       NULL,
		       0);
#else
    r = SQLTables( hStmt,
		   NULL,
		   0,
		   (SQLCHAR*)user.local8Bit().data(),
		   user.length(),
		   NULL,
		   0,
		   NULL,
		   0);
#endif
#ifdef CHECK_RANGE
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
    r = SQLFreeStmt( hStmt, SQL_CLOSE );
    return tl;
}

QSqlIndex QODBCDriver::primaryIndex( const QString& tablename ) const
{
    QSqlIndex index;
    SQLHANDLE hStmt;
    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef CHECK_RANGE
	qSqlWarning( "QODBCDriver::primaryIndex: Unable to list primary key", d );
#endif
	return index;
    }
    r = SQLSetStmtAttr( hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                        SQL_IS_UINTEGER );
#if defined(_OS_WIN32_)
    if ( qApp->winVersion() & Qt::WV_NT_based )
	r = SQLPrimaryKeys( hStmt,
			    NULL,
			    0,
			    NULL,
			    0,
			    (SQLTCHAR*)qt_winTchar(tablename, FALSE),
			    tablename.length());
    else
	r = SQLPrimaryKeysA( hStmt,
			    NULL,
			    0,
			    NULL,
			    0,
			    (SQLCHAR*)tablename.local8Bit().data(),
			    tablename.length());
#else
    r = SQLPrimaryKeys( hStmt,
			NULL,
			0,
			NULL,
			0,
			(SQLCHAR*)tablename.local8Bit().data(),
			tablename.length());
#endif
#ifdef CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QODBCDriver::primaryIndex: Unable to execute primary key list", d );
#endif
    r = SQLFetchScroll( hStmt,
                        SQL_FETCH_NEXT,
                        0);
    int count = 0;
    while ( r == SQL_SUCCESS ) {
	SQLINTEGER lengthIndicator = 0;
	bool isNull;
	QString fieldVal = qGetStringData( hStmt, 3, lengthIndicator, isNull ); // column name
	QSqlField f = qMakeField( d, tablename, fieldVal );
	f.setFieldNumber( count );
	index.append( f );
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0);
	count++;
    }
    r = SQLFreeStmt( hStmt, SQL_CLOSE );
    return index;
}

QSqlRecord QODBCDriver::record( const QString& tablename ) const
{
    QSqlRecord fil;
    SQLHANDLE hStmt;
    SQLRETURN r = SQLAllocHandle( SQL_HANDLE_STMT,
				  d->hDbc,
				  &hStmt );
    if ( r != SQL_SUCCESS ) {
#ifdef CHECK_RANGE
	qSqlWarning( "QODBCDriver::record: Unable to allocate handle", d );
#endif
	return fil;
    }
    r = SQLSetStmtAttr( hStmt,
                        SQL_ATTR_CURSOR_TYPE,
                        (SQLPOINTER)SQL_CURSOR_FORWARD_ONLY,
                        SQL_IS_UINTEGER );
#if defined(_OS_WIN32_)
    if ( qApp->winVersion() & Qt::WV_NT_based )
	r =  SQLColumns( hStmt,
			 NULL,
			 0,
			 NULL,
			 0,
			 (SQLTCHAR*)qt_winTchar(tablename, FALSE),
			 tablename.length(),
			 NULL,
			 0 );
    else
	r =  SQLColumnsA( hStmt,
			 NULL,
			 0,
			 NULL,
			 0,
			 (SQLCHAR*)tablename.local8Bit().data(),
			 tablename.length(),
			 NULL,
			 0 );
#else
    r =  SQLColumns( hStmt,
		     NULL,
		     0,
		     NULL,
		     0,
		     (SQLCHAR*)tablename.local8Bit().data(),
		     tablename.length(),
		     NULL,
		     0 );
#endif
#ifdef CHECK_RANGE
    if ( r != SQL_SUCCESS )
	qSqlWarning( "QODBCDriver::record: Unable to execute column list", d );
#endif
    r = SQLFetchScroll( hStmt,
                        SQL_FETCH_NEXT,
                        0);
    int count = 0;
    while ( r == SQL_SUCCESS ) {
	bool isNull;
	SQLINTEGER lengthIndicator(0);
	QString fieldname = qGetStringData( hStmt, 3, lengthIndicator, isNull );
	QSqlField f = qMakeField( d, tablename, fieldname );
	f.setFieldNumber( count );
	fil.append( f );
	r = SQLFetchScroll( hStmt,
			    SQL_FETCH_NEXT,
			    0);
	count++;
    }
    r = SQLFreeStmt( hStmt, SQL_CLOSE );
    return fil;
}

QSqlRecord QODBCDriver::record( const QSqlQuery& query ) const
{
    QSqlRecord fil;
    if ( query.isActive() && query.driver() == this ) {
	QODBCResult* result = (QODBCResult*)query.result();
	SQLRETURN r;
	SQLSMALLINT count;
	r = SQLNumResultCols( result->d->hStmt, &count );
#ifdef CHECK_RANGE
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
