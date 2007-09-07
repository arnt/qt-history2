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
#include "ExceptIterator.h"
#include "GenericSequenceType.h"
#include "IntersectIterator.h"
#include "ItemMappingIterator.h"
#include "ListIterator.h"
#include "UnionIterator.h"

#include "CombineNodes.h"

using namespace Patternist;

CombineNodes::CombineNodes(const Expression::Ptr &operand1,
                           const Operator op,
                           const Expression::Ptr &operand2) : PairContainer(operand1, operand2),
                                                              m_operator(op)
{
    Q_ASSERT(op == Union    ||
             op == Except   ||
             op == Intersect);
}

Item::Iterator::Ptr CombineNodes::evaluateSequence(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    const Item::Iterator::Ptr op1(m_operand1->evaluateSequence(context));
    const Item::Iterator::Ptr op2(m_operand2->evaluateSequence(context));

    switch(m_operator)
    {
        case Intersect:
            return Item::Iterator::Ptr(new IntersectIterator(op1, op2));
        case Except:
            return Item::Iterator::Ptr(new ExceptIterator(op1, op2));
        default:
        {
            Q_ASSERT(m_operator == Union);
            return Item::Iterator::Ptr(new UnionIterator(op1, op2));
        }
    }
}

bool CombineNodes::evaluateEBV(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO;
    /* If it's the union operator, we can possibly avoid
     * evaluating the second operand. */
    if(m_operator == Union)
    {
        return m_operand1->evaluateEBV(context) ||
               m_operand2->evaluateEBV(context);
    }
    else
        return PairContainer::evaluateEBV(context);
}

QString CombineNodes::displayName(const Operator op)
{
    switch(op)
    {
        case Intersect:
            return QLatin1String("intersect");
        case Except:
            return QLatin1String("except");
        default:
        {
            Q_ASSERT(op == Union);
            return QLatin1String("union");
        }
    }
}

SequenceType::Ptr CombineNodes::staticType() const
{
    return CommonSequenceTypes::ZeroOrMoreNodes;
}

SequenceType::List CombineNodes::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreNodes);
    result.append(CommonSequenceTypes::ZeroOrMoreNodes);
    return result;
}

CombineNodes::Operator CombineNodes::operatorID() const
{
    return m_operator;
}

ExpressionVisitorResult::Ptr CombineNodes::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
