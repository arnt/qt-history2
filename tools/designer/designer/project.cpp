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

#include <qfile.h>
#include <qtextstream.h>
#include <qurl.h>

Project::Project( const QString &fn, const QString &pName )
    : proName( pName )
{
    setFileName( fn );
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
	while ( TRUE ) {
	    i = contents.find( '\n', i + 1 );
	    if ( i == -1 ) {
		end = contents.length() - 1;
		break;
	    }
	    int j = i;
	    bool atEnd = FALSE;
	    while ( j > i ) {
		if ( contents[ j ].isSpace() ||
		     contents[ j ] == '\t' ||
		     contents[ j ] == '\r' ||
		     contents[ j ] == '\n' ) {
		    --j;
		    continue;
		}
		if ( contents[ j ] != '\\' )
		    atEnd = TRUE;
		break;
	    }
	    if ( atEnd ) {
		end = i + 1;
		break;
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
    contents += "DBFILE\t= " + dbFile + "\n";
    
    i = contents.find( "PROJECTNAME" );
    if ( i != -1 ) {
	int start = i;
	int end = contents.find( '\n', i );
	if ( end == -1 )
	    end = contents.length() - 1;
	contents.remove( start, end - start + 1 );
    }
    contents += "PROJECTNAME\t= " + proName + "\n";
    
    if ( !f.open( IO_WriteOnly ) ) {
	qWarning( "Couldn't write project file...." );
	return;
    }
	
    QTextStream os( &f );
    os << contents;

    f.close();
}
