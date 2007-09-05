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

#include "Boolean.h"
#include "BuiltinTypes.h"
#include "CommonSequenceTypes.h"
#include "Debug.h"
#include "EmptySequence.h"
#include "GenericSequenceType.h"
#include "Literal.h"
#include "PatternistLocale.h"
#include "Numeric.h"
#include "UntypedAtomicConverter.h"

#include "ArithmeticExpression.h"

using namespace Patternist;

ArithmeticExpression::ArithmeticExpression(const Expression::Ptr &op1,
                                           const AtomicMathematician::Operator op,
                                           const Expression::Ptr &op2) : PairContainer(op1, op2),
                                                                         m_op(op)
{
}

Item ArithmeticExpression::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item op1(m_operand1->evaluateSingleton(context));
    if(!op1)
        return Item();

    const Item op2(m_operand2->evaluateSingleton(context));
    if(!op2)
        return Item();

    return flexiblyCalculate(op1, m_op, op2, m_mather, context, this);
}

Item ArithmeticExpression::flexiblyCalculate(const Item &op1,
                                             const AtomicMathematician::Operator op,
                                             const Item &op2,
                                             const AtomicMathematician::Ptr &mather,
                                             const DynamicContext::Ptr &context,
                                             const SourceLocationReflection *const reflection,
                                             const ReportContext::ErrorCode code)
{
    qDebug() << Q_FUNC_INFO;
    if(mather)
        return mather->calculate(op1, op, op2, context);

    Expression::Ptr a1(new Literal(op1));
    Expression::Ptr a2(new Literal(op2));

    const AtomicMathematician::Ptr ingela(fetchMathematician(a1, a2, op, true, context, reflection, code));

    return ingela->calculate(a1->evaluateSingleton(context),
                             op,
                             a2->evaluateSingleton(context),
                             context);
}

Expression::Ptr ArithmeticExpression::typeCheck(const StaticContext::Ptr &context,
                                                const SequenceType::Ptr &reqType)
{
    const Expression::Ptr me(PairContainer::typeCheck(context, reqType));
    const ItemType::Ptr t1(m_operand1->staticType()->itemType());
    const ItemType::Ptr t2(m_operand2->staticType()->itemType());

    if(*CommonSequenceTypes::Empty == *t1 ||
       *CommonSequenceTypes::Empty == *t2)
    {
        return EmptySequence::create(this, context);
    }

    if(*BuiltinTypes::xsAnyAtomicType == *t1    ||
       *BuiltinTypes::xsAnyAtomicType == *t2    ||
       *BuiltinTypes::numeric == *t1            ||
       *BuiltinTypes::numeric == *t2)
    {
        qDebug() << Q_FUNC_INFO << "Doing AtomicMathematician lookup at runtime";
        /* The static type of (at least) one of the operands could not
         * be narrowed further than xs:anyAtomicType, so we do the operator
         * lookup at runtime. */
        return me;
    }

    m_mather = fetchMathematician(m_operand1, m_operand2, m_op, true, context, this);

    return me;
}

AtomicMathematician::Ptr
ArithmeticExpression::fetchMathematician(Expression::Ptr &op1,
                                         Expression::Ptr &op2,
                                         const AtomicMathematician::Operator op,
                                         const bool issueError,
                                         const ReportContext::Ptr &context,
                                         const SourceLocationReflection *const reflection,
                                         const ReportContext::ErrorCode code)
{
    qDebug() << Q_FUNC_INFO;
    ItemType::Ptr t1(op1->staticType()->itemType());
    ItemType::Ptr t2(op2->staticType()->itemType());

    if(BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(t1))
    {
        op1 = Expression::Ptr(new UntypedAtomicConverter(op1, BuiltinTypes::xsDouble));
        /* The types might have changed, reload. */
        t1 = op1->staticType()->itemType();
    }

    if(BuiltinTypes::xsUntypedAtomic->xdtTypeMatches(t2))
    {
        op2 = Expression::Ptr(new UntypedAtomicConverter(op2, BuiltinTypes::xsDouble));
        /* The types might have changed, reload. */
        t2 = op2->staticType()->itemType();
    }

    const AtomicMathematicianLocator::Ptr locator
        (static_cast<const AtomicType *>(t1.get())->mathematicianLocator());

    if(!locator)
    {
        qDebug() << "Doing AtomicMathematician lookup at runtime..";
        if(!issueError)
            return AtomicMathematician::Ptr();

        context->error(tr("No arithmetics, such as %1, can be performed "
                                        "involving the type %2.")
                                        .arg(formatKeyword(AtomicMathematician::displayName(op)))
                                        .arg(formatType(context->namePool(), t1)),
                                   code, reflection);
        return AtomicMathematician::Ptr();
    }

    const AtomicMathematician::Ptr comp
        (static_cast<const AtomicType *>(t2.get())->accept(locator, op, reflection));

    if(comp)
        return comp;

    if(!issueError)
        return AtomicMathematician::Ptr();

    context->error(tr("Operator %1 is not available between atomic values of type %2 and %3.")
                                    .arg(formatKeyword(AtomicMathematician::displayName(op)))
                                    .arg(formatType(context->namePool(), t1))
                                    .arg(formatType(context->namePool(), t2)),
                   code, reflection);
    return AtomicMathematician::Ptr();
}

