#include "config.h"
#include "tokenizer.h"

#include <qhash.h>
#include <qregexp.h>
#include <qstring.h>

#include <ctype.h>
#include <string.h>

#define LANGUAGE_CPP			"Cpp"

/* qmake ignore Q_OBJECT */

/*
  Keep in sync with tokenizer.h.
*/
static const char *kwords[] = {
    "char", "class", "const", "double", "enum", "friend", "int", "long", "namespace", "operator",
    "private", "protected", "public", "short", "signals", "signed", "slots", "static", "struct",
    "template", "typedef", "union", "unsigned", "virtual", "void", "volatile", "Q_OBJECT",
    "Q_OVERRIDE", "Q_PROPERTY", "Q_DECLARE_ITERATOR", "Q_DECLARE_ASSOCIATIVE_ITERATOR",
    "Q_DECLARE_FLAGS", "QT_COMPAT", "QT_COMPAT_CONSTRUCTOR", "QT_MOC_COMPAT", "QDOC_PROPERTY"
};

static const int KwordHashTableSize = 2048;
static int kwordHashTable[KwordHashTableSize];

static QHash<QByteArray, bool> *ignoredTokensAndDirectives = 0;

static QRegExp *comment = 0;
static QRegExp *versionX = 0;
static QRegExp *definedX = 0;

static QRegExp *defines = 0;
static QRegExp *falsehoods = 0;

/*
  This function is a perfect hash function for the 37 keywords of C99
  (with a hash table size of 512). It should perform well on our
  Qt-enhanced C++ subset.
*/
static int hashKword(const char *s, int len)
{
    return (((uchar) s[0]) + (((uchar) s[2]) << 5) +
	     (((uchar) s[len - 1]) << 3) ) % KwordHashTableSize;
}

static void insertKwordIntoHash(const char *s, int number)
{
    int k = hashKword(s, strlen(s));
    while (kwordHashTable[k]) {
	if (++k == KwordHashTableSize)
	    k = 0;
    }
    kwordHashTable[k] = number;
}

Tokenizer::~Tokenizer()
{
    delete[] yyLexBuf1;
    delete[] yyLexBuf2;
}

