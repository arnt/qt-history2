/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   main.cpp
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include <qfile.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include <errno.h>
#include <metatranslator.h>
#include <proparser.h>
#include <string.h>

// defined in fetchtr.cpp
extern void fetchtr_cpp( const char *fileName, MetaTranslator *tor,
			 const char *defaultContext, bool mustExist );
extern void fetchtr_ui( const char *fileName, MetaTranslator *tor,
			const char *defaultContext, bool mustExist );

// defined in merge.cpp
extern void merge( MetaTranslator *tor, const MetaTranslator *virginTor,
		   bool verbose );

typedef QValueList<MetaTranslatorMessage> TML;

static void printUsage()
{
    fprintf( stderr, "Usage:\n"
	     "    lupdate [ options ] project-file\n"
	     "    lupdate [ options ] source-files -ts ts-files\n"
	     "Options:\n"
	     "    -help  Display this information and exit\n"
	     "    -noobsolete\n"
	     "           Drop all obsolete strings\n"
	     "    -verbose\n"
	     "           Explain what is being done\n"
	     "    -version\n"
	     "           Display the version of lupdate and exit\n" );
}

static void updateTsFiles( const MetaTranslator& fetchedTor,
			   const QStringList& tsFileNames, const QString& codec,
			   bool noObsolete, bool verbose )
{
    QStringList::ConstIterator t = tsFileNames.begin();
    while ( t != tsFileNames.end() ) {
	MetaTranslator tor;
	tor.load( *t );
	if ( !codec.isEmpty() )
	    tor.setCodec( codec );
	if ( verbose )
	    fprintf( stderr, "Updating '%s'...\n", (*t).latin1() );
	merge( &tor, &fetchedTor, verbose );
	if ( noObsolete )
	    tor.stripObsoleteMessages();
	tor.stripEmptyContexts();
	if ( !tor.save(*t) )
	    fprintf( stderr, "lupdate error: Cannot save '%s': %s\n",
		     (*t).latin1(), strerror(errno) );
	++t;
    }
}

int main( int argc, char **argv )
{
    QString defaultContext( "@default" );
    MetaTranslator fetchedTor;
    QCString codec;
    QStringList tsFileNames;

    bool verbose = FALSE;
    bool noObsolete = FALSE;
    bool metSomething = FALSE;
    int numFiles = 0;
    bool standardSyntax = TRUE;
    bool metTsFlag = FALSE;

    for ( int i = 1; i < argc; i++ ) {
	if ( qstrcmp(argv[i], "-ts") == 0 )
	    standardSyntax = FALSE;
    }

    for ( int i = 1; i < argc; i++ ) {
	if ( qstrcmp(argv[i], "-help") == 0 ) {
	    printUsage();
	    return 0;
	} else if ( qstrcmp(argv[i], "-noobsolete") == 0 ) {
	    noObsolete = TRUE;
	    continue;
	} else if ( qstrcmp(argv[i], "-verbose") == 0 ) {
	    verbose = TRUE;
	    continue;
	} else if ( qstrcmp(argv[i], "-version") == 0 ) {
	    fprintf( stderr, "lupdate version %s\n", QT_VERSION_STR );
	    return 0;
	} else if ( qstrcmp(argv[i], "-ts") == 0 ) {
	    metTsFlag = TRUE;
	    continue;
	}

	numFiles++;

	QString fullText;

	if ( !metTsFlag ) {
	    QFile f( argv[i] );
	    if ( !f.open(IO_ReadOnly) ) {
		fprintf( stderr, "lupdate error: Cannot open file '%s': %s\n",
			 argv[i], strerror(errno) );
		return 1;
	    }

	    QTextStream t( &f );
	    fullText = t.read();
	    f.close();
	}

	if ( standardSyntax ) {
	    fetchedTor = MetaTranslator();
	    codec.truncate( 0 );
	    tsFileNames.clear();

	    QMap<QString, QString> tagMap = proFileTagMap( fullText );
	    QMap<QString, QString>::Iterator it;

	    for ( it = tagMap.begin(); it != tagMap.end(); ++it ) {
        	QStringList toks = QStringList::split( QChar(' '), it.data() );
		QStringList::Iterator t;

        	for ( t = toks.begin(); t != toks.end(); ++t ) {
                    if ( it.key() == QString("HEADERS") ||
                	 it.key() == QString("SOURCES") ) {
                	fetchtr_cpp( *t, &fetchedTor, defaultContext, TRUE );
                	metSomething = TRUE;
                    } else if ( it.key() == QString("INTERFACES") ||
				it.key() == QString("FORMS") ) {
                	fetchtr_ui( *t, &fetchedTor, defaultContext, TRUE );
			fetchtr_cpp( *t + QString(".h"), &fetchedTor,
				     defaultContext, FALSE );
                	metSomething = TRUE;
                    } else if ( it.key() == QString("TRANSLATIONS") ) {
                	tsFileNames.append( *t );
                	metSomething = TRUE;
                    } else if ( it.key() == QString("CODEC") ) {
                	codec = (*t).latin1();
                    }
        	}
            }

	    updateTsFiles( fetchedTor, tsFileNames, codec, noObsolete,
			   verbose );

	    if ( !metSomething ) {
		fprintf( stderr,
			 "lupdate warning: File '%s' does not look like a"
			 " project file\n",
			 argv[i] );
	    } else if ( tsFileNames.isEmpty() ) {
		fprintf( stderr,
			 "lupdate warning: Met no 'TRANSLATIONS' entry in"
			 " project file '%s'\n",
			 argv[i] );
	    }
	} else {
	    if ( metTsFlag ) {
		tsFileNames.append( QString(argv[i]) );
	    } else {
		if ( fullText.find(QString("<!DOCTYPE UI>")) == -1 ) {
        	    fetchtr_cpp( QString(argv[i]), &fetchedTor, defaultContext,
				 TRUE );
		} else {
        	    fetchtr_ui( QString(argv[i]), &fetchedTor, defaultContext,
				TRUE );
		    fetchtr_cpp( QString(argv[i]) + QString(".h"), &fetchedTor,
				 defaultContext, FALSE );
		}
	    }
	}
    }

    if ( !standardSyntax )
	updateTsFiles( fetchedTor, tsFileNames, codec, noObsolete, verbose );

    if ( numFiles == 0 ) {
	printUsage();
	return 1;
    }
    return 0;
}
