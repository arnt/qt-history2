/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdatetime.cpp#24 $
**
** Implementation of date and time classes
**
** Author  : Haavard Nord
** Created : 940124
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qdatetm.h"
#include "qdstream.h"
#include <stdio.h>
#include <time.h>
#if defined(_OS_MSDOS_)
#include <dos.h>
#elif defined(_OS_OS2_)
#include <os2.h>
#elif defined(UNIX)
#include <sys/time.h>
#include <unistd.h>
#endif

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qdatetime.cpp#24 $";
#endif


static const ulong FIRST_DAY	 = 2361222L;	// Julian day for 17520914
static const int   FIRST_YEAR	 = 1752;
static const ulong SECS_PER_DAY	 = 86400L;
static const ulong MSECS_PER_DAY = 86400000L;
static const ulong SECS_PER_HOUR = 3600L;
static const ulong MSECS_PER_HOUR= 3600000L;
static const uint  SECS_PER_MIN	 = 60;
static const ulong MSECS_PER_MIN = 60000L;

static short monthDays[] ={0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

const char *QDate::monthNames[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

const char *QDate::weekdayNames[] ={
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };


// --------------------------------------------------------------------------
// QDate member functions
//

/*!
\class QDate qdatetm.h

\brief The QDate class provides date functions.

  \ingroup tools

The QDate is based on the Gregorian (modern western)
calendar. England adopted the Gregorian calendar on September 14th
1752, which is the earliest date that is supported by QDate.  Using
earlier dates will give undefined results. Some countries adopted
the Gregorian calendar later than England, thus the week dat of
early dates might be incorrect for these countries (but correct for
England).  The end of time is reached around 8000AD, by which time we
expect Qt to be obsolete. */


/*! \fn QDate::QDate()
  Constructs a null date. Null dates are invalid. */

/*! Constructs a date with the year \e y, month \e m and day \e d. */

QDate::QDate( int y, int m, int d )		// set date and time
{
#if defined(CHECK_RANGE)
    if ( !isValid(y,m,d) )
	warning( "QDate: Invalid date" );
#endif
    jd = greg2jul( y, m, d );
#if defined(DEBUG)
    ASSERT( year() == y );
    ASSERT( month() == m );
    ASSERT( day() == d );
#endif
}


/*!
\fn bool QDate::isNull() const
Returns TRUE if the date is null.
\sa isValid().
*/

/*!
Returns TRUE if the date is valid.
\sa isNull().
*/

bool QDate::isValid() const			// valid date
{
    return jd >= FIRST_DAY;
}


/*!
Returns the year part (>= 1752) of the date.
\sa month(), day().
*/

int QDate::year() const				// 1752..
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return y;
}

/*!
Returns the month part (1..12) of the date.
\sa year(), day().
*/

int QDate::month() const			// 1..12
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return m;
}

/*!
Returns the day part (1..31) of the date.
\sa year(), month().
*/

int QDate::day() const				// 1..31
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return d;
}

/*!
Returns the weekday (1..7) of the date. Monday is the first day.
*/

int QDate::dayOfWeek() const			// 1..7 (monday==1)
{
    return (int)((((jd+1) % 7) + 6)%7) + 1;
}

/*!
Returns the day of the year (1..365) of the date.
*/

int QDate::dayOfYear() const			// 1..365
{
    return (int)(jd - greg2jul( year(), 1, 1 )) + 1;
}

/*!
Returns the day of the month (1..31) of the date.
*/

int QDate::daysInMonth() const			// 1..31
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    if ( m == 2 && leapYear(y) )
	return 29;
    else
	return monthDays[m];
}

/*!
Returns the number of days in the year (365 or 366).
*/

int QDate::daysInYear() const			// 365 or 366
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return leapYear(y) ? 366 : 365;
}


/*!
Returns the name of the \e month.  Month 1 == "Jan", month 2 == "Feb" etc.
*/

const char *QDate::monthName( int month) const	// name of month
{
#if defined(DEBUG)
    ASSERT( month > 0 && month <= 12 );
#endif
    return monthNames[month-1];
}

