/*  $Id: testdate.cpp,v 1.6 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

    This program tests the xdate routines 

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

    V 1.0   10/10/97   - Initial release of software
    V 1.5   1/2/98     - Added memo field support
    V 1.6a  5/1/98     - Added expression support
    V 1.8   11/29/98   - Version 1.8 upgrade
*/

#include <xdb/xbase.h>

int main()
{
   xbXBase x;
   long l;

   cout << "This program tests the XDATE routines" << endl;

   cout << "\nThis year is  " << x.YearOf ( x.Sysdate() );
   cout << "\nThis Month is " << x.MonthOf( x.Sysdate() );
   cout << "\nToday is day " << x.DayOf( XB_FMT_WEEK, x.Sysdate()) << "  of the week"; 
   cout << "\nToday is day " << x.DayOf( XB_FMT_MONTH, x.Sysdate()) << " of the month";
   cout << "\nToday is day " << x.DayOf( XB_FMT_YEAR, x.Sysdate()) << " of the year";

   if( x.IsLeapYear( x.Sysdate()))
      cout << "\nThis is a leapyear";
   else
      cout << "\nThis is not a leap year."; 

   cout << "\nToday is " << x.Sysdate();

   if( x.DateIsValid( "19951301" ))
      cout << "\n19951301 is a valid date";
   else
      cout << "\n19951301 is not a valid date";

   l =  x.JulianDays( "19951101" ) - x.JulianDays( "19951001" );

   cout << "\nThere are " << l
        << " days between 10/1/95 and 11/1/95.";

   cout << "\nIn 7 days it will be "  
        << x.JulToDate8( x.JulianDays( x.Sysdate()) + 7L );

   cout << "\nToday is " << x.CharDayOf( x.Sysdate());
   cout << "\nThis month is " << x.CharMonthOf( x.Sysdate());

   cout << "\nFormat (YYDDD)              " << x.FormatDate( "YYDDD", x.Sysdate());
   cout << "\nFormat (MM/DD/YY)           " << x.FormatDate( "MM/DD/YY", x.Sysdate());
   cout << "\nFormat (MMMM DD,YYYY)       " << x.FormatDate( "MMMM DD,YYYY", x.Sysdate());
   cout << "\nFormat (DDDD, MMMM DD YYYY) " << x.FormatDate( "DDDD, MMMM DD YYYY", x.Sysdate());

   cout << "\nDefault DateFormat is       " << x.GetDefaultDateFormat();
   //cout << "  " << x.DTOC( x.Sysdate() );
   cout << "\nSetting DefaultDateFormat to DD/MM/YY";
   x.SetDefaultDateFormat( "DD/MM/YY" );
   cout << "\nDefault DateFormat is now   " << x.GetDefaultDateFormat();
   //cout << "  " << x.DTOC( x.Sysdate() );

   cout << "\n";
   return 0;
}
    
