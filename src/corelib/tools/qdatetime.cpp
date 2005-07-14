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
#include "qdatetime.h"
#include "qregexp.h"
#ifndef QT_NO_DEBUG_STREAM
#include "qdebug.h"
#endif
#if defined(Q_OS_WIN32)
#include <windows.h>
#include <time.h>
#endif
#ifndef Q_WS_WIN
#include <locale.h>
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
static const char * const qt_longMonthNames[] = {
    "January", "February", "Mars", "April", "May", "June",
    "July", "August", "September", "October", "November", "December" };
static const char * const qt_shortDayNames[] = {
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
static const char * const qt_longDayNames[] = {
    "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday" };

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
    \code

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
    THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
    IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
    \endcode

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
        if (GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, mmm_t, buf, 255))
            return QString::fromUtf16((ushort*)buf);
    } , {
        char buf[255];
        if (GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, "MMM", (char*)&buf, 255))
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
        if (GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, mmmm_t, buf, 255))
            return QString::fromUtf16((ushort*)buf);
    } , {
        char buf[255];
        if (GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, "MMMM", (char*)&buf, 255))
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
        if (GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, ddd_t, buf, 255))
            return QString::fromUtf16((ushort*)buf);
    } , {
        char buf[255];
        if (GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, "ddd", (char*)&buf, 255))
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
        if (GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, dddd_t, buf, 255))
            return QString::fromUtf16((ushort*)buf);
    } , {
        char buf[255];
        if (GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, "dddd", (char*)&buf, 255))
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

    If the \a format is \c Qt::TextDate, the string is formatted in
    the default way. QDate::shortDayName() and QDate::shortMonthName()
    are used to generate the string, so the day and month names will
    be localized names. An example of this formatting is
    "Sat May 20 1995".

    If the \a format is \c Qt::ISODate, the string format corresponds
    to the ISO 8601 extended specification for representations of
    dates and times, taking the form YYYY-MM-DD, where YYYY is the
    year, MM is the month of the year (between 01 and 12), and DD is
    the day of the month between 01 and 31.

    If the \a format is \c Qt::LocalDate, the string format depends
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
                if (GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, 0, buf, 255))
                    return QString::fromUtf16((ushort*)buf);
            } , {
                char buf[255];
                if (GetDateFormatA(LOCALE_USER_DEFAULT, 0, &st, 0, (char*)&buf, 255))
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

#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant version of localtime() where available
    tm res;
    t = localtime_r(&ltime, &res);
#else
    t = localtime(&ltime);
#endif // QT_THREAD_SUPPORT && _POSIX_THREAD_SAFE_FUNCTIONS

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

    Note for \c Qt::TextDate: It is recommended that you use the
    English short month names (e.g. "Jan"). Although localized month
    names can also be used, they depend on the user's locale settings.

    \warning \c Qt::LocalDate cannot be used here.
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
#endif //QT_NO_DATESTRING

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
    \row    \i Year   \i The current year
    \row    \i Month  \i 1
    \row    \i Day    \i 1
    \endtable

    The following examples demonstrate the default values:

    \code
        QDate::fromString("1.30", "M.d");           // January 30 in the current year
        QDate::fromString("20000110", "yyyyMMdd");  // January 10, 2000
        QDate::fromString("20000110", "yyyyMd");    // January 10, 2000
    \endcode

    \sa QDateTime::fromString(), QTime::fromString(), QDate::toString(),
        QDateTime::toString(), QTime::toString()
*/

QDate QDate::fromString(const QString &string, const QString &format)
{
    QDateTimeParser dt(format, QVariant::Date);
    QDate date;
    return dt.fromString(string, &date, 0) ? date : QDate();
}

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

    If \a f is \c Qt::TextDate, the string format is HH:MM:SS; e.g. 1
    second before midnight would be "23:59:59".

    If \a f is \c Qt::ISODate, the string format corresponds to the
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

    Example format strings (assuming that the QTime is 14:13:09.042)

    \table
    \header \i Format \i Result
    \row \i hh:mm:ss.zzz \i 14:13:09.042
    \row \i h:m:s ap     \i 2:13:9 pm
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

#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
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

    \warning Note that \c Qt::LocalDate cannot be used here.
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
    int msec(s.mid(9, 3).toInt());
    return QTime(hour, minute, second, msec);
}
#endif


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
    QDateTimeParser dt(format, QVariant::Time);
    QTime time;
    return dt.fromString(string, 0, &time) ? time : QTime(-1, -1, -1);
}

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
    Constructs a datetime with the given \a date, and a null but valid
    time (00:00:00.000).
