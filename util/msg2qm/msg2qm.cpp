/****************************************************************************
** $Id: //depot/qt/main/util/msg2qm/msg2qm.cpp#7 $
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
#include <qtextcodec.h>
#include <qtranslator.h>

static QString* defaultScope = 0;

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
	    qDebug("Error: \"%s\" already in use", msgid.ascii() );
	}
	else {
	    // #### TODO: use encoding mentioned in msg file, if any,
	    // ####       only default to locale if no other choice.
            QTextCodec *codec = QTextCodec::codecForLocale();

            if ( !codec ) {
                qDebug("No QTextCodec for this locale.");
                exit(1);
            }
            QString u_msgstr = codec->toUnicode(msgstr.ascii(), msgstr.length());
	    translator->insert( scope.latin1(), msgid.latin1(), u_msgstr );
	    //debug("'%s':'%s'-->'%s'", scope.ascii(), msgid.ascii(), msgstr.ascii() );
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
	}
	else
	    line = QString::null;
    }
    f.close();
    translator->save( qmfile );
}


int main( int argc, char* argv[] )
{

    int infile = 1;
    if (argc > 1) {
	if ( QString("-scope") == argv[1] ) {
	    defaultScope = new QString(argv[2]);
	    infile += 2;
	}
    }

    if ( argc <= infile ) {
	qDebug("usage: %s [-scope default] infile [outfile]", argv[0]);
	exit(1);
    }

    translate(argv[infile], argc > infile+1 ? argv[infile+1] : "tr.qm");
}
