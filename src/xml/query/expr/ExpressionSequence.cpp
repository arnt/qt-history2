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

#include "CardinalityVerifier.h"
#include "CommonSequenceTypes.h"
#include "EmptySequence.h"
#include "ListIterator.h"
#include "SequenceMappingIterator.h"

#include "ExpressionSequence.h"

using namespace Patternist;

ExpressionSequence::ExpressionSequence(const Expression::List &ops) : UnlimitedContainer(ops)
{
    Q_ASSERT_X(1 < ops.count(), Q_FUNC_INFO,
               "It makes no sense to have an ExpressionSequence containing less than two expressions.");
}

Item::Iterator::Ptr ExpressionSequence::mapToSequence(const Expression::Ptr &expr,
                                                      const DynamicContext::Ptr &context) const
{
    return expr->evaluateSequence(context);
}

Item::Iterator::Ptr ExpressionSequence::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return makeSequenceMappingIterator<Item>
                (ExpressionSequence::Ptr(const_cast<ExpressionSequence *>(this)),
                                         makeListIterator(m_operands),
                                         context);
}

void ExpressionSequence::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    Expression::List::const_iterator it(m_operands.constBegin());
    const Expression::List::const_iterator end(m_operands.constEnd());
    Expression::List result;

    for(; it != end; ++it)
        (*it)->evaluateToSequenceReceiver(context);
}

Expression::Ptr ExpressionSequence::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(UnlimitedContainer::compress(context));

    if(me.get() != this)
        return me;

    Expression::List::const_iterator it(m_operands.constBegin());
    const Expression::List::const_iterator end(m_operands.constEnd());
    Expression::List result;

    for(; it != end; ++it)
    {
        const ID Id = (*it)->id();

        /* User function call sites that are of type empty-sequence() must be avoided since
         * they may contain calls to fn:error(), which we would rewrite away otherwise. */
        if((*it)->staticType()->cardinality().isEmpty() && Id != IDUserFunctionCallsite)
        {
            /* Rewrite "(1, (), 2)" into "(1, 2)" by not
             * adding (*it) to result. */
            continue;
        }
        else if(Id == IDExpressionSequence)
        {
            /* Rewrite "(1, (2, 3), 4)" into "(1, 2, 3, 4)" */
            Expression::List::const_iterator seqIt((*it)->operands().constBegin());
            const Expression::List::const_iterator seqEnd((*it)->operands().constEnd());

            for(; seqIt != seqEnd; ++seqIt)
                result.append(*seqIt);
        }
        else
            result.append(*it);
    }

    if(result.isEmpty())
        return EmptySequence::create(this, context);
    else if(result.count() == 1)
        return result.first();
    else
    {
        m_operands = result;
        return me;
    }
}

Expression::Ptr ExpressionSequence::typeCheck(const StaticContext::Ptr &context,
                                              const SequenceType::Ptr &reqType)
{
    Q_ASSERT(reqType);
    Expression::List::iterator it(m_operands.begin());
    const Expression::List::iterator end(m_operands.end());

    /* We treat the cardinality differently here by allowing the empty sequence
     * for each individual Expression, since the Cardinality can be conformed to by
     * the ExpressionSequence as a whole(which we check for at the end). */
    const SequenceType::Ptr testOnlyIT(makeGenericSequenceType(reqType->itemType(),
                                                               Cardinality::empty() |
                                                               reqType->cardinality()));

    for(; it != end; ++it)
        *it = (*it)->typeCheck(context, testOnlyIT);

    /* The above loop is only guaranteed to find item type errors, but the cardinality
     * can still be wrong since the operands were treated individually. */
    return CardinalityVerifier::verifyCardinality(Expression::Ptr(this), reqType->cardinality(), context);
}

Expression::Properties ExpressionSequence::properties() const
{
    const Expression::List::const_iterator end(m_operands.constEnd());
    Expression::List::const_iterator it;
    bool allEvaled = true;
    Expression::Properties props = DisableElimination;

    for(it = m_operands.constBegin(); it != end; ++it)
    {
        const Expression::Properties newp((*it)->properties());
        props |= newp;

        if((newp & IsEvaluated) != IsEvaluated)
        {
            allEvaled = false;
            break;
        }
    }

    if(allEvaled)
        return props;
    else
        return props & ~IsEvaluated; /* Remove IsEvaluated. */
}

SequenceType::Ptr ExpressionSequence::staticType() const
{
    return operandsUnionType<ProductOfCardinality>();
}

SequenceType::List ExpressionSequence::expectedOperandTypes() const
{
    SequenceType::List result;
    /* ExpressionSequence is a bit strange type wise since it has an
     * infinite amount of operands. */
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

ExpressionVisitorResult::Ptr ExpressionSequence::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::ID ExpressionSequence::id() const
{
    return IDExpressionSequence;
}

// vim: et:ts=4:sw=4:sts=4
