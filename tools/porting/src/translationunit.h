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
#include "codemodel.h"
#include "smallobject.h"
#include "cpplexer.h"
#include "parser.h"
#include "semantic.h"

class TranslationUnitData : public QSharedData
{
public:
    TranslationUnitData(const TokenEngine::TokenSectionSequence &t)
    :tokens(t),  globalScope(0) {};
    TokenEngine::TokenSectionSequence tokens;
    CodeModel::NamespaceScope *globalScope;
    TypedPool<CodeModel::Item> codeModelMemoryPool;
};

class TranslationUnit
{
public:
    TranslationUnit();
    TranslationUnit(const TokenEngine::TokenSectionSequence &tokens);
    TokenEngine::TokenSectionSequence tokens() const;
    CodeModel::NamespaceScope *codeModel();
    TypedPool<CodeModel::Item> *codeModelMemoryPool();
private:
    friend class TranslationUnitAnalyzer;
    void setCodeModel(CodeModel::NamespaceScope *globalScope);
    QExplicitlySharedDataPointer<TranslationUnitData> d;
};

class TranslationUnitAnalyzer
{
public:
    TranslationUnit analyze
            (const TokenEngine::TokenSectionSequence &translationUnitTokens, int targetMaxASTNodes = 10000);
private:
    CppLexer lexer;
    Parser parser;
};

#endif
