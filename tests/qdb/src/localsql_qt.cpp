/*
    Xbase project source code

    This file contains the LocalSQL Qt SQL driver implementation

    Copyright (C) 2000 Dave Berton (db@trolltech.com)
		       Jasmin Blanchette (jasmin@trolltech.com)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#include "../include/localsql_qt.h"

#include <qdatetime.h>
#include <qregexp.h>
#include <qdir.h>
#include "../include/localsql.h"

class LocalSQLResult::LocalSQLPrivate
{
public:
    LocalSQLPrivate(){}
    QSqlError makeError( const QString& err )
    {
	return QSqlError("LocalSQL: " + err, env.lastError(), 0);
    }
    QString databasePath;
    LocalSQL env;
};

LocalSQLResult::LocalSQLResult( const LocalSQLDriver* db, const QString& path )
    : QSqlResult( db )
{
    d =   new LocalSQLPrivate();
    d->databasePath = path;
}

LocalSQLResult::~LocalSQLResult()
{
    cleanup();
    delete d;
}

void LocalSQLResult::cleanup()
{
    d->env.reset();
    setAt( QSql::BeforeFirst );
    setActive( FALSE );
}

bool LocalSQLResult::fetch( int i )
{
    if ( !isActive() ) {
	return FALSE;
    }
    if ( at() == i ) {
	return TRUE;
    }
    if ( i >= (int)d->env.resultSet(0)->size() ) {
	return FALSE;
    }
    if ( i < 0 ) {
	return FALSE;
    }
    if ( at() == QSql::AfterLast ) {
	if ( !d->env.resultSet(0)->last() ) {
	    return FALSE;
	}
	setAt( (int)d->env.resultSet(0)->size()-1 );
    }
    int it = 0;
    if ( at() < i ) {
	while ( at() < i ) {
	    if ( !d->env.resultSet(0)->next() ) {
		setAt( QSql::AfterLast );
		return FALSE;
	    }
	    ++it;
	    setAt( at() + 1 );
	}
    } else {
	while ( at() > i ) {
	    if ( !d->env.resultSet(0)->prev() ) {
		setAt( QSql::BeforeFirst );
		return FALSE;
	    }
	    ++it;
	    setAt( at() - 1 );
	}
    }
    setAt( i );
    return TRUE;
}

bool LocalSQLResult::fetchFirst()
{
    return fetch( 0 );
}

bool LocalSQLResult::fetchLast()
{
    return fetch( d->env.resultSet(0)->size()-1 );
}

QVariant LocalSQLResult::data( int i )
{
    if ( !isValid() ) {
	return QVariant();
    }
    return d->env.resultSet(0)->currentRecord()[i];
}

bool LocalSQLResult::isNull( int /*field*/ )
{
    return FALSE;
}

bool LocalSQLResult::reset ( const QString& query )
{
    cleanup();
    if ( !driver() )
	return FALSE;
    if ( !driver()-> isOpen() || driver()->isOpenError() )
	return FALSE;
    setActive( FALSE );
    setAt( QSql::BeforeFirst );
    d->env.reset();
    if ( !d->env.parse( query ) ) {
	setLastError( d->makeError( "Unable to parse query" ) );
	return FALSE;
    }
    if ( !d->env.execute() ) {
	//## this error is not propogating backwards!
	setLastError( d->makeError( "Unable to execute query" ) );
	return FALSE;
    }
    setSelect( TRUE ); //## fix
    setActive( TRUE );
    return TRUE;
}

int LocalSQLResult::size()
{
    return d->env.resultSet(0)->size();
}

int LocalSQLResult::numRowsAffected()
{
    qWarning("LocalSQLResult::numRowsAffected::not yet implemented!");
    return -1;
}

///////////////////////////////////////////////////////////////////

LocalSQLDriver::LocalSQLDriver( QObject * parent, const char * name )
    : QSqlDriver(parent,name ? name : "LocalSQL")
{
    init();
}

