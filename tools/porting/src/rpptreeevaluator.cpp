/****************************************************************************
**
** Copyright (C) 2001-2004 Roberto Raggi
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "rpptreeevaluator.h"

using namespace TokenEngine;
namespace Rpp {

RppTreeEvaluator::RppTreeEvaluator()
{
    QByteArray text("\n");
    TokenEngine::Token token;
    token.start = 0;
    token.length = 1;
    QList<TokenEngine::Token> tokenList;
    tokenList.append(token);
    TokenContainer newLineContainer(text, tokenList, new TokenEngine::GeneratedInfo());
    newlineSection= new TokenSection(newLineContainer, 0, 1);
}

RppTreeEvaluator::~RppTreeEvaluator()
{
    delete newlineSection;
}

TokenSectionSequence RppTreeEvaluator::evaluate(const Source *source,
                                                DefineMap *activeDefinitions)
{
    m_tokenSections.clear();
    m_activeDefinitions = activeDefinitions;
    evaluateSource(source);
    return TokenSectionSequence(m_tokenSections);
}


void RppTreeEvaluator::evaluateItem(const Item *item)
{
    if(Directive *directive = item->toDirective())
        evaluateDirective(directive);
    else if(IfSection *ifSection = item->toIfSection())
        evaluateIfSection(ifSection);
    else if (Text *text = item->toText())
        evaluateText(text);
}

void RppTreeEvaluator::evaluateItemComposite(const ItemComposite *itemComposite)
{
    for(int i=0; i<itemComposite->count(); ++i) {
        evaluateItem(itemComposite->item(i));
    }
}

void RppTreeEvaluator::evaluateSource(const Source *source)
{
    evaluateItemComposite(source->toItemComposite());
}

void RppTreeEvaluator::evaluateText(const Text *textLine)
{
    const int numTokens = textLine->count();
   // cout << "numtokens:" << numTokens << endl;;
    TokenContainer tokenContainer = textLine->text().tokenContainer(0);

    int t = 0;
    int startTokenRun = 0;
    while(t < numTokens) {
        const Token *currentToken = textLine->token(t);
        int currentContainerIndex = currentToken->index();

        //handle comments
        if(currentToken->toLineComment() || currentToken->toMultiLineComment()) {
            //create section
            TokenSection section(tokenContainer, textLine->token(startTokenRun)->index(), t - startTokenRun );
            m_tokenSections.append(section);
            ++t; //skip comment
            startTokenRun = t;
        }

        // handle escaped newlines
        // TODO handle other escaped stuff
        if (tokenContainer.text(currentContainerIndex) == "\\" )   {
            if(tokenContainer.text(currentContainerIndex + 1 ) == "\n") {
                //create section
                TokenSection section(tokenContainer, textLine->token(startTokenRun)->index(), t - startTokenRun );
                m_tokenSections.append(section);
                t += 2;
                startTokenRun = t;
            }
        }

        //handle macro replacements
        if(currentToken->toIdToken()) {
            const int tokenIndex = currentToken->index();
            const QByteArray tokenText = tokenContainer.text(tokenIndex);
            if(m_activeDefinitions->contains(tokenText)) {
                //crate section
                TokenSection section(tokenContainer, textLine->token(startTokenRun)->index(), t - startTokenRun  );
                m_tokenSections.append(section);
                //evaluate macro
                TokenContainer evaluatedText = evaluateMacro(tokenContainer, currentContainerIndex);
                TokenSection evalSection(evaluatedText, 0, evaluatedText.count());
                m_tokenSections.append(evalSection);
                t = currentContainerIndex;
                startTokenRun = t;
            }
        }
        ++t;
    }
    //round up any tokens at the end and put them in a section
    if(t - startTokenRun > 1) {
        TokenSection section(tokenContainer, textLine->token(startTokenRun)->index(), t - startTokenRun );
        m_tokenSections.append(section);
    }

    m_tokenSections.append(*newlineSection);
}

void RppTreeEvaluator::evaluateDirective(const Directive *directive)
{
    Q_ASSERT(directive);
    if(IncludeDirective *dir = directive->toIncludeDirective())
        evaluateIncludeDirective(dir);
    else if(DefineDirective *dir = directive->toDefineDirective())
        evaluateDefineDirective(dir);
    else if(UndefDirective *dir = directive->toUndefDirective())
        evaluateUndefDirective(dir);
}

/*
    Evaluates and ifsection by selecting which one of the if-elif-else
    groups and then evaling that.
*/
void RppTreeEvaluator::evaluateIfSection(const IfSection *ifSection)
{
    ConditionalDirective *ifGroup = ifSection->ifGroup();
    if(evaluateCondition(ifGroup)) {
        evaluateConditionalDirective(ifGroup);
        return;
    }

    QList<ConditionalDirective *> elifGroups = ifSection->elifGroups();
    foreach(ConditionalDirective *elifGroup, elifGroups) {
        if(evaluateCondition(elifGroup)) {
            evaluateConditionalDirective(elifGroup);
            return;
        }
    }

    ConditionalDirective *elseGroup = ifSection->elseGroup();
    if(elseGroup)
        evaluateConditionalDirective(elseGroup);
}
/*
    Every conditionaldirective is an ItemComposite, so we evaluate that.
*/
void RppTreeEvaluator::evaluateConditionalDirective(const ConditionalDirective *conditionalDirective)
{
    if (ItemComposite *itemComposite = conditionalDirective->toItemComposite())
        evaluateItemComposite(itemComposite);
}

