/****************************************************************************
**
** Implementation of MYSQL driver classes
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

#include "qsql_mysql.h"

#include <qdatetime.h>
#include <qmap.h>

#define QMYSQL_DRIVER_NAME "QMYSQL3"

class QMYSQLDriverPrivate
{
public:
    QMYSQLDriverPrivate() : mysql(0) {}
    MYSQL*     mysql;
};

class QMYSQLResultPrivate : public QMYSQLDriverPrivate
{
public:
    QMYSQLResultPrivate() : QMYSQLDriverPrivate(), result(0) {}
    MYSQL_RES* result;
    MYSQL_ROW  row;
    QMap< int, int > fieldTypes;
};

QSqlError qMakeError( const QString& err, int type, const QMYSQLDriverPrivate* p )
{
    return QSqlError(QMYSQL_DRIVER_NAME ": " + err, QString(mysql_error( p->mysql )), type);
}

QVariant::Type qDecodeMYSQLType( int mysqltype )
{
    QVariant::Type type;
    switch ( mysqltype ) {
    case FIELD_TYPE_TINY :
    case FIELD_TYPE_SHORT :
    case FIELD_TYPE_LONG :
    case FIELD_TYPE_INT24 :
    case FIELD_TYPE_LONGLONG :
    case FIELD_TYPE_YEAR :
	type = QVariant::Int;
	break;
    case FIELD_TYPE_DECIMAL :
    case FIELD_TYPE_FLOAT :
    case FIELD_TYPE_DOUBLE :
	type = QVariant::Double;
	break;
    case FIELD_TYPE_DATE :
	type = QVariant::Date;
	break;
    case FIELD_TYPE_TIME :
	type = QVariant::Time;
	break;
    case FIELD_TYPE_DATETIME :
    case FIELD_TYPE_TIMESTAMP :
	type = QVariant::DateTime;
	break;
    case FIELD_TYPE_BLOB :
    case FIELD_TYPE_TINY_BLOB :
    case FIELD_TYPE_MEDIUM_BLOB :
    case FIELD_TYPE_LONG_BLOB :
	type = QVariant::ByteArray;
	break;
    default:
    case FIELD_TYPE_ENUM :
    case FIELD_TYPE_SET :
    case FIELD_TYPE_STRING :
    case FIELD_TYPE_VAR_STRING :
	type = QVariant::String;
	break;
    }
    return type;
}

QMYSQLResult::QMYSQLResult( const QMYSQLDriver* db )
: QSqlResult( db )
{
    d =   new QMYSQLResultPrivate();
    d->mysql = db->d->mysql;
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
    if ( at() == i )
	return TRUE;
    mysql_data_seek( d->result, i );
    d->row = mysql_fetch_row( d->result );
    if ( !d->row )
	return FALSE;
    setAt( i );
    return TRUE;
}

bool QMYSQLResult::fetchLast()
{
    return fetch( mysql_num_rows( d->result ) - 1 );
}

bool QMYSQLResult::fetchFirst()
{
    return fetch( 0 );
}

QVariant QMYSQLResult::data( int field )
{
    if ( !isSelect() )
	return QVariant();
    QString val( ( d->row[field] ) );
    QVariant::Type type = qDecodeMYSQLType( d->fieldTypes[ field ] );
    switch ( type ) {
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
    case QVariant::ByteArray: {
	MYSQL_FIELD* f = mysql_fetch_field_direct( d->result, field );
	if ( ! ( f->flags & BLOB_FLAG ) )
	    return QVariant( QByteArray() );
	unsigned long * fl = mysql_fetch_lengths( d->result );
	QByteArray ba;
	ba.duplicate( d->row[field], fl[field] );
	return QVariant( ba );
	break;
    }
    default:
    case QVariant::String:
    case QVariant::CString:
	return QVariant( val );
	break;
    }
#ifdef QT_CHECK_RANGE
    qWarning("QMYSQLResult::data: unknown data type");
#endif
    return QVariant();
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
    if ( mysql_real_query( d->mysql, query, query.length() ) ) {
	setLastError( qMakeError("Unable to execute query", QSqlError::Statement, d ) );
	return FALSE;
    }
    d->result = mysql_store_result( d->mysql );
    if ( !d->result && mysql_field_count( d->mysql ) > 0 ) {
	setLastError( qMakeError( "Unable to store result", QSqlError::Statement, d ) );
	return FALSE;
    }
    int numFields = mysql_field_count( d->mysql );
    setSelect( !( numFields == 0) );
    d->fieldTypes.clear();
    if ( isSelect() ) {
	for( int i = 0; i < numFields; i++) {
	    MYSQL_FIELD* field = mysql_fetch_field_direct( d->result, i );
	    d->fieldTypes[i] = field->type;
	}
    }
    setActive( TRUE );
    return TRUE;
}

int QMYSQLResult::size()
{
    return (int)mysql_num_rows( d->result );
}

int QMYSQLResult::numRowsAffected()
{
    return (int)mysql_affected_rows( d->mysql );
}

/////////////////////////////////////////////////////////

QMYSQLDriver::QMYSQLDriver( QObject * parent, const char * name )
: QSqlDriver(parent, name ? name : QMYSQL_DRIVER_NAME)
{
    init();
}

void QMYSQLDriver::init()
{
    d = new QMYSQLDriverPrivate();
}

QMYSQLDriver::~QMYSQLDriver()
{
    delete d;
}

bool QMYSQLDriver::hasTransactionSupport() const
{

    if ( (d->mysql->server_capabilities & CLIENT_TRANSACTIONS) == CLIENT_TRANSACTIONS )
	return TRUE;
    return FALSE;
}

bool QMYSQLDriver::hasQuerySizeSupport() const
{
    return TRUE;
}

bool QMYSQLDriver::canEditBinaryFields() const
{
    return TRUE;
}

bool QMYSQLDriver::open( const QString & db,
			 const QString & user,
			 const QString & password,
			 const QString & host,
			 int port  )
{
    if ( isOpen() )
	close();
    if ( (d->mysql = mysql_init((MYSQL*) 0)) &&
	    mysql_real_connect( d->mysql,
				host,
				user,
				password,
				db,
				(port > -1) ? port : 0,
				NULL,
				0))
    {
	if ( mysql_select_db( d->mysql, db )) {
	    setLastError( qMakeError("Unable to real connect", QSqlError::Connection, d ) );
	    mysql_close( d->mysql );
	    setOpenError( TRUE );
	    return FALSE;
	}
    } else {
	    setLastError( qMakeError( "Unable to connect", QSqlError::Connection, d ) );
	    mysql_close( d->mysql ) ;
	    return FALSE;
    }
    setOpen( TRUE );
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

QStringList QMYSQLDriver::tables( const QString& ) const
{
    MYSQL_RES* tableRes = mysql_list_tables( d->mysql, NULL );
    MYSQL_ROW	row;
    QStringList tl;
    int i = 0;
    while ( TRUE ) {
	mysql_data_seek( tableRes, i );
	row = mysql_fetch_row( tableRes );
	if ( !row )
	    break;
	tl.append( QString(row[0]) );
	i++;
    }
    mysql_free_result( tableRes );
    return tl;
}

QSqlIndex QMYSQLDriver::primaryIndex( const QString& tablename ) const
{
    QSqlIndex idx;
    QSqlQuery i = createQuery();
    QString stmt( "show index from %1;" );
    QSqlRecord fil = record( tablename );
    i.exec( stmt.arg( tablename ) );
    while ( i.isActive() && i.next() ) {
	if ( i.value(2).toString() == "PRIMARY" ) {
	    idx.append( *fil.field( i.value(3).toInt()-1 ) );
	    idx.setCursorName( i.value(0).toString() );
	    idx.setName( i.value(2).toString() );
	}
    }
    return idx;
}

QSqlRecord QMYSQLDriver::record( const QString& tablename ) const
{
    MYSQL_RES* r = mysql_list_fields( d->mysql, tablename.local8Bit().data(), 0);
    QSqlRecord fil;
    MYSQL_FIELD* field;
    while ( (field = mysql_fetch_field( r ))) {
	QSqlField f ( QString( field->name ) , qDecodeMYSQLType( (int)field->type ) );
	fil.append ( f );
    }
    mysql_free_result( r );
    return fil;
}

QSqlRecord QMYSQLDriver::record( const QSqlQuery& query ) const
{
    QSqlRecord fil;
    if ( query.isActive() && query.driver() == this ) {
	QMYSQLResult* result =  (QMYSQLResult*)query.result();
	QMYSQLResultPrivate* p = result->d;
	if ( !mysql_errno( p->mysql ) ) {
	    for ( ;; ) {
		MYSQL_FIELD* f = mysql_fetch_field( p->result );
		if ( f ) {
		    QSqlField fi( QString((const char*)f->name), qDecodeMYSQLType( f->type ) );
		    fil.append( fi  );
		} else
		    break;
	    }
	}
	mysql_field_seek( p->result, 0 );
    }
    return fil;
}

MYSQL* QMYSQLDriver::mysql()
{
    return d->mysql;
}
