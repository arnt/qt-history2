/*  $Id: xdate.cpp,v 1.5 1999/03/19 10:56:34 willy Exp $

    Xbase project source code

    These functions are used for processing dates.
    All functions assume a standard date format of CCYYMMDD
    for Century,Year,Month and Day

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

    V 1.0     10/10/97   - Initial release of software
    V 1.2     11/30/97   - Updated leap-year processing logic
    V 1.5     1/2/98     - Added memo field support
    V 1.6a    4/1/98     - Added expression support
    V 1.6b    4/8/98     - Numiric index keys 
    V 1.7.1   5/25/98    - Enhancements/bug fixes from eljorgo@fontun.com 
    V 1.8.0.a 1/27/99    - Added default date format processing
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>  
#include <string.h>
#include <time.h>

#include <xdb/xbase.h>
#include <xdb/xdate.h>

/***************************************************************/
xbDate::xbDate() {
  LeapTable[0]  = 0;
  LeapTable[1]  = 31;
  LeapTable[2]  = 60;
  LeapTable[3]  = 91;
  LeapTable[4]  = 121;
  LeapTable[5]  = 152;
  LeapTable[6]  = 182;
  LeapTable[7]  = 213;
  LeapTable[8]  = 244;
  LeapTable[9]  = 274;
  LeapTable[10] = 305;
  LeapTable[11] = 335;

  NonLeapTable[0]  = 0;
  NonLeapTable[1]  = 31;
  NonLeapTable[2]  = 59;
  NonLeapTable[3]  = 90;
  NonLeapTable[4]  = 120;
  NonLeapTable[5]  = 151;
  NonLeapTable[6]  = 181;
  NonLeapTable[7]  = 212;
  NonLeapTable[8]  = 243;
  NonLeapTable[9]  = 273;
  NonLeapTable[10] = 304;
  NonLeapTable[11] = 334;

#ifdef XB_CASTELLANO
  Days[0] = "Domingo\0";
  Days[1] = "Lunes\0";
  Days[2] = "Martes\0";
  Days[3] = "Miercoles\0";
  Days[4] = "Jueves\0";
  Days[5] = "Viernes\0";
  Days[6] = "Sabado\0";
  Months[0]  = "Enero\0";
  Months[1]  = "Febrero\0";
  Months[2]  = "Marzo\0";
  Months[3]  = "Abril\0";
  Months[4]  = "Mayo\0";
  Months[5]  = "Junio\0";
  Months[6]  = "Julio\0";
  Months[7]  = "Agosto\0";
  Months[8]  = "Septiembre\0";
  Months[9]  = "Octubre\0";
  Months[10] = "Noviembre\0";
  Months[11] = "Diciembre\0";
#else
  Days[0] = "Sunday\0";
  Days[1] = "Monday\0";
  Days[2] = "Tuesday\0";
  Days[3] = "Wednesday\0";
  Days[4] = "Thursday\0";
  Days[5] = "Friday\0";
  Days[6] = "Saturday\0";
  Months[0]  = "January\0";
  Months[1]  = "February\0";
  Months[2]  = "March\0";
  Months[3]  = "April\0";
  Months[4]  = "May\0";
  Months[5]  = "June\0";
  Months[6]  = "July\0";
  Months[7]  = "August\0";
  Months[8]  = "September\0";
  Months[9]  = "October\0";
  Months[10] = "November\0";
  Months[11] = "December\0";
#endif
}
/***************************************************************/
/* this function returns century and year from a CCYYMMDD date */
int xbDate::YearOf( const char * Date8 )
{
   char year[5];
   year[0] = Date8[0];
   year[1] = Date8[1];
   year[2] = Date8[2];
   year[3] = Date8[3];
   year[4] = 0x00;
   return( atoi( year ));
}
/***************************************************************/
/* this function returns the month from a CCYYMMDD date        */
int xbDate::MonthOf( const char * Date8 )
{
   char month[3];
   month[0] = Date8[4];
   month[1] = Date8[5];
   month[2] = 0x00;
   return( atoi( month ));
}
/***************************************************************/
/* this function returns TRUE if a CCYYMMDD date is a leap year*/
int xbDate::IsLeapYear( const char * Date8 )
{
   int year;
   year = YearOf( Date8 );
   if(( year % 4 == 0 && year % 100 != 0 ) || year % 400 == 0 )
      return 1;
   else
      return 0;
}
/***************************************************************/
/* this function returns the "day of" from a CCYYMMDD date     */

/*  format = XB_FMT_WEEK       Number of day in WEEK  0-6 ( Sun - Sat )
    format = XB_FMT_MONTH      Number of day in MONTH 1-31 
    format = XB_FMT_YEAR       Number of day in YEAR  1-366
*/

