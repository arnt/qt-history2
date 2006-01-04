/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include "private/qdatetime_p.h"

#include "qdatastream.h"
#include "qset.h"
#include "qlocale.h"
#include "qdatetime.h"
#include "qregexp.h"
#include "qdebug.h"
#if defined(Q_OS_WIN32)
#include <windows.h>
#endif
#ifndef Q_WS_WIN
#include <locale.h>
#endif

#include <time.h>

//#define QDATETIMEPARSER_DEBUG
#if defined (QDATETIMEPARSER_DEBUG) && !defined(QT_NO_DEBUG_STREAM)
#  define QDTPDEBUG qDebug() << QString("%1:%2").arg(__FILE__).arg(__LINE__)
#  define QDTPDEBUGN qDebug
#else
#  define QDTPDEBUG if (false) qDebug()
#  define QDTPDEBUGN if (false) qDebug
#endif

#if defined(Q_WS_MAC)
#include <private/qcore_mac_p.h>
extern QString qt_mac_from_pascal_string(const Str255); // qglobal.cpp
#endif

enum {
    FIRST_DAY = 2361222,        // Julian day for 1752-09-14
    FIRST_YEAR = 1752,
    SECS_PER_DAY = 86400,
    MSECS_PER_DAY = 86400000,
    SECS_PER_HOUR = 3600,
    MSECS_PER_HOUR = 3600000,
    SECS_PER_MIN = 60,
    MSECS_PER_MIN = 60000
};

static const short monthDays[] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

#ifndef QT_NO_TEXTDATE
static const char * const qt_shortMonthNames[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static QString fmtDateTime(const QString& f, const QTime* dt = 0, const QDate* dd = 0);
#endif

/*****************************************************************************
  QDate member functions
 *****************************************************************************/

/*!
    \class QDate
    \reentrant
    \brief The QDate class provides date functions.

    \ingroup time
    \mainclass

    A QDate object contains a calendar date, i.e. year, month, and day
    numbers, in the modern Western (Gregorian) calendar. It can read
    the current date from the system clock. It provides functions for
    comparing dates, and for manipulating dates. For example, it is
    possible to add and subtract days, months, and years to dates.

    A QDate object is typically created either by giving the year,
    month, and day numbers explicitly, or by using the static function
    currentDate() that creates a QDate object containing the system
    clock's date. An explicit date can also be set using setYMD(). The
    fromString() function returns a QDate given a string and a date
    format which is used to interpret the date within the string.

    The year(), month(), and day() functions provide access to the
    year, month, and day numbers. Also, dayOfWeek() and dayOfYear()
    functions are provided. The same information is provided in
    textual format by the toString(), shortDayName(), longDayName(),
    shortMonthName(), and longMonthName() functions.

    QDate provides a full set of operators to compare two QDate
    objects where smaller means earlier, and larger means later.

    You can increment (or decrement) a date by a given number of days
    using addDays(). Similarly you can use addMonths() and addYears().
    The daysTo() function returns the number of days between two
    dates.

    The daysInMonth() and daysInYear() functions return how many days
    there are in this date's month and year, respectively. The
    isLeapYear() function indicates whether this date is in a leap year.

    Note that QDate should not be used for date calculations for
    dates prior to the introduction of the Gregorian calendar. This
    calendar was adopted by England from the 14 September 1752 (hence
    this is the earliest valid QDate), and subsequently by most other
    Western countries, by 1923. The latest valid year within this
    scheme is the year 8000.

    \sa QTime QDateTime QDateEdit QDateTimeEdit
*/

/*!
    \fn QDate::QDate()

    Constructs a null date. Null dates are invalid.

    \sa isNull(), isValid()
*/


/*!
    Constructs a date with year \a y, month \a m and day \a d.

    \a y must be in the range 1752 to 8000, \a m must be in the range
    1 to 12, and \a d must be in the range 1 to 31.

    \warning If \a y is in the range 0 to 99, it is interpreted as
     a year in the range 1900 to 1999.

    \sa isValid()
*/

QDate::QDate(int y, int m, int d)
{
    jd = 0;
    setYMD(y, m, d);
}


/*!
    \fn bool QDate::isNull() const

    Returns true if the date is null; otherwise returns false. A null
    date is invalid.

    \sa isValid()
*/


/*!
    Returns true if this date is valid; otherwise returns false.

    \sa isNull()
*/

bool QDate::isValid() const
{
    return jd >= FIRST_DAY;
}


/*!
    Returns the year (1752 to 8000) of this date.

    \sa month(), day()
*/

int QDate::year() const
{
    int y, m, d;
    julianToGregorian(jd, y, m, d);
    return y;
}

/*!
    Returns the number corresponding to the month of this date, using
    the following convention:

    \list
    \i 1 = "January"
    \i 2 = "February"
    \i 3 = "March"
    \i 4 = "April"
    \i 5 = "May"
    \i 6 = "June"
    \i 7 = "July"
    \i 8 = "August"
    \i 9 = "September"
    \i 10 = "October"
    \i 11 = "November"
    \i 12 = "December"
    \endlist

    \sa year(), day()
*/

int QDate::month() const
{
    int y, m, d;
    julianToGregorian(jd, y, m, d);
    return m;
}

/*!
    Returns the day of the month (1 to 31) of this date.

    \sa year(), month(), dayOfWeek()
*/

int QDate::day() const
{
    int y, m, d;
    julianToGregorian(jd, y, m, d);
    return d;
}

/*!
    Returns the weekday for this date.

    \sa day(), dayOfYear(), Qt::DayOfWeek
*/

int QDate::dayOfWeek() const
{
    return (jd % 7) + 1;
}

/*!
    Returns the day of the year (1 to 365) for this date.

    \sa day(), dayOfWeek()
*/

int QDate::dayOfYear() const
{
    return jd - gregorianToJulian(year(), 1, 1) + 1;
}

/*!
    Returns the number of days in the month (28 to 31) for this date.

    \sa day(), daysInYear()
*/

int QDate::daysInMonth() const
{
    int y, m, d;
    julianToGregorian(jd, y, m, d);
    if (m == 2 && isLeapYear(y))
        return 29;
    else
        return monthDays[m];
}

/*!
    Returns the number of days in the year (365 or 366) for this date.

    \sa day(), daysInMonth()
*/

int QDate::daysInYear() const
{
    int y, m, d;
    julianToGregorian(jd, y, m, d);
    return isLeapYear(y) ? 366 : 365;
}

/*!
    Returns the week number (1 to 53), and stores the year in
    *\a{yearNumber} unless \a yearNumber is null (the default).

    Returns 0 if the date is invalid.

    In accordance with ISO 8601, weeks start on Qt::Monday and the first
    Qt::Thursday of a year is always in week 1 of that year. Most years
    have 52 weeks, but some have 53.

    *\a{yearNumber} is not always the same as year(). For example, 1
    January 2000 has week number 52 in the year 1999, and 31 December
    2002 has week number 1 in the year 2003.

    \legalese
    Copyright (c) 1989 The Regents of the University of California.
    All rights reserved.

    Redistribution and use in source and binary forms are permitted
    provided that the above copyright notice and this paragraph are
    duplicated in all such forms and that any documentation,
    advertising materials, and other materials related to such
    distribution and use acknowledge that the software was developed
    by the University of California, Berkeley.  The name of the
    University may not be used to endorse or promote products derived
    from this software without specific prior written permission.
    THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

    \sa isValid()
*/

int QDate::weekNumber(int *yearNumber) const
{
    if (!isValid())
        return 0;

    int year = QDate::year();
    int yday = dayOfYear() - 1;
    int wday = dayOfWeek();
    if (wday == 7)
        wday = 0;
    int w;

    for (;;) {
        int len;
        int bot;
        int top;

        len = isLeapYear(year) ? 366 : 365;
        /*
        ** What yday (-3 ... 3) does
        ** the ISO year begin on?
        */
        bot = ((yday + 11 - wday) % 7) - 3;
        /*
        ** What yday does the NEXT
        ** ISO year begin on?
        */
        top = bot - (len % 7);
        if (top < -3)
            top += 7;
        top += len;
        if (yday >= top) {
            ++year;
            w = 1;
            break;
        }
        if (yday >= bot) {
            w = 1 + ((yday - bot) / 7);
            break;
        }
        --year;
        yday += isLeapYear(year) ? 366 : 365;
    }
    if (yearNumber != 0)
        *yearNumber = year;
    return w;
}

#ifndef QT_NO_TEXTDATE
/*!
    Returns the name of the \a month using the following
    convention:

    \list
    \i 1 = "Jan"
    \i 2 = "Feb"
    \i 3 = "Mar"
    \i 4 = "Apr"
    \i 5 = "May"
    \i 6 = "Jun"
    \i 7 = "Jul"
    \i 8 = "Aug"
    \i 9 = "Sep"
    \i 10 = "Oct"
    \i 11 = "Nov"
    \i 12 = "Dec"
    \endlist

    The month names will be localized according to the system's locale
    settings.

    \sa toString(), longMonthName(), shortDayName(), longDayName()
*/

QString QDate::shortMonthName(int month)
{
    if (month < 1 || month > 12) {
        qWarning("QDate::shortMonthName: Parameter out ouf range");
        month = 1;
    }
#ifndef Q_WS_WIN
    char buffer[255];
    tm tt;
    memset(&tt, 0, sizeof(tm));
    tt.tm_mon = month - 1;
    const QByteArray lctime(setlocale(LC_TIME, ""));
    if (strftime(buffer, sizeof(buffer), "%b", &tt)) {
        setlocale(LC_TIME, lctime.data());
        return QString::fromLocal8Bit(buffer);
    }
    setlocale(LC_TIME, lctime.data());
#else
    SYSTEMTIME st;
    memset(&st, 0, sizeof(SYSTEMTIME));
    st.wYear = 2000;
    st.wMonth = month;
    st.wDay = 1;
    const wchar_t mmm_t[] = L"MMM"; // workaround for Borland
    QT_WA({
        TCHAR buf[255];
        if (GetDateFormat(GetThreadLocale(), 0, &st, mmm_t, buf, 255))
            return QString::fromUtf16((ushort*)buf);
    } , {
        char buf[255];
        if (GetDateFormatA(GetThreadLocale(), 0, &st, "MMM", (char*)&buf, 255))
            return QString::fromLocal8Bit(buf);
    });
#endif
    return QString();
}

/*!
    Returns the long name of the \a month using the following
    convention:

    \list
    \i 1 = "January"
    \i 2 = "February"
    \i 3 = "March"
    \i 4 = "April"
    \i 5 = "May"
    \i 6 = "June"
    \i 7 = "July"
    \i 8 = "August"
    \i 9 = "September"
    \i 10 = "October"
    \i 11 = "November"
    \i 12 = "December"
    \endlist

    The month names will be localized according to the system's locale
    settings.

    \sa toString(), shortMonthName(), shortDayName(), longDayName()
*/

QString QDate::longMonthName(int month)
{
    if (month < 1 || month > 12) {
        qWarning("QDate::longMonthName: Parameter out ouf range");
        month = 1;
    }
#ifndef Q_WS_WIN
    char buffer[255];
    tm tt;
    memset(&tt, 0, sizeof(tm));
    tt.tm_mon = month - 1;
    const QByteArray lctime(setlocale(LC_TIME, ""));
    if (strftime(buffer, sizeof(buffer), "%B", &tt)) {
        setlocale(LC_TIME, lctime.data());
        return QString::fromLocal8Bit(buffer);
    }
    setlocale(LC_TIME, lctime.data());
#else
    SYSTEMTIME st;
    memset(&st, 0, sizeof(SYSTEMTIME));
    st.wYear = 2000;
    st.wMonth = month;
    st.wDay = 1;
    const wchar_t mmmm_t[] = L"MMMM"; // workaround for Borland
    QT_WA({
        TCHAR buf[255];
        if (GetDateFormat(GetThreadLocale(), 0, &st, mmmm_t, buf, 255))
            return QString::fromUtf16((ushort*)buf);
    } , {
        char buf[255];
        if (GetDateFormatA(GetThreadLocale(), 0, &st, "MMMM", (char*)&buf, 255))
            return QString::fromLocal8Bit(buf);
    })
#endif
    return QString();
}

/*!
    Returns the name of the \a weekday using the following
    convention:

    \list
    \i 1 = "Mon"
    \i 2 = "Tue"
    \i 3 = "Wed"
    \i 4 = "Thu"
    \i 5 = "Fri"
    \i 6 = "Sat"
    \i 7 = "Sun"
    \endlist

    The day names will be localized according to the system's locale
    settings.

    \sa toString(), shortMonthName(), longMonthName(), longDayName()
*/

QString QDate::shortDayName(int weekday)
{
    if (weekday < 1 || weekday > 7) {
        qWarning("QDate::shortDayName: Parameter out of range");
        weekday = 1;
    }
#ifndef Q_WS_WIN
    char buffer[255];
    tm tt;
    memset(&tt, 0, sizeof(tm));
    tt.tm_wday = (weekday == 7) ? 0 : weekday;
    const QByteArray lctime(setlocale(LC_TIME, ""));
    if (strftime(buffer, sizeof(buffer), "%a", &tt)) {
        setlocale(LC_TIME, lctime.data());
        return QString::fromLocal8Bit(buffer);
    }
    setlocale(LC_TIME, lctime.data());

#else
    SYSTEMTIME st;
    memset(&st, 0, sizeof(SYSTEMTIME));
    st.wYear = 2001;
    st.wMonth = 10;
    st.wDayOfWeek = (weekday == 7) ? 0 : weekday;
    st.wDay = 21 + st.wDayOfWeek;
    const wchar_t ddd_t[] = L"ddd"; // workaround for Borland
    QT_WA({
        TCHAR buf[255];
        if (GetDateFormat(GetThreadLocale(), 0, &st, ddd_t, buf, 255))
            return QString::fromUtf16((ushort*)buf);
    } , {
        char buf[255];
        if (GetDateFormatA(GetThreadLocale(), 0, &st, "ddd", (char*)&buf, 255))
            return QString::fromLocal8Bit(buf);
    });
#endif
    return QString();
}

/*!
    Returns the long name of the \a weekday using the following
    convention:

    \list
    \i 1 = "Monday"
    \i 2 = "Tuesday"
    \i 3 = "Wednesday"
    \i 4 = "Thursday"
    \i 5 = "Friday"
    \i 6 = "Saturday"
    \i 7 = "Sunday"
    \endlist

    The day names will be localized according to the system's locale
    settings.

    \sa toString(), shortDayName(), shortMonthName(), longMonthName()
*/

QString QDate::longDayName(int weekday)
{
    if (weekday < 1 || weekday > 7) {
        qWarning("QDate::longDayName: Parameter out of range");
        weekday = 1;
    }
#ifndef Q_WS_WIN
    char buffer[255];
    tm tt;
    memset(&tt, 0, sizeof(tm));
    tt.tm_wday = (weekday == 7) ? 0 : weekday;
    const QByteArray lctime(setlocale(LC_TIME, ""));
    if (strftime(buffer, sizeof(buffer), "%A", &tt)) {
        setlocale(LC_TIME, lctime.data());
        return QString::fromLocal8Bit(buffer);
    }
    setlocale(LC_TIME, lctime.data());
#else
    SYSTEMTIME st;
    memset(&st, 0, sizeof(SYSTEMTIME));
    st.wYear = 2001;
    st.wMonth = 10;
    st.wDayOfWeek = (weekday == 7) ? 0 : weekday;
    st.wDay = 21 + st.wDayOfWeek;
    const wchar_t dddd_t[] = L"dddd"; // workaround for Borland
    QT_WA({
        TCHAR buf[255];
        if (GetDateFormat(GetThreadLocale(), 0, &st, dddd_t, buf, 255))
            return QString::fromUtf16((ushort*)buf);
    } , {
        char buf[255];
        if (GetDateFormatA(GetThreadLocale(), 0, &st, "dddd", (char*)&buf, 255))
            return QString::fromLocal8Bit(buf);
    });
#endif
    return QString();
}
#endif //QT_NO_TEXTDATE

#ifndef QT_NO_DATESTRING

/*!
    \fn QString QDate::toString(Qt::DateFormat format) const

    \overload

    Returns the date as a string. The \a format parameter determines
    the format of the string.

    If the \a format is Qt::TextDate, the string is formatted in
    the default way. QDate::shortDayName() and QDate::shortMonthName()
    are used to generate the string, so the day and month names will
    be localized names. An example of this formatting is
    "Sat May 20 1995".

    If the \a format is Qt::ISODate, the string format corresponds
    to the ISO 8601 extended specification for representations of
    dates and times, taking the form YYYY-MM-DD, where YYYY is the
    year, MM is the month of the year (between 01 and 12), and DD is
    the day of the month between 01 and 31.

    If the \a format is Qt::LocalDate, the string format depends
    on the locale settings of the system.

    If the datetime is invalid, an empty string will be returned.

    \sa shortDayName(), shortMonthName()
*/
QString QDate::toString(Qt::DateFormat f) const
{
    if (!isValid())
        return QString();
    int y, m, d;
    julianToGregorian(jd, y, m, d);
    switch (f) {
    case Qt::LocalDate:
        {
#ifdef Q_WS_WIN
            SYSTEMTIME st;
            memset(&st, 0, sizeof(SYSTEMTIME));
            st.wYear = year();
            st.wMonth = month();
            st.wDay = day();
            QT_WA({
                TCHAR buf[255];
                if (GetDateFormat(GetThreadLocale(), 0, &st, 0, buf, 255))
                    return QString::fromUtf16((ushort*)buf);
            } , {
                char buf[255];
                if (GetDateFormatA(GetThreadLocale(), 0, &st, 0, (char*)&buf, 255))
                    return QString::fromLocal8Bit(buf);
            });
#elif defined(Q_WS_MAC)
            CFGregorianDate macGDate;
            macGDate.year = year();
            macGDate.month = month();
            macGDate.day = day();
            macGDate.hour = 0;
            macGDate.minute = 0;
            macGDate.second = 0.0;
            QCFType<CFDateRef> myDate = CFDateCreate(0, CFGregorianDateGetAbsoluteTime(macGDate, 0));
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                QCFType<CFLocaleRef> mylocale = CFLocaleCopyCurrent();
                QCFType<CFDateFormatterRef> myFormatter = CFDateFormatterCreate(kCFAllocatorDefault,
                                                                                mylocale, kCFDateFormatterLongStyle,
                                                                                kCFDateFormatterNoStyle);
                return QCFString(CFDateFormatterCreateStringWithDate(0, myFormatter, myDate));
            } else
#endif
            {
                Handle intlHandle = GetIntlResource(1);
                LongDateTime oldDate;
                UCConvertCFAbsoluteTimeToLongDateTime(CFGregorianDateGetAbsoluteTime(macGDate, 0),
                                                      &oldDate);
                Str255 pString;
                LongDateString(&oldDate, longDate, pString, intlHandle);
                return qt_mac_from_pascal_string(pString);
            }
#else
            tm tt;
            memset(&tt, 0, sizeof(tm));
            char buf[255];
            tt.tm_mday = day();
            tt.tm_mon = month() - 1;
            tt.tm_year = year() - 1900;

            const char *avoidEgcsWarning = "%x";
            const QByteArray lctime(setlocale(LC_TIME, ""));
            if (strftime(buf, sizeof(buf), avoidEgcsWarning, &tt)) {
                setlocale(LC_TIME, lctime.data());
                return QString::fromLocal8Bit(buf);
            }
            setlocale(LC_TIME, lctime.data());
#endif
            return QString();
        }
    default:
#ifndef QT_NO_TEXTDATE
    case Qt::TextDate:
        {
            return QString::fromLatin1("%0 %1 %2 %3")
                .arg(shortDayName(dayOfWeek()))
                .arg(shortMonthName(m))
                .arg(d)
                .arg(y);
        }
#endif
    case Qt::ISODate:
        {
            QString month(QString::number(m).rightJustified(2, QLatin1Char('0')));
            QString day(QString::number(d).rightJustified(2, QLatin1Char('0')));
            return QString::number(y) + QLatin1Char('-') + month + QLatin1Char('-') + day;
        }
    }
}