SequenceType::Ptr ArithmeticExpression::staticType() const
{
    Cardinality card;

    /* These variables are important because they ensure staticType() only
     * gets called once from this function. Before, this lead to strange
     * semi-infinite recursion involving many arithmetic expressions. */
    const SequenceType::Ptr st1(m_operand1->staticType());
    const SequenceType::Ptr st2(m_operand2->staticType());

    if(st1->cardinality().allowsEmpty() ||
       st2->cardinality().allowsEmpty())
    {
        card = Cardinality::zeroOrOne();
    }
    else
        card = Cardinality::exactlyOne();

    if(m_op == AtomicMathematician::IDiv)
        return makeGenericSequenceType(BuiltinTypes::xsInteger, card);

    const ItemType::Ptr t1(st1->itemType());
    const ItemType::Ptr t2(st2->itemType());
    ItemType::Ptr returnType;

    /* Please, make this beautiful? */
    if(BuiltinTypes::xsTime->xdtTypeMatches(t1) ||
       BuiltinTypes::xsDate->xdtTypeMatches(t1) ||
       BuiltinTypes::xsDateTime->xdtTypeMatches(t1))
    {
        if(BuiltinTypes::xsDuration->xdtTypeMatches(t2))
            returnType = t1;
        else
            returnType = BuiltinTypes::xsDayTimeDuration;
    }
    else if(BuiltinTypes::xsYearMonthDuration->xdtTypeMatches(t1))
    {
        if(m_op == AtomicMathematician::Div &&
           BuiltinTypes::xsYearMonthDuration->xdtTypeMatches(t2))
        {
            returnType = BuiltinTypes::xsDecimal;
        }
        else if(BuiltinTypes::numeric->xdtTypeMatches(t2))
            returnType = BuiltinTypes::xsYearMonthDuration;
        else
            returnType = t2;
    }
    else if(BuiltinTypes::xsYearMonthDuration->xdtTypeMatches(t2))
    {
        returnType = BuiltinTypes::xsYearMonthDuration;
    }
    else if(BuiltinTypes::xsDayTimeDuration->xdtTypeMatches(t1))
    {
        if(m_op == AtomicMathematician::Div &&
           BuiltinTypes::xsDayTimeDuration->xdtTypeMatches(t2))
        {
            returnType = BuiltinTypes::xsDecimal;
        }
        else if(BuiltinTypes::numeric->xdtTypeMatches(t2))
            returnType = BuiltinTypes::xsDayTimeDuration;
        else
            returnType = t2;
    }
    else if(BuiltinTypes::xsDayTimeDuration->xdtTypeMatches(t2))
    {
        returnType = BuiltinTypes::xsDayTimeDuration;
    }
    else if(BuiltinTypes::xsDouble->xdtTypeMatches(t1) ||
            BuiltinTypes::xsDouble->xdtTypeMatches(t2))
    {
        returnType = BuiltinTypes::xsDouble;
    }
    else if(BuiltinTypes::xsFloat->xdtTypeMatches(t1) ||
            BuiltinTypes::xsFloat->xdtTypeMatches(t2))
    {
        returnType = BuiltinTypes::xsFloat;
    }
    else if(BuiltinTypes::xsInteger->xdtTypeMatches(t1) &&
            BuiltinTypes::xsInteger->xdtTypeMatches(t2))
    {
        /* "A div B  numeric  numeric  op:numeric-divide(A, B)
         * numeric; but xs:decimal if both operands are xs:integer" */
        if(m_op == AtomicMathematician::Div)
            returnType = BuiltinTypes::xsDecimal;
        else
            returnType = BuiltinTypes::xsInteger;
    }
    else
    {
        /* The types are either xs:decimals, or xs:anyAtomicType(meaning the static type could
         * not be inferred), or empty-sequence(). So we use the union of the two types.
         * The combinations could also be wrong. */
        returnType = t1 | t2;
    }

    return makeGenericSequenceType(returnType, card);
}

SequenceType::List ArithmeticExpression::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrOneAtomicType);
    result.append(CommonSequenceTypes::ZeroOrOneAtomicType);
    return result;
}

AtomicMathematician::Operator ArithmeticExpression::operatorID() const
{
    return m_op;
}

ExpressionVisitorResult::Ptr ArithmeticExpression::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
