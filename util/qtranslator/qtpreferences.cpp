/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/qtpreferences.cpp#1 $
**
** This is a utility program for translating Qt applications
**
**
** Copyright (C) 1999 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtpreferences.h"
#include "qtconfig.h"

#include <qapplication.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdir.h>

#include <stdlib.h>

/****************************************************************************
 *
 * Class: QTPreferences
 *
 ****************************************************************************/

QTPreferences::Sources::Sources()
    : directories(), extensions()
{
}

QTPreferences::Traslation::Traslation()
    : directory(), prefix(), folders( FALSE )
{
}

QTPreferences::QTPreferences()
    : sources(), projectFile( QString::null),
      projectConfig( 0L )
{
    QString currDir( qApp->argv()[ 0 ] );
    QFileInfo finf( currDir );
    currDir = finf.dirPath( TRUE );

    if ( !QFileInfo( currDir + "/qtranslator.inf" ).exists() )
        createGlobalConfigFile( currDir + "/qtranslator.inf" );

    globalConfig = new QTConfig( currDir + "/qtranslator.inf" );
}

QTPreferences::~QTPreferences()
{
    delete globalConfig;
    if ( projectConfig )
        delete projectConfig;
}

void QTPreferences::createProjectConfig()
{
    if ( projectConfig )
        delete projectConfig;

    projectConfig = new QTConfig( projectFile );

    getLanguages();
}

void QTPreferences::saveProjectConfig()
{
    projectConfig->setGroup( "Sources" );
    projectConfig->writeEntry( "Directories", sources.directories, QChar( ';' ) );
    projectConfig->writeEntry( "Extensions", sources.extensions, QChar( ';' ) );

    projectConfig->setGroup( "Translation" );
    projectConfig->writeEntry( "Directory", translation.directory );
    projectConfig->writeEntry( "Prefix", translation.prefix );
    projectConfig->writeEntry( "Folders", translation.folders );

    projectConfig->write();
}

void QTPreferences::readProjectConfig()
{
    if ( projectConfig )
        delete projectConfig;

    projectConfig = new QTConfig( projectFile );

    projectConfig->setGroup( "Sources" );
    sources.directories = projectConfig->readListEntry( "Directories", QChar( ';' ) );
    sources.extensions = projectConfig->readListEntry( "Extensions", QChar( ';' ) );

    projectConfig->setGroup( "Translation" );
    translation.directory = projectConfig->readEntry( "Directory" );
    translation.prefix = projectConfig->readEntry( "Prefix" );
    translation.folders = projectConfig->readBoolEntry( "Folders" );

    getLanguages();
}

void QTPreferences::createGlobalConfigFile( const QString &filename )
{
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) ) {
        qWarning( "could not open for writing `%s'", filename.latin1() );
        return;
    }

    QTextStream s( &f );
    QString qtdir = getenv( "QTDIR" );
    s << "[General]\n";
    s << "findtr = " << qtdir << "/bin/findtr\n";

    f.close();

}

void QTPreferences::getLanguages()
{
    QDir dir( translation.directory );
    languages.clear();
    if ( !translation.folders ) {
        QStringList lst = dir.entryList( "*.po" );
        QStringList::Iterator it = lst.begin();
        for ( ; it != lst.end(); ++it ) {
            QString l = *it;
            l.remove( 0, translation.prefix.length() );
            int dot = l.find( '.' );
            if ( dot != -1 )
                l.remove( dot, l.length() - dot );
            languages.append( l );
        }
    } else {
        QStringList lst = dir.entryList( QDir::Dirs );
        QStringList::Iterator it = lst.begin();
        for ( ; it != lst.end(); ++it ) {
            QDir d( translation.directory + "/" + *it );
            if ( d.entryList( "*.po" ).count() > 0 )
                languages.append( *it );
        }
    }
}
