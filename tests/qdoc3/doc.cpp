/*
  doc.cpp
*/

#include <qdatetime.h>
#include <qdict.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qregexp.h>

#include "codemarker.h"
#include "config.h"
#include "doc.h"
#include "location.h"
#include "messages.h"
#include "openedlist.h"
#include "quoter.h"
#include "text.h"

enum {
    COMMAND_A, COMMAND_ABSTRACT, COMMAND_ALSO, COMMAND_BASENAME,
    COMMAND_BOLD, COMMAND_BRIEF, COMMAND_C, COMMAND_CAPTION,
    COMMAND_CHAPTER, COMMAND_CODE, COMMAND_ENDABSTRACT,
    COMMAND_ENDCHAPTER, COMMAND_ENDCODE, COMMAND_ENDFOOTNOTE,
    COMMAND_ENDLIST, COMMAND_ENDOMIT, COMMAND_ENDPART,
    COMMAND_ENDQUOTATION, COMMAND_ENDSECTION1, COMMAND_ENDSECTION2,
    COMMAND_ENDSECTION3, COMMAND_ENDSECTION4, COMMAND_ENDSIDEBAR,
    COMMAND_ENDTABLE, COMMAND_EXPIRE, COMMAND_FOOTNOTE,
    COMMAND_GRANULARITY, COMMAND_HEADER, COMMAND_I, COMMAND_IMAGE,
    COMMAND_INCLUDE, COMMAND_INDEX, COMMAND_KEYWORD, COMMAND_L,
    COMMAND_LIST, COMMAND_O, COMMAND_OMIT, COMMAND_PART,
    COMMAND_QUOTATION, COMMAND_QUOTEFILE, COMMAND_QUOTEFROMFILE,
    COMMAND_QUOTEFUNCTION, COMMAND_QUOTELINE, COMMAND_QUOTETO,
    COMMAND_QUOTEUNTIL, COMMAND_RAW, COMMAND_ROW, COMMAND_SECTION1,
    COMMAND_SECTION2, COMMAND_SECTION3, COMMAND_SECTION4,
    COMMAND_SIDEBAR, COMMAND_SKIPLINE, COMMAND_SKIPTO,
    COMMAND_SKIPUNTIL, COMMAND_SUB, COMMAND_SUP, COMMAND_TABLE,
    COMMAND_TABLEOFCONTENTS, COMMAND_TARGET, COMMAND_TT,
    COMMAND_UNDERLINE, COMMAND_VALUE, COMMAND_WARNING,
    UNKNOWN_COMMAND
};

static struct {
    const char *english;
    int no;
    QString translation;
} cmds[] = {
    { "a", COMMAND_A, 0 },
    { "abstract", COMMAND_ABSTRACT, 0 },
    { "also", COMMAND_ALSO, 0 },
    { "basename", COMMAND_BASENAME, 0 },
    { "bold", COMMAND_BOLD, 0 },
    { "brief", COMMAND_BRIEF, 0 },
    { "c", COMMAND_C, 0 },
    { "caption", COMMAND_CAPTION, 0 },
    { "chapter", COMMAND_CHAPTER, 0 },
    { "code", COMMAND_CODE, 0 },
    { "endabstract", COMMAND_ENDABSTRACT, 0 },
    { "endchapter", COMMAND_ENDCHAPTER, 0 },
    { "endcode", COMMAND_ENDCODE, 0 },
    { "endfootnote", COMMAND_ENDFOOTNOTE, 0 },
    { "endlist", COMMAND_ENDLIST, 0 },
    { "endomit", COMMAND_ENDOMIT, 0 },
    { "endpart", COMMAND_ENDPART, 0 },
    { "endquotation", COMMAND_ENDQUOTATION, 0 },
    { "endsection1", COMMAND_ENDSECTION1, 0 },
    { "endsection2", COMMAND_ENDSECTION2, 0 },
    { "endsection3", COMMAND_ENDSECTION3, 0 },
    { "endsection4", COMMAND_ENDSECTION4, 0 },
    { "endsidebar", COMMAND_ENDSIDEBAR, 0 },
    { "endtable", COMMAND_ENDTABLE, 0 },
    { "expire", COMMAND_EXPIRE, 0 },
    { "footnote", COMMAND_FOOTNOTE, 0 },
    { "granularity", COMMAND_GRANULARITY, 0 },
    { "header", COMMAND_HEADER, 0 },
    { "i", COMMAND_I, 0 },
    { "image", COMMAND_IMAGE, 0 },
    { "include", COMMAND_INCLUDE, 0 },
    { "index", COMMAND_INDEX, 0 },
    { "keyword", COMMAND_KEYWORD, 0 },
    { "l", COMMAND_L, 0 },
    { "list", COMMAND_LIST, 0 },
    { "o", COMMAND_O, 0 },
    { "omit", COMMAND_OMIT, 0 },
    { "part", COMMAND_PART, 0 },
    { "quotation", COMMAND_QUOTATION, 0 },
    { "quotefile", COMMAND_QUOTEFILE, 0 },
    { "quotefromfile", COMMAND_QUOTEFROMFILE, 0 },
    { "quotefunction", COMMAND_QUOTEFUNCTION, 0 },
    { "quoteline", COMMAND_QUOTELINE, 0 },
    { "quoteto", COMMAND_QUOTETO, 0 },
    { "quoteuntil", COMMAND_QUOTEUNTIL, 0 },
    { "raw", COMMAND_RAW, 0 },
    { "row", COMMAND_ROW, 0 },
    { "section1", COMMAND_SECTION1, 0 },
    { "section2", COMMAND_SECTION2, 0 },
    { "section3", COMMAND_SECTION3, 0 },
    { "section4", COMMAND_SECTION4, 0 },
    { "sidebar", COMMAND_SIDEBAR, 0 },
    { "skipline", COMMAND_SKIPLINE, 0 },
    { "skipto", COMMAND_SKIPTO, 0 },
    { "skipuntil", COMMAND_SKIPUNTIL, 0 },
    { "sub", COMMAND_SUB, 0 },
    { "sup", COMMAND_SUP, 0 },
    { "table", COMMAND_TABLE, 0 },
    { "tableofcontents", COMMAND_TABLEOFCONTENTS, 0 },
    { "target", COMMAND_TARGET, 0 },
    { "tt", COMMAND_TT, 0 },
    { "underline", COMMAND_UNDERLINE, 0 },
    { "value", COMMAND_VALUE, 0 },
    { "warning", COMMAND_WARNING, 0 },
    { 0, 0, 0 }
};

