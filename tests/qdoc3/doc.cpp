/*
  doc.cpp
*/

#include <qdatetime.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qhash.h>
#include <qregexp.h>

#include "codemarker.h"
#include "config.h"
#include "doc.h"
#include "editdistance.h"
#include "openedlist.h"
#include "quoter.h"
#include "text.h"
#include "tokenizer.h"

static Set<QString> null_Set_QString;
static QList<Text> null_QList_Text;

struct Macro
{
    QString defaultDef;
    Location defaultDefLocation;
    QMap<QString, QString> otherDefs;
    int numParams;
};

enum {
    CMD_A, CMD_ABSTRACT, CMD_ALSO, CMD_BASENAME, CMD_BOLD, CMD_BRIEF, CMD_C, CMD_CAPTION,
    CMD_CHAPTER, CMD_CODE, CMD_ELSE, CMD_ENDABSTRACT, CMD_ENDCHAPTER, CMD_ENDCODE, CMD_ENDFOOTNOTE,
    CMD_ENDIF, CMD_ENDLIST, CMD_ENDOMIT, CMD_ENDPART, CMD_ENDQUOTATION, CMD_ENDSECTION1,
    CMD_ENDSECTION2, CMD_ENDSECTION3, CMD_ENDSECTION4, CMD_ENDSIDEBAR, CMD_ENDTABLE, CMD_EXPIRE,
    CMD_FOOTNOTE, CMD_GENERATELIST, CMD_GRANULARITY, CMD_HEADER, CMD_I, CMD_IF, CMD_IMAGE,
    CMD_INCLUDE, CMD_INDEX, CMD_KEYWORD, CMD_L, CMD_LEGALESE, CMD_LIST, CMD_O, CMD_OMIT,
    CMD_OMITVALUE, CMD_PART, CMD_PRINTLINE, CMD_PRINTTO, CMD_PRINTUNTIL, CMD_QUOTATION,
    CMD_QUOTEFILE, CMD_QUOTEFROMFILE, CMD_QUOTEFUNCTION, CMD_RAW, CMD_ROW, CMD_SECTION1,
    CMD_SECTION2, CMD_SECTION3, CMD_SECTION4, CMD_SIDEBAR, CMD_SKIPLINE, CMD_SKIPTO, CMD_SKIPUNTIL,
    CMD_SUB, CMD_SUP, CMD_TABLE, CMD_TABLEOFCONTENTS, CMD_TARGET, CMD_TT, CMD_UNDERLINE, CMD_VALUE,
    CMD_WARNING, UNKNOWN_COMMAND
};

static struct {
    const char *english;
    int no;
    QString alias;
} cmds[] = {
    { "a", CMD_A, 0 },
    { "abstract", CMD_ABSTRACT, 0 },
    { "also", CMD_ALSO, 0 },
    { "basename", CMD_BASENAME, 0 },
    { "bold", CMD_BOLD, 0 },
    { "brief", CMD_BRIEF, 0 },
    { "c", CMD_C, 0 },
    { "caption", CMD_CAPTION, 0 },
    { "chapter", CMD_CHAPTER, 0 },
    { "code", CMD_CODE, 0 },
    { "else", CMD_ELSE, 0 },
    { "endabstract", CMD_ENDABSTRACT, 0 },
    { "endchapter", CMD_ENDCHAPTER, 0 },
    { "endcode", CMD_ENDCODE, 0 },
    { "endfootnote", CMD_ENDFOOTNOTE, 0 },
    { "endif", CMD_ENDIF, 0 },
    { "endlist", CMD_ENDLIST, 0 },
    { "endomit", CMD_ENDOMIT, 0 },
    { "endpart", CMD_ENDPART, 0 },
    { "endquotation", CMD_ENDQUOTATION, 0 },
    { "endsection1", CMD_ENDSECTION1, 0 },
    { "endsection2", CMD_ENDSECTION2, 0 },
    { "endsection3", CMD_ENDSECTION3, 0 },
    { "endsection4", CMD_ENDSECTION4, 0 },
    { "endsidebar", CMD_ENDSIDEBAR, 0 },
    { "endtable", CMD_ENDTABLE, 0 },
    { "expire", CMD_EXPIRE, 0 },
    { "footnote", CMD_FOOTNOTE, 0 },
    { "generatelist", CMD_GENERATELIST, 0 },
    { "granularity", CMD_GRANULARITY, 0 },
    { "header", CMD_HEADER, 0 },
    { "i", CMD_I, 0 },
    { "if", CMD_IF, 0 },
    { "image", CMD_IMAGE, 0 },
    { "include", CMD_INCLUDE, 0 },
    { "index", CMD_INDEX, 0 },
    { "keyword", CMD_KEYWORD, 0 },
    { "l", CMD_L, 0 },
    { "legalese", CMD_LEGALESE, 0 },
    { "list", CMD_LIST, 0 },
    { "o", CMD_O, 0 },
    { "omit", CMD_OMIT, 0 },
    { "omitvalue", CMD_OMITVALUE, 0 },
    { "part", CMD_PART, 0 },
    { "printline", CMD_PRINTLINE, 0 },
    { "printto", CMD_PRINTTO, 0 },
    { "printuntil", CMD_PRINTUNTIL, 0 },
    { "quotation", CMD_QUOTATION, 0 },
    { "quotefile", CMD_QUOTEFILE, 0 },
    { "quotefromfile", CMD_QUOTEFROMFILE, 0 },
    { "quotefunction", CMD_QUOTEFUNCTION, 0 },
    { "raw", CMD_RAW, 0 },
    { "row", CMD_ROW, 0 },
    { "section1", CMD_SECTION1, 0 },
    { "section2", CMD_SECTION2, 0 },
    { "section3", CMD_SECTION3, 0 },
    { "section4", CMD_SECTION4, 0 },
    { "sidebar", CMD_SIDEBAR, 0 },
    { "skipline", CMD_SKIPLINE, 0 },
    { "skipto", CMD_SKIPTO, 0 },
    { "skipuntil", CMD_SKIPUNTIL, 0 },
    { "sub", CMD_SUB, 0 },
    { "sup", CMD_SUP, 0 },
    { "table", CMD_TABLE, 0 },
    { "tableofcontents", CMD_TABLEOFCONTENTS, 0 },
    { "target", CMD_TARGET, 0 },
    { "tt", CMD_TT, 0 },
    { "underline", CMD_UNDERLINE, 0 },
    { "value", CMD_VALUE, 0 },
    { "warning", CMD_WARNING, 0 },
    { 0, 0, 0 }
};

static QMap<QString, QString> aliasMap;
static QHash<QString, int> commandHash;
static QHash<QString, Macro> macroHash;

class DocPrivateExtra
{
public:
    QString baseName;
    Doc::SectioningUnit granularity;
    Doc::SectioningUnit sectioningUnit; // ###

