/*
  main.cpp
*/

#include <qapplication.h>
#include <qdict.h>
#include <qdir.h>
#include <qtranslator.h>

#include "ccodeparser.h"
#include "codemarker.h"
#include "codeparser.h"
#include "config.h"
#include "cppcodemarker.h"
#include "cppcodeparser.h"
#include "cpptoqsconverter.h"
#include "doc.h"
#include "htmlgenerator.h"
#include "loutgenerator.h"
#include "mangenerator.h"
#include "plaincodemarker.h"
#include "polyarchiveextractor.h"
#include "polyuncompressor.h"
#include "qsakernelparser.h"
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
    { CONFIG_LANGUAGE, "C++" },
    { CONFIG_OUTPUTFORMATS, "HTML" },
    { CONFIG_TABSIZE, "8" },
    { 0, 0 }
};

static QDict<Tree> trees;

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
    QList<QTranslator *> translators;

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
    CppToQsConverter::initialize( config );
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

    Set<QString> outputFormats = config.getStringSet( CONFIG_OUTPUTFORMATS );

    CodeMarker *marker = CodeMarker::markerForLanguage( lang );
    if ( marker == 0 && !outputFormats.isEmpty() )
	config.lastLocation().fatal( tr("Cannot output documentation for"
					" language '%1'")
				     .arg(lang) );

    QStringList headers =
	    config.getAllFiles( CONFIG_HEADERS, CONFIG_HEADERDIRS,
				codeParser->headerFileNameFilter() );
    QStringList::ConstIterator h = headers.begin();
    while ( h != headers.end() ) {
	codeParser->parseHeaderFile( config.location(), *h, tree );
	++h;
    }
    codeParser->doneParsingHeaderFiles( tree );

    QStringList sources =
	    config.getAllFiles( CONFIG_SOURCES, CONFIG_SOURCEDIRS,
				codeParser->sourceFileNameFilter() );
    QStringList::ConstIterator s = sources.begin();
    while ( s != sources.end() ) {
	codeParser->parseSourceFile( config.location(), *s, tree );
	++s;
    }
    codeParser->doneParsingSourceFiles( tree );

    Set<QString>::ConstIterator of = outputFormats.begin();
    while ( of != outputFormats.end() ) {
	Generator *generator = Generator::generatorForFormat( *of );
	if ( generator == 0 )
	    config.lastLocation().fatal( tr("Unknown output format '%1'")
					 .arg(*of) );
	generator->generateTree( tree, marker );
	++of;
    }

    Generator::terminate();
    CodeParser::terminate();
    CodeMarker::terminate();
    CppToQsConverter::terminate();
    Doc::terminate();
    Tokenizer::terminate();
    Location::terminate();
    QDir::setCurrent( prevCurrentDir );

    Q_FOREACH (QTranslator *translator, translators)
	delete translator;
}

int main( int argc, char **argv )
{
    QApplication app( argc, argv, FALSE );

    trees.setAutoDelete( TRUE );

    PolyArchiveExtractor qsaExtractor( QStringList() << "qsa",
				       "qsauncompress \1 \2" );
    PolyArchiveExtractor tarExtractor( QStringList() << "tar",
				       "tar -C \2 -xf \1" );
    PolyArchiveExtractor tazExtractor( QStringList() << "taz",
				       "tar -C \2 -Zxf \1" );
    PolyArchiveExtractor tbz2Extractor( QStringList() << "tbz" << "tbz2",
					"tar -C \2 -jxf \1" );
    PolyArchiveExtractor tgzExtractor( QStringList() << "tgz",
				       "tar -C \2 -zxf \1" );
    PolyArchiveExtractor zipExtractor( QStringList() << "zip",
				       "unzip \1 -d \2" );

    PolyUncompressor bz2Uncompressor( QStringList() << "bz" << "bz2",
				      "bunzip2 -c \1 > \2" );
    PolyUncompressor gzAndZUncompressor( QStringList() << "gz" << "z" << "Z",
					 "gunzip -c \1 > \2" );
    PolyUncompressor zipUncompressor( QStringList() << "zip",
				      "unzip -c \1 > \2" );

    CCodeParser cParser;
    CppCodeParser cppParser;

    Tree *cppTree = treeForLanguage( cppParser.language() );

    QsCodeParser qsParser( cppTree );
    QsaKernelParser qsaKernelParser( cppTree );

    PlainCodeMarker plainMarker;
    CppCodeMarker cppMarker;
    QsCodeMarker qsMarker;

    HtmlGenerator htmlGenerator;
    LoutGenerator loutGenerator;
    ManGenerator manGenerator;
    SgmlGenerator smglGenerator;

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

    trees.clear();
    return EXIT_SUCCESS;
}
