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

#include "qunlimitedcontainer_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

UnlimitedContainer::UnlimitedContainer(const Expression::List &ops) : m_operands(ops)
{
}

void UnlimitedContainer::setOperands(const Expression::List &list)
{
    m_operands = list;
}

Expression::List UnlimitedContainer::operands() const
{
    return m_operands;
}

bool UnlimitedContainer::compressOperands(const StaticContext::Ptr &context)
{
    const Expression::List::iterator end(m_operands.end());
    Expression::List::iterator it(m_operands.begin());
    int evaled = 0;

    for(; it != end; ++it)
    {
        Q_ASSERT((*it));
        rewrite((*it), (*it)->compress(context), context);
        if((*it)->isEvaluated())
                ++evaled;
    }

    return evaled == m_operands.count();
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