static int endCommandOf( int command )
{
    switch ( command ) {
    case COMMAND_ABSTRACT:
	return COMMAND_ENDABSTRACT;
    case COMMAND_CHAPTER:
	return COMMAND_ENDCHAPTER;
    case COMMAND_CODE:
	return COMMAND_ENDCODE;
    case COMMAND_FOOTNOTE:
	return COMMAND_ENDFOOTNOTE;
    case COMMAND_LIST:
	return COMMAND_ENDLIST;
    case COMMAND_OMIT:
	return COMMAND_ENDOMIT;
    case COMMAND_PART:
	return COMMAND_ENDPART;
    case COMMAND_QUOTATION:
	return COMMAND_ENDQUOTATION;
    case COMMAND_SECTION1:
	return COMMAND_ENDSECTION1;
    case COMMAND_SECTION2:
	return COMMAND_ENDSECTION2;
    case COMMAND_SECTION3:
	return COMMAND_ENDSECTION3;
    case COMMAND_SECTION4:
	return COMMAND_ENDSECTION4;
    case COMMAND_SIDEBAR:
	return COMMAND_ENDSIDEBAR;
    case COMMAND_TABLE:
	return COMMAND_ENDTABLE;
    default:
#if 0 // ###
	if ( !deja ) {
	    i = 0;
	    while ( cmds[i].english != 0 ) {
		if ( qstrncmp(cmds[i].english, "end", 3) == 0 &&
		     qstrcmp(cmds[i].english + 3, cmds[command].english) == 0 )
		    Messages::internalError( Qdoc::tr("'\\%1' end of nothing")
					     .arg(cmds[i].english) );
		i++;
	    }
	    deja = TRUE;
	}
#endif
	return command;
    }
}

struct Macro
{
    QString defaultDef;
    QMap<QString, QString> otherDefs;
    int numParams;
};

static QDict<int> *commandDict = 0;
static QDict<Macro> *macroDict = 0;

static QString commandName( int command )
{
    return cmds[command].translation;
}

static QString endCommandName( int command )
{
    return commandName( endCommandOf(command) );
}

static int numParams( const QString& str )
{
    int max = 0;
    for ( int i = 0; i < (int) str.length(); i++ ) {
	if ( str[i].unicode() > 0 && str[i].unicode() < 8 )
	    max = QMAX( max, str[i].unicode() );
    }
    return max;
}

static QString untabifyEtc( const QString& str )
{
    QString result;
    int column = 0;

    for ( int i = 0; i < (int) str.length(); i++ ) {
	if ( str[i] == '\t' ) {
	    result += "        " + ( column & 0x7 );
	    column = ( column + 8 ) & ~0x7;
	} else {
	    result += str[i];
	    if ( str[i] == '\n' )
		column = 0;
	    else
		column++;
	}
    }

    result.replace( QRegExp(" +\n"), "\n" );
    while ( result.endsWith("\n\n") )
	result.truncate( result.length() - 1 );
    while ( result.startsWith("\n") )
	result = result.mid( 1 );
    return result;
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

static int editDistance( const QString& s, const QString& t )
{
#define D( i, j ) d[(i) * n + (j)]
    int i;
    int j;
    int m = s.length() + 1;
    int n = t.length() + 1;
    int *d = new int[m * n];
    int result;

    for ( i = 0; i < m; i++ )
	D( i, 0 ) = i;
    for ( j = 0; j < n; j++ )
	D( 0, j ) = j;
    for ( i = 1; i < m; i++ ) {
	for ( j = 1; j < n; j++ ) {
	    if ( s[i - 1] == t[j - 1] ) {
		D( i, j ) = D( i - 1, j - 1 );
	    } else {
		int x = D( i - 1, j );
		int y = D( i - 1, j - 1 );
		int z = D( i, j - 1 );
		D( i, j ) = 1 + QMIN( QMIN(x, y), z );
	    }
	}
    }
    result = D( m - 1, n - 1 );
    delete[] d;
    return result;
#undef D
}

class DocPrivateExtra
{
public:
    QString baseName;
    Doc::SectioningUnit granularity;
    Doc::SectioningUnit sectioningUnit; // ###

    DocPrivateExtra()
	: granularity( Doc::Part ) { }
};

class DocPrivate : public QShared
{
public:
    DocPrivate( const Location& location = Location::null );
    ~DocPrivate();

    void addAlso( const Text& also );
    void constructExtra();

    Location loc;
    Text text;
    Set<QString> *params;
    QValueList<Text> *alsoList;
    Set<QString> *metaCommandSet;
    QMap<QString, QStringList> *metaCommandMap;
    DocPrivateExtra *extra;
};

DocPrivate::DocPrivate( const Location& location )
    : loc( location ), params( 0 ), alsoList( 0 ), metaCommandSet( 0 ),
      metaCommandMap( 0 ), extra( 0 )
{
}

DocPrivate::~DocPrivate()
{
    delete params;
    delete alsoList;
    delete metaCommandSet;
    delete metaCommandMap;
    delete extra;
}

void DocPrivate::addAlso( const Text& also )
{
    if ( alsoList == 0 )
	alsoList = new QValueList<Text>;
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
		const Set<QString>& metaCommandSet );

private:
    const Location& location();
    QString detailsUnknownCommand( const Set<QString>& metaCommandSet,
				   const QString& str );
    void checkExpiry( const QString& date );
    void insertBaseName( const QString& baseName );
    void insertTarget( const QString& target );
    void include( const QString& fileName );
    void startFormat( const QString& format, int command );
    void startHeading( Atom::Type leftType, Atom::Type rightType,
		       const QString& string = "" );
    void endHeading();
    bool openCommand( int command );
    bool closeCommand( int endCommand );
    void startSection( Doc::SectioningUnit unit, int command );
    void endSection( int unit, int endCommand );
    void parseAlso();
    void append( Atom::Type type, const QString& string = "" );
    void appendChar( QChar ch );
    void appendToCode( const QString& code );
    void startNewParagraph();
    void enterParagraph();
    void leaveParagraph();
    void quoteFromFile( int command );
    Doc::SectioningUnit getSectioningUnit();
    QString getArgument( bool code = FALSE );
    QString getOptionalArgument();
    QString getRestOfLine();
    QString getUntilRight( int command );
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
    QValueStack<int> openedCommands;
    QValueStack<OpenedList> openedLists;
    Quoter quoter;
};

