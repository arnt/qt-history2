/*  $Id: deletall.cpp,v 1.4 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

    This sample program deletes all records in an Xbase DBF file

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
    V 1.8   11/29/98   - Version 1.8 upgrade
*/

#include <xdb/xbase.h>

int main(int ac,char** av)
{
    if (ac <= 1) {
        cout <<
            "\nUsage: deletall filename...\n"
            ;
        return 1;
    }
    
    for (int i=1; i<ac; ++i) {
        char* filename = av[i];

   xbXBase x;
   xbDbf MyFile( &x );

        if( MyFile.OpenDatabase( filename )) {
      cout << "Could not open file " << filename << "\n";
            return 2;
   }

        xbShort rc = MyFile.DeleteAllRecords(); 
        if( rc != XB_NO_ERROR ) {
      cout << "\nError Deleting all records - database ==> " << filename;
      cout << " Return Code = " << rc;
   }
   MyFile.CloseDatabase();	/* close database */

   cout << "\nDone...\n\n";
    }
    
    return 0;
}     
