/*  $Id: dumpdbt.cpp,v 1.8 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

    This program is used for debugging the memo file logic

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
    V 1.5   1/2/97     - Added memo field support
    V 1.6a  5/1/98     - Added expression support
    V 1.8   11/29/98   - Version 1.8 upgrade
*/

#include <xdb/xbase.h>

int main( int ac, char **av )
{
#ifdef XB_MEMO_FIELDS
   xbXBase x;
   xbDbf dbf( &x );

   if( ac <= 1 ){
     cout << "\nUsage: dumpdbt filename...\n";
     return 1;
   }

   for( int i = 1; i < ac; ++i ){
     char* filename = av[i];
     xbDbf dbf( &x );
      

     if( dbf.OpenDatabase( filename )){
        cout << "\nCant open input file " << filename;
        return 2;
     }
  
     cout << "\n\nFree Block Chain....";
#ifdef XBASE_DEBUG
     dbf.DumpMemoFreeChain();
     cout <<"\nEnd of free block chain\n***********************************";
#else
     cout << "\nXBASE_DEBUG is not compiled in\n";
#endif

   /* lock the memo file */
#  ifdef XB_LOCKING_ON
     dbf.LockMemoFile( F_SETLK, F_RDLCK );
#  endif /* XB_LOCKING_ON */

        if( !dbf.MemoFieldsPresent() ) {
            cout << "No memo fields exist in " << filename << endl;
        } else {
            xbLong BufSize = 0L;
            char* Buf = NULL;
            for( xbLong l = 1; l <= dbf.NoOfRecords(); l++ )
      {
         dbf.GetRecord( l );
         cout << "\nRecord # " << dbf.GetCurRecNo();
                for( int j = 0; j < dbf.FieldCount(); j++ ) {
                    if( dbf.GetFieldType( j ) == 'M' ) {
                        int len = dbf.GetMemoFieldLen( j );
               cout << "\nMemo field " << dbf.GetFieldName(j) << " length = " << len;
               cout << " Head Block = " << dbf.GetLongField( j ) << "\n";
               if( len > BufSize )
               {
                  if( BufSize ) free( Buf );
                  if(( Buf = (( char *) malloc( len ))) == NULL )
                     return XB_NO_MEMORY;
                  BufSize = len;
               }
               dbf.GetMemoField( j, len, Buf, F_SETLKW );
                        for( int i = 0; i < len; i++ )
                  cout << Buf[i];
            }
      }
   }
        

   /* unlock the memo file */
#  ifdef XB_LOCKING_ON
       dbf.LockMemoFile( F_SETLK, F_UNLCK );
#  endif /* XB_LOCKING_ON */

       cout << "\n";
       dbf.CloseDatabase();
     }
#else
     cout << "\nXB_MEMO_FIELDS is not compiled in\n";
#endif
   }
   return 0;
}
