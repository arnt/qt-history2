/*  $Id: sample1.cpp,v 1.6 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

    This program creates a sample database and four indexes

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

/* set the stack large for dos compiles */
#ifdef __XBDOS
#include <stdio.h>
extern unsigned _stklen = 100000;
#endif

int main()
{

  xbSchema MyRecord[] =
  {
    { "FIRSTNAME", XB_CHAR_FLD,     15, 0 },
    { "LASTNAME",  XB_CHAR_FLD,     20, 0 },
    { "BIRTHDATE", XB_DATE_FLD,      8,  0 },
    { "AMOUNT",    XB_NUMERIC_FLD,   9,  2 },
    { "SWITCH",    XB_LOGICAL_FLD,   1,  0 },
    { "FLOAT1",    XB_FLOAT_FLD,     9,  2 },
    { "FLOAT2",    XB_FLOAT_FLD,     9,  1 },
    { "FLOAT3",    XB_FLOAT_FLD,     9,  2 },
    { "FLOAT4",    XB_FLOAT_FLD,     9,  3 },
    { "MEMO1",     XB_MEMO_FLD,     10, 0 },
    { "ZIPCODE",   XB_NUMERIC_FLD,   5,  0 },
    { "",0,0,0 }
  };

  /* define the classes */
  xbXBase x;			/* initialize xbase  */
  xbDbf MyFile( &x );		/* class for table   */
#ifdef XB_INDEX_NDX
  lkjsdflkjsdlkjdsf
  xbNdx MyIndex1( &MyFile );	/* class for index 1 */
  xbNdx MyIndex2( &MyFile );	/* class for index 2 */
  xbNdx MyIndex3( &MyFile );	/* class for index 3 */
  xbNdx MyIndex4( &MyFile );	/* class for index 4 */
#endif // XB_INDEX_NDX


  xbShort rc;
  MyFile.SetVersion( 4 );   /* create dbase IV style files */

  if(( rc = MyFile.CreateDatabase( "MYFILE.DBF", MyRecord, XB_OVERLAY ))
	!= XB_NO_ERROR )
     cout << "\nError creating database = " << rc << "\n";
  else
  {
     /* define a simple index */
#ifdef XB_INDEX_NDX
     if(( rc = MyIndex1.CreateIndex(
       "MYINDEX1.NDX", "LASTNAME", XB_NOT_UNIQUE, XB_OVERLAY )) != XB_NO_ERROR )
	cout << "\nError creating index 1 = " << rc << "\n";

     /* define a multi-field index "LASTNAME            FIRSTNAME" */
     if(( rc = MyIndex2.CreateIndex(
       "MYINDEX2.NDX", "LASTNAME+FIRSTNAME", XB_NOT_UNIQUE, XB_OVERLAY )) != XB_NO_ERROR )
	cout << "\nError creating index 2 = " << rc << "\n";

     /* define a multi-field index "LASTNAMEFIRSTNAME" */
     if(( rc = MyIndex3.CreateIndex(
       "MYINDEX3.NDX", "LASTNAME-FIRSTNAME", XB_NOT_UNIQUE, XB_OVERLAY )) != XB_NO_ERROR )
	cout << "\nError creating index 3 = " << rc << "\n";

     /* define a numeric index "ZIPCODE" */
     if(( rc = MyIndex4.CreateIndex(
       "MYINDEX4.NDX", "ZIPCODE", XB_NOT_UNIQUE, XB_OVERLAY )) != XB_NO_ERROR )
	cout << "\nError creating index 4 = " << rc << "\n";
#endif // XB_INDEX_NDX
  }

  MyFile.CloseDatabase();   /* Close database and associated indexes */
  return 0;
}
