/*
  doc.cpp
*/

#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qvaluestack.h>

#include <limits.h>

#include "codemarker.h"
#include "doc.h"
#include "messages.h"
#include "molecule.h"
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

    t.replace( QRegExp(" +\n"), "\n" );
    while ( t.endsWith("\n\n") )
	t.truncate( t.length() - 1 );
    while ( t.startsWith("\n") )
	t = t.mid( 1 );
    return t;
}

static int indentLevel( const QString& str )
{
    int minIndent = INT_MAX;
    int column = 0;

    for ( int i = 0; i < (int) str.length(); i++ ) {
	if ( str[i] == '\n' ) {
	    column = 0;
	} else {
	    if ( str[i] != ' ' && column < minIndent )
		minIndent = column;
	    column++;
	}
    }
    return minIndent;
}

static QString unindent( int level, const QString& str )
{
    if ( level == 0 )
	return str;

    QString t;
    int column = 0;

    for ( int i = 0; i < (int) str.length(); i++ ) {
	if ( str[i] == '\n' ) {
	    t += '\n';
	    column = 0;
	} else {
	    if ( column >= level )
		t += str[i];
	    column++;
	}
    }
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

class DocPrivateExtra
{
public:
    QString baseName;
    Doc::SectioningUnit granularity;
    Doc::SectioningUnit sectioningUnit;

    DocPrivateExtra()
	: granularity( Doc::Part ), sectioningUnit( Doc::Chapter ) { }
};

class DocPrivate : public QShared
{
public:
    DocPrivate()
	: params( 0 ), alsoList( 0 ), metaCommandSet( 0 ),
	  metaCommandMap( 0 ), extra( 0 ) { }
    DocPrivate( const Location& location )
	: loc( location ), params( 0 ), alsoList( 0 ), metaCommandSet( 0 ),
	  metaCommandMap( 0 ), extra( 0 ) { }
    ~DocPrivate();

    void addAlso( const Molecule& also );
    void constructExtra();

    Location loc;
    Molecule molecule;
    StringSet *params;
    QValueList<Molecule> *alsoList;
    StringSet *metaCommandSet;
    QMap<QString, QStringList> *metaCommandMap;
    DocPrivateExtra *extra;
};

DocPrivate::~DocPrivate()
{
    delete params;
    delete alsoList;
    delete metaCommandSet;
    delete metaCommandMap;
    delete extra;
}

void DocPrivate::addAlso( const Molecule& also )
{
    if ( alsoList == 0 )
	alsoList = new QValueList<Molecule>;
    alsoList->append( also );
}

void DocPrivate::constructExtra()
{
    if ( extra == 0 )
	extra = new DocPrivateExtra;
}

class DocParser
{
public:
    void parse( const QString& input, DocPrivate *docPrivate,
		const StringSet& metaCommandSet );

private:
    const Location& location();
    void checkExpiry( const QString& date );
    void insertBaseName( const QString& baseName );
    void insertTarget( const QString& target );
    void include( const QString& fileName );
    void startFormat( const QString& format, const QString& command );
    void startHeading( Atom::Type leftType, Atom::Type rightType,
		       const QString& string = "" );
    void endHeading();
    bool openCommand( const QString& command );
    bool closeCommand( const QString& command );
    void startSection( Doc::SectioningUnit unit, const QString& command );
    void endSection( int unit, const QString& command );
    void parseAlso();
    void append( Atom::Type type, const QString& string = "" );
    void appendChar( QChar ch );
    void appendToCode( const QString& code );
    void startNewParagraph();
    void enterParagraph();
    void leaveParagraph();
    void quoteFromFile( const QString& command );
    Doc::SectioningUnit getSectioningUnit();
    QString getArgument( bool code = FALSE );
    QString getOptionalArgument();
    QString getRestOfLine();
    bool isBlankLine();
    bool isLeftBraceAhead();
    void skipSpacesOnLine();
    void skipSpacesOrOneEndl();
    void skipAllSpaces();

    QString in;
    int pos;
    int len;
    Location cachedLoc;
    int cachedPos;
    DocPrivate *priv;
    bool inPara;
    bool indexStartedPara; // ### rename
    int braceDepth;
    int minIndent;
    Doc::SectioningUnit currentSectioningUnit;
    QMap<QString, Location> targetMap;
    QMap<int, QString> pendingFormats;
    bool inHeading;
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
    priv->molecule << Atom::Nop;
    inPara = FALSE;
    indexStartedPara = FALSE;
    braceDepth = 0;
    minIndent = INT_MAX;
    currentSectioningUnit = Doc::Book;
    inHeading = FALSE;
    pendingHeadingRightType = Atom::Nop;
    openedCommands.push( "doc" );
    quoter.reset();

    const CodeMarker *marker;
    QString link;
    QString x;
    int begin;
    int end;
    int indent;

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
			skipAllSpaces();
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
		    parseAlso();
		    break;
		case HASH( 'a', 8 ):
		    CONSUME( "abstract" );
		    if ( openCommand("abstract") ) {
			leaveParagraph();
			append( Atom::AbstractBegin );
		    }
		    break;
		case HASH( 'b', 1 ):
		    CONSUME( "b" );
		    startFormat( "bold", "b" );
		    break;
		case HASH( 'b', 5 ):
		    CONSUME( "brief" );
		    startHeading( Atom::BriefBegin, Atom::BriefEnd );
		    break;
		case HASH( 'b', 8 ):
		    CONSUME( "basename" );
		    leaveParagraph();
		    insertBaseName( getArgument() );
		    break;
		case HASH( 'c', 1 ):
		    CONSUME( "c" );
		    enterParagraph();
		    x = untabify( getArgument(TRUE) );
		    marker = CodeMarker::markerForCode( x );
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
			x = untabify( in.mid(begin, end - begin) + "\n" );
			indent = indentLevel( x );
			if ( indent < minIndent )
			    minIndent = indent;
			x = unindent( minIndent, x );
			marker = CodeMarker::markerForCode( x );
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
			startSection( Doc::Chapter, "chapter" );
		    }
		    break;
		case HASH( 'c', 8 ):
		    CONSUME( "citation" );
		    if ( openCommand("citation") ) {
			leaveParagraph();
			append( Atom::CitationBegin );
		    }
		    break;
		case HASH( 'e', 6 ):
		    CONSUME( "expiry" );
		    checkExpiry( getArgument() );
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
		case HASH( 'g', 11 ):
		    CONSUME( "granularity" );
		    priv->constructExtra();
		    priv->extra->granularity = getSectioningUnit();
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
		    startFormat( "italic", "i" );
		    break;
		case HASH( 'i', 5 ):
		    if ( command[1] == 'm' ) {
			CONSUME( "image" );
			/* ... */
		    } else {
			CONSUME( "index" );
			if ( inPara ) {
			    const Atom *last = priv->molecule.lastAtom();
			    if ( indexStartedPara &&
				 (last->type() != Atom::FormatEnd ||
				  last->string() != "index") )
				indexStartedPara = FALSE;
			} else {
			    enterParagraph();
			    indexStartedPara = TRUE;
			}
			startFormat( "index", "index" );
		    }
		    break;
		case HASH( 'i', 7 ):
		    CONSUME( "include" );
		    include( getArgument() );
		    break;
		case HASH( 'k', 7 ):
		    CONSUME( "keyword" );
		    x = getArgument();
		    insertTarget( x );
		    break;
		case HASH( 'l', 1 ):
		    CONSUME( "l" );
		    enterParagraph();
		    if ( isLeftBraceAhead() ) {
			x = getArgument();
			append( Atom::Link, x );
			if ( isLeftBraceAhead() ) {
			    startFormat( "link", "l" );
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
						     getOptionalArgument()) );
		    }
		    break;
		case HASH( 'l', 8 ):
		    CONSUME( "location" );
		    cachedLoc = Location::fromString( getArgument() );
		    cachedPos = pos;
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
			skipSpacesOrOneEndl();
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
		    startSection( Doc::Part, "part" );
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
		case HASH( 's', 3 ):
		    if ( command[2] == 'b' ) {
			CONSUME( "sub" );
			startFormat( "subscript", "sub" );
		    } else {
			CONSUME( "sup" );
			startFormat( "superscript", "sup" );
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
			startSection( Doc::Section1, "section1" );
		    } else if ( command[7] == '2' ) {
			CONSUME( "section2" );
			startSection( Doc::Section2, "section2" );
		    } else if ( command[7] == '3' ) {
			CONSUME( "section3" );
			startSection( Doc::Section3, "section3" );
		    } else if ( command[7] == '4' ) {
			CONSUME( "section4" );
			startSection( Doc::Section4, "section4" );
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
		    startFormat( "teletype", "tt" );
		    break;
		case HASH( 't', 5 ):
		    CONSUME( "table" );
		    if ( openCommand("table") ) {
			leaveParagraph();
			append( Atom::TableBegin );
		    }
		    break;
		case HASH( 't', 6 ):
		    CONSUME( "target" );
		    insertTarget( getArgument() );
		    break;
		case HASH( 't', 15 ):
		    CONSUME( "tableofcontents" );
		    append( Atom::TableOfContents,
			    QString::number((int) getSectioningUnit()) );
		    /* ... */
		    break;
		case HASH( 'u', 9 ):
		    CONSUME( "underline" );
		    startFormat( "underlined", "underline" );
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
			enterParagraph();
			append( Atom::UnknownCommand, command );
		    }
		}
	    }
	} else if ( ch == '{' ) {
	    enterParagraph();
	    appendChar( '{' );
	    braceDepth++;
	    pos++;
	} else if ( ch == '}' ) {
	    braceDepth--;
	    pos++;

	    QMap<int, QString>::Iterator f = pendingFormats.find( braceDepth );
	    if ( f == pendingFormats.end() ) {
		enterParagraph();
		appendChar( '}' );
	    } else {
		append( Atom::FormatEnd, *f );
		if ( *f == "index" && indexStartedPara )
		    skipAllSpaces();
		pendingFormats.remove( f );
	    }
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

    if ( openedCommands.top() != "doc" )
	warning( 1, location(), "Missing '\\end%s'",
		 openedCommands.top().latin1() );

    if ( priv->extra != 0 &&
	 priv->extra->granularity < priv->extra->sectioningUnit )
	priv->extra->granularity = priv->extra->sectioningUnit;
}

