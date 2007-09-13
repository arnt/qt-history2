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

#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qdistinctiterator_p.h"
#include "qemptysequence_p.h"
#include "qgenericsequencetype_p.h"
#include "qindexofiterator_p.h"
#include "qinsertioniterator_p.h"
#include "qinteger_p.h"
#include "qlistiterator_p.h"
#include "qremovaliterator_p.h"
#include "qsubsequenceiterator_p.h"

#include "qsequencefns_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

bool BooleanFN::evaluateEBV(const DynamicContext::Ptr &context) const
{
    return m_operands.first()->evaluateEBV(context);
}

Expression::Ptr BooleanFN::typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType)
{
    if(*CommonSequenceTypes::EBV->itemType() == *reqType->itemType())
        return m_operands.first()->typeCheck(context, reqType);
    else
        return FunctionCall::typeCheck(context, reqType);
}

Item::Iterator::Ptr IndexOfFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return Item::Iterator::Ptr(new IndexOfIterator(m_operands.first()->evaluateSequence(context),
                                                   m_operands.at(1)->evaluateSingleton(context),
                                                   comparator(), context,
                                                   Expression::Ptr(const_cast<IndexOfFN *>(this))));
}

Expression::Ptr IndexOfFN::typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType)
{
    const Expression::Ptr me(FunctionCall::typeCheck(context, reqType));
    const ItemType::Ptr t1(m_operands.first()->staticType()->itemType());
    const ItemType::Ptr t2(m_operands.at(1)->staticType()->itemType());

    if(*CommonSequenceTypes::Empty == *t1 ||
       *CommonSequenceTypes::Empty == *t2)
    {
        return EmptySequence::create(this, context);
    }
    else
    {
        prepareComparison(fetchComparator(t1, t2, context));
        return me;
    }
}

Item::Iterator::Ptr DistinctValuesFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return Item::Iterator::Ptr(new DistinctIterator(m_operands.first()->evaluateSequence(context),
                                                    comparator(),
                                                    Expression::Ptr(const_cast<DistinctValuesFN *>(this)),
                                                    context));
}

Expression::Ptr DistinctValuesFN::typeCheck(const StaticContext::Ptr &context,
                                            const SequenceType::Ptr &reqType)
{
    const Expression::Ptr me(FunctionCall::typeCheck(context, reqType));
    const ItemType::Ptr t1(m_operands.first()->staticType()->itemType());

    if(*CommonSequenceTypes::Empty == *t1)
        return EmptySequence::create(this, context);
    else if(!m_operands.first()->staticType()->cardinality().allowsMany())
        return m_operands.first();
    else if(BuiltinTypes::xsAnyAtomicType->xdtTypeMatches(t1))
        return me;
    else
    {
        prepareComparison(fetchComparator(t1, t1, context));
        return me;
    }
}

SequenceType::Ptr DistinctValuesFN::staticType() const
{
    const SequenceType::Ptr t(m_operands.first()->staticType());
    return makeGenericSequenceType(t->itemType(),
                                   t->cardinality().allowsMany() ? Cardinality::oneOrMore()
                                                                 : Cardinality::exactlyOne());
}

Item::Iterator::Ptr InsertBeforeFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr target(m_operands.first()->evaluateSequence(context));
    const Item::Iterator::Ptr inserts(m_operands.at(2)->evaluateSequence(context));

    xsInteger position = m_operands.at(1)->evaluateSingleton(context).as<Numeric>()->toInteger();

    if(position < 1)
        position = 1;

    return Item::Iterator::Ptr(new InsertionIterator(target, position, inserts));
}

Item InsertBeforeFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return evaluateSequence(context)->next();
}

SequenceType::Ptr InsertBeforeFN::staticType() const
{
    const SequenceType::Ptr t1(m_operands.first()->staticType());
    const SequenceType::Ptr t2(m_operands.last()->staticType());

    return makeGenericSequenceType(t1->itemType() | t2->itemType(),
                                   t1->cardinality() + t2->cardinality());
}

