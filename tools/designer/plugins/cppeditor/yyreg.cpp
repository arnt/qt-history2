/*
  yyreg.cpp

  This is a parser for Reginald Stadlbauer. It borrows many things
  from qdoc.
*/

#include <qregexp.h>

#include <ctype.h>
#include <stdio.h>

#include "yyreg.h"

/*
  First the tokenizer. We need something that knows not very much
  about C++.
*/

enum { Tok_Eoi, Tok_Ampersand, Tok_Aster, Tok_LeftParen, Tok_RightParen,
       Tok_Equal, Tok_LeftBrace, Tok_RightBrace, Tok_Semicolon, Tok_Colon,
       Tok_LeftAngle, Tok_RightAngle, Tok_Comma, Tok_Ellipsis, Tok_Gulbrandsen,
       Tok_LeftBracket, Tok_RightBracket, Tok_Tilde, Tok_SomeOperator,
       Tok_Number, Tok_String, Tok_Comment, Tok_Ident,

       Tok_char, Tok_const, Tok_double, Tok_int, Tok_long, Tok_operator,
       Tok_short, Tok_signed, Tok_unsigned };

static QString yyIn;
static int yyPos;
static int yyCurPos;
static char yyLex[65536]; // big enough for long comments (unlike this one)
static int yyLexLen;
static int yyLineNo;
static int yyCurLineNo;
static int yyCh;

static inline void readChar()
{
    if ( yyCh == EOF )
	return;

    if ( yyLexLen < (int) sizeof(yyLex) + 1 ) {
	yyLex[yyLexLen++] = (char) yyCh;
	yyLex[yyLexLen] = '\0';
    }

    if ( yyCurPos == (int) yyIn.length() ) {
	yyCh = EOF;
    } else {
	yyCh = yyIn[yyCurPos++].unicode();
	if ( yyCh == '\n' )
	    yyLineNo++;
    }
}

static void startTokenizer( const QString& in )
{
    yyIn = in;
    yyPos = 0;
    yyCurPos = 0;
    yyLex[0] = '\0';
    yyLexLen = 0;
    yyLineNo = 0;
    yyCurLineNo = 0;
    yyCh = '\0';
    readChar();
}

#define HASH( ch, len ) ( (ch) | ((len) << 8) )
#define CHECK( target ) \
    if ( strcmp((target), yyLex) != 0 ) \
	break;

