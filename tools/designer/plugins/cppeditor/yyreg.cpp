/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qregexp.h>

#include <ctype.h>
#include <stdio.h>

#include "yyreg.h"

/*
  First comes the tokenizer. We don't need something that knows much
  about C++. However, we need something that give tokens from the end
  of the file to the start.
*/

enum { Tok_Boi, Tok_Ampersand, Tok_Aster, Tok_LeftParen, Tok_RightParen,
       Tok_Equal, Tok_LeftBrace, Tok_RightBrace, Tok_Semicolon, Tok_Colon,
       Tok_LeftAngle, Tok_RightAngle, Tok_Comma, Tok_Ellipsis, Tok_Gulbrandsen,
       Tok_LeftBracket, Tok_RightBracket, Tok_Tilde, Tok_Something, Tok_Comment,
       Tok_Ident,

       Tok_char, Tok_const, Tok_double, Tok_int, Tok_long, Tok_operator,
       Tok_short, Tok_signed, Tok_unsigned };

static QString yyIn;
static int yyPos;
static int yyCurPos;
static char yyLexBuf[65536]; // big enough for long comments (unlike this one)
static char *yyLex;
static int yyCh;

static inline void readChar()
{
    if ( yyCh == EOF )
	return;

    if ( yyLex > yyLexBuf )
	*--yyLex = (char) yyCh;

    if ( yyCurPos < 0 )
	yyCh = EOF;
    else
	yyCh = yyIn[yyCurPos].unicode();

    yyCurPos--;
}

static void startTokenizer( const QString& in )
{
    yyIn = in;
    yyPos = yyIn.length() - 1;
    yyCurPos = yyPos;
    yyLex = yyLexBuf + sizeof(yyLexBuf) - 1;
    *yyLex = '\0';
    yyCh = '\0';
    readChar();
}

#define HASH( ch, len ) ( (ch) | ((len) << 8) )
#define CHECK( target ) \
    if ( strcmp((target), yyLex) != 0 ) \
	break;

