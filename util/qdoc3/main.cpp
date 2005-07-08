/*
  main.cpp
*/

#include <QtCore>

#include <stdlib.h>

#include "apigenerator.h"
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
    { CONFIG_LANGUAGE, "Cpp" },
    { CONFIG_OUTPUTFORMATS, "HTML" },
    { CONFIG_TABSIZE, "8" },
    { 0, 0 }
};

static QStringList defines;

static QHash<QString, Tree *> trees;

static Tree *treeForLanguage(const QString &lang)
{
    Tree *tree = trees.value(lang);
    if ( tree == 0 ) {
	tree = new Tree;
	trees.insert( lang, tree );
    }
    return tree;
}

static void printHelp()
{
    Location::information( tr("Usage: qdoc [options] file1.qdocconf ...\n"
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

static void processQdocconfFile(const QString &fileName)
{
    QList<QTranslator *> translators;

    Config config( tr("qdoc") );

    int i = 0;
    while (defaults[i].key) {
	config.setStringList(defaults[i].key, QStringList() << defaults[i].value);
	++i;
    }

    Location::initialize( config );
    config.load( fileName );
    config.setStringList(CONFIG_DEFINES, defines + config.getStringList(CONFIG_DEFINES));

    Location::terminate();

    QString prevCurrentDir = QDir::currentPath();
    QString dir = QFileInfo( fileName ).path();
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
	QCoreApplication::instance()->installTranslator( translator );
	translators.append( translator );
	++fn;
    }

    QString lang = config.getString(CONFIG_LANGUAGE);
    Location langLocation = config.lastLocation();

    Tree *tree = treeForLanguage(lang);
    tree->setVersion(config.getString(CONFIG_VERSION));
    CodeParser *codeParser = CodeParser::parserForLanguage( lang );
    if ( codeParser == 0 )
	config.lastLocation().fatal(tr("Cannot parse programming language '%1'").arg(lang));

    QSet<QString> outputFormats = config.getStringSet(CONFIG_OUTPUTFORMATS);
    Location outputFormatsLocation = config.lastLocation();

    CodeMarker *marker = CodeMarker::markerForLanguage(lang);
    if (!marker && !outputFormats.isEmpty())
	langLocation.fatal(tr("Cannot output documentation for programming language '%1'")
			   .arg(lang));

    QStringList indexFiles = config.getStringList(CONFIG_INDEXES);
    tree->readIndexes(indexFiles);

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
    tree->resolveGroups();
    tree->resolveTargets();

    QSet<QString>::ConstIterator of = outputFormats.begin();
    while ( of != outputFormats.end() ) {
	Generator *generator = Generator::generatorForFormat( *of );
	if ( generator == 0 )
	    outputFormatsLocation.fatal(tr("Unknown output format '%1'").arg(*of));
	generator->generateTree( tree, marker );
	++of;
    }
    tree->setVersion("");

    Generator::terminate();
    CodeParser::terminate();
    CodeMarker::terminate();
    CppToQsConverter::terminate();
    Doc::terminate();
    Tokenizer::terminate();
    Location::terminate();
    QDir::setCurrent( prevCurrentDir );

    foreach (QTranslator *translator, translators)
	delete translator;
}

int main( int argc, char **argv )
{
    QCoreApplication app(argc, argv);

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

    ApiGenerator apiGenerator;
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
        } else if ( opt.startsWith("-D") ) {
            QString define = opt.mid(2);
            defines += define;
	} else {
	    qdocFiles.append( opt );
	}
    }

    if ( qdocFiles.isEmpty() ) {
	printHelp();
	return EXIT_FAILURE;
    }

    foreach (QString qf, qdocFiles)
	processQdocconfFile( qf );

    qDeleteAll(trees);
    return EXIT_SUCCESS;
}
