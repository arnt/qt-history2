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
#include "translationunit.h"
#include <iostream>

using namespace std;
using namespace TokenEngine;
using namespace CodeModel;
using namespace TokenStreamAdapter;

TranslationUnit::TranslationUnit()
{
    TokenSectionSequence empty;
    d = new TranslationUnitData(empty);
}

TranslationUnit::TranslationUnit(const TokenEngine::TokenSectionSequence &tokens)
{ d = new TranslationUnitData(tokens); }

TokenSectionSequence TranslationUnit::tokens() const
{ return d->tokens; }

void TranslationUnit::setCodeModel(NamespaceScope *globalScope)
{ d->globalScope = globalScope; }

NamespaceScope *TranslationUnit::codeModel()
{ return d->globalScope; }

TypedPool<CodeModel::Item> *TranslationUnit::codeModelMemoryPool()
{ return &d->codeModelMemoryPool; }



/*
    Performs C++ parsing and semantic analysis on a translation unit.
    Returns a TranslationUnit, which contains all the data.
*/
TranslationUnit TranslationUnitAnalyzer::analyze
        (const TokenSectionSequence &translationUnitTokens, int targetMaxASTNodes)
{
    TranslationUnit translationUnit(translationUnitTokens);
    CodeModel::NamespaceScope *codeModel =
        CodeModel::Create<CodeModel::NamespaceScope>(translationUnit.codeModelMemoryPool());
    translationUnit.setCodeModel(codeModel);

    // run lexical analysis
    QVector< ::Type> typeList = lexer.lex(translationUnitTokens);
    TokenStreamAdapter::TokenStream tokenStream(translationUnitTokens, typeList);

    Semantic semantic(codeModel, &tokenStream, translationUnit.codeModelMemoryPool());

    // parse and run semantic on the translation unit
    bool done = false;
    while(!done) {
        pool p;
        TranslationUnitAST *node = parser.parse(&tokenStream, &p, targetMaxASTNodes, done);
        semantic.parseTranslationUnit(node);
    }

    return translationUnit;
}
