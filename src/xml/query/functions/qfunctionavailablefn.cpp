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

#include "qboolean_p.h"
#include "qinteger_p.h"
#include "qqnameconstructor_p.h"

#include "qfunctionavailablefn_p.h"

using namespace Patternist;

Item FunctionAvailableFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const QString lexQName(m_operands.first()->evaluateSingleton(context).stringValue());

    const QName name
        (QNameConstructor::expandQName<DynamicContext::Ptr,
                                       ReportContext::XTDE1400,
                                       ReportContext::XTDE1400>(lexQName,
                                                                context,
                                                                m_resolver,
                                                                this));

    /* Note, value() is used, not at(). */
    const Integer::Ptr arityArg(m_operands.value(1)->evaluateSingleton(context).as<Integer>());
    xsInteger arity;

    if(arityArg)
        arity = arityArg->toInteger();
    else
        arity = FunctionSignature::UnlimitedArity;

    return Boolean::fromValue(m_functionFactory->isAvailable(context->namePool(), name, arity));
}

Expression::Ptr FunctionAvailableFN::typeCheck(const StaticContext::Ptr &context,
                                               const SequenceType::Ptr &reqType)
{
    m_functionFactory = context->functionSignatures();
    Q_ASSERT(m_functionFactory);
    m_defFuncNS = context->defaultFunctionNamespace();
    /* m_defFuncNS can be empty/null or an actual value. */
    m_resolver = context->namespaceBindings();
    Q_ASSERT(m_resolver);

    return FunctionCall::typeCheck(context, reqType);
}

// vim: et:ts=4:sw=4:sts=4
