/*
  main.cpp
*/

#include <qapplication.h>

#include "config.h"
#include "doc.h"
#include "messages.h"

static void printHelp()
{
    Messages::information(
	    qdoc::tr("Usage: qdoc [options] file1.qdoc...\n"
		     "Options:\n"
		     "    -help  Display this information and exit\n"
		     "    -verbose\n"
		     "           Explain what is being done\n"
		     "    -version\n"
		     "           Display version of qdoc and exit") );
}

static void printVersion()
{
    Messages::information( qdoc::tr("qdoc version 3.0") );
}

static void processQdocFile( const QString& fileName )
{
    QPtrList<QTranslator> translators;
    translators.setAutoDelete( TRUE );

    Config config;
    config.load( fileName );

    Doc::initialize( config );
    Location::initialize( config );

    QStringList fileNames = config.getStringList( CONFIG_TRANSLATORS );
    QStringList::Iterator fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	QTranslator *translator = new QTranslator( 0 );
	if ( !translator->load(*fn) )
	    Messages::error( Location(fileName),
			     qdoc::tr("Cannot load translator '%1'")
			     .arg(*fn) );
	qApp->installTranslator( translator );
	translators.append( translator );
	++fn;
    }

    Doc::terminate();
}

int main( int argc, char **argv )
{
    QApplication app( argc, argv, FALSE );

    QStringList qdocFiles;
    QString opt;
    int i = 1;

    while ( i < argc ) {
	opt = argv[i++];

	if ( opt == "-help" ) {
	    printHelp();
	    return EXIT_SUCCESS;
	} else if ( opt == "-version" ) {
	    printVersion();
	    return EXIT_SUCCESS;
	} else if ( opt == "--" ) {
	    while ( i < argc )
		qdocFiles.append( argv[i++] );
	} else {
	    qdocFiles.append( opt );
	}
    }

    if ( qdocFiles.isEmpty() ) {
	printHelp();
	return EXIT_FAILURE;
    }

    QStringList::Iterator qf = qdocFiles.begin();
    while ( qf != qdocFiles.end() ) {
	processQdocFile( *qf );
	++qf;
    }
    return EXIT_SUCCESS;
}
