/*
  main.cpp
*/

#include <qapplication.h>
#include <qdict.h>
#include <qdir.h>

#include "codemarker.h"
#include "codeparser.h"
#include "config.h"
#include "cppcodemarker.h"
#include "cppcodeparser.h"
#include "doc.h"
#include "htmlgenerator.h"
#include "loutgenerator.h"
#include "mangenerator.h"
#include "messages.h"
#include "plaincodemarker.h"
#include "qscodemarker.h"
#include "qscodeparser.h"
#include "sgmlgenerator.h"
#include "tokenizer.h"
#include "tree.h"

static QDict<Tree> trees;
static QPtrList<CodeParser> parsers;
static QPtrList<CodeMarker> markers;
static QPtrList<Generator> generators;

static Tree *treeForLanguage( const QString& lang )
{
    Tree *tree = trees[lang];
    if ( tree == 0 ) {
	tree = new Tree;
	trees.insert( lang, tree );
    }
    return tree;
}

static void printHelp()
{
    Messages::information(
	    Qdoc::tr("Usage: qdoc [options] file1.qdoc...\n"
		     "Options:\n"
		     "    -help  Display this information and exit\n"
		     "    -verbose\n"
		     "           Explain what is being done\n"
		     "    -version\n"
		     "           Display version of qdoc and exit") );
}

static void printVersion()
{
    Messages::information( Qdoc::tr("qdoc version 3.0") );
}

static void processQdocFile( const QString& fileName )
{
    QPtrList<QTranslator> translators;
    translators.setAutoDelete( TRUE );

    Config config( Qdoc::tr("qdoc") );

    Messages::initialize( config );
    config.load( fileName );
    Messages::terminate();

    QString prevCurrentDir = QDir::currentDirPath();
    QString dir = QFileInfo( fileName ).dirPath();
    if ( !dir.isEmpty() )
	QDir::setCurrent( dir );

    Messages::initialize( config );
    Location::initialize( config );
    Tokenizer::initialize( config );
    Doc::initialize( config );
    CodeMarker::initialize( config );
    CodeParser::initialize( config );
    Generator::initialize( config );

    QString lang = config.getString( CONFIG_LANGUAGE );
    Tree *tree = treeForLanguage( lang );

    QStringList fileNames = config.getStringList( CONFIG_TRANSLATORS );
    QStringList::Iterator fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	QTranslator *translator = new QTranslator( 0 );
	if ( !translator->load(*fn) )
	    Messages::error( config.location(),
			     Qdoc::tr("Cannot load translator '%1'")
			     .arg(*fn) );
	qApp->installTranslator( translator );
	translators.append( translator );
	++fn;
    }

    CodeParser *codeParser = CodeParser::parserForLanguage( lang );
    if ( codeParser == 0 )
	Messages::fatal( config.location(),
			 Qdoc::tr("Cannot parse language '%1'").arg(lang) );

    QStringList headers = config.getAllFiles( CONFIG_HEADERS, CONFIG_HEADERDIRS,
					      "*.h" );
    if ( headers.isEmpty() )
	headers = config.getAllFiles( CONFIG_SOURCES, CONFIG_SOURCEDIRS,
				      "*.h" );
    QStringList::ConstIterator h = headers.begin();
    while ( h != headers.end() ) {
	codeParser->parseHeaderFile( config.location(), *h, tree );
	++h;
    }

    QStringList sources = config.getAllFiles( CONFIG_SOURCES, CONFIG_SOURCEDIRS,
					      "*.cpp" );
    QStringList::ConstIterator s = sources.begin();
    while ( s != sources.end() ) {
	codeParser->parseSourceFile( config.location(), *s, tree );
	++s;
    }

    CodeMarker *marker = CodeMarker::markerForLanguage( lang );
    if ( marker == 0 )
	Messages::fatal( config.location(),
			 Qdoc::tr("Cannot output documentation for"
				  " language '%1'")
			 .arg(lang) );

    Set<QString> formats = config.getStringSet( CONFIG_FORMATS );
    Set<QString>::ConstIterator f = formats.begin();
    while ( f != formats.end() ) {
	Generator *generator = Generator::generatorForFormat( *f );
	if ( generator == 0 )
	    Messages::fatal( config.location(),
			     Qdoc::tr("Unknown documentation format '%1'")
			     .arg(*f) );
	generator->generateTree( tree, marker );
	++f;
    }

    Generator::terminate();
    CodeParser::terminate();
    CodeMarker::terminate();
    Doc::terminate();
    Tokenizer::terminate();
    Location::terminate();
    Messages::terminate();
    QDir::setCurrent( prevCurrentDir );
}

int main( int argc, char **argv )
{
    QApplication app( argc, argv, FALSE );

    CodeParser *cppParser = new CppCodeParser;
    parsers.append( cppParser );
    parsers.append(
	    new QsCodeParser(treeForLanguage(cppParser->language())) );

    markers.append( new PlainCodeMarker );
    markers.append( new CppCodeMarker );
    markers.append( new QsCodeMarker );

    generators.append( new HtmlGenerator );
    generators.append( new LoutGenerator );
    generators.append( new ManGenerator );
    generators.append( new SgmlGenerator );

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

    trees.setAutoDelete( TRUE );
    trees.clear();
    parsers.setAutoDelete( TRUE );
    parsers.clear();
    markers.setAutoDelete( TRUE );
    markers.clear();
    generators.setAutoDelete( TRUE );
    generators.clear();

    return EXIT_SUCCESS;
}
