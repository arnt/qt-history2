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

#include "qatomictype_p.h"
#include "qitem_p.h"
#include "qboolean_p.h"
#include "qbuiltintypes_p.h"
#include "qcommonsequencetypes_p.h"
#include "qcommonvalues_p.h"
#include "qdebug_p.h"
#include "qlistiterator_p.h"
#include "qliteral_p.h"

#include "qcastableas_p.h"

using namespace Patternist;

CastableAs::CastableAs(const Expression::Ptr &operand,
                       const SequenceType::Ptr &tType) : SingleContainer(operand),
                                                         m_targetType(tType)
{
    Q_ASSERT(tType);
    Q_ASSERT(!tType->cardinality().allowsMany());
    Q_ASSERT(tType->itemType()->isAtomicType());
}

bool CastableAs::evaluateEBV(const DynamicContext::Ptr &context) const
{
    Item item;

    if(m_operand->staticType()->cardinality().allowsMany())
    {
        const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
        item = it->next();

        if(it->next())
            return false;
    }
    else
        item = m_operand->evaluateSingleton(context);

    if(item)
        return !cast(item, context).as<AtomicValue>()->hasError();
    else
        return m_targetType->cardinality().allowsEmpty();
}

Expression::Ptr CastableAs::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(SingleContainer::compress(context));

    if(me.get() != this) /* We already managed to const fold, how convenient. */
        return me;

    const AtomicType::Ptr t(m_targetType->itemType());

    const SequenceType::Ptr opType(m_operand->staticType());

    /* Casting to these always succeeds. */
    if(*t == *BuiltinTypes::xsString ||
       *t == *BuiltinTypes::xsUntypedAtomic ||
       (*t == *opType->itemType() &&
       (m_targetType->cardinality().isMatch(opType->cardinality()))))
    {
        qDebug() << "Optimizing castable as..";
        return wrapLiteral(CommonValues::BooleanTrue, context, this);
    }
    else
        return me;
}

Expression::Ptr CastableAs::typeCheck(const StaticContext::Ptr &context,
                                      const SequenceType::Ptr &reqType)
{
    qDebug();
    checkTargetType(context);
    const Expression::Ptr me(SingleContainer::typeCheck(context, reqType));

    return me;
    if(BuiltinTypes::xsQName->xdtTypeMatches(m_targetType->itemType()))
    {
        const SequenceType::Ptr seqt(m_operand->staticType());
        /* We can cast a string literal, an xs:QName value, and an
         * empty sequence(if empty is allowed), to xs:QName. */
        if(m_operand->is(IDStringValue) ||
           BuiltinTypes::xsQName->xdtTypeMatches(seqt->itemType()) ||
           *seqt->itemType() == *CommonSequenceTypes::Empty && m_targetType->cardinality().allowsEmpty())
        {
            return wrapLiteral(CommonValues::BooleanTrue, context, this)->typeCheck(context, reqType);
        }
        else
            return wrapLiteral(CommonValues::BooleanFalse, context, this)->typeCheck(context, reqType);
    }
    else
    {
        /* Let the CastingPlatform look up its AtomicCaster. */
        prepareCasting(context, m_operand->staticType()->itemType());

        return me;
    }
}

SequenceType::List CastableAs::expectedOperandTypes() const
{
    qDebug() << Q_FUNC_INFO;
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreAtomicTypes);
    return result;
}

SequenceType::Ptr CastableAs::staticType() const
{
    return CommonSequenceTypes::ExactlyOneBoolean;
}

ExpressionVisitorResult::Ptr CastableAs::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
