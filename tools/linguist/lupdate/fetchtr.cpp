/**********************************************************************
**   Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
**   fetchtr.cpp
**
**   This file is part of Qt Linguist.
**
**   See the file LICENSE included in the distribution for the usage
**   and distribution terms.
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include <qfile.h>
#include <qstring.h>
#include <qtextstream.h>

#include <ctype.h>
#include <errno.h>
#include <metatranslator.h>
#include <stdio.h>
#include <string.h>
#include <qxml.h>

static const char MagicComment[] = "TRANSLATOR ";

/*
  The first part of this source file is the C++ tokenizer.  We skip
  most of C++; the only tokens that interest us are defined here.
  Thus, the code fragment

      int main() {
	  printf( "Hello, world!\n" );
	  return 0;
      }

  is broken down into the following tokens:

      Ident Ident LeftParen RightParen
	  Ident LeftParen String RightParen Semicolon
	  Ident Semicolon
      RightBrace.

  Notice that the left brace and the 0 don't produce any token. (The
  left brace is not completely ignored, as it increments
  yyBraceDepth.)
*/

enum { Tok_Eof, Tok_class, Tok_tr, Tok_translate, Tok_Ident, Tok_Comment,
       Tok_String, Tok_Gulbrandsen, Tok_RightBrace, Tok_LeftParen,
       Tok_RightParen, Tok_Comma, Tok_Semicolon };

/*
  The tokenizer maintains the following global variables. The names
  should be self-explanatory.
*/
static QCString yyName;
static FILE *yyIn;
static int yyCh;
static char yyIdent[32];
static size_t yyIdentLen;
static char yyComment[65536];
static size_t yyCommentLen;
static char yyString[8192];
static size_t yyStringLen;
static int yyBraceDepth;
static int yyParenDepth;
static int yyLineNo;
static int yyCurLineNo;

static inline int getChar()
{
    int c = getc( yyIn );
    if ( c == '\n' )
	yyCurLineNo++;
    return c;
}

static void startTokenizer( const char *name )
{
    yyName = name;
    yyIn = fopen( name, "r" );
    if ( yyIn == 0 ) {
	qWarning( "lupdate error: cannot open C++ source file '%s': %s",
		  name, strerror(errno) );
	return;
    }
    yyCh = getChar();
    yyBraceDepth = 0;
    yyParenDepth = 0;
    yyCurLineNo = 1;
}

static void stopTokenizer()
{
    fclose( yyIn );
}

