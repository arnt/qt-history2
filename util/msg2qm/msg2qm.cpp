/****************************************************************************
** $Id: //depot/qt/main/util/msg2qm/msg2qm.cpp#2 $
**
** This is a utility program for converting findtr msgfiles to
** qtranslator messagefiles
**
** Author  : Matthias Ettrich
** Created : 982402
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qtranslator.h>


bool hasHandle( const QString& line, const QString& handle)
{
    return line.left(handle.length()) == handle;
}


QString extractContents( const QString& line )
{
    QString contents;
    if ( line.contains('\"') < 2)
	return contents;
    unsigned int pos = 0;
    while ( pos < line.length() && line[pos] != '\"' )
	pos++;
    pos++;
    while ( pos < line.length() && line[pos] != '\"' ) {
	if ( line[pos] == '\\') {
	    pos++;
	    switch (char(line[pos]) ) {
	    case 'n':
		contents += '\n';
		break;
	    case 't':
		contents += '\t';
		break;
	    case 'r':
		contents += '\r';
		break;
	    case 'a':
		contents += '\a';
		break;
	    case 'f':
		contents += '\f';
		break;
	    case 'e':
		contents += '\e';
		break;
	    case 'b':
		contents += '\b';
		break;
	    default:
		contents += char(line[pos]);
		break;
	    }
	}
	else
	    contents += line[pos];
	pos++;
    }
    return contents;
}


void addTranslation( QTranslator* translator, const QString& msgid, const QString& msgstr)
{
    if (!msgid.isNull() && !msgstr.isNull() ) {
	//#### add codec crap here for unicode. Beware of the segfaults.
	QString scope = msgid.left( msgid.find("::") );
	QString id = msgid.right( msgid.length() - scope.length() - 2 );
	int hash = translator->hash( scope.ascii(), id.ascii() );
	if (translator->contains( hash, scope.ascii(), id.ascii() ) ) {
	    debug("Error: \"%s\" already in use", msgid.ascii() );
	}
	else {
	    translator->insert( hash, msgstr.ascii() );
	}
    }
}


void translate( const QString& filename, const QString& qmfile )
{
    QFile f(filename);
    if ( !f.open( IO_ReadOnly) )
	return;
    QTranslator* translator = new QTranslator(0);
    QTextStream t( &f );
    QString line;
    QString msgid;
    QString msgstr;
    while ( !t.atEnd() ) {
	if (line.isEmpty()) {
	    t.eatWhiteSpace();
	    line = t.readLine();
	}
	if ( hasHandle( line, "msgid") ) {
	    msgstr = QString::null;
	    msgid = extractContents( line );
	    if (!t.atEnd()) {
		t.eatWhiteSpace();
		line = t.readLine();
	    }
	    else
		line = QString::null;
	    while ( hasHandle( line, "\"") ) {
		msgid += extractContents( line );
		if (!t.atEnd()) {
		    t.eatWhiteSpace();
		    line = t.readLine();
		}
		else
		    line = QString::null;
	    }
	}
	else if ( hasHandle( line, "msgstr") ) {
	    msgstr = extractContents( line );
	    if (!t.atEnd()) {
		t.eatWhiteSpace();
		line = t.readLine();
	    }
	    else
		line = QString::null;
	    while ( hasHandle( line, "\"") ) {
		msgstr += extractContents( line );
		if (!t.atEnd()) {
		    t.eatWhiteSpace();
		    line = t.readLine();
		}
		else
		    line = QString::null;
	    }
	    addTranslation( translator, msgid, msgstr);
	}
	else
	    line = QString::null;
    }
    addTranslation( translator, msgid, msgstr);
    f.close();
    translator->save( qmfile );
}


int main( int argc, char* argv[] )
{
    if ( argc < 2 ) {
	debug("usage: %s infile [outfile]", argv[0]);
	exit(1);
    }
    translate(argv[1], argc > 2 ? argv[2] : "tr.qm");
}
