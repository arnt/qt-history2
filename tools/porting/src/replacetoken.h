/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef REPLACETOKEN_H
#define REPLACETOKEN_H

#include "tokenengine.h"
#include "tokenreplacements.h"
#include "textreplacement.h"
#include <QList>
#include <QMultiMap>

class ReplaceToken
{
public:
    ReplaceToken(const QList<TokenReplacement*> &tokenReplacementRules);
    TextReplacements getTokenTextReplacements(const TokenEngine::TokenContainer &tokenContainer);
private:
    bool isInterestingToken(const QByteArray &token);
    bool isPreprocessorDirective(const QByteArray &token);
    QMultiMap<QByteArray, TokenReplacement*> tokenRuleLookup;
    const QList<TokenReplacement*> tokenReplacementRules;
};

#endif
