/*  $Id: indextst.cpp,v 1.2 1999/03/19 10:56:32 willy Exp $

    Xbase project source code

    This program creates a sample database and multiple indices.
    It tests the index logic.

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
/* FIXME?  what compiler requires this and how to tell that compiler?
   __XBDOS has very unclean semantics -- willy */
extern unsigned _stklen = 100000;
#endif

main()
{
  xbShort f1, f2, f3, rc, sts = 0;
  char charbuf[10];

  xbSchema MyRecord[] = 
  {
    { "CHARFLD1",  XB_CHAR_FLD,      6, 0 },
    { "CHARFLD2",  XB_CHAR_FLD,      6, 0 },
    { "NUMFLD1",   XB_NUMERIC_FLD,   6, 0 },
    { "",0,0,0 }
  };

  /* define the classes */
  xbXBase x;			/* initialize xbase  */
  xbDbf MyFile( &x );		/* class for table   */
#ifdef XB_INDEX_ANY
#ifdef XB_INDEX_NDX
  xbNdx indx1( &MyFile );	/* class for ndx index 1 */
  xbNdx indx2( &MyFile );	/* class for ndx index 2 */
  xbNdx indx3( &MyFile );	/* class for ndx index 3 */
#endif // XB_INDEX_NDX
#ifdef XB_INDEX_NTX
  xbNtx intx1( &MyFile );	/* class for ntx index 1 */
  xbNtx intx2( &MyFile );	/* class for ntx index 2 */
  xbNtx intx3( &MyFile );	/* class for ntx index 3 */
#endif // XB_INDEX_NTX
#endif // XB_INDEX_ANY

  cout << "Creating test database and indices" << endl;
  if(( rc = MyFile.CreateDatabase( "IXTEST.DBF", MyRecord, XB_OVERLAY )) 
        != XB_NO_ERROR )
     cout << "Error creating database = " << rc << "\n";
  else
  {
#ifdef XB_INDEX_ANY
#ifdef XB_INDEX_NDX  
     if(( rc = indx1.CreateIndex( 
       "IXNDX1.NDX", "CHARFLD1", XB_NOT_UNIQUE, XB_OVERLAY )) != XB_NO_ERROR )
     {
        cout << "Error creating index 1 = " << rc << endl;       
        exit( 1 );
     }

     if(( rc = indx2.CreateIndex( 
       "IXNDX2.NDX", "CHARFLD1+CHARFLD2", XB_NOT_UNIQUE, XB_OVERLAY )) != XB_NO_ERROR )
     {
        cout << "Error creating index 2 = " << rc << endl;
        exit( 1 );
     }

     if(( rc = indx3.CreateIndex( 
       "IXNDX3.NDX", "NUMFLD1", XB_NOT_UNIQUE, XB_OVERLAY )) != XB_NO_ERROR )
     {
        cout << "Error creating index 3 = " << rc << endl;
        exit( 1 );
     }
#endif // XB_INDEX_NDX

#ifdef XB_INDEX_NTX
     if(( rc = intx1.CreateIndex( 
       "IXNTX1.NTX", "CHARFLD1", XB_NOT_UNIQUE, XB_OVERLAY )) != XB_NO_ERROR )
     {
        cout << "Error creating index 4 = " << rc << endl;
        exit( 1 );
     }

     if(( rc = intx2.CreateIndex( 
       "IXNTX2.NTX", "CHARFLD1+CHARFLD2", XB_NOT_UNIQUE, XB_OVERLAY )) != XB_NO_ERROR )
     {
        cout << "Error creating index 5 = " << rc << endl;
        exit( 1 );
     }

     if(( rc = intx3.CreateIndex( 
       "IXNTX3.NTX", "NUMFLD1", XB_NOT_UNIQUE, XB_OVERLAY )) != XB_NO_ERROR )
     {
        cout << "Error creating index 6 = " << rc << endl;
        exit( 1 );
     }
#endif // XB_INDEX_NTX
#endif // XB_INDEX_ANY
  }

  f1 = MyFile.GetFieldNo( "CHARFLD1" );
  f2 = MyFile.GetFieldNo( "CHARFLD2" );
  f3 = MyFile.GetFieldNo( "NUMFLD1" ); 

  cout << "Populating database and indices with data" << endl;
  for( int i = 0; i < 1000; i++ ){
    memset( charbuf, 0x00, 10 );
    sprintf( charbuf, "%d", i );
    MyFile.BlankRecord();
    MyFile.PutField( f1, charbuf );
    MyFile.PutField( f2, charbuf );
    MyFile.PutLongField( f3, i );
    MyFile.AppendRecord();
  }

#ifdef XB_INDEX_ANY
#ifdef XB_INDEX_NDX
  cout << "Testing NDX index 1" << endl;
  if(( rc = indx1.CheckIndexIntegrity(0)) != XB_NO_ERROR ){
    cout << "Error " << rc << " with index indx1" << endl;
    sts++;
  }

  cout << "Testing NDX index 2" << endl;
  if(( rc = indx2.CheckIndexIntegrity(0)) != XB_NO_ERROR ){
    cout << "Error " << rc << " with index indx2" << endl;
    sts++;
  }

  cout << "Testing NDX index 3" << endl;
  if(( rc = indx3.CheckIndexIntegrity(0)) != XB_NO_ERROR ){
    cout << "Error " << rc << " with index indx3" << endl;
    sts++;
  }
#endif

#ifdef XB_INDEX_NTX
  cout << "Testing NTX index 1" << endl;
  if(( rc = intx1.CheckIndexIntegrity(1)) != XB_NO_ERROR ){
    cout << "Error " << rc << " with index intx1" << endl;
    sts++;
  }

  cout << "Testing NTX index 2" << endl;
  if(( rc = intx2.CheckIndexIntegrity(1)) != XB_NO_ERROR ){
    cout << "Error " << rc << " with index intx2" << endl;
    sts++;
  }

  cout << "Testing NTX index 3" << endl;
  if(( rc = intx3.CheckIndexIntegrity(1)) != XB_NO_ERROR ){
    cout << "Error " << rc << " with index intx3" << endl;
    sts++;
  }
#endif // XB_INDEX_NTX
#endif // XB_INDEX_ANY

  cout << "Index testing completed" << endl;
  MyFile.CloseDatabase();   /* Close database and associated indexes */
  return sts;
}     