/*!
Returns the name of the \e weekday.  Weekday 1 == "Mon", day 2 == "Tue" etc.
*/

const char *QDate::dayName( int weekday) const	// name of week day
{
#if defined(DEBUG)
    ASSERT( weekday > 0 && weekday <= 7 );
#endif
    return weekdayNames[weekday-1];
}


/*!
Returns the date as a string.  The string format is "Sat May 20 1995".
*/

QString QDate::toString() const			// date to string
{
    QString buf;
    int y, m, d;
    jul2greg( jd, y, m, d );
    buf.sprintf( "%s %s %d %d", dayName(dayOfWeek()), monthName(m), d, y);
    return buf;
}


/*!
Sets the year \e y, month \e m and day \e d.
Returns TRUE if the date is valid, FALSE if it is invalid.
*/

bool QDate::setYMD( int y, int m, int d )	// set year, month, day
{
    if ( !isValid(y,m,d) ) {
#if defined(CHECK_RANGE)
	 warning( "QDate::setYMD: Invalid date" );
#endif
	 return FALSE;
    }
    jd = greg2jul( y, m, d );
    return TRUE;
}


/*!
Returns this date plus \e ndays days.
*/

QDate QDate::addDays( long ndays ) const	// add days
{
    QDate d;
    d.jd = jd + ndays;
    return d;
}

/*!
Returns the number of days between this date and \e e:
\code
    QDate d1( 1995, 5, 17 );		\/ May 17th 1995
    QDate d2( 1995, 5, 20 );		\/ May 20th 1995
    d1.daysTo( d2 );			\/ returns 3
    d2.daysTo( d1 );			\/ returns -3
\endcode
*/

long QDate::daysTo( const QDate &d ) const	// days difference
{
    return d.jd - jd;
}


/*!
Returns the current date.
*/

QDate QDate::currentDate()			// get current date
{
    time_t ltime;
    time( &ltime );
    tm *t = localtime( &ltime );
    QDate d;
    d.jd = greg2jul( t->tm_year + 1900, t->tm_mon + 1, t->tm_mday );
    return d;
}

/*!
Returns TRUE if the specified date is valid.
*/

bool QDate::isValid( int y, int m, int d )	// is valid date?
{
    if ( y <= 99 )
	y += 1900;
    else if ( y < FIRST_YEAR || (y == FIRST_YEAR && (m < 9 ||
						    (m == 9 && d < 14))) )
	return FALSE;
    return (d > 0 && m > 0 && m <= 12) &&
	   (d <= monthDays[m] || (d == 29 && m == 2 && leapYear(y)));
}

/*!
Returns TRUE if the specified year \e y is a leap year.
*/

bool QDate::leapYear( int y )			// is leap year (from K&R)
{
    return y % 4 == 0 && y % 100 != 0 || y % 400 == 0;
}

ulong QDate::greg2jul( int y, int m, int d )	// Gregorian date -> Julian day
{
    ulong c, ya;
    if ( y <= 99 )
	y += 1900;
    if ( m > 2 )
	m -= 3;
    else {
	m += 9;
	y--;
    }
    c = y;					// NOTE: Sym C++ 6.0 bug
    c /= 100;
    ya = y - 100*c;
    return (146097L*c)/4 + (1461*ya)/4 + (153*m+2)/5 + d + 1721119L;
}

void QDate::jul2greg( ulong jd, int &y, int &m, int &d )
{						// Julian day -> Gregorian date
    ulong x;
    ulong j = jd - 1721119L;
    y = (int) ((j*4 - 1) / 146097L);
    j = j*4 - 1 - 146097L*y;
    x = j/4;
    j = (x*4 + 3) / 1461;
    x = (x*4) + 3 - 1461*j;
    x = (x + 4)/4;
    m = (int)(5*x - 3)/153;
    x = 5*x - 3 - 153*m;
    d = (int)((x + 5)/5);
    y = (int)(100*y + j);
    if ( m < 10 )
	m += 3;
    else {
	m -= 9;
	y++;
    }
}


