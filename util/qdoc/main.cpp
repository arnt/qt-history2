/*
  main.cpp
*/

#include <qdir.h>
#include <qstring.h>

#include <stdlib.h>

#include "config.h"
#include "emitter.h"
#include "messages.h"

// defined in cppparser.cpp
extern void parseCppHeaderFile( Emitter *emitter, const QString& fileName );
extern void parseCppSourceFile( Emitter *emitter, const QString& fileName );

// defined in htmlparser.cpp
extern void parseHtmlFile( Emitter *emitter, const QString& fileName );

static int compareMtime( const void *n1, const void *n2 )
{
    if ( n1 == 0 || n2 == 0 )
	return 0;

    QFileInfo f1( *(QString *) n1 );
    QFileInfo f2( *(QString *) n2 );
    if ( f1.lastModified() < f2.lastModified() )
	return -1;
    else if ( f1.lastModified() > f2.lastModified() )
	return 1;
    else
	return 0;
}

static QStringList find( const QString & rootDir, const QString & nameFilter )
{
    QStringList result;
    QStringList fileNames;
    QStringList::Iterator fn;
    QDir dir( rootDir );

    dir.setNameFilter( nameFilter );
    dir.setSorting( QDir::Name );
    dir.setFilter( QDir::Files );
    fileNames = dir.entryList();
    fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	result += dir.filePath(*fn);
	++fn;
    }

    dir.setNameFilter( QChar('*') );
    dir.setFilter( QDir::Dirs );
    fileNames = dir.entryList();
    fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	if ( *fn != QChar('.') && *fn != QString("..") )
	    result += find( dir.filePath(*fn), nameFilter );
	++fn;
    }
    return result;
}

int main( int argc, char **argv )
{
    config = new Config( argc, argv );
    Emitter emitter;

    if ( config->outputDir().isEmpty() ) {
	warning( 1, "No output directory specified (OUTPUTDIR)" );
	return EXIT_FAILURE;
    }

    QDir dir( config->outputDir() );
    if ( !dir.exists() ) {
	if ( !dir.mkdir(config->outputDir()) ) {
	    warning( 1, "Cannot create '%s'", config->outputDir().latin1() );
	    return EXIT_FAILURE;
	}
    }

    QStringList::ConstIterator s;

    // read the header files in any old order
    QStringList headerFiles;
    s = config->includeDirList().begin();
    while ( s != config->includeDirList().end() ) {
	headerFiles += find( *s, QString("*.h") );
	++s;
    }
    s = headerFiles.begin();
    while( s != headerFiles.end() ) {
	parseCppHeaderFile( &emitter, *s );
	++s;
    }

    emitter.nailDownDecls();

    // then read the .cpp and .doc files, sorted strictly by
    // modification time, most recent first
    QStringList sourceFiles;
    s = config->sourceDirList().begin();
    while ( s != config->sourceDirList().end() ) {
	sourceFiles += find( *s, QString("*.cpp") );
	++s;
    }

    s = config->docDirList().begin();
    while ( s != config->docDirList().end() ) {
	sourceFiles += find( *s, QString("*.doc") );
	++s;
    }

    int i = 0;
    int n = sourceFiles.count();
    QString *files = new QString[n];
    s = sourceFiles.begin();
    while ( s != sourceFiles.end() && i < n ) {
	files[i++] = *s;
	++s;
    }
    qsort( files, n, sizeof(QString), compareMtime );
    i = n;
    while( i-- > 0 )
	parseCppSourceFile( &emitter, files[i] );
    delete[] files;
    files = 0;

    // finally, pick up old output for the supervisor
    QStringList outputFiles;
    if ( config->supervisor() ) {
	outputFiles = find( config->outputDir(), QString("*.html") );
	s = outputFiles.begin();
	while ( s != outputFiles.end() ) {
	    parseHtmlFile( &emitter, *s );
	    ++s;
	}
    }

    emitter.nailDownDocs();
    emitter.emitHtml();

    warnAboutOmitted();

    config = 0;
    delete config;
    return EXIT_SUCCESS;
}
