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

#include "Boolean.h"
#include "Debug.h"
#include "GenericSequenceType.h"

#include "RangeVariableReference.h"

using namespace Patternist;

RangeVariableReference::RangeVariableReference(const Expression::Ptr &source,
                                               const VariableSlotID slotP) : VariableReference(slotP),
                                                                             m_sourceExpression(source)
{
    qDebug() << Q_FUNC_INFO << "slot: " << slotP;
    Q_ASSERT(source);
}

bool RangeVariableReference::evaluateEBV(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot();
    Q_ASSERT_X(context->rangeVariable(slot()), Q_FUNC_INFO, "The range variable must be set.");
    return Boolean::evaluateEBV(context->rangeVariable(slot()), context);
}

Item RangeVariableReference::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot();
    Q_ASSERT_X(context->rangeVariable(slot()), Q_FUNC_INFO, "The range variable must be set.");
    return context->rangeVariable(slot());
}

SequenceType::Ptr RangeVariableReference::staticType() const
{
    return makeGenericSequenceType(m_sourceExpression->staticType()->itemType(),
                                   Cardinality::exactlyOne());
}

Expression::ID RangeVariableReference::id() const
{
    return IDRangeVariableReference;
}

ExpressionVisitorResult::Ptr
RangeVariableReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

Expression::Properties RangeVariableReference::properties() const
{
    return DependsOnLocalVariable;
}

// vim: et:ts=4:sw=4:sts=4