static int getToken()
{
    const char tab[] = "abfnrtv";
    const char backTab[] = "\a\b\f\n\r\t\v";
    int n;

    yyIdentLen = 0;
    yyCommentLen = 0;
    yyStringLen = 0;

    while ( yyCh != EOF ) {
	yyLineNo = yyCurLineNo;

	if ( isalpha(yyCh) || yyCh == '_' ) {
	    do {
		if ( yyIdentLen < sizeof(yyIdent) - 1 )
		    yyIdent[yyIdentLen++] = (char) yyCh;
		yyCh = getChar();
	    } while ( isalnum(yyCh) || yyCh == '_' );
	    yyIdent[yyIdentLen] = '\0';

	    switch ( yyIdent[0] ) {
	    case 'Q':
		if ( strcmp(yyIdent + 1, "T_TR_NOOP") == 0 )
		    return Tok_tr;
		else if ( strcmp(yyIdent + 1, "T_TRANSLATE_NOOP") == 0 )
		    return Tok_translate;
		break;
	    case 'c':
		if ( strcmp(yyIdent + 1, "lass") == 0 )
		    return Tok_class;
		break;
	    case 's':
		if ( strcmp(yyIdent + 1, "truct") == 0 )
		    return Tok_class;
		break;
	    case 't':
		if ( strcmp(yyIdent + 1, "r") == 0 )
		    return Tok_tr;
		else if ( strcmp(yyIdent + 1, "ranslate") == 0 )
		    return Tok_translate;
	    }
	    return Tok_Ident;
	} else {
	    switch ( yyCh ) {
	    case '/':
		yyCh = getChar();
		if ( yyCh == '/' ) {
		    do {
			yyCh = getChar();
		    } while ( yyCh != EOF && yyCh != '\n' );
		} else if ( yyCh == '*' ) {
		    bool metAster = FALSE;
		    bool metAsterSlash = FALSE;

		    while ( !metAsterSlash ) {
			yyCh = getChar();
			if ( yyCh == EOF ) {
			    qWarning( "%s: Unterminated C++ comment starting at"
				      " line %d", (const char *) yyName,
				      yyLineNo );
			    yyComment[yyCommentLen] = '\0';
			    return Tok_Comment;
			}
			if ( yyCommentLen < sizeof(yyComment) - 1 )
			    yyComment[yyCommentLen++] = (char) yyCh;

			if ( yyCh == '*' )
			    metAster = TRUE;
			else if ( metAster && yyCh == '/' )
			    metAsterSlash = TRUE;
			else
			    metAster = FALSE;
		    }
		    yyCommentLen -= 2;
		    yyComment[yyCommentLen] = '\0';
		    return Tok_Comment;
		}
		break;
	    case '"':
		yyCh = getChar();

		while ( yyCh != EOF && yyCh != '"' ) {
		    if ( yyCh == '\\' ) {
			yyCh = getChar();

			if ( yyCh == 'x' ) {
			    QCString hex = "0";

			    yyCh = getChar();
			    while ( isxdigit(yyCh) ) {
				hex += (char) yyCh;
				yyCh = getChar();
			    }
			    sscanf( hex, "%x", &n );
			    if ( yyStringLen < sizeof(yyString) - 1 )
				yyString[yyStringLen++] = (char) n;
			} else if ( yyCh >= '0' && yyCh < '8' ) {
			    QCString oct = "";

			    do {
				oct += (char) yyCh;
				yyCh = getChar();
			    } while ( yyCh >= '0' && yyCh < '8' );
			    sscanf( oct, "%o", &n );
			    if ( yyStringLen < sizeof(yyString) - 1 )
				yyString[yyStringLen++] = (char) n;
			} else {
			    const char *p = strchr( tab, yyCh );
			    if ( yyStringLen < sizeof(yyString) - 1 )
				yyString[yyStringLen++] = ( p == 0 ) ?
					(char) yyCh : backTab[p - tab];
			    yyCh = getChar();
			}
		    } else {
			if ( yyStringLen < sizeof(yyString) - 1 )
			    yyString[yyStringLen++] = (char) yyCh;
			yyCh = getChar();
		    }
		}
		yyString[yyStringLen] = '\0';
		if ( yyCh == EOF ) {
		    qWarning( "%s: Unterminated C++ string starting at line %d",
			      (const char *) yyName, yyLineNo );
		    return Tok_Eof;
		} else {
		    yyCh = getChar();
		    return Tok_String;
		}
		break;
	    case ':':
		yyCh = getChar();
		if ( yyCh == ':' ) {
		    yyCh = getChar();
		    return Tok_Gulbrandsen;
		}
		break;
	    case '\'':
		yyCh = getChar();
		if ( yyCh == '\\' )
		    yyCh = getChar();

		do {
		    yyCh = getChar();
		} while ( yyCh != EOF && yyCh != '\'' );
		yyCh = getChar();
		break;
	    case '{':
		yyBraceDepth++;
		yyCh = getChar();
		break;
	    case '}':
		yyBraceDepth--;
		yyCh = getChar();
		return Tok_RightBrace;
	    case '(':
		yyParenDepth++;
		yyCh = getChar();
		return Tok_LeftParen;
	    case ')':
		yyParenDepth--;
		yyCh = getChar();
		return Tok_RightParen;
	    case ',':
		yyCh = getChar();
		return Tok_Comma;
	    case ';':
		yyCh = getChar();
		return Tok_Semicolon;
	    default:
		yyCh = getChar();
	    }
	}
    }
    if ( yyBraceDepth != 0 )
	qWarning( "%s: Brace depth is %d at end of file (should be 0 in C++)",
		  (const char *) yyName, yyBraceDepth );
    if ( yyParenDepth != 0 )
	qWarning( "%s: Parenthesis depth is %d at end of file (should be 0 in"
		  " C++)", (const char *) yyName, yyParenDepth );
    return Tok_Eof;
}

/*
  The second part of this source file is the parser. It accomplishes
  a very easy task: It finds all strings inside a tr() or translate()
  call, and possibly finds out the context of the call. It supports
  three cases: (1) the context is specified, as in
  FunnyDialog::tr("Hello") or translate("FunnyDialog", "Hello");
  (2) the call appears within an inlined function; (3) the call
  appears within a function defined outside the class definition.
*/

static int yyTok;

static bool match( int t )
{
    bool matches = ( yyTok == t );
    if ( matches )
	yyTok = getToken();
    return matches;
}

static bool matchString( QCString *s )
{
    bool matches = ( yyTok == Tok_String );
    *s = "";
    while ( yyTok == Tok_String ) {
	*s += yyString;
	yyTok = getToken();
    }
    return matches;
}