// --------------------------------------------------------------------------
// QTime class member functions
//

/*! \class QTime qdatetm.h

  \brief The QTime class provides time functions 24 hours a day.

  \ingroup tools

  The time resolution of QTime is a millisecond, although the accuracy
  depends on the underlying operating system. Some operating systems
  (e.g. Linux) support a one-millisecond resolution, while others
  (i.e. MS-DOS and Windows) support only a 55 millisecond
  resolution.*/

/*!
\fn QTime::QTime()
Constructs a null time 00:00:00.000. Null times are valid.
*/

/*!
Constructs a time with hour \e h, minute \e m, seconds \e s and milliseconds
\e ms.
*/

QTime::QTime( int h, int m, int s, int ms )	// set time
{
#if defined(CHECK_RANGE)
    if ( !isValid(h,m,s,ms) )
	warning( "QTime: Invalid time" );
#endif
    ds = (h*SECS_PER_HOUR + m*SECS_PER_MIN + s)*1000 + ms;
#if defined(DEBUG)
    ASSERT( hour() == h );
    ASSERT( minute() == m );
    ASSERT( second() == s );
    ASSERT( msec() == ms );
#endif
}


/*!
Returns TRUE if the time is valid, or FALSE if the time is invalid or
null.  The time 23:30:55.746 is valid, while 24:12:30 is invalid.
*/

bool QTime::isValid() const			// valid time
{
    return ds < MSECS_PER_DAY;
}


/*!
Returns the hour part (0..23) of the time.
*/

int QTime::hour() const				// 0..23
{
    return (int)(ds / MSECS_PER_HOUR);
}

/*!
Returns the minute part (0..59) of the time.
*/

int QTime::minute() const			// 0..59
{
    return (int)((ds % MSECS_PER_HOUR)/MSECS_PER_MIN);
}

/*!
Returns the second part (0..59) of the time.
*/

int QTime::second() const			// 0..59
{
    return (int)((ds / 1000)%SECS_PER_MIN);
}

/*!
Returns the millisecond part (0..999) of the time.
*/

int QTime::msec() const				// 0..999
{
    return (int)(ds % 1000);
}


/*!
Converts the date to a string, which is returned.  Milliseconds are
not included. The string format is "03:40:13".
*/

QString QTime::toString() const			// time to string
{
    QString buf;
    buf.sprintf( "%.2d:%.2d:%.2d", hour(), minute(), second() );
    return buf;
}


/*!
Sets the hour \e h, minute \e m, seconds \e s and milliseconds
\e ms.
Returns TRUE if the time is valid, FALSE if it is invalid.
*/

bool QTime::setHMS( int h, int m, int s, int ms ) // set time of day
{
    if ( !isValid(h,m,s,ms) ) {
#if defined(CHECK_RANGE)
	 warning( "QTime::setHMS: Invalid time" );
#endif
	 return FALSE;
    }
    ds = (h*SECS_PER_HOUR + m*SECS_PER_MIN + s)*1000 + ms;
    return TRUE;
}


/*!
Returns the time plus \e nsecs seconds.
*/

QTime QTime::addSecs( long nsecs ) const	// add seconds
{
    QTime t;
    t.ds = (ds + nsecs*1000) % MSECS_PER_DAY;
    return t;
}

/*!
Returns the number of seconds between this time and \e t.
*/

long QTime::secsTo( const QTime &t ) const	// seconds difference
{
    return ((long)t.ds - (long)ds)/1000L;
}

/*!
Returns the time plus \e ms milliseconds.
*/

QTime QTime::addMSecs( long ms ) const		// add milliseconds
{
    QTime t;
    t.ds = (ds + ms) % MSECS_PER_DAY;
    return t;
}

/*!
Returns the number of milliseconds between this time and \e t.
*/

long QTime::msecsTo( const QTime &t ) const	// milliseconds difference
{
    return t.ds - ds;
}


#if defined(_OS_SUN_)
extern "C" int gettimeofday( struct timeval * );
#endif

