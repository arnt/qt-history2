/*  $Id: index.h,v 1.4 1999/03/19 10:56:34 willy Exp $

    Xbase project source code

    This file contains a header file for the NTX object, which is used
    for handling NTX type indices. NTX are the Clipper equivalant of xbNdx
    files.

    Copyright (C) 1998  SynXis Corp., Bob Cotton
    email - bob@synxis.com
    www   - http://www.synxis.com

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

    V 1.0   9/1/98   - Initial release of software
*/

#ifndef __XB_INDEX_H__
#define __XB_INDEX_H__

#include <xdb/xbase.h>
#include <string.h>

#define XB_UNIQUE     1
#define XB_NOT_UNIQUE 0

class XBDLLEXPORT xbIndex
{
 public:
// protected:
    xbIndex *index;
    xbDbf *dbf;
    xbExpNode *ExpressionTree;

    xbString IndexName;
    FILE *indexfp;

    int IndexStatus;            /* 0 = closed, 1 = open */

    xbLong  CurDbfRec;		/* current Dbf record number */
    char  *KeyBuf;               /* work area key buffer */
    char  *KeyBuf2;              /* work area key buffer */

#ifdef XB_LOCKING_ON
protected:
    int CurLockCount;
    int CurLockType;
#endif

    xbShort NodeSize;

public:
    xbIndex(xbDbf *);

    virtual xbShort  OpenIndex ( const char * ) = 0;
    virtual xbShort  CloseIndex( void ) = 0;
#ifdef XBASE_DEBUG
    virtual void     DumpHdrNode  ( void ) = 0;
    virtual void     DumpNodeRec  ( xbLong ) = 0;
    virtual void     DumpNodeChain( void ) = 0;
    virtual xbShort  CheckIndexIntegrity( const xbShort ) = 0;
#endif
    virtual xbShort  CreateIndex( const char *, const char *, xbShort, xbShort ) = 0;
//   virtual xbLong   GetTotalNodes( void ) = 0;
    virtual xbLong   GetCurDbfRec( void ) = 0;
    virtual xbShort  CreateKey( xbShort, xbShort ) = 0;
    virtual xbShort  GetCurrentKey(char *key) = 0;
    virtual xbShort  AddKey( xbLong ) = 0;
    virtual xbShort  UniqueIndex( void ) = 0;
    virtual xbShort  DeleteKey( xbLong DbfRec ) = 0;
    virtual xbShort  KeyWasChanged( void ) = 0;
    virtual xbShort  FindKey( const char * ) = 0;
    virtual xbShort  FindKey( void ) = 0;
    virtual xbShort  FindKey( xbDouble ) = 0;
    virtual xbShort  GetNextKey( void ) = 0;
    virtual xbShort  GetLastKey( void ) = 0;
    virtual xbShort  GetFirstKey( void ) = 0;
    virtual xbShort  GetPrevKey( void ) = 0;
    virtual xbShort  ReIndex(void (*statusFunc)(xbLong itemNum, xbLong numItems) = 0) = 0;
//   virtual xbShort  KeyExists( char * Key ) { return FindKey( Key, strlen( Key ), 0 ); }
    virtual xbShort  KeyExists( xbDouble ) = 0;

#ifdef XB_LOCKING_ON
    virtual xbShort  LockIndex( const xbShort, const xbShort );
#else
    virtual xbShort  LockIndex( const xbShort, const xbShort ) const { return XB_NO_ERROR; }
#endif

    virtual xbShort TouchIndex( void ) { return XB_NO_ERROR; }

    virtual void    SetNodeSize(xbShort size) {}
    virtual xbShort GetNodeSize(void) { return NodeSize; }

    virtual void    GetExpression(char *buf, int len) = 0;

    virtual ~xbIndex(){};
};


#endif /* __XB_INDEX_H__ */
