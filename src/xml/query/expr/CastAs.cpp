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

#include "Item.h"
#include "BuiltinTypes.h"
#include "CardinalityVerifier.h"
#include "CommonSequenceTypes.h"
#include "EmptySequence.h"
#include "GenericSequenceType.h"
#include "Literal.h"
#include "PatternistLocale.h"
#include "NamespaceResolver.h"
#include "QNameConstructor.h"
#include "QNameValue.h"
#include "AtomicString.h"
#include "ValidationError.h"

#include "CastAs.h"

using namespace Patternist;

CastAs::CastAs(const Expression::Ptr &source,
               const SequenceType::Ptr &tType) : SingleContainer(source),
                                                 m_targetType(tType)
{
    Q_ASSERT(source);
    Q_ASSERT(tType);
    Q_ASSERT(!tType->cardinality().allowsMany());
    Q_ASSERT(tType->itemType()->isAtomicType());
}

Item CastAs::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    Q_ASSERT(context);
    const Item val(m_operand->evaluateSingleton(context));

    if(val)
        return cast(val, context);
    else
    {
        /* No item supplied, let's handle the cardinality part. */

        if(m_targetType->cardinality().allowsEmpty())
            return Item();
        else
        {
            Q_ASSERT(context);
            context->error(tr("Type error in cast, expected %1, but recieved %2.")
                                         .arg(formatType(Cardinality::exactlyOne()))
                                         .arg(formatType(Cardinality::empty())),
                                    ReportContext::XPTY0004, this);
            return Item();
        }
    }
}

Expression::Ptr CastAs::typeCheck(const StaticContext::Ptr &context,
                                  const SequenceType::Ptr &reqType)
{
    checkTargetType(context);
    const SequenceType::Ptr seqt(m_operand->staticType());
    ItemType::Ptr t(seqt->itemType());

    /* Special case xs:QName */
    if(BuiltinTypes::xsQName->xdtTypeMatches(m_targetType->itemType()))
    {
        /* Ok, We're casting to xs:QName. */
        if(m_operand->is(IDStringValue)) /* A valid combination, let's do the cast. */
            return castToQName(context)->typeCheck(context, reqType);
        else if(BuiltinTypes::xsQName->xdtTypeMatches(t))
            return m_operand->typeCheck(context, reqType);
        else if(seqt->cardinality().isEmpty() && m_targetType->cardinality().allowsEmpty())
            return EmptySequence::create(this, context);
        else if(!(seqt->cardinality().isEmpty() && !m_targetType->cardinality().allowsEmpty()))
        {
            context->error(tr("When casting to %1 or types derived from it, "
                                         "the source value must be a string literal or a value of "
                                         "the same type. Therefore, the type %2 doesn't work.")
                                         .arg(formatType(context->namePool(), BuiltinTypes::xsQName))
                                         .arg(formatType(context->namePool(), seqt)),
                                    ReportContext::XPTY0004, this);
            return Expression::Ptr(this);
        }
    }

    const Expression::Ptr me(SingleContainer::typeCheck(context, reqType));
    /* Type may have changed, such as that atomization has been applied. */
    t = m_operand->staticType()->itemType();

    if(m_targetType->itemType()->xdtTypeMatches(t) &&
       !BuiltinTypes::xsDayTimeDuration->xdtTypeMatches(t) &&
       !BuiltinTypes::xsYearMonthDuration->xdtTypeMatches(t))
    { /* At least casting is superflorous. */
        if(m_operand->staticType()->cardinality().isMatch(m_targetType->cardinality()))
            return m_operand; /* The whole cast expression is redundant. */
        else
        { /* Only cardinality check is needed, rewrite to CardinalityVerifier. */
            return Expression::Ptr(new CardinalityVerifier(m_operand,
                                                           m_targetType->cardinality(),
                                                           ReportContext::FORG0001));
        }
    }

    /* Let the CastingPlatform look up its AtomicCaster. */
    prepareCasting(context, t);

    return me;
}

Expression::Ptr CastAs::compress(const StaticContext::Ptr &context)
{
    /* Simplify casts to itself. */
    if(*m_targetType->itemType() == *m_operand->staticType()->itemType())
        return m_operand->compress(context);
    else
        return SingleContainer::compress(context);
}

Expression::Ptr CastAs::castToQName(const StaticContext::Ptr &context) const
{
    /* Apply the whitespace facet by calling trimmed(). */
    /* We can assume m_operand is an Expression because this is a requirement
     * for casting to xs:QName. */
    const QString lexQName(m_operand->as<Literal>()->item().as<AtomicValue>()->stringValue().trimmed());

    const QName
        expName(QNameConstructor::expandQName<StaticContext::Ptr,
                                              ReportContext::FORG0001,
                                              ReportContext::FONS0004>(lexQName,
                                                                       context,
                                                                       context->namespaceBindings(), this));
    return wrapLiteral(toItem(QNameValue::fromValue(context->namePool(), expName)), context, this);
}

SequenceType::Ptr CastAs::staticType() const
{
    if(m_operand->staticType()->cardinality().allowsEmpty())
        return m_targetType;
    else
        return makeGenericSequenceType(m_targetType->itemType(),
                                       Cardinality::exactlyOne());
}

SequenceType::List CastAs::expectedOperandTypes() const
{
    SequenceType::List result;

    if(m_targetType->cardinality().allowsEmpty())
        result.append(CommonSequenceTypes::ZeroOrOneAtomicType);
    else
        result.append(CommonSequenceTypes::ExactlyOneAtomicType);

    return result;
}

ExpressionVisitorResult::Ptr CastAs::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
