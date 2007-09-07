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

#include "qboolean_p.h"
#include "qcommonvalues_p.h"
#include "qliteral_p.h"

#include "qorexpression_p.h"

using namespace Patternist;

OrExpression::OrExpression(const Expression::Ptr &operand1,
                           const Expression::Ptr &operand2) : AndExpression(operand1, operand2)
{
}

bool OrExpression::evaluateEBV(const DynamicContext::Ptr &context) const
{
    return m_operand1->evaluateEBV(context) || m_operand2->evaluateEBV(context);
}

Expression::Ptr OrExpression::compress(const StaticContext::Ptr &context)
{
    const Expression::Ptr newMe(PairContainer::compress(context));

    if(newMe.get() != this)
        return newMe;

    /* Both operands mustn't be evaluated in order to be able to compress. */
    if(m_operand1->isEvaluated() && m_operand1->evaluateEBV(context->dynamicContext()))
        return wrapLiteral(CommonValues::BooleanTrue, context, this);
    else if(m_operand2->isEvaluated() && m_operand2->evaluateEBV(context->dynamicContext()))
        return wrapLiteral(CommonValues::BooleanTrue, context, this);
    else
        return Expression::Ptr(this);
}

ExpressionVisitorResult::Ptr OrExpression::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
