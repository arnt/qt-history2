/*
  doc.cpp
*/

#include <qmap.h>
#include <qregexp.h>
#include <qstring.h>
#include <qstringlist.h>

#include <ctype.h>
#include <limits.h>

#include "binarywriter.h"
#include "codeprocessor.h"
#include "config.h"
#include "doc.h"
#include "english.h"
#include "htmlwriter.h"
#include "messages.h"
#include "resolver.h"
#include "stringset.h"

/*
  These three macros are used so often that all-upper-case names are
  undesirable.

  hash() combines a character (the first character of a word) and a
  length to form a hash value.

  check() and consume() compare variable 'command' with a target
  string. If they are not equal, break. If they are equal,
  consume(), unlike check(), removes the '\command' from the text.
*/

#define hash( ch, len ) ( (ch) | ((len) << 8) )
#define check( target ) \
    if ( strcmp(target, command.latin1()) != 0 ) \
	break;
#define consume( target ) \
    check( target ); \
    consumed = TRUE;

static QString punctuation( ".,:;" );

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
    return isalnum( ch.latin1() ) || ch == QChar( '_' ) || ch == QChar( ':' );
}

static QString what( Doc::Kind kind )
{
    switch ( kind ) {
    case Doc::Null:
    case Doc::Fn:
	return QString( "function" );
    case Doc::Enum:
	return QString( "type" );
    case Doc::Class:
	return QString( "class" );
    default:
	return QString( "member" );
    }
}

// ### I hate the word 'sanitize'... I have to get rid of the function real
// soon... 100% bureaucraty.
static void sanitize( QString& str )
{
    if ( str.isNull() )
	str = QString( "" );
}

