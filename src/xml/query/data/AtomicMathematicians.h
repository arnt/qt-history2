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
***************************************************************************
*/

#ifndef Patternist_AtomicMathematicians_H
#define Patternist_AtomicMathematicians_H

#include "AtomicMathematician.h"
#include "SourceLocationReflection.h"

/**
 * @file
 * @short Contains classes performing arithemetic operations between atomic values, such as
 * substracting two dates.
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DecimalMathematician : public AtomicMathematician
                               , public DelegatingSourceLocationReflection
    {
    public:
        inline DecimalMathematician(const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r)
        {
        }

        virtual Item calculate(const Item &o1,
                                    const Operator op,
                                    const Item &o2,
                                    const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Performs arithmetics between Integer values.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class IntegerMathematician : public AtomicMathematician
                               , public DelegatingSourceLocationReflection
    {
    public:
        inline IntegerMathematician(const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r)
        {
        }

        virtual Item calculate(const Item &o1,
                                    const Operator op,
                                    const Item &o2,
                                    const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Performs division or multiplication between either DayTimeDuration or YearMonthDuration
     * and Double values.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DurationNumericMathematician : public AtomicMathematician
                                       , public DelegatingSourceLocationReflection
    {
    public:
        inline DurationNumericMathematician(const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r)
        {
        }

        virtual Item calculate(const Item &o1,
                                    const Operator op,
                                    const Item &o2,
                                    const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Performs division between DayTimeDuration and DayTimeDuration, or
     * YearMonthDuration and YearMonthDuration.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DurationDurationDivisor : public AtomicMathematician
    {
    public:
        virtual Item calculate(const Item &o1,
                                    const Operator op,
                                    const Item &o2,
                                    const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Performs arithmetics between DayTimeDuration and DayTimeDuration, or
     * YearMonthDuration and YearMonthDuration.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DurationDurationMathematician : public AtomicMathematician
    {
    public:
        virtual Item calculate(const Item &o1,
                                    const Operator op,
                                    const Item &o2,
                                    const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Delegates an AtomicMathematician and switches its operands.
     *
     * Switches the operands of the call to a call to the calculate()
     * on an AtomicMathematician such that the left operand becomes the right, and
     * vice versa.
     *
     * Its constructor takes an AtomicMathematician instance which this OperandSwitcherMathematician
     * should act as as a middle-man for, having the role of switching the two operands. Thus,
     * OperandSwitcherMathematician can be described as a proxy or delegator class.
     *
     * This class is used for implementing operator combinations such as
     * <tt>numeric * xs:yearMonthDuration</tt> and
     * <tt>xs:yearMonthDuration * numeric</tt>.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class OperandSwitcherMathematician : public AtomicMathematician
    {
    public:
        /**
         * Creates an OperandSwitcherMathematician.
         *
         * @param mathematician the AtomicMathematician this OperandSwitcherMathematician
         * should switch the operands for. Must be a non @c null, valid pointer.
         */
        OperandSwitcherMathematician(const AtomicMathematician::Ptr &mathematician);

        /**
         * Switch @p o1 and @p o2, and returns the value from the AtomicMathematician
         * this OperandSwitcherMathematician represents.
         */
        virtual Item calculate(const Item &o1,
                                    const Operator op,
                                    const Item &o2,
                                    const PlainSharedPtr<DynamicContext> &context) const;
    private:
        const AtomicMathematician::Ptr m_mather;
    };

    /**
     * @short Performs arithmetics between an AbstractDateTime value and
     * an AbstractDuration value.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DateTimeDurationMathematician : public AtomicMathematician
                                        , public DelegatingSourceLocationReflection
    {
    public:

        inline DateTimeDurationMathematician(const SourceLocationReflection *const r) : DelegatingSourceLocationReflection(r)
        {
        }

        /**
         * @p o1 is an AbstractDateTime and @p o2 is an AbstractDuration.
         *
         */
        virtual Item calculate(const Item &o1,
                                    const Operator op,
                                    const Item &o2,
                                    const PlainSharedPtr<DynamicContext> &context) const;
    };

    /**
     * @short Performs arithmetics between two AbstractDateTime values.
     *
     * @ingroup Patternist_xdm
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AbstractDateTimeMathematician : public AtomicMathematician
    {
    public:
        virtual Item calculate(const Item &o1,
                                    const Operator op,
                                    const Item &o2,
                                    const PlainSharedPtr<DynamicContext> &context) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
