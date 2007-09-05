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

#include "Boolean.h"
#include "CommonSequenceTypes.h"
#include "CommonValues.h"
#include "Debug.h"
#include "ListIterator.h"

#include "DynamicContextStore.h"

using namespace Patternist;

DynamicContextStore::DynamicContextStore(const Expression::Ptr &operand,
                                         const DynamicContext::Ptr &context) : SingleContainer(operand),
                                                                               m_context(context)
{
    Q_ASSERT(context);
}

bool DynamicContextStore::evaluateEBV(const DynamicContext::Ptr &) const
{
    return m_operand->evaluateEBV(m_context);
}

Item::Iterator::Ptr DynamicContextStore::evaluateSequence(const DynamicContext::Ptr &) const
{
    return m_operand->evaluateSequence(m_context);
}

Item DynamicContextStore::evaluateSingleton(const DynamicContext::Ptr &) const
{
    return m_operand->evaluateSingleton(m_context);
}

SequenceType::Ptr DynamicContextStore::staticType() const
{
    return m_operand->staticType();
}

SequenceType::List DynamicContextStore::expectedOperandTypes() const
{
    SequenceType::List result;
    result.append(CommonSequenceTypes::ZeroOrMoreItems);
    return result;
}

ExpressionVisitorResult::Ptr DynamicContextStore::accept(const ExpressionVisitor::Ptr &visitor) const
{
    return visitor->visit(this);
}

const SourceLocationReflection *DynamicContextStore::actualReflection() const
{
    return m_operand->actualReflection();
}

// vim: et:ts=4:sw=4:sts=4
