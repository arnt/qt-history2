/****************************************************************************
**
** Implementation of MYSQL driver classes.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsql_mysql.h"

#include <qdatetime.h>
#include <qvector.h>
#include <qsqlrecord.h>
#include <qtextcodec.h>

#define QMYSQL_DRIVER_NAME "QMYSQL3"

#ifdef Q_OS_WIN32
// comment the next line out if you want to use MySQL/embedded on Win32 systems.
// note that it will crash if you don't statically link to the mysql/e library!
# define Q_NO_MYSQL_EMBEDDED
#endif

class QMYSQLDriverPrivate
{
public:
    QMYSQLDriverPrivate() : mysql(0), tc(0) {}
    MYSQL*     mysql;
    QTextCodec *tc;
};

class QMYSQLResultPrivate : public QMYSQLDriverPrivate
{
public:
    QMYSQLResultPrivate() : QMYSQLDriverPrivate(), result(0), tc(QTextCodec::codecForLocale()) {}
    MYSQL_RES* result;
    MYSQL_ROW  row;
    QVector<QCoreVariant::Type> fieldTypes;
    QTextCodec *tc;
};

QTextCodec* codec(MYSQL* mysql)
{
    QTextCodec* heuristicCodec = QTextCodec::codecForName(mysql_character_set_name(mysql), 2);
    if (heuristicCodec)
	return heuristicCodec;
    return QTextCodec::codecForLocale();
}

QSqlError qMakeError( const QString& err, int type, const QMYSQLDriverPrivate* p )
{
    return QSqlError(QMYSQL_DRIVER_NAME ": " + err, QString(mysql_error( p->mysql )), type, mysql_errno( p->mysql ));
}

QCoreVariant::Type qDecodeMYSQLType( int mysqltype, uint flags )
{
    QCoreVariant::Type type;
    switch ( mysqltype ) {
    case FIELD_TYPE_TINY :
    case FIELD_TYPE_SHORT :
    case FIELD_TYPE_LONG :
    case FIELD_TYPE_INT24 :
	type = (flags & UNSIGNED_FLAG) ? QCoreVariant::UInt : QCoreVariant::Int;
	break;
    case FIELD_TYPE_YEAR :
	type = QCoreVariant::Int;
	break;
    case FIELD_TYPE_LONGLONG :
	type = (flags & UNSIGNED_FLAG) ? QCoreVariant::ULongLong : QCoreVariant::LongLong;
	break;
    case FIELD_TYPE_DECIMAL :
    case FIELD_TYPE_FLOAT :
    case FIELD_TYPE_DOUBLE :
	type = QCoreVariant::Double;
	break;
    case FIELD_TYPE_DATE :
	type = QCoreVariant::Date;
	break;
    case FIELD_TYPE_TIME :
	type = QCoreVariant::Time;
	break;
    case FIELD_TYPE_DATETIME :
    case FIELD_TYPE_TIMESTAMP :
	type = QCoreVariant::DateTime;
	break;
    case FIELD_TYPE_BLOB :
    case FIELD_TYPE_TINY_BLOB :
    case FIELD_TYPE_MEDIUM_BLOB :
    case FIELD_TYPE_LONG_BLOB :
	type = (flags & BINARY_FLAG) ? QCoreVariant::ByteArray : QCoreVariant::CString;
	break;
    default:
    case FIELD_TYPE_ENUM :
    case FIELD_TYPE_SET :
    case FIELD_TYPE_STRING :
    case FIELD_TYPE_VAR_STRING :
	type = QCoreVariant::String;
	break;
    }
    return type;
}

QMYSQLResult::QMYSQLResult( const QMYSQLDriver* db )
: QSqlResult( db )
{
    d = new QMYSQLResultPrivate();
    d->mysql = db->d->mysql;
    d->tc = db->d->tc;
}

QMYSQLResult::~QMYSQLResult()
{
    cleanup();
    delete d;
}

MYSQL_RES* QMYSQLResult::result()
{
    return d->result;
}

void QMYSQLResult::cleanup()
{
    if ( d->result ) {
	mysql_free_result( d->result );
    }
    d->result = NULL;
    d->row = NULL;
    setAt( -1 );
    setActive( FALSE );
}

bool QMYSQLResult::fetch( int i )
{
    if ( isForwardOnly() ) { // fake a forward seek
	if ( at() < i ) {
	    int x = i - at();
	    while ( --x && fetchNext() );
	    return fetchNext();
	} else {
	    return FALSE;
	}
    }    
    if ( at() == i )
	return TRUE;
    mysql_data_seek( d->result, i );
    d->row = mysql_fetch_row( d->result );
    if ( !d->row )
	return FALSE;
    setAt( i );
    return TRUE;
}

bool QMYSQLResult::fetchNext()
{
    d->row = mysql_fetch_row( d->result );
    if ( !d->row )
	return FALSE;
    setAt( at() + 1 );
    return TRUE;
}

bool QMYSQLResult::fetchLast()
{
    if ( isForwardOnly() ) { // fake this since MySQL can't seek on forward only queries
	bool success = fetchNext(); // did we move at all?
	while ( fetchNext() );
	return success;
    }
    my_ulonglong numRows = mysql_num_rows( d->result );
    if ( !numRows )
	return FALSE;
    return fetch( numRows - 1 );
}

bool QMYSQLResult::fetchFirst()
{
    if ( isForwardOnly() ) // again, fake it
	return fetchNext();
    return fetch( 0 );
}

QCoreVariant QMYSQLResult::data( int field )
{
    if ( !isSelect() || field >= (int) d->fieldTypes.count() ) {
	qWarning( "QMYSQLResult::data: column %d out of range", field );
	return QCoreVariant();
    }
 
    if (d->row[field] == NULL) {
        // NULL value
        return QCoreVariant(d->fieldTypes.at(field));
    }
   
    QString val;
    if (d->fieldTypes.at(field) != QCoreVariant::ByteArray)
	val = d->tc->toUnicode(d->row[field]);
    switch ( d->fieldTypes.at( field ) ) {
    case QCoreVariant::LongLong:
	if ( val[0] == '-' ) // signed or unsigned?
	    return QCoreVariant( val.toLongLong() );
	else
	    return QCoreVariant( val.toULongLong() );
    case QCoreVariant::Int: 
	return QCoreVariant( val.toInt() );
    case QCoreVariant::Double:
	return QCoreVariant( val.toDouble() );
    case QCoreVariant::Date:
	if ( val.isEmpty() ) {
	    return QCoreVariant( QDate() );
	} else {
	    return QCoreVariant( QDate::fromString( val, Qt::ISODate )  );
	}
    case QCoreVariant::Time:
	if ( val.isEmpty() ) {
	    return QCoreVariant( QTime() );
	} else {
	    return QCoreVariant( QTime::fromString( val, Qt::ISODate ) );
	}
    case QCoreVariant::DateTime:
	if ( val.isEmpty() )
	    return QCoreVariant( QDateTime() );
	if ( val.length() == 14u )
	    // TIMESTAMPS have the format yyyyMMddhhmmss
	    val.insert(4, "-").insert(7, "-").insert(10, 'T').insert(13, ':').insert(16, ':');
	return QCoreVariant( QDateTime::fromString( val, Qt::ISODate ) );
    case QCoreVariant::ByteArray: {
	unsigned long* fl = mysql_fetch_lengths( d->result );
	QByteArray ba(d->row[field], fl[field]);
	return QCoreVariant( ba );
    }
    default:
    case QCoreVariant::String:
	return QCoreVariant(val);
    }
#ifdef QT_CHECK_RANGE
    qWarning("QMYSQLResult::data: unknown data type");
#endif
    return QCoreVariant();
}

bool QMYSQLResult::isNull( int field )
{
    if ( d->row[field] == NULL )
	return TRUE;
    return FALSE;
}

bool QMYSQLResult::reset ( const QString& query )
{
    if ( !driver() )
	return FALSE;
    if ( !driver()-> isOpen() || driver()->isOpenError() )
	return FALSE;
    cleanup();
    const QByteArray encQuery(d->tc->fromUnicode(query));
    if ( mysql_real_query( d->mysql, encQuery.data(), encQuery.length() ) ) {
	setLastError( qMakeError("Unable to execute query", QSqlError::Statement, d ) );
	return FALSE;
    }
    if ( isForwardOnly() ) {
 	if ( isActive() || isValid() ) // have to empty the results from previous query
	    fetchLast();
 	d->result = mysql_use_result( d->mysql );
    } else {
	d->result = mysql_store_result( d->mysql );
    }
    if ( !d->result && mysql_field_count( d->mysql ) > 0 ) {
	setLastError( qMakeError( "Unable to store result", QSqlError::Statement, d ) );
	return FALSE;
    }
    int numFields = mysql_field_count( d->mysql );
    setSelect( !( numFields == 0) );
    d->fieldTypes.resize( numFields );
    if ( isSelect() ) {
	for( int i = 0; i < numFields; i++) {
	    MYSQL_FIELD* field = mysql_fetch_field_direct( d->result, i );
	    if ( field->type == FIELD_TYPE_DECIMAL )
		d->fieldTypes[i] = QCoreVariant::String;
	    else
		d->fieldTypes[i] = qDecodeMYSQLType( field->type, field->flags );
	}
    }
    setActive( TRUE );
    return TRUE;
}

int QMYSQLResult::size()
{
    return isSelect() ? (int)mysql_num_rows( d->result ) : -1;
}

int QMYSQLResult::numRowsAffected()
{
    return (int)mysql_affected_rows( d->mysql );
}

QSqlRecord QMYSQLResult::record() const
{
    QSqlRecord info;
    if (!isActive() || !isSelect())
	return info;
    if (!mysql_errno(d->mysql)) {
	MYSQL_FIELD* field = mysql_fetch_field(d->result);
	while(field) {
	    info.append ( QSqlField( d->tc->toUnicode( field->name ),
					 qDecodeMYSQLType( (int)field->type, field->flags ),
					 IS_NOT_NULL( field->flags ),
					 (int)field->length,
					 (int)field->decimals,
					 QCoreVariant(),
					 (int)field->type ) );
	    field = mysql_fetch_field( d->result );		
	}
    }
    mysql_field_seek(d->result, 0);
    return info;
}


/////////////////////////////////////////////////////////

static void qServerInit()
{
#ifndef Q_NO_MYSQL_EMBEDDED
# if MYSQL_VERSION_ID >= 40000
    static bool init = FALSE;
    if ( init )
	return;

    // this should only be called once
    // has no effect on client/server library
    // but is vital for the embedded lib
    if ( mysql_server_init( 0, 0, 0 ) ) {
#  ifdef QT_CHECK_RANGE
	qWarning( "QMYSQLDriver::qServerInit: unable to start server." );
#  endif
    }
    init = TRUE;    
# endif // MYSQL_VERSION_ID
#endif // Q_NO_MYSQL_EMBEDDED
}

QMYSQLDriver::QMYSQLDriver(QObject * parent)
    : QSqlDriver(parent)
{
    init();
    qServerInit();
}

/*!
    Create a driver instance with an already open connection handle.
*/

