/****************************************************************************
** $Id: //depot/qt/main/util/msgmerge/mergetr.cpp#1 $
**
** This is a utility program for merging findtr msgfiles
**
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/
//#include <qapplication.h>
#include <qfile.h>
#include <qtextstream.h>
//#include <qtextcodec.h>

#include <stdio.h>

bool isEmpty( const QString& line )
{
    int i = 0;
    while ( i < int(line.length()) ) {
	if ( !line[i].isSpace() )
	    return FALSE;
	i++;
    }
    return TRUE;
}
	    

bool hasHandle( const QString& line, const QString& handle)
{
    return line.left(handle.length()) == handle;
}

bool isUserComment( const QString& line )
{
    return line.length() > 2 && line[0] == '#' && line[1].isSpace();
}

bool isSystemComment( const QString& line )
{
    return line.length() > 1 && line[0] == '#' && !line[1].isSpace();
}


QString extractContents( const QString& line )
{
    QString contents;
    unsigned int pos = 0;
    bool inStr = FALSE;
    while ( pos < line.length()  ) {
	if ( line[pos] == '"' ) {
	    inStr = !inStr;
	    pos++;
	} else {
	    if ( inStr ) {
		if ( line[pos] == '\\') {
		    pos++;
		    // if ( pos >= line.length() ) qWarning(...);
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
		} else {
 		    contents += line[pos];
		}
	    } else {
		if ( !line[pos].isSpace() ) {
		    //qWarning( "mergetr: %s", line.ascii() );
		}
	    }
	    pos++;
	}
    }
    return contents;
}

struct Item
{
    Item() :null(TRUE) {}
    Item( const Item &i ) :null(i.null),usercomments(i.usercomments),
	systemcomments(i.systemcomments), msgid(i.msgid), msgstr(i.msgstr){}
    Item &operator=( const Item &i ) {
	null = i.null; usercomments = i.usercomments;
	systemcomments = i.systemcomments; msgid = i.msgid; msgstr = i.msgstr;
	return *this;
    }
    bool isNull() const { return null; }

    bool null;
    QString usercomments;
    QString systemcomments;
    QString msgid;
    QString msgstr;
};

enum Status { Equal, First, Second };

Item getItem( QTextStream &str )
{
    Item i;
    str.skipWhiteSpace();
    QString s = str.readLine();
    while ( !str.atEnd() && s.isEmpty() ) {
	s = str.readLine();
    }
    while (  !str.atEnd() && !s.isEmpty() ) {
	if ( isSystemComment(s) ) {
	    i.systemcomments += s + "\n";
	    s = str.readLine();
	} else if ( isUserComment(s) ) {
	    i.usercomments += s + "\n";
	    s = str.readLine();
	} else if ( hasHandle( s, "msgid" ) ) {
	    QString r = s + "\n";
	    str.skipWhiteSpace();
	    s = str.readLine();
	    while (  !str.atEnd() && hasHandle(s, "\"") ) {
		r += s;
		str.skipWhiteSpace();
		s = str.readLine();
		r += "\n";
	    }
	    i.msgid = r;
	} else if ( hasHandle( s, "msgstr" ) ) {
	    QString r = s + "\n";
	    str.skipWhiteSpace();
	    s = str.readLine();
	    while ( hasHandle(s, "\"") ) {
		r += s;
		str.skipWhiteSpace();
		s = str.readLine();
		r += "\n";
	    }
	    i.msgstr = r;
	    i.null = FALSE;
	    //debug( "gotItem: %s -> %s", i.msgid.ascii(), i.msgstr.ascii() );
	    return i;
	} else {
	    qDebug( "skipping %s", s.ascii() );
	    s = str.readLine();
	}
    }
    i.null = TRUE;
    //debug( "notItem: %s -> %s", i.msgid.ascii(), i.msgstr.ascii() );
    return i;
}

void writemerge( QTextStream &str, const Item &i1, const Item &i2 )
{
    //qDebug( "writemerge" );
    if ( !i2.usercomments.isEmpty() )
	str << i2.usercomments;
    if ( !i1.systemcomments.isEmpty() )
	str << i1.systemcomments;
    str << i1.msgid;
    str << i2.msgstr;
    str << "\n";
}
void writenew( QTextStream &str, const Item &i1 )
{
    //qDebug( "writenew" );
    if ( !i1.usercomments.isEmpty() )
	str << i1.usercomments;
    if ( !i1.systemcomments.isEmpty() )
	str << i1.systemcomments;
    str << i1.msgid;
    str << i1.msgstr;
    str << "\n";
}
void writeold( QTextStream &str, const Item &i2 )
{
    //qDebug( "writeold" );
    if ( !i2.usercomments.isEmpty() )
	str << "# " << i2.usercomments;
    if ( !i2.systemcomments.isEmpty() )
	str << "# " << i2.systemcomments;
    str << "# " << i2.msgid;
    str << "# " << i2.msgstr;
    str << "\n";
}

Status compare( const Item &i1, const Item &i2 )
{
    if ( i1.isNull() && i2.isNull() )
	return Equal;
    if ( i1.isNull() )
	return Second;
    if ( i2.isNull() )
	return First;
    QString s1 = extractContents( i1.msgid );
    QString s2 = extractContents( i2.msgid );
    int i = strcmp( s1.ascii(), s2.ascii() ); 
    if ( i < 0 ) 
	return First;
    if ( i == 0 )
	return Equal;
    // i > 0 
	return Second;
}

void merge( const QString& newname, const QString& oldname,
	    const QString& resname )
{
    QFile f1(newname);
    if ( !f1.open( IO_ReadOnly) )
	return;

    QFile f2(oldname);
    if ( !f2.open( IO_ReadOnly) )
	return;

    QFile fout(resname);
    if ( !fout.open( IO_WriteOnly) )
	return;

    QTextStream in1( &f1 );
    QTextStream in2( &f2 );
    QTextStream out( &fout );



    Item i1 = getItem( in1 );
    Item i2 = getItem( in2 );
    while ( !i1.isNull() || !i2.isNull() ) {
	Status s = compare( i1, i2 );
	if ( s == Equal ) {
	    writemerge(out,i1,i2);
	    i1 = getItem( in1 );
	    i2 = getItem( in2 );
	} else if ( s == First ) {
	    //i1 < i2 || i2 == 0
	    writenew( out, i1 );
	    i1 = getItem( in1 );
	} else if ( s == Second ) {
	    //i1 > i2 || i1 == 0
	    writeold( out, i2 );
	    i2 = getItem( in2 );
	}
    }

    f1.close();
    f2.close();
    fout.close();
}


int main( int argc, char* argv[] )
{

    int orgfile = 1;
    int newfile = 2;

    if ( argc <= newfile ) {
	qDebug("usage: %s  orgfile newfile [outfile]", argv[0]);
	exit(1);
    }

    merge( argv[newfile], argv[orgfile], 
	   argc > newfile+1 ? argv[newfile+1] : "merge.tr" );
}