void DocParser::parse( const QString& input, DocPrivate *docPrivate,
		       const Set<QString>& metaCommandSet )
{
    in = input;
    pos = 0;
    len = (int) input.length();
    cachedLoc = docPrivate->loc;
    cachedPos = 0;
    priv = docPrivate;
    priv->text << Atom::Nop;
    inPara = FALSE;
    indexStartedPara = FALSE;
    braceDepth = 0;
    minIndent = INT_MAX;
    currentSectioningUnit = Doc::Book;
    inHeading = FALSE;
    pendingHeadingRightType = Atom::Nop;
    openedCommands.push( COMMAND_OMIT );
    quoter.reset();

    CodeMarker *marker;
    QString link;
    QString x;
    int begin;
    int end;
    int indent;

    while ( pos < len ) {
	QChar ch = in[pos];

	if ( ch == '\\' ) {
	    QString commandStr;
	    pos++;
	    begin = pos;
	    while ( pos < len ) {
		ch = in[pos];
		if ( ch.isLetterOrNumber() ) {
		    commandStr += ch;
		    pos++;
		} else {
		    break;
		}
	    }
	    if ( commandStr.isEmpty() ) {
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
		int *entry = commandDict->find( commandStr );
		int command = ( entry != 0 ) ? *entry : UNKNOWN_COMMAND;

		switch ( command ) {
		case COMMAND_A:
		    enterParagraph();
		    x = getArgument();
		    append( Atom::FormatLeft, "parameter" );
		    append( Atom::String, x );
		    append( Atom::FormatRight, "parameter" );
		    if ( priv->params == 0 )
			priv->params = new Set<QString>;
		    priv->params->insert( x );
		    break;
		case COMMAND_ABSTRACT:
		    if ( openCommand(command) ) {
			leaveParagraph();
			append( Atom::AbstractLeft );
		    }
		    break;
		case COMMAND_ALSO:
		    parseAlso();
		    break;
		case COMMAND_BASENAME:
		    leaveParagraph();
		    insertBaseName( getArgument() );
		    break;
		case COMMAND_BOLD:
		    startFormat( ATOM_FORMAT_BOLD, command );
		    break;
		case COMMAND_BRIEF:
		    startHeading( Atom::BriefLeft, Atom::BriefRight );
		    break;
		case COMMAND_C:
		    enterParagraph();
		    x = untabifyEtc( getArgument(TRUE) );
		    marker = CodeMarker::markerForCode( x );
		    append( Atom::C, marker->markedUpCode(x, 0, "") );
		    break;
		case COMMAND_CAPTION:
		    leaveParagraph();
		    /* ... */
		    break;
		case COMMAND_CHAPTER:
		    startSection( Doc::Chapter, command );
		    break;
		case COMMAND_CODE:
		    leaveParagraph();
		    begin = pos;
		    x = getUntilRight( command );
		    x = untabifyEtc( x );
		    indent = indentLevel( x );
		    if ( indent < minIndent )
			minIndent = indent;
		    x = unindent( minIndent, x );
		    marker = CodeMarker::markerForCode( x );
		    append( Atom::Code, marker->markedUpCode(x, 0, "") );
		    break;
		case COMMAND_ENDABSTRACT:
		    if ( closeCommand(command) ) {
			leaveParagraph();
			append( Atom::AbstractRight );
		    }
		    break;
		case COMMAND_ENDCHAPTER:
		    endSection( 0, command );
		    break;
		case COMMAND_ENDCODE:
		    closeCommand( command );
		    break;
		case COMMAND_ENDFOOTNOTE:
		    if ( closeCommand(command) ) {
			leaveParagraph();
			append( Atom::FootnoteRight );
			inPara = TRUE;
		    }
		    break;
		case COMMAND_ENDLIST:
		    if ( closeCommand(command) ) {
			leaveParagraph();
			if ( openedLists.top().isStarted() ) {
			    append( Atom::ListItemRight,
				    openedLists.top().numberString() );
			    append( Atom::ListRight,
				    openedLists.top().styleString() );
			}
			openedLists.pop();
		    }
		    break;
		case COMMAND_ENDOMIT:
		    closeCommand( command );
		    break;
		case COMMAND_ENDPART:
		    endSection( -1, command );
		    break;
		case COMMAND_ENDQUOTATION:
		    if ( closeCommand(command) ) {
			leaveParagraph();
			append( Atom::QuotationRight );
		    }
		    break;
		case COMMAND_ENDSECTION1:
		    endSection( 1, command );
		    break;
		case COMMAND_ENDSECTION2:
		    endSection( 2, command );
		    break;
		case COMMAND_ENDSECTION3:
		    endSection( 3, command );
		    break;
		case COMMAND_ENDSECTION4:
		    endSection( 4, command );
		    break;
		case COMMAND_ENDSIDEBAR:
		    if ( closeCommand(command) ) {
			leaveParagraph();
			append( Atom::SidebarRight );
		    }
		    break;
		case COMMAND_ENDTABLE:
		    if ( closeCommand(command) ) {
			append( Atom::TableRight );
			/* ... */
		    }
		    break;
		case COMMAND_EXPIRE:
		    checkExpiry( getArgument() );
		    break;
		case COMMAND_FOOTNOTE:
		    if ( openCommand(command) ) {
			enterParagraph();
			append( Atom::FootnoteLeft );
			inPara = FALSE;
		    }
		    break;
		case COMMAND_GRANULARITY:
		    priv->constructExtra();
		    priv->extra->granularity = getSectioningUnit();
		    break;
		case COMMAND_HEADER:
		    /* ... */
		    break;
		case COMMAND_I:
		    startFormat( ATOM_FORMAT_ITALIC, command );
		    break;
		case COMMAND_IMAGE:
		    /* ... */
		    break;
		case COMMAND_INCLUDE:
		    include( getArgument() );
		    break;
		case COMMAND_INDEX:
		    if ( inPara ) {
			const Atom *last = priv->text.lastAtom();
			if ( indexStartedPara &&
			     (last->type() != Atom::FormatRight ||
			      last->string() != ATOM_FORMAT_INDEX) )
			    indexStartedPara = FALSE;
		    } else {
			enterParagraph();
			indexStartedPara = TRUE;
		    }
		    startFormat( ATOM_FORMAT_INDEX, command );
		    break;
		case COMMAND_KEYWORD:
		    x = getArgument();
		    insertTarget( x );
		    break;
		case COMMAND_L:
		    enterParagraph();
		    if ( isLeftBraceAhead() ) {
			x = getArgument();
			append( Atom::Link, x );
			if ( isLeftBraceAhead() ) {
			    startFormat( ATOM_FORMAT_LINK, command );
			} else {
			    append( Atom::FormatLeft, ATOM_FORMAT_LINK );
			    append( Atom::String, x );
			    append( Atom::FormatRight, ATOM_FORMAT_LINK );	
			}
		    } else {
			x = getArgument();
			append( Atom::Link, x );
			append( Atom::FormatLeft, ATOM_FORMAT_LINK );
			append( Atom::String, x );
			append( Atom::FormatRight, ATOM_FORMAT_LINK );
		    }
		    break;
		case COMMAND_LIST:
		    if ( openCommand(command) ) {
			leaveParagraph();
			openedLists.push( OpenedList(location(),
						     getOptionalArgument()) );
		    }
		    break;
		case COMMAND_O:
		    leaveParagraph();
		    if ( openedLists.isEmpty() ) {
			Messages::warning( location(),
					   Qdoc::tr("Command '\\%1' outside"
						    " '\\%2'")
					   .arg(commandName(command))
					   .arg(commandName(COMMAND_LIST)) );
		    } else {
			if ( openedLists.top().isStarted() ) {
			    leaveParagraph();
			    append( Atom::ListItemRight,
				    openedLists.top().numberString() );
			} else {
			    append( Atom::ListLeft,
				    openedLists.top().styleString() );
			}
			openedLists.top().next();
			append( Atom::ListItemNumber,
				QString::number(openedLists.top().number()) );
			append( Atom::ListItemLeft,
				openedLists.top().numberString() );
			enterParagraph();
			skipSpacesOrOneEndl();
		    }
		    break;
		case COMMAND_OMIT:
		    getUntilRight( command );
		    break;
		case COMMAND_PART:
		    startSection( Doc::Part, command );
		    break;
		case COMMAND_QUOTATION:
		    if ( openCommand(command) ) {
			leaveParagraph();
			append( Atom::QuotationLeft );
		    }
		    break;
		case COMMAND_QUOTEFILE:
		    leaveParagraph();
		    quoteFromFile( command );
		    append( Atom::Code,
			    quoter.quoteUntil(location(), commandStr) );
		    quoter.reset();
		    break;
		case COMMAND_QUOTEFROMFILE:
		    leaveParagraph();
		    quoteFromFile( command );
		    break;
		case COMMAND_QUOTEFUNCTION:
		    leaveParagraph();
		    quoteFromFile( command );
		    quoter.quoteTo( location(), commandStr, getRestOfLine() );
		    append( Atom::Code,
			    quoter.quoteUntil(location(), commandStr,
					      "/^\\}/") );
		    quoter.reset();
		    break;
		case COMMAND_QUOTELINE:
		    leaveParagraph();
		    appendToCode( quoter.quoteLine(location(), commandStr,
						   getRestOfLine()) );
		    break;
		case COMMAND_QUOTETO:
		    leaveParagraph();
		    appendToCode( quoter.quoteTo(location(), commandStr,
				  getRestOfLine()) );
		    break;
		case COMMAND_QUOTEUNTIL:
		    leaveParagraph();
		    appendToCode( quoter.quoteUntil(location(), commandStr,
						    getRestOfLine()) );
		    break;
		case COMMAND_RAW:
		    leaveParagraph();
		    begin = pos;
		    x = getUntilRight( command );
		    x = untabifyEtc( in.mid(begin, end - begin) );
		    append( Atom::RawFormat, "html" ); // ###
		    append( Atom::RawString, x );
		    break;
		case COMMAND_ROW:
		    /* ... */
		    break;
		case COMMAND_SECTION1:
		    startSection( Doc::Section1, command );
		    break;
		case COMMAND_SECTION2:
		    startSection( Doc::Section2, command );
		    break;
		case COMMAND_SECTION3:
		    startSection( Doc::Section3, command );
		    break;
		case COMMAND_SECTION4:
		    startSection( Doc::Section4, command );
		    break;
		case COMMAND_SIDEBAR:
		    if ( openCommand(command) ) {
			leaveParagraph();
			append( Atom::SidebarLeft );
		    }
		    break;
		case COMMAND_SKIPLINE:
		    leaveParagraph();
		    quoter.quoteLine( location(), commandStr, getRestOfLine() );
		    break;
		case COMMAND_SKIPTO:
		    leaveParagraph();
		    quoter.quoteTo( location(), commandStr, getRestOfLine() );
		    break;
		case COMMAND_SKIPUNTIL:
		    leaveParagraph();
		    quoter.quoteUntil( location(), commandStr, getRestOfLine() );
		    break;
		case COMMAND_SUB:
		    startFormat( ATOM_FORMAT_SUBSCRIPT, command );
		    break;
		case COMMAND_SUP:
		    startFormat( ATOM_FORMAT_SUPERSCRIPT, command );
		    break;
		case COMMAND_TABLE:
		    if ( openCommand(command) ) {
			leaveParagraph();
			append( Atom::TableLeft );
		    }
		    break;
		case COMMAND_TABLEOFCONTENTS:
		    append( Atom::TableOfContents,
			    QString::number((int) getSectioningUnit()) );
		    /* ... */
		    break;
		case COMMAND_TARGET:
		    insertTarget( getArgument() );
		    break;
		case COMMAND_TT:
		    startFormat( ATOM_FORMAT_TELETYPE, command );
		    break;
		case COMMAND_UNDERLINE:
		    startFormat( ATOM_FORMAT_UNDERLINE, command );
		    break;
		case COMMAND_VALUE:
		    /* ... */
		    break;
		case COMMAND_WARNING:
		    startNewParagraph();
		    append( Atom::FormatLeft, ATOM_FORMAT_BOLD );
		    append( Atom::String, "Warning: " );
		    append( Atom::FormatRight, ATOM_FORMAT_BOLD );
		    break;
		case UNKNOWN_COMMAND:
		    if ( metaCommandSet.contains(commandStr) ) {
			if ( priv->metaCommandSet == 0 ) {
			    priv->metaCommandSet = new Set<QString>;
			    priv->metaCommandMap =
				    new QMap<QString, QStringList>;
			}
			priv->metaCommandSet->insert( commandStr );
			(*priv->metaCommandMap)[commandStr].append(
				getRestOfLine() );
		    } else {
			Messages::warning( location(),
					   Qdoc::tr("Unknown command '\\%1'")
					   .arg(commandStr),
					   detailsUnknownCommand(metaCommandSet,
								 commandStr) );
			enterParagraph();
			append( Atom::UnknownCommand, commandStr );
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
		append( Atom::FormatRight, *f );
		if ( *f == ATOM_FORMAT_INDEX && indexStartedPara )
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

    if ( openedCommands.top() != COMMAND_OMIT )
	Messages::warning( location(),
			   Qdoc::tr("Missing '\\%1'")
			   .arg(endCommandName(openedCommands.top())) );
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

QString DocParser::detailsUnknownCommand( const Set<QString>& metaCommandSet,
					  const QString& str )
{
    Set<QString> commandSet = metaCommandSet;
    int i = 0;
    while ( cmds[i].english != 0 ) {
	if ( cmds[i].english == str )
	    return Qdoc::tr( "The command '\\%1' was renamed '\\%2' by the"
			     " configuration file. Use the new name." )
		   .arg( str ).arg( cmds[i].translation );
	commandSet.insert( cmds[i].translation );
	i++;
    }

    int deltaBest = 666;
    int numBest;
    QString best;

    Set<QString>::ConstIterator c = commandSet.begin();
    while ( c != commandSet.end() ) {
	int delta = editDistance( *c, str );
	if ( delta < deltaBest ) {
	    deltaBest = delta;
	    numBest = 1;
	    best = *c;
	} else if ( delta == deltaBest ) {
	    numBest++;
	}
	++c;
    }
    if ( numBest == 1 && deltaBest <= 2 && str.length() >= 3 ) {
	return Qdoc::tr( "Maybe you meant '\\%1'?" ).arg( best );
    } else {
	return Qdoc::tr( "###" );
    }
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
	if ( expiryDate.isValid() ) {
	    int days = expiryDate.daysTo( QDate::currentDate() );
	    if ( days == 0 ) {
		Messages::warning( location(),
				   Qdoc::tr("Documentation expires today") );
	    } else if ( days == 1 ) {
		Messages::warning( location(),
				   Qdoc::tr("Documentation expired"
					    " yesterday") );
	    } else if ( days >= 2 ) {
		Messages::warning( location(),
				   Qdoc::tr("Documentation expired %1 days ago")
				   .arg(days) );
	    }
	} else {
	    Messages::warning( location(),
			       Qdoc::tr("Date '%1' invalid").arg(date) );
	}
    } else {
	Messages::warning( location(),
			   Qdoc::tr("Date '%1' not in YYYY-MM-DD format")
			   .arg(date) );
    }
}

void DocParser::insertBaseName( const QString& baseName )
{
    priv->constructExtra();
    if ( currentSectioningUnit == priv->extra->sectioningUnit ) {
	priv->extra->baseName = baseName;
    } else {
	Atom *atom = priv->text.firstAtom();
	Atom *sectionLeft = 0;

	int delta = currentSectioningUnit - priv->extra->sectioningUnit;

	while ( atom != 0 ) {
	    if ( atom->type() == Atom::SectionLeft &&
		 atom->string().toInt() == delta )
		sectionLeft = atom;
	    atom = atom->next();
	}
	if ( sectionLeft != 0 )
	    (void) new Atom( sectionLeft, Atom::BaseName, baseName );
    }
}

void DocParser::insertTarget( const QString& target )
{
    if ( targetMap.contains(target) ) {
	Messages::warning( location(),
			   Qdoc::tr("Duplicate target name '%1'").arg(target) );
	Messages::warning( targetMap[target],
			   Qdoc::tr("(The previous occurrence is here)") );
    } else {
	targetMap.insert( target, location() );
	append( Atom::Target, target );
    }
}

void DocParser::include( const QString& /* fileName */ )
{
#if notyet // ###
    QFile inFile( "src/" + fileName );
    if ( !inFile.open(IO_ReadOnly) ) {
	Messages::error( location(),
			 Qdoc::tr("Cannot open leaf file '%1'").arg(fileName) );
    } else {
	QString s = inFile.readAll();
	s.prepend( "\\location {" + Location(fileName).toString() + "}" );
	s.append( "\\location {" + location().toString() + "}" );

	in.insert( pos, s );
	len = in.length();
    }
#endif
}

void DocParser::startFormat( const QString& format, int command )
{
    enterParagraph();

    QMap<int, QString>::ConstIterator f = pendingFormats.begin();
    while ( f != pendingFormats.end() ) {
	if ( *f == format ) {
	    Messages::warning( location(),
			       Qdoc::tr("Cannot nest '\\%1' commands")
			       .arg(commandName(command)) );
	    return;
	}
	++f;
    }

    append( Atom::FormatLeft, format );

    if ( isLeftBraceAhead() ) {
	skipSpacesOrOneEndl();
	pendingFormats.insert( braceDepth, format );
	braceDepth++;
	pos++;
    } else {
	append( Atom::String, getArgument() );
	append( Atom::FormatRight, format );
	if ( format == ATOM_FORMAT_INDEX && indexStartedPara ) {
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

bool DocParser::openCommand( int command )
{
    int top = openedCommands.top();
    bool ok = TRUE;

    if ( top != COMMAND_OMIT && top != COMMAND_LIST ) {
	QValueList<int> ordering;
	ordering << COMMAND_ABSTRACT << COMMAND_SIDEBAR << COMMAND_QUOTATION
		 << COMMAND_TABLE << COMMAND_FOOTNOTE;
	ok = ordering.findIndex( top ) < ordering.findIndex( command );
    }

    if ( ok ) {
	openedCommands.push( command );
    } else {
	Messages::warning( location(),
			   Qdoc::tr("Cannot use '\\%1' within '\\%2'")
			   .arg(commandName(command)).arg(commandName(top)) );
    }
    return ok;
}

bool DocParser::closeCommand( int endCommand )
{
    if ( endCommandOf(openedCommands.top()) == endCommand ) {
	openedCommands.pop();
	return TRUE;
    } else {
	bool contains = FALSE;
	QValueStack<int> opened2 = openedCommands;
	while ( !opened2.isEmpty() ) {
	    if ( endCommandOf(opened2.top()) == endCommand ) {
		contains = TRUE;
		break;
	    }
	    opened2.pop();
	}

	if ( contains ) {
	    while ( endCommandOf(openedCommands.top()) != endCommand ) {
		Messages::warning( location(),
				   Qdoc::tr("Missing '\\%1' before '\\%2'")
				   .arg(endCommandName(openedCommands.top()))
				   .arg(commandName(endCommand)) );
		openedCommands.pop();
	    }
	} else {
	    Messages::warning( location(),
			       Qdoc::tr("Missing '\\%1'")
			       .arg(commandName(endCommand)) );
	}
	return FALSE;
    }
}

void DocParser::startSection( Doc::SectioningUnit unit, int command )
{
    leaveParagraph();

    if ( currentSectioningUnit == Doc::Book ) {
	if ( unit > Doc::Section1 )
	    Messages::warning( location(),
			       Qdoc::tr("Unexpected '\\%1' without '\\%2'")
			       .arg(commandName(command))
			       .arg(commandName(COMMAND_SECTION1)) );
	currentSectioningUnit = (Doc::SectioningUnit) ( unit - 1 );
	priv->constructExtra();
	priv->extra->sectioningUnit = currentSectioningUnit;
    }

    if ( unit <= priv->extra->sectioningUnit ) {
	Messages::warning( location(),
			   Qdoc::tr("Unexpected '\\%1' in this documentation")
			   .arg(commandName(command)) );
    } else if ( unit - currentSectioningUnit > 1 ) {
	Messages::warning( location(),
			   Qdoc::tr("Unexpected '\\%1' at this point")
			   .arg(commandName(command)) );
    } else {
	if ( currentSectioningUnit >= unit )
	    endSection( unit, command );

	int delta = unit - priv->extra->sectioningUnit;
	append( Atom::SectionLeft, QString::number(delta) );
	startHeading( Atom::SectionHeadingLeft, Atom::SectionHeadingRight,
		      QString::number(delta) );
	currentSectioningUnit = unit;
    }
}

void DocParser::endSection( int unit, int endCommand )
{
    leaveParagraph();

    if ( unit < priv->extra->sectioningUnit ) {
	Messages::warning( location(),
			   Qdoc::tr("Unexpected '\\%1' in this documentation")
			   .arg(commandName(endCommand)) );
    } else if ( unit > currentSectioningUnit ) {
	Messages::warning( location(),
			   Qdoc::tr("Unexpected '\\%1' at this point")
			   .arg(commandName(endCommand)) );
    } else {
	while ( currentSectioningUnit >= unit ) {
	    int delta = currentSectioningUnit - priv->extra->sectioningUnit;
	    append( Atom::SectionRight, QString::number(delta) );
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
	QString str;

	if ( in[pos] == '{' ) {
	    target = getArgument();
	    skipSpacesOnLine();
	    if ( in[pos] == '{' ) {
		str = getArgument();
	    } else {
		str = target;
	    }
	} else {
	    target = getArgument();
	    str = target;
	}

	Text also;
	also << Atom( Atom::Link, target )
	     << Atom( Atom::FormatLeft, ATOM_FORMAT_LINK ) << str
	     << Atom( Atom::FormatRight, ATOM_FORMAT_LINK );
	priv->addAlso( also );

	skipSpacesOnLine();
	if ( pos < len && in[pos] == ',' ) {
	    pos++;
	    skipSpacesOrOneEndl();
	} else if ( in[pos] != '\n' ) {
	    Messages::warning( location(),
			       Qdoc::tr("Missing comma in '\\%1'")
			       .arg(commandName(COMMAND_ALSO)) );
	}
    }
}

void DocParser::append( Atom::Type type, const QString& string )
{
    if ( priv->text.lastAtom()->type() == Atom::Code &&
	 priv->text.lastAtom()->string().endsWith("\n\n") )
	priv->text.lastAtom()->chopString();
    priv->text << Atom( type, string );
}

void DocParser::appendChar( QChar ch )
{
    if ( priv->text.lastAtom()->type() != Atom::String )
	append( Atom::String );
    if ( ch.isSpace() ) {
	if ( !priv->text.lastAtom()->string().endsWith(" ") )
	    priv->text.lastAtom()->appendChar( ' ' );
    } else {
	priv->text.lastAtom()->appendChar( ch );
    }
}

void DocParser::appendToCode( const QString& markedCode )
{
    if ( priv->text.lastAtom()->type() != Atom::Code )
	append( Atom::Code );
    priv->text.lastAtom()->appendString( markedCode );
}

void DocParser::startNewParagraph()
{
    leaveParagraph();
    enterParagraph();
}

void DocParser::enterParagraph()
{
    if ( !inPara ) {
	append( Atom::ParagraphLeft );
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
		Messages::warning( location(), Qdoc::tr("Missing '}'") );
		pendingFormats.clear();
	    }

	    if ( priv->text.lastAtom()->type() == Atom::String &&
		 priv->text.lastAtom()->string().endsWith(" ") )
		priv->text.lastAtom()->chopString();
	    append( Atom::ParagraphRight );
	    inPara = FALSE;
	    indexStartedPara = FALSE;
	}
    }
}

void DocParser::quoteFromFile( int /* command */ )
{
    quoter.reset();

    QString code;
    QString fileName = getArgument();

    QString filePath = fileName; /* ### */
    QFile inFile( filePath );
    if ( !inFile.open(IO_ReadOnly) ) {
	Messages::warning( location(),
			   Qdoc::tr("Cannot open example file '%1'")
			   .arg(filePath) );
    } else {
	QTextStream inStream( &inFile );
	code = untabifyEtc( inStream.read() );
	inFile.close();
    }

    QString dirPath = QFileInfo( filePath ).dirPath();
    CodeMarker *marker = CodeMarker::markerForFileName( fileName );
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
	Messages::warning( location(),
			   Qdoc::tr("Invalid sectioning unit '%1'")
			   .arg(name) );
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
	    Messages::warning( location(), Qdoc::tr("Missing '}'") );
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

QString DocParser::getUntilRight( int endCommand )
{
    QRegExp rx( "\\\\" + commandName(endCommand) + "\\b" );
    QString t;
    int end = rx.search( in, pos );

    if ( end == -1 ) {
	Messages::warning( location(),
			   Qdoc::tr("Missing '\\%1'")
			   .arg(commandName(endCommand)) );
	pos = (int) in.length();
    } else {
	t = in.mid( pos, end - pos );
	pos = end + rx.matchedLength();
    }
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

Doc::Doc( const Location& location, const QString& str,
	  const Set<QString>& metaCommandSet )
{
    priv = new DocPrivate( location );
    DocParser parser;
    parser.parse( str, priv, metaCommandSet );
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

const Location& Doc::location() const
{
    return priv->loc;
}

bool Doc::isEmpty() const
{
    return body().isEmpty();
}

const Text& Doc::body() const
{
    return priv->text;
}

const QString& Doc::baseName() const
{
    if ( priv->extra == 0 ) {
	return QString::null;
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

#if notyet // ###
Doc::SectioningUnit Doc::sectioningUnit() const
{
    if ( priv->extra == 0 ) {
	return DocPrivateExtra().sectioningUnit;
    } else {
	return priv->extra->sectioningUnit;
    }
}
#endif

const Set<QString> *Doc::metaCommandsUsed() const
{
    return priv->metaCommandSet;
}

QStringList Doc::metaCommandArgs( const QString& metaCommand ) const
{
    if ( priv->metaCommandMap == 0 ) {
	return QStringList();
    } else {
	return (*priv->metaCommandMap)[metaCommand];
    }
}

const QValueList<Text> *Doc::alsoList() const
{
    return priv->alsoList;
}

#if notyet // ###
Doc Doc::propertyFunctionDoc( const Doc& propertyDoc, const QString& role,
			      const QString& param )
{
    Doc doc;
    doc.priv->loc = propertyDoc.location();

    Text brief = propertyDoc.body().subText( Atom::BriefLeft, Atom::BriefRight );
    doc.priv->text << Atom::ParagraphLeft;
    if ( !brief.isEmpty() ) {
	bool whether = ( brief.firstAtom()->type() == Atom::String &&
			 brief.firstAtom()->string().lower()
			      .startsWith("whether") );

	if ( role == "getter" ) {
	    if ( whether ) {
		doc.priv->text << "Returns TRUE if"
			       << brief.firstAtom()->string().mid( 7 )
			       << brief.subText( brief.firstAtom()->next() )
			       << "; otherwise returns FALSE.";
	    } else {
		doc.priv->text << "Returns " << brief << ".";
	    }
	} else if ( role == "setter" ) {
	    if ( param.isEmpty() ) {
		doc.priv->text << "Sets " << brief << ".";
	    } else {
		doc.priv->text << "Sets " << brief << " to "
			       << Atom( Atom::FormatLeft, "parameter" )
			       << param
			       << Atom( Atom::FormatRight, "parameter" )
			       << ".";
	    }
	} else if ( role == "resetter" ) {
	    doc.priv->text << "Resets " << brief << ".";
	}
    }
    doc.priv->text << Atom::ParagraphRight;
    // set priv->params
    return doc;
}
#endif

void Doc::initialize( const Config& config )
{
    delete commandDict;
    commandDict = new QDict<int>( 251 );

    int i = 0;
    while ( cmds[i].english != 0 ) {
	QString translation = config.getString(
		Config::dot(CONFIG_ALIAS, cmds[i].english) );
	if ( translation.isEmpty() )
	    translation = cmds[i].english;
	cmds[i].translation = translation;
	int *entry = commandDict->find( translation );
	if ( entry != 0 )
	    Messages::warning( config.location(),
			       Qdoc::tr("Command name '\\%1' cannot stand"
					" for both '\\%2' and '\\%3'")
			       .arg(translation)
			       .arg(cmds[*entry].english)
			       .arg(cmds[i].english) );
	commandDict->replace( translation, &cmds[i].no );

	if ( cmds[i].no != i )
	    Messages::internalError( Qdoc::tr("command %1 missing").arg(i) );
	i++;
    }

    delete macroDict;
    macroDict = new QDict<Macro>( 251 );
    macroDict->setAutoDelete( TRUE );

    Set<QString> macroNames = config.subVars( CONFIG_MACRO );
    Set<QString>::ConstIterator n = macroNames.begin();
    while ( n != macroNames.end() ) {
	QString macroDotName = Config::dot( CONFIG_MACRO, *n );
	Macro *macro = new Macro;
	macro->numParams = -1;
	macro->defaultDef = config.getString( macroDotName );
	if ( !macro->defaultDef.isEmpty() )
	    macro->numParams = numParams( macro->defaultDef );
	bool silent = FALSE;

	Set<QString> formats = config.subVars( macroDotName );
	Set<QString>::ConstIterator f = formats.begin();

	while ( f != formats.end() ) {
	    QString def = config.getString( Config::dot(macroDotName, *f) );
	    if ( !def.isEmpty() ) {
		macro->otherDefs.insert( *f, def );
		int m = numParams( macro->defaultDef );
		if ( macro->numParams == -1 ) {
		    macro->numParams = m;
		} else if ( macro->numParams != m ) {
		    if ( !silent ) {
			QString other = Qdoc::tr( "default" );
			if ( macro->defaultDef.isEmpty() )
			    other = macro->otherDefs.begin().key();
			Messages::warning( config.location(),
					   Qdoc::tr("Macro '\\%1' takes"
						    " inconsistent number of"
						    " arguments (%2 %3, %4 %5)")
					   .arg(*n)
					   .arg(*f)
					   .arg(m)
					   .arg(other)
					   .arg(macro->numParams) );
			silent = TRUE;
		    }
		    if ( macro->numParams < m )
			macro->numParams = m;
		}
	    }
	    ++f;
	}

	if ( macro->numParams == -1 ) {
	    delete macro;
	} else {
	    macroDict->insert( *n, macro );
	}
	++n;
    }
}

void Doc::terminate()
{
    delete commandDict;
    commandDict = 0;
    delete macroDict;
    macroDict = 0;
}
