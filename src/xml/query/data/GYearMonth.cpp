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

#include "GYearMonth.h"

using namespace Patternist;

GYearMonth::GYearMonth(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

GYearMonth::Ptr GYearMonth::fromLexical(const QString &lexical)
{
    static const CaptureTable captureTable( // STATIC DATA
        QRegExp(QLatin1String(
                "^\\s*"                             /* Any preceding whitespace. */
                "(\\d{4,})"                         /* The year part. */
                "-"                                 /* Delimiter. */
                "(\\d{2})"                          /* The month part. */
                "(?:(\\+|-)(\\d{2}):(\\d{2})|(Z))?" /* The zone offset, "+08:24". */
                "\\s*$"                             /* Any terminating whitespace. */)),
        /*zoneOffsetSignP*/         3,
        /*zoneOffsetHourP*/         4,
        /*zoneOffsetMinuteP*/       5,
        /*zoneOffsetUTCSymbolP*/    6,
        /*yearP*/                   1,
        /*monthP*/                  2);

    AtomicValue::Ptr err;
    const QDateTime retval(create(err, lexical, captureTable));

    return err ? err : GYearMonth::Ptr(new GYearMonth(retval));
}

GYearMonth::Ptr GYearMonth::fromDateTime(const QDateTime &dt)
{
    QDateTime result(QDate(dt.date().year(), dt.date().month(), DefaultDay));
    copyTimeSpec(dt, result);

    return GYearMonth::Ptr(new GYearMonth(result));
}

QString GYearMonth::stringValue() const
{
    return m_dateTime.toString(QLatin1String("%Y-%m")) + zoneOffsetToString();
}

ItemType::Ptr GYearMonth::type() const
{
    return BuiltinTypes::xsGYearMonth;
}

// vim: et:ts=4:sw=4:sts=4
