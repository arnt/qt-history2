/*
  doc.cpp
*/

#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qvaluestack.h>

#include "atom.h"
#include "codemarker.h"
#include "doc.h"
#include "messages.h"
#include "quoter.h"
#include "separator.h"
#include "stringset.h"

static const char roman[] = "m\2d\5c\2l\5x\2v\5i";

/*
  HASH() combines the first character and the length of a command
  name to form a hash value.
*/
#define HASH( ch, len ) ( (ch) | ((len) << 8) )
#define CONSUME( target ) \
    if ( command != target ) \
	break; \
    consumed = TRUE

static QString untabify( const QString& str )
{
    QString t;
    int column = 0;

    for ( int i = 0; i < (int) str.length(); i++ ) {
	if ( str[i] == '\t' ) {
	    t += "        " + ( column & 0x7 );
	    column = ( column + 8 ) & ~0x7;
	} else {
	    t += str[i];
	    if ( str[i] == '\n' )
		column = 0;
	    else
		column++;
	}
    }

    t += '\n';
    t.replace( QRegExp(" +\n"), "\n" );
    while ( t.endsWith("\n\n") )
	t.truncate( t.length() - 1 );
    while ( t.startsWith("\n") )
	t = t.mid( 1 );
    return t;
}

class OpenedList
{
public:
    enum Style { Bullet, Numeric, UpperAlpha, LowerAlpha, UpperRoman,
		 LowerRoman };

    OpenedList()
	: sty( Bullet ), ini( 1 ), nex( 0 ) { }
    OpenedList( const Location& location, const QString& hint );

    void next() { nex++; }

    bool isStarted() const { return nex >= ini; }
    Style style() const { return sty; }
    QString styleString() const;
    int item() const { return nex; }
    QString itemString() const;

private:
    static QString toAlpha( int n );
    static int fromAlpha( const QString& str );
    static QString toRoman( int n );
    static int fromRoman( const QString& str );

    Style sty;
    int ini;
    int nex;
};

OpenedList::OpenedList( const Location& location, const QString& hint )
    : sty( Bullet ), ini( 1 )
{
    bool ok;
    int asNumeric = hint.toInt( &ok );
    int asRoman = fromRoman( hint );
    int asAlpha = fromAlpha( hint );

    if ( ok ) {
        sty = Numeric;
	ini = asNumeric;
    } else if ( asRoman > 0 && asRoman != 100 && asRoman != 500 ) {
	sty = ( hint == hint.lower() ) ? LowerRoman : UpperRoman;
	ini = asRoman;
    } else if ( asAlpha > 0 ) {
	sty = ( hint == hint.lower() ) ? LowerAlpha : UpperAlpha;
	ini = asAlpha;
    } else {
	if ( !hint.isEmpty() )
	    warning( 1, location, "Unrecognized list type '%s'",
		     hint.latin1() );
    }
    nex = ini - 1;
}

QString OpenedList::styleString() const
{
    switch ( style() ) {
    case Numeric:
	return "numeric";
    case UpperAlpha:
	return "upperalpha";
    case LowerAlpha:
	return "loweralpha";
    case UpperRoman:
	return "upperroman";
    case LowerRoman:
	return "lowerroman";
    case Bullet:
    default:
	return "bullet";
    }
}

QString OpenedList::itemString() const
{
    switch ( style() ) {
    case Numeric:
	return QString::number( item() );
    case UpperAlpha:
	return toAlpha( item() ).upper();
    case LowerAlpha:
	return toAlpha( item() );
    case UpperRoman:
	return toRoman( item() ).upper();
    case LowerRoman:
	return toRoman( item() );
    case Bullet:
    default:
	return "*";
    }
}

QString OpenedList::toAlpha( int n )
{
    QString str;

    while ( n > 0 ) {
	n--;
	str.prepend( (n % 26) + 'a' );
	n /= 26;
    }
    return str;
}

int OpenedList::fromAlpha( const QString& str )
{
    int n = 0;
    int u;

    for ( int i = 0; i < (int) str.length(); i++ ) {
	u = str[i].lower().unicode();
	if ( u >= 'a' && u <= 'z' ) {
	    n *= 26;
	    n += u - 'a' + 1;
	} else {
	    return 0;
	}
    }
    return n;
}

QString OpenedList::toRoman( int n )
{
    /*
      See p. 30 of Donald E. Knuth's "TeX: The Program".
    */
    QString str;
    int j = 0;
    int k;
    int u;
    int v = 1000;

    for ( ;; ) {
	while ( n >= v ) {
	    str += roman[j];
	    n -= v;
	}

	if ( n <= 0 )
	    break;

	k = j + 2;
	u = v / roman[k - 1];
	if ( roman[k - 1] == 2 ) {
	    k += 2;
	    u /= 5;
	}
	if ( n + u >= v ) {
	    str += roman[k];
	    n += u;
	} else {
	    j += 2;
	    v /= roman[j - 1];
	}
    }
    return str;
}

