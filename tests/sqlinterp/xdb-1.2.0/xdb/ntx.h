/*  $Id: ntx.h,v 1.3 1999/03/19 10:56:34 willy Exp $

    Xbase project source code

    This file contains a header file for the xbNdx object, which is used
    for handling xbNdx type indices.

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
*/

#ifndef __XB_NTX_H__
#define __XB_NTX_H__

#include <xdb/xbase.h>
#include <string.h>

#define XB_NTX_NODE_SIZE 1024

struct NtxHeadNode {			/* ntx header on disk */
    xbUShort Signature;           /* Clipper 5.x or Clipper 87 */
    xbUShort Version;             /* Compiler Version */
                                /* Also turns out to be a last modified counter */
    xbULong StartNode;            /* Offset in file for first index */
    xbULong  UnusedOffset;        /* First free page offset */
    xbUShort KeySize;             /* Size of items (KeyLen + 8) */
    xbUShort KeyLen;              /* Size of the Key */
    xbUShort DecimalCount;        /* Number of decimal positions */
    xbUShort KeysPerNode;         /* Max number of keys per page */
    xbUShort HalfKeysPerNode;     /* Min number of keys per page */
    char KeyExpression[256];    /* Null terminated key expression */
    unsigned  Unique;              /* Unique Flag */
    char NotUsed[745];
};

struct NtxLeafNode {			/* ndx node on disk */
    xbUShort NoOfKeysThisNode;
    char     KeyRecs[XB_NTX_NODE_SIZE];
};


struct NtxItem
{
    xbULong Node;
    xbULong RecordNumber;
    char Key[256];
};

struct xbNodeLink {			/* ndx node memory */
   xbNodeLink * PrevNode;
   xbNodeLink * NextNode;
   xbUShort       CurKeyNo;                 /* 0 - KeysPerNode-1 */
   xbULong       NodeNo;
   struct NtxLeafNode Leaf;
    xbUShort *offsets;
};

class XBDLLEXPORT xbNtx : public xbIndex  {
 public:
   NtxHeadNode HeadNode;
   NtxLeafNode LeafNode;
   xbLong NodeLinkCtr;
   xbLong ReusedNodeLinks;

   char  Node[XB_NTX_NODE_SIZE];

   xbNodeLink * NodeChain;        /* pointer to node chain of index nodes */
   xbNodeLink * FreeNodeChain;    /* pointer to chain of free index nodes */
   xbNodeLink * CurNode;          /* pointer to current node              */
   xbNodeLink * DeleteChain;      /* pointer to chain to delete           */
   xbNodeLink * CloneChain;       /* pointer to node chain copy (add dup) */