static int getToken()
{
    while ( TRUE ) {
	yyPos = yyCurPos - 1; // yyCurPos is one character ahead
	yyLex[0] = '\0';
	yyLexLen = 0;
	yyLineNo = yyCurLineNo;

	if ( yyCh == EOF ) {
	    break;
	} else if ( isspace(yyCh) ) {
	    do {
		readChar();
	    } while ( isspace(yyCh) );
	} else if ( isalpha(yyCh) || yyCh == '_' ) {
	    do {
		readChar();
	    } while ( isalnum(yyCh) || yyCh == '_' );

	    switch ( HASH(yyLex[0], yyLexLen) ) {
	    case HASH( 'c', 4 ):
		CHECK( "char" );
		return Tok_char;
	    case HASH( 'c', 5 ):
		CHECK( "const" );
		return Tok_const;
	    case HASH( 'd', 6 ):
		CHECK( "double" );
		return Tok_double;
	    case HASH( 'i', 3 ):
		CHECK( "int" );
		return Tok_int;
	    case HASH( 'l', 4 ):
		CHECK( "long" );
		return Tok_long;
	    case HASH( 'o', 8 ):
		CHECK( "operator" );
		return Tok_operator;
	    case HASH( 's', 5 ):
		CHECK( "short" );
		return Tok_short;
	    case HASH( 's', 6 ):
		CHECK( "signed" );
		return Tok_signed;
	    case 'u':
		CHECK( "unsigned" );
		return Tok_unsigned;
	    }
	    return Tok_Ident;
	} else {
	    switch ( yyCh ) {
	    case '!':
	    case '%':
	    case '^':
		readChar();
		if ( yyCh == '=' )
		    readChar();
		return Tok_SomeOperator;
	    case '"':
		readChar();

		while ( yyCh != EOF && yyCh != '"' ) {
		    if ( yyCh == '\\' )
			readChar();
		    readChar();
		}
		readChar();
		return Tok_String;
		break;
	    case '#':
		while ( yyCh != EOF && yyCh != '\n' ) {
		    if ( yyCh == '\\' )
			readChar();
		    readChar();
		}
		break;
	    case '&':
		readChar();
		if ( yyCh == '&' || yyCh == '=' ) {
		    readChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_Ampersand;
		}
	    case '\'':
		readChar();
		if ( yyCh == '\\' )
		    readChar();
		do {
		    readChar();
		} while ( yyCh != EOF && yyCh != '\'' );

		readChar();
		return Tok_Number;
	    case '(':
		readChar();
		return Tok_LeftParen;
	    case ')':
		readChar();
		return Tok_RightParen;
	    case '*':
		readChar();
		if ( yyCh == '=' ) {
		    readChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_Aster;
		}
	    case '+':
		readChar();
		if ( yyCh == '+' || yyCh == '=' )
		    readChar();
		return Tok_SomeOperator;
	    case ',':
		readChar();
		return Tok_Comma;
	    case '-':
		readChar();
		if ( yyCh == '-' || yyCh == '=' ) {
		    readChar();
		} else if ( yyCh == '>' ) {
		    readChar();
		    if ( yyCh == '*' )
			readChar();
		}
		return Tok_SomeOperator;
	    case '.':
		readChar();
		if ( yyCh == '*' ) {
		    readChar();
		} else if ( yyCh == '.' ) {
		    do {
			readChar();
		    } while ( yyCh == '.' );
		    return Tok_Ellipsis;
		} else if ( isdigit(yyCh) ) {
		    do {
			readChar();
		    } while ( isalnum(yyCh) || yyCh == '.' || yyCh == '+' ||
			      yyCh == '-' );
		    return Tok_Number;
		}
		return Tok_SomeOperator;
	    case '/':
		readChar();
		if ( yyCh == '/' ) {
		    do {
			readChar();
		    } while ( yyCh != EOF && yyCh != '\n' );
		} else if ( yyCh == '*' ) {
		    bool metAster = FALSE;
		    bool metAsterSlash = FALSE;

		    readChar();

		    while ( !metAsterSlash ) {
			if ( yyCh == EOF )
			    break;

			if ( yyCh == '*' )
			    metAster = TRUE;
			else if ( metAster && yyCh == '/' )
			    metAsterSlash = TRUE;
			else
			    metAster = FALSE;
			readChar();
		    }
		    return Tok_Comment;
		} else {
		    if ( yyCh == '=' )
			readChar();
		    return Tok_SomeOperator;
		}
		break;
	    case ':':
		readChar();
		if ( yyCh == ':' ) {
		    readChar();
		    return Tok_Gulbrandsen;
		} else {
		    return Tok_Colon;
		}
	    case ';':
		readChar();
		return Tok_Semicolon;
	    case '<':
		readChar();
		if ( yyCh == '<' ) {
		    readChar();
		    if ( yyCh == '=' )
			readChar();
		    return Tok_SomeOperator;
		} else if ( yyCh == '=' ) {
		    readChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_LeftAngle;
		}
	    case '=':
		readChar();
		if ( yyCh == '=' ) {
		    readChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_Equal;
		}
	    case '>':
		readChar();
		if ( yyCh == '>' ) {
		    readChar();
		    if ( yyCh == '=' )
			readChar();
		    return Tok_SomeOperator;
		} else if ( yyCh == '=' ) {
		    readChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_RightAngle;
		}
	    case '?':
		readChar();
		return Tok_SomeOperator;
	    case '[':
		readChar();
		return Tok_LeftBracket;
	    case '\\':
		readChar();
		readChar(); // skip one character
		break;
	    case ']':
		readChar();
		return Tok_RightBracket;
	    case '{':
		readChar();
		return Tok_LeftBrace;
	    case '}':
		readChar();
		return Tok_RightBrace;
	    case '|':
		readChar();
		if ( yyCh == '|' || yyCh == '=' )
		    readChar();
		return Tok_SomeOperator;
	    case '~':
		readChar();
		return Tok_Tilde;
	    default:
		readChar();
	    }
	}
    }
    return Tok_Eoi;
}