int OpenedList::fromRoman( const QString& str )
{
    int n = 0;
    int j;
    int u;
    int v = 0;

    for ( int i = str.length() - 1; i >= 0; i-- ) {
	j = 0;
	u = 1000;
	while ( roman[j] != 'i' && roman[j] != str[i].lower() ) {
	    j += 2;
	    u /= roman[j - 1];
	}
	if ( u < v ) {
	    n -= u;
	} else {
	    n += u;
	}
	v = u;
    }

    if ( str.lower() == toRoman(n) ) {
	return n;
    } else {
	return 0;
    }
}

class Also
{
public:
    Also() { }
    Also( const QString& target, const QString& text )
	: tar( target ), tex( text ) { }

    const QString& target() const { return tar; }
    const QString& text() const { return tex; }

private:
    QString tar;
    QString tex;
};

class DocPrivate : public QShared
{
public:
    DocPrivate()
	: firstAtom( 0 ), params( 0 ), alsoList( 0 ), metaCommandSet( 0 ),
	  metaCommandMap( 0 ) { }
    DocPrivate( const Location& location )
	: loc( location ), firstAtom( 0 ), params( 0 ), alsoList( 0 ),
	  metaCommandSet( 0 ), metaCommandMap( 0 ) { }
    ~DocPrivate();

    void addAlso( const QString& target, const QString& text = "" );

    Location loc;
    Atom *firstAtom;
    StringSet *params;
    QValueList<Also> *alsoList;
    StringSet *metaCommandSet;
    QMap<QString, QStringList> *metaCommandMap;
};

DocPrivate::~DocPrivate()
{
    delete firstAtom;
    delete params;
    delete alsoList;
    delete metaCommandSet;
    delete metaCommandMap;
}

void DocPrivate::addAlso( const QString& target, const QString& text )
{
    if ( !target.isEmpty() ) {
	if ( alsoList == 0 )
	    alsoList = new QValueList<Also>;
	alsoList->append( Also(target, text) );
    }
}

class DocParser
{
public:
    void parse( const QString& input, DocPrivate *docPrivate,
		const StringSet& metaCommandSet );

private:
    const Location& location();
    void startFormat( const QString& format );
    void startHeading( Atom::Type leftType, Atom::Type rightType,
		       const QString& string = "" );
    void endHeading();
    bool openCommand( const QString& command );
    bool closeCommand( const QString& command );
    void startSection( int level, const QString& command );
    void endSection( int level, const QString& command );
    void parseAlso();
    void append( Atom::Type type, const QString& string = "" );
    void appendChar( QChar ch );
    void appendToCode( const QString& code );
    void startNewParagraph();
    void enterParagraph();
    void leaveParagraph();
    void quoteFromFile( const QString& command );
    QString getSectioningUnit();
    QString getArgument();
    QString getRestOfLine();
    bool isBlankLine();
    bool isLeftBraceAhead();
    void skipSpaces();
    void skipSpacesOrEndl();

    QString in;
    int pos;
    int len;
    Location cachedLoc;
    int cachedPos;
    DocPrivate *priv;
    Atom *lastAtom;
    bool inPara;
    int braceDepth;
    int topSectionLevel;
    int prevSectionLevel;
    QMap<QString, Location> targetMap;
    QMap<int, QString> pendingFormats;
    Atom::Type pendingHeadingRightType;
    QString pendingHeadingString;
    QValueStack<QString> openedCommands;
    QValueStack<OpenedList> openedLists;
    Quoter quoter;
};

