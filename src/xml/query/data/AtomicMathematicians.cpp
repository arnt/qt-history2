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

#include <cmath>

#include <qnumeric.h>

#include "AbstractDateTime.h"
#include "AbstractDuration.h"
#include "AbstractFloat.h"
#include "DayTimeDuration.h"
#include "Debug.h"
#include "Decimal.h"
#include "Integer.h"
#include "PatternistLocale.h"

#include "AtomicMathematicians.h"

using namespace Patternist;

/* The translation strings is place here once, in order to reduce work for translators,
 * and provide consistency. */

static inline QString idivZeroInvalid()
{
    return tr("The first operand in an integer division, %1, cannot be zero(%2).")
             .arg(formatKeyword("idiv"))
             .arg(formatData("0"));
}

static inline QString divZeroInvalid()
{
    return tr("The first operand in an division, %1, cannot be zero(%2).")
             .arg(formatKeyword("div"))
             .arg(formatData("0"));
}

static inline QString modZeroInvalid()
{
    return tr("The second operand in a modulus division, %1, cannot be zero(%2).")
             .arg(formatKeyword("mod"))
             .arg(formatData("0"));
}

Item DecimalMathematician::calculate(const Item &o1,
                                          const Operator op,
                                          const Item &o2,
                                          const PlainSharedPtr<DynamicContext> &context) const
{
    switch(op)
    {
        case Div:
        {
            if(o2.as<Numeric>()->toInteger() == 0)
            {
                context->error(divZeroInvalid(), ReportContext::FOAR0001, this);
                return Item(); /* Silences source code analyzer warning. */
            }
            else
                return toItem(Decimal::fromValue(o1.as<Numeric>()->toDecimal() / o2.as<Numeric>()->toDecimal()));
        }
        case IDiv:
        {
            if(o2.as<Numeric>()->toInteger() == 0)
            {
                context->error(idivZeroInvalid(), ReportContext::FOAR0001, this);
                return Item(); /* Silences source code analyzer warning. */
            }
            else
                return Integer::fromValue(static_cast<xsInteger>(o1.as<Numeric>()->toDecimal() /
                                                                 o2.as<Numeric>()->toDecimal()));
        }
        case Substract:
            return toItem(Decimal::fromValue(o1.as<Numeric>()->toDecimal() - o2.as<Numeric>()->toDecimal()));
        case Mod:
        {
            if(o2.as<Numeric>()->toInteger() == 0)
            {
                context->error(modZeroInvalid(), ReportContext::FOAR0001, this);
                return Item(); /* Silences source code analyzer warning. */
            }
            else
                return toItem(Decimal::fromValue(std::fmod(o1.as<Numeric>()->toDecimal(), o2.as<Numeric>()->toDecimal())));
        }
        case Multiply:
            return toItem(Decimal::fromValue(o1.as<Numeric>()->toDecimal() * o2.as<Numeric>()->toDecimal()));
        case Add:
            return toItem(Decimal::fromValue(o1.as<Numeric>()->toDecimal() + o2.as<Numeric>()->toDecimal()));
    }

    Q_ASSERT(false);
    return Item(); /* GCC unbarfer. */
}

Item IntegerMathematician::calculate(const Item &o1,
                                          const Operator op,
                                          const Item &o2,
                                          const PlainSharedPtr<DynamicContext> &context) const
{
    switch(op)
    {
        case Div:
            if(o2.as<Numeric>()->toInteger() == 0)
            {
                context->error(divZeroInvalid(), ReportContext::FOAR0001, this);
                return Item(); /* Silences source code analyzer warning. */
            }
            else /* C++ automatically performs truncation of long integer(xsInteger). */
                return toItem(Decimal::fromValue(o1.as<Numeric>()->toDecimal() / o2.as<Numeric>()->toDecimal()));
        case IDiv:
        {
            if(o2.as<Numeric>()->toInteger() == 0)
            {
                context->error(idivZeroInvalid(), ReportContext::FOAR0001, this);
                return Item(); /* Silences source code analyzer warning. */
            }
            else /* C++ automatically performs truncation of long integer(xsInteger). */
                return Integer::fromValue(o1.as<Numeric>()->toInteger() / o2.as<Numeric>()->toInteger());
        }
        case Substract:
            return Integer::fromValue(o1.as<Numeric>()->toInteger() - o2.as<Numeric>()->toInteger());
        case Mod:
        {
            const xsInteger divisor = o2.as<Numeric>()->toInteger();

            if(divisor == 0)
            {
                context->error(modZeroInvalid(), ReportContext::FOAR0001, this);
                return Item(); /* Silences source code analyzer warning. */
            }
            else
                return Integer::fromValue(o1.as<Numeric>()->toInteger() % divisor);
        }
        case Multiply:
            return Integer::fromValue(o1.as<Numeric>()->toInteger() * o2.as<Numeric>()->toInteger());
        case Add:
            return Integer::fromValue(o1.as<Numeric>()->toInteger() + o2.as<Numeric>()->toInteger());
    }

    Q_ASSERT(false);
    return Item(); /* GCC unbarfer. */
}

