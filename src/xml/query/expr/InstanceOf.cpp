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

#include "Boolean.h"
#include "CommonSequenceTypes.h"
#include "CommonValues.h"
#include "Debug.h"
#include "ListIterator.h"
#include "Literal.h"

#include "InstanceOf.h"

using namespace Patternist;

InstanceOf::InstanceOf(const Expression::Ptr &operand,
                       const SequenceType::Ptr &tType) : SingleContainer(operand),
                                                         m_targetType(tType)
{
    Q_ASSERT(operand);
    Q_ASSERT(m_targetType);
}

bool InstanceOf::evaluateEBV(const DynamicContext::Ptr &context) const
{
    const Item::Iterator::Ptr it(m_operand->evaluateSequence(context));
    Item item(it->next());
    unsigned int count = 1;

    if(!item)
        return m_targetType->cardinality().allowsEmpty();

    do
    {
        if(!m_targetType->itemType()->itemMatches(item))
            return false;

        if(count == 2 && !m_targetType->cardinality().allowsMany())
            return false;

        item = it->next();
        ++count;
    } while(item);

    return true;
}

Expression::Ptr InstanceOf::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(SingleContainer::compress(context));

    if(me.get() != this || m_operand->has(DisableTypingDeduction))
        return me;

    const SequenceType::Ptr opType(m_operand->staticType());
    const ItemType::Ptr itType(m_targetType->itemType());
    const ItemType::Ptr ioType(opType->itemType());

    if(m_targetType->cardinality().isMatch(opType->cardinality()))
    {
        if(itType->xdtTypeMatches(ioType))
            return wrapLiteral(CommonValues::BooleanTrue, context, this);
        else if(!ioType->xdtTypeMatches(itType))
        {
            qDebug() << Q_FUNC_INFO << "Const folding 1";
            return wrapLiteral(CommonValues::BooleanFalse, context, this);
        }
    }
    /* The cardinality is not guaranteed to match; it will need testing. */
    else if(!ioType->xdtTypeMatches(itType))
    {
        qDebug() << Q_FUNC_INFO << "Const folding 2";
        /* There's no way it's gonna match. The cardinality is not only
         * wrong, but the item type as well. */
        return wrapLiteral(CommonValues::BooleanFalse, context, this);
    }

    qDebug() << Q_FUNC_INFO << "DIDN'T const-fold";
    return me;
}

SequenceType::Ptr InstanceOf::targetType() const
{
    return m_targetType;
}

SequenceType::Ptr InstanceOf::staticType() const
{
    return CommonSequenceTypes::ExactlyOneBoolean;
}

SequenceType::List InstanceOf::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

ExpressionVisitorResult::Ptr InstanceOf::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
