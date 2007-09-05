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

#include "BuiltinTypes.h"
#include "Item.h"

#include "Time.h"

using namespace Patternist;

Time::Time(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

Time::Ptr Time::fromLexical(const QString &lexical)
{
    static const CaptureTable captureTable( // STATIC DATA
        QRegExp(QLatin1String(
                "^\\s*"                             /* Any preceding whitespace. */
                "(\\d{2})"                          /* Hour part */
                ":"                                 /* Delimiter. */
                "(\\d{2})"                          /* Minutes part */
                ":"                                 /* Delimiter. */
                "(\\d{2,})"                         /* Seconds part. */
                "(?:\\.(\\d+))?"                    /* Milli seconds part. */
                "(?:(\\+|-)(\\d{2}):(\\d{2})|(Z))?" /* The zone offset, "+08:24". */
                "\\s*$"                             /* Any terminating whitespace. */)),
        /*zoneOffsetSignP*/         5,
        /*zoneOffsetHourP*/         6,
        /*zoneOffsetMinuteP*/       7,
        /*zoneOffsetUTCSymbolP*/    8,
        /*yearP*/                   -1,
        /*monthP*/                  -1,
        /*dayP*/                    -1,
        /*hourP*/                   1,
        /*minutesP*/                2,
        /*secondsP*/                3,
        /*msecondsP*/               4);

    AtomicValue::Ptr err;
    const QDateTime retval(create(err, lexical, captureTable));

    return err ? err : Time::Ptr(new Time(retval));
}

Time::Ptr Time::fromDateTime(const QDateTime &dt)
{
    Q_ASSERT(dt.isValid());
    /* Singleton value, allocated once instead of each time it's needed. */
    // STATIC DATA
    static const QDate time_defaultDate(AbstractDateTime::DefaultYear,
                                        AbstractDateTime::DefaultMonth,
                                        AbstractDateTime::DefaultDay);

    return Time::Ptr(new Time(QDateTime(time_defaultDate,
                                        dt.time(),
                                        dt.timeSpec())));
}

Item Time::fromValue(const QDateTime &dt) const
{
    Q_ASSERT(dt.isValid());
    return fromDateTime(dt);
}

QString Time::stringValue() const
{
    return timeToString() + zoneOffsetToString();
}

ItemType::Ptr Time::type() const
{
    return BuiltinTypes::xsTime;
}

// vim: et:ts=4:sw=4:sts=4