const Location& DocParser::location()
{
    while ( cachedPos < pos )
	cachedLoc.advance( in[cachedPos++] );
    return cachedLoc;
}

void DocParser::checkExpiry( const QString& date )
{
    QRegExp ymd( "(\\d{4})(?:-(\\d{2})(?:-(\\d{2})))" );

    if ( ymd.exactMatch(date) ) {
	int y = ymd.cap( 1 ).toInt();
	int m = ymd.cap( 2 ).toInt();
	int d = ymd.cap( 3 ).toInt();

	if ( m == 0 )
	    m = 1;
	if ( d == 0 )
	    d = 1;
	QDate expiryDate( y, m, d );
	if ( !expiryDate.isValid() ) {
	    warning( 1, location(),
		     "Invalid date '%s' specified with '\\expiry'",
		     date.latin1() );
	} else {
	    int days = expiryDate.daysTo( QDate::currentDate() );
	    if ( days == 0 ) {
		warning( 1, location(), "Documentation expires today" );
	    } else if ( days == 1 ) {
		warning( 1, location(), "Documentation expired yesterday" );
	    } else if ( days >= 2 ) {
		warning( 1, location(), "Documentation expired %d days ago",
			 days );
	    }
	}
    } else {
	warning( 1, location(),
		 "Date '%s' specified with '\\expiry' not in YYYY-MM-DD format",
		 date.latin1() );
    }
}