/*
  A few member functions of Function.
*/

CppFunction::CppFunction( const CppFunction& f )
    : ret( f.ret ), nam( f.nam ), params( f.params ), cnst( f.cnst ),
      bod( f.bod ), doc( f.doc )
{
}

CppFunction& CppFunction::operator=( const CppFunction& f )
{
    ret = f.ret;
    nam = f.nam;
    params = f.params;
    cnst = f.cnst;
    bod = f.bod;
    doc = f.doc;
    return *this;
}

QString CppFunction::prototype() const
{
    QString proto;

    if ( !returnType().isEmpty() )
	proto = returnType() + QChar( ' ' );
    proto += scopedName();
    proto += QChar( '(' );
    if ( !parameterList().isEmpty() ) {
	QStringList::ConstIterator p = parameterList().begin();
	proto += *p;
	++p;
	while ( p != parameterList().end() ) {
	    proto += QString( ", " );
	    proto += *p;
	    ++p;
	}
    }
    proto += QChar( ')' );
    if ( isConst() )
	proto += QString( " const" );
    return proto;
}

/*
  The parser follows. We are not really parsing C++, just trying to
  find the start and end of function definitions. One pitfall is that
  the parsed code needs not be valid. In particular, braces might be
  unbalanced.
*/

static int yyTok;

static QString matchTemplateAngles()
{
    QString t;

    if ( yyTok == Tok_LeftAngle ) {
	int depth = 0;
	do {
	    if ( yyTok == Tok_LeftAngle )
		depth++;
	    else if ( yyTok == Tok_RightAngle )
		depth--;
	    t += yyLex;
	    yyTok = getToken();
	} while ( depth > 0 && yyTok != Tok_Eoi );
    }
    return t;
}

enum VariableNamePolicy { DontParseVarNames, StripVarNames, KeepVarNames };

static void appendToType( QString *type, const QString& trailer )
{
    if ( !type->isEmpty() && !trailer.isEmpty() ) {
	QString newType = *type;
	if ( newType[(int)newType.length() - 1].isLetter() ) {
	    if ( trailer[0].isLetter() || trailer[0] == QChar('*') ||
		 trailer[0] == QChar('&') )
		*type += QChar( ' ' );
	}
    }
    *type += trailer;
}

