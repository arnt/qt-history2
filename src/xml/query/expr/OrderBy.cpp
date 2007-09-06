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

#include <QtAlgorithms>

#include "CommonSequenceTypes.h"
#include "ListIterator.h"
#include "PatternistLocale.h"
#include "NodeBuilder.h"
#include "SortTuple.h"

#include "OrderBy.h"

using namespace Patternist;

OrderBy::OrderBy(const Stability stability,
                 const OrderSpec::Vector &aOrderSpecs,
                 const Expression::Ptr &op) : SingleContainer(op),
                                              m_stability(stability),
                                              m_orderSpecs(aOrderSpecs)
{
}

/**
 * @short Functor used by Qt's qSort() and qStableSort(). Used for FLWOR's
 * <tt>order by</tt> expression.
 */
template<>
class qLess<Item::List>
{
public:
    inline qLess(const OrderBy::OrderSpec::Vector &orderspecs) : m_orderSpecs(orderspecs)
    {
        Q_ASSERT(!m_orderSpecs.isEmpty());
    }

    inline bool operator()(const Item &item1, const Item &item2) const
    {
        const SortTuple *const s1 = item1.as<SortTuple>();
        const SortTuple *const s2 = item2.as<SortTuple>();

        const Item::Vector &sortKeys1 = s1->sortKeys();
        const Item::Vector &sortKeys2 = s2->sortKeys();
        const int len = sortKeys1.count();
        Q_ASSERT(sortKeys1.count() == sortKeys2.count());

        for(int i = 0; i < len; ++i)
        {
            const AtomicComparator::ComparisonResult result = m_orderSpecs.at(i).comparator->compare(sortKeys1.at(i),
                                                                                                     AtomicComparator::OperatorLessThan,
                                                                                                     sortKeys2.at(i));
            Q_ASSERT(m_orderSpecs.at(i).direction == OrderBy::OrderSpec::Ascending ||
                     m_orderSpecs.at(i).direction == OrderBy::OrderSpec::Descending);

            switch(result)
            {
                case AtomicComparator::LessThan:
                    return m_orderSpecs.at(i).direction == OrderBy::OrderSpec::Ascending ? true : false;
                case AtomicComparator::GreaterThan:
                    return m_orderSpecs.at(i).direction == OrderBy::OrderSpec::Ascending ? false : true;
                case AtomicComparator::Equal:
                    continue;
            }
        }

        return false;
    }

private:
    const OrderBy::OrderSpec::Vector m_orderSpecs;
};

Item::Iterator::Ptr OrderBy::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return m_operand->evaluateSequence(context);

#if 0
    Item::List tuples(m_operand->evaluateSequence(context)->toList());

    const qLess<Item::List> sorter(m_orderSpecs);

    Q_ASSERT(m_stability == StableOrder || m_stability == UnstableOrder);

    if(m_stability == StableOrder)
        qStableSort(tuples.begin(), tuples.end(), sorter);
    else
        qSort(tuples.begin(), tuples.end(), sorter);

    typedef QPair<Item::List, Item::Iterator::Ptr> SparseSortTuple;
    QList<SparseSortTuple> tuples;

    /* Potentially, we deal with many items here, so deallocate the Iterator as early
     * as possible by using a scope. */
    {
        const Item::Iterator::Ptr sortTuples(m_operand->evaluateSequence(context));
        Item tuple(sortTuples->next());


        while(tuple)
        {
            const SortTuple *const st = tuple->as<SortTuple>();
            tuples.append(qMakePair(st->sortKeys(), st->value()));
            tuple = sortTuples->next();
        }
    }

    /* Now, we sort. */
    //qSort(tuples.begin() tuples.end());
    return m_operand->evaluateSequence(context);
#endif
}

Expression::Ptr OrderBy::typeCheck(const StaticContext::Ptr &context,
                                   const SequenceType::Ptr &reqType)
{
    /* It's not meaningful to sort a single item or less, so rewrite ourselves
     * away if that is the case. This is an optimization. */
    return SingleContainer::typeCheck(context, reqType);
    /* TODO: How do we remove ReturnOrderBy?
    if(Cardinality::zeroOrOne().isMatch(m_operand->staticType()->cardinality()))
        return m_operand->typeCheck(context, reqType);
    else
        return SingleContainer::typeCheck(context, reqType);
     */
}

SequenceType::Ptr OrderBy::staticType() const
{
    return m_operand->staticType();
}

SequenceType::List OrderBy::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

ExpressionVisitorResult::Ptr
OrderBy::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