/*!
    Returns the date as a string. The \a format parameter determines
    the format of the result string.

    These expressions may be used:

    \table
    \header \i Expression \i Output
    \row \i d \i the day as number without a leading zero (1 to31)
    \row \i dd \i the day as number with a leading zero (01 to 31)
    \row \i ddd
         \i the abbreviated localized day name (e.g. 'Mon' to 'Sun').
            Uses QDate::shortDayName().
    \row \i dddd
         \i the long localized day name (e.g. 'Qt::Monday' to 'Qt::Sunday').
            Uses QDate::longDayName().
    \row \i M \i the month as number without a leading zero (1-12)
    \row \i MM \i the month as number with a leading zero (01-12)
    \row \i MMM
         \i the abbreviated localized month name (e.g. 'Jan' to 'Dec').
            Uses QDate::shortMonthName().
    \row \i MMMM
         \i the long localized month name (e.g. 'January' to 'December').
            Uses QDate::longMonthName().
    \row \i yy \i the year as two digit number (00 to 99)
    \row \i yyyy \i the year as four digit number (1752 to 8000)
    \endtable

    All other input characters will be ignored. Any sequence of characters that
    are enclosed in singlequotes will be treated as text and not be used as an
    expression.

    Example format strings (assuming that the QDate is the 20 July
    1969):

    \table
    \header \o Format            \o Result
    \row    \o dd.MM.yyyy        \o 20.07.1969
    \row    \o ddd MMMM d yy     \o Sun July 20 69
    \row    \o 'The day is' dddd \o The day is Sunday
    \endtable

    If the datetime is invalid, an empty string will be returned.

    \sa QDateTime::toString() QTime::toString()

*/
QString QDate::toString(const QString& format) const
{
    return fmtDateTime(format, 0, this);
}
#endif //QT_NO_DATESTRING

/*!
    Sets the date's year \a y, month \a m, and day \a d.

    \a y must be in the range 1752 to 8000, \a m must be in the range
    1 to 12, and \a d must be in the range 1 to 31.

    \warning If \a y is in the range 0 to 99, it is interpreted as
    1900 to 1999.

    Returns true if the date is valid; otherwise returns false.
*/

bool QDate::setYMD(int y, int m, int d)
{
    if (year() == y && month() == m && day() == d)
        return isValid();
    if (!isValid(y,m,d)) {
        jd = 0;
        return false;
    }
    jd = gregorianToJulian(y, m, d);
    return true;
}

/*!
    Returns a QDate object containing a date \a ndays later than the
    date of this object (or earlier if \a ndays is negative).

    \sa addMonths() addYears() daysTo()
*/

QDate QDate::addDays(int ndays) const
{
    QDate d;
    d.jd = jd + ndays;
    return d;
}

/*!
    Returns a QDate object containing a date \a nmonths later than the
    date of this object (or earlier if \a nmonths is negative).

    \sa addDays() addYears()
*/

QDate QDate::addMonths(int nmonths) const
{
    int y, m, d;
    julianToGregorian(jd, y, m, d);

    while (nmonths != 0) {
        if (nmonths < 0 && nmonths + 12 <= 0) {
            y--;
            nmonths+=12;
        } else if (nmonths < 0) {
            m+= nmonths;
            nmonths = 0;
            if (m <= 0) {
                --y;
                m+=12;
            }
        } else if (nmonths - 12 >= 0) {
            y++;
            nmonths -= 12;
        } else if (m == 12) {
            y++;
            m = 0;
        } else {
            m+= nmonths;
            nmonths = 0;
            if (m > 12) {
                ++y;
                m -= 12;
            }
        }
    }

    QDate tmp(y, m, 1);
    if (d > tmp.daysInMonth())
        d = tmp.daysInMonth();

    return QDate(y, m, d);
}

/*!
    Returns a QDate object containing a date \a nyears later than the
    date of this object (or earlier if \a nyears is negative).

    \sa addDays(), addMonths()
*/

QDate QDate::addYears(int nyears) const
{
    int y, m, d;
    julianToGregorian(jd, y, m, d);
    y += nyears;

    QDate tmp(y,m,1);

    if(d > tmp.daysInMonth())
        d = tmp.daysInMonth();

    QDate date(y, m, d);
    return date;
}



/*!
    Returns the number of days from this date to \a d (which is
    negative if \a d is earlier than this date).

    Example:
    \code
        QDate d1(1995, 5, 17);  // May 17, 1995
        QDate d2(1995, 5, 20);  // May 20, 1995
        d1.daysTo(d2);          // returns 3
        d2.daysTo(d1);          // returns -3
    \endcode

    \sa addDays()
*/

int QDate::daysTo(const QDate &d) const
{
    return d.jd - jd;
}


/*!
    \fn bool QDate::operator==(const QDate &d) const

    Returns true if this date is equal to \a d; otherwise returns
    false.
*/

/*!
    \fn bool QDate::operator!=(const QDate &d) const

    Returns true if this date is different from \a d; otherwise
    returns false.
*/

/*!
    \fn bool QDate::operator<(const QDate &d) const

    Returns true if this date is earlier than \a d; otherwise returns
    false.
*/

/*!
    \fn bool QDate::operator<=(const QDate &d) const

    Returns true if this date is earlier than or equal to \a d;
    otherwise returns false.
*/

/*!
    \fn bool QDate::operator>(const QDate &d) const

    Returns true if this date is later than \a d; otherwise returns
    false.
*/

/*!
    \fn bool QDate::operator>=(const QDate &d) const

    Returns true if this date is later than or equal to \a d;
    otherwise returns false.
*/

/*!
    \overload
    Returns the current date, as reported by the system clock.

    \sa QTime::currentTime(), QDateTime::currentDateTime()
*/

QDate QDate::currentDate()
{
    QDate d;
#if defined(Q_OS_WIN32)
    SYSTEMTIME st;
    memset(&st, 0, sizeof(SYSTEMTIME));
    GetLocalTime(&st);
    d.jd = QDate::gregorianToJulian(st.wYear, st.wMonth, st.wDay);
#else
    // posix compliant system
    time_t ltime;
    time(&ltime);
    tm *t;

#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant version of localtime() where available
    tm res;
    t = localtime_r(&ltime, &res);
#else
    t = localtime(&ltime);
#endif // !QT_NO_THREAD && _POSIX_THREAD_SAFE_FUNCTIONS

    d.jd = gregorianToJulian(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
#endif
    return d;
}

#ifndef QT_NO_DATESTRING
/*!
    \fn QDate QDate::fromString(const QString &string, Qt::DateFormat format)

    Returns the QDate represented by the \a string, using the
    \a format given, or an invalid date if the string cannot be
    parsed.

    Note for Qt::TextDate: It is recommended that you use the
    English short month names (e.g. "Jan"). Although localized month
    names can also be used, they depend on the user's locale settings.

    \warning Qt::LocalDate cannot be used here.
*/
QDate QDate::fromString(const QString& s, Qt::DateFormat f)
{
    if ((s.isEmpty()) || (f == Qt::LocalDate)) {
        QDate d;
        d.jd = 0;
        return d;
    }
    switch (f) {
    case Qt::ISODate:
        {
            int year(s.mid(0, 4).toInt());
            int month(s.mid(5, 2).toInt());
            int day(s.mid(8, 2).toInt());
            if (year && month && day)
                return QDate(year, month, day);
        }
        break;
    default:
#ifndef QT_NO_TEXTDATE
    case Qt::TextDate:
        {
            /*
              This will fail gracefully if the input string doesn't
              contain any space.
            */
            int monthPos = s.indexOf(QLatin1Char(' ')) + 1;
            int dayPos = s.indexOf(QLatin1Char(' '), monthPos) + 1;

            QString monthName(s.mid(monthPos, dayPos - monthPos - 1));
            int month = -1;

            // try English names first
            for (int i = 0; i < 12; i++) {
                if (monthName == QLatin1String(qt_shortMonthNames[i])) {
                    month = i + 1;
                    break;
                }
            }

            // try the localized names
            if (month == -1) {
                for (int i = 0; i < 12; i++) {
                    if (monthName == shortMonthName(i + 1)) {
                        month = i + 1;
                        break;
                    }
                }
            }
            if (month < 1 || month > 12) {
                QDate d;
                d.jd = 0;
                return d;
            }
            int day = s.mid(dayPos, 2).trimmed().toInt();
            int year = s.right(4).toInt();
            return QDate(year, month, day);
        }
#else
        break;
#endif
    }
    return QDate();
}

/*!
    \fn QDate::fromString(const QString &string, const QString &format)

    Returns the QDate represented by the \a string, using the \a
    format given, or an invalid date if the string cannot be parsed.

    These expressions may be used for the format:

    \table
    \header \i Expression \i Output
    \row \i d \i The day as a number without a leading zero (1 to 31)
    \row \i dd \i The day as a number with a leading zero (01 to 31)
    \row \i ddd
         \i The abbreviated localized day name (e.g. 'Mon' to 'Sun').
            Uses QDate::shortDayName().
    \row \i dddd
         \i The long localized day name (e.g. 'Monday' to 'Sunday').
            Uses QDate::longDayName().
    \row \i M \i The month as a number without a leading zero (1 to 12)
    \row \i MM \i The month as a number with a leading zero (01 to 12)
    \row \i MMM
         \i The abbreviated localized month name (e.g. 'Jan' to 'Dec').
            Uses QDate::shortMonthName().
    \row \i MMMM
         \i The long localized month name (e.g. 'January' to 'December').
            Uses QDate::longMonthName().
    \row \i yy \i The year as two digit number (00 to 99)
    \row \i yyyy \i The year as four digit number (1752 to 8000)
    \endtable

    All other input characters will be treated as text. Any sequence
    of characters that are enclosed in single quotes will also be
    treated as text and will not be used as an expression. For example:

    \code
        QDate date = QDate::fromString("1MM12car2003", "d'MM'MMcaryyyy");
        // date is 1 December 2003
    \endcode

    If the format is not satisfied, an invalid QDate is returned. The
    expressions that don't expect leading zeroes (d, M) will be
    greedy. This means that they will use two digits even if this
    will put them outside the accepted range of values and leaves too
    few digits for other sections. For example, the following format
    string could have meant January 30 but the M will grab two
    digits, resulting in an invalid date:

    \code
        QDate date = QDate::fromString("130", "Md"); // invalid
    \endcode

    For any field that is not represented in the format the following
    defaults are used:

    \table
    \header \i Field  \i Default value
    \row    \i Year   \i 1900
    \row    \i Month  \i 1
    \row    \i Day    \i 1
    \endtable

    The following examples demonstrate the default values:

    \code
        QDate::fromString("1.30", "M.d");           // January 30 1900
        QDate::fromString("20000110", "yyyyMMdd");  // January 10, 2000
        QDate::fromString("20000110", "yyyyMd");    // January 10, 2000
    \endcode

    \sa QDateTime::fromString(), QTime::fromString(), QDate::toString(),
        QDateTime::toString(), QTime::toString()
*/

QDate QDate::fromString(const QString &string, const QString &format)
{
    QDate date;
#ifndef QT_BOOTSTRAPPED
    QDateTimeParser dt(QVariant::Date);
    if (dt.parseFormat(format))
        dt.fromString(string, &date, 0);
#else
    Q_UNUSED(string);
    Q_UNUSED(format);
#endif
    return date;
}
#endif // QT_NO_DATESTRING

/*!
    \overload

    Returns true if the specified date (year \a y, month \a m, and day
    \a d) is valid; otherwise returns false.

    Example:
    \code
        QDate::isValid(2002, 5, 17);  // true
        QDate::isValid(2002, 2, 30);  // false (Feb 30 does not exist)
        QDate::isValid(2004, 2, 29);  // true (2004 is a leap year)
        QDate::isValid(1202, 6, 6);   // false (1202 is pre-Gregorian)
    \endcode

    \warning A \a y value in the range 00 to 99 is interpreted as
    1900 to 1999.

    \sa isNull(), setYMD()
*/

bool QDate::isValid(int y, int m, int d)
{
    if (y >= 0 && y <= 99)
        y += 1900;
    else if (y < FIRST_YEAR || (y == FIRST_YEAR && (m < 9 || (m == 9 && d < 14))))
        return false;
    return (d > 0 && m > 0 && m <= 12) &&
           (d <= monthDays[m] || (d == 29 && m == 2 && isLeapYear(y)));
}

/*!
    \fn bool QDate::isLeapYear(int year)

    Returns true if the specified \a year is a leap year; otherwise
    returns false.
*/

bool QDate::isLeapYear(int y)
{
    return y % 4 == 0 && y % 100 != 0 || y % 400 == 0;
}

/*!
  \internal
  Converts a Gregorian date to a Julian day.
  This algorithm is taken from Communications of the ACM, Vol 6, No 8.
  \sa julianToGregorian()
*/

uint QDate::gregorianToJulian(int y, int m, int d)
{
    uint c, ya;
    if (y <= 99)
        y += 1900;
    if (m > 2) {
        m -= 3;
    } else {
        m += 9;
        y--;
    }
    c = y;                                        // NOTE: Sym C++ 6.0 bug
    c /= 100;
    ya = y - 100*c;
    return 1721119 + d + (146097*c)/4 + (1461*ya)/4 + (153*m+2)/5;
}

/*!
  \internal
  Converts a Julian day to a Gregorian date.
  This algorithm is taken from Communications of the ACM, Vol 6, No 8.
  \sa gregorianToJulian()
*/

void QDate::julianToGregorian(uint jd, int &y, int &m, int &d)
{
    uint x;
    uint j = jd - 1721119;
    y = (j*4 - 1)/146097;
    j = j*4 - 146097*y - 1;
    x = j/4;
    j = (x*4 + 3) / 1461;
    y = 100*y + j;
    x = (x*4) + 3 - 1461*j;
    x = (x + 4)/4;
    m = (5*x - 3)/153;
    x = 5*x - 3 - 153*m;
    d = (x + 5)/5;
    if (m < 10) {
        m += 3;
    } else {
        m -= 9;
        y++;
    }
}

/*! \fn static QDate QDate::fromJulianDay(int jd)

    Converts the Julian day \a jd to a QDate.

    \sa toJulianDay()
*/

/*! \fn int QDate::toJulianDay() const

    Converts the date to a Julian day.

    \sa fromJulianDay()
*/

/*****************************************************************************
  QTime member functions
 *****************************************************************************/

/*!
    \class QTime
    \reentrant

    \brief The QTime class provides clock time functions.

    \ingroup time
    \mainclass

    A QTime object contains a clock time, i.e. the number of hours,
    minutes, seconds, and milliseconds since midnight. It can read the
    current time from the system clock and measure a span of elapsed
    time. It provides functions for comparing times and for
    manipulating a time by adding a number of (milli)seconds.

    QTime uses the 24-hour clock format; it has no concept of AM/PM.
    It operates in local time; it knows nothing about time zones or
    daylight savings time.

    A QTime object is typically created either by giving the number of
    hours, minutes, seconds, and milliseconds explicitly, or by using
    the static function currentTime(), which creates a QTime object
    that contains the system's clock time. Note that the accuracy
    depends on the accuracy of the underlying operating system; not
    all systems provide 1-millisecond accuracy.

    The hour(), minute(), second(), and msec() functions provide
    access to the number of hours, minutes, seconds, and milliseconds
    of the time. The same information is provided in textual format by
    the toString() function.

    QTime provides a full set of operators to compare two QTime
    objects. One time is considered smaller than another if it is
    earlier than the other.

    The time a given number of seconds or milliseconds later than a
    given time can be found using the addSecs() or addMSecs()
    functions. Correspondingly, the number of (milli)seconds between
    two times can be found using the secsTo() or msecsTo() functions.

    QTime can be used to measure a span of elapsed time using the
    start(), restart(), and elapsed() functions.

    \sa QDate, QDateTime
*/

/*!
    \fn QTime::QTime()

    Constructs the time 0 hours, minutes, seconds and milliseconds,
    i.e. 00:00:00.000 (midnight). This is a valid time.

    \sa isValid()
*/

/*!
    Constructs a time with hour \a h, minute \a m, seconds \a s and
    milliseconds \a ms.

    \a h must be in the range 0 to 23, \a m and \a s must be in the
    range 0 to 59, and \a ms must be in the range 0 to 999.

    \sa isValid()
*/

QTime::QTime(int h, int m, int s, int ms)
{
    setHMS(h, m, s, ms);
}


/*!
    \fn bool QTime::isNull() const

    Returns true if the time is equal to 00:00:00.000; otherwise
    returns false. A null time is valid.

    \sa isValid()
*/

/*!
    Returns true if the time is valid; otherwise returns false. For example,
    the time 23:30:55.746 is valid, but 24:12:30 is invalid.

    \sa isNull()
*/

bool QTime::isValid() const
{
    return mds > NullTime && mds < MSECS_PER_DAY;
}


/*!
    Returns the hour part (0 to 23) of the time.

    \sa minute(), second(), msec()
*/

int QTime::hour() const
{
    return ds() / MSECS_PER_HOUR;
}

/*!
    Returns the minute part (0 to 59) of the time.

    \sa hour(), second(), msec()
*/

int QTime::minute() const
{
    return (ds() % MSECS_PER_HOUR)/MSECS_PER_MIN;
}

/*!
    Returns the second part (0 to 59) of the time.

    \sa hour(), minute(), msec()
*/

int QTime::second() const
{
    return (ds() / 1000)%SECS_PER_MIN;
}

/*!
    Returns the millisecond part (0 to 999) of the time.

    \sa hour(), minute(), second()
*/

int QTime::msec() const
{
    return ds() % 1000;
}

#ifndef QT_NO_DATESTRING
/*!
    \overload

    Returns the time as a string. Milliseconds are not included. The
    \a f parameter determines the format of the string.

    If \a f is Qt::TextDate, the string format is HH:MM:SS; e.g. 1
    second before midnight would be "23:59:59".

    If \a f is Qt::ISODate, the string format corresponds to the
    ISO 8601 extended specification for representations of dates,
    which is also HH:MM:SS.

    If \a f is Qt::LocalDate, the string format depends on the locale
    settings of the system.

    If the datetime is invalid, an empty string will be returned.
*/

QString QTime::toString(Qt::DateFormat f) const
{
    if (!isValid())
        return QString();

    switch (f) {
    case Qt::LocalDate:
        {
#ifdef Q_WS_WIN
            SYSTEMTIME st;
            memset(&st, 0, sizeof(SYSTEMTIME));
            st.wHour = hour();
            st.wMinute = minute();
            st.wSecond = second();
            st.wMilliseconds = 0;
            QT_WA({
                TCHAR buf[255];
                if (GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, 0, buf, 255))
                    return QString::fromUtf16((ushort*)buf);
            } , {
                char buf[255];
                if (GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, 0, (char*)&buf, 255))
                    return QString::fromLocal8Bit(buf);
            });
#elif defined (Q_WS_MAC)
            CFGregorianDate macGDate;
            // Assume this is local time and the current date
            QDate dt = QDate::currentDate();
            macGDate.year = dt.year();
            macGDate.month = dt.month();
            macGDate.day = dt.day();
            macGDate.hour = hour();
            macGDate.minute = minute();
            macGDate.second = second();
            QCFType<CFTimeZoneRef> myTz = CFTimeZoneCopyDefault();
            QCFType<CFDateRef> myDate = CFDateCreate(0,
                                                     CFGregorianDateGetAbsoluteTime(macGDate,
                                                                                    myTz));
#  if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {

                QCFType<CFLocaleRef> mylocale = CFLocaleCopyCurrent();
                QCFType<CFDateFormatterRef> myFormatter = CFDateFormatterCreate(kCFAllocatorDefault,
                                                                       mylocale,
                                                                       kCFDateFormatterNoStyle,
                                                                       kCFDateFormatterMediumStyle);
                return QCFString(CFDateFormatterCreateStringWithDate(0, myFormatter, myDate));
            } else
#  endif
            {
                // For Jaguar, must use the older non-recommended Stuff
                Handle intlHandle = GetIntlResource(0);
                LongDateTime oldDate;
                UCConvertCFAbsoluteTimeToLongDateTime(CFGregorianDateGetAbsoluteTime(macGDate, myTz),
                                                      &oldDate);
                Str255 pString;
                LongTimeString(&oldDate, true, pString, intlHandle);
                return qt_mac_from_pascal_string(pString);
            }
#else
            tm tt;
            memset(&tt, 0, sizeof(tm));
            char buf[255];
            tt.tm_sec = second();
            tt.tm_min = minute();
            tt.tm_hour = hour();

            const QByteArray lctime(setlocale(LC_TIME, ""));
            if (strftime(buf, sizeof(buf), "%X", &tt)) {
                setlocale(LC_TIME, lctime.data());
                return QString::fromLocal8Bit(buf);
            }
            setlocale(LC_TIME, lctime.data());
#endif
            return QString();
        }
    default:
    case Qt::ISODate:
    case Qt::TextDate:
        return QString::fromLatin1("%1:%2:%3")
            .arg(hour(), 2, 10, QLatin1Char('0'))
            .arg(minute(), 2, 10, QLatin1Char('0'))
            .arg(second(), 2, 10, QLatin1Char('0'));
    }
}

