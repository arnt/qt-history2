#include "qsql_psql.h"

#include <qregexp.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qpointarray.h>
#include <postgres.h>
#include <libpq-fe.h>
#include <catalog/pg_type.h>
#include <utils/geo_decls.h>
#include <utils/timestamp.h>
#include <math.h>

class QPSQLPrivate
{
public:
  QPSQLPrivate():connection(0), result(0){}
    PGconn 	*connection;
    PGresult 	*result;
};

QSqlError qMakeError( const QString& err, int type, const QPSQLPrivate* p )
{
    return QSqlError("QPSQL: " + err, QString(PQerrorMessage( p->connection )), type);
}

QVariant::Type qDecodePSQLType( int t )
{
    QVariant::Type type = QVariant::Invalid;
    switch ( t ) {
    case BOOLOID	:
	type = QVariant::Bool;
	break;
    case INT8OID	:
    case INT2OID	:
    case INT2VECTOROID  :
    case INT4OID        :
    case OIDOID         :
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
    return type;
}

QVariant::Type qFieldType( QPSQLPrivate* p, int i )
{
    QVariant::Type type = qDecodePSQLType( PQftype( p->result, i ) );
    return type;
}

bool qIsPrimaryIndex( const QSqlDriver* driver, const QString& tablename, const QString& fieldname )
{
    bool ispIdx = FALSE;
    QString pIdx( "select count(1) "
		  "from pg_attribute a, pg_class c1, pg_class c2, pg_index x "
		  "where c1.relname='%1' "
		  "and a.attname='%2' "
		  "and a.attnum > 0 "
		  "and c1.oid=x.indrelid "
		  "and c2.oid=a.attrelid "
		  "and (x.indexrelid=c2.oid "
		  "and a.attrelid=c2.oid);");
    QSql pIdxs = driver->createResult();
    pIdxs.setQuery( pIdx.arg( tablename ).arg( fieldname ) );
    if ( pIdxs.next() )
	ispIdx = pIdxs.value(0).toInt();
    return ispIdx;
}

QSqlField qMakeField( const QSqlDriver* driver, const QString& tablename, const QString& fieldname )
{
    QString stmt ( "select a.atttypid "
		   "from pg_user u, pg_class c, pg_attribute a, pg_type t "
		   "where c.relname = '%1' "
		   "and a.attname = '%2' "
		   "and int4out(u.usesysid) = int4out(c.relowner) "
		   "and c.oid= a.attrelid "
		   "and a.atttypid = t.oid "
		   "and (a.attnum > 0);");
    QSql fi = driver->createResult();
    fi.setQuery( stmt.arg( tablename ).arg( fieldname ) );
    if ( fi.next() ) {
	QSqlField f( fieldname, 0, qDecodePSQLType( fi.value(0).toInt()) );
	f.setPrimaryIndex( qIsPrimaryIndex( driver, tablename, fieldname ) );
	return f;
    }
    return QSqlField();
}

QPSQLResult::QPSQLResult( const QPSQLDriver* db, const QPSQLPrivate* p )
: QSqlResult( db ),
  currentSize( 0 ),
  binary(FALSE)
{
    d =   new QPSQLPrivate();
    (*d) = (*p);
}

QPSQLResult::~QPSQLResult()
{
    cleanup();
    delete d;
}

void QPSQLResult::cleanup()
{
    if ( isActive() ) {
	if ( d->result )
	    PQclear( d->result );
        d->result = 0;
        setAt( -1 );
    }
    currentSize = 0;
    binary = FALSE;
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

// some Postgres conversions
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

QDate qDateFromUInt( uint dt )
{
    int y,m,d;
    QDate::jul2greg( dt, y, m, d );
    return QDate( y, m, d );
}

QTime qTimeFromDouble( double tm )
{
    int hour = ((int)tm / ( 60 * 60 ) );
    int min = (((int) (tm / 60)) % 60 );
    int sec = (((int) tm) % 60 );
    return QTime( hour, min, sec );
}

QVariant QPSQLResult::data( int i )
{
    QVariant::Type type = qFieldType( d, i );
    if ( binary ) {
	char* rawdata = PQgetvalue( d->result, at(), i );
	int rawsize = PQfsize( d->result, i );
	switch ( type ) {
	case QVariant::Bool:
	    { // ###
		QString bs( rawdata );
		bool b = ( bs == "t" );
		return QVariant( b );
	    }
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
		uint dt = *((uint*)rawdata) + QDate::greg2jul(2000,1,1);
 		return QVariant( qDateFromUInt( dt ) );
	    }
	case QVariant::Time:
	    {
		double tm = *((double*)rawdata);
		return QVariant( qTimeFromDouble( tm ) );
	    }
	case QVariant::DateTime:
	    {
		Timestamp* ts = (Timestamp*)rawdata;
		double time, date;
		double ds = (double)(24*60*60);
		time = *ts;
		date = ( time < 0 ) ? ceil( time / ds ) : floor ( time / ds );
		if ( date != 0 )
		    time -= rint( date * ds );
		if ( time < 0 ) {
		    time += ds;
		    date -= 1;
		}
		uint dt = (uint)date + QDate::greg2jul(2000,1,1);
		return QVariant( QDateTime( qDateFromUInt( dt ), qTimeFromDouble( time ) ) );
	    }
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
#ifdef CHECK_RANGE
	    qWarning("QPSQLResult::data Warning: unknown data type");
#endif
	    return QVariant();
	}
    } else {
	QString val( PQgetvalue( d->result, at(), i ) );
	switch ( type ) {
	case QVariant::Bool:
	    {
	    QVariant b ( (bool)(val == "t"), 0 );
	    return ( b );
	    }
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
#ifdef CHECK_RANGE
	    qWarning("QPSQLResult::data Warning: unknown data type");
#endif
	    return QVariant();
	}
    }
    return QVariant();
}

bool QPSQLResult::isNull( int field )
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
	binary = PQbinaryTuples( d->result );
	setActive( TRUE );
	return TRUE;
    }
    setLastError( qMakeError( "Unable to create query", QSqlError::Statement, d ) );
    return FALSE;
}

