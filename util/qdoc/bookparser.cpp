/*
  bookparser.cpp
*/

#include <qfile.h>
#include <qptrstack.h>
#include <qvaluestack.h>

#include <ctype.h>
#include <stdio.h>

#include "binarywriter.h"
#include "bookparser.h"
#include "config.h"
#include "html.h"
#include "htmlwriter.h"
#include "location.h"
#include "messages.h"
#include "parsehelpers.h"
#include "resolver.h"
#include "walkthrough.h"

#define BCONSUME( target, where ) \
    CONSUME( target ); \
    where()

static void noWarning( int /* level */, const Location& /* loc */,
		       const char * /* message */, ... )
{
}

static QString fixBackslashes( const QString& str )
{
    QString t;
    int i = 0;

    while ( i < (int) str.length() ) {
	QChar ch = str[i++];
	if ( ch == '\\' && i < (int) str.length() )
	    ch = str[i++];
	t += ch;
    }
    return t;
}

static QString unhtmlize( const QString& html )
{
    static QRegExp tag( QString("<[^>]*>") );
    static QRegExp lt( QString("&lt;") );
    static QRegExp gt( QString("&gt;") );
    static QRegExp amp( QString("&amp;") );

    QString t = html;
    t.replace( tag, QString::null );
    t.replace( lt, QChar('<') );
    t.replace( gt, QChar('>') );
    t.replace( amp, QChar('&') );
    return t;
}

static QString sgmlProtect( const QString& str )
{
#if 1
    return htmlProtect( str );
#else
    static QRegExp amp( QChar('&') );
    static QRegExp amp( QChar('<') );
    static QRegExp amp( QChar('>') );

    QString t = str;
#endif
}

class Processor
{
public:
    Processor( const QString& filePath, const Resolver *resolver = 0 );
    virtual ~Processor() { }

    void start( bool verbose );

protected:
    virtual bool supportsCodeHtml() const { return FALSE; }

    virtual void processAlias( const QString& /* alias */,
			       const QStringList& /* args */ ) { }
    virtual void processC( const QString& text );
    virtual void processCaptionBegin() { }
    virtual void processCaptionEnd() { }
    virtual void processChar( QChar /* ch */ ) { }
    virtual void processCode( const QString& /* text */ ) { }
    virtual void processCodeHtml( const QString& /* text */ ) { }
    virtual void processE( const QString& text );
    virtual void processFootnoteBegin() { }
    virtual void processFootnoteEnd() { }
    virtual void processGranularity( int /* level */ ) { }
    virtual void processImg( const QString& /* fileName */,
			     const QString& /* alt */,
			     bool /* inParagraph */ ) { }
    virtual void processIndex( const QString& /* text */ ) { }
    virtual void processLink( const QString& name,
			      const QString& text = QString::null );
    virtual void processListBegin( OpenedList * /* ol */ ) { }
    virtual void processListItem( OpenedList * /* ol */ ) { }
    virtual void processListEnd( OpenedList * /* ol */ ) { }
    virtual void processParagraphBegin() { }
    virtual void processParagraphEnd() { }
    virtual void processQuoteBegin() { }
    virtual void processQuoteEnd() { }
    virtual void processSectionBegin( int /* level */, int /* topLevel */ ) { }
    virtual void processSectionHeadingEnd( int /* level */,
					   int /* topLevel */ ) { }
    virtual void processSectionEnd( int /* level */, int /* topLevel */ ) { }
    virtual void processSidebarBegin() { }
    virtual void processSidebarHeadingEnd() { }
    virtual void processSidebarEnd() { }
    virtual void processString( const QString& str );
    virtual void processTableOfContents() { }
    virtual void processTarget( const QString& /* target */ ) { }
    virtual void processTheIndex() { }
    virtual void processTitle( const QString& /* title */ ) { }

    const Resolver *resolver() const { return res; }
    const Location& location();

private:
    // ### rename State Context
    enum State { Normal, InCaption, InPrint, InQuote, InSectionHeading,
		 InSidebarHeading, InFootnote };
    enum SpacePolicy { Keeping, Skipping, Pending };

    static bool isParagraphState( State s ) {
	return s == Normal || s == InQuote;
    }

    void input( const QString& filePath );
    QString getFilePath( const QStringList& dirList, const QString& command );
    QString fileContents( const QString& fileName, const QString& fileKind );
    void endSections( int prevLevel, int newLevel, int topLevel );
    void enterState( State newState, const QString& command );
    void maybeInlined();
    void inprint();
    void inlined();
    void outlined();

    void (*warning)( int, const Location&, const char *, ... );
    const Resolver *res;
    Location loc;
    int locPos;
    bool inParagraph;
    bool inSidebar;
    State state;
    SpacePolicy spacePolicy;
    QString pendingPrint;

    QString yyIn;
    int yyPos;
    int yyLen;
};

Processor::Processor( const QString& filePath, const Resolver *resolver )
    : res( resolver ), locPos( 0 ), inParagraph( FALSE ), inSidebar( FALSE ),
      state( Normal ), spacePolicy( Skipping ), yyPos( 0 ), yyLen( 0 )
{
    input( filePath );
}

