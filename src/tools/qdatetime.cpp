/****************************************************************************
** $Id$
**
** Implementation of date and time classes
**
** Created : 940124
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

// Get the system specific includes and defines
#include "qplatformdefs.h"

#include "qdatetime.h"
#include "qdatastream.h"
#include "qregexp.h"

#include <stdio.h>
#ifndef Q_OS_TEMP
#include <time.h>
#endif

#if defined(Q_OS_WIN32)
#include <windows.h>
#elif defined(Q_OS_MSDOS)
#include <dos.h>
#elif defined(Q_OS_OS2)
#include <os2.h>
#endif

static const uint FIRST_DAY	= 2361222;	// Julian day for 1752/09/14
static const int  FIRST_YEAR	= 1752;		// ### wrong for many countries
static const uint SECS_PER_DAY	= 86400;
static const uint MSECS_PER_DAY = 86400000;
static const uint SECS_PER_HOUR = 3600;
static const uint MSECS_PER_HOUR= 3600000;
static const uint SECS_PER_MIN	= 60;
static const uint MSECS_PER_MIN = 60000;

static const short monthDays[] = {
    0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

static const char * const qt_shortMonthNames[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

#ifndef QT_NO_DATESTRING
/*****************************************************************************
  Some static function used by QDate, QTime and QDateTime
 *****************************************************************************/

// Replaces tokens by their value. See QDateTime::toString() for a list of valid tokens
static QString getFmtString( const QString& f, const QTime* dt = 0, const QDate* dd = 0, bool am_pm = FALSE )
{
    if ( f.isEmpty() )
	return QString::null;

    QString buf = f;

    if ( dt ) {
	if ( f == "h" ) {
	    if ( ( am_pm ) && ( dt->hour() >= 12 ) )
		buf = QString::number( dt->hour() - 12 );
	    else
		buf = QString::number( dt->hour() );
        } else if ( f == "hh" ) {
	    if ( ( am_pm ) && ( dt->hour() > 12 ) )
		buf = QString::number( dt->hour() - 12 ).rightJustify( 2, '0', TRUE );
	    else if ( ( am_pm ) && ( dt->hour() == 0 ) )
		buf = "12";
	    else
		buf = QString::number( dt->hour() ).rightJustify( 2, '0', TRUE );
	} else if ( f == "m" ) {
	    buf = QString::number( dt->minute() );
	} else if ( f == "mm" ) {
	    buf = QString::number( dt->minute() ).rightJustify( 2, '0', TRUE );
	} else if ( f == "s" ) {
	    buf = QString::number( dt->second() );
	} else if ( f == "ss" ) {
	    buf = QString::number( dt->second() ).rightJustify( 2, '0', TRUE );
	} else if ( f == "z" ) {
	    buf = QString::number( dt->msec() );
	} else if ( f == "zzz" ) {
	    buf = QString::number( dt->msec() ).rightJustify( 3, '0', TRUE );
	} else if ( f == "ap" ) {
	    buf = dt->hour() < 12 ? "am" : "pm";
	} else if ( f == "AP" ) {
	    buf = dt->hour() < 12 ? "AM" : "PM";
	}
    }

    if ( dd ) {
	if ( f == "d" ) {
	    buf = QString::number( dd->day() );
	} else if ( f == "dd" ) {
            buf = QString::number( dd->day() ).rightJustify( 2, '0', TRUE );
	} else if ( f == "M" ) {
	    buf = QString::number( dd->month() );
	} else if ( f == "MM" ) {
            buf = QString::number( dd->month() ).rightJustify( 2, '0', TRUE );
#ifndef QT_NO_TEXTDATE
	} else if ( f == "ddd" ) {
            buf = dd->shortDayName( dd->dayOfWeek() );
	} else if ( f == "dddd" ) {
	    buf = dd->longDayName( dd->dayOfWeek() );
	} else if ( f == "MMM" ) {
            buf = dd->shortMonthName( dd->month() );
	} else if ( f == "MMMM" ) {
            buf = dd->longMonthName( dd->month() );
#endif
	} else if ( f == "yy" ) {
            buf = QString::number( dd->year() ).right( 2 );
	} else if ( f == "yyyy" ) {
            buf = QString::number( dd->year() );
	}
    }

    return buf;
}

// Parses the format string and uses getFmtString to get the values for the tokens. Ret
static QString fmtDateTime( const QString& f, const QTime* dt = 0, const QDate* dd = 0 )
{
    if ( f.isEmpty() ) {
	return QString::null;
    }

    bool ap = ( f.contains( "AP" ) || f.contains( "ap" ) );

    QString buf;
    QString frm;
    QChar status = '0';

    for ( int i = 0; i < (int)f.length(); ++i ) {

	if ( f[ i ] == status ) {
	    if ( ( ap ) && ( ( f[ i ] == 'P' ) || ( f[ i ] == 'p' ) ) )
		status = '0';
	    frm += f[ i ];
	} else {
	    buf += getFmtString( frm, dt, dd, ap );
	    frm = QString::null;
            if ( ( f[ i ] == 'h' ) || ( f[ i ] == 'm' ) || ( f[ i ] == 's' ) || ( f[ i ] == 'z' ) ) {
		status = f[ i ];
		frm += f[ i ];
            } else if ( ( f[ i ] == 'd' ) || ( f[ i ] == 'M' ) || ( f[ i ] == 'y' ) ) {
		status = f[ i ];
		frm += f[ i ];
	    } else if ( ( ap ) && ( f[ i ] == 'A' ) ) {
	        status = 'P';
		frm += f[ i ];
	    } else  if( ( ap ) && ( f[ i ] == 'a' ) ) {
	        status = 'p';
		frm += f[ i ];
            } else {
                buf += f[ i ];
                status = '0';
            }
        }
    }

    buf += getFmtString( frm, dt, dd, ap );

    return buf;
}
#endif // QT_NO_DATESTRING

/*****************************************************************************
  QDate member functions
 *****************************************************************************/

// REVISED: aavit

/*!
  \class QDate qdatetime.h
  \brief The QDate class provides date functions.

  \ingroup time
  \mainclass

  A QDate object contains a calendar date, i.e. year, month, and day
  numbers in the modern western (Gregorian) calendar. It can read the
  current date from the system clock. It provides functions for
  comparing dates and for manipulating dates, e.g. by adding a number of
  days or months or years.

  A QDate object is typically created either by giving the year, month
  and day numbers explicitly, or by using the static function
  currentDate(), which makes a QDate object which contains the
  system clock's date. An explicit date can also be set using
  setYMD(). The fromString() function returns a QDate given a string and
  a date format which is used to interpret the date within the string.

  The year(), month(), and day() functions provide access to the year,
  month, and day numbers. Also, dayOfWeek() and dayOfYear() functions
  are provided. The same information is provided in textual format by
  the toString(), shortDayName(), longDayName(), shortMonthName()
  and longMonthName() functions.

  QDate provides a full set of operators to compare two QDate
  objects where smaller means earlier and larger means later.

  You can increment (or decrement) a date by a given number of days
  using addDays(). Similarly you can use addMonths() and addYears().
  The daysTo() function returns the number of days between two dates.

  The daysInMonth() and daysInYear() functions return how many days
  there are in this date's month and year, respectively. The
  leapYear() function indicates whether this date is in a leap year.

  Note that QDate should not be used for date calculations for dates
  prior to the introduction of the Gregorian calendar. This calendar was
  adopted by England from 14<sup><small>th</small></sup> September 1752
  (hence this is the earliest valid QDate), and subsequently by most
  other western countries, until 1923.

  The end of time is reached around 8000, by which time we expect Qt
  to be obsolete.

  \sa QTime QDateTime QDateEdit QDateTimeEdit
*/

/*!
    \enum Qt::DateFormat

    \value TextDate (default) Qt format
    \value ISODate ISO 8601 format
    \value LocalDate locale dependent format
*/


/*!
  \fn QDate::QDate()
  Constructs a null date. Null dates are invalid.

  \sa isNull(), isValid()
*/


/*!
  Constructs a date with year \a y, month \a m and day \a d.

  \a y must be in the range 1752..8000, \a m must be in the range
  1..12, and \a d must be in the range 1..31. Exception: if \a y is in
  the range 0..99, it is interpreted as 1900..1999.

  \sa isValid()
*/

