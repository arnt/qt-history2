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

#include <QList>
#include <QMultiMap>
#include "tokenengine.h"
#include "tokenreplacements.h"
#include "textreplacement.h"

class ReplaceToken
{
public:
    ReplaceToken(QList<TokenReplacement*> &tokenReplacementRules);
    TextReplacements getTokenTextReplacements(const TokenEngine::TokenContainer &tokenContainer);
private:
    bool isInterestingToken(const QByteArray &token);
    bool isPreprocessorDirective(const QByteArray &token);
    QMultiMap<QByteArray, TokenReplacement*> tokenRuleLookup;
    QList<TokenReplacement*> tokenReplacementRules;
};

#endif
