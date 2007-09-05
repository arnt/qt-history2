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

#ifndef Patternist_DayTimeDuration_H
#define Patternist_DayTimeDuration_H

#include "AbstractDuration.h"
#include "Item.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the value instance of the @c xs:dayTimeDuration type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class DayTimeDuration : public AbstractDuration
    {
    public:

        typedef PlainSharedPtr<DayTimeDuration> Ptr;

        /**
         * Creates an instance from the lexical representation @p string.
         */
        static DayTimeDuration::Ptr fromLexical(const QString &string);

        static DayTimeDuration::Ptr fromComponents(const bool isPositive,
                                                   const DayCountProperty days,
                                                   const HourProperty hours,
                                                   const MinuteProperty minutes,
                                                   const SecondProperty seconds,
                                                   const MSecondProperty mseconds);
        /**
         * Creates a DayTimeDuration that has the value expressed in seconds @p secs
         * and milli seconds @p msecs. The signedness of @p secs communicates
         * whether this DayTimeDuration is positive or negative. @p msecs must always
         * be positive.
         */
        static DayTimeDuration::Ptr fromSeconds(const SecondCountProperty secs,
                                                const MSecondProperty msecs = 0);

        virtual ItemType::Ptr type() const;
        virtual QString stringValue() const;

        /**
         * @returns always 0.
         */
        virtual YearProperty years() const;

        /**
         * @returns always 0.
         */
        virtual MonthProperty months() const;
        virtual DayCountProperty days() const;
        virtual HourProperty hours() const;
        virtual MinuteProperty minutes() const;
        virtual MSecondProperty mseconds() const;
        virtual SecondProperty seconds() const;

        /**
         * @returns the value of this xs:dayTimeDuration
         * in milli seconds.
         * @see <a href="http://www.w3.org/TR/xpath-functions/#dt-dayTimeDuration">XQuery 1.0
         * and XPath 2.0 Functions and Operators, 10.3.2.2 Calculating the value of a
         * xs:dayTimeDuration from the lexical representation</a>
         */
        virtual Value value() const;

        /**
         * Creates a DayTimeDuration containing the value @p val. @p val is
         * expressed in milli seconds.
         *
         * If @p val is zero, is CommonValues::DayTimeDurationZero returned.
         */
        virtual Item fromValue(const Value val) const;

    protected:
        friend class CommonValues;

        DayTimeDuration(const bool isPositive,
                        const DayCountProperty days,
                        const HourProperty hours,
                        const MinuteProperty minutes,
                        const SecondProperty seconds,
                        const MSecondProperty mseconds);

    private:
        const DayCountProperty  m_days;
        const HourProperty      m_hours;
        const MinuteProperty    m_minutes;
        const SecondProperty    m_seconds;
        const MSecondProperty   m_mseconds;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
