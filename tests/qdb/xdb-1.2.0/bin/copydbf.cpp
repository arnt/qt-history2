/*  $Id: copydbf.cpp,v 1.4 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

    This sample program copies the structure of one dbf to another
    dbf file

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

*/

#include <xdb/xbase.h>

int main(int ac,char** av)
{
   if (3 != ac) {
      cout <<
         "\nUsage: copydbf filename1 filename2\n"
         "This program copies the structure of one dbf file to another\n";
        return 1;
    }
    
   char* sfilename = av[1];
   char* tfilename = av[2];

   xbXBase x;
   xbDbf MyFile( &x );

    xbShort rc = MyFile.OpenDatabase( sfilename );
    if( rc != XB_NO_ERROR ) {
      cout << "Could not open file " << sfilename << " Error = " << rc << "\n";
        return 2;
   }

    //   OverlaySwitch = 0;    	/* dont overlay existing file if it exists */
    xbShort OverlaySwitch = 1;  /* overlay existing file if it exists */

   if(( rc = MyFile.CopyDbfStructure( tfilename, OverlaySwitch )) != XB_NO_ERROR )
   {
      cout << "Could not copy file " << tfilename << " Error = " << rc << "\n";
      return 3;
   }

   MyFile.CloseDatabase();	/* close database */
   return 0;
}     