void DocParser::insertBaseName( const QString& baseName )
{
    priv->constructExtra();
    if ( currentSectioningUnit == priv->extra->sectioningUnit ) {
	priv->extra->baseName = baseName;
    } else {
	Atom *atom = priv->molecule.firstAtom();
	Atom *sectionBegin = 0;

	int delta = currentSectioningUnit - priv->extra->sectioningUnit;

	while ( atom != 0 ) {
	    if ( atom->type() == Atom::SectionBegin &&
		 atom->string().toInt() == delta )
		sectionBegin = atom;
	    atom = atom->next();
	}
	if ( sectionBegin != 0 )
	    (void) new Atom( sectionBegin, Atom::BaseName, baseName );
    }
}

void DocParser::insertTarget( const QString& target )
{
    if ( targetMap.contains(target) ) {
	warning( 1, location(), "Duplicate target name" );
	warning( 1, targetMap[target], "(the previous occurrence is here)" );
    } else {
	targetMap.insert( target, location() );
	append( Atom::Target, target );
    }
}

void DocParser::include( const QString& fileName )
{
    QFile inFile( "src/" + fileName );
    if ( !inFile.open(IO_ReadOnly) ) {
	warning( 0, location(), "Cannot open leaf file '%s'",
		 fileName.latin1() );
    } else {
	QString s = inFile.readAll();
	s.prepend( "\\location {" + Location(fileName).toString() + "}" );
	s.append( "\\location {" + location().toString() + "}" );

	in.insert( pos, s );
	len = in.length();
    }
}