QDate::QDate( int y, int m, int d )
{
    jd = 0;
    setYMD( y, m, d );
}


/*!
  \fn bool QDate::isNull() const

  Returns TRUE if the date is null; otherwise returns FALSE.  A null
  date is invalid.

  \sa isValid()
*/


/*!
  Returns TRUE if this date is valid; otherwise returns FALSE.

  \sa isNull()
*/

bool QDate::isValid() const
{
    return jd >= FIRST_DAY;
}


/*!
  Returns the year (1752..8000) of this date.

  \sa month(), day()
*/

int QDate::year() const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    return y;
}

/*!
  Returns the month (January=1..December=12) of this date.

  \sa year(), day()
*/

int QDate::month() const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    return m;
}

/*!
  Returns the day of the month (1..31) of this date.

  \sa year(), month(), dayOfWeek()
*/

int QDate::day() const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    return d;
}

/*!
  Returns the weekday (Monday=1..Sunday=7) for this date.

  \sa day(), dayOfYear()
*/

int QDate::dayOfWeek() const
{
    return (((jd+1) % 7) + 6)%7 + 1;
}

/*!
  Returns the day of the year (1..365) for this date.

  \sa day(), dayOfWeek()
*/

int QDate::dayOfYear() const
{
    return jd - gregorianToJulian(year(), 1, 1) + 1;
}

/*!
  Returns the number of days in the month (28..31) for this date.

  \sa day(), daysInYear()
*/

int QDate::daysInMonth() const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    if ( m == 2 && leapYear(y) )
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
    julianToGregorian( jd, y, m, d );
    return leapYear(y) ? 366 : 365;
}

/*!
  If it returns TRUE, then the ISO 8601 week number in \a *week and year number in
  \a *year for this date.  Otherwise it returns FALSE, and \a *week and \a *year are
  undefined.

  ISO 8601 defines weeks that start on Monday, and the first week on the first Thursday
  on the year and an implied maximum of 53 weeks.  That's why the year is also returned.
  (For example, 2000 January 1 has week number 52 in the year 1999.  And 2002 December 31
  has week number 1 in the year 2003.)
*/

bool QDate::weekNumber( int *weekNumber, int *yearNumber ) const
{
    int weekNum,
	yearNum,
	jan1WeekDay,
	dow,
	doy,
	currYear;

    if ( !isValid() )
	return FALSE;

    currYear = year();
    jan1WeekDay = QDate( currYear, 1, 1 ).dayOfWeek();
    dow = dayOfWeek();
    doy = dayOfYear();

    // find the Jan1Weekday;
    if ( doy <= ( 8 - jan1WeekDay) && jan1WeekDay > 4 ) {
	yearNum = currYear - 1;
	if ( jan1WeekDay == 5 || (jan1WeekDay == 6 && QDate::leapYear(yearNum)) )
	    weekNum = 53;
	else
	    weekNum = 52;
    } else
	yearNum = currYear;
    if ( yearNum == currYear ) {
	int totalDays = 365;
	if ( QDate::leapYear(yearNum) )
	    totalDays++;
	if ( ((totalDays - doy) < (4 - dow) )
	     || (jan1WeekDay == 7) && (totalDays - doy) < 3) {
	    yearNum++;
	    weekNum = 1;
	}
    }
    if ( yearNum == currYear ) {
	int j = doy + (7 - dow) + (jan1WeekDay - 1);
	weekNum = j / 7;
	if ( jan1WeekDay > 4 )
	    weekNum--;
    }
    *weekNumber = weekNum;
    *yearNumber = yearNum;
    return TRUE;
}

/*!
  \fn QString QDate::monthName( int month )
  \obsolete

  Use shortMonthName() instead.
*/
#ifndef QT_NO_TEXTDATE
/*!
  Returns the name of the \a month.

  1 = "Jan", 2 = "Feb", ... 12 = "Dec"

  The month names will be localized according to the system's locale settings.

  \sa toString(), longMonthName(), shortDayName(), longDayName()
*/

QString QDate::shortMonthName( int month )
{
#if defined(QT_CHECK_RANGE)
    if ( month < 1 || month > 12 ) {
	qWarning( "QDate::shortMonthName: Parameter out ouf range." );
	month = 1;
    }
#endif
#ifndef Q_WS_WIN
    char buffer[255];
    tm tt;
    memset( &tt, 0, sizeof( tm ) );
    tt.tm_mon = month - 1;
    if ( strftime( buffer, sizeof( buffer ), "%b", &tt ) )
	return QString::fromLocal8Bit( buffer );
#else
    SYSTEMTIME st;
    memset( &st, 0, sizeof(SYSTEMTIME) );
    st.wYear = 2000;
    st.wMonth = month;
    st.wDay = 1;
#if defined(UNICODE)
#ifndef Q_OS_TEMP
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	TCHAR buf[255];
	if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, L"MMM", (TCHAR*)&buf, 255 ) )
	    return qt_winQString( &buf );
#ifndef Q_OS_TEMP
    } else
#endif
#endif
#ifndef Q_OS_TEMP
    {
	char buf[255];
	if ( GetDateFormatA( LOCALE_USER_DEFAULT, 0, &st, "MMM", (char*)&buf, 255 ) )
	    return QString::fromLocal8Bit( buf );
    }
#endif
#endif

    return QString::null;
}

/*!
  Returns the long name of the \a month.

  1 = "January", 2 = "February", ... 12 = "December"

  The month names will be localized according to the system's locale settings.

  \sa toString(), shortMonthName(), shortDayName(), longDayName()
*/

QString QDate::longMonthName( int month )
{
#if defined(QT_CHECK_RANGE)
    if ( month < 1 || month > 12 ) {
	qWarning( "QDate::longMonthName: Parameter out ouf range." );
	month = 1;
    }
#endif
#ifndef Q_WS_WIN
    char buffer[255];
    tm tt;
    memset( &tt, 0, sizeof( tm ) );
    tt.tm_mon = month - 1;
    if ( strftime( buffer, sizeof( buffer ), "%B", &tt ) )
	return QString::fromLocal8Bit( buffer );
#else
    SYSTEMTIME st;
    memset( &st, 0, sizeof(SYSTEMTIME) );
    st.wYear = 2000;
    st.wMonth = month;
    st.wDay = 1 ;
#if defined(UNICODE)
#ifndef Q_OS_TEMP
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	TCHAR buf[255];
	if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, L"MMMM", (TCHAR*)&buf, 255 ) )
	    return qt_winQString( &buf );
#ifndef Q_OS_TEMP
    } else
#endif
#endif
#ifndef Q_OS_TEMP
    {
	char buf[255];
	if ( GetDateFormatA( LOCALE_USER_DEFAULT, 0, &st, "MMMM", (char*)&buf, 255 ) )
	    return QString::fromLocal8Bit( buf );
    }
#endif
#endif

    return QString::null;
}

/*!
  \fn QString QDate::dayName( int weekday )
  \obsolete

  Use shortDayName() instead.
*/

/*!
  Returns the name of the \a weekday.

  1 = "Mon", 2 = "Tue", ... 7 = "Sun"

  The day names will be localized according to the system's locale settings.

  \sa toString(), shortMonthName(), longMonthName(), longDayName()
*/

QString QDate::shortDayName( int weekday )
{
#if defined(QT_CHECK_RANGE)
    if ( weekday < 1 || weekday > 7 ) {
	qWarning( "QDate::shortDayName: Parameter out of range." );
	weekday = 1;
    }
#endif
#ifndef Q_WS_WIN
    char buffer[255];
    tm tt;
    memset( &tt, 0, sizeof( tm ) );
    tt.tm_wday = ( weekday == 7 ) ? 0 : weekday;
    if ( strftime( buffer, sizeof( buffer ), "%a", &tt ) )
	return QString::fromLocal8Bit( buffer );
#else
    SYSTEMTIME st;
    memset( &st, 0, sizeof(SYSTEMTIME) );
    st.wYear = 2001;
    st.wMonth = 10;
    st.wDayOfWeek = ( weekday == 7 ) ? 0 : weekday;
    st.wDay = 21 + st.wDayOfWeek;
#if defined(UNICODE)
#ifndef Q_OS_TEMP
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	TCHAR buf[255];
	if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, L"ddd", (TCHAR*)&buf, 255 ) )
	    return qt_winQString( &buf );
