/*
  doc.cpp
*/

#include <qregexp.h>
#include <qstringlist.h>

#include <ctype.h>
#include <limits.h>

#include "binarywriter.h"
#include "codeprocessor.h"
#include "config.h"
#include "doc.h"
#include "english.h"
#include "html.h"
#include "htmlwriter.h"
#include "messages.h"
#include "resolver.h"
#include "stringset.h"

static QString parenParen( QString("()") );

// see also parsehelpers.cpp
static QString punctuation( ".,:;" );

// see also bookparser.cpp
static QString openCaption( "<blockquote><p align=\"center\"><em>" );
static QString closeCaption( "</em></p>\n</blockquote>" );
static QString openSidebar( "<blockquote><p align=\"center\"><b>" );
static QString closeSidebarHeading( "</b>\n<p>" );
static QString closeSidebar( "</blockquote>\n<p>" );

static QString linkBase( const QString& link )
{
    int k = link.find( QChar('#') );
    if ( k == -1 )
	return link;
    else
	return link.left( k );
}

// ### speed up
static bool isCppSym( QChar ch )
{
    return isalnum( (uchar) ch.latin1() ) || ch == QChar( '_' ) ||
	   ch == QChar( ':' );
}

static QString what( Doc::Kind kind )
{
    switch ( kind ) {
    case Doc::Null:
    case Doc::Fn:
	return QString( "function" );
    case Doc::Enum:
	return QString( "type" );
    case Doc::Property:
	return QString( "property" );
    case Doc::Class:
	return QString( "class" );
    default:
	return QString( "documentation" );
    }
}

// ### I hate the word 'sanitize'... I have to get rid of the function real
// soon... 100% bureaucraty.
static void sanitize( QString& str )
{
    if ( str.isNull() )
	str = QString( "" );
}

static QString getEscape( const QString& in, int& pos )
{
    int ch = 0;
    if ( pos < (int) in.length() )
	ch = in[pos].unicode();

    switch ( ch ) {
    case '&':
	pos++;
	return QString( "&amp;" );
    case '<':
	pos++;
	return QString( "&lt;" );
    case '>':
	pos++;
	return QString( "&gt;" );
    case '\\':
	// double backslash becomes &#92; so that pass 2 leaves it alone
	pos++;
	return QString( "&#92;" );
    default:
	return QString( "&#92;" );
    }
}

static QString fixBackslashes( const QString& str )
{
    QString t;
    int i = 0;

    while ( i < (int) str.length() ) {
	QChar ch = str[i++];
	if ( ch == QChar('\\') ) {
	    t += getEscape( str, i );
	} else {
	    t += ch;
	}
    }
    return t;
}

/*
  This function is imperfect. If sophisticated '\keyword's are
  needed, it can always be changed.
*/
static QString keywordRef( const QString& str )
{
    static QRegExp unfriendly( QString("[^a-zA-Z0-9_-]+") );
    QString t = str;
    t.replace( unfriendly, QChar('-') );
    return t;
}

static bool isLongDoc( Doc::Kind k )
{
    return k == Doc::Class || k == Doc::Page || k == Doc::Defgroup;
}

/*
  This function makes sure no two automatic links for the same
  identifier are too close to each other. It returns TRUE if it's OK
  to have a new link to name, otherwise FALSE. It also updates
  offsetMap.

  The criterion is that two automatic links to the same place should
  be separated by at least 1009 characters.
*/
static bool offsetOK( QMap<QString, int> *offsetMap, int off,
		      const QString& name )
{
    QString lowerName = name.lower();

    QMap<QString, int>::Iterator prevOff = offsetMap->find( lowerName );
    bool ok = ( prevOff == offsetMap->end() || *prevOff <= off - 1009 );
    if ( ok )
	offsetMap->insert( lowerName, off );
    return ok;
}

QMap<QString, LinkMap> Doc::includeLinkMaps;
QMap<QString, LinkMap> Doc::walkthroughLinkMaps;
QMap<QString, QMap<int, ExampleLocation> > Doc::megaExampleMap;
StringSet Doc::includedExamples;
StringSet Doc::thruwalkedExamples;

// QMap<example file, example link>
QMap<QString, QString> Doc::includedExampleLinks;
QMap<QString, QString> Doc::thruwalkedExampleLinks;

/*
  The DocParser class is an internal class that implements the first
  pass of doc parsing. (See Doc::finalHtml() for the second pass.)
*/
class DocParser
{
public:
    DocParser() { }

    Doc *parse( const Location& loc, const QString& in );

private:
#if defined(Q_DISABLE_COPY)
    DocParser( const DocParser& );
    DocParser& operator=( const DocParser& );
#endif

    const Location& location();
    void flushWalkthrough( const Walkthrough& walk, StringSet *included,
			   StringSet *thruwalked );
    void setKind( Doc::Kind kind, const QString& command );
    void setKindHasToBe( Doc::Kind kind, const QString& command );

    Doc::Kind kindIs;
    Doc::Kind kindHasToBe;
    QString clueCommand;

    QStringList getStringList();
    bool somethingAheadPreventingNewParagraph();
    bool valueIsDocumented();
    void enterWalkthroughSnippet();
    void leaveWalkthroughSnippet();

    Location yyLoc;
    int yyLocPos;

    QString yyIn;
    int yyPos;
    int yyLen;
    QString yyOut;
    bool yyInWalkthroughSnippet;
};