void DocParser::startFormat( const QString& format, const QString& command )
{
    enterParagraph();

    QMap<int, QString>::ConstIterator f = pendingFormats.begin();
    while ( f != pendingFormats.end() ) {
	if ( *f == format ) {
	    warning( 1, location(), "Cannot nest '\\%s' commands",
		     command.latin1() );
	    return;
	}
	++f;
    }

    append( Atom::FormatBegin, format );

    if ( isLeftBraceAhead() ) {
	skipSpacesOrOneEndl();
	pendingFormats.insert( braceDepth, format );
	braceDepth++;
	pos++;
    } else {
	append( Atom::String, getArgument() );
	append( Atom::FormatEnd, format );
	if ( format == "index" && indexStartedPara ) {
	    skipAllSpaces();
	    indexStartedPara = FALSE;
	}
    }
}

void DocParser::startHeading( Atom::Type leftType, Atom::Type rightType,
			      const QString& string )
{
    leaveParagraph();
    skipSpacesOnLine();
    if ( pos < len && in[pos] != '\n' ) {
	append( leftType, string );
	inPara = TRUE;
	indexStartedPara = FALSE;
	inHeading = TRUE;
	pendingHeadingRightType = rightType;
	pendingHeadingString = string;
    }
}

void DocParser::endHeading()
{
    if ( inHeading ) {
	append( pendingHeadingRightType, pendingHeadingString );
	inPara = FALSE;
	indexStartedPara = FALSE;
	inHeading = FALSE;
	pendingHeadingRightType = Atom::Nop;
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

void DocParser::startSection( Doc::SectioningUnit unit, const QString& command )
{
    leaveParagraph();

    if ( currentSectioningUnit == Doc::Book ) {
	if ( unit > Doc::Section1 )
	    warning( 1, location(), "Unexpected '\\%1' without '\\section1'",
		     command.latin1() );
	currentSectioningUnit = (Doc::SectioningUnit) ( unit - 1 );
	priv->constructExtra();
	priv->extra->sectioningUnit = currentSectioningUnit;
    }

    if ( unit <= priv->extra->sectioningUnit ) {
	warning( 0, location(), "Unexpected '\\%s' in this documentation",
		 command.latin1() );
    } else if ( unit - currentSectioningUnit > 1 ) {
	warning( 0, location(), "Unexpected '\\%s' at this point",
		 command.latin1() );
    } else {
	if ( currentSectioningUnit >= unit )
	    endSection( unit, command );

	int delta = unit - priv->extra->sectioningUnit;
	append( Atom::SectionBegin, QString::number(delta) );
	startHeading( Atom::SectionHeadingBegin, Atom::SectionHeadingEnd,
		      QString::number(delta) );
	currentSectioningUnit = unit;
    }
}

void DocParser::endSection( int unit, const QString& command )
{
    leaveParagraph();

    if ( unit < priv->extra->sectioningUnit ) {
	warning( 0, location(), "Unexpected '\\end%s' in this documentation",
		 command.latin1() );
    } else if ( unit > currentSectioningUnit ) {
	warning( 0, location(), "Unexpected '\\end%s' at this point",
		 command.latin1() );
    } else {
	while ( currentSectioningUnit >= unit ) {
	    int delta = currentSectioningUnit - priv->extra->sectioningUnit;
	    append( Atom::SectionEnd, QString::number(delta) );
	    currentSectioningUnit =
		    (Doc::SectioningUnit) ( currentSectioningUnit - 1 );
	}
    }
}

void DocParser::parseAlso()
{
    leaveParagraph();
    skipSpacesOnLine();
    while ( pos < len && in[pos] != '\n' ) {
	QString target;
	QString text;

	if ( in[pos] == '{' ) {
	    target = getArgument();
	    skipSpacesOnLine();
	    if ( in[pos] == '{' ) {
		text = getArgument();
	    } else {
		text = target;
	    }
	} else {
	    target = getArgument();
	    text = target;
	}

	Molecule also;
	also << Atom( Atom::Link, target ) << Atom( Atom::FormatBegin, "link" )
	     << text << Atom( Atom::FormatEnd, "link" );
	priv->addAlso( also );

	skipSpacesOnLine();
	if ( pos < len && in[pos] == ',' ) {
	    pos++;
	    skipSpacesOrOneEndl();
	} else if ( in[pos] != '\n' ) {
	    warning( 1, location(), "Missing comma in '\\also'" );
	}
    }
}

void DocParser::append( Atom::Type type, const QString& string )
{
    if ( priv->molecule.lastAtom()->type() == Atom::Code &&
	 priv->molecule.lastAtom()->string().endsWith("\n\n") )
	priv->molecule.lastAtom()->chopString();
    priv->molecule << Atom( type, string );
}

void DocParser::appendChar( QChar ch )
{
    if ( priv->molecule.lastAtom()->type() != Atom::String )
	append( Atom::String );
    if ( ch.isSpace() ) {
	if ( !priv->molecule.lastAtom()->string().endsWith(" ") )
	    priv->molecule.lastAtom()->appendChar( ' ' );
    } else {
	priv->molecule.lastAtom()->appendChar( ch );
    }
}

void DocParser::appendToCode( const QString& markedCode )
{
    if ( priv->molecule.lastAtom()->type() != Atom::Code )
	append( Atom::Code );
    priv->molecule.lastAtom()->appendString( markedCode );
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
	indexStartedPara = FALSE;
    }
}

void DocParser::leaveParagraph()
{
    if ( inPara ) {
	if ( inHeading ) {
	    endHeading();
	} else {
	    if ( !pendingFormats.isEmpty() ) {
		warning( 1, location(), "Missing '}'" );
		pendingFormats.clear();
	    }

	    if ( priv->molecule.lastAtom()->type() == Atom::String &&
		 priv->molecule.lastAtom()->string().endsWith(" ") )
		priv->molecule.lastAtom()->chopString();
	    append( Atom::ParagraphEnd );
	    inPara = FALSE;
	    indexStartedPara = FALSE;
	}
    }
}

void DocParser::quoteFromFile( const QString& /* command ### */ )
{
    quoter.reset();

    QString code;
    QString fileName = getArgument();

    QString filePath = fileName; /* ### */
    QFile inFile( filePath );
    if ( !inFile.open(IO_ReadOnly) ) {
	warning( 1, location(), "Cannot open example file '%s'",
		 filePath.latin1() );
    } else {
	QTextStream inStream( &inFile );
	code = untabify( inStream.read() );
	inFile.close();
    }

    QString dirPath = QFileInfo( filePath ).dirPath();
    const CodeMarker *marker = CodeMarker::markerForFileName( fileName );
    quoter.quoteFromFile( filePath, code,
			  marker->markedUpCode(code, 0, dirPath) );
}

Doc::SectioningUnit DocParser::getSectioningUnit()
{
    QString name = getArgument();

    if ( name == "part" ) {
	return Doc::Part;
    } else if ( name == "chapter" ) {
	return Doc::Chapter;
    } else if ( name == "section1" ) {
	return Doc::Section1;
    } else if ( name == "section2" ) {
	return Doc::Section2;
    } else if ( name == "section3" ) {
	return Doc::Section3;
    } else if ( name == "section4" ) {
	return Doc::Section4;
    } else {
	warning( 1, location(), "Invalid sectioning unit '%s'",
		 name.latin1() );
	return Doc::Book;
    }
}

QString DocParser::getArgument( bool code )
{
    QString arg;
    int delimDepth = 0;

    skipSpacesOrOneEndl();

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
	while ( pos < (int) in.length() && delimDepth >= 0 ) {
	    switch ( in[pos].unicode() ) {
	    case '{':
		delimDepth++;
		arg += "{";
		pos++;
		break;
	    case '}':
		delimDepth--;
		if ( delimDepth >= 0 )
		    arg += "}";
		pos++;
		break;
	    case '\\':
		if ( code ) {
		    arg += in[pos];
		    pos++;
		} else {
		    pos++;
		    if ( pos < (int) in.length() ) {
			if ( in[pos].isLetterOrNumber() )
			    break;
			arg += in[pos];
			if ( in[pos].isSpace() ) {
			    skipAllSpaces();
			} else {
			    pos++;
			}
		    }
		}
		break;
	    default:
		arg += in[pos];
		pos++;
	    }
	}
	if ( delimDepth > 0 )
	    warning( 1, location(), "Missing '}'" );
    } else {
	while ( pos < (int) in.length() &&
		(delimDepth > 0 || (delimDepth == 0 && !in[pos].isSpace())) ) {
	    switch ( in[pos].unicode() ) {
	    case '(':
	    case '[':
	    case '{':
		delimDepth++;
		arg += in[pos];
		pos++;
		break;
	    case ')':
	    case ']':
	    case '}':
		delimDepth--;
		if ( delimDepth >= 0 ) {
		    arg += in[pos];
		    pos++;
		}
		break;
	    case '\\':
		if ( code ) {
		    arg += in[pos];
		    pos++;
		} else {
		    pos++;
		    if ( pos < (int) in.length() ) {
			if ( in[pos].isLetterOrNumber() )
			    break;
			arg += in[pos];
			if ( in[pos].isSpace() ) {
			    skipAllSpaces();
			} else {
			    pos++;
			}
		    }
		}
		break;
	    default:
		arg += in[pos];
		pos++;
	    }
	}

	if ( arg.length() > 1 && QString(".,:;!?").find(in[pos - 1]) != -1 &&
	     !arg.endsWith("...") ) {
	    arg.truncate( arg.length() - 1 );
	    pos--;
	}
	if ( arg.length() > 2 && in.mid(pos - 2, 2) == "'s" ) {
	    arg.truncate( arg.length() - 2 );
	    pos -= 2;
	}
    }
    return arg.simplifyWhiteSpace();
}