int Tokenizer::getToken()
{
    char *t = yyPrevLex;
    yyPrevLex = yyLex;
    yyLex = t;

    while ( yyCh != EOF ) {
	yyTokLoc = yyCurLoc;
	yyLexLen = 0;

	if ( isspace(yyCh) ) {
	    do {
		yyCh = getChar();
	    } while ( isspace(yyCh) );
	} else if ( isalpha(yyCh) || yyCh == '_' ) {
	    do {
		yyCh = getChar();
	    } while ( isalnum(yyCh) || yyCh == '_' );

	    int k = hashKword( yyLex, yyLexLen );
	    for ( ;; ) {
		int i = kwordHashTable[k];
		if ( i == 0 ) {
		    return Tok_Ident;
		} else if ( i == -1 ) {
		    if (ignoredTokensAndDirectives->contains(yyLex)) {
                        if (ignoredTokensAndDirectives->value(yyLex)) { // it's a directive
                            int parenDepth = 0;
                            while (yyCh != EOF && (yyCh != ')' || parenDepth > 1)) {
                                if (yyCh == '(')
                                    ++parenDepth;
                                else if (yyCh == ')')
                                    --parenDepth;
                                yyCh = getChar();
                            }
			    if (yyCh == ')')
				yyCh = getChar();
                        }
			break;
                    }
		} else if ( strcmp(yyLex, kwords[i - 1]) == 0 ) {
		    return (int) Tok_FirstKeyword + i - 1;
		}

		if ( ++k == KwordHashTableSize )
		    k = 0;
	    }
	} else if ( isdigit(yyCh) ) {
	    do {
		yyCh = getChar();
	    } while ( isalnum(yyCh) || yyCh == '.' || yyCh == '+' ||
		      yyCh == '-' );
	    return Tok_Number;
	} else {
	    switch ( yyCh ) {
	    case '!':
	    case '%':
	    case '^':
		yyCh = getChar();
		if ( yyCh == '=' )
		    yyCh = getChar();
		return Tok_SomeOperator;
	    case '"':
		yyCh = getChar();

		while ( yyCh != EOF && yyCh != '"' ) {
		    if ( yyCh == '\\' )
			yyCh = getChar();
		    yyCh = getChar();
		}
		yyCh = getChar();

		if ( yyCh == EOF )
		    yyTokLoc.warning( tr("Unterminated C++ string literal") );
		else
		    return Tok_String;
		break;
	    case '#':
		return getTokenAfterPreprocessor();
	    case '&':
		yyCh = getChar();
		if ( yyCh == '&' || yyCh == '=' ) {
		    yyCh = getChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_Ampersand;
		}
	    case '\'':
		yyCh = getChar();
		if ( yyCh == '\\' )
		    yyCh = getChar();
		do {
		    yyCh = getChar();
		} while ( yyCh != EOF && yyCh != '\'' );

		if ( yyCh == EOF ) {
		    yyTokLoc.warning( tr("Unterminated C++ character"
					 " literal") );
		} else {
		    yyCh = getChar();
		    return Tok_Number;
		}
		break;
	    case '(':
		yyCh = getChar();
		if ( yyNumPreprocessorSkipping == 0 )
		    yyParenDepth++;
		if ( isspace(yyCh) ) {
		    do {
			yyCh = getChar();
		    } while ( isspace(yyCh) );
		    yyLexLen = 1;
		    yyLex[1] = '\0';
		}
		if ( yyCh == '*' ) {
		    yyCh = getChar();
		    return Tok_LeftParenAster;
		}
		return Tok_LeftParen;
	    case ')':
		yyCh = getChar();
		if ( yyNumPreprocessorSkipping == 0 )
		    yyParenDepth--;
		return Tok_RightParen;
	    case '*':
		yyCh = getChar();
		if ( yyCh == '=' ) {
		    yyCh = getChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_Aster;
		}
	    case '+':
		yyCh = getChar();
		if ( yyCh == '+' || yyCh == '=' )
		    yyCh = getChar();
		return Tok_SomeOperator;
	    case ',':
		yyCh = getChar();
		return Tok_Comma;
	    case '-':
		yyCh = getChar();
		if ( yyCh == '-' || yyCh == '=' ) {
		    yyCh = getChar();
		} else if ( yyCh == '>' ) {
		    yyCh = getChar();
		    if ( yyCh == '*' )
			yyCh = getChar();
		}
		return Tok_SomeOperator;
	    case '.':
		yyCh = getChar();
		if ( yyCh == '*' ) {
		    yyCh = getChar();
		} else if ( yyCh == '.' ) {
		    do {
			yyCh = getChar();
		    } while ( yyCh == '.' );
		    return Tok_Ellipsis;
		} else if ( isdigit(yyCh) ) {
		    do {
			yyCh = getChar();
		    } while ( isalnum(yyCh) || yyCh == '.' || yyCh == '+' ||
			      yyCh == '-' );
		    return Tok_Number;
		}
		return Tok_SomeOperator;
	    case '/':
		yyCh = getChar();
		if ( yyCh == '/' ) {
		    do {
			yyCh = getChar();
		    } while ( yyCh != EOF && yyCh != '\n' );
		} else if ( yyCh == '*' ) {
		    bool metDoc = false; // empty doc is no doc
		    bool metSlashAsterBang = false;
		    bool metAster = false;
		    bool metAsterSlash = false;

		    yyCh = getChar();
		    if ( yyCh == '!' )
			metSlashAsterBang = true;

		    while ( !metAsterSlash ) {
			if ( yyCh == EOF ) {
			    yyTokLoc.warning( tr("Unterminated C++ comment") );
			    break;
			} else {
			    if ( yyCh == '*' ) {
				metAster = true;
			    } else if ( metAster && yyCh == '/' ) {
				metAsterSlash = true;
			    } else {
				metAster = false;
				if ( isgraph(yyCh) )
				    metDoc = true;
			    }
			}
			yyCh = getChar();
		    }
		    if ( metSlashAsterBang && metDoc )
			return Tok_Doc;
		    else if ( yyParenDepth > 0 )
			return Tok_Comment;
		} else {
		    if ( yyCh == '=' )
			yyCh = getChar();
		    return Tok_SomeOperator;
		}
		break;
	    case ':':
		yyCh = getChar();
		if ( yyCh == ':' ) {
		    yyCh = getChar();
		    return Tok_Gulbrandsen;
		} else {
		    return Tok_Colon;
		}
	    case ';':
		yyCh = getChar();
		return Tok_Semicolon;
	    case '<':
		yyCh = getChar();
		if ( yyCh == '<' ) {
		    yyCh = getChar();
		    if ( yyCh == '=' )
			yyCh = getChar();
		    return Tok_SomeOperator;
		} else if ( yyCh == '=' ) {
		    yyCh = getChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_LeftAngle;
		}
	    case '=':
		yyCh = getChar();
		if ( yyCh == '=' ) {
		    yyCh = getChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_Equal;
		}
	    case '>':
		yyCh = getChar();
		if ( yyCh == '>' ) {
		    yyCh = getChar();
		    if ( yyCh == '=' )
			yyCh = getChar();
		    return Tok_SomeOperator;
		} else if ( yyCh == '=' ) {
		    yyCh = getChar();
		    return Tok_SomeOperator;
		} else {
		    return Tok_RightAngle;
		}
	    case '?':
		yyCh = getChar();
		return Tok_SomeOperator;
	    case '[':
		yyCh = getChar();
		if ( yyNumPreprocessorSkipping == 0 )
		    yyBracketDepth++;
		return Tok_LeftBracket;
	    case '\\':
		yyCh = getChar();
		yyCh = getChar(); // skip one character
		break;
	    case ']':
		yyCh = getChar();
		if ( yyNumPreprocessorSkipping == 0 )
		    yyBracketDepth--;
		return Tok_RightBracket;
	    case '{':
		yyCh = getChar();
		if ( yyNumPreprocessorSkipping == 0 )
		    yyBraceDepth++;
		return Tok_LeftBrace;
	    case '}':
		yyCh = getChar();
		if ( yyNumPreprocessorSkipping == 0 )
		    yyBraceDepth--;
		return Tok_RightBrace;
	    case '|':
		yyCh = getChar();
		if ( yyCh == '|' || yyCh == '=' )
		    yyCh = getChar();
		return Tok_SomeOperator;
	    case '~':
		yyCh = getChar();
		return Tok_Tilde;
	    default:
		yyTokLoc.warning( tr("Hostile character 0x%1 in C++ source")
				  .arg((uchar)yyCh, 1, 16) );
		yyCh = getChar();
	    }
	}
    }

    if ( yyPreprocessorSkipping.count() > 1 )
	yyTokLoc.warning( tr("Expected #endif before end of file") );

    strcpy( yyLex, "end-of-input" );
    yyLexLen = strlen( yyLex );
    return Tok_Eoi;
}

