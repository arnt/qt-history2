/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdatetm.cpp#4 $
**
** Implementation of date and time classes
**
** Author  : Haavard Nord
** Created : 940124
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
*****************************************************************************/

#include "qdatetm.h"
#include "qdstream.h"
#include <stdio.h>
#include <time.h>
#if defined(UNIX)
#include <sys/time.h>
#include <unistd.h>
#endif

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qdatetm.cpp#4 $";
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

QDate::QDate( uint y, uint m, uint d )		// set date and time
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


uint QDate::year() const			// 1752..
{
    uint y, m, d;
    jul2greg( jd, y, m, d );
    return y;
}

uint QDate::month() const			// 1..12
{
    uint y, m, d;
    jul2greg( jd, y, m, d );
    return m;
}

uint QDate::day() const				// 1..31
{
    uint y, m, d;
    jul2greg( jd, y, m, d );
    return d;
}

uint QDate::dayOfWeek() const			// 1..7 (monday==1)
{
    return (uint)((((jd+1) % 7) + 6)%7) + 1;
}

uint QDate::dayOfYear() const			// 1..365
{
    return (uint)(jd - greg2jul( year(), 1, 1 )) + 1;
}

uint QDate::daysInMonth() const			// 1..31
{
    uint y, m, d;
    jul2greg( jd, y, m, d );
    if ( m == 2 && leapYear(y) )
	return 29;
    else
	return monthDays[m];
}

uint QDate::daysInYear() const			// 1..365
{
    uint y, m, d;
    jul2greg( jd, y, m, d );
    return leapYear(y) ? 366 : 365;
}


const char *QDate::monthName( uint month) const // name of month
{
#if defined(DEBUG)
    ASSERT( month > 0 && month <= 12 );
#endif
    return monthNames[month-1];
}

const char *QDate::dayName( uint weekday) const // name of week day
{
#if defined(DEBUG)
    ASSERT( weekday > 0 && weekday <= 7 );
#endif
    return weekdayNames[weekday-1];
}


QString QDate::asString() const			// date as string
{
    QString buf;
    uint y, m, d;
    jul2greg( jd, y, m, d );
    buf.sprintf( "%s %s %d %d", dayName(dayOfWeek()), monthName(m), d, y);
    return buf;
}


bool QDate::setYMD( uint y, uint m, uint d )	// set year, month, day
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

bool QDate::isValid( uint y, uint m, uint d )	// is valid date?
{
    if ( y <= 99 )
	y += 1900;
    else
    if ( y < FIRST_YEAR || y == FIRST_YEAR && (m < 9 || (m == 9 && d < 14)) )
	return FALSE;
    return (d > 0 && m > 0 && m <= 12) &&
	   (d <= monthDays[m] || (d == 29 && m == 2 && leapYear(y)));
}

bool QDate::leapYear( uint y )			// is leap year (from K&R)
{
    return y % 4 == 0 && y % 100 != 0 || y % 400 == 0;
}

ulong QDate::greg2jul( uint y, uint m, uint d ) // Gregorian date -> Julian day
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

void QDate::jul2greg( ulong jd, uint &y, uint &m, uint &d )
{						// Julian day -> Gregorian date
    ulong x;
    ulong j = jd - 1721119L;
    y = (uint) ((j*4 - 1) / 146097L);
    j = j*4 - 1 - 146097L*y;
    x = j/4;
    j = (x*4 + 3) / 1461;
    x = (x*4) + 3 - 1461*j;
    x = (x + 4)/4;
    m = (uint)(5*x - 3)/153;
    x = 5*x - 3 - 153*m;
    d = (uint)((x + 5)/5);
    y = (uint)(100*y + j);
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

QTime::QTime( uint h, uint m, uint s, uint ms )	// set time
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


uint QTime::hour() const			// 0..23
{
    return (uint)(ds / MSECS_PER_HOUR);
}

uint QTime::minute() const			// 0..59
{
    return (uint)((ds % MSECS_PER_HOUR)/MSECS_PER_MIN);
}

uint QTime::second() const			// 0..59
{
    return (uint)((ds / 1000)%SECS_PER_MIN);
}

uint QTime::msec() const			// 0..999
{
    return (uint)(ds % 1000);
}


QString QTime::asString() const			// time as string
{
    QString buf;
    buf.sprintf( "%.2d:%.2d:%.2d", hour(), minute(), second() );
    return buf;
}


bool QTime::setHMS( uint h, uint m, uint s, uint ms ) // set time of day
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


QTime QTime::currentTime()			// get current time
{
    static long msdiff = -1;			// msec diff from GMT
    struct timeval tv;
    QTime ct;
    gettimeofday( &tv, 0 );
    if ( msdiff == -1 ) {
	++msdiff;
	tm *t = localtime( &tv.tv_sec );
	long ms = MSECS_PER_HOUR*t->tm_hour + MSECS_PER_MIN*t->tm_min +
	    	  1000L*t->tm_sec;
	ct.ds = ms + (tv.tv_usec)/1000;
	msdiff = ms - (tv.tv_sec % 86400L)*1000L;
    }
    else
	ct.ds = (tv.tv_sec % 86400L)*1000L + msdiff + tv.tv_usec/1000L;
    return ct;
}

bool QTime::isValid( uint h, uint m, uint s, uint ms ) // is valid time?
{
    return h < 24 && m < 60 && s < 60 && ms < 1000;
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


QString QDateTime::asString() const		// datetime as string
{
    QString buf;
    QString time = t.asString();
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
