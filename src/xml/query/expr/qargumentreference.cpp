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

#include "qdebug_p.h"
#include "qlistiterator_p.h"

#include "qargumentreference_p.h"

using namespace Patternist;

ArgumentReference::ArgumentReference(const SequenceType::Ptr &sourceType,
                                     const VariableSlotID slotP) : VariableReference(slotP),
                                                                   m_type(sourceType)
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot();
    Q_ASSERT(m_type);
}

bool ArgumentReference::evaluateEBV(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot();
    return context->expressionVariable(slot())->evaluateEBV(context);
}

Item ArgumentReference::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot();
    return context->expressionVariable(slot())->evaluateSingleton(context);
}

Item::Iterator::Ptr ArgumentReference::evaluateSequence(const DynamicContext::Ptr &context) const
{
    qDebug() << Q_FUNC_INFO << "slot: " << slot();
    return context->expressionVariable(slot())->evaluateSequence(context);
}

SequenceType::Ptr ArgumentReference::staticType() const
{
    return m_type;
}

ExpressionVisitorResult::Ptr ArgumentReference::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

// vim: et:ts=4:sw=4:sts=4