int xbDate::DayOf( int Format, const char * Date8 )
{
   char day[3];
   int  iday, imonth, iyear, iday2;

   /* check for valid format switch */

   if( Format != XB_FMT_WEEK && Format != XB_FMT_MONTH && Format != XB_FMT_YEAR )
      return -1;

   if( Format == XB_FMT_WEEK )
   {
      iday =   DayOf( XB_FMT_MONTH, Date8 );  
      imonth = MonthOf( Date8 );
      iyear  = YearOf ( Date8 );

      /* The following formula uses Zeller's Congruence to determine
         the day of the week */

      if( imonth > 2 )           /* init to February */
         imonth -= 2;
      else
      {
         imonth += 10;
         iyear--;
      }

      iday2 = ((13 * imonth - 1) / 5) +iday + ( iyear % 100 ) +
              (( iyear % 100 ) / 4) + ((iyear /100 ) / 4 ) - 2 *
              ( iyear / 100 ) + 77 ;

      return( iday2 - 7 * ( iday2 / 7 ));
   }

   else if( Format == XB_FMT_MONTH )
   {
      day[0] = Date8[6];
      day[1] = Date8[7];
      day[2] = 0x00;
      return( atoi( day ));
   }
   else
   {      
      imonth = MonthOf( Date8 );
      iday   = DayOf( XB_FMT_MONTH, Date8 );

      if( !IsLeapYear( Date8 ))
         iday2 = NonLeapTable[imonth-1] + iday;
      else
         iday2 = LeapTable[imonth-1] + iday;

      return iday2;
   }
}
/***************************************************************/
/* this function returns a pointer to a system date            */
char * xbDate::Sysdate( void )
{
   static char dt[9];
   time_t timer;
   struct tm *tblock;
   timer = time( NULL );
   tblock = localtime( &timer );
   tblock->tm_year += 1900;
   tblock->tm_mon++;
   sprintf( dt,"%4d%02d%02d",tblock->tm_year,tblock->tm_mon,tblock->tm_mday );
   dt[8] = 0x00;
   return dt;
}
/***************************************************************/
/* this function checks a date for validity - returns 1 if OK  */

int xbDate::DateIsValid( const char * Date8 )
{
   int year, month, day;

   if(!isdigit( Date8[0] ) || !isdigit( Date8[1] ) || !isdigit( Date8[2] ) ||
      !isdigit( Date8[3] ) || !isdigit( Date8[4] ) || !isdigit( Date8[5] ) ||
      !isdigit( Date8[6] ) || !isdigit( Date8[7] ) )
         return 0;

   year  = YearOf ( Date8 );
   month = MonthOf( Date8 );
   day   = DayOf  ( XB_FMT_MONTH, Date8 );   
   
   /* check the basics */
   if( year == 0 || month < 1 || month > 12 || day < 1 || day > 31 )
      return 0;

   /* April, June, September and November have 30 days */
   if(( month==4 || month==6 || month==9 || month==11 )&& day > 30 )
      return 0;

   /* check for February with leap year */
   if( month == 2 && IsLeapYear( Date8 ) && day > 29 )
      return 0;
   else if( month==2 && day>28 )
      return 0;
   return 1;
}
/***************************************************************/
/* this returns the number of days since 1/1/1900              */

long xbDate::JulianDays( const char * Date8 )
{
   int year, i;
   long days;

   year = YearOf( Date8 );
   if( year < 1900 ) return -1;

   for( i = 1900, days = 0L; i < year; i++ )
   {
      days += 365;
      if( i % 4 == 0 && i % 100 != 0 || i % 400 == 0 ) days++;
   }

   days+= (long) DayOf( XB_FMT_YEAR, Date8 ) - 1;
   return days;
}
/***************************************************************/
/* this function does the opposite of the JulianDays function  */
/* it converts a julian based date into a Date8 format         */

char * xbDate::JulToDate8( long days )
{
   static char Date8[9];
   int year, leap, month, i;

   year = 1900;
   leap = 0;               /* 1900 is not a leap year */

/* this while loop calculates the year of the date by incrementing
   the years counter as it decrements the days counter */

   while( days > ( 364+leap ))
   {
      days -= 365+leap;
      year++;
      if( year % 4 == 0 && year % 100 != 0 || year % 400 == 0 )
         leap = 1;
      else 
         leap = 0;
   }    

/* this for loop calculates the month and day of the date by
   comparing the number of days remaining to one of the tables   */

/* this loop cant handle january */

   for( i = 11, month = 0; i >= 0 && month == 0; i-- )
   {
      if( leap && days >= (long) LeapTable[i] )
      {
         month = i+1;
         days -= LeapTable[i]-1;
      }
      else if( !leap && days >= (long) NonLeapTable[i] )
      {
         month = i+1;
         days -= NonLeapTable[i]-1;
      }
   }   

   sprintf( Date8, "%4d%02d%02ld", year, month, days );
   Date8[8] = 0x00;
   return( Date8 );
}
/***************************************************************/
/* this routine returns a pointer to the day of the week(Sun-Sat)*/