    DocPrivateExtra()
	: granularity( Doc::Part ) { }
};

struct Shared
{
    Shared()
	: count(1) { }
    void ref() { ++count; }
    bool deref() { return (--count == 0); }

    int count;
};

class DocPrivate : public Shared
{
public:
    DocPrivate( const Location& location = Location::null,
		const QString& source = "" );
    ~DocPrivate();

    void addAlso( const Text& also );
    void constructExtra();

    // ### move some of this in DocPrivateExtra
    Location loc;
    QString src;
    Text text;
    Set<QString> params;
    QList<Text> alsoList;
    Set<QString> enumItemSet;
    Set<QString> omitEnumItemSet;
    Set<QString> metaCommandSet;
    QMap<QString, QStringList> metaCommandMap;
    bool hasLegalese : 8;
    DocPrivateExtra *extra;
};

DocPrivate::DocPrivate(const Location &location, const QString &source)
    : loc(location), src(source), hasLegalese(false), extra(0)
{
    null_Set_QString.ensure_constructed();
    null_QList_Text.ensure_constructed();
}

DocPrivate::~DocPrivate()
{
    delete extra;
}

void DocPrivate::addAlso( const Text& also )
{
    alsoList.append( also );
}

void DocPrivate::constructExtra()
{
    if ( extra == 0 )
	extra = new DocPrivateExtra;
}

class DocParser
{
public:
    void parse( const QString& source, DocPrivate *docPrivate,
		const Set<QString>& metaCommandSet );

    static int tabSize;
    static QStringList exampleFiles;
    static QStringList exampleDirs;
    static QStringList sourceFiles;
    static QStringList sourceDirs;

private:
    Location& location();
    QString detailsUnknownCommand( const Set<QString>& metaCommandSet,
				   const QString& str );
    void checkExpiry( const QString& date );
    void insertBaseName(const QString &baseName);
    void insertTarget( const QString& target );
    void include( const QString& fileName );
    void startFormat( const QString& format, int command );
    bool openCommand( int command );
    bool closeCommand( int endCommand );
    void startSection( Doc::SectioningUnit unit, int command );
    void endSection( int unit, int endCommand );
    void parseAlso();
    void append( Atom::Type type, const QString& string = "" );
    void appendChar( QChar ch );
    void appendToCode( const QString& code );
    void startNewPara();
    void enterPara( Atom::Type leftType = Atom::ParaLeft,
		    Atom::Type rightType = Atom::ParaRight,
		    const QString& string = "" );
    void leavePara();
    void leaveValue();
    void leaveValueList();
    CodeMarker *quoteFromFile( int command );
    void expandMacro( const QString& name, const QString& def, int numParams );
    Doc::SectioningUnit getSectioningUnit();
    QString getArgument( bool verbatim = false );
    QString getOptionalArgument();
    QString getRestOfLine();
    QString getUntilEnd(int command);

    bool isBlankLine();
    bool isLeftBraceAhead();
    void skipSpacesOnLine();
    void skipSpacesOrOneEndl();
    void skipAllSpaces();
    void skipToNextPreprocessorCommand();

    static int endCommandFor( int command );
    static QString commandName( int command );
    static QString endCommandName( int command );
    static QString untabifyEtc( const QString& str );
    static int indentLevel( const QString& str );
    static QString unindent( int level, const QString& str );
    static QString slashed( const QString& str );

    QStack<int> openedInputs;

    QString in;
    int pos;
    int len;
    Location cachedLoc;
    int cachedPos;

    DocPrivate *priv;
    enum ParaState { OutsidePara, InsideSingleLinePara, InsideMultiLinePara };
    ParaState paraState;
    bool indexStartedPara; // ### rename
    Atom::Type pendingParaLeftType;
    Atom::Type pendingParaRightType;
    QString pendingParaString;

    int braceDepth;
    int minIndent;
    Doc::SectioningUnit currentSectioningUnit;
    QMap<QString, Location> targetMap;
    QMap<int, QString> pendingFormats;
    QStack<int> openedCommands;
    QStack<OpenedList> openedLists;
    Quoter quoter;
};

int DocParser::tabSize;
QStringList DocParser::exampleFiles;
QStringList DocParser::exampleDirs;
QStringList DocParser::sourceFiles;
QStringList DocParser::sourceDirs;