QMYSQLDriver::QMYSQLDriver(MYSQL * con, QObject * parent)
    : QSqlDriver(parent)
{
    init();
    if ( con ) {
	d->mysql = (MYSQL *) con;
	d->tc = codec(con);
	setOpen( TRUE );
	setOpenError( FALSE );
    } else {
	qServerInit();
    }
}

void QMYSQLDriver::init()
{
    d = new QMYSQLDriverPrivate();
    d->mysql = 0;
}

QMYSQLDriver::~QMYSQLDriver()
{
    delete d;
#ifndef Q_NO_MYSQL_EMBEDDED
# if MYSQL_VERSION_ID > 40000
    mysql_server_end();
# endif
#endif
}

bool QMYSQLDriver::hasFeature( DriverFeature f ) const
{
    switch ( f ) {
    case Transactions:
// CLIENT_TRANSACTION should be defined in all recent mysql client libs > 3.23.34
#ifdef CLIENT_TRANSACTIONS
	if ( d->mysql ) {
	    if ( ( d->mysql->server_capabilities & CLIENT_TRANSACTIONS ) == CLIENT_TRANSACTIONS )
		return TRUE;
	}
#endif
	return FALSE;
    case QuerySize:
	return TRUE;
    case BLOB:
	return TRUE;
    case Unicode:
	return FALSE;
    default:
	return FALSE;
    }
}