*/

QDateTime::QDateTime(const QDate &date)
{
    d = new QDateTimePrivate;
    d->date = date;
}

/*!
    Constructs a datetime with the given \a date and \a time, using
    the time specification defined by \a spec.
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
    behave as if local time were \c Qt::UTC.

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

    If the \a format is \c Qt::TextDate, the string is formatted in
    the default way. QDate::shortDayName(), QDate::shortMonthName(),
    and QTime::toString() are used to generate the string, so the
    day and month names will be localized names. An example of this
    formatting is "Wed May 20 03:40:13 1998".

    If the \a format is \c Qt::ISODate, the string format corresponds
    to the ISO 8601 extended specification for representations of
    dates and times, taking the form YYYY-MM-DDTHH:MM:SS.

    If the \a format is \c Qt::LocalDate, the string format depends
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

/*!
    Returns a QDateTime object containing a datetime \a nsecs seconds
    later than the datetime of this object (or earlier if \a nsecs is
    negative).

    \sa secsTo(), addDays(), addMonths(), addYears()
*/

QDateTime QDateTime::addSecs(int nsecs) const
{
    QDate utcDate;
    QTime utcTime;
    d->getUTC(utcDate, utcTime);

    uint dd = utcDate.jd;
    int tt = utcTime.ds();
    int sign = 1;
    if (nsecs < 0) {
        nsecs = -nsecs;
        sign = -1;
    }
    if (nsecs >= (int)SECS_PER_DAY) {
        dd += sign * (nsecs / SECS_PER_DAY);
        nsecs %= SECS_PER_DAY;
    }
    tt += sign * nsecs * 1000;
    if (tt < 0) {
        tt = MSECS_PER_DAY - tt - 1;
        dd -= tt / MSECS_PER_DAY;
        tt = tt % MSECS_PER_DAY;
        tt = MSECS_PER_DAY - tt - 1;
    } else if (tt >= (int)MSECS_PER_DAY) {
        dd += tt / MSECS_PER_DAY;
        tt = tt % MSECS_PER_DAY;
    }
    utcDate.jd = dd;
    utcTime.mds = tt;
    return QDateTime(utcDate, utcTime, Qt::UTC).toTimeSpec(timeSpec());
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
        QDateTime xmas(QDate(dt.date().year(), 12, 25), QTime(0, 0));
        qDebug("There are %d seconds to Christmas", dt.secsTo(xmas));
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

    Note for \c Qt::TextDate: It is recommended that you use the
    English short month names (e.g. "Jan"). Although localized month
    names can also be used, they depend on the user's locale settings.

    \warning Note that \c Qt::LocalDate cannot be used here.
*/
QDateTime QDateTime::fromString(const QString& s, Qt::DateFormat f)
{
    if (s.isEmpty() || f == Qt::LocalDate) {
        qWarning("QDateTime::fromString: Parameter out of range");
        return QDateTime();
    }
    if (f == Qt::ISODate) {
        return QDateTime(QDate::fromString(s.mid(0, 10), Qt::ISODate),
                         QTime::fromString(s.mid(11), Qt::ISODate));
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
#endif //QT_NO_DATESTRING

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
    of characters that are enclosed in singlequotes will also be
    treated as text and not be used as an expression.

    \code
        QDateTime dateTime = QDateTime::fromString("M1d1y9800:01:02",
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
    \row    \i Year   \i The current year
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
    QDateTimeParser dt(format, QVariant::DateTime);
    QTime time;
    QDate date;
    return dt.fromString(string, &date, &time) ? QDateTime(date, time) : QDateTime(QDate(), QTime(-1, -1, -1));
}

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
        if (f.startsWith(QLatin1String("hh"))) {
            if ((am_pm) && (dt->hour() > 12))
                buf = QString::number(dt->hour() - 12).rightJustified(2, QLatin1Char('0'), true);
            else if ((am_pm) && (dt->hour() == 0))
                buf = QLatin1String("12");
            else
                buf = QString::number(dt->hour()).rightJustified(2, QLatin1Char('0'), true);
            removed = 2;
        } else if (f.at(0) == QLatin1Char('h')) {
            if ((am_pm) && (dt->hour() > 12))
                buf = QString::number(dt->hour() - 12);
            else if ((am_pm) && (dt->hour() == 0))
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
        } else if (f.startsWith(QLatin1String("ap"))) {
            buf = dt->hour() < 12 ? QLatin1String("am") : QLatin1String("pm");
            removed = 2;
        } else if (f.startsWith(QLatin1String("AP"))) {
            buf = dt->hour() < 12 ? QLatin1String("AM") : QLatin1String("PM");
            removed = 2;
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
    if (removed == 0 || removed >= f.size())
        return buf;
    return buf + getFmtString(f.mid(removed), dt, dd, am_pm);
}

// checks if there is an unqoted 'AP' or 'ap' in the string
static bool hasUnqutedAP(const QString &f)
{
    const char quote = '\'';
    QChar status = QLatin1Char('0');
    for (int i=0; i<f.size(); ++i) {
        if (f.at(i) == quote) {
            if (status == quote) {
                if (f.at(i - 1) != QLatin1Char('\\'))
                    status = QLatin1Char('0');
            } else {
                status = quote;
            }
        } else if (status != quote) {
            if (f.at(i).toUpper() == QLatin1Char('A')) {
                status = f.at(i);
            } else if ((f.at(i) == QLatin1Char('p') && status == QLatin1Char('a'))
                    || (f.at(i) == QLatin1Char('P') && status == QLatin1Char('A'))) {
                return true;
            } else {
                status = QLatin1Char('0');
            }
        }
    }

    return false;
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

    bool ap = hasUnqutedAP(f);

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

#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant version of localtime() where available
    tm res;
    tm *brokenDown = localtime_r(&secsSince1Jan1970UTC, &res);
#else
    tm *brokenDown = localtime(&secsSince1Jan1970UTC);
#endif // QT_THREAD_SUPPORT && _POSIX_THREAD_SAFE_FUNCTIONS
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

#if defined(QT_THREAD_SUPPORT) && defined(_POSIX_THREAD_SAFE_FUNCTIONS)
    // use the reentrant version of gmtime() where available
    tm res;
    tm *brokenDown = gmtime_r(&secsSince1Jan1970UTC, &res);
#else
    tm *brokenDown = gmtime(&secsSince1Jan1970UTC);
#endif // QT_THREAD_SUPPORT && _POSIX_THREAD_SAFE_FUNCTIONS
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

QFormatSection QDateTimeParser::firstSection = QFormatSection(0, QDateTimeParser::FirstSection);
QFormatSection QDateTimeParser::lastSection = QFormatSection(-1, QDateTimeParser::LastSection);

QFormatSection::QFormatSection(int ind, const QString &sep)
    : index(ind), chars(sep), type(QDateTimeParser::Separator)
{
}

QFormatSection::QFormatSection(int ind, QDateTimeParser::Section typ)
    : index(ind), type(typ)
{
}

int QFormatSection::length() const
{
    return type == QDateTimeParser::Separator ? chars.size() : QFormatSection::length(type);
}

int QFormatSection::length(QDateTimeParser::Section t)
{
    switch (t) {
    case QDateTimeParser::Day1: case QDateTimeParser::Month1: case QDateTimeParser::Hour1: case QDateTimeParser::Minute1:
    case QDateTimeParser::Second1: case QDateTimeParser::MSecond1: case QDateTimeParser::Quote: return 1;

    case QDateTimeParser::Day2: case QDateTimeParser::Month2: case QDateTimeParser::Year2: case QDateTimeParser::Hour2:
    case QDateTimeParser::Minute2: case QDateTimeParser::Second2: case QDateTimeParser::APLower: case QDateTimeParser::APUpper: return 2;

    case QDateTimeParser::Day3: case QDateTimeParser::Month3: case QDateTimeParser::MSecond3: return 3;

    case QDateTimeParser::Day4: case QDateTimeParser::Month4: case QDateTimeParser::Year4: return 4;

    default:
        qWarning("%s:%d QDateTimeParser::length() %d should never be called here", __FILE__, __LINE__, t);
        return 0;
    }
}

QDateTimeParser::QDateTimeParser(const QString &f, QVariant::Type t)
    : display(0)
{
    parseFormat(f, t);
}

bool QDateTimeParser::withinBounds(QDateTimeParser::Section t, int num)
{
    int min, max;
    if (t == QDateTimeParser::Year2) {
        min = 0; max = 99;
    } else if (t == QDateTimeParser::Day3 || t == QDateTimeParser::Day4) {
        min = 1; max = 7;
    } else if (t == QDateTimeParser::Year4) {
        min = 1752; max = 7999;
    } else if (t & QDateTimeParser::MonthMask) {
        min = 1; max = 12;
    } else if (t & QDateTimeParser::DayMask) {
        min = 1; max = 31;
    } else if (t & QDateTimeParser::HourMask) {
        min = 0; max = 23;
    } else if (t & QDateTimeParser::MinuteMask) {
        min = 0; max = 59;
    } else if (t & QDateTimeParser::SecondMask) {
        min = 0; max = 59;
    } else if (t & QDateTimeParser::MSecondMask) {
        min = 0; max = 999;
    } else {
        qWarning("%s:%d QDateTimeParser::withinBounds() %0x should never be called with this argument", __FILE__, __LINE__, t);
        return false;
    }

    return num >= min && num <= max;
}

int QDateTimeParser::getNumber(int index, const QString &str, int mindigits, int maxdigits, bool *ok, int *digits)
{
    if (str.size() < index + mindigits) {
        *ok = false;
        *digits = 0;
        return 0;
    }
    *digits = 0;
    int i = index;

    while (i < str.size() && str.at(i++).isNumber() && *digits < maxdigits)
        ++(*digits);

    if (*digits < mindigits) {
        *ok = false;
        *digits = 0;
        return 0;
    } else {
        return str.mid(index, *digits).toInt(ok);
    }
}

bool QDateTimeParser::isSpecial(const QChar &c) const
{
    switch (c.cell()) {
    case 'd': case 'M': case 'y':
        return (formatType == QVariant::Date || formatType == QVariant::DateTime);
    case 'h': case 'm': case 's': case 'z': case 'a': case 'p': case 'A': case 'P':
        return (formatType == QVariant::Time || formatType == QVariant::DateTime);
    case '\'': return true;
    default: return false;
    }
}

QFormatSection QDateTimeParser::findNextFormat(const QString &str, const int start)
{
    const char quote = '\'';
    int i = start;
    QDateTimeParser::Section typ = QDateTimeParser::NoSection;
    while (i < str.size()) {
        const QChar &ch = str.at(i);
        if (isSpecial(ch)) {
            const QString rest = str.mid(i);
            switch (ch.cell()) {
            case quote: typ = QDateTimeParser::Quote; break;
            case 'd':
                if (rest.startsWith(QLatin1String("dddd"))) {
                    typ = QDateTimeParser::Day4;
                } else if (rest.startsWith(QLatin1String("ddd"))) {
                    typ = QDateTimeParser::Day3;
                } else if (rest.startsWith(QLatin1String("dd"))) {
                    typ = QDateTimeParser::Day2;
                } else {
                    typ = QDateTimeParser::Day1;
                }
                break;
            case 'M':
                if (rest.startsWith(QLatin1String("MMMM"))) {
                    typ = QDateTimeParser::Month4;
                } else if (rest.startsWith(QLatin1String("MMM"))) {
                    typ = QDateTimeParser::Month3;
                } else if (rest.startsWith(QLatin1String("MM"))) {
                    typ = QDateTimeParser::Month2;
                } else {
                    typ = QDateTimeParser::Month1;
                }
                break;

            case 'y':
                if (rest.startsWith(QLatin1String("yyyy"))) {
                    typ = QDateTimeParser::Year4;
                } else if (rest.startsWith(QLatin1String("yy"))) {
                    typ = QDateTimeParser::Year2;
                }
                break;

            case 'h':
                if (rest.startsWith(QLatin1String("hh"))) {
                    typ = QDateTimeParser::Hour2;
                } else {
                    typ = QDateTimeParser::Hour1;
                }
                break;

            case 'm':
                if (rest.startsWith(QLatin1String("mm"))) {
                    typ = QDateTimeParser::Minute2;
                } else {
                    typ = QDateTimeParser::Minute1;
                }
                break;

            case 's':
                if (rest.startsWith(QLatin1String("ss"))) {
                    typ = QDateTimeParser::Second2;
                } else {
                    typ = QDateTimeParser::Second1;
                }
                break;

            case 'z':
                if (rest.startsWith(QLatin1String("zzz"))) {
                    typ = QDateTimeParser::MSecond3;
                } else {
                    typ = QDateTimeParser::MSecond1;
                }
                break;

            case 'a':
                if (rest.count() > 1 && rest.at(1) == QLatin1Char('p')) {
                    typ = QDateTimeParser::APLower;
                }
                break;

            case 'A':
                if (rest.count() > 1 && rest.at(1) == QLatin1Char('P')) {
                    typ = QDateTimeParser::APUpper;
                }
                break;

            default: qFatal("Should never happen"); break;
            }

            if (typ != QDateTimeParser::NoSection) {
                if (i == start) {
                    return QFormatSection(start, typ);
                } else {
                    break; // found a separator before this section
                }
            }
        }
        ++i;
    }
    return QFormatSection(start, str.mid(start, i - start));
}

void QDateTimeParser::parseFormat(const QString &f, QVariant::Type t)
{
    const char quote = '\'';
    display = 0;
    formatType = t;
    format = f;
    sect.clear();

    int i = 0;
    while (i < format.size()) {
        QFormatSection s;
        if (format.at(i) == quote) {
            int nextQuote = format.indexOf(quote, i + 1);
            if (nextQuote == -1)
                nextQuote = format.size() + 1;
            s = QFormatSection(i, format.mid(i, nextQuote - i + 1));
        } else {
            s = findNextFormat(format, i);
        }
        if (s.type == QDateTimeParser::Separator && !sect.isEmpty() && sect.last().type == QDateTimeParser::Separator) {
            sect.last().chars += s.chars;
        } else {
            sect << s;
            display |= s.type;
        }
        i = s.index + s.length();
    }
}

bool QDateTimeParser::fromString(const QString &string, QDate *dateIn, QTime *timeIn)
{
    Q_ASSERT(dateIn || timeIn);
    const char quote = '\'';
    int msec = -1;
    int sec = -1;
    int minute = -1;
    int hour = -1;
    int day = -1;
    int month = -1;
    int year = -1;
    int ampm = -1;
    int dayOfWeek = -1;

    int index = 0;
    int i = 0;
    while (i<sect.size()) {
        if (index >= string.size()) {
            return false;
        }
        int *num = 0;
        QString (*nameFunction)(int) = 0;
        const char * const * nameArray = 0;
        int max = -1;
        int min = 1;
        const QFormatSection &s = sect.at(i);
        switch (s.type) {
        case QDateTimeParser::Separator: {
            QString sep = s.chars;
            sep.remove(quote);

            if (string.mid(index, sep.length()) != sep) {
                return false;
            }
            index += sep.size();
            break; }

        case QDateTimeParser::APLower: {
        case QDateTimeParser::APUpper:
            const QChar a = s.type == QDateTimeParser::APLower ? QLatin1Char('a') : QLatin1Char('A');
            const QChar p = s.type == QDateTimeParser::APLower ? QLatin1Char('p') : QLatin1Char('P');
            const QChar m = s.type == QDateTimeParser::APLower ? QLatin1Char('m') : QLatin1Char('M');

            if ((string.at(index) != a && string.at(index) != p)
                || string.size() < index + 2
                || string.at(index + 1) != m) {
                return false;
            }
            int newampm = string.at(index) == a ? 0 : 1;
            if (ampm != -1 && newampm != ampm) {
                return false;
            }
            ampm = newampm;
            index += 2;
            break; }

#ifndef QT_NO_TEXTDATE
        case QDateTimeParser::Day3: num = &day; nameFunction = &QDate::shortDayName; nameArray = qt_shortDayNames; max = 7; break;
        case QDateTimeParser::Day4: num = &day; nameFunction = &QDate::longDayName; nameArray = qt_longDayNames; max = 7; break;
        case QDateTimeParser::Month3: num = &month; nameFunction = &QDate::shortMonthName; nameArray = qt_shortMonthNames; max = 12; break;
        case QDateTimeParser::Month4: num = &month; nameFunction = &QDate::longMonthName; nameArray = qt_longMonthNames; max = 12; break;
#else
        case QDateTimeParser::Day3: num = &day; max = 7; break;
        case QDateTimeParser::Day4: num = &day; max = 7; break;
        case QDateTimeParser::Month3: num = &month; max = 12; break;
        case QDateTimeParser::Month4: num = &month; max = 12; break;
#endif
            
        case QDateTimeParser::Day1: num = &day; max = 2; break;
        case QDateTimeParser::Month1: num = &month; max = 2; break;
        case QDateTimeParser::Hour1: num = &hour; max = 2; break;
        case QDateTimeParser::Minute1: num = &minute; max = 2; break;
        case QDateTimeParser::Second1: num = &sec; max = 2; break;
        case QDateTimeParser::MSecond1: num = &msec; max = 3; break;
        case QDateTimeParser::Day2: num = &day; min = 2; max = 2; break;
        case QDateTimeParser::Month2: num = &month; min = 2; max = 2; break;
        case QDateTimeParser::Year2: num = &year; min = 2; max = 2; break;
        case QDateTimeParser::Hour2: num = &hour; min = 2; max = 2; break;
        case QDateTimeParser::Minute2: num = &minute; min = 2; max = 2; break;
        case QDateTimeParser::Second2: num = &sec; min = 2; max = 2; break;
        case QDateTimeParser::MSecond3: num = &msec; min = 3; max = 3; break;
        case QDateTimeParser::Year4: num = &year; min = 4; max = 4; break;

        default:
            qWarning("%s:%d QDateTimeParser::fromString() %d should never be called here", __FILE__, __LINE__, s.type);
            return false;
        }

        if (nameFunction) {
            const QString rest = string.mid(index);
            int add = -1;
            int j;
            for (j=min; j<=max; ++j) {
                const QString tmp = nameFunction(j);
                if (rest.startsWith(tmp)) {
                    add = tmp.size();
                    break;
                }
                const QLatin1String tmp2(nameArray[j - 1]);
                if (rest.startsWith(tmp2)) {
                    add = strlen(tmp2.latin1());
                    break;
                }
            }
            if (j > max || (*num != -1 && *num != j) || add == -1) {
                return false;
            }
            *num = j;
            index += add;
        } else if (num) {
            bool ok;
            int digits;
            int number = getNumber(index, string, min, max, &ok, &digits);
            if (!ok || !withinBounds(s.type, number) || (*num != -1 && *num != number)) {
                return false;
            }

            *num = number;
            index += digits;
        }
        ++i;
    }
    if (index < string.size()) {
        return false;
    }

    if (month == -1)
        month = 1;
    if (year == -1)
        year = QDate::currentDate().year();
    if (dayOfWeek != -1) {
        if (day != -1) {
            QDate dt(year, month, day);
            if (dt.dayOfWeek() != dayOfWeek) {
                return false;
            }
        } else {
            QDate dt(year, month, 1);
            if (dt.dayOfWeek() < dayOfWeek) {
                dt = dt.addDays(dayOfWeek - dt.dayOfWeek());
            } else if (dt.dayOfWeek() > dayOfWeek) {
                dt = dt.addDays(7 + dayOfWeek - dt.dayOfWeek());
            }
            day = dt.day();
        }
    }
    if (day == -1)
        day = 1;
    if (hour == -1)
        hour = 0;
    if (minute == -1)
        minute = 0;
    if (sec == -1)
        sec = 0;
    if (msec == -1)
        msec = 0;
    if (ampm == 0){
        if (hour == 12) {
            hour = 0;
        } else if (hour > 12) {
            return false;
        }
    } else if (ampm == 1) {
        if (hour < 12) {
            hour += 12;
        } else if (hour > 12) {
            return false;
        }
    }

    if (timeIn) {
        QTime t(hour, minute, sec, msec);
        if (!t.isValid()) {
            return false;
        }
        *timeIn = t;
    }

    if (dateIn) {
        QDate dt(year, month, day);
        if (!dt.isValid()) {
            return false;
        }
        *dateIn = dt;
    }

    return true;
}
