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
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtextstream.h>

#include <errno.h>
#include <metatranslator.h>
#include <proparser.h>

typedef QValueList<MetaTranslatorMessage> TML;

static void printUsage()
{
    fprintf( stderr, "Usage:\n"
	      "    lrelease [ options ] project-file\n"
	      "    lrelease [ options ] ts-files\n"
	      "Options:\n"
	      "    -help  Display this information and exit\n"
	      "    -verbose\n"
	      "           Explain what is being done\n"
	      "    -version\n"
	      "           Display the version of lrelease and exit\n" );
}

static void releaseQmFile( const QString& tsFileName, bool verbose )
{
    MetaTranslator tor;
    QString qmFileName = tsFileName;
    qmFileName.replace( QRegExp(QString("\\.ts$")), QString::null );
    qmFileName += QString( ".qm" );

    if ( tor.load(tsFileName) ) {
	if ( verbose )
	    fprintf( stderr, "Updating '%s'...\n", qmFileName.latin1() );
	if ( !tor.release(qmFileName, verbose) )
	    fprintf( stderr,
		     "lrelease warning: For some reason, I cannot save '%s'\n",
		     qmFileName.latin1() );
    } else {
	fprintf( stderr,
		 "lrelease warning: For some reason, I cannot load '%s'\n",
		 tsFileName.latin1() );
    }
}

int main( int argc, char **argv )
{
    bool verbose = FALSE;
    bool metTranslations = FALSE;
    int numFiles = 0;

    for ( int i = 1; i < argc; i++ ) {
	if ( qstrcmp(argv[i], "-help") == 0 ) {
	    printUsage();
	    return 0;
	} else if ( qstrcmp(argv[i], "-verbose") == 0 ) {
	    verbose = TRUE;
	    continue;
	} else if ( qstrcmp(argv[i], "-version") == 0 ) {
	    fprintf( stderr, "lrelease version %s\n", QT_VERSION_STR );
	    return 0;
	}

	numFiles++;
	QFile f( argv[i] );
	if ( !f.open(IO_ReadOnly) ) {
	    fprintf( stderr,
		     "lrelease error: Cannot open file '%s': %s\n", argv[i],
		     strerror(errno) );
	    return 1;
	}

	QTextStream t( &f );
	QString fullText = t.read();
	f.close();

	if ( fullText.find(QString("<!DOCTYPE TS>")) == -1 ) {
	    QMap<QString, QString> tagMap = proFileTagMap( fullText );
	    QMap<QString, QString>::Iterator it;

	    for ( it = tagMap.begin(); it != tagMap.end(); ++it ) {
        	QStringList toks = QStringList::split( QChar(' '), it.data() );
		QStringList::Iterator t;

        	for ( t = toks.begin(); t != toks.end(); ++t ) {
		    if ( it.key() == QString("TRANSLATIONS") ) {
			metTranslations = TRUE;
			releaseQmFile( *t, verbose );
		    }
		}
	    }
	    if ( !metTranslations )
		fprintf( stderr,
			 "lrelease warning: Met no 'TRANSLATIONS' entry in"
			 " project file '%s'\n",
			 argv[i] );
	} else {
	    releaseQmFile( QString(argv[i]), verbose );
	}
    }

    if ( numFiles == 0 ) {
	printUsage();
	return 1;
    }
    return 0;
}
