#include "qsql_mysql.h"

#include <qlist.h>
#include <qdatetime.h>

#if defined (_OS_WIN32_)
#define NO_CLIENT_LONG_LONG
#include <qt_windows.h>
#endif
#include <mysql.h>

class QMySQLPrivate
{
public:
    MYSQL_RES	*result;
    MYSQL_ROW 	row;
    MYSQL 	*mysql;
};

QSqlError makeError( const QString& err, int type, const QMySQLPrivate* p )
{
    return QSqlError("QMySQL: " + err, QString(mysql_error( p->mysql )), type);
}

QSqlFieldInfo makeFieldInfo( const MYSQL_FIELD* f )
{
    const char* c = (const char*)f->name;
    QVariant::Type type;
    switch ( f->type ) {
    case FIELD_TYPE_SHORT :
    case FIELD_TYPE_LONG :
    case FIELD_TYPE_INT24 :
    case FIELD_TYPE_LONGLONG :
	type = QVariant::Int;
	break;
    case FIELD_TYPE_DECIMAL :
    case FIELD_TYPE_FLOAT :
    case FIELD_TYPE_DOUBLE :
	type = QVariant::Double;
	break;
    case FIELD_TYPE_DATE :
    case FIELD_TYPE_YEAR :
	type = QVariant::Date;
	break;
    case FIELD_TYPE_TIME :
	type = QVariant::Time;
	break;
    case FIELD_TYPE_DATETIME :
    case FIELD_TYPE_TIMESTAMP :
	type = QVariant::DateTime;
	break;
//     case FIELD_TYPE_BLOB :
//     case FIELD_TYPE_SET :
//     case FIELD_TYPE_ENUM :
//     case FIELD_TYPE_NULL :
// 	type = QSqlFieldInfo::Binary;
// 	break;
    default:
    case FIELD_TYPE_STRING :
    case FIELD_TYPE_CHAR :
	type = QVariant::String;
	break;
    }
    return QSqlFieldInfo( QString(c), type, f->length, f->decimals );
}

QMySQLResult::QMySQLResult( const QMySQLDriver* db )
: QSqlResult( db ),
  resultInfo( 0 )
{
    d =   new QMySQLPrivate();
    (*d) = (*db->d);
}

QMySQLResult::~QMySQLResult()
{
    cleanup();
    delete d;
}

const QSqlResultInfo* QMySQLResult::info()
{
    if ( resultInfo ) {
        delete resultInfo;
        resultInfo = 0;
    }
    if ( isActive() )
        resultInfo = new QMySQLResultInfo( d );
    return resultInfo;
}

void QMySQLResult::cleanup()
{
    if ( isActive() ) {
        mysql_free_result( d->result );
        d->result = NULL;
        d->row = NULL;
        setAt( -1 );
    }
    if ( resultInfo )
        delete resultInfo;
    setActive( FALSE );
}

bool QMySQLResult::fetch( int i )
{
    if ( at() == i )
        return TRUE;
    mysql_data_seek( d->result, i );
    d->row = mysql_fetch_row( d->result );
    if ( !d->row )
        return FALSE;
    setAt( i );
    return TRUE;
}

bool QMySQLResult::fetchLast()
{
    return fetch( mysql_num_rows( d->result ) - 1 );
}

bool QMySQLResult::fetchFirst()
{
    return fetch( 0 );
}

QVariant QMySQLResult::data( int field )
{
    if ( d->row[field] ) {
	MYSQL_FIELD* f = mysql_fetch_field_direct( d->result, field );
	if ( f ) {
	    QString val( ( d->row[field] ) );
	    QSqlFieldInfo info = makeFieldInfo( f );
	    switch ( info.type ) {
	    case QVariant::Int:
		return QVariant( val.toInt() );
		break;
	    case QVariant::Double:
		return QVariant( val.toDouble() );
		break;
	    case QVariant::Date:
		return QVariant( QDate::fromString( val, Qt::ISODate )  );
		break;
	    case QVariant::Time:
		return QVariant( QTime::fromString( val, Qt::ISODate ) );
		break;
	    case QVariant::DateTime:
		return QVariant( QDateTime::fromString( val, Qt::ISODate ) );
		break;
		//     case QSqlFieldInfo::Binary:
		//    if ( PQbinaryTuples( d->result ) )
		// 	return QVariant( );
		// 	break;
	    default:
	    case QVariant::String:
		return QVariant( val );
		break;
	    }
	}
    }
    return QVariant();
}

