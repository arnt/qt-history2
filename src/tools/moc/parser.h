/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PARSER_H
#define PARSER_H

#include <QStack>
#include "symbols.h"

class Parser
{
public:
    Parser():index(0), displayWarnings(true){}
    Symbols symbols;
    int index;
    bool displayWarnings;

    QStack<QByteArray> currentFilenames;

    inline bool hasNext() const { return (index < symbols.size()); }
    inline Token next() { return symbols.at(index++).token; }
    bool test(Token);
    void next(Token);
    void next(Token, const char *msg);
    bool until(Token);
    QByteArray lexemUntil(Token);
    QByteArray lexemUntilExclusive(Token);
    inline void prev() {--index;}
    inline Token lookup(int k = 1);
    inline const Symbol &symbol_lookup(int k = 1) { return symbols.at(index-1+k);}
    inline Token token() { return symbols.at(index-1).token;}
    inline QByteArray lexem() { return symbols.at(index-1).lexem();}
    inline QByteArray unquotedLexem() { return symbols.at(index-1).unquotedLexem();}
    inline const Symbol &symbol() { return symbols.at(index-1);}

    void error(int rollback);
    void error(const char *msg = 0);
    void warning(const char * = 0);

};

inline bool Parser::test(Token token)
{
    if (index < symbols.size() && symbols.at(index).token == token) {
        ++index;
        return true;
    }
    return false;
}

inline Token Parser::lookup(int k)
{
    const int l = index - 1 + k;
    return l < symbols.size() ? symbols.at(l).token : NOTOKEN;
}

inline void Parser::next(Token token)
{
    if (!test(token))
        error();
}

inline void Parser::next(Token token, const char *msg)
{
    if (!test(token))
        error(msg);
}

#endif
