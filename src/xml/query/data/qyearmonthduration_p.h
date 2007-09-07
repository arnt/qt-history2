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

#ifndef Patternist_YearMonthDuration_H
#define Patternist_YearMonthDuration_H

#include "qabstractduration_p.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the value instance of the @c xs:yearMonthDuration type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class YearMonthDuration : public AbstractDuration
    {
    public:
        typedef AtomicValue::Ptr Ptr;

        /**
         * Creates an instance from the lexical representation @p string.
         */
        static YearMonthDuration::Ptr fromLexical(const QString &string);

        static YearMonthDuration::Ptr fromComponents(const bool isPositive,
                                                     const YearProperty years,
                                                     const MonthProperty months);

        virtual ItemType::Ptr type() const;
        virtual QString stringValue() const;

        /**
         * @returns the value of this @c xs:yearMonthDuration in months.
         * @see <a href="http://www.w3.org/TR/xpath-functions/#dt-yearMonthDuration">XQuery 1.0
         * and XPath 2.0 Functions and Operators, 10.3.2.2 Calculating the value of a
         * xs:dayTimeDuration from the lexical representation</a>
         */
        virtual Value value() const;

        /**
         * If @p val is zero, is CommonValues::YearMonthDurationZero returned.
         */
        virtual Item fromValue(const Value val) const;

        /**
         * @returns the years component. Always positive.
         */
        virtual YearProperty years() const;

        /**
         * @returns the months component. Always positive.
         */
        virtual MonthProperty months() const;

        /**
         * @returns always 0.
         */
        virtual DayCountProperty days() const;

        /**
         * @returns always 0.
         */
        virtual HourProperty hours() const;

        /**
         * @returns always 0.
         */
        virtual MinuteProperty minutes() const;

        /**
         * @returns always 0.
         */
        virtual SecondProperty seconds() const;

        /**
         * @returns always 0.
         */
        virtual MSecondProperty mseconds() const;

    protected:
        friend class CommonValues;

        YearMonthDuration(const bool isPositive,
                          const YearProperty years,
                          const MonthProperty months);

    private:
        const YearProperty  m_years;
        const MonthProperty m_months;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
