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
#include <string.h>

// defined in fetchtr.cpp
extern void fetchtr_cpp( const char *name, MetaTranslator *tor,
			 const char *defaultContext );
extern void fetchtr_ui( const char *name, MetaTranslator *tor,
			 const char *defaultContext );

// defined in merge.cpp
extern void merge( MetaTranslator *tor, const MetaTranslator *virginTor );

typedef QValueList<MetaTranslatorMessage> TML;

/*
  This utility reads a project file, extracts the strings to be translated, and
  updates the message files.

  For example, if the project file contains the lines

HEADERS		= funnydialog.h
SOURCES		= funnydialog.cpp \
		  main.cpp
INTERFACES	= help.ui
TRANSLATIONS	= funnyapp_fr.ts \
		  funnyapp_jp.ts

  strings are extracted from funnydialog.h, funnydialog.cpp, main.cpp, and
  help.ui, and are merged with the message files funnyapp_fr.ts and
  funnyapp_jp.ts.
*/

int main( int argc, char **argv )
{
    if ( argc != 2 ) {
	qWarning( "Usage:\n    lupdate file.pro" );
	return 1;
    }

    QFile f( argv[1] );
    if ( !f.open(IO_ReadOnly) ) {
	qWarning( "lupdate error: cannot open project file '%s': %s", argv[1],
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

    MetaTranslator fetchedTor;
    QString defaultContext = "@default";
    QCString codec;
    QStringList translatorFiles;
    QStringList::Iterator tf;

    QStringList lines = QStringList::split( QChar(';'), fullText );
    QStringList::Iterator line;
    for ( line = lines.begin(); line != lines.end(); ++line ) {
	QStringList toks = QStringList::split( QChar(' '), *line );

	if ( toks.count() >= 3 && toks[1] == QString("=") ) {
	    QStringList::Iterator t;
	    for ( t = toks.at(2); t != toks.end(); ++t ) {
		if ( toks.first() == QString("HEADERS") ||
		     toks.first() == QString("SOURCES") )
		    fetchtr_cpp( *t, &fetchedTor, defaultContext );
		else if ( toks.first() == QString("INTERFACES") )
		    fetchtr_ui( *t, &fetchedTor, defaultContext );
		else if ( toks.first() == QString("TRANSLATIONS") )
		    translatorFiles.append( *t );
		else if ( toks.first() == QString("CODEC") )
		    codec = (*t).latin1();
	    }
	}
    }
    for ( tf = translatorFiles.begin(); tf != translatorFiles.end(); ++tf ) {
	MetaTranslator tor;
	tor.load( *tf );
	if ( !codec.isEmpty() )
	    tor.setCodec( codec );
	merge( &tor, &fetchedTor );
	if ( !tor.save(*tf) )
	    qWarning( "lupdate error: cannot save '%s': %s", (*tf).latin1(),
		      strerror(errno) );
    }
    return 0;
}
