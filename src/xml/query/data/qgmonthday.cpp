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

#include "BuiltinTypes.h"

#include "GMonthDay.h"

using namespace Patternist;

GMonthDay::GMonthDay(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

GMonthDay::Ptr GMonthDay::fromLexical(const QString &lexical)
{
    static const CaptureTable captureTable( // STATIC DATA
        QRegExp(QLatin1String(
                "^\\s*"                             /* Any preceding whitespace. */
                "--"                                /* Delimiter. */
                "(\\d{2})"                          /* The month part. */
                "-"                                 /* Delimiter. */
                "(\\d{2})"                          /* The day part. */
                "(?:(\\+|-)(\\d{2}):(\\d{2})|(Z))?" /* The zone offset, "+08:24". */
                "\\s*$"                             /* Any terminating whitespace. */)),
        /*zoneOffsetSignP*/         3,
        /*zoneOffsetHourP*/         4,
        /*zoneOffsetMinuteP*/       5,
        /*zoneOffsetUTCSymbolP*/    6,
        /*yearP*/                   -1,
        /*monthP*/                  1,
        /*dayP*/                    2);

    AtomicValue::Ptr err;
    const QDateTime retval(create(err, lexical, captureTable));

    return err ? err : GMonthDay::Ptr(new GMonthDay(retval));
}

GMonthDay::Ptr GMonthDay::fromDateTime(const QDateTime &dt)
{
    QDateTime result(QDate(DefaultYear, dt.date().month(), dt.date().day()));
    copyTimeSpec(dt, result);

    return GMonthDay::Ptr(new GMonthDay(result));
}

QString GMonthDay::stringValue() const
{
    return m_dateTime.toString(QLatin1String("--%m-%d")) + zoneOffsetToString();
}

ItemType::Ptr GMonthDay::type() const
{
    return BuiltinTypes::xsGMonthDay;
}

// vim: et:ts=4:sw=4:sts=4
