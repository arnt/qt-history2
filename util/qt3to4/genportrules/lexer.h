/* This file is part of KDevelop
    Copyright (C) 2002,2003,2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef LEXER_H
#define LEXER_H

#define KDEV_COMPAT

#include <qvector.h>
#include <qbytearray.h>
#include <qlist.h>

class Lexer;
class TokenStream;
class AST;
class TranslationUnitAST;
class FileSymbol;

typedef void (Lexer::*scan_fun_ptr)(int *kind);

struct Token
{
    uint kind : 16;
    uint hidden : 1;
    int position;
    int length;
    AST *node;
};
Q_DECLARE_TYPEINFO(Token, Q_MOVABLE_TYPE);

struct PPLine
{
    QByteArray fileName;
    int position;
    int line;
};
Q_DECLARE_TYPEINFO(PPLine, Q_MOVABLE_TYPE);

struct TokenStream
{
    inline const Token &token() const
    { return m_tokens.at(m_cursor); }

    inline Token &tokenAt(int index = 0)
    { return m_tokens[index]; }

    inline const Token &tokenAt(int index = 0) const
    { return m_tokens.at(index); }

    inline int lookAhead(int n = 0) const
    { return m_tokens.at(m_cursor + n).kind; }

    inline int currentToken() const
    { return m_tokens.at(m_cursor).kind; }

    inline QByteArray currentTokenText() const
    { return tokenText(cursor()); }

    inline QByteArray tokenText(int index = 0) const
    {
        const Token &t = m_tokens.at(index);
        const char *buffer = m_contents;
        return QByteArray(buffer + t.position, t.length);
        //return m_contents.mid(t.position, t.length);
    }

    inline int lineOffset(int line) const
    { return m_lines.at(line); }

    void positionAt(int position, int *line, int *column, QByteArray *fileName = 0) const;

    inline void getTokenStartPosition(int index, int *line, int *column, QByteArray *fileName = 0) const
    { positionAt(m_tokens.at(index).position, line, column, fileName); }

    inline void getTokenEndPosition(int index, int *line, int *column, QByteArray *fileName = 0) const
    {
        const Token &tk = m_tokens.at(index);
        positionAt(tk.position + tk.length, line, column, fileName);
    }

    inline void rewind(int index)
    { m_cursor = index; }

    inline int cursor() const
    { return m_cursor; }

    inline void nextToken()
    { ++m_cursor; }

    inline bool tokenAtEnd()
    { return m_cursor >= m_tokens.size(); }

//private:
    QByteArray m_contents;

    QList<int> m_lines;
    QList<PPLine> m_pplines;

    QList<Token> m_tokens;
    int m_cursor;

    void positionAtAux(int position, int *line, int *column = 0) const;
};

class Lexer
{
public:
    Lexer();
    ~Lexer();

    TokenStream *tokenize(const FileSymbol *fileSymbol, bool preproc = false);

private:
    void nextToken(Token &tok);

    void scanChar(int *kind);
    void scanUnicodeChar(int *kind);
    void scanNewline(int *kind);
    void scanWhiteSpaces(int *kind);
    void scanCharLiteral(int *kind);
    void scanStringLiteral(int *kind);
    void scanNumberLiteral(int *kind);
    void scanIdentifier(int *kind);
    void scanComment(int *kind);
    void scanPreprocessor(int *kind);
    void scanOperator(int *kind);

    void scanKeyword0(int *kind);
    void scanKeyword2(int *kind);
    void scanKeyword3(int *kind);
    void scanKeyword4(int *kind);
    void scanKeyword5(int *kind);
    void scanKeyword6(int *kind);
    void scanKeyword7(int *kind);
    void scanKeyword8(int *kind);
    void scanKeyword9(int *kind);
    void scanKeyword10(int *kind);
    void scanKeyword11(int *kind);
    void scanKeyword12(int *kind);
    void scanKeyword14(int *kind);
    void scanKeyword16(int *kind);

    bool scanPPLine(int *line, QByteArray *fileName);

    void setupScanTable();

private:
    static bool s_initialized;
    static scan_fun_ptr s_scan_table[128 + 1];
    static int s_attr_table[256];
    static scan_fun_ptr s_scan_keyword_table[16 + 1];

    QByteArray m_contents;
    const char *m_buffer;
    int m_ptr;

    QByteArray m_fileName;
    QByteArray m_currentFileName;

    QList<int> m_lines;
    QList<PPLine> m_pplines;
    QList<Token> m_tokens;

    bool m_skipping;
};

struct FileSymbol
{
    QByteArray fileName;
    QByteArray contents;
    TokenStream *tokenStream;
    TranslationUnitAST *ast;

    FileSymbol()
        : tokenStream(0), ast(0) {}

    inline ~FileSymbol()
    { delete tokenStream; }
};

#endif
