/*  $Id: sample5.cpp,v 1.5 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

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
    V 1.8   1/29/99    - V1.8 upgrade
*/

#include <xdb/xbase.h>

/* set the stack large for dos compiles */
#ifdef __XBDOS
#include <stdio.h>
extern unsigned _stklen = 100000;
#endif

int main()
{
   xbShort rc;     /* return code */
   xbXBase x;
   xbDbf d( &x );
#ifdef XB_INDEX_NDX
   xbNdx i1( &d );
   xbNdx i2( &d );
   xbNdx i3( &d );
   xbNdx i4( &d );
#endif // XB_INDEX_NDX

   rc = d.OpenDatabase( "MYFILE" );
   cout << "OpenDatabase rc = " << rc << "\n";

#ifdef XB_INDEX_NDX
   rc = i1.OpenIndex( "MYINDEX1" );
   cout << "OpenIndex 1 rc = " << rc << "\n";

   rc = i2.OpenIndex( "MYINDEX2" );
   cout << "OpenIndex 2 rc = " << rc << "\n";

   rc = i3.OpenIndex( "MYINDEX3" );
   cout << "OpenIndex 3 rc = " << rc << "\n";

   rc = i4.OpenIndex( "MYINDEX4" );
   cout << "OpenIndex 4 rc = " << rc << "\n";

   rc = i1.FindKey( "Queue" );			/* alpha search   */
   cout << "Find Key rc = " << rc << "\n";

   rc = i4.FindKey( (xbDouble) 77.77 );		/* numeric search */
   cout << "Find Key rc = " << rc << "\n";

   rc = i1.GetFirstKey();
   cout << "GetFirstKey rc = " << rc << "\n";

   rc = i1.GetNextKey();
   cout << "GetNextKey rc = " << rc << "\n";

   rc = i1.GetLastKey();
   cout << "GetLastKey rc = " << rc << "\n";

   rc = i1.GetPrevKey();
   cout << "GetPrevKey rc = " << rc << "\n";
#endif // XB_INDEX_NDX

   d.CloseDatabase();
   return 0;
}