void Tokenizer::initialize(const Config &config)
{
    QString versionSym = config.getString(CONFIG_VERSIONSYM);

    comment = new QRegExp( "/(?:\\*.*\\*/|/.*\n|/[^\n]*$)" );
    comment->setMinimal( true );
    versionX = new QRegExp( "$cannot possibly match^" );
    if (!versionSym.isEmpty())
	versionX->setPattern("[ \t]*(?:" + QRegExp::escape(versionSym)
			     + ")[ \t]+\"([^\"]*)\"[ \t]*");
    definedX = new QRegExp("defined ?\\( ?([A-Z_0-9a-z]+) ?\\)");

    QStringList d = config.getStringList(CONFIG_DEFINES);
    d += "qdoc";
    defines = new QRegExp(d.join("|"));
    falsehoods = new QRegExp(config.getStringList(CONFIG_FALSEHOODS).join("|"));

    memset(kwordHashTable, sizeof(kwordHashTable), 0);
    for (int i = 0; i < Tok_LastKeyword - Tok_FirstKeyword + 1; i++)
	insertKwordIntoHash(kwords[i], i + 1);

    ignoredTokensAndDirectives = new QHash<QByteArray, bool>;

    QStringList tokens = config.getStringList(LANGUAGE_CPP + Config::dot + CONFIG_IGNORETOKENS);
    tokens.append("explicit");
    tokens.append("inline");
    tokens.append("typename");

    foreach (QString t, tokens) {
	ignoredTokensAndDirectives->insert(t.toAscii(), false);
	insertKwordIntoHash(t.toAscii().data(), -1);
    }

    QStringList directives = config.getStringList(LANGUAGE_CPP + Config::dot
                                                  + CONFIG_IGNOREDIRECTIVES);
    foreach (QString d, directives) {
	ignoredTokensAndDirectives->insert(d.toAscii(), true);
	insertKwordIntoHash(d.toAscii().data(), -1);
    }
}