bool QMYSQLDriver::open( const QString& db,
			 const QString& user,
			 const QString& password,
			 const QString& host,
			 int port,
			 const QString& connOpts )
{
    if ( isOpen() )
	close();
 
    unsigned int optionFlags = 0;
    
    QStringList raw = connOpts.split(';');
    QStringList opts;
    QStringList::ConstIterator it;
    
    // extract the real options from the string
    for ( it = raw.begin(); it != raw.end(); ++it ) {
	QString tmp( *it );
	int idx;
	if ( (idx = tmp.indexOf( '=' )) != -1 ) {
	    QString val( tmp.mid( idx + 1 ) );
	    val.simplified();
	    if ( val == "TRUE" || val == "1" )
		opts << tmp.left( idx );
	    else
		qWarning( "QMYSQLDriver::open: Illegal connect option value '%s'", tmp.latin1() );
	} else {
	    opts << tmp;
	}
    }
    
    for ( it = opts.begin(); it != opts.end(); ++it ) {
	QString opt( (*it).toUpper() );
	if ( opt == "CLIENT_COMPRESS" )
	    optionFlags |= CLIENT_COMPRESS;
	else if ( opt == "CLIENT_FOUND_ROWS" )
	    optionFlags |= CLIENT_FOUND_ROWS;
	else if ( opt == "CLIENT_IGNORE_SPACE" )
	    optionFlags |= CLIENT_IGNORE_SPACE;
	else if ( opt == "CLIENT_INTERACTIVE" )
	    optionFlags |= CLIENT_INTERACTIVE;
	else if ( opt == "CLIENT_NO_SCHEMA" )
	    optionFlags |= CLIENT_NO_SCHEMA;
	else if ( opt == "CLIENT_ODBC" )
	    optionFlags |= CLIENT_ODBC;
	else if ( opt == "CLIENT_SSL" )
	    optionFlags |= CLIENT_SSL;
	else 
	    qWarning( "QMYSQLDriver::open: Unknown connect option '%s'", (*it).latin1() );
    }

    if ( (d->mysql = mysql_init((MYSQL*) 0)) &&
	    mysql_real_connect( d->mysql,
				host.local8Bit(),
				user.local8Bit(),
				password.local8Bit(),
				db.isNull() ? "" : db.local8Bit(),
				(port > -1) ? port : 0,
				NULL,
				optionFlags ) )
    {
	if ( mysql_select_db( d->mysql, db.local8Bit() )) {
	    setLastError( qMakeError("Unable open database '" + db + "'", QSqlError::Connection, d ) );
	    mysql_close( d->mysql );
	    setOpenError( TRUE );
	    return FALSE;
	}
    } else {
	    setLastError( qMakeError( "Unable to connect", QSqlError::Connection, d ) );
	    mysql_close( d->mysql );
	    setOpenError( TRUE );
	    return FALSE;
    }
    d->tc = codec(d->mysql);
    setOpen( TRUE );
    setOpenError( FALSE );
    return TRUE;
}

