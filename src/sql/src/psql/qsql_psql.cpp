/****************************************************************************
**
** Implementation of PostgreSQL driver classes
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

#include "qsql_psql.h"

#include <qregexp.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qpointarray.h>
#include <postgres.h>
#include <libpq/libpq-fs.h>
#include <catalog/pg_type.h>
#include <utils/geo_decls.h>
#include <math.h>

class QPSQLPrivate
{
public:
  QPSQLPrivate():connection(0), result(0){}
    PGconn	*connection;
    PGresult	*result;
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
	//    case INT2VECTOROID  : // 7.x
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
	//    case TIMETZOID      : // 7.x
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
	//    case ZPBITOID	: // 7.x
	//    case VARBITOID	: // 7.x
    case OIDOID         :
	type = QVariant::ByteArray;
	break;
    case REGPROCOID     :
    case TIDOID         :
    case XIDOID         :
    case CIDOID         :
	//    case OIDVECTOROID   : // 7.x
    case UNKNOWNOID     :
    case CASHOID        : // deprecated
    case INETOID        :
    case CIDROID        :
	//    case TINTERVALOID   : // 7.x
    case CIRCLEOID      :
    case PATHOID        :
    case LSEGOID        :
    case LINEOID        :
	type = QVariant::Invalid;
	break;
    default:
    case CHAROID	:
    case BPCHAROID	:
	//    case LZTEXTOID	: // 7.x
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

QSqlField qMakeField( QPSQLDriver::Protocol protocol, const QSqlDriver* driver, const QString& tablename, const QString& fieldname )
{
    QString stmt;
    switch( protocol ) {
    case QPSQLDriver::Version6:
	stmt = "select int(a.atttypid) "
		   "from pg_user u, pg_class c, pg_attribute a, pg_type t "
		   "where c.relname = '%1' "
		   "and a.attname = '%2' "
		   "and int4out(u.usesysid) = int4out(c.relowner) "
		   "and c.oid= a.attrelid "
		   "and a.atttypid = t.oid "
		   "and (a.attnum > 0);";
	break;
    case QPSQLDriver::Version7:
	stmt = "select a.atttypid::int "
		   "from pg_user u, pg_class c, pg_attribute a, pg_type t "
		   "where c.relname = '%1' "
		   "and a.attname = '%2' "
		   "and int4out(u.usesysid) = int4out(c.relowner) "
		   "and c.oid= a.attrelid "
		   "and a.atttypid = t.oid "
		   "and (a.attnum > 0);";
	break;
    }
    QSqlQuery fi = driver->createQuery();
    fi.exec( stmt.arg( tablename ).arg( fieldname ) );
    if ( fi.next() ) {
	QSqlField f( fieldname, qDecodePSQLType( fi.value(0).toInt()) );
	return f;
    }
    return QSqlField();
}

QPSQLResult::QPSQLResult( const QPSQLDriver* db, const QPSQLPrivate* p )
: QSqlResult( db ),
  currentSize( 0 )
{
    d =   new QPSQLPrivate();
    (*d) = (*p);
}

QPSQLResult::~QPSQLResult()
{
    cleanup();
    delete d;
}

PGresult* QPSQLResult::result()
{
    return d->result;
}

void QPSQLResult::cleanup()
{
    if ( d->result )
	PQclear( d->result );
    d->result = 0;
    setAt( -1 );
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
	    return QVariant( parray );
	}
    case QVariant::ByteArray: {
	QByteArray ba;
	((QSqlDriver*)driver())->beginTransaction();
	Oid oid = val.toInt();
	int fd = lo_open( d->connection, oid, INV_READ );
#ifdef QT_CHECK_RANGE
	if ( fd < 0) {
	    qWarning( "QPSQLResult::data: unable to open large object for read" );
	    ((QSqlDriver*)driver())->commitTransaction();
	    return QVariant( ba );
	}
#endif
	int size = 0;
	int retval = lo_lseek( d->connection, fd, 0L, SEEK_END );
	if ( retval >= 0 ) {
	    size = lo_tell( d->connection, fd );
	    lo_lseek( d->connection, fd, 0L, SEEK_SET );
	}
	if ( size == 0 ) {
	    lo_close( d->connection, fd );
	    ((QSqlDriver*)driver())->commitTransaction();
	    return QVariant( ba );
	}
	char* buf;
	buf = new char[ size ];
	retval = lo_read( d->connection, fd, buf, size );
	if (retval < 0) {
	    qWarning( "QPSQLResult::data: unable to read large object" );
	} else {
	    ba.duplicate( buf, size );
	}
	delete [] buf;
	lo_close( d->connection, fd );
	((QSqlDriver*)driver())->commitTransaction();
	return QVariant( ba );
    }
    default:
    case QVariant::Invalid:
#ifdef QT_CHECK_RANGE
	qWarning("QPSQLResult::data: unknown data type");
#endif
	return QVariant();
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
    setAt( QSql::BeforeFirst );
    if ( d->result )
	PQclear( d->result );
    d->result = PQexec( d->connection, (const char*)query );
    int status =  PQresultStatus( d->result );
    if ( status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK ) {
	setSelect( (status == PGRES_TUPLES_OK) );
	currentSize = PQntuples( d->result );
	setActive( TRUE );
	return TRUE;
    }
    setLastError( qMakeError( "Unable to create query", QSqlError::Statement, d ) );
    return FALSE;
}

int QPSQLResult::size()
{
    return currentSize;
}

int QPSQLResult::numRowsAffected()
{
    return QString( PQcmdTuples( d->result ) ).toInt();
}

///////////////////////////////////////////////////////////////////

QPSQLDriver::QPSQLDriver( Protocol protocol, QObject * parent, const char * name )
    : QSqlDriver(parent,name ? name : "QPSQL"),
      pro( protocol )
{
    init();
}

void QPSQLDriver::init()
{
    d = new QPSQLPrivate();
}

QPSQLDriver::~QPSQLDriver()
{
    if ( d->connection )
	PQfinish( d->connection );
    delete d;
}

PGconn* QPSQLDriver::connection()
{
    return d->connection;
}

bool QPSQLDriver::hasTransactionSupport() const
{
    return TRUE;
}

bool QPSQLDriver::hasQuerySizeSupport() const
{
    return TRUE;
}

bool QPSQLDriver::canEditBinaryFields() const
{
    return FALSE;
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
    PGresult* dateResult;
    switch( pro ) {
    case QPSQLDriver::Version6:
	dateResult = PQexec( d->connection, "SET DATESTYLE TO 'ISO';" );
	break;
    case QPSQLDriver::Version7:
	dateResult = PQexec( d->connection, "SET DATESTYLE=ISO;" );
	break;
    }
#ifdef QT_CHECK_RANGE
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

QSqlQuery QPSQLDriver::createQuery() const
{
    return QSqlQuery( new QPSQLResult( this, d ) );
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
    QSqlQuery t = createQuery();
    QString stmt;
    switch( pro ) {
    case QPSQLDriver::Version6:
    case QPSQLDriver::Version7:
	stmt = "select relname from pg_class, pg_user "
		  "where usename like '%1'"
		  "and relkind = 'r' "
		  "and int4out(usesysid) = int4out(relowner) "
		  "order by relname;";
	break;
    }
    t.exec( stmt.arg( user ) );
    QStringList tl;
    while ( t.isActive() && t.next() )
	tl.append( t.value(0).toString() );
    return tl;
}

QSqlIndex QPSQLDriver::primaryIndex( const QString& tablename ) const
{
    QSqlIndex idx( tablename );
    QSqlQuery i = createQuery();
    QString stmt;
    switch( pro ) {
    case QPSQLDriver::Version6:
    case QPSQLDriver::Version7:
	stmt = "select a.attname, c2.relname from pg_attribute a, pg_class c1,"
		  "pg_class c2, pg_index i where c1.relname = '%1' "
		  "and c1.oid = i.indrelid and i.indexrelid = c2.oid "
		  "and a.attrelid = c2.oid;";
	break;
    }
    i.exec( stmt.arg( tablename ) );
    while ( i.isActive() && i.next() ) {
	QSqlField f = qMakeField( pro, this, tablename,  i.value(0).toString() );
	idx.append( f );
	idx.setName( i.value(1).toString() );
    }
    return idx;
}

QSqlRecord QPSQLDriver::record( const QString& tablename ) const
{
    QSqlRecord fil;
    QString stmt;
    switch( pro ) {
    case QPSQLDriver::Version6:
    case QPSQLDriver::Version7:
	stmt = "select a.attname "
	       "from pg_user u, pg_class c, pg_attribute a, pg_type t "
	       "where c.relname = '%1' "
	       "and int4out(u.usesysid) = int4out(c.relowner) "
	       "and c.oid= a.attrelid "
	       "and a.atttypid = t.oid "
	       "and (a.attnum > 0);";
	break;
    }
    QSqlQuery fi = createQuery();
    fi.exec( stmt.arg( tablename ) );
    while ( fi.next() ) {
	QSqlField f = qMakeField( pro, this, tablename, fi.value(0).toString() );
	fil.append( f );
    }
    return fil;
}

QSqlRecord QPSQLDriver::record( const QSqlQuery& query ) const
{
    QSqlRecord fil;
    if ( query.isActive() && query.driver() == this ) {
	QPSQLResult* result = (QPSQLResult*)query.result();
	int count = PQnfields ( result->d->result );
	for ( int i = 0; i < count; ++i ) {
	    QString name = PQfname( result->d->result, i );
	    QVariant::Type type = qDecodePSQLType( PQftype( result->d->result, i ) );
	    QSqlField rf( name, type );
	    fil.append( rf );
	}
    }
    return fil;
}

QString QPSQLDriver::formatValue( const QSqlField* field,
				  bool ) const
{
    QString r;
    if ( field->isNull() )
	r = nullText();
    else if ( field->type() == QVariant::DateTime ) {
	if ( field->value().toDateTime().isValid() ){
	    QDate dt = field->value().toDateTime().date();
	    QTime tm = field->value().toDateTime().time();
	    r = "'" + QString::number(dt.year()) + "-" +
		QString::number(dt.month()) + "-" +
		QString::number(dt.day()) + " " +
		tm.toString() + "'";
	} else
	    r = nullText();
    } else {
	r = QSqlDriver::formatValue( field );
    }
    return r;
}
