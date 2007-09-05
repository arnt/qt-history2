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

#ifndef Patternist_TimezoneFNs_H
#define Patternist_TimezoneFNs_H

#include "AtomicComparator.h"
#include "FunctionCall.h"

/**
 * @file
 * @short Contains classes implementing the functions found in
 * <a href="http://www.w3.org/TR/xpath-functions/#timezone.functions">XQuery 1.0 and
 * XPath 2.0 Functions and Operators, 10.7 Timezone Adjustment on Dates and Time Values</a>.
 * @ingroup Patternist_functions
 */

QT_BEGIN_HEADER 

namespace Patternist
{
    /**
     * @short Base class for classes implementing functions changing the timezone
     * on values.
     *
     * It would be possible to implement this with the Curiously Recurring Template Pattern, in order
     * to avoid the virtual call dispatching that is done via createValue(). However, these are not
     * very hot code paths and evaluateSingleton() is quite large, which would lead to heavy code
     * expansion.
     *
     * @see <a href="http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern">Curiously
     * Recurring Template Pattern, Wikipedia, the free encyclopedia</a>
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AdjustTimezone : public FunctionCall
    {
    public:
        virtual Item evaluateSingleton(const DynamicContext::Ptr &context) const;

    protected:
        virtual Item createValue(const QDateTime &dt) const = 0;
    };

    /**
     * @short Implements the function <tt>fn:adjust-dateTime-to-timezone()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AdjustDateTimeToTimezoneFN : public AdjustTimezone
    {
    protected:
        virtual Item createValue(const QDateTime &dt) const;
    };

    /**
     * @short Implements the function <tt>fn:adjust-dateTime-to-timezone()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AdjustDateToTimezoneFN : public AdjustTimezone
    {
    protected:
        virtual Item createValue(const QDateTime &dt) const;
    };

    /**
     * @short Implements the function <tt>fn:adjust-time-to-timezone()</tt>.
     *
     * @ingroup Patternist_functions
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class AdjustTimeToTimezoneFN : public AdjustTimezone
    {
    protected:
        virtual Item createValue(const QDateTime &dt) const;
    };
}

QT_END_HEADER 

#endif
// vim: et:ts=4:sw=4:sts=4