void DocParser::parse( const QString& input, DocPrivate *docPrivate,
		       const StringSet& metaCommandSet )
{
    in = input;
    pos = 0;
    len = (int) input.length();
    cachedLoc = docPrivate->loc;
    cachedPos = 0;
    priv = docPrivate;
    lastAtom = new Atom( Atom::DocBegin );
    inPara = FALSE;
    braceDepth = 0;
    topSectionLevel = -2;
    prevSectionLevel = -2;
    pendingHeadingRightType = Atom::DocEnd;
    openedCommands.push( "doc" );
    quoter.reset();

    priv->firstAtom = lastAtom;

    const CodeMarker *marker;
    QString link;
    QString x;
    int begin;
    int end;

    while ( pos < len ) {
	QChar ch = in[pos];

	if ( ch == '\\' ) {
	    QString command;
	    pos++;
	    begin = pos;
	    while ( pos < len ) {
		ch = in[pos];
		if ( ch.isLetterOrNumber() ) {
		    command += ch;
		    pos++;
		} else {
		    break;
		}
	    }
	    if ( command.isEmpty() ) {
		if ( pos < len ) {
		    enterParagraph();
		    if ( in[pos].isSpace() ) {
			while ( pos < len && in[pos].isSpace() )
			    pos++;
			appendChar( ' ' );
		    } else {
			appendChar( in[pos++] );
		    }
		}
	    } else {
		int h = HASH( command[0].unicode(), command.length() );
		bool consumed = FALSE;

		switch ( h ) {
		case HASH( 'a', 1 ):
		    CONSUME( "a" );
		    enterParagraph();
		    x = getArgument();
		    append( Atom::FormatBegin, "parameter" );
		    append( Atom::String, x );
		    append( Atom::FormatEnd, "parameter" );
		    if ( priv->params == 0 )
			priv->params = new StringSet;
		    priv->params->insert( x );
		    break;
		case HASH( 'a', 4 ):
		    CONSUME( "also" );
		    leaveParagraph();
		    parseAlso();
		    break;
		case HASH( 'a', 8 ):
		    CONSUME( "abstract" );
		    if ( openCommand("abstract") ) {
			leaveParagraph();
			append( Atom::AbstractBegin );
		    }
		    break;
		case HASH( 'a', 18 ):
		    CONSUME( "annotatedclasslist" );
		    append( Atom::GeneratedList, "annotatedclasses" );
		    break;
		case HASH( 'b', 1 ):
		    CONSUME( "b" );
		    startFormat( "bold" );
		    break;
		case HASH( 'b', 5 ):
		    CONSUME( "brief" );
		    startHeading( Atom::BriefBegin, Atom::BriefEnd );
		    break;
		case HASH( 'c', 1 ):
		    CONSUME( "c" );
		    enterParagraph();
		    x = untabify( getArgument() );
		    marker = CodeMarker::markerForCode( x, "C++" );
		    append( Atom::C, marker->markedUpCode(x, 0, "") );
		    break;
		case HASH( 'c', 4 ):
		    CONSUME( "code" );
		    leaveParagraph();
		    begin = pos;
		    end = in.find( QRegExp("\\\\endcode\\b"), pos );
		    if ( end == -1 ) {
			warning( 2, location(), "Missing '\\endcode'" );
		    } else {
			x = untabify( in.mid(begin, end - begin) );
			marker = CodeMarker::markerForCode( x, "C++" );
			append( Atom::Code, marker->markedUpCode(x, 0, "") );
			pos = end + 8;
		    }
		    break;
		case HASH( 'c', 7 ):
		    if ( command[1] == 'a' ) {
			CONSUME( "caption" );
			leaveParagraph();
			/* ... */
		    } else {
			CONSUME( "chapter" );
			startSection( 0, "chapter" );
		    }
		    break;
		case HASH( 'c', 8 ):
		    CONSUME( "citation" );
		    if ( openCommand("citation") ) {
			leaveParagraph();
			append( Atom::CitationBegin );
		    }
		    break;
		case HASH( 'c', 9 ):
		    CONSUME( "classlist" );
		    append( Atom::GeneratedList, "classes" );
		    break;
		case HASH( 'c', 14 ):
		    CONSUME( "classhierarchy" );
		    append( Atom::GeneratedList, "classhierarchy" );
		    break;
		case HASH( 'e', 7 ):
		    if ( command[5] == 'd' ) {
			CONSUME( "endcode" );
			closeCommand( "code" );
		    } else if ( command[5] == 'i' ) {
			CONSUME( "endomit" );
			closeCommand( "omit" );
		    } else if ( command[5] == 'r' ) {
			CONSUME( "endpart" );
			endSection( -1, "part" );
		    } else {
			CONSUME( "endlist" );
			if ( closeCommand("list") ) {
			    leaveParagraph();
			    if ( openedLists.top().isStarted() ) {
				append( Atom::ListItemEnd,
					openedLists.top().itemString() );
				append( Atom::ListEnd,
					openedLists.top().styleString() );
			    }
			    openedLists.pop();
			}
		    }
		    break;
		case HASH( 'e', 8 ):
		    CONSUME( "endtable" );
		    if ( closeCommand("table") ) {
			append( Atom::TableEnd );
			/* ... */
		    }
		    break;
		case HASH( 'e', 10 ):
		    if ( command[3] == 'c' ) {
			CONSUME( "endchapter" );
			endSection( 0, "chapter" );
		    } else {
			CONSUME( "endsidebar" );
			if ( closeCommand("sidebar") ) {
			    leaveParagraph();
			    append( Atom::SidebarEnd );
			}
		    }
		    break;
		case HASH( 'e', 11 ):
		    if ( command[10] == '1' ) {
			CONSUME( "endsection1" );
			endSection( 1, "section1" );
		    } else if ( command[10] == '2' ) {
			CONSUME( "endsection2" );
			endSection( 2, "section2" );
		    } else if ( command[10] == '3' ) {
			CONSUME( "endsection3" );
			endSection( 3, "section3" );
		    } else if ( command[10] == '4' ) {
			CONSUME( "endsection4" );
			endSection( 4, "section4" );
		    } else if ( command[10] == 'e' ) {
			CONSUME( "endfootnote" );
			if ( closeCommand("footnote") ) {
			    leaveParagraph();
			    append( Atom::FootnoteEnd );
			    inPara = TRUE;
			}
		    } else if ( command[10] == 'n' ) {
			CONSUME( "endcitation" );
			if ( closeCommand("citation") ) {
			    leaveParagraph();
			    append( Atom::CitationEnd );
			}
		    } else if ( command[10] == 't' ) {
			CONSUME( "endabstract" );
			if ( closeCommand("abstract") ) {
			    leaveParagraph();
			    append( Atom::AbstractEnd );
			}
		    }
		    break;
		case HASH( 'f', 8 ):
		    CONSUME( "footnote" );
		    if ( openCommand("footnote") ) {
			enterParagraph();
			append( Atom::FootnoteBegin );
			inPara = FALSE;
		    }
		    break;
		case HASH( 'h', 6 ):
		    CONSUME( "header" );
		    /* ... */
		    break;
		case HASH( 'h', 14 ):
		    CONSUME( "headerfilelist" );
		    append( Atom::GeneratedList, "headerfiles" );
		    break;
		case HASH( 'i', 1 ):
		    CONSUME( "i" );
		    startFormat( "italic" );
		    break;
		case HASH( 'i', 5 ):
		    if ( command[1] == 'm' ) {
			CONSUME( "image" );
			/* ... */
		    } else {
			CONSUME( "index" );
			/* ... */
		    }
		    break;
		case HASH( 'i', 7 ):
		    CONSUME( "include" );
		    /* ... */
		    break;
		case HASH( 'k', 7 ):
		    CONSUME( "keyword" );
		    /* ... */
		    break;
		case HASH( 'l', 1 ):
		    CONSUME( "l" );
		    enterParagraph();
		    if ( isLeftBraceAhead() ) {
			x = getArgument();
			append( Atom::Link, x );
			if ( isLeftBraceAhead() ) {
			    startFormat( "link" );
			} else {
			    append( Atom::FormatBegin, "link" );
			    append( Atom::String, x );
			    append( Atom::FormatEnd, "link" );			
			}
		    } else {
			x = getArgument();
			append( Atom::Link, x );
			append( Atom::FormatBegin, "link" );
			append( Atom::String, x );
			append( Atom::FormatEnd, "link" );			
		    }
		    break;
		case HASH( 'l', 4 ):
		    CONSUME( "list" );
		    if ( openCommand("list") ) {
			leaveParagraph();
			openedLists.push( OpenedList(location(),
						     getArgument()) );
		    }
		    break;
		case HASH( 'l', 6 ):
		    CONSUME( "linkto" );
		    enterParagraph();
		    append( Atom::Link, getArgument() );
		    startFormat( "link" );
		    break;
		case HASH( 'l', 8 ):
		    CONSUME( "location" );
		    cachedLoc = Location::fromString( getArgument() );
		    cachedPos = pos;
		    break;
		case HASH( 'm', 13 ):
		    CONSUME( "mainclasslist" );
		    append( Atom::GeneratedList, "mainclasses" );
		    break;
		case HASH( 'o', 1 ):
		    CONSUME( "o" );
		    leaveParagraph();
		    if ( openedLists.isEmpty() ) {
			warning( 1, location(),
				 "Command '\\o' outside '\\list'" );
		    } else {
			if ( openedLists.top().isStarted() ) {
			    leaveParagraph();
			    append( Atom::ListItemEnd,
				    openedLists.top().itemString() );
			} else {
			    append( Atom::ListBegin,
				    openedLists.top().styleString() );
			}
			openedLists.top().next();
			append( Atom::ListItemNumber,
				QString::number(openedLists.top().item()) );
			append( Atom::ListItemBegin,
				openedLists.top().itemString() );
			enterParagraph();
		    }
		    break;
		case HASH( 'o', 4 ):
		    CONSUME( "omit" );
		    end = in.find( QRegExp("\\\\endomit\\b"), pos );
		    if ( end == -1 ) {
			pos = len;
		    } else {
			pos = end + 9;
		    }
		    break;
		case HASH( 'p', 4 ):
		    CONSUME( "part" );
		    startSection( -1, "part" );
		    break;
		case HASH( 'q', 7 ):
		    CONSUME( "quoteto" );
		    leaveParagraph();
		    appendToCode( quoter.quoteTo(location(), command,
				  getRestOfLine()) );
		    break;
		case HASH( 'q', 9 ):
		    if ( command[5] == 'f' ) {
			CONSUME( "quotefile" );
			leaveParagraph();
			quoteFromFile( command );
			append( Atom::Code,
				quoter.quoteUntil(location(), command) );
			quoter.reset();
		    } else {
			CONSUME( "quoteline" );
			leaveParagraph();
			appendToCode( quoter.quoteLine(location(), command,
						       getRestOfLine()) );
		    }
		    break;
		case HASH( 'q', 10 ):
		    CONSUME( "quoteuntil" );
		    leaveParagraph();
		    appendToCode( quoter.quoteUntil(location(), command,
						    getRestOfLine()) );
		    break;
		case HASH( 'q', 13 ):
		    if ( command[6] == 'r' ) {
			CONSUME( "quotefromfile" );
			leaveParagraph();
			quoteFromFile( command );
		    } else {
			CONSUME( "quotefunction" );
			leaveParagraph();
			quoteFromFile( command );
			quoter.quoteTo( location(), command, getRestOfLine() );
			append( Atom::Code,
				quoter.quoteUntil(location(), command,
						  "/^\\}/") );
			quoter.reset();
		    }
		    break;
		case HASH( 'r', 3 ):
		    if ( command[1] == 'a' ) {
			CONSUME( "raw" );
			leaveParagraph();
			begin = pos;
			end = in.find( QRegExp("\\\\endraw\\b"), pos );
			if ( end == -1 ) {
			    warning( 2, location(), "Missing '\\endraw'" );
			} else {
			    append( Atom::RawFormat, "html" ); // ###
			    x = untabify( in.mid(begin, end - begin) );
			    append( Atom::RawString, x );
			    pos = end + 7;
			}
		    } else {
			CONSUME( "row" );
			/* ... */
		    }
		    break;
		case HASH( 's', 6 ):
		    CONSUME( "skipto" );
		    leaveParagraph();
		    quoter.quoteTo( location(), command, getRestOfLine() );
		    break;
		case HASH( 's', 7 ):
		    CONSUME( "sidebar" );
		    if ( openCommand("sidebar") ) {
			leaveParagraph();
			append( Atom::SidebarBegin );
		    }
		    break;
		case HASH( 's', 8 ):
		    if ( command[7] == '1' ) {
			CONSUME( "section1" );
			startSection( 1, "section1" );
		    } else if ( command[7] == '2' ) {
			CONSUME( "section2" );
			startSection( 2, "section2" );
		    } else if ( command[7] == '3' ) {
			CONSUME( "section3" );
			startSection( 3, "section3" );
		    } else if ( command[7] == '4' ) {
			CONSUME( "section4" );
			startSection( 4, "section4" );
		    } else {
			CONSUME( "skipline" );
			leaveParagraph();
			quoter.quoteLine( location(), command,
					  getRestOfLine() );
		    }
		    break;
		case HASH( 's', 9 ):
		    CONSUME( "skipuntil" );
		    leaveParagraph();
		    quoter.quoteUntil( location(), command, getRestOfLine() );
		    break;
		case HASH( 't', 2 ):
		    CONSUME( "tt" );
		    startFormat( "typewriter" );
		    break;
		case HASH( 't', 5 ):
		    if ( command[1] == 'a' ) {
			CONSUME( "table" );
			if ( openCommand("table") ) {
			    leaveParagraph();
			    append( Atom::TableBegin );
			}
		    } else {
			CONSUME( "title" );
			startHeading( Atom::TitleBegin, Atom::TitleEnd );
		    }
		    break;
		case HASH( 't', 6 ):
		    CONSUME( "target" );
		    x = getRestOfLine();
		    if ( targetMap.contains(x) ) {
			warning( 1, location(), "Duplicate target name" );
			warning( 1, targetMap[x],
				 "(the previous occurrence is here)" );
		    } else {
			targetMap.insert( x, location() );
			append( Atom::Target, x );
		    }
		    break;
		case HASH( 't', 8 ):
		    CONSUME( "theindex" );
		    append( Atom::GeneratedList, "index" );
		    break;
		case HASH( 't', 15 ):
		    CONSUME( "tableofcontents" );
		    append( Atom::TableOfContents, getSectioningUnit() );
		    break;
		case HASH( 'u', 1 ):
		    CONSUME( "u" );
		    startFormat( "underlined" );
		    break;
		case HASH( 'v', 5 ):
		    CONSUME( "value" );
		    /* ... */
		    break;
		case HASH( 'w', 7 ):
		    CONSUME( "warning" );
		    startNewParagraph();
		    append( Atom::FormatBegin, "bold" );
		    append( Atom::String, "Warning: " );
		    append( Atom::FormatEnd, "bold" );
		}

		if ( !consumed ) {
		    if ( metaCommandSet.contains(command) ) {
			if ( priv->metaCommandSet == 0 ) {
			    priv->metaCommandSet = new StringSet;
			    priv->metaCommandMap =
				    new QMap<QString, QStringList>;
			}
			priv->metaCommandSet->insert( command );
			(*priv->metaCommandMap)[command].append(
				getRestOfLine() );
		    } else {
			warning( 1, location(), "Unknown command '\\%s'",
				 command.latin1() );
		    }
		}
	    }
	} else if ( ch == '{' ) {
	    enterParagraph();
	    appendChar( ch );
	    braceDepth++;
	    pos++;
	} else if ( ch == '}' ) {
	    braceDepth--;
	    QMap<int, QString>::Iterator f = pendingFormats.find( braceDepth );
	    if ( f == pendingFormats.end() ) {
		enterParagraph();
		appendChar( ch );
	    } else {
		append( Atom::FormatEnd, *f );
		pendingFormats.remove( f );
	    }
	    pos++;
	} else {
	    if ( inPara ) {
		pos++;
		if ( ch == '\n' ) {
		    if ( isBlankLine() ) {
			leaveParagraph();
		    } else {
			endHeading();
			appendChar( ' ' );
		    }
		} else {
		    appendChar( ch );
		}
	    } else {
		if ( !ch.isSpace() ) {
		    enterParagraph();
		    appendChar( ch );
		}
		pos++;
	    }
	}
    }
    leaveParagraph();
    append( Atom::DocEnd );

    if ( openedCommands.top() != "doc" )
	warning( 1, location(), "Missing '\\end%s'",
		 openedCommands.top().latin1() );
}

