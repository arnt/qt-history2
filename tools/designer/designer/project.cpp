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

    int i = contents.find( "INTERFACES" );
    if ( i == -1 )
	return;
	
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

    // ##### parse project name
    proName = "A Project";
    uifiles = lst;
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
}

bool Project::isValid() const
{
    return TRUE; // #### do actual checking here....
}

void Project::setFormWindow( const QString &f, FormWindow *fw )
{
    formWindows.remove( fw );
    formWindows.insert( fw, f );
}

void Project::setFormWindowFileName( FormWindow *fw, const QString &f )
{
    QString s = *formWindows.find( fw );
    uifiles.remove( s );
    uifiles << f;
    formWindows.remove( fw );
    formWindows.insert( fw, f );
}
