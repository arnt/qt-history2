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

#include <QStringList>

#include "BuiltinTypes.h"
#include "Item.h"
#include "PatternistLocale.h"
#include "ValidationError.h"

#include "AbstractDateTime.h"

using namespace Patternist;

AbstractDateTime::AbstractDateTime(const QDateTime &dateTime) : m_dateTime(dateTime)
{
    Q_ASSERT(dateTime.isValid());
}

#define badData(msg)        errorMessage = ValidationError::createError(msg); return QDateTime()
#define getCapt(sym)        ((captTable.sym == -1) ? QString() : capts.at(captTable.sym))
#define getSafeCapt(sym)    ((captTable.sym == -1) ? QString() : capts.value(captTable.sym))

QDateTime AbstractDateTime::create(AtomicValue::Ptr &errorMessage,
                                   const QString &lexicalSource,
                                   const CaptureTable &captTable)
{
    QRegExp myExp(captTable.regExp);

    if(!myExp.exactMatch(lexicalSource))
    {
        badData(QString());
    }

    const QStringList capts(myExp.capturedTexts());
    qDebug();
    qDebug() << "capts: " << capts;
    const QString yearStr(getCapt(year));

    if(yearStr.size() > 4 && yearStr.at(0) == QLatin1Char('0'))
    {
        badData(tr("If the year part is larger than four digits the "
                   "first digit cannot be %1. %2 is therefore invalid.")
                    .arg(formatData("0"))
                    .arg(formatData(yearStr)));
    }

    /* If the strings are empty, load default values which are
     * guranteed to pass the validness tests. */
    const QString monthStr(getCapt(month));
    const QString dayStr(getCapt(day));
    const YearProperty year = yearStr.isEmpty() ? DefaultYear : yearStr.toInt();
    const MonthProperty month = monthStr.isEmpty() ? DefaultMonth : monthStr.toInt();
    const MonthProperty day = dayStr.isEmpty() ? DefaultDay : dayStr.toInt();

    if(!QDate::isValid(year, month, day))
    {
        /* Try to give an intelligent message. */
        if(day > 31 || day < 1)
        {
            badData(tr("%1 is an invalid day property, it cannot "
                         "be larger than %2 or less than %3.")
                         .arg(formatData(QString::number(day)))
                         .arg(formatData("31"))
                         .arg(formatData("01")));
        }
        else if(month > 12 || month < -12 || month == 0)
        {
            badData(tr("%1 is an invalid month property, it cannot be "
                         "larger than 12 or less than 11.").arg(month));
        }
        else if(QDate::isValid(DefaultYear, month, day))
        {
            /* We can't use the badData() macro here because we need a different
             * error code: FODT0001 instead of FORG0001. */
            errorMessage = ValidationError::createError(tr("Overflow: the date %1 cannot be represented")
                                                           .arg(formatData(QLatin1String("%1-%2-%3"))
                                                           .arg(year)
                                                           .arg(month)
                                                           .arg(day)),
                                                        ReportContext::FODT0001);
            return QDateTime();
        }
        else
        {
            badData(tr("%1 is an invalid day in the month number %2.")
                         .arg(formatData(QString::number(day)))
                         .arg(formatData(QString::number(month))));
        }
    }

    /* Parse the zone offset. */
    ZoneOffsetParseResult zoResult;
    const ZOTotal offset = parseZoneOffset(zoResult, capts, captTable);

    if(zoResult == Error)
    {
        errorMessage = ValidationError::createError();
        /* We encountered an error, so stop processing. */
        return QDateTime();
    }

    QDate date(year, month, day);

    /* Only deal with time if time is needed. */
    if(captTable.hour == -1)
    {
        QDateTime result(date);
        setUtcOffset(result, zoResult, offset);
        return result;
    }
    else
    {
        /* Now, it's time for the time-part.
         *
         * If the strings are empty, toInt() will return 0, which
         * in all cases is valid properties. */
        const QString hourStr(getCapt(hour));
        const QString minutesStr(getCapt(minutes));
        const QString secondsStr(getCapt(seconds));
        HourProperty hour = hourStr.toInt();
        const MinuteProperty mins = minutesStr.toInt();
        const SecondProperty secs = secondsStr.toInt();

        QString msecondsStr(getSafeCapt(mseconds));
        if(!msecondsStr.isEmpty())
            msecondsStr = msecondsStr.leftJustified(3, QLatin1Char('0'));
        const MSecondProperty msecs = msecondsStr.toInt();

        if(hour == 24)
        {
            /* 24:00:00.00 is an invalid time for QTime, so handle it here. */
            if(mins != 0 || secs != 0 || msecs != 0)
            {
                badData(tr("When the time hour property is 24, the minutes and seconds "
                             "properties must be 0, not %1 and %2, respectively.").arg(mins).arg(secs));
            }
            else
            {
                hour = 0;
                date = date.addDays(1);
            }
        }
        else if(!QTime::isValid(hour, mins, secs, msecs))
        {
            badData(tr("The time %1:%2:%3.%4 is invalid.")
                         .arg(hour).arg(mins).arg(secs).arg(msecs));
        }

        const QTime time(hour, mins, secs, msecs);
        Q_ASSERT(time.isValid());

        QDateTime result(date, time);
        setUtcOffset(result, zoResult, offset);
        return result;
    }
}