/*!
    Returns the time as a string. The \a format parameter determines
    the format of the result string.

    These expressions may be used:

    \table
    \header \i Expression \i Output
    \row \i h
         \i the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)
    \row \i hh
         \i the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
    \row \i H
         \i the hour without a leading zero (0 to 23, even with AM/PM display)
    \row \i HH
         \i the hour with a leading zero (00 to 23, even with AM/PM display)
    \row \i m \i the minute without a leading zero (0 to 59)
    \row \i mm \i the minute with a leading zero (00 to 59)
    \row \i s \i the second whithout a leading zero (0 to 59)
    \row \i ss \i the second whith a leading zero (00 to 59)
    \row \i z \i the milliseconds without leading zeroes (0 to 999)
    \row \i zzz \i the milliseconds with leading zeroes (000 to 999)
    \row \i AP or A
         \i use AM/PM display. \e AP will be replaced by either "AM" or "PM".
    \row \i ap or a
         \i use am/pm display. \e ap will be replaced by either "am" or "pm".
    \endtable

    All other input characters will be ignored. Any sequence of characters that
    are enclosed in singlequotes will be treated as text and not be used as an
    expression.

    Example format strings (assuming that the QTime is 14:13:09.042)

    \table
    \header \i Format \i Result
    \row \i hh:mm:ss.zzz \i 14:13:09.042
    \row \i h:m:s ap     \i 2:13:9 pm
    \row \i H:m:s a      \i 14:13:9 pm
    \endtable

    If the datetime is invalid, an empty string will be returned.

    \sa QDate::toString() QDateTime::toString()
*/
QString QTime::toString(const QString& format) const
{
    return fmtDateTime(format, this, 0);
}
#endif //QT_NO_DATESTRING
/*!
    Sets the time to hour \a h, minute \a m, seconds \a s and
    milliseconds \a ms.

    \a h must be in the range 0 to 23, \a m and \a s must be in the
    range 0 to 59, and \a ms must be in the range 0 to 999.
    Returns true if the set time is valid; otherwise returns false.

    \sa isValid()
*/

bool QTime::setHMS(int h, int m, int s, int ms)
{
    if (!isValid(h,m,s,ms)) {
        mds = NullTime;                // make this invalid
        return false;
    }
    mds = (h*SECS_PER_HOUR + m*SECS_PER_MIN + s)*1000 + ms;
    return true;
}

/*!
    Returns a QTime object containing a time \a nsecs seconds later
    than the time of this object (or earlier if \a nsecs is negative).

    Note that the time will wrap if it passes midnight.

    Example:

    \code
        QTime n(14, 0, 0);                // n == 14:00:00
        QTime t;
        t = n.addSecs(70);                // t == 14:01:10
        t = n.addSecs(-70);               // t == 13:58:50
        t = n.addSecs(10 * 60 * 60 + 5);  // t == 00:00:05
        t = n.addSecs(-15 * 60 * 60);     // t == 23:00:00
    \endcode

    \sa addMSecs(), secsTo(), QDateTime::addSecs()
*/

QTime QTime::addSecs(int nsecs) const
{
    return addMSecs(nsecs * 1000);
}

/*!
    Returns the number of seconds from this time to \a t.
    If \a t is earlier than this time, the number of seconds returned
    is negative.

    Because QTime measures time within a day and there are 86400
    seconds in a day, the result is always between -86400 and 86400.

    \sa addSecs(), QDateTime::secsTo()
*/

int QTime::secsTo(const QTime &t) const
{
    return (t.ds() - ds()) / 1000;
}

/*!
    Returns a QTime object containing a time \a ms milliseconds later
    than the time of this object (or earlier if \a ms is negative).

    Note that the time will wrap if it passes midnight. See addSecs()
    for an example.

    \sa addSecs(), msecsTo()
*/

QTime QTime::addMSecs(int ms) const
{
    QTime t;
    if (ms < 0) {
        // % not well-defined for -ve, but / is.
        int negdays = (MSECS_PER_DAY-ms) / MSECS_PER_DAY;
        t.mds = (ds() + ms + negdays*MSECS_PER_DAY)
                % MSECS_PER_DAY;
    } else {
        t.mds = (ds() + ms) % MSECS_PER_DAY;
    }
    return t;
}

/*!
    Returns the number of milliseconds from this time to \a t.
    If \a t is earlier than this time, the number of milliseconds returned
    is negative.

    Because QTime measures time within a day and there are 86400
    seconds in a day, the result is always between -86400000 and
    86400000 msec.

    \sa secsTo(), addMSecs()
*/

int QTime::msecsTo(const QTime &t) const
{
    return t.ds() - ds();
}


/*!
    \fn bool QTime::operator==(const QTime &t) const

    Returns true if this time is equal to \a t; otherwise returns false.
*/

/*!
    \fn bool QTime::operator!=(const QTime &t) const

    Returns true if this time is different from \a t; otherwise returns false.
*/

/*!
    \fn bool QTime::operator<(const QTime &t) const

    Returns true if this time is earlier than \a t; otherwise returns false.
*/

/*!
    \fn bool QTime::operator<=(const QTime &t) const

    Returns true if this time is earlier than or equal to \a t;
    otherwise returns false.
*/

/*!
    \fn bool QTime::operator>(const QTime &t) const

    Returns true if this time is later than \a t; otherwise returns false.
*/

/*!
    \fn bool QTime::operator>=(const QTime &t) const

    Returns true if this time is later than or equal to \a t;
    otherwise returns false.
*/

/*!
    \overload

    Returns the current time as reported by the system clock.

    Note that the accuracy depends on the accuracy of the underlying
    operating system; not all systems provide 1-millisecond accuracy.
*/

QTime QTime::currentTime()
{
    QTime ct;

#if defined(Q_OS_WIN32)
    SYSTEMTIME st;
    memset(&st, 0, sizeof(SYSTEMTIME));
    GetLocalTime(&st);
    ct.mds = MSECS_PER_HOUR * st.wHour + MSECS_PER_MIN * st.wMinute + 1000 * st.wSecond
             + st.wMilliseconds;
#elif defined(Q_OS_UNIX)
    // posix compliant system
    struct timeval tv;
    gettimeofday(&tv, 0);
    time_t ltime = tv.tv_sec;
    tm *t;

#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant version of localtime() where available
    tm res;
    t = localtime_r(&ltime, &res);
#else
    t = localtime(&ltime);
#endif

    ct.mds = MSECS_PER_HOUR * t->tm_hour + MSECS_PER_MIN * t->tm_min + 1000 * t->tm_sec
             + tv.tv_usec / 1000;
#else
    time_t ltime; // no millisecond resolution
    ::time(&ltime);
    tm *t;
    localtime(&ltime);
    ct.mds = MSECS_PER_HOUR * t->tm_hour + MSECS_PER_MIN * t->tm_min + 1000 * t->tm_sec;
#endif
    return ct;
}

#ifndef QT_NO_DATESTRING
/*!
    \fn QTime QTime::fromString(const QString &string, Qt::DateFormat format)

    Returns the time represented in the \a string as a QTime using the
    \a format given, or an invalid time if this is not possible.

    \warning Note that Qt::LocalDate cannot be used here.
*/
QTime QTime::fromString(const QString& s, Qt::DateFormat f)
{
    if (s.isEmpty() || f == Qt::LocalDate) {
        qWarning("QTime::fromString: Parameter out of range");
        QTime t;
        t.mds = NullTime;
        return t;
    }

    int hour(s.mid(0, 2).toInt());
    int minute(s.mid(3, 2).toInt());
    int second(s.mid(6, 2).toInt());

    QString msec_s(QLatin1String("0.") + s.mid(9, 4));
    float msec(msec_s.toFloat());
    return QTime(hour, minute, second, qRound(msec * 1000.0));
}

/*!
    \fn QTime::fromString(const QString &string, const QString &format)

    Returns the QTime represented by the \a string, using the \a
    format given, or an invalid time if the string cannot be parsed.

    These expressions may be used for the format:

    \table
    \header \i Expression \i Output
    \row \i h
         \i the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)
    \row \i hh
         \i the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
    \row \i m \i the minute without a leading zero (0 to 59)
    \row \i mm \i the minute with a leading zero (00 to 59)
    \row \i s \i the second whithout a leading zero (0 to 59)
    \row \i ss \i the second whith a leading zero (00 to 59)
    \row \i z \i the milliseconds without leading zeroes (0 to 999)
    \row \i zzz \i the milliseconds with leading zeroes (000 to 999)
    \row \i AP
         \i interpret as an AM/PM time. \e AP must be either "AM" or "PM".
    \row \i ap
         \i Interpret as an AM/PM time. \e ap must be either "am" or "pm".
    \endtable

    All other input characters will be treated as text. Any sequence
    of characters that are enclosed in single quotes will also be
    treated as text and not be used as an expression.

    \code
        QTime time = QTime::fromString("1mm12car00", "m'mm'hcarss");
        // time is 12:01.00
    \endcode

    If the format is not satisfied an invalid QTime is returned.
    Expressions that do not expect leading zeroes to be given (h, m, s
    and z) are greedy. This means that they will use two digits even if
    this puts them outside the range of accepted values and leaves too
    few digits for other sections. For example, the following string
    could have meant 00:07:10, but the m will grab two digits, resulting
    in an invalid time:

    \code
        QTime time = QTime::fromString("00:710", "hh:ms"); // invalid
    \endcode

    Any field that is not represented in the format will be set to zero.
    For example:

    \code
        QTime time = QTime::fromString("1.30", "m.s");
        // time is 00:01:30.000
    \endcode

    \sa QDateTime::fromString() QDate::fromString() QDate::toString()
    QDateTime::toString() QTime::toString()
*/

QTime QTime::fromString(const QString &string, const QString &format)
{
    QTime time;
#ifndef QT_BOOTSTRAPPED
    QDateTimeParser dt(QVariant::Time);
    if (dt.parseFormat(format))
        dt.fromString(string, 0, &time);
#else
    Q_UNUSED(string);
    Q_UNUSED(format);
#endif
    return time;
}

#endif // QT_NO_DATESTRING


/*!
    \overload

    Returns true if the specified time is valid; otherwise returns
    false.

    The time is valid if \a h is in the range 0 to 23, \a m and
    \a s are in the range 0 to 59, and \a ms is in the range 0 to 999.

    Example:

    \code
        QTime::isValid(21, 10, 30); // returns true
        QTime::isValid(22, 5,  62); // returns false
    \endcode
*/

bool QTime::isValid(int h, int m, int s, int ms)
{
    return (uint)h < 24 && (uint)m < 60 && (uint)s < 60 && (uint)ms < 1000;
}


/*!
    Sets this time to the current time. This is practical for timing:

    \code
        QTime t;
        t.start();
        some_lengthy_task();
        qDebug("Time elapsed: %d ms", t.elapsed());
    \endcode

    \sa restart(), elapsed(), currentTime()
*/

void QTime::start()
{
    *this = currentTime();
}

/*!
    Sets this time to the current time and returns the number of
    milliseconds that have elapsed since the last time start() or
    restart() was called.

    This function is guaranteed to be atomic and is thus very handy
    for repeated measurements. Call start() to start the first
    measurement, and restart() for each later measurement.

    Note that the counter wraps to zero 24 hours after the last call
    to start() or restart().

    \warning If the system's clock setting has been changed since the
    last time start() or restart() was called, the result is
    undefined. This can happen when daylight savings time is turned on
    or off.

    \sa start(), elapsed(), currentTime()
*/

int QTime::restart()
{
    QTime t = currentTime();
    int n = msecsTo(t);
    if (n < 0)                                // passed midnight
        n += 86400*1000;
    *this = t;
    return n;
}

/*!
    Returns the number of milliseconds that have elapsed since the
    last time start() or restart() was called.

    Note that the counter wraps to zero 24 hours after the last call
    to start() or restart.

    Note that the accuracy depends on the accuracy of the underlying
    operating system; not all systems provide 1-millisecond accuracy.

    \warning If the system's clock setting has been changed since the
    last time start() or restart() was called, the result is
    undefined. This can happen when daylight savings time is turned on
    or off.

    \sa start(), restart()
*/

int QTime::elapsed() const
{
    int n = msecsTo(currentTime());
    if (n < 0)                                // passed midnight
        n += 86400 * 1000;
    return n;
}


