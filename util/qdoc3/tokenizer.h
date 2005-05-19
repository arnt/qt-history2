/*
  tokenizer.h
*/

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <qstack.h>
#include <qstring.h>

#include <stdio.h>

#include "location.h"

/*
  Here come the C++ tokens we support.  The first part contains
  all-purpose tokens; then come keywords.
  
  If you add a keyword, make sure to modify the keyword array in
  tokenizer.cpp as well, and possibly adjust Tok_FirstKeyword and
  Tok_LastKeyword.
*/
enum { Tok_Eoi, Tok_Ampersand, Tok_Aster, Tok_LeftParen, Tok_RightParen, Tok_LeftParenAster,
       Tok_Equal, Tok_LeftBrace, Tok_RightBrace, Tok_Semicolon, Tok_Colon, Tok_LeftAngle,
       Tok_RightAngle, Tok_Comma, Tok_Ellipsis, Tok_Gulbrandsen, Tok_LeftBracket, Tok_RightBracket,
       Tok_Tilde, Tok_SomeOperator, Tok_Number, Tok_String, Tok_Doc, Tok_Comment, Tok_Ident,

       Tok_char, Tok_class, Tok_const, Tok_double, Tok_enum, Tok_explicit, Tok_friend, Tok_int,
       Tok_long, Tok_namespace, Tok_operator, Tok_private, Tok_protected, Tok_public, Tok_short,
       Tok_signals, Tok_signed, Tok_slots, Tok_static, Tok_struct, Tok_template, Tok_typedef,
       Tok_union, Tok_unsigned, Tok_virtual, Tok_void, Tok_volatile, Tok_int64, Tok_Q_OBJECT,
       Tok_Q_OVERRIDE, Tok_Q_PROPERTY, Tok_Q_DECLARE_SEQUENTIAL_ITERATOR,
       Tok_Q_DECLARE_MUTABLE_SEQUENTIAL_ITERATOR, Tok_Q_DECLARE_ASSOCIATIVE_ITERATOR,
       Tok_Q_DECLARE_MUTABLE_ASSOCIATIVE_ITERATOR, Tok_Q_DECLARE_FLAGS, Tok_QT_COMPAT,
       Tok_QT_COMPAT_CONSTRUCTOR, Tok_QT_MOC_COMPAT, Tok_QT3_SUPPORT, Tok_QT3_SUPPORT_CONSTRUCTOR,
       Tok_QT3_MOC_SUPPORT, Tok_QDOC_PROPERTY,

       Tok_FirstKeyword = Tok_char, Tok_LastKeyword = Tok_QDOC_PROPERTY };

/*
  The Tokenizer class implements lexical analysis of C++ source
  files.

  Not every operator or keyword of C++ is recognized; only those that
  are interesting to us. Some Qt keywords or macros are also
  recognized.

  The class is an abstract base class inherited by FileTokenizer and
  StringTokenizer.
*/
class Tokenizer
{
public:
    virtual ~Tokenizer();

    int getToken();

    const Location &location() const { return yyTokLoc; }
    QString previousLexeme() const { return QString( yyPrevLex ); }
    QString lexeme() const { return QString( yyLex ); }
    QString version() const { return yyVersion; }
    int braceDepth() const { return yyBraceDepth; }
    int parenDepth() const { return yyParenDepth; }
    int bracketDepth() const { return yyBracketDepth; }

    static void initialize(const Config &config);
    static void terminate();
    static bool isTrue(const QString &condition);

protected:
    Tokenizer();

    virtual int getch();

    void start( const Location& loc );

private:
    /*
      This limit on the length of a lexeme seems fairly high, but a
      doc comment can be arbitrarily long. The previous 65,536 limit
      was reached by Mark Summerfield.
    */
    enum { yyLexBufSize = 524288 };

    int getChar();
    int getTokenAfterPreprocessor();
    void pushSkipping( bool skip );
    bool popSkipping();

    Location yyTokLoc;
    Location yyCurLoc;
    char *yyLexBuf1;
    char *yyLexBuf2;
    char *yyPrevLex;
    char *yyLex;
    size_t yyLexLen;
    QStack<bool> yyPreprocessorSkipping;
    int yyNumPreprocessorSkipping;
    int yyBraceDepth;
    int yyParenDepth;
    int yyBracketDepth;
    int yyCh;

    QString yyVersion;
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
  The FileTokenizer class is a Tokenizer that gets its input from a
  FILE *.
*/
class FileTokenizer : public Tokenizer
{
public:
    FileTokenizer( const Location& loc, FILE *in );

    virtual int getch();

private:
    FILE *yyIn;
};

/*
  The StringTokenizer class is a Tokenizer that gets its input from a
  char string.
*/
class StringTokenizer : public Tokenizer
{
public:
    StringTokenizer( const Location& loc, const char *in, int len );

    virtual int getch();

private:
    const char *yyIn;
    int yyPos;
    int yyLen;
};

#endif