/*
    Evaluate an IncludeDirective by evaluating the Source for the included
    file. The source is found by emitting the  includeCallback signal, which
    must be handled outside RppTreeEvaluator.
*/
void RppTreeEvaluator::evaluateIncludeDirective(const IncludeDirective *directive)
{
    Source *currentSource = getParentSource(directive);
    IncludeType includeType = includeTypeFromDirective(directive);
    Source *newSource = 0;
    emit includeCallback(newSource, currentSource, directive->filename(), includeType);
    Q_ASSERT(newSource);    // If you get an assert here you probably
                            // forgot to connect to the includeCallack signal
    evaluateSource(newSource);
}

void RppTreeEvaluator::evaluateDefineDirective(const DefineDirective *directive)
{
    m_tokenSections.append(*newlineSection);
    m_activeDefinitions->insert(directive->identifier().fullText(), directive);
}

void RppTreeEvaluator::evaluateUndefDirective(const UndefDirective *directive)
{
    m_tokenSections.append(*newlineSection);
    const QByteArray text = directive->identifier().fullText();
    m_activeDefinitions->remove(text);
}

/*
    Evaluate the truth-value of an conditionalDirective
*/
bool RppTreeEvaluator::evaluateCondition(const ConditionalDirective *conditionalDirective)
{
    if (IfDirective *ifDirective = conditionalDirective->toIfDirective())
        return (evaluateExpression(ifDirective->expression()) != 0);
    if (ElifDirective *elifDirective = conditionalDirective->toElifDirective())
        return (evaluateExpression(elifDirective->expression()) != 0);
    if (IfdefDirective *ifdefDirective = conditionalDirective->toIfdefDirective())
        return m_activeDefinitions->contains(ifdefDirective->identifier().fullText());
    if (IfndefDirective *ifndefDirective = conditionalDirective->toIfndefDirective())
        return !m_activeDefinitions->contains(ifndefDirective->identifier().fullText());
    else
        return false; //error!
}