#ifndef Q_OS_TEMP
    } else
#endif
#endif
#ifndef Q_OS_TEMP
    {
	char buf[255];
	if ( GetDateFormatA( LOCALE_USER_DEFAULT, 0, &st, "ddd", (char*)&buf, 255 ) )
	    return QString::fromLocal8Bit( buf );
    }
#endif
#endif

    return QString::null;
}

/*!
  Returns the long name of the \a weekday.

  1 = "Monday", 2 = "Tuesday", ... 7 = "Sunday"

  The day names will be localized according to the system's locale settings.

  \sa toString(), shortDayName(), shortMonthName(), longMonthName()
*/

QString QDate::longDayName( int weekday )
{
#if defined(QT_CHECK_RANGE)
    if ( weekday < 1 || weekday > 7 ) {
	qWarning( "QDate::longDayName: Parameter out of range." );
	weekday = 1;
    }
#endif
#ifndef Q_WS_WIN
    char buffer[255];
    tm tt;
    memset( &tt, 0, sizeof( tm ) );
    tt.tm_wday = ( weekday == 7 ) ? 0 : weekday;
    if ( strftime( buffer, sizeof( buffer ), "%A", &tt ) )
	return QString::fromLocal8Bit( buffer );
#else
    SYSTEMTIME st;
    memset( &st, 0, sizeof(SYSTEMTIME) );
    st.wYear = 2001;
    st.wMonth = 10;
    st.wDayOfWeek = ( weekday == 7 ) ? 0 : weekday;
    st.wDay = 21 + st.wDayOfWeek;
#if defined(UNICODE)
#ifndef Q_OS_TEMP
    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	TCHAR buf[255];
	if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, L"dddd", (TCHAR*)&buf, 255 ) )
	    return qt_winQString( &buf );
#ifndef Q_OS_TEMP
    } else
#endif
#endif
#ifndef Q_OS_TEMP
    {
	char buf[255];
	if ( GetDateFormatA( LOCALE_USER_DEFAULT, 0, &st, "dddd", (char*)&buf, 255 ) )
	    return QString::fromLocal8Bit( buf );
    }
#endif
#endif

    return QString::null;
}
#endif //QT_NO_TEXTDATE

#ifndef QT_NO_DATESTRING

#if !defined(QT_NO_SPRINTF)
/*!
    \overload

    Returns the date as a string.  The \a f parameter determines the
  format of the string.

  If \a f is Qt::TextDate, the string format is "Sat May 20 1995" (using
  the shortDayName() and shortMonthName() functions to generate the string).

  If \a f is Qt::ISODate, the string format corresponds to the ISO
  8601 specification for representations of dates, which is YYYY-MM-DD
  where YYYY is the year, MM is the month of the year (between 01 and
  12), and DD is the day of the month between 01 and 31.

  If \a f is Qt::LocalDate, the string format depends on the locale
  settings of the system.

  \sa shortDayName(), shortMonthName()
*/
QString QDate::toString( Qt::DateFormat f ) const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    switch ( f ) {
    case Qt::LocalDate:
	{
#ifndef Q_WS_WIN
	    tm tt;
	    memset( &tt, 0, sizeof( tm ) );
	    char buf[255];
	    tt.tm_mday = day();
	    tt.tm_mon = month() - 1;
	    tt.tm_year = year() - 1900;
	    // Get rid of a warning
	    static const char * egcsWorkaround = "%x";
	    if ( strftime( buf, sizeof(buf), egcsWorkaround, &tt ) )
		return QString::fromLocal8Bit( buf );
#else
	    SYSTEMTIME st;
	    memset( &st, 0, sizeof(SYSTEMTIME) );
	    st.wYear = year();
	    st.wMonth = month();
	    st.wDay = day();
#if defined(UNICODE)
#ifndef Q_OS_TEMP
	    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
		TCHAR buf[255];
		if ( GetDateFormat( LOCALE_USER_DEFAULT, 0, &st, 0, (TCHAR*)&buf, 255 ) )
		    return qt_winQString( &buf );
#ifndef Q_OS_TEMP
	    } else
#endif
#endif
#ifndef Q_OS_TEMP
	    {
		char buf[255];
		if ( GetDateFormatA( LOCALE_USER_DEFAULT, 0, &st, 0, (char*)&buf, 255 ) )
		    return QString::fromLocal8Bit( buf );
	    }
#endif
#endif
	    return QString::null;
	}
#ifdef QT_NO_TEXTDATE
    default:
#endif
    case Qt::ISODate:
	{
	    QString month( QString::number( m ).rightJustify( 2, '0' ) );
	    QString day( QString::number( d ).rightJustify( 2, '0' ) );
	    return QString::number( y ) + "-" + month + "-" + day;
	}
#ifndef QT_NO_TEXTDATE
    default:
    case Qt::TextDate:
	{
#ifndef Q_WS_WIN
	    QString buf = shortDayName( dayOfWeek() );
	    buf += ' ';
	    buf += shortMonthName(m);
	    QString t;
	    t.sprintf( " %d %d", d, y);
	    buf += t;
	    return buf;
#else
	    QString buf;
	    QString winstr;
#if defined(UNICODE)
#ifndef Q_OS_TEMP
	    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
		TCHAR out[255];
		GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_ILDATE, (TCHAR*)&out, 255 );
		winstr = qt_winQString( &out );
#ifndef Q_OS_TEMP
	    } else
#endif
#endif
#ifndef Q_OS_TEMP
	    {
		char out[255];
		GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_ILDATE, (char*)&out, 255 );
		winstr = QString::fromLocal8Bit( out );
	    }
#endif
	    switch ( winstr.toInt() ) {
	    case 1:
		buf = shortDayName( dayOfWeek() ) + " " + QString().setNum( d ) + ". " + shortMonthName( m ) + " " + QString().setNum( y );
		break;
	    case 2:
		buf = shortDayName( dayOfWeek() ) + " " + QString().setNum( y ) + " " + shortMonthName( m ) + " " + QString().setNum( d );
		break;
	    default:
		buf = shortDayName( dayOfWeek() ) + " " + shortMonthName( m ) + " " + QString().setNum( d ) + " " + QString().setNum( y );
		break;
	    }

	    return buf;
#endif
	}
#endif
    }
}
#endif //QT_NO_SPRINTF

/*!  Returns the datetime as a string.  The \a format parameter determines
  the format of the result string.

  These expressions may be used:

  \list
  \i \e d - the day as number without a leading zero (1-31)
  \i \e dd - the day as number with a leading zero (01-31)
  \i \e ddd - the abbrevated day name (Mon - Sun). Uses QDate::shortDayName().
  \i \e dddd - the long day name (Monday - Sunday). Uses QDate::longDayName().
  \i \e M - the month as number without a leading zero (1-12)
  \i \e MM - the month as number with a leading zero (01-12)
  \i \e MMM - the abbrevated month name (Jan - Dec). Uses QDate::shortMonthName().
  \i \e MMMM - the long month name (January - December). Uses QDate::longMonthName().
  \i \e yy - the year as two digit number (00-99)
  \i \e yyyy - the year as four digit number (0000-9999)
  \endlist

  All other input characters will be ignored.

  Example format Strings (assumed that the QDate is 21. May 2001)

  \list
  \i "dd.MM.yyyy" will result in "21.05.2001"
  \i "ddd MMMM d yy" will result in "Tue May 21 01"
  \endlist

  \sa QDate::toString() QTime::toString()

*/
QString QDate::toString( const QString& format ) const
{
    return fmtDateTime( format, 0, this );
}
#endif //QT_NO_DATESTRING

/*!
  Sets the date's year \a y, month \a m and day \a d.

  \a y must be in the range 1752..8000, \a m must be in the range
  1..12, and \a d must be in the range 1..31. Exception: if \a y is in
  the range 0..99, it is interpreted as 1900..1999.

  Returns TRUE if the date is valid, otherwise returns FALSE.
*/

bool QDate::setYMD( int y, int m, int d )
{
    if ( year() == y && month() == m && day() == d )
	return isValid();
    if ( !isValid(y,m,d) ) {
#if defined(QT_CHECK_RANGE)
	 qWarning( "QDate::setYMD: Invalid date %04d/%02d/%02d", y, m, d );
#endif
	 return FALSE;
    }
    jd = gregorianToJulian( y, m, d );
    return TRUE;
}