static QString htmlProtect( const QString& str )
{
    static QRegExp amp( QChar('&') ); // HTML metacharacters
    static QRegExp lt( QChar('<') );
    static QRegExp gt( QChar('>') );
    static QRegExp backslash( QString("\\\\") ); // qdoc metacharacter

    QString t = str;
    t.replace( amp, QString("&amp;") );
    t.replace( lt, QString("&lt;") );
    t.replace( gt, QString("&gt;") );
    t.replace( backslash, QString("&#92;") );
    return t;
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

static QString processBackslashes( const QString& str )
{
    QString t;
    int i = 0;

    while ( i < (int) str.length() ) {
	QChar ch = str[i++];
	if ( ch == QChar('\\') )
	    t += getEscape( str, i );
	else
	    t += ch;
    }
    return t;
}

/*
  This function is imperfect. If sophisticated '\keyword's are
  needed, it can always be changed.
*/
static QString indexAnchor( const QString& str )
{
    static QRegExp unfriendly( QString("[^a-zA-Z0-9_-]+") );
    QString t = str;
    t.replace( unfriendly, QChar('-') );
    return t;
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

static void skipSpaces( const QString& in, int& pos )
{
    while ( pos < (int) in.length() && in[pos].isSpace() &&
	    in[pos].unicode() != '\n' )
	pos++;
}

/*
  This function is highly magical. It tries to somehow reproduce the
  old qdoc behavior.
*/
static void skipSpacesOrNL( const QString& in, int& pos )
{
    int firstNL = -1;
    while ( pos < (int) in.length() && in[pos].isSpace() ) {
	QChar ch = in[pos];
	if ( ch == QChar('\n') ) {
	    if ( firstNL == -1 ) {
		firstNL = pos;
	    } else {
		pos = firstNL;
		break;
	    }
	}
	pos++;
    }
}

static void skipRestOfLine( const QString& in, int& pos )
{
    while ( pos < (int) in.length() && in[pos] != '\n' )
	pos++;
}

static QString getWord( const QString& in, int& pos )
{
    skipSpaces( in, pos );
    int begin = pos;
    while ( pos < (int) in.length() && !in[pos].isSpace()
	    && in[pos].unicode() != '\\' )
	pos++;
    return in.mid( begin, pos - begin );
}

static QString getRestOfLine( const QString& in, int& pos )
{
    skipSpaces( in, pos );
    int begin = pos;
    while ( pos < (int) in.length() && in[pos].unicode() != '\n' )
	pos++;

    QString t = in.mid( begin, pos - begin );
    skipSpacesOrNL( in, pos );
    return t;
}

static QString getPrototype( const QString& in, int& pos )
{
    skipSpaces( in, pos );
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

    QString t = in.mid( begin, pos - begin );
    skipSpaces( in, pos );
    return t;
}

static QString getArgument( const QString& in, int& pos )
{
    int parenDepth = 0;
    int bracketDepth = 0;

    skipSpacesOrNL( in, pos );
    int begin = pos;

    /*
      Typically, an argument ends at the next white-space. However,
      braces can be used to group words:

	  {a few words}

      Also, opening and closing parentheses have to match. Thus,

	  printf( "%d\n", x )

      is an argument too, although it contains spaces. Finally,
      trailing punctuation is not included in an argument.
    */
    if ( pos < (int) in.length() && in[pos] == QChar('{') ) {
	pos++;
	while ( pos < (int) in.length() && in[pos] != QChar('}') )
	    pos++;
	pos++;
	return in.mid( begin + 1, pos - begin - 2 ).stripWhiteSpace();
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
	    if ( parenDepth < 0 || bracketDepth < 0 || ch == QChar('\\') )
		break;
	    if ( ch.isSpace() && parenDepth <= 0 && bracketDepth <= 0 )
		break;
	    pos++;
	}

	if ( pos > begin + 1 && punctuation.find(in[pos - 1]) != -1 )
	    pos--;
	return in.mid( begin, pos - begin );
    }
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
    void setKind( Doc::Kind kind, const QString& thanksToCommand );
    void setKindHasToBe( Doc::Kind kind, const QString& thanksToCommand );

    Doc::Kind kindIs;
    Doc::Kind kindHasToBe;
    QString clueCommand;

    QStringList getStringList();
    bool somethingAheadPreventingNewParagraph();
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

    QString enumPrefix;
    QString title;
    QString heading;
    QString prototype;
    QString relates;
    QString value;
    QString x;

    StringSet included, thruwalked;
    StringSet groups, headers, keywords;
    StringSet documentedParams, documentedValues;
    QStringList seeAlso, important;
    bool obsolete = FALSE;
    int briefBegin = -1;
    int briefEnd = 0;
    int mustquoteBegin = -1;
    uint mustquoteEnd = 0;
    bool internal = FALSE;
    bool overloads = FALSE;
    int numBugs = 0;
    bool inValue = FALSE;
    bool metNL = FALSE; // never met N.L.
    int begin, end;
    int k;

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
		if ( isalnum(ch.latin1()) )
		    command += ch;
		else
		    break;
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

	    int h = hash( command[0].unicode(), command.length() );
	    bool consumed = FALSE;

	    switch ( h ) {
	    case hash( '\0', 1 ):
		consume( "" );
		yyOut += getEscape( yyIn, yyPos );
		break;
	    case hash( 'a', 1 ):
		consume( "a" );
		arg = processBackslashes( getArgument(yyIn, yyPos) );
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
	    case hash( 'a', 3 ):
		consume( "arg" );
		warning( 1, location(),
			 "Command '\\arg' is not supported, use '\\a'" );
		break;
	    case hash( 'b', 3 ):
		// see also \value
		consume( "bug" );
		if ( numBugs == 0 ) {
		    leaveWalkthroughSnippet();
		    yyOut += QString( "<p>Bugs and limitations:\n<ul>\n" );
		}
		numBugs++;
		yyOut += QString( "<li>" );
		break;
	    case hash( 'b', 5 ):
		consume( "brief" );
		skipSpaces( yyIn, yyPos );
		briefBegin = yyOut.length();
		briefEnd = INT_MAX;
		setKindHasToBe( Doc::Class, command );
		break;
	    case hash( 'b', 6 ):
		consume( "base64" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( fileName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected file name after '\\base64'" );
		} else {
		    yyOut = yyIn.mid( yyPos );
		    setKind( Doc::Base64, command );
		}
		break;
	    case hash( 'c', 1 ):
		consume( "c" );
		arg = htmlProtect( getArgument(yyIn, yyPos) );
		if ( arg.isEmpty() ) {
		    warning( 2, location(), "Expected code chunk after '\\c'" );
		} else {
		    yyOut += QString( "<tt>" );
		    yyOut += arg;
		    yyOut += QString( "</tt>" );
		}
		break;
	    case hash( 'c', 4 ):
		consume( "code" );
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
	    case hash( 'c', 5 ):
		consume( "class" );
		className = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( className.isEmpty() ) {
		    warning( 2, location(),
			     "Expected class name after '\\class'" );
		    setKindHasToBe( Doc::Class, command );
		} else {
		    setKind( Doc::Class, command );
		}
		break;
	    case hash( 'd', 6 ):
		consume( "define" );
		getWord( yyIn, yyPos );
		break;
	    case hash( 'd', 8 ):
		consume( "defgroup" );
		groupName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( groupName.right(5) == QString(".html") ) {
		    warning( 3, location(),
			     "Group name after '\\defgroup' no longer requires"
			     " '.html'" );
		    groupName.truncate( groupName.length() - 5 );
		}
		if ( groupName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected group name after '\\defgroup'" );
		    setKindHasToBe( Doc::Defgroup, command );
		} else {
		    setKind( Doc::Defgroup, command );
		}
		break;
	    case hash( 'e', 1 ):
		consume( "e" );
		arg = processBackslashes( getArgument(yyIn, yyPos) );
		if ( arg.isEmpty() ) {
		    warning( 2, location(), "Expected word after '\\e'" );
		} else {
		    yyOut += QString( "<em>" );
		    yyOut += arg;
		    yyOut += QString("</em>" );
		}
		break;
	    case hash( 'e', 4 ):
		consume( "enum" );
		enumName = getWord( yyIn, yyPos );
		k = enumName.findRev( QString("::") );
		if ( k != -1 )
		    enumPrefix = enumName.left( k + 2 );
		skipRestOfLine( yyIn, yyPos );
		setKind( Doc::Enum, command );
		break;
	    case hash( 'e', 7 ):
		if ( command.length() != 7 )
		    break;
		if ( command[3] == QChar('c') ) {
		    consume( "endcode" );
		    warning( 2, location(), "Missing '\\code'" );
		} else if ( command[3] == QChar('l') ) {
		    consume( "endlink" );
		    // we have found the missing link: Eirik Aavitsland
		    warning( 2, location(), "Missing '\\link'" );
		} else {
#if 1
		    consume( "example" );
		    warning( 2, location(),
			     "Command '%s' has been renamed '%s'", "\\example",
			     "\\file" );
		    fileName = getWord( yyIn, yyPos );
		    skipRestOfLine( yyIn, yyPos );
		    setKind( Doc::Example, command );
#endif
		}
		break;
	    case hash( 'e', 9 ):
		consume( "extension" );
		extName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( extName.isEmpty() )
		    warning( 2, location(),
			     "Expected extension name after '\\extension'" );
		else
		    setKindHasToBe( Doc::Class, command );
		break;
	    case hash( 'f', 2 ):
		consume( "fn" );
		// see also \overload
		prototype = getPrototype( yyIn, yyPos );
		if ( prototype.isEmpty() )
		    warning( 2, location(),
			     "Expected function prototype after '\\fn'" );
		else
		    setKind( Doc::Fn, command );
		break;
	    case hash( 'f', 4 ):
		consume( "file" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );
		// ### use page instead of example
		setKind( Doc::Example, command );
		break;
	    case hash( 'h', 6 ):
		consume( "header" );
		x = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( x.isEmpty() ) {
		    warning( 2, location(),
			     "Expected file name after '\\header'" );
		} else {
		    headers.insert( x );
		    setKindHasToBe( Doc::Class, command );
		}
		break;
	    case hash( 'h', 7 ):
		consume( "heading" );
		heading = getRestOfLine( yyIn, yyPos ).simplifyWhiteSpace();
		break;
	    case hash( 'i', 7 ):
		if ( command.length() != 7 )
		    break;
		if ( command[2] == QChar('c') ) {
		    consume( "include" );
		    x = getWord( yyIn, yyPos );
		    skipRestOfLine( yyIn, yyPos );
		    flushWalkthrough( walk, &included, &thruwalked );

		    if ( x.isEmpty() )
			warning( 2, location(),
				 "Expected file name after '\\include'" );
		    else
			walk.includePass1( x, Doc::resolver() );
		    yyOut += QString( "\\include " ) + x + QChar( '\n' );
		} else {
		    consume( "ingroup" );
		    groupName = getWord( yyIn, yyPos );
		    skipRestOfLine( yyIn, yyPos );

		    if ( groupName.right(5) == QString(".html") ) {
			warning( 3, location(),
				 "Group name after '\\ingroup' should not end"
				 " with '.html'" );
			groupName.truncate( groupName.length() - 5 );
		    }

		    if ( groupName.isEmpty() )
			warning( 2, location(),
				 "Expected group name after '\\ingroup'" );
		    else
			groups.insert( groupName );
		}
		break;
	    case hash( 'i', 8 ):
		consume( "internal" );
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
	    case hash( 'i', 9 ):
		consume( "important" );
		important = getStringList();
		setKindHasToBe( Doc::Class, command );
		break;
	    case hash( 'k', 7 ):
		consume( "keyword" );
		x = getRestOfLine( yyIn, yyPos ).simplifyWhiteSpace();
		keywords.insert( x );

		/*
		  The <a name="..."> for '\page's is put right here,
		  because a page can contain many topics. Otherwise,
		  no new <a name="..."> is created; the link given by
		  setLink() is used.
		*/
		if ( kindIs == Doc::Page )
		    yyOut += QString( "<a name=\"%1\"></a>" ).arg( x );
		break;
	    case hash( 'l', 4 ):
		consume( "link" );
		begin = yyPos;
		end = yyIn.find( QString("\\endlink"), yyPos );
		if ( end == -1 ) {
		    warning( 2, location(), "Missing '\\endlink'" );
		} else {
		    yyOut += QString( "\\link" );
		    yyOut += processBackslashes( yyIn.mid(begin, end - begin) );
		    yyOut += QString( "\\endlink" );
		    yyPos = end + 8;
		}
		break;
	    case hash( 'm', 6 ):
		consume( "module" );
		moduleName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( moduleName.isEmpty() )
		    warning( 2, location(),
			     "Expected module name after '\\module'" );
		else
		    setKindHasToBe( Doc::Class, command );
		break;
	    case hash( 'm', 9 ):
		consume( "mustquote" );
		skipSpaces( yyIn, yyPos );
		mustquoteBegin = yyOut.length();
		mustquoteEnd = INT_MAX;
		break;
	    case hash( 'n', 4 ):
		consume( "note" );
		yyOut += QString( "Note:" );
		warning( 2, location(), "No such command '%s'", "\\note" );
		break;
	    case hash( 'o', 8 ):
		if ( command.length() != 8 )
		    break;
		if ( command[1] == QChar('b') ) {
		    consume( "obsolete" );
		    obsolete = TRUE;
		    yyOut += QString( "<b>This %1 is obsolete.</b> It is"
				      " provided to keep old source working,"
				      " and will probably be removed in a"
				      " future version of %2. We strongly"
				      " advise against using it in new"
				      " code.\n" )
			     .arg( what(kindIs) )
			     .arg( config->product() );
		    metNL = TRUE;
		} else {
		    consume( "overload" );
		    overloads = TRUE;
		    yyOut += QString( "This is an overloaded member function, "
				      "provided for convenience. It behaves "
				      "essentially like the above function.\n");
		    metNL = TRUE;

		    // see also \fn
		    if ( prototype.isEmpty() )
			prototype = getPrototype( yyIn, yyPos );
		    if ( kindIs != Doc::Fn )
			setKind( Doc::Fn, command );
		}
		break;
	    case hash( 'p', 4 ):
		consume( "page" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );
		setKind( Doc::Page, command );
		break;
	    case hash( 'p', 7 ):
		consume( "printto" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.printto( substr, location() );
		enterWalkthroughSnippet();
		yyOut += QString( "\\printto " ) + substr + QChar( '\n' );
		leaveWalkthroughSnippet();
		break;
	    case hash( 'p', 9 ):
		if ( command.length() != 9 )
		    break;
		if ( command[1] == QChar('l') ) {
		    consume( "plainpage" );
		    fileName = getWord( yyIn, yyPos );
		    skipRestOfLine( yyIn, yyPos );

		    if ( fileName.isEmpty() ) {
			warning( 2, location(),
				 "Expected file name after '\\plainpage'" );
		    } else {
			yyOut = yyIn.mid( yyPos );
			setKind( Doc::Plainpage, command );
		    }
		} else {
		    consume( "printline" );
		    substr = getRestOfLine( yyIn, yyPos );
		    walk.printline( substr, location() );
		    enterWalkthroughSnippet();
		    yyOut += QString( "\\printline " ) + substr + QChar( '\n' );
		    leaveWalkthroughSnippet();
		}
		break;
	    case hash( 'p', 10 ):
		consume( "printuntil" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.printuntil( substr, location() );
		enterWalkthroughSnippet();
		yyOut += QString( "\\printuntil " ) + substr + QChar( '\n' );
		leaveWalkthroughSnippet();
		break;
	    case hash( 'r', 5 ):
		consume( "reimp" );
		yyOut += QString( "Reimplemented for internal reasons; the API"
				  " is not affected." );
		internal = TRUE;
		break;
	    case hash( 'r', 7 ):
		consume( "relates" );
		relates = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( relates.isEmpty() )
		    warning( 2, location(),
			     "Expected class name after '\\relates'" );
		break;
	    case hash( 's', 2 ):
		consume( "sa" );
		mustquoteEnd = yyOut.length();
		if ( numBugs > 0 ) {
		    yyOut += QString( "</ul>" );
		    numBugs = 0;
		}

		seeAlso = getStringList();
		if ( !seeAlso.isEmpty() )
		    yyOut += QString( "\\sa" );
		break;
	    case hash( 's', 6 ):
		consume( "skipto" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipto( substr, location() );
		yyOut += QString( "\\skipto " ) + substr + QChar( '\n' );
		break;
	    case hash( 's', 8 ):
		consume( "skipline" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipline( substr, location() );
		yyOut += QString( "\\skipline " ) + substr + QChar( '\n' );
		break;
	    case hash( 's', 9 ):
		consume( "skipuntil" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipuntil( substr, location() );
		yyOut += QString( "\\skipuntil " ) + substr + QChar( '\n' );
		break;
	    case hash( 't', 5 ):
		consume( "title" );
		title = getRestOfLine( yyIn, yyPos ).simplifyWhiteSpace();
		break;
	    case hash( 'v', 5 ):
		// see also \bug
		consume( "value" );
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
		yyOut += QString( "<a name=\"%1\"></a>" ).arg( value );
		yyOut += QString( "<tt>" );
		yyOut += enumPrefix + value;
		yyOut += QString( "</tt> - " );
		documentedValues.insert( value );
		break;
	    case hash( 'w', 7 ):
		consume( "warning" );
		yyOut += QString( "<b>Warning:</b>" );
		break;
#if 1 // ###
	    case hash( 'd', 11 ):
		consume( "dontinclude" );
		warning( 2, location(), "Command '%s' has been renamed '%s'",
			 "\\dontinclude", "\\walkthrough" );
		command = QString( "walkthrough" );
		/* fall through */
#endif
	    case hash( 'w', 11 ):
		consume( "walkthrough" );
		x = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );
		if ( x != walk.fileName() )
		    flushWalkthrough( walk, &included, &thruwalked );

		if ( x.isEmpty() )
		    warning( 2, location(),
			     "Expected file name after '\\walkthrough'" );
		else
		    walk.startPass1( x, Doc::resolver() );

		yyOut += QString( "\\walkthrough " ) + x + QChar( '\n' );
	    }
	    if ( !consumed ) {
		yyOut += QString( "\\" );
		yyOut += command;
	    }
	} else if ( ch == '>' && yyOut.right(4) == QString("<pre") ) {
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
		    if ( briefEnd == INT_MAX )
			briefEnd = yyOut.length();
		    yyOut += QString( "<p>" );
		    metNL = FALSE;
		}
	    }
	}
    }

    // ###

    if ( numBugs > 0 || inValue )
	yyOut += QString( "</ul>" );
    flushWalkthrough( walk, &included, &thruwalked );

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
	if ( briefBegin == -1 )
	    warning( 2, loc, "No '\\brief' in '\\class' doc" );
	else
	    brief = yyOut.mid( briefBegin, briefEnd - briefBegin )
			 .stripWhiteSpace();
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
			    headers, important );
	break;
    case Doc::Enum:
	sanitize( enumName );
	doc = new EnumDoc( loc, yyOut, enumName, documentedValues );
	break;
    case Doc::Page:
	sanitize( fileName );
	doc = new PageDoc( loc, yyOut, fileName, title, heading );
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
	doc = new DefgroupDoc( loc, yyOut, groupName, title, heading );
	break;
    case Doc::Example:
	yyOut += "\\include ";
	yyOut += fileName;
	doc = new ExampleDoc( loc, yyOut, fileName, title, heading );
	break;
    default:
	doc = new Doc( Doc::Null, loc, yyOut );
    }
    doc->setInternal( internal );
    doc->setObsolete( obsolete );
    doc->setSeeAlso( seeAlso );
    doc->setKeywords( keywords );
    doc->setGroups( groups );
    doc->setContainsExamples( included, thruwalked );
    if ( mustquoteBegin >= 0 )
	doc->setHtmlMustQuote( yyOut.mid(mustquoteBegin,
					 mustquoteEnd - mustquoteBegin)
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
    // this function is often called for nothing
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
	if ( ((*score).inInclude() && !alreadyIncluded) ||
	     (!(*score).inInclude() && !alreadyThruwalked) ) {
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

void DocParser::setKind( Doc::Kind kind, const QString& thanksToCommand )
{
    setKindHasToBe( kind, thanksToCommand );
    if ( kind == kindIs )
	warning( 2, location(), "Command '\\%s' is redundant or contradictory",
		 thanksToCommand.latin1() );
    kindIs = kind;
}

void DocParser::setKindHasToBe( Doc::Kind kind, const QString& thanksToCommand )
{
    if ( kindHasToBe == Doc::Null ) {
	kindHasToBe = kind;
	clueCommand = thanksToCommand;
    } else if ( kindHasToBe != kind ) {
	warning( 3, location(),
		 "Cannot have both '\\%s' and '\\%s' in same doc",
		 clueCommand.latin1(), thanksToCommand.latin1() );
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
    while ( TRUE ) {
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
	if ( punctuation.find(yyIn[end - 1]) != -1 )
	    end--;
	else if ( yyPos < yyLen && punctuation.find(yyIn[yyLen]) != -1 )
	    yyPos++;

	if ( end > begin )
	    stringl.append( processBackslashes(yyIn.mid(begin, end - begin)) );
	skipSpacesOrNL( yyIn, yyPos );
    }
    return stringl;
}

bool DocParser::somethingAheadPreventingNewParagraph()
{
    int inPos0 = yyPos;
    bool something = FALSE;

    skipSpaces( yyIn, yyPos );
    if ( yyPos < yyLen - 4 ) {
	QString lookahead = yyIn.mid( yyPos, 4 );
	if ( lookahead == QString("<li>") || lookahead == QString("\\bug") ||
	     lookahead == QString("\\val") )
	    something = TRUE;
    }
    yyPos = inPos0;
    return something;
}

void DocParser::enterWalkthroughSnippet()
{
    if ( !yyInWalkthroughSnippet ) {
	if ( yyOut.right(6) == QString("</pre>") )
	    yyOut.truncate( yyOut.length() - 6 );
	else
	    yyOut += QString( "<pre>" );
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
	walkthroughCommands.insert( QString("\\printline") );
	walkthroughCommands.insert( QString("\\printline") );
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
QMap<QString, QMap<QString, QString> > Doc::quotes;
QMap<QString, QString> Doc::keywordLinks;
StringSet Doc::hflist;
QMap<QString, QString> Doc::clist;
QMap<QString, StringSet> Doc::findex;
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

void Doc::setClassList( const QMap<QString, QString>& classList )
{
    clist = classList;

    /*
      Why is here the best place to build the mega regular expression?
      ### Donno.
    */
    if ( !config->autoHrefs() )
	return;

    // careful with this mega regexp: white space magic below
    QString t( "(?:<(?:a [^>]*|pre)>.*</(?:a|pre)>|"
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

QString Doc::href( const QString& name, const QString& text )
{
    QString t = text;
    QString y = res->href( name, t );
    if ( t.isEmpty() )
	t = name;
    if ( y.length() != t.length() )
	return y;

    // try a keyword
    QString k = keywordLinks[t];
    // try without the plural
    if ( k.isEmpty() && t.right(1) == QChar('s') )
	k = keywordLinks[t.left(t.length() - 1)]; 
    // try a URL
    if ( k.isEmpty() ) {
	if ( name.startsWith(QString("file:")) ||
	     name.startsWith(QString("ftp:")) ||
	     name.startsWith(QString("http:")) ||
	     name.startsWith(QString("mailto:")) ) {
	    k = name;

	    // chop the protocol
	    if ( t == name && !t.startsWith(QString("http:")) )
		t = name.mid( name.find(QChar(':')) + 1 );
	}
    }
    if ( k.isEmpty() && name[0] == QChar('#') )
	k = name;

    if ( k.isEmpty() )
	return t;
    else
	return QString( "<a href=\"%1\">%2</a>" ).arg( k ).arg( t );
}

QString Doc::htmlQuoteList()
{
    QString html;

    QMap<QString, QMap<QString, QString> >::ConstIterator q = quotes.begin();
    while ( q != quotes.end() ) {
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
    /*
      The problem here is to transform a list of classes into a
      five-column table, with the constraint that all classes starting
      by the same letter should appear in the same column.
    */

    const int NumParagraphs = 27; // 26 letters in Alphabits, plus tax
    const int NumColumns = 5; // number of columns in the result
    QString html( "" );

    if ( clist.isEmpty() )
	return html;

    /*
      First, find out the common prefix of all classes. For Qt, the
      prefix is Q. It can easily be derived from the first and last
      classes in alphabetical order (QAccel and QXtWidget in Qt 2.1).
    */
    int commonPrefixLen = 0;
    QString first = clist.begin().key();
    QMap<QString, QString>::ConstIterator beforeEnd = clist.end();
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

    QMap<QString, QString>::ConstIterator c = clist.begin();
    while ( c != clist.end() ) {
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
    int numRows[NumColumns][NumParagraphs];

    for ( i = 1; i < NumColumns; i++ ) {
	prevEnd[i][0] = 0;
	numRows[i][0] = paragraphOffset[1];
    }
    for ( j = 0; j < NumParagraphs; j++ ) {
	prevEnd[0][j] = -1;
	numRows[0][j] = paragraphOffset[j + 1];
    }

    /*
      Everything is, at last, set up properly for the real work. For
      any (i columns, j paragraphs) pair, we never use information on
      the right or below in the prevEnd and numRows matrices, as this
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
	    numRows[i][j] = INT_MAX;

	    for ( k = prevEnd[i][j - 1]; k < j; k++ ) {
		/*
		  What's the cost of breaking the column just after
		  paragraph k? It's whichever is largest between the
		  cost of breaking paragraphs 0, 1, ..., k in i - 1
		  columns and the cost of the last column.
		*/
		int m = paragraphOffset[j + 1] - paragraphOffset[k + 1];
		if ( numRows[i - 1][k] > m )
		    m = numRows[i - 1][k];

		if ( m <= numRows[i][j] ) {
		    prevEnd[i][j] = k;
		    numRows[i][j] = m;
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
      We also initialize currentOffset, currentParagraphNo, and
      prevParagraphNo by the same occasion; they will be useful
      later.
    */
    int firstOffset[NumColumns + 1];
    int currentOffset[NumColumns];
    int currentParagraphNo[NumColumns];
    int prevParagraphNo[NumColumns];

    k = NumParagraphs - 1;
    firstOffset[NumColumns] = paragraphOffset[k + 1];

    for ( i = NumColumns - 1; i >= 0; i-- ) {
	k = prevEnd[i][k];
	firstOffset[i] = paragraphOffset[k + 1];
	currentOffset[i] = firstOffset[i];
	currentParagraphNo[i] = k + 1;
	prevParagraphNo[i] = -1;
    }

    /*
      At this point, Professor Bellman leaves us and Finn Arild
      Aasheim comes in. Seriously, we have to generate all columns in
      parallel. The offset array guides us.
    */
    html += QString( "<table>\n" );
    for ( k = 0; k < numRows[NumColumns - 1][NumParagraphs - 1]; k++ ) {
	html += QString( "<tr>\n" );
	for ( i = 0; i < NumColumns; i++ ) {
	    if ( currentOffset[i] >= firstOffset[i + 1] ) {
		// this column is finished
		html += QString( "<td>\n<td>\n" );
	    } else {
		while ( paragraph[currentParagraphNo[i]].isEmpty() )
		    currentParagraphNo[i]++;

		html += QString( "<td>" );
		if ( currentParagraphNo[i] > prevParagraphNo[i] ) {
		    // start a new paragraph
		    html += QString( "<b>" );
		    html += paragraphName[currentParagraphNo[i]];
		    html += QString( "</b>" );
		    prevParagraphNo[i] = currentParagraphNo[i];
		}
		html += QChar( '\n' );

		QMap<QString, QString>::Iterator first;
		first = paragraph[currentParagraphNo[i]].begin();

		html += QString( "<td>%1\n" ).arg( href(*first) );
		if ( classext.contains(*first) )
		    html += QChar( '*' );

		paragraph[currentParagraphNo[i]].remove( first );
		currentOffset[i]++;
	    }
	}
    }
    html += QString( "</table>\n" );
    return html;
}

QString Doc::htmlAnnotatedClassList()
{
    /*
      We fight hard just to go through the QMap in case-insensitive
      order. In Qt, this gets class Qt among the t's and Quebec among
      the u's.
    */
    StringSet cset;
    QString html = QString( "<table>\n" );
    QMap<QString, QString>::ConstIterator c = clist.begin();
    while ( c != clist.end() ) {
	cset.insert( c.key() );
	++c;
    }
    QStringList stringl = cset.toIStringList();
    QStringList::ConstIterator s = stringl.begin();
    while ( s != stringl.end() ) {
	c = clist.find( *s );
	html += QString( "<tr bgcolor=#f0f0f0>" );
	html += QString( "<td><b>%1</b>" ).arg( href(c.key()) );
	if ( !(*c).isEmpty() )
	    html += QString( "<td>" ) + *c;
	html += QString( "\n" );
	++s;
    }
    html += QString( "</table>\n" );
    return html;
}

QString Doc::htmlFunctionIndex()
{
    QString gulbrandsen( "::" );
    QString html = QString( "<ul>\n" );
    QMap<QString, StringSet>::ConstIterator f = findex.begin();
    while ( f != findex.end() ) {
	html += QString( "<li>%1:\n" ).arg( htmlProtect(f.key()) );
	StringSet::ConstIterator s = (*f).begin();
	while ( s != (*f).end() ) {
	    html += QChar( ' ' );
	    html += href( *s + gulbrandsen + f.key(), *s );
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
	    html += child;
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
    QString html( "" );

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

Doc::Doc( Kind kind, const Location& loc, const QString& htmlText,
	  const QString& name, const QString& whatsThis )
    : ki( kind ), lo( loc ), html( htmlText ), nam( name ), whats( whatsThis ),
      inter( FALSE ), obs( FALSE )
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
	quotes[q].insert( link, title );
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
	    if ( kind() == Page )
		kwordLnk = base + QChar( '#' ) + indexAnchor( *s );
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
    QValueStack<QString> seps = separators( sa.count(), QString(".\n") );
    QString html( "<p>See also " );

    QStringList::ConstIterator s = sa.begin();
    while ( s != sa.end() ) {
	QString name = *s;
	QString text;

	if ( name.right(1) != QChar(')') && resolver()->resolvefn(name) )
	    name += QString( "()" );

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

	QString y = href( name, text );
	if ( text.isEmpty() )
	    text = name;
	if ( y.length() == text.length() && text.startsWith(QString("<a")) )
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
	    if ( resolver()->warnChangedSinceLastRun(location(), lnk, t)
		 && !dependsOn().isEmpty() ) {
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
		if ( isalnum(ch.latin1()) )
		    command += ch;
		else
		    break;
		yyPos++;
	    }
	    if ( yyPos == begin )
		command = QChar( '\0' );

	    /*
	      If you have to teach qdoc a new command, here's the
	      other place to do it.
	    */

	    int h = hash( command[0].unicode(), command.length() );
	    bool consumed = FALSE;

	    switch ( h ) {
	    case hash( 'a', 18 ):
		consume( "annotatedclasslist" );
		yyOut += htmlAnnotatedClassList();
		break;
	    case hash( 'c', 4 ):
		consume( "code" );
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
	    case hash( 'c', 9 ):
		consume( "classlist" );
		yyOut += htmlClassList();
		break;
	    case hash( 'c', 14 ):
		consume( "classhierarchy" );
		yyOut += htmlClassHierarchy();
		break;
	    case hash( 'e', 13 ):
		consume( "extensionlist" );
		yyOut += htmlExtensionList();
		break;
	    case hash( 'f', 13 ):
		consume( "functionindex" );
		yyOut += htmlFunctionIndex();
		break;
	    case hash( 'h', 14 ):
		consume( "headerfilelist" );
		yyOut += htmlHeaderFileList();
		break;
	    case hash( 'i', 7 ):
		consume( "include" );
		fileName = getWord( yyIn, yyPos );

		if ( !fileName.isEmpty() ) {
		    yyOut += QString( "<pre>" );
		    yyOut += walk.includePass2( fileName, resolver(),
						includeLinkMaps[fileName],
						walkthroughLinkMaps[fileName] );
		    yyOut += QString( "</pre>" );
		    dependsOn.insert( walk.filePath() );
		}
		break;
	    case hash( 'l', 1 ):
		consume( "l" );
		name = getArgument( yyIn, yyPos );
		ahref = href( name );
		if ( ahref.length() == name.length() )
		    warning( 2, location(), "Unresolved '\\l' to '%s'",
			     name.latin1() );
		yyOut += ahref;
		break;
	    case hash( 'l', 4 ):
		consume( "link" );
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
	    case hash( 'p', 7 ):
		consume( "printto" );
		substr = getRestOfLine( yyIn, yyPos );
		yyOut += walk.printto( substr, location() );
		break;
	    case hash( 'p', 9 ):
		consume( "printline" );
		substr = getRestOfLine( yyIn, yyPos );
		yyOut += walk.printline( substr, location() );
		break;
	    case hash( 'p', 10 ):
		consume( "printuntil" );
		substr = getRestOfLine( yyIn, yyPos );
		yyOut += walk.printuntil( substr, location() );
		break;
	    case hash( 'q', 9 ):
		consume( "quotelist" );
		yyOut += htmlQuoteList();
		break;
	    case hash( 's', 2 ):
		consume( "sa" );
		yyOut += htmlSeeAlso();
		break;
	    case hash( 's', 6 ):
		consume( "skipto" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipto( substr, location() );
		break;
	    case hash( 's', 8 ):
		consume( "skipline" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipline( substr, location() );
		break;
	    case hash( 's', 9 ):
		consume( "skipuntil" );
		substr = getRestOfLine( yyIn, yyPos );
		walk.skipuntil( substr, location() );
		break;
	    case hash( 'v', 7 ):
		consume( "version" );
		yyOut += config->version();
		break;
	    case hash( 'w', 11 ):
		consume( "walkthrough" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( !fileName.isEmpty() ) {
		    walk.startPass2( fileName, resolver(),
				     walkthroughLinkMaps[fileName] );
		    dependsOn.insert( walk.filePath() );
		    /// ### put dependsOn in first pass
		}
	    }
	    if ( !consumed ) {
		yyOut += QString( "\\" );
		yyOut += command;
	    }
	} else if ( ch == '(' ) {
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
		/*
		  It's always a good idea to include the '()', as it
		  provides some typing (in at least two senses of the
		  word).
		*/
		if ( ch == QChar('(') && yyIn.mid(yyPos, 1) == QChar(')') ) {
		    yyOut.replace( begin, end - begin,
				   href(yyOut.mid(begin)) + QString("()") );
		    yyPos++;
		} else {
		    yyOut.replace( begin, end - begin, href(yyOut.mid(begin)) );
		    yyOut += ch;
		}
	    } else {
		yyOut += ch;
	    }
	} else {
	    if ( metSpace && isCppSym(ch) && config->autoHrefs() ) {
		begin = yyPos - 1;
		end = begin + 1;
		while ( end < yyLen && isCppSym(yyIn[end]) )
		    end++;
		QString t = yyIn.mid( begin, end - begin );

		// we don't 'Qt' to links to the Qt class
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
      '\walkthrough'.
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
if ( link.isEmpty() )
qWarning( "'%s' included? %d  is empty", (*e).fileName().latin1(), (*e).inInclude() );

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
	warning( 1, location(), "Ignored '\\mustquote' (fix qdoc)" );
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
		    const StringSet& headers, const QStringList& important )
    : Doc( Class, loc, html, className ), bf( brief ), mod( module ),
      ext( extension ), h( headers ), imp( important )
{
    setFileName( config->classRefHref(className) );

    if ( !ext.isEmpty() ) {
	extlist.insert( ext );
	classext.insert( className, ext );
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
    bool finalStop = ( bf.right(1) == QChar('.') );
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
	 (w.first() == QString("class") ||
	  w.first() == QString("widget")) )
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
		  const QString& enumName, const StringSet& documentedValues )
    : Doc( Enum, loc, html, enumName ), values( documentedValues )
{
}

PageLikeDoc::PageLikeDoc( Kind kind, const Location& loc, const QString& html,
			  const QString& title, const QString& heading )
    : Doc( kind, loc, html ), ttl( title ), hding( heading )
{
}

QString PageLikeDoc::heading() const
{
    static QRegExp linkEndlink( QString("\\\\link(.*)\\\\endlink") );
    int k = linkEndlink.search( hding );
    if ( k == -1 ) {
	return hding;
    } else {
	QStringList toks = QStringList::split( QChar(' '),
		linkEndlink.cap(1).stripWhiteSpace() );
	if ( toks.count() < 2 ) {
	    warning( 2, location(),
		     "Bad '\\link ... \\endlink' syntax in '\\heading'" );
	    return hding;
	}

	QString name = toks.first();
	toks.remove( toks.begin() );
	QString text = toks.join( QChar(' ') );
	return hding.left( k ) + href( name, text ) +
	       hding.mid( k + linkEndlink.matchedLength() );
    }
}

PageDoc::PageDoc( const Location& loc, const QString& html,
		  const QString& fileName, const QString& title,
		  const QString& heading )
    : PageLikeDoc( Page, loc, html, title, heading )
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
    out.putsBase64( htmlData().latin1() );
}

PlainpageDoc::PlainpageDoc( const Location& loc, const QString& html,
			    const QString& fileName )
    : PageLikeDoc( Plainpage, loc, html, fileName )
{
}

void PlainpageDoc::print( BinaryWriter& out )
{
    out.puts( htmlData().latin1() );
}

DefgroupDoc::DefgroupDoc( const Location& loc, const QString& html,
			  const QString& groupName, const QString& title,
			  const QString& heading )
    : PageLikeDoc( Defgroup, loc, html, title, heading )
{
    setName( groupName );
    setFileName( config->defgroupHref(groupName) );
}

ExampleDoc::ExampleDoc( const Location& loc, const QString& html,
			const QString& fileName, const QString& title,
			const QString& heading )
    : PageLikeDoc( Example, loc, html, title, heading )
{
    setFileName( fileName );
}