void fetchtr_cpp( const char *name, MetaTranslator *tor,
		  const char *defaultContext )
{
    QCString context, text, com;
    QCString functionContext;
    QCString prefix;

    startTokenizer( name );
    if ( yyIn == 0 )
	return;

    yyTok = getToken();
    while ( yyTok != Tok_Eof ) {
	switch ( yyTok ) {
	case Tok_class:
	    /*
	      Partial support for inlined functions.
	    */
	    yyTok = getToken();
	    if ( yyBraceDepth == 0 && yyParenDepth == 0 ) {
		functionContext = yyIdent;
		yyTok = getToken();
		while ( yyTok == Tok_Gulbrandsen ) {
		    yyTok = getToken();
		    functionContext += "::";
		    functionContext += yyIdent;
		    yyTok = getToken();
		}
	    }
	    break;
	case Tok_tr:
	    yyTok = getToken();
	    if ( match(Tok_LeftParen) && matchString(&text) ) {
		com = "";
		if ( match(Tok_RightParen) || (match(Tok_Comma) &&
			matchString(&com) && match(Tok_RightParen)) ) {
		    if ( prefix.isNull() )
			context = functionContext;
		    else
			context = prefix;
		    prefix = (const char *) 0;

		    tor->insert( MetaTranslatorMessage(context, text, com) );
		}
	    }
	    break;
	case Tok_translate:
	    yyTok = getToken();
	    if ( match(Tok_LeftParen) &&
		 matchString(&context) &&
		 match(Tok_Comma) &&
		 matchString(&text) ) {
		com = "";
		if ( match(Tok_RightParen) ||
		     (match(Tok_Comma) &&
		      matchString(&com) &&
		      match(Tok_RightParen)) )
		    tor->insert( MetaTranslatorMessage(context, text, com) );
	    }
	    break;
	case Tok_Ident:
	    if ( !prefix.isNull() )
		prefix += "::";
	    prefix += yyIdent;
	    yyTok = getToken();
	    if ( yyTok != Tok_Gulbrandsen )
		prefix = (const char *) 0;
	    break;
	case Tok_Comment:
	    com = yyComment;
	    com = com.simplifyWhiteSpace();
	    if ( com.left(sizeof(MagicComment) - 1) == MagicComment ) {
		com.remove( 0, sizeof(MagicComment) - 1 );
		int k = com.find( ' ' );
		if ( k >= 0 ) {
		    context = com.left( k );
		    com.remove( 0, k + 1 );
		    tor->insert( MetaTranslatorMessage(context, "", com) );
		}
	    }
	    yyTok = getToken();
	    break;
	case Tok_Gulbrandsen:
	    if ( yyBraceDepth == 0 && yyParenDepth == 0 )
		functionContext = prefix;
	    yyTok = getToken();
	    break;
	case Tok_RightBrace:
	case Tok_Semicolon:
	    if ( yyBraceDepth == 0 )
		functionContext = defaultContext;
	    yyTok = getToken();
	    break;
	default:
	    yyTok = getToken();
	}
    }
    stopTokenizer();
}

/*
  In addition to C++, we support Qt Designer UI files.
*/

class UiHandler : public QXmlDefaultHandler
{
public:
    UiHandler( MetaTranslator *translator )
	: tor( translator ), comment( "" ) { }

    virtual bool startElement( const QString& namespaceURI,
			       const QString& localName, const QString& qName,
			       const QXmlAttributes& atts );
    virtual bool endElement( const QString& namespaceURI,
			     const QString& localName, const QString& qName );
    virtual bool characters( const QString& ch );
    virtual bool fatalError( const QXmlParseException& exception );

private:
    void flush();

    MetaTranslator *tor;
    QString context;
    QString source;
    QString comment;

    QString accum;
};

bool UiHandler::startElement( const QString& /* namespaceURI */,
			      const QString& /* localName */,
			      const QString& qName,
			      const QXmlAttributes& /* atts */ )
{
    if ( qName == QString("string") )
	flush();
    accum.truncate( 0 );
    return TRUE;
}

bool UiHandler::endElement( const QString& /* namespaceURI */,
			    const QString& /* localName */,
			    const QString& qName )
{
    if ( qName == QString("class") ) {
	if ( context.isEmpty() )
	    context = accum;
    } else if ( qName == QString("string") ) {
	source = accum;
    } else if ( qName == QString("comment") ) {
	comment = accum;
	flush();
    } else {
	flush();
    }
    return TRUE;
}

bool UiHandler::characters( const QString& ch )
{
    accum += ch;
    return TRUE;
}

bool UiHandler::fatalError( const QXmlParseException& exception )
{
    QString msg;
    msg.sprintf( "Parse error at line %d, column %d (%s).",
		 exception.lineNumber(), exception.columnNumber(),
		 exception.message().latin1() );
    qWarning( "XML error: %s", msg.latin1() );
    return FALSE;
}

void UiHandler::flush()
{
    if ( !context.isEmpty() && !source.isEmpty() )
	tor->insert( MetaTranslatorMessage(context, source, comment) );
    source.truncate( 0 );
    comment.truncate( 0 );
}

void fetchtr_ui( const char *name, MetaTranslator *tor,
		 const char * /* defaultContext */ )
{
    QFile f( name );
    if ( !f.open(IO_ReadOnly) ) {
	qWarning( "lupdate error: cannot open UI source file '%s': %s", name,
		  strerror(errno) );
	return;
    }

    QTextStream t( &f );
    QXmlInputSource in( t );
    QXmlSimpleReader reader;
    // don't click on these!
    reader.setFeature( "http://xml.org/sax/features/namespaces", FALSE );
    reader.setFeature( "http://xml.org/sax/features/namespace-prefixes", TRUE );
    reader.setFeature( "http://trolltech.com/xml/features/report-whitespace"
		       "-only-CharData", FALSE );
    QXmlDefaultHandler *hand = new UiHandler( tor );
    reader.setContentHandler( hand );
    reader.setErrorHandler( hand );

    if ( !reader.parse(in) )
	qWarning( "%s: Parse error in UI file", name );
    reader.setContentHandler( 0 );
    reader.setErrorHandler( 0 );
    delete hand;
    f.close();
}