void DocParser::parse( const QString& source, DocPrivate *docPrivate,
		       const Set<QString>& metaCommandSet )
{
    in = source;
    pos = 0;
    len = in.length();
    cachedLoc = docPrivate->loc;
    cachedPos = 0;
    priv = docPrivate;
    priv->text << Atom::Nop;

    paraState = OutsidePara;
    indexStartedPara = false;
    pendingParaLeftType = Atom::Nop;
    pendingParaRightType = Atom::Nop;

    braceDepth = 0;
    minIndent = INT_MAX;
    currentSectioningUnit = Doc::Book;
    openedCommands.push( CMD_OMIT );
    quoter.reset();

    CodeMarker *marker;
    QString link;
    QString x;
    int begin;
    int indent;
    QStack<bool> preprocessorSkipping;
    int numPreprocessorSkipping = 0;
    bool inLegalese = false;

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
		    enterPara();
		    if ( in[pos].isSpace() ) {
			skipAllSpaces();
			appendChar( ' ' );
		    } else {
			appendChar( in[pos++] );
		    }
		}
	    } else {
		int command = commandHash.value(commandStr, UNKNOWN_COMMAND);

		switch ( command ) {
		case CMD_A:
		    enterPara();
		    x = getArgument();
		    append( Atom::FormattingLeft, ATOM_FORMATTING_PARAMETER );
		    append( Atom::String, x );
		    append( Atom::FormattingRight, ATOM_FORMATTING_PARAMETER );
		    priv->params.insert( x );
		    break;
		case CMD_ABSTRACT:
		    if ( openCommand(command) ) {
			leavePara();
			append( Atom::AbstractLeft );
		    }
		    break;
		case CMD_ALSO:
		    parseAlso();
		    break;
		case CMD_BASENAME:
		    leavePara();
		    insertBaseName(getArgument());
		    break;
		case CMD_BOLD:
		    startFormat( ATOM_FORMATTING_BOLD, command );
		    break;
		case CMD_BRIEF:
		    leavePara();
		    enterPara( Atom::BriefLeft, Atom::BriefRight );
		    break;
		case CMD_C:
		    enterPara();
		    x = untabifyEtc( getArgument(true) );
		    marker = CodeMarker::markerForCode( x );
		    append( Atom::C, marker->markedUpCode(x, 0, "") );
		    break;
		case CMD_CAPTION:
		    leavePara();
		    /* ... */
		    break;
		case CMD_CHAPTER:
		    startSection( Doc::Chapter, command );
		    break;
		case CMD_CODE:
		    leavePara();
		    begin = pos;
		    x = getUntilEnd( command );
		    x = untabifyEtc( x );
		    indent = indentLevel( x );
		    if ( indent < minIndent )
			minIndent = indent;
		    x = unindent( minIndent, x );
		    marker = CodeMarker::markerForCode( x );
		    append( Atom::Code, marker->markedUpCode(x, 0, "") );
		    break;
		case CMD_ELSE:
		    if (preprocessorSkipping.size() > 0) {
			if (preprocessorSkipping.top()) {
			    --numPreprocessorSkipping;
                        } else {
			    ++numPreprocessorSkipping;
                        }
                        preprocessorSkipping.top() = !preprocessorSkipping.top();
                        (void)getRestOfLine(); // ### should ensure that it's empty
                        if (numPreprocessorSkipping)
			    skipToNextPreprocessorCommand();
		    } else {
			location().warning(tr("Unexpected '\\%1'").arg(commandName(CMD_ELSE)));
                    }
                    break;
		case CMD_ENDABSTRACT:
		    if ( closeCommand(command) ) {
			leavePara();
			append( Atom::AbstractRight );
		    }
		    break;
		case CMD_ENDCHAPTER:
		    endSection( 0, command );
		    break;
		case CMD_ENDCODE:
		    closeCommand( command );
		    break;
		case CMD_ENDFOOTNOTE:
		    if ( closeCommand(command) ) {
			leavePara();
			append( Atom::FootnoteRight );
			paraState = InsideMultiLinePara; // ###
		    }
		    break;
		case CMD_ENDIF:
		    if (preprocessorSkipping.count() > 0) {
			if (preprocessorSkipping.pop())
			    --numPreprocessorSkipping;
                        (void)getRestOfLine(); // ### should ensure that it's empty
			if (numPreprocessorSkipping)
			    skipToNextPreprocessorCommand();
		    } else {
			location().warning(tr("Unexpected '\\%1'").arg(commandName(CMD_ENDIF)));
                    }
		    break;
		case CMD_ENDLIST:
		    if ( closeCommand(command) ) {
			leavePara();
			if ( openedLists.top().isStarted() ) {
			    append( Atom::ListItemRight,
				    openedLists.top().styleString() );
			    append( Atom::ListRight,
				    openedLists.top().styleString() );
			}
			openedLists.pop();
		    }
		    break;
		case CMD_ENDOMIT:
		    closeCommand( command );
		    break;
		case CMD_ENDPART:
		    endSection( -1, command );
		    break;
		case CMD_ENDQUOTATION:
		    if ( closeCommand(command) ) {
			leavePara();
			append( Atom::QuotationRight );
		    }
		    break;
		case CMD_ENDSECTION1:
		    endSection( 1, command );
		    break;
		case CMD_ENDSECTION2:
		    endSection( 2, command );
		    break;
		case CMD_ENDSECTION3:
		    endSection( 3, command );
		    break;
		case CMD_ENDSECTION4:
		    endSection( 4, command );
		    break;
		case CMD_ENDSIDEBAR:
		    if ( closeCommand(command) ) {
			leavePara();
			append( Atom::SidebarRight );
		    }
		    break;
		case CMD_ENDTABLE:
		    if ( closeCommand(command) ) {
			append( Atom::TableRight );
			/* ... */
		    }
		    break;
		case CMD_EXPIRE:
		    checkExpiry( getArgument() );
		    break;
		case CMD_FOOTNOTE:
		    if ( openCommand(command) ) {
			enterPara();
			append( Atom::FootnoteLeft );
			paraState = OutsidePara; // ###
		    }
		    break;
		case CMD_GENERATELIST:
		    append(Atom::GeneratedList, getArgument());
		    break;
		case CMD_GRANULARITY:
		    priv->constructExtra();
		    priv->extra->granularity = getSectioningUnit();
		    break;
		case CMD_HEADER:
		    /* ... */
		    break;
		case CMD_I:
		    startFormat( ATOM_FORMATTING_ITALIC, command );
		    break;
		case CMD_IF:
                    preprocessorSkipping.push(!Tokenizer::isTrue(getRestOfLine()));
                    if (preprocessorSkipping.top())
			++numPreprocessorSkipping;
                    if (numPreprocessorSkipping)
			skipToNextPreprocessorCommand();
		    break;
		case CMD_IMAGE:
		    append( Atom::Image, getArgument() );
		    append( Atom::ImageText, getRestOfLine() );
		    break;
		case CMD_INCLUDE:
		    include( getArgument() );
		    break;
		case CMD_INDEX:
		    if ( paraState == OutsidePara ) {
			enterPara();
			indexStartedPara = true;
		    } else {
			const Atom *last = priv->text.lastAtom();
			if ( indexStartedPara &&
			     (last->type() != Atom::FormattingRight ||
			      last->string() != ATOM_FORMATTING_INDEX) )
			    indexStartedPara = false;
		    }
		    startFormat( ATOM_FORMATTING_INDEX, command );
		    break;
		case CMD_KEYWORD:
		    x = getArgument();
		    insertTarget( x );
		    break;
		case CMD_L:
		    enterPara();
		    if ( isLeftBraceAhead() ) {
			x = getArgument();
			append( Atom::Link, x );
			if ( isLeftBraceAhead() ) {
			    startFormat( ATOM_FORMATTING_LINK, command );
			} else {
			    append( Atom::FormattingLeft,
				    ATOM_FORMATTING_LINK );
			    append( Atom::String, x );
			    append( Atom::FormattingRight,
				    ATOM_FORMATTING_LINK );	
			}
		    } else {
			x = getArgument();
			append( Atom::Link, x );
			append( Atom::FormattingLeft, ATOM_FORMATTING_LINK );
			append( Atom::String, x );
			append( Atom::FormattingRight, ATOM_FORMATTING_LINK );
		    }
		    break;
		case CMD_LEGALESE:
		    leavePara();
		    if (!inLegalese) {
			append(Atom::LegaleseLeft);
			inLegalese = true;
		    }
                    docPrivate->hasLegalese = true;
		    break;
		case CMD_LIST:
		    if ( openCommand(command) ) {
			leavePara();
			openedLists.push( OpenedList(location(),
						     getOptionalArgument()) );
		    }
		    break;
		case CMD_O:
		    leavePara();
		    if ( openedLists.isEmpty() ) {
#if 0
			location().warning( tr("Command '\\%1' outside '\\%2'")
					    .arg(commandName(command))
					    .arg(commandName(CMD_LIST)) );
#endif
		    } else {
			if ( openedLists.top().isStarted() ) {
			    append( Atom::ListItemRight,
				    openedLists.top().styleString() );
			} else {
			    append( Atom::ListLeft,
				    openedLists.top().styleString() );
			}
			openedLists.top().next();
			append( Atom::ListItemNumber,
				openedLists.top().numberString() );
			append( Atom::ListItemLeft,
				openedLists.top().styleString() );
			enterPara();
		    }
		    break;
		case CMD_OMIT:
		    getUntilEnd( command );
		    break;
		case CMD_OMITVALUE:
		    leaveValue();
		    x = getArgument();
		    priv->enumItemSet.insert( x );
		    priv->omitEnumItemSet.insert( x );
		    break;
		case CMD_PART:
		    startSection( Doc::Part, command );
		    break;
		case CMD_PRINTLINE:
		    leavePara();
		    appendToCode( quoter.quoteLine(location(), commandStr,
						   getRestOfLine()) );
		    break;
		case CMD_PRINTTO:
		    leavePara();
		    appendToCode( quoter.quoteTo(location(), commandStr,
				  getRestOfLine()) );
		    break;
		case CMD_PRINTUNTIL:
		    leavePara();
		    appendToCode( quoter.quoteUntil(location(), commandStr,
						    getRestOfLine()) );
		    break;
		case CMD_QUOTATION:
		    if ( openCommand(command) ) {
			leavePara();
			append( Atom::QuotationLeft );
		    }
		    break;
		case CMD_QUOTEFILE:
		    leavePara();
		    quoteFromFile( command );
		    append( Atom::Code,
			    quoter.quoteUntil(location(), commandStr) );
		    quoter.reset();
		    break;
		case CMD_QUOTEFROMFILE:
		    leavePara();
		    quoteFromFile( command );
		    break;
		case CMD_QUOTEFUNCTION:
		    leavePara();
		    marker = quoteFromFile( command );
		    x = getRestOfLine();
		    quoter.quoteTo( location(), commandStr,
				    slashed(marker->functionBeginRegExp(x)) );
		    append( Atom::Code,
			    quoter.quoteUntil(location(), commandStr,
				    slashed(marker->functionEndRegExp(x))) );
		    quoter.reset();
		    break;
		case CMD_RAW:
		    leavePara();
		    begin = pos;
		    x = getUntilEnd( command );
		    x = untabifyEtc( x );
#if 0
		    append( Atom::RawFormat, "HTML" ); // ###
		    append( Atom::RawString, x );
#endif
		    break;
		case CMD_ROW:
		    /* ... */
		    break;
		case CMD_SECTION1:
		    startSection( Doc::Section1, command );
		    break;
		case CMD_SECTION2:
		    startSection( Doc::Section2, command );
		    break;
		case CMD_SECTION3:
		    startSection( Doc::Section3, command );
		    break;
		case CMD_SECTION4:
		    startSection( Doc::Section4, command );
		    break;
		case CMD_SIDEBAR:
		    if ( openCommand(command) ) {
			leavePara();
			append( Atom::SidebarLeft );
		    }
		    break;
		case CMD_SKIPLINE:
		    leavePara();
		    quoter.quoteLine( location(), commandStr, getRestOfLine() );
		    break;
		case CMD_SKIPTO:
		    leavePara();
		    quoter.quoteTo( location(), commandStr, getRestOfLine() );
		    break;
		case CMD_SKIPUNTIL:
		    leavePara();
		    quoter.quoteUntil( location(), commandStr, getRestOfLine() );
		    break;
		case CMD_SUB:
		    startFormat( ATOM_FORMATTING_SUBSCRIPT, command );
		    break;
		case CMD_SUP:
		    startFormat( ATOM_FORMATTING_SUPERSCRIPT, command );
		    break;
		case CMD_TABLE:
		    if ( openCommand(command) ) {
			leavePara();
			append( Atom::TableLeft );
		    }
		    break;
		case CMD_TABLEOFCONTENTS:
		    append( Atom::TableOfContents,
			    QString::number((int) getSectioningUnit()) );
		    /* ... */
		    break;
		case CMD_TARGET:
		    insertTarget( getArgument() );
		    break;
		case CMD_TT:
		    startFormat( ATOM_FORMATTING_TELETYPE, command );
		    break;
		case CMD_UNDERLINE:
		    startFormat( ATOM_FORMATTING_UNDERLINE, command );
		    break;
		case CMD_VALUE:
		    leaveValue();
		    if ( openedLists.top().style() == OpenedList::Value ) {
			x = getArgument();
			priv->enumItemSet.insert( x );

			openedLists.top().next();
			append( Atom::ListTagLeft, ATOM_LIST_VALUE );
			append( Atom::FormattingLeft, ATOM_FORMATTING_BOLD );
			append( Atom::FormattingLeft,
				ATOM_FORMATTING_TELETYPE );
			append( Atom::String, x );
			append( Atom::FormattingRight,
				ATOM_FORMATTING_TELETYPE );
			append( Atom::FormattingRight, ATOM_FORMATTING_BOLD );
			append( Atom::ListTagRight, ATOM_LIST_VALUE );
			append( Atom::ListItemLeft, ATOM_LIST_VALUE );
			enterPara();
		    } else {
			// ### problems
		    }
		    break;
		case CMD_WARNING:
		    startNewPara();
		    append( Atom::FormattingLeft, ATOM_FORMATTING_BOLD );
		    append( Atom::String, "Warning:" );
		    append( Atom::FormattingRight, ATOM_FORMATTING_BOLD );
		    append( Atom::String, " " );
		    break;
		case UNKNOWN_COMMAND:
		    if ( metaCommandSet.contains(commandStr) ) {
			priv->metaCommandSet.insert( commandStr );
			priv->metaCommandMap[commandStr].append(getRestOfLine());
		    } else if (macroHash.contains(commandStr)) {
			const Macro &macro = macroHash.value(commandStr);
			int numPendingFi = 0;
			QMap<QString, QString>::ConstIterator d = macro.otherDefs.begin();
			while ( d != macro.otherDefs.end() ) {
			    append( Atom::FormatIf, d.key() );
			    expandMacro( commandStr, *d, macro.numParams );
			    ++d;
			    if (macro.defaultDef.isEmpty() && d == macro.otherDefs.end()) {
				append( Atom::FormatEndif );
			    } else {
				append( Atom::FormatElse );
				numPendingFi++;
			    }
			}
			if (!macro.defaultDef.isEmpty()) {
	                    location().push(macro.defaultDefLocation.filePath()); // ###
	                    in.insert(pos, macro.defaultDef);
	                    len = in.length();
	                    openedInputs.push(pos + macro.defaultDef.length());
			}
			while ( numPendingFi-- > 0 )
			    append( Atom::FormatEndif );
		    } else {
			location().warning(
				tr("Unknown command '\\%1'").arg(commandStr),
				detailsUnknownCommand(metaCommandSet, commandStr));
			enterPara();
			append(Atom::UnknownCommand, commandStr);
		    }
		}
	    }
	} else if ( ch == '{' ) {
	    enterPara();
	    appendChar( '{' );
	    braceDepth++;
	    pos++;
	} else if ( ch == '}' ) {
	    braceDepth--;
	    pos++;

	    QMap<int, QString>::Iterator f = pendingFormats.find( braceDepth );
	    if ( f == pendingFormats.end() ) {
		enterPara();
		appendChar( '}' );
	    } else {
		append( Atom::FormattingRight, *f );
		if ( *f == ATOM_FORMATTING_INDEX && indexStartedPara )
		    skipAllSpaces();
		pendingFormats.erase( f );
	    }
	} else {
	    if ( paraState == OutsidePara ) {
		if ( !ch.isSpace() ) {
		    enterPara();
		    appendChar( ch );
		}
		pos++;
	    } else {
		pos++;
		if ( ch == '\n' ) {
		    if ( paraState == InsideSingleLinePara || isBlankLine() ) {
			leavePara();
		    } else {
			appendChar( ' ' );
		    }
		} else {
		    appendChar( ch );
		}
	    }
	}
    }
    leaveValueList();

    if (inLegalese)
	append(Atom::LegaleseRight);

    if (openedCommands.top() != CMD_OMIT) {
	location().warning(tr("Missing '\\%1'").arg(endCommandName(openedCommands.top())));
    } else if (preprocessorSkipping.count() > 0) {
	location().warning(tr("Missing '\\%1'").arg(commandName(CMD_ENDIF)));
    }
    if (priv->extra && priv->extra->granularity < priv->extra->sectioningUnit)
	priv->extra->granularity = priv->extra->sectioningUnit;
    priv->text.stripFirstAtom();
}

