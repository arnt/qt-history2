#if defined(QT_SQL_SUPPORT)
#if defined(QT_SQL_MSQL_SUPPORT)

#include "qsql_msql.h"

#include <qlist.h>
#include <qdatetime.h>
#include <msql.h>

extern char msqlTypeNames[12][12];

void unused()
{
    msqlTypeNames = msqlTypeNames;
}

class QMSQLPrivate
{
public:
    int 	sock;
    m_result* 	result;
    m_row	row;
    int		result_rows;
};

QSqlError makeError( const QString& err, int type, const QMSQLPrivate* p )
{
    return QSqlError("QMSQL: " + err, QString( msqlErrMsg ), type );
    Q_CONST_UNUSED( p );
}

QMSQLResult::QMSQLResult( const QMSQLDriver* db )
: QSqlResult( db ),
  resultInfo( 0 )
{
    d =   new QMSQLPrivate();
    (*d) = (*db->d);
}

QMSQLResult::~QMSQLResult()
{
    cleanup();
    delete d;
}

const QSqlResultInfo* QMSQLResult::info()
{
    if ( resultInfo ) {
        delete resultInfo;
        resultInfo = 0;
    }
    if ( isActive() )
        resultInfo = new QMSQLResultInfo( d );
    return resultInfo;
}

void QMSQLResult::cleanup()
{
    if ( isActive() ) {
	msqlFreeResult( d->result );
        d->result = NULL;
        setAt( -1 );
    }
    if ( resultInfo )
        delete resultInfo;
    setActive( FALSE );
}

bool QMSQLResult::fetch( int i )
{
    if ( at() == i )
        return TRUE;
    msqlDataSeek( d->result, i );
    d->row = msqlFetchRow( d->result );
    if ( !d->row ) {
        return FALSE;
    }
    setAt( i );
    return TRUE;
}

bool QMSQLResult::fetchFirst()
{
    return fetch( 0 );
}

bool QMSQLResult::fetchLast()
{
    return fetch( msqlNumRows( d->result ) - 1 );
}

QString QMSQLResult::string( int field )
{
    return QString( ( d->row[field] ) );
}

QByteArray QMSQLResult::binary( int field )
{
    return QByteArray();
}

bool QMSQLResult::isNull( int i ) const
{
    return FALSE;
}

bool QMSQLResult::reset ( const QString& query )
{
    cleanup();
    if ( !driver() )
        return FALSE;
    if ( !driver()-> isOpen() || driver()->isOpenError() )
        return FALSE;
    setActive( FALSE );
    QString cleanQuery ( query );
    int delim = cleanQuery.findRev( ";" );
    int len = cleanQuery.length()-1;
    if ( delim > -1 && delim == len )
	cleanQuery.replace( cleanQuery.length()-1, 1, "" );
    if ( msqlQuery( d->sock, cleanQuery.local8Bit().data() ) > -1 )
        d->result = msqlStoreResult();
    if ( d->result )
        setActive( TRUE );
    else
	setLastError( makeError( "Unable to execute query", QSqlError::Statement, d ) );
    return isActive();
}

///////////////////////////////////////////////////////////////////////////

QMSQLResultInfo::QMSQLResultInfo( QMSQLPrivate* p )
{
    for ( ;; ) {
    	m_field* f = msqlFetchField( p->result );
        if ( f ) {
            const char* c = (const char*)f->name;
            if ( c )
		appendField( QString(c) );
        }
        else
            break;
    }
    setSize( msqlNumRows( p->result ) );
}

QMSQLResultInfo::~QMSQLResultInfo()
{
}

///////////////////////////////////////////////////////////////////////////

QMSQLDriver::QMSQLDriver( QObject * parent, const char * name )
: QSqlDriver(parent,name ? name : "QMSQL")
{
    init();
}

void QMSQLDriver::init()
{
    setTransactionSupport( FALSE );
    d = new QMSQLPrivate();
}

QMSQLDriver::~QMSQLDriver()
{
    delete d;
}

bool QMSQLDriver::open( const QString & db,
    				const QString & user,
				const QString & password,
				const QString & host)
{
    if ( isOpen() )
        close();
    if ( host.length() > 0 )
	d->sock = msqlConnect( host.local8Bit().data() );
    else
	d->sock = msqlConnect( NULL ); // fast local connect
    if ( d->sock == -1 ) {
	setLastError( makeError( "Unable to connect" , QSqlError::Connection, d ) );
        setOpenError( TRUE );
        return FALSE;
    }
    int dbr = msqlSelectDB( d->sock, db.local8Bit().data() );
    if ( dbr == -1 ) {
    	setLastError( makeError( "Unable to select database", QSqlError::Connection, d ) );
	close();
        setOpenError( TRUE );
        return FALSE;
    }
    setOpen( TRUE );
    return TRUE;
    Q_CONST_UNUSED(user);
    Q_CONST_UNUSED(password);
}

void QMSQLDriver::close()
{
    if ( isOpen() ) {
	msqlClose( d->sock );
	setOpen( FALSE );
	setOpenError( FALSE );
    }
}

QSql QMSQLDriver::createResult() const
{
    return QSql( new QMSQLResult( this ) );
}

#endif
#endif