ZOTotal AbstractDateTime::parseZoneOffset(ZoneOffsetParseResult &result,
                                          const QStringList &capts,
                                          const CaptureTable &captTable)
{
    const QString zoneOffsetSignStr(getCapt(zoneOffsetSign));

    if(zoneOffsetSignStr.isEmpty())
    {
        const QString zoneOffsetUTCStr(getCapt(zoneOffsetUTCSymbol));
        Q_ASSERT(zoneOffsetUTCStr.isEmpty() || zoneOffsetUTCStr == QLatin1String("Z"));

        if(zoneOffsetUTCStr.isEmpty())
            result = LocalTime;
        else
            result = UTC;

        return 0;
    }

    Q_ASSERT(zoneOffsetSignStr == QLatin1String("-") || zoneOffsetSignStr == QLatin1String("+"));

    const QString zoneOffsetHourStr(getCapt(zoneOffsetHour));
    Q_ASSERT(!zoneOffsetHourStr.isEmpty());
    const ZOHourProperty zoHour = zoneOffsetHourStr.toInt();

    if(zoHour > 14 || zoHour < -14)
    {
        result = Error;
        return 0;
        /*
        badZOData(tr("%1 it is not a valid hour property in a zone offset. "
                       "It must be less than or equal to 14.").arg(zoHour));
                       */
    }

    const QString zoneOffsetMinuteStr(getCapt(zoneOffsetMinute));
    Q_ASSERT(!zoneOffsetMinuteStr.isEmpty());
    const ZOHourProperty zoMins = zoneOffsetMinuteStr.toInt();

    if(zoHour == 14 && zoMins != 0)
    {
        /*
        badZOData(tr("When the hour property in a zone offset is 14, the minute property "
                       "must be 0, not %1.").arg(zoMins));
                       */
        result = Error;
        return 0;
    }
    else if(zoMins > 59 || zoMins < -59)
    {
        /*
        badZOData(tr("The minute property in a zone offset cannot be larger than 59. "
                       "%1 is therefore invalid.").arg(zoMins));
                       */
        result = Error;
        return 0;
    }

    if(zoHour == 0 && zoMins == 0) /* "-00:00" and "+00:00" is equal to 'Z'. */
    {
        result = UTC;
        return 0;
    }
    else
    {
        qDebug() << "zoHour: " << zoHour;
        qDebug() << "zoMins: " << zoMins;
        ZOTotal zoneOffset = (zoHour * 60 + zoMins) * 60;

        if(zoneOffsetSignStr == QChar::fromLatin1('-'))
            zoneOffset = -zoneOffset;

        result = Offset;
        return zoneOffset;
    }
}
//#undef badZOData

void AbstractDateTime::setUtcOffset(QDateTime &result,
                                     const ZoneOffsetParseResult zoResult,
                                     const int zoOffset)
{
    if(zoResult == UTC)
        result.setTimeSpec(Qt::UTC);
    else if(zoResult == LocalTime)
        result.setTimeSpec(Qt::LocalTime);
    else
    {
        Q_ASSERT(zoResult == Offset);
        result.setUtcOffset(zoOffset);
    }
}

#undef badData
#undef getCapt
#undef getSafeCapt

bool AbstractDateTime::isRangeValid(const QDate &date,
                                    QString &message)
{
    if(date.isValid())
        return true;
    else
    {
        message = tr("Overflow: the date cannot be represented");
        return false;
    }
}

QString AbstractDateTime::dateToString() const
{
    return m_dateTime.toString(QLatin1String("yyyy-MM-dd"));
}

QString AbstractDateTime::serializeMSeconds(const MSecondProperty mseconds)
{
    QString retval;
    retval.append(QLatin1Char('.'));
    int div = 100;
    MSecondProperty msecs = mseconds;

    while(msecs > 0)
    {
        int d = msecs / div;
        retval.append(QLatin1Char(d + '0'));
        msecs = msecs % div;
        div = div / 10;
    }

    return retval;
}

QString AbstractDateTime::timeToString() const
{
    QString base(m_dateTime.toString(QLatin1String("hh:mm:ss")));
    const MSecondProperty msecs = m_dateTime.time().msec();

    if(msecs)
        base.append(serializeMSeconds(msecs));

    return base;
}

QString AbstractDateTime::zoneOffsetToString() const
{
    switch(m_dateTime.timeSpec())
    {
        case Qt::LocalTime:
            return QString();
        case Qt::UTC:
            return QLatin1String("Z");
        default:
        {
            Q_ASSERT(m_dateTime.timeSpec() == Qt::OffsetFromUTC);

            const int zoneOffset = m_dateTime.utcOffset();
            Q_ASSERT(zoneOffset != 0);
            const int posZoneOffset = qAbs(zoneOffset);

            /* zoneOffset is in seconds. */
            const int hours = posZoneOffset/(60 * 60);
            const int minutes = (posZoneOffset % (60 * 60)) / 60;

            QString result;
            result.reserve(6);

            result.append(zoneOffset < 0 ? QLatin1Char('-') : QLatin1Char('+'));
            result.append(QString::number(hours).rightJustified(2, QLatin1Char('0')));
            result.append(QLatin1Char(':'));
            result.append(QString::number(minutes).rightJustified(2, QLatin1Char('0')));
            return result;
        }
    }
}

void AbstractDateTime::copyTimeSpec(const QDateTime &from,
                                    QDateTime &to)
{
    switch(from.timeSpec())
    {
        case Qt::UTC:
        /* Fallthrough. */
        case Qt::LocalTime:
        {
            to.setTimeSpec(from.timeSpec());
            return;
        }
        case Qt::OffsetFromUTC:
        {
            to.setUtcOffset(from.utcOffset());
            Q_ASSERT(to.timeSpec() == Qt::OffsetFromUTC);
            return;
        }
    }
}

Item AbstractDateTime::fromValue(const QDateTime &) const
{
    Q_ASSERT_X(false, Q_FUNC_INFO,
               "Calling AbstractDateTime::fromDateTime() makes no sense.");
    return Item();
}
// vim: et:ts=4:sw=4:sts=4