Location &DocParser::location()
{
    while (!openedInputs.isEmpty() && openedInputs.top() <= pos) {
	cachedLoc.pop();
	cachedPos = openedInputs.pop();
    }
    while (cachedPos < pos)
	cachedLoc.advance(in[cachedPos++]);
    return cachedLoc;
}

QString DocParser::detailsUnknownCommand(const Set<QString> &metaCommandSet, const QString &str)
{
    Set<QString> commandSet = metaCommandSet;
    int i = 0;
    while ( cmds[i].english != 0 ) {
	commandSet.insert( cmds[i].alias );
	i++;
    }

    if ( aliasMap.contains(str) )
	return tr( "The command '\\%1' was renamed '\\%2' by the configuration"
		   " file. Use the new name." )
	       .arg(str).arg(aliasMap[str]);

    QString best = nearestName( str, commandSet );
    if ( best.isEmpty() ) {
	return "";
    } else {
	return tr( "Maybe you meant '\\%1'?" ).arg( best );
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
		location().warning( tr("Documentation expires today") );
	    } else if ( days == 1 ) {
		location().warning( tr("Documentation expired yesterday") );
	    } else if ( days >= 2 ) {
		location().warning( tr("Documentation expired %1 days ago")
				    .arg(days) );
	    }
	} else {
	    location().warning( tr("Date '%1' invalid").arg(date) );
	}
    } else {
	location().warning( tr("Date '%1' not in YYYY-MM-DD format")
			    .arg(date) );
    }
}