void Processor::start( bool verbose )
{
    QRegExp sectionX( QString("section([1-4])") );

    Walkthrough walk;
    QString alt;
    QString fileName;
    QString filePath;
    QString name;
    QString substr;
    QString x;
    int begin;
    int end;

    warning = verbose ? ::warning : noWarning;

    int topSectionLevel = -1;
    int prevSectionLevel = 0;
    int sectionLevel = 0;

    QValueStack<OpenedList> openedLists;

    while ( yyPos < yyLen ) {
	QChar ch = yyIn[yyPos++];

	if ( ch == '\\' ) {
	    QString command;
	    begin = yyPos;
	    while ( yyPos < yyLen ) {
		ch = yyIn[yyPos];
		if ( isalpha((uchar) ch.latin1()) ) {
		    command += ch;
		} else {
		    break;
		}
		yyPos++;
	    }

	    if ( yyPos == begin )
		command = QChar( '\0' );

	    int h = HASH( command[0].unicode(), command.length() );
	    bool consumed = FALSE;

	    switch ( h ) {
	    case HASH( '\0', 1 ):
		BCONSUME( "", inlined );
		if ( yyPos < yyLen )
		    processChar( yyIn[yyPos++] );
		break;
	    case HASH( 'a', 8 ):
		BCONSUME( "abstract", outlined );
		break;
	    case HASH( 'c', 1 ):
		BCONSUME( "c", inlined );
		processC( getArgument(yyIn, yyPos) );
		break;
	    case HASH( 'c', 4 ):
		BCONSUME( "code", outlined );
		begin = yyPos;
		end = yyIn.find( QString("\\endcode"), yyPos );
		if ( end == -1 ) {
		    warning( 2, location(), "Missing '\\endcode'" );
		} else {
		    processCode( yyIn.mid(begin, end - begin) );
		    yyPos = end + 8;
		}
		break;
	    case HASH( 'c', 7 ):
		if ( command[1] == QChar('a') ) {
		    BCONSUME( "caption", outlined );
		    enterState( InCaption, command );
		    processCaptionBegin();
		    skipSpacesOrNL( yyIn, yyPos );
		} else {
		    BCONSUME( "chapter", outlined );
		    sectionLevel = 0;

		    if ( sectionLevel < topSectionLevel ) {
			warning( 2, location(),
				 "Unexpected '\\chapter' in this document" );
		    } else {
			// see also '\chapter'
			if ( topSectionLevel == -1 ) {
			    topSectionLevel = sectionLevel;
			} else {
			    endSections( prevSectionLevel, sectionLevel,
					 topSectionLevel );
			}

			enterState( InSectionHeading, command );
			processSectionBegin( sectionLevel, topSectionLevel );
			skipSpacesOrNL( yyIn, yyPos );
		    }
		}
		break;
	    case HASH( 'e', 1 ):
		BCONSUME( "e", inlined );
		processE( fixBackslashes(getArgument(yyIn, yyPos)) );
		break;
	    case HASH( 'e', 7 ):
		if ( command[5] == QChar('d') ) {
		    BCONSUME( "endcode", outlined );
		    warning( 2, location(), "Missing '\\code'" );
		} else if ( command[5] == QChar('i') ) {
		    BCONSUME( "endomit", outlined );
		    warning( 2, location(), "Missing '\\omit'" );
		} else if ( command[5] == QChar('n') ) {
		    BCONSUME( "endlink", outlined );
		    warning( 2, location(), "Missing '\\link'" );
		} else if ( command[5] == QChar('s') ) {
		    BCONSUME( "endlist", outlined );
		    if ( openedLists.isEmpty() ) {
			warning( 2, location(), "Missing '\\list'" );
		    } else {
			processListEnd( &openedLists.top() );
			openedLists.pop();
		    }
		}
		break;
	    case HASH( 'e', 8 ):
		if ( command[3] == QChar('q') ) {
		    BCONSUME( "endquote", outlined );
		    if ( state == InQuote ) {
			state = Normal;
			processQuoteEnd();
		    } else {
			warning( 2, location(), "Unexpected '\\endquote'" );
		    }
		} else {
		    BCONSUME( "endtable", outlined );
		}
		break;
	    case HASH( 'e', 10 ):
		BCONSUME( "endsidebar", outlined );
		if ( inSidebar ) {
		    inSidebar = FALSE;
		    processSidebarEnd();
		} else {
		    warning( 2, location(), "Unexpected '\\endsidebar'" );
		}
		break;
	    case HASH( 'e', 11 ):
		BCONSUME( "endfootnote", outlined );
		if ( state == InFootnote ) {
		    state = Normal;
		    processFootnoteEnd();
		} else {
		    warning( 2, location(), "Missing '\\footnote'" );
		}
		break;
	    case HASH( 'f', 8 ):
		BCONSUME( "footnote", inlined );
		enterState( InFootnote, command );
		processFootnoteBegin();
		break;
	    case HASH( 'g', 11 ):
		BCONSUME( "granularity", outlined );
		x = getWord( yyIn, yyPos );
		if ( x == QString("chapter") ) {
		    processGranularity( 0 );
		} else if ( sectionX.exactMatch(x) ) {
		    processGranularity( sectionX.cap(1)[0].digitValue() );
		} else {
		    warning( 2, location(),
			     "Expected 'chapter' or 'section1' or ... or"
			     " 'section4' after '\\granularity'" );
		}
		break;
	    case HASH( 'i', 1 ):
		BCONSUME( "i", outlined );
		if ( openedLists.isEmpty() ) {
		    warning( 2, location(), "Command '\\i' outside '\\list'" );
		} else {
		    processListItem( &openedLists.top() );
		}
		break;
	    case HASH( 'i', 3 ):
		BCONSUME( "img", maybeInlined );
		x = getWord( yyIn, yyPos );
		alt = fixBackslashes( getRestOfLine(yyIn, yyPos) );
		processImg( x, alt, inParagraph );
		break;
	    case HASH( 'i', 5 ):
		if ( command[2] == QChar('d') ) {
		    CONSUME( "index" );
		    processIndex( getRestOfLine(yyIn, yyPos) );
		} else {
		    BCONSUME( "input", outlined );
		    filePath = getFilePath( config->bookDirList(), command );
		    if ( !filePath.isEmpty() )
			input( filePath );
		}
		break;
	    case HASH( 'i', 7 ):
		BCONSUME( "include", outlined );
		fileName = getWord( yyIn, yyPos );
		if ( !fileName.isEmpty() ) {
		    QString text = walk.includePass2( fileName, resolver() );
		    if ( supportsCodeHtml() ) {
			processCodeHtml( text );
		    } else {
			processCode( unhtmlize(text) );
		    }
		}
		break;
	    case HASH( 'l', 1 ):
		BCONSUME( "l", inlined );
		processLink( getArgument(yyIn, yyPos) );
		break;
	    case HASH( 'l', 4 ):
		if ( command[2] == QChar('n') ) {
		    BCONSUME( "link", inlined );
		    name = getArgument( yyIn, yyPos );
		    begin = yyPos;
		    end = yyIn.find( QString("\\endlink"), yyPos );
		    if ( end == -1 ) {
			warning( 2, location(), "Missing '\\endlink'" );
		    } else {
			processLink( name, yyIn.mid(begin, end - begin)
					       .simplifyWhiteSpace() );
			yyPos = end + 8;
		    }
		} else {
		    BCONSUME( "list", outlined );
		    openedLists.push( openList(location(),
					       getWord(yyIn, yyPos)) );
		    processListBegin( &openedLists.top() );
		}
		break;
	    case HASH( 'l', 8 ):
		BCONSUME( "location", outlined );
		loc = Location::fromString( getArgument(yyIn, yyPos) );
		locPos = yyPos;
		break;
	    case HASH( 'o', 4 ):
		BCONSUME( "omit", outlined );
		end = yyIn.find( QString("\\endomit"), yyPos );
		if ( end == -1 ) {
		    yyPos = yyIn.length();
		} else {
		    yyPos = end + 8;
		}
		break;
	    case HASH( 'p', 7 ):
		BCONSUME( "printto", inprint );
		substr = getRestOfLine( yyIn, yyPos );
		pendingPrint += walk.printto( substr, location() );
		break;
	    case HASH( 'p', 9 ):
		BCONSUME( "printline", inprint );
		substr = getRestOfLine( yyIn, yyPos );
		pendingPrint += walk.printline( substr, location() );
		break;
	    case HASH( 'p', 10 ):
		BCONSUME( "printuntil", inprint );
		substr = getRestOfLine( yyIn, yyPos );
		pendingPrint += walk.printuntil( substr, location() );
		break;
	    case HASH( 'q', 5 ):
		BCONSUME( "quote", outlined );
		enterState( InQuote, command );
		processQuoteBegin();
		break;
	    case HASH( 'q', 9 ):
		BCONSUME( "quotefile", outlined );
		fileName = getWord( yyIn, yyPos );
		if ( !fileName.isEmpty() )
		    walk.startPass2( fileName, resolver() );
		break;
	    case HASH( 's', 6 ):
		CONSUME( "skipto" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipto( substr, location() );
		break;
	    case HASH( 's', 7 ):
		if ( command[1] == QChar('e') ) {
		    BCONSUME( "section", outlined );
		    x = getWord( yyIn, yyPos );
		    if ( x.length() != 1 || x[0].unicode() < '1' ||
			 x[0].unicode() > '4' ) {
			warning( 2, location(),
				 "Expected digit between '1' and '4' after"
				 " '\\section'" );
			x = QChar( '1' );
		    }

		    sectionLevel = x[0].digitValue();

		    if ( sectionLevel - prevSectionLevel > 1 ) {
			warning( 2, location(),
				 "Unexpected '\\section%d' within"
				 " '\\section%d'", sectionLevel,
				 prevSectionLevel );
		    } else if ( sectionLevel < topSectionLevel ) {
			warning( 2, location(),
				 "Unexpected '\\section%d in this document",
				 sectionLevel );
		    } else {
			// see also '\chapter'
			if ( topSectionLevel == -1 ) {
			    topSectionLevel = sectionLevel;
			} else {
			    endSections( prevSectionLevel, sectionLevel,
					 topSectionLevel );
			}

			enterState( InSectionHeading, command );
			processSectionBegin( sectionLevel, topSectionLevel );
			skipSpacesOrNL( yyIn, yyPos );
		    }
		} else {
		    BCONSUME( "sidebar", outlined );
		    enterState( InSidebarHeading, command );
		    processSidebarBegin();
		}
		break;
	    case HASH( 's', 8 ):
		CONSUME( "skipline" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipline( substr, location() );
		break;
	    case HASH( 's', 9 ):
		CONSUME( "skipuntil" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipuntil( substr, location() );
		break;
	    case HASH( 't', 5 ):
		if ( command[1] == QChar('a') ) {
		    BCONSUME( "table", outlined );
		} else {
		    BCONSUME( "title", outlined );
		    processTitle( fixBackslashes(
			    getRestOfParagraph(yyIn, yyPos)) );
		}
		break;
	    case HASH( 't', 6 ):
		BCONSUME( "target", outlined );
		processTarget( getWord(yyIn, yyPos) );
		break;
	    case HASH( 't', 8 ):
		BCONSUME( "theindex", outlined );
		processTheIndex();
		break;
	    case HASH( 't', 15 ):
		BCONSUME( "tableofcontents", outlined );
		processTableOfContents();
		break;
	    default:
		if ( isupper(command[0].unicode()) ) {
		    BCONSUME( command, inlined );

		    int n = ( command == command.upper() ) ? 0 : 1;
		    if ( yyPos < yyLen &&
			 isdigit((uchar) yyIn[yyPos].latin1()) ) {
			n = ch.digitValue();
			yyPos++;
		    }

		    QStringList args;
		    for ( int i = 0; i < n; i++ )
			args.push_back( getArgument(yyIn, yyPos) );
		    processAlias( command, args );
		}
	    }

	    if ( !consumed )
		warning( 1, location(), "No such command '\\%s'",
			 command.latin1() );
	} else if ( !ch.isSpace() || inParagraph || !isParagraphState(state) ) {
	    if ( !ch.isSpace() )
		inlined();

	    if ( ch == '&' || ch == '<' || ch == '>' )
		warning( 1, location(), "Unescaped '%c'", ch.latin1() );

	    if ( ch.isSpace() ) {
		if ( spacePolicy == Keeping )
		    spacePolicy = Pending;
	    } else {
		if ( spacePolicy == Pending )
		    processChar( ' ' );
		spacePolicy = Keeping;
		processChar( ch );
	    }

	    if ( ch == '\n' ) {
		bool blankLine = FALSE;

		while ( yyPos < yyLen && yyIn[yyPos].isSpace() ) {
		    ch = yyIn[yyPos++];
		    if ( ch == '\n' )
			blankLine = TRUE;
		}

		if ( blankLine ) {
		    switch ( state ) {
		    case Normal:
			break;
		    case InCaption:
			processCaptionEnd();
			state = Normal;
			spacePolicy = Keeping; // ### Skipping2
			break;
		    case InFootnote:
			break;
		    case InPrint:
			if ( yyPos == yyLen ) {
			    if ( supportsCodeHtml() ) {
				processCodeHtml( pendingPrint );
			    } else {
				processCode( unhtmlize(pendingPrint) );
			    }
			    state = Normal;
			    spacePolicy = Keeping; // ### Skipping2
			}
			break;
		    case InQuote:
			break;
		    case InSectionHeading:
			processSectionHeadingEnd( sectionLevel,
						  topSectionLevel );
			prevSectionLevel = sectionLevel;
			state = Normal;
			spacePolicy = Keeping; // ### Skipping2
			break;
		    case InSidebarHeading:
			processSidebarHeadingEnd();
			inSidebar = TRUE;
			state = Normal;
			spacePolicy = Keeping; // ### Skipping2
		    }

		    outlined();
		}
	    }
	}
    }

    if ( topSectionLevel != -1 )
	endSections( prevSectionLevel, sectionLevel, topSectionLevel );
    if ( inSidebar )
	processSidebarEnd();
}

void Processor::processC( const QString& text )
{
    processString( text );
}

void Processor::processE( const QString& text )
{
    processString( text );
}

void Processor::processLink( const QString& name, const QString& text )
{
    processString( text.isEmpty() ? name : text );
}

void Processor::processString( const QString& str )
{
    for ( int i = 0; i < (int) str.length(); i++ )
	processChar( str[i] );
}

const Location& Processor::location()
{
    while ( locPos < yyPos )
	loc.advance( yyIn[locPos++].latin1() );
    return loc;
}

void Processor::input( const QString& filePath )
{
    QString t = fileContents( filePath, QString("book") ) + QString( "\n\n" );
    t.prepend( QString("\\location {%1}").arg(Location(filePath).toString()) );
    t.append( QString("\\location {%1}").arg(location().toString()) );

    yyIn.insert( yyPos, t );
    yyLen = yyIn.length();
}

QString Processor::getFilePath( const QStringList& dirList,
				const QString& command )
{
    QString fileName = getWord( yyIn, yyPos );
    if ( fileName.isEmpty() ) {
	warning( 1, location(),
		 "Expected file name after '\\%s'", command.latin1() );
    } else {
	QString filePath = config->findDepth( fileName, dirList );
	if ( filePath.isEmpty() ) {
	    warning( 1, location(),
		     "Cannot find file '%s' in BOOKDIRS",
		     fileName.latin1() );
	} else {
	    return filePath;
	}
    }
    return QString::null;
}

QString Processor::fileContents( const QString& filePath,
				 const QString& fileKind )
{
    QFile f( filePath );
    if ( !f.open(IO_ReadOnly) ) {
	warning( 0, location(), "Cannot open %s file '%s'", fileKind.latin1(),
		 filePath.latin1() );
	return QString::null;
    }

    QTextStream t( &f );
    QString text = t.read();
    f.close();
    return text;
}

void Processor::endSections( int prevLevel, int newLevel, int topLevel )
{
    int i = prevLevel;
    while ( i >= newLevel ) {
	processSectionEnd( i, topLevel );
	i--;
    }
}

void Processor::enterState( State newState, const QString& command )
{
    if ( state == InPrint ) {
	processCodeHtml( pendingPrint );
	state = Normal;
    }

    if ( state != Normal )
	warning( 2, location(), "Unexpected '\\%s'", command.latin1() );

    state = newState;
}

void Processor::maybeInlined()
{
    QRegExp caption( QString("^[^\n]*\n[ \t]*\\\\caption") );
    QRegExp blankLine( QString("^[^\n]*\n[ \t]*\n") );

    if ( yyIn.mid(yyPos).find(caption) == -1 &&
	 (inParagraph || yyIn.mid(yyPos).find(blankLine) == -1) ) {
	inlined();
    } else {
	outlined();
    }
}

void Processor::inprint()
{
    if ( inParagraph ) {
	processParagraphEnd();
	inParagraph = FALSE;
    }

    if ( state != InPrint ) {
	if ( state != Normal )
	    warning( 2, location(), "Unexpected quote command" );
	state = InPrint;
	pendingPrint = QString::null;
    }
}

void Processor::inlined()
{
    if ( spacePolicy == Pending ) {
	processChar( ' ' );
	spacePolicy = Keeping;
    }

    if ( state == InPrint ) {
	processCodeHtml( pendingPrint );
	state = Normal;
    }

    if ( !inParagraph && isParagraphState(state) ) {
	processParagraphBegin();
	inParagraph = TRUE;
    }
}

void Processor::outlined()
{
    if ( inParagraph ) {
	processParagraphEnd();
	inParagraph = FALSE;
	spacePolicy = Keeping; // ### Skipping
	skipSpaces( yyIn, yyPos );
    }

    if ( state == InPrint ) {
	processCodeHtml( pendingPrint );
	state = Normal;
    }

    if ( !isParagraphState(state) )
	warning( 2, location(), "Missing '\\end...' or blank line" );
}

class Analyzer : public Processor
{
public:
    Analyzer( const QString& filePath );

    const QValueList<Section>& tableOfContents() const { return toc; }
    const QString& title() const { return ttl; }
    int topSectionLevel() const { return topLev; }
    int granularity() const;
    Section *resolveSection( const QString& heading ) const;
    QString fileSuffixForTarget( const QString& target ) const;

protected:
    virtual void processChar( QChar ch );
    virtual void processGranularity( int level );
    virtual void processSectionBegin( int level, int topLevel );
    virtual void processSectionHeadingEnd( int level, int topLevel );
    virtual void processString( const QString& str );
    virtual void processTarget( const QString& target );
    virtual void processTitle( const QString& title );

private:
    QValueList<Section> toc;
    SectionNumber sectionCounter;
    QString ttl;
    QMap<QString, QString> targetMap;
    bool inSectionHeading;
    QString accum;
    int topLev;
    int granul;
};

Analyzer::Analyzer( const QString& filePath )
    : Processor( filePath ), inSectionHeading( FALSE ), topLev( -1 ),
      granul( -1 )
{
}

int Analyzer::granularity() const
{
    return config->friendly() ? 0 : granul;
}

Section *Analyzer::resolveSection( const QString& heading ) const
{
    QValueList<Section *> candidates =
	    recursiveSectionResolve( (QValueList<Section> *) &toc,
				     QStringList::split(QString("::"),
							heading) );
    return candidates.count() == 1 ? candidates.front() : 0;
}

// careful: we distinguish "" from QString::null
QString Analyzer::fileSuffixForTarget( const QString& target ) const
{
    QMap<QString, QString>::ConstIterator s = targetMap.find( target );
    if ( s == targetMap.end() ) {
	return QString::null;
    } else {
	return (*s).isEmpty() ? QString( "" ) : *s;
    }
}

void Analyzer::processChar( QChar ch )
{
    if ( inSectionHeading )
	accum += ch;
}

void Analyzer::processGranularity( int level )
{
    if ( !toc.isEmpty() )
	::warning( 2, location(),
		   "Granularity must be specified at beginning of file" );
    granul = level;
}

void Analyzer::processSectionBegin( int level, int topLevel )
{
    inSectionHeading = TRUE;
    topLev = topLevel;
    sectionCounter.advance( level - topLevel );
}

void Analyzer::processSectionHeadingEnd( int level, int topLevel )
{
    QValueList<Section> *subsects = &toc;
    for ( int i = topLevel; i < level; i++ ) {
	if ( subsects->empty() )
	    subsects->push_back( Section() );
	subsects = subsects->last().subsections();
    }

    Section sect;
    sect.title = accum;

    subsects->push_back( sect );
    subsects->last().number = sectionCounter;

    inSectionHeading = FALSE;
    accum = QString::null;
}

void Analyzer::processString( const QString& str )
{
    if ( inSectionHeading )
	accum += str;
}

void Analyzer::processTarget( const QString& target )
{
    int n = granularity() - topSectionLevel();
    targetMap.insert( target, sectionCounter.fileSuffix(n) );
}

void Analyzer::processTitle( const QString& title )
{
    ttl = title;
}

class Synthetizer : public Processor
{
public:
    Synthetizer( const QString& filePath, const Analyzer *analyzer,
		 const Resolver *resolver = 0 );

protected:
    virtual void processTableOfContents();

    virtual void emitTocListBegin( int /* level */ ) { }
    virtual void emitTocListItem( int /* level */,
				  const Section& /* sect */ ) { }
    virtual void emitTocListEnd( int /* level */ ) { }

    const Analyzer *analyzer() const { return a; }
    const QString& outFileBase() const { return fb; }

private:
    void emitTocSections( const QValueList<Section>& sections, int level = 0 );

    const Analyzer *a;
    QString fb;
};

Synthetizer::Synthetizer( const QString& filePath, const Analyzer *analyzer,
			  const Resolver *resolver )
    : Processor( filePath, resolver ), a( analyzer )
{
    fb = filePath.mid( filePath.findRev(QChar('/')) + 1 );
    int k = fb.findRev( QChar('.') );
    if ( k != -1 )
	fb.truncate( k );
}

void Synthetizer::processTableOfContents()
{
    emitTocSections( analyzer()->tableOfContents() );
}

void Synthetizer::emitTocSections( const QValueList<Section>& sections,
				   int level )
{
    if ( !sections.isEmpty() ) {
	emitTocListBegin( level );

	QValueList<Section>::ConstIterator s = sections.begin();
	while ( s != sections.end() ) {
	    emitTocListItem( level, *s );
	    emitTocSections( *(*s).subsections(), level + 1 );
	    ++s;
	}
	emitTocListEnd( level );
    }
}

class HtmlSynthetizer : public Synthetizer
{
public:
    HtmlSynthetizer( const QString& filePath, const Analyzer *analyzer,
		     const Resolver *resolver );

protected:
    virtual bool supportsCodeHtml() const { return TRUE; }

    virtual void processAlias( const QString& alias, const QStringList& args );
    virtual void processC( const QString& text );
    virtual void processCaptionBegin();
    virtual void processCaptionEnd();
    virtual void processChar( QChar ch );
    virtual void processCode( const QString& text );
    virtual void processCodeHtml( const QString& text );
    virtual void processE( const QString& text );
    virtual void processFootnoteBegin();
    virtual void processFootnoteEnd();
    virtual void processImg( const QString& fileName, const QString& alt,
			     bool inParagraph );
    virtual void processIndex( const QString& text );
    virtual void processLink( const QString& name, const QString& text );
    virtual void processListBegin( OpenedList *ol );
    virtual void processListItem( OpenedList *ol );
    virtual void processListEnd( OpenedList *ol );
    virtual void processParagraphBegin();
    virtual void processParagraphEnd();
    virtual void processQuoteBegin();
    virtual void processQuoteEnd();
    virtual void processSectionBegin( int level, int topLevel );
    virtual void processSectionHeadingEnd( int level, int topLevel );
    virtual void processSectionEnd( int level, int topLevel );
    virtual void processSidebarBegin();
    virtual void processSidebarHeadingEnd();
    virtual void processSidebarEnd();
    virtual void processString( const QString& str );
    virtual void processTarget( const QString& target );

    virtual void emitTocListBegin( int level );
    virtual void emitTocListItem( int level, const Section& sect );
    virtual void emitTocListEnd( int level );

private:
    QString targetForSectionNumber( const SectionNumber& number ) const;
    void fillHtmlPageSequence( const QValueList<Section> *sects, int level );
    QString makeBanner();

    QValueStack<QString> banners;
    QPtrStack<HtmlWriter> w;
    SectionNumber sectionCounter;
    QValueList<const Section *> htmlPageSequence;
    int delta;
};

HtmlSynthetizer::HtmlSynthetizer( const QString& filePath,
				  const Analyzer *analyzer,
				  const Resolver *resolver )
    : Synthetizer( filePath, analyzer, resolver )
{
    w.setAutoDelete( TRUE );
    w.push( new HtmlWriter(location(), outFileBase() + QString(".html")) );
    w.top()->setTitle( analyzer->title() );
    w.top()->setHeading( analyzer->title() );
    if ( config->friendly() )
	w.top()->putsMeta( "<!-- friendly -->\n" );

    /*
      Construct a list of all HTML pages in sequence, with two
      sentinels.
    */
    htmlPageSequence.append( (Section *) 0 );
    fillHtmlPageSequence( &analyzer->tableOfContents(), 0 );
    htmlPageSequence.append( (Section *) 0 );

    delta = 2;
    if ( config->friendly() ) {
	/*
	  Help qdoc2latex by using '<h1>' rather than '<h2>' for
	  section1.
	*/
	delta = 1;
    }
}

void HtmlSynthetizer::processAlias( const QString& alias,
				    const QStringList& args )
{
    w.top()->putsMeta( config->unalias(location(), alias, QString("html"),
				       args) );
}

void HtmlSynthetizer::processC( const QString& text )
{
    w.top()->putsMeta( "<tt>" );
    w.top()->puts( text.latin1() );
    w.top()->putsMeta( "</tt>" );
}

void HtmlSynthetizer::processCaptionBegin()
{
    // see also doc.cpp
    w.top()->putsMeta( "<blockquote><p align=\"center\"><em>" );
}

void HtmlSynthetizer::processCaptionEnd()
{
    w.top()->putsMeta( "</em></p></blockquote>\n" );
}

void HtmlSynthetizer::processChar( QChar ch )
{
    w.top()->putchar( ch.latin1() );
}

void HtmlSynthetizer::processCode( const QString& text )
{
    w.top()->putsMeta( "<pre>" );
    w.top()->puts( text.latin1() );
    w.top()->putsMeta( "</pre>\n" );
}

void HtmlSynthetizer::processCodeHtml( const QString& text )
{
    w.top()->putsMeta( "<pre>" );
    w.top()->putsMeta( text.latin1() );
    w.top()->putsMeta( "</pre>\n" );
}

void HtmlSynthetizer::processE( const QString& text )
{
    w.top()->putsMeta( "<em>" );
    w.top()->puts( text.latin1() );
    w.top()->putsMeta( "</em>" );
}

void HtmlSynthetizer::processFootnoteBegin()
{
}

void HtmlSynthetizer::processFootnoteEnd()
{
}

void HtmlSynthetizer::processImg( const QString& fileName, const QString& alt,
				  bool inParagraph )
{
    int width;
    int height;

    if ( !inParagraph )
	w.top()->putsMeta( "<p align=\"center\">" );
    w.top()->putsMeta( "<img align=\"middle\" src=\"" );
    w.top()->puts( fileName.latin1() );
    w.top()->putsMeta( "\"" );
    if ( !alt.isEmpty() ) {
	w.top()->putsMeta( " alt=\"" );
	w.top()->putsMeta( alt.latin1() );
	w.top()->putsMeta( "\"" );
    }

    if ( config->needImage(location(), fileName, &width, &height) )
	w.top()->printfMeta( " width=\"%d\" height=\"%d\"", width, height );

    w.top()->putsMeta( ">\n" );
    if ( !inParagraph )
	w.top()->putsMeta( "</p>\n" );
}

void HtmlSynthetizer::processIndex( const QString& text )
{
    w.top()->printfMeta( "<!-- index %s -->", text.latin1() );
}

void HtmlSynthetizer::processLink( const QString& name,
				   const QString& text )
{
    // see also doc.cpp
    static QRegExp allProtos( QString("(?:f(?:ile|tp)|http|mailto):.*") );

    QString textx = text;

    QString y = resolver()->href( name, text );
    if ( textx.isEmpty() )
	textx = name;

    if ( y.length() == textx.length() ) {
	QString namex;

	if ( name.startsWith(QChar('#')) ) {
	    namex = analyzer()->fileSuffixForTarget( name.mid(1) );
	    if ( !namex.isNull() ) {
		namex.prepend( outFileBase() );
		namex.append( QString(".html#") + name );
	    }
	} else {
	    Section *s = analyzer()->resolveSection( name );
	    if ( s == 0 ) {
		if ( allProtos.exactMatch(name) )
		    namex = name;
	    } else {
		namex = targetForSectionNumber( s->number );
		if ( text.isEmpty() )
		    textx = s->title;
	    }
	}

	if ( !name.isEmpty() && namex.isEmpty() )
	    ::warning( 2, location(), "Unresolved link to '%s'",
		       name.latin1() );

	if ( !namex.isEmpty() )
    	    w.top()->printfMeta( "<a href=\"%s\">", namex.latin1() );
	if ( !textx.isEmpty() )
	    w.top()->puts( textx.latin1() );
	if ( !namex.isEmpty() )
	    w.top()->putsMeta( "</a>" );
    } else {
	w.top()->putsMeta( y.latin1() );
    }
}

void HtmlSynthetizer::processListBegin( OpenedList *ol )
{
    w.top()->putsMeta( ol->beginHtml().latin1() );
}

void HtmlSynthetizer::processListItem( OpenedList *ol )
{
    w.top()->putsMeta( ol->itemHtml().latin1() );
}

void HtmlSynthetizer::processListEnd( OpenedList *ol )
{
    w.top()->putsMeta( ol->endHtml().latin1() );
}

void HtmlSynthetizer::processParagraphBegin()
{
    w.top()->putsMeta( "<p>" );
}

void HtmlSynthetizer::processParagraphEnd()
{
    w.top()->putsMeta( "</p>\n" );
}

void HtmlSynthetizer::processQuoteBegin()
{
    w.top()->putsMeta( "<blockquote>\n" );
}

void HtmlSynthetizer::processQuoteEnd()
{
    w.top()->putsMeta( "</blockquote>\n" );
}

void HtmlSynthetizer::processSectionBegin( int level, int topLevel )
{
    w.top()->printfMeta( "<h%d>", level - topLevel + delta );
    sectionCounter.advance( level - topLevel );

    if ( level <= analyzer()->granularity() ) {
	int n = analyzer()->granularity() - topLevel;

	w.top()->putsMeta( "<a href=\"" );
	w.top()->puts( outFileBase().latin1() );
	w.top()->puts( sectionCounter.fileSuffix(n).latin1() );
	w.top()->putsMeta( ".html\">" );
	w.top()->startRecording();
    } else {
	int n = analyzer()->granularity() - topLevel;
	processTarget( sectionCounter.target(n) );
    }
}

void HtmlSynthetizer::processSectionHeadingEnd( int level, int topLevel )
{
    QString heading;
    int n = analyzer()->granularity() - topLevel;

    if ( level <= analyzer()->granularity() ) {
	heading = w.top()->endRecording();
	w.top()->putsMeta( "</a>" );
    }

    w.top()->printfMeta( "</h%d>\n", level - topLevel + delta );

    if ( level <= analyzer()->granularity() ) {
	banners.push( makeBanner() );
	w.push( new HtmlWriter(location(),
			       outFileBase() + sectionCounter.fileSuffix(n) +
			       QString(".html")) );
	w.top()->setTitle( heading.latin1() );
	w.top()->putsMeta( banners.top().latin1() );
	w.top()->printfMeta( "<h%d align=\"center\">",
			     level - topLevel + delta );
	w.top()->putsMeta( heading.latin1() );
	w.top()->printfMeta( "</h%d>\n", level - topLevel + delta );
    }
}

void HtmlSynthetizer::processSectionEnd( int level, int /* topLevel */ )
{
    if ( level <= analyzer()->granularity() ) {
	w.top()->enterFooter();
	w.top()->putsMeta( banners.pop().latin1() );
	delete w.pop();
    }
}

void HtmlSynthetizer::processSidebarBegin()
{
    w.top()->putsMeta( "<blockquote>\n<p align=\"center\"><b>" );
}

void HtmlSynthetizer::processSidebarHeadingEnd()
{
    w.top()->putsMeta( "</b></p>\n" );
}

void HtmlSynthetizer::processSidebarEnd()
{
    w.top()->putsMeta( "</blockquote>\n" );
}

void HtmlSynthetizer::processString( const QString& str )
{
    w.top()->puts( str.latin1() );
}

void HtmlSynthetizer::processTarget( const QString& target )
{
    w.top()->printfMeta( "<a name=\"%s\"></a>", target.latin1() );
}

void HtmlSynthetizer::emitTocListBegin( int level )
{
    if ( level == 0 )
	w.top()->putsMeta( "<!-- toc -->\n" );
    w.top()->putsMeta( "<ul>\n" );
}

void HtmlSynthetizer::emitTocListItem( int /* level */, const Section& sect )
{
    w.top()->printfMeta( "<li><a href=\"%s\">",
			 targetForSectionNumber(sect.number).latin1() );
    w.top()->puts( sect.title.latin1() );
    w.top()->putsMeta( "</a>\n" );
}

void HtmlSynthetizer::emitTocListEnd( int level )
{
    w.top()->putsMeta( "</ul>\n" );
    if ( level == 0 )
	w.top()->putsMeta( "<!-- endtoc -->\n" );
}

QString HtmlSynthetizer::targetForSectionNumber(
	const SectionNumber& number ) const
{
    int n = analyzer()->granularity() - analyzer()->topSectionLevel();
    QString t = outFileBase() + number.fileSuffix( n ) + QString( ".html" );
    if ( n < (int) number.number.count() - 1 )
	t += QChar( '#' ) + number.target( n );
    return t;
}

void HtmlSynthetizer::fillHtmlPageSequence( const QValueList<Section> *sects,
					    int level )
{
    if ( level <= analyzer()->granularity() ) {
	QValueList<Section>::ConstIterator s = sects->begin();
	while ( s != sects->end() ) {
	    htmlPageSequence.append( &*s );
	    fillHtmlPageSequence( (*s).subsections(), level + 1 );
	    ++s;
	}
    }
}

QString HtmlSynthetizer::makeBanner()
{
    const Section *prev = htmlPageSequence[0];
    const Section *next = htmlPageSequence[2];
    QString t;

    if ( prev != 0 || next != 0 ) {
	t += QString( "<p align=\"right\">" );
	if ( prev != 0 ) {
	    t += QString( "[<a href=\"%1\">Prev: " )
		 .arg( targetForSectionNumber(prev->number) );
	    t += htmlProtect( prev->title );
	    t += QString( "</a>] " );
	}
	t += QString( "[<a href=\"%1.html\">Home</a>]" )
	     .arg( outFileBase().latin1() );
	if ( next != 0 ) {
	    t += QString( " [<a href=\"%1\">Next: " )
		 .arg( targetForSectionNumber(next->number) );
	    t += htmlProtect( next->title );
	    t += QString( "</a>]" );
	}
	t += QString( "</p>\n" );
    }

    htmlPageSequence.remove( htmlPageSequence.begin() );
    return t;
}

class SgmlSynthetizer : public Synthetizer
{
public:
    SgmlSynthetizer( const QString& filePath, const Analyzer *analyzer );

protected:
    virtual void processAlias( const QString& alias, const QStringList& args );
    virtual void processC( const QString& text );
    virtual void processCaptionBegin();
    virtual void processCaptionEnd();
    virtual void processChar( QChar ch );
    virtual void processCode( const QString& text );
    virtual void processE( const QString& text );
    virtual void processFootnoteBegin();
    virtual void processFootnoteEnd();
    virtual void processImg( const QString& fileName, const QString& alt,
			     bool inParagraph );
    virtual void processIndex( const QString& text );
    virtual void processLink( const QString& name, const QString& text );
    virtual void processListBegin( OpenedList *ol );
    virtual void processListItem( OpenedList *ol );
    virtual void processListEnd( OpenedList *ol );
    virtual void processParagraphBegin();
    virtual void processParagraphEnd();
    virtual void processQuoteBegin();
    virtual void processQuoteEnd();
    virtual void processSectionBegin( int level, int topLevel );
    virtual void processSectionHeadingEnd( int level, int topLevel );
    virtual void processSectionEnd( int level, int topLevel );
    virtual void processSidebarBegin();
    virtual void processSidebarHeadingEnd();
    virtual void processSidebarEnd();
    virtual void processString( const QString& str );
    virtual void processTarget( const QString& target );

    virtual void emitTocListBegin( int level );
    virtual void emitTocListItem( int level, const Section& sect );
    virtual void emitTocListEnd( int level );

private:
    BinaryWriter w;
};

SgmlSynthetizer::SgmlSynthetizer( const QString& filePath,
				  const Analyzer *analyzer )
    : Synthetizer( filePath, analyzer ), w( outFileBase() + QString(".sgml") )
{
}

void SgmlSynthetizer::processAlias( const QString& alias,
				    const QStringList& args )
{
    w.puts( config->unalias(location(), alias, QString("sgml"), args)
	    .latin1() );
}

void SgmlSynthetizer::processC( const QString& text )
{
    w.puts( "<literal>" );
    w.puts( sgmlProtect(text).latin1() );
    w.puts( "</literal>" );
}

void SgmlSynthetizer::processCaptionBegin()
{
    // ###
}

void SgmlSynthetizer::processCaptionEnd()
{
    // ###
}

void SgmlSynthetizer::processChar( QChar ch )
{
    w.puts( sgmlProtect(ch).latin1() );
}

void SgmlSynthetizer::processCode( const QString& text )
{
    w.puts( "<programlisting>" );
    w.puts( sgmlProtect(text).latin1() );
    w.puts( "</programlisting>\n" );
}

void SgmlSynthetizer::processE( const QString& text )
{
    w.puts( "<emphasis>" );
    w.puts( sgmlProtect(text).latin1() );
    w.puts( "</emphasis>\n" );
}

void SgmlSynthetizer::processFootnoteBegin()
{
}

void SgmlSynthetizer::processFootnoteEnd()
{
}

void SgmlSynthetizer::processImg( const QString& /* fileName */, const QString& /* alt */,
				  bool /* inParagraph */ )
{
}

void SgmlSynthetizer::processIndex( const QString& /* text */ )
{
}

void SgmlSynthetizer::processLink( const QString& /* name */, const QString& /* text */ )
{
}

void SgmlSynthetizer::processListBegin( OpenedList *ol )
{
    w.puts( ol->beginSgml().latin1() );
}

void SgmlSynthetizer::processListItem( OpenedList *ol )
{
    w.puts( ol->itemSgml().latin1() );
}

void SgmlSynthetizer::processListEnd( OpenedList *ol )
{
    w.puts( ol->endSgml().latin1() );
}

void SgmlSynthetizer::processParagraphBegin()
{
    w.puts( "<para>" );
}

void SgmlSynthetizer::processParagraphEnd()
{
    w.puts( "</para>\n" );
}

void SgmlSynthetizer::processQuoteBegin()
{
}

void SgmlSynthetizer::processQuoteEnd()
{
}

void SgmlSynthetizer::processSectionBegin( int level, int /* topLevel */ )
{
    w.puts( QString("<sect%1><title>").arg(level).latin1() );
}

void SgmlSynthetizer::processSectionHeadingEnd( int /* level */,
						int /* topLevel */ )
{
    w.puts( "</title>\n" );
}

void SgmlSynthetizer::processSectionEnd( int level, int /* topLevel */ )
{
    w.puts( QString("</sect%1>\n").arg(level).latin1() );
}

void SgmlSynthetizer::processSidebarBegin()
{
    w.puts( "<sidebar><title>" );
}

void SgmlSynthetizer::processSidebarHeadingEnd()
{
    w.puts( "</title>\n" );
}

void SgmlSynthetizer::processSidebarEnd()
{
    w.puts( "</sidebar>\n" );
}

void SgmlSynthetizer::processString( const QString& str )
{
    w.puts( sgmlProtect(str).latin1() );
}

void SgmlSynthetizer::processTarget( const QString& /* target */ )
{
}

void SgmlSynthetizer::emitTocListBegin( int /* level */ )
{
}

void SgmlSynthetizer::emitTocListItem( int /* level */,
				       const Section& /* sect */ )
{
}

void SgmlSynthetizer::emitTocListEnd( int /* level */ )
{
}

void parseBookFile( const QString& filePath, int fmt, const Resolver *resolver )
{
    Analyzer *analyzer = new Analyzer( filePath );

    for ( int pass = 0; pass < 3; pass++ ) {
	if ( pass == 0 ) {
	    analyzer->start( TRUE );
	} else {
	    Processor *p = 0;

	    switch ( pass ) {
	    case 1:
		if ( (fmt & Html) != 0 )
		    p = new HtmlSynthetizer( filePath, analyzer, resolver );
		break;
	    case 2:
#if 0
		if ( (fmt & Sgml) != 0 )
		    p = new SgmlSynthetizer( filePath, analyzer );
#else
		;
#endif
	    }

	    if ( p != 0 ) {
		p->start( FALSE );
		delete p;
	    }
	}
    }
}