/*!
  Returns a QDate object containing a date \a ndays later than the
  date of this object (or earlier if \a ndays is negative).

  \sa daysTo()
*/

QDate QDate::addDays( int ndays ) const
{
    QDate d;
    d.jd = jd + ndays;
    return d;
}

/*!
  Returns a QDate object containing a date \a nmonths later than the
  date of this object (or earlier if \a nmonths is negative).

*/

QDate QDate::addMonths( int nmonths ) const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );

    while ( nmonths != 0 ) {
	if ( nmonths < 0 && nmonths + 12 <= 0 ) {
	    y--;
	    nmonths+=12;
	} else if ( nmonths < 0 ) {
	    m+= nmonths;
	    nmonths = 0;
	    if ( m <= 0 ) {
		--y;
		m+=12;
	    }
	} else if ( nmonths - 12 >= 0 ) {
	    y++;
	    nmonths-=12;
	} else if ( m == 12 ) {
	    y++;
	    m = 0;
	} else {
	    m+= nmonths;
	    nmonths = 0;
	    if ( m > 12 ) {
		++y;
		m -= 12;
	    }
	}
    }

    QDate tmp(y,m,1);

    if( d > tmp.daysInMonth() )
	d = tmp.daysInMonth();

    QDate date(y, m, d);
    return date;

}

/*!
  Returns a QDate object containing a date \a nyears later than the
  date of this object (or earlier if \a nyears is negative).

*/

QDate QDate::addYears( int nyears ) const
{
    int y, m, d;
    julianToGregorian( jd, y, m, d );
    y += nyears;
    QDate date(y, m, d);
    return date;
}



/*!
  Returns the number of days from this date to \a d (which is negative
  if \a d is earlier than this date).

  Example:
  \code
    QDate d1( 1995, 5, 17 );  // May 17th 1995
    QDate d2( 1995, 5, 20 );  // May 20th 1995
    d1.daysTo( d2 );          // returns 3
    d2.daysTo( d1 );          // returns -3
  \endcode

  \sa addDays()
*/

int QDate::daysTo( const QDate &d ) const
{
    return d.jd - jd;
}


/*!
  \fn bool QDate::operator==( const QDate &d ) const
  Returns TRUE if this date is equal to \a d; otherwise returns FALSE.
*/

/*!
  \fn bool QDate::operator!=( const QDate &d ) const
  Returns TRUE if this date is different from \a d; otherwise returns FALSE.
*/

/*!
  \fn bool QDate::operator<( const QDate &d ) const
  Returns TRUE if this date is earlier than \a d, otherwise returns FALSE.
*/

/*!
  \fn bool QDate::operator<=( const QDate &d ) const
  Returns TRUE if this date is earlier than or equal to \a d, otherwise returns FALSE.
*/

/*!
  \fn bool QDate::operator>( const QDate &d ) const
  Returns TRUE if this date is later than \a d, otherwise returns FALSE.
*/

/*!
  \fn bool QDate::operator>=( const QDate &d ) const
  Returns TRUE if this date is later than or equal to \a d, otherwise returns FALSE.
*/


/*!
  Returns the current date, as reported by the system clock.

  \sa QTime::currentTime(), QDateTime::currentDateTime()
*/

QDate QDate::currentDate()
{
#if defined(Q_OS_WIN32)

    SYSTEMTIME t;
    memset( &t, 0, sizeof(SYSTEMTIME) );
    GetLocalTime( &t );
    QDate d;
    d.jd = gregorianToJulian( t.wYear, t.wMonth, t.wDay );
    return d;

#else

    time_t ltime;
    time( &ltime );
    tm *t = localtime( &ltime );
    QDate d;
    d.jd = gregorianToJulian( t->tm_year + 1900, t->tm_mon + 1, t->tm_mday );
    return d;

#endif
}
#ifndef QT_NO_DATESTRING
/*!
  Returns the QDate represented by the string \a s, using the format \a
  f, or an invalid date if this is not possible.

  Qt::LocalDate cannot be used here.

  Note for Qt::TextDate: It is recommended to use the English short
  month names (e.g. Jan). Localized month names may also be used, but
  they depend on the user's locale settings.
*/
QDate QDate::fromString( const QString& s, Qt::DateFormat f )
{
    if ( ( s.isEmpty() ) || ( f == Qt::LocalDate ) ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QDate::fromString: Parameter out of range." );
#endif
    	return QDate();
    }
    switch ( f ) {
    case Qt::ISODate:
	{
	    int year( s.mid( 0, 4 ).toInt() );
	    int month( s.mid( 5, 2 ).toInt() );
	    int day( s.mid( 8, 2 ).toInt() );
	    if ( year && month && day )
		return QDate( year, month, day );
	}
	break;
    default:
#ifndef QT_NO_TEXTDATE
    case Qt::TextDate:
	{
	    QString monthName( s.mid( 4, 3 ) );
	    int month = -1;
	    // Assume that English monthnames are the default
	    for ( int i = 0; i < 12; ++i ) {
		if ( monthName == qt_shortMonthNames[i] ) {
		    month = i + 1;
		    break;
		}
	    }
	    // If English names can't be found, search the localized ones
	    if ( month == -1 ) {
		for ( int i = 1; i <= 12; ++i ) {
		    if ( monthName == shortMonthName( i ) ) {
			month = i;
			break;
		    }
		}
	    }
#if defined(QT_CHECK_RANGE)
    	    if ( month < 1 || month > 12 ) {
		qWarning( "QDate::fromString: Parameter out of range." );
		month = 1;
	    }
#endif
	    int day = s.mid( 8, 2 ).simplifyWhiteSpace().toInt();
	    int year = s.right( 4 ).toInt();
	    return QDate( year, month, day );
	}
#else
	break;
#endif
    }
    return QDate();
}
#endif //QT_NO_DATESTRING

/*!
    \overload

  Returns TRUE if the specified date (year \a y, month \a m and day \a
  d) is valid.

  Example:
  \code
    QDate::isValid( 2002, 5, 17 );  // TRUE   May 17th 2002 is valid
    QDate::isValid( 2002, 2, 30 );  // FALSE  Feb 30th does not exist
    QDate::isValid( 2004, 2, 29 );  // TRUE   2004 is a leap year
    QDate::isValid( 1202, 6, 6 );   // FALSE  1202 is pre-Gregorian
  \endcode

  Note that a \a y value in the range 00..99 is interpreted as
  1900..1999.

  \sa isNull(), setYMD()
*/

bool QDate::isValid( int y, int m, int d )
{
    if ( y >= 0 && y <= 99 )
	y += 1900;
    else if ( y < FIRST_YEAR || (y == FIRST_YEAR && (m < 9 ||
						    (m == 9 && d < 14))) )
	return FALSE;
    return (d > 0 && m > 0 && m <= 12) &&
	   (d <= monthDays[m] || (d == 29 && m == 2 && leapYear(y)));
}

/*!
  Returns TRUE if the specified year \a y is a leap year.
*/

bool QDate::leapYear( int y )
{
    return y % 4 == 0 && y % 100 != 0 || y % 400 == 0;
}

/*!
  \internal
  Converts a Gregorian date to a Julian day.
  This algorithm is taken from Communications of the ACM, Vol 6, No 8.
  \sa julianToGregorian()
*/

uint QDate::gregorianToJulian( int y, int m, int d )
{
    uint c, ya;
    if ( y <= 99 )
	y += 1900;
    if ( m > 2 ) {
	m -= 3;
    } else {
	m += 9;
	y--;
    }
    c = y;					// NOTE: Sym C++ 6.0 bug
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

void QDate::julianToGregorian( uint jd, int &y, int &m, int &d )
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
    if ( m < 10 ) {
	m += 3;
    } else {
	m -= 9;
	y++;
    }
}


/*****************************************************************************
  QTime member functions
 *****************************************************************************/