void QMYSQLDriver::close()
{
    if ( isOpen() ) {
	mysql_close( d->mysql );
	setOpen( FALSE );
	setOpenError( FALSE );
    }
}

QSqlQuery QMYSQLDriver::createQuery() const
{
    return QSqlQuery( new QMYSQLResult( this ) );
}

QStringList QMYSQLDriver::tables( const QString& typeName ) const
{
    QStringList tl;
    if ( !isOpen() )
	return tl;
    if ( !typeName.isEmpty() && !(typeName.toInt() & (int)QSql::Tables) )
	return tl;

    MYSQL_RES* tableRes = mysql_list_tables( d->mysql, NULL );
    MYSQL_ROW	row;
    int i = 0;
    while ( tableRes && TRUE ) {
	mysql_data_seek( tableRes, i );
	row = mysql_fetch_row( tableRes );
	if ( !row )
	    break;
	tl.append(d->tc->toUnicode(row[0]));
	i++;
    }
    mysql_free_result( tableRes );
    return tl;
}

QSqlIndex QMYSQLDriver::primaryIndex( const QString& tablename ) const
{
    QSqlIndex idx;
    if ( !isOpen() )
	return idx;
    QSqlQuery i = createQuery();
    QString stmt( "show index from %1;" );
    QSqlRecord fil = record( tablename );
    i.exec( stmt.arg( tablename ) );
    while ( i.isActive() && i.next() ) {
	if ( i.value(2).toString() == "PRIMARY" ) {
	    idx.append( *fil.field( i.value(4).toString() ) );
	    idx.setCursorName( i.value(0).toString() );
	    idx.setName( i.value(2).toString() );
	}
    }
    return idx;
}

