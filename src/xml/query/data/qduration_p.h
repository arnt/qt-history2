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

#ifndef Patternist_Duration_H
#define Patternist_Duration_H

#include "AbstractDuration.h"

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Implements the value instance of the @c xs:duration type.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     * @ingroup Patternist_xdm
     */
    class Duration : public AbstractDuration
    {
    public:
        typedef AtomicValue::Ptr Ptr;

        /**
         * Creates an instance from the lexical representation @p string.
         */
        static Duration::Ptr fromLexical(const QString &string);
        static Duration::Ptr fromComponents(const bool isPositive,
                                            const YearProperty years,
                                            const MonthProperty months,
                                            const DayCountProperty days,
                                            const HourProperty hours,
                                            const MinuteProperty minutes,
                                            const SecondProperty seconds,
                                            const MSecondProperty mseconds);

        virtual ItemType::Ptr type() const;
        virtual QString stringValue() const;

        /**
         * Always results in an assert crash. Calling this function makes no
         * sense due to that the value space of xs:duration is not well defined.
         */
        virtual Value value() const;

        /**
         * Always results in an assert crash. Calling this function makes no
         * sense due to that the value space of xs:duration is not well defined.
         */
        virtual Item fromValue(const Value val) const;

        virtual YearProperty years() const;
        virtual MonthProperty months() const;
        virtual DayCountProperty days() const;
        virtual HourProperty hours() const;
        virtual MinuteProperty minutes() const;
        virtual SecondProperty seconds() const;
        virtual MSecondProperty mseconds() const;

    protected:
        friend class CommonValues;

        Duration(const bool isPositive,
                 const YearProperty years,
                 const MonthProperty months,
                 const DayCountProperty days,
                 const HourProperty hours,
                 const MinuteProperty minutes,
                 const SecondProperty seconds,
                 const MSecondProperty mseconds);
    private:
        const YearProperty      m_years;
        const MonthProperty     m_months;
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
