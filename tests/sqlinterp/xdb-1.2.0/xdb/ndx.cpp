/*  $Id: ndx.cpp,v 1.15 1999/03/19 10:56:34 willy Exp $

    Xbase project source code

    NDX indexing routines for X-Base

    Copyright (C) 1997 StarTech, Gary A. Kunkel   
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

    V 1.0     10/10/97   - Initial release of software
    V 1.3     11/30/97   - Moved GetLong and GetShort to DBF class
    V 1.5     1/2/98     - Added Dbase IV memo field support
    V 1.6a    4/1/98     - Added expression support
    V 1.6b    4/8/98     - Numeric index keys
    V 1.7.4b  6/2/98     - Big Endian fix in PutLeafNode - JAM
                         - Fix in Reindex  - PS
                         - Fix in Findkey( Tkey, DbfRec )
              9/29/98    - Fix in CreateIndex to calculate KeyLen & size 
                           Paul Koufalis pkoufalis@cogicom.com
                         - sequence duplicate keys in dbf# order
    V 1.8     11/30/98   - Version 1.8 upgrade
 
*/

#include <xdb/xbase.h>

#ifdef XB_INDEX_NDX

#ifdef HAVE_IO_H
#include <io.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xdb/xbexcept.h>

#define USE_BSEARCH

/***********************************************************************/
xbShort xbNdx::CloneNodeChain( void )
{
   xbNdxNodeLink * TempNodeS;
   xbNdxNodeLink * TempNodeT;
   xbNdxNodeLink * TempNodeT2;

   if( CloneChain ) ReleaseNodeMemory( CloneChain );
   CloneChain = NULL;

   if( !NodeChain ) return XB_NO_ERROR;
   TempNodeS = NodeChain;
   TempNodeT2 = NULL;

   while( TempNodeS )
   {
      if(( TempNodeT = GetNodeMemory()) == NULL ) {
	 xb_memory_error;
      }
      memcpy( TempNodeT, TempNodeS, sizeof( struct xbNdxNodeLink ));
      TempNodeT->NextNode = NULL;
      TempNodeT->PrevNode = TempNodeT2;
      if( !CloneChain )
      {
         TempNodeT2 = TempNodeT;
         CloneChain = TempNodeT;
      }
      else
      {
         TempNodeT2->NextNode = TempNodeT;
         TempNodeT2 = TempNodeT2->NextNode;
      }
      TempNodeS = TempNodeS->NextNode;
   }
   return XB_NO_ERROR;
}
/***********************************************************************/
xbShort xbNdx::UncloneNodeChain( void )
{
   if( NodeChain )
      ReleaseNodeMemory( NodeChain );
   NodeChain = CloneChain;
   CloneChain = NULL;
   CurNode = NodeChain;
   while( CurNode->NextNode )
      CurNode = CurNode->NextNode;
   return XB_NO_ERROR;
}
/***********************************************************************/
/* This routine dumps the node chain to stdout                         */
#ifdef XBASE_DEBUG
void xbNdx::DumpNodeChain( void )
{
   xbNdxNodeLink  *n;
   cout << "\n*************************\n";
   cout <<   "xbNodeLinkCtr = " << xbNodeLinkCtr;
   cout << "\nReused      = " << ReusedxbNodeLinks << "\n";

   n = NodeChain;
   while(n)
   {
      cout << "xbNodeLink Chain" << n->NodeNo << "\n";
      n = n->NextNode;
   }
   n = FreeNodeChain;
   while(n)
   {
      cout << "FreexbNodeLink Chain" << n->NodeNo << "\n";
      n = n->NextNode;
   }
   n = DeleteChain;
   while(n)
   {
      cout << "DeleteLink Chain" << n->NodeNo << "\n";
      n = n->NextNode;
   }
}
#endif
/***********************************************************************/
/* This routine returns a chain of one or more index nodes back to the */
/* free node chain                                                     */

void xbNdx::ReleaseNodeMemory( xbNdxNodeLink * n )
{
   xbNdxNodeLink * temp;

   if( !FreeNodeChain )
      FreeNodeChain = n;
   else    /* put this list at the end */
   {
      temp = FreeNodeChain;
      while( temp->NextNode )
         temp = temp->NextNode;
      temp->NextNode = n;
   }
   return;
}
/***********************************************************************/
/* This routine returns a node from the free chain if available,       */
/* otherwise it allocates new memory for the requested node             */

xbNdxNodeLink * xbNdx::GetNodeMemory( void )
{
   xbNdxNodeLink * temp;
   if( FreeNodeChain )
   {
      temp = FreeNodeChain;
      FreeNodeChain = temp->NextNode;
      ReusedxbNodeLinks++;
   }
   else
   {
      temp = (xbNdxNodeLink *) malloc( sizeof( xbNdxNodeLink ));
      xbNodeLinkCtr++;
   }
   memset( temp, 0x00, sizeof( xbNdxNodeLink ));
   return temp;
}
/***********************************************************************/
#ifdef XBASE_DEBUG
void xbNdx::DumpHdrNode( void )
{
   cout << "\nStart node    = " << HeadNode.StartNode;
   cout << "\nTotal nodes   = " << HeadNode.TotalNodes;
   cout << "\nNo of keys    = " << HeadNode.NoOfKeys;
   cout << "\nKey Length    = " << HeadNode.KeyLen;
   cout << "\nKeys Per Node = " << HeadNode.KeysPerNode;
   cout << "\nKey type      = " << HeadNode.KeyType;
   cout << "\nKey size      = " << HeadNode.KeySize;
   cout << "\nUnknown 2     = " << HeadNode.Unknown2;
   cout << "\nUnique        = " << HeadNode.Unique;
   cout << "\nKeyExpression = " << HeadNode.KeyExpression;
#ifdef XB_VAR_NODESIZE
   cout << "\nNodeSize      = " << NodeSize;
#endif // XB_VAR_NODESIZE
   cout << "\n";
}
#endif
/***********************************************************************/
xbNdx::xbNdx(xbDbf *pdbf) : xbIndex(pdbf) {
#ifndef XB_VAR_NODESIZE
   memset( Node, 0x00, XB_NDX_NODE_SIZE );
#else
   memset( Node, 0x00, XB_MAX_NDX_NODE_SIZE );
#endif
   memset( &HeadNode, 0x00, sizeof( xbNdxHeadNode )); 
//   ndxfp           = NULL;
   NodeChain       = NULL;
   CloneChain      = NULL;
   FreeNodeChain   = NULL;
   DeleteChain     = NULL;
   CurNode         = NULL;
//   NdxStatus       = 0;
   xbNodeLinkCtr     = 0L;
   ReusedxbNodeLinks = 0L;
//   ndx             = this;
//   dbf             = pdbf;
//   KeyBuf          = NULL;
//   KeyBuf2         = NULL;
//   IndexName       = NULL;
#ifndef XB_VAR_NODESIZE
   NodeSize = XB_NDX_NODE_SIZE;
#else
   NodeSize = XB_DEFAULT_NDX_NODE_SIZE;
#endif // XB_VAR_NODESIZE
}
/***********************************************************************/
xbShort xbNdx::OpenIndex( const char * FileName )
{
   int NameLen, rc;

   NameLen = strlen( FileName ) + 1;
   if(( rc = dbf->NameSuffixMissing( 2, FileName )) > 0 )
     if(( rc = dbf->NameSuffixMissing( 4, FileName )) > 0 )
       NameLen += 4;

   IndexName = FileName;

   if( rc == 1 )
     IndexName += ".ndx";
   else
     if ( rc == 2 )
       IndexName += ".NDX";

   /* open the file */
   if(( indexfp = fopen( IndexName, "r+b" )) == NULL )
     xb_open_error(IndexName);

#ifdef XB_LOCKING_ON   
   /*
   **  Must turn off buffering when multiple programs may be accessing
   **  index files.
   */
   setvbuf(indexfp, NULL, _IONBF, 0);
#endif   

#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      if((rc = LockIndex(F_SETLKW, F_RDLCK)) != 0)
        return rc;
#endif
       
   IndexStatus = 1; 
   if(( rc = GetHeadNode()) != 0) 
   { 
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      fclose( indexfp );
      return rc;
   }

   /* parse the expression */
   if(( rc = dbf->xbase->BuildExpressionTree( HeadNode.KeyExpression,
      strlen( HeadNode.KeyExpression ), dbf )) != XB_NO_ERROR )
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      return rc;
   }
   ExpressionTree = dbf->xbase->GetTree();
   dbf->xbase->SetTreeToNull(); 

//dbf->xbase->DumpExpressionTree(ExpressionTree);
   
   KeyBuf  = (char *) malloc( HeadNode.KeyLen + 1 ); 
   KeyBuf2 = (char *) malloc( HeadNode.KeyLen + 1);
   memset( KeyBuf,  0x00, HeadNode.KeyLen + 1 );
   memset( KeyBuf2, 0x00, HeadNode.KeyLen + 1 );

   rc = dbf->AddIndexToIxList( index, IndexName );
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
   return rc;
}
/***********************************************************************/
xbShort xbNdx::CloseIndex( void )
{
   if( KeyBuf )    { free ( KeyBuf );    KeyBuf = NULL;    }
   if( KeyBuf2 )   { free ( KeyBuf2 );   KeyBuf2 = NULL;   }
   dbf->RemoveIndexFromIxList( index );

   fclose( indexfp );
   IndexStatus = 0;
   return 0;
}
/***********************************************************************/
xbShort xbNdx::GetHeadNode( void )
{
   char *p, *q;
   xbShort i;

   if( !IndexStatus )
     xb_error(XB_NOT_OPEN);

   if( fseek( indexfp, 0, SEEK_SET ))
     xb_io_error(XB_SEEK_ERROR, IndexName);

   if(( fread( Node, XB_NDX_NODE_SIZE, 1, indexfp )) != 1 )
     xb_io_error(XB_READ_ERROR, IndexName);

   /* load the head node structure */
   p = Node;
   HeadNode.StartNode   = dbf->xbase->GetLong ( p ); p+=4;
   HeadNode.TotalNodes  = dbf->xbase->GetLong ( p ); p+=4;
   HeadNode.NoOfKeys    = dbf->xbase->GetLong ( p ); p+=4;
   HeadNode.KeyLen      = dbf->xbase->GetShort( p ); p+=2;
   HeadNode.KeysPerNode = dbf->xbase->GetShort( p ); p+=2;
   HeadNode.KeyType     = dbf->xbase->GetShort( p ); p+=2;
   HeadNode.KeySize     = dbf->xbase->GetLong ( p ); p+=4;
   HeadNode.Unknown2    = *p++;
   HeadNode.Unique      = *p++;
   
#ifdef XB_VAR_NODESIZE
   //
   //  Automagically determine the node size.  Note the (2 * sizeof(xbLong))
   //  is taken directly from CreateIndex().  I don't understand it exactly,
   //  but this is the value used to calculate the number of keys per node.
   //  DTB.
   //
   NodeSize = (2 * sizeof(xbLong)) + HeadNode.KeySize * HeadNode.KeysPerNode;
//printf("NodeSize = %d\n", NodeSize);
   if(NodeSize % XB_NDX_NODE_MULTIPLE)
     NodeSize = ((NodeSize + XB_NDX_NODE_MULTIPLE) / XB_NDX_NODE_MULTIPLE) * 
                 XB_NDX_NODE_MULTIPLE;
//printf("NodeSize = %d\n", NodeSize);
#endif

   q = HeadNode.KeyExpression;
   for( i = XB_NDX_NODE_BASESIZE; i < XB_NDX_NODE_SIZE && *p; i++ )
      *q++ = *p++;

   return 0;
}
/***********************************************************************/
/* This routine reads a leaf node from disk                            */
/*                                                                     */
/*  If SetNodeChain 2, then the node is not appended to the node chain */
/*                     but the CurNode pointer points to the node read */
/*  If SetNodeChain 1, then the node is appended to the node chain     */
/*  If SetNodeChain 0, then record is only read to Node memory         */

