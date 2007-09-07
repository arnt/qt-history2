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

#include "CommonNamespaces.h"

#include "Boolean.h"
#include "CommonValues.h"
#include "Integer.h"
#include "AtomicString.h"

#include "CompareStringFNs.h"

using namespace Patternist;

Item CodepointEqualFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item op1(m_operands.first()->evaluateSingleton(context));
    if(!op1)
        return Item();

    const Item op2(m_operands.last()->evaluateSingleton(context));
    if(!op2)
        return Item();

    if(caseSensitivity() == Qt::CaseSensitive)
        return Boolean::fromValue(op1.stringValue() == op2.stringValue());
    else
    {
        const QString s1(op1.stringValue());
        const QString s2(op2.stringValue());

        return Boolean::fromValue(s1.length() == s2.length() &&
                                  s1.startsWith(s2, Qt::CaseInsensitive));
    }
}

Item CompareFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item op1(m_operands.first()->evaluateSingleton(context));
    if(!op1)
        return Item();

    const Item op2(m_operands.at(1)->evaluateSingleton(context));
    if(!op2)
        return Item();

    const int retval = caseSensitivity() == Qt::CaseSensitive
                       ? op1.stringValue().compare(op2.stringValue())
                       : op1.stringValue().toLower().compare(op2.stringValue().toLower());

    if(retval > 0)
        return CommonValues::IntegerOne;
    else if(retval < 0)
        return CommonValues::IntegerOneNegative;
    else
    {
        Q_ASSERT(retval == 0);
        return CommonValues::IntegerZero;
    }
}

// vim: et:ts=4:sw=4:sts=4