/*!
Returns the current time.
*/

QTime QTime::currentTime()			// get current time
{
#if defined(_OS_MSDOS_)

    QTime ct;
    _dostime_t t;
    _dos_gettime( &t );
    ct.ds = MSECS_PER_HOUR*t.hour + MSECS_PER_MIN*t.minute +
	    t.second*1000L + t.hsecond*10L;
    return ct;

#elif defined(_OS_OS2_)

    QTime ct;
    DATETIME t;
    DosGetDateTime( &t );
    ct.ds = MSECS_PER_HOUR*t.hours + MSECS_PER_MIN*t.minutes +
	    1000*t.seconds + 10*t.hundredths;
    return ct;

#elif defined(UNIX)

    QTime ct;
    struct timeval tv;
#if defined(_OS_SUN_)
    gettimeofday( &tv );
#else
    gettimeofday( &tv, 0 );
#endif
    time_t ltime = tv.tv_sec;
    tm *t = localtime( &ltime );
    ct.ds = MSECS_PER_HOUR*t->tm_hour + MSECS_PER_MIN*t->tm_min +
	    1000*t->tm_sec + tv.tv_usec/1000;
    return ct;

#else						// !! no millisec resolution

    QTime ct;
    time_t ltime;
    ::time( &ltime );
    tm *t = localtime( &ltime );
    ct.ds = MSECS_PER_HOUR*t->tm_hour + MSECS_PER_MIN*t->tm_min +
	    1000*t->tm_sec;
    return ct;

#endif
}

/*!
Returns TRUE if the specified time is valid.
*/

bool QTime::isValid( int h, int m, int s, int ms ) // is valid time?
{
    return (uint)h < 24 && (uint)m < 60 && (uint)s < 60 && (uint)ms < 1000;
}


/*! Sets the time to the current time, e.g. for timing:
  \code
  QTime t;
  t.start();				\/ start clock
  ... \/ some lengthy task
  printf( "%d\n", t.elapsed() );	\/ prints # msecs elapsed
  \endcode

  \sa restart(), elapsed() */

void QTime::start()				// start clock
{
    *this = currentTime();
}

/*!
Restarts for timing, and returns the number of milliseconds that have elapsed
since start() or restart().
\sa start(), elapsed()
*/

long QTime::restart()				// restart clock
{
    QTime t = currentTime();
    long  n = msecsTo( t );
    *this = t;
    return n;
}

/*!
Returns the number of milliseconds that have elapsed since start() or
restart() were called.
*/

long QTime::elapsed()				// msecs since start/restart
{
    long  n = msecsTo( currentTime() );
    return n;
}


// --------------------------------------------------------------------------
// QDateTime member functions
//

/*! \class QDateTime qdatetm.h

  \brief The QDateTime class combines QDate and QTime into a single class.

  \ingroup tools

  QDateTime provides high precision date and time functions since it
  can work with all dates since September 14. 1752, with the resolution up
  to a millisecond. */

/*!
\fn QDateTime::QDateTime()
Constructs a null datetime (i.e. null date and null time).
*/

/*!
Constructs a datetime with date \e date and null time (00:00:00.000).
*/

QDateTime::QDateTime( const QDate &date )	// set date
	: d(date)
{
}

/*!
Constructs a datetime with date \e date and time \e time.
*/

QDateTime::QDateTime( const QDate &date, const QTime &time )
	: d(date), t(time)			// set date and time
{
}


/*!
Returns the datetime as a string. The string format is
"Sat May 20 1995 03:40:13".
*/

QString QDateTime::toString() const		// datetime to string
{
    QString buf;
    QString time = t.toString();
    buf.sprintf( "%s %s %d %s %d", d.dayName(d.dayOfWeek()),
		 d.monthName(d.month()), d.day(), (const char*)time, d.year());
    return buf;
}


/*!
Returns the datetime plus \e ndays days.
*/

QDateTime QDateTime::addDays( long ndays ) const // add days
{
    QDateTime dt( d.addDays(ndays), t );
    return dt;
}