   NtxItem PushItem;

/* private functions */
   xbULong     GetLeftNodeNo( xbShort, xbNodeLink * );
   xbShort    CompareKey( const char *, const char *, xbShort );
   xbShort    CompareKey( const char *, const char * );
   xbLong     GetDbfNo( xbShort, xbNodeLink * );
   char *   GetKeyData( xbShort, xbNodeLink * );
   xbUShort   GetItemOffset ( xbShort, xbNodeLink *, xbShort );
   xbUShort   InsertKeyOffset ( xbShort, xbNodeLink * );
   xbUShort   GetKeysPerNode( void );
   xbShort    GetHeadNode( void );    
   xbShort    GetLeafNode( xbLong, xbShort );
   xbNodeLink * GetNodeMemory( void );
   xbULong    GetNextNodeNo( void );
   void     ReleaseNodeMemory( xbNodeLink * );
   xbULong     GetLeafFromInteriorNode( const char *, xbShort );
   xbShort    CalcKeyLen( void );
   xbShort    PutKeyData( xbShort, xbNodeLink * );
   xbShort    PutLeftNodeNo( xbShort, xbNodeLink *, xbLong );
   xbShort    PutLeafNode( xbLong, xbNodeLink * );
   xbShort    PutHeadNode( NtxHeadNode *, FILE *, xbShort );
   xbShort    TouchIndex( void );
   xbShort    PutDbfNo( xbShort, xbNodeLink *, xbLong );
   xbShort    PutKeyInNode( xbNodeLink *, xbShort, xbLong, xbLong, xbShort );
   xbShort    SplitLeafNode( xbNodeLink *, xbNodeLink *, xbShort, xbLong ); 
   xbShort    SplitINode( xbNodeLink *, xbNodeLink *, xbLong );
   xbShort    AddToIxList( void );
   xbShort    RemoveFromIxList( void );
   xbShort    RemoveKeyFromNode( xbShort, xbNodeLink * );
   xbShort    DeleteKeyFromNode( xbShort, xbNodeLink * );
   xbShort    JoinSiblings(xbNodeLink *, xbShort, xbNodeLink *, xbNodeLink *);
   xbUShort   DeleteKeyOffset( xbShort, xbNodeLink *);
   xbShort    FindKey( const char *, xbShort, xbShort );      
   xbShort    UpdateParentKey( xbNodeLink * );
   xbShort    GetFirstKey( xbShort );
   xbShort    GetNextKey( xbShort );
   xbShort    GetLastKey( xbLong, xbShort );
   xbShort    GetPrevKey( xbShort ); 
   void     UpdateDeleteList( xbNodeLink * );
   void     ProcessDeleteList( void );
//    xbNodeLink * LeftSiblingHasSpace( xbNodeLink * );
//    xbNodeLink * RightSiblingHasSpace( xbNodeLink * );
//    xbShort    DeleteSibling( xbNodeLink * );
//    xbShort    MoveToLeftNode( xbNodeLink *, xbNodeLink * );
//    xbShort    MoveToRightNode( xbNodeLink *, xbNodeLink * );
   xbShort    FindKey( const char *, xbLong );         /* for a specific dbf no */

   xbShort    CloneNodeChain( void );          /* test */
   xbShort    UncloneNodeChain( void );        /* test */
   



   xbNtx      ( xbDbf * );

/* note to gak - don't uncomment next line - it causes seg faults */
//   ~NTX() { if( NtxStatus ) CloseIndex(); }  

   xbShort  OpenIndex ( const char * );
   xbShort  CloseIndex( void );
   void   DumpHdrNode  ( void );
   void   DumpNodeRec  ( xbLong ); 
   xbShort  CreateIndex( const char *, const char *, xbShort, xbShort );
   xbLong   GetTotalNodes( void );
   xbLong   GetCurDbfRec( void ) { return CurDbfRec; }
   void   DumpNodeChain( void );
   xbShort  CreateKey( xbShort, xbShort );
   xbShort  GetCurrentKey(char *key);
   xbShort  AddKey( xbLong );
   xbShort  UniqueIndex( void ) { return HeadNode.Unique; }
   xbShort  DeleteKey( xbLong DbfRec );
   xbShort  KeyWasChanged( void );
   xbShort  FindKey( const char * );
   xbShort  FindKey( void );
   xbShort  FindKey( xbDouble );
#ifdef XBASE_DEBUG
   xbShort  CheckIndexIntegrity( const xbShort Option );
#endif
   xbShort  GetNextKey( void )  { return GetNextKey( 1 ); }
   xbShort  GetLastKey( void )  { return GetLastKey( 0, 1 ); }
   xbShort  GetFirstKey( void ) { return GetFirstKey( 1 ); }
   xbShort  GetPrevKey( void )  { return GetPrevKey( 1 ); }
   xbShort  ReIndex(void (*statusFunc)(xbLong itemNum, xbLong numItems) = 0) ;
   xbShort  KeyExists( char * Key ) { return FindKey( Key, strlen( Key ), 0 ); }
   xbShort  KeyExists( xbDouble );

   virtual void GetExpression(char *buf, int len);
};
#endif		/* __XB_NTX_H__ */