Doc *DocParser::parse( const Location& loc, const QString& in )
{
    static QRegExp unfriendly( QString("[^0-9A-Z_a-z]+") );
    static Resolver defaultResolver;

    if ( Doc::resolver() == 0 )
	Doc::setResolver( &defaultResolver );

    kindIs = Doc::Null;
    kindHasToBe = Doc::Null;

    yyLoc = loc;
    yyLocPos = 0;

    yyIn = in;
    yyPos = 0;
    yyLen = yyIn.length();
    yyOut.truncate( 0 );
    yyInWalkthroughSnippet = FALSE;

    Walkthrough walk;
    QString substr;
    QString arg, brief;
    QString className;
    QString enumName;
    QString extName;
    QString fileName;
    QString groupName;
    QString moduleName;
    QString propName;

    QString enumPrefix;
    QString title;
    QString prototype;
    QString relates;
    QString value;
    QString alt;
    QString x;

    StringSet included, thruwalked;
    StringSet groups, headers, keywords;
    StringSet documentedParams, documentedValues;
    QStringList seeAlso, important, footnotes;
    bool obsolete = FALSE;
    bool preliminary = FALSE;
    bool mainClass = FALSE;
    int base;
    int briefBegin = -1;
    int briefEnd = 0;
    int legaleseBegin = -1;
    uint legaleseEnd = 0;
    int footnoteBegin = -1;
    bool internal = FALSE;
    bool overloads = FALSE;
    int numBugs = 0;
    bool inCaption = FALSE;
    bool inHeader = FALSE;
    bool inQuote = FALSE;
    bool inSidebar = FALSE;
    bool inSidebarHeading = FALSE;
    bool inTable = FALSE;
    bool inValue = FALSE;
    int numPendingRows = 0;
    bool useRowDarkColor = FALSE;
    int headingBegin = -1;
    bool metNL = FALSE; // never met N.L.
    int begin;
    int end;
    int width;
    int height;
    int k;

    QValueStack<OpenedList> openedLists;
    int prevSectionLevel = 0;
    int sectionLevel = 0;
    QValueList<Section> *toc = 0;
    SectionNumber sectionCounter;

    while ( yyPos < yyLen ) {
	QChar ch = yyIn[yyPos++];
	metNL = ( ch == '\n' );
	if ( metNL )
	    leaveWalkthroughSnippet();

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

	    /*
	      We use poor man's hashing to identify the qdoc commands
	      (e.g., '\a', '\class', '\enum'). These commands are not
	      frequent enough to justify advanced techniques, and it
	      turns out that we can let the C++ compiler do the job
	      for us by means of a mega-switch and simple hashing.

	      If you have to insert a new command to qdoc, here's one
	      of the two places to do it. In the unlikely event that
	      you have a hash collision (that is, two commands start
	      with the same letter and have the same length), handle
	      it like '\endcode' and '\endlink' below.

	      A second pass of processing will take care of the
	      last-minute details. See Doc::finalHtml().
	    */

	    int h = HASH( command[0].unicode(), command.length() );
	    bool consumed = FALSE;

	    switch ( h ) {
	    case HASH( '\0', 1 ):
		CONSUME( "" );
		yyOut += getEscape( yyIn, yyPos );
		break;
	    case HASH( 'a', 1 ):
		CONSUME( "a" );
		arg = fixBackslashes( getArgument(yyIn, yyPos) );
		if ( arg.isEmpty() ) {
		    warning( 2, location(),
			     "Expected variable name after '\\a'" );
		} else {
		    QString toks = arg;
		    toks.replace( unfriendly, QChar(' ') );
		    QStringList tokl = QStringList::split( QChar(' '), toks );
		    while ( !tokl.isEmpty() ) {
			if ( tokl.first()[0].isLetter() )
			    documentedParams.insert( tokl.first() );
			tokl.remove( tokl.begin() );
		    }
		    yyOut += QString( "<em>" );
		    yyOut += arg;
		    yyOut += QString( "</em>" );
		    setKindHasToBe( Doc::Fn, command );
		}
		break;
	    case HASH( 'b', 3 ):
		// see also \value
		CONSUME( "bug" );
		if ( numBugs == 0 ) {
		    leaveWalkthroughSnippet();
		    yyOut += QString( "<p>Bugs and limitations:\n<ul>\n" );
		}
		numBugs++;
		yyOut += QString( "<li>" );
		break;
	    case HASH( 'b', 4 ):
		CONSUME( "base" );
		base = getWord( yyIn, yyPos ).toInt();
		if ( base != 64 )
		    warning( 2, location(), "Base has to be 64" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( fileName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected file name after '\\base64'" );
		} else {
		    yyOut = yyIn.mid( yyPos );
		    yyPos = yyLen;
		    setKind( Doc::Base64, command );
		}
		break;
	    case HASH( 'b', 5 ):
		CONSUME( "brief" );
		skipSpaces( yyIn, yyPos );
		briefBegin = yyOut.length();
		briefEnd = INT_MAX;
		setKindHasToBe( Doc::Class, command );
		break;
	    case HASH( 'c', 4 ):
		CONSUME( "code" );
		begin = yyPos;
		end = yyIn.find( QString("\\endcode"), yyPos );
		if ( end == -1 ) {
		    warning( 2, location(), "Missing '\\endcode'" );
		} else {
		    yyOut += QString( "\\code" );
		    yyOut += yyIn.mid( begin, end - begin );
		    yyOut += QString( "\\endcode " );
		    yyPos = end + 8;
		}
		break;
	    case HASH( 'c', 5 ):
		CONSUME( "class" );
		className = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( className.isEmpty() ) {
		    warning( 2, location(),
			     "Expected class name after '\\class'" );
		    setKindHasToBe( Doc::Class, command );
		} else {
		    setKind( Doc::Class, command );
		}
		prevSectionLevel = 1;
		break;
	    case HASH( 'c', 7 ):
		CONSUME( "caption" );
		if ( inSidebarHeading ) {
		    warning( 2, location(),
			     "Unexpected '\\caption' in sidebar heading" );
		} else {
		    yyOut += openCaption;
		    inCaption = TRUE;
		}
		break;
	    case HASH( 'd', 8 ):
		CONSUME( "defgroup" );
		groupName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( groupName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected group name after '\\defgroup'" );
		    setKindHasToBe( Doc::Defgroup, command );
		} else {
		    setKind( Doc::Defgroup, command );
		}
		break;
	    case HASH( 'e', 1 ):
		CONSUME( "e" );
		arg = fixBackslashes( getArgument(yyIn, yyPos) );
		yyOut += QString( "<em>" );
		yyOut += arg;
		yyOut += QString("</em>" );
		break;
	    case HASH( 'e', 4 ):
		CONSUME( "enum" );
		enumName = getWord( yyIn, yyPos );
		k = enumName.findRev( QString("::") );
		if ( k != -1 )
		    enumPrefix = enumName.left( k + 2 );
		skipRestOfLine( yyIn, yyPos );
		setKind( Doc::Enum, command );
		break;
	    case HASH( 'e', 7 ):
		if ( command[5] == QChar('d') ) {
		    CONSUME( "endcode" );
		    warning( 2, location(), "Missing '\\code'" );
		} else if ( command[5] == QChar('n') ) {
		    CONSUME( "endlink" );
		    // we have found the missing link: Eirik Aavitsland
		    warning( 2, location(), "Missing '\\link'" );
		} else if ( command[5] == QChar('o') ) {
		    CONSUME( "endomit" );
		    warning( 2, location(), "Missing '\\omit'" );
		} else {
		    CONSUME( "endlist" );
		    if ( openedLists.isEmpty() )
			warning( 2, location(), "Missing '\\list'" );
		    else
			yyOut += openedLists.pop().endHtml();
		}
		break;
	    case HASH( 'e', 8 ):
		if ( command[3] == QChar('q') ) {
		    CONSUME( "endquote" );
		    if ( inQuote ) {
			yyOut += QString( "</blockquote>" );
			inQuote = FALSE;
		    } else {
			warning( 2, location(), "Missing '\\quote'" );
		    }
		} else {
		    CONSUME( "endtable" );
		    if ( inTable ) {
			yyOut += QString( "</table></center>" );
			inTable = FALSE;
		    } else {
			warning( 2, location(), "Missing '\\table'" );
		    }
		}
		break;
	    case HASH( 'e', 9 ):
		CONSUME( "extension" );
		extName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( extName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected extension name after '\\extension'" );
		} else {
		    setKindHasToBe( Doc::Class, command );
		}
		break;
	    case HASH( 'e', 10 ):
		CONSUME( "endsidebar" );
		if ( inSidebarHeading ) {
		    warning( 2, location(), "No text in '\\sidebar'" );
		    inSidebarHeading = FALSE;
		}
		if ( inSidebar ) {
		    yyOut += closeSidebar;
		    inSidebar = FALSE;
		} else {
		    warning( 2, location(), "Missing '\\sidebar'" );
		}
		break;
	    case HASH( 'e', 11 ):
		CONSUME( "endfootnote" );
		if ( footnoteBegin == -1 ) {
		    warning( 2, location(), "Missing '\\footnote'" );
		} else {
		    footnotes.append( yyOut.mid(footnoteBegin) );
		    yyOut.truncate( footnoteBegin );
		    yyOut += QString( "<a href=\"#footnote%1\"><sup>(%2)"
				      "</sup></a>"
				      "<a name=\"footnote-call%3\"></a> " )
			     .arg( footnotes.count() )
			     .arg( footnotes.count() )
			     .arg( footnotes.count() );
		    footnoteBegin = -1;
		}
		break;
	    case HASH( 'f', 2 ):
		CONSUME( "fn" );
		// see also \overload
		prototype = getPrototype( yyIn, yyPos );
		if ( prototype.isEmpty() ) {
		    warning( 2, location(),
			     "Expected function prototype after '\\fn'" );
		} else {
		    setKind( Doc::Fn, command );
		}
		break;
	    case HASH( 'f', 4 ):
		CONSUME( "file" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );
		// ### use page instead of example
		setKind( Doc::Example, command );
		break;
	    case HASH( 'f', 8 ):
		// see also \legalese
		CONSUME( "footnote" );
		skipSpaces( yyIn, yyPos );
		footnoteBegin = yyOut.length();
		break;
	    case HASH( 'h', 6 ):
		CONSUME( "header" );
		if ( inTable ) {
		    if ( numPendingRows > 0 )
			numPendingRows--;
		    yyOut += QString( "<tr bgcolor=\"#a2c511\">" );
		    inHeader = TRUE;
		} else {
		    warning( 2, location(),
			     "Command '\\header' outside '\\table'" );
		}
		break;
	    case HASH( 'h', 10 ):
		CONSUME( "headerfile" );
		x = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( x.isEmpty() ) {
		    warning( 2, location(),
			     "Expected file name after '\\headerfile'" );
		} else {
		    headers.insert( x );
		    setKindHasToBe( Doc::Class, command );
		}
		break;
	    case HASH( 'i', 1 ):
		CONSUME( "i" );
		if ( openedLists.isEmpty() ) {
		    if ( inTable ) {
			int numCols = 1;
			int numRows = 1;

			skipSpaces( yyIn, yyPos );

			yyOut += QString( "<t%1 valign=\"top\"" )
				 .arg( inHeader ? "h" : "d" );
			if ( yyPos + 1 < (int) yyIn.length() &&
			     (numCols = yyIn[yyPos].digitValue()) > 0 &&
			     (numRows = yyIn[yyPos + 1].digitValue()) > 0 ) {
			    getWord( yyIn, yyPos );
			    yyOut += QString( " colspan=\"%1\"" )
				     .arg( numCols );
			    yyOut += QString( " rowspan=\"%1\"" )
				     .arg( numRows );
			    numPendingRows = QMAX( numPendingRows,
						   numRows - 1 );
			}
			yyOut += QChar( '>' );
		    } else {
			warning( 2, location(),
				 "Met '\\i' outside '\\list' or '\\table'" );
		    }
		} else {
		    yyOut += openedLists.top().itemHtml();
		}
		break;
	    case HASH( 'i', 3 ):
		CONSUME( "img" );
		x = getWord( yyIn, yyPos );
		alt = fixBackslashes( getRestOfLine(yyIn, yyPos) );
		yyOut += QString( "<center><img src=\"%1\"" ).arg( x );
		if ( !alt.isEmpty() )
		    yyOut += QString( " alt=\"%1\"" ).arg( alt );
		if ( config->needImage(location(), x, &width, &height) )
		    yyOut += QString( " width=\"%1\" height=\"%2\"" )
			     .arg( width ).arg( height );
		yyOut += QString( "></center> " );
		break;
	    case HASH( 'i', 5 ):
		CONSUME( "index" );
		yyOut += QString( "<!-- index %1 -->" )
			 .arg( getRestOfLine(yyIn, yyPos) );
		break;
	    case HASH( 'i', 7 ):
		if ( command[2] == QChar('c') ) {
		    CONSUME( "include" );
		    x = getWord( yyIn, yyPos );
		    skipRestOfLine( yyIn, yyPos );
		    flushWalkthrough( walk, &included, &thruwalked );

		    if ( x.isEmpty() ) {
			warning( 2, location(),
				 "Expected file name after '\\include'" );
		    } else {
			walk.includePass1( x, Doc::resolver() );
		    }
		    yyOut += QString( "\\include " ) + x + QChar( '\n' );
		} else {
		    CONSUME( "ingroup" );
		    groupName = getWord( yyIn, yyPos );
		    skipRestOfLine( yyIn, yyPos );

		    if ( groupName.isEmpty() )
			warning( 2, location(),
				 "Expected group name after '\\ingroup'" );
		    else
			groups.insert( groupName );
		}
		break;
	    case HASH( 'i', 8 ):
		CONSUME( "internal" );
		internal = TRUE;
		if ( config->isInternal() &&
		     !yyIn.mid(yyPos).stripWhiteSpace().isEmpty() ) {
		    yyOut += QString( "<b>Internal comment:</b>\n" );
		    skipSpacesOrNL( yyIn, yyPos );
		} else {
		    yyOut += QString( "For internal use only." );
		    yyPos = yyLen;
		}
		break;
	    case HASH( 'i', 9 ):
		CONSUME( "important" );
		important = getStringList();
		setKindHasToBe( Doc::Class, command );
		break;
	    case HASH( 'k', 7 ):
		CONSUME( "keyword" );
		x = getRestOfLine( yyIn, yyPos );
		keywords.insert( x );
		yyOut += QString( "<!-- index %1 -->" ).arg( x );

		/*
		  The <a name="..."> for '\page's and similar docs is
		  put right here, because a page can contain many
		  topics. Otherwise, no new <a name="..."> is
		  created; the link given by setLink() is used.
		*/
		if ( isLongDoc(kindIs) )
		    yyOut += QString( "<a name=\"%1\"></a>" )
			     .arg( keywordRef(x) );
		break;
	    case HASH( 'l', 4 ):
		if ( command[2] == QChar('n') ) {
		    CONSUME( "link" );
		    begin = yyPos;
		    end = yyIn.find( QString("\\endlink"), yyPos );
		    if ( end == -1 ) {
			warning( 2, location(), "Missing '\\endlink'" );
		    } else {
			yyOut += QString( "\\link" );
			yyOut += fixBackslashes( yyIn.mid(begin, end - begin) );
			yyOut += QString( "\\endlink" );
			yyPos = end + 8;
		    }
		} else {
		    CONSUME( "list" );
		    openedLists.push( openList(location(),
				      getWord(yyIn, yyPos)) );
		    yyOut += openedLists.top().beginHtml();
		}
		break;
	    case HASH( 'l', 8 ):
		CONSUME( "legalese" );
		skipSpaces( yyIn, yyPos );
		legaleseBegin = yyOut.length();
		legaleseEnd = INT_MAX;
		break;
	    case HASH( 'm', 6 ):
		CONSUME( "module" );
		moduleName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( moduleName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected module name after '\\module'" );
		} else {
		    setKindHasToBe( Doc::Class, command );
		}
		break;
	    case HASH( 'm', 9 ):
		CONSUME( "mainclass" );
		setKindHasToBe( Doc::Class, command );
		mainClass = TRUE;
		break;
	    case HASH( 'n', 4 ):
		CONSUME( "note" );
		yyOut += QString( "Note:" );
		warning( 2, location(), "No such command '\\%s'", "note" );
		break;
	    case HASH( 'o', 4 ):
		CONSUME( "omit" );
		end = yyIn.find( QString("\\endomit"), yyPos );
		if ( end == -1 ) {
		    yyPos = yyIn.length();
		} else {
		    yyPos = end + 8;
		}
		break;
	    case HASH( 'o', 8 ):
		if ( command[1] == QChar('b') ) {
		    CONSUME( "obsolete" );
		    obsolete = TRUE;
		    yyOut += QString( "<b>This %1 is obsolete.</b> It is"
				      " provided to keep old source working."
				      " We strongly advise against using it"
				      " in new code.\n" )
			     .arg( what(kindIs) );
		    metNL = TRUE;
		} else {
		    CONSUME( "overload" );
		    overloads = TRUE;
		    yyOut += QString( "This is an overloaded member function, "
				      "provided for convenience. It behaves "
				      "essentially like the above function.\n");
		    metNL = TRUE;

		    // see also \fn
		    if ( prototype.isEmpty() ) {
			prototype = getPrototype( yyIn, yyPos );
			if ( !prototype.isEmpty() )
			    setKind( Doc::Fn, command );
		    }
		    setKindHasToBe( Doc::Fn, command );
		}
		break;
	    case HASH( 'p', 4 ):
		CONSUME( "page" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );
		setKind( Doc::Page, command );
		break;
	    case HASH( 'p', 7 ):
		CONSUME( "printto" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.printto( substr, location() );
		enterWalkthroughSnippet();
		yyOut += QString( "\\printto " ) + substr + QChar( '\n' );
		leaveWalkthroughSnippet();
		break;
	    case HASH( 'p', 8 ):
		CONSUME( "property" );
		propName = getWord( yyIn, yyPos );

		if ( propName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected property name after '\\property'" );
		    setKindHasToBe( Doc::Property, command );
		} else {
		    setKind( Doc::Property, command );
		}

		skipSpacesOrNL( yyIn, yyPos );
		while ( yyPos < (int) yyIn.length() &&
			yyIn[yyPos].unicode() == '\n' ) {
		    yyPos++;
		    skipSpacesOrNL( yyIn, yyPos );
		}
		if ( yyIn.mid(yyPos, 6) == QString("\\brief") ) {
		    yyPos += 6;
		    brief = getRestOfParagraph( yyIn, yyPos );
		    if ( !brief.isEmpty() ) {
			brief[0] = brief[0].lower();
			if ( brief.endsWith(QChar('.')) )
			    brief.truncate( brief.length() - 1 );
			if ( brief.endsWith(QString(" or not")) )
			    brief.truncate( brief.length() - 7 );

			yyOut += QString( "<p>This property holds " ) + brief +
				 QString( ".\n<p>" );
		    }
		} else {
		    warning( 2, location(),
			    "Expected '\\brief' after '\\property %s'",
			    propName.latin1() );
		}
		break;
	    case HASH( 'p', 9 ):
		if ( command[1] == QChar('l') ) {
		    CONSUME( "plainpage" );
		    fileName = getWord( yyIn, yyPos );
		    skipRestOfLine( yyIn, yyPos );

		    if ( fileName.isEmpty() ) {
			warning( 2, location(),
				 "Expected file name after '\\plainpage'" );
		    } else {
			yyOut = yyIn.mid( yyPos );
			yyPos = yyIn.length();
			setKind( Doc::Plainpage, command );
		    }
		} else {
		    CONSUME( "printline" );
		    substr = getRestOfLine( yyIn, yyPos );
		    walk.printline( substr, location() );
		    enterWalkthroughSnippet();
		    yyOut += QString( "\\printline " ) + substr + QChar( '\n' );
		    leaveWalkthroughSnippet();
		}
		break;
	    case HASH( 'p', 10 ):
		CONSUME( "printuntil" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.printuntil( substr, location() );
		enterWalkthroughSnippet();
		yyOut += QString( "\\printuntil " ) + substr + QChar( '\n' );
		leaveWalkthroughSnippet();
		break;
	    case HASH( 'p', 11 ):
		CONSUME( "preliminary" );
		preliminary = TRUE;
		if ( kindIs != Doc::Class ) {
		    // classes are taken care of elsewhere
		    yyOut += QString( "<p><b>This %1 is under development and"
				      " is subject to change.</b>\n" )
				.arg( what(kindIs) );
		    metNL = TRUE;
		}
		break;
	    case HASH( 'q', 5 ):
		CONSUME( "quote" );
		if ( inQuote ) {
		    warning( 2, location(), "Cannot nest '\\quote' commands" );
		} else {
		    yyOut += QString( "<blockquote>" );
		    inQuote = TRUE;
		}
		break;
#if 1 // ###
	    case HASH( 'w', 11 ):
		CONSUME( "walkthrough" );
		warning( 2, location(), "Command '%s' has been renamed '%s'",
			 "\\walkthrough", "\\quotefile" );
		command = QString( "quotefile" );
		// fall through
#endif
	    case HASH( 'q', 9 ):
		CONSUME( "quotefile" );
		x = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );
		if ( x != walk.fileName() )
		    flushWalkthrough( walk, &included, &thruwalked );

		if ( x.isEmpty() ) {
		    warning( 2, location(),
			     "Expected file name after '\\quotefile'" );
		} else {
		    walk.startPass1( x, Doc::resolver() );
		}
		yyOut += QString( "\\quotefile " ) + x + QChar( '\n' );
		break;
	    case HASH( 'r', 3 ):
		// see also \header
		CONSUME( "row" );
		if ( inTable ) {
		    if ( numPendingRows > 0 ) {
			numPendingRows--;
		    } else {
			useRowDarkColor = !useRowDarkColor;
		    }
		    inHeader = FALSE;
		    yyOut += QString( "<tr bgcolor=\"%1\">" )
			     .arg( useRowDarkColor ? "#f0f0f0" : "#d0d0d0" );
		} else {
		    warning( 2, location(),
			     "Command '\\row' outside '\\table'" );
		}
		break;
	    case HASH( 'r', 5 ):
		CONSUME( "reimp" );
		yyOut += QString( "Reimplemented for internal reasons; the API"
				  " is not affected." );
		internal = TRUE;
		break;
	    case HASH( 'r', 7 ):
		CONSUME( "relates" );
		relates = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( relates.isEmpty() )
		    warning( 2, location(),
			     "Expected class name after '\\relates'" );
		break;
	    case HASH( 's', 2 ):
		CONSUME( "sa" );
		legaleseEnd = yyOut.length();
		if ( numBugs > 0 ) {
		    yyOut += QString( "</ul>" );
		    numBugs = 0;
		}

		seeAlso = getStringList();
		if ( !seeAlso.isEmpty() )
		    yyOut += QString( "\\sa" );
		break;
	    case HASH( 's', 6 ):
		CONSUME( "skipto" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipto( substr, location() );
		yyOut += QString( "\\skipto " ) + substr + QChar( '\n' );
		break;
	    case HASH( 's', 7 ):
		if ( command[1] == QChar('e') ) {
		    CONSUME( "section" );
		    x = getWord( yyIn, yyPos );
		    if ( x.length() != 1 || x[0].unicode() < '1' ||
			 x[0].unicode() > '4' ) {
			warning( 2, location(),
				 "Expected digit between '1' and '4' after"
				 " '\\section'" );
			x = QChar( '1' );
		    }
		    sectionLevel = x[0].unicode() - '0';
		    if ( sectionLevel - prevSectionLevel > 1 )
			warning( 2, location(),
				 "Unexpected '\\section%d' within"
				 " '\\section%d'",
				 sectionLevel, prevSectionLevel );

		    if ( toc == 0 )
			toc = new QValueList<Section>;
		    QValueList<Section> *subsects = toc;
		    for ( int i = 1; i < sectionLevel; i++ ) {
			if ( subsects->isEmpty() )
			    subsects->push_back( Section() );
			subsects = subsects->last().subsections();
		    }
		    subsects->push_back( Section() );

		    prevSectionLevel = sectionLevel;
		    if ( kindIs == Doc::Class )
			prevSectionLevel++;
		    yyOut += QString( "<h%1>" ).arg( prevSectionLevel + 1 );
		    if ( headingBegin != -1 )
			warning( 2, location(),
				 "Missing blank line after '\\section'" );
		    headingBegin = yyOut.length();
		} else {
		    CONSUME( "sidebar" );
		    if ( inSidebar ) {
			warning( 2, location(),
				 "Cannot nest '\\sidebar' commands" );
		    } else {
			yyOut += openSidebar;
			inSidebar = TRUE;
			inSidebarHeading = TRUE;
		    }
		}
		break;
	    case HASH( 's', 8 ):
		CONSUME( "skipline" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipline( substr, location() );
		yyOut += QString( "\\skipline " ) + substr + QChar( '\n' );
		break;
	    case HASH( 's', 9 ):
		CONSUME( "skipuntil" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipuntil( substr, location() );
		yyOut += QString( "\\skipuntil " ) + substr + QChar( '\n' );
		break;
	    case HASH( 't', 5 ):
		if ( command[1] == QChar('a') ) {
		    CONSUME( "table" );
		    if ( inTable ) {
			warning( 2, location(),
				 "Cannot nest '\\table' commands" );
		    } else {
			yyOut += QString( "<center><table cellpadding=\"4\""
					  " cellspacing=\"2\" border=\"0\">" );
			inTable = TRUE;
		    }
		} else {
		    CONSUME( "title" );
		    if ( inSidebarHeading ) {
			warning( 2, location(),
				 "Unexpected '\\title' in sidebar heading" );
		    } else {
			title = getRestOfParagraph( yyIn, yyPos );
		    }
		}
		break;
	    case HASH( 't', 6 ):
		CONSUME( "target" );
		yyOut += QString( "<a name=\"" );
		yyOut += getWord( yyIn, yyPos );
		yyOut += QString( "\"></a>" );
		break;
	    case HASH( 'v', 5 ):
		// see also \bug
		CONSUME( "value" );
		if ( enumName.isEmpty() ) {
		    warning( 2, location(), "Unexpected command '\\value'" );
		    break;
		}

		if ( !inValue ) {
		    yyOut += QString( "<ul>\n" );
		    inValue = TRUE;
		}
		yyOut += QString( "<li>" );
		value = getWord( yyIn, yyPos );
		k = value.findRev( QString("::") );
		if ( k != -1 ) {
		    if ( value.left(k + 2) == enumPrefix )
			warning( 3, location(),
				 "Needless '%s' in enum '\\value'",
				 enumPrefix.latin1() );
		    else
			warning( 2, location(),
				 "Contradictory '%s' in enum '\\value'",
				 value.left(k + 2).latin1() );
		    value = value.mid( k + 2 );
		}
		yyOut += QString( "<tt>%1</tt>" ).arg( enumPrefix + value );
		if ( valueIsDocumented() )
		    yyOut += QString( " - " );
		documentedValues.insert( value );
		skipSpaces( yyIn, yyPos );

		// skip any needless hyphen
		if ( yyPos < yyLen + 2 && yyIn[yyPos] == QChar('-') &&
		     yyIn[yyPos + 1].isSpace() )
		    yyPos += 2;
		break;
	    case HASH( 'w', 7 ):
		CONSUME( "warning" );
		yyOut += QString( "<b>Warning:</b>" );
	    }
	    if ( !consumed ) {
		yyOut += QString( "\\" );
		yyOut += command;
	    }
	} else if ( ch == '>' && yyOut.endsWith(QString("<pre")) ) {
	    yyOut += '>';
	    while ( yyPos < yyLen ) {
		ch = yyIn[yyPos++];
		if ( ch == '\\' && yyPos < yyLen ) {
		    yyOut += getEscape( yyIn, yyPos );
		} else {
		    yyOut += ch;
		    if ( ch == '>' &&
			 yyIn.mid(yyPos - 6, 6) == QString("</pre>") )
			break;
		}
	    }
	    metNL = TRUE;
	} else {
	    if ( ch == QChar('#') && yyIn.mid(yyPos, 2) == QString("##") )
		warning( 2, location(), "Met '#" "##' in documentation" );
	    yyOut += ch;
	}

	if ( metNL ) {
	    while ( yyPos < yyLen && yyIn[yyPos].isSpace() ) {
		ch = yyIn[yyPos++];
		if ( metNL && ch == QChar('\n') &&
		     !somethingAheadPreventingNewParagraph() ) {
		    if ( inValue ) {
			yyOut += QString( "</ul>" );
			inValue = FALSE;
		    }
		    if ( inCaption ) {
			yyOut += closeCaption;
			inCaption = FALSE;
		    }
		    if ( inSidebarHeading ) {
			yyOut += closeSidebarHeading;
			inSidebarHeading = FALSE;
		    } 
		    if ( headingBegin != -1 ) {
			QValueList<Section> *subsects = toc;
			while ( !subsects->last().subsections()->isEmpty() )
			    subsects = subsects->last().subsections();
			subsects->last().title = yyOut.mid( headingBegin );

			yyOut += QString( "</h%1>\n" )
				 .arg( prevSectionLevel + 1 );
			sectionCounter.advance( prevSectionLevel - 1 );
			subsects->last().number = sectionCounter;
			yyOut += QString( "<a name=\"%1\"></a>" )
				 .arg( subsects->last().number.target() );
			headingBegin = -1;
		    }
		    if ( briefEnd == INT_MAX )
			briefEnd = yyOut.length();
		    yyOut += QString( "<p> " );
		    metNL = FALSE;
		}
	    }
	}
    }

    if ( numBugs > 0 || inValue )
	yyOut += QString( "</ul>" );
    if ( inCaption )
	yyOut += closeCaption;
    if ( inQuote )
    	warning( 2, location(), "Missing '\\endquote'" );
    if ( inTable )
	warning( 2, location(), "Missing '\\endtable'" );
    if ( headingBegin != -1 )
	warning( 2, location(), "Trailing '\\section'" );

    flushWalkthrough( walk, &included, &thruwalked );

    if ( footnotes.count() > 0 ) {
	yyOut += QString( "\n<hr>\n<ol>" );

	QStringList::Iterator f = footnotes.begin();
	int no = 1;
	while ( f != footnotes.end() ) {
	    yyOut += QString( " <li><a name=\"footnote%1\"></a>\n" ).arg( no );
	    yyOut += *f;
	    yyOut += QString( " <a href=\"#footnote-call%1\">Back...</a>" )
		     .arg( no );
	    no++;
	    ++f;
	}
	yyOut += QString( "</hr>" );
    }

    // add a "see also" for the groups
    if ( seeAlso.isEmpty() && !groups.isEmpty() ) {
	legaleseEnd = yyOut.length();
	yyOut += QString( "\\sa" );
    }

    yyOut += QChar( '\n' );

    if ( kindIs == Doc::Null ) {
	if ( kindHasToBe != Doc::Null && kindHasToBe != Doc::Fn )
	    warning( 3, loc, "Unexpected '%s' in doc", clueCommand.latin1() );
	kindIs = Doc::Fn;
    }

    Doc *doc = 0;

    switch ( kindIs ) {
    case Doc::Fn:
	sanitize( prototype );
	sanitize( relates );
	doc = new FnDoc( loc, yyOut, prototype, relates, documentedParams,
			 overloads );
	break;
    case Doc::Class:
	if ( briefBegin == -1 ) {
	    warning( 2, loc, "No '\\brief' in '\\class' doc" );
	} else {
	    brief = yyOut.mid( briefBegin, briefEnd - briefBegin )
			 .stripWhiteSpace();
	}
	sanitize( className );
	sanitize( extName );
	sanitize( moduleName );
	sanitize( brief ); /// ### don't assume !isNull() elsewhere

	if ( !extName.isEmpty() )
	    yyOut.prepend( QString("<p> This class is defined in the"
		    " <b>%1 %2 Extension</b>, which can be found in the"
		    " <tt>qt/extensions</tt> directory. It is not included in"
		    " the main %3 API.\n<p>")
		    .arg(config->product())
		    .arg(extName)
		    .arg(config->product()) );

	doc = new ClassDoc( loc, yyOut, className, brief, moduleName, extName,
			    headers, important, mainClass );
	break;
    case Doc::Enum:
	sanitize( enumName );
	doc = new EnumDoc( loc, yyOut, enumName, documentedValues );
	break;
    case Doc::Property:
	doc = new PropertyDoc( loc, yyOut, propName, brief );
	break;
    case Doc::Page:
	sanitize( fileName );
	doc = new PageDoc( loc, yyOut, fileName, title );
	break;
    case Doc::Base64:
	sanitize( fileName );
	doc = new Base64Doc( loc, yyOut, fileName );
	break;
    case Doc::Plainpage:
	sanitize( fileName );
	doc = new PlainpageDoc( loc, yyOut, fileName );
	break;
    case Doc::Defgroup:
	sanitize( groupName );
	doc = new DefgroupDoc( loc, yyOut, groupName, title );
	break;
    case Doc::Example:
	yyOut += "\\include ";
	yyOut += fileName;
	doc = new ExampleDoc( loc, yyOut, fileName, title );
	break;
    default:
	doc = new Doc( Doc::Null, loc, yyOut );
    }
    doc->setInternal( internal );
    doc->setObsolete( obsolete );
    doc->setPreliminary( preliminary );
    doc->setSeeAlso( seeAlso );
    doc->setKeywords( keywords );
    doc->setGroups( groups );
    doc->setContainsExamples( included, thruwalked );
    doc->setTOC( toc );
    if ( legaleseBegin >= 0 )
	doc->setHtmlLegalese( yyOut.mid(legaleseBegin,
					legaleseEnd - legaleseBegin)
				   .stripWhiteSpace() );
    return doc;
}

const Location& DocParser::location()
{
    while ( yyLocPos < yyPos )
	yyLoc.advance( yyIn[yyLocPos++].latin1() );
    return yyLoc;
}

static const int AbsoluteMaxExamples = 7;

static int xunique = 1;

void DocParser::flushWalkthrough( const Walkthrough& walk, StringSet *included,
				  StringSet *thruwalked )
{
    if ( walk.scoreMap().isEmpty() )
	return;

    bool alreadyIncluded = Doc::includedExamples.contains( walk.fileName() );
    bool alreadyThruwalked =
	    Doc::thruwalkedExamples.contains( walk.fileName() );

    int numIncludes = 0;
    int numWalkthroughs = 0;

    ScoreMap::ConstIterator score = walk.scoreMap().begin();
    while ( score != walk.scoreMap().end() ) {
	// score.key() is qaction.html#setWhatsThis, (*score) is a HighScore
	if ( (*score).inInclude() && alreadyIncluded ) {
	    warning( 2, location(), "Example file '%s' included twice",
		     walk.fileName().latin1() );
	} else if ( (*score).inInclude() || !alreadyThruwalked ) {
	    if ( (*score).inInclude() )
		numIncludes++;
	    else
		numWalkthroughs++;

	    ExampleLocation exloc( walk.fileName(), (*score).inInclude(),
				   (*score).lineNum(), xunique++ );
	    int total = (*score).total();

	    QMap<QString, QMap<int, ExampleLocation> >::Iterator entry =
		    Doc::megaExampleMap.find( score.key() );
	    if ( entry == Doc::megaExampleMap.end() ) {
		Doc::megaExampleMap[score.key()].insert( total, exloc );
	    } else {
		// avoid collisions by decrementing the score gently
		// ### evil because depends on mtime
		while ( (*entry).contains(total) )
		    total--;
		(*entry).insert( total, exloc );
		if ( (int) (*entry).count() > AbsoluteMaxExamples )
		    (*entry).remove( (*entry).begin() );
	    }
	}

	++score;
    }

    if ( !alreadyIncluded && numIncludes > 0 ) {
	included->insert( walk.fileName() );
	Doc::includedExamples.insert( walk.fileName() );
    }
    if ( !alreadyThruwalked && numWalkthroughs > 0 ) {
	thruwalked->insert( walk.fileName() );
	Doc::thruwalkedExamples.insert( walk.fileName() );
    }
}

void DocParser::setKind( Doc::Kind kind, const QString& command )
{
    setKindHasToBe( kind, command );
    if ( kind == kindIs )
	warning( 2, location(), "Command '\\%s' is redundant or contradictory",
		 command.latin1() );
    kindIs = kind;
}

void DocParser::setKindHasToBe( Doc::Kind kind, const QString& command )
{
    if ( kindHasToBe == Doc::Null ) {
	kindHasToBe = kind;
	clueCommand = command;
    } else if ( kindHasToBe != kind ) {
	warning( 3, location(),
		 "Cannot have both '\\%s' and '\\%s' in same doc",
		 clueCommand.latin1(), command.latin1() );
    }
}

QStringList DocParser::getStringList()
{
    static QRegExp ahref( QString("^<a[ \t\n]+href=[^>]*>.*</a>") );
    static QRegExp bracy( QString("^\\{.+\\}") );
    QStringList stringl;

    ahref.setMinimal( TRUE );
    bracy.setMinimal( TRUE );

    skipSpaces( yyIn, yyPos );
    for (;;) {
	int begin = yyPos;
	int end;

	if ( yyIn.mid(yyPos, 5) == QString("\\link") ) {
	    yyPos += 5;
	    end = yyIn.find( QString("\\endlink"), yyPos );
	    if ( end == -1 ) {
		warning( 2, location(), "Missing '\\endlink'" );
		break;
	    } else {
		yyPos = end + 8;
	    }
	} else if ( yyIn[yyPos].unicode() == '<' &&
		    ahref.search(yyIn.mid(yyPos)) != -1 ) {
	    yyPos = begin + ahref.matchedLength();
	    end = yyPos;
	} else if ( yyIn[yyPos].unicode() == '{' &&
		    bracy.search(yyIn.mid(yyPos)) != -1 ) {
	    yyPos = begin + bracy.matchedLength();
	    begin++;
	    end = yyPos - 1;
	} else {
	    while ( yyPos < yyLen && !yyIn[yyPos].isSpace() )
		yyPos++;
	    if ( yyPos == begin )
		break;
	    end = yyPos;
	}
	if ( punctuation.find(yyIn[end - 1]) != -1 ) {
	    end--;
	} else if ( yyPos < yyLen && punctuation.find(yyIn[yyLen]) != -1 ) {
	    yyPos++;
	}

	if ( end > begin )
	    stringl.append( fixBackslashes(yyIn.mid(begin, end - begin)) );
	skipSpacesOrNL( yyIn, yyPos );
    }
    return stringl;
}

bool DocParser::somethingAheadPreventingNewParagraph()
{
    int pos = yyPos;

    skipSpaces( yyIn, pos );
    if ( pos < yyLen - 4 ) {
	QString lookahead = yyIn.mid( pos, 4 );
	if ( lookahead == QString("<li>") || lookahead == QString("\\bug") ||
	     lookahead == QString("\\hea") || lookahead == QString("\\row") ||
	     lookahead == QString("\\val") )
	    return TRUE;
    }
    return FALSE;
}

bool DocParser::valueIsDocumented()
{
    int pos = yyPos;
    int numNLs = 0;

    while ( pos < yyLen && yyIn[pos].isSpace() ) {
	if ( yyIn[pos] == QChar('\n') )
	    numNLs++;
	pos++;
    }
    return pos < yyLen && numNLs < 2 &&
	   yyIn.mid( pos, 6 ) != QString( "\\value" );
}

void DocParser::enterWalkthroughSnippet()
{
    if ( !yyInWalkthroughSnippet ) {
	if ( yyOut.endsWith(QString("</pre>")) ) {
	    yyOut.truncate( yyOut.length() - 6 );
	} else {
	    yyOut += QString( "<pre>" );
	}
	yyInWalkthroughSnippet = TRUE;
    }
}

void DocParser::leaveWalkthroughSnippet()
{
    static QRegExp endOfCommand( QString("[^\\a-z0-9]") );
    static StringSet walkthroughCommands;

    // ### put elsewhere
    if ( walkthroughCommands.isEmpty() ) {
	walkthroughCommands.insert( QString("\\printline") );
	walkthroughCommands.insert( QString("\\printto") );
	walkthroughCommands.insert( QString("\\printuntil") );
	walkthroughCommands.insert( QString("\\skipline") );
	walkthroughCommands.insert( QString("\\skipto") );
	walkthroughCommands.insert( QString("\\skipuntil") );
    }

    if ( yyInWalkthroughSnippet ) { // ### needless if
	int k = yyIn.find( endOfCommand, yyPos );
	QString lookahead = yyIn.mid( yyPos, k );

	if ( !walkthroughCommands.contains(lookahead) ) {
	    yyOut += QString( "</pre>" );
	    yyInWalkthroughSnippet = FALSE;
	}
    }
}

ExampleLocation& ExampleLocation::operator=( const ExampleLocation& el )
{
    fname = el.fname;
    ininc = el.ininc;
    ln = el.ln;
    uniq = el.uniq;
    return *this;
}

const Resolver *Doc::res = 0;
QRegExp *Doc::megaRegExp = 0;
QMap<QString, QMap<QString, QString> > Doc::legaleses;
QMap<QString, QString> Doc::keywordLinks;
StringSet Doc::hflist;
QMap<QString, QString> Doc::clist;
QMap<QString, QString> Doc::mainclist;
QMap<QString, StringSet> Doc::findex;
QMap<QString, QString> Doc::grmap;
QMap<QString, StringSet> Doc::chierarchy;
StringSet Doc::extlist;
QMap<QString, QString> Doc::classext;

Doc *Doc::create( const Location& loc, const QString& text )
{
    static DocParser parser;
    return parser.parse( loc, text );
}

void Doc::setHeaderFileList( const StringSet& headerFiles )
{
    hflist = headerFiles;
}

void Doc::setClassLists( const QMap<QString, QString>& allClasses,
			 const QMap<QString, QString>& mainClasses )
{
    clist = allClasses;
    mainclist = mainClasses;

    /*
      Why is here the best place to build the mega regular expression?
      Donno.
    */
    if ( !config->autoHrefs() )
	return;

    // careful with this mega regexp: white space magic below
    QString t( "(?:<(?:a [^>]*|pre)>.*</(?:a|pre)>|"
	       "<img [^>]*>|"
	       "(?:Qmagicwordthatyoushouldavoid" );
    QMap<QString, QString>::ConstIterator s = keywordLinks.begin();
    while ( s != keywordLinks.end() ) {
	t += QChar( '|' );
	t += s.key();
	++s;
    }

    // white space magic
    t.replace( QRegExp(QChar(' ')), QString("[ \t\n]+") );
    t.prepend( QString("[ \t\n>]") );
    t.append( QString(")\\b)") );

    delete megaRegExp;
    megaRegExp = new QRegExp( t );
    megaRegExp->setMinimal( TRUE );

    /*
      Fill in the maps used for putting '<a name="...">'s in
      walkthroughs.
    */
    QMap<QString, QMap<int, ExampleLocation> >::ConstIterator f =
	    megaExampleMap.begin();
    while ( f != megaExampleMap.end() ) {
	// f.key() is the link
	QMap<int, ExampleLocation>::ConstIterator g = (*f).begin();
	while ( g != (*f).end() ) {
	    // g.key() is the score, *g is the ExampleLocation
	    QMap<QString, LinkMap> *linkMaps =
		    (*g).inInclude() ? &includeLinkMaps : &walkthroughLinkMaps;
	    (*linkMaps)[(*g).fileName()][(*g).lineNum()]
		    .insert( QChar('x') + QString::number((*g).uniqueNum()) );
	    ++g;
	}
	++f;
    }
}

void Doc::setFunctionIndex( const QMap<QString, StringSet>& index )
{
    findex = index;
}

void Doc::setGroupMap( const QMap<QString, QString>& groupMap )
{
    grmap = groupMap;
}

void Doc::setClassHierarchy( const QMap<QString, StringSet>& hierarchy )
{
    chierarchy = hierarchy;
}

void Doc::printHtmlIncludeHeader( HtmlWriter& out, const QString& fileName )
{
    out.printfMeta( "<p><tt>#include &lt;<a href=\"%s\">%s</a>&gt;</tt>\n",
		    config->verbatimHref(fileName).latin1(),
		    fileName.latin1() );
}

QString Doc::href( const QString& name, const QString& text, bool propertize )
{
    // see also bookparser.cpp
    static QRegExp allProtos( QString("(?:f(?:ile|tp)|http|mailto):.*") );
    static QRegExp uglyProtos( QString("(?:file|mailto):(.*)") );

    QString namex = name;
    QString textx = text;

    if ( propertize && namex.endsWith(parenParen) ) {
	QString prop = res->relatedProperty( name );
	if ( !prop.isEmpty() ) {
	    namex = prop;
	    textx = prop;
	}
    }

    QString y = res->href( namex, textx );
    if ( textx.isEmpty() )
	textx = namex;
    if ( y.length() != textx.length() )
	return y;

    // try a keyword
    QString k = keywordLinks[namex];
    if ( k.isEmpty() ) {
	// try without the plural
	if ( namex.endsWith(QChar('s')) )
	    k = keywordLinks[namex.left(textx.length() - 1)];
	if ( k.isEmpty() ) {
	    // try an example file
	    k = includedExampleLinks[namex];
	    if ( k.isEmpty() ) {
		k = thruwalkedExampleLinks[namex];
		if ( k.isEmpty() ) {
		    // try a URL
		    if ( allProtos.exactMatch(namex) ) {
			k = namex;
			if ( textx == namex && uglyProtos.exactMatch(textx) )
			    textx = uglyProtos.cap( 1 );
		    }
		    if ( k.isEmpty() && namex.startsWith(QChar('#')) )
			k = namex;
		}
	    }
	}
    }

    if ( k.isEmpty() )
	return textx;
    else
	return QString( "<a href=\"%1\">%2</a>" ).arg( k ).arg( textx );
}

QString Doc::htmlLegaleseList()
{
    QString html;

    QMap<QString, QMap<QString, QString> >::ConstIterator q = legaleses.begin();
    while ( q != legaleses.end() ) {
	html += QString( "<hr>\n" );
	html += q.key();
	html += QString( "<ul>\n" );

	QMap<QString, QString>::ConstIterator p = (*q).begin();
	while ( p != (*q).end() ) {
	    html += QString( "<li><a href=\"%1\">%2</a>\n" ).arg( p.key() )
		    .arg( *p );
	    ++p;
	}
	html += QString( "</ul>\n" );
	++q;
    }
    return html;
}

QString Doc::htmlHeaderFileList()
{
    QString html = QString( "<ul>\n" );
    StringSet::ConstIterator h = hflist.begin();
    while ( h != hflist.end() ) {
	html += QString( "<li><a href=\"%1\">%2</a>\n" )
		.arg( config->verbatimHref(*h).latin1() ).arg( *h );
	++h;
    }
    html += QString( "</ul>\n" );
    return html;
}

QString Doc::htmlClassList()
{
    return htmlCompactList( clist );
}

QString Doc::htmlMainClassList()
{
    return htmlCompactList( mainclist );
}

QString Doc::htmlAnnotatedClassList()
{
    return htmlNormalList( clist );
}

QString Doc::htmlFunctionIndex()
{
    QString hook( "QIntDict::operator=()" );
    QString gulbrandsen( "::" );

    QString html;

    html += QString( "<center><font size=+1><b>" );
    for ( int i = 0; i < 26; i++ ) {
	QChar ch( 'a' + i );
	html += QString( "<a href=\"#%1\">%2</a> " )
		.arg( ch ).arg( ch.upper() );
    }
    html += QString( "</b></font></center>\n" );

    char nextLetter = 'a';
    char currentLetter;

    html += QString( "<ul>\n" );
    QMap<QString, StringSet>::ConstIterator f = findex.begin();
    while ( f != findex.end() ) {
	html += QString( "<li>" );

	currentLetter = f.key()[0].unicode();
	while ( islower(currentLetter) && currentLetter >= nextLetter ) {
	    html += QString( "<a name=\"%1\"></a>\n" ).arg( nextLetter );
	    nextLetter++;
	}

	html += QString( "%1:\n" ).arg( htmlProtect(f.key()) );
	StringSet::ConstIterator s = (*f).begin();
	while ( s != (*f).end() ) {
	    QString t = *s + gulbrandsen + f.key() + parenParen;
	    QString y = href( t, *s );
	    if ( y != t )
		html += QChar( ' ' );
	    html += y;
	    if ( t == hook )
		html += QString( " <a href=\"http://www.kbuxton.com/discordia/"
				 "fnord.html\">fnord</a>" );
	    ++s;
	}
	++f;
    }
    html += QString( "</ul>\n" );
    return html;
}

QString Doc::htmlClassHierarchy()
{
    QValueStack<QStringList> stack;
    QString html;

    stack.push( chierarchy[QString::null].toIStringList() );

    html += QString( "<ul>\n" );
    while ( !stack.isEmpty() ) {
	QStringList& top = stack.top();

	if ( top.isEmpty() ) {
	    stack.pop();
	    html += QString( "</ul>\n" );
	} else {
	    QString child = *top.begin();
	    html += QString( "<li>" );
	    html += href( child );
#if 0
	    // bad idea
	    QString brief = clist[child];
	    if ( !brief.isEmpty() )
		html += QString( " - " ) + brief;
#endif
	    html += QString( "\n" );
	    top.remove( top.begin() );

	    StringSet newTop = chierarchy[child];
	    if ( !newTop.isEmpty() ) {
		stack.push( newTop.toIStringList() );
		html += QString( "<ul>\n" );
	    }
	}
    }
    html += QString( "</ul>\n" );
    return html;
}

QString Doc::htmlExtensionList()
{
    QString html;

    if ( !extlist.isEmpty() ) {
	StringSet::ConstIterator e;
	QValueStack<QString> seps = separators( extlist.count(),
						QString(".\n") );
	html += QString( "* Extension classes of " );
	e = extlist.begin();
	while ( e != extlist.end() ) {
	    html += *e;
	    html += seps.pop();
	    ++e;
	}
    }
    return html;
}

QString Doc::htmlCompactList( const QMap<QString, QString>& list )
{
    /*
      The problem here is to transform a list of classes into a
      five-column table, with the constraint that all classes starting
      by the same letter should appear in the same column.
    */

    const int NumParagraphs = 27; // 26 letters in Alphabits plus tax
    const int NumColumns = 5; // number of columns in the result
    QString html( "" );

    if ( list.isEmpty() )
	return html;

    /*
      First, find out the common prefix of all classes. For Qt, the
      prefix is Q. It can easily be derived from the first and last
      classes in alphabetical order (QAccel and QXtWidget in Qt 2.1).
    */
    int commonPrefixLen = 0;
    QString first = list.begin().key();
    QMap<QString, QString>::ConstIterator beforeEnd = list.end();
    QString last = (--beforeEnd).key();

    while ( commonPrefixLen < (int) first.length() + 1 &&
	    commonPrefixLen < (int) last.length() + 1 &&
	    first[commonPrefixLen] == last[commonPrefixLen] )
	commonPrefixLen++;

    /*
      Divide the data into 27 paragraphs: A, B, ..., Z, misc. QAccel
      will fall in paragraph 0 (A) and QXtWidget in paragraph 23 (X).
      This is the only place where we assume that NumParagraphs is 27.

      Each paragraph is a QMap<QString, QString>. The entry for
      QAccel is the pair (accel, QAccel). The entry for QNPlugin is
      (nplugin, QNPlugin*).
    */
    QMap<QString, QString> paragraph[NumParagraphs];
    QString paragraphName[NumParagraphs];

    QMap<QString, QString>::ConstIterator c = list.begin();
    while ( c != list.end() ) {
	QString key = c.key().mid( commonPrefixLen ).lower();
	int paragraphNo = NumParagraphs - 1;

	if ( key[0].unicode() >= 'a' && key[0].unicode() < 'z' ) {
	    paragraphNo = key[0].unicode() - 'a';
	    paragraphName[paragraphNo] = key[0].upper();
	} else {
	    paragraphNo = 26;
	    paragraphName[paragraphNo] = QChar( '?' );
	}
	paragraph[paragraphNo].insert( key, c.key() );
	++c;
    }

    /*
      Each paragraph j has a size: paragraph[j].count(). In the
      discussion, we will assume paragraphs 0 to 5 will have sizes
      3, 1, 4, 1, 5, 9.

      We now want to compute the paragraph offset. Paragraphs 0 to 6
      start at offsets 0, 3, 4, 8, 9, 14, 23.
    */
    int paragraphOffset[NumParagraphs + 1];
    int i, j, k;

    paragraphOffset[0] = 0;
    for ( j = 0; j < NumParagraphs; j++ )
	paragraphOffset[j + 1] = paragraphOffset[j] + paragraph[j].count();

    int firstOffset[NumColumns + 1];
    int currentOffset[NumColumns];
    int currentParagraphNo[NumColumns];
    int currentOffsetInParagraph[NumColumns];

#ifdef USE_DYNAMIC_ALGORITH
    /*
      Here comes the dynamic programming algorithm that does the job.
      The number of columns is fixed (NumColumns). We want to
      minimize the number of rows of the biggest column (the cost).
      We will build two tables, prevEnd and numRows, such that the
      following condition holds:  When paragraphs 0, 1, ..., j in i
      columns, column (i - 1) should end with paragraph
      prevEnd[i][j]. Furthermore, numRows[i][j] should give the
      number of rows of the biggest column.

      For column 0, there is no previous column, so prevEnd[0][j] is
      artificially set to -1. This value is highly magical, as it
      allows other computations to work well; -2 wouldn't do.

      If only one paragraph (paragraph 0) is split among i columns,
      it is artificially put completely in column 0.

      By doing this kind of work now, we unify the rest of the
      algorithm.
    */
    int prevEnd[NumColumns][NumParagraphs];
    int numRowss[NumColumns][NumParagraphs];

    for ( i = 1; i < NumColumns; i++ ) {
	prevEnd[i][0] = 0;
	numRowss[i][0] = paragraphOffset[1];
    }
    for ( j = 0; j < NumParagraphs; j++ ) {
	prevEnd[0][j] = -1;
	numRowss[0][j] = paragraphOffset[j + 1];
    }

    /*
      Everything is, at last, set up properly for the real work. For
      any (i columns, j paragraphs) pair, we never use information on
      the right or below in the prevEnd and numRowss matrices, as this
      information is not filled in yet.
    */
    for ( i = 1; i < NumColumns; i++ ) {
	for ( j = 1; j < NumParagraphs; j++ ) {
	    /*
	      Let's concentrate on a single (i columns, j paragraphs)
	      pair; that is, how to break j paragraphs into i columns
	      and minimize the number of rows. We already know how to
	      solve the related (i columns, j - 1 paragraphs)
	      problem: end column i - 1 at prevEnd[i][j - 1]. If we
	      add one paragraph, it might turn out that
	      prevEnd[i][j - 1] is also the right place to end column
	      i - 1. But maybe column prevEnd[i][j - 1] + 1 is a
	      better place, or prevEnd[i][j - 1] + 2, or even column
	      j - 1. We'll try them in order, but we'll stop as soon
	      as the situation is not improving anymore.
	    */
	    numRowss[i][j] = INT_MAX;

	    for ( k = prevEnd[i][j - 1]; k < j; k++ ) {
		/*
		  What's the cost of breaking the column just after
		  paragraph k? It's whichever is largest between the
		  cost of breaking paragraphs 0, 1, ..., k in i - 1
		  columns and the cost of the last column.
		*/
		int m = paragraphOffset[j + 1] - paragraphOffset[k + 1];
		if ( numRowss[i - 1][k] > m )
		    m = numRowss[i - 1][k];

		if ( m <= numRowss[i][j] ) {
		    prevEnd[i][j] = k;
		    numRowss[i][j] = m;
		} else {
		    break;
		}
	    }
	}
    }

    /*
      Finally, start at prevEnd[NumColumns - 1][NumParagraphs - 1]
      and find our way back home. The information we derive is put
      into firstOffset, which tells where each column should start.
      We initialize currentOffset, currentParagraphNo, and
      currentOffsetInParagraph by the same occasion; they will be
      useful later.
    */
    k = NumParagraphs - 1;
    firstOffset[NumColumns] = paragraphOffset[k + 1];

    for ( i = NumColumns - 1; i >= 0; i-- ) {
	k = prevEnd[i][k];
	firstOffset[i] = paragraphOffset[k + 1];
	currentOffset[i] = firstOffset[i];
	currentParagraphNo[i] = k + 1;
	currentOffsetInParagraph[i] = 0;
    }

    int numRows = numRowss[NumColumns - 1][NumParagraphs - 1];
#else
    int numRows = ( list.count() + NumColumns - 1 ) / NumColumns;
    int curParagNo = 0;

    for ( i = 0; i < NumColumns; i++ ) {
	firstOffset[i] = i * numRows;
	currentOffset[i] = firstOffset[i];

	for ( j = curParagNo; j < NumParagraphs; j++ ) {
	    if ( paragraphOffset[j] > firstOffset[i] )
		break;
	    if ( paragraphOffset[j] <= firstOffset[i] )
		curParagNo = j;
	}
	currentParagraphNo[i] = curParagNo;
	currentOffsetInParagraph[i] = firstOffset[i] -
				      paragraphOffset[curParagNo];
    }
    firstOffset[NumColumns] = list.count();
#endif

    /*
      At this point, Professor Bellman leaves us and Finn Arild
      Aasheim comes in. Seriously, we have to generate all columns in
      parallel. The offset array guides us.
    */
    html += QString( "<p><table width=\"100%\">\n" );
    for ( k = 0; k < numRows; k++ ) {
	html += QString( "<tr>\n" );
	for ( i = 0; i < NumColumns; i++ ) {
	    if ( currentOffset[i] >= firstOffset[i + 1] ) {
		// this column is finished
		html += QString( "<td>\n<td>\n" );
	    } else {
		while ( currentOffsetInParagraph[i] ==
			(int) paragraph[currentParagraphNo[i]].count() ) {
		    currentParagraphNo[i]++;
		    currentOffsetInParagraph[i] = 0;
		}

		html += QString( "<td align=\"right\">" );
		if ( currentOffsetInParagraph[i] == 0 ) {
		    // start a new paragraph
		    html += QString( "<b>" );
		    html += paragraphName[currentParagraphNo[i]];
		    html += QString( "</b>" );
		}
		html += QChar( '\n' );

		// bad loop
		QMap<QString, QString>::Iterator it;
		it = paragraph[currentParagraphNo[i]].begin();
		for ( j = 0; j < currentOffsetInParagraph[i]; j++ )
		    ++it;

		html += QString( "<td>%1\n" ).arg( href(*it) );
		if ( classext.contains(*it) )
		    html += QChar( '*' );

		currentOffset[i]++;
		currentOffsetInParagraph[i]++;
	    }
	}
    }
    html += QString( "</table>\n" );
    return html;
}

QString Doc::htmlNormalList( const QMap<QString, QString>& list )
{
    QString html;

    if ( list.isEmpty() )
	return html;

    html += QString( "<p><table width=\"100%\">\n" );
    QMap<QString, QString>::ConstIterator c = list.begin();
    while ( c != list.end() ) {
	html += QString( "<tr bgcolor=#f0f0f0>" );
	// ### possibly double href()
	html += QString( "<td><b>%1</b>" ).arg( href(c.key()) );
	if ( !(*c).isEmpty() )
	    html += QString( "<td>" ) + *c;
	html += QChar( '\n' );
	++c;
    }
    html += QString( "</table>\n" );
    return html;
}

static QString emitTOCSections( const QValueList<Section>& sections )
{
    QString html;

    if ( !sections.isEmpty() ) {
	html += QString( "<ul>\n" );

	QValueList<Section>::ConstIterator s = sections.begin();
	while ( s != sections.end() ) {
	    html += QString( "<li><a href=\"#%1\">%2</a>\n" )
		    .arg( (*s).number.target() ).arg( (*s).title );
	    html += emitTOCSections( *(*s).subsections() );
	    ++s;
	}
	html += QString( "</ul>\n" );
    }
    return html;
}

QString Doc::htmlTableOfContents() const
{
    if ( toc == 0 ) {
	return QString::null;
    } else {
	return QString( "<!-- toc -->\n" ) + emitTOCSections( *toc ) +
	       QString( "<!-- endtoc -->\n" );
    }
}

Doc::Doc( Kind kind, const Location& loc, const QString& htmlText,
	  const QString& name, const QString& whatsThis )
    : html( htmlText ), ki( kind ), lo( loc ), nam( name ), whats( whatsThis ),
      inter( FALSE ), obs( FALSE ), prel( FALSE ), toc( 0 )
{
}

void Doc::setContainsExamples( const StringSet& included,
			       const StringSet& thruwalked )
{
    incl = included;
    thru = thruwalked;
}

void Doc::setLink( const QString& link, const QString& title )
{
    QString kwordLnk;

    if ( !q.isEmpty() ) {
	legaleses[q].insert( link, title );
	q = QString::null;
    }

    /*
      If there are '\keyword' commands here, find out their full
      addresses.
    */
    if ( !kwords.isEmpty() ) {
	QString base = linkBase( link );
	StringSet::ConstIterator s = kwords.begin();
	while ( s != kwords.end() ) {
	    if ( isLongDoc(kind()) )
		kwordLnk = base + QChar( '#' ) + keywordRef( *s );
	    else
		kwordLnk = link;

	    keywordLinks.insert( *s, kwordLnk );
	    if ( (*s)[0] != (*s)[0].upper() )
		keywordLinks.insert( (*s)[0].upper() + (*s).mid(1), kwordLnk );
	    ++s;
	}
	kwords.clear();
    }

    /*
      Rainer M. Schmid suggested that auto-referential links are
      removed automatically from '\sa'. This is to ease his typing if
      f1() refers to f2() and f3(); f2() to f1() and f3(); and f3()
      to f1() and f2(). He then copies and pastes
      '\sa f1() f2() f3()'.
     */
    if ( !sa.isEmpty() ) {
	QString who = title.mid( title.findRev(QChar(':')) + 1 );
	QString whoElse = who + QString( "()" );
	QStringList::Iterator s = sa.begin();
	while ( s != sa.end() ) {
	    if ( *s == whoElse || *s == who ) {
		sa.remove( s );
		break;
	    }
	    ++s;
	}
    }

    StringSet::ConstIterator exfile;
    exfile = incl.begin();
    while ( exfile != incl.end() ) {
	includedExampleLinks.insert( *exfile, link );
	++exfile;
    }

    exfile = thru.begin();
    while ( exfile != thru.end() ) {
	thruwalkedExampleLinks.insert( *exfile, link );
	++exfile;
    }

    lnk = link;
}

QString Doc::htmlSeeAlso() const
{
    QStringList see = sa;

    StringSet::ConstIterator g = groups().begin();
    while ( g != groups().end() ) {
	QString groupTitle;
	if ( grmap.contains(*g) && !grmap[*g].isEmpty() )
	    see.append( QString("&#92;link %1.html %2").arg(*g)
						       .arg(grmap[*g]) );
	++g;
    }

    QValueStack<QString> seps = separators( see.count(), QString(".\n") );
    QString html( "<p>See also " );

    QStringList::ConstIterator s = see.begin();
    while ( s != see.end() ) {
	QString name = *s;
	QString text;

	if ( name.startsWith(QString("&#92;link")) ) {
	    QStringList toks =
		    QStringList::split( QChar(' '),
					name.mid(9).stripWhiteSpace() );
	    if ( toks.count() < 2 ) {
		warning( 2, location(), "Bad '\\link' syntax in '\\sa'" );
	    } else {
		name = toks.first();
		toks.remove( toks.begin() );
		text = toks.join( QChar(' ') );
	    }
	}

	QString y = href( name, text, TRUE );
	if ( text.isEmpty() )
	    text = name;
	if ( y.length() == text.length() )
	    warning( 1, location(), "Unresolved '\\sa' to '%s'",
		     name.latin1() );

	html += y;
	html += seps.pop();
	++s;
    }
    return html;
}

void Doc::printHtml( HtmlWriter& out ) const
{
    QString t = finalHtml();
    out.putsMeta( t.latin1() );
    if ( config->supervisor() && !lnk.isEmpty() ) {
	/*
	  This belongs elsewhere. It's an easy solution to problems
	  caused by '\important'.
	*/
	if ( linkBase(lnk) == out.fileName() ) {
	    if ( resolver()->warnChangedSinceLastRun(location(), lnk, t) &&
		 !dependsOn().isEmpty() ) {
		StringSet::ConstIterator d = dependsOn().begin();
		while ( d != dependsOn().end() ) {
		    warning( 0, Location(*d), "(influences the above)" );
		    ++d;
		}
	    }
	}
    }
}

QString Doc::finalHtml() const
{
    QMap<QString, int> offsetMap;
    static QRegExp hashHashHash( QString("#" "##") );

    QString yyOut;

    QString yyIn = html;
    int yyPos = 0;
    int yyLen = yyIn.length();
    int begin;
    int end;

    Walkthrough walk;
    QString substr;
    QString fileName;
    QString link;
    QString name, ahref;
    StringSet dependsOn;
    bool metSpace = TRUE;

    while ( yyPos < yyLen ) {
	QChar ch = yyIn[yyPos++];

	if ( ch == QChar('\\') ) {
	    QString command;
	    begin = yyPos;
	    while ( yyPos < yyLen ) {
		ch = yyIn[yyPos];
		if ( isalnum((uchar) ch.latin1()) ) {
		    command += ch;
		} else {
		    break;
		}
		yyPos++;
	    }
	    if ( yyPos == begin )
		command = QChar( '\0' );

	    /*
	      If you have to teach qdoc a new command, here's the
	      other place to do it.
	    */

	    int h = HASH( command[0].unicode(), command.length() );
	    bool consumed = FALSE;

	    switch ( h ) {
	    case HASH( 'a', 18 ):
		CONSUME( "annotatedclasslist" );
		yyOut += htmlAnnotatedClassList();
		break;
	    case HASH( 'c', 1 ):
		/*
		  We try to turn '\c MyEnumValue' into a link.
		  Otherwise, '\c' means code (monospace and verbatim
		  treatment of special characters).
		*/
		CONSUME( "c" );
		name = getArgument( yyIn, yyPos );
		ahref = href( name );
		if ( ahref.length() == name.length() ||
		     name.startsWith(QChar('#')) ) {
		    yyOut += QString( "<tt>" );
		    yyOut += htmlProtect( name );
		    yyOut += QString( "</tt>" );
		} else {
		    yyOut += ahref;
		}
		break;
	    case HASH( 'c', 4 ):
		CONSUME( "code" );
		begin = yyPos;
		end = yyIn.find( QString("\\endcode"), yyPos );
		if ( end == -1 ) {
		    warning( 2, location(), "Missing '\\endcode'" );
		} else {
		    yyOut += QString( "<pre>" );
		    yyOut += processCodeHtml( yyIn.mid(begin, end - begin),
					      resolver() );
		    yyOut += QString( "</pre>\n" );
		    yyPos = end + 8;
		}
		break;
	    case HASH( 'c', 9 ):
		CONSUME( "classlist" );
		yyOut += htmlClassList();
		break;
	    case HASH( 'c', 14 ):
		CONSUME( "classhierarchy" );
		yyOut += htmlClassHierarchy();
		break;
	    case HASH( 'e', 13 ):
		CONSUME( "extensionlist" );
		yyOut += htmlExtensionList();
		break;
	    case HASH( 'f', 13 ):
		CONSUME( "functionindex" );
		yyOut += htmlFunctionIndex();
		break;
	    case HASH( 'h', 14 ):
		CONSUME( "headerfilelist" );
		yyOut += htmlHeaderFileList();
		break;
	    case HASH( 'i', 7 ):
		CONSUME( "include" );
		fileName = getWord( yyIn, yyPos );

		if ( !fileName.isEmpty() ) {
		    yyOut += QString( "<pre>" );
		    begin = yyPos;
		    yyOut += walk.includePass2( fileName, resolver(),
						includeLinkMaps[fileName],
						walkthroughLinkMaps[fileName] );
		    yyOut += QString( "</pre>" );
		    dependsOn.insert( walk.filePath() );
#if 0
		    if ( yyOut.find(hashHashHash, begin) != -1 )
			warning( 2, Location(fileName),
				 "Met '#" "##' in example" );
#endif
		}
		break;
	    case HASH( 'l', 1 ):
		CONSUME( "l" );
		name = getArgument( yyIn, yyPos );
		ahref = href( name );
		if ( ahref.length() == name.length() )
		    warning( 2, location(), "Unresolved '\\l' to '%s'",
			     name.latin1() );
		yyOut += ahref;
		break;
	    case HASH( 'l', 4 ):
		CONSUME( "link" );
		link = getArgument( yyIn, yyPos );
		begin = yyPos;
		end = yyIn.find( QString("\\endlink"), yyPos );
		if ( end == -1 ) {
		    warning( 2, location(), "Missing '\\endlink'" );
		} else {
		    QString x = yyIn.mid( begin, end - begin )
				    .stripWhiteSpace();
		    QString y = href( link, x );
		    if ( y.length() == x.length() )
			warning( 1, location(), "Unresolved '\\link' to '%s'",
				 link.latin1() );
		    yyOut += y;
		    yyPos = end + 8;
		}
		break;
	    case HASH( 'l', 12 ):
		CONSUME( "legaleselist" );
		yyOut += htmlLegaleseList();
		break;
	    case HASH( 'm', 13 ):
		CONSUME( "mainclasslist" );
		yyOut += htmlMainClassList();
		break;
	    case HASH( 'p', 7 ):
		CONSUME( "printto" );
		substr = getRestOfLine( yyIn, yyPos );
		yyOut += walk.printto( substr, location() );
		break;
	    case HASH( 'p', 9 ):
		CONSUME( "printline" );
		substr = getRestOfLine( yyIn, yyPos );
		yyOut += walk.printline( substr, location() );
		break;
	    case HASH( 'p', 10 ):
		CONSUME( "printuntil" );
		substr = getRestOfLine( yyIn, yyPos );
		yyOut += walk.printuntil( substr, location() );
		break;
	    case HASH( 'q', 9 ):
		CONSUME( "quotefile" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( !fileName.isEmpty() ) {
		    walk.startPass2( fileName, resolver(),
				     walkthroughLinkMaps[fileName] );
		    dependsOn.insert( walk.filePath() );
		    /// ### put dependsOn in first pass
		}
		break;
	    case HASH( 's', 2 ):
		CONSUME( "sa" );
		yyOut += htmlSeeAlso();
		break;
	    case HASH( 's', 6 ):
		CONSUME( "skipto" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipto( substr, location() );
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
	    case HASH( 't', 15 ):
		CONSUME( "tableofcontents" );
		yyOut += htmlTableOfContents();
		break;
	    case HASH( 'v', 7 ):
		CONSUME( "version" );
		yyOut += config->version();
	    }
	    if ( !consumed ) {
		yyOut += QString( "\\" );
		yyOut += command;
	    }
	} else if ( ch == '(' && !yyOut.endsWith(QString("</a>")) ) {
	    end = yyOut.length();
	    begin = end;
	    while ( begin > 0 && !yyOut[begin - 1].isSpace() &&
		    yyOut[begin - 1] != QChar('.') )
		begin--;

	    // deal with '(func())'
	    if ( yyOut[begin] == QChar('(') )
		begin++;

	    if ( begin < end &&
		 offsetOK(&offsetMap, yyOut.length(), yyOut.mid(begin)) ) {
		yyOut.replace( begin, end - begin,
			       href(yyOut.mid(begin) + parenParen,
				    yyOut.mid(begin)) );
	    }
	    yyOut += ch;
	} else {
	    if ( metSpace && isCppSym(ch) && config->autoHrefs() ) {
		begin = yyPos - 1;
		end = begin + 1;
		while ( end < yyLen && isCppSym(yyIn[end]) )
		    end++;
		QString t = yyIn.mid( begin, end - begin );

		// we don't want 'Qt' to link to the Qt class
		if ( clist.contains(t) && t != config->product() &&
		     offsetOK(&offsetMap, yyOut.length(), t) )
		    t = href( t );
		yyOut += t;
		yyPos = end;
	    } else {
		yyOut += ch;
	    }
	}
	metSpace = ( ch == ' ' || ch == '\n' || ch == '(' );
    }

    /*
      Time to add some automatic hrefs. Add links to class names and
      to '\keyword's.
    */
    if ( megaRegExp != 0 ) {
	int k = 0;
	while ( (k = megaRegExp->search(yyOut, k)) != -1 ) {
	    /*
	      Make sure we didn't match a '<pre>...</pre>' or
	      '<a>...</a>' thingy, but rather skip over it. (See the
	      construction of megaRegExp.)
	    */
	    if ( yyOut[k + 1] != QChar('<') ) {
		QString t = megaRegExp->cap( 0 ).mid( 1 ).simplifyWhiteSpace();

		/*
		  Insert a href, but rule out two cases: (1) The
		  current link is foo.html and the '\keyword' entry
		  is at foo.html#big-mice; (2) The current doc and
		  the entry are both at foo.html#printBar.
		*/
		if ( !keywordLinks[t].startsWith(lnk) &&
		     offsetOK(&offsetMap, yyOut.length(), t) ) {
		    yyOut.replace( k + 1, t.length(),
				   QString("<a href=\"%1\">%2</a>")
				   .arg(keywordLinks[t]).arg(t) );
		}
	    }
	    k += megaRegExp->matchedLength() + 1;
	}
    }

    ((Doc *) this)->setDependsOn( dependsOn );

    /*
      Generate the "Examples:" section. Example file names are put in
      alphabetical order. If we were not careful, the same example
      file could appear twice: once in an '\include' and once in a
      '\quotefile'.
    */
    QMap<QString, ExampleLocation> alphabetizedExamples;

    QMap<int, ExampleLocation> exampleMap = megaExampleMap[lnk];
    QMap<int, ExampleLocation>::ConstIterator exloc = exampleMap.begin();
    while ( exloc != exampleMap.end() ) {
	alphabetizedExamples.insert( (*exloc).fileName(), *exloc );
	++exloc;
    }

    if ( !alphabetizedExamples.isEmpty() ) {
	yyOut += QString( "<p>Example" );
	if ( alphabetizedExamples.count() > 1 )
	    yyOut += QChar( 's' );
	yyOut += QString( ": " );

	QValueStack<QString> seps = separators( alphabetizedExamples.count(),
						QString(".\n") );
	QMap<QString, ExampleLocation>::Iterator e =
		alphabetizedExamples.begin();
	while ( e != alphabetizedExamples.end() ) {
	    QString link;
	    if ( (*e).inInclude() )
		link = includedExampleLinks[(*e).fileName()];
	    else
		link = thruwalkedExampleLinks[(*e).fileName()];

	    link = linkBase( link ) + QString( "#x%1" ).arg( (*e).uniqueNum() );
	    yyOut += QString( "<a href=\"%1\">%2</a>" )
		     .arg( link ).arg( (*e).fileName() );
	    yyOut += seps.pop();
	    ++e;
	}
    }

    /*
      Complain before it's too late.
    */
    if ( !q.isEmpty() )
	warning( 1, location(), "Ignored '\\legalese' (fix qdoc)" );
    else if ( !kwords.isEmpty() )
	warning( 1, location(), "Ignored '\\keyword' (fix qdoc)" );

    return yyOut;
}

FnDoc::FnDoc( const Location& loc, const QString& html,
	      const QString& prototype, const QString& relates,
	      const StringSet& documentedParams, bool overloads )
    : Doc( Fn, loc, html ), proto( prototype ), rel( relates ),
      params( documentedParams ), over( overloads )
{
}

ClassDoc::ClassDoc( const Location& loc, const QString& html,
		    const QString& className, const QString& brief,
		    const QString& module, const QString& extension,
		    const StringSet& headers, const QStringList& important,
		    bool mainClass )
    : Doc( Class, loc, html, className ), bf( brief ), mod( module ),
      ext( extension ), h( headers ), main( mainClass )
{
    setFileName( config->classRefHref(className) );

    if ( !ext.isEmpty() ) {
	extlist.insert( ext );
	classext.insert( className, ext );
    }

    StringSet::ConstIterator i = important.begin();
    while ( i != important.end() ) {
	QString name = *i;
	if ( name.endsWith(parenParen) )
	    name.truncate( name.length() - 2 );
	imp.append( name );
	++i;
    }

    /*
      Derive the what's this from the '\brief' text (e.g., "The QFoo
      class is a bar class." becomes "A bar class"). ###

      A Qt 3.0 regular expression could do all of that with five
      lines of code. Unfortunately, when this code was written, Qt
      3.0 QRegExp was not yet available.
    */
    QString whats;
    bool standardWording = TRUE;
    bool finalStop = ( bf.endsWith(QChar('.')) );
    if ( !finalStop )
	bf += QChar( '.' );

    QStringList w = QStringList::split( QChar(' '), bf.simplifyWhiteSpace() );

    if ( !w.isEmpty() && w.first() == QString("The") )
	w.remove( w.begin() );
    else
	standardWording = FALSE;

    if ( !w.isEmpty() && w.first() == className )
	w.remove( w.begin() );
    else
	standardWording = FALSE;

    if ( !w.isEmpty() &&
	 (w.first() == QString("class") || w.first() == QString("widget")) )
	w.remove( w.begin() );
    else
	standardWording = FALSE;

    if ( !w.isEmpty() &&
	 (w.first() == QString("is") ||
	  w.first() == QString("provides") ||
	  w.first() == QString("contains") ||
	  w.first() == QString("specifies")) )
	w.remove( w.begin() );

    if ( !w.isEmpty() &&
	 (w.first() == QChar('a') ||
	  w.first() == QString("an")) )
	w.remove( w.begin() );

    whats = w.join( QChar(' ') );
    if ( !whats.isEmpty() )
	whats.truncate( whats.length() - 1 ); // chop the final stop

    if ( whats.isEmpty() )
	standardWording = FALSE;
    else
	whats[0] = whats[0].upper();

    if ( !standardWording )
	warning( 3, location(),
		 "Nonstandard wording in '\\brief' text for '%s'",
		 className.latin1() );
    else if ( !finalStop )
	warning( 3, location(), "Final stop missing in '\\brief' text for '%s'",
		 className.latin1() );
    setWhatsThis( whats );
}

EnumDoc::EnumDoc( const Location& loc, const QString& html,
		  const QString& name, const StringSet& documentedValues )
    : Doc( Enum, loc, html, name ), values( documentedValues )
{
}

PropertyDoc::PropertyDoc( const Location& loc, const QString& html,
			  const QString& name, const QString& brief )
    : Doc( Property, loc, html, name ), bf( brief )
{
}

void PropertyDoc::setFunctions( const QString& read, const QString& readRef,
				const QString& write, const QString& writeRef,
				const QString& reset, const QString& resetRef )
{
    static QRegExp seeAlso( "<p>\\\\sa[^a-zA-Z]" );

    QStringList funcs;
    QStringList refs;
    QStringList roles;

    if ( !write.isEmpty() ) {
	funcs.append( write );
	refs.append( writeRef );
	roles.append( QString("set") );
    }
    if ( !read.isEmpty() ) {
	funcs.append( read );
	refs.append( readRef );
	roles.append( QString("get") );
    }
    if ( !reset.isEmpty() ) {
	funcs.append( reset );
	refs.append( resetRef );
	roles.append( QString("reset") );
    }

    QStringList::ConstIterator f = funcs.begin();
    QStringList::ConstIterator g = refs.begin();
    QStringList::ConstIterator r = roles.begin();
    QValueStack<QString> seps = separators( funcs.count(), QString(".\n") );

    QString t;
    t = QString( "<p>" );
    while ( f != funcs.end() ) {
	QString role = *r;
	if ( f == funcs.begin() )
	    role[0] = role[0].upper();

	t += role;
	t += QString( " this property's value with <a href=\"#%1\">%2</a>()" )
	     .arg( *g ).arg( *f );
	t += seps.pop();
	++g;
	++r;
	++f;
    }

    // this is hackish
    int k = seeAlso.search( html );
    if ( k == -1 )
	k = html.length();
    html.insert( k, t );
}

PageLikeDoc::PageLikeDoc( Kind kind, const Location& loc, const QString& html,
			  const QString& title )
    : Doc( kind, loc, html ), ttl( title )
{
}

PageDoc::PageDoc( const Location& loc, const QString& html,
		  const QString& fileName, const QString& title )
    : PageLikeDoc( Page, loc, html, title )
{
    setName( title );
    setFileName( fileName );
}

Base64Doc::Base64Doc( const Location& loc, const QString& html,
		      const QString& fileName )
    : PageLikeDoc( Base64, loc, html )
{
    setFileName( fileName );
}

void Base64Doc::print( BinaryWriter& out )
{
    out.putsBase64( html.latin1() );
}

PlainpageDoc::PlainpageDoc( const Location& loc, const QString& html,
			    const QString& fileName )
    : PageLikeDoc( Plainpage, loc, html )
{
    setFileName( fileName );
}

void PlainpageDoc::print( BinaryWriter& out )
{
    out.puts( html.latin1() );
}

DefgroupDoc::DefgroupDoc( const Location& loc, const QString& html,
			  const QString& groupName, const QString& title )
    : PageLikeDoc( Defgroup, loc, html, title )
{
    setName( groupName );
    setFileName( config->defgroupHref(groupName) );
}

ExampleDoc::ExampleDoc( const Location& loc, const QString& html,
			const QString& fileName, const QString& title )
    : PageLikeDoc( Example, loc, html, title )
{
    setFileName( fileName );
}
