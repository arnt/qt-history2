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

#include "qcommonsequencetypes_p.h"
#include "qgenericsequencetype_p.h"
#include "qlistiterator_p.h"
#include "qoptimizationpasses_p.h"

#include "qifthenclause_p.h"

using namespace Patternist;

IfThenClause::IfThenClause(const Expression::Ptr &test,
                           const Expression::Ptr &then,
                           const Expression::Ptr &el) : TripleContainer(test, then, el)
{
}

Item::Iterator::Ptr IfThenClause::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return m_operand1->evaluateEBV(context)
            ? m_operand2->evaluateSequence(context)
            : m_operand3->evaluateSequence(context);
}

Item IfThenClause::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return m_operand1->evaluateEBV(context)
            ? m_operand2->evaluateSingleton(context)
            : m_operand3->evaluateSingleton(context);
}

bool IfThenClause::evaluateEBV(const DynamicContext::Ptr &context) const
{
    return m_operand1->evaluateEBV(context)
            ? m_operand2->evaluateEBV(context)
            : m_operand3->evaluateEBV(context);
}

void IfThenClause::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    if(m_operand1->evaluateEBV(context))
        m_operand2->evaluateToSequenceReceiver(context);
    else
        m_operand3->evaluateToSequenceReceiver(context);
}

Expression::Ptr IfThenClause::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(TripleContainer::compress(context));

    if(me.get() != this)
        return me;

    /* All operands mustn't be evaluated in order for const folding to
     * be possible. Let's see how far we get. */

    if(m_operand1->isEvaluated())
    {
        if(m_operand1->evaluateEBV(context->dynamicContext()))
            return m_operand2;
        else
            return m_operand3;
    }
    else
        return me;
}

QList<PlainSharedPtr<OptimizationPass> > IfThenClause::optimizationPasses() const
{
    return OptimizationPasses::ifThenPasses;
}

SequenceType::List IfThenClause::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::EBV);
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

SequenceType::Ptr IfThenClause::staticType() const
{
    const SequenceType::Ptr t1(m_operand2->staticType());
    const SequenceType::Ptr t2(m_operand3->staticType());

    return makeGenericSequenceType(t1->itemType() | t2->itemType(),
                                   t1->cardinality() | t2->cardinality());
}

ExpressionVisitorResult::Ptr IfThenClause::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID IfThenClause::id() const
{
    return IDIfThenClause;
}

// vim: et:ts=4:sw=4:sts=4
