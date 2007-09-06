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
***************************************************************************
*/

#include "CommonSequenceTypes.h"
#include "PatternistLocale.h"
#include "QNameValue.h"

#include "QNameConstructor.h"

using namespace Patternist;

QNameConstructor::QNameConstructor(const Expression::Ptr &source,
                                   const NamespaceResolver::Ptr &nsResolver) : SingleContainer(source),
                                                                               m_nsResolver(nsResolver)
{
    Q_ASSERT(m_nsResolver);
}

Item QNameConstructor::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    Q_ASSERT(context);
    const QString lexQName(m_operand->evaluateSingleton(context).stringValue());

    const QName expQName(expandQName<DynamicContext::Ptr,
                                     ReportContext::XQDY0074,
                                     ReportContext::XQDY0074>(lexQName,
                                                              context,
                                                              m_nsResolver,
                                                              this));
    return toItem(QNameValue::fromValue(context->namePool(), expQName));
}

QName::NamespaceCode QNameConstructor::namespaceForPrefix(const QName::PrefixCode prefix,
                                                          const StaticContext::Ptr &context,
                                                          const SourceLocationReflection *const r)
{
    Q_ASSERT(context);
    const QName::NamespaceCode ns(context->namespaceBindings()->lookupNamespaceURI(prefix));
    qDebug() << Q_FUNC_INFO << "prefix:" << prefix << "ns:" << ns;

    if(ns == NamespaceResolver::NoBinding)
    {
        context->error(tr("No namespace binding exist for the prefix %1")
                          .arg(formatKeyword(context->namePool()->stringForPrefix(prefix))),
                       ReportContext::XPST0081,
                       r);
        return NamespaceResolver::NoBinding;
    }
    else
        return ns;
}

SequenceType::Ptr QNameConstructor::staticType() const
{
    return CommonSequenceTypes::ExactlyOneQName;
}

SequenceType::List QNameConstructor::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ExactlyOneString);
    return result;
}

ExpressionVisitorResult::Ptr QNameConstructor::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

const SourceLocationReflection *QNameConstructor::actualReflection() const
{
    return m_operand.get();
}

// vim: et:ts=4:sw=4:sts=4
