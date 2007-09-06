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
#include "EmptySequence.h"
#include "Integer.h"
#include "Numeric.h"
#include "RangeIterator.h"

#include "NodeComparison.h"

using namespace Patternist;

NodeComparison::NodeComparison(const Expression::Ptr &operand1,
                               const Node::DocumentOrder op,
                               const Expression::Ptr &operand2)
                               : PairContainer(operand1, operand2)
                               , m_op(op)
{
    Q_ASSERT(op == Node::Precedes   ||
             op == Node::Follows    ||
             op == Node::Is);
}

bool NodeComparison::evaluateEBV(const DynamicContext::Ptr &context) const
{
    const Item op1(m_operand1->evaluateSingleton(context));
    if(!op1)
        return Item();

    const Item op2(m_operand2->evaluateSingleton(context));
    if(!op2)
        return Item();

    switch(m_op)
    {
        case Node::Is:
            return op1.asNode().is(op2.asNode());
        case Node::Precedes:
            return op1.asNode().compareOrderTo(op2.asNode()) == Node::Precedes;
        case Node::Follows:
            return op1.asNode().compareOrderTo(op2.asNode()) == Node::Follows;
    }

    Q_ASSERT_X(false, Q_FUNC_INFO, "This line should never be reached.");
    return false;
}

SequenceType::List NodeComparison::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrOneNode);
    result.append(CommonSequenceTypes::ZeroOrOneNode);
    return result;
}

Expression::Ptr NodeComparison::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr me(PairContainer::compress(context));

    if(me.get() != this)
    /* We're already rewritten. */
        return me;

    if(m_operand1->staticType()->cardinality().isEmpty() ||
       m_operand2->staticType()->cardinality().isEmpty())
    {
        // TODO issue a warning in the @p context saying that one of the operands
        // were empty, and that the expression always result in the empty sequence
        // (which never is the intent, right?).
        return EmptySequence::create(this, context);
    }

    return Expression::Ptr(this);
}

QString NodeComparison::displayName(const Node::DocumentOrder op)
{
    switch(op)
    {
        case Node::Is:
            return QLatin1String("is");
        case Node::Precedes:
            return QLatin1String("<<");
        case Node::Follows:
            return QLatin1String(">>");
    }

    Q_ASSERT(false);
    return QString(); /* Silence GCC. */
}

SequenceType::Ptr NodeComparison::staticType() const
{
    if(m_operand1->staticType()->cardinality().allowsEmpty() ||
       m_operand2->staticType()->cardinality().allowsEmpty())
        return CommonSequenceTypes::ZeroOrOneBoolean;
    else
        return CommonSequenceTypes::ExactlyOneBoolean;
}

Node::DocumentOrder NodeComparison::operatorID() const
{
    return m_op;
}

ExpressionVisitorResult::Ptr NodeComparison::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