void Tokenizer::terminate()
{
    delete comment;
    comment = 0;
    delete versionX;
    versionX = 0;
    delete definedX;
    definedX = 0;
    delete defines;
    defines = 0;
    delete falsehoods;
    falsehoods = 0;
    delete ignoredTokensAndDirectives;
    ignoredTokensAndDirectives = 0;
}

Tokenizer::Tokenizer()
{
    yyLexBuf1 = new char[(int) yyLexBufSize];
    yyLexBuf2 = new char[(int) yyLexBufSize];
    yyPrevLex = yyLexBuf1;
    yyPrevLex[0] = '\0';
    yyLex = yyLexBuf2;
    yyLex[0] = '\0';
    yyLexLen = 0;
    yyPreprocessorSkipping.push( false );
    yyNumPreprocessorSkipping = 0;
    yyBraceDepth = 0;
    yyParenDepth = 0;
    yyBracketDepth = 0;
    yyCh = '\0';
}

int Tokenizer::getch()
{
    return EOF;
}

void Tokenizer::start( const Location& loc )
{
    yyTokLoc = loc;
    yyCurLoc = loc;
    yyCurLoc.start();
    strcpy( yyPrevLex, "beginning-of-input" );
    strcpy( yyLex, "beginning-of-input" );
    yyLexLen = strlen( yyLex );
    yyBraceDepth = 0;
    yyParenDepth = 0;
    yyBracketDepth = 0;
    yyCh = '\0';
    yyCh = getChar();
}

/*
  Returns the next token, if # was met.  This function interprets the
  preprocessor directive, skips over any #ifdef'd out tokens, and returns the
  token after all of that.
*/
int Tokenizer::getTokenAfterPreprocessor()
{
    yyCh = getChar();
    while ( isspace(yyCh) && yyCh != '\n' )
	yyCh = getChar();

    /*
      #directive condition
    */
    QString directive;
    QString condition;

    while ( isalpha(yyCh) ) {
	directive += QChar( yyCh );
	yyCh = getChar();
    }
    if ( !directive.isEmpty() ) {
	while ( yyCh != EOF && yyCh != '\n' ) {
	    if ( yyCh == '\\' )
		yyCh = getChar();
	    condition += yyCh;
	    yyCh = getChar();
	}
	condition.replace( *comment, "" );
	condition = condition.simplified();

	/*
	  The #if, #ifdef, #ifndef, #elif, #else, and #endif
	  directives have an effect on the skipping stack.  For
	  instance, if the code processed so far is

	      #if 1
	      #if 0
	      #if 1
	      // ...
	      #else

	  the skipping stack contains, from bottom to top, false true
	  true (assuming 0 is false and 1 is true).  If at least one
	  entry of the stack is true, the tokens are skipped.

	  This mechanism is simple yet hard to understand.
	*/
	if ( directive[0] == QChar('i') ) {
	    if ( directive == QString("if") )
		pushSkipping( !isTrue(condition) );
	    else if ( directive == QString("ifdef") )
		pushSkipping( !defines->exactMatch(condition) );
	    else if ( directive == QString("ifndef") )
		pushSkipping( defines->exactMatch(condition) );
	} else if ( directive[0] == QChar('e') ) {
	    if ( directive == QString("elif") ) {
		bool old = popSkipping();
		if ( old )
		    pushSkipping( !isTrue(condition) );
		else
		    pushSkipping( true );
	    } else if ( directive == QString("else") ) {
		pushSkipping( !popSkipping() );
	    } else if ( directive == QString("endif") ) {
		popSkipping();
	    }
	} else if ( directive == QString("define") ) {
	    if (versionX->exactMatch(condition))
		yyVersion = versionX->cap(1);
	}
    }

    int tok;
    do {
	/*
	  We set yyLex now, and after getToken() this will be
	  yyPrevLex. This way, we skip over the preprocessor
	  directive.
	*/
	qstrcpy( yyLex, yyPrevLex );

	/*
	  If getToken() meets another #, it will call
	  getTokenAfterPreprocessor() once again, which could in turn
	  call getToken() again, etc. Unless there are 10,000 or so
	  preprocessor directives in a row, this shouldn't overflow
	  the stack.
	*/
	tok = getToken();
    } while ( yyNumPreprocessorSkipping > 0 );
    return tok;
}

