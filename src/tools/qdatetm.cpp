/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdatetm.cpp#8 $
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
static char ident[] = "$Id: //depot/qt/main/src/tools/qdatetm.cpp#8 $";
#endif


static const ulong FIRST_DAY	 = 2361222L;	// Julian day for 17520914
static const uint  FIRST_YEAR	 = 1752;
static const ulong SECS_PER_DAY	 = 86400L;
static const ulong MSECS_PER_DAY = 86400000L;
static const ulong SECS_PER_HOUR = 3600L;
static const ulong MSECS_PER_HOUR= 3600000L;
static const uint  SECS_PER_MIN	 = 60;
static const ulong MSECS_PER_MIN = 60000L;

static ushort monthDays[] ={0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

const char *QDate::monthNames[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

const char *QDate::weekdayNames[] ={
    "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };


// --------------------------------------------------------------------------
// QDate member functions
//

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


bool QDate::isValid() const			// valid date
{
    return jd >= FIRST_DAY;
}


int QDate::year() const				// 1752..
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return y;
}

int QDate::month() const			// 1..12
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return m;
}

int QDate::day() const				// 1..31
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return d;
}

int QDate::dayOfWeek() const			// 1..7 (monday==1)
{
    return (int)((((jd+1) % 7) + 6)%7) + 1;
}

int QDate::dayOfYear() const			// 1..365
{
    return (int)(jd - greg2jul( year(), 1, 1 )) + 1;
}

int QDate::daysInMonth() const			// 1..31
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    if ( m == 2 && leapYear(y) )
	return 29;
    else
	return monthDays[m];
}

int QDate::daysInYear() const			// 1..365
{
    int y, m, d;
    jul2greg( jd, y, m, d );
    return leapYear(y) ? 366 : 365;
}


const char *QDate::monthName( int month) const	// name of month
{
#if defined(DEBUG)
    ASSERT( month > 0 && month <= 12 );
#endif
    return monthNames[month-1];
}

const char *QDate::dayName( int weekday) const	// name of week day
{
#if defined(DEBUG)
    ASSERT( weekday > 0 && weekday <= 7 );
#endif
    return weekdayNames[weekday-1];
}


QString QDate::toString() const			// date to string
{
    QString buf;
    int y, m, d;
    jul2greg( jd, y, m, d );
    buf.sprintf( "%s %s %d %d", dayName(dayOfWeek()), monthName(m), d, y);
    return buf;
}


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


QDate QDate::addDays( long days ) const		// add days
{
    QDate d;
    d.jd = jd + days;
    return d;
}

long QDate::daysTo( const QDate &d ) const	// days difference
{
    return d.jd - jd;
}


QDate QDate::currentDate()			// get current date
{
    time_t ltime;
    time( &ltime );
    tm *t = localtime( &ltime );
    QDate d;
    d.jd = greg2jul( t->tm_year + 1900, t->tm_mon + 1, t->tm_mday );
    return d;
}

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


bool QTime::isValid() const			// valid time
{
    return ds < MSECS_PER_DAY;
}


int QTime::hour() const				// 0..23
{
    return (int)(ds / MSECS_PER_HOUR);
}

int QTime::minute() const			// 0..59
{
    return (int)((ds % MSECS_PER_HOUR)/MSECS_PER_MIN);
}

int QTime::second() const			// 0..59
{
    return (int)((ds / 1000)%SECS_PER_MIN);
}

int QTime::msec() const				// 0..999
{
    return (int)(ds % 1000);
}


QString QTime::toString() const			// time to string
{
    QString buf;
    buf.sprintf( "%.2d:%.2d:%.2d", hour(), minute(), second() );
    return buf;
}


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


QTime QTime::addSecs( long secs ) const		// add seconds
{
    QTime t;
    t.ds = (ds + secs*1000) % MSECS_PER_DAY;
    return t;
}

