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
#include "walkthrough.h"

/*
  These three macros are used so often that all-upper-case names are
  undesirable.

  hash() combines a character (the first character of a word) and a
  lenght to form a hash value.

  check() and consume() compare variable 'command' with a target
  string.  If they are not equal, break.  If they are equal,
  consume(), but not check(), removes the '\command' from the text.
*/

#define hash( ch, len ) ( (ch) | ((len) << 8) )
#define check( target ) \
    if ( strcmp(target, command.latin1()) != 0 ) \
	break;
#define consume( target ) \
    check( target ); \
    consumed = TRUE;

static QString punctuation( ".,:;" );

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
	return QString( "enum" );
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
    static QRegExp amp( QChar('&') );
    static QRegExp lt( QChar('<') );
    static QRegExp gt( QChar('>') );

    QString t = str;
    t.replace( amp, QString("&amp;") );
    t.replace( lt, QString("&lt;") );
    t.replace( gt, QString("&gt;") );
    return t;
}

/*
  This function is imperfect.  If sophisticated '\keyword's are needed, it can
  always be changed.
 */
static QString indexAnchor( const QString& str )
{
    static QRegExp unfriendly( QString("[^a-zA-Z0-9_-]+") );
    QString t = str;
    t.replace( unfriendly, QChar('-') );
    return t;
}