const Location& DocParser::location()
{
    while ( cachedPos < pos )
	cachedLoc.advance( in[cachedPos++] );
    return cachedLoc;
}

void DocParser::startFormat( const QString& format )
{
    enterParagraph();
    append( Atom::FormatBegin, format );

    if ( isLeftBraceAhead() ) {
	skipSpacesOrEndl();
	pendingFormats.insert( braceDepth, format );
	braceDepth++;
	pos++;
    } else {
	append( Atom::String, getArgument() );
	append( Atom::FormatEnd, format );
    }
}

void DocParser::startHeading( Atom::Type leftType, Atom::Type rightType,
			      const QString& string )
{
    leaveParagraph();
    skipSpaces();
    if ( pos < len && in[pos] != '\n' ) {
	append( leftType, string );
	enterParagraph();
	pendingHeadingRightType = rightType;
	pendingHeadingString = string;
    }
}

void DocParser::endHeading()
{
    if ( pendingHeadingRightType != Atom::DocEnd ) {
	append( pendingHeadingRightType, pendingHeadingString );
	pendingHeadingRightType = Atom::DocEnd;
	pendingHeadingString = "";
    }
}

bool DocParser::openCommand( const QString& command )
{
    QString top = openedCommands.top();
    bool ok;

    if ( top == "doc" || top == "list" ) {
	ok = TRUE;
    } else {
	QStringList ordering;
	ordering << "abstract" << "sidebar" << "citation" << "table"
		 << "footnote";
	ok = ordering.findIndex( top ) < ordering.findIndex( command );
    }

    if ( ok ) {
	openedCommands.push( command );
    } else {
	warning( 1, location(), "Cannot use '\\%s' within '\\%s'",
		 command.latin1(), top.latin1() );
    }
    return ok;
}

