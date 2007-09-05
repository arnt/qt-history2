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
 * @file
 * @short This file is included by AtomicMathematicians.h
 * if you need some includes, put them in AbstractFloatMathematician.h, outside of the namespace.
 */

template <const bool isDouble>
Item AbstractFloatMathematician<isDouble>::calculate(const Item &o1,
                                                          const Operator op,
                                                          const Item &o2,
                                                          const PlainSharedPtr<DynamicContext> &context) const
{
    const Numeric *const num1 = o1.as<Numeric>();
    const Numeric *const num2 = o2.as<Numeric>();
    switch(op)
    {
        case Div:
            return toItem(AbstractFloat<isDouble>::fromValue(num1->toDouble() / num2->toDouble()));
        case IDiv:
        {
            if(num1->isNaN() || num2->isNaN())
            {
                context->error(tr("No operand in an integer division, %1, can be %2.")
                                  .arg(formatKeyword("idiv"))
                                  .arg(formatData("NaN")),
                               ReportContext::FOAR0002, this);
            }
            else if(num1->isInf())
            {
                context->error(tr("The first operand in an integer division, "
                                  "%1, cannot be infinity(%2).")
                                  .arg(formatKeyword("idiv"))
                                  .arg(formatData("INF")),
                               ReportContext::FOAR0002, this);
            }
            else if(num2->toInteger() == 0)
                context->error(tr("The first operand in an integer division, "
                                  "%1, cannot be zero(%2).")
                                  .arg(formatKeyword("idiv"))
                                  .arg(formatData("0")),
                               ReportContext::FOAR0001, this);

            return Integer::fromValue(static_cast<xsInteger>(num1->toDouble() / num2->toDouble()));
        }
        case Substract:
            return toItem(AbstractFloat<isDouble>::fromValue(num1->toDouble() - num2->toDouble()));
        case Mod:
            return toItem(AbstractFloat<isDouble>::fromValue(std::fmod(num1->toDouble(), num2->toDouble())));
        case Multiply:
            return toItem(AbstractFloat<isDouble>::fromValue(num1->toDouble() * num2->toDouble()));
        case Add:
            return toItem(AbstractFloat<isDouble>::fromValue(num1->toDouble() + num2->toDouble()));
    }

    Q_ASSERT(false);
    return Item(); /* GCC unbarfer. */
}

// vim: et:ts=4:sw=4:sts=4
