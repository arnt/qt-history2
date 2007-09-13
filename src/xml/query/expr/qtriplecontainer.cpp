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

#include <QList>

#include "qtriplecontainer_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

TripleContainer::TripleContainer(const Expression::Ptr &operand1,
                                 const Expression::Ptr &operand2,
                                 const Expression::Ptr &operand3) : m_operand1(operand1),
                                                                    m_operand2(operand2),
                                                                    m_operand3(operand3)
{
    Q_ASSERT(operand1);
    Q_ASSERT(operand2);
    Q_ASSERT(operand3);
}

Expression::List TripleContainer::operands() const
{
    Expression::List result;
    result.append(m_operand1);
    result.append(m_operand2);
    result.append(m_operand3);
    return result;
}

void TripleContainer::setOperands(const Expression::List &ops)
{
    Q_ASSERT(ops.count() == 3);
    m_operand1 = ops.first();
    m_operand2 = ops.at(1);
    m_operand3 = ops.at(2);
}

bool TripleContainer::compressOperands(const StaticContext::Ptr &context)
{
    rewrite(m_operand1, m_operand1->compress(context), context);
    rewrite(m_operand2, m_operand2->compress(context), context);
    rewrite(m_operand3, m_operand3->compress(context), context);

    return m_operand1->isEvaluated() && m_operand2->isEvaluated() && m_operand3->isEvaluated();
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
