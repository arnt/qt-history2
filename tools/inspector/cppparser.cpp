/*
  yyvolk.cpp

  This is a parser for Volker Hilsheimer. It borrows many things from
  the one for Reggie Stadlbauer, and other things from qdoc.
*/

#include <qfile.h>
#include <qregexp.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "cppparser.h"

/*
  First the basic C++ tokenizer.
*/

enum { Tok_Eof, Tok_Ampersand, Tok_Aster, Tok_LeftParen, Tok_RightParen,
       Tok_Equal, Tok_LeftBrace, Tok_RightBrace, Tok_Semicolon, Tok_Colon,
       Tok_LeftAngle, Tok_RightAngle, Tok_Comma, Tok_Ellipsis, Tok_Gulbrandsen,
       Tok_LeftBracket, Tok_RightBracket, Tok_Tilde, Tok_Number, Tok_Define,
       Tok_Ident,

       Tok_char, Tok_class, Tok_const, Tok_double, Tok_int, Tok_long,
       Tok_private, Tok_protected, Tok_public, Tok_short, Tok_signed,
       Tok_struct, Tok_unsigned, Tok_virtual };

static FILE *yyIn;
static char yyLex[128];
static int yyLexLen;
static int yyLineNo;
static int yyCurLineNo;
static int yyBraceDepth;
static int yyNumber;
static int yyCh;

static inline void readChar()
{
    if ( yyCh == EOF )
	return;

    if ( yyLexLen < (int) sizeof(yyLex) + 1 ) {
	yyLex[yyLexLen++] = (char) yyCh;
	yyLex[yyLexLen] = '\0';
    }

    yyCh = getc( yyIn );
    if ( yyCh == '\n' )
	yyCurLineNo++;
}

static void startTokenizer( FILE *in )
{
    yyIn = in;
    yyLex[0] = '\0';
    yyLexLen = 0;
    yyLineNo = 0;
    yyCurLineNo = 0;
    yyBraceDepth = 0;
    yyNumber = 0;
    yyCh = '\0';
    readChar();
}

#define HASH( ch, len ) ( (ch) | ((len) << 8) )
#define CHECK( target ) \
    if ( strcmp((target), yyLex) != 0 ) \
	break;

static int getToken()
{
    const char *start;

    while ( TRUE ) {
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
		if ( yyLex[1] == 'l' ) {
		    CHECK( "class" );
		    return Tok_class;
		} else {
		    CHECK( "const" );
		    return Tok_const;
		}
	    case HASH( 'd', 6 ):
		CHECK( "double" );
		return Tok_double;
	    case HASH( 'i', 3 ):
		CHECK( "int" );
		return Tok_int;
	    case HASH( 'l', 4 ):
		CHECK( "long" );
		return Tok_long;
	    case HASH( 'p', 6 ):
		CHECK( "public" );
		return Tok_public;
	    case HASH( 'p', 7 ):
		CHECK( "private" );
		return Tok_private;
	    case HASH( 'p', 9 ):
		CHECK( "protected" );
		return Tok_protected;
	    case HASH( 's', 5 ):
		CHECK( "short" );
		return Tok_short;
	    case HASH( 's', 6 ):
		if ( yyLex[1] == 'i' ) {
		    CHECK( "signed" );
		    return Tok_signed;
		} else {
		    CHECK( "struct" );
		    return Tok_struct;
		}
	    case HASH( 'u', 8 ):
		CHECK( "unsigned" );
		return Tok_unsigned;
	    case HASH( 'v', 7 ):
		CHECK( "virtual" );
		return Tok_virtual;
	    }
	    return Tok_Ident;
	} else if ( isdigit(yyCh) ) {
	    do {
		readChar();
	    } while ( isalnum(yyCh) );

	    yyNumber = (int) strtoul( yyLex, (char **) 0, 0 );
	    return Tok_Number;
	} else {
	    switch ( yyCh ) {
	    case '"':
		readChar();

		while ( yyCh != EOF && yyCh != '"' ) {
		    if ( yyCh == '\\' )
			readChar();
		    readChar();
		}
		readChar();
		break;
	    case '#':
		readChar();
		while ( isspace(yyCh) )
		    readChar();

		start = yyLex + yyLexLen;
		while ( isalnum(yyCh) )
		    readChar();
		if ( strcmp(start, "define") == 0 )
		    return Tok_Define;
		break;
	    case '&':
		readChar();
		return Tok_Ampersand;
	    case '\'':
		readChar();
		if ( yyCh == '\\' )
		    readChar();
		do {
		    readChar();
		} while ( yyCh != EOF && yyCh != '\'' );

		readChar();
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
		}
		break;
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
		}
		break;
	    case ':':
		readChar();
		if ( yyCh == ':' ) {
		    readChar();
		    return Tok_Gulbrandsen;
		}
		break;
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
		yyBraceDepth++;
		return Tok_LeftBrace;
	    case '}':
		readChar();
		yyBraceDepth--;
		return Tok_RightBrace;
	    case '~':
		readChar();
		return Tok_Tilde;
	    default:
		readChar();
	    }
	}
    }
    return Tok_Eof;
}

/*
  Here starts the parser.
*/

int yyTok;

