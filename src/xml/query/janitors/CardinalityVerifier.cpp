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
#include "CommonValues.h"
#include "Debug.h"
#include "GenericSequenceType.h"
#include "InsertionIterator.h"
#include "ListIterator.h"
#include "PatternistLocale.h"

#include "CardinalityVerifier.h"

using namespace Patternist;

QString CardinalityVerifier::wrongCardinality(const Cardinality &req,
                                              const Cardinality &got)
{
    return tr("Required cardinality is %1, but encountered %2")
                .arg(formatType(req), formatType(got));
}

Expression::Ptr CardinalityVerifier::verifyCardinality(const Expression::Ptr &operand,
                                                       const Cardinality &requiredCard,
                                                       const ReportContext::Ptr &context,
                                                       const ReportContext::ErrorCode code)
{
    const Cardinality opCard(operand->staticType()->cardinality());
    qDebug() << Q_FUNC_INFO << requiredCard.displayName(Cardinality::IncludeExplanation)
                            << opCard.displayName(Cardinality::IncludeExplanation);

    if(requiredCard.isMatch(opCard))
        return operand;
    else if(requiredCard.canMatch(opCard))
        return Expression::Ptr(new CardinalityVerifier(operand, requiredCard, code));
    else
    {
        /* Sequences within this cardinality can never match. */
        context->error(wrongCardinality(requiredCard, opCard), code, operand.get());
        return operand;
    }
}

CardinalityVerifier::CardinalityVerifier(const Expression::Ptr &operand,
                                         const Cardinality &card,
                                         const ReportContext::ErrorCode code)
                                         : SingleContainer(operand),
                                           m_reqCard(card),
                                           m_allowsMany(operand->staticType()->cardinality().allowsMany()),
                                           m_errorCode(code)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT_X(m_reqCard != Cardinality::zeroOrMore(), Q_FUNC_INFO,
               "It makes no sense to use CardinalityVerifier for cardinality zero-or-more.");
}

Item::Iterator::Ptr CardinalityVerifier::evaluateSequence(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
    const Item next(it->next());

    if(next)
    {
        const Item next2(it->next());

        if(next2)
        {
            if(m_reqCard.allowsMany())
            {
                Item::List start;
                start.append(next);
                start.append(next2);

                return Item::Iterator::Ptr(new InsertionIterator(it, 1, makeListIterator(start)));
            }
            else
            {
                context->error(wrongCardinality(m_reqCard, Cardinality::twoOrMore()), m_errorCode, this);
                return CommonValues::emptyIterator;
            }
        }
        else
            return makeSingletonIterator(next);
    }
    else
    {
        if(m_reqCard.allowsEmpty())
            return CommonValues::emptyIterator;
        else
        {
            context->error(wrongCardinality(m_reqCard, Cardinality::twoOrMore()), m_errorCode, this);
            return CommonValues::emptyIterator;
        }
    }
}

Item CardinalityVerifier::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    if(m_allowsMany)
    {
        const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
        const Item item(it->next());

        if(item)
        {
            if(it->next())
            {
                context->error(wrongCardinality(m_reqCard, it->copy()->cardinality()),
                               m_errorCode, this);
                return Item();
            }
            else
                return item;
        }
        else if(m_reqCard.allowsEmpty())
            return Item();
        else
        {
            context->error(wrongCardinality(m_reqCard), m_errorCode, this);
            return Item();
        }
    }
    else
    {
        const Item item(m_operand->evaluateSingleton(context));

        if(item)
            return item;
        else if(m_reqCard.allowsEmpty())
            return Item();
        else
        {
            context->error(wrongCardinality(m_reqCard), m_errorCode, this);
            return Item();
        }
    }
}

const SourceLocationReflection *CardinalityVerifier::actualReflection() const
{
    return m_operand->actualReflection();
}

Expression::Ptr CardinalityVerifier::typeCheck(const StaticContext::Ptr &context,
                                               const SequenceType::Ptr &reqType)
{
    if(m_reqCard.isMatch(m_operand->staticType()->cardinality()))
        return m_operand->typeCheck(context, reqType);
    else
        return SingleContainer::typeCheck(context, reqType);
}

SequenceType::List CardinalityVerifier::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

SequenceType::Ptr CardinalityVerifier::staticType() const
{
    return makeGenericSequenceType(m_operand->staticType()->itemType(), m_reqCard);
}

ExpressionVisitorResult::Ptr CardinalityVerifier::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