void DocParser::insertBaseName(const QString &baseName)
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
	location().warning( tr("Duplicate target name '%1'").arg(target) );
	targetMap[target].warning( tr("(The previous occurrence is here)") );
    } else {
	targetMap.insert( target, location() );
	append( Atom::Target, target );
    }
}

void DocParser::include( const QString& fileName )
{
    if ( location().depth() > 16 )
	location().fatal( tr("Too many nested '\\%1's")
			  .arg(commandName(CMD_INCLUDE)) );

    QString userFriendlyFilePath;
    QString filePath = Config::findFile( location(), sourceFiles, sourceDirs,
					 fileName, userFriendlyFilePath );
    if ( filePath.isEmpty() ) {
	location().warning( tr("Cannot find leaf file '%1'").arg(fileName) );
    } else {
	QFile inFile( filePath );
	if ( !inFile.open(IO_ReadOnly) ) {
	    location().warning( tr("Cannot open leaf file '%1'")
				.arg(userFriendlyFilePath) );
	} else {
	    location().push( userFriendlyFilePath );

	    QTextStream inStream( &inFile );
	    QString includedStuff = inStream.read();
	    inFile.close();

	    in.insert( pos, includedStuff );
	    len = in.length();
	    openedInputs.push( pos + includedStuff.length() );
	}
    }
}

void DocParser::startFormat( const QString& format, int command )
{
    enterPara();

    QMap<int, QString>::ConstIterator f = pendingFormats.begin();
    while ( f != pendingFormats.end() ) {
	if ( *f == format ) {
	    location().warning( tr("Cannot nest '\\%1' commands")
				.arg(commandName(command)) );
	    return;
	}
	++f;
    }

    append( Atom::FormattingLeft, format );

    if ( isLeftBraceAhead() ) {
	skipSpacesOrOneEndl();
	pendingFormats.insert( braceDepth, format );
	braceDepth++;
	pos++;
    } else {
	append( Atom::String, getArgument() );
	append( Atom::FormattingRight, format );
	if ( format == ATOM_FORMATTING_INDEX && indexStartedPara ) {
	    skipAllSpaces();
	    indexStartedPara = false;
	}
    }
}

bool DocParser::openCommand( int command )
{
    int top = openedCommands.top();
    bool ok = true;

    if ( top != CMD_OMIT && top != CMD_LIST ) {
	QList<int> ordering;
	ordering << CMD_ABSTRACT << CMD_SIDEBAR << CMD_QUOTATION << CMD_TABLE
		 << CMD_FOOTNOTE;
	ok = ordering.indexOf( top ) < ordering.indexOf( command );
    }

    if ( ok ) {
	openedCommands.push( command );
    } else {
	location().warning( tr("Cannot use '\\%1' within '\\%2'")
			    .arg(commandName(command)).arg(commandName(top)) );
    }
    return ok;
}

bool DocParser::closeCommand( int endCommand )
{
    if ( endCommandFor(openedCommands.top()) == endCommand ) {
	openedCommands.pop();
	return true;
    } else {
	bool contains = false;
	QStack<int> opened2 = openedCommands;
	while ( !opened2.isEmpty() ) {
	    if ( endCommandFor(opened2.top()) == endCommand ) {
		contains = true;
		break;
	    }
	    opened2.pop();
	}

	if ( contains ) {
	    while ( endCommandFor(openedCommands.top()) != endCommand ) {
		location().warning( tr("Missing '\\%1' before '\\%2'")
				    .arg(endCommandName(openedCommands.top()))
				    .arg(commandName(endCommand)) );
		openedCommands.pop();
	    }
	} else {
	    location().warning( tr("Missing '\\%1'")
				.arg(commandName(endCommand)) );
	}
	return false;
    }
}

