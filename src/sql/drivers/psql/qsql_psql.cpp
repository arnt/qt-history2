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

#include <qsqlrecord.h>
#include <qregexp.h>
#include <qdatetime.h>
#include <qpointarray.h>
#include <postgres.h>
#include <libpq/libpq-fs.h>
#include <catalog/pg_type.h>
#ifndef Q_WS_MAC
#include <utils/geo_decls.h>
#endif
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
#ifdef DATETIMEOID
    // Postgres 6.x datetime workaround.
    // DATETIMEOID == TIMESTAMPOID (only the names have changed)
    case DATETIMEOID    :
#endif
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
    case CASHOID        :
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
    if ( i < 0 )
	return FALSE;
    if ( i >= currentSize )
	return FALSE;
    if ( at() == i )
	return TRUE;
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
    QDate::julianToGregorian( dt, y, m, d );
    return QDate( y, m, d );
}

/* // ### this should be obsolete?
QTime qTimeFromDouble( double tm )
{
    int hour = ((int)tm / ( 60 * 60 ) );
    int min = (((int) (tm / 60)) % 60 );
    int sec = (((int) tm) % 60 );
    return QTime( hour, min, sec );
}
*/

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
	if ( val.length() < 10 )
	    return QVariant( QDateTime() );
	// remove the timezone
	if ( val.find( "+", val.length() - 3 ) >= 0 )
	    val = val.left( val.length() - 3 );
	// for some reasons the milliseconds are sometimes only 2 digits
	// (12 instead of 120)
	if ( val.find( ".", val.length() - 3 ) >= 0 )
	    val += "0";
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
	char * buf = new char[ size ];

#ifdef Q_OS_WIN32
	// ### For some reason lo_read() fails if we try to read more than
	// ### 32760 bytes
	char * p = buf;
	int nread = 0;

	while( size < nread ){
		retval = lo_read( d->connection, fd, p, 32760 );
		nread += retval;
		p += retval;
	}
#else
	retval = lo_read( d->connection, fd, buf, size );
#endif

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
	;
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

QPSQLDriver::QPSQLDriver( QObject * parent, const char * name )
    : QSqlDriver(parent,name ? name : "QPSQL"), pro( QPSQLDriver::Version6 )
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


bool QPSQLDriver::hasFeature( DriverFeature f ) const
{
    switch ( f ) {
    case Transactions:
	return TRUE;
    case QuerySize:
	return TRUE;
    case BLOB:
	return FALSE;
    default:
	return FALSE;
    }
}

static QPSQLDriver::Protocol getPSQLVersion( PGconn* connection )
{
    PGresult* result = PQexec( connection, "select version()" );
    int status =  PQresultStatus( result );
    if ( status == PGRES_COMMAND_OK || status == PGRES_TUPLES_OK ) {
	QString val( PQgetvalue( result, 0, 0 ) );
	PQclear( result );	
	QRegExp rx( "(\\d*)\\.(\\d*)" );
	rx.setMinimal ( TRUE ); // enforce non-greedy RegExp
        if ( rx.search( val ) != -1 ) {
	    int vMaj = rx.cap( 1 ).toInt();
	    int vMin = rx.cap( 2 ).toInt();
	    if ( vMaj < 6 ) {
#ifdef QT_CHECK_RANGE
		qWarning( "This version of PostgreSQL is not supported and may not work." );
#endif
		return QPSQLDriver::Version6;
	    }
	    if ( vMaj == 6 ) {
		return QPSQLDriver::Version6;
	    }
	    if ( vMaj == 7 ) {
		if ( vMin < 1 ) {
		    return QPSQLDriver::Version7;
		} else {
		    return QPSQLDriver::Version71;
		}
	    }
	    if ( vMaj > 7 ) {
		return QPSQLDriver::Version71;
	    }
	}
    } else {
#ifdef QT_CHECK_RANGE
	qWarning( "This version of PostgreSQL is not supported and may not work." );
#endif
    }

    return QPSQLDriver::Version6;
}