QSqlRecord QMYSQLDriver::record( const QString& tablename ) const
{
    QSqlRecord info;
    if ( !isOpen() )
	return info;
    MYSQL_RES* r = mysql_list_fields( d->mysql, tablename.local8Bit(), 0);
    if ( !r ) {
	return info;
    }
    MYSQL_FIELD* field;
    while ( (field = mysql_fetch_field( r ))) {
	info.append ( QSqlField( d->tc->toUnicode( field->name ),
				qDecodeMYSQLType( (int)field->type, field->flags ),
				IS_NOT_NULL( field->flags ),
				(int)field->length,
				(int)field->decimals,
				QString( field->def ),
				(int)field->type ) );
    }
    mysql_free_result( r );
    return info;
}

MYSQL* QMYSQLDriver::mysql()
{
     return d->mysql;
}

bool QMYSQLDriver::beginTransaction()
{
#ifndef CLIENT_TRANSACTIONS
    return FALSE;
#endif
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QMYSQLDriver::beginTransaction: Database not open" );
#endif
	return FALSE;
    }
    if ( mysql_query( d->mysql, "BEGIN WORK" ) ) {
	setLastError( qMakeError("Unable to begin transaction", QSqlError::Statement, d ) );
	return FALSE;
    }
    return TRUE;
}

bool QMYSQLDriver::commitTransaction()
{
#ifndef CLIENT_TRANSACTIONS
    return FALSE;
#endif
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QMYSQLDriver::commitTransaction: Database not open" );
#endif
	return FALSE;
    }
    if ( mysql_query( d->mysql, "COMMIT" ) ) {
	setLastError( qMakeError("Unable to commit transaction", QSqlError::Statement, d ) );
	return FALSE;
    }
    return TRUE;
}

bool QMYSQLDriver::rollbackTransaction()
{
#ifndef CLIENT_TRANSACTIONS
    return FALSE;
#endif
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QMYSQLDriver::rollbackTransaction: Database not open" );
#endif
	return FALSE;
    }
    if ( mysql_query( d->mysql, "ROLLBACK" ) ) {
	setLastError( qMakeError("Unable to rollback transaction", QSqlError::Statement, d ) );
	return FALSE;
    }
    return TRUE;
}

QString QMYSQLDriver::formatValue( const QSqlField* field, bool trimStrings ) const
{
    QString r;
    if ( field->isNull() ) {
	r = nullText();
    } else {
	switch( field->type() ) {
	case QCoreVariant::ByteArray: {
	
	    const QByteArray ba = field->value().toByteArray();
	    // buffer has to be at least length*2+1 bytes
	    char* buffer = new char[ ba.size() * 2 + 1 ];
	    int escapedSize = (int)mysql_escape_string( buffer, ba.data(), ba.size() );
	    r.reserve(escapedSize + 3);
	    r.append("'").append(buffer).append("'");
	    delete[] buffer;
	}
	break;
	case QCoreVariant::String:
	    // Escape '\' characters
	    r = QSqlDriver::formatValue( field );
	    r.replace( "\\", "\\\\" );
	    break;
	default:
	    r = QSqlDriver::formatValue( field, trimStrings );
	}
    }
    return r;
}
