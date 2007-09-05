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

/*
 * NOTE: This file is included by AbstractFloatCasters.h
 * if you need some includes, put them in AbstractFloatCasters.h (outside of the namespace)
 */

template <const bool isDouble>
Item NumericToAbstractFloatCaster<isDouble>::castFrom(const Item &from,
                                                           const PlainSharedPtr<DynamicContext> &) const
{
    // toDouble() returns same thing than toFloat()
    return toItem(AbstractFloat<isDouble>::fromValue(from.as<Numeric>()->toDouble()));
}

template <const bool isDouble>
Item StringToAbstractFloatCaster<isDouble>::castFrom(const Item &from,
                                                          const PlainSharedPtr<DynamicContext> &) const
{
    return toItem(AbstractFloat<isDouble>::fromLexical(from.stringValue()));
}

template <const bool isDouble>
Item BooleanToAbstractFloatCaster<isDouble>::castFrom(const Item &from,
                                                           const PlainSharedPtr<DynamicContext> &context) const
{
    if(from.as<AtomicValue>()->evaluateEBV(context))
        return isDouble ? toItem(CommonValues::DoubleOne) : toItem(CommonValues::FloatOne);
    else
        return isDouble ? toItem(CommonValues::DoubleZero) : toItem(CommonValues::FloatZero);
}

// vim: et:ts=4:sw=4:sts=4
