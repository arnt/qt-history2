/****************************************************************************
** $Id: //depot/qt/main/util/qtranslator/msg2qm.cpp#1 $
**
** This is a utility program for converting findtr msgfiles to
** qtranslator message files
**
** Author  : Matthias Ettrich
** Created : 982402
**
** Copyright (C) 1998 by Trolltech AS.  All rights reserved.
**
*****************************************************************************/
#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qtranslator.h>
#include <stdlib.h>

#include "mergetr.h"

static QString* defaultScope = 0;

// bool hasHandle( const QString& line, const QString& handle)
// {
//     return line.left(handle.length()) == handle;
// }


// QString extractContents( const QString& line )
// {
//     QString contents;
//     if ( line.contains('\"') < 2)
// 	return contents;
//     int pos = 0;
//     while ( pos < int(line.length()) && line[pos] != '\"' )
// 	pos++;
//     pos++;
//     while ( pos < int(line.length()) && line[pos] != '\"' ) {
// 	if ( line[pos] == '\\') {
// 	    pos++;
// 	    switch (char(line[pos]) ) {
// 	    case 'n':
// 		contents += '\n';
// 		break;
// 	    case 't':
// 		contents += '\t';
// 		break;
// 	    case 'r':
// 		contents += '\r';
// 		break;
// 	    case 'a':
// 		contents += '\a';
// 		break;
// 	    case 'f':
// 		contents += '\f';
// 		break;
// 	    case 'v':
// 		contents += '\v';
// 		break;
// 	    case 'b':
// 		contents += '\b';
// 		break;
// 	    default:
// 		contents += char(line[pos]);
// 		break;
// 	    }
// 	}
// 	else
// 	    contents += line[pos];
// 	pos++;
//     }
//     return contents;
// }


void addTranslation( QTranslator* translator, const QString& msgid, const QString& msgstr)
{
    if (!msgid.isNull() && !msgstr.isNull() ) {
	QString scope = "";
	QString id = msgid;
	int coloncolon = msgid.find("::");
	if (coloncolon != -1) {
	    scope = msgid.left( coloncolon );
	    id = msgid.right( msgid.length() - scope.length() - 2 );
	}
	else if (defaultScope)
	    scope = *defaultScope;

	//int hash = translator->hash( scope.ascii(), id.ascii() );
	if (translator->contains( /*hash,*/ scope.ascii(), id.ascii() ) ) {
	    qWarning("Error: \"%s\" already in use", msgid.ascii() );
	}
	else {
	    translator->insert( scope.latin1(), id.latin1(), msgstr );
	    //debug("'%s':'%s'-->'%s'", scope.ascii(), id.ascii(), msgstr.ascii() );
	}
    }
}



void translate( const QString& filename, const QString& qmfile )
{
    //we start in locale-specific mode, thenswitch when we find the char set

    bool firstRound = TRUE;

    QFile f(filename);
    if ( !f.open( IO_ReadOnly) )
	return;
    QTranslator* translator = new QTranslator(0);
    QTextStream t( &f );
    QString line;
    QString msgid;
    QString msgstr;
    while ( !t.atEnd() || !line.isEmpty() ) {
	if (line.isEmpty()) {
	    t.skipWhiteSpace();
	    line = t.readLine();
	}
	if ( hasHandle( line, "msgid") ) {
	    msgstr = QString::null;
	    msgid = extractContents( line );
	    if (!t.atEnd()) {
		t.skipWhiteSpace();
		line = t.readLine();
	    }
	    else
		line = QString::null;
	    while ( hasHandle( line, "\"") ) {
		msgid += extractContents( line );
		if (!t.atEnd()) {
		    t.skipWhiteSpace();
		    line = t.readLine();
		}
		else
		    line = QString::null;
	    }
	}
	else if ( hasHandle( line, "msgstr") ) {
	    msgstr = extractContents( line );
	    if (!t.atEnd()) {
		t.skipWhiteSpace();
		line = t.readLine();
	    }
	    else
		line = QString::null;
	    while ( hasHandle( line, "\"") ) {
		msgstr += extractContents( line );
		if (!t.atEnd()) {
		    t.skipWhiteSpace();
		    line = t.readLine();
		}
		else
		    line = QString::null;
	    }
	    //debug("%s --> %s", msgid.ascii(), msgstr.ascii() );
	    addTranslation( translator, msgid, msgstr);
	    if ( firstRound && msgid.isEmpty() ) {
		// Check for the encoding. We are not supposed to
		// change encodings in mid-stream, but it is reasonably safe
		// when we are doing just readLine().
		//###### should be done better in the final version.
		int cpos = msgstr.find( "charset=" );
		if ( cpos >= 0 ) {
		    cpos = cpos + 8; //skip "charset="
		    int i = cpos;
		    int len = msgstr.length();
		    while ( i < len && !msgstr[i].isSpace() )
			i++;
		    QString charset = msgstr.mid( cpos, i-cpos );
		    QTextCodec *codec = QTextCodec::codecForName( charset.ascii() );
		    if ( codec ) {
		        debug( "PO file character set: %s. Codec: %s",
			       charset.ascii(), codec->name() );
			t.setCodec( codec );		
		    } else {
			debug( "No codec for %s", charset.ascii() );
		    }
		}
	    }
	    firstRound = FALSE;
	}
	else
	    line = QString::null;
    }
    f.close();
    translator->save( qmfile );
}


// int main( int argc, char* argv[] )
// {

//     int infile = 1;
//     if (argc > 1) {
// 	if ( QString("-scope") == argv[1] ) {
// 	    defaultScope = new QString(argv[2]);
// 	    infile += 2;
// 	}
//     }

//     if ( argc <= infile ) {
// 	qDebug("usage: %s [-scope default] infile [outfile]", argv[0]);
// 	exit(1);
//     }

//     translate(argv[infile], argc > infile+1 ? argv[infile+1] : "tr.qm");
//     return 0;
// }