/*
  Tries to match a data type and possibly a variable name, and returns
  these. The variable name belongs here because of cases such as
  'char *xpm[]' and 'int (*f)(int)'.
*/
static QString matchDataType( VariableNamePolicy policy )
{
    QString type;

    /*
      This code is really hard to follow... sorry. The loop is there to match
      Alpha::Beta::Gamma::...::Omega.
    */
    while ( TRUE ) {
	bool virgin = TRUE;

	if ( yyTok != Tok_Ident ) {
	    /*
	      People may write 'const unsigned short' or
	      'short unsigned const' or any other permutation.
	    */
	    while ( yyTok == Tok_const ) {
		appendToType( &type, yyLex );
		yyTok = getToken();
	    }
	    while ( yyTok == Tok_signed || yyTok == Tok_unsigned ||
		    yyTok == Tok_short || yyTok == Tok_long ) {
		appendToType( &type, yyLex );
		yyTok = getToken();
		virgin = FALSE;
	    }
	    while ( yyTok == Tok_const ) {
		appendToType( &type, yyLex );
		yyTok = getToken();
	    }

	    if ( yyTok == Tok_Tilde ) {
		appendToType( &type, yyLex );
		yyTok = getToken();
	    }
	}

	if ( virgin ) {
	    if ( yyTok == Tok_Ellipsis || yyTok == Tok_Ident ||
		 yyTok == Tok_char || yyTok == Tok_int ||
		 yyTok == Tok_double ) {
		appendToType( &type, yyLex );
		yyTok = getToken();
	    } else {
		return QString::null;
	    }
	} else if ( yyTok == Tok_int || yyTok == Tok_char ||
		    yyTok == Tok_double ) {
	    appendToType( &type, yyLex );
	    yyTok = getToken();
	}

	appendToType( &type, matchTemplateAngles() );

	while ( yyTok == Tok_const ) {
	    appendToType( &type, yyLex );
	    yyTok = getToken();
	}

	if ( yyTok == Tok_Gulbrandsen ) {
	    appendToType( &type, yyLex );
	    yyTok = getToken();
	} else {
	    break;
	}
    }

    while ( yyTok == Tok_Ampersand || yyTok == Tok_Aster ||
	    yyTok == Tok_const ) {
	appendToType( &type, yyLex );
	yyTok = getToken();
    }

    /*
      Look for an optional identifier, then for some array brackets.
      We don't recognize pointers to functions.
    */
    if ( policy != DontParseVarNames ) {
	if ( yyTok == Tok_Ident ) {
	    if ( policy == KeepVarNames )
		appendToType( &type, yyLex );
	    yyTok = getToken();
	}

	while ( yyTok == Tok_LeftBracket ) {
	    do {
		yyTok = getToken();
		appendToType( &type, yyLex );
	    } while ( yyTok != Tok_Eoi && yyTok != Tok_RightBracket );
	    yyTok = getToken();
	    appendToType( &type, yyLex );
	}
    }
    return type;
}

static bool isCtorOrDtor( const QString& thingy )
{
    // e.g., Alpha<a>::Beta<Bar<b, c> >::~Beta
    static QRegExp xtor( QString(
	    "(?:([A-Z_a-z][0-9A-Z_a-z]*)" // class name
	       "(?:<(?:[^>]|<[^>]*>)*>)*" // template arguments
	       "::)+"                     // many in a row
	    "~?"                          // ctor or dtor?
	    "\\1") );                     // function has same name as class
    return xtor.exactMatch( thingy );
}

static CppFunction matchFunctionPrototype( bool stripParamNames )
{
    CppFunction func;
    QString returnType;
    QString scopedName;
    QStringList params;
    bool cnst = FALSE;
    QString qualifier;

    if ( yyTok == Tok_Comment )
	yyTok = getToken();

    returnType = matchDataType( DontParseVarNames );
    if ( returnType.isEmpty() )
	return func;

    if ( isCtorOrDtor(returnType) ) {
	scopedName = returnType;
	returnType = QString::null;
    } else if ( yyTok == Tok_Ident ) {
	scopedName = yyLex;
	yyTok = getToken();

	scopedName += matchTemplateAngles();

	while ( yyTok == Tok_Gulbrandsen ) {
	    scopedName += yyLex;
	    yyTok = getToken();

	    bool isOperator = ( yyTok == Tok_operator );
	    scopedName += yyLex;
	    yyTok = getToken();

	    if ( isOperator ) {
		while ( yyTok != Tok_Eoi && yyTok != Tok_LeftParen ) {
		    scopedName += yyLex;
		    yyTok = getToken();
		}
	    }
	}
    } else {
	return func;
    }

    if ( yyTok != Tok_LeftParen )
	return func;
    yyTok = getToken();

    if ( yyTok != Tok_RightParen ) {
	while ( TRUE ) {
	    QString param = matchDataType( stripParamNames ? StripVarNames
					   : KeepVarNames );
	    if ( param.isEmpty() )
		return func;
	    params.append( param );

	    if ( yyTok != Tok_Comma )
		break;

	    yyTok = getToken();
	}
	if ( yyTok != Tok_RightParen )
	    return func;
    }
    yyTok = getToken();

    if ( yyTok == Tok_const ) {
	cnst = TRUE;
	yyTok = getToken();
    }

    if ( yyTok == Tok_Colon ) {
	while ( yyTok != Tok_LeftBrace && yyTok != Tok_Eoi )
	    yyTok = getToken();
    }

    func.setReturnType( returnType );
    func.setScopedName( scopedName );
    func.setParameterList( params );
    func.setConst( cnst );
    return func;
}