/*
  This function makes sure no two automatic links for the same identifier are
  too close to each other.  It returns TRUE if it's OK to have a new link to
  name, otherwise FALSE.  It also updates offsetMap.

  The criterion is that two automatic links to the same place should be
  separated by at least 1009 characters.
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
  This function is highly magical.  It tries to somehow reproduce the old qdoc's
  behavior.
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
      Typically, an argument ends at the next white-space.  However, braces can
      be used to group words:

	  {a few words}

      Also, opening and closing parentheses have to match.  Thus,

	  printf( "%d\n", x )

      is an argument too, although it contains spaces.  Finally, trailing
      punctuation is not included in an argument.
    */
    if ( pos < (int) in.length() && in[pos] == QChar('{') ) {
	pos++;
	while ( in[pos].unicode() != '}' )
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

static QString getEscape( const QString& in, int& pos )
{
    switch ( in[pos].unicode() ) {
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
	// double backslash is turned into &#92; so that pass 2 leaves it alone
	pos++;
	return QString( "&#92;" );
    default:
	return QChar( '\\' );
    }
}

/*
  The DocParser class is an internal class that implements the first pass of
  doc parsing.  (See Doc::finalHtml() for the second pass.)
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

    Location& location();

    void setKind( Doc::Kind kind, const QString& thanksToCommand );
    void setKindHasToBe( Doc::Kind kind, const QString& thanksToCommand );

    Doc::Kind kindIs;
    Doc::Kind kindHasToBe;
    QString clueCommand;

    QStringList getStringList();
    bool somethingAhead();
    void startPreOutput();
    void stopPreOutput();

    Location yyLoc;
    int yyLocPos;

    QString yyIn;
    int yyPos;
    int yyLen;
    QString yyOut;
    bool yyInPreOutput;
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
    yyInPreOutput = FALSE;

    QString arg, brief;
    QString className, enumName, extName, fileName, groupName, moduleName;
    QString title, heading, prototype, relates, x;
    StringSet groups, headers, parameters, keywords;
    QStringList seeAlso, important, anamesToPrepend;
    bool obsolete = FALSE;
    int briefBegin = -1;
    int briefEnd = 0;
    int mustquoteBegin = -1;
    uint mustquoteEnd = 0;
    bool internal = FALSE;
    bool overloads = FALSE;
    int numBugs = 0;
    bool metNL = FALSE; // never met N.L.
    int begin, end;

    while ( yyPos < yyLen ) {
	QChar ch = yyIn[yyPos++];
	metNL = ( ch == '\n' );

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
	      We use poor man's hashing to identify the qdoc commands (e.g.,
	      '\a', '\class', '\enum').  These commands are not frequent enough
	      to justify advanced techniques, and it turns out that we can let
	      the C++ compiler do the job for us by means of a mega-switch and
	      simple hashing.

	      If you have to insert a new command to qdoc, here's one of the
	      two places to do it.  In the unlikely event that you have a hash
	      collision (that is, two commands start with the same letter and
	      have the same length), handle it like '\endcode', '\endlink', and
	      '\example' below.

	      A second pass of processing will take care of the last-minute
	      details.  See Doc::finalHtml().
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
		arg = getArgument( yyIn, yyPos );
		if ( arg.isEmpty() ) {
		    warning( 2, location(),
			     "Expected variable name after '\\a'" );
		} else {
		    QString toks = arg;
		    toks.replace( unfriendly, QChar(' ') );
		    QStringList tokl = QStringList::split( QChar(' '), toks );
		    while ( !tokl.isEmpty() ) {
			if ( tokl.first()[0].isLetter() )
			    parameters.insert( tokl.first() );
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
		consume( "bug" );
		if ( numBugs == 0 ) {
		    stopPreOutput();
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
	    case hash( 'b', 7 ):
		consume( "base256" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( fileName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected file name after '\\base256'" );
		} else {
		    yyOut = yyIn.mid( yyPos );
		    setKind( Doc::Base256, command );
		}
		break;
	    case hash( 'c', 1 ):
		consume( "c" );
		arg = getArgument( yyIn, yyPos );
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
		arg = getArgument( yyIn, yyPos );
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
		skipRestOfLine( yyIn, yyPos );
		setKind( Doc::Enum, command );
		break;
	    case hash( 'e', 7 ):
		if ( command.length() != 7 )
		    break;
		if ( command[3] == QChar('c') ) {
		    consume( "endcode" );
		    warning( 2, location(), "Unexpected '\\endcode'" );
		} else if ( command[3] == QChar('l') ) {
		    consume( "endlink" );
		    // we've found the missing link: Eirik Aavitsland
		    warning( 2, location(), "Missing '\\link'" );
		} else {
		    consume( "example" );
		    fileName = getWord( yyIn, yyPos );
		    skipRestOfLine( yyIn, yyPos );
		    setKind( Doc::Example, command );
		}
		break;
	    case hash( 'e', 9 ):
		consume( "extension" );
		extName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( extName.isEmpty() )
		    warning( 2, location(),
			     "Expected module name after '\\extension'" );
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
	    case hash( 'h', 6 ):
		consume( "header" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( fileName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected file name after '\\header'" );
		} else {
		    headers.insert( fileName );
		    setKindHasToBe( Doc::Class, command );
		}
		break;
	    case hash( 'h', 7 ):
		consume( "heading" );
		heading = getRestOfLine( yyIn, yyPos ).simplifyWhiteSpace();
		break;
	    case hash( 'i', 7 ):
		consume( "ingroup" );
		groupName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( groupName.right(5) == QString(".html") ) {
		    warning( 3, location(),
			     "Group name after '\\ingroup' should not end with"
			     " '.html'" );
		    groupName.truncate( groupName.length() - 5 );
		}
		if ( groupName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected group name after '\\ingroup'" );
		} else {
		    groups.insert( groupName );
		    setKindHasToBe( Doc::Class, command );
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
		  The <a name="..."> for '\page's is put right here, because a
		  page can contain many topics.  Otherwise, no new
		  <a name="..."> is created; the link given by setLink() is
		  used.
		*/
		if ( kindIs == Doc::Page )
		    yyOut += QString( "<a name=\"%1\"></a>" ).arg( x );
		break;
	    case hash( 'l', 4 ):
		if ( command.length() != 4 )
		    break;
		if ( command[3] == QChar('e') ) {
		    consume( "line" );
		    warning( 2, location(),
			     "Command '%s' is obsolete, use '%s'",
			     "\\line", "\\printline" );
		    startPreOutput();
		    yyOut += QString( "\\printline" );
		} else {
		    consume( "link" );
		    begin = yyPos;
		    end = yyIn.find( QString("\\endlink"), yyPos );
		    if ( end == -1 ) {
			warning( 2, location(), "Missing '\\endlink'" );
		    } else {
			yyOut += QString( "\\link" );
			yyOut += yyIn.mid( begin, end - begin );
			yyOut += QString( "\\endlink" );
			yyPos = end + 8;
		    }
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
				      " future version of %2.  We strongly"
				      " advise against using it in new"
				      " code.\n" ).arg( what(kindIs) )
						  .arg( config->moduleShort() );
		    metNL = TRUE;
		} else {
		    consume( "overload" );
		    overloads = TRUE;
		    yyOut += QString( "This is an overloaded member function, "
				      "provided for convenience.  It behaves "
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
		check( "printto" );
		startPreOutput();
		break;
	    case hash( 'p', 9 ):
		check( "printline" );
		startPreOutput();
		break;
	    case hash( 'p', 10 ):
		check( "printuntil" );
		startPreOutput();
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
	    case hash( 's', 4 ):
		consume( "skip" );
		warning( 2, location(), "Command '%s' is obsolete, use '%s'",
			 "\\skip", "\\skipto" );
		startPreOutput();
		yyOut += QString( "\\skipto" );
		break;
	    case hash( 's', 6 ):
		check( "skipto" );
		startPreOutput();
		break;
	    case hash( 's', 8 ):
		check( "skipline" );
		startPreOutput();
		break;
	    case hash( 's', 9 ):
		check( "skipuntil" );
		startPreOutput();
		break;
	    case hash( 't', 5 ):
		consume( "title" );
		title = getRestOfLine( yyIn, yyPos ).simplifyWhiteSpace();
		break;
	    case hash( 'u', 5 ):
		consume( "until" );
		warning( 2, location(), "Command '%s' is obsolete, use '%s'",
			 "\\until", "\\printuntil" );
		startPreOutput();
		yyOut += QString( "\\printuntil" );
		break;
	    case hash( 'w', 7 ):
		consume( "warning" );
		yyOut += QString( "<b>Warning:</b>" );
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
		    if ( ch == '>' && yyIn.mid(yyPos - 6, 6) == QString("</pre>") )
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
		if ( metNL && ch == QChar('\n') && !somethingAhead() ) {
		    stopPreOutput();
		    if ( briefEnd == INT_MAX )
			briefEnd = yyOut.length();
		    yyOut += QString( "<p>" );
		    metNL = FALSE;
		}
	    }
	}
    }
    if ( numBugs > 0 )
	yyOut += QString( "</ul>" );

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
	doc = new FnDoc( loc, yyOut, prototype, relates, parameters,
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
		    " <b>Qt %1 Extension</b>, which can be found in the"
		    " <tt>qt/extensions</tt> directory.  It is not included in"
		    " the main Qt API.\n<p>").arg(extName) );

	doc = new ClassDoc( loc, yyOut, className, brief, moduleName, extName,
			    groups, headers, important );
	break;
    case Doc::Enum:
	sanitize( enumName );
	doc = new EnumDoc( loc, yyOut, enumName );
	break;
    case Doc::Page:
	sanitize( fileName );
	doc = new PageDoc( loc, yyOut, fileName, title, heading );
	break;
    case Doc::Base64:
	sanitize( fileName );
	doc = new Base64Doc( loc, yyOut, fileName );
	break;
    case Doc::Base256:
	sanitize( fileName );
	doc = new Base256Doc( loc, yyOut, fileName );
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
    if ( mustquoteBegin >= 0 )
	doc->setHtmlMustQuote( yyOut.mid(mustquoteBegin,
					 mustquoteEnd - mustquoteBegin)
				    .stripWhiteSpace() );
    return doc;
}

