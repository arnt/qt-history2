/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "profile.h"
#include <qxml.h>
#include <qtextcodec.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qdir.h>
#include <qlist.h>
#include <qlibraryinfo.h>

#define QT_TITLE         QLatin1String("Qt Reference Documentation")
#define DESIGNER_TITLE   QLatin1String("Qt Designer Manual")
#define ASSISTANT_TITLE  QLatin1String("Qt Assistant Manual")
#define LINGUIST_TITLE   QLatin1String("Qt Linguist Manual")
#define QMAKE_TITLE      QLatin1String("qmake Manual")

Profile *Profile::createDefaultProfile()
{
    QString path = QLibraryInfo::location(QLibraryInfo::DocumentationPath) + QLatin1String("/html/");
    Profile *profile = new Profile;
    profile->valid = true;
    profile->type = DefaultProfile;
    profile->props[QLatin1String("name")] = QLatin1String("default");
    profile->props[QLatin1String("applicationicon")] = QLatin1String("assistant.png");
    profile->props[QLatin1String("aboutmenutext")] = QLatin1String("About Qt");
    profile->props[QLatin1String("abouturl")] = QLatin1String("about_qt");
    profile->props[QLatin1String("title")] = QLatin1String("Qt Assistant");
    profile->props[QLatin1String("basepath")] = path;
    profile->props[QLatin1String("startpage")] = path + QLatin1String("index.html");

    profile->addDCFTitle( path + QLatin1String("qt.dcf"), QT_TITLE );
    profile->addDCFTitle( path + QLatin1String("designer.dcf"), DESIGNER_TITLE );
    profile->addDCFTitle( path + QLatin1String("assistant.dcf"), ASSISTANT_TITLE );
    profile->addDCFTitle( path + QLatin1String("linguist.dcf"), LINGUIST_TITLE );
    profile->addDCFTitle( path + QLatin1String("qmake.dcf"), QMAKE_TITLE );

    profile->addDCFIcon( QT_TITLE, QLatin1String("qt.png") );
    profile->addDCFIcon( DESIGNER_TITLE, QLatin1String("designer.png") );
    profile->addDCFIcon( ASSISTANT_TITLE, QLatin1String("assistant.png") );
    profile->addDCFIcon( LINGUIST_TITLE, QLatin1String("linguist.png") );

    profile->addDCFIndexPage( QT_TITLE, path + QLatin1String("index.html") );
    profile->addDCFIndexPage( DESIGNER_TITLE, path + QLatin1String("designer-manual.html") );
    profile->addDCFIndexPage( ASSISTANT_TITLE, path + QLatin1String("assistant-manual.html") );
    profile->addDCFIndexPage( LINGUIST_TITLE, path + QLatin1String("linguist-manual.html") );
    profile->addDCFIndexPage( QMAKE_TITLE, path + QLatin1String("qmake-manual.html") );

    profile->addDCFImageDir( QT_TITLE, QLatin1String("../../gif/") );
    profile->addDCFImageDir( DESIGNER_TITLE, QLatin1String("../../gif/") );
    profile->addDCFImageDir( ASSISTANT_TITLE, QLatin1String("../../gif/") );
    profile->addDCFImageDir( LINGUIST_TITLE, QLatin1String("../../gif/") );
    profile->addDCFImageDir( QMAKE_TITLE, QLatin1String("../../gif/") );

    return profile;
}


Profile::Profile()
    : valid( true ), dparser( 0 )
{
}


void Profile::removeDocFileEntry( const QString &docfile )
{
    docs.removeAll( docfile );

    QStringList titles;

    for( QMap<QString,QString>::Iterator it = dcfTitles.begin();
         it != dcfTitles.end(); ++it ) {
        if( (*it) == docfile ) {
            indexPages.remove( *it );
            icons.remove( *it );
            imageDirs.remove( *it );
            titles << it.key();
        }
    }

    for( QStringList::ConstIterator title = titles.begin();
         title != titles.end(); ++title ) {

        dcfTitles.remove( *title );
    }

#ifdef ASSISTANT_DEBUG
    qDebug( "docs:\n  - " + docs.join( "\n  - " ) );
    qDebug( "titles:\n  - " + titles.join( "\n  - " ) );
    qDebug( "keys:\n  - " + ( (QStringList*) &(dcfTitles.keys()) )->join( "\n  - " ) );
    qDebug( "values:\n  - " + ( (QStringList*) &(dcfTitles.values()) )->join( "\n  - " ) );
#endif
}
