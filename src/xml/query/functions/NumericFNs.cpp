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

#include "CommonValues.h"
#include "GenericSequenceType.h"
#include "Numeric.h"

#include "NumericFNs.h"

using namespace Patternist;

Item FloorFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item num(m_operands.first()->evaluateSingleton(context));

    if(!num)
        return Item();

    return toItem(num.as<Numeric>()->floor());
}

Item AbsFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item num(m_operands.first()->evaluateSingleton(context));

    if(!num)
        return Item();

    return toItem(num.as<Numeric>()->abs());
}

Item RoundFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item num(m_operands.first()->evaluateSingleton(context));

    if(!num)
        return Item();

    return toItem(num.as<Numeric>()->round());
}

Item CeilingFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item num(m_operands.first()->evaluateSingleton(context));

    if(!num)
        return Item();

    return toItem(num.as<Numeric>()->ceiling());
}

Item RoundHalfToEvenFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item num(m_operands.first()->evaluateSingleton(context));

    if(!num)
        return Item();

    xsInteger scale = 0;

    if(m_operands.count() == 2)
        scale = m_operands.at(1)->evaluateSingleton(context).as<Numeric>()->toInteger();

    return toItem(num.as<Numeric>()->roundHalfToEven(scale));
}

// vim: et:ts=4:sw=4:sts=4
