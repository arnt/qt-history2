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
#include "plaincodemarker.h"
#include "qscodemarker.h"
#include "qscodeparser.h"
#include "sgmlgenerator.h"
#include "tokenizer.h"
#include "tree.h"

static const struct {
    const char *key;
    const char *value;
} defaults[] = {
    { CONFIG_FALSEHOODS, "0" },
    { CONFIG_FORMATS, "HTML" },
    { CONFIG_LANGUAGE, "C++" },
    { CONFIG_TABSIZE, "8" },
    { 0, 0 }
};

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
    Location::information( tr("Usage: qdoc [options] file1.qdoc...\n"
			      "Options:\n"
			      "    -help  Display this information and exit\n"
			      "    -verbose\n"
			      "           Explain what is being done\n"
			      "    -version\n"
			      "           Display version of qdoc and exit") );
}

static void printVersion()
{
    Location::information( tr("qdoc version 3.0") );
}

static void processQdocFile( const QString& fileName )
{
    QPtrList<QTranslator> translators;
    translators.setAutoDelete( TRUE );

    Config config( tr("qdoc") );

    int i = 0;
    while ( defaults[i].key != 0 ) {
	config.setStringList( defaults[i].key,
			      QStringList() << defaults[i].value );
	i++;
    }

    Location::initialize( config );
    config.load( fileName );
    Location::terminate();

    QString prevCurrentDir = QDir::currentDirPath();
    QString dir = QFileInfo( fileName ).dirPath();
    if ( !dir.isEmpty() )
	QDir::setCurrent( dir );

    Location::initialize( config );
    Tokenizer::initialize( config );
    Doc::initialize( config );
    CodeMarker::initialize( config );
    CodeParser::initialize( config );
    Generator::initialize( config );

    QStringList fileNames = config.getStringList( CONFIG_TRANSLATORS );
    QStringList::Iterator fn = fileNames.begin();
    while ( fn != fileNames.end() ) {
	QTranslator *translator = new QTranslator( 0 );
	if ( !translator->load(*fn) )
	    config.lastLocation().error( tr("Cannot load translator '%1'")
					 .arg(*fn) );
	qApp->installTranslator( translator );
	translators.append( translator );
	++fn;
    }

    QString lang = config.getString( CONFIG_LANGUAGE );
    Tree *tree = treeForLanguage( lang );
    CodeParser *codeParser = CodeParser::parserForLanguage( lang );
    if ( codeParser == 0 )
	config.lastLocation().fatal( tr("Cannot parse language '%1'")
				     .arg(lang) );
    CodeMarker *marker = CodeMarker::markerForLanguage( lang );
    if ( marker == 0 )
	config.lastLocation().fatal( tr("Cannot output documentation for"
					" language '%1'")
				     .arg(lang) );

    QStringList headers = config.getAllFiles( CONFIG_HEADERS, CONFIG_HEADERDIRS,
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

    Set<QString> formats = config.getStringSet( CONFIG_FORMATS );
    Set<QString>::ConstIterator f = formats.begin();
    while ( f != formats.end() ) {
	Generator *generator = Generator::generatorForFormat( *f );
	if ( generator == 0 )
	    config.lastLocation().fatal( tr("Unknown documentation format '%1'")
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