void DocParser::startSection( Doc::SectioningUnit unit, int command )
{
    leavePara();

    if ( currentSectioningUnit == Doc::Book ) {
	if ( unit > Doc::Section1 )
	    location().warning( tr("Unexpected '\\%1' without '\\%2'")
				.arg(commandName(command))
				.arg(commandName(CMD_SECTION1)) );
	currentSectioningUnit = (Doc::SectioningUnit) ( unit - 1 );
	priv->constructExtra();
	priv->extra->sectioningUnit = currentSectioningUnit;
    }

    if ( unit <= priv->extra->sectioningUnit ) {
	location().warning( tr("Unexpected '\\%1' in this documentation")
			    .arg(commandName(command)) );
    } else if ( unit - currentSectioningUnit > 1 ) {
	location().warning( tr("Unexpected '\\%1' at this point")
			    .arg(commandName(command)) );
    } else {
	if ( currentSectioningUnit >= unit )
	    endSection( unit, command );

	int delta = unit - priv->extra->sectioningUnit;
	append( Atom::SectionLeft, QString::number(delta) );
	enterPara( Atom::SectionHeadingLeft, Atom::SectionHeadingRight,
		   QString::number(delta) );
	currentSectioningUnit = unit;
    }
}

void DocParser::endSection( int unit, int endCommand )
{
    leavePara();

    if ( unit < priv->extra->sectioningUnit ) {
	location().warning( tr("Unexpected '\\%1' in this documentation")
			    .arg(commandName(endCommand)) );
    } else if ( unit > currentSectioningUnit ) {
	location().warning( tr("Unexpected '\\%1' at this point")
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
    leavePara();
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
	     << Atom( Atom::FormattingLeft, ATOM_FORMATTING_LINK ) << str
	     << Atom( Atom::FormattingRight, ATOM_FORMATTING_LINK );
	priv->addAlso( also );

	skipSpacesOnLine();
	if ( pos < len && in[pos] == ',' ) {
	    pos++;
	    skipSpacesOrOneEndl();
	} else if ( in[pos] != '\n' ) {
	    location().warning( tr("Missing comma in '\\%1'")
				.arg(commandName(CMD_ALSO)) );
	}
    }
}

void DocParser::append(Atom::Type type, const QString &string)
{
    if (priv->text.lastAtom()->type() == Atom::Code
	&& priv->text.lastAtom()->string().endsWith("\n\n"))
	priv->text.lastAtom()->chopString();
    priv->text << Atom(type, string);
}

void DocParser::appendChar(QChar ch)
{
    if (priv->text.lastAtom()->type() != Atom::String)
	append(Atom::String);
    if (ch.isSpace()) {
	if (!priv->text.lastAtom()->string().endsWith(" "))
	    priv->text.lastAtom()->appendChar(' ');
    } else {
	priv->text.lastAtom()->appendChar(ch);
    }
}

void DocParser::appendToCode( const QString& markedCode )
{
    if ( priv->text.lastAtom()->type() != Atom::Code )
	append( Atom::Code );
    priv->text.lastAtom()->appendString( markedCode );
}

void DocParser::startNewPara()
{
    leavePara();
    enterPara();
}

void DocParser::enterPara( Atom::Type leftType, Atom::Type rightType,
			   const QString& string )
{
    if ( paraState == OutsidePara ) {
	leaveValueList();
	append( leftType, string );
	indexStartedPara = false;
	pendingParaLeftType = leftType;
	pendingParaRightType = rightType;
	pendingParaString = string;
	if ( 
#if 0
	     leftType == Atom::BriefLeft ||
#endif
	     leftType == Atom::SectionHeadingLeft ) {
	    paraState = InsideSingleLinePara;
	} else {
	    paraState = InsideMultiLinePara;
	}
	skipSpacesOrOneEndl();
    }
}

void DocParser::leavePara()
{
    if ( paraState != OutsidePara ) {
	if ( !pendingFormats.isEmpty() ) {
	    location().warning( tr("Missing '}'") );
	    pendingFormats.clear();
	}

	if ( priv->text.lastAtom()->type() == pendingParaLeftType ) {
	    priv->text.stripLastAtom();
	} else {
	    if ( priv->text.lastAtom()->type() == Atom::String &&
		 priv->text.lastAtom()->string().endsWith(" ") ) {
		priv->text.lastAtom()->chopString();
	    }
	    append( pendingParaRightType, pendingParaString );
	}
	paraState = OutsidePara;
	indexStartedPara = false;
	pendingParaRightType = Atom::Nop;
	pendingParaString = "";
    }
}

void DocParser::leaveValue()
{
    leavePara();
    if ( openedLists.isEmpty() ) {
	openedLists.push( OpenedList(OpenedList::Value) );
	append( Atom::ListLeft, ATOM_LIST_VALUE );
    } else {
	append( Atom::ListItemRight, ATOM_LIST_VALUE );
    }
}

void DocParser::leaveValueList()
{
    leavePara();
    if ( !openedLists.isEmpty() &&
	 openedLists.top().style() == OpenedList::Value &&
	 priv->text.lastAtom()->type() != Atom::ListItemLeft ) {
	append( Atom::ListItemRight, ATOM_LIST_VALUE );
	append( Atom::ListRight, ATOM_LIST_VALUE );
	openedLists.pop();
    }
}

CodeMarker *DocParser::quoteFromFile( int /* command */ )
{
    quoter.reset();

    QString code;
    QString fileName = getArgument();

    QString userFriendlyFilePath;
    QString filePath = Config::findFile( location(), exampleFiles, exampleDirs,
					 fileName, userFriendlyFilePath );
    if ( filePath.isEmpty() ) {
	location().warning( tr("Cannot find example file '%1'").arg(fileName) );
    } else {
	QFile inFile( filePath );
	if ( !inFile.open(IO_ReadOnly) ) {
	    location().warning( tr("Cannot open example file '%1'")
				.arg(userFriendlyFilePath) );
	} else {
	    QTextStream inStream( &inFile );
	    code = untabifyEtc( inStream.read() );
	    inFile.close();
	}
    }

    QString dirPath = QFileInfo( filePath ).dirPath();
    CodeMarker *marker = CodeMarker::markerForFileName( fileName );
    quoter.quoteFromFile( userFriendlyFilePath, code,
			  marker->markedUpCode(code, 0, dirPath) );
    return marker;
}

void DocParser::expandMacro( const QString& name, const QString& def,
			     int numParams )
{
    if ( numParams == 0 ) {
	append( Atom::RawString, def );
    } else {
	QStringList args;
	QString rawString;

	for ( int i = 0; i < numParams; i++ ) {
	    if ( numParams == 1 || isLeftBraceAhead() ) {
		args << getArgument( true );
	    } else {
		location().warning( tr("Macro '\\%1' invoked with too few"
				       " arguments (expected %2, got %3)")
				    .arg(name).arg(numParams).arg(i) );
		break;
	    }
	}

	int i = 0;
	while ( i < (int) def.length() ) {
	    int paramNo = def[i + 1].digitValue();
	    if ( def[i] == '\\' && paramNo >= 1 && paramNo <= numParams ) {
		if ( !rawString.isEmpty() ) {
		    append( Atom::RawString, rawString );
		    rawString = "";
		}
		append( Atom::String, args[paramNo - 1] );
		i += 2;
	    } else {
		rawString += def[i++];
	    }
	}
	if ( !rawString.isEmpty() )
	    append( Atom::RawString, rawString );
    }
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
	location().warning( tr("Invalid sectioning unit '%1'").arg(name) );
	return Doc::Book;
    }
}

