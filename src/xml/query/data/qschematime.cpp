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

#include "qbuiltintypes_p.h"
#include "qitem_p.h"

#include "qschematime_p.h"

QT_BEGIN_NAMESPACE

using namespace Patternist;

SchemaTime::SchemaTime(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

SchemaTime::Ptr SchemaTime::fromLexical(const QString &lexical)
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

    return err ? err : SchemaTime::Ptr(new SchemaTime(retval));
}

SchemaTime::Ptr SchemaTime::fromDateTime(const QDateTime &dt)
{
    Q_ASSERT(dt.isValid());
    /* Singleton value, allocated once instead of each time it's needed. */
    // STATIC DATA
    static const QDate time_defaultDate(AbstractDateTime::DefaultYear,
                                        AbstractDateTime::DefaultMonth,
                                        AbstractDateTime::DefaultDay);

    return SchemaTime::Ptr(new SchemaTime(QDateTime(time_defaultDate,
                                        dt.time(),
                                        dt.timeSpec())));
}

Item SchemaTime::fromValue(const QDateTime &dt) const
{
    Q_ASSERT(dt.isValid());
    return fromDateTime(dt);
}

QString SchemaTime::stringValue() const
{
    return timeToString() + zoneOffsetToString();
}

ItemType::Ptr SchemaTime::type() const
{
    return BuiltinTypes::xsTime;
}

// vim: et:ts=4:sw=4:sts=4

QT_END_NAMESPACE
