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
#ifndef RPPTREEEVALUATOR_H
#define RPPTREEEVALUATOR_H

#include <QObject>
#include <QList>
#include <QHash>
#include <QSet>
#include "tokenengine.h"
#include "rpp.h"
#include "rpptreewalker.h"

namespace Rpp {

class DefineMap : public QHash<QByteArray, const DefineDirective *>
{

};

class RppTreeEvaluator: public QObject, public RppTreeWalker
{
Q_OBJECT
public:
    RppTreeEvaluator();
    ~RppTreeEvaluator();
    TokenEngine::TokenSectionSequence evaluate(const Source *source,
                                              DefineMap *activedefinitions);
    enum IncludeType {QuoteInclude, AngleBracketInclude};
signals:
    void includeCallback(Source *&includee, const Source *includer,
                         const QString &filename, IncludeType includeType);
protected:
    void evaluateIncludeDirective(const IncludeDirective *directive);
    void evaluateDefineDirective(const DefineDirective *directive);
    void evaluateUndefDirective(const UndefDirective *directive);
    void evaluateIfSection(const IfSection *ifSection);
    void evaluateText(const Text *text);
    bool evaluateCondition(const ConditionalDirective *conditionalDirective);
    int evaluateExpression(Expression *expression);

    TokenEngine::TokenContainer evaluateMacro(TokenEngine::TokenContainer tokenContainer, int &identiferTokenIndex);
    TokenEngine::TokenContainer evaluateMacroInternal(QSet<QByteArray> skip, TokenEngine::TokenContainer tokenContainer);
    TokenEngine::TokenContainer cloneTokenList(const TokenEngine::TokenList &list);
    Source *getParentSource(const Item *item) const;
    IncludeType includeTypeFromDirective(
                    const IncludeDirective *includeDirective) const;
private:
    QVector<TokenEngine::TokenSection> m_tokenSections;
    DefineMap *m_activeDefinitions;
    TokenEngine::TokenSection *newlineSection;
};

class MacroFunctionParser
{
public:
    MacroFunctionParser(const TokenEngine::TokenContainer &tokenContainer, int startToken);
    bool isValid();
    int tokenCount();
    int argumentCount();
    TokenEngine::TokenSection argument(int argumentIndex);
private:
    const TokenEngine::TokenContainer &m_tokenContainer;
    const int m_startToken;
    int m_numTokens;
    bool m_valid;
    QVector<TokenEngine::TokenSection> m_arguments;
};

}//namespace Rpp
#endif
