/*  $Id: ndx.h,v 1.10 1999/03/19 10:56:34 willy Exp $

    Xbase project source code

    This file contains a header file for the xbNdx object, which is used
    for handling NDX type indices.

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
    V 1.02  10/25/97   - Index performance enhancements
    V 1.3   11/30/97   - Moved GetLong and GetShort to DBF class for memos
    V 1.5   1/2/98     - Added Dbase IV memo field support
    V 1.6a  4/1/98     - Added expression support
    V 1.6b  4/8/98     - Numeric index support
*/

#ifndef __XB_NDX_H__
#define __XB_NDX_H__

#include <xdb/xbase.h>
#include <string.h>

#define XB_NDX_NODE_BASESIZE            24      // size of base header data

#define XB_VAR_NODESIZE                 // define to enable variable node sizes

#ifndef XB_VAR_NODESIZE
#define XB_NDX_NODE_SIZE 2048
//#define XB_NDX_NODE_SIZE 512          // standard dbase node size
#else
#define XB_DEFAULT_NDX_NODE_SIZE        512
#define XB_MAX_NDX_NODE_SIZE            4096
#define XB_NDX_NODE_SIZE                NodeSize
#define XB_NDX_NODE_MULTIPLE            512
#endif // XB_VAR_NODESIZE

struct xbNdxHeadNode {			/* ndx header on disk */
   xbLong   StartNode;                    /* header node is node 0 */
   xbLong   TotalNodes;                   /* includes header node */
   xbLong   NoOfKeys;                     /* actual count + 1 */
   xbUShort KeyLen;                       /* length of key data */
   xbUShort KeysPerNode;
   xbUShort KeyType;                      /* 00 = Char, 01 = Numeric */
   xbLong   KeySize;                      /* key len + 8 bytes */
   char   Unknown2;
   char   Unique;
//   char   KeyExpression[488];
#ifndef XB_VAR_NODESIZE
   char   KeyExpression[XB_NDX_NODE_SIZE - 24];
#else
   char   KeyExpression[XB_MAX_NDX_NODE_SIZE - 24];
#endif // XB_VAR_NODESIZE
};

struct xbNdxLeafNode {			/* ndx node on disk */
   xbLong   NoOfKeysThisNode;
#ifndef XB_VAR_NODESIZE
   char   KeyRecs[XB_NDX_NODE_SIZE-4];
#else
   char     KeyRecs[XB_MAX_NDX_NODE_SIZE - 4];
#endif // XB_VAR_NODESIZE
};

struct xbNdxNodeLink {			/* ndx node memory */
   xbNdxNodeLink * PrevNode;
   xbNdxNodeLink * NextNode;
   xbLong       CurKeyNo;                 /* 0 - KeysPerNode-1 */
   xbLong       NodeNo;
   struct xbNdxLeafNode Leaf;
};

class XBDLLEXPORT xbNdx : public xbIndex{
//   xbNdx * ndx;
//   xbDbf * dbf;
//   xbExpNode * ExpressionTree;    /* Expression tree for index */
   xbNdxHeadNode HeadNode;
   xbNdxLeafNode LeafNode;
   xbLong xbNodeLinkCtr;
   xbLong ReusedxbNodeLinks;

   xbString IndexName;
#ifndef XB_VAR_NODESIZE
   char  Node[XB_NDX_NODE_SIZE];
#else
   char  Node[XB_MAX_NDX_NODE_SIZE];
#endif // XB_VAR_NODESIZE
//   FILE  *ndxfp;
//   int   NdxStatus;                /* 0 = closed, 1 = open */

