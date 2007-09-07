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

#ifndef Patternist_DateTimeFNs_H
#define Patternist_DateTimeFNs_H

#include "qatomiccomparator_p.h"
#include "qcommonvalues_p.h"
#include "qdatetime_p.h"
#include "qdaytimeduration_p.h"
#include "qdecimal_p.h"
#include "qinteger_p.h"
#include "qfunctioncall_p.h"

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#component-exraction-functions">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 10.5 Component Extraction Functions on Durations, Dates and Times</a>.
 *
 * @ingroup Patternist_functions
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Helper class for implementing functions extracting components from durations.
     *
     * Each sub-class must implement this function:
     *
     * @code
     * Item extract(const AbstractDuration *const duration) const;
     * @endcode
     *
     * This function performs the actual component extraction from the argument, that
     * is guaranteed to never be @c null.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename TSubClass>
    class ExtractFromDurationFN : public FunctionCall
    {
    public:
        /**
         * Takes care of the argument handling, and, if applicable,
         * calls extract() with the value of the operand.
         */
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Implements the function <tt>fn:years-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class YearsFromDurationFN : public ExtractFromDurationFN<YearsFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Implements the function <tt>fn:months-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class MonthsFromDurationFN : public ExtractFromDurationFN<MonthsFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Implements the function <tt>fn:days-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DaysFromDurationFN : public ExtractFromDurationFN<DaysFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Implements the function <tt>fn:hours-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class HoursFromDurationFN : public ExtractFromDurationFN<HoursFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Implements the function <tt>fn:minutes-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class MinutesFromDurationFN : public ExtractFromDurationFN<MinutesFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Implements the function <tt>fn:seconds-from-duration()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SecondsFromDurationFN : public ExtractFromDurationFN<SecondsFromDurationFN>
    {
    public:
        inline Item extract(const AbstractDuration *const duration) const;
    };

    /**
     * @short Helper class for implementing functions extracting components
     * from date/time values.
     *
     * Each sub-class must implement this function:
     *
     * @code
     * Item extract(const AbstractDuration *const duration) const;
     * @endcode
     *
     * This function performs the actual component extraction from the argument, that
     * is guaranteed to never be @c null.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    template<typename TSubClass>
    class ExtractFromDateTimeFN : public FunctionCall
    {
    public:
        /**
         * Takes care of the argument handling, and, if applicable,
         * calls extract() with the value of the operand.
         */
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;
    };

    /**
     * @short Extracts the year property from a sub-class of AbstractDateTime such as DateTime or Date.
     * This function implements <tt>fn:year-from-dateTime()</tt> and <tt>fn:year-from-date()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class YearFromAbstractDateTimeFN : public ExtractFromDateTimeFN<YearFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short Extracts the day property from a sub-class of AbstractDateTime such as DateTime or Date.
     * This function implements <tt>fn:day-from-dateTime()</tt> and <tt>fn:day-from-date()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class DayFromAbstractDateTimeFN : public ExtractFromDateTimeFN<DayFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short Extracts the minute property from a sub-class of AbstractDateTime such as DateTime or SchemaTime.
     * Implements the functions <tt>fn:hours-from-dateTime()</tt> and
     * <tt>fn:hours-from-time()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class HoursFromAbstractDateTimeFN : public ExtractFromDateTimeFN<HoursFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short Extracts the minutes property from a sub-class of AbstractDateTime such as DateTime or Date.
     * Implements the functions <tt>fn:minutes-from-dateTime()</tt> and
     * <tt>fn:minutes-from-time()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class MinutesFromAbstractDateTimeFN : public ExtractFromDateTimeFN<MinutesFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short Extracts the seconds property from a sub-class of AbstractDateTime such as DateTime or Date.
     * Implements the functions <tt>fn:seconds-from-dateTime()</tt> and
     * <tt>fn:seconds-from-time()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class SecondsFromAbstractDateTimeFN : public ExtractFromDateTimeFN<SecondsFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short Extracts the timezone property from a sub-class of AbstractDateTime such as DateTime or Date.
     * Implements the functions <tt>fn:timezone-from-dateTime()</tt>,
     * <tt>fn:timezone-from-time()</tt> and <tt>fn:timezone-from-date()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class TimezoneFromAbstractDateTimeFN : public ExtractFromDateTimeFN<TimezoneFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

    /**
     * @short implements the functions <tt>fn:month-from-dateTime()</tt> and <tt>fn:month-from-date()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class MonthFromAbstractDateTimeFN : public ExtractFromDateTimeFN<MonthFromAbstractDateTimeFN>
    {
    public:
        inline Item extract(const QDateTime &dt) const;
    };

#include "qdatetimefns.cpp"
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