QSqlFieldList QPSQLResult::fields()
{
    QSqlFieldList fil;
    int count = PQnfields ( d->result );
    for ( int i = 0; i < count; ++i ) {
	QString name = PQfname( d->result, i );
	QVariant::Type type = qDecodePSQLType( PQftype( d->result, i ) );
	QSqlField rf( name, i, type );
	if ( isActive() && isValid() )
	    rf.setValue( data( i ) );
	fil.append( rf );
    }
    return fil;
}

int QPSQLResult::size()
{
    return currentSize;
}

int QPSQLResult::affectedRows()
{
    return QString( PQcmdTuples( d->result ) ).toInt();
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
    setQuerySizeSupport( TRUE );
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
	setLastError( qMakeError("Unable to connect", QSqlError::Connection, d ) );
        setOpenError( TRUE );
        return FALSE;
    }
    PGresult* dateResult = PQexec( d->connection, "SET DATESTYLE=ISO;" );
#ifdef CHECK_RANGE
    int status =  PQresultStatus( dateResult );
    if ( status != PGRES_COMMAND_OK )
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
	setLastError( qMakeError( "Could not being transaction", QSqlError::Transaction, d ) );
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
	setLastError( qMakeError( "Could not commit transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    PQclear( res );
    return TRUE;
}

bool QPSQLDriver::rollbackTransaction()
{
    PGresult* res = PQexec( d->connection, "ROLLBACK" );
    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK ) {
    	setLastError( qMakeError( "Could not rollback transaction", QSqlError::Transaction, d ) );
	PQclear( res );
	return FALSE;
    }
    PQclear( res );
    return TRUE;
}

QStringList QPSQLDriver::tables( const QString& user ) const
{
    QSql t = createResult();
    QString stmt( "select relname from pg_class, pg_user "
		  "where usename like '%1'"
		  "and relkind = 'r' "
		  "and int4out(usesysid) = int4out(relowner) "
		  "order by relname;" );
    t.setQuery( stmt.arg( user ) );
    QStringList tl;
    while ( t.isActive() && t.next() )
	tl.append( t.value(0).toString() );
    return tl;
}

QSqlIndex QPSQLDriver::primaryIndex( const QString& tablename ) const
{
    QSqlIndex idx;
    QSql i = createResult();
    QString stmt( "select a.attname from pg_attribute a, pg_class c1,"
		  "pg_class c2, pg_index i where c1.relname = '%1' "
		  "and c1.oid = i.indrelid and i.indexrelid = c2.oid "
		  "and a.attrelid = c2.oid;");
    i.setQuery( stmt.arg( tablename ) );
    while ( i.isActive() && i.next() ) {
	QSqlField f = qMakeField( this, tablename,  i.value(0).toString() );
	f.setFieldNumber( i.at() );
	idx.append( f );
    }
    return idx;
}

QSqlFieldList QPSQLDriver::fields( const QString& tablename ) const
{
    QSqlFieldList fil;
    QString stmt ( "select a.attname "
		   "from pg_user u, pg_class c, pg_attribute a, pg_type t "
		   "where c.relname = '%1' "
		   "and int4out(u.usesysid) = int4out(c.relowner) "
		   "and c.oid= a.attrelid "
		   "and a.atttypid = t.oid "
		   "and (a.attnum > 0);");
    QSql fi = createResult();
    fi.setQuery( stmt.arg( tablename ) );
    while ( fi.next() ) {
	QSqlField f = qMakeField( this, tablename, fi.value(0).toString() );
	f.setFieldNumber( fi.at() );
	fil.append( f );
    }
    return fil;
}
