/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGlobal>

#include "qexpression_p.h"
#include "qstaticcontext_p.h"

#include "qparsercontext_p.h"

using namespace Patternist;

ParserContext::ParserContext(const StaticContext::Ptr &context,
                             const ExpressionFactory::LanguageAccent lang,
                             const Tokenizer::Ptr &tokener) : staticContext(context)
                                                            , tokenizer(tokener)
                                                            , languageAccent(lang)
                                                            , nodeTestSource(BuiltinTypes::element)
                                                            , moduleNamespace(StandardNamespaces::empty)
                                                            , isPreviousEnclosedExpr(false)
                                                            , elementConstructorDepth(0)
                                                            , m_evaluationCacheSlot(-1)
                                                            , m_rangeSlot(-1)
                                                            , m_expressionSlot(0)
                                                            , m_positionSlot(-1)
{
    resolvers.push(context->namespaceBindings());
    Q_ASSERT(tokenizer);
    Q_ASSERT(context);
}

// vim: et:ts=4:sw=4:sts=4
