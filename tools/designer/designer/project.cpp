/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "project.h"
#include "formwindow.h"
#include "config.h"

#include <qfile.h>
#include <qtextstream.h>
#include <qurl.h>
#include <qsqlrecord.h>
#include <qsqltable.h>
#include <qobjectlist.h>

Project::Project( const QString &fn, const QString &pName )
    : proName( pName )
{
    setFileName( fn );
    if ( !pName.isEmpty() )
	proName = pName;
}

void Project::setFileName( const QString &fn, bool doClear )
{
    filename = fn;
    if ( !doClear )
	return;
    clear();
    if ( QFile::exists( fn ) )
	parse();
}

QString Project::fileName() const
{
    return filename;
}

QStringList Project::uiFiles() const
{
    return uifiles;
}

QString Project::databaseDescription() const
{
    return dbFile;
}

void Project::setProjectName( const QString &n )
{
    proName = n;
    save();
}

QString Project::projectName() const
{
    return proName;
}

void Project::parse()
{
    QFile f( filename );
    if ( !f.exists() || !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    QString contents = ts.read();
    f.close();

    QString fl( QFileInfo( filename ).baseName() );
    proName = fl[ 0 ].upper() + fl.mid( 1 );

    int i = contents.find( "INTERFACES" );
    if ( i != -1 ) {
	QString part = contents.mid( i + QString( "INTERFACES" ).length() );
	QStringList lst;
	bool inName = FALSE;
	QString currName;
	for ( i = 0; i < (int)part.length(); ++i ) {
	    QChar c = part[ i ];
	    if ( ( c.isLetter() || c.isDigit() || c == '.' || c == '/' ) &&
		 c != ' ' && c != '\t' && c != '\n' && c != '=' && c != '\\' ) {
		if ( !inName )
		    currName = QString::null;
		currName += c;
		inName = TRUE;
	    } else {
		if ( inName ) {
		    inName = FALSE;
		    if ( currName.right( 3 ).lower() == ".ui" )
			lst.append( currName );
		}
	    }
	}

	uifiles = lst;
    }

    i = contents.find( "DBFILE" );
    if ( i != -1 ) {
	QString part = contents.mid( i + QString( "DBFILE" ).length() );
	bool inName = FALSE;
	QString currName;
	for ( i = 0; i < (int)part.length(); ++i ) {
	    QChar c = part[ i ];
	    if ( !inName ) {
		if ( c != ' ' && c != '\t' && c != '\n' && c != '=' && c != '\\' )
		    inName = TRUE;
		else
		    continue;
	    }
	    if ( inName ) {
		if ( c == '\n' )
		    break;
		dbFile += c;
	    }
	}
    }

    i = contents.find( "PROJECTNAME" );
    if ( i != -1 ) {
	proName = "";
	QString part = contents.mid( i + QString( "PROJECTNAME" ).length() );
	bool inName = FALSE;
	QString currName;
	for ( i = 0; i < (int)part.length(); ++i ) {
	    QChar c = part[ i ];
	    if ( !inName ) {
		if ( c != ' ' && c != '\t' && c != '\n' && c != '=' && c != '\\' )
		    inName = TRUE;
		else
		    continue;
	    }
	    if ( inName ) {
		if ( c == '\n' )
		    break;
		proName += c;
	    }
	}
    }

    loadConnections();
}

void Project::clear()
{
    uifiles.clear();
    dbFile = "";
    proName = "unnamed";
    loadedForms.clear();
    desc = "";
}

void Project::addUiFile( const QString &f, FormWindow *fw )
{
    uifiles << f;
    if ( fw )
	formWindows.insert( fw, f );
    save();
}

void Project::removeUiFile( const QString &f, FormWindow *fw )
{
    formWindows.remove( fw );
    uifiles.remove( f );
    save();
}

void Project::setDatabaseDescription( const QString &db )
{
    dbFile = db;
    save();
}

void Project::setDescription( const QString &s )
{
    desc = s;
}

QString Project::description() const
{
    return desc;
}

void Project::setUiFiles( const QStringList &lst )
{
    uifiles = lst;
    save();
}

bool Project::isValid() const
{
    return TRUE; // #### do actual checking here....
}

bool Project::hasFormWindow( FormWindow* fw ) const
{
    return formWindows.contains( fw );
}

void Project::setFormWindow( const QString &f, FormWindow *fw )
{
    formWindows.remove( fw );
    formWindows.insert( fw, f );
    save();
}

void Project::setFormWindowFileName( FormWindow *fw, const QString &f )
{
    QString s = *formWindows.find( fw );
    uifiles.remove( s );
    uifiles << f;
    formWindows.remove( fw );
    formWindows.insert( fw, f );
    save();
}

QString Project::makeAbsolute( const QString &f )
{
    QUrl u( QFileInfo( filename ).dirPath( TRUE ), f );
    return u.path();
}

QString Project::makeRelative( const QString &f )
{
    QString p = QFileInfo( filename ).dirPath( TRUE );
    QString f2 = f;
    if ( f2.left( p.length() ) == p )
	f2.remove( 0, p.length() + 1 );
    return f2;
}

void Project::save()
{
    if ( proName == "<No Project>" )
	return;
    QFile f( filename );
    QString contents;
    if ( f.open( IO_ReadOnly ) ) {
	QTextStream ts( &f );
	contents = ts.read();
	f.close();
    } else {
	contents += "TEMPLATE\t= app\nCONFIG\t= qt warn_on release\nTARGET\t= " + proName.lower() + "\n";
    }

    int i = contents.find( "INTERFACES" );
    if ( i != -1 ) {
	int start = i;
	int end = i;
	i = contents.find( '\n', i );
	if ( i == -1 ) {
	    end = contents.length() - 1;
	} else {
	    end = i;
	    int lastNl = i;
	    for ( ; i < (int)contents.length(); ++i ) {
		int j = contents.find( '\n', lastNl + 1 );
		if ( i == -1 ) {
		    end = contents.length() - 1;
		    break;
		} else {
		    if ( contents.mid( lastNl, j - lastNl + 1 ).find( '=' ) == -1 )
			lastNl = j;
		    else
			break;
		}
	    }
	}
	contents.remove( start, end - start + 1 );
    }

    if ( !uifiles.isEmpty() ) {
	contents += "INTERFACES\t= ";
	for ( QStringList::Iterator it = uifiles.begin(); it != uifiles.end(); ++it )
	    contents += *it + " ";
	contents += "\n";
    }

    i = contents.find( "DBFILE" );
    if ( i != -1 ) {
	int start = i;
	int end = contents.find( '\n', i );
	if ( end == -1 )
	    end = contents.length() - 1;
	contents.remove( start, end - start + 1 );
    }
    if ( !dbFile.isEmpty() )
	contents += "DBFILE\t= " + dbFile + "\n";

    i = contents.find( "PROJECTNAME" );
    if ( i != -1 ) {
	int start = i;
	int end = contents.find( '\n', i );
	if ( end == -1 )
	    end = contents.length() - 1;
	contents.remove( start, end - start + 1 );
    }
    if ( !proName.isEmpty() )
	contents += "PROJECTNAME\t= " + proName + "\n";

    if ( !f.open( IO_WriteOnly ) ) {
	//## more of a warning here? mbox?
	qWarning( "Couldn't write project file " + filename );
	return;
    }

    QTextStream os( &f );
    os << contents;

    f.close();

    saveConnections();
}

QList<Project::DatabaseConnection> Project::databaseConnections() const
{
    return dbConnections;
}

void Project::setDatabaseConnections( const QList<Project::DatabaseConnection> &lst )
{
    dbConnections = lst;
}

void Project::addDatabaseConnection( Project::DatabaseConnection *conn )
{
    dbConnections.append( conn );
    saveConnections();
}

Project::DatabaseConnection *Project::databaseConnection( const QString &name )
{
    for ( Project::DatabaseConnection *conn = dbConnections.first(); conn; conn = dbConnections.next() ) {
	if ( conn->name == name )
	    return conn;
    }
    return 0;
}

bool Project::DatabaseConnection::refreshCatalog()
{
    if ( loaded )
	return TRUE;
    if ( !open() )
	return FALSE;
    tables = connection->tables();
    fields.clear();
    for ( QStringList::Iterator it = tables.begin(); it != tables.end(); ++it ) {
	QSqlRecord fil = connection->record( *it );
	QStringList lst;
	for ( uint j = 0; j < fil.count(); ++j )
	    lst << fil.field( j )->name();
	fields.insert( *it, lst );
    }
    loaded = TRUE;
    connection->close();
    return loaded;
}

bool Project::DatabaseConnection::open()
{
    // register our name, if nec
    if ( name == "(default)" ) {
	if ( !QSqlDatabase::contains() ) // default doesn't exists?
	    connection = QSqlDatabase::addDatabase( driver );
	else
	    connection = QSqlDatabase::database();
    } else {
	if ( !QSqlDatabase::contains( name ) )
	    connection = QSqlDatabase::addDatabase( driver, name );
	else
	    connection = QSqlDatabase::database( name );
    }
    connection->setDatabaseName( dbName );
    connection->setUserName( username );
    connection->setPassword( password );
    connection->setHostName( hostname );
    return connection->open();
}

void Project::DatabaseConnection::close()
{
    if ( !loaded )
	return;
    if ( connection )
	connection->close();
}

QStringList Project::databaseConnectionList()
{
    QStringList lst;
    for ( DatabaseConnection *conn = dbConnections.first(); conn; conn = dbConnections.next() )
	lst << conn->name;
    return lst;
}

QStringList Project::databaseTableList( const QString &connection )
{
    DatabaseConnection *conn = databaseConnection( connection );
    if ( !conn ) {
	return QStringList();
    }
    return conn->tables;
}

QStringList Project::databaseFieldList( const QString &connection, const QString &table )
{
    DatabaseConnection *conn = databaseConnection( connection );
    if ( !conn )
	return QStringList();
    return conn->fields[ table ];
}

static bool inSaveConnections = FALSE;

void Project::saveConnections()
{
    if ( inSaveConnections )
	return;
    inSaveConnections = TRUE;
    if ( !QFile::exists( dbFile ) )
	setDatabaseDescription( QFileInfo( filename ).dirPath( TRUE ) + "/" + "database.db" );

    Config conf( dbFile );
    conf.setGroup( "Connections" );
    conf.writeEntry( "Connections", databaseConnectionList(), ',' );

    for ( Project::DatabaseConnection *conn = dbConnections.first(); conn; conn = dbConnections.next() ) {
	conf.setGroup( conn->name );
	conf.writeEntry( "Tables", conn->tables, ',' );
	for ( QStringList::Iterator it = conn->tables.begin(); it != conn->tables.end(); ++it )
	    conf.writeEntry( "Fields[" + *it + "]", conn->fields[ *it ], ',' );
	conf.writeEntry( "Driver", conn->driver );
	conf.writeEntry( "DatabaseName", conn->dbName );
	conf.writeEntry( "Username", conn->username );
	conf.writeEntry( "Password", conn->password ); // ##################### Critical: figure out how to save passwd
	conf.writeEntry( "Hostname", conn->hostname );
    }

    conf.write();
    inSaveConnections = FALSE;
}

void Project::loadConnections()
{
    if ( !QFile::exists( dbFile ) )
	return;

    Config conf( dbFile );
    conf.setGroup( "Connections" );

    QStringList conns = conf.readListEntry( "Connections", ',' );
    for ( QStringList::Iterator it = conns.begin(); it != conns.end(); ++it ) {
	DatabaseConnection *conn = new DatabaseConnection( this );
	conn->name = *it;
	conf.setGroup( *it );
	conn->tables = conf.readListEntry( "Tables", ',' );
	for ( QStringList::Iterator it2 = conn->tables.begin(); it2 != conn->tables.end(); ++it2 ) {
	    QStringList lst = conf.readListEntry( "Fields[" + *it2 + "]", ',' );
	    conn->fields.insert( *it2, lst );
	}
	conn->driver = conf.readEntry( "Driver" );
	conn->dbName = conf.readEntry( "DatabaseName" );
	conn->username = conf.readEntry( "Username" );
	conn->password = conf.readEntry( "Password" );
	conn->hostname = conf.readEntry( "Hostname" );
	dbConnections.append( conn );
    }
}

/*! Opens the database \a connection.  The connection remains open and
can be closed again with closeDatabase().
*/

void Project::openDatabase( const QString &connection )
{
    DatabaseConnection *conn = databaseConnection( connection );
    if ( connection.isEmpty() && !conn )
	conn = databaseConnection( "(default)" );
    if ( !conn )
	return;
    conn->open();
}

/*! Closes the database \a connection.
*/
void Project::closeDatabase( const QString &connection )
{
    DatabaseConnection *conn = databaseConnection( connection );
    if ( connection.isEmpty() && !conn )
	conn = databaseConnection( "(default)" );
    if ( !conn )
	return;
    conn->close();
}

void Project::formClosed( FormWindow *fw )
{
    formWindows.remove( fw );
}

QObjectList *Project::formList() const
{
    QObjectList *l = new QObjectList;
    for ( QMap<FormWindow*, QString>::ConstIterator it = formWindows.begin(); it != formWindows.end(); ++it )
	l->append( it.key() );
    return l;
}
