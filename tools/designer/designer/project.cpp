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
	    if ( ( c.isLetter() || c.isDigit() || c == '.' ) &&
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

bool Project::isFormLoaded( const QString &form )
{
    return loadedForms.find( form ) != loadedForms.end();
}

void Project::setFormLoaded( const QString &form, bool loaded )
{
    if ( loaded && !isFormLoaded( form ) )
	loadedForms << form;
    else if ( !loaded )
	loadedForms.remove( form );
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
    QFile f( filename );
    if ( !f.exists() || !f.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &f );
    QString contents = ts.read();
    f.close();

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
	qWarning( "Couldn't write project file...." );
	return;
    }
	
    QTextStream os( &f );
    os << contents;

    f.close();
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



bool Project::DatabaseConnection::connect()
{
    if ( name != "(default)" )
	connection = QSqlDatabase::addDatabase( driver );
    else
	connection = QSqlDatabase::addDatabase( driver, name );
    connection->setDatabaseName( dbName );
    connection->setUserName( username );
    connection->setPassword( password );
    connection->setHostName( hostname );
    if ( !connection->open() )
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

    connection->close();

    return TRUE;
}

bool Project::DatabaseConnection::sync()
{
    return TRUE;
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
    if ( !conn )
	return QStringList();
    return conn->tables;
}

QStringList Project::databaseFieldList( const QString &connection, const QString &table )
{
    DatabaseConnection *conn = databaseConnection( connection );
    if ( !conn )
	return QStringList();
    return conn->fields[ table ];
}

void Project::saveConnections()
{
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
