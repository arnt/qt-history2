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
    qWarning( "Usage: lrelease [options] file.pro...\n"
	      "Options:\n"
	      "    -help  Display this information and exits\n"
	      "    -verbose\n"
	      "           Explain what is being done\n"
	      "    -version\n"
	      "           Display the version of lrelease and exits" );
}

int main( int argc, char **argv )
{
    bool verbose = FALSE;
    bool metTranslations = FALSE;
    int numProFiles = 0;

    for ( int i = 1; i < argc; i++ ) {
	if ( qstrcmp(argv[i], "-help") == 0 ) {
	    printUsage();
	    return 0;
	} else if ( qstrcmp(argv[i], "-verbose") == 0 ) {
	    verbose = TRUE;
	    continue;
	} else if ( qstrcmp(argv[i], "-version") == 0 ) {
	    qWarning( "lrelease version %s", QT_VERSION_STR );
	    return 0;
	}

	numProFiles++;
	QFile f( argv[i] );
	if ( !f.open(IO_ReadOnly) ) {
	    qWarning( "lrelease error: Cannot open project file '%s': %s",
		      argv[i], strerror(errno) );
	    return 1;
	}

	QTextStream t( &f );
	QString fullText = t.read();
	f.close();

	QMap<QString, QString> tagMap = proFileTagMap( fullText );
	QMap<QString, QString>::Iterator it;

	for ( it = tagMap.begin(); it != tagMap.end(); ++it ) {
            QStringList toks = QStringList::split( QChar(' '), it.data() );
	    QStringList::Iterator t;

            for ( t = toks.begin(); t != toks.end(); ++t ) {
		if ( it.key() == QString("TRANSLATIONS") ) {
		    metTranslations = TRUE;

		    MetaTranslator tor;
		    QString f = *t;
		    QString g = *t;
		    g.replace( QRegExp(QString(".ts$")), QString("") );
		    g += QString( ".qm" );

		    if ( tor.load(f) ) {
			if ( verbose )
			    qWarning( "Updating '%s'...", g.latin1() );
			if ( !tor.release(g, verbose) )
			    qWarning( "lrelease warning: For some reason, I"
				      " cannot save '%s'", g.latin1() );
		    } else {
			qWarning( "lrelease warning: For some reason, I"
				  " cannot load '%s'", f.latin1() );
		    }
		}
	    }
	}
	if ( !metTranslations )
	    qWarning( "lrelease warning: Met no 'TRANSLATIONS' entry in"
		      " project file '%s'", argv[i] );
    }

    if ( numProFiles == 0 ) {
	printUsage();
	return 1;
    }
    return 0;
}
