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
    qWarning( "Usage: lupdate [options] file.pro...\n"
	      "Options:\n"
	      "    -help  Display this information and exits\n"
	      "    -noobsolete\n"
	      "           Drop all obsolete strings\n"
	      "    -verbose\n"
	      "           Explains what is being done\n"
	      "    -version\n"
	      "           Display the version of lupdate and exits" );
}

int main( int argc, char **argv )
{
    bool verbose = FALSE;
    bool noObsolete = FALSE;
    bool metSomething = FALSE;
    int numProFiles = 0;

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
	    qWarning( "lupdate version %s", QT_VERSION_STR );
	    return 0;
	}

	numProFiles++;
	QFile f( argv[i] );
	if ( !f.open(IO_ReadOnly) ) {
	    qWarning( "lupdate error: Cannot open project file '%s': %s",
		      argv[i], strerror(errno) );
	    return 1;
	}

	QTextStream t( &f );
	QString fullText = t.read();
	f.close();

	MetaTranslator fetchedTor;
	QString defaultContext = "@default";
	QCString codec;
	QStringList translatorFiles;
	QStringList::Iterator tf;

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
                    translatorFiles.append( *t );
                    metSomething = TRUE;
                } else if ( it.key() == QString("CODEC") ) {
                    codec = (*t).latin1();
                }
            }
        }

	for ( tf = translatorFiles.begin(); tf != translatorFiles.end(); ++tf ) {
	    MetaTranslator tor;
	    tor.load( *tf );
	    if ( !codec.isEmpty() )
		tor.setCodec( codec );
	    if ( verbose )
		qWarning( "Updating '%s'...", (*tf).latin1() );
	    merge( &tor, &fetchedTor, verbose );
	    if ( noObsolete )
		tor.stripObsoleteMessages();
	    tor.stripEmptyContexts();
	    if ( !tor.save(*tf) )
		qWarning( "lupdate error: Cannot save '%s': %s", (*tf).latin1(),
			  strerror(errno) );
	}
	if ( !metSomething ) {
	    qWarning( "lupdate warning: File '%s' does not look like a project"
		      " file", argv[i] );
	} else if ( translatorFiles.isEmpty() ) {
	    qWarning( "lupdate warning: Met no 'TRANSLATIONS' entry in project"
		      " file '%s'", argv[i] );
	}
    }

    if ( numProFiles == 0 ) {
	printUsage();
	return 1;
    }
    return 0;
}
