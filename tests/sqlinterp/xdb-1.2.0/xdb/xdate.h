/*  $Id: xdate.h,v 1.6 1999/03/19 10:56:35 willy Exp $

    Xbase project source code

    This file contains a header file for the xbDate object, which is
    used for handling dates.

    Copyright (C) 1997  StarTech, Gary A. Kunkel
    email - xbase@startech.keller.tx.us
    www   - http://www.startech.keller.tx.us/xbase.html

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    V 1.0    10/10/97   - Initial release of software
    V 1.5    1/2/98     - Added memo field support
    V 1.6a   4/1/98     - Added expression support
    V 1.6b   4/8/98     - Numeric index keys
    V 1.7.1  5/25/98    - Enhancements/bug fixes from eljorgo@fontun.com
    V 1.8.0a 1/29/99    - Added Default Date Format processing
*/

#ifndef __XB_XDATE_H__
#define __XB_XDATE_H__

#include <xdb/xbconfig.h>

#define XB_FMT_WEEK   1
#define XB_FMT_MONTH  2
#define XB_FMT_YEAR   3

class XBDLLEXPORT xbDate {
  public:
    xbDate( void );
    long JulianDays    ( const char *Date8 );
    int  YearOf        ( const char *Date8 );
    int  MonthOf       ( const char *Date8 );
    int  IsLeapYear    ( const char *Date8 );
    int  DayOf         ( int Format, const char *Date8 );
    char * Sysdate     ( void );
    int  DateIsValid   ( const char *Date8 );
    char * JulToDate8  ( long );
    char * CharDayOf   ( const char *Date8 );
    char * CharMonthOf ( const char *Date8 );
    char * FormatDate  ( const char *Format, const char *Date8 );
    long LastDayOfMonth( const char *d );
  private:
    int LeapTable[12];
    int NonLeapTable[12];
    char *Days[7];
    char *Months[12];
};

#endif    // __XB_XDATE_H__