/*****************************************************************************
  QDateTime member functions
 *****************************************************************************/

/*!
    \class QDateTime
    \reentrant
    \brief The QDateTime class provides date and time functions.

    \ingroup time
    \mainclass

    A QDateTime object contains a calendar date and a clock time (a
    "datetime"). It is a combination of the QDate and QTime classes.
    It can read the current datetime from the system clock. It
    provides functions for comparing datetimes and for manipulating a
    datetime by adding a number of seconds, days, months, or years.

    A QDateTime object is typically created either by giving a date
    and time explicitly in the constructor, or by using the static
    function currentDateTime() that returns a QDateTime object set
    to the system clock's time. The date and time can be changed with
    setDate() and setTime(). A datetime can also be set using the
    setTime_t() function that takes a POSIX-standard "number of
    seconds since 00:00:00 on January 1, 1970" value. The fromString()
    function returns a QDateTime, given a string and a date format
    used to interpret the date within the string.

    The date() and time() functions provide access to the date and
    time parts of the datetime. The same information is provided in
    textual format by the toString() function.

    QDateTime provides a full set of operators to compare two
    QDateTime objects where smaller means earlier and larger means
    later.

    You can increment (or decrement) a datetime by a given number of
    seconds using addSecs(), or days using addDays(). Similarly you can
    use addMonths() and addYears(). The daysTo() function returns the
    number of days between two datetimes, and secsTo() returns the
    number of seconds between two datetimes.

    QDateTime can store datetimes as \l{Qt::LocalTime}{local time} or
    as \l{Qt::UTC}{UTC}. QDateTime::currentDateTime() returns a
    QDateTime expressed as local time; use toUTC() to convert it to
    UTC. You can also use timeSpec() to find out if a QDateTime
    object stores a UTC time or a local time. Operations such as
    addSecs() and secsTo() are aware of daylight saving time (DST).

    \sa QDate QTime QDateTimeEdit
*/

/*!
    Constructs a null datetime (i.e. null date and null time). A null
    datetime is invalid, since the date is invalid.

    \sa isValid()
*/
QDateTime::QDateTime()
{
    d = new QDateTimePrivate;
}


/*!
    Constructs a datetime with the given \a date, and a valid
    time (00:00:00.000).
*/

QDateTime::QDateTime(const QDate &date)
{
    d = new QDateTimePrivate;
    d->date = date;
    d->time = QTime(0, 0, 0);
}

/*!
    Constructs a datetime with the given \a date and \a time, using
    the time specification defined by \a spec.

    If \a date is valid and \a time is not, the time will be set to midnight.
*/

QDateTime::QDateTime(const QDate &date, const QTime &time, Qt::TimeSpec spec)
{
    d = new QDateTimePrivate;
    d->date = date;
    d->time = date.isValid() && !time.isValid() ? QTime(0, 0, 0) : time;
    d->spec = (spec == Qt::UTC) ? QDateTimePrivate::UTC : QDateTimePrivate::LocalUnknown;
}

/*!
    Constructs a copy of the \a other datetime.
*/

QDateTime::QDateTime(const QDateTime &other)
{
    d = other.d;
    d->ref.ref();
}

/*!
    Destroys the datetime.
*/
QDateTime::~QDateTime()
{
    if (!d->ref.deref())
        delete d;
}

/*!
    Makes a copy of the \a other datetime and returns a reference to the
    copy.
*/

QDateTime &QDateTime::operator=(const QDateTime &other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    Returns true if both the date and the time are null; otherwise
    returns false. A null datetime is invalid.

    \sa QDate::isNull(), QTime::isNull(), isValid()
*/

bool QDateTime::isNull() const
{
    return d->date.isNull() && d->time.isNull();
}

/*!
    Returns true if both the date and the time are valid; otherwise
    returns false.

    \sa QDate::isValid(), QTime::isValid()
*/

bool QDateTime::isValid() const
{
    return d->date.isValid() && d->time.isValid();
}

/*!
    Returns the date part of the datetime.

    \sa setDate(), time(), timeSpec()
*/

QDate QDateTime::date() const
{
    return d->date;
}

/*!
    Returns the time part of the datetime.

    \sa setTime(), date(), timeSpec()
*/

QTime QDateTime::time() const
{
    return d->time;
}

/*!
    Returns the time specification of the datetime.

    \sa setTimeSpec(), date(), time(), Qt::TimeSpec
*/

Qt::TimeSpec QDateTime::timeSpec() const
{
    return d->spec == QDateTimePrivate::UTC ? Qt::UTC : Qt::LocalTime;
}

/*!
    Sets the date part of this datetime to \a date.
    If no time is set, it is set to midnight.

    \sa date(), setTime(), setTimeSpec()
*/

void QDateTime::setDate(const QDate &date)
{
    detach();
    d->date = date;
    if (date.isValid() && !d->time.isValid())
        d->time = QTime(0, 0, 0);
}

/*!
    Sets the time part of this datetime to \a time.

    \sa time(), setDate(), setTimeSpec()
*/

void QDateTime::setTime(const QTime &time)
{
    detach();
    d->time = time;
}

/*!
    Sets the time specification used in this datetime to \a spec.

    \sa timeSpec(), setDate(), setTime(), Qt::TimeSpec
*/

void QDateTime::setTimeSpec(Qt::TimeSpec spec)
{
    detach();
    d->spec = (spec == Qt::UTC) ? QDateTimePrivate::UTC : QDateTimePrivate::LocalUnknown;
}

static uint toTime_t(const QDate &utcDate, const QTime &utcTime)
{
    return (QDate(1970, 1, 1).daysTo(utcDate) * SECS_PER_DAY) + QTime().secsTo(utcTime);
}

/*!
    Returns the datetime as the number of seconds that have passed
    since 1970-01-01T00:00:00, Coordinated Universal Time (Qt::UTC).

    On systems that do not support timezones, this function will
    behave as if local time were Qt::UTC.

    \sa setTime_t()
*/

uint QDateTime::toTime_t() const
{
    QDate utcDate;
    QTime utcTime;
    d->getUTC(utcDate, utcTime);

    int secsSince1Jan1970UTC = ::toTime_t(utcDate, utcTime);
    if (secsSince1Jan1970UTC < 0)
        return (uint)-1;
    return (uint)secsSince1Jan1970UTC;
}

/*!
    \fn void QDateTime::setTime_t(uint seconds)

    Sets the date and time given the number of \a seconds that have
    passed since 1970-01-01T00:00:00, Coordinated Universal Time
    (Qt::UTC). On systems that do not support timezones this function
    will behave as if local time were Qt::UTC.

    \sa toTime_t()
*/

void QDateTime::setTime_t(uint secsSince1Jan1970UTC)
{
    detach();

    QDateTimePrivate::Spec oldSpec = d->spec;

    d->date = QDate(1970, 1, 1).addDays(secsSince1Jan1970UTC / SECS_PER_DAY);
    d->time = QTime().addSecs(secsSince1Jan1970UTC % SECS_PER_DAY);
    d->spec = QDateTimePrivate::UTC;

    if (oldSpec != QDateTimePrivate::UTC)
        d->spec = d->getLocal(d->date, d->time);
}

#ifndef QT_NO_DATESTRING
/*!
    \fn QString QDateTime::toString(Qt::DateFormat format) const

    \overload

    Returns the datetime as a string in the \a format given.

    If the \a format is Qt::TextDate, the string is formatted in
    the default way. QDate::shortDayName(), QDate::shortMonthName(),
    and QTime::toString() are used to generate the string, so the
    day and month names will be localized names. An example of this
    formatting is "Wed May 20 03:40:13 1998".

    If the \a format is Qt::ISODate, the string format corresponds
    to the ISO 8601 extended specification for representations of
    dates and times, taking the form YYYY-MM-DDTHH:MM:SS.

    If the \a format is Qt::LocalDate, the string format depends
    on the locale settings of the system.

    If the datetime is invalid, an empty string will be returned.

    \sa QDate::toString() QTime::toString() Qt::DateFormat
*/

QString QDateTime::toString(Qt::DateFormat f) const
{
    QString buf;
    if (!isValid())
        return buf;

    if (f == Qt::ISODate) {
        buf = d->date.toString(Qt::ISODate);
        buf += QLatin1Char('T');
        buf += d->time.toString(Qt::ISODate);
    }
#ifndef QT_NO_TEXTDATE
    else if (f == Qt::TextDate) {
#ifndef Q_WS_WIN
        buf = d->date.shortDayName(d->date.dayOfWeek());
        buf += QLatin1Char(' ');
        buf += d->date.shortMonthName(d->date.month());
        buf += QLatin1Char(' ');
        buf += QString::number(d->date.day());
#else
        QString winstr;
        QT_WA({
            TCHAR out[255];
            GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_ILDATE, out, 255);
            winstr = QString::fromUtf16((ushort*)out);
        } , {
            char out[255];
            GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_ILDATE, (char*)&out, 255);
            winstr = QString::fromLocal8Bit(out);
        });
        switch (winstr.toInt()) {
        case 1:
            buf = d->date.shortDayName(d->date.dayOfWeek());
            buf += QLatin1Char(' ');
            buf += QString::number(d->date.day());
            buf += QLatin1String(". ");
            buf += d->date.shortMonthName(d->date.month());
            break;
        default:
            buf = d->date.shortDayName(d->date.dayOfWeek());
            buf += QLatin1Char(' ');
            buf += d->date.shortMonthName(d->date.month());
            buf += QLatin1Char(' ');
            buf += QString::number(d->date.day());
        }
#endif
        buf += QLatin1Char(' ');
        buf += d->time.toString();
        buf += QLatin1Char(' ');
        buf += QString::number(d->date.year());
    }
#endif
    else if (f == Qt::LocalDate) {
        buf = d->date.toString(Qt::LocalDate);
        buf += QLatin1Char(' ');
        buf += d->time.toString(Qt::LocalDate);
    }
    return buf;
}

/*!
    Returns the datetime as a string. The \a format parameter
    determines the format of the result string.

    These expressions may be used for the date:

    \table
    \header \i Expression \i Output
    \row \i d \i the day as number without a leading zero (1 to 31)
    \row \i dd \i the day as number with a leading zero (01 to 31)
    \row \i ddd
            \i the abbreviated localized day name (e.g. 'Mon' to 'Sun').
            Uses QDate::shortDayName().
    \row \i dddd
            \i the long localized day name (e.g. 'Qt::Monday' to 'Qt::Sunday').
            Uses QDate::longDayName().
    \row \i M \i the month as number without a leading zero (1-12)
    \row \i MM \i the month as number with a leading zero (01-12)
    \row \i MMM
            \i the abbreviated localized month name (e.g. 'Jan' to 'Dec').
            Uses QDate::shortMonthName().
    \row \i MMMM
            \i the long localized month name (e.g. 'January' to 'December').
            Uses QDate::longMonthName().
    \row \i yy \i the year as two digit number (00-99)
    \row \i yyyy \i the year as four digit number (1752-8000)
    \endtable

    These expressions may be used for the time:

    \table
    \header \i Expression \i Output
    \row \i h
         \i the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)
    \row \i hh
         \i the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
    \row \i m \i the minute without a leading zero (0 to 59)
    \row \i mm \i the minute with a leading zero (00 to 59)
    \row \i s \i the second whithout a leading zero (0 to 59)
    \row \i ss \i the second whith a leading zero (00 to 59)
    \row \i z \i the milliseconds without leading zeroes (0 to 999)
    \row \i zzz \i the milliseconds with leading zeroes (000 to 999)
    \row \i AP
            \i use AM/PM display. \e AP will be replaced by either "AM" or "PM".
    \row \i ap
            \i use am/pm display. \e ap will be replaced by either "am" or "pm".
    \endtable

    All other input characters will be ignored. Any sequence of characters that
    are enclosed in singlequotes will be treated as text and not be used as an
    expression.

    Example format strings (assumed that the QDateTime is 21 May 2001
    14:13:09):

    \table
    \header \i Format       \i Result
    \row \i dd.MM.yyyy      \i 21.05.2001
    \row \i ddd MMMM d yy   \i Tue May 21 01
    \row \i hh:mm:ss.zzz    \i 14:13:09.042
    \row \i h:m:s ap        \i 2:13:9 pm
    \endtable

    If the datetime is invalid, an empty string will be returned.

    \sa QDate::toString() QTime::toString()
*/
QString QDateTime::toString(const QString& format) const
{
    return fmtDateTime(format, &d->time, &d->date);
}
#endif //QT_NO_DATESTRING

/*!
    Returns a QDateTime object containing a datetime \a ndays days
    later than the datetime of this object (or earlier if \a ndays is
    negative).

    \sa daysTo(), addMonths(), addYears(), addSecs()
*/

QDateTime QDateTime::addDays(int ndays) const
{
    return QDateTime(d->date.addDays(ndays), d->time, timeSpec());
}

/*!
    Returns a QDateTime object containing a datetime \a nmonths months
    later than the datetime of this object (or earlier if \a nmonths
    is negative).

    \sa daysTo(), addDays(), addYears(), addSecs()
*/

QDateTime QDateTime::addMonths(int nmonths) const
{
    return QDateTime(d->date.addMonths(nmonths), d->time, timeSpec());
}

/*!
    Returns a QDateTime object containing a datetime \a nyears years
    later than the datetime of this object (or earlier if \a nyears is
    negative).

    \sa daysTo(), addDays(), addMonths(), addSecs()
*/

QDateTime QDateTime::addYears(int nyears) const
{
    return QDateTime(d->date.addYears(nyears), d->time, timeSpec());
}


QDateTime QDateTimePrivate::addMSecs(const QDateTime &dt, qint64 msecs)
{
    QDate utcDate;
    QTime utcTime;
    dt.d->getUTC(utcDate, utcTime);

    uint dd = utcDate.jd;
    int tt = utcTime.ds();
    int sign = 1;
    if (msecs < 0) {
        msecs = -msecs;
        sign = -1;
    }
    if (msecs >= int(MSECS_PER_DAY)) {
        dd += sign * (msecs / MSECS_PER_DAY);
        msecs %= MSECS_PER_DAY;
    }

    tt += sign * msecs;
    if (tt < 0) {
        tt = MSECS_PER_DAY - tt - 1;
        dd -= tt / MSECS_PER_DAY;
        tt = tt % MSECS_PER_DAY;
        tt = MSECS_PER_DAY - tt - 1;
    } else if (tt >= int(MSECS_PER_DAY)) {
        dd += tt / MSECS_PER_DAY;
        tt = tt % MSECS_PER_DAY;
    }

    utcDate.jd = dd;
    utcTime.mds = tt;
    return QDateTime(utcDate, utcTime, Qt::UTC).toTimeSpec(dt.timeSpec());
}

/*!
    Returns a QDateTime object containing a datetime \a nsecs seconds
    later than the datetime of this object (or earlier if \a nsecs is
    negative).

    \sa addMSecs(), secsTo(), addDays(), addMonths(), addYears()
*/

QDateTime QDateTime::addSecs(int nsecs) const
{
    return d->addMSecs(*this, qint64(nsecs) * 1000);
}

/*!
    Returns a QDateTime object containing a datetime \a msecs miliseconds
    later than the datetime of this object (or earlier if \a msecs is
    negative).

    \sa addSecs(), secsTo(), addDays(), addMonths(), addYears()
*/
QDateTime QDateTime::addMSecs(qint64 msecs) const
{
    return d->addMSecs(*this, msecs);
}

/*!
    Returns the number of days from this datetime to the \a other
    datetime. If the \a other datetime is earlier than this datetime,
    the value returned is negative.

    \sa addDays(), secsTo()
*/

int QDateTime::daysTo(const QDateTime &other) const
{
    return d->date.daysTo(other.d->date);
}

/*!
    Returns the number of seconds from this datetime to the \a other
    datetime. If the \a other datetime is earlier than this datetime,
    the value returned is negative.

    Before performing the comparison, the two datetimes are converted
    to Qt::UTC to ensure that the result is correct if one of the two
    datetimes has daylight saving time (DST) and the other doesn't.

    Example:
    \code
        QDateTime now = QDateTime::currentDateTime();
        QDateTime xmas(QDate(now.date().year(), 12, 25), QTime(0, 0));
        qDebug("There are %d seconds to Christmas", now.secsTo(xmas));
    \endcode

    \sa addSecs(), daysTo(), QTime::secsTo()
*/

int QDateTime::secsTo(const QDateTime &other) const
{
    QDate date1, date2;
    QTime time1, time2;

    d->getUTC(date1, time1);
    other.d->getUTC(date2, time2);

    return (date1.daysTo(date2) * SECS_PER_DAY) + time1.secsTo(time2);
}

/*!
    \fn QDateTime QDateTime::toTimeSpec(Qt::TimeSpec specification) const

    Returns a copy of this datetime configured to use the given time
    \a specification.

    \sa timeSpec(), toUTC(), toLocalTime()
*/

QDateTime QDateTime::toTimeSpec(Qt::TimeSpec spec) const
{
    if ((d->spec == QDateTimePrivate::UTC) == (spec == Qt::UTC))
        return *this;

    QDateTime ret;
    if (spec == Qt::UTC) {
        d->getUTC(ret.d->date, ret.d->time);
        ret.d->spec = QDateTimePrivate::UTC;
    } else {
        ret.d->spec = d->getLocal(ret.d->date, ret.d->time);
    }
    return ret;
}

/*!
    Returns true if this datetime is equal to the \a other datetime;
    otherwise returns false.

    \sa operator!=()
*/

bool QDateTime::operator==(const QDateTime &other) const
{
    if (d->spec != other.d->spec) {
        if (d->spec == QDateTimePrivate::UTC || other.d->spec == QDateTimePrivate::UTC)
            return false;
        if (d->spec != QDateTimePrivate::LocalUnknown
                && other.d->spec != QDateTimePrivate::LocalUnknown)
            return false;

        QDate date1, date2;
        QTime time1, time2;
        d->getUTC(date1, time1);
        other.d->getUTC(date2, time2);
        return time1 == time2 && date1 == date2;
    } else {
        return d->time == other.d->time && d->date == other.d->date;
    }
}

