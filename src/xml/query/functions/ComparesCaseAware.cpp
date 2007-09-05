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

#include "ValueComparison.h"

#include "ComparesCaseAware.h"

using namespace Patternist;

ComparesCaseAware::ComparesCaseAware() : m_caseSensitivity(Qt::CaseSensitive)
{
}

Expression::Ptr ComparesCaseAware::compress(const StaticContext::Ptr &context)
{
    Q_ASSERT(m_operands.size() >= 2);

    if(ValueComparison::isCaseInsensitiveCompare(m_operands.first(), m_operands[1]))
        m_caseSensitivity = Qt::CaseInsensitive;
    else
    {
        /* Yes, we could probably skip this since m_caseSensitivity is initialized to this value,
         * but perhaps subsequent calls to compress() can make isCaseInsensitiveCompare() return
         * a different value. */
        m_caseSensitivity = Qt::CaseSensitive;
    }

    return FunctionCall::compress(context);
}

// vim: et:ts=4:sw=4:sts=4