bool DocParser::closeCommand( const QString& command )
{
    if ( openedCommands.top() == command ) {
	openedCommands.pop();
	return TRUE;
    } else {
	if ( openedCommands.contains(command) ) {
	    while ( openedCommands.top() != command ) {
		warning( 1, location(), "Missing '\\end%s' before '\\end%s'",
			 openedCommands.top().latin1(), command.latin1() );
		openedCommands.pop();
	    }
	} else {
	    warning( 1, location(), "Missing '\\%s'", command.latin1() );
	}
	return FALSE;
    }
}

void DocParser::startSection( int level, const QString& command )
{
    leaveParagraph();

    if ( topSectionLevel == -2 ) {
	if ( level > 1 )
	    warning( 1, location(),
		     "Unexpected '\\%1' without '\\section1'",
		     command.latin1() );
	topSectionLevel = level;
	prevSectionLevel = level - 1;
    }

    if ( level < topSectionLevel ) {
	warning( 0, location(), "Unexpected '\\%s' in this documentation",
		 command.latin1() );
    } else if ( level - prevSectionLevel > 1 ) {
	warning( 0, location(), "Unexpected '\\%s' at this point",
		 command.latin1() );
    } else {
	int i = prevSectionLevel;
	while ( i >= level ) {
	    endSection( i, command );
	    i--;
	}
	append( Atom::SectionBegin, QString::number(level) );
	startHeading( Atom::SectionHeadingBegin, Atom::SectionHeadingEnd,
		      QString::number(level) );
	prevSectionLevel = level;
    }
}