bool QPSQLDriver::open( const QString & db,
			const QString & user,
			const QString & password,
			const QString & host,
			int port )
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
    if ( port > -1 )
	connectString += QString("port=%1 ").arg( port );
    d->connection = PQconnectdb( connectString.local8Bit().data() );
    if ( PQstatus( d->connection ) == CONNECTION_BAD ) {
	setLastError( qMakeError("Unable to connect", QSqlError::Connection, d ) );
	setOpenError( TRUE );
	return FALSE;
    }

    pro = getPSQLVersion( d->connection );

    PGresult* dateResult = 0;
    switch( pro ) {
    case QPSQLDriver::Version6:
	dateResult = PQexec( d->connection, "SET DATESTYLE TO 'ISO'" );
	break;
    case QPSQLDriver::Version7:
    case QPSQLDriver::Version71:
	dateResult = PQexec( d->connection, "SET DATESTYLE=ISO" );
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
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QPSQLDriver::beginTransaction: Database not open" );
#endif
	return FALSE;
    }
    PGresult* res = PQexec( d->connection, "BEGIN" );
    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	PQclear( res );
	setLastError( qMakeError( "Could not begin transaction", QSqlError::Transaction, d ) );
	return FALSE;
    }
    PQclear( res );
    return TRUE;
}

bool QPSQLDriver::commitTransaction()
{
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QPSQLDriver::commitTransaction: Database not open" );
#endif
	return FALSE;
    }
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
    if ( !isOpen() ) {
#ifdef QT_CHECK_RANGE
	qWarning( "QPSQLDriver::rollbackTransaction: Database not open" );
#endif
	return FALSE;
    }
    PGresult* res = PQexec( d->connection, "ROLLBACK" );
    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK ) {
	setLastError( qMakeError( "Could not rollback transaction", QSqlError::Transaction, d ) );
	PQclear( res );
	return FALSE;
    }
    PQclear( res );
    return TRUE;
}

QStringList QPSQLDriver::tables( const QString& /* user */ ) const
{
    QStringList tl;
    if ( !isOpen() )
	return tl;
    QSqlQuery t = createQuery();
    QString stmt;
    stmt = "select relname from pg_class where ( relkind = 'r' or relkind = 'v' ) "
		"and ( relname !~ '^Inv' ) "
		"and ( relname !~ '^pg_' ) ";
    t.exec( stmt );
    while ( t.isActive() && t.next() )
	tl.append( t.value(0).toString() );
    return tl;
}

QSqlIndex QPSQLDriver::primaryIndex( const QString& tablename ) const
{
    QSqlIndex idx( tablename );
    if ( !isOpen() )
	return idx;
    QSqlQuery i = createQuery();
    QString stmt;

    switch( pro ) {
    case QPSQLDriver::Version6:
	stmt = "select pg_att1.attname, int(pg_att1.atttypid), pg_att2.attnum, pg_cl.relname "
		"from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
		"where pg_cl.relname = '%1_pkey' AND pg_cl.oid = pg_ind.indexrelid "
		"and pg_att2.attrelid = pg_ind.indexrelid "
		"and pg_att1.attrelid = pg_ind.indrelid "
		"and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
		"order by pg_att2.attnum";
	break;
    case QPSQLDriver::Version7:
    case QPSQLDriver::Version71:
	stmt = "select pg_att1.attname, pg_att1.atttypid::int, pg_cl.relname "
		"from pg_attribute pg_att1, pg_attribute pg_att2, pg_class pg_cl, pg_index pg_ind "
		"where pg_cl.relname = '%1_pkey' AND pg_cl.oid = pg_ind.indexrelid "
		"and pg_att2.attrelid = pg_ind.indexrelid "
		"and pg_att1.attrelid = pg_ind.indrelid "
		"and pg_att1.attnum = pg_ind.indkey[pg_att2.attnum-1] "
		"order by pg_att2.attnum";
	break;
    }
    i.exec( stmt.arg( tablename ) );
    while ( i.isActive() && i.next() ) {
	QSqlField f( i.value(0).toString(), qDecodePSQLType( i.value(1).toInt() ) );
	idx.append( f );
	idx.setName( i.value(2).toString() );
    }
    return idx;
}

