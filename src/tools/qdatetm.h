/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdatetm.h#6 $
**
** Definition of date and time classes
**
** Author  : Haavard Nord
** Created : 940124
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QDATETM_H
#define QDATETM_H

#include "qstring.h"


// --------------------------------------------------------------------------
// QDate class
//

class QDate
{
friend QDataStream &operator<<( QDataStream &, const QDate & );
friend QDataStream &operator>>( QDataStream &, QDate & );
public:
    QDate()  { jd=0; }				// set null date
    QDate( int y, int m, int d );		// set date

    bool   isNull()	 const { return jd == 0; }
    bool   isValid()	 const;			// valid date

    int    year()	 const;			// 1752..
    int    month()	 const;			// 1..12
    int    day()	 const;			// 1..31
    int    dayOfWeek()	 const;			// 1..7 (monday==1)
    int    dayOfYear()	 const;			// 1..365
    int    daysInMonth() const;			// 1..31
    int    daysInYear()	 const;			// 1..365

    virtual const char *monthName( int month ) const;
    virtual const char *dayName( int weekday ) const;

    QString toString()	 const;			// date to string

    bool   setYMD( int y, int m, int d );	// set year, month, day

    QDate  addDays( long days )		const;	// add days
    long   daysTo( const QDate & )	const;	// days difference

    bool   operator==( const QDate &d ) const { return jd == d.jd; }
    bool   operator!=( const QDate &d ) const { return jd != d.jd; }
    bool   operator<( const QDate &d )	const { return jd < d.jd; }
    bool   operator<=( const QDate &d ) const { return jd <= d.jd; }
    bool   operator>( const QDate &d )	const { return jd > d.jd; }
    bool   operator>=( const QDate &d ) const { return jd >= d.jd; }

    static QDate currentDate();			// get current date
    static bool	 isValid( int y, int m, int d );
    static bool	 leapYear( int year );

protected:
    static ulong greg2jul( int y, int m, int d );
    static void	 jul2greg( ulong jd, int &y, int &m, int &d );
    ulong  jd;					// julian date
    static const char *monthNames[];
    static const char *weekdayNames[];
};


// --------------------------------------------------------------------------
// QTime class
//

class QTime
{
friend class QDateTime;
friend QDataStream &operator<<( QDataStream &, const QTime & );
friend QDataStream &operator>>( QDataStream &, QTime & );
public:
    QTime() { ds=0; }				// set null time
    QTime( int h, int m, int s=0, int ms=0 );	// set time

    bool   isNull()	 const { return ds == 0; }
    bool   isValid()	 const;			// valid time

    int    hour()	 const;			// 0..23
    int    minute()	 const;			// 0..59
    int    second()	 const;			// 0..59
    int    msec()	 const;			// 0..999

    QString toString()	 const;			// time to string

    bool   setHMS( int h, int m, int s, int ms=0 ); // set time of day

    QTime  addSecs( long secs )		const;	// add seconds
    long   secsTo( const QTime & )	const;	// seconds difference
    QTime  addMSecs( long ms )		const;	// add milliseconds
    long   msecsTo( const QTime & )	const;	// milliseconds difference

    bool   operator==( const QTime &d ) const { return ds == d.ds; }
    bool   operator!=( const QTime &d ) const { return ds != d.ds; }
    bool   operator<( const QTime &d )	const { return ds < d.ds; }
    bool   operator<=( const QTime &d ) const { return ds <= d.ds; }
    bool   operator>( const QTime &d )	const { return ds > d.ds; }
    bool   operator>=( const QTime &d ) const { return ds >= d.ds; }

    static QTime currentTime();			// get current time
    static bool	 isValid( int h, int m, int s, int ms=0 );

protected:
    ulong  ds;					// seconds
};


// --------------------------------------------------------------------------
// QDateTime class
//

class QDateTime
{
friend QDataStream &operator<<( QDataStream &, const QDateTime & );
friend QDataStream &operator>>( QDataStream &, QDateTime & );
public:
    QDateTime() {}				// set null date and null time
    QDateTime( const QDate & );			// set date and null time
    QDateTime( const QDate &, const QTime & );	// set date and time

    bool   isNull()	const { return d.isNull() && t.isNull(); }
    bool   isValid()	const { return d.isValid() && t.isValid(); }

    QDate  date()	const { return d; }	// get date
    QTime  time()	const { return t; }	// get time
    void   setDate( QDate date ) { d=date; }	// set date
    void   setTime( QTime time ) { t=time; }	// set time

    QString toString()  const;			// datetime to string

    QDateTime addDays( long days )	const;	// add days
    QDateTime addSecs( long secs )	const;	// add seconds (wrap date)
    long   daysTo( const QDateTime & )	const;	// day difference
    long   secsTo( const QDateTime & )	const;	// second difference

    bool   operator==( const QDateTime &dt ) const;
    bool   operator!=( const QDateTime &dt ) const;
    bool   operator<( const QDateTime &dt )  const;
    bool   operator<=( const QDateTime &dt ) const;
    bool   operator>( const QDateTime &dt )  const;
    bool   operator>=( const QDateTime &dt ) const;

    static QDateTime currentDateTime();		// get current datetime

private:
    QDate  d;					// date part
    QTime  t;					// time part
};


// --------------------------------------------------------------------------
// Date/time stream functions
//

QDataStream &operator<<( QDataStream &, const QDate & );
QDataStream &operator>>( QDataStream &, QDate & );
QDataStream &operator<<( QDataStream &, const QTime & );
QDataStream &operator>>( QDataStream &, QTime & );
QDataStream &operator<<( QDataStream &, const QDateTime & );
QDataStream &operator>>( QDataStream &, QDateTime & );


#endif // QDATETM_H
