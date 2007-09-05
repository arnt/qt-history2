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

#include "AtomicComparator.h"
#include "CommonValues.h"
#include "DateTime.h"
#include "DayTimeDuration.h"
#include "Decimal.h"
#include "Integer.h"
#include "PatternistLocale.h"

#include "DateTimeFN.h"

using namespace Patternist;

Item DateTimeFN::evaluateSingleton(const DynamicContext::Ptr &context) const
{
    const Item di(m_operands.first()->evaluateSingleton(context));
    if(!di)
        return Item();

    const Item ti(m_operands.last()->evaluateSingleton(context));
    if(!ti)
        return Item();

    QDateTime date(di.as<AbstractDateTime>()->toDateTime());
    Q_ASSERT(date.isValid());
    QDateTime time(ti.as<AbstractDateTime>()->toDateTime());
    Q_ASSERT(time.isValid());

    if(date.timeSpec() == time.timeSpec() || /* Identical timezone properties. */
       time.timeSpec() == Qt::LocalTime) /* time has no timezone, but date do. */
    {
        date.setTime(time.time());
        Q_ASSERT(date.isValid());
        return DateTime::fromDateTime(date);
    }
    else if(date.timeSpec() == Qt::LocalTime) /* date has no timezone, but time do. */
    {
        time.setDate(date.date());
        Q_ASSERT(time.isValid());
        return DateTime::fromDateTime(time);
    }
    else
    {
        context->error(tr("If both values have timezones, they must be identical. %1 "
                          "and %2 is therefore invalid.")
                          .arg(formatData(di.stringValue()), formatData(di.stringValue())),
                       ReportContext::FORG0008, this);
        return Item(); /* Silence GCC warning. */
    }
}

// vim: et:ts=4:sw=4:sts=4
