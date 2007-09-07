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
#include "Debug.h"

#include "GYear.h"

using namespace Patternist;

GYear::GYear(const QDateTime &dateTime) : AbstractDateTime(dateTime)
{
}

GYear::Ptr GYear::fromLexical(const QString &lexical)
{
    static const CaptureTable captureTable( // STATIC DATA
        QRegExp(QLatin1String(
                "^\\s*"                             /* Any preceding whitespace. */
                "(-?\\d{4,})"                       /* The year part, "1999". */
                "(?:(\\+|-)(\\d{2}):(\\d{2})|(Z))?" /* The zone offset, "+08:24". */
                "\\s*$"                             /* Any terminating whitespace. */)),
        /*zoneOffsetSignP*/         2,
        /*zoneOffsetHourP*/         3,
        /*zoneOffsetMinuteP*/       4,
        /*zoneOffsetUTCSymbolP*/    5,
        /*yearP*/                   1);

    AtomicValue::Ptr err;
    const QDateTime retval(create(err, lexical, captureTable));

    return err ? err : GYear::Ptr(new GYear(retval));
}

GYear::Ptr GYear::fromDateTime(const QDateTime &dt)
{
    QDateTime result(QDate(dt.date().year(), DefaultMonth, DefaultDay));
    copyTimeSpec(dt, result);

    return GYear::Ptr(new GYear(result));
}

QString GYear::stringValue() const
{
    // We want to pad with zeros, so year 12 becomes 0012. */
    const int year = m_dateTime.date().year();

    QString retval;
    retval.reserve(5);

    if(year < 0)
        retval.append(QLatin1Char('-'));

    return retval
           + QString::number(m_dateTime.date().year()).rightJustified(4, QLatin1Char(' '))
           + zoneOffsetToString();
}

ItemType::Ptr GYear::type() const
{
    return BuiltinTypes::xsGYear;
}

// vim: et:ts=4:sw=4:sts=4