/*
    Recursively evaluates an Expression
*/
int RppTreeEvaluator::evaluateExpression(Expression *expression)
{
    if (IntLiteral *e = expression->toIntLiteral()) {
        return e->value();
    } else if (StringLiteral *e = expression->toStringLiteral()) {
        return e->value().size();
    } else if (MacroReference *e = expression->toMacroReference()) {
       switch(e->type()) {
           case MacroReference::DefinedRef: {
               return m_activeDefinitions->contains(e->name().fullText()) ? 1 : 0;
           } case MacroReference::ValueRef: {
               const QByteArray identifier = e->name().fullText();
               if (m_activeDefinitions->contains(identifier)) {
                   int token = e->name().containerIndex(0);
                   TokenContainer value = evaluateMacro(e->name().tokenContainer(token), token);
                   return QString(value.fullText()).toInt(0, 0);
               } else {
                   return 0; // error
               }
           }
           default: Q_ASSERT(0);
        }
    } else if (MacroFunctionReference *e = expression->toMacroFunctionReference()) {
        Q_UNUSED(e);
        //TODO handle MacroFunctionReference
//        DefineDirective *def = e->findDefinition(e->name());
//        Q_ASSERT(def->toMacroFunctionDefinition());
//        qWarning("not implemented yet");
        return 0;
    } else if (UnaryExpression *e = expression->toUnaryExpression()) {
        int result = evaluateExpression(e->expression());
        switch (e->op()) {
            case '+': return + result;
            case '-': return - result;
            case '!': return ! result;
            case '~': return ~ result;
            default:  Q_ASSERT(0);
        }
    } else if (BinaryExpression *e = expression->toBinaryExpression()) {
        int v1 = evaluateExpression(e->leftExpression());
        int v2 = evaluateExpression(e->rightExpression());

        switch (e->op()) {
            case '/': { return v2 ? v1 / v2 : 0; } //avoid division by zero
            case '*':                  return v1 * v2;
            case '%':                  return v1 % v2;
            case '+':                  return v1 + v2;
            case '-':                  return v1 - v2;
            case '<':                  return v1 < v2;
            case '>':                  return v1 > v2;
            case '&':                  return v1 & v2;
            case '^':                  return v1 ^ v2;
            case '|':                  return v1 | v2;
            case Expression::LtEqOp:   return v1 <= v2;
            case Expression::GtEqOp:   return v1 >= v2;
            case Expression::EqOp:     return v1 == v2;
            case Expression::NotEqOp:  return v1 != v2;
            case Expression::AndOp:    return v1 && v2;
            case Expression::OrOp:     return v1 || v2;
            case Expression::LShiftOp: return v1 << v2;
            case Expression::RShiftOp: return v1 >> v2;
            default:    Q_ASSERT(0);
        }

    } else if ( ConditionalExpression *e = expression->toConditionalExpression()){
        return e->condition() ? evaluateExpression(e->leftExpression()) : evaluateExpression(e->rightExpression());
    }
    return 0;
}

TokenContainer RppTreeEvaluator::evaluateMacro(TokenContainer tokenContainer, int &identiferTokenIndex)
{
    QByteArray identifierText = tokenContainer.text(identiferTokenIndex);

    if(!m_activeDefinitions->contains(identifierText))
        return TokenContainer();

    const Rpp::DefineDirective *directive = m_activeDefinitions->value(identifierText);
    Q_ASSERT(directive);

    if(Rpp::MacroDefinition *macro = directive->toMacroDefinition()) {
        ++identiferTokenIndex;
        return cloneTokenList(macro->replacementList());
    } else if (Rpp::MacroFunctionDefinition *macro = directive->toMacroFunctionDefinition()) {
        Q_UNUSED(macro);
    }

    return TokenContainer();
}

TokenContainer RppTreeEvaluator::cloneTokenList(const TokenList &list)
{
    QByteArray text;
    QList<TokenEngine::Token> tokens;
    int index = 0;
    for (int t = 0; t<list.count(); ++t) {
        const QByteArray tokenText = list.text(t);
        const int textLength = tokenText.count();
        text += tokenText;
        TokenEngine::Token token;
        token.start = index;
        token.length = textLength;
        tokens.append(token);
        index += textLength;
    }
    TokenContainer container(text, tokens, new GeneratedInfo());
    return container;
}

/*
    returns the parent Source for a given item.
*/
Source *RppTreeEvaluator::getParentSource(const Item *item) const
{
    Q_ASSERT(item);
    while(item->toSource() == 0) {
        item = item->parent();
        Q_ASSERT(item);
    }

    return item->toSource();
}
/*
    We have to IncludeType enums, one in IncludeDirective and one in
    RppTreeEvaluator. This function translates between them.
*/
RppTreeEvaluator::IncludeType RppTreeEvaluator::includeTypeFromDirective(
                    const IncludeDirective *includeDirective) const
{
    if(includeDirective->includeType() == IncludeDirective::QuoteInclude)
        return QuoteInclude;
    else
        return AngleBracketInclude;
}



} //namespace Rpp
