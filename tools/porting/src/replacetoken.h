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

#ifndef REPLACETOKEN_H
#define REPLACETOKEN_H

#include "tokenreplacements.h"
#include "textreplacement.h"
#include "lexer.h"
#include <QList>
#include <QMultiMap>

void printTokenStream(TokenStream *stream);
void printContents(QByteArray contents);

class ReplaceToken
{
public:
    ReplaceToken(QList<TokenReplacement*> tokenReplacementRules);
    TextReplacements getTokenTextReplacements(FileSymbol *inFileSymbol);
private:
    bool isInterestingToken(QByteArray token);
    bool isPreprocessorDirective(QByteArray token);
    QMultiMap<QByteArray, TokenReplacement*> tokenRuleLookup;
    QList<TokenReplacement*> tokenReplacementRules;
};

#endif
