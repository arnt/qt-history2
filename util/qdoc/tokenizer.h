/*
  tokenizer.h
*/

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <qstring.h>
#include <qvaluestack.h>

#include <stdio.h>

#include "location.h"

/*
  Here come the C++ tokens we support.  The first part contains all-purpose
  tokens; then comes keywords.  If you add a keyword, make sure to modify the
  keyword array in tokenizer.cpp as well, and make sure Tok_FirstKeyword and
  Tok_LastKeyword are right.
*/
enum { Tok_Eoi, Tok_Ampersand, Tok_Aster, Tok_LeftParen, Tok_RightParen,
       Tok_LeftParenAster, Tok_Equal, Tok_LeftBrace, Tok_RightBrace,
       Tok_Semicolon, Tok_Colon, Tok_LeftAngle, Tok_RightAngle, Tok_Comma,
       Tok_Ellipsis, Tok_Gulbrandsen, Tok_LeftBracket, Tok_RightBracket,
       Tok_Tilde, Tok_SomeOperator, Tok_Number, Tok_String, Tok_Doc,
       Tok_Comment, Tok_Ident,

       Tok_char, Tok_class, Tok_const, Tok_double, Tok_enum, Tok_inline,
       Tok_int, Tok_long, Tok_operator, Tok_private, Tok_protected, Tok_public,
       Tok_short, Tok_signals, Tok_signed, Tok_slots, Tok_static, Tok_struct,
       Tok_template, Tok_typedef, Tok_union, Tok_unsigned, Tok_virtual,
       Tok_void, Tok_volatile, Tok_Q_ENUMS, Tok_Q_EXPORT, Tok_Q_OBJECT,
       Tok_Q_OVERRIDE, Tok_Q_PROPERTY,

       Tok_FirstKeyword = Tok_char, Tok_LastKeyword = Tok_Q_PROPERTY };

/*
  The Tokenizer class implements lexical analysis of C++ source files.

  Not every operator or keyword of C++ is recognized; only those that are
  interesting to us.  Some Qt keywords or macros are also recognized.

  The class is an abstract base class inherited by FileTokenizer and
  StringTokenizer.
*/
class Tokenizer
{
public:
    virtual ~Tokenizer() { }

    int getToken();

    Location location() const { return yyTokLoc; }
    QString previousLexeme() const { return QString( yyPrevLex ); }
    QString lexeme() const { return QString( yyLex ); }
    int braceDepth() const { return yyBraceDepth; }
    int parenDepth() const { return yyParenDepth; }
    int bracketDepth() const { return yyBracketDepth; }

protected:
    Tokenizer();

    virtual int getch();

    void start( const Location& loc );

private:
#if defined(Q_DISABLE_COPY)
    Tokenizer( const Tokenizer& );
    Tokenizer& operator=( const Tokenizer& );
#endif

    int getChar();
    int getTokenAfterPreprocessor();
    void pushSkipping( bool skip );
    bool popSkipping();
    bool isTrue( const QString& condition ) const;

    /*
      This limit on the length of a lexeme seems fairly high, but a
      doc comment can be arbitrarily long. The previous 65536 limit
      was reached by some of Mark Summerfield's documentation.
    */
    enum { yyLexBufSize = 524288 };

    Location yyTokLoc;
    Location yyCurLoc;
    char yyLexBuf1[yyLexBufSize];
    char yyLexBuf2[yyLexBufSize];
    char *yyPrevLex;
    char *yyLex;
    size_t yyLexLen;
    QValueStack<bool> yyPreprocessorSkipping;
    int yyNumPreprocessorSkipping;
    int yyBraceDepth;
    int yyParenDepth;
    int yyBracketDepth;
    int yyCh;
};

inline int Tokenizer::getChar() {
    if ( yyCh == EOF )
	return EOF;
    if ( yyLexLen < yyLexBufSize - 1 ) {
	yyLex[yyLexLen++] = (char) yyCh;
	yyLex[yyLexLen] = '\0';
    }
    yyCurLoc.advance( yyCh );
    return getch();
}

/*
  The FileTokenizer class is a Tokenizer that gets its input from a FILE *.
*/
class FileTokenizer : public Tokenizer
{
public:
    FileTokenizer();

    virtual int getch();

    void start( const Location& loc, FILE *in );
    void stop() { yyIn = 0; }

private:
#if defined(Q_DISABLE_COPY)
    FileTokenizer( const FileTokenizer& );
    FileTokenizer& operator=( const FileTokenizer& );
#endif

    FILE *yyIn;
};

/*
  The StringTokenizer class is a Tokenizer that gets its input from a char
  string.
*/
class StringTokenizer : public Tokenizer
{
public:
    StringTokenizer();

    virtual int getch();

    void start( const Location& loc, const char *in, int len );
    void stop();

private:
#if defined(Q_DISABLE_COPY)
    StringTokenizer( const StringTokenizer& );
    StringTokenizer& operator=( const StringTokenizer& );
#endif

    const char *yyIn;
    int yyPos;
    int yyLen;
};

#endif