/*!
  \class QTime qdatetime.h

  \brief The QTime class provides clock time functions.

  \ingroup time
  \mainclass

  A QTime object contains a clock time, i.e. the number of hours,
  minutes, seconds, and milliseconds since midnight. It can read the
  current time from the system clock and measure a span of elapsed
  time. It provides functions for comparing times and for manipulating
  a time by adding a number of (milli)seconds.

  QTime operates with 24-hour clock format; it has no concept of
  AM/PM. It operates in local time; it knows nothing about
  time zones or daylight savings time.

  A QTime object is typically created either by giving the number of
  hours, minutes, seconds, and milliseconds explicitly, or by using
  the static function currentTime(), which makes a QTime object that
  contains the system's clock time. Note that the accuracy depends on
  the accuracy of the underlying operating system; not all systems
  provide 1-millisecond accuracy.

  The hour(), minute(), second(), and msec() functions provide access
  to the number of hours, minutes, seconds, and milliseconds of the
  time. The same information is provided in textual format by the
  toString() function.

  QTime provides a full set of operators to compare two QTime
  objects. One time is considered smaller than another if it is earlier
  than the other.

  The time a given number of seconds or milliseconds later than a
  given time can be found using the addSecs() or addMSecs()
  functions. Correspondingly, the number of (milli)seconds between two
  times can be found using the secsTo() or msecsTo() functions.

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

  \a h must be in the range 0..23, \a m and \a s must be in the range
  0..59, and \a ms must be in the range 0..999.

  \sa isValid()
*/

QTime::QTime( int h, int m, int s, int ms )
{
    setHMS( h, m, s, ms );
}


/*!
  \fn bool QTime::isNull() const
  Returns TRUE if the time is equal to 00:00:00.000; otherwise returns FALSE.
  A null time is valid.

  \sa isValid()
*/

/*!
  Returns TRUE if the time is valid; otherwise returns FALSE.
  The time 23:30:55.746 is valid, whereas 24:12:30 is invalid.

  \sa isNull()
*/

bool QTime::isValid() const
{
    return ds < MSECS_PER_DAY;
}


/*!
  Returns the hour part (0..23) of the time.
*/

int QTime::hour() const
{
    return ds / MSECS_PER_HOUR;
}

/*!
  Returns the minute part (0..59) of the time.
*/

int QTime::minute() const
{
    return (ds % MSECS_PER_HOUR)/MSECS_PER_MIN;
}

/*!
  Returns the second part (0..59) of the time.
*/

int QTime::second() const
{
    return (ds / 1000)%SECS_PER_MIN;
}

/*!
  Returns the millisecond part (0..999) of the time.
*/

int QTime::msec() const
{
    return ds % 1000;
}

#ifndef QT_NO_DATESTRING
#ifndef QT_NO_SPRINTF
/*!
    \overload
    Returns the time as a string.  Milliseconds are not included.
  The \a f parameter determines the format of the string.

  If \a f is Qt::TextDate, the string format is HH:MM:SS; e.g. 1
  second before midnight would be "23:59:59".

  If \a f is Qt::ISODate, the string format corresponds to the ISO
  8601 specification for representations of dates, which is also
  HH:MM:SS.

  If \a f is Qt::LocalDate, the string format depends on the locale
  settings of the system.
*/

QString QTime::toString( Qt::DateFormat f ) const
{
    switch ( f ) {
    case Qt::LocalDate:
	{
#ifndef Q_WS_WIN
	    tm tt;
	    memset( &tt, 0, sizeof( tm ) );
	    char buf[255];
	    tt.tm_sec = second();
	    tt.tm_min = minute();
	    tt.tm_hour = hour();
	    if ( strftime( buf, sizeof(buf), "%X", &tt ) )
		return QString::fromLocal8Bit( buf );
#else
	    SYSTEMTIME st;
	    memset( &st, 0, sizeof(SYSTEMTIME) );
	    st.wHour = hour();
	    st.wMinute = minute();
	    st.wSecond = second();
	    st.wMilliseconds = 0;
#if defined(UNICODE)
#ifndef Q_OS_TEMP
	    if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
		TCHAR buf[255];
		if ( GetTimeFormat( LOCALE_USER_DEFAULT, 0, &st, 0, (TCHAR*)&buf, 255 ) )
		    return qt_winQString( &buf );
#ifndef Q_OS_TEMP
	    } else
#endif
#endif
#ifndef Q_OS_TEMP
	    {
		char buf[255];
		if ( GetTimeFormatA( LOCALE_USER_DEFAULT, 0, &st, 0, (char*)&buf, 255 ) )
		    return QString::fromLocal8Bit( buf );
	    }
#endif
#endif
	    return QString::null;
	}
    default:
    case Qt::ISODate:
    case Qt::TextDate:
	QString buf;
	buf.sprintf( "%.2d:%.2d:%.2d", hour(), minute(), second() );
	return buf;
    }
}
#endif

/*!  Returns the time as a string.  The \a format parameter determines
  the format of the result string.

  These expressions may be used:

  \list
  \i \e h - the hour without a leading zero (0..23 or 1..12 if AM/PM display)
  \i \e hh - the hour with a leading zero (00..23 or 01..12 if AM/PM display)
  \i \e m - the minute without a leading zero (0..59)
  \i \e mm - the minute with a leading zero (00..59)
  \i \e s - the second whithout a leading zero (0..59)
  \i \e ss - the second whith a leading zero (00..59)
  \i \e z - the milliseconds without leading zeroes (0..999)
  \i \e zzz - the milliseconds with leading zeroes (000..999)
  \i \e AP - switch to AM/PM display. \e AP will be replaced by either "AM" or "PM".
  \i \e ap - switch to am/pm display. \e ap will be replaced by either "am" or "pm".
  \endlist

  All other input characters will be ignored.

  Example format Strings (assuming that the QTime is 14:13:09.042)

  \list
  \i "hh:mm:ss.zzz"  will result in "14:13:09.042"
  \i "h:m:s ap" will result in "2:13:9 pm"
  \endlist

  \sa QDate::toString() QTime::toString()

*/
QString QTime::toString( const QString& format ) const
{
    return fmtDateTime( format, this, 0 );
}
#endif //QT_NO_DATESTRING
/*!
  Sets the time to hour \a h, minute \a m, seconds \a s and
  milliseconds \a ms.

  \a h must be in the range 0..23, \a m and \a s must be in the range
  0..59, and \a ms must be in the range 0..999. Returns TRUE if the set
  time is valid; otherwise returns FALSE.

  \sa isValid()
*/

bool QTime::setHMS( int h, int m, int s, int ms )
{
    if ( !isValid(h,m,s,ms) ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QTime::setHMS Invalid time %02d:%02d:%02d.%03d", h, m, s,
		 ms );
#endif
	ds = MSECS_PER_DAY;		// make this invalid
	return FALSE;
    }
    ds = (h*SECS_PER_HOUR + m*SECS_PER_MIN + s)*1000 + ms;
    return TRUE;
}

/*!
  Returns a QTime object containing a time \a nsecs seconds later than
  the time of this object (or earlier if \a nsecs is negative).

  Note that the time will wrap if it passes midnight.

  Example:
  \code
    QTime n( 14, 0, 0 );                // n == 14:00:00
    QTime t;
    t = n.addSecs( 70 );                // t == 14:01:10
    t = n.addSecs( -70 );               // t == 13:58:50
    t = n.addSecs( 10*60*60 + 5 );      // t == 00:00:05
    t = n.addSecs( -15*60*60 );         // t == 23:00:00
  \endcode

  \sa addMSecs(), secsTo(), QDateTime::addSecs()
*/

QTime QTime::addSecs( int nsecs ) const
{
    return addMSecs( nsecs * 1000 );
}

/*!
  Returns the number of seconds from this time to \a t (which is
  negative if \a t is earlier than this time).

  Because QTime measures time within a day and there are 86400 seconds
  in a day, the result is between -86400 and 86400.

  \sa addSecs() QDateTime::secsTo()
*/

int QTime::secsTo( const QTime &t ) const
{
    return ((int)t.ds - (int)ds)/1000;
}

/*!
  Returns a QTime object containing a time \a ms milliseconds later than
  the time of this object (or earlier if \a ms is negative).

  Note that the time will wrap if it passes midnight. See addSecs()
  for an example.

  \sa addSecs(), msecsTo()
*/

QTime QTime::addMSecs( int ms ) const
{
    QTime t;
    if ( ms < 0 ) {
	// % not well-defined for -ve, but / is.
	int negdays = (MSECS_PER_DAY-ms) / MSECS_PER_DAY;
	t.ds = ((int)ds + ms + negdays*MSECS_PER_DAY)
		% MSECS_PER_DAY;
    } else {
	t.ds = ((int)ds + ms) % MSECS_PER_DAY;
    }
    return t;
}