/*!
Returns the datetime plus \e nsecs seconds.
*/

QDateTime QDateTime::addSecs( long nsecs ) const // add seconds
{
    long dd = (t.ds + nsecs*1000)/MSECS_PER_DAY;
    return QDateTime( d.addDays(dd), t.addSecs(nsecs) );
}

/*!
Returns the number of days between this datetime and \e dt.
*/

long QDateTime::daysTo( const QDateTime &dt ) const // days difference
{
    return d.daysTo( dt.d );
}

/*!
Returns the number of seconds between this datetime and \e dt.
*/

long QDateTime::secsTo( const QDateTime &dt ) const // seconds difference
{
    return t.secsTo( dt.t ) + d.daysTo( dt.d )*SECS_PER_DAY;
}


/*!
Returns the TRUE if this datetime is equal to \e dt, otherwise FALSE.
*/

bool QDateTime::operator==( const QDateTime &dt ) const
{
    return d == dt.d && t == dt.t;
}

/*!
Returns the TRUE if this datetime is not equal to \e dt, otherwise FALSE.
*/

bool QDateTime::operator!=( const QDateTime &dt ) const
{
    return d != dt.d || t != dt.t;
}

/*!
Returns the TRUE if this datetime is before \e dt, otherwise FALSE.
*/

bool QDateTime::operator<( const QDateTime &dt ) const
{
    if ( d < dt.d )
	return TRUE;
    return d == dt.d ? t < dt.t : FALSE;
}

/*!
Returns the TRUE if this datetime is before or equal to \e dt, otherwise FALSE.
*/

bool QDateTime::operator<=( const QDateTime &dt ) const
{
    if ( d < dt.d )
	return TRUE;
    return d == dt.d ? t <= dt.t : FALSE;
}

/*!
Returns the TRUE if this datetime is after \e dt, otherwise FALSE.
*/

bool QDateTime::operator>( const QDateTime &dt ) const
{
    if ( d > dt.d )
	return TRUE;
    return d == dt.d ? t > dt.t : FALSE;
}

/*!
Returns the TRUE if this datetime is equal to or after \e dt, otherwise FALSE.
*/

bool QDateTime::operator>=( const QDateTime &dt ) const
{
    if ( d > dt.d )
	return TRUE;
    return d == dt.d ? t >= dt.t : FALSE;
}


/*!
Returns the current datetime.
\sa QDate::currentDate() and QTime::currentTime().
*/

QDateTime QDateTime::currentDateTime()		// get current datetime
{
    QDate dd = QDate::currentDate();
    QTime tt = QTime::currentTime();
    return QDateTime( dd, tt );
}


// --------------------------------------------------------------------------
// Date/time stream functions
//

/*!
Writes the date \e d to the stream \e s and returns a reference to \e s.

\related QDate
*/

QDataStream &operator<<( QDataStream &s, const QDate &d )
{
    return s << d.jd;
}

/*!
Reads the date \e d from the stream \e s and returns a reference to \e s.

\related QDate
*/

QDataStream &operator>>( QDataStream &s, QDate &d )
{
    return s >> d.jd;
}

/*!
Writes the time \e t to the stream \e s and returns a reference to \e s.

\related QTime
*/

QDataStream &operator<<( QDataStream &s, const QTime &t )
{
    return s << t.ds;
}

/*!
Reads the time \e t from the stream \e s and returns a reference to \e s.

\related QTime
*/

QDataStream &operator>>( QDataStream &s, QTime &t )
{
    return s >> t.ds;
}

/*!
Writes the datetime \e dt to the stream \e s and returns a reference to \e s.

\related QDateTime
*/

QDataStream &operator<<( QDataStream &s, const QDateTime &dt )
{
    return s << dt.d << dt.t;
}

/*!
Reads the datetime \e dt from the stream \e s and returns a reference to \e s.

\related QDateTime
*/

QDataStream &operator>>( QDataStream &s, QDateTime &dt )
{
    s >> dt.d;
    return s >> dt.t;
}