QSqlRecord QPSQLDriver::record( const QString& tablename ) const
{
    QSqlRecord fil;
    if ( !isOpen() )
	return fil;
    QString stmt;
    switch( pro ) {
    case QPSQLDriver::Version6:
	stmt = "select pg_attribute.attname, int(pg_attribute.atttypid) "
			"from pg_class, pg_attribute "
			"where pg_class.relname = '%1' "
			"and pg_attribute.attnum > 0 "
			"and pg_attribute.attrelid = pg_class.oid ";
	break;
    case QPSQLDriver::Version7:
    case QPSQLDriver::Version71:
	stmt = "select pg_attribute.attname, pg_attribute.atttypid::int "
			"from pg_class, pg_attribute "
			"where pg_class.relname = '%1' "
			"and pg_attribute.attnum > 0 "
			"and pg_attribute.attrelid = pg_class.oid ";
	break;
    }

    QSqlQuery fi = createQuery();
    fi.exec( stmt.arg( tablename ) );
    while ( fi.next() ) {
	QSqlField f( fi.value(0).toString(), qDecodePSQLType( fi.value(1).toInt() ) );
	fil.append( f );
    }
    return fil;
}

QSqlRecord QPSQLDriver::record( const QSqlQuery& query ) const
{
    QSqlRecord fil;
    if ( !isOpen() )
	return fil;
    if ( query.isActive() && query.driver() == this ) {
	QPSQLResult* result = (QPSQLResult*)query.result();
	int count = PQnfields( result->d->result );
	for ( int i = 0; i < count; ++i ) {
	    QString name = PQfname( result->d->result, i );
	    QVariant::Type type = qDecodePSQLType( PQftype( result->d->result, i ) );
	    QSqlField rf( name, type );
	    fil.append( rf );
	}
    }
    return fil;
}

QSqlRecordInfo QPSQLDriver::recordInfo( const QString& tablename ) const
{
    QSqlRecordInfo info;
    if ( !isOpen() )
	return info;

    QString stmt;
    switch( pro ) {
    case QPSQLDriver::Version6:
	stmt = "select pg_attribute.attname, int(pg_attribute.atttypid), pg_attribute.attnotnull, "
		"pg_attribute.attlen, pg_attribute.atttypmod, int(pg_attribute.attrelid), pg_attribute.attnum "
		"from pg_class, pg_attribute "
		"where pg_class.relname = '%1' "
		"and pg_attribute.attnum > 0 "
		"and pg_attribute.attrelid = pg_class.oid ";
	break;
    case QPSQLDriver::Version7:
	stmt = "select pg_attribute.attname, pg_attribute.atttypid::int, pg_attribute.attnotnull, "
		"pg_attribute.attlen, pg_attribute.atttypmod, pg_attribute.attrelid::int, pg_attribute.attnum "
		"from pg_class, pg_attribute "
		"where pg_class.relname = '%1' "
		"and pg_attribute.attnum > 0 "
		"and pg_attribute.attrelid = pg_class.oid ";
	break;
    case QPSQLDriver::Version71:
	stmt = "select pg_attribute.attname, pg_attribute.atttypid::int, pg_attribute.attnotnull, "
		"pg_attribute.attlen, pg_attribute.atttypmod, pg_attrdef.adsrc "
		"from pg_class, pg_attribute "
		"left join pg_attrdef on (pg_attrdef.adrelid = pg_attribute.attrelid and pg_attrdef.adnum = pg_attribute.attnum) "
		"where pg_class.relname = '%1' "
		"and pg_attribute.attnum > 0 "
		"and pg_attribute.attrelid = pg_class.oid ";
	break;
    }

    QSqlQuery query = createQuery();
    query.exec( stmt.arg( tablename ) );
    if ( pro == QPSQLDriver::Version71 ) {
	while ( query.next() ) {
	    int len = query.value( 3 ).toInt();
	    int precision = query.value( 4 ).toInt();
	    // swap length and precision if length == -1
	    if ( len == -1 && precision > -1 ) {
		len = precision - 4;
		precision = -1;
	    }
	    QString defVal = query.value( 5 ).toString();
	    if ( !defVal.isEmpty() && defVal.startsWith( "'" ) )
		defVal = defVal.mid( 1, defVal.length() - 2 );
	    info.append( QSqlFieldInfo( query.value( 0 ).toString(),
					qDecodePSQLType( query.value( 1 ).toInt() ),
					query.value( 2 ).toBool(),
					len,
					precision,
					defVal,
					query.value( 1 ).toInt() ) );
	}
    } else {
	// Postgres < 7.1 cannot handle outer joins
	while ( query.next() ) {
	    QString defVal;
	    QString stmt2 = ( "select pg_attrdef.adsrc from pg_attrdef where "
				"pg_attrdef.adrelid = %1 and pg_attrdef.adnum = %2 " );
	    QSqlQuery query2 = createQuery();
	    query2.exec( stmt2.arg( query.value( 5 ).toInt() ).arg( query.value( 6 ).toInt() ) );
	    if ( query2.isActive() && query2.next() )
		defVal = query2.value( 0 ).toString();
	    if ( !defVal.isEmpty() && defVal.startsWith( "'" ) )
		defVal = defVal.mid( 1, defVal.length() - 2 );
	    int len = query.value( 3 ).toInt();
	    int precision = query.value( 4 ).toInt();
	    // swap length and precision if length == -1
	    if ( len == -1 && precision > -1 ) {
		len = precision - 4;
		precision = -1;
	    }
	    info.append( QSqlFieldInfo( query.value( 0 ).toString(),
					qDecodePSQLType( query.value( 1 ).toInt() ),
					query.value( 2 ).toBool(),
					len,
					precision,
					defVal,
					query.value( 1 ).toInt() ) );
	}
    }

    return info;
}