QString DocParser::getOptionalArgument()
{
    skipSpacesOrOneEndl();
    if ( pos + 1 < (int) in.length() && in[pos] == '\\' &&
	 in[pos + 1].isLetterOrNumber() ) {
	return "";
    } else {
	return getArgument();
    }
}

QString DocParser::getRestOfLine()
{
    skipSpacesOnLine();

    int begin = pos;

    while ( pos < (int) in.length() && in[pos] != '\n' )
	pos++;

    QString t = in.mid( begin, pos - begin ).simplifyWhiteSpace();
    skipSpacesOnLine();
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
	// ### bug with '\\'
	if ( in[i] == '\n' )
	    numEndl++;
    	i++;
    }
    return numEndl < 2 && i < len && in[i] == '{';
}

void DocParser::skipSpacesOnLine()
{
    while ( pos < (int) in.length() && in[pos].isSpace() &&
	    in[pos].unicode() != '\n' )
	pos++;
}

void DocParser::skipSpacesOrOneEndl()
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

void DocParser::skipAllSpaces()
{
    while ( pos < len && in[pos].isSpace() )
	pos++;
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

// ### change addAlso and extractMetaCommand to avoid detach()
void Doc::addAlso( const Molecule& also )
{
    priv->addAlso( also );
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
    return body().isEmpty();
}

const StringSet *Doc::metaCommands() const
{
    return priv->metaCommandSet;
}

const Molecule& Doc::body() const
{
    return priv->molecule;
}

const QString& Doc::baseName() const
{
    if ( priv->extra == 0 ) {
	return QString();
    } else {
	return priv->extra->baseName;
    }
}

Doc::SectioningUnit Doc::granularity() const
{
    if ( priv->extra == 0 ) {
	return DocPrivateExtra().granularity;
    } else {
	return priv->extra->granularity;
    }
}

Doc::SectioningUnit Doc::sectioningUnit() const
{
    if ( priv->extra == 0 ) {
	return DocPrivateExtra().sectioningUnit;
    } else {
	return priv->extra->sectioningUnit;
    }
}

const QValueList<Molecule> *Doc::alsoList() const
{
    return priv->alsoList;
}

Doc Doc::propertyFunctionDoc( const Doc& propertyDoc, const QString& role,
			      const QString& param )
{
    Doc doc;
    doc.priv->loc = propertyDoc.location();

    Molecule brief = propertyDoc.body().subMolecule( Atom::BriefBegin,
						     Atom::BriefEnd );
    doc.priv->molecule << Atom::ParagraphBegin;
    if ( !brief.isEmpty() ) {
	bool whether = ( brief.firstAtom()->type() == Atom::String &&
			 brief.firstAtom()->string().lower()
			      .startsWith("whether") );

	if ( role == "getter" ) {
	    if ( whether ) {
		doc.priv->molecule << "Returns TRUE if"
				   << brief.firstAtom()->string().mid( 7 )
				   << brief.subMolecule(
					      brief.firstAtom()->next() )
				   << "; otherwise returns FALSE.";
	    } else {
		doc.priv->molecule << "Returns " << brief << ".";
	    }
	} else if ( role == "setter" ) {
	    if ( param.isEmpty() ) {
		doc.priv->molecule << "Sets " << brief << ".";
	    } else {
		doc.priv->molecule << "Sets " << brief << " to "
				   << Atom( Atom::FormatBegin, "parameter" )
				   << param
				   << Atom( Atom::FormatEnd, "parameter" )
				   << ".";
	    }
	} else if ( role == "resetter" ) {
	    doc.priv->molecule << "Resets " << brief << ".";
	}
    }
    doc.priv->molecule << Atom::ParagraphEnd;
    // set priv->params
    return doc;
}
