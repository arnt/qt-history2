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

#include "AbstractFloat.h"
#include "Boolean.h"
#include "BuiltinTypes.h"
#include "CommonSequenceTypes.h"
#include "EmptySequence.h"
#include "FirstItemPredicate.h"
#include "GenericSequenceType.h"
#include "ItemMappingIterator.h"
#include "ListIterator.h"
#include "Literal.h"
#include "PatternistLocale.h"
#include "TruthPredicate.h"

#include "GenericPredicate.h"

using namespace Patternist;

GenericPredicate::GenericPredicate(const Expression::Ptr &sourceExpression,
                                   const Expression::Ptr &predicate) : PairContainer(sourceExpression,
                                                                                     predicate)
{
}

Expression::Ptr GenericPredicate::create(const Expression::Ptr &sourceExpression,
                                         const Expression::Ptr &predicateExpression,
                                         const StaticContext::Ptr &context,
                                         const QSourceLocation &location)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(sourceExpression);
    Q_ASSERT(predicateExpression);
    Q_ASSERT(context);
    const ItemType::Ptr type(predicateExpression->staticType()->itemType());

    if(predicateExpression->is(IDIntegerValue) &&
       predicateExpression->as<Literal>()->item().as<Numeric>()->toInteger() == 1)
    { /* Handle [1] */
        return Expression::Ptr(new FirstItemPredicate(sourceExpression));
    }
    else if(BuiltinTypes::numeric->xdtTypeMatches(type))
    { /* A numeric predicate, other than [1]. */
        /* TODO at somepoint we'll return a specialized expr here, NumericPredicate or so.
         * Dependency analysis is a bit tricky, since the contained expression can depend on
         * some loop component. */
        return Expression::Ptr(new GenericPredicate(sourceExpression, predicateExpression));
    }
    else if(*CommonSequenceTypes::Empty == *type)
    {
        return EmptySequence::create(predicateExpression.get(), context);
    }
    else if(*BuiltinTypes::item == *type ||
            *BuiltinTypes::xsAnyAtomicType == *type)
    {
        /* The type couldn't be narrowed at compile time, so we use
         * a generic predicate. This check is before the CommonSequenceTypes::EBV check,
         * because the latter matches these types as well. */
        return Expression::Ptr(new GenericPredicate(sourceExpression, predicateExpression));
    }
    else if(CommonSequenceTypes::EBV->itemType()->xdtTypeMatches(type))
    {
        return Expression::Ptr(new TruthPredicate(sourceExpression, predicateExpression));
    }
    else
    {
        context->error(tr("Values of type %1 cannot be predicates. A predicate must either "
                                     "be of a numeric type or an Effective Boolean Value type.")
                                     .arg(formatType(context->namePool(), sourceExpression->staticType())),
                                ReportContext::FORG0006, location);
        return Expression::Ptr(); /* Silence compiler warning. */
    }
}

Item GenericPredicate::mapToItem(const Item &item,
                                      const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr it(m_operand2->evaluateSequence(context));
    const Item pcateItem(it->next());
    qDebug() << Q_FUNC_INFO << "Checking for:" << item;

    if(!pcateItem)
        return Item(); /* The predicate evaluated to the empty sequence */
    else if(pcateItem.isNode())
        return item;
    /* Ok, now it must be an AtomicValue */
    else if(BuiltinTypes::numeric->xdtTypeMatches(pcateItem.type()))
    { /* It's a positional predicate. */
        qDebug() << Q_FUNC_INFO << "Evaluating a numeric predicate, COMP:" << context->contextPosition() << "and" << pcateItem.as<Numeric>()->toInteger();
        if(it->next())
        {
            context->error(tr("A numeric predicate can only exist of one numeric value."
                              "A numeric value and one or more values were passed."),
                              ReportContext::FORG0006, this);
            return Item();
        }

        if(Double::isEqual(static_cast<xsDouble>(context->contextPosition()),
                           pcateItem.as<Numeric>()->toDouble()))
        {
            return item;
        }
        else
            return Item();
    }
    else if(Boolean::evaluateEBV(pcateItem, it, context)) /* It's a truth predicate. */
        return item;
    else
        return Item();
}

Item::Iterator::Ptr GenericPredicate::evaluateSequence(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr focus(m_operand1->evaluateSequence(context));
    const DynamicContext::Ptr newContext(context->createFocus());
    newContext->setFocusIterator(focus);

    return makeItemMappingIterator<Item>(GenericPredicate::Ptr(const_cast<GenericPredicate *>(this)),
                                              focus,
                                              newContext);
}

Item GenericPredicate::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr focus(m_operand1->evaluateSequence(context));
    const DynamicContext::Ptr newContext(context->createFocus());
    newContext->setFocusIterator(focus);
    return mapToItem(focus->next(), newContext);
}

SequenceType::List GenericPredicate::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

SequenceType::Ptr GenericPredicate::staticType() const
{
    const SequenceType::Ptr type(m_operand1->staticType());
    return makeGenericSequenceType(type->itemType(),
                                   type->cardinality() | Cardinality::zeroOrOne());
}

ExpressionVisitorResult::Ptr GenericPredicate::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

ItemType::Ptr GenericPredicate::newContextItemType() const
{
    return m_operand1->staticType()->itemType();
}

Expression::Properties GenericPredicate::properties() const
{
    return CreatesFocusForLast;
}

QString GenericPredicate::description() const
{
    return QLatin1String("predicate");
}

// vim: et:ts=4:sw=4:sts=4