/*!
  Returns the number of milliseconds from this time to \a t (which is
  negative if \a t is earlier than this time).

  Because QTime measures time within a day and there are 86400
  seconds in a day, the result is between -86400 and 86400s.

  \sa secsTo()
*/

int QTime::msecsTo( const QTime &t ) const
{
    return (int)t.ds - (int)ds;
}


/*!
  \fn bool QTime::operator==( const QTime &t ) const

  Returns TRUE if this time is equal to \a t; otherwise returns FALSE.
*/

/*!
  \fn bool QTime::operator!=( const QTime &t ) const

  Returns TRUE if this time is different from \a t; otherwise returns FALSE.
*/

/*!
  \fn bool QTime::operator<( const QTime &t ) const

  Returns TRUE if this time is earlier than \a t; otherwise returns FALSE.
*/

/*!
  \fn bool QTime::operator<=( const QTime &t ) const

  Returns TRUE if this time is earlier than or equal to \a t;
  otherwise returns FALSE.
*/

/*!
  \fn bool QTime::operator>( const QTime &t ) const

  Returns TRUE if this time is later than \a t; otherwise returns FALSE.
*/

/*!
  \fn bool QTime::operator>=( const QTime &t ) const

  Returns TRUE if this time is later than or equal to \a t; otherwise
  returns FALSE.
*/



/*!
  Returns the current time as reported by the system clock.

  Note that the accuracy depends on the accuracy of the underlying
  operating system; not all systems provide 1-millisecond accuracy.
*/

QTime QTime::currentTime()
{
    QTime ct;
    currentTime( &ct );
    return ct;
}

#ifndef QT_NO_DATESTRING
/*!
  Returns the representation \a s as a QTime using the format \a f, or
  an invalid time if this is not possible.

  Note that Qt::LocalDate cannot be used here.
 */
QTime QTime::fromString( const QString& s, Qt::DateFormat f )
{
    if ( ( s.isEmpty() ) || ( f == Qt::LocalDate ) ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QTime::fromString: Parameter out of range." );
#endif
	return QTime();
    }

    int hour( s.mid( 0, 2 ).toInt() );
    int minute( s.mid( 3, 2 ).toInt() );
    int second( s.mid( 6, 2 ).toInt() );
    int msec( s.mid( 9, 3 ).toInt() );
    return QTime( hour, minute, second, msec );
}
#endif

/*!
  \internal

  Fetches the current time and returns TRUE if the time is within one
  minute after midnight, otherwise FALSE. The return value is used by
  QDateTime::currentDateTime() to ensure that the date there is correct.
*/

bool QTime::currentTime( QTime *ct )
{
    if ( !ct ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QTime::currentTime(QTime *): Null pointer not allowed" );
#endif
	return FALSE;
    }

#if defined(Q_OS_WIN32)

    SYSTEMTIME t;
    GetLocalTime( &t );
    ct->ds = MSECS_PER_HOUR*t.wHour + MSECS_PER_MIN*t.wMinute +
	     1000*t.wSecond + t.wMilliseconds;
    return (t.wHour == 0 && t.wMinute == 0);

#elif defined(Q_OS_OS2)

    DATETIME t;
    DosGetDateTime( &t );
    ct->ds = MSECS_PER_HOUR*t.hours + MSECS_PER_MIN*t.minutes +
	     1000*t.seconds + 10*t.hundredths;
    return (t.hours == 0 && t.minutes == 0);

#elif defined(Q_OS_MSDOS)

    _dostime_t t;
    _dos_gettime( &t );
    ct->ds = MSECS_PER_HOUR*t.hour + MSECS_PER_MIN*t.minute +
	     t.second*1000 + t.hsecond*10;
    return (t.hour== 0 && t.minute == 0);

#elif defined(Q_OS_UNIX)

    struct timeval tv;
    gettimeofday( &tv, 0 );
    time_t ltime = tv.tv_sec;
    tm *t = localtime( &ltime );
    ct->ds = (uint)( MSECS_PER_HOUR*t->tm_hour + MSECS_PER_MIN*t->tm_min +
		     1000*t->tm_sec + tv.tv_usec/1000 );
    return (t->tm_hour== 0 && t->tm_min == 0);

#else

    time_t ltime;			// no millisecond resolution!!
    ::time( &ltime );
    tm *t = localtime( &ltime );
    ct->ds = MSECS_PER_HOUR*t->tm_hour + MSECS_PER_MIN*t->tm_min +
	     1000*t->tm_sec;
    return (t->tm_hour== 0 && t->tm_min == 0);
#endif
}

/*!
    \overload
  Returns TRUE if the specified time is valid; otherwise returns FALSE.

  The time is valid if \a h is in the range 0..23, \a m and \a s are in
  the range 0..59, and \a ms is in the range 0..999.

  Example:
  \code
    QTime::isValid(21, 10, 30); // returns TRUE
    QTime::isValid(22, 5,  62); // returns FALSE
  \endcode
*/

bool QTime::isValid( int h, int m, int s, int ms )
{
    return (uint)h < 24 && (uint)m < 60 && (uint)s < 60 && (uint)ms < 1000;
}


/*!
  Sets this time to the current time. This is practical for timing:

  \code
    QTime t;
    t.start();
    some_lengthy_task();
    qDebug( "%d milliseconds", t.elapsed() );
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

  This function is guaranteed to be atomic and is thus very handy for
  repeated measurements. Call start() to start the first measurement and
  then restart() for each later measurement.

  Note that the counter wraps to zero 24 hours after the last call to
  start() or restart().

  \warning If the system's clock setting has been changed since the
  last time start() or restart() was called, the result is undefined.
  This can happen when daylight savings time is turned on or off.

  \sa start(), elapsed(), currentTime()
*/

int QTime::restart()
{
    QTime t = currentTime();
    int n = msecsTo( t );
    if ( n < 0 )				// passed midnight
	n += 86400*1000;
    *this = t;
    return n;
}

/*!
  Returns the number of milliseconds that have elapsed since the last
  time start() or restart() was called.

  Note that the counter wraps to zero 24 hours after the last call to
  start() or restart.

  Note that the accuracy depends on the accuracy of the underlying
  operating system; not all systems provide 1-millisecond accuracy.

  \warning If the system's clock setting has been changed since the
  last time start() or restart() was called, the result is undefined.
  This can happen when daylight savings time is turned on or off.

  \sa start(), restart()
*/

int QTime::elapsed() const
{
    int n = msecsTo( currentTime() );
    if ( n < 0 )				// passed midnight
	n += 86400*1000;
    return n;
}


/*****************************************************************************
  QDateTime member functions
 *****************************************************************************/

/*!
  \class QDateTime qdatetime.h
  \brief The QDateTime class provides date and time functions.

  \ingroup time
  \mainclass

  A QDateTime object contains a calendar date and a clock time (a
  "datetime"). It is a combination of the QDate and QTime classes. It
  can read the current datetime from the system clock. It provides
  functions for comparing datetimes and for manipulating a datetime by
  adding a number of seconds, days, months or years.

  A QDateTime object is typically created either by giving a date and
  time explicitly in the constructor, or by using the static function
  currentDateTime(), which returns a QDateTime object set to the system
  clock's time. The date and time can be changed with setDate() and
  setTime(). A datetime can also be set using the setTime_t() function,
  which takes a POSIX-standard "number of seconds since 00:00:00 on
  January 1, 1970" value. The fromString() function returns a QDate
  given a string and a date format which is used to interpret the date
  within the string.

  The date() and time() functions provide access to the date and time
  parts of the datetime. The same information is provided in textual
  format by the toString() function.

  QDateTime provides a full set of operators to compare two QDateTime
  objects where smaller means earlier and larger means later.

    You can increment (or decrement) a datetime by a given number of
    seconds using addSecs() or days using addDays(). Similarly you can
    use addMonths() and addYears(). The daysTo() function returns the
    number of days between two datetimes, and sectTo() returns the
    number of seconds between two datetimes.

  The range of a datetime object is constrained to the ranges of the
  QDate and QTime objects which it embodies.

  \sa QDate QTime QDateTimeEdit
*/