Item DurationNumericMathematician::calculate(const Item &o1,
                                             const Operator op,
                                             const Item &o2,
                                             const PlainSharedPtr<DynamicContext> &context) const
{
    Q_ASSERT(op == Div || op == Multiply);

    const AbstractDuration::Ptr duration(o1.as<AbstractDuration>());
    const xsDouble dbl = o2.as<Numeric>()->toDouble();

    switch(op)
    {
        case Div:
        {
            if(qIsInf(dbl))
                return duration->fromValue(0);
            else if(qIsNaN(dbl))
            {
                context->error(tr("In a division involving %1, the right operand cannot "
                                                "be %2(not-a-number)")
                                                .arg(formatType(context->namePool(), duration->type()))
                                                .arg(formatData("NaN")),
                                           ReportContext::FOCA0005,
                                           this);
                return Item();
            }
            else if(Double::isEqual(dbl, 0))
            {
                context->error(tr("In a division involving %1, the right "
                                                "operand cannot be %2 or %3(negative "
                                                "or positive zero)")
                                                .arg(duration->type())
                                                .arg(formatData("-0"))
                                                .arg(formatData("0")),
                                           ReportContext::FODT0002,
                                           this);
                return Item();
            }

            return duration->fromValue(static_cast<AbstractDuration::Value>(duration->value() / dbl));
        }
        case Multiply:
        {
            if(Double::isEqual(dbl, 0))
                return duration->fromValue(0);
            else if(qIsNaN(dbl))
            {
                context->error(tr("In a division involving %1, the right operand cannot "
                                                "be %2(not-a-number)")
                                                .arg(duration->type())
                                                .arg(formatData("NaN")),
                                           ReportContext::FOCA0005,
                                           this);
                return Item();
            }
            else if(qIsInf(dbl))
            {
                context->error(tr("In a multiplication involving %1, the right "
                                                "operand cannot be %2 or %3(negative "
                                                "or positive infinity)")
                                                .arg(duration->type())
                                                .arg(formatData("-INF"))
                                                .arg(formatData("INF")),
                                           ReportContext::FODT0002,
                                           this);
                return Item();
            }

            return duration->fromValue(static_cast<AbstractDuration::Value>(duration->value() * dbl));
        }
        default:
        {
            Q_ASSERT(false);
            return Item(); /* Silence warning. */
        }
    }
}

Item DurationDurationMathematician::calculate(const Item &o1,
                                                   const Operator op,
                                                   const Item &o2,
                                                   const PlainSharedPtr<DynamicContext> &) const
{
    const AbstractDuration::Ptr duration(o1.as<AbstractDuration>());
    const AbstractDuration::Value op2 = o2.as<AbstractDuration>()->value();
    qDebug() << " OP2: " << op2;

    switch(op)
    {
        case Div:
            return toItem(Decimal::fromValue(static_cast<xsDecimal>(duration->value()) / op2));
        case Substract:
            return duration->fromValue(duration->value() - op2);
        case Add:
            return duration->fromValue(duration->value() + op2);
        default:
        {
            Q_ASSERT(false);
            return Item(); /* Silence warning. */
        }
    }
}

OperandSwitcherMathematician::
OperandSwitcherMathematician(const AtomicMathematician::Ptr &mathematician) : m_mather(mathematician)
{
    Q_ASSERT(mathematician);
}

Item OperandSwitcherMathematician::calculate(const Item &o1,
                                                  const Operator op,
                                                  const Item &o2,
                                                  const PlainSharedPtr<DynamicContext> &context) const
{
    return m_mather->calculate(o2, op, o1, context);
}


Item DateTimeDurationMathematician::calculate(const Item &o1,
                                                   const Operator op,
                                                   const Item &o2,
                                                   const PlainSharedPtr<DynamicContext> &context) const
{
    Q_ASSERT(op == Substract || op == Add);

    const AbstractDateTime::Ptr adt(o1.as<AbstractDateTime>());
    const AbstractDuration::Ptr dur(o2.as<AbstractDuration>());
    QDateTime dt(adt->toDateTime());
    //qDebug() << "DateTimeDurationMathematician::calculate():" << dt.toString();
    //dt.setDateOnly(false);
    const qint8 sign = (op == Add ? 1 : -1) * (dur->isPositive() ? 1 : -1);

    // TODO milli seconds
    dt = dt.addSecs(sign * (dur->seconds() + dur->minutes() * 60 + dur->hours() * 60 * 60));
    dt = dt.addDays(sign * dur->days());
    dt = dt.addMonths(sign * dur->months());
    dt = dt.addYears(sign * dur->years());

    QString msg;

    if(AbstractDateTime::isRangeValid(dt.date(), msg))
        return adt->fromValue(dt);
    else
    {
        context->error(msg, ReportContext::FODT0001,
                       this);
        return Item();
    }
}

Item AbstractDateTimeMathematician::calculate(const Item &o1,
                                                   const Operator op,
                                                   const Item &o2,
                                                   const PlainSharedPtr<DynamicContext> &) const
{
    Q_ASSERT(op == Substract || op == Add);
    QDateTime dt1(o1.as<AbstractDateTime>()->toDateTime());
    QDateTime dt2(o2.as<AbstractDateTime>()->toDateTime());
    /*
    qDebug() << "AbstractDateTimeMathematician::calculate():" << dt1.toString() << dt2.toString();
    qDebug() << "Date-only:" << dt1.isDateOnly() << dt2.isDateOnly();
    */
    //dt1.setDateOnly(false);
    //dt2.setDateOnly(false);

    int diff = op == Add ? dt1.secsTo(dt2) : dt2.secsTo(dt1);

    return toItem(DayTimeDuration::fromSeconds(diff));
}

// vim: et:ts=4:sw=4:sts=4
