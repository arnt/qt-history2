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

#include "qexternalenvironment_p.h"
#include "qatomicstring_p.h"
#include "qqnameconstructor_p.h"

#include "qsystempropertyfn_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

Item SystemPropertyFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const QString lexQName(m_operands.first()->evaluateSingleton(context).stringValue());

    const QName name
        (QNameConstructor::expandQName<DynamicContext::Ptr,
                                       ReportContext::XTDE1390,
                                       ReportContext::XTDE1390>(lexQName,
                                                                context,
                                                                m_resolver, this));

    return AtomicString::fromValue(ExternalEnvironment::retrieveProperty(name));
}

Expression::Ptr SystemPropertyFN::typeCheck(const StaticContext::Ptr &context,
                                            const SequenceType::Ptr &reqType)
{
    m_resolver = NamespaceResolver::Ptr(context->namespaceBindings());
    Q_ASSERT(m_resolver);

    return FunctionCall::typeCheck(context, reqType);
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
