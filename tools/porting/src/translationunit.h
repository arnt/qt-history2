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
#ifndef TRANSLATIONUNIT_H
#define TRANSLATIONUNIT_H

#include <QSharedData>
#include "tokenengine.h"
#include "tokenstreamadapter.h"
#include "ast.h"
#include "codemodel2.h"
#include "smallobject.h"
#include "cpplexer.h"
#include "parser.h"
#include "semantic2.h"


class TranslationUnitData : public QSharedData
{
public:
    TranslationUnitData(const TokenEngine::TokenSectionSequence &t)
    :tokens(t) {};
    ~TranslationUnitData();
    TokenEngine::TokenSectionSequence tokens;
    const TranslationUnitAST *syntaxTree;
    const CodeModel::NamespaceScope *globalScope;
    const TokenStreamAdapter::TokenStream *tokenStream;
    pool memoryPool;
};

class TranslationUnit
{
public:
    TranslationUnit();
    TranslationUnit(const TokenEngine::TokenSectionSequence &tokens);
    TokenEngine::TokenSectionSequence tokens() const;
    const TokenStreamAdapter::TokenStream *tokenStream() const;
    const TranslationUnitAST *syntaxTree() const;
    const CodeModel::NamespaceScope *codeModel() const;
    pool *memoryPool();
private:
    friend class TranslationUnitAnalyzer;
    void setTokenStream(const TokenStreamAdapter::TokenStream *tokenStream);
    void setSyntaxTree(const TranslationUnitAST *syntaxTree);
    void setCodeModel(const CodeModel::NamespaceScope *globalScope);
    QExplicitlySharedDataPointer<TranslationUnitData> d;
};

class TranslationUnitAnalyzer
{
public:
    TranslationUnit analyze
            (const TokenEngine::TokenSectionSequence &translationUnitTokens);
private:
    void assignSemantic(TokenEngine::TokenSectionSequence tokens,
                        QMap<int, CodeModel::NameUse *> nameUses);
    CppLexer lexer;
    Parser parser;
    Semantic semantic;
};

#endif
