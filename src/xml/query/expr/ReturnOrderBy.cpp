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
#include "ListIterator.h"
#include "SortTuple.h"

#include "ReturnOrderBy.h"

using namespace Patternist;

ReturnOrderBy::ReturnOrderBy(const OrderBy::Stability aStability,
                             const OrderBy::OrderSpec::Vector &oSpecs,
                             const Expression::List &ops) : UnlimitedContainer(ops),
                                                            m_stability(aStability),
                                                            m_orderSpecs(oSpecs)
{
    Q_ASSERT_X(ops.size() >= 2, Q_FUNC_INFO,
               "ReturnOrderBy must have the return expression, and at least one sort key.");
    Q_ASSERT(m_orderSpecs.size() == ops.size() - 1);
}

Item ReturnOrderBy::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    // TODO This is temporary code.
    return m_operands.first()->evaluateSingleton(context);

    Q_ASSERT(m_operands.size() > 1);
    const Item::Iterator::Ptr value(m_operands.first()->evaluateSequence(context));
    Item::Vector sortKeys;

    /* We're skipping the first operand. */
    const int len = m_operands.size() - 1;
    sortKeys.reserve(len);

    for(int i = 1; i < len; ++i)
        sortKeys[i - 1] = m_operands.at(i)->evaluateSingleton(context);

    return Item(new SortTuple(value, sortKeys));
}

Item::Iterator::Ptr ReturnOrderBy::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return m_operands.first()->evaluateSequence(context);
}

bool ReturnOrderBy::evaluateEBV(const DynamicContext::Ptr &context) const
{
    // TODO This is temporary code.
    return m_operands.first()->evaluateEBV(context);
}

Expression::Ptr ReturnOrderBy::compress(const StaticContext::Ptr &context)
{
    /* We don't need the members, so don't keep a reference to them. */
    m_orderSpecs.clear();

    return UnlimitedContainer::compress(context);
}

ExpressionVisitorResult::Ptr ReturnOrderBy::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

SequenceType::Ptr ReturnOrderBy::staticType() const
{
    return m_operands.first()->staticType();
}

SequenceType::List ReturnOrderBy::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    result.append(CommonSequenceTypes::ZeroOrMoreAtomicTypes);
    return result;
}

Expression::ID ReturnOrderBy::id() const
{
    qDebug() << Q_FUNC_INFO;
    return IDReturnOrderBy;
}

// vim: et:ts=4:sw=4:sts=4
