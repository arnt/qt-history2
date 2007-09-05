/****************************************************************************
**
** Copyright (C) 2006-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the Patternist project on Trolltech Labs.
**
** $TROLLTECH_GPL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************
*/

#include "CachingIterator.h"
#include "CommonSequenceTypes.h"
#include "OperandsIterator.h"
#include "ListIterator.h"
#include "NodeBuilder.h"

#include "EvaluationCache.h"

using namespace Patternist;

EvaluationCache::EvaluationCache(const Expression::Ptr &op,
                                 const VariableDeclaration::Ptr &varDecl,
                                 const VariableSlotID aSlot) : SingleContainer(op),
                                                               m_declaration(varDecl),
                                                               m_slot(aSlot)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(m_declaration);
    Q_ASSERT(m_slot > -1);
}

Item EvaluationCache::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    ItemCacheCell &cell = context->itemCacheCell(m_slot);
    if(cell.cacheState == ItemCacheCell::Full)
        return cell.cachedItem;
    else
    {
        Q_ASSERT(cell.cacheState == ItemCacheCell::Empty);
        cell.cachedItem = m_operand->evaluateSingleton(context);
        cell.cacheState = ItemCacheCell::Full;
        return cell.cachedItem;
    }
}

Item::Iterator::Ptr EvaluationCache::evaluateSequence(const DynamicContext::Ptr &context) const
{
    ItemSequenceCacheCell::Vector &cells = context->itemSequenceCacheCells(m_slot);
    ItemSequenceCacheCell &cell = cells[m_slot];

    qDebug() << Q_FUNC_INFO << "CELL STATE: " << cell.cacheState << "slot:" << m_slot;

    switch(cell.cacheState)
    {
        case ItemSequenceCacheCell::Full:
            return makeListIterator(cell.cachedItems);
        case ItemSequenceCacheCell::Empty:
        {
            cell.sourceIterator = m_operand->evaluateSequence(context);
            cell.cacheState = ItemSequenceCacheCell::PartiallyPopulated;
            /* Fallthrough. */
        }
        case ItemSequenceCacheCell::PartiallyPopulated:
            return Item::Iterator::Ptr(new CachingIterator(cells, m_slot, context));
        default:
        {
            Q_ASSERT_X(false, Q_FUNC_INFO, "This path is not supposed to be run.");
            return Item::Iterator::Ptr();
        }
    }
}

Expression::Ptr EvaluationCache::typeCheck(const StaticContext::Ptr &context,
                                           const SequenceType::Ptr &reqType)
{
    qDebug() << Q_FUNC_INFO << "reqType:" << formatType(context->namePool(), reqType);
    /* It's important that we do the typeCheck() before checking for the use of local variables,
     * because ExpressionVariableReference can reference an expression that is a local variable,
     * so it must rewrite itself to it operand before, and it does that in EvaluationCache::typeCheck(). */
    const Expression::Ptr me(SingleContainer::typeCheck(context, reqType));

    OperandsIterator it(me);
    Expression::Ptr next(it.next());

    /* If our operand or any sub operand gets its value from a for-loop, we cannot
     * cache it since then our cache would be filled -- but not invalidated -- on the
     * first for-iteration. Consider this query:
     *
     * <tt>for $i in expr
     * let $v := $i/p
     * return ($v, $v)</tt>
     *
     * An evaluation cache is inserted for the two operands in the return clause. However,
     * $i changes for each iteration so the cache can only be active on a per-iteration basis,
     * it it's possible(which it isn't).
     *
     * This means that for some queries we don't cache what we really should, and hence evaluate
     * in a sub-optimal way, since this DependsOnLocalVariable don't communicate whether it references
     * a loop that affects us. The correct fix for this would be to let ForExpression reset the
     * relevant caches only, but we don't know which ones that are. */
    while(next)
    {
        if(next->has(DependsOnLocalVariable))
        {
            qDebug() << Q_FUNC_INFO << "Rewriting to operand" << endl;
            return m_operand->typeCheck(context, reqType);
        }

        next = it.next();
    }

    qDebug() << "Returning me!";
    return me;
}

Expression::Ptr EvaluationCache::compress(const StaticContext::Ptr &context)
{
    qDebug() << Q_FUNC_INFO;
    const Expression::Ptr me(SingleContainer::compress(context));

    if(me.get() != this)
        return me;

    qDebug() << "Testing.." << m_operand->id();
    if(m_operand->is(IDRangeVariableReference))
        return m_operand;

    qDebug() << "Used by:" << m_declaration->references.count();
    if(m_declaration->usedByMany())
    {
        qDebug() << "Used by many..";
        /* If it's only an atomic value an EvaluationCache is overkill. However,
         * it's still needed for functions like fn:current-time() that must adhere to
         * query stability. */
        const Properties props(m_operand->properties());

        if((props & IsEvaluated) &&
           (!(props & DisableElimination)) &&
           CommonSequenceTypes::ExactlyOneAtomicType->matches(m_operand->staticType()))
        {
            qDebug() << "Ret op.";
            return m_operand;
        }
        else
            return me;
    }
    else
    {
        qDebug() << "Used by one ONLY.";
        /* If we're only used once, there's no need for an EvaluationCache. */
        return m_operand;
    }
}

SequenceType::Ptr EvaluationCache::staticType() const
{
    return m_operand->staticType();
}

SequenceType::List EvaluationCache::expectedOperandTypes() const
{
    /* Remember that EvaluationCache::typeCheck() will be called from multiple locations,
     * which potentially have different type requirements. For instance, one wants a node,
     * and another requires atomization and casting.
     *
     * Returning ZeroOrMoreItems is safe here because staticType() returns the operand's type
     * and therefore the convertors like Atomizer will be parents to us, and hence only affect
     * the relevant path.
     *
     * ZeroOrMoreItems also make sense logically since we're actually only used where the
     * variable references reference us. */
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);

    return result;
}

Expression::Properties EvaluationCache::properties() const
{
    /* We cannot return the operand's properties unconditionally, because some
     * doesn't hold for this Expression.
     *
     * However, some of the properties must propagate through, which are the ones being OR'd here.
     */
    return m_operand->properties() & (DisableElimination | IsEvaluated | DisableTypingDeduction);
}

ExpressionVisitorResult::Ptr
EvaluationCache::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

const SourceLocationReflection *EvaluationCache::actualReflection() const
{
    return m_operand->actualReflection();
}

// vim: et:ts=4:sw=4:sts=4
