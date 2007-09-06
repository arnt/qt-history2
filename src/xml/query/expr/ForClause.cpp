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
#include "EmptySequence.h"
#include "GenericSequenceType.h"
#include "ItemMappingIterator.h"
#include "ListIterator.h"
#include "OptimizationPasses.h"
#include "SequenceMappingIterator.h"

#include "ForClause.h"

using namespace Patternist;

ForClause::ForClause(const VariableSlotID varSlot,
                     const Expression::Ptr &bindingSequence,
                     const Expression::Ptr &returnExpression,
                     const VariableSlotID positionSlot) : PairContainer(bindingSequence, returnExpression),
                                                          m_varSlot(varSlot),
                                                          m_positionSlot(positionSlot),
                                                          m_allowsMany(false)
{
    Q_ASSERT(m_positionSlot > -2);
    qDebug() << Q_FUNC_INFO << "varSlot: " << varSlot << "positionSlot: " << positionSlot;
}

Item ForClause::mapToItem(const Item &item,
                               const DynamicContext::Ptr &context) const
{
    context->setRangeVariable(m_varSlot, item);
    return m_operand2->evaluateSingleton(context);
}

Item::Iterator::Ptr ForClause::mapToSequence(const Item &item,
                                             const DynamicContext::Ptr &context) const
{
    context->setRangeVariable(m_varSlot, item);
    return m_operand2->evaluateSequence(context);
}

void ForClause::riggPositionalVariable(const DynamicContext::Ptr &context,
                                       const Item::Iterator::Ptr &source) const
{
    if(m_positionSlot > -1)
        context->setPositionIterator(m_positionSlot, source);
}

Item::Iterator::Ptr ForClause::evaluateSequence(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    const Item::Iterator::Ptr source(m_operand1->evaluateSequence(context));

    riggPositionalVariable(context, source);

    if(m_allowsMany)
    {
        return makeSequenceMappingIterator<Item>(ForClause::Ptr(const_cast<ForClause *>(this)),
                                                      source,
                                                      context);
    }
    else
    {
        return makeItemMappingIterator<Item>(ForClause::Ptr(const_cast<ForClause *>(this)),
                                                  source,
                                                  context);
    }
}

Item ForClause::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return evaluateSequence(context)->next();
}

void ForClause::evaluateToSequenceReceiver(const DynamicContext::Ptr &context) const
{
    Item::Iterator::Ptr it;
    const Item::Iterator::Ptr source(m_operand1->evaluateSequence(context));

    riggPositionalVariable(context, source);

    Item next(source->next());

    while(next)
    {
        context->setRangeVariable(m_varSlot, next);
        m_operand2->evaluateToSequenceReceiver(context);
        next = source->next();
    }
}

Expression::Ptr ForClause::typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType)
{
    const Expression::Ptr me(PairContainer::typeCheck(context, reqType));
    const Cardinality card(m_operand1->staticType()->cardinality());

    if(card.isEmpty())
        return EmptySequence::create(this, context);
    else
        return me;

    /* This breaks because  the variable references haven't rewritten themselves, so
     * they dangle. When this is fixed, evaluateSingleton can be removed. */
    /*
    else if(card->allowsMany())
        return me;
    else
        return m_operand2;
        */
}

Expression::Ptr ForClause::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(PairContainer::compress(context));
    if(me.get() != this)
        return me;

    m_allowsMany = m_operand2->staticType()->cardinality().allowsMany();
    return me;
}

SequenceType::Ptr ForClause::staticType() const
{
    const SequenceType::Ptr returnType(m_operand2->staticType());

    return makeGenericSequenceType(returnType->itemType(),
                                   m_operand1->staticType()->cardinality()
                                        * /* multiply operator */
                                   returnType->cardinality());
}

SequenceType::List ForClause::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

ExpressionVisitorResult::Ptr ForClause::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

OptimizationPass::List ForClause::optimizationPasses() const
{
    return OptimizationPasses::forPasses;
}

// vim: et:ts=4:sw=4:sts=4