QString DocParser::getArgument( bool verbatim )
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
		if ( verbatim ) {
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
	    location().warning( tr("Missing '}'") );
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
		if ( verbatim ) {
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

	if ( arg.length() > 1 && QString(".,:;!?").indexOf(in[pos - 1]) != -1 &&
	     !arg.endsWith("...") ) {
	    arg.truncate( arg.length() - 1 );
	    pos--;
	}
	if ( arg.length() > 2 && in.mid(pos - 2, 2) == "'s" ) {
	    arg.truncate( arg.length() - 2 );
	    pos -= 2;
	}
    }
    return arg.simplified();
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

    QString t = in.mid( begin, pos - begin ).simplified();
    skipSpacesOnLine();
    return t;
}

QString DocParser::getUntilEnd(int command)
{
    int endCommand = endCommandFor( command );
    QRegExp rx( "\\\\" + commandName(endCommand) + "\\b" );
    QString t;
    int end = rx.search( in, pos );

    if ( end == -1 ) {
	location().warning( tr("Missing '\\%1'").arg(commandName(endCommand)) );
	pos = in.length();
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
	    return true;
	i++;
    }
    return false;
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

void DocParser::skipToNextPreprocessorCommand()
{
    QRegExp rx("\\\\(?:" + commandName(CMD_IF) + "|" + commandName(CMD_ELSE) + "|"
	       + commandName(CMD_ENDIF) + ")\\b");
    int end = rx.search(in, pos + 1); // ### + 1 necessary?

    if (end == -1)
	pos = in.length();
    else
	pos = end;
}

int DocParser::endCommandFor( int command )
{
    switch ( command ) {
    case CMD_ABSTRACT:
	return CMD_ENDABSTRACT;
    case CMD_CHAPTER:
	return CMD_ENDCHAPTER;
    case CMD_CODE:
	return CMD_ENDCODE;
    case CMD_FOOTNOTE:
	return CMD_ENDFOOTNOTE;
    case CMD_LIST:
	return CMD_ENDLIST;
    case CMD_OMIT:
	return CMD_ENDOMIT;
    case CMD_PART:
	return CMD_ENDPART;
    case CMD_QUOTATION:
	return CMD_ENDQUOTATION;
    case CMD_SECTION1:
	return CMD_ENDSECTION1;
    case CMD_SECTION2:
	return CMD_ENDSECTION2;
    case CMD_SECTION3:
	return CMD_ENDSECTION3;
    case CMD_SECTION4:
	return CMD_ENDSECTION4;
    case CMD_SIDEBAR:
	return CMD_ENDSIDEBAR;
    case CMD_TABLE:
	return CMD_ENDTABLE;
    default:
	return command;
    }
}

QString DocParser::commandName( int command )
{
    return cmds[command].alias;
}

QString DocParser::endCommandName( int command )
{
    return commandName( endCommandFor(command) );
}

QString DocParser::untabifyEtc( const QString& str )
{
    QString result;
    int column = 0;

    for ( int i = 0; i < (int) str.length(); i++ ) {
	if ( str[i] == '\t' ) {
	    result += "        " + ( column % tabSize );
	    column = ( (column / tabSize) + 1 ) * tabSize;
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

int DocParser::indentLevel( const QString& str )
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

QString DocParser::unindent( int level, const QString& str )
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

QString DocParser::slashed( const QString& str )
{
    QString result = str;
    result.replace( "/", "\\/" );
    return "/" + result + "/";
}

#define COMMAND_BRIEF                   Doc::alias("brief")

Doc::Doc( const Location& location, const QString& source,
	  const Set<QString>& metaCommandSet )
{
    priv = new DocPrivate( location, source );
    DocParser parser;
    parser.parse( source, priv, metaCommandSet );
}

Doc::Doc( const Doc& doc )
    : priv( 0 )
{
    operator=( doc );
}

Doc::~Doc()
{
    if ( priv != 0 && priv->deref() )
	delete priv;
}

Doc& Doc::operator=( const Doc& doc )
{
    if ( doc.priv != 0 )
	doc.priv->ref();
    if ( priv != 0 && priv->deref() )
	delete priv;
    priv = doc.priv;
    return *this;
}

const Location& Doc::location() const
{
    static const Location dummy;
    return priv == 0 ? dummy : priv->loc;
}

const QString& Doc::source() const
{
    static QString null;
    return priv == 0 ? null : priv->src;
}

bool Doc::isEmpty() const
{
    return priv == 0 || priv->src.isEmpty();
}

const Text& Doc::body() const
{
    static const Text dummy;
    return priv == 0 ? dummy : priv->text;
}

Text Doc::briefText() const
{
    return body().subText( Atom::BriefLeft, Atom::BriefRight );
}

Text Doc::trimmedBriefText(const QString &className) const
{
    Text text = briefText();
    Atom *atom = text.firstAtom();
    if (atom && atom->type() == Atom::String) {
        QString whats;
        bool standardWording = true;

        QStringList w = atom->string().split(" ");
        if (!w.isEmpty() && w.first() == "The")
	    w.removeFirst();
        else
	    standardWording = false;

        if (!w.isEmpty() && w.first() == className)
	    w.removeFirst();
        else
	    standardWording = false;

        if (!w.isEmpty() && (w.first() == "class" || w.first() == "widget"))
	    w.removeFirst();
        else
	    standardWording = false;

        if (!w.isEmpty() && (w.first() == "is" || w.first() == "provides" || w.first() == "contains"
			     || w.first() == "specifies"))
	    w.removeFirst();

        if (!w.isEmpty() && (w.first() == "a" || w.first() == "an"))
	    w.removeFirst();

        whats = w.join(" ");
        if (whats.endsWith("."))
	    whats.truncate(whats.length() - 1);

        if (whats.isEmpty())
	    standardWording = false;
        else
	    whats[0] = whats[0].toUpper();

	// ### move this once \brief is abolished for properties
        if (!standardWording) {
	    location().warning(tr("Nonstandard wording in '\\%1' text for '%2'")
			       .arg(COMMAND_BRIEF).arg(className));
	} else {
	    atom->setString(whats);
        }
    }
    return text;
}

Text Doc::legaleseText() const
{
    if (priv == 0 || !priv->hasLegalese)
	return Text();
    else
	return body().subText(Atom::LegaleseLeft, Atom::LegaleseRight);
}

const QString& Doc::baseName() const
{
    static QString null;
    if ( priv == 0 || priv->extra == 0 ) {
	return null;
    } else {
	return priv->extra->baseName;
    }
}

Doc::SectioningUnit Doc::granularity() const
{
    if ( priv == 0 || priv->extra == 0 ) {
	return DocPrivateExtra().granularity;
    } else {
	return priv->extra->granularity;
    }
}

#if notyet // ###
Doc::SectioningUnit Doc::sectioningUnit() const
{
    if ( priv == 0 || priv->extra == 0 ) {
	return DocPrivateExtra().sectioningUnit;
    } else {
	return priv->extra->sectioningUnit;
    }
}
#endif

const Set<QString> &Doc::parameterNames() const
{
    return priv == 0 ? null_Set_QString : priv->params;
}

const Set<QString> &Doc::enumItemNames() const
{
    return priv == 0 ? null_Set_QString : priv->enumItemSet;
}

const Set<QString> &Doc::omitEnumItemNames() const
{
    return priv == 0 ? null_Set_QString : priv->omitEnumItemSet;
}

const Set<QString> &Doc::metaCommandsUsed() const
{
    return priv == 0 ? null_Set_QString : priv->metaCommandSet;
}

QStringList Doc::metaCommandArgs( const QString& metaCommand ) const
{
    if ( priv == 0 ) {
	return QStringList();
    } else {
	return priv->metaCommandMap.value(metaCommand);
    }
}

const QList<Text> &Doc::alsoList() const
{
    return priv == 0 ? null_QList_Text : priv->alsoList;
}

void Doc::initialize( const Config& config )
{
    DocParser::tabSize = config.getInt( CONFIG_TABSIZE );
    DocParser::exampleFiles = config.getStringList( CONFIG_EXAMPLES );
    DocParser::exampleDirs = config.getStringList( CONFIG_EXAMPLEDIRS );
    DocParser::sourceFiles = config.getStringList( CONFIG_SOURCES );
    DocParser::sourceDirs = config.getStringList( CONFIG_SOURCEDIRS );

    QMap<QString, QString> reverseAliasMap;

    aliasMap.ensure_constructed();
    commandHash.ensure_constructed();

    Set<QString> commands = config.subVars( CONFIG_ALIAS );
    Set<QString>::ConstIterator c = commands.begin();
    while ( c != commands.end() ) {
	QString alias = config.getString( CONFIG_ALIAS + Config::dot + *c );
	if ( reverseAliasMap.contains(alias) ) {
	    config.lastLocation().warning( tr("Command name '\\%1' cannot stand"
					      " for both '\\%2' and '\\%3'")
					   .arg(alias)
					   .arg(reverseAliasMap[alias])
					   .arg(*c) );
	} else {
	    reverseAliasMap.insert( alias, *c );
	}
	aliasMap.insert(*c, alias);
	++c;
    }

    int i = 0;
    while ( cmds[i].english ) {
	cmds[i].alias = alias( cmds[i].english );
	commandHash.insert(cmds[i].alias, cmds[i].no);

	if ( cmds[i].no != i )
	    Location::internalError( tr("command %1 missing").arg(i) );
	i++;
    }

    macroHash.ensure_constructed();

    Set<QString> macroNames = config.subVars( CONFIG_MACRO );
    Set<QString>::ConstIterator n = macroNames.begin();
    while ( n != macroNames.end() ) {
	QString macroDotName = CONFIG_MACRO + Config::dot + *n;
	Macro macro;
	macro.numParams = -1;
	macro.defaultDef = config.getString( macroDotName );
	if ( !macro.defaultDef.isEmpty() ) {
	    macro.defaultDefLocation = config.lastLocation();
	    macro.numParams = Config::numParams( macro.defaultDef );
	}
	bool silent = false;

	Set<QString> formats = config.subVars( macroDotName );
	Set<QString>::ConstIterator f = formats.begin();
	while ( f != formats.end() ) {
	    QString def = config.getString( macroDotName + Config::dot + *f );
	    if ( !def.isEmpty() ) {
		macro.otherDefs.insert( *f, def );
		int m = Config::numParams( macro.defaultDef );
		if ( macro.numParams == -1 ) {
		    macro.numParams = m;
		} else if ( macro.numParams != m ) {
		    if ( !silent ) {
			QString other = tr( "default" );
			if ( macro.defaultDef.isEmpty() )
			    other = macro.otherDefs.begin().key();
			config.lastLocation().warning( tr("Macro '\\%1' takes"
							  " inconsistent number"
							  " of arguments (%2"
							  " %3, %4 %5)")
						       .arg(*n)
						       .arg(*f)
						       .arg(m)
						       .arg(other)
						       .arg(macro.numParams) );
			silent = true;
		    }
		    if ( macro.numParams < m )
			macro.numParams = m;
		}
	    }
	    ++f;
	}

	if (macro.numParams != -1)
	    macroHash.insert(*n, macro);
	++n;
    }
}

void Doc::terminate()
{
    DocParser::exampleFiles.clear();
    DocParser::exampleDirs.clear();
    DocParser::sourceFiles.clear();
    DocParser::sourceDirs.clear();
    aliasMap.clear();
    commandHash.clear();
    macroHash.clear();
}

QString Doc::alias(const QString &english)
{
    return aliasMap.value(english, english);
}

void Doc::trimCStyleComment( Location& location, QString& str )
{
    QString cleaned;
    Location m = location;
    bool metAsterColumn = true;
    int asterColumn = location.columnNo() + 1;
    int i;

    for ( i = 0; i < (int) str.length(); i++ ) {
	if ( m.columnNo() == asterColumn ) {
	    if ( str[i] != '*' )
		break;
	    cleaned += ' ';
	    metAsterColumn = true;
	} else {
	    if ( str[i] == '\n' ) {
		if ( !metAsterColumn )
		    break;
		metAsterColumn = false;
	    }
	    cleaned += str[i];
	}
	m.advance( str[i] );
    }
    if ( cleaned.length() == str.length() )
	str = cleaned;

    for ( int i = 0; i < 3; i++ )
	location.advance( str[i] );
    str = str.mid( 3, str.length() - 5 );
}