void LocalSQLDriver::init()
{
}

LocalSQLDriver::~LocalSQLDriver()
{
}

bool LocalSQLDriver::hasTransactionSupport() const
{
    return FALSE;
}

bool LocalSQLDriver::hasQuerySizeSupport() const
{
    return TRUE;
}

bool LocalSQLDriver::canEditBinaryFields() const
{
    return FALSE;
}

bool LocalSQLDriver::open( const QString & db,
			   const QString &,
			   const QString &,
			   const QString &,
			   int )
{
    if ( isOpen() )
	close();
    databasePath = db;
    if ( !databasePath ) {
	setOpenError( TRUE );
	setLastError( QSqlError( "LocalSQLDriver:: No database name specified" ) );
	return FALSE;
    }
    QDir dbdir ( databasePath );
    if ( !dbdir.exists() ) {
	setOpenError( TRUE );
	setLastError( QSqlError( "LocalSQLDriver:: Database does not exist" ) );
	return FALSE;
    }
    setOpen( TRUE );
    return TRUE;
}

void LocalSQLDriver::close()
{
    if ( isOpen() ) {
	databasePath = QString::null;
	setOpen( FALSE );
	setOpenError( FALSE );
    }
}

QSqlQuery LocalSQLDriver::createQuery() const
{
    return QSqlQuery( new LocalSQLResult( this, databasePath ) );
}

bool LocalSQLDriver::beginTransaction()
{
    return FALSE;
}

bool LocalSQLDriver::commitTransaction()
{
    return FALSE;
}

bool LocalSQLDriver::rollbackTransaction()
{
    return FALSE;
}

QStringList LocalSQLDriver::tables( const QString& /*user*/ ) const
{
    QStringList tl;
    QDir dir( databasePath );
    tl = dir.entryList( "*.dbf", QDir::Files );
    for ( uint i = 0; i < tl.count(); ++i ) {
	tl[i] = tl[i].replace( QRegExp("\\.dbf") , "" );
    }
    return tl;
}

QSqlIndex LocalSQLDriver::primaryIndex( const QString& tablename ) const
{
    LocalSQL env;
    env.addFileDriver( 0, databasePath + "/" + tablename );
    localsql::FileDriver* driver = env.fileDriver( 0 );
    if ( !driver->open() ) {
	qWarning( "LocalSQLDriver::record: Unable to open table:" + tablename );
	return QSqlIndex();
    }
    QSqlIndex idx( tablename );
    QStringList names = driver->columnNames();
    QValueList<QVariant::Type> types = driver->columnTypes();
    QStringList primaryIndexFields = driver->primaryIndex();
    for ( uint i = 0; i < primaryIndexFields.count(); ++i ) {
	for ( uint j = 0; j < names.count(); ++j ) {
	    if ( names[j] == primaryIndexFields[i] ) {
		QSqlField f( names[j], types[j] );
		idx.append( f );
	    }
	}
    }
    return idx;
}

QSqlRecord LocalSQLDriver::record( const QString& tablename ) const
{
    LocalSQL env;
    env.addFileDriver( 0, databasePath + "/" + tablename );
    localsql::FileDriver* driver = env.fileDriver( 0 );
    if ( !driver->open() ) {
	qWarning( "LocalSQLDriver::record: Unable to open table:" + tablename );
	return QSqlRecord();
    }
    QSqlRecord fil;
    QStringList names = driver->columnNames();
    QValueList<QVariant::Type> types = driver->columnTypes();
    for ( uint i = 0; i < names.count(); ++i ) {
	QSqlField f( names[i], types[i] );
	fil.append( f );
    }
    return fil;
}

QSqlRecord LocalSQLDriver::record( const QSqlQuery& /*query*/ ) const
{
    QSqlRecord fil;
    qWarning("LocalSQLDriver::record:: not yet implemented!");
    return fil;
}
