/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qfile.h>
#include <qtextstream.h>
#include <qtextcodec.h>
#include <qtranslator.h>

#include <stdio.h>
#include <stdlib.h>

QT_BEGIN_NAMESPACE

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
    int pos = 0;
    while ( pos < int(line.length()) && line[pos] != '\"' )
	pos++;
    pos++;
    while ( pos < int(line.length()) && line[pos] != '\"' ) {
	// 0xa5: the yen sign is the Japanese backslash
	if ( line[pos] == '\\' || line[pos] == QChar(0xa5) ) {
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
	    case 'v':
		contents += '\v';
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

	if (translator->contains( scope.ascii(), id.ascii() ) ) {
	    qWarning("Error: \"%s\" already in use", msgid.ascii() );
	}
	else {
	    translator->insert( scope.latin1(), id.latin1(), msgstr );
	}
    }
}



void translate( const QString& filename, const QString& qmfile )
{
    QFile f(filename);
    if ( !f.open( IO_ReadOnly) )
	return;
    QTranslator* translator = new QTranslator(0);
    QTextCodec *codec = 0;
    for (int pass =  0; pass < 2; pass++) {
	f.at(0);
	QTextStream t( &f );
	QString line;
	QString msgid;
	QString msgstr;
	if ( codec != 0 ) {
	    t.setCodec( codec );
	}
	while ( !t.atEnd() || !line.isEmpty() ) {
	    if (line.isEmpty()) {
		t.skipWhiteSpace();
		line = t.readLine();
	    }
	    if ( hasHandle( line, "msgid") ) {
		msgstr.clear();
		msgid = extractContents( line );
		if (!t.atEnd()) {
		    t.skipWhiteSpace();
		    line = t.readLine();
		}
		else
		    line.clear();
		while ( hasHandle( line, "\"") ) {
		    msgid += extractContents( line );
		    if (!t.atEnd()) {
			t.skipWhiteSpace();
			line = t.readLine();
		    }
		    else
			line.clear();
		}
	    }
	    else if ( hasHandle( line, "msgstr") ) {
		msgstr = extractContents( line );
		if (!t.atEnd()) {
		    t.skipWhiteSpace();
		    line = t.readLine();
		}
		else
		    line.clear();
		while ( hasHandle( line, "\"") ) {
		    msgstr += extractContents( line );
		    if (!t.atEnd()) {
			t.skipWhiteSpace();
			line = t.readLine();
		    }
		    else
			line.clear();
		}
		if ( pass == 1 )
		    addTranslation( translator, msgid, msgstr);

		if ( pass == 0 && msgid.isEmpty() ) {
		    // Check for the encoding.
		    int cpos = msgstr.find( "charset=" );
		    if ( cpos >= 0 ) {
			cpos = cpos + 8; //skip "charset="
			int i = cpos;
			int len = msgstr.length();
			while ( i < len && !msgstr[i].isSpace() )
			    i++;
			QString charset = msgstr.mid( cpos, i-cpos );
			codec = QTextCodec::codecForName( charset.ascii() );
			if ( codec ) {
			    debug( "PO file character set: %s. Codec: %s",
				   charset.ascii(), codec->name() );
			} else {
			    debug( "No codec for %s", charset.ascii() );
			}
		    }
		    break;
		}
	    }
	    else
		line.clear();
	}
    }
    f.close();
    translator->save( qmfile );
}


// workaround for BCC problem, qtranslator.h includes qwindowdefs.h via qobject.h, see NEEDS_QMAIN
#if defined(main)
#undef main
#endif

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
	printf("usage: %s [-scope default] infile [outfile]\n", argv[0]);
	exit(1);
    }

    translate(argv[infile], argc > infile+1 ? argv[infile+1] : "tr.qm");

QT_END_NAMESPACE
    return 0;
}