/*!
    \fn bool QDateTime::operator!=(const QDateTime &other) const

    Returns true if this datetime is different from the \a other
    datetime; otherwise returns false.

    Two datetimes are different if either the date, the time, or the
    time zone components are different.

    \sa operator==()
*/

/*!
    Returns true if this datetime is earlier than the \a other
    datetime; otherwise returns false.
*/

bool QDateTime::operator<(const QDateTime &other) const
{
    if (d->spec == other.d->spec) {
        if (d->date != other.d->date)
            return d->date < other.d->date;
        return d->time < other.d->time;
    } else {
        QDate date1, date2;
        QTime time1, time2;
        d->getUTC(date1, time1);
        other.d->getUTC(date2, time2);
        if (date1 != date2)
            return date1 < date2;
        return time1 < time2;
    }
}

/*!
    \fn bool QDateTime::operator<=(const QDateTime &other) const

    Returns true if this datetime is earlier than or equal to the
    \a other datetime; otherwise returns false.
*/

/*!
    \fn bool QDateTime::operator>(const QDateTime &other) const

    Returns true if this datetime is later than the \a other datetime;
    otherwise returns false.
*/

/*!
    \fn bool QDateTime::operator>=(const QDateTime &other) const

    Returns true if this datetime is later than or equal to the
    \a other datetime; otherwise returns false.
*/

/*!
    Returns the current datetime, as reported by the system clock, in
    the local time zone.

    \sa QDate::currentDate(), QTime::currentTime(), toTimeSpec()
*/

QDateTime QDateTime::currentDateTime()
{
#if defined(Q_OS_WIN32)
    QDate d;
    QTime t;
    SYSTEMTIME st;
    memset(&st, 0, sizeof(SYSTEMTIME));
    GetLocalTime(&st);
    d.jd = QDate::gregorianToJulian(st.wYear, st.wMonth, st.wDay);
    t.mds = MSECS_PER_HOUR * st.wHour + MSECS_PER_MIN * st.wMinute + 1000 * st.wSecond
            + st.wMilliseconds;
    return QDateTime(d, t);
#else
    QDateTime dt;
    QTime t;
    dt.setDate(QDate::currentDate());
    t = QTime::currentTime();
    if (t.ds() < MSECS_PER_MIN)                // midnight or right after?
        dt.setDate(QDate::currentDate());          // fetch date again
    dt.setTime(t);
    return dt;
#endif
}

#ifndef QT_NO_DATESTRING
/*!
    \fn QDateTime QDateTime::fromString(const QString &string, Qt::DateFormat format)

    Returns the QDateTime represented by the \a string, using the
    \a format given, or an invalid datetime if this is not possible.

    Note for Qt::TextDate: It is recommended that you use the
    English short month names (e.g. "Jan"). Although localized month
    names can also be used, they depend on the user's locale settings.

    \warning Note that Qt::LocalDate cannot be used here.
*/
QDateTime QDateTime::fromString(const QString& s, Qt::DateFormat f)
{
    if (s.isEmpty() || f == Qt::LocalDate) {
        qWarning("QDateTime::fromString: Parameter out of range");
        return QDateTime();
    }
    if (f == Qt::ISODate) {
        QString tmp = s;
        Qt::TimeSpec ts = Qt::LocalTime;

        // Recognize UTC specifications
        if (tmp.endsWith(QLatin1Char('Z'))) {
            ts = Qt::UTC;
            tmp.chop(1);
        }
        return QDateTime(QDate::fromString(tmp.mid(0, 10), Qt::ISODate),
                         QTime::fromString(tmp.mid(11), Qt::ISODate), ts);
    }
#if !defined(QT_NO_REGEXP) && !defined(QT_NO_TEXTDATE)
    else if (f == Qt::TextDate) {
        QString monthName(s.mid(4, 3));
        int month = -1;
        // Assume that English monthnames are the default
        for (int i = 0; i < 12; ++i) {
            if (monthName == QLatin1String(qt_shortMonthNames[i])) {
                month = i + 1;
                break;
            }
        }
        // If English names can't be found, search the localized ones
        if (month == -1) {
            for (int i = 1; i <= 12; ++i) {
                if (monthName == QDate::shortMonthName(i)) {
                    month = i;
                    break;
                }
            }
        }
        if (month < 1 || month > 12) {
            qWarning("QDateTime::fromString: Parameter out of range");
            return QDateTime();
        }
        int day = s.mid(8, 2).simplified().toInt();
        int year = s.right(4).toInt();
        QDate date(year, month, day);
        QTime time;
        int hour, minute, second;
        int pivot = s.indexOf(QRegExp(QString::fromLatin1("[0-9][0-9]:[0-9][0-9]:[0-9][0-9]")));
        if (pivot != -1) {
            hour = s.mid(pivot, 2).toInt();
            minute = s.mid(pivot + 3, 2).toInt();
            second = s.mid(pivot + 6, 2).toInt();
            time.setHMS(hour, minute, second);
        }
        return QDateTime(date, time);
    }
#endif //QT_NO_REGEXP
    return QDateTime();
}

/*!
    \fn QDateTime::fromString(const QString &string, const QString &format)

    Returns the QDateTime represented by the \a string, using the \a
    format given, or an invalid datetime if the string cannot be parsed.

    These expressions may be used for the date part of the format string:

    \table
    \header \i Expression \i Output
    \row \i d \i the day as number without a leading zero (1 to 31)
    \row \i dd \i the day as number with a leading zero (01 to 31)
    \row \i ddd
            \i the abbreviated localized day name (e.g. 'Mon' to 'Sun').
            Uses QDate::shortDayName().
    \row \i dddd
            \i the long localized day name (e.g. 'Qt::Monday' to 'Qt::Sunday').
            Uses QDate::longDayName().
    \row \i M \i the month as number without a leading zero (1-12)
    \row \i MM \i the month as number with a leading zero (01-12)
    \row \i MMM
            \i the abbreviated localized month name (e.g. 'Jan' to 'Dec').
            Uses QDate::shortMonthName().
    \row \i MMMM
            \i the long localized month name (e.g. 'January' to 'December').
            Uses QDate::longMonthName().
    \row \i yy \i the year as two digit number (00-99)
    \row \i yyyy \i the year as four digit number (1752-8000)
    \endtable

    These expressions may be used for the time part of the format string:

    \table
    \header \i Expression \i Output
    \row \i h
            \i the hour without a leading zero (0 to 23 or 1 to 12 if AM/PM display)
    \row \i hh
            \i the hour with a leading zero (00 to 23 or 01 to 12 if AM/PM display)
    \row \i H
            \i the hour without a leading zero (0 to 23, even with AM/PM display)
    \row \i HH
            \i the hour with a leading zero (00 to 23, even with AM/PM display)
    \row \i m \i the minute without a leading zero (0 to 59)
    \row \i mm \i the minute with a leading zero (00 to 59)
    \row \i s \i the second whithout a leading zero (0 to 59)
    \row \i ss \i the second whith a leading zero (00 to 59)
    \row \i z \i the milliseconds without leading zeroes (0 to 999)
    \row \i zzz \i the milliseconds with leading zeroes (000 to 999)
    \row \i AP or A
         \i interpret as an AM/PM time. \e AP must be either "AM" or "PM".
    \row \i ap or a
         \i Interpret as an AM/PM time. \e ap must be either "am" or "pm".
    \endtable

    All other input characters will be treated as text. Any sequence
    of characters that are enclosed in singlequotes will also be
    treated as text and not be used as an expression.

    \code
        QTime time1 = QTime::fromString("131", "HHh");
        // time1 is 13:00:00
        QTime time1 = QTime::fromString("1apA", "1amAM");
        // time1 is 01:00:00

        QDateTime dateTime2 = QDateTime::fromString("M1d1y9800:01:02",
                                                    "'M'M'd'd'y'yyhh:mm:ss");
        // dateTime is 1 January 1998 00:01:02
    \endcode

    If the format is not satisfied an invalid QDateTime is returned.
    The expressions that don't have leading zeroes (d, M, h, m, s, z) will be
    greedy. This means that they will use two digits even if this will
    put them outside the range and/or leave too few digits for other
    sections.

    \code
        QDateTime dateTime = QDateTime::fromString("130", "Mm"); // invalid
    \endcode

    This could have meant 1 January 00:30.00 but the M will grab
    two digits.

    For any field that is not represented in the format the following
    defaults are used:

    \table
    \header \i Field  \i Default value
    \row    \i Year   \i 1900
    \row    \i Month  \i 1 (January)
    \row    \i Day    \i 1
    \row    \i Hour   \i 0
    \row    \i Minute \i 0
    \row    \i Second \i 0
    \endtable

    For example:

    \code
        QDateTime dateTime = QDateTime::fromString("1.30.1", "M.d.s");
        // dateTime is January 30 in the current year 00:00:01
    \endcode

    \sa QDate::fromString() QTime::fromString() QDate::toString()
    QDateTime::toString() QTime::toString()
*/

QDateTime QDateTime::fromString(const QString &string, const QString &format)
{
#ifndef QT_BOOTSTRAPPED
    QTime time;
    QDate date;

    QDateTimeParser dt(QVariant::DateTime);
    if (dt.parseFormat(format) && dt.fromString(string, &date, &time))
        return QDateTime(date, time);
#else
    Q_UNUSED(string);
    Q_UNUSED(format);
#endif
    return QDateTime(QDate(), QTime(-1, -1, -1));
}

#endif // QT_NO_DATESTRING
/*!
    \fn QDateTime QDateTime::toLocalTime() const

    Returns a datetime containing the date and time information in
    this datetime, but specified using the Qt::LocalTime definition.

    \sa toTimeSpec()
*/

/*!
    \fn QDateTime QDateTime::toUTC() const

    Returns a datetime containing the date and time information in
    this datetime, but specified using the Qt::UTC definition.

    \sa toTimeSpec()
*/

/*! \internal
 */
void QDateTime::detach()
{
    qAtomicDetach(d);
}

/*****************************************************************************
  Date/time stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
/*!
    \relates QDate

    Writes the \a date to stream \a out.

    \sa {Format of the QDataStream operators}
*/

QDataStream &operator<<(QDataStream &out, const QDate &date)
{
    return out << (quint32)(date.jd);
}

/*!
    \relates QDate

    Reads a date from stream \a in into the \a date.

    \sa {Format of the QDataStream operators}
*/

QDataStream &operator>>(QDataStream &in, QDate &date)
{
    quint32 jd;
    in >> jd;
    date.jd = jd;
    return in;
}

/*!
    \relates QTime

    Writes \a time to stream \a out.

    \sa {Format of the QDataStream operators}
*/

QDataStream &operator<<(QDataStream &out, const QTime &time)
{
    return out << quint32(time.mds);
}

/*!
    \relates QTime

    Reads a time from stream \a in into the given \a time.

    \sa {Format of the QDataStream operators}
*/

QDataStream &operator>>(QDataStream &in, QTime &time)
{
    quint32 ds;
    in >> ds;
    time.mds = int(ds);
    return in;
}

/*!
    \relates QDateTime

    Writes \a dateTime to the \a out stream.

    \sa {Format of the QDataStream operators}
*/
QDataStream &operator<<(QDataStream &out, const QDateTime &dateTime)
{
    out << dateTime.d->date << dateTime.d->time;
    if (out.version() >= 7)
        out << (qint8)dateTime.d->spec;
    return out;
}

/*!
    \relates QDateTime

    Reads a datetime from the stream \a in into \a dateTime.

    \sa {Format of the QDataStream operators}
*/

QDataStream &operator>>(QDataStream &in, QDateTime &dateTime)
{
    dateTime.detach();

    qint8 ts = (qint8)QDateTimePrivate::LocalUnknown;
    in >> dateTime.d->date >> dateTime.d->time;
    if (in.version() >= 7)
        in >> ts;
    dateTime.d->spec = (QDateTimePrivate::Spec)ts;
    return in;
}
#endif // QT_NO_DATASTREAM


/*!
    \fn QString QDate::monthName(int month)

    Use shortMonthName() instead.
*/

/*!
    \fn QString QDate::dayName(int weekday)

    Use shortDayName() instead.
*/

/*!
    \fn bool QDate::leapYear(int year)

    Use isLeapYear() instead.
*/

/*!
    \fn QDate QDate::currentDate(Qt::TimeSpec spec)

    Use the currentDate() overload that takes no parameters; or
    extract from currentDateTime() instead.
*/

/*!
    \fn QDate QTime::currentDate(Qt::TimeSpec spec)

    Use the currentDate() overload that takes no parameters; or
    extract from currentDateTime() instead.
*/

/*!
    \fn QTime QTime::currentTime(Qt::TimeSpec spec)

    Use the currentTime() overload that takes no parameters; or
    extract from currentDateTime() instead.
*/

/*!
    \fn void QDateTime::setTime_t(uint secsSince1Jan1970UTC, Qt::TimeSpec spec)

    Use the single-argument overload of setTime_t() instead.
*/

/*!
    \fn QDateTime QDateTime::currentDateTime(Qt::TimeSpec spec)

    Use the currentDateTime() overload that takes no parameters
    instead.
*/

// checks if there is an unqoted 'AP' or 'ap' in the string
static bool hasUnquotedAP(const QString &f)
{
    const char quote = '\'';
    bool inquote = false;
    QChar status = QLatin1Char('0');
    for (int i=0; i<f.size(); ++i) {
        if (f.at(i) == quote) {
            inquote = !inquote;
        } else if (!inquote && f.at(i).toUpper() == QLatin1Char('A')) {
            return true;
        }
    }
    return false;
}

#ifndef QT_NO_DATESTRING
/*****************************************************************************
  Some static function used by QDate, QTime and QDateTime
*****************************************************************************/

// Replaces tokens by their value. See QDateTime::toString() for a list of valid tokens
static QString getFmtString(const QString& f, const QTime* dt = 0, const QDate* dd = 0, bool am_pm = false)
{
    if (f.isEmpty())
        return QString();

    QString buf = f;
    int removed = 0;

    if (dt) {
        if (f.startsWith(QLatin1String("hh")) || f.startsWith(QLatin1String("HH"))) {
            const bool hour12 = f.at(0) == QLatin1Char('h') && am_pm;
            if (hour12 && dt->hour() > 12)
                buf = QString::number(dt->hour() - 12).rightJustified(2, QLatin1Char('0'), true);
            else if (hour12 && dt->hour() == 0)
                buf = QLatin1String("12");
            else
                buf = QString::number(dt->hour()).rightJustified(2, QLatin1Char('0'), true);
            removed = 2;
        } else if (f.at(0) == QLatin1Char('h') || f.at(0) == QLatin1Char('H')) {
            const bool hour12 = f.at(0) == QLatin1Char('h') && am_pm;
            if (hour12 && dt->hour() > 12)
                buf = QString::number(dt->hour() - 12);
            else if (hour12 && dt->hour() == 0)
                buf = QLatin1String("12");
            else
                buf = QString::number(dt->hour());
            removed = 1;
        } else if (f.startsWith(QLatin1String("mm"))) {
            buf = QString::number(dt->minute()).rightJustified(2, QLatin1Char('0'), true);
            removed = 2;
        } else if (f.at(0) == (QLatin1Char('m'))) {
            buf = QString::number(dt->minute());
            removed = 1;
        } else if (f.startsWith(QLatin1String("ss"))) {
            buf = QString::number(dt->second()).rightJustified(2, QLatin1Char('0'), true);
            removed = 2;
        } else if (f.at(0) == QLatin1Char('s')) {
            buf = QString::number(dt->second());
        } else if (f.startsWith(QLatin1String("zzz"))) {
            buf = QString::number(dt->msec()).rightJustified(3, QLatin1Char('0'), true);
            removed = 3;
        } else if (f.at(0) == QLatin1Char('z')) {
            buf = QString::number(dt->msec());
            removed = 1;
        } else if (f.at(0).toUpper() == QLatin1Char('A')) {
            const bool upper = f.at(0) == QLatin1Char('A');
            buf = dt->hour() < 12 ? QLatin1String("am") : QLatin1String("pm");
            if (upper)
                buf = buf.toUpper();
            if (f.size() > 1 && f.at(1).toUpper() == QLatin1Char('P') &&
                f.at(0).isUpper() == f.at(1).isUpper()) {
                removed = 2;
            } else {
                removed = 1;
            }
        }
    }

    if (dd) {
        if (f.startsWith(QLatin1String("dddd"))) {
            buf = dd->longDayName(dd->dayOfWeek());
            removed = 4;
        } else if (f.startsWith(QLatin1String("ddd"))) {
            buf = dd->shortDayName(dd->dayOfWeek());
            removed = 3;
        } else if (f.startsWith(QLatin1String("dd"))) {
            buf = QString::number(dd->day()).rightJustified(2, QLatin1Char('0'), true);
            removed = 2;
        } else if (f.at(0) == QLatin1Char('d')) {
            buf = QString::number(dd->day());
            removed = 1;
        } else if (f.startsWith(QLatin1String("MMMM"))) {
            buf = dd->longMonthName(dd->month());
            removed = 4;
        } else if (f.startsWith(QLatin1String("MMM"))) {
            buf = dd->shortMonthName(dd->month());
            removed = 3;
        } else if (f.startsWith(QLatin1String("MM"))) {
            buf = QString::number(dd->month()).rightJustified(2, QLatin1Char('0'), true);
            removed = 2;
        } else if (f.at(0) == QLatin1Char('M')) {
            buf = QString::number(dd->month());
            removed = 1;
        } else if (f.startsWith(QLatin1String("yyyy"))) {
            buf = QString::number(dd->year());
            removed = 4;
        } else if (f.startsWith(QLatin1String("yy"))) {
            buf = QString::number(dd->year()).right(2);
            removed = 2;
        }
    }
    if (removed == 0 || removed >= f.size()) {
        return buf;
    }

    return buf + getFmtString(f.mid(removed), dt, dd, am_pm);
}