/*
  Pushes a new skipping value onto the stack.  This corresponds to entering a
  new #if block.
*/
void Tokenizer::pushSkipping( bool skip )
{
    yyPreprocessorSkipping.push( skip );
    if ( skip )
	yyNumPreprocessorSkipping++;
}

/*
  Pops a skipping value from the stack.  This corresponds to reaching a #endif.
*/
bool Tokenizer::popSkipping()
{
    if ( yyPreprocessorSkipping.isEmpty() ) {
	yyTokLoc.warning( tr("Unexpected #elif, #else or #endif") );
	return true;
    }

    bool skip = yyPreprocessorSkipping.pop();
    if ( skip )
	yyNumPreprocessorSkipping--;
    return skip;
}

/*
  Returns true if the condition evaluates as true, otherwise false.  The
  condition is represented by a string.  Unsophisticated parsing techniques are
  used.  The preprocessing method could be named StriNg-Oriented PreProcessing,
  as SNOBOL stands for StriNg-Oriented symBOlic Language.
*/
bool Tokenizer::isTrue(const QString &condition)
{
    int firstOr = -1;
    int firstAnd = -1;
    int parenDepth = 0;

    /*
      Find the first logical operator at top level, but be careful
      about precedence. Examples:

	  X || Y          // the or
	  X || Y || Z     // the leftmost or
	  X || Y && Z     // the or
	  X && Y || Z     // the or
	  (X || Y) && Z   // the and
    */
    for ( int i = 0; i < (int) condition.length() - 1; i++ ) {
	QChar ch = condition[i];
	if ( ch == QChar('(') ) {
	    parenDepth++;
	} else if ( ch == QChar(')') ) {
	    parenDepth--;
	} else if ( parenDepth == 0 ) {
	    if ( condition[i + 1] == ch ) {
		if ( ch == QChar('|') ) {
		    firstOr = i;
		    break;
		} else if ( ch == QChar('&') ) {
		    if ( firstAnd == -1 )
			firstAnd = i;
		}
	    }
	}
    }
    if ( firstOr != -1 )
	return isTrue( condition.left(firstOr) ) ||
	       isTrue( condition.mid(firstOr + 2) );
    if ( firstAnd != -1 )
	return isTrue( condition.left(firstAnd) ) &&
	       isTrue( condition.mid(firstAnd + 2) );

    QString t = condition.simplified();
    if ( t.isEmpty() )
	return true;

    if ( t[0] == QChar('!') )
	return !isTrue( t.mid(1) );
    if ( t[0] == QChar('(') && t.right(1)[0] == QChar(')') )
	return isTrue( t.mid(1, t.length() - 2) );

    if ( definedX->exactMatch(t) )
	return defines->exactMatch( definedX->cap(1) );
    else
	return !falsehoods->exactMatch( t );
}


FileTokenizer::FileTokenizer( const Location& loc, FILE *in )
    : yyIn( in )
{
    start( loc );
}

int FileTokenizer::getch()
{
    return getc( yyIn );
}

StringTokenizer::StringTokenizer( const Location& loc, const char *in, int len )
    : yyIn( in ), yyPos( 0 ), yyLen( len )
{
    start( loc );
}

int StringTokenizer::getch()
{
    return yyPos == yyLen ? EOF : yyIn[yyPos++];
}
