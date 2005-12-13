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

#include "parser.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef USE_LEXEM_STORE
Symbol::LexemStore Symbol::lexemStore;
#endif

static const char *error_msg = 0;

#ifdef Q_CC_MSVC
#define ErrorFormatString "%s(%d): "
#else
#define ErrorFormatString "%s:%d: "
#endif

void Parser::error(int rollback) {
    index -= rollback;
    error();
}
void Parser::error(const char *msg) {
    if (msg || error_msg)
        qWarning(ErrorFormatString "Error: %s",
                 currentFilenames.top().constData(), symbol().lineNum, msg?msg:error_msg);
    else
        qWarning(ErrorFormatString "Parse error at \"%s\"",
                 currentFilenames.top().constData(), symbol().lineNum, symbol().lexem().data());
    exit(EXIT_FAILURE);
}

void Parser::warning(const char *msg) {
    if (displayWarnings && msg)
        fprintf(stderr, ErrorFormatString "Warning: %s\n",
                currentFilenames.top().constData(), qMax(0, index > 0 ? symbol().lineNum : 0), msg);
}

bool Parser::until(Token target) {
    int braceCount = 0;
    int brackCount = 0;
    int parenCount = 0;
    int angleCount = 0;
    if (index) {
        switch(symbols.at(index-1).token) {
        case LBRACE: ++braceCount; break;
        case LBRACK: ++brackCount; break;
        case LPAREN: ++parenCount; break;
        case LANGLE: ++angleCount; break;
        default: ;
        }
    }
    while (index < symbols.size()) {
        Token t = symbols.at(index++).token;
        switch (t) {
        case LBRACE: ++braceCount; break;
        case RBRACE: --braceCount; break;
        case LBRACK: ++brackCount; break;
        case RBRACK: --brackCount; break;
        case LPAREN: ++parenCount; break;
        case RPAREN: --parenCount; break;
        case LANGLE: ++angleCount; break;
        case RANGLE: --angleCount; break;
        default: break;
        }
        if (t == target
            && braceCount <= 0
            && brackCount <= 0
            && parenCount <= 0
            && (target != RANGLE || angleCount <= 0))
            return true;

        if (braceCount < 0 || brackCount < 0 || parenCount < 0
            || (target == RANGLE && angleCount < 0)) {
            --index;
            break;
        }
    }
    return false;
}

QByteArray Parser::lexemUntil(Token target)
{
    int from = index;
    until(target);
    QByteArray s;
    while (from <= index) {
        QByteArray n = symbols.at(from++-1).lexem();
        if (s.size() && n.size()
            && is_ident_char(s.at(s.size()-1))
            && is_ident_char(n.at(0)))
            s += ' ';
        s += n;
    }
    return s;
}

QByteArray Parser::lexemUntilExclusive(Token target)
{
   int from = index;
   until(target);
   QByteArray s;
   ++from;
   while (from <= index - 1) {
       QByteArray n = symbols.at(from++-1).lexem();
       if (s.size() && n.size()
           && is_ident_char(s.at(s.size()-1))
           && is_ident_char(n.at(0)))
           s += ' ';
       s += n;
   }
   return s;
}