xbShort xbNdx::GetLeafNode( xbLong NodeNo, xbShort SetNodeChain )
{
   xbNdxNodeLink *n;

   if( !IndexStatus )
     xb_error(XB_NOT_OPEN);

   if( fseek( indexfp, NodeNo * XB_NDX_NODE_SIZE, SEEK_SET ))
     xb_io_error(XB_SEEK_ERROR, IndexName);

   if(( fread( Node, XB_NDX_NODE_SIZE, 1, indexfp )) != 1 )
     xb_io_error(XB_READ_ERROR, IndexName);

   if( !SetNodeChain ) return 0;

   if(( n = GetNodeMemory()) == NULL )
     xb_memory_error;

   n->NodeNo = NodeNo;
   n->CurKeyNo = 0L;
   n->NextNode = NULL;
   n->Leaf.NoOfKeysThisNode = dbf->xbase->GetLong( Node );
   memcpy( n->Leaf.KeyRecs, Node+4, XB_NDX_NODE_SIZE - 4 );

   /* put the node in the chain */
   if( SetNodeChain == 1 )
   {
      if( NodeChain == NULL )      /* first one ? */
      { 
         NodeChain = n;
         CurNode = n;
         CurNode->PrevNode = NULL;
      }
      else
      {
         n->PrevNode = CurNode;
         CurNode->NextNode = n;
         CurNode = n;
      }
   }
   else
      CurNode = n;
   return 0;
}
/***********************************************************************/
#ifdef XBASE_DEBUG
void xbNdx::DumpNodeRec( xbLong n )
{
   char *p;
   xbLong NoOfKeys, LeftBranch, RecNo;
   xbShort i,j;

   GetLeafNode( n, 0 );
   NoOfKeys = dbf->xbase->GetLong( Node );
   p = Node + 4;			/* go past no of keys */
   cout << "\n--------------------------------------------------------";
   cout << "\nNode # " << n << " Number of keys = " << NoOfKeys << "\n";

   cout << "\n Key     Left     Rec      Key";
   cout << "\nNumber  Branch   Number    Data";

   for( i = 0; i < GetKeysPerNode() /*NoOfKeys*/; i++ )
   {
      LeftBranch = dbf->xbase->GetLong( p );
      p+=4;
      RecNo = dbf->xbase->GetLong( p );
      p+=4;
      cout << "\n" << i << "         " << LeftBranch << "          " << RecNo << "         ";
      if( !HeadNode.KeyType )
         for( j = 0; j < HeadNode.KeyLen; j++ ) cout << *p++;
      else
      {
         cout << dbf->xbase->GetDouble( p );
         p += 8;
      }
   }
}
#endif
/***********************************************************************/
#if 0
xbLong xbNdx::GetDbfNo( xbShort RecNo, xbNdxNodeLink * n )
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
/***********************************************************************/
xbLong xbNdx::GetLeftNodeNo( xbShort RecNo, xbNdxNodeLink * n )
{
   xbNdxLeafNode *temp;
   char *p;
   if( !n ) return 0L;
   temp = &n->Leaf;
   if( RecNo < 0 || RecNo > temp->NoOfKeysThisNode ) return 0L;
   p = temp->KeyRecs;
   p += RecNo * ( 8 + HeadNode.KeyLen );
   return( dbf->xbase->GetLong( p ));
}
/***********************************************************************/
char * xbNdx::GetKeyData( xbShort RecNo, xbNdxNodeLink * n )
{
   xbNdxLeafNode *temp;
   char *p;
   if( !n ) return 0L;
   temp = &n->Leaf;
   if( RecNo < 0 || RecNo > ( temp->NoOfKeysThisNode - 1 )) return 0L;
   p = temp->KeyRecs + 8;
   p += RecNo * ( 8 + HeadNode.KeyLen );
   return( p );
}
/***********************************************************************/
xbLong xbNdx::GetTotalNodes( void ) 
{
   if( &HeadNode )
      return HeadNode.TotalNodes;
   else
      return 0L;
}
/***********************************************************************/
xbUShort xbNdx::GetKeysPerNode( void ) 
{
   if( &HeadNode )
      return HeadNode.KeysPerNode;
   else
      return 0L;
}
/***********************************************************************/
xbShort xbNdx::GetFirstKey( xbShort RetrieveSw )
{
/* This routine returns 0 on success and sets CurDbfRec to the record  */
/* corresponding to the first index pointer                            */

   xbLong TempNodeNo;
   xbShort rc;
   
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      if((rc = LockIndex(F_SETLKW, F_RDLCK)) != 0)
        return rc;
#endif
       
   /* initialize the node chain */
   if( NodeChain )
   {
      ReleaseNodeMemory( NodeChain );
      NodeChain = NULL;
   }

   if(( rc = GetHeadNode()) != 0 )
   {
      CurDbfRec = 0L;
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      return rc;
   }

   /* get a node and add it to the link */

   if(( rc = GetLeafNode( HeadNode.StartNode, 1 )) != 0 )
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      return rc;
   }      

/* traverse down the left side of the tree */
   while( GetLeftNodeNo( 0, CurNode ))
   {
      TempNodeNo = GetLeftNodeNo( 0, CurNode );     
      if(( rc = GetLeafNode( TempNodeNo, 1 )) != 0 )
      {
         CurDbfRec = 0L;
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         return rc;     
      }
      CurNode->CurKeyNo = 0;
   }
   CurDbfRec = GetDbfNo( 0, CurNode );
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
   if( RetrieveSw )
      return dbf->GetRecord( CurDbfRec );
   else
      return XB_NO_ERROR;
}
/***********************************************************************/
xbShort xbNdx::GetNextKey( xbShort RetrieveSw )
{
/* This routine returns 0 on success and sets CurDbfRec to the record  */
/* corresponding to the next index pointer                             */

   xbNdxNodeLink * TempxbNodeLink;

   xbLong TempNodeNo;
   xbShort rc = 0;
    
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      if((rc = LockIndex(F_SETLKW, F_RDLCK)) != 0)
        return rc;
#endif

   if( !IndexStatus )
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      CurDbfRec = 0L;
      xb_error(XB_NOT_OPEN);
   }

   if( !CurNode )
   {
      rc = GetFirstKey( RetrieveSw );
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      return rc;
   }

   /* more keys on this node ? */
   if(( CurNode->Leaf.NoOfKeysThisNode-1) > CurNode->CurKeyNo )
   {
      CurNode->CurKeyNo++;
      CurDbfRec = GetDbfNo( CurNode->CurKeyNo, CurNode );
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      if( RetrieveSw )
         return dbf->GetRecord( CurDbfRec );
      else
         return XB_NO_ERROR;
   }

   /* if head node we are at eof */
   if( CurNode->NodeNo == HeadNode.StartNode ) {
#ifdef XB_LOCKING_ON
          if( dbf->GetAutoLock() )
             LockIndex(F_SETLKW, F_UNLCK);
#endif
	   xb_eof_error;
   }

   /* this logic assumes that interior nodes have n+1 left node no's where */
   /* n is the number of keys in the node                                  */

   /* pop up one node to the interior node level & free the leaf node      */

   TempxbNodeLink = CurNode;
   CurNode = CurNode->PrevNode;
   CurNode->NextNode = NULL;
   ReleaseNodeMemory( TempxbNodeLink );

   /* while no more right keys && not head node, pop up one node */
   while(( CurNode->CurKeyNo >= CurNode->Leaf.NoOfKeysThisNode ) &&
          ( CurNode->NodeNo != HeadNode.StartNode ))
   {
      TempxbNodeLink = CurNode;
      CurNode = CurNode->PrevNode;
      CurNode->NextNode = NULL;
      ReleaseNodeMemory( TempxbNodeLink );
   }

   /* if head node && right most key, return end-of-file */
   if(( HeadNode.StartNode == CurNode->NodeNo ) &&
      ( CurNode->CurKeyNo >= CurNode->Leaf.NoOfKeysThisNode )) {
#ifdef XB_LOCKING_ON
           if( dbf->GetAutoLock() )
              LockIndex(F_SETLKW, F_UNLCK);
#endif
	   xb_eof_error;
   }

   /* move one to the right */
   CurNode->CurKeyNo++;
   TempNodeNo = GetLeftNodeNo( CurNode->CurKeyNo, CurNode );

   if(( rc = GetLeafNode( TempNodeNo, 1 )) != 0 )
   {
#ifdef XB_LOCKING_ON
           if( dbf->GetAutoLock() )
              LockIndex(F_SETLKW, F_UNLCK);
#endif
	   return rc;
   }