void DocParser::endSection( int level, const QString& command )
{
    leaveParagraph();

    if ( level < topSectionLevel ) {
	warning( 0, location(), "Unexpected '\\end%s' in this documentation",
		 command.latin1() );
    } else if ( level != prevSectionLevel ) {
	warning( 0, location(), "Unexpected '\\end%s' at this point",
		 command.latin1() );
    } else {
	append( Atom::SectionEnd, QString::number(level) );
	prevSectionLevel--;
    }
}

void DocParser::parseAlso()
{
    /* .. */
}

void DocParser::append( Atom::Type type, const QString& string )
{
    if ( lastAtom->type() == Atom::Code && lastAtom->string().endsWith("\n\n") )
	lastAtom->chopString();
    lastAtom = new Atom( lastAtom, type, string );
}

void DocParser::appendChar( QChar ch )
{
    if ( lastAtom->type() != Atom::String )
	append( Atom::String );
    if ( ch.isSpace() ) {
	if ( !lastAtom->string().endsWith(" ") )
	    lastAtom->appendChar( ' ' );
    } else {
	lastAtom->appendChar( ch );
    }
}

void DocParser::appendToCode( const QString& markedCode )
{
    if ( lastAtom->type() != Atom::Code )
	append( Atom::Code );
    lastAtom->appendString( markedCode );
}

