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

#include "CommonSequenceTypes.h"
#include "GenericSequenceType.h"
#include "ListIterator.h"
#include "SequenceMappingIterator.h"

#include "Path.h"

using namespace Patternist;

Path::Path(const Expression::Ptr &operand1,
           const Expression::Ptr &operand2) : PairContainer(operand1, operand2)
{
}

Item::Iterator::Ptr Path::mapToSequence(const Item &item,
                                        const DynamicContext::Ptr &context) const
{
    Q_ASSERT(item);
    Q_UNUSED(item); /* Needed when compiling in release mode. */
    return m_operand2->evaluateSequence(context);
}

Item::Iterator::Ptr Path::evaluateSequence(const DynamicContext::Ptr &context) const
{
    /* Note, we use the old context for m_operand1. */
    const Item::Iterator::Ptr source(m_operand1->evaluateSequence(context));

    const DynamicContext::Ptr focus(context->createFocus());
    focus->setFocusIterator(source);

    return makeSequenceMappingIterator<Item>(Ptr(const_cast<Path *>(this)), source, focus);
}

Item Path::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    /* This function is called if both operands' cardinality is exactly-one. Therefore
     * we manually go forward in the focus by calling next(). */

    /* Note, we use the old context for m_operand1. */
    const Item::Iterator::Ptr source(m_operand1->evaluateSequence(context));

    const DynamicContext::Ptr focus(context->createFocus());
    focus->setFocusIterator(source);

    /* This test is needed because if the focus is empty, we don't want to(nor can't) evaluate
     * the next step. */
    if(source->next())
        return m_operand2->evaluateSingleton(focus);
    else
        return Item();
}

Expression::Ptr Path::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(PairContainer::compress(context));
    // TODO
    return me;
}

SequenceType::List Path::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreNodes);
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

SequenceType::Ptr Path::staticType() const
{
    const SequenceType::Ptr opType(m_operand2->staticType());

    // TODO infer the type properly
    /* For each parent step, we evaluate the child step. So multiply the two
     * cardinalities. */
    return makeGenericSequenceType(opType->itemType(),
                                   m_operand1->staticType()->cardinality() * opType->cardinality());
}

ExpressionVisitorResult::Ptr Path::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::Properties Path::properties() const
{
    return CreatesFocusForLast;
}

ItemType::Ptr Path::newContextItemType() const
{
    return m_operand1->staticType()->itemType();
}

// vim: et:ts=4:sw=4:sts=4