/* traverse down the left side of the tree */
   while( GetLeftNodeNo( 0, CurNode ))
   {
      TempNodeNo = GetLeftNodeNo( 0, CurNode );     
      if(( rc = GetLeafNode( TempNodeNo, 1 )) != 0 )
      {
         CurDbfRec = 0L;
				 return rc;
      }
      CurNode->CurKeyNo = 0;
   }
   CurDbfRec = GetDbfNo( 0, CurNode );
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
   if( RetrieveSw )
      return dbf->GetRecord( CurDbfRec );
   else
      return XB_NO_ERROR;
}
/***********************************************************************/
xbShort xbNdx::GetLastKey( xbLong NodeNo, xbShort RetrieveSw )
{
/* This routine returns 0 on success and sets CurDbfRec to the record  */
/* corresponding to the last index pointer                             */

/* If NodeNo = 0, start at head node, otherwise start at NodeNo        */

   xbLong TempNodeNo;
   xbShort rc;
  
   if( NodeNo < 0 || NodeNo > HeadNode.TotalNodes )
		 xb_error(XB_INVALID_NODE_NO);

   /* initialize the node chain */
   if( NodeChain )
   {
      ReleaseNodeMemory( NodeChain );
      NodeChain = NULL;
   }
   if( NodeNo == 0L )
      if(( rc = GetHeadNode()) != 0 )
      { 
         CurDbfRec = 0L;
				 return rc;
      }

#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      if((rc = LockIndex(F_SETLKW, F_RDLCK)) != 0)
        return rc;
#endif

   /* get a node and add it to the link */

   if( NodeNo == 0L )
   {
      if(( rc = GetLeafNode( HeadNode.StartNode, 1 )) != 0 )
      {
         CurDbfRec = 0L;
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
           LockIndex(F_SETLKW, F_UNLCK);
#endif
				 return rc;
      }
   }
   else
   {
      if(( rc = GetLeafNode( NodeNo, 1 )) != 0 )
      {
         CurDbfRec = 0L;
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
				 return rc;
      }
   }
   CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode;

/* traverse down the right side of the tree */
   while( GetLeftNodeNo( CurNode->Leaf.NoOfKeysThisNode, CurNode ))
   {
      TempNodeNo = GetLeftNodeNo( CurNode->Leaf.NoOfKeysThisNode, CurNode );     
      if(( rc = GetLeafNode( TempNodeNo, 1 )) != 0 )
      {
         CurDbfRec = 0L;
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
				 return rc;
      }
      CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode;
   }
   CurNode->CurKeyNo--;           /* leaf node has one fewer ix recs */
   CurDbfRec = GetDbfNo( CurNode->Leaf.NoOfKeysThisNode-1, CurNode );
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
   if( RetrieveSw )
      return dbf->GetRecord( CurDbfRec );
   else
      return XB_NO_ERROR;
}
/***********************************************************************/
xbShort xbNdx::GetPrevKey( xbShort RetrieveSw )
{
/* This routine returns 0 on success and sets CurDbfRec to the record  */
/* corresponding to the previous index pointer                         */

   xbNdxNodeLink * TempxbNodeLink;

   xbLong TempNodeNo;
   xbShort rc = 0;
    
   if( !IndexStatus )
   {
     CurDbfRec = 0L;
     xb_error(XB_NOT_OPEN);
   }

   if( !CurNode )
   {
      CurDbfRec = 0L;
      return GetFirstKey( RetrieveSw );
   }

#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      if((rc = LockIndex(F_SETLKW, F_RDLCK)) != 0)
        return rc;
#endif

   /* more keys on this node ? */
   if( CurNode->CurKeyNo > 0 )
   {
      CurNode->CurKeyNo--;
      CurDbfRec = GetDbfNo( CurNode->CurKeyNo, CurNode );
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      if( RetrieveSw )
         return dbf->GetRecord( CurDbfRec );
      else
         return XB_NO_ERROR;
   }

   /* this logic assumes that interior nodes have n+1 left node no's where */
   /* n is the number of keys in the node                                  */

   /* pop up one node to the interior node level & free the leaf node      */

   if( !CurNode->PrevNode ) {      /* michael - make sure prev node exists */
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
	   xb_eof_error;
   }

   TempxbNodeLink = CurNode;
   CurNode = CurNode->PrevNode;
   CurNode->NextNode = NULL;
   ReleaseNodeMemory( TempxbNodeLink );

   /* while no more left keys && not head node, pop up one node */
   while(( CurNode->CurKeyNo == 0 ) && 
          ( CurNode->NodeNo != HeadNode.StartNode ))
   {
      TempxbNodeLink = CurNode;
      CurNode = CurNode->PrevNode;
      CurNode->NextNode = NULL;
      ReleaseNodeMemory( TempxbNodeLink );
   }

   /* if head node && left most key, return end-of-file */
   if(( HeadNode.StartNode == CurNode->NodeNo ) &&
      ( CurNode->CurKeyNo == 0 )) {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
	   xb_eof_error;
   }

   /* move one to the left */
   CurNode->CurKeyNo--;
   TempNodeNo = GetLeftNodeNo( CurNode->CurKeyNo, CurNode );

   if(( rc = GetLeafNode( TempNodeNo, 1 )) != 0 )
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
	   return rc;
   }

   if( GetLeftNodeNo( 0, CurNode ))	/* if interior node */
      CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode;
   else					/* leaf node */
      CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode - 1;

/* traverse down the right side of the tree */
   while( GetLeftNodeNo( 0, CurNode ))		/* while interior node */
   {
      TempNodeNo = GetLeftNodeNo( CurNode->Leaf.NoOfKeysThisNode, CurNode );     
      if(( rc = GetLeafNode( TempNodeNo, 1 )) != 0 )
      {
         CurDbfRec = 0L;
				 return rc;
      }
      if( GetLeftNodeNo( 0, CurNode ))	/* if interior node */
         CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode;
      else					/* leaf node */
         CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode - 1;
   }
   CurDbfRec = GetDbfNo( CurNode->Leaf.NoOfKeysThisNode - 1, CurNode );
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
   if( RetrieveSw )
      return dbf->GetRecord( CurDbfRec );
   else
      return XB_NO_ERROR;
}