// Parses the format string and uses getFmtString to get the values for the tokens. Ret
static QString fmtDateTime(const QString& f, const QTime* dt, const QDate* dd)
{
    const char quote = '\'';
    if (f.isEmpty())
        return QString();
    if (dt && !dt->isValid())
        return QString();
    if (dd && !dd->isValid())
        return QString();

    const bool ap = hasUnquotedAP(f);

    QString buf;
    QString frm;
    QChar status = QLatin1Char('0');

    for (int i = 0; i < (int)f.length(); ++i) {
        if (f.at(i) == quote) {
            if (status == quote) {
                status = QLatin1Char('0');
            } else {
                if (!frm.isEmpty()) {
                    buf += getFmtString(frm, dt, dd, ap);
                    frm.clear();
                }
                status = quote;
            }
        } else if (status == quote) {
            buf += f.at(i);
        } else if (f.at(i) == status) {
            if ((ap) && ((f.at(i) == QLatin1Char('P')) || (f.at(i) == QLatin1Char('p'))))
                status = QLatin1Char('0');
            frm += f.at(i);
        } else {
            buf += getFmtString(frm, dt, dd, ap);
            frm.clear();
            if ((f.at(i) == QLatin1Char('h')) || (f.at(i) == QLatin1Char('m'))
                || (f.at(i) == QLatin1Char('H'))
                || (f.at(i) == QLatin1Char('s')) || (f.at(i) == QLatin1Char('z'))) {
                status = f.at(i);
                frm += f.at(i);
            } else if ((f.at(i) == QLatin1Char('d')) || (f.at(i) == QLatin1Char('M')) || (f.at(i) == QLatin1Char('y'))) {
                status = f.at(i);
                frm += f.at(i);
            } else if ((ap) && (f.at(i) == QLatin1Char('A'))) {
                status = QLatin1Char('P');
                frm += f.at(i);
            } else  if((ap) && (f.at(i) == QLatin1Char('a'))) {
                status = QLatin1Char('p');
                frm += f.at(i);
            } else {
                buf += f.at(i);
                status = QLatin1Char('0');
            }
        }
    }

    buf += getFmtString(frm, dt, dd, ap);

    return buf;
}
#endif // QT_NO_DATESTRING

#ifdef Q_OS_WIN
static const int LowerYear = 1980;
#else
static const int LowerYear = 1970;
#endif

static QDateTimePrivate::Spec utcToLocal(QDate &date, QTime &time)
{
    QDate lowerLimit(LowerYear, 1, 2);
    QDate upperLimit(2037, 12, 30);

    QDate fakeDate = date;

    if (fakeDate < lowerLimit) {
        fakeDate = lowerLimit;
    } else if (fakeDate > upperLimit) {
        fakeDate = upperLimit;
    }

    time_t secsSince1Jan1970UTC = toTime_t(fakeDate, time);
    tm *brokenDown = 0;

#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant version of localtime() where available
    tm res;
    brokenDown = localtime_r(&secsSince1Jan1970UTC, &res);
#elif defined(_MSC_VER) && _MSC_VER >= 1400
    tm res;
    if (!_localtime64_s(&res, &secsSince1Jan1970UTC))
        brokenDown = &res;
#else
    brokenDown = localtime(&secsSince1Jan1970UTC);
#endif
    if (!brokenDown) {
        date = QDate(1970, 1, 1);
        time = QTime();
        return QDateTimePrivate::LocalUnknown;
    } else {
        int deltaDays = fakeDate.daysTo(date);
        date = QDate(brokenDown->tm_year + 1900, brokenDown->tm_mon + 1, brokenDown->tm_mday);
        time = QTime(brokenDown->tm_hour, brokenDown->tm_min, brokenDown->tm_sec, time.msec());
        date = date.addDays(deltaDays);
        if (brokenDown->tm_isdst > 0)
            return QDateTimePrivate::LocalDST;
        else if (brokenDown->tm_isdst < 0)
            return QDateTimePrivate::LocalUnknown;
        else
            return QDateTimePrivate::LocalStandard;
    }
}

static void localToUtc(QDate &date, QTime &time, int isdst)
{
    if (!date.isValid())
        return;

    QDate lowerLimit(LowerYear, 1, 2);
    QDate upperLimit(2037, 12, 30);

    QDate fakeDate = date;

    if (fakeDate < lowerLimit) {
        fakeDate = lowerLimit;
        isdst = false;
    } else if (fakeDate > upperLimit) {
        fakeDate = upperLimit;
        isdst = false;
    }

    tm localTM;
    localTM.tm_sec = time.second();
    localTM.tm_min = time.minute();
    localTM.tm_hour = time.hour();
    localTM.tm_mday = fakeDate.day();
    localTM.tm_mon = fakeDate.month() - 1;
    localTM.tm_year = fakeDate.year() - 1900;
    localTM.tm_isdst = (int)isdst;

    time_t secsSince1Jan1970UTC = mktime(&localTM);
    tm *brokenDown = 0;

#if !defined(QT_NO_THREAD) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant version of gmtime() where available
    tm res;
    brokenDown = gmtime_r(&secsSince1Jan1970UTC, &res);
#elif defined(_MSC_VER) && _MSC_VER >= 1400
    tm res;
    if (!_gmtime64_s(&res, &secsSince1Jan1970UTC))
        brokenDown = &res;
#else
    brokenDown = gmtime(&secsSince1Jan1970UTC);
#endif // !QT_NO_THREAD && _POSIX_THREAD_SAFE_FUNCTIONS
    if (!brokenDown) {
        date = QDate(1970, 1, 1);
        time = QTime();
    } else {
        int deltaDays = fakeDate.daysTo(date);
        date = QDate(brokenDown->tm_year + 1900, brokenDown->tm_mon + 1, brokenDown->tm_mday);
        time = QTime(brokenDown->tm_hour, brokenDown->tm_min, brokenDown->tm_sec, time.msec());
        date = date.addDays(deltaDays);
    }
}

QDateTimePrivate::Spec QDateTimePrivate::getLocal(QDate &outDate, QTime &outTime) const
{
    outDate = date;
    outTime = time;
    if (spec == QDateTimePrivate::UTC)
        return utcToLocal(outDate, outTime);
    return spec;
}

void QDateTimePrivate::getUTC(QDate &outDate, QTime &outTime) const
{
    outDate = date;
    outTime = time;
    if (spec != QDateTimePrivate::UTC)
        localToUtc(outDate, outTime, (int)spec);
}

#if !defined(QT_NO_DEBUG_STREAM) && !defined(QT_NO_DATESTRING)
QDebug operator<<(QDebug dbg, const QDate &date)
{
    dbg.nospace() << "QDate(" << date.toString() << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const QTime &time)
{
    dbg.nospace() << "QTime(" << time.toString() << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const QDateTime &date)
{
    dbg.nospace() << "QDateTime(" << date.toString() << ")";
    return dbg.space();
}
#endif

#ifndef QT_BOOTSTRAPPED
bool QDateTimeParser::isSpecial(const QChar &c) const
{
    switch (c.cell()) {
    case 'd': case 'M': case 'y':
        return (typ == QVariant::Date || typ == QVariant::DateTime);
    case 'H': case 'h': case 'm': case 's': case 'z': case 'a': case 'p': case 'A':
        return (typ == QVariant::Time || typ == QVariant::DateTime);
    case '\'': return true;
    default: return false;
    }
}


/*!
  \internal
  Gets the digit from a corevariant. E.g.

  QVariant var(QDate(2004, 02, 02));
  int digit = getDigit(var, Year);
  // digit = 2004
  */

int QDateTimeParser::getDigit(const QVariant &t, Section s) const
{
    switch (s) {
    case Hour24Section: case Hour12Section: return t.toTime().hour();
    case MinuteSection: return t.toTime().minute();
    case SecondSection: return t.toTime().second();
    case MSecSection: return t.toTime().msec();
    case YearSection: return t.toDate().year();
    case MonthSection: return t.toDate().month();
    case DaySection: return t.toDate().day();
    case AmPmSection: return t.toTime().hour() > 11 ? 1 : 0;

    default: break;
    }
    qFatal("%s passed to getDigit. This should never happen", sectionName(s).toLatin1().constData());
    return -1;
}

/*!
  \internal
  Sets a digit in a variant. E.g.

  QVariant var(QDate(2004, 02, 02));
  int digit = getDigit(var, Year);
  // digit = 2004
  setDigit(&var, Year, 2005);
  digit = getDigit(var, Year);
  // digit = 2005
  */

void QDateTimeParser::setDigit(QVariant &v, Section section, int newVal) const
{
    int year, month, day, hour, minute, second, msec;
    const QDateTime &dt = v.toDateTime();
    year = dt.date().year();
    month = dt.date().month();
    day = dt.date().day();
    hour = dt.time().hour();
    minute = dt.time().minute();
    second = dt.time().second();
    msec = dt.time().msec();

    switch (section) {
    case Hour24Section: case Hour12Section: hour = newVal; break;
    case MinuteSection: minute = newVal; break;
    case SecondSection: second = newVal; break;
    case MSecSection: msec = newVal; break;
    case YearSection: year = newVal; break;
    case MonthSection: month = newVal; break;
    case DaySection: day = newVal; break;
    case AmPmSection: hour = (newVal == 0 ? hour % 12 : (hour % 12) + 12); break;
    default:
        qFatal("%s passed to setDigit. This should never happen", sectionName(section).toLatin1().constData());
        break;
    }

    if (section != DaySection) {
        day = qMax<int>(cachedDay, day);
    }

    if (!QDate::isValid(year, month, day)) {
        if (year <= QDATE_MIN.year() && (month < QDATE_MIN.month()
                                         || (month == QDATE_MIN.month() && day < QDATE_MIN.day()))) {
            month = QDATE_MIN.month();
            day = QDATE_MIN.day();
        } else {
            day = qMin<int>(day, QDate(year, month, 1).daysInMonth());
        }
    }
    v = QVariant(QDateTime(QDate(year, month, day), QTime(hour, minute, second, msec)));
}



/*!
  \

  Returns the absolute maximum for a section
*/

int QDateTimeParser::absoluteMax(int s) const
{
    const SectionNode sn = sectionNode(s);
    switch (sn.type) {
    case Hour24Section:
    case Hour12Section: return 23; // we want to be able to toggle the hour field and change ampm
    case MinuteSection:
    case SecondSection: return 59;
    case MSecSection: return 999;
    case YearSection: return sn.count == 4 ? 7999 : 99;
    case MonthSection: return 12;
    case DaySection: return 31;
    case AmPmSection: return 1;
    default: break;
    }
    qFatal("%s passed to max. This should never happen", sectionName(s).toLatin1().constData());
    return -1;

}

/*!
  \internal

  Returns the absolute minimum for a section
*/

int QDateTimeParser::absoluteMin(int s) const
{
    const SectionNode sn = sectionNode(s);
    switch (sn.type)
    case Hour24Section:{
    case Hour12Section:
    case MinuteSection:
    case SecondSection:
    case MSecSection: return 0;
    case YearSection: return sn.count == 4 ? 1752 : 0;
    case MonthSection:
    case DaySection: return 1;
    case AmPmSection: return 0;
    default: break;
    }
    qFatal("%s passed to min. This should never happen", sectionName(s).toLatin1().constData());
    return -1;
}

/*!
  \internal

  Returns a copy of the sectionNode for the Section \a s.
*/

QDateTimeParser::SectionNode QDateTimeParser::sectionNode(int sectionIndex) const
{
    if (sectionIndex == FirstSectionIndex) {
        return first;
    } else if (sectionIndex == LastSectionIndex) {
        return last;
    } else if (sectionIndex == NoSectionIndex) {
        return none;
    }
    Q_ASSERT(sectionIndex >= 0 && sectionIndex < sectionNodes.size());
    return sectionNodes.at(sectionIndex);
}

QDateTimeParser::Section QDateTimeParser::sectionType(int sectionIndex) const
{
    return sectionNode(sectionIndex).type;
}


/*!
  \internal

  Returns the starting position for section \a s.
*/

int QDateTimeParser::sectionPos(int sectionIndex) const
{
    return sectionPos(sectionNode(sectionIndex));
}

int QDateTimeParser::sectionPos(const SectionNode &sn) const
{
    switch (sn.type) {
    case FirstSection: return 0;
    case LastSection: return displayText().size() - 1;
    default: break;
    }
    if (sn.pos == -1)
        QDTPDEBUG << sectionName(sn.type) << sectionNodes.indexOf(sn);
    Q_ASSERT(sn.pos != -1);
    return sn.pos;
}


/*!
  \internal helper function for parseFormat. removes quotes that are
  not escaped and removes the escaping on those that are escaped

*/

static QString unquote(const QString &str)
{
    const char quote = '\'';
    const char slash = '\\';
    const char zero = '0';
    QString ret;
    QChar status = zero;
    for (int i=0; i<str.size(); ++i) {
        if (str.at(i) == quote) {
            if (status != quote) {
                status = quote;
            } else if (!ret.isEmpty() && str.at(i - 1) == slash) {
                ret[ret.size() - 1] = quote;
            } else {
                status = zero;
            }
        } else {
            ret += str.at(i);
        }
    }
    return ret;
}
/*!
  \internal

  Parses the format \a newFormat. If successful, returns true and
  sets up the format. Else keeps the old format and returns false.

*/

static int countRepeat(const QString &str, int index)
{
    Q_ASSERT(index >= 0 && index < str.size());
    int count = 1;
    const QChar ch = str.at(index);
    while (index + count < str.size() && str.at(index + count) == ch)
        ++count;
    return count;
}

bool QDateTimeParser::parseFormat(const QString &newFormat)
{
    const char quote = '\'';
    const char slash = '\\';
    const char zero = '0';
    if (newFormat == displayFormat && !newFormat.isEmpty()) {
        //&& layoutDirection == QApplication::layoutDirection()) {
        return true;
    }
    //layoutDirection = QApplication::layoutDirection();

    QDTPDEBUGN("parseFormat: %s", newFormat.toLatin1().constData());

    const bool ap = hasUnquotedAP(newFormat);
    QList<SectionNode> newSectionNodes;
    Sections newDisplay = 0;
    QStringList newSeparators;
    int i, index = 0;
    int add = 0;
    QChar status = zero;
    for (i = 0; i<newFormat.size(); ++i) {
        if (newFormat.at(i) == quote) {
            ++add;
            if (status != quote) {
                status = quote;
            } else if (newFormat.at(i - 1) != slash) {
                status = zero;
            }
        } else if (i < newFormat.size() && status != quote) {
            const int repeat = qMin(4, countRepeat(newFormat, i));
            if (isSpecial(newFormat.at(i))) {
                const char sect = newFormat.at(i).toLatin1();
                switch (sect) {
                case 'H':
                case 'h': {
                    const Section hour = (ap && sect == 'h') ? Hour12Section : Hour24Section;
                    const SectionNode sn = { hour, i - add, qMin(2, repeat) };
                    newSectionNodes << sn;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= hour;
                    break; }
                case 'm': {
                    const SectionNode sn = { MinuteSection, i - add, qMin(2, repeat) };
                    newSectionNodes << sn;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= MinuteSection;
                    break; }
                case 's': {
                    const SectionNode sn = { SecondSection, i - add, qMin(2, repeat) };
                    newSectionNodes << sn;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    i += qMin(2, repeat) - 1;
                    index = i + 1;
                    newDisplay |= SecondSection;
                    break; }

                case 'z': {
                    const SectionNode sn = { MSecSection, i - add, (repeat < 3 ? 1 : 3) };
                    newSectionNodes << sn;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= MSecSection;
                    break; }
                case 'A':
                case 'a': {
                    const bool cap = newFormat.at(i) == QLatin1Char('A');
                    const SectionNode sn = { AmPmSection, i - add, (cap ? 1 : 0) };
                    newSectionNodes << sn;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    newDisplay |= AmPmSection;
                    if (i + 1 < newFormat.size()
                        && newFormat.at(i+1) == (cap ? QLatin1Char('P') : QLatin1Char('p'))) {
                        ++i;
                    }
                    index = i + 1;
                    break; }
                case 'y':
                    if (repeat >= 2) {
                        const bool four = repeat >= 4;
                        const SectionNode sn = { YearSection, i - add, four ? 4 : 2 };
                        newSectionNodes << sn;
                        newSeparators << unquote(newFormat.mid(index, i - index));
                        i += sn.count - 1;
                        index = i + 1;
                        newDisplay |= YearSection;
                    }
                    break;
                case 'M': {
                    const SectionNode sn = { MonthSection, i - add, repeat };
                    newSectionNodes << sn;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= MonthSection;
                    break; }
                case 'd': {
                    const SectionNode sn = { DaySection, i - add, repeat };
                    newSectionNodes << sn;
                    newSeparators << unquote(newFormat.mid(index, i - index));
                    i += sn.count - 1;
                    index = i + 1;
                    newDisplay |= DaySection;
                    break; }

                default:
                    break;
                }
            }
        }
    }
    if (newSectionNodes.isEmpty() && !allowEmpty) {
        return false;
    }

    newSeparators << (index < newFormat.size() ? unquote(newFormat.mid(index)) : QString());

    displayFormat = newFormat;
    separators = newSeparators;
    sectionNodes = newSectionNodes;
    display = newDisplay;
    last.pos = -1;
    reversedFormat.clear();
    if (isRightToLeft()) {
        for (int i=newSectionNodes.size() - 1; i>=0; --i) {
            reversedFormat += newSeparators.at(i + 1);
            reversedFormat += sectionFormat(i);
        }
        reversedFormat += newSeparators.at(0);
    }

//     for (int i=0; i<sectionNodes.size(); ++i) {
//         QDTPDEBUG << sectionName(sectionNodes.at(i).type) << sectionNodes.at(i).count;
//     }

    QDTPDEBUG << newFormat << displayFormat;
    QDTPDEBUGN("separators:\n'%s'", separators.join("\n").toLatin1().constData());

    return true;
}