static void setBody( CppFunction *func, const QString& somewhatBody )
{
    QString body = somewhatBody;

    int braceDepth = 0;
    int i = 0;
    while ( i < (int) body.length() ) {
	if ( body[i] == QChar('{') ) {
	    braceDepth++;
	} else if ( body[i] == QChar('}') ) {
	    braceDepth--;
	    if ( braceDepth == 0 ) {
		body.truncate( i + 1 );
		break;
	    }
	}
	i++;
    }

    func->setBody( body );
}

static void matchTranslationUnit( QValueList<CppFunction> *flist )
{
    CppFunction pendingFunc;
    int startBody = -1;
    int endBody;

    while ( yyTok != Tok_Eoi ) {
	endBody = yyPos;
	CppFunction func = matchFunctionPrototype( FALSE );
	if ( !func.scopedName().isEmpty() && yyTok == Tok_LeftBrace ) {
	    if ( startBody != -1 ) {
		setBody( &pendingFunc,
			 yyIn.mid(startBody, endBody - startBody) );
		flist->append( pendingFunc );
	    }

	    startBody = yyPos;
	    pendingFunc = func;
	}

	while ( yyTok != Tok_Eoi && yyTok != Tok_RightBrace &&
		yyTok != Tok_Semicolon )
	    yyTok = getToken();
	yyTok = getToken();
    }
    if ( startBody != -1 ) {
	setBody( &pendingFunc, yyIn.mid(startBody) );
	flist->append( pendingFunc );
    }
}

/*
  Extracts C++ function from source code and put them in a list.
*/
void extractCppFunctions( const QString& code, QValueList<CppFunction> *flist )
{
    startTokenizer( code );
    yyTok = getToken();
    matchTranslationUnit( flist );
}

/*
  Returns the prototype with the parameter names removed.
*/
QString canonicalCppProto( const QString& proto )
{
    startTokenizer( proto );
    yyTok = getToken();
    CppFunction func = matchFunctionPrototype( TRUE );
    return func.prototype();
}

/*
  Follows a test driver that is not really part of the parser.
*/
#include <qfile.h>
#include <qtextstream.h>

#include <errno.h>
#include <stdlib.h>

int main( int argc, char **argv )
{
    const char * const tab[] = {
	"int Foo::printFoo(int x, int y) const",
	"void Foo::Bar::printBar(QWidget *z)"
    };

    for ( int i = 0; i < 2; i++ )
	qDebug( "Canonical: %s\n",
		 canonicalCppProto(QString(tab[i])).latin1() );

    if ( argc != 2 ) {
	qWarning( "Usage: yyreg file.cpp" );
	return EXIT_FAILURE;
    }

    QFile f( argv[1] );
    if ( !f.open(IO_ReadOnly) ) {
	qWarning( "Cannot open '%s' for reading: %s", argv[1],
		  strerror(errno) );
	return EXIT_FAILURE;
    }

    QTextStream t( &f );
    QString code = t.read();
    f.close();

    QValueList<CppFunction> flist;
    extractCppFunctions( code, &flist );
    QValueList<CppFunction>::ConstIterator func = flist.begin();
    while ( func != flist.end() ) {
	qDebug( "Function: %s", (*func).prototype().latin1() );
	qDebug( "Body:     %s\n",
		(*func).body().simplifyWhiteSpace().latin1() );
	++func;
    }
    return EXIT_SUCCESS;
}
