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

#include <qregexp.h>
#include <qstring.h>
#include <qtranslator.h>

#include <metatranslator.h>

typedef QValueList<QTranslatorMessage> TML;

static void printUsage()
{
    fprintf( stderr, "Usage:\n"
	     "    qm2ts [ options ] qm-files\n"
	     "Options:\n"
	     "    -help  Display this information and exit\n"
	     "    -verbose\n"
	     "           Explain what is being done\n"
	     "    -version\n"
	     "           Display the version of qm2ts and exit\n" );
}

int main( int argc, char **argv )
{
    bool verbose = FALSE;
    int numQmFiles = 0;

    for ( int i = 1; i < argc; i++ ) {
	if ( qstrcmp(argv[i], "-help") == 0 ) {
	    printUsage();
	    return 0;
	} else if ( qstrcmp(argv[i], "-verbose") == 0 ) {
	    verbose = TRUE;
	    continue;
	} else if ( qstrcmp(argv[i], "-version") == 0 ) {
	    fprintf( stderr, "qm2ts version %s\n", QT_VERSION_STR );
	    return 0;
	}

	numQmFiles++;
	QTranslator tor( 0 );
	if ( tor.load(argv[i], ".") ) {
	    QString g = argv[i];
	    g.replace( QRegExp(QString("\\.qm$")), QString::null );
	    g += QString( ".ts" );

	    if ( verbose )
		fprintf( stderr, "Generating '%s'...\n", g.latin1() );

	    MetaTranslator metator;
	    int ignored = 0;

	    TML all = tor.messages();
	    TML::Iterator it;
	    for ( it = all.begin(); it != all.end(); ++it ) {
		if ( (*it).context() == 0 || (*it).sourceText() == 0 ) {
		    ignored++;
		} else {
		    metator.insert( MetaTranslatorMessage((*it).context(),
				    (*it).sourceText(), (*it).comment(),
				    (*it).translation(),
				    MetaTranslatorMessage::Finished) );
		}
	    }

	    if ( !metator.save(g) ) {
		fprintf( stderr,
			 "qm2ts warning: For some reason, I cannot save '%s'\n",
			 g.latin1() );
	    } else {
		if ( verbose ) {
		    int converted = (int) metator.messages().count();
		    fprintf( stderr, " %d converted message%s (%d ignored)\n",
			     converted, converted == 1 ? "" : "s", ignored );
		}
		if ( ignored > 0 )
		    fprintf( stderr,
			     "qm2ts warning: File '%s' is not a Qt 2.x .qm"
			     " file (some information is lost)\n",
			     argv[i] );
	    }
	} else {
	    fprintf( stderr,
		     "qm2ts warning: For some reason, I cannot load '%s'\n",
		     argv[i] );
	}
    }

    if ( numQmFiles == 0 ) {
	printUsage();
	return 1;
    }
    return 0;
}