/*!
  \internal

  Returns the size of section \a s.
*/

int QDateTimeParser::sectionSize(int sectionIndex) const
{
    if (sectionIndex < 0)
        return 0;
    Q_ASSERT(sectionIndex < sectionNodes.size());
    if (sectionIndex == sectionNodes.size() - 1) {
        return displayText().size() - sectionPos(sectionIndex) - separators.last().size();
    } else {
        return sectionPos(sectionIndex + 1) - sectionPos(sectionIndex)
            - separators.at(sectionIndex + 1).size();
    }
}


int QDateTimeParser::sectionMaxSize(Section s, int count) const
{
#ifndef QT_NO_TEXTDATE
    int mcount = 12;
    QString(*nameFunction)(int) = &QDate::longMonthName;
#endif

    switch (s) {
    case FirstSection:
    case NoSection:
    case LastSection: return 0;

    case AmPmSection: {
        const int lowerMax = qMin(getAmPmText(AmText, LowerCase).size(),
                                  getAmPmText(PmText, LowerCase).size());
        const int upperMax = qMin(getAmPmText(AmText, UpperCase).size(),
                                  getAmPmText(PmText, UpperCase).size());
        return qMin(4, qMin(lowerMax, upperMax));
    }

    case Hour24Section:
    case Hour12Section:
    case MinuteSection:
    case SecondSection: return 2;
    case DaySection:
#ifdef QT_NO_TEXTDATE
        return 2;
#else
        nameFunction = &QDate::longDayName;
        mcount = 7;
        // fall through
#endif
    case MonthSection:
#ifdef QT_NO_TEXTDATE
        return 2;
#else
        if (count <= 3) {
            return qMax(2, count);
        } else {
            int ret = 0;
            for (int i=1; i<=mcount; ++i) { // ### optimize? cache results?
                ret = qMax(nameFunction(i).size(), ret);
            }
            return ret;
        }
#endif
    case MSecSection: return 3;
    case YearSection: return count;

    case Internal:
    case TimeSectionMask:
    case DateSectionMask: qWarning("QDateTimeParser::sectionMaxSize: Invalid section %s", sectionName(s).toLatin1().constData());
    }
    return -1;
}


int QDateTimeParser::sectionMaxSize(int index) const
{
    const SectionNode sn = sectionNode(index);
    return sectionMaxSize(sn.type, sn.count);
}

/*!
  \internal

  Returns the text of section \a s. This function operates on the
  arg text rather than edit->text().
*/


QString QDateTimeParser::sectionText(const QString &text, int sectionIndex, int index) const
{
    const SectionNode &sn = sectionNode(sectionIndex);
    switch (sn.type) {
    case NoSectionIndex:
    case FirstSectionIndex:
    case LastSectionIndex:
        return QString();
    default: break;
    }

    return text.mid(index, sectionSize(sectionIndex));
}

#ifndef QT_NO_TEXTDATE
/*!
  \internal

  Parses the part of \a text that corresponds to \a s and returns
  the value of that field. Sets *stateptr to the right state if
  stateptr != 0.
*/

int QDateTimeParser::parseSection(int sectionIndex, QString &text, int index,
                                  State &state, int *usedptr) const
{
    state = Invalid;
    int num = 0;
    const SectionNode sn = sectionNode(sectionIndex);
    Q_ASSERT(sn.type != NoSection && sn.type != FirstSection && sn.type != LastSection);

    QString sectiontext = text.mid(index, sectionMaxSize(sectionIndex));

    QDTPDEBUG << "sectionValue for" << sectionName(sn.type)
              << "with text" << text << "and st" << sectiontext
              << text.mid(index, sectionMaxSize(sectionIndex))
              << index;

    int used = 0;
    if (false && sectiontext.trimmed().isEmpty()) {
        state = Intermediate;
    } else {
        switch (sn.type) {
        case AmPmSection: {
            const int ampm = findAmPm(sectiontext, sectionIndex, &used);
            switch (ampm) {
            case AM: // sectiontext == AM
            case PM: // sectiontext == PM
                num = ampm;
                state = Acceptable;
                break;
            case PossibleAM: // sectiontext => AM
            case PossiblePM: // sectiontext => PM
                num = ampm - 2;
                state = Intermediate;
                break;
            case PossibleBoth: // sectiontext => AM|PM
                num = 0;
                state = Intermediate;
                break;
            case Neither:
                state = Invalid;
                QDTPDEBUG << "invalid because findAmPm(" << sectiontext << ") returned -1";
                break;
            default:
                QDTPDEBUGN("This should never happen(findAmPm returned %d", ampm);
                break;
            }
            if (state != Invalid) {
                QString str = text;
                text.replace(index, used, sectiontext.left(used));
            }
            break;
        }
        case MonthSection:
        case DaySection:
            if (sn.count >= 3) {
                if (sn.type == MonthSection) {
                    num = findMonth(sectiontext.toLower(), 1, sectionIndex, &sectiontext, &used);
                } else {
                    num = findDay(sectiontext.toLower(), 1, sectionIndex, &sectiontext, &used);
                }

                if (num != -1) {
                    state = (used == sectiontext.size() ? Acceptable : Intermediate);
                    QString str = text;
                    text.replace(index, used, sectiontext.left(used));
                } else {
                    state = Intermediate;
                }
                break;
            }
            // fall through
        case YearSection:
        case Hour12Section:
        case Hour24Section:
        case MinuteSection:
        case SecondSection:
        case MSecSection: {
            if (sectiontext.isEmpty()) {
                num = 0;
                used = 0;
                state = Intermediate;
            } else {
                const int absMax = absoluteMax(sectionIndex);
                QLocale loc;
                bool ok = true;
                int last = -1;
                used = -1;

                const int max = qMin(sectionMaxSize(sectionIndex), sectiontext.size());
                for (int digits=1; digits<=max; ++digits) {
                    if (sectiontext.at(digits - 1).isSpace()) // loc.toUInt will allow spaces at the end
                        break;
                    int tmp = (int)loc.toUInt(sectiontext.left(digits), &ok, 10);
                    if (ok && tmp <= absMax) {
                        QDTPDEBUG << sectiontext.left(digits) << tmp << digits;
                        last = tmp;
                        used = digits;
                    } else {
                        break;
                    }
                }
                if (last == -1) {
                    const QChar &first = sectiontext.at(0);
                    if (separators.at(sectionIndex + 1).startsWith(first)) {
                        used = 0;
                        state = Intermediate;
                    } else {
                        state = Invalid;
                        QDTPDEBUG << "invalid because" << sectiontext << "can't become a uint" << last << ok;
                    }
                } else {
                    num += last;
                    const bool done = used == sectionMaxSize(sectionIndex);
                    if (num < absoluteMin(sectionIndex)) {
                        state = done ? Invalid : Intermediate;
                        if (done)
                            QDTPDEBUG << "invalid because" << num << "is less than absoluteMin" << absoluteMin(sectionIndex);
                    } else if (num > absMax) {
                        state = Intermediate;
                    } else if (!done && isFixedNumericSection(sectionIndex)) {
                        state = Intermediate;
                    } else {
                        state = Acceptable;
                    }
                }
            }
            break; }
        default: qFatal("NoSection or Internal. This should never happen"); break;
        }
    }

    if (usedptr)
        *usedptr = used;

    return (state != Invalid ? num : -1);
}
#endif // QT_NO_TEXTDATE

#ifndef QT_NO_DATESTRING
/*!
  \internal
*/

QDateTimeParser::StateNode QDateTimeParser::parse(const QString &inp,
                                                  const QVariant &currentValue, bool fixup) const
{
    QString input = inp;
    State state = Acceptable;
    const QVariant maximum = getMaximum();
    const QVariant minimum = getMinimum();

    QVariant tmp;
    SectionNode sn = {NoSection, 0, false};
    int pos = 0;
    bool conflicts = false;

//    QDTPDEBUG << "validateAndInterpret" << input;
    {
        int year, month, day, hour12, hour, minute, second, msec, ampm, dayofweek, year2digits;
        const QDateTime &dt = currentValue.toDateTime();
        year = dt.date().year();
        year2digits = year % 100;
        month = dt.date().month();
        day = dt.date().day();
        hour = dt.time().hour();
        hour12 = -1;
        minute = dt.time().minute();
        second = dt.time().second();
        msec = dt.time().msec();
        dayofweek = dt.date().dayOfWeek();
        ampm = -1;
        QSet<int*> isSet;
        int num;
        State tmpstate;
        int *current;

        state = Acceptable;

        for (int index=0; state != Invalid && index<sectionNodes.size(); ++index) {
            QString sep = input.mid(pos, separators.at(index).size());

            if (sep != separators.at(index)) {
                QDTPDEBUG << "invalid because" << sep << "!=" << separators.at(index)
                          << index << pos << currentSectionIndex;
                state = Invalid;
                goto end;
            }
            pos += separators.at(index).size();
            sectionNodes[index].pos = pos;
            current = 0;
            sn = sectionNodes.at(index);
            int used;

            num = parseSection(index, input, pos, tmpstate, &used);
            QDTPDEBUG << "sectionValue" << sectionName(sectionType(index)) << input
                      << "pos" << pos << "used" << used << stateName(tmpstate);
            if (fixup && tmpstate == Intermediate && isFixedNumericSection(index) && used < sn.count) {
                input.insert(pos, QString().fill(QLatin1Char('0'), sn.count - used)); // ### ltor?
                num = parseSection(index, input, pos, tmpstate, &used);
            }
            pos += qMax(0, used);

            state = qMin<State>(state, tmpstate);
            QDTPDEBUG << index << sectionName(sectionType(index)) << "is set to"
                      << pos << "state is" << stateName(state);


            if (state != Invalid) {
                switch (sn.type) {
                case Hour24Section: current = &hour; break;
                case Hour12Section: current = &hour12; break;
                case MinuteSection: current = &minute; break;
                case SecondSection: current = &second; break;
                case MSecSection: current = &msec; break;
                case YearSection:
                    if (sn.count == 2) {
                        current = &year2digits;
                    } else {
                        current = &year;
                    }
                    break;
                case MonthSection: current = &month; break;
                case DaySection:
                    if (sn.count >= 3) {
                        current = &dayofweek;
                    } else {
                        current = &day; num = qMax<int>(1, num);
                    }
                    break;
                case AmPmSection: current = &ampm; break;
                default:
                    qFatal("%s found in sections validateAndInterpret. This should never happen",
                           sectionName(sn.type).toLatin1().constData());
                    break;
                }
                Q_ASSERT(current);
                if (isSet.contains(current) && *current != num) {
                    QDTPDEBUG << "CONFLICT " << sectionName(sn.type) << *current << num;
                    conflicts = true;
                    if (index != currentSectionIndex || num == -1) {
                        continue;
                    }
                }
                if (num != -1)
                    *current = num;
                isSet.insert(current);
            }
        }

        if (state != Invalid && input.mid(pos) != separators.last()) {
            QDTPDEBUG << "1invalid because" << input.mid(pos)
                      << "!=" << separators.last() << pos;
            state = Invalid;
        }

        if (state != Invalid) {
            if (typ != QVariant::Time) {
                if (year % 100 != year2digits) {
                    if (isSet.contains(&year2digits) && !isSet.contains(&year)) {
                        year = (year / 100) * 100;
                        year += year2digits;
                    } else if (isSet.contains(&year2digits) && isSet.contains(&year)) {
                        conflicts = true;
                        SectionNode sn = sectionNode(currentSectionIndex);
                        if (sn.type == YearSection) {
                            if (sn.count == 2) {
                                year = (year / 100) * 100;
                                year += year2digits;
                            }
                        }
                    }
                }

                const QDate date(year, month, day);
                const int diff = dayofweek - date.dayOfWeek() && isSet.contains(&dayofweek);
                if (diff != 0 && state == Acceptable) {
                    conflicts = true;
                    const SectionNode &sn = sectionNode(currentSectionIndex);
                    if (sn.type == DaySection && sn.count >= 3) {
                        day -= diff;
                        if (day < 0) {
                            day += 7;
                        } else if (day > date.daysInMonth()) {
                            day -= 7;
                        }
                        QDTPDEBUG << year << month << day << dayofweek
                                  << diff << QDate(year, month, day).dayOfWeek();

                        Q_ASSERT(QDate(year, month, day).dayOfWeek() == dayofweek); // ### remove those
                        Q_ASSERT(qAbs(QDate(year, month, day).daysTo(date)) <= 7);
                    }
                }
                bool needfixday = false;
                if (sectionType(currentSectionIndex) == DaySection) {
                    cachedDay = day;
                } else if (cachedDay > day) {
                    day = cachedDay;
                    needfixday = true;
                }

                if (!QDate::isValid(year, month, day)) {
                    if (day < 32) {
                        cachedDay = day;
                    }
                    if (day > 28 && QDate::isValid(year, month, 1)) {
                        needfixday = true;
                    }
                }
                if (needfixday) {
                    if (state == Acceptable && fixday) {
                        day = qMin<int>(day, QDate(year, month, 1).daysInMonth());

                        const QLocale loc;
                        for (int i=0; i<sectionNodes.size(); ++i) {
                            if (sectionType(i) == DaySection) {
                                input.replace(sectionPos(i), sectionSize(i), loc.toString(day));
                            }
                        }
                    } else {
                        state = qMin(Intermediate, state);
                    }

                }
            }

            if (typ != QVariant::Date) {
                if (isSet.contains(&hour12)) {
                    const bool hasHour = isSet.contains(&hour);
                    if (ampm == -1) {
                        if (hasHour) {
                            ampm = (hour < 12 ? 0 : 1);
                        } else {
                            ampm = 0; // no way to tell if this is am or pm so I assume am
                        }
                    }
                    hour12 = (ampm == 0 ? hour12 % 12 : (hour12 % 12) + 12);
                    if (!hasHour) {
                        hour = hour12;
                    } else if (hour != hour12) {
                        conflicts = true;
                    }
                } else if (ampm != -1) {
                    if (!isSet.contains(&hour)) {
                        hour = (12 * ampm); // special case. Only ap section
                    } else if ((ampm == 0) != (hour < 12)) {
                        conflicts = true;
                    }
                }

            }

            tmp = QVariant(QDateTime(QDate(year, month, day), QTime(hour, minute, second, msec)));
            QDTPDEBUG << year << month << day << hour << minute << second << msec;

        }
        QDTPDEBUGN("'%s' => '%s'(%s)", input.toLatin1().constData(),
                   tmp.toString().toLatin1().constData(), stateName(state).toLatin1().constData());
    }
end:
    if (tmp.toDateTime().isValid()) {
        if (state != Invalid && dateTimeCompare(tmp, minimum) < 0) {
            state = checkIntermediate(tmp.toDateTime(), input);
        } else {
            if (dateTimeCompare(tmp, maximum) > 0)
                state = Invalid;
            QDTPDEBUG << "not checking intermediate because tmp is" << tmp  << minimum << maximum;
        }
    }
    StateNode node;
    node.input = input;
    node.state = state;
    node.conflicts = conflicts;
    node.value = tmp;
    text = input; // ### do I need this?
    return node;
}
#endif // QT_NO_DATESTRING

#ifndef QT_NO_TEXTDATE
/*!
  \internal finds the first possible monthname that \a str1 can
  match. Starting from \a index; str should already by lowered
*/

int QDateTimeParser::findMonth(const QString &str1, int startMonth, int sectionIndex, QString *usedMonth, int *used) const
{
    int bestMatch = -1;
    int bestCount = 0;
    if (!str1.isEmpty()) {
    const SectionNode sn = sectionNode(sectionIndex);
    Q_ASSERT(sn.type == MonthSection);
    QString(*nameFunction)(int) = sn.count == 3
                                  ? &QDate::shortMonthName
                                  : &QDate::longMonthName;

    for (int month=startMonth; month<=12; ++month) {
        QString str2 = nameFunction(month).toLower();

        if (str1.startsWith(str2)) {
            if (used) {
                QDTPDEBUG << "used is set to" << str2.size();
                *used = str2.size();
            }
            if (usedMonth)
                *usedMonth = nameFunction(month);
            return month;
        }

        const int limit = qMin(str1.size(), str2.size());

        QDTPDEBUG << "limit is" << limit << str1 << str2;
        bool found = true;
        for (int i=0; i<limit; ++i) {
            if (str1.at(i) != str2.at(i)) {
                if (i > bestCount) {
                    bestCount = i;
                    bestMatch = month;
                }
                found = false;
                break;
            }

        }
        if (found) {
            if (used) {
                *used = limit;
            }
            if (usedMonth)
                *usedMonth = nameFunction(month);
            QDTPDEBUG << "used is set to" << limit << *usedMonth;

            return month;
        }
    }
        if (usedMonth && bestMatch != -1)
            *usedMonth = nameFunction(bestMatch);

    }
    if (used) {
        QDTPDEBUG << "used is set to" << bestCount;
        *used = bestCount;
    }
    return bestMatch;
}

