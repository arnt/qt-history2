/****************************************************************************
** $Id: //depot/qt/main/src/tools/qdatetm.h#2 $
**
** Definition of date and time classes
**
** Author  : Haavard Nord
** Created : 940124
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
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
    QDate( uint y, uint m, uint d );		// set date

    bool   isNull()	 const { return jd == 0; }
    bool   isValid()	 const;			// valid date

    uint   year()	 const;			// 1752..
    uint   month()	 const;			// 1..12
    uint   day()	 const;			// 1..31
    uint   dayOfWeek()	 const;			// 1..7 (monday==1)
    uint   dayOfYear()	 const;			// 1..365
    uint   daysInMonth() const;			// 1..31
    uint   daysInYear()	 const;			// 1..365

    virtual const char *monthName( uint month ) const;
    virtual const char *dayName( uint weekday ) const;

    virtual QString asString()		const;	// date as string

    bool   setYMD( uint y, uint m, uint d );	// set year, month, day

    QDate  addDays( long days )		const;	// add days
    long   daysTo( const QDate & )	const;	// days difference

    bool   operator==( const QDate &d ) const { return jd == d.jd; }
    bool   operator!=( const QDate &d ) const { return jd != d.jd; }
    bool   operator<( const QDate &d )	const { return jd < d.jd; }
    bool   operator<=( const QDate &d ) const { return jd <= d.jd; }
    bool   operator>( const QDate &d )	const { return jd > d.jd; }
    bool   operator>=( const QDate &d ) const { return jd >= d.jd; }

    static QDate currentDate();			// get current date
    static bool	 isValid( uint y, uint m, uint d );
    static bool	 leapYear( uint year );

protected:
    static ulong greg2jul( uint y, uint m, uint d );
    static void	 jul2greg( ulong jd, uint &y, uint &m, uint &d );
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
    QTime( uint h, uint m, uint s=0 );		// set time

    bool   isNull()	 const { return ds == 0; }
    bool   isValid()	 const;			// valid time

    uint   hour()	 const;			// 0..23
    uint   minute()	 const;			// 0..59
    uint   second()	 const;			// 0..59

    virtual QString asString()		const;	// time as string

    bool   setHMS( uint h, uint m, uint s );	// set hour, minute, seconds

    QTime  addSecs( long secs )		const;	// add seconds
    long   secsTo( const QTime & )	const;	// seconds difference

    bool   operator==( const QTime &d ) const { return ds == d.ds; }
    bool   operator!=( const QTime &d ) const { return ds != d.ds; }
    bool   operator<( const QTime &d )	const { return ds < d.ds; }
    bool   operator<=( const QTime &d ) const { return ds <= d.ds; }
    bool   operator>( const QTime &d )	const { return ds > d.ds; }
    bool   operator>=( const QTime &d ) const { return ds >= d.ds; }

    static QTime currentTime();			// get current time
    static bool	 isValid( uint h, uint m, uint s );

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

    virtual QString asString() const;		// datetime as string

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
// QDataStream functions for reading and writing date and time classes
//

QDataStream &operator<<( QDataStream &, const QDate & );
QDataStream &operator>>( QDataStream &, QDate & );
QDataStream &operator<<( QDataStream &, const QTime & );
QDataStream &operator>>( QDataStream &, QTime & );
QDataStream &operator<<( QDataStream &, const QDateTime & );
QDataStream &operator>>( QDataStream &, QDateTime & );


#endif // QDATETM_H