void DocParser::startNewParagraph()
{
    leaveParagraph();
    enterParagraph();
}

void DocParser::enterParagraph()
{
    if ( !inPara ) {
	append( Atom::ParagraphBegin );
	inPara = TRUE;
	skipSpacesOrEndl();
    }
}

void DocParser::leaveParagraph()
{
    if ( inPara ) {
	if ( !pendingFormats.isEmpty() ) {
	    warning( 1, location(), "Missing '}'" );
	    pendingFormats.clear();
	}

	if ( lastAtom->type() == Atom::String &&
	     lastAtom->string().endsWith(" ") )
	    lastAtom->chopString();
	append( Atom::ParagraphEnd );
	inPara = FALSE;
	endHeading();
    }
}

void DocParser::quoteFromFile( const QString& command )
{
    quoter.reset();

    QString fileName = getArgument();

    if ( fileName.isEmpty() ) {
	warning( 1, location(), "Expected file name after '\\%s'",
		 command.latin1() );
	return;
    }

    QString filePath = fileName; /* ### */
    if ( filePath.isEmpty() ) {
	warning( 1, location(), "Cannot find example file '%s'",
		 fileName.latin1() );
	return;
    }

    QFile inFile( filePath );
    if ( !inFile.open(IO_ReadOnly) ) {
	warning( 1, location(), "Cannot open example file '%s'",
		 filePath.latin1() );
	return;
    }

    QTextStream inStream( &inFile );
    QString code = untabify( inStream.read() );
    inFile.close();
    if ( code.isEmpty() ) {
	warning( 1, location(), "Example file '%s' is empty",
		 filePath.latin1() );
	return;
    }

    const CodeMarker *marker = CodeMarker::markerForFileName( fileName, "C++" );
    quoter.quoteFromFile( filePath, code,
			  marker->markedUpCode(code, 0,
					       QFileInfo(filePath).dirPath()) );
}

QString DocParser::getSectioningUnit()
{
    QString x = getRestOfLine();
    if ( !QRegExp("part|chapter|section[1-4]").exactMatch(x) ) {
	if ( !x.isEmpty() )
	    warning( 1, location(), "Invalid sectioning unit '%s'",
		     x.latin1() );
	x = "section4";
    }
    return x;
}