char * xbDate::CharDayOf( const char * Date8 )
{
   return( Days[DayOf( XB_FMT_WEEK, Date8 )]);
}
/***************************************************************/
/* this routine returns a pointer to the month                 */

char * xbDate::CharMonthOf( const char * Date8 )
{
   return( Months[MonthOf( Date8 ) - 1]);
}
/***************************************************************/
/* This function formats a date and returns a pointer to a     */
/* static buffer containing the date                           */

char * xbDate::FormatDate( const char * Format, const char * Date8 )
{
   const char *FmtPtr;     /* format pointer */
   char *BufPtr;           /* buffer pointer */
   char *ptr;              /* misc ptr */
   char type;        
   char cbuf[10];
   int  type_ctr, i;
   static char buf[50];

   memset( buf, 0x00, 50 );
   if( strstr( Format, "YYDDD" ))
   {
      buf[0] = Date8[2];
      buf[1] = Date8[3];
      sprintf( buf+2, "%03d", DayOf( XB_FMT_YEAR, Date8 ));
   }
   else
   {
      BufPtr = buf;
      FmtPtr = Format;
      memset( cbuf, 0x00, 10 );
      while( *FmtPtr )
      {
         if( *FmtPtr != 'D' && *FmtPtr != 'M' && *FmtPtr != 'Y' )
         {
            *BufPtr = *FmtPtr;
            BufPtr++;
            FmtPtr++;
         }
         else
         { 
            type = *FmtPtr;
            type_ctr = 0;
            while( *FmtPtr == type )
            {
               type_ctr++;
               FmtPtr++;
            }
            switch( type )
            {
               case 'D':         
                  if( type_ctr == 1 )
                  {
                     sprintf( cbuf, "%d", DayOf( XB_FMT_MONTH, Date8 ));
                     strcat( buf, cbuf );
                     BufPtr += strlen( cbuf );
                  }
                  else if( type_ctr == 2 )
                  {
                     cbuf[0] = Date8[6];
                     cbuf[1] = Date8[7];
                     cbuf[2] = 0x00;
                     strcat( buf, cbuf );
                     BufPtr += 2;
                  }
                  else
                  {
                     ptr = CharDayOf( Date8 );
                     if( type_ctr == 3 )
                     {
                        strncat( buf, ptr, 3 );
                        BufPtr += 3;   
                     }
                     else
                     {
                        strcpy( cbuf, CharDayOf( Date8 ));
                        for( i = 0; i < 9; i++ )
                           if( cbuf[i] == 0x20 ) cbuf[i] = 0x00;
                        strcat( buf, cbuf );
                        BufPtr += strlen( cbuf );
                     }
                  }         
                  break;

               case 'M':
                  if( type_ctr == 1 )
                  {
                     sprintf( cbuf, "%d", MonthOf( Date8 ));
                     strcat( buf, cbuf );
                     BufPtr += strlen( cbuf );
                  }
                  else if( type_ctr == 2 )
                  {
                     cbuf[0] = Date8[4];
                     cbuf[1] = Date8[5];
                     cbuf[2] = 0x00;
                     strcat( buf, cbuf );
                     BufPtr += 2;
                  }
                  else
                  {
                     ptr = CharMonthOf( Date8 );
                     if( type_ctr == 3 )
                     {
                        strncat( buf, ptr, 3 );
                        BufPtr += 3;   
                     }
                     else
                     {
                        strcpy( cbuf, CharMonthOf( Date8 ));
                        for( i = 0; i < 9; i++ )
                           if( cbuf[i] == 0x20 ) cbuf[i] = 0x00;
                        strcat( buf, cbuf );
                        BufPtr += strlen( cbuf );
                     }
                  }
                  break;
            
               case 'Y':
                  if( type_ctr == 2 )
                  {
                     cbuf[0] = Date8[2];
                     cbuf[1] = Date8[3];
                     cbuf[2] = 0x00;
                     strcat( buf, cbuf );
                     BufPtr += 2;
                  }
                  else if( type_ctr == 4 )
                  {
                     cbuf[0] = Date8[0];
                     cbuf[1] = Date8[1];
                     cbuf[2] = Date8[2];
                     cbuf[3] = Date8[3];
                     cbuf[4] = 0x00;
                     strcat( buf, cbuf );
                     BufPtr += 4;
                  }
                  break;

               default:
                  break;
            }
         }
      }
   }
   return buf;
}
/***************************************************************/
/* this routine returns the julian last day of the month for
   a given input date */

long xbDate::LastDayOfMonth( const char * d )
{
   int y,m;
   char wdate[9];

   y = YearOf( d );
   if(( m = MonthOf( d )) == 12 )
   {
      m = 1;
      y++;
   }
   else
      m++;

   memset( wdate, 0x00, 9 );
   sprintf( wdate, "%4d%2d01", y, m );
   return( JulianDays( wdate ) - 1L );
}
