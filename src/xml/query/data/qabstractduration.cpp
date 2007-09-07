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

#include <QStringList>

#include "BuiltinTypes.h"
#include "PatternistLocale.h"
#include "ValidationError.h"

#include "AbstractDuration.h"

using namespace Patternist;

AbstractDuration::AbstractDuration(const bool isPos) : m_isPositive(isPos)
{
}

#define error(msg) return ValidationError::createError(msg);
#define getCapt(sym)        ((captTable.sym == -1) ? QString() : capts.at(captTable.sym))

AtomicValue::Ptr AbstractDuration::create(const CaptureTable &captTable,
                                          const QString &lexical,
                                          bool *isPositive,
                                          YearProperty *years,
                                          MonthProperty *months,
                                          DayCountProperty *days,
                                          HourProperty *hours,
                                          MinuteProperty *minutes,
                                          SecondProperty *seconds,
                                          MSecondProperty *mseconds)
{
    /* We don't directly write into the arguments(eg @p years) but uses these
     * because the arguments are intended for normalized values, and therefore
     * can cause overflows. */
    MonthCountProperty monthCount = 0;
    MinuteCountProperty minCount = 0;
    HourCountProperty hourCount = 0;
    SecondCountProperty secCount = 0;

    qDebug() << Q_FUNC_INFO;
    Q_ASSERT(isPositive);
    QRegExp myExp(captTable.regExp); /* Copy, in order to stay thread safe. */

    if(!myExp.exactMatch(lexical))
    {
        error(QString());
    }

    const QStringList capts(myExp.capturedTexts());

    qDebug();
    qDebug() << "capts: " << capts;

    if(days)
    {
        if(getCapt(tDelimiter).isEmpty())
        {
            if((years && getCapt(year).isEmpty() && getCapt(month).isEmpty() && getCapt(day).isEmpty())
                ||
               (!years && getCapt(day).isEmpty()))
            {
                error(tr("At least one component must be present."));
            }
        }
        else if(getCapt(hour).isEmpty() &&
                getCapt(minutes).isEmpty() &&
                getCapt(seconds).isEmpty() &&
                getCapt(mseconds).isEmpty())
        {
            error(tr("When the %1-delimiter is present, at least one time component "
                       "must be it as well.").arg(formatKeyword("T")));
        }
    }
    else if(getCapt(year).isEmpty() && getCapt(month).isEmpty()) /* This checks xs:yearMonthDuration. */
    {
        error(tr("At least one component must be present."));
    }

    /* If we got no '-', we are positive. */
    *isPositive = capts.at(1).isEmpty();

    if(days)
    {
        Q_ASSERT(hours);
        Q_ASSERT(minutes);
        Q_ASSERT(seconds);
        Q_ASSERT(mseconds);

        *days = getCapt(day).toInt();
        qDebug() << "Days: " << *days;
        hourCount = getCapt(hour).toInt();
        minCount = getCapt(minutes).toInt();
        secCount = getCapt(seconds).toInt();

        const QString msecondsStr(getCapt(mseconds));
        if(!msecondsStr.isEmpty())
            *mseconds = msecondsStr.leftJustified(3, QLatin1Char('0')).toInt();
        else
            *mseconds = msecondsStr.toInt();

        if(secCount > 59)
        {
            minCount += secCount / 60;
            *seconds = secCount % 60;
        }
        else
            *seconds = secCount;

        if(minCount > 59)
        {
            hourCount += minCount / 60;
            *minutes = minCount % 60;
        }
        else
            *minutes = minCount;

        if(hourCount > 23)
        {
            *days += hourCount / 24;
            *hours = hourCount % 24;
        }
        else
            *hours = hourCount;
    }

    if(!years)
        return AtomicValue::Ptr();

    /* We're supposed to handle years/months. */
    Q_ASSERT(months);

    *years = getCapt(year).toInt();
    monthCount = getCapt(month).toInt();

    if(monthCount > 11)
    {
        *years += monthCount / 12;
        *months = monthCount % 12;
    }
    else
        *months = monthCount;

    return AtomicValue::Ptr();
}
#undef error
#undef getCapt

bool AbstractDuration::operator==(const AbstractDuration &other) const
{
    if(years() == other.years()
       && months() == other.months()
       && days() == other.days()
       && hours() == other.hours()
       && minutes() == other.minutes()
       && seconds() == other.seconds()
       && mseconds() == other.mseconds())
    {
        if(isPositive() == other.isPositive())
            return true;
        else if(years() == 0
                && months() == 0
                && days() == 0
                && hours() == 0
                && minutes() == 0
                && seconds () == 0
                && mseconds() == 0)
        {
            return true; /* Signedness doesn't matter if all are zero. */
        }
    }

    return false;
}

QString AbstractDuration::serializeMSeconds(const MSecondProperty mseconds)
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

bool AbstractDuration::isPositive() const
{
    return m_isPositive;
}

// vim: et:ts=4:sw=4:sts=4
