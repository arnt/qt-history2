/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of Qt Linguist.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "proparser.h"

#include <qregexp.h>
#include <qstringlist.h>

QMap<QString, QString> proFileTagMap( const QString& text )
{
    QString t = text;

    /*
      Strip comments, merge lines ending with backslash, add
      spaces around '=' and '+=', replace '\n' with ';', and
      simplify white spaces.
    */
    t.replace( QRegExp(QString("#[^\n]*\n")), QString(" ") );
    t.replace( QRegExp(QString("\\\\\n")), QString(" ") );
    t.replace( "=", QString(" = ") );
    t.replace( "+ =", QString(" += ") );
    t.replace( "\n", QString(";") );
    t = t.simplifyWhiteSpace();

    QMap<QString, QString> tagMap;

    QStringList lines = QStringList::split( QChar(';'), t );
    QStringList::Iterator line;
    for ( line = lines.begin(); line != lines.end(); ++line ) {
	QStringList toks = QStringList::split( QChar(' '), *line );

	if ( toks.count() >= 3 &&
	     (toks[1] == QString("=") || toks[1] == QString("+=")) ) {
	    QString tag = toks.first();
	    int k = tag.findRev( QChar(':') ); // as in 'unix:'
	    if ( k != -1 )
		tag = tag.mid( k + 1 );
	    toks.remove( toks.begin() );

	    QString action = toks.first();
	    toks.remove( toks.begin() );

	    if ( tagMap.contains(tag) ) {
		if ( action == QString("=") )
		    tagMap.replace( tag, toks.join(" ") );
		else
		    tagMap[tag] += QChar( ' ' ) + toks.join( " " );
	    } else {
		tagMap[tag] = toks.join( " " );
	    }
	}
    }

    QRegExp var( "\\$\\$[a-zA-Z0-9_]+" );
    QMap<QString, QString>::Iterator it;
    for ( it = tagMap.begin(); it != tagMap.end(); ++it ) {
	int i = 0;

	while ( (i = var.search(it.data(), i)) != -1 ) {
	    int len = var.matchedLength();
	    QString invocation = (*it).mid( i + 2, len - 2 );
	    QString after;
	    if ( tagMap.contains(invocation) )
		after = tagMap[invocation];
	    (*it).replace( i, len, after );
	    i += after.length();
	}
    }
    return tagMap;
}
