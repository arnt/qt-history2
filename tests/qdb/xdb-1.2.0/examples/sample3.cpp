/*  $Id: sample3.cpp,v 1.8 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

    This sample program reads the database created and updated by the sample1 
    and sample2 program

    This program demonstrates the use of the following functions/methods
       OpenDatabase, GetFieldNo, GetRecord, GetLastRecord, GetFirstRecord, 
       GetNextRecord, GetPrevRecord, NoOfRecords and CloseDatabase, FieldCount

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

    V 1.0    10/10/97    - Initial release of software
    V 1.5     1/2/98     - Added memo field support
    V 1.6a    5/1/98     - Added expression support
    V 1.7.4c 10/26/98    - Added some memo field logic
    V 1.8    11/29/98    - Version 1.8 upgrade 
*/

#include <xdb/xbase.h>

/* set the stack large for dos compiles */
#ifdef __XBDOS
#include <stdio.h>
extern unsigned _stklen = 100000;
#endif

int main()
{
  char buf[40];
  xbShort rc,i;
  xbShort lname, fname, birthdate, amount, sw, f1, memo;
  xbULong recs;
  char *p;
  xbFloat f;

  xbXBase x;
  xbDbf MyFile( &x );
  MyFile.OpenDatabase( "MYFILE.DBF" );

  lname     = MyFile.GetFieldNo( "LASTNAME" );
  fname     = MyFile.GetFieldNo( "FIRSTNAME" );
  birthdate = MyFile.GetFieldNo( "BIRTHDATE" ); 
  amount    = MyFile.GetFieldNo( "AMOUNT" );
  sw        = MyFile.GetFieldNo( "SWITCH" );
  f1        = MyFile.GetFieldNo( "FLOAT1" );
  memo      = MyFile.GetFieldNo( "MEMO1" );

  cout << "\nThere are " << MyFile.FieldCount() << " fields in the file";

  cout << "\nSample NoOfRecs" << endl;
  recs = MyFile.NoOfRecords();
  cout << "There are " << recs << " records in the file." << endl;  

  cout << "\nLASTNAME  is type: " << MyFile.GetFieldType( lname )
   << " length: " << MyFile.GetFieldLen( lname );
  cout << "\nFIRSTNAME  is type: " << MyFile.GetFieldType( fname )
   << " length: " << MyFile.GetFieldLen( fname );
  cout << "\nBIRTHDATE is type: " << MyFile.GetFieldType( birthdate )
   << " length: " << MyFile.GetFieldLen( birthdate );
  cout << "\nAMOUNT    is type: " << MyFile.GetFieldType( amount )
   << " length: " << MyFile.GetFieldLen( amount );
  cout << "\nSWITCH    is type: " << MyFile.GetFieldType( sw )
   << " length: " << MyFile.GetFieldLen( sw ) << "\n\n";

  cout << "Sample GetRecord" << endl; 
  MyFile.GetRecord( 1L );
  MyFile.GetField( lname, buf );  
  cout << "Name 1 = " << buf << endl;

  f = MyFile.GetFloatField( f1 );
  cout << "FLOAT1 = " << f << endl;
  f = MyFile.GetFloatField( "FLOAT2" );
  cout << "FLOAT2 = " << f << endl;

  cout << "Sample GetRecordBuf" << endl; 
  p = MyFile.GetRecordBuf();			/* pointer to record buffer */
  for( i = 0; i < 49; i++ ) cout << *p++;	/* display the buffer */

  cout << "\n\nLoop through forwards..." << endl;
  rc = MyFile.GetFirstRecord(); 
  while( rc == XB_NO_ERROR )
  {
    MyFile.GetField( lname, buf );  
    cout << MyFile.GetCurRecNo();
    cout << " Name = " << buf << endl;
    rc = MyFile.GetNextRecord();
#ifdef XB_MEMO_FIELDS
    if( MyFile.MemoFieldExists( memo ))
     cout << "Memo field MEMO1 len = " << MyFile.GetMemoFieldLen(memo) << endl;
#endif
  }

  cout << "\n\nSample PutRecord" << endl << endl;
  MyFile.PutField( lname, "Stantonbob" );
  MyFile.PutField( lname, "Sally" );
  MyFile.PutRecord( MyFile.GetCurRecNo() );

  cout << "\nLoop through backwards.." << endl;
  rc = MyFile.GetLastRecord(); 
  while( rc == XB_NO_ERROR )
  {
//    MyFile.GetField( lname, buf );  
    cout << "Last Name = " << MyFile.GetStringField( lname ) << endl;
    cout << "Logical Field = " << MyFile.GetLogicalField( "SWITCH" ) << endl;
    rc = MyFile.GetPrevRecord();
  }

  cout << endl;
  MyFile.CloseDatabase();				/* close database */
  return 0;
}     
