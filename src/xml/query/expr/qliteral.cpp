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

#include "Boolean.h"
#include "BuiltinTypes.h"
#include "CommonSequenceTypes.h"
#include "GenericSequenceType.h"

#include "Literal.h"

using namespace Patternist;

Literal::Literal(const Item &i) : m_item(i)
{
    qDebug() << Q_FUNC_INFO << this;
    Q_ASSERT(m_item);
    Q_ASSERT(m_item.isAtomicValue());
}

Item Literal::evaluateSingleton(const DynamicContext::Ptr &) const
{
    return m_item;
}

bool Literal::evaluateEBV(const DynamicContext::Ptr &context) const
{
    return Boolean::evaluateEBV(m_item, context);
}

void Literal::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    context->outputReceiver()->item(m_item);
}

SequenceType::Ptr Literal::staticType() const
{
    return makeGenericSequenceType(m_item.type(), Cardinality::exactlyOne());
}

ExpressionVisitorResult::Ptr Literal::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID Literal::id() const
{
    const ItemType::Ptr t(m_item.type());

    if(BuiltinTypes::xsBoolean->xdtTypeMatches(t))
        return IDBooleanValue;
    else if(BuiltinTypes::xsString->xdtTypeMatches(t) ||
            BuiltinTypes::xsAnyURI->xdtTypeMatches(t) ||
            BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(t))
    {
        return IDStringValue;
    }
    else if(BuiltinTypes::xsInteger->xdtTypeMatches(t))
        return IDIntegerValue;
    else
        return IDIgnorableExpression;
}

Expression::Properties Literal::properties() const
{
    return IsEvaluated;
}

QString Literal::description() const
{
    return m_item.stringValue();
}

// vim: et:ts=4:sw=4:sts=4