static void matchDefineDirective( QMap<QString, QString> *uuidMap )
{
    static QRegExp uuidDefine( QString("[^_]*_([0-9A-Z_a-z]+)") );
    static const int numDigits[11] = {
	8, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2
    };
    static const int hyphens[11] = {
	1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0
    };

    yyTok = getToken(); // skip Tok_Define
    if ( uuidDefine.exactMatch(QString(yyLex)) ) {
	QString name = uuidDefine.cap( 1 );
	QString uuid;
	QString uufrag;

	yyTok = getToken();
	if ( strcmp(yyLex, "QUuid") != 0 )
	    return;
	yyTok = getToken();
	if ( yyTok != Tok_LeftParen )
	    return;
	yyTok = getToken();
	uuid = "{";
	for ( int i = 0; i < 11; i++ ) {
	    if ( yyTok != Tok_Number )
		return;

	    uufrag.sprintf( "%.*X", numDigits[i], yyNumber );
	    uuid += uufrag;
	    if ( hyphens[i] != 0 )
		uuid += QChar( '-' );
	    yyTok = getToken();
	    if ( yyTok == Tok_Comma )
		yyTok = getToken();
	}
	uuid += "}";
	if ( yyTok != Tok_RightParen )
	    return;
	yyTok = getToken();

	uuidMap->insert( name, uuid );
    }
}

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
	} while ( depth > 0 && yyTok != Tok_Eof );
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
	    } while ( yyTok != Tok_Eof && yyTok != Tok_RightBracket );
	    yyTok = getToken();
	    appendToType( &type, yyLex );
	}
    }
    return type;
}

static QString matchMemberFunctionDeclaration()
{
    QString returnType;
    QString name;
    QString params;

    if ( yyTok == Tok_public || yyTok == Tok_protected || yyTok == Tok_private )
	yyTok = getToken();
    if ( yyTok == Tok_virtual )
	yyTok = getToken();

    returnType = matchDataType( DontParseVarNames );
    if ( returnType.isEmpty() )
	return QString::null;

    if ( yyTok != Tok_Ident )
	return QString::null;
    name = yyLex;
    yyTok = getToken();

    if ( yyTok != Tok_LeftParen )
	return QString::null;
    yyTok = getToken();

    if ( yyTok != Tok_RightParen ) {
	while ( TRUE ) {
	    QString param = matchDataType( StripVarNames );
	    if ( param.isEmpty() )
		return QString::null;
	    params.append( param );

	    if ( yyTok != Tok_Comma )
		break;

	    params += QString( ", " );
	    yyTok = getToken();
	}
	if ( yyTok != Tok_RightParen )
	    return QString::null;
    }
    yyTok = getToken();

    return returnType + QChar( '\t' ) + name + QChar( '(' ) + params +
	   QChar( ')' );
}

static void matchClassDefinition( QValueList<Interface> *wannabeList )
{
    QString name;
    QStringList funcs;

    if ( yyBraceDepth != 0 )
	return;
    if ( yyTok != Tok_class && yyTok != Tok_struct )
	return;
    yyTok = getToken();

    while ( yyTok == Tok_Ident ) { // skip over Q_EXPORT and similar hacks
	name = yyLex;
	yyTok = getToken();
    }
    if ( name.isEmpty() )
	return;

    while ( yyTok != Tok_Eof && yyTok != Tok_LeftBrace &&
	    yyTok != Tok_Semicolon )
	yyTok = getToken();
    if ( yyTok != Tok_LeftBrace )
	return;
    yyTok = getToken();

    while ( yyTok != Tok_Eof && yyBraceDepth > 0 ) {
	QString f = matchMemberFunctionDeclaration();
	if ( f.isEmpty() ) {
	    while ( yyTok != Tok_Semicolon && yyTok != Tok_RightParen &&
		    yyTok != Tok_RightBrace )
		yyTok = getToken();
	    yyTok = getToken();
	} else {
	    funcs.append( f );
	}
    }

    Interface i;
    i.setName( name );
    i.setFunctionList( funcs );
    wannabeList->append( i );
}

void parseHeaderFile( const QString& name,
		      QMap<QString, Interface> *interfaceMap )
{
    QValueList<Interface> wannabeList;
    QMap<QString, QString> uuidMap;

    FILE *in = fopen( QFile::encodeName(name), "r" );
    if ( in == 0 )
	return;
    startTokenizer( in );

    yyTok = getToken();
    while ( yyTok != Tok_Eof ) {
	switch ( yyTok ) {
	case Tok_Define:
	    matchDefineDirective( &uuidMap );
	    break;
	case Tok_class:
	case Tok_struct:
	    matchClassDefinition( &wannabeList );
	    break;
	default:
	    yyTok = getToken();
	}
    }

    QValueList<Interface>::Iterator i = wannabeList.begin();
    while ( i != wannabeList.end() ) {
	QString uuid = uuidMap[(*i).name()];
	if ( !uuid.isEmpty() ) {
	    Interface iface = *i;
	    iface.setUuid( uuid );
	    interfaceMap->insert( iface.name(), iface );
	}
	++i;
    }
}
/*
int main()
{
    QMap<QString, Interface> interfaceMap;
    parseHeaderFile( QString("/home/jasmin/qt/include/qcomponentinterface.h"),
		     &interfaceMap );

    QMap<QString, Interface>::ConstIterator i = interfaceMap.begin();
    while ( i != interfaceMap.end() ) {
	qDebug( "%s: %s", (*i).name().latin1(), (*i).uuid().latin1() );
	QStringList::ConstIterator f = (*i).functionList().begin();
	while ( f != (*i).functionList().end() ) {
	    qDebug( "  %s", (*f).latin1() );
	    ++f;
	}
	++i;
    }
    return EXIT_SUCCESS;
}
*/