// QByteArray QMySQLResult::binary( int field )
// {
//     unsigned long *lengths;
//     lengths = mysql_fetch_lengths( d->result );
//     QByteArray b;
//     b.duplicate( d->row[field], lengths[field]);
//     return b;
// }

bool QMySQLResult::isNull( int field ) const
{
    if ( d->row[field] == NULL )
	return TRUE;
    return FALSE;
}

bool QMySQLResult::reset ( const QString& query )
{
    cleanup();
    if ( !driver() )
        return FALSE;
    if ( !driver()-> isOpen() || driver()->isOpenError() )
        return FALSE;
    if ( mysql_real_query( d->mysql, query, query.length() ) ) {
	setLastError( makeError("Unable to execute query", QSqlError::Statement, d ) );
	return FALSE;
    }
    d->result = mysql_store_result( d->mysql );
    if ( !d->result ) {
	setLastError( makeError( "Unable to store result", QSqlError::Statement, d ) );
	return FALSE;
    }
    setActive( TRUE );
    return TRUE;
}

/////////////////////////////////////////////////////////

QMySQLResultInfo::QMySQLResultInfo( QMySQLPrivate* p )
{
    if ( !mysql_errno( p->mysql ) ) {
    	for ( ;; ) {
	    MYSQL_FIELD* f = mysql_fetch_field( p->result );
	    if ( f )
		appendField( makeFieldInfo( f ) );
	    else
            	break;
    	}
	setSize( (int)mysql_num_rows( p->result ) );
	setAffectedRows( (int)mysql_affected_rows( p->mysql ) );
    }
}

QMySQLResultInfo::~QMySQLResultInfo()
{
}

/////////////////////////////////////////////////////////

QMySQLDriver::QMySQLDriver( QObject * parent, const char * name )
: QSqlDriver(parent, name ? name : "QMySQL")
{
    init();
}

void QMySQLDriver::init()
{
    setTransactionSupport( FALSE );
    d = new QMySQLPrivate();
}

QMySQLDriver::~QMySQLDriver()
{
    delete d;
}

bool QMySQLDriver::open( const QString & db,
    			const QString & user,
			const QString & password,
			const QString & host)
{
    if ( isOpen() )
        close();
    if ( (d->mysql = mysql_init((MYSQL*) 0)) &&
	    mysql_real_connect( d->mysql,
				host,
				user,
				password,
				db,
				0,
				NULL,
				0))
    {
	if ( mysql_select_db( d->mysql, db )) {
	    setLastError( makeError("Unable to real connect", QSqlError::Connection, d ) );
	    mysql_close( d->mysql );
	    setOpenError( TRUE );
	    return FALSE;
	}
    } else {
    	    setLastError( makeError( "Unable to connect", QSqlError::Connection, d ) );
	    mysql_close( d->mysql ) ;
	    return FALSE;
    }
    setOpen( TRUE );
    return TRUE;
}

void QMySQLDriver::close()
{
    if ( isOpen() ) {
        mysql_close( d->mysql );
	setOpen( FALSE );
	setOpenError( FALSE );
    }
}

QSql QMySQLDriver::createResult() const
{
    return QSql(new QMySQLResult( this ) );
}

QStringList QMySQLDriver::tables() const
{
    MYSQL_RES* tableRes = mysql_list_tables( d->mysql, NULL );
    MYSQL_ROW 	row;
    QStringList tl;
    int i = 0;
    while ( TRUE ) {
	mysql_data_seek( tableRes, i );
	row = mysql_fetch_row( tableRes );
	if ( !row )
	    break;
	tl.append( QString(row[0]) );
    }
    mysql_free_result( tableRes );
    return tl;
}