/*!
  \fn QDateTime::QDateTime()

  Constructs a null datetime (i.e. null date and null time).  A null
  datetime is invalid, since the date is invalid.

  \sa isValid()
*/


/*!
  Constructs a datetime with date \a date and null time (00:00:00.000).
*/

QDateTime::QDateTime( const QDate &date )
    : d(date)
{
}

/*!
  Constructs a datetime with date \a date and time \a time.
*/

QDateTime::QDateTime( const QDate &date, const QTime &time )
    : d(date), t(time)
{
}


/*!
  \fn bool QDateTime::isNull() const

  Returns TRUE if both the date and the time are null; otherwise returns
  FALSE. A null datetime is invalid.

  \sa QDate::isNull(), QTime::isNull()
*/

/*!
  \fn bool QDateTime::isValid() const

  Returns TRUE if both the date and the time are valid; otherwise
  returns FALSE.

  \sa QDate::isValid(), QTime::isValid()
*/

/*!
  \fn QDate QDateTime::date() const

  Returns the date part of the datetime.

  \sa setDate(), time()
*/

/*!
  \fn QTime QDateTime::time() const

  Returns the time part of the datetime.

  \sa setTime(), date()
*/

/*!
  \fn void QDateTime::setDate( const QDate &date )

  Sets the date part of this datetime to \a date.

  \sa date(), setTime()
*/

/*!
  \fn void QDateTime::setTime( const QTime &time )

  Sets the time part of this datetime to \a time.

  \sa time(), setDate()
*/


/*!
  Sets the date and time to local time given the number of seconds that
  have passed since 00:00:00 on January 1, 1970, Coordinated Universal
  Time (UTC). On systems that do not support timezones this function
  will behave as if local time were UTC.

  Note that Microsoft Windows supports only a limited range of values for
  \a secsSince1Jan1970UTC.
*/

void QDateTime::setTime_t( uint secsSince1Jan1970UTC )
{
    time_t tmp = (time_t) secsSince1Jan1970UTC;
    tm *tM = localtime( &tmp );
    if ( !tM ) {
	tM = gmtime( &tmp );
	if ( !tM ) {
	    d.jd = QDate::gregorianToJulian( 1970, 1, 1 );
	    t.ds = 0;
	    return;
	}
    }
    d.jd = QDate::gregorianToJulian( tM->tm_year + 1900, tM->tm_mon + 1, tM->tm_mday );
    t.ds = MSECS_PER_HOUR*tM->tm_hour + MSECS_PER_MIN*tM->tm_min +
	    1000*tM->tm_sec;
}

#ifndef QT_NO_DATESTRING
#ifndef QT_NO_SPRINTF
/*!
    \overload

    Returns the datetime as a string.  The \a f parameter determines
  the format of the string.

  If \a f is Qt::TextDate, the string format is "Wed May 20 03:40:13
  1998" (using QDate::shortDayName(), QDate::shortMonthName(), and
  QTime::toString() to generate the string).

  If \a f is Qt::ISODate, the string format corresponds to the ISO
  8601 specification for representations of dates and times, which is
  YYYY-MM-DDTHH:MM:SS.

  If \a f is Qt::LocalDate, the string format depends on the locale
  settings of the system.

  If the format \a f is invalid, toString() returns a null string.

  \sa QDate::toString() QTime::toString()

*/

QString QDateTime::toString( Qt::DateFormat f ) const
{
    if ( f == Qt::ISODate ) {
	return d.toString( Qt::ISODate ) + "T" + t.toString( Qt::ISODate );
    }
#ifndef QT_NO_TEXTDATE
    else if ( f == Qt::TextDate ) {
#ifndef Q_WS_WIN
	QString buf = d.shortDayName( d.dayOfWeek() );
	buf += ' ';
	buf += d.shortMonthName( d.month() );
	buf += ' ';
	buf += QString().setNum( d.day() );
	buf += ' ';
#else
	QString buf;
	QString winstr;
#if defined(UNICODE)
#ifndef Q_OS_TEMP
	if ( qWinVersion() & Qt::WV_NT_based ) {
#endif
	    TCHAR out[255];
	    GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_ILDATE, (TCHAR*)&out, 255 );
	    winstr = qt_winQString( &out );
#ifndef Q_OS_TEMP
	} else
#endif
#endif
#ifndef Q_OS_TEMP
	{
	    char out[255];
	    GetLocaleInfoA( LOCALE_USER_DEFAULT, LOCALE_ILDATE, (char*)&out, 255 );
	    winstr = QString::fromLocal8Bit( out );
	}
#endif
	switch ( winstr.toInt() ) {
	case 1:
	    buf = d.shortDayName( d.dayOfWeek() ) + " " + QString().setNum( d.day() ) + ". " + d.shortMonthName( d.month() ) + " ";
	    break;
	default:
	    buf = d.shortDayName( d.dayOfWeek() ) + " " + d.shortMonthName( d.month() ) + " " + QString().setNum( d.day() ) + " ";
	    break;
	}
#endif
	buf += t.toString();
	buf += ' ';
	buf += QString().setNum( d.year() );
	return buf;
    }
#endif
    else if ( f == Qt::LocalDate ) {
	return d.toString( Qt::LocalDate ) + " " + t.toString( Qt::LocalDate );
    }
    return QString::null;
}
#endif

/*!  Returns the datetime as a string.  The \a format parameter determines
  the format of the result string.

  These expressions may be used for the date:

  \list
  \i \e d - the day as number without a leading zero (1-31)
  \i \e dd - the day as number with a leading zero (01-31)
  \i \e ddd - the abbrevated day name (Mon - Sun). Uses QDate::shortDayName().
  \i \e dddd - the long day name (Monday - Sunday). Uses QDate::longDayName().
  \i \e M - the month as number without a leading zero (1-12)
  \i \e MM - the month as number with a leading zero (01-12)
  \i \e MMM - the abbrevated month name (Jan - Dec). Uses QDate::shortMonthName().
  \i \e MMMM - the long month name (January - December). Uses QDate::longMonthName().
  \i \e yy - the year as two digit number (00-99)
  \i \e yyyy - the year as four digit number (0000-9999)
  \endlist

  These expressions may be used for the time:

  \list
  \i \e h - the hour without a leading zero (0-23 or 1-12 if AM/PM display)
  \i \e hh - the hour with a leading zero (00-23 or 01-12 if AM/PM display)
  \i \e m - the minute without a leading zero (0-59)
  \i \e mm - the minute with a leading zero (00-59)
  \i \e s - the second whithout a leading zero (0-59)
  \i \e ss - the second whith a leading zero (00-59)
  \i \e z - the milliseconds without leading zeroes (0-999)
  \i \e zzz - the milliseconds with leading zeroes (000-999)
  \i \e AP - switch to AM/PM display. \e AP will be replaced by either "AM" or "PM".
  \i \e ap - switch to AM/PM display. \e ap will be replaced by either "am" or "pm".
  \endlist

  All other input characters will be ignored.

  Example format Strings (assumed that the QDateTime is 21. May 2001 14:13:09)

  \list
  \i "dd.MM.yyyy" will result in "21.05.2001"
  \i "ddd MMMM d yy" will result in "Tue May 21 01"
  \i "hh:mm:ss.zzz"  will result in "14:13:09.042"
  \i "h:m:s ap" will result in "2:13:9 pm"
  \endlist

  \sa QDate::toString() QTime::toString()

*/
QString QDateTime::toString( const QString& format ) const
{
    return fmtDateTime( format, &t, &d );
}
#endif //QT_NO_DATESTRING

/*!
  Returns a QDateTime object containing a datetime \a ndays days later
  than the datetime of this object (or earlier if \a ndays is
  negative).

  \sa daysTo(), addMonths(), addYears(), addSecs()
*/

QDateTime QDateTime::addDays( int ndays ) const
{
    return QDateTime( d.addDays(ndays), t );
}

/*!
  Returns a QDateTime object containing a datetime \a nmonths months later
  than the datetime of this object (or earlier if \a nmonths is
  negative).

  \sa daysTo(), addDays(), addYears(), addSecs()
*/

QDateTime QDateTime::addMonths( int nmonths ) const
{
    return QDateTime( d.addMonths(nmonths), t );
}