QString DocParser::getArgument()
{
    int parenDepth = 0;
    int bracketDepth = 0;

    skipSpacesOrEndl();
    int begin = pos;

    /*
      Typically, an argument ends at the next white-space. However,
      braces can be used to group words:

	  {a few words}

      Also, opening and closing parentheses have to match. Thus,

	  printf( "%d\n", x )

      is an argument too, although it contains spaces. Finally,
      trailing punctuation is not included in an argument, nor is 's.
    */
    if ( pos < (int) in.length() && in[pos] == '{' ) {
	pos++;
	while ( pos < (int) in.length() && in[pos] != '}' )
	    pos++;
	pos++;
	return in.mid( begin + 1, pos - begin - 2 ).simplifyWhiteSpace();
    } else {
	begin = pos;
	while ( pos < (int) in.length() ) {
	    QChar ch = in[pos];

	    switch ( ch.unicode() ) {
	    case '(':
		parenDepth++;
		break;
	    case ')':
		parenDepth--;
		break;
	    case '[':
		bracketDepth++;
		break;
	    case ']':
		bracketDepth--;
	    }
	    if ( parenDepth < 0 || bracketDepth < 0 || ch == '\\' ||
		 ch == '{' || ch == '}' )
		break;
	    if ( ch.isSpace() && parenDepth <= 0 && bracketDepth <= 0 )
		break;
	    pos++;
	}

	if ( pos > begin + 1 && QString(".,:;").find(in[pos - 1]) != -1 &&
	     in.mid(pos - 3, 3) != "..." )
	    pos--;
	if ( in.mid(pos - 2, 2) == "'s" )
	    pos -= 2;
	return in.mid( begin, pos - begin ).simplifyWhiteSpace();
    }
}

QString DocParser::getRestOfLine()
{
    skipSpaces();

    int begin = pos;
    int level = 0;
    int ch;

    while ( pos < (int) in.length() &&
	    ((ch = in[pos].unicode()) != '\n' || level > 0) )
    {
	if ( ch == '(' )
	    level++;
	else if ( ch == ')' )
	    level--;
	pos++;
    }
    // ### look for 'const' alone on its line

    QString t = in.mid( begin, pos - begin ).simplifyWhiteSpace();
    skipSpaces();
    return t;
}

bool DocParser::isBlankLine()
{
    int i = pos;

    while ( i < len && in[i].isSpace() ) {
	if ( in[i] == '\n' )
	    return TRUE;
	i++;
    }
    return FALSE;
}

bool DocParser::isLeftBraceAhead()
{
    int numEndl = 0;
    int i = pos;

    while ( i < len && in[i].isSpace() && numEndl < 2 ) {
	if ( in[i] == '\n' )
	    numEndl++;
    	i++;
    }
    return i < len && in[i] == '{';
}

void DocParser::skipSpaces()
{
    while ( pos < (int) in.length() && in[pos].isSpace() &&
	    in[pos].unicode() != '\n' )
	pos++;
}

void DocParser::skipSpacesOrEndl()
{
    int firstEndl = -1;
    while ( pos < (int) in.length() && in[pos].isSpace() ) {
	QChar ch = in[pos];
	if ( ch == '\n' ) {
	    if ( firstEndl == -1 ) {
		firstEndl = pos;
	    } else {
		pos = firstEndl;
		break;
	    }
	}
	pos++;
    }
}

Doc::Doc()
{
    priv = new DocPrivate;
}

Doc::Doc( const Location& location, const QString& text,
	  const StringSet& metaCommandSet )
{
    priv = new DocPrivate( location );
    DocParser parser;
    parser.parse( text, priv, metaCommandSet );
}

Doc::Doc( const Doc& doc )
{
    priv = new DocPrivate;
    operator=( doc );
}

Doc::~Doc()
{
    if ( priv->deref() )
	delete priv;
}

Doc& Doc::operator=( const Doc& doc )
{
    doc.priv->ref();
    if ( priv->deref() )
	delete priv;
    priv = doc.priv;
    return *this;
}

void Doc::addAlso( const QString& target, const QString& text )
{
    priv->addAlso( target, text );
}

QStringList Doc::extractMetaCommand( const QString& command )
{
    QStringList args;

    if ( priv->metaCommandSet != 0 ) {
	QMap<QString, QStringList>::Iterator c =
		priv->metaCommandMap->find( command );
	if ( c != priv->metaCommandMap->end() ) {
	    args = *c;
	    priv->metaCommandSet->remove( command );
	    priv->metaCommandMap->remove( c );
	    if ( priv->metaCommandSet->isEmpty() ) {
		delete priv->metaCommandSet;
		delete priv->metaCommandMap;
		priv->metaCommandSet = 0;
		priv->metaCommandMap = 0;
	    }
	}
    }
    return args;
}

const Location& Doc::location() const
{
    return priv->loc;
}

bool Doc::isEmpty() const
{
    return priv->firstAtom == 0;
}

const StringSet *Doc::metaCommands() const
{
    return priv->metaCommandSet;
}

Atom *Doc::createAlsoAtomList() const
{
    Atom *firstAtom = new Atom( Atom::DocBegin );
    Atom *lastAtom = firstAtom;
    lastAtom = new Atom( lastAtom, Atom::DocEnd );
    return firstAtom;
}

const Atom *Doc::atomList() const
{
    return priv->firstAtom;
}
