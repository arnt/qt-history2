/*  $Id: checkndx.cpp,v 1.6 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

    Copyright (C) 1997  Startech, Gary A. Kunkel   
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
    V 1.5   1/2/98     - Added memo file support
    V 1.6a  5/1/98     - Added expression suppport
    V 1.8   11/29/98   - Version 1.8 upgrade
*/

#include <xdb/xbase.h>

int main(int ac,char** av)
{
#ifdef XBASE_DEBUG
   xbXBase x;
   xbDbf d( &x );
   xbNdx i( &d );
   xbShort rc;

   if( 3 != ac ){
     cout << 
       "\nUsage: checkndx dbf_file index_file\n";
       return 1;
   }
 
   if(( rc = d.OpenDatabase( av[1] )) != XB_NO_ERROR )
   {
      cout << "\nCould not open file " << av[1] << " rc = " << rc << "\n";
      return 2;
   }

   if(( rc = i.OpenIndex( av[2] )) != XB_NO_ERROR )
   {
      cout << "\nCould not open file " << av[2] << " rc = " << rc << "\n";
      return 3;
   }

   cout << "\nRunning...\n";
   rc = i.CheckIndexIntegrity( 1 );
   cout << "\nNdx integrity check = " << rc << "\n"; 

   i.DumpHdrNode();

   d.CloseDatabase();
#else
   cout << "\nXBASE_DEBUG is not compiled in\n";
#endif
   return 0;
}
