/*
  main.cpp
*/

#include <qdir.h>
#include <qstring.h>

#include <stdlib.h>

#include "configuration.h"
#include "messages.h"
#include "steering.h"

// defined in cppparser.cpp
extern void parseCppHeaderFile( Steering *steering, const QString& fileName );
extern void parseCppSourceFile( Steering *steering, const QString& fileName );

// defined in htmlparser.cpp
extern void parseHtmlFile( Steering *steering, const QString& fileName );

static void applyDepth( void (*apply)(Steering *, const QString&),
			Steering *steering, const QString& rootDir,
			const QString& nameFilter )
{
    QStringList fileNames;
    QStringList::Iterator fn;
    QDir dir( rootDir );
    if ( !dir.exists() )
	return;

    dir.setSorting( QDir::Name );

    dir.setNameFilter( nameFilter );
    dir.setFilter( QDir::Files );
    fileNames = dir.entryList();
    fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	apply( steering, dir.filePath(*fn) );
	++fn;
    }

    dir.setNameFilter( QChar('*') );
    dir.setFilter( QDir::Dirs );
    fileNames = dir.entryList();
    fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	if ( *fn != QChar('.') && *fn != QString("..") )
	    applyDepth( apply, steering, dir.filePath(*fn), nameFilter );
	++fn;
    }
}

int main( int argc, char **argv )
{
    config = new Configuration( argc, argv );
    Steering steering;

    QStringList::ConstIterator s;

    s = config->includeDirList().begin();
    while ( s != config->includeDirList().end() ) {
	applyDepth( parseCppHeaderFile, &steering, *s, QString("*.h") );
	++s;
    }

    steering.nailDownDecls();

    s = config->sourceDirList().begin();
    while ( s != config->sourceDirList().end() ) {
	applyDepth( parseCppSourceFile, &steering, *s, QString("*.cpp") );
	++s;
    }

    s = config->docDirList().begin();
    while ( s != config->docDirList().end() ) {
	applyDepth( parseCppSourceFile, &steering, *s, QString("*.doc") );
	++s;
    }

    if ( config->supervisor() )
	applyDepth( parseHtmlFile, &steering, config->outputDir(),
		    QString("*.html") );

    steering.nailDownDocs();
    steering.emitHtml();

    warnAboutOmitted();

    config = 0;
    delete config;
    return EXIT_SUCCESS;
}
