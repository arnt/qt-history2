/****************************************************************************
**
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
#include "rpptreewalker.h"

namespace Rpp {

void RppTreeWalker::evaluateItem(const Item *item)
{
    if(!item)
        return;
    if (Source *source = item->toSource())
        evaluateSource(source);
    else if (Directive *directive = item->toDirective())
        evaluateDirective(directive);
    else if (IfSection *ifSection = item->toIfSection())
        evaluateIfSection(ifSection);
    else if (Text *text = item->toText())
        evaluateText(text);
}

void RppTreeWalker::evaluateItemComposite(const ItemComposite *itemComposite)
{
    if (!itemComposite)
        return;
    for (int i = 0; i < itemComposite->count(); ++i) {
        evaluateItem(itemComposite->item(i));
    }
}

void RppTreeWalker::evaluateSource(const Source *source)
{
    evaluateItemComposite(source->toItemComposite());
}

void RppTreeWalker::evaluateDirective(const Directive *directive)
{
    if (!directive)
        return;
    if (EmptyDirective *dir = directive->toEmptyDirective())
        evaluateEmptyDirective(dir);
    else if (ErrorDirective *dir = directive->toErrorDirective())
        evaluateErrorDirective(dir);
    else if (PragmaDirective *dir = directive->toPragmaDirective())
        evaluatePragmaDirective(dir);
    else if (IncludeDirective *dir = directive->toIncludeDirective())
        evaluateIncludeDirective(dir);
    else if (DefineDirective *dir = directive->toDefineDirective())
        evaluateDefineDirective(dir);
    else if (UndefDirective *dir = directive->toUndefDirective())
        evaluateUndefDirective(dir);
    else if (LineDirective *dir = directive->toLineDirective())
        evaluateLineDirective(dir);
    else if (NonDirective *dir = directive->toNonDirective())
        evaluateNonDirective(dir);
    else if (NonDirective *dir = directive->toNonDirective())
        evaluateNonDirective(dir);
    else if (ConditionalDirective *dir = directive->toConditionalDirective())
        evaluateConditionalDirective(dir);
}

/*
    This function evaluates all the branches of an IfSection. You should 
    override it if you want to only evaluate the "correct" branch.
*/
void RppTreeWalker::evaluateIfSection(const IfSection *ifSection)
{
    if (!ifSection)
        return;
    evaluateItemComposite(ifSection->toItemComposite());
}

void RppTreeWalker::evaluateConditionalDirective(const ConditionalDirective *conditionalDirective)
{
    if (!conditionalDirective)
        return;
    if (IfdefDirective *dir = conditionalDirective->toIfdefDirective())
         evaluateIfdefDirective(dir);
    else if (IfndefDirective *dir = conditionalDirective->toIfndefDirective())
         evaluateIfndefDirective(dir);
    else if (IfDirective *dir = conditionalDirective->toIfDirective())
         evaluateIfDirective(dir);
    else if (ElifDirective *dir = conditionalDirective->toElifDirective())
         evaluateElifDirective(dir);
    else if (ElseDirective *dir = conditionalDirective->toElseDirective())
         evaluateElseDirective(dir);
}

void RppTreeWalker::evaluateIfdefDirective(const IfdefDirective *directive)
{
   if (!directive) 
       return;
   evaluateItemComposite(directive->toItemComposite());
}

void RppTreeWalker::evaluateIfndefDirective(const IfndefDirective *directive)
{
   if (!directive) 
       return;
   evaluateItemComposite(directive->toItemComposite());
}

void RppTreeWalker::evaluateIfDirective(const IfDirective *directive)
{
   if (!directive) 
       return;
   evaluateItemComposite(directive->toItemComposite());
}

void RppTreeWalker::evaluateElifDirective(const ElifDirective *directive)
{
   if (!directive) 
       return;
   evaluateItemComposite(directive->toItemComposite());
}

void RppTreeWalker::evaluateElseDirective(const ElseDirective *directive)
{
   if (!directive) 
       return;
   evaluateItemComposite(directive->toItemComposite());
}

}
