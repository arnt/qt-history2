/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
** Copyright (C) 2001-2004 Roberto Raggi
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef TOKENSTREAMADAPTER_H
#define TOKENSTREAMADAPTER_H

#include <QList>
#include <QVector>

#include "tokenengine.h"
#include "tokens.h"
#include "ast.h"

namespace TokenStreamAdapter {

struct Token
{
    uint kind : 16;
    uint hidden : 1;
    int position;
    int length;
    AST *node;
};

//Q_DECLARE_TYPEINFO(Token, Q_MOVABLE_TYPE);

struct TokenStream
{
    TokenStream(TokenEngine::TokenSectionSequence translationUnit, QList<Type> tokenKindList)
    :m_translationUnit(translationUnit),
     m_tokenKindList(tokenKindList),
     m_cursor(0)
     {
        Q_ASSERT(translationUnit.count() == tokenKindList.count());
        m_tokenAstList.resize(tokenKindList.count());
     }

    inline AST* nodeAt(int index = 0) const
    {
        if(index >= m_tokenAstList.count())
            return 0;
        return m_tokenAstList.at(index);
    }

    void setNodeAt(int index, AST *node)
    {
        if(index < m_tokenAstList.count())
            m_tokenAstList[index] = node;
    }

    bool isHidden(int index) const
    {
       if(index >= m_tokenKindList.count())
            return false;
        ::Type type = m_tokenKindList.at(index);
        return (type == Token_whitespaces || type == 10 /*newline*/ ||
                type == Token_comment || type == Token_preproc );
    }

    inline int lookAhead(int n = 0) const
    {
        if(m_cursor + n >= m_tokenKindList.count())
            return 0;
        return m_tokenKindList.at(m_cursor + n);
    }

    inline int currentToken() const
    { return lookAhead(); }

    inline QByteArray currentTokenText() const
    {
        return tokenText(cursor());
    }

    inline QByteArray tokenText(int index = 0) const
    {
        if(index <  m_tokenKindList.count())
            return m_translationUnit.text(index);
        else
            return QByteArray();
    }
/*
    inline int lineOffset(int line) const
    { return m_lines.at(line); }


    void positionAt(int position, int *line, int *column, QByteArray *fileName = 0) const
    {line = 0; column =0; }

    inline void getTokenStartPosition(int index, int *line, int *column, QByteArray *fileName = 0) const
    { positionAt(m_tokens.at(index).position, line, column, fileName); }

    inline void getTokenEndPosition(int index, int *line, int *column, QByteArray *fileName = 0) const
    {
        const Token &tk = m_tokens.at(index);
        positionAt(tk.position + tk.length, line, column, fileName);
    }
*/
    inline void rewind(int index)
    { m_cursor = index; }

    inline int cursor() const
    { return m_cursor; }

    inline void nextToken()
    { ++m_cursor; }

    inline bool tokenAtEnd()
    { return m_cursor >= m_tokenKindList.size(); }

    TokenEngine::TokenSectionSequence tokenSections() const
    { return m_translationUnit;  }

private:
   TokenEngine::TokenSectionSequence m_translationUnit;
   QList<Type> m_tokenKindList;
   QVector<AST*> m_tokenAstList;
   int m_cursor;

/*
    QList<int> m_lines;
    QList<PPLine> m_pplines;
    QList<Token> m_tokens;
    void positionAtAux(int position, int *line, int *column = 0) const;
*/
};

} //namespace TokenStreamAdapter

#endif