#if 0
/***********************************************************************/
xbShort xbNdx::CompareKey( const char * Key1, const char * Key2, xbShort Klen )
{
/*   if key1 = key2  --> return 0      */
/*   if key1 > key2  --> return 1      */
/*   if key1 < key2  --> return 2      */

   const  char *k1, *k2;
   xbShort  i;
   xbDouble d1, d2;
   int c;

   if(!( Key1 && Key2 )) return -1;

   if( Klen > HeadNode.KeyLen ) Klen = HeadNode.KeyLen;

   if( HeadNode.KeyType == 0 )
   {
#if 0
      k1 = Key1;
      k2 = Key2;
      for( i = 0; i < Klen; i++ )
      {
         if( *k1 > *k2 ) return 1;
         if( *k1 < *k2 ) return 2;
         k1++;
         k2++;
      }
      return 0;
#else
      c = memcmp(Key1, Key2, Klen);
      if(c < 0)
        return 2;
      else if(c > 0)
        return 1;
      return 0;
#endif
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

/*************************************************************************
**
**  This is a pretty basic binary search with two exceptions:  1) it will
**  find the first of duplicate key values and 2) will return the index
**  and the value of the last comparision even if it doesn't find a 
**  match.
*/
xbShort
xbNdx::BSearchNode(const char *key, xbShort klen, const xbNdxNodeLink *node, 
                   xbShort *comp)
{
  xbShort
    c = 1,
    p = -1,
    start = 0,
    end = node->Leaf.NoOfKeysThisNode - 1;
    
  if(start > end)
  {
    *comp = 2;
    return 0;
  }
  
  do
  {
    p = (start + end) / 2;
    c = CompareKey(key, GetKeyData(p, (xbNdxNodeLink *)node), klen);
    switch(c)
    {
      case 1 : /* greater than */
        start = p + 1;
      break;

      case 2 : /* less than */
        end = p - 1;
      break;
    }
  } while(start <= end && c);

  if(c == 1)
    while(p < node->Leaf.NoOfKeysThisNode &&
          (c = CompareKey(key, GetKeyData(p, (xbNdxNodeLink *)node), klen)) == 1)
      p++;
    
  *comp = c;
  
  if(!c)
    while(p > 0 && !CompareKey(key, GetKeyData(p - 1, (xbNdxNodeLink *)node), klen))
      p--;
    
  return p;
}

/***********************************************************************/
xbLong xbNdx::GetLeafFromInteriorNode( const char * Tkey, xbShort Klen )
{
   /* This function scans an interior node for a key and returns the   */
   /* correct interior leaf node no                                    */

   xbShort p, c;

   /* if Tkey > any keys in node, return right most key */
   p = CurNode->Leaf.NoOfKeysThisNode - 1;
   if( CompareKey( Tkey, GetKeyData( p, CurNode ), Klen ) == 1 )
   {
      CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode;
      return GetLeftNodeNo( CurNode->Leaf.NoOfKeysThisNode, CurNode );
   }

#ifndef USE_BSEARCH
   /* otherwise, start at the beginning and scan up */
   p = 0;
   while( p < CurNode->Leaf.NoOfKeysThisNode &&
          ( CompareKey( Tkey, GetKeyData( p, CurNode ), Klen ) == 1 ))
      p++;
#else
   p = BSearchNode(Tkey, Klen, CurNode, &c);
//   if(c == 1)
//     p++;
#endif      

   CurNode->CurKeyNo = p;
   return GetLeftNodeNo( p, CurNode );
}
/***********************************************************************/
xbShort xbNdx::KeyExists( xbDouble d )
{
   char buf[9];
   memset( buf, 0x00, 9 );
   dbf->xbase->PutDouble( buf, d );
   return FindKey( buf, 8, 0 );
}
/***********************************************************************/
xbShort xbNdx::FindKey( xbDouble d )
{
   char buf[9];
   memset( buf, 0x00, 9 );
   dbf->xbase->PutDouble( buf, d );
   return FindKey( buf, 8, 1 );
}
/***********************************************************************/
xbShort xbNdx::FindKey( const char * Key )
{
   return FindKey( Key, strlen( Key ), 1 );
}
/***********************************************************************/
xbShort xbNdx::FindKey( const char * Tkey, xbLong DbfRec )
{
   /* find a key with a specifc DBF record number */
   xbShort rc;

   xbLong CurDbfRecNo;
   xbLong CurNdxDbfNo;

#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      if((rc = LockIndex(F_SETLKW, F_RDLCK)) != 0)
        return rc;
#endif

   /* if we are already on the correct key, return XB_FOUND */
   if( CurNode )
   {
    CurDbfRecNo = dbf->GetCurRecNo();
    CurNdxDbfNo = GetDbfNo( CurNode->CurKeyNo, CurNode );
    if( CurDbfRecNo == CurNdxDbfNo && 
      (strncmp(Tkey, GetKeyData( CurNode->CurKeyNo, CurNode ),
         HeadNode.KeyLen ) == 0 ))
    {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
          return XB_FOUND;
    }
   }

   rc =  FindKey( Tkey, HeadNode.KeyLen, 0 );

   while( rc == 0 || rc == XB_FOUND )
   {
      if( strncmp( Tkey, GetKeyData( CurNode->CurKeyNo, CurNode ),
          HeadNode.KeyLen ) == 0 )
      {
         if( DbfRec == GetDbfNo( CurNode->CurKeyNo, CurNode ))
         {
#ifdef XB_LOCKING_ON
            if( dbf->GetAutoLock() )
              LockIndex(F_SETLKW, F_UNLCK);
#endif
            return XB_FOUND;
         }
         else
            rc = GetNextKey( 0 );
      }
      else
      {
#ifdef XB_LOCKING_ON
          if( dbf->GetAutoLock() )
             LockIndex(F_SETLKW, F_UNLCK);
#endif
	   return XB_NOT_FOUND;
      }
   }
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
	 return XB_NOT_FOUND;
}
/***********************************************************************/
xbShort xbNdx::FindKey( void )
{
   /* if no paramaters given, use KeyBuf */
   return( FindKey( KeyBuf, HeadNode.KeyLen, 0 ));
}
/***********************************************************************/
xbShort xbNdx::FindKey( const char * Tkey, xbShort Klen, xbShort RetrieveSw )
{
   /* This routine sets the current key to the found key */
 
   /* if RetrieveSw is true, the method positions the dbf record */
   xbShort rc,i;
   xbLong TempNodeNo;

   if( NodeChain )
   {
      ReleaseNodeMemory( NodeChain );
      NodeChain = NULL;
   }

#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      if((rc = LockIndex(F_SETLKW, F_RDLCK)) != 0)
        return rc;
#endif

   if(( rc = GetHeadNode()) != 0 )
   {
      CurDbfRec = 0L;
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      return rc;
   }

   /* load first node */
   if(( rc = GetLeafNode( HeadNode.StartNode, 1 )) != 0 )
   {
      CurDbfRec = 0L;
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      return rc;
   }

   /* traverse down the tree until it hits a leaf */
   while( GetLeftNodeNo( 0, CurNode ))	/* while interior node */
   {
      TempNodeNo = GetLeafFromInteriorNode( Tkey, Klen );
      if(( rc = GetLeafNode( TempNodeNo, 1 )) != 0 )
      {
         CurDbfRec = 0L;
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         return rc;
      }
   }

   /* leaf level */
#ifndef USE_BSEARCH
   for( i = 0; i < CurNode->Leaf.NoOfKeysThisNode; i++ ) {
      rc = CompareKey(  Tkey, GetKeyData( i, CurNode ), Klen );
      if( rc == 0 ) 
      {
         CurNode->CurKeyNo = i;
         CurDbfRec = GetDbfNo( i, CurNode );
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
	 if (RetrieveSw)
	    dbf->GetRecord(CurDbfRec);
                                         
         return XB_FOUND;
      }
      else if( rc == 2 )
      {
         CurNode->CurKeyNo = i;
         CurDbfRec = GetDbfNo( i, CurNode );
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         if (RetrieveSw)
            dbf->GetRecord(CurDbfRec);
                                         
	 return XB_NOT_FOUND;
      }
   }
#else
   i = BSearchNode(Tkey, Klen, CurNode, &rc);
   switch(rc)
   {
     case 0 : /* found! */
       CurNode->CurKeyNo = i;
       CurDbfRec = GetDbfNo( i, CurNode );
#ifdef XB_LOCKING_ON
       if( dbf->GetAutoLock() )
          LockIndex(F_SETLKW, F_UNLCK);
#endif
       if (RetrieveSw)
         dbf->GetRecord(CurDbfRec);
     return XB_FOUND;
     
     case 1 : /* less than */
       if(i < CurNode->Leaf.NoOfKeysThisNode)
         break;
//       i++;
     
     case 2 : /* greater than */
       CurNode->CurKeyNo = i;
       CurDbfRec = GetDbfNo( i, CurNode );
#ifdef XB_LOCKING_ON
       if( dbf->GetAutoLock() )
          LockIndex(F_SETLKW, F_UNLCK);
#endif
       if (RetrieveSw)
          dbf->GetRecord(CurDbfRec);
     return XB_NOT_FOUND;
   }
#endif

   CurNode->CurKeyNo = i;
   CurDbfRec = GetDbfNo( i, CurNode );
   if ((RetrieveSw) && (CurDbfRec > 0))
      dbf->GetRecord( CurDbfRec );
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
   return XB_NOT_FOUND;
}
/***********************************************************************/
xbShort xbNdx::CalcKeyLen( void )
{
   xbShort rc;
   xbExpNode * TempNode;
   char FieldName[11];
   char Type;

   TempNode = dbf->xbase->GetFirstTreeNode( ExpressionTree );

   if( !TempNode )
		 return 0;

   if( TempNode->Type == 'd' ) return -8;
   if( TempNode->Type == 'D' )
   {
      memset( FieldName, 0x00, 11 );
      memcpy( FieldName, TempNode->NodeText, TempNode->Len );
      Type = dbf->GetFieldType( dbf->GetFieldNo( FieldName ));
      if( Type == 'N' || Type == 'F' )
         return -8;
   }

   if(( rc = dbf->xbase->ProcessExpression( ExpressionTree )) != XB_NO_ERROR )
      return 0;
      
//   dbf->xbase->DumpExpressionTree(ExpressionTree);

   TempNode = (xbExpNode *) dbf->xbase->Pop();
   if( !TempNode )
		 return 0;
   rc = TempNode->DataLen;

   if( !TempNode->InTree )
		 dbf->xbase->FreeExpNode( TempNode );

//printf("CalcKeyLen returning %d\n", rc);
   return rc;
}
/***********************************************************************/
xbShort xbNdx::CreateIndex(const char * IxName, const char * Exp, 
         xbShort Unique, xbShort Overlay )
{
   xbShort i, NameLen, KeyLen, rc;

   IndexStatus = XB_CLOSED;
   if( strlen( Exp ) > 488 )
     xb_error(XB_INVALID_KEY_EXPRESSION);

   if( dbf->GetDbfStatus() == 0 )
     xb_error(XB_NOT_OPEN);

   /* Get the index file name and store it in the class */
   NameLen = strlen( IxName ) + 1;

   if(( rc = dbf->NameSuffixMissing( 2, IxName )) > 0 )
      NameLen +=4;

//   if(( IndexName = (char *) malloc( NameLen )) == NULL ) {
//     xb_memory_error;
//   }

   IndexName = IxName;
   
   if( rc == 1 )
		 IndexName += ".ndx";
   else if( rc == 2 )
		 IndexName += ".NDX";

   /* check if the file already exists */
   if (((indexfp = fopen( IndexName, "r" )) != NULL ) && !Overlay ) {
      fclose( indexfp );
      xb_io_error(XB_FILE_EXISTS, IndexName);
   }

   if (indexfp)
     fclose(indexfp);

   if(( indexfp = fopen( IndexName, "w+b" )) == NULL )
		 xb_open_error(IndexName);

#ifdef XB_LOCKING_ON   
   /*
   **  Must turn off buffering when multiple programs may be accessing
   **  index files.
   */
   setvbuf(indexfp, NULL, _IONBF, 0);
#endif   

#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      if((rc = LockIndex(F_SETLKW, F_WRLCK)) != 0)
        return rc;
#endif

   /* parse the expression */
   if(( rc = dbf->xbase->BuildExpressionTree( Exp, strlen( Exp ), dbf )) != XB_NO_ERROR )
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      return rc;
   }

   ExpressionTree = dbf->xbase->GetTree();
   dbf->xbase->SetTreeToNull(); 

   /* build the header record */
   memset( &HeadNode, 0x00, sizeof( xbNdxHeadNode ));
   HeadNode.StartNode  = 1L;
   HeadNode.TotalNodes = 2L;
   HeadNode.NoOfKeys   = 1L;

   KeyLen = CalcKeyLen();

   if( KeyLen == 0 || KeyLen > 100 )       /* 100 byte key length limit */
		 xb_error(XB_INVALID_KEY)
   else if( KeyLen == -8 )
   { 
      HeadNode.KeyType = 1;                /* numeric key */
      HeadNode.KeyLen = 8; 
   }
   else
   { 
      HeadNode.KeyType = 0;                /* character key */
      HeadNode.KeyLen = KeyLen; 
   }

//   HeadNode.KeysPerNode = (xbUShort) ( XB_NDX_NODE_SIZE - (2*sizeof( xbLong ))) /
//      (HeadNode.KeyLen + 8 );
//   HeadNode.KeySize = HeadNode.KeyLen + 8;
//   while(( HeadNode.KeySize % 4 ) != 0 ) HeadNode.KeySize++;  /* multiple of 4*/

/* above code replaced with following by Paul Koufalis pkoufalis@cogicom.com */
   while(( HeadNode.KeyLen % 4 ) != 0 ) HeadNode.KeyLen++;  /* multiple of 4*/
   HeadNode.KeySize = HeadNode.KeyLen + 8;
   HeadNode.KeysPerNode = (xbUShort) ( XB_NDX_NODE_SIZE - (2*sizeof( xbLong ))) /
      (HeadNode.KeyLen + 8 );

   HeadNode.Unique = Unique;
   strncpy( HeadNode.KeyExpression, Exp, 488 );
   KeyBuf  = (char *) malloc( HeadNode.KeyLen + 1 ); 
   KeyBuf2 = (char *) malloc( HeadNode.KeyLen + 1 ); 
   memset( KeyBuf,  0x00, HeadNode.KeyLen + 1 );
   memset( KeyBuf2, 0x00, HeadNode.KeyLen + 1 );

   if(( rc = PutHeadNode( &HeadNode, indexfp, 0 )) != 0 ) 
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      return rc;
   }
   /* write node #1 all 0x00 */
   for( i = 0; i < XB_NDX_NODE_SIZE; i++ )
   {
      if ((fwrite("\x00", 1, 1, indexfp)) != 1) 
      {
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
   	 fclose( indexfp );
	 xb_io_error(XB_WRITE_ERROR, IndexName);
      }
   }
   IndexStatus = XB_OPEN;
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
   return dbf->AddIndexToIxList( index, IndexName );  
}
/***********************************************************************/
xbShort xbNdx::PutLeftNodeNo( xbShort RecNo, xbNdxNodeLink *n, xbLong NodeNo )
{
   /* This routine sets n node's leftnode number */
   xbNdxLeafNode *temp;
   char *p;
   if( !n )
		 xb_error(XB_INVALID_NODELINK);

   temp = &n->Leaf;
   if( RecNo < 0 || RecNo > HeadNode.KeysPerNode)
		 xb_error(XB_INVALID_KEY);

   p = temp->KeyRecs;
   p+= RecNo * ( 8 + HeadNode.KeyLen );
   dbf->xbase->PutLong( p, NodeNo );
   return XB_NO_ERROR;
}
/***********************************************************************/
xbShort xbNdx::PutDbfNo( xbShort RecNo, xbNdxNodeLink *n, xbLong DbfNo )
{
   /* This routine sets n node's dbf number */

   xbNdxLeafNode *temp;
   char *p;
   if( !n )
		 xb_error(XB_INVALID_NODELINK);

   temp = &n->Leaf;
   if( RecNo < 0 || RecNo > (HeadNode.KeysPerNode-1))
		 xb_error(XB_INVALID_KEY);

   p = temp->KeyRecs + 4;
   p+= RecNo * ( 8 + HeadNode.KeyLen );
   dbf->xbase->PutLong( p, DbfNo );
   return XB_NO_ERROR;
}
/************************************************************************/
xbShort xbNdx::PutLeafNode( xbLong l, xbNdxNodeLink *n )
{
   if ((fseek(indexfp, l * XB_NDX_NODE_SIZE , SEEK_SET)) != 0) {
     fclose( indexfp );
     xb_io_error(XB_SEEK_ERROR, IndexName);
   }
   dbf->xbase->PutLong( Node, n->Leaf.NoOfKeysThisNode );

   if(( fwrite( Node, 4, 1, indexfp )) != 1 )
   {
     fclose( indexfp );
     xb_io_error(XB_WRITE_ERROR, IndexName);
   }
   if(( fwrite( &n->Leaf.KeyRecs, XB_NDX_NODE_SIZE-4, 1, indexfp )) != 1 )
   {
     fclose( indexfp );
     xb_io_error(XB_WRITE_ERROR, IndexName);
   }
   return 0;   
}
/************************************************************************/
xbShort xbNdx::PutHeadNode( xbNdxHeadNode * Head, FILE * f, xbShort UpdateOnly )
{
   char buf[4];
 
   if(( fseek( f, 0L, SEEK_SET )) != 0 )
   {
     fclose( f );
     xb_io_error(XB_SEEK_ERROR, IndexName);
   }

   memset( buf, 0x00, 4 );
   dbf->xbase->PutLong( buf, Head->StartNode );
   if(( fwrite( &buf, 4, 1, f )) != 1 )
   {
     fclose( f );
     xb_io_error(XB_WRITE_ERROR, IndexName);
   }
   memset( buf, 0x00, 4 );
   dbf->xbase->PutLong( buf, Head->TotalNodes );
   if(( fwrite( &buf, 4, 1, f )) != 1 )
   {
		 fclose( f );
		 xb_io_error(XB_WRITE_ERROR, IndexName);
   }
   memset( buf, 0x00, 4 );
   dbf->xbase->PutLong( buf, Head->NoOfKeys );
   if(( fwrite( &buf, 4, 1, f )) != 1 )
   {
		 fclose( f );
		 xb_io_error(XB_WRITE_ERROR, IndexName);
   }
   if( UpdateOnly )
      return XB_NO_ERROR;
   memset( buf, 0x00, 2 );
   dbf->xbase->PutLong( buf, Head->KeyLen );
   if(( fwrite( &buf, 2, 1, f )) != 1 )
   {
		 fclose( f );
		 xb_io_error(XB_WRITE_ERROR, IndexName);
   }
   memset( buf, 0x00, 2 );
   dbf->xbase->PutLong( buf, Head->KeysPerNode );
   if(( fwrite( &buf, 2, 1, f )) != 1 )
   {
		 fclose( f );
		 xb_io_error(XB_WRITE_ERROR, IndexName);
   }
   memset( buf, 0x00, 2 );
   dbf->xbase->PutLong( buf, Head->KeyType );
   if(( fwrite( &buf, 2, 1, f )) != 1 )
   {
		 fclose( f );
		 xb_io_error(XB_WRITE_ERROR, IndexName);
   }
   memset( buf, 0x00, 4 );
   dbf->xbase->PutLong( buf, Head->KeySize );
   if(( fwrite( &buf, 4, 1, f )) != 1 )
   {
		 fclose( f );
		 xb_io_error(XB_WRITE_ERROR, IndexName);
   }
//   if(( fwrite( &Head->Unknown2, 490, 1, f )) != 1 )
   if(( fwrite( &Head->Unknown2, XB_NDX_NODE_SIZE - 22, 1, f )) != 1 )
   {
		 fclose( f );
		 xb_io_error(XB_WRITE_ERROR, IndexName);
   }
   return 0;   
}
/************************************************************************/
xbShort xbNdx::PutKeyData( xbShort RecNo, xbNdxNodeLink *n )
{
   /* This routine copies the KeyBuf data into xbNdxNodeLink n */
   xbNdxLeafNode *temp;
   char *p;
   xbShort i;
   if( !n )
		 xb_error(XB_INVALID_NODELINK);

   temp = &n->Leaf;
   if( RecNo < 0 || RecNo > (HeadNode.KeysPerNode-1))
		 xb_error(XB_INVALID_KEY);

   p = temp->KeyRecs + 8;
   p+= RecNo * ( 8 + HeadNode.KeyLen );
   for( i = 0; i < HeadNode.KeyLen; i++ )
   {
      *p = KeyBuf[i];
      p++;
   }
   return XB_NO_ERROR;
}
/************************************************************************/
xbShort xbNdx::PutKeyInNode( xbNdxNodeLink * n, xbShort pos, xbLong d, 
    xbLong l, xbShort w )
{
   xbShort i;

   /* check the node */
   if (!n)
		 xb_error(XB_INVALID_NODELINK);

   if(pos < 0 || pos > HeadNode.KeysPerNode)
		 xb_error(XB_INVALID_RECORD);

   if(n->Leaf.NoOfKeysThisNode >= HeadNode.KeysPerNode)
		 xb_error(XB_NODE_FULL);

   /* if key movement, save the original key */
   if( pos < n->Leaf.NoOfKeysThisNode )
      memcpy( KeyBuf2, KeyBuf, HeadNode.KeyLen + 1);

   /* if interior node, handle the right most left node no */
   if( GetLeftNodeNo( 0, n ))
      PutLeftNodeNo( n->Leaf.NoOfKeysThisNode+1, n,
         GetLeftNodeNo( n->Leaf.NoOfKeysThisNode, n ));

   for( i = n->Leaf.NoOfKeysThisNode; i > pos; i-- )
   {
      memcpy( KeyBuf, GetKeyData(i-1,n), HeadNode.KeyLen );  
      PutKeyData( i, n );
      PutDbfNo( i, n, GetDbfNo(i-1,n));
      PutLeftNodeNo(i, n, GetLeftNodeNo(i-1,n));
   }
   /* put new key in node */

   if( pos < n->Leaf.NoOfKeysThisNode )
      memcpy( KeyBuf, KeyBuf2, HeadNode.KeyLen + 1);

   PutKeyData( pos, n );
   PutDbfNo( pos, n, d );
   PutLeftNodeNo( pos, n, l );
   n->Leaf.NoOfKeysThisNode++;
   if( w )
      return PutLeafNode( n->NodeNo, n );
   else
      return 0;
}
/************************************************************************/
xbShort xbNdx::SplitLeafNode( xbNdxNodeLink *n1, xbNdxNodeLink *n2,
   xbShort pos, xbLong d )
{
   xbShort i,j,rc;

   if( !n1 || !n2 )
		 xb_error(XB_INVALID_NODELINK);

   if( pos < 0 || pos > HeadNode.KeysPerNode )
		 xb_error(XB_INVALID_NODELINK);

   if( pos < HeadNode.KeysPerNode ) /* if it belongs in node */
   {
      /* save the original key */
      memcpy( KeyBuf2, KeyBuf, HeadNode.KeyLen + 1);
      PutKeyData( HeadNode.KeysPerNode, n2 ); 
      for( j = 0,i = pos; i < n1->Leaf.NoOfKeysThisNode; j++,i++ )
      {
         memcpy( KeyBuf, GetKeyData( i, n1 ), HeadNode.KeyLen );
         PutKeyData   ( j, n2 );
         PutDbfNo     ( j, n2, GetDbfNo  ( i, n1 ));
         n2->Leaf.NoOfKeysThisNode++; 
      }

      /* restore original key */
      memcpy( KeyBuf, KeyBuf2, HeadNode.KeyLen + 1);

      /* update original leaf */
      PutKeyData( pos, n1 );
      PutDbfNo  ( pos, n1, d );
      n1->Leaf.NoOfKeysThisNode = pos+1;
   }         
   else    /* put the key in a new node because it doesn't fit in the CurNode*/
   {
      PutKeyData   ( 0, n2 );
      PutDbfNo     ( 0, n2, d );
      n2->Leaf.NoOfKeysThisNode++; 
   }
   if(( rc = PutLeafNode( n1->NodeNo, n1 )) != 0 )
       return rc;
   if(( rc = PutLeafNode( n2->NodeNo, n2 )) != 0 )
       return rc;
   return 0;
}
/************************************************************************/
xbShort xbNdx::SplitINode( xbNdxNodeLink *n1, xbNdxNodeLink *n2, xbLong t )
                   /* parent, tempnode, tempnodeno */
{
   xbShort i,j,rc;
   xbNdxNodeLink * SaveNodeChain;
   xbNdxNodeLink * SaveCurNode;

   /* if not at the end of the node shift everthing to the right */
   if( n1->CurKeyNo+1 < HeadNode.KeysPerNode )   /* this clause appears to work */
   {
      if( CurNode->NodeNo == HeadNode.StartNode ) cout << "\nHead node ";
   
      for( j = 0,i = n1->CurKeyNo+1; i < n1->Leaf.NoOfKeysThisNode; i++,j++ ) 
      {
         memcpy( KeyBuf, GetKeyData( i, n1 ), HeadNode.KeyLen ); 
         PutKeyData( j, n2 );
         PutLeftNodeNo( j, n2, GetLeftNodeNo( i, n1 ));
      }
      PutLeftNodeNo( j, n2, GetLeftNodeNo( i, n1 ));
   
      n2->Leaf.NoOfKeysThisNode = n1->Leaf.NoOfKeysThisNode -
         n1->CurKeyNo - 1;
      n1->Leaf.NoOfKeysThisNode = n1->Leaf.NoOfKeysThisNode - 
         n2->Leaf.NoOfKeysThisNode;

      /* attach the new leaf to the original parent */
      SaveNodeChain = NodeChain;
      NodeChain = NULL;
      SaveCurNode = CurNode;
      GetLastKey( CurNode->NodeNo, 0 );
      memcpy( KeyBuf, GetKeyData( CurNode->CurKeyNo, CurNode ),HeadNode.KeyLen);
      ReleaseNodeMemory( NodeChain );
      NodeChain = SaveNodeChain;
      CurNode = SaveCurNode;
      PutKeyData( n1->CurKeyNo, n1 );
      PutLeftNodeNo( n1->CurKeyNo + 1, n1, t );
   }
   else if( n1->CurKeyNo + 1 == HeadNode.KeysPerNode )
   {
      SaveNodeChain = NodeChain;
      NodeChain = NULL;
      SaveCurNode = CurNode;
      GetLastKey( t, 0 );
      memcpy( KeyBuf,GetKeyData(CurNode->CurKeyNo,CurNode), HeadNode.KeyLen ); 
      PutKeyData( 0, n2 );
      PutLeftNodeNo( 0, n2, t ); 
      PutLeftNodeNo( 1, n2, GetLeftNodeNo( n1->Leaf.NoOfKeysThisNode, n1 ));
      ReleaseNodeMemory( NodeChain );
      NodeChain = SaveNodeChain;
      CurNode = SaveCurNode;
      n2->Leaf.NoOfKeysThisNode = 1;
      n1->Leaf.NoOfKeysThisNode--;
   }
   else /* pos = HeadNode.KeysPerNode */
   {
      SaveNodeChain = NodeChain;
      NodeChain = NULL;
      SaveCurNode = CurNode;
      GetLastKey( CurNode->NodeNo, 0 );
      memcpy( KeyBuf, GetKeyData( CurNode->CurKeyNo, CurNode ), HeadNode.KeyLen );
      ReleaseNodeMemory( NodeChain );
      NodeChain = SaveNodeChain;
      CurNode = SaveCurNode;

      PutKeyData( 0, n2 );
      PutLeftNodeNo( 0, n2, CurNode->NodeNo );
      PutLeftNodeNo( 1, n2, t );
      n2->Leaf.NoOfKeysThisNode = 1;
      n1->Leaf.NoOfKeysThisNode--;
   }
   n2->NodeNo = HeadNode.TotalNodes++; 
   if((rc = PutLeafNode( n1->NodeNo,n1 )) != 0) return rc;
   if((rc = PutLeafNode( n2->NodeNo,n2 )) != 0) return rc;
   return 0;
}
/************************************************************************/
xbShort xbNdx::CreateKey( xbShort RecBufSw, xbShort KeyBufSw )
{ 
   /* RecBufSw   0   Use RecBuf    */
   /*            1   Use RecBuf2   */
   /* KeyBufSw   0   Use KeyBuf    */
   /*            1   Use KeyBuf2   */

   xbShort rc;
   xbExpNode * TempNode;

   if(( rc = dbf->xbase->ProcessExpression( ExpressionTree, RecBufSw )) != XB_NO_ERROR )
	   return rc;
   TempNode = (xbExpNode *) dbf->xbase->Pop();
   if( !TempNode )
		 xb_error(XB_INVALID_KEY);

   if( KeyBufSw )
   {
      if( HeadNode.KeyType == 1 )    /* numeric key   */
         dbf->xbase->PutDouble( KeyBuf2, TempNode->DoubResult );
      else                           /* character key */
      {
         memset( KeyBuf2, 0x00, HeadNode.KeyLen + 1 );
         memcpy( KeyBuf2, TempNode->Result, TempNode->DataLen );
      }
   }
   else
   {
      if( HeadNode.KeyType == 1 )    /* numeric key   */
         dbf->xbase->PutDouble( KeyBuf, TempNode->DoubResult );
      else                           /* character key */
      {
         memset( KeyBuf, 0x00, HeadNode.KeyLen + 1 );
         memcpy( KeyBuf, TempNode->Result, TempNode->DataLen );
      }
   }
   if( !TempNode->InTree ) dbf->xbase->FreeExpNode( TempNode );
   return 0;
}
/************************************************************************/
xbShort  
xbNdx::GetCurrentKey(char *key)
{
  CreateKey(0, 0);
  if(HeadNode.KeyType == 1)
    memcpy(key, KeyBuf, 8);
  else
    memcpy(key, KeyBuf, HeadNode.KeyLen + 1);
    
  return 0;
}
/************************************************************************/
xbShort xbNdx::AddKey( xbLong DbfRec )
{
 /* This routine assumes KeyBuf contains the contents of the index to key */

   xbShort i,rc;
   xbNdxNodeLink * TempNode;   
   xbNdxNodeLink * Tparent;
   xbLong TempNodeNo = 0L;          /* new, unattached leaf node no */
   xbNdxNodeLink * SaveNodeChain;
   xbNdxNodeLink * SaveCurNode;
   char * p;

   /* find node key belongs in */
   rc = FindKey( KeyBuf, HeadNode.KeyLen, 0 );
   if( rc == XB_FOUND && HeadNode.Unique )
   {
		 xb_error(XB_KEY_NOT_UNIQUE);
   }

   /* 1.7.4b  sequence keys in dbf record number order */
  if( CurNode->Leaf.NoOfKeysThisNode > 0 && rc == XB_FOUND )
  {
    rc = 0;
    while( rc == 0 )
    {
      if(( p = GetKeyData( CurNode->CurKeyNo, CurNode )) == NULL )  
        rc = -1;
      else
      {
        rc = CompareKey( KeyBuf, p, HeadNode.KeyLen );
        if( rc == 0 && DbfRec >= GetDbfNo( CurNode->CurKeyNo, CurNode ))
        {
#ifdef HAVE_EXCEPTIONS
					try {
#endif
						if((rc = GetNextKey(0)) == XB_EOF) {
							if((rc == GetLastKey(0, 0)) != XB_NO_ERROR)
                                                        {
								return rc;
                                                        }
							CurNode->CurKeyNo++;
						}
#ifdef HAVE_EXCEPTIONS
					} catch (xbEoFException &) {
							GetLastKey(0, 0);
							CurNode->CurKeyNo++;
					}
#endif
        }
        else
          break;
      }
    }
  }

// 9/29/98
// these lines commented out - compatibity wins over efficiency for this library
// 1.02 next statement moves to key match w/ space in nodes to reduce splits 
//   if( !HeadNode.Unique && rc == XB_FOUND &&
//        HeadNode.KeysPerNode == CurNode->Leaf.NoOfKeysThisNode )
//   {
//      if(( rc = CloneNodeChain()) != XB_NO_ERROR ) return rc;
//      if(( rc = GetNextKey( 0 )) != XB_NO_ERROR )
//         UncloneNodeChain();
//      while( HeadNode.KeysPerNode == CurNode->Leaf.NoOfKeysThisNode &&
//             rc == XB_NO_ERROR && 
//             (CompareKey( KeyBuf, GetKeyData( CurNode->CurKeyNo, CurNode ),
//                HeadNode.KeyLen ) == 0 )
//           )
//         if(( rc = GetNextKey( 0 )) != XB_NO_ERROR )
//            UncloneNodeChain();
//   }

   /* update header node */
   HeadNode.NoOfKeys++;
   /************************************************/
   /* section A - if room in node, add key to node */
   /************************************************/

   if( CurNode->Leaf.NoOfKeysThisNode < HeadNode.KeysPerNode )
   {
      if(( rc = PutKeyInNode( CurNode,CurNode->CurKeyNo,DbfRec,0L,1)) != 0)
      {
         return rc;
      }
      if(( rc = PutHeadNode( &HeadNode, indexfp, 1 )) != 0)
      {
         return rc;
      }
      return XB_NO_ERROR;
   }   

   /***********************************************************************/
   /* section B - split leaf node if full and put key in correct position */
   /***********************************************************************/

   TempNode = GetNodeMemory();
   TempNode->NodeNo = HeadNode.TotalNodes++;

   rc = SplitLeafNode( CurNode, TempNode, CurNode->CurKeyNo, DbfRec );
   if( rc ) 
   {
      return rc;
   }

   TempNodeNo = TempNode->NodeNo;
   ReleaseNodeMemory( TempNode );

   /*****************************************************/
   /* section C go up tree splitting nodes as necessary */
   /*****************************************************/
   Tparent = CurNode->PrevNode;

   while( Tparent && 
          Tparent->Leaf.NoOfKeysThisNode >= HeadNode.KeysPerNode )
   {
      TempNode = GetNodeMemory();
      if( !TempNode ) {
	      xb_memory_error;
      }

      rc = SplitINode( Tparent, TempNode, TempNodeNo );
      if( rc ) return rc;

      TempNodeNo = TempNode->NodeNo;
      ReleaseNodeMemory( TempNode );
      ReleaseNodeMemory( CurNode );
      CurNode = Tparent;
      CurNode->NextNode = NULL;
      Tparent = CurNode->PrevNode;
   }

   /************************************************************/
   /* Section D  if CurNode is split root, create new root     */
   /************************************************************/

   /* at this point
       CurNode = The node that was just split
       TempNodeNo = The new node split off from CurNode */

   if(CurNode->NodeNo == HeadNode.StartNode ) 
   {
      TempNode = GetNodeMemory();
      if( !TempNode ) {
	      xb_memory_error;
      }

      SaveNodeChain = NodeChain;
      NodeChain = NULL;
      SaveCurNode = CurNode;
      GetLastKey( CurNode->NodeNo, 0 );
      memcpy( KeyBuf, GetKeyData( CurNode->CurKeyNo,CurNode ),HeadNode.KeyLen );

      ReleaseNodeMemory( NodeChain );
      NodeChain = SaveNodeChain;
      CurNode = SaveCurNode;

      PutKeyData( 0, TempNode );
      PutLeftNodeNo( 0, TempNode, CurNode->NodeNo );
      PutLeftNodeNo( 1, TempNode, TempNodeNo );
      TempNode->NodeNo = HeadNode.TotalNodes++;
      TempNode->Leaf.NoOfKeysThisNode++;
      HeadNode.StartNode = TempNode->NodeNo;
      rc = PutLeafNode( TempNode->NodeNo, TempNode );
      if( rc ) return rc;
      rc = PutHeadNode( &HeadNode, indexfp, 1 );
      if( rc ) return rc;
      ReleaseNodeMemory( TempNode );
      return XB_NO_ERROR;
   }
   /**********************************/
   /* Section E  make room in parent */
   /**********************************/
   for( i = Tparent->Leaf.NoOfKeysThisNode; i > Tparent->CurKeyNo; i-- )
   {
      memcpy( KeyBuf, GetKeyData( i-1, Tparent ), HeadNode.KeyLen );
      PutKeyData( i, Tparent );
      PutLeftNodeNo( i+1, Tparent, GetLeftNodeNo( i, Tparent ));
   }

   /* put key in parent */

   SaveNodeChain = NodeChain;
   NodeChain = NULL;
   SaveCurNode = CurNode;
   GetLastKey( CurNode->NodeNo, 0 );

   memcpy( KeyBuf,GetKeyData( CurNode->CurKeyNo, CurNode ), HeadNode.KeyLen );

   ReleaseNodeMemory( NodeChain );
   NodeChain = SaveNodeChain;
   CurNode = SaveCurNode;

   PutKeyData( i, Tparent );
   PutLeftNodeNo( i+1, Tparent, TempNodeNo );
   Tparent->Leaf.NoOfKeysThisNode++;
   rc = PutLeafNode( Tparent->NodeNo, Tparent );
   if( rc ) return rc;
   rc = PutHeadNode( &HeadNode, indexfp, 1 );
   if( rc ) return rc;
 
   return XB_NO_ERROR;
}
/***********************************************************************/
xbShort xbNdx::RemoveKeyFromNode( xbShort pos, xbNdxNodeLink *n )
{
   xbShort i;

   /* check the node */
   if( !n )
      xb_error(XB_INVALID_NODELINK);

   if( pos < 0 || pos > HeadNode.KeysPerNode )
      xb_error(XB_INVALID_KEY);

   for( i = pos; i < n->Leaf.NoOfKeysThisNode-1; i++ )
   {
      memcpy( KeyBuf, GetKeyData( i+1, n), HeadNode.KeyLen );
   
      PutKeyData( i, n );
      PutDbfNo( i, n, GetDbfNo( i+1, n ));
      PutLeftNodeNo( i, n, GetLeftNodeNo( i+1, n ));
   }   
   PutLeftNodeNo( i, n, GetLeftNodeNo( i+1, n ));
   n->Leaf.NoOfKeysThisNode--;
   /* if last key was deleted, decrement CurKeyNo */
   if( n->CurKeyNo > n->Leaf.NoOfKeysThisNode )
      n->CurKeyNo--;
   return PutLeafNode( n->NodeNo, n );
}   
/***********************************************************************/
xbShort xbNdx::UpdateParentKey( xbNdxNodeLink * n )
{
/* this routine goes backwards thru the node chain looking for a parent
   node to update */

   xbNdxNodeLink * TempNode;
   
   if( !n )
		 xb_error(XB_INVALID_NODELINK);

   if( !GetDbfNo( 0, n ))
   {
      cout << "Fatal index error - Not a leaf node" << n->NodeNo << "\n";
			/*      exit(0); */ /* Great code!!! We exit, then return error!!! */
      /* Library SHOULD NOT do exit */
		 xb_error(XB_NOT_LEAFNODE);
   }

   TempNode = n->PrevNode;

   while( TempNode )
   {
      if( TempNode->CurKeyNo < TempNode->Leaf.NoOfKeysThisNode )
      {
         memcpy(KeyBuf,GetKeyData(n->Leaf.NoOfKeysThisNode-1,n),HeadNode.KeyLen);
         PutKeyData( TempNode->CurKeyNo, TempNode );
         return PutLeafNode( TempNode->NodeNo, TempNode );
      }
      TempNode = TempNode->PrevNode;
   }
   return XB_NO_ERROR;
}
/***********************************************************************/
/* This routine queues up a list of nodes which have been emptied      */
void xbNdx::UpdateDeleteList( xbNdxNodeLink *n )
{
   n->NextNode = DeleteChain;
   DeleteChain = n;
}
/***********************************************************************/
/* Delete nodes from the node list - for now we leave the empty nodes  */
/* dangling in the file. Eventually we will remove nodes from the file */