Item::Iterator::Ptr RemoveFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const xsInteger pos = m_operands.last()->evaluateSingleton(context).as<Numeric>()->toInteger();
    const Item::Iterator::Ptr it(m_operands.first()->evaluateSequence(context));

    if(pos < 1)
        return it;

    return Item::Iterator::Ptr(new RemovalIterator(it, pos));
}

Item RemoveFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const xsInteger pos = m_operands.last()->evaluateSingleton(context).as<Numeric>()->toInteger();
    if(pos <= 1)
        return Item();

    return m_operands.first()->evaluateSingleton(context);
}

SequenceType::Ptr RemoveFN::staticType() const
{
    const SequenceType::Ptr opType(m_operands.first()->staticType());
    const Cardinality c(opType->cardinality());

    if(c.minimum() == 0)
        return makeGenericSequenceType(opType->itemType(), c);
    else
    {
        return makeGenericSequenceType(opType->itemType(),
                                       Cardinality::fromRange(c.minimum() - 1,
                                                              c.maximum()));
    }
}

Item::Iterator::Ptr ReverseFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    return m_operands.first()->evaluateSequence(context)->toReversed();
}

Expression::Ptr ReverseFN::typeCheck(const StaticContext::Ptr &context,
                                     const SequenceType::Ptr &reqType)
{
    if(m_operands.first()->staticType()->cardinality().allowsMany())
        return FunctionCall::typeCheck(context, reqType);
    else
        return m_operands.first()->typeCheck(context, reqType);
}

SequenceType::Ptr ReverseFN::staticType() const
{
    return m_operands.first()->staticType();
}

Item::Iterator::Ptr SubsequenceFN::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr it(m_operands.first()->evaluateSequence(context));

    xsInteger startingLoc = m_operands.at(1)->evaluateSingleton(context).as<Numeric>()->round()->toInteger();
    xsInteger length = -1;

    if(m_operands.count() == 3)
    {
        length = m_operands.last()->evaluateSingleton(context).as<Numeric>()->toInteger();

        if(startingLoc + length < 1 || (startingLoc > (startingLoc + length)))
            return CommonValues::emptyIterator;
    }

    /* F&O, 15.1.10, "If $startingLoc is zero or negative, the
     * subsequence includes items from the beginning of the $sourceSeq." */
    if(startingLoc < 1)
        startingLoc = 1;

    if(length < 1 && length != -1)
        return CommonValues::emptyIterator;
    else
        return Item::Iterator::Ptr(new SubsequenceIterator(it, startingLoc, length));
}

Item SubsequenceFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    return evaluateSequence(context)->next();
}

Expression::Ptr SubsequenceFN::compress(const StaticContext::Ptr &context)
{
    qDebug() << Q_FUNC_INFO;
    const Expression::Ptr me(FunctionCall::compress(context));
    if(me.get() != this)
        return me;

    const Expression::Ptr lenArg(m_operands.value(2));
    if(lenArg && lenArg->isEvaluated())
    {
        const xsInteger length = lenArg->as<Literal>()->item().as<Numeric>()->round()->toInteger();

        if(length <= 0)
            return EmptySequence::create(this, context);
    }

    return me;
}

SequenceType::Ptr SubsequenceFN::staticType() const
{
    const SequenceType::Ptr opType(m_operands.first()->staticType());
    const Cardinality opCard(opType->cardinality());

    /* The subsequence(expr, 1, 1), add empty-sequence() to the static type. */
    if(m_operands.at(1)->isEvaluated()                                                      &&
       m_operands.count() == 3                                                              &&
       m_operands.at(2)->isEvaluated()                                                      &&
       m_operands.at(1)->as<Literal>()->item().as<Numeric>()->round()->toInteger() == 1    &&
       m_operands.at(2)->as<Literal>()->item().as<Numeric>()->round()->toInteger() == 1)
    {
        return makeGenericSequenceType(opType->itemType(),
                                       opCard.toWithoutMany());
    }
    else
    {
        return makeGenericSequenceType(opType->itemType(),
                                       opCard | Cardinality::zeroOrOne());
    }

    Cardinality card;

    if(opCard.isEmpty())
        card = Cardinality::empty();
    else
        card = opCard.toWithoutMany();

    return makeGenericSequenceType(opType->itemType(), card);
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
