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

#include "CommonSequenceTypes.h"
#include "Debug.h"
#include "GenericSequenceType.h"
#include "ItemMappingIterator.h"
#include "ListIterator.h"

#include "QuantifiedExpression.h"

using namespace Patternist;

QuantifiedExpression::QuantifiedExpression(const VariableSlotID varSlot,
                                           const Operator quantifier,
                                           const Expression::Ptr &inClause,
                                           const Expression::Ptr &testExpression)
                                           : PairContainer(inClause, testExpression),
                                             m_varSlot(varSlot),
                                             m_quantifier(quantifier)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(quantifier == Some || quantifier == Every);
}

Item QuantifiedExpression::mapToItem(const Item &item,
                                          const DynamicContext::Ptr &context) const
{
    context->setRangeVariable(m_varSlot, item);
    return item;
}

bool QuantifiedExpression::evaluateEBV(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    const Item::Iterator::Ptr
    it(makeItemMappingIterator<Item>(QuantifiedExpression::Ptr(const_cast<QuantifiedExpression *>(this)),
                                          m_operand1->evaluateSequence(context),
                                          context));

    Item item(it->next());

    if(m_quantifier == Some)
    {
        while(item)
        {
            if(m_operand2->evaluateEBV(context))
                return true;
            else
                item = it->next();
        };

        return false;
    }
    else
    {
        Q_ASSERT(m_quantifier == Every);

        while(item)
        {
            if(m_operand2->evaluateEBV(context))
                item = it->next();
            else
                return false;
        }

        return true;
    }
}

QString QuantifiedExpression::displayName(const Operator quantifier)
{
    if(quantifier == Some)
        return QLatin1String("some");
    else
    {
        Q_ASSERT(quantifier == Every);
        return QLatin1String("every");
    }
}

SequenceType::Ptr QuantifiedExpression::staticType() const
{
    return CommonSequenceTypes::ExactlyOneBoolean;
}

SequenceType::List QuantifiedExpression::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    result.append(CommonSequenceTypes::EBV);
    return result;
}

QuantifiedExpression::Operator QuantifiedExpression::operatorID() const
{
    return m_quantifier;
}

ExpressionVisitorResult::Ptr QuantifiedExpression::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