long QTime::secsTo( const QTime &t ) const	// seconds difference
{
    return ((long)t.ds - (long)ds)/1000L;
}

QTime QTime::addMSecs( long ms ) const		// add milliseconds
{
    QTime t;
    t.ds = (ds + ms) % MSECS_PER_DAY;
    return t;
}

long QTime::msecsTo( const QTime &t ) const	// milliseconds difference
{
    return t.ds - ds;
}


//
// There are many ifdef's in the following function because millisecond
// time isn't standardized.
//
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
    gettimeofday( &tv, 0 );
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

bool QTime::isValid( int h, int m, int s, int ms ) // is valid time?
{
    return (uint)h < 24 && (uint)m < 60 && (uint)s < 60 && (uint)ms < 1000;
}


// --------------------------------------------------------------------------
// QDateTime member functions
//

QDateTime::QDateTime( const QDate &date )	// set date
	: d(date)
{
}

QDateTime::QDateTime( const QDate &date, const QTime &time )
	: d(date), t(time)			// set date and time
{
}


QString QDateTime::toString() const		// datetime to string
{
    QString buf;
    QString time = t.toString();
    buf.sprintf( "%s %s %d %s %d", d.dayName(d.dayOfWeek()),
		 d.monthName(d.month()), d.day(), (const char*)time, d.year());
    return buf;
}


QDateTime QDateTime::addDays( long days ) const // add days
{
    QDateTime dt( d.addDays(days), t );
    return dt;
}

QDateTime QDateTime::addSecs( long secs ) const // add seconds
{
    long dd = (t.ds + secs*1000)/MSECS_PER_DAY;
    return QDateTime( d.addDays(dd), t.addSecs(secs) );
}

long QDateTime::daysTo( const QDateTime &dt ) const // days difference
{
    return d.daysTo( dt.d );
}

long QDateTime::secsTo( const QDateTime &dt ) const // seconds difference
{
    return t.secsTo( dt.t ) + d.daysTo( dt.d )*SECS_PER_DAY;
}


bool QDateTime::operator==( const QDateTime &dt ) const
{
    return d == dt.d && t == dt.t;
}

bool QDateTime::operator!=( const QDateTime &dt ) const
{
    return d != dt.d || t != dt.t;
}

bool QDateTime::operator<( const QDateTime &dt ) const
{
    if ( d < dt.d )
	return TRUE;
    return d == dt.d ? t < dt.t : FALSE;
}

bool QDateTime::operator<=( const QDateTime &dt ) const
{
    if ( d < dt.d )
	return TRUE;
    return d == dt.d ? t <= dt.t : FALSE;
}

bool QDateTime::operator>( const QDateTime &dt ) const
{
    if ( d > dt.d )
	return TRUE;
    return d == dt.d ? t > dt.t : FALSE;
}

bool QDateTime::operator>=( const QDateTime &dt ) const
{
    if ( d > dt.d )
	return TRUE;
    return d == dt.d ? t >= dt.t : FALSE;
}


QDateTime QDateTime::currentDateTime()		// get current datetime
{
    QDate dd = QDate::currentDate();
    QTime tt = QTime::currentTime();
    return QDateTime( dd, tt );
}


// --------------------------------------------------------------------------
// Date/time stream functions
//

QDataStream &operator<<( QDataStream &s, const QDate &d )
{
    return s << d.jd;
}

QDataStream &operator>>( QDataStream &s, QDate &d )
{
    return s >> d.jd;
}

QDataStream &operator<<( QDataStream &s, const QTime &t )
{
    return s << t.ds;
}

QDataStream &operator>>( QDataStream &s, QTime &t )
{
    return s >> t.ds;
}

QDataStream &operator<<( QDataStream &s, const QDateTime &dt )
{
    return s << dt.d << dt.t;
}

QDataStream &operator>>( QDataStream &s, QDateTime &dt )
{
    s >> dt.d;
    return s >> dt.t;
}