void xbNdx::ProcessDeleteList( void )
{
   if( DeleteChain )
   {
      ReleaseNodeMemory( DeleteChain );
      DeleteChain = NULL;
   }
}
/***********************************************************************/
xbShort xbNdx::KeyWasChanged( void )
{
   CreateKey( 0, 0 );            /* use KeyBuf,  RecBuf    */
   CreateKey( 1, 1 );            /* use KeyBuf2, RecBuf2   */
   if( CompareKey( KeyBuf, KeyBuf2, HeadNode.KeyLen ) != 0 )
      return 1;
   else
      return 0;
}
/***********************************************************************/
xbNdxNodeLink * xbNdx::LeftSiblingHasSpace( xbNdxNodeLink * n )
{
   xbNdxNodeLink * TempNode;
   xbNdxNodeLink * SaveCurNode;

   /* returns a Nodelink to xbNdxNodeLink n's left sibling if it has space */

   /* if left most node in parent return NULL */
   if( n->PrevNode->CurKeyNo == 0 )  
      return NULL;

   SaveCurNode = CurNode;
   GetLeafNode( GetLeftNodeNo( n->PrevNode->CurKeyNo-1, n->PrevNode ), 2 ); 
   if( CurNode->Leaf.NoOfKeysThisNode < HeadNode.KeysPerNode )
   {
      TempNode = CurNode;  
      CurNode = SaveCurNode;
      TempNode->PrevNode = n->PrevNode;
      return TempNode;
   }
   else  /* node is already full */
   {
      ReleaseNodeMemory( CurNode );
      CurNode = SaveCurNode;
      return NULL;
   }
}
/***********************************************************************/
xbNdxNodeLink * xbNdx::RightSiblingHasSpace( xbNdxNodeLink * n )
{
 /* returns a Nodelink to xbNdxNodeLink n's right sibling if it has space */

   xbNdxNodeLink * TempNode;
   xbNdxNodeLink * SaveCurNode;

   /* if left most node in parent return NULL */
   if( n->PrevNode->CurKeyNo >= n->PrevNode->Leaf.NoOfKeysThisNode )  
      return NULL;

   SaveCurNode = CurNode;
   /* point curnode to right sib*/
   GetLeafNode( GetLeftNodeNo( n->PrevNode->CurKeyNo+1, n->PrevNode ), 2 ); 

   if( CurNode->Leaf.NoOfKeysThisNode < HeadNode.KeysPerNode )
   {
      TempNode = CurNode;  
      CurNode = SaveCurNode;
      TempNode->PrevNode = n->PrevNode;
      return TempNode;
   }
   else /* node is already full */
   {
      ReleaseNodeMemory( CurNode );
      CurNode = SaveCurNode;
      return NULL;
   }
}
/*************************************************************************/
xbShort xbNdx::MoveToRightNode( xbNdxNodeLink * n, xbNdxNodeLink * Right )
{
   xbShort j;
   xbNdxNodeLink * TempNode;
   xbNdxNodeLink * SaveCurNode;
   xbNdxNodeLink * SaveNodeChain;

   if( n->CurKeyNo == 0 )
   {
      j = 1;
      SaveNodeChain = NodeChain;
      SaveCurNode = CurNode;
      NodeChain = NULL;
      GetLastKey( n->NodeNo, 0 ); 
      memcpy( KeyBuf, GetKeyData( CurNode->CurKeyNo, CurNode),HeadNode.KeyLen);
      ReleaseNodeMemory( NodeChain );
      NodeChain = SaveNodeChain;
      CurNode = SaveCurNode;
   }
   else
   {
      j = 0;
      memcpy( KeyBuf, GetKeyData( j, n ), HeadNode.KeyLen);
   }
   PutKeyInNode( Right, 0, 0L, GetLeftNodeNo( j, n ), 1 );
   ReleaseNodeMemory( Right );
   TempNode = n;
   CurNode = n->PrevNode;
   n = n->PrevNode;
   n->NextNode = NULL;
   UpdateDeleteList( TempNode );
   DeleteSibling( n );
   return XB_NO_ERROR;
}
/***********************************************************************/
xbShort xbNdx::MoveToLeftNode( xbNdxNodeLink * n, xbNdxNodeLink * Left )
{
   xbShort j, rc;
   xbNdxNodeLink * SaveNodeChain;
   xbNdxNodeLink * TempNode;

   if( n->CurKeyNo == 0 )
      j = 1;
   else
      j = 0;
   
   /* save the original node chain */
   SaveNodeChain = NodeChain;
   NodeChain = NULL;

   /* determine new right most key for left node */
   GetLastKey( Left->NodeNo, 0 );
   memcpy( KeyBuf, GetKeyData( CurNode->CurKeyNo, CurNode ), HeadNode.KeyLen);
   ReleaseNodeMemory( NodeChain );
   NodeChain = NULL;			/* for next GetLastKey */
   PutKeyData( Left->Leaf.NoOfKeysThisNode, Left); 
   PutLeftNodeNo( Left->Leaf.NoOfKeysThisNode+1, Left, GetLeftNodeNo( j,n ));
   Left->Leaf.NoOfKeysThisNode++;
   Left->CurKeyNo = Left->Leaf.NoOfKeysThisNode;
   if(( rc = PutLeafNode( Left->NodeNo, Left )) != 0 )
	   return rc;

   n->PrevNode->NextNode = NULL;
   UpdateDeleteList( n );

   /* get the new right most key for left to update parents */
   GetLastKey( Left->NodeNo, 0 );
   
   /* assemble the chain */
   TempNode = Left->PrevNode;
   TempNode->CurKeyNo--;
   NodeChain->PrevNode = Left->PrevNode;
   UpdateParentKey( CurNode );
   ReleaseNodeMemory( NodeChain );
   ReleaseNodeMemory( Left );
   CurNode = TempNode;
   NodeChain = SaveNodeChain;
   TempNode->CurKeyNo++;    
   DeleteSibling( TempNode );
   return XB_NO_ERROR;
}
/***********************************************************************/
xbShort xbNdx::DeleteSibling( xbNdxNodeLink * n )
{
   xbNdxNodeLink * Left;
   xbNdxNodeLink * Right;
   xbNdxNodeLink * SaveCurNode;
   xbNdxNodeLink * SaveNodeChain;
   xbNdxNodeLink * TempNode;
   xbShort  rc;

   /* this routine deletes sibling CurRecNo out of xbNodeLink n */
   if( n->Leaf.NoOfKeysThisNode > 1 )
   {  
      RemoveKeyFromNode( n->CurKeyNo, n );
      if( n->CurKeyNo == n->Leaf.NoOfKeysThisNode )
      {
         SaveNodeChain = NodeChain;
         SaveCurNode = CurNode;
         NodeChain = NULL;
         GetLastKey( n->NodeNo, 0 );
         /* assemble the node chain */
         TempNode = NodeChain->NextNode;
         NodeChain->NextNode = NULL;
         ReleaseNodeMemory( NodeChain );
         TempNode->PrevNode = n;
         UpdateParentKey( CurNode );
         /* take it back apart */
         ReleaseNodeMemory( TempNode );
         NodeChain = SaveNodeChain;
         CurNode = SaveCurNode;
      }
   }
   else if( n->NodeNo == HeadNode.StartNode )
   {
      /* get here if root node and only one child remains */
      /* make remaining node the new root */
      if( n->CurKeyNo == 0 )   
         HeadNode.StartNode = GetLeftNodeNo( 1, n );
      else
         HeadNode.StartNode = GetLeftNodeNo( 0, n );
      UpdateDeleteList( n );
      NodeChain = NULL;
      CurNode = NULL;
   }
   else if (( Left = LeftSiblingHasSpace( n )) != NULL )
   {
      return MoveToLeftNode( n, Left );
   }
   else if (( Right = RightSiblingHasSpace( n )) != NULL )
   {
      return MoveToRightNode( n, Right );
   }
   /* else if left sibling exists */   
   else if( n->PrevNode->CurKeyNo > 0 )
   {
      /* move right branch from left sibling to this node */  
      SaveCurNode = CurNode;
      SaveNodeChain = NodeChain;
      NodeChain = NULL;
      GetLeafNode( GetLeftNodeNo( n->PrevNode->CurKeyNo-1, n->PrevNode ), 2 );
      Left = CurNode;
      Left->PrevNode = SaveCurNode->PrevNode;
      GetLastKey( Left->NodeNo, 0 );
      
      strncpy( KeyBuf, GetKeyData( CurNode->CurKeyNo,CurNode),HeadNode.KeyLen );
      if( n->CurKeyNo == 1 )
         PutLeftNodeNo( 1, n, GetLeftNodeNo( 0, n ));
      PutKeyData( 0, n );
      PutLeftNodeNo( 0, n, GetLeftNodeNo( Left->Leaf.NoOfKeysThisNode, Left ));
      if(( rc = PutLeafNode( n->NodeNo, n )) != XB_NO_ERROR ) return rc;
      SaveCurNode = n->PrevNode;
      SaveCurNode->NextNode = NULL;
      ReleaseNodeMemory( n );
      Left->Leaf.NoOfKeysThisNode--;
      if(( rc = PutLeafNode( Left->NodeNo, Left )) != XB_NO_ERROR ) return rc;
      /* rebuild left side of tree */
      GetLastKey( Left->NodeNo, 0 );
      NodeChain->PrevNode = SaveCurNode;
      SaveCurNode->CurKeyNo--;
      UpdateParentKey( CurNode );
      ReleaseNodeMemory( NodeChain );
      ReleaseNodeMemory( Left );
      CurNode = SaveCurNode;
      NodeChain = SaveNodeChain;
   }
   /* right sibling must exist */
   else if( n->PrevNode->CurKeyNo <= n->PrevNode->Leaf.NoOfKeysThisNode )
   {
      /* move left branch from left sibling to this node */   
      SaveCurNode = CurNode;
      SaveNodeChain = NodeChain;
      NodeChain = NULL;

      /* move the left node number one to the left if necessary */
      if( n->CurKeyNo == 0 )
      {
         PutLeftNodeNo( 0, n, GetLeftNodeNo( 1, n ));
         GetLastKey( GetLeftNodeNo( 0, n ), 0 );
         memcpy(KeyBuf,GetKeyData(CurNode->CurKeyNo,CurNode),HeadNode.KeyLen);
         PutKeyData( 0, n );
         ReleaseNodeMemory( NodeChain );
         NodeChain = NULL;
      }
      GetLeafNode( GetLeftNodeNo( n->PrevNode->CurKeyNo+1, n->PrevNode ), 2 );
      
      /* put leftmost node number from right node in this node */
      PutLeftNodeNo( 1, n, GetLeftNodeNo( 0, CurNode ));      
      if(( rc = PutLeafNode( n->NodeNo, n )) != XB_NO_ERROR ) return rc;

      /* remove the key from the right node */
      RemoveKeyFromNode( 0, CurNode );
      if(( rc = PutLeafNode( CurNode->NodeNo, CurNode )) != XB_NO_ERROR ) return rc;
      ReleaseNodeMemory( CurNode );

      /* update new parent key value */
      GetLastKey( n->NodeNo, 0 );
      NodeChain->PrevNode = n->PrevNode;
      UpdateParentKey( CurNode );
      ReleaseNodeMemory( NodeChain );
      
      NodeChain = SaveNodeChain;
      CurNode = SaveCurNode;
   }
   else
   {
      /* this should never be true-but could be if 100 byte limit is ignored*/
      cout << "Fatal index error\n";
      exit(0);
   }
   return XB_NO_ERROR;   
}
/***********************************************************************/
xbShort xbNdx::DeleteKey( xbLong DbfRec )
{
/* this routine assumes the key to be deleted is in KeyBuf */

   xbNdxNodeLink * TempNode;
   xbShort rc;

   if( HeadNode.KeyType != 0x00 )
     xb_error(XB_INVALID_KEY_TYPE);

   if(( rc = FindKey( KeyBuf, DbfRec )) != XB_FOUND )
      return rc;

   /* found the record to delete at this point */
   HeadNode.NoOfKeys--;

   /* delete the key from the node                                    */
   if(( rc = RemoveKeyFromNode( CurNode->CurKeyNo, CurNode )) != 0 )
      return rc;

   /* if root node, we are done */
   if( !( CurNode->NodeNo == HeadNode.StartNode ))
   {
      /* if leaf node now empty */
      if( CurNode->Leaf.NoOfKeysThisNode == 0 )
      {
         TempNode = CurNode->PrevNode;
         TempNode->NextNode = NULL;
         UpdateDeleteList( CurNode );
         CurNode = TempNode;
         DeleteSibling( CurNode );
         ProcessDeleteList();
      }

      /* if last key of leaf updated, update key in parent node */
      /* this logic updates the correct parent key              */

      else if( CurNode->CurKeyNo == CurNode->Leaf.NoOfKeysThisNode )
      {
         UpdateParentKey( CurNode );
      }
   }
   
   CurDbfRec = GetDbfNo( CurNode->CurKeyNo, CurNode );

   if(( rc = PutHeadNode( &HeadNode, indexfp, 1 )) != 0 )
      return rc;
   return XB_NO_ERROR;
}
/************************************************************************/
#ifdef XBASE_DEBUG
xbShort xbNdx::CheckIndexIntegrity( const xbShort option )
{
   /* if option = 1, print out some stats */

   xbShort rc;
   xbLong ctr = 1L;

   rc = dbf->GetRecord( ctr );
   while( ctr < dbf->NoOfRecords() )
   {
      ctr++;
      if( option ) cout << "\nChecking Record " << ctr;
      if(!dbf->RecordDeleted())      
      {
         CreateKey( 0, 0 );
         rc = FindKey( KeyBuf, dbf->GetCurRecNo());
         if( rc != XB_FOUND )
         {
            if( option )
            {
               cout << "\nRecord number " << dbf->GetCurRecNo() <<  " Not Found\n";
               cout << "Key = " << KeyBuf << "\n";
            }
            return rc;
         }
      }
      if(( rc = dbf->GetRecord( ctr )) != XB_NO_ERROR )
         return rc;
   }
   if( option ){
      cout << "\nTotal records checked = " << ctr << "\n";
      cout << "Exiting with rc = " << rc << "\n";
   }

   return XB_NO_ERROR;
}
#endif
/***********************************************************************/
xbShort xbNdx::ReIndex(void (*statusFunc)(xbLong itemNum, xbLong numItems))
{
   /* this method assumes the index has been locked in exclusive mode */

   xbLong l;
   xbShort rc = XB_NO_ERROR, i, NameLen, saveAutoLock;
   xbNdxHeadNode TempHead;
   FILE *t;
   xbString TempName;

   memcpy( &TempHead, &HeadNode, sizeof( struct xbNdxHeadNode ));

   TempHead.NoOfKeys = 1L;
   TempHead.TotalNodes = 2L;
   TempHead.StartNode = 1L;

   if(( rc = dbf->xbase->DirectoryExistsInName( IndexName )) > 0 )
      NameLen = rc + 13;
   else
      NameLen = 13;

   if (rc) 
   {
      TempName.assign(IndexName, 0, rc);
      TempName += "TEMPFILE.NDX";
   } 
   else
      TempName = "TEMPFILE.NDX";
 
   if(( t = fopen( TempName, "w+b" )) == NULL )
      xb_open_error(TempName);

   if(( rc = PutHeadNode( &TempHead, t, 0 )) != 0 )
   {
      fclose( t );
      remove(TempName);
      return rc;
   }

   for( i = 0; i < XB_NDX_NODE_SIZE; i++ )
   {
      if(( fwrite( "\x00", 1, 1, t )) != 1 )
      {
         fclose( t );
         remove(TempName);
         xb_io_error(XB_WRITE_ERROR, TempName);
      }
   }

   if( fclose( indexfp ) != 0 )
      xb_io_error(XB_CLOSE_ERROR, IndexName);

   if( fclose( t ) != 0 )
      xb_io_error(XB_CLOSE_ERROR, TempName);

   if( remove( IndexName ) != 0 )
      xb_io_error(XB_CLOSE_ERROR, IndexName);
    
   if( rename(TempName, IndexName ) != 0 )
      xb_io_error(XB_WRITE_ERROR, IndexName);

   if(( indexfp = fopen( IndexName, "r+b" )) == NULL )
      xb_open_error(IndexName);

   saveAutoLock = dbf->GetAutoLock();
   dbf->AutoLockOff();
   
   for( l = 1; l <= dbf->PhysicalNoOfRecords(); l++ )
   {
      if(statusFunc && (l == 1 || !(l % 100) || l == dbf->PhysicalNoOfRecords()))
         statusFunc(l, dbf->PhysicalNoOfRecords());
         
      if(( rc = dbf->GetRecord(l)) != XB_NO_ERROR )
         goto Outtahere;

      if(!dbf->GetRealDelete() || !dbf->RecordDeleted())
      {
         /* Create the key */
         CreateKey( 0, 0 );

         /* add key to index */
         if(( rc = AddKey( l )) != XB_NO_ERROR )
            goto Outtahere;
      }
   } 
 
Outtahere:
   if(saveAutoLock)
      dbf->AutoLockOn();
   return rc;
}

void 
xbNdx::SetNodeSize(xbShort size)
{
#ifdef XB_VAR_NODESIZE
  if(size >= XB_DEFAULT_NDX_NODE_SIZE)
  {
    if(size % XB_NDX_NODE_MULTIPLE)
      NodeSize = ((size + XB_NDX_NODE_MULTIPLE) / XB_NDX_NODE_MULTIPLE) *
                 XB_NDX_NODE_MULTIPLE;
    else
      NodeSize = size;
  }
  else
    NodeSize = XB_DEFAULT_NDX_NODE_SIZE;
#endif
}

void
xbNdx::GetExpression(char *buf, int len)
{
  memcpy(buf, HeadNode.KeyExpression, 
         len < XB_NDX_NODE_SIZE ? len : XB_NDX_NODE_SIZE - XB_NDX_NODE_BASESIZE);
}

#endif     /* XB_INDEX_NDX */
