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

/*
  This utility converts unstripped message files (.qm files) into source message
  files (.ts files).  The .qm files to convert are expected as command-line
  arguments.
*/

int main( int argc, char **argv )
{
    if ( argc < 2 ) {
	qWarning( "Usage:\n    qm2ts file.qm..." );
	return 1;
    }

    for ( int i = 1; i < argc; i++ ) {
	QTranslator tor( 0 );
	if ( tor.load(argv[i], ".") ) {
	    MetaTranslator metator;
	    TML all = tor.messages();
	    TML::Iterator it;
	    for ( it = all.begin(); it != all.end(); ++it ) {
		metator.insert( MetaTranslatorMessage((*it).context(),
				(*it).sourceText(), (*it).comment(),
				(*it).translation(),
				MetaTranslatorMessage::Finished) );
	    }

	    QString g = argv[i];
	    g.replace( QRegExp(QString(".qm$")), QString::null );
	    g += QString( ".ts" );
	    if ( !metator.save(g) )
		qWarning( "qm2ts warning: For some reason, I cannot save '%s'",
			  g.latin1() );
	} else {
	    qWarning( "qm2ts warning: For some reason, I cannot load '%s'",
		      argv[i] );
	}
    }
}
