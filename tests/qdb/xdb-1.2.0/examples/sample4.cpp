/*  $Id: sample4.cpp,v 1.6 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

    This sample program reads the database created and updated by the sample1 
    and sample2 program

    This program demonstrates the use of the following functions/methods
      DeleteRecord, UndeleteRecord, RecordDeleted

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
    V 1.5   1/2/97     - Added memo field support
    V 1.6a  5/1/98     - Added expression support
    V 1.8   11/29/98   - Version 1.8 upgrade
*/

#include <xdb/xbase.h>

#ifdef __XBDOS
#include <stdio.h>
extern unsigned _stklen = 100000;
#endif

int main()
{
  xbShort rc;
  xbXBase x;
  xbDbf MyFile( &x );
#ifdef XB_INDEX_NDX
  xbNdx MyIndex1( &MyFile );
  xbNdx MyIndex2( &MyFile );
  xbNdx MyIndex3( &MyFile );
  xbNdx MyIndex4( &MyFile );
#endif // XB_INDEX_NDX

  if(( rc = MyFile.OpenDatabase( "MYFILE.DBF" )) != XB_NO_ERROR )
  {
     cout << "\nError opening file rc = " << rc << "\n";
     exit(1);
  }
#ifdef XB_INDEX_NDX
  if(( rc = MyIndex1.OpenIndex( "MYINDEX1.NDX" )) != XB_NO_ERROR )
  {
     cout << "\nError opening index1 rc = " << rc << "\n";
     exit(1);
  }

  if(( rc = MyIndex2.OpenIndex( "MYINDEX2.NDX" )) != XB_NO_ERROR )
  {
     cout << "\nError opening index2 rc = " << rc << "\n";
     exit(1);
  }
  if(( rc = MyIndex3.OpenIndex( "MYINDEX3.NDX" )) != XB_NO_ERROR )
  {
     cout << "\nError opening index3 rc = " << rc << "\n";
     exit(1);
  }
  if(( rc = MyIndex4.OpenIndex( "MYINDEX4.NDX" )) != XB_NO_ERROR )
  {
     cout << "\nError opening index4 rc = " << rc << "\n";
     exit(1);
  }
#endif // XB_INDEX_NDX


  cout << "Sample GetRecord\n"; 

  MyFile.ExclusiveLock( F_SETLKW ); /* lock the files for our exclusive use */

  MyFile.GetRecord( 2L );

  MyFile.DeleteRecord();
  
  if( MyFile.RecordDeleted() )
    cout << "Record is deleted...\n";
  else 
    cout << "Record is not deleted...\n";


/* to undelete a record the following commented code could be used

  MyFile.UndeleteRecord();
  if( MyFile.RecordDeleted() )
     cout << "Record is deleted...\n";
  else 
    cout << "Record is not deleted...\n";
*/

/* to permanently remove deleted records from the file, pack the database */

   if(( rc = MyFile.PackDatabase( F_SETLKW )) != XB_NO_ERROR )
      cout << "\nError packing database rc = " << rc;
   else
      cout << "\nDatabase packed.\n";

  MyFile.ExclusiveUnlock();     /* unlock the files */
  MyFile.CloseDatabase();	/* close database */
  return 0;
}     
