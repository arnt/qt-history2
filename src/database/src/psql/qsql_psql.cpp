#include "qsql_psql.h"

#include <qlist.h>
#include <qdatetime.h>
#include <qpointarray.h>
#include <postgres.h>
#include <libpq-fe.h>
#include <catalog/pg_type.h>
#include <utils/geo_decls.h>

class QPSQLPrivate
{
public:
  QPSQLPrivate():connection(0), result(0){}
    PGconn 	*connection;
    PGresult 	*result;
};

QSqlError makeError( const QString& err, int type, const QPSQLPrivate* p )
{
    return QSqlError("QPSQL: " + err, QString(PQerrorMessage( p->connection )), type);
}

QSqlFieldInfo makeFieldInfo( QPSQLPrivate* p, int i )
{
    QVariant::Type type;
    switch ( PQftype( p->result, i ) ) {
    case BOOLOID	:
    case INT8OID	:
    case INT2OID	:
    case INT2VECTOROID  :
    case INT4OID        :
	type = QVariant::Int;
	break;
    case NUMERICOID     :
    case FLOAT4OID      :
    case FLOAT8OID      :
	type = QVariant::Double;
	break;
    case ABSTIMEOID     :
    case RELTIMEOID     :
    case DATEOID	:
	type = QVariant::Date;
	break;
    case TIMEOID	:
    case TIMETZOID      :
	type = QVariant::Time;
	break;
    case TIMESTAMPOID   :
	type = QVariant::DateTime;
	break;
    case POINTOID       :
	type = QVariant::Point;
	break;
    case BOXOID         :
	type = QVariant::Rect;
	break;
    case POLYGONOID     :
	type = QVariant::PointArray;
	break;
    case ZPBITOID	:
    case VARBITOID	:
	type = QVariant::ByteArray;
	break;
    case REGPROCOID     :
    case OIDOID         :
    case TIDOID         :
    case XIDOID         :
    case CIDOID         :
    case OIDVECTOROID   :
    case UNKNOWNOID     :
    case CASHOID        : // deprecated
    case INETOID        :
    case CIDROID        :
    case TINTERVALOID   :
    case CIRCLEOID      :
    case PATHOID        :
    case LSEGOID        :
    case LINEOID        :
	type = QVariant::Invalid;
	break;
    default:
    case CHAROID	:
    case BPCHAROID	:
    case LZTEXTOID	:
    case VARCHAROID	:
    case TEXTOID	:
    case NAMEOID	:
    case BYTEAOID	:
	type = QVariant::String;
	break;
    }
    return QSqlFieldInfo( PQfname( p->result, i ), type, PQfsize( p->result, i ), 0  );
}

QPSQLResult::QPSQLResult( const QPSQLDriver* db, const QPSQLPrivate* p )
: QSqlResult( db ),
  currentSize( 0 ),
  resultInfo( 0 )
{
    d =   new QPSQLPrivate();
    (*d) = (*p);
    qDebug("QPSQLResult( const QPSQLDriver* db, const QPSQLPrivate* p )");
}

QPSQLResult::~QPSQLResult()
{
    cleanup();
    delete d;
    qDebug("~QPSQLResult()");
}

const QSqlResultInfo* QPSQLResult::info()
{
    if ( resultInfo ) {
        delete resultInfo;
        resultInfo = 0;
    }
    if ( isActive() )
        resultInfo = new QPSQLResultInfo( d );
    return resultInfo;
}

void QPSQLResult::cleanup()
{
    if ( isActive() ) {
	if ( d->result )
	    PQclear( d->result );
        d->result = 0;
        setAt( -1 );
    }
    if ( resultInfo )
        delete resultInfo;
    currentSize = 0;
    setActive( FALSE );
}

bool QPSQLResult::fetch( int i )
{
    if ( !isActive() )
        return FALSE;
    if ( at() == i )
        return TRUE;
    if ( i >= currentSize )
	return FALSE;
    if ( i < 0 )
	return FALSE;
    setAt( i );
    return TRUE;
}

bool QPSQLResult::fetchFirst()
{
    return fetch( 0 );
}

bool QPSQLResult::fetchLast()
{
    return fetch( PQntuples( d->result ) - 1 );
}