int QDateTimeParser::findDay(const QString &str1, int startDay, int sectionIndex, QString *usedDay, int *used) const
{
    int bestMatch = -1;
    int bestCount = 0;
    if (!str1.isEmpty()) {
    const SectionNode sn = sectionNode(sectionIndex);
    Q_ASSERT(sn.type == DaySection);
    QString(*nameFunction)(int) = sn.count == 3
                                  ? &QDate::shortDayName
                                  : &QDate::longDayName;

    for (int day=startDay; day<=7; ++day) {
        QString str2 = nameFunction(day).toLower();

        if (str1.startsWith(str2)) {
            if (used)
                *used = str2.size();
            if (usedDay)
                *usedDay = nameFunction(day);
            return day;
        }

        const int limit = qMin(str1.size(), str2.size());
        bool found = true;
        for (int i=0; i<limit; ++i) {
            if (str1.at(i) != str2.at(i) && !str1.at(i).isSpace()) {
                if (i > bestCount) {
                    bestCount = i;
                    bestMatch = day;
                }
                found = false;
                break;
            }

        }
        if (found) {
            if (used)
                *used = limit;
            if (usedDay)
                *usedDay = nameFunction(day);
            return day;
        }
    }
    if (usedDay && bestMatch != -1)
        *usedDay = nameFunction(bestMatch);
    }
    if (used)
        *used = bestCount;

    return bestMatch;
}
#endif // QT_NO_TEXTDATE

/*!
  \internal

  returns
  0 if str == QDateTimeEdit::tr("AM")
  1 if str == QDateTimeEdit::tr("PM")
  2 if str can become QDateTimeEdit::tr("AM")
  3 if str can become QDateTimeEdit::tr("PM")
  4 if str can become QDateTimeEdit::tr("PM") and can become QDateTimeEdit::tr("AM")
  -1 can't become anything sensible

*/

int QDateTimeParser::findAmPm(QString &str, int index, int *used) const
{
    const SectionNode s = sectionNode(index);
    Q_ASSERT(s.type == AmPmSection);
    if (used)
        *used = str.size();
    if (str.trimmed().isEmpty()) {
        return PossibleBoth;
    }
    const char space = ' ';
    int size = sectionMaxSize(index);

    enum {
        amindex = 0,
        pmindex = 1
    };
    QString ampm[2];
    ampm[amindex] = getAmPmText(AmText, s.count == 1 ? UpperCase : LowerCase);
    ampm[pmindex] = getAmPmText(PmText, s.count == 1 ? UpperCase : LowerCase);
    for (int i=0; i<2; ++i)
        ampm[i].truncate(size);

    QDTPDEBUG << "findAmPm" << str << ampm[0] << ampm[1];

    if (str.indexOf(ampm[amindex], 0, Qt::CaseInsensitive) == 0) {
        str = ampm[amindex];
        return AM;
    } else if (str.indexOf(ampm[pmindex], 0, Qt::CaseInsensitive) == 0) {
        str = ampm[pmindex];
        return PM;
    } else if (str.count(space) == 0 && str.size() >= size) {
        return Neither;
    }
    size = qMin(size, str.size());

    bool broken[2] = {false, false};
    for (int i=0; i<size; ++i) {
        if (str.at(i) != space) {
            for (int j=0; j<2; ++j) {
                if (!broken[j]) {
                    int index = ampm[j].indexOf(str.at(i));
                    QDTPDEBUG << "looking for" << str.at(i)
                              << "in" << ampm[j] << "and got" << index;
                    if (index == -1) {
                        if (str.at(i).category() == QChar::Letter_Uppercase) {
                            index = ampm[j].indexOf(str.at(i).toLower());
                            QDTPDEBUG << "trying with" << str.at(i).toLower()
                                      << "in" << ampm[j] << "and got" << index;
                        } else if (str.at(i).category() == QChar::Letter_Lowercase) {
                            index = ampm[j].indexOf(str.at(i).toUpper());
                            QDTPDEBUG << "trying with" << str.at(i).toUpper()
                                      << "in" << ampm[j] << "and got" << index;
                        }
                        if (index == -1) {
                            broken[j] = true;
                            if (broken[amindex] && broken[pmindex]) {
                                QDTPDEBUG << str << "didn't make it";
                                return Neither;
                            }
                            continue;
                        } else {
                            str[i] = ampm[j].at(index); // fix case
                        }
                    }
                    ampm[j].remove(index, 1);
                }
            }
        }
    }
    if (!broken[pmindex] && !broken[amindex])
        return PossibleBoth;
    return (!broken[amindex] ? PossibleAM : PossiblePM);
}

/*!
  \internal
  Max number of units that can be changed by this section.
*/

int QDateTimeParser::maxChange(int index) const
{
    const SectionNode sn = sectionNode(index);
    switch (sn.type) {
        // Time. unit is msec
    case MSecSection: return 999;
    case SecondSection: return 59 * 1000;
    case MinuteSection: return 59 * 60 * 1000;
    case Hour24Section: case Hour12Section: return 59 * 60 * 60 * 1000;

        // Date. unit is day
    case DaySection: return 30;
    case MonthSection: return 365 - 31;
    case YearSection: return sn.count == 2
            ? 100 * 365
            : (7999 - 1752) * 365;
    default: qFatal("%s passed to maxChange. This should never happen", sectionName(sectionType(index)).toLatin1().constData());
    }
    return -1;
}


int QDateTimeParser::multiplier(int index) const
{
    switch (sectionType(index)) {
        // Time. unit is msec
    case MSecSection: return 1;
    case SecondSection: return 1000;
    case MinuteSection: return 60 * 1000;
    case Hour24Section: case Hour12Section: return 60 * 60 * 1000;

        // Date. unit is day
    case DaySection: return 1;
    case MonthSection: return 30;
    case YearSection: return 365;

    default: break;
    }
    qFatal("%s passed to multiplier. This should never happen", sectionName(sectionType(index)).toLatin1().constData());
    return -1;
}

bool QDateTimeParser::isFixedNumericSection(int index) const
{
    const SectionNode sn = sectionNode(index);
    switch (sectionType(index)) {
    case MSecSection:
    case SecondSection:
    case MinuteSection:
    case Hour24Section: case Hour12Section: return sn.count != 1;
    case MonthSection:
    case DaySection: return sn.count == 2;
    case AmPmSection: return false;
    case YearSection: return true;
    default: qFatal("This should not happen %d %s", index, qPrintable(sectionName(sn.type)));
    }
    return false;
}




/*!
  \internal Get a number that str can become which is between min
  and max or -1 if this is not possible.
*/


QString QDateTimeParser::sectionFormat(int index) const
{
    const SectionNode sn = sectionNode(index);
    return sectionFormat(sn.type, sn.count);
}

QString QDateTimeParser::sectionFormat(Section s, int count) const
{
    QChar fillChar;
    switch (s) {
    case AmPmSection: return count == 1 ? QLatin1String("AP") : QLatin1String("ap");
    case MSecSection: fillChar = QLatin1Char('z'); break;
    case SecondSection: fillChar = QLatin1Char('s'); break;
    case MinuteSection: fillChar = QLatin1Char('m'); break;
    case Hour24Section: fillChar = QLatin1Char('H'); break;
    case Hour12Section: fillChar = QLatin1Char('h'); break;
    case DaySection: fillChar = QLatin1Char('d'); break;
    case MonthSection: fillChar = QLatin1Char('M'); break;
    case YearSection: fillChar = QLatin1Char('y'); break;
    default:
        qFatal("%s passed to sectionFormat. This should never happen", sectionName(s).toLatin1().constData());
        return QString();
    }
    Q_ASSERT(!fillChar.isNull());
    QString str;
    str.fill(fillChar, count);
    return str;
}

/*!
  \internal Get a number that str can become which is between min
  and max or -1 if this is not possible.
*/

int QDateTimeParser::potentialValue(const QString &str, int min, int max, int index,
                                    const QVariant &currentValue) const
{
    const SectionNode sn = sectionNode(index);

    int size = sectionMaxSize(index);
    const int add = (sn.type == YearSection && sn.count == 2) ? currentValue.toDate().year() % 100 : 0;
    min -= add;
    max -= add; // doesn't matter if max is -1 checking for < 0
    QString simplified = str.simplified();
    if (simplified.isEmpty()) {
        return min + add;
    } else if (simplified.toInt() > max && max >= 0) {
        return -1;
    } else {
        QString temp = simplified;
        while (temp.size() < size)
            temp.prepend(QLatin1Char('9'));
        const int t = temp.toInt();
        if (t < min) {
            return -1;
        } else if (t <= max || max < 0) {
            return t + add;
        }
    }

    const int ret = potentialValueHelper(simplified, min, max, size);
    if (ret == -1)
        return -1;
    return ret + add;
}

/*!
  \internal internal helper function called by potentialValue
*/

int QDateTimeParser::potentialValueHelper(const QString &str, int min, int max, int size) const
{
    if (str.size() == size) {
        const int val = str.toInt();
        if (val < min || val > max)
            return -1;
        QDTPDEBUG << "SUCCESS" << val << "is >=" << min << "and <=" << max;
        return val;
    }

    for (int i=0; i<=str.size(); ++i) {
        for (int j=0; j<10; ++j) {
            QString tmp = str;
            if (i == str.size()) {
                tmp.append(QChar('0' + j));
            } else {
                tmp.insert(i, QChar('0' + j));
            }
            int ret = potentialValueHelper(tmp, min, max, size);
            if (ret != -1)
                return ret;
        }
    }
    return -1;
}

#ifndef QT_NO_DATESTRING
/*!
  \internal Returns whether \a str is a string which value cannot be
  parsed but still might turn into something valid.
*/

QDateTimeParser::State QDateTimeParser::checkIntermediate(const QDateTime &dt, const QString &s) const
{
    const char space = ' ';

    const QVariant minimum = getMinimum();
    const QVariant maximum = getMaximum();
    Q_ASSERT(dateTimeCompare(dt, minimum) < 0);

    bool found = false;
    for (int i=0; i<sectionNodes.size(); ++i) {
        const SectionNode &sn = sectionNodes.at(i);
        QString t = sectionText(s, i, sn.pos).toLower();
        if (t.contains(space) || t.size() < sectionMaxSize(i)) {
            if (found) {
                QDTPDEBUG << "invalid because no spaces";
                return Invalid;
            }
            found = true;
            switch (sn.type) {
            case AmPmSection:
                switch (findAmPm(t, i)) {
                case AM:
                case PM: qFatal("%d This should not happen", __LINE__); return Acceptable;
                case Neither: return Invalid;
                case PossibleAM:
                case PossiblePM:
                case PossibleBoth: {
                    const QVariant copy(dt.addSecs(12 * 60 * 60));
                    if (dateTimeCompare(copy, minimum) >= 0 && dateTimeCompare(copy, maximum) <= 0)
                        return Intermediate;
                    return Invalid; }
                }
            case MonthSection:
                if (sn.count >= 3) {
                    int tmp = dt.date().month();
                    // I know the first possible month makes the date too early
                    while ((tmp = findMonth(t, tmp + 1, sn.count)) != -1) {
                        const QVariant copy(dt.addMonths(tmp - dt.date().month()));
                        if (dateTimeCompare(copy, minimum) >= 0 && dateTimeCompare(copy, maximum) <= 0)
                            break;
                    }
                    if (tmp == -1) {
                        return Invalid;
                    }
                }
                // fallthrough

            default: {
                int toMin;
                int toMax;
                int multi = multiplier(i);

                if (sn.type & TimeSectionMask) {
                    if (dt.daysTo(minimum.toDateTime()) != 0) {
                        QDTPDEBUG << "if (dt.daysTo(minimum.toDateTime()) != 0)" << dt.daysTo(minimum.toDateTime());
                        return Invalid;
                    }
                    toMin = dt.time().msecsTo(minimum.toDateTime().time());
                    if (dt.daysTo(maximum.toDateTime()) > 0) {
                        toMax = -1; // can't get to max
                    } else {
                        toMax = dt.time().msecsTo(maximum.toDateTime().time());
                    }
                } else {
                    toMin = dt.daysTo(minimum.toDateTime());
                    toMax = dt.daysTo(maximum.toDateTime());
                }
                int maxChange = QDateTimeParser::maxChange(i);
                qlonglong maxChangeUnits = (qint64)maxChange * (qint64)multi;
                if (toMin > maxChangeUnits) {
                    QDTPDEBUG << "invalid because toMin > maxChangeUnits" << toMin
                              << maxChangeUnits << t << dt << minimum.toDateTime()
                              << multi;

                    return Invalid;
                } else if (toMax > maxChangeUnits) {
                    toMax = -1; // can't get to max
                }

                int min = getDigit(minimum, sn.type);
                int max = toMax != -1 ? getDigit(maximum, sn.type) : -1;
                int tmp = potentialValue(t, min, max, i, dt);
                QDTPDEBUG << tmp << t << min << max << sectionName(sn.type)
                          << minimum.toDate() << maximum.toDate();
                if (tmp == -1) {
                    QDTPDEBUG << "invalid because potentialValue(" << t << min << max
                              << sectionName(sn.type) << "returned" << tmp;
                    return Invalid;
                }

                QVariant var(dt);
                setDigit(var, sn.type, tmp);
                if (dateTimeCompare(var, maximum) > 0) {
                    QDTPDEBUG << "invalid because" << var.toString() << ">" << maximum.toString();
                    return Invalid;
                }
                break; }
            }
        }
    }
    return found ? Intermediate : Invalid;
}
#endif // QT_NO_DATESTRING

/*!
  \internal
  For debugging. Returns the name of the section \a s.
*/

QString QDateTimeParser::sectionName(int s) const
{
    switch (s) {
    case QDateTimeParser::AmPmSection: return QLatin1String("AmPmSection");
    case QDateTimeParser::DaySection: return QLatin1String("DaySection");
    case QDateTimeParser::Hour24Section: return QLatin1String("Hour24Section");
    case QDateTimeParser::Hour12Section: return QLatin1String("Hour12Section");
    case QDateTimeParser::MSecSection: return QLatin1String("MSecSection");
    case QDateTimeParser::MinuteSection: return QLatin1String("MinuteSection");
    case QDateTimeParser::MonthSection: return QLatin1String("MonthSection");
    case QDateTimeParser::SecondSection: return QLatin1String("SecondSection");
    case QDateTimeParser::YearSection: return QLatin1String("YearSection");
    case QDateTimeParser::NoSection: return QLatin1String("NoSection");
    case QDateTimeParser::FirstSection: return QLatin1String("FirstSection");
    case QDateTimeParser::LastSection: return QLatin1String("LastSection");
    default: return QLatin1String("Unknown section ") + QString::number(s);
    }
}

/*!
  \internal
  For debugging. Returns the name of the state \a s.
*/

QString QDateTimeParser::stateName(int s) const
{
    switch (s) {
    case Invalid: return "Invalid";
    case Intermediate: return "Intermediate";
    case Acceptable: return "Acceptable";
    default: return "Unknown state " + QString::number(s);
    }
}

#ifndef QT_NO_DATESTRING
bool QDateTimeParser::fromString(const QString &text, QDate *date, QTime *time) const
{
    QVariant val;
    if (date && time) {
        val = QDateTime(QDate(1900, 1, 1), QTIME_MIN);
    } else if (date) {
        val = QDate(1900, 1, 1);
    } else {
        Q_ASSERT(time);
        val = QTIME_MIN;
    }
    const StateNode tmp = parse(text, val, false);
    if (tmp.state != Acceptable || tmp.conflicts) {
        return false;
    }
    if (time) {
        const QTime t = tmp.value.toTime();
        if (!t.isValid()) {
            return false;
        }
        *time = t;
    }

    if (date) {
        const QDate d = tmp.value.toDate();
        if (!d.isValid()) {
            return false;
        }
        *date = d;
    }
    return true;
}

QVariant QDateTimeParser::getMinimum() const
{
    switch (typ) {
    case QVariant::Time: return QTIME_MIN;
    case QVariant::Date: return QDATE_MIN;
    case QVariant::DateTime: return QDATETIME_MIN;
    default: break;
    }
    return QVariant();
}
QVariant QDateTimeParser::getMaximum() const
{
    switch (typ) {
    case QVariant::Time: return QTIME_MAX;
    case QVariant::Date: return QDATE_MAX;
    case QVariant::DateTime: return QDATETIME_MAX;
    default: break;
    }
    return QVariant();
}
#endif // QT_NO_DATESTRING

QString QDateTimeParser::getAmPmText(AmPm ap, Case cs) const
{
    if (ap == AmText) {
        return (cs == UpperCase ? QLatin1String("AM") : QLatin1String("am"));
    } else {
        return (cs == UpperCase ? QLatin1String("PM") : QLatin1String("pm"));
    }
}

/*
  \internal

  I give arg2 preference because arg1 is always a QDateTime.
*/

int QDateTimeParser::dateTimeCompare(const QVariant &arg1, const QVariant &arg2)
{
    if ((arg1.type() == QVariant::Time && arg2.type() == QVariant::Date)
        || (arg1.type() == QVariant::Date && arg2.type() == QVariant::Time)) {
        qWarning("QDateTimeParser::dateTimeCompare: Different types (%s vs. %s)",
                 arg1.typeName(), arg2.typeName());
    }
    switch (arg2.type()) {
    case QVariant::Date:
        if (arg1.toDate() == arg2.toDate()) {
            return 0;
        } else if (arg1.toDate() < arg2.toDate()) {
            return -1;
        } else {
            return 1;
        }
    case QVariant::Time:
        if (arg1.toTime() == arg2.toTime()) {
            return 0;
        } else if (arg1.toTime() < arg2.toTime()) {
            return -1;
        } else {
            return 1;
        }

    case QVariant::DateTime:
        if (arg1.toDateTime() == arg2.toDateTime()) {
            return 0;
        } else if (arg1.toDateTime() < arg2.toDateTime()) {
            return -1;
        } else {
            return 1;
        }
    default: break;
    }
    qWarning("QDateTimeParser::dateTimeCompare: Unsupported types (%s, %s)",
             arg1.typeName(), arg2.typeName());

    return -2;
}

bool operator==(const QDateTimeParser::SectionNode &s1, const QDateTimeParser::SectionNode &s2)
{
    return (s1.type == s2.type) && (s1.pos == s2.pos) && (s1.count == s2.count);
}


#endif // QT_BOOTSTRAPPED
