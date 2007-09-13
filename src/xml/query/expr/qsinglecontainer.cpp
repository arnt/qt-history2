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

#include "qdebug_p.h"

#include "qsinglecontainer_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

SingleContainer::SingleContainer(const Expression::Ptr &operand) : m_operand(operand)
{
    Q_ASSERT(operand);
}

Expression::List SingleContainer::operands() const
{
    Expression::List list;
    list.append(m_operand);
    return list;
}

void SingleContainer::setOperands(const Expression::List &ops)
{
    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(ops.count() == 1);
    m_operand = ops.first();
}

bool SingleContainer::compressOperands(const StaticContext::Ptr &context)
{
    rewrite(m_operand, m_operand->compress(context), context);

    return m_operand->isEvaluated();
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