QPoint pointFromString( const QString& s)
{
    // format '(x,y)'
    int pivot = s.find( QRegExp(",") );
    if ( pivot != -1 ) {
	int x = s.mid( 1, pivot-1 ).toInt();
	int y = s.mid( pivot+1, s.length()-pivot-2 ).toInt();
	return QPoint( x, y ) ;
    } else
	return QPoint();
}

QVariant QPSQLResult::data( int i )
{
    QSqlFieldInfo info = makeFieldInfo( d, i );
    if ( PQbinaryTuples( d->result ) ) {
	char* rawdata = PQgetvalue( d->result, at(), i );
	int rawsize = PQfsize( d->result, i );
	switch ( info.type ) {
	case QVariant::String:
    	    return QVariant( QString(rawdata) );
	case QVariant::Int:
	    {
		int* i = (int*)rawdata;
		return QVariant( (*i) );
	    }
	case QVariant::Double:
	    {
		double* dbl = (double*)rawdata;
		return QVariant( (*dbl) );
	    }
	case QVariant::Date:
	    {
// 		uint dt = *((uint*)rawdata) + QDate::greg2jul(2000,1,1);  // ### these are protected QDate methods...?
// 		int y,m,d;
// 		QDate::jul2greg( dt, y, m, d );
// 		return QVariant( QDate( y, m, d ) );
	    }
	case QVariant::Time:
	    {
		double tm = *((double*)rawdata);
		int hour = ((int)tm / ( 60 * 60 ) );
		int min = (((int) (tm / 60)) % 60 );
		int sec = (((int) tm) % 60 );
		return QVariant( QTime( hour, min, sec ) );
	    }
	case QVariant::DateTime:
	    // ### same problem as QVariant::Date above
	case QVariant::Point:
	    {
		Point* p = (Point*) rawdata;
		return QVariant( QPoint( p->x, p->y ) );
	    }
	case QVariant::Rect:
	    {
		BOX* b = (BOX*) rawdata;
		return QVariant( QRect( QPoint(b->high.x, b->high.y), QPoint(b->low.x, b->low.y) ) );
	    }
	case QVariant::PointArray:
	    {
		POLYGON* p = (POLYGON *) rawdata;
		QPointArray pa( p->size );
		for ( int i = 0; i < p->size; ++i )
		    pa[i] = QPoint( p->p[i].x, p->p[i].y );
		return QVariant( pa );
	    }
	case QVariant::ByteArray:
	    {
		QByteArray ba;
		ba.duplicate( rawdata, rawsize );
		return QVariant( ba );
	    }
	default:
	case QVariant::Invalid:
	    return QVariant();
	}
    } else {
	QString val( PQgetvalue( d->result, at(), i ) );
	switch ( info.type ) {
	case QVariant::String:
	    return QVariant( val );
	case QVariant::Int:
	    return QVariant( val.toInt() );
	case QVariant::Double:
	    return QVariant( val.toDouble() );
	case QVariant::Date:
	    return QVariant( QDate::fromString( val, Qt::ISODate ) );
	case QVariant::Time:
	    return QVariant( QTime::fromString( val, Qt::ISODate ) );
	case QVariant::DateTime:
	    return QVariant( QDateTime::fromString( val, Qt::ISODate ) );
	case QVariant::Point:
	    return QVariant( pointFromString( val ) );
	case QVariant::Rect: // format '(x,y),(x',y')'
	    {
		int pivot = val.find( QRegExp( "\\),\\(" ) );
		if ( pivot != -1 )
		    return QVariant( QRect( pointFromString( val.mid(0,pivot+1) ), pointFromString( val.mid(pivot+2,val.length()) ) ) );
		return QVariant( QRect() );
	    }
	case QVariant::PointArray: // format '((x,y),(x1,y1),...,(xn,yn))'
	    {
		QRegExp pointPattern("\\([0-9-]*,[0-9-]*\\)");
		int points = val.contains( pointPattern );
		QPointArray parray( points );
		int idx = 1;
		for ( int i = 0; i < points; i++ ){
		    int start = val.find( pointPattern, idx );
		    int end = -1;
		    if ( start != -1 ) {
			end = val.find( QRegExp("\\)"), start+1 );
			if ( end != -1 ) {
			    parray.setPoint( i, pointFromString( val.mid(idx, end-idx+1) ) );
			}
			else
			    parray.setPoint( i, QPoint() );
		    } else {
			parray.setPoint( i, QPoint() );
			break;
		    }
		    idx = end+2;
		}
		return parray;
	    }
	default:
	case QVariant::Invalid:
	    return QVariant();
	}
    }
    return QVariant();
}

