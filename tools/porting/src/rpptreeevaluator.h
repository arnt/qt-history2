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
#include "tokenengine.h"
#include "rpp.h"

namespace Rpp {

class DefineMap : public QMap<QByteArray, const DefineDirective *>
{

};


class RppTreeEvaluator: public QObject
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
    void evaluateItem(const Item *item);
    void evaluateSource(const Source *source);
    void evaluateDirective(const Directive *directive);
    void evaluateIncludeDirective(const IncludeDirective *directive);
    void evaluateDefineDirective(const DefineDirective *directive);
    void evaluateUndefDirective(const UndefDirective *directive);
    void evaluateIfdefDirective(const IfdefDirective *directive);
    void evaluateIfndefDirective(const IfndefDirective *directive);
    void evaluateIfDirective(const IfDirective *directive);
    void evaluateElseDirective(const ElseDirective *directive);
    void evaluateEndifDirective(const EndifDirective *directive);
    void evaluateErrorDirective(const ErrorDirective *directive);
    void evaluatePragmaDirective(const PragmaDirective *directive);
    void evaluateIfSection(const IfSection *ifSection);
    void evaluateConditionalDirective(const ConditionalDirective *conditionalDirective);
    void evaluateText(const Text *text);
    void evaluateItemComposite(const ItemComposite *itemComposite);
    bool evaluateCondition(const ConditionalDirective *conditionalDirective);
    int evaluateExpression(Expression *expression);

    TokenEngine::TokenContainer evaluateMacro(TokenEngine::TokenContainer tokenContainer, int &identiferTokenIndex);
    TokenEngine::TokenContainer cloneTokenList(const TokenEngine::TokenList &list);
    Source *getParentSource(const Item *item) const;
    IncludeType includeTypeFromDirective(
                    const IncludeDirective *includeDirective) const;
private:
    QList<TokenEngine::TokenSection> m_tokenSections;
    DefineMap *m_activeDefinitions;
    TokenEngine::TokenSection *newlineSection;
};

}//namespace Rpp
#endif