/*!
  Returns a QDateTime object containing a datetime \a nyears years later
  than the datetime of this object (or earlier if \a nyears is
  negative).

  \sa daysTo(), addDays(), addMonths(), addSecs()
*/

QDateTime QDateTime::addYears( int nyears ) const
{
    return QDateTime( d.addYears(nyears), t );
}

/*!
  Returns a QDateTime object containing a datetime \a nsecs seconds
  later than the datetime of this object (or earlier if \a nsecs is
  negative).

  \sa secsTo(), addDays(), addMonths(), addYears()
*/

QDateTime QDateTime::addSecs( int nsecs ) const
{
    uint dd = d.jd;
    int  tt = t.ds;
    int  sign = 1;
    if ( nsecs < 0 ) {
	nsecs = -nsecs;
	sign = -1;
    }
    if ( nsecs >= (int)SECS_PER_DAY ) {
	dd += sign*(nsecs/SECS_PER_DAY);
	nsecs %= SECS_PER_DAY;
    }
    tt += sign*nsecs*1000;
    if ( tt < 0 ) {
	tt = MSECS_PER_DAY - tt - 1;
	dd -= tt / MSECS_PER_DAY;
	tt = tt % MSECS_PER_DAY;
	tt = MSECS_PER_DAY - tt - 1;
    } else if ( tt >= (int)MSECS_PER_DAY ) {
	dd += ( tt / MSECS_PER_DAY );
	tt = tt % MSECS_PER_DAY;
    }
    QDateTime ret;
    ret.t.ds = tt;
    ret.d.jd = dd;
    return ret;
}

/*!
  Returns the number of days from this datetime to \a dt (which is
  negative if \a dt is earlier than this datetime).

  \sa addDays(), secsTo()
*/

int QDateTime::daysTo( const QDateTime &dt ) const
{
    return d.daysTo( dt.d );
}

/*!
  Returns the number of seconds from this datetime to \a dt (which is
  negative if \a dt is earlier than this datetime).

  Example:
  \code
    QDateTime dt = QDateTime::currentDateTime();
    QDateTime xmas( QDate(dt.year(), 12, 24), QTime(17, 00) );
    qDebug( "%d seconds to Christmas", dt.secsTo(xmas) );
  \endcode

  \sa addSecs(), daysTo(), QTime::secsTo()
*/

int QDateTime::secsTo( const QDateTime &dt ) const
{
    return t.secsTo(dt.t) + d.daysTo(dt.d)*SECS_PER_DAY;
}


/*!
  Returns TRUE if this datetime is equal to \a dt; otherwise returns FALSE.

  \sa operator!=()
*/

bool QDateTime::operator==( const QDateTime &dt ) const
{
    return  t == dt.t && d == dt.d;
}

/*!
  Returns TRUE if this datetime is different from \a dt; otherwise
  returns FALSE.

  \sa operator==()
*/

bool QDateTime::operator!=( const QDateTime &dt ) const
{
    return  t != dt.t || d != dt.d;
}

/*!
  Returns TRUE if this datetime is earlier than \a dt, otherwise returns FALSE.
*/

bool QDateTime::operator<( const QDateTime &dt ) const
{
    if ( d < dt.d )
	return TRUE;
    return d == dt.d ? t < dt.t : FALSE;
}

/*!
  Returns TRUE if this datetime is earlier than or equal to \a dt,
  otherwise returns FALSE.
*/

bool QDateTime::operator<=( const QDateTime &dt ) const
{
    if ( d < dt.d )
	return TRUE;
    return d == dt.d ? t <= dt.t : FALSE;
}

/*!
  Returns TRUE if this datetime is later than \a dt, otherwise returns FALSE.
*/

bool QDateTime::operator>( const QDateTime &dt ) const
{
    if ( d > dt.d )
	return TRUE;
    return d == dt.d ? t > dt.t : FALSE;
}

/*!
  Returns TRUE if this datetime is later than or equal to \a dt,
  otherwise returns FALSE.
*/

bool QDateTime::operator>=( const QDateTime &dt ) const
{
    if ( d > dt.d )
	return TRUE;
    return d == dt.d ? t >= dt.t : FALSE;
}

/*!
  Returns the current datetime, as reported by the system clock.

  \sa QDate::currentDate(), QTime::currentTime()
*/

QDateTime QDateTime::currentDateTime()
{
    QDate cd = QDate::currentDate();
    QTime ct;
    if ( QTime::currentTime(&ct) )		// too close to midnight?
	cd = QDate::currentDate();		// YES! time for some midnight
						// voodoo, fetch date again
    return QDateTime( cd, ct );
}

#ifndef QT_NO_DATESTRING
/*!
  Returns the QDateTime represented by the string \a s, using the format
  \a f, or an invalid datetime if this is not possible.

  Note that Qt::LocalDate cannot be used here.

  Note for Qt::TextDate: It is recommended to use the English short
  month names (e.g. Jan). Localized month names may also be used, but
  they depend on the user's locale settings.
 */
QDateTime QDateTime::fromString( const QString& s, Qt::DateFormat f )
{
    if ( ( s.isEmpty() ) || ( f == Qt::LocalDate ) ) {
#if defined(QT_CHECK_RANGE)
	qWarning( "QDateTime::fromString: Parameter out of range." );
#endif
	return QDateTime();
    }
    if ( f == Qt::ISODate ) {
	return QDateTime( QDate::fromString( s.mid(0,10), Qt::ISODate ),
			  QTime::fromString( s.mid(11), Qt::ISODate ) );
    }
#if !defined(QT_NO_REGEXP) && !defined(QT_NO_TEXTDATE)
    else if ( f == Qt::TextDate ) {
	QString monthName( s.mid( 4, 3 ) );
	int month = -1;
	// Assume that English monthnames are the default
	for ( int i = 0; i < 12; ++i ) {
	    if ( monthName == qt_shortMonthNames[i] ) {
		month = i + 1;
		break;
	    }
	}
	// If English names can't be found, search the localized ones
	if ( month == -1 ) {
	    for ( int i = 1; i <= 12; ++i ) {
		if ( monthName == QDate::shortMonthName( i ) ) {
		    month = i;
		    break;
		}
	    }
	}
#if defined(QT_CHECK_RANGE)
    	if ( month < 1 || month > 12 ) {
	    qWarning( "QDateTime::fromString: Parameter out of range." );
	    month = 1;
	}
#endif
	int day = s.mid( 8, 2 ).simplifyWhiteSpace().toInt();
	int year = s.right( 4 ).toInt();
	QDate date( year, month, day );
	QTime time;
	int hour, minute, second;
	int pivot = s.find( QRegExp("[0-9][0-9]:[0-9][0-9]:[0-9][0-9]") );
	if ( pivot != -1 ) {
	    hour = s.mid( pivot, 2 ).toInt();
	    minute = s.mid( pivot+3, 2 ).toInt();
	    second = s.mid( pivot+6, 2 ).toInt();
	    time.setHMS( hour, minute, second );
	}
	return QDateTime( date, time );
    }
#endif //QT_NO_REGEXP
    return QDateTime();
}
#endif //QT_NO_DATESTRING


/*****************************************************************************
  Date/time stream functions
 *****************************************************************************/

#ifndef QT_NO_DATASTREAM
/*!
  \relates QDate
  Writes the date, \a d, to the data stream, \a s.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QDate &d )
{
    return s << (Q_UINT32)(d.jd);
}

/*!
  \relates QDate
  Reads a date from the stream \a s into \a d.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QDate &d )
{
    Q_UINT32 jd;
    s >> jd;
    d.jd = jd;
    return s;
}

/*!
  \relates QTime
  Writes time \a t to the stream \a s.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QTime &t )
{
    return s << (Q_UINT32)(t.ds);
}

/*!
  \relates QTime
  Reads a time from the stream \a s into \a t.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QTime &t )
{
    Q_UINT32 ds;
    s >> ds;
    t.ds = ds;
    return s;
}

/*!
  \relates QDateTime
  Writes the datetime \a dt to the stream \a s.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<( QDataStream &s, const QDateTime &dt )
{
    return s << dt.d << dt.t;
}

/*!
  \relates QDateTime
  Reads a datetime from the stream \a s into \a dt.

  \sa \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>( QDataStream &s, QDateTime &dt )
{
    s >> dt.d >> dt.t;
    return s;
}
#endif //QT_NO_DATASTREAM

