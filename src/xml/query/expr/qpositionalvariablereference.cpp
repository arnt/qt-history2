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
#include "Integer.h"
#include "ListIterator.h"

#include "PositionalVariableReference.h"

using namespace Patternist;

PositionalVariableReference::PositionalVariableReference(const VariableSlotID s) : VariableReference(s)
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot();
}

Item PositionalVariableReference::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot();
    Q_ASSERT(context);
    Q_ASSERT(context->positionIterator(slot()));
    return Integer::fromValue(context->positionIterator(slot())->position());
}

bool PositionalVariableReference::evaluateEBV(const DynamicContext::Ptr &) const
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot();
    return true;
}

SequenceType::Ptr PositionalVariableReference::staticType() const
{
    return CommonSequenceTypes::ExactlyOneInteger;
}

Expression::Properties PositionalVariableReference::properties() const
{
    return DependsOnLocalVariable;
}

ExpressionVisitorResult::Ptr

PositionalVariableReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
