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

typedef QValueList<MetaTranslatorMessage> TML;

/*
  This utility reads a project file and creates a stripped version of all the
  message files specified there.

  For example, if the project file contains the lines

TRANSLATIONS	= funnyapp_fr.ts \
		  funnyapp_no.xyz

  then funnyapp_fr.ts and funnyapp_no.xyz are converted into funnyapp_fr.qm and
  funnyapp_no.xyz.qm (sic).
*/

int main( int argc, char **argv )
{
    if ( argc != 2 ) {
	qWarning( "Usage:\n    lrelease file.pro" );
	return 1;
    }

    QFile f( argv[1] );
    if ( !f.open(IO_ReadOnly) ) {
	qWarning( "lrelease error: cannot open project file '%s': %s", argv[1],
		  strerror(errno) );
	return 1;
    }

    QTextStream t( &f );
    QString fullText = t.read();
    f.close();

    /*
      Strip comments, merge lines ending with backslash, add spaces around '=',
      replace '\n' with ';', and simplify white spaces.
    */
    fullText.replace( QRegExp(QString("#[^\n]$")), QString(" ") );
    fullText.replace( QRegExp(QString("\\\\\\s*\n")), QString(" ") );
    fullText.replace( QRegExp(QString("=")), QString(" = ") );
    fullText.replace( QRegExp(QString("\n")), QString(";") );
    fullText = fullText.simplifyWhiteSpace();

    QStringList lines = QStringList::split( QChar(';'), fullText );
    QStringList::Iterator line;
    for ( line = lines.begin(); line != lines.end(); ++line ) {
	QStringList toks = QStringList::split( QChar(' '), *line );

	if ( toks.count() >= 3 && toks[1] == QString("=") ) {
	    if ( toks.first() == QString("TRANSLATIONS") ) {
		QStringList::Iterator t;
		for ( t = toks.at(2); t != toks.end(); ++t ) {
		    MetaTranslator tor;
		    QString f = *t;
		    QString g = *t;
		    g.replace( QRegExp(QString(".ts$")), QString("") );
		    g += QString( ".qm" );
		    if ( tor.load(f) ) {
			if ( !tor.release(g) )
			    qWarning( "lrelease warning: For some reason, I"
				      " cannot save '%s'", g.latin1() );
		    } else {
			qWarning( "lrelease warning: For some reason, I cannot"
				  " load '%s'", f.latin1() );
		    }
		}
	    }
	}
    }
    return 0;
}