   xbNdxNodeLink * NodeChain;     /* pointer to node chain of index nodes */
   xbNdxNodeLink * FreeNodeChain; /* pointer to chain of free index nodes */
   xbNdxNodeLink * CurNode;       /* pointer to current node              */
   xbNdxNodeLink * DeleteChain;   /* pointer to chain to delete           */
   xbNdxNodeLink * CloneChain;    /* pointer to node chain copy (add dup) */
   xbLong  CurDbfRec;             /* current Dbf record number */
   char  *KeyBuf;                 /* work area key buffer */
   char  *KeyBuf2;                /* work area key buffer */

/* private functions */
   xbLong     GetLeftNodeNo( xbShort, xbNdxNodeLink * );
#if 0   
   xbShort    CompareKey( const char *Key1, const char *Key2, xbShort Klen );
#else
   inline xbShort    CompareKey( const char *Key1, const char *Key2, xbShort Klen )
   {
#if 0
     const  char *k1, *k2;
     xbShort  i;
#endif
     xbDouble d1, d2;
     int c;

     if(!( Key1 && Key2 )) return -1;

     if( Klen > HeadNode.KeyLen ) Klen = HeadNode.KeyLen;

     if( HeadNode.KeyType == 0 )
     {
       c = memcmp(Key1, Key2, Klen);
       if(c < 0)
         return 2;
       else if(c > 0)
         return 1;
       return 0;
     }
     else		/* key is numeric */
     {
        d1 = dbf->xbase->GetDouble( Key1 );
        d2 = dbf->xbase->GetDouble( Key2 );
        if( d1 == d2 ) return 0;
        else if( d1 > d2 ) return 1;
        else return 2;
     }
   }
#endif   
#if 0
   xbLong     GetDbfNo( xbShort, xbNdxNodeLink * );
#else
   inline xbLong     GetDbfNo( xbShort RecNo, xbNdxNodeLink *n )
   {
     xbNdxLeafNode *temp;
     char *p;
     if( !n ) return 0L;
     temp = &n->Leaf;
     if( RecNo < 0 || RecNo > ( temp->NoOfKeysThisNode - 1 )) return 0L;
     p = temp->KeyRecs + 4;
     p += RecNo * ( 8 + HeadNode.KeyLen );
     return( dbf->xbase->GetLong( p ));
   }
#endif   
   char *     GetKeyData( xbShort, xbNdxNodeLink * );
   xbUShort   GetKeysPerNode( void );
   xbShort    GetHeadNode( void );    
   xbShort    GetLeafNode( xbLong, xbShort );
   xbNdxNodeLink * GetNodeMemory( void );
   void       ReleaseNodeMemory( xbNdxNodeLink * );
   xbShort    xbNdx::BSearchNode(const char *key, xbShort klen, 
                                 const xbNdxNodeLink *node, 
                                 xbShort *comp);
   xbLong     GetLeafFromInteriorNode( const char *Tkey, xbShort Klen );
   xbShort    CalcKeyLen( void );
   xbShort    PutKeyData( xbShort, xbNdxNodeLink * );
   xbShort    PutLeftNodeNo( xbShort, xbNdxNodeLink *, xbLong );
   xbShort    PutLeafNode( xbLong, xbNdxNodeLink * );
   xbShort    PutHeadNode( xbNdxHeadNode *, FILE *, xbShort );
   xbShort    PutDbfNo( xbShort, xbNdxNodeLink *, xbLong );
   xbShort    PutKeyInNode( xbNdxNodeLink *, xbShort, xbLong, xbLong, xbShort );
   xbShort    SplitLeafNode( xbNdxNodeLink *, xbNdxNodeLink *, xbShort, xbLong ); 
   xbShort    SplitINode( xbNdxNodeLink *, xbNdxNodeLink *, xbLong );
   xbShort    AddToIxList( void );
   xbShort    RemoveFromIxList( void );
   xbShort    RemoveKeyFromNode( xbShort, xbNdxNodeLink * );
   xbShort    FindKey( const char *Tkey, xbShort Klen, xbShort RetrieveSw );      
   xbShort    UpdateParentKey( xbNdxNodeLink * );
   xbShort    GetFirstKey( xbShort );
   xbShort    GetNextKey( xbShort );
   xbShort    GetLastKey( xbLong, xbShort );
   xbShort    GetPrevKey( xbShort ); 
   void       UpdateDeleteList( xbNdxNodeLink * );
   void       ProcessDeleteList( void );
   xbNdxNodeLink * LeftSiblingHasSpace( xbNdxNodeLink * );
   xbNdxNodeLink * RightSiblingHasSpace( xbNdxNodeLink * );
   xbShort    DeleteSibling( xbNdxNodeLink * );
   xbShort    MoveToLeftNode( xbNdxNodeLink *, xbNdxNodeLink * );
   xbShort    MoveToRightNode( xbNdxNodeLink *, xbNdxNodeLink * );

   xbShort    CloneNodeChain( void );          /* test */
   xbShort    UncloneNodeChain( void );        /* test */
   
public:
   xbNdx      ( xbDbf * );

/* don't uncomment next line - it causes seg faults for some undiagnosed reason*/
//   ~NDX() { if( NdxStatus ) CloseIndex(); }  

   xbShort  OpenIndex ( const char * FileName );
   xbShort  CloseIndex( void );
   void   DumpHdrNode  ( void );
   void   DumpNodeRec  ( xbLong ); 
   xbShort  CreateIndex( const char *IxName, const char *Exp, 
          xbShort Unique, xbShort OverLay );
   xbLong   GetTotalNodes( void );
   xbLong   GetCurDbfRec( void ) { return CurDbfRec; }
   void   DumpNodeChain( void );
   xbShort  CreateKey( xbShort, xbShort );
   xbShort  GetCurrentKey(char *key);
   xbShort  AddKey( xbLong );
   xbShort  UniqueIndex( void ) { return HeadNode.Unique; }
   xbShort  DeleteKey( xbLong RecNo );
   xbShort  KeyWasChanged( void );
   xbShort  FindKey( const char *Key );
   xbShort  FindKey( void );
   xbShort  FindKey( xbDouble );
   xbShort  FindKey( const char *Tkey, xbLong DbfRec );   /* for a specific dbf no */
#ifdef XBASE_DEBUG
   xbShort  CheckIndexIntegrity( const xbShort Option );
#endif
   xbShort  GetNextKey( void )  { return GetNextKey( 1 ); }
   xbShort  GetLastKey( void )  { return GetLastKey( 0, 1 ); }
   xbShort  GetFirstKey( void ) { return GetFirstKey( 1 ); }
   xbShort  GetPrevKey( void )  { return GetPrevKey( 1 ); }
   xbShort  ReIndex(void (*statusFunc)(xbLong itemNum, xbLong numItems) = 0);
   xbShort  KeyExists( const char * Key ) { return FindKey( Key, strlen( Key ), 0 ); }
   xbShort  KeyExists( xbDouble );

//#ifdef XB_LOCKING_ON
//   xbShort  LockIndex( const xbShort, const xbShort ) const;
//#else
//   xbShort  LockIndex( const xbShort, const xbShort ) const { return NO_ERROR; }
//#endif

   virtual void SetNodeSize(xbShort size);

   virtual void GetExpression(char *buf, int len);
};
#endif		/* __XB_NDX_H__ */