static int getToken()
{
    yyPos = yyCurPos + 2;

    while ( TRUE ) {
	yyLex = yyLexBuf + sizeof(yyLexBuf) - 1;
	*yyLex = '\0';

	if ( yyCh == EOF ) {
	    break;
	} else if ( isspace(yyCh) ) {
	    do {
		readChar();
	    } while ( isspace(yyCh) );
	} else if ( isalnum(yyCh) || yyCh == '_' ) {
	    do {
		readChar();
	    } while ( isalnum(yyCh) || yyCh == '_' );

	    switch ( HASH(yyLex[0], strlen(yyLex)) ) {
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
	    if ( isdigit(*yyLex) )
		return Tok_Something;
	    else
		return Tok_Ident;
	} else {
	    int quote;

	    switch ( yyCh ) {
	    case '!':
	    case '%':
	    case '^':
	    case '+':
	    case '-':
	    case '?':
	    case '|':
		readChar();
		return Tok_Something;
	    case '"':
	    case '\'':
		quote = yyCh;
		readChar();

		while ( yyCh != EOF ) {
		    if ( yyCh == quote ) {
			readChar();
			if ( yyCh != '\\' )
			    break;
		    } else {
			readChar();
		    }
		}
		return Tok_Something;
	    case '&':
		readChar();
		if ( yyCh == '&' ) {
		    readChar();
		    return Tok_Something;
		} else {
		    return Tok_Ampersand;
		}
	    case '(':
		readChar();
		return Tok_LeftParen;
	    case ')':
		readChar();
		return Tok_RightParen;
	    case '*':
		readChar();
		return Tok_Aster;
	    case ',':
		readChar();
		return Tok_Comma;
	    case '.':
		readChar();
		if ( yyCh == '.' ) {
		    do {
			readChar();
		    } while ( yyCh == '.' );
		    return Tok_Ellipsis;
		} else {
		    return Tok_Something;
		}
	    case '/':
		// we don't treat C++-style comments specially
		readChar();
		if ( yyCh == '*' ) {
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
		    return Tok_Something;
		}
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
		return Tok_LeftAngle;
	    case '=':
		readChar();
		return Tok_Equal;
	    case '>':
		readChar();
		return Tok_RightAngle;
	    case '[':
		readChar();
		return Tok_LeftBracket;
	    case ']':
		readChar();
		return Tok_RightBracket;
	    case '{':
		readChar();
		return Tok_LeftBrace;
	    case '}':
		readChar();
		return Tok_RightBrace;
	    case '~':
		readChar();
		return Tok_Tilde;
	    default:
		readChar();
	    }
	}
    }
    return Tok_Boi;
}

/*
  Follow a few member functions of CppFunction.
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
  find the start and end of function definitions.

  One pitfall is that the parsed code needs not be valid. Parsing
  from right to left helps cope with that.
*/

static int yyTok;

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

static QString matchTemplateAngles()
{
    QString t;

    if ( yyTok == Tok_RightAngle ) {
	int depth = 0;
	do {
	    if ( yyTok == Tok_RightAngle )
		depth++;
	    else if ( yyTok == Tok_LeftAngle )
		depth--;
	    t.prepend( yyLex );
	    yyTok = getToken();
	} while ( depth > 0 && yyTok != Tok_Boi );
    }
    return t;
}

static QString matchArrayBrackets()
{
    QString t;

    while ( yyTok == Tok_RightBracket ) {
	t.prepend( yyLex );
	if ( yyTok == Tok_Something )
	    t.prepend( yyLex );
	if ( yyTok != Tok_LeftBracket )
	    return QString::null;
	yyTok = getToken();
    }
    return t;
}

static void prependToType( QString *type, const QString& prefix )
{
    if ( !type->isEmpty() && !prefix.isEmpty() ) {
	QChar left = prefix[prefix.length() - 1];
	QChar right = (*type)[0];

	// style can be enforced here
	if ( left.isLetter() &&
	     (right.isLetter() || right == QChar('*') || right == QChar('&')) )
	    type->prepend( QChar(' ') );
    }
    type->prepend( prefix );
}

static QString matchDataType()
{
    QString type;

    while ( yyTok == Tok_Ampersand || yyTok == Tok_Aster ||
	    yyTok == Tok_const ) {
	prependToType( &type, yyLex );
	yyTok = getToken();
    }

    /*
      This code is really hard to follow... sorry. The loop is there to match
      Alpha::Beta::Gamma::...::Omega.
    */
    while ( TRUE ) {
	bool virgin = TRUE;

	prependToType( &type, matchTemplateAngles() );

	if ( yyTok != Tok_Ident ) {
	    /*
	      People may write 'const unsigned short' or
	      'short unsigned const' or any other permutation.
	    */
	    while ( yyTok == Tok_const || yyTok == Tok_signed ||
		    yyTok == Tok_unsigned || yyTok == Tok_short ||
		    yyTok == Tok_long ) {
		prependToType( &type, yyLex );
		yyTok = getToken();
		if ( yyTok != Tok_const )
		    virgin = FALSE;
	    }

	    if ( yyTok == Tok_Tilde ) {
		prependToType( &type, yyLex );
		yyTok = getToken();
	    }
	}

	if ( virgin ) {
	    if ( yyTok == Tok_Ellipsis || yyTok == Tok_Ident ||
		 yyTok == Tok_char || yyTok == Tok_int ||
		 yyTok == Tok_double ) {
		prependToType( &type, yyLex );
		yyTok = getToken();
	    } else {
		return QString::null;
	    }
	} else if ( yyTok == Tok_int || yyTok == Tok_char ||
		    yyTok == Tok_double ) {
	    prependToType( &type, yyLex );
	    yyTok = getToken();
	}

	while ( yyTok == Tok_const ) {
	    prependToType( &type, yyLex );
	    yyTok = getToken();
	}

	if ( yyTok == Tok_Gulbrandsen ) {
	    prependToType( &type, yyLex );
	    yyTok = getToken();
	} else {
	    break;
	}
    }
    return type;
}

static CppFunction matchFunctionPrototype( bool stripParamNames )
{
    CppFunction func;
    QString documentation;
    QString returnType;
    QString scopedName;
    QStringList params;
    QString qualifier;
    bool cnst = FALSE;

    if ( yyTok == Tok_const ) {
	cnst = TRUE;
	yyTok = getToken();
    }

    if ( yyTok != Tok_RightParen )
	return func;
    yyTok = getToken();

    if ( yyTok != Tok_LeftParen ) {
	while ( TRUE ) {
	    QString brackets = matchArrayBrackets();
	    QString name;
	    if ( yyTok == Tok_Ident ) {
		name = yyLex;
		yyTok = getToken();
	    }
	    QString type = matchDataType();

	    if ( type.isEmpty() ) {
		if ( name.isEmpty() )
		    return func;
		type = name;
		name = QString::null;
	    }
	    if ( stripParamNames )
		name = QString::null;

	    QString param = type + QChar( ' ' ) + name + brackets;
	    params.prepend( param.stripWhiteSpace() );

	    if ( yyTok != Tok_Comma )
		break;
	    yyTok = getToken();
	}
	if ( yyTok != Tok_LeftParen )
	    return func;
    }
    yyTok = getToken();

    while ( TRUE ) {
	scopedName.prepend( matchTemplateAngles() );

	if ( yyTok != Tok_Ident ) {
	    // the operator keyword should be close
	    int i = 0;
	    while ( i < 4 && yyTok != Tok_operator ) {
		scopedName.prepend( yyLex );
		i++;
	    }
	    if ( yyTok != Tok_operator )
		return func;
	}
	scopedName.prepend( yyLex );
	yyTok = getToken();

	if ( yyTok != Tok_Gulbrandsen )
	    break;
	scopedName.prepend( yyLex );
	yyTok = getToken();
    }

    if ( !isCtorOrDtor(scopedName) ) {
	returnType = matchDataType();
	if ( returnType.isEmpty() )
	    return func;
    }

    if ( yyTok == Tok_Comment ) {
	documentation = yyLex;
	yyTok = getToken();
    }

    func.setDocumentation( documentation );
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
    int endBody = -1;
    int startBody;

    while ( TRUE ) {
	if ( endBody == -1 )
	    endBody = yyPos;

	while ( yyTok != Tok_Boi && yyTok != Tok_LeftBrace )
	    yyTok = getToken();
	if ( yyTok == Tok_Boi )
	    break;

	yyTok = getToken();
	int startBody = yyPos;
	CppFunction func = matchFunctionPrototype( FALSE );
	if ( !func.scopedName().isEmpty() ) {
	    setBody( &func, yyIn.mid(startBody, endBody - startBody) );
	    flist->prepend( func );
	    endBody = -1;
	}
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