bool QPSQLResult::isNull( int field ) const
{
    PQgetvalue( d->result, at(), field );
    return PQgetisnull( d->result, at(), field );
}

bool QPSQLResult::reset ( const QString& query )
{
    cleanup();
    if ( !driver() )
        return FALSE;
    if ( !driver()-> isOpen() || driver()->isOpenError() )
        return FALSE;
    setActive( FALSE );
    setAt( BeforeFirst );
    if ( d->result )
    	PQclear( d->result );
    d->result = PQexec( d->connection, (const char*)query );
    int status =  PQresultStatus( d->result );
    if ( status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK ) {
	currentSize = PQntuples( d->result );
	setActive( TRUE );
	return TRUE;
    }
    setLastError( makeError( "Unable to create query", QSqlError::Statement, d ) );
    return FALSE;
}

///////////////////////////////////////////////////////////////////

QPSQLResultInfo::QPSQLResultInfo( QPSQLPrivate* p )
{
    int count = PQnfields ( p->result );
    for ( int i = 0; i < count; ++i ) {
	appendField( makeFieldInfo( p, i ) );
    }
    setSize( PQntuples( p->result ) );
}

QPSQLResultInfo::~QPSQLResultInfo()
{
}

///////////////////////////////////////////////////////////////////

QPSQLDriver::QPSQLDriver( QObject * parent, const char * name )
: QSqlDriver(parent,name ? name : "QPSQL")
{
    init();
}

void QPSQLDriver::init()
{
    setTransactionSupport( TRUE );
    d = new QPSQLPrivate();
}

QPSQLDriver::~QPSQLDriver()
{
    if ( d->connection )
	PQfinish( d->connection );
    delete d;
}

bool QPSQLDriver::open( const QString & db,
    			const QString & user,
			const QString & password,
			const QString & host)
{
    if ( isOpen() )
        close();
    QString connectString;
    if ( host.length() )
	connectString += QString("host=%1 ").arg( host );
    if ( db.length() )
	connectString += QString("dbname=%1 ").arg( db );
    if ( user.length() )
	connectString += QString("user=%1 ").arg( user );
    if ( password.length() )
	connectString += QString("password=%1 ").arg( password );
    d->connection = PQconnectdb( connectString.local8Bit().data() );
    if ( PQstatus( d->connection) == CONNECTION_BAD ) {
	setLastError( makeError("Unable to connect", QSqlError::Connection, d ) );
        setOpenError( TRUE );
        return FALSE;
    }
    PGresult* dateResult = PQexec( d->connection, "SET DATESTYLE=ISO;" );
#ifdef CHECK_RANGE
    int status =  PQresultStatus( dateResult );
    if ( status == PGRES_COMMAND_OK )
	qWarning( PQerrorMessage( d->connection ) );
#endif
    setOpen( TRUE );
    return TRUE;
}

void QPSQLDriver::close()
{
    if ( isOpen() ) {
	PQfinish( d->connection );
	d->connection = 0;
	setOpen( FALSE );
	setOpenError( FALSE );
    }
}

QSql QPSQLDriver::createResult() const
{
    return QSql( new QPSQLResult( this, d ) );
}

bool QPSQLDriver::beginTransaction()
{
    PGresult* res = PQexec( d->connection, "BEGIN" );
    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	PQclear( res );
	setLastError( makeError( "Could not being transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    PQclear( res );
    return TRUE;
}

bool QPSQLDriver::commitTransaction()
{
    PGresult* res = PQexec( d->connection, "COMMIT" );
    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	PQclear( res );
	setLastError( makeError( "Could not commit transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    PQclear( res );
    return TRUE;
}

bool QPSQLDriver::rollbackTransaction()
{
    PGresult* res = PQexec( d->connection, "ROLLBACK" );
    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK ) {
    	setLastError( makeError( "Could not rollback transaction", QSqlError::Transaction, d ) );
	PQclear( res );
	return FALSE;
    }
    PQclear( res );
    return TRUE;
}

QStringList QPSQLDriver::tables() const
{
    QSql t = createResult();
    t << "select tablename from pg_tables;";
    QStringList tl;
    while ( t.next() )
	tl.append( t[0].toString() );
    return tl;
}
