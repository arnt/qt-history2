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
#include <treedump.h>

using namespace TokenEngine;
using namespace CodeModel;
using namespace TokenStreamAdapter;

TranslationUnitData::~TranslationUnitData()
{
    delete tokenStream;
}

TranslationUnit::TranslationUnit()
{
    TokenSectionSequence empty;
    d = new TranslationUnitData(empty);
}

TranslationUnit::TranslationUnit(const TokenEngine::TokenSectionSequence &tokens)
{ d = new TranslationUnitData(tokens); }

TokenSectionSequence TranslationUnit::tokens() const
{ return d->tokens; }

void TranslationUnit::setTokenStream(const TokenStreamAdapter::TokenStream *tokenStream)
{ d->tokenStream = tokenStream;}

const TokenStreamAdapter::TokenStream *TranslationUnit::tokenStream() const
{ return d->tokenStream; }

void TranslationUnit::setSyntaxTree(const TranslationUnitAST *syntaxTree)
{ d->syntaxTree = syntaxTree; }

const TranslationUnitAST *TranslationUnit::syntaxTree() const
{ return d->syntaxTree; }

void TranslationUnit::setCodeModel(const NamespaceScope *globalScope)
{ d->globalScope = globalScope; }

const NamespaceScope *TranslationUnit::codeModel() const
{ return d->globalScope; }

pool *TranslationUnit::memoryPool()
{ return &d->memoryPool; }


/*
    Performs C++ parsing and semantic analysis on a translation unit.
    Returns a TranslationUnit, which contains all the data.
*/
TranslationUnit TranslationUnitAnalyzer::analyze
        (const TokenSectionSequence &translationUnitTokens)
{
    TranslationUnit translationUnit(translationUnitTokens);

    // run lexical analysis
    QList< ::Type> typeList = lexer.lex(translationUnitTokens);
    TokenStreamAdapter::TokenStream *tokenStream
            = new TokenStreamAdapter::TokenStream(translationUnitTokens, typeList);

    translationUnit.setTokenStream(tokenStream);

    // run C++ parser, create AST
    TranslationUnitAST *node = parser.parse
            (tokenStream, translationUnit.memoryPool());

    translationUnit.setSyntaxTree(node);

    // run semantic analysis
    CodeModel::SemanticInfo semanticInfo =  semantic.parseTranslationUnit
                    (node, tokenStream, translationUnit.memoryPool());

    translationUnit.setCodeModel(semanticInfo.codeModel);

    //Assign per-token semantic info to each token.
    assignSemantic(translationUnit.tokens(), semanticInfo.nameUses);

    return translationUnit;
}

void TranslationUnitAnalyzer::assignSemantic(TokenSectionSequence tokens,
                            QMap<int, CodeModel::NameUse *> nameUses)
{
    for(int t=0; t < tokens.count(); ++t) {
        if(!nameUses.contains(t))
            continue;
        TokenContainer container = tokens.tokenContainer(t);
        const int containerIndex = tokens.containerIndex(t);
        TokenAttributes *attributes = container.tokenAttributes();
        attributes->addAttribute("Name Use", containerIndex, nameUses.value(t));
    }
}
