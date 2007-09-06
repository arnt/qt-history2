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
***************************************************************************
*/

#include "BuiltinTypes.h"

#include "GMonth.h"

using namespace Patternist;

GMonth::GMonth(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

GMonth::Ptr GMonth::fromLexical(const QString &lexical)
{
    static const CaptureTable captureTable( // STATIC DATA
        QRegExp(QLatin1String(
                "^\\s*"                             /* Any preceding whitespace. */
                "--"                                /* Delimier. */
                "(\\d{2})"                          /* The month part, "03". */
                "(?:(\\+|-)(\\d{2}):(\\d{2})|(Z))?" /* Timezone, "+08:24". */
                "\\s*$"                             /* Any terminating whitespace. */)),
        /*zoneOffsetSignP*/         2,
        /*zoneOffsetHourP*/         3,
        /*zoneOffsetMinuteP*/       4,
        /*zoneOffsetUTCSymbolP*/    5,
        /*yearP*/                   -1,
        /*monthP*/                  1);

    AtomicValue::Ptr err;
    const QDateTime retval(create(err, lexical, captureTable));

    return err ? err : GMonth::Ptr(new GMonth(retval));
}

GMonth::Ptr GMonth::fromDateTime(const QDateTime &dt)
{
    QDateTime result(QDate(DefaultYear, dt.date().month(), DefaultDay));
    copyTimeSpec(dt, result);

    return GMonth::Ptr(new GMonth(result));
}

QString GMonth::stringValue() const
{
    return m_dateTime.toString(QLatin1String("--%m")) + zoneOffsetToString();
}

ItemType::Ptr GMonth::type() const
{
    return BuiltinTypes::xsGMonth;
}

// vim: et:ts=4:sw=4:sts=4