QSqlRecordInfo QPSQLDriver::recordInfo( const QSqlQuery& query ) const
{
    QSqlRecordInfo info;
    if ( !isOpen() )
	return info;
    if ( query.isActive() && query.driver() == this ) {
	QPSQLResult* result = (QPSQLResult*)query.result();
	int count = PQnfields( result->d->result );
	for ( int i = 0; i < count; ++i ) {
	    QString name = PQfname( result->d->result, i );
	    int len = PQfsize( result->d->result, i );
	    int precision = PQfmod( result->d->result, i );
	    // swap length and precision if length == -1
	    if ( len == -1 && precision > -1 ) {
		len = precision - 4;
		precision = -1;
	    }
	    info.append( QSqlFieldInfo( name,
					qDecodePSQLType( PQftype( result->d->result, i ) ),
					-1,
					len,
					precision,
					QVariant(),
					PQftype( result->d->result, i ) ) );
	}
    }
    return info;
}

QString QPSQLDriver::formatValue( const QSqlField* field,
				  bool ) const
{
    QString r;
    if ( field->isNull() ) {
	r = nullText();
    } else {
	switch ( field->type() ) {
	case QVariant::DateTime:
	    if ( field->value().toDateTime().isValid() ) {
		QDate dt = field->value().toDateTime().date();
		QTime tm = field->value().toDateTime().time();
		r = "'" + QString::number(dt.year()) + "-" +
			  QString::number(dt.month()) + "-" +
			  QString::number(dt.day()) + " " +
			  tm.toString() + "." +
			  QString::number(tm.msec()) + "'";
	    } else {
		r = nullText();
	    }
	    break;
	case QVariant::String:
	case QVariant::CString: {
	    // Escape '\' characters
	    r = QSqlDriver::formatValue( field );
	    r.replace( QRegExp( "\\\\" ), "\\\\" );
	    break;
	}
	case QVariant::Bool:
	    if ( field->value().toBool() )
		r = "TRUE";
	    else
		r = "FALSE";
	    break;
	case QVariant::ByteArray:
#ifdef QT_CHECKRANGE
	    // bytearrays cannot be inserted directly into postgresql
	    qWarning( "QPSQLDriver::formatValue: cannot format ByteArray." );
#endif
	    return QString();
	    break;
	default:
	    r = QSqlDriver::formatValue( field );
	    break;
	}
    }
    return r;
}