Location& DocParser::location()
{
    while ( yyLocPos < yyPos )
	yyLoc.advance( yyIn[yyLocPos++].latin1() );
    return yyLoc;
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
	    stringl.append( yyIn.mid(begin, end - begin) );
	skipSpacesOrNL( yyIn, yyPos );
    }
    return stringl;
}

bool DocParser::somethingAhead()
{
    int inPos0 = yyPos;
    bool something = FALSE;

    skipSpaces( yyIn, yyPos );
    if ( yyPos < yyLen - 4 ) {
	QString lookahead = yyIn.mid( yyPos, 4 );
	if ( lookahead == QString("<li>") || lookahead == QString("\\bug") )
	    something = TRUE;
    }
    yyPos = inPos0;
    return something;
}

void DocParser::startPreOutput()
{
    if ( !yyInPreOutput ) {
	yyOut += QString( "<pre>" );
	yyInPreOutput = TRUE;
    }
}

void DocParser::stopPreOutput()
{
    if ( yyInPreOutput ) {
	yyOut += QString( "</pre>" );
	yyInPreOutput = FALSE;
    }
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

    QString t( "(?:<pre>.*</pre>|(?:Qmagicwordthatyoushouldavoid" );
    QMap<QString, QString>::ConstIterator s = keywordLinks.begin();
    while ( s != keywordLinks.end() ) {
	if ( s == keywordLinks.end() )
	    break;
	t += QChar( '|' );
	t += s.key();
	++s;
    }
    t.replace( QRegExp(QChar(' ')), QString("[ \t\n]+") );
    t.prepend( QString("[ \t\n>]") );
    t.append( QString(")\\b)") );

    delete megaRegExp;
    megaRegExp = new QRegExp( t );
    megaRegExp->setMinimal( TRUE );
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
    if ( y.length() == t.length() ) {
	QString k = keywordLinks[t];
	if ( !k.isEmpty() )
	    return QString( "<a href=\"%1\">%2</a>" ).arg( k ).arg( t );
    }
    return y;
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
      The problem here is to transform a list of classes into a five-column
      table, with the constraint that all classes starting by the same letter
      should appear in the same column.
    */

    const int NumParagraphs = 27; // 26 letters in Alphabits plus tax
    const int NumColumns = 5; // number of columns in the result
    QString html( "" );

    if ( clist.isEmpty() )
	return html;

    /*
      First, find out the common prefix of all classes.  For Qt, the prefix is
      Q.  It can easily be derived from the first and last classes in
      alphabetical order (QAccel and QXtWidget in Qt 2.1).
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
      Now, divide the data into 27 paragraphs: A, B, ..., Z, misc.  QAccel will
      fall in paragraph 0 (A) and QXtWidget in paragraph 23 (X).  This is the
      only place where we assume that NumParagraphs is 27.

      Each paragraph is a QMap<QString, QString>.  The entry for QAccel is the
      pair (accel, QAccel).  The entry for QNPlugin is (nplugin, QNPlugin*).
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
      Each paragraph j has a size: paragraph[j].count().  In the discussion, we
      will assume paragraphs 0 to 5 will have sizes 3, 1, 4, 1, 5, 9,
      respectively.

      We now want to compute the paragraph offset.  Paragraphs 0 to 6 start at
      offsets 0, 3, 4, 8, 9, 14, 23, respectively.
    */
    int paragraphOffset[NumParagraphs + 1];
    int i, j, k;

    paragraphOffset[0] = 0;
    for ( j = 0; j < NumParagraphs; j++ )
	paragraphOffset[j + 1] = paragraphOffset[j] + paragraph[j].count();

    /*
      Here comes the dynamic programming algorithm that does the job.  The
      number of columns is fixed (NumColumns).  We want to minimize the number
      of rows of the biggest column (the cost).  We will build two tables,
      prevEnd and numRows, such that the following condition holds:  When
      paragraphs 0, 1, ..., j in i columns, column (i - 1) should end with
      paragraph prevEnd[i][j].  Furthermore, numRows[i][j] should give the
      number of rows of the biggest column.

      For column 0, there is no previous column, so prevEnd[0][j] is
      artificially set to -1.  This value is highly magical, as it allows other
      computations to work well; -2 wouldn't do.

      If only one paragraph (paragraph 0) is split among i columns, it is
      artificially put completely in column 0.

      By doing this kind of work now, we unify the rest of the algorithm.
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
      Everything is, at last, set up properly for the real work.  For any
      (i columns, j paragraphs) pair, we never use information on the right or
      below in the prevEnd and numRows matrices, as this information is not
      filled in yet.
    */
    for ( i = 1; i < NumColumns; i++ ) {
	for ( j = 1; j < NumParagraphs; j++ ) {
	    /*
	      Let's concentrate on a single (i columns, j paragraphs) pair; that
	      is, how to break j paragraphs into i columns and minimize the
	      number of rows.  We already know how to solve the related
	      (i columns, j - 1 paragraphs) problem: end column i - 1 at
	      prevEnd[i][j - 1].  If we add one paragraph, it might turn out
	      that prevEnd[i][j - 1] is also the right place to end column
	      i - 1.  But maybe column prevEnd[i][j - 1] + 1 is a better place,
	      or maybe prevEnd[i][j - 1] + 2, or even column j - 1.  We'll try
	      them in order, but we'll stop as soon as the situation is not
	      improving anymore.
	    */
	    numRows[i][j] = INT_MAX;

	    for ( k = prevEnd[i][j - 1]; k < j; k++ ) {
		/*
		  What's the cost of breaking the column just after paragraph k?
		  It's whichever is largest between the cost of breaking
		  paragraphs 0, 1, ..., k in i - 1 columns and the cost of the
		  last column.
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
      Finally, start at prevEnd[NumColumns - 1][NumParagraphs - 1] and find our
      way back home.  The information we derive is put into firstOffset, which
      tells where each column should start.  We also initialize currentOffset,
      currentParagraphNo, and prevParagraphNo by the same occasion; they will be
      useful later.
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
      At this point, Professor Bellman leaves us and Finn Arild Aasheim comes
      in.  Seriously, we have to generate all columns in parallel.  The offset
      array guides us.
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
      We fight hard just to go through the QMap in case-insensitive order.  In
      Qt, this gets class Qt among the t's and Quebec among the u's.
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
	html += QString( "<td><b>%1</a></b>" ).arg( href(c.key()) );
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

Doc::Doc( Kind kind, const Location& loc, const QString& htmlText )
    : ki( kind ), lo( loc ), html( htmlText ), inter( FALSE ), obs( FALSE )
{
}

void Doc::setLink( const QString& link, const QString& title )
{
    QString kwordLnk;

    if ( !q.isEmpty() ) {
	quotes[q].insert( link, title );
	q = QString::null;
    }

    /*
      If there are '\keyword' commands here, find out their full addresses.
     */
    if ( !kwords.isEmpty() ) {
	int k = link.find( QChar('#') );
	if ( k == -1 )
	    k = link.length();

	StringSet::ConstIterator s = kwords.begin();
	while ( s != kwords.end() ) {
	    if ( kind() == Page )
		kwordLnk = link.left( k ) + QChar( '#' ) + indexAnchor( *s );
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
      Rainer M. Schmid suggested that auto-referential links should be removed
      automatically from '\sa'.  This is to ease his typing if f1() refers to
      f2() and f3(); f2() to f1() and f3(); and f3() to f1() and f2().  He then
      copies and pastes '\sa f1() f2() f3()'.
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
    lnk = link;
}

bool Doc::changedSinceLastRun() const
{
    return resolver()->changedSinceLastRun( lnk, html );
}

QString Doc::htmlSeeAlso() const
{
    QValueStack<QString> seps = separators( sa.count(), QString(".\n") );
    QString html( "<p>See also " );

    QStringList::ConstIterator s = sa.begin();
    while ( s != sa.end() ) {
	QString name = *s;
	QString text;

	if ( name.left(5) == QString("\\link") ) {
	    QStringList toks =
		    QStringList::split( QChar(' '),
					name.mid(5).stripWhiteSpace() );
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
	if ( y.length() == text.length() && text.left(2) != QString("<a") )
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
	  This does not belong here.  It's an easy solution to problems caused
	  by '\important'.
	*/
	QString fileName = lnk;
	int k = fileName.find( QChar('#') );
	if ( k != -1 )
	    fileName.truncate( k );

	if ( fileName == out.fileName() )
	    resolver()->warnChangedSinceLastRun( location(), lnk, t );
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

    Walkthrough walkthrough;
    QString fileName;
    QString link;
    QString name, ahref;
    QString substr;
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
		command = QChar( ' ' );

	    /*
	      If you have to insert a new command to qdoc, here's the other
	      place to do it.
	    */

	    int h = hash( command[0].unicode(), command.length() );
	    bool consumed = FALSE;

	    switch ( h ) {
	    case hash( ' ', 1 ):
		consume( " " );
		yyOut += QChar( '\\' );
		break;
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
	    case hash( 'd', 11 ):
		consume( "dontinclude" );
		fileName = getWord( yyIn, yyPos );
		skipRestOfLine( yyIn, yyPos );

		if ( fileName.isEmpty() )
		    warning( 2, location(),
			     "Expected file name after '\\dontinclude'" );
		else
		    walkthrough.dontstart( fileName, resolver() );
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
		skipRestOfLine( yyIn, yyPos );

		if ( fileName.isEmpty() ) {
		    warning( 2, location(),
			     "Expected file name after '\\include'" );
		} else {
		    yyOut += QString( "<pre>" );
		    yyOut += walkthrough.start( fileName, resolver() );
		    yyOut += QString( "</pre>" );
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
		yyOut += walkthrough.printto( substr, location() );
		break;
	    case hash( 'p', 9 ):
		consume( "printline" );
		substr = getRestOfLine( yyIn, yyPos );
		yyOut += walkthrough.printline( substr, location() );
		break;
	    case hash( 'p', 10 ):
		consume( "printuntil" );
		substr = getRestOfLine( yyIn, yyPos );
		yyOut += walkthrough.printuntil( substr, location() );
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
		walkthrough.skipto( substr, location() );
		break;
	    case hash( 's', 8 ):
		consume( "skipline" );
		substr = getRestOfLine( yyIn, yyPos );
		walkthrough.skipline( substr, location() );
		break;
	    case hash( 's', 9 ):
		consume( "skipuntil" );
		substr = getRestOfLine( yyIn, yyPos );
		walkthrough.skipuntil( substr, location() );
		break;
	    case hash( 'v', 7 ):
		consume( "version" );
		yyOut += config->version();
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
		  It's always a good idea to include the '()', as it provides
		  some typing (in at least two senses of the word).
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

		if ( clist.contains(t) && t != config->moduleShort() &&
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
      Time to add some automatic hrefs.  Add links to class names and to
      '\keyword's.
    */
    if ( megaRegExp != 0 ) {
	int k = 0;
	while ( (k = megaRegExp->search(yyOut, k)) != -1 ) {
	    /*
	      Make sure we didn't match a '<pre>...</pre>' thingy, but rather
	      skip over it.  (See the construction of megaRegExp.)
	    */
	    if ( yyOut[k + 1] != QChar('<') ) {
		QString t = megaRegExp->cap( 0 ).mid( 1 ).simplifyWhiteSpace();

		/*
		  Insert a href, but rule out two cases:  (1) The current link
		  is foo.html and the '\keyword' entry is at foo.html#big-mice;
		  (2) The current doc and the entry are both at
		  foo.html#printBar.
		*/
		if ( lnk != keywordLinks[t].left(lnk.length()) &&
		     offsetOK(&offsetMap, yyOut.length(), t) ) {
		    yyOut.replace( k + 1, t.length(),
				   QString("<a href=\"%1\">%2</a>")
				   .arg(keywordLinks[t]).arg(t) );
		}
	    }
	    k += megaRegExp->matchedLength() + 1;
	}
    }

    /*
      Complain before it's too late.
    */
    if ( !q.isEmpty() ) {
	warning( 1, location(), "Ignored '\\mustquote' (fix qdoc)" );
    } else if ( !kwords.isEmpty() ) {
	warning( 1, location(), "Ignored '\\keyword' (fix qdoc)" );
    }

    return yyOut;
}

FnDoc::FnDoc( const Location& loc, const QString& html,
	      const QString& prototype, const QString& relates,
	      const StringSet& parameters, bool overloads )
    : Doc( Fn, loc, html ), proto( prototype ), rel( relates ),
      params( parameters ), over( overloads )
{
}

ClassDoc::ClassDoc( const Location& loc, const QString& html,
		    const QString& className, const QString& brief,
		    const QString& module, const QString& extension,
		    const StringSet& groups, const StringSet& headers,
		    const QStringList& important )
    : Doc( Class, loc, html ), cname( className ), bf( brief ), mod( module ),
      ext( extension ), ingroups( groups ), h( headers ), imp( important )
{
    if ( !ext.isEmpty() ) {
	extlist.insert( ext );
	classext.insert( className, ext );
    }

    /*
      Derive the what's this from the '\brief' text (e.g., "The QFoo class is a
      bar class." becomes "A bar class").

      A Qt 3.0 regular expression could do all of that with five lines of code.
      Unfortunately, when this code was written, Qt 3.0 QRegExp was not yet
      available.
    */
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
}

EnumDoc::EnumDoc( const Location& loc, const QString& html,
		  const QString& enumName )
    : Doc( Enum, loc, html ), ename( enumName )
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
    : PageLikeDoc( Page, loc, html, title, heading ), fname( fileName )
{
}

Base64Doc::Base64Doc( const Location& loc, const QString& html,
		      const QString& fileName )
    : Doc( Base64, loc, html ), fname( fileName )
{
}

void Base64Doc::print( BinaryWriter& out )
{
    out.putsBase64( htmlData().latin1() );
}

Base256Doc::Base256Doc( const Location& loc, const QString& html,
			const QString& fileName )
    : Doc( Base256, loc, html ), fname( fileName )
{
}

void Base256Doc::print( BinaryWriter& out )
{
    out.putsBase256( htmlData().latin1() );
}

DefgroupDoc::DefgroupDoc( const Location& loc, const QString& html,
			  const QString& groupName, const QString& title,
			  const QString& heading )
    : PageLikeDoc( Defgroup, loc, html, title, heading ),
      gname( groupName )
{
}

ExampleDoc::ExampleDoc( const Location& loc, const QString& html,
			const QString& fileName, const QString& title,
			const QString& heading )
    : PageLikeDoc( Example, loc, html, title, heading ), fname( fileName )
{
}
