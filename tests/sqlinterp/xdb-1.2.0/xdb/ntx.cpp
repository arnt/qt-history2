/*  $Id: ntx.cpp,v 1.5 1999/03/19 10:56:34 willy Exp $

    Xbase project source code

    NTX (Clipper) indexing routines for X-Base

    Copyright (C) 1999 SynXis Corp., Bob Cotton
    email - bob@synxis.com

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

#include <xdb/xbase.h>

#ifdef XB_INDEX_NTX

#ifdef HAVE_IO_H
#include <io.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ctype.h>
#include <sys/stat.h>

/* FIXME?	Why <unistd.h> is there?  Nothing bad happens if it isn't.
   -- willy */
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <xdb/xbexcept.h>

/***********************************************************************/
xbShort xbNtx::CloneNodeChain( void )
{
   xbNodeLink * TempNodeS;
   xbNodeLink * TempNodeT;
   xbNodeLink * TempNodeT2;
   xbUShort *sp;

   if( CloneChain ) ReleaseNodeMemory( CloneChain );
   CloneChain = NULL;

   if( !NodeChain ) return XB_NO_ERROR;
   TempNodeS = NodeChain;
   TempNodeT2 = NULL;

   while( TempNodeS )
   {
      if(( TempNodeT = GetNodeMemory()) == NULL )
#ifdef HAVE_EXCEPTIONS
      throw xbOutOfMemoryException();
#else
      return XB_NO_MEMORY;
#endif
      sp = TempNodeT->offsets;
      memcpy( TempNodeT, TempNodeS, sizeof( struct xbNodeLink ));
      TempNodeT->offsets = sp;
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
xbShort xbNtx::UncloneNodeChain( void )
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
void xbNtx::DumpNodeChain( void )
{
   xbNodeLink  *n;
   cout << "\n*************************\n";
   cout <<   "NodeLinkCtr = " << NodeLinkCtr;
   cout << "\nReused      = " << ReusedNodeLinks << "\n";

   n = NodeChain;
   while(n)
   {
      cout << "xbNodeLink Chain" << n->NodeNo << "\n";
      n = n->NextNode;
   }
   n = FreeNodeChain;
   while(n)
   {
      cout << "FreeNodeLink Chain" << n->NodeNo << "\n";
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

void xbNtx::ReleaseNodeMemory( xbNodeLink * n )
{
   xbNodeLink * temp;

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

xbNodeLink * xbNtx::GetNodeMemory( void )
{
   xbNodeLink * temp;
   if( FreeNodeChain )
   {
      temp = FreeNodeChain;
      temp->offsets = FreeNodeChain->offsets;
      FreeNodeChain = temp->NextNode;
      ReusedNodeLinks++;
      
      memset( temp->Leaf.KeyRecs, 0x00, XB_NTX_NODE_SIZE );
      temp->Leaf.NoOfKeysThisNode = 0;

      temp->PrevNode = 0x00;
      temp->NextNode = 0x00;
      temp->CurKeyNo = 0L;
      temp->NodeNo = 0L;

      for (int i = 0; i < HeadNode.KeysPerNode + 1; i++)
      {
          temp->offsets[i] = 2 + ((HeadNode.KeysPerNode + 1) * 2) + (HeadNode.KeySize * i);
      }
      
      
      
   }
   else
   {
      temp = (xbNodeLink *) malloc( sizeof( xbNodeLink ));
      memset( temp, 0x00, sizeof( xbNodeLink ));
      temp->offsets = (xbUShort *)malloc( (HeadNode.KeysPerNode + 1) * sizeof(xbUShort));
      NodeLinkCtr++;
   }
   return temp;
}
/***********************************************************************/
#ifdef XBASE_DEBUG
void xbNtx::DumpHdrNode( void )
{
    cout << "\nSignature          = " << HeadNode.Signature;
    cout << "\nVersion            = " << HeadNode.Version;
    cout << "\nStartPahe          = " << HeadNode.StartNode;
    cout << "\nUnusedOffset       = " << HeadNode.UnusedOffset;
    cout << "\nKeySize            = " << HeadNode.KeySize;
    cout << "\nKeyLen             = " << HeadNode.KeyLen;
    cout << "\nDecimalCount       = " << HeadNode.DecimalCount;
    cout << "\nKeysPerNode        = " << HeadNode.KeysPerNode;
    cout << "\nHalfKeysPerPage    = " << HeadNode.HalfKeysPerNode;
    cout << "\nKeyExpression      = " << HeadNode.KeyExpression;
    cout << "\nUnique             = " << HeadNode.Unique;
    cout << "\n";
}
#endif
/***********************************************************************/
xbNtx::xbNtx( xbDbf * pdbf )  : xbIndex (pdbf)
{
   memset( Node, 0x00, XB_NTX_NODE_SIZE );
   memset( &HeadNode, 0x00, sizeof( NtxHeadNode )); 
   NodeChain       = NULL;
   CloneChain      = NULL;
   FreeNodeChain   = NULL;
   DeleteChain     = NULL;
   CurNode         = NULL;
   NodeLinkCtr     = 0L;
   ReusedNodeLinks = 0L;
}
/***********************************************************************/
xbShort xbNtx::OpenIndex( const char * FileName )
{
   int NameLen, rc;

   NameLen = strlen( FileName ) + 1;
   if (( rc = dbf->NameSuffixMissing( 4, FileName )) > 0)
       NameLen += 4;

   IndexName = FileName;
   
   if( rc == 1 )
       IndexName += ".ntx";
   else if ( rc == 2 )
       IndexName += ".NTX";

   /* open the file */
   if(( indexfp = fopen( IndexName, "r+b" )) == NULL )
   {
     xb_open_error(IndexName);
   }
   
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

   KeyBuf  = (char *) malloc( HeadNode.KeyLen + 1 ); 
   KeyBuf2 = (char *) malloc( HeadNode.KeyLen + 1);
   memset( KeyBuf,  0x00, HeadNode.KeyLen + 1 );
   memset( KeyBuf2, 0x00, HeadNode.KeyLen + 1 );

#ifdef XBASE_DEBUG
//   CheckIndexIntegrity( 0 );
#endif

#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
   return dbf->AddIndexToIxList( index, IndexName );  
}
/***********************************************************************/
xbShort xbNtx::CloseIndex( void )
{
   if( KeyBuf )    { free ( KeyBuf );    KeyBuf = NULL;    }
   if( KeyBuf2 )   { free ( KeyBuf2 );   KeyBuf2 = NULL;   }
   dbf->RemoveIndexFromIxList( index );

   fclose( indexfp );
   IndexStatus = 0;
   return 0;
}
/***********************************************************************/
xbShort xbNtx::GetHeadNode( void )
{
   char *p;

   if( !IndexStatus )
       xb_error(XB_NOT_OPEN);

   if( fseek( indexfp, 0, SEEK_SET ))
       xb_io_error(XB_SEEK_ERROR, IndexName);

   if(( fread( Node, XB_NTX_NODE_SIZE, 1, indexfp )) != 1 )
       xb_io_error(XB_READ_ERROR, IndexName);

   /* load the head node structure */
   p = Node;
   HeadNode.Signature = dbf->xbase->GetShort( p ); p += sizeof(xbUShort);
   HeadNode.Version = dbf->xbase->GetShort( p ); p += sizeof(xbUShort);
   HeadNode.StartNode = dbf->xbase->GetULong( p ); p += sizeof(xbULong);
   HeadNode.UnusedOffset = dbf->xbase->GetULong( p ); p += sizeof(xbULong);
   HeadNode.KeySize = dbf->xbase->GetShort( p ); p += sizeof(xbUShort);
   HeadNode.KeyLen = dbf->xbase->GetShort( p ); p += sizeof(xbUShort);
   HeadNode.DecimalCount = dbf->xbase->GetShort( p ); p += sizeof(xbUShort);
   HeadNode.KeysPerNode = dbf->xbase->GetShort( p ); p += sizeof(xbUShort);
   HeadNode.HalfKeysPerNode = dbf->xbase->GetShort( p ); p += sizeof(xbUShort);
   strncpy(HeadNode.KeyExpression, p, 256); p+= 256;
   HeadNode.Unique = *p++;

   p = HeadNode.KeyExpression;
   while (*p)
   {
       *p = toupper(*p);
       p++;
   }

   return 0;
}
/***********************************************************************/
/* This routine reads a leaf node from disk                            */
/*                                                                     */
/*  If SetNodeChain 2, then the node is not appended to the node chain */
/*                     but the CurNode pointer points to the node read */
/*  If SetNodeChain 1, then the node is appended to the node chain     */
/*  If SetNodeChain 0, then record is only read to Node memory         */

xbShort xbNtx::GetLeafNode( xbLong NodeNo, xbShort SetNodeChain )
{
   xbNodeLink *n;
   char *p;

   if( !IndexStatus )
       xb_error(XB_NOT_OPEN);

   if( fseek( indexfp, NodeNo, SEEK_SET ))
       xb_io_error(XB_SEEK_ERROR, IndexName);

   if(( fread( Node, XB_NTX_NODE_SIZE, 1, indexfp )) != 1 )
       xb_io_error(XB_READ_ERROR, IndexName);

   if( !SetNodeChain ) return 0;

   if(( n = GetNodeMemory()) == NULL )
       xb_memory_error;

   n->NodeNo = NodeNo;
   n->CurKeyNo = 0L;
   n->NextNode = NULL;

   // The offsets at the head of each leaf are not necessarly in order.
   p = Node + 2;
   for (int i = 0; i < HeadNode.KeysPerNode + 1; i++)
   {
       n->offsets[i] = dbf->xbase->GetShort( p );
       p += 2;
   }
   

   // Do the edian translation correctly
   n->Leaf.NoOfKeysThisNode = dbf->xbase->GetShort( Node );
   memcpy( n->Leaf.KeyRecs,
           Node,
           XB_NTX_NODE_SIZE);
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
void xbNtx::DumpNodeRec( xbLong n )
{
   char *p;
   xbShort NoOfKeys;
   xbLong LeftBranch, RecNo;
   xbShort i,j;

   GetLeafNode( n, 0 );
   NoOfKeys = dbf->xbase->GetShort( Node );
   p = Node + 4;			/* go past no of keys */
   cout << "\n--------------------------------------------------------";
   cout << "\nNode # " << n << " Number of keys = " << NoOfKeys << "\n";

   cout << "\n Key     Left     Rec      Key";
   cout << "\nNumber  Branch   Number    Data";

   for( i = 0; i < GetKeysPerNode()+1 /*NoOfKeys*/; i++ )
   {
      LeftBranch = dbf->xbase->GetLong( p );
      p+=4;
      RecNo = dbf->xbase->GetLong( p );
      p+=4;
      cout << "\n" << i << "         " << LeftBranch << "          " << RecNo << "         ";
      for( j = 0; j < HeadNode.KeyLen; j++ ) cout << *p++;
   }
}
#endif
/***********************************************************************/
xbLong xbNtx::GetDbfNo( xbShort RecNo, xbNodeLink * n )
{
   NtxLeafNode *temp;
   char *p;
   xbUShort itemOffset;
   
   if( !n ) return 0L;
   temp = &n->Leaf;
   p = temp->KeyRecs;
   if( RecNo < 0 || RecNo > ( temp->NoOfKeysThisNode  )) return 0L;
   itemOffset = GetItemOffset(RecNo, n, 0);
   // ItemOffset is from the beginning of the record.
   p += itemOffset;
   p += 4;
   return( dbf->xbase->GetLong( p ));
}
/***********************************************************************/
xbULong xbNtx::GetLeftNodeNo( xbShort RecNo, xbNodeLink * n )
{
   NtxLeafNode *temp;
   char *p;
   xbUShort itemOffset;
   
   if( !n ) return 0L;
   temp = &n->Leaf;
   p = temp->KeyRecs;
   if( RecNo < 0 || RecNo > temp->NoOfKeysThisNode  ) return 0L;
   itemOffset = GetItemOffset(RecNo, n, 0);
   // ItemOffset is from the beginning of the record.
   p += itemOffset;
   return( dbf->xbase->GetULong( p ));
}

    

/***********************************************************************/
char * xbNtx::GetKeyData( xbShort RecNo, xbNodeLink * n )
{
   NtxLeafNode *temp;
   char *p;
   xbUShort itemOffset;
   if( !n ) return 0L;
   temp = &n->Leaf;
   p = temp->KeyRecs;
   if( RecNo < 0 || RecNo > ( temp->NoOfKeysThisNode )) return 0L;
   itemOffset = GetItemOffset(RecNo, n, 0);
   // ItemOffset is from the beginning of the record.
   p += itemOffset + 8;
   return( p );
}
/***********************************************************************/
xbUShort
xbNtx::GetItemOffset(xbShort RecNo, xbNodeLink *n, xbShort NewOffset = 0)
{
    if (RecNo > (this->HeadNode.KeysPerNode + 1))
    {
        cout << "RecNo = " << RecNo << endl;
        cout << "this->HeadNode.KeysPerNode = " << this->HeadNode.KeysPerNode << endl;
        cout << "********************* BUG ***********************" << endl;
        // ;-)
        exit(1);
    }
    

    return n->offsets[RecNo];
}

xbUShort
xbNtx::InsertKeyOffset(xbShort pos, xbNodeLink *n)
{
    xbUShort temp;

    // save the new offset
    temp  = n->offsets[n->Leaf.NoOfKeysThisNode + 1];

    for( int i = n->Leaf.NoOfKeysThisNode + 1; i > pos; i-- )
    {
        n->offsets[i] = n->offsets[i-1];
    }
    n->offsets[pos] = temp;
    
    return n->offsets[pos];
}

xbUShort
xbNtx::DeleteKeyOffset(xbShort pos, xbNodeLink *n)
{
    xbUShort temp;
    xbShort i;
    // save the old offset
    temp  = n->offsets[pos];

    for( i = pos; i < n->Leaf.NoOfKeysThisNode; i++ )
    {
        n->offsets[i] = n->offsets[i+1];
    }
    n->offsets[i] = temp;
    
    return n->offsets[i];
}


/***********************************************************************/
xbLong xbNtx::GetTotalNodes( void ) 
{
//   if( &HeadNode )
//      return HeadNode.TotalNodes;
//   else
      return 0L;
}
/***********************************************************************/
xbUShort xbNtx::GetKeysPerNode( void ) 
{
   if( &HeadNode )
      return HeadNode.KeysPerNode;
   else
      return 0L;
}
/***********************************************************************/
xbShort xbNtx::GetFirstKey( xbShort RetrieveSw )
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
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
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
xbShort xbNtx::GetNextKey( xbShort RetrieveSw )
{
/* This routine returns 0 on success and sets CurDbfRec to the record  */
/* corresponding to the next index pointer                             */

   xbNodeLink * TempNodeLink;

   xbLong TempNodeNo;
   xbShort rc;
    
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
   if(( CurNode->Leaf.NoOfKeysThisNode -1 ) > CurNode->CurKeyNo )
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
   if( CurNode->NodeNo == HeadNode.StartNode )
   {
#ifdef XB_LOCKING_ON
          if( dbf->GetAutoLock() )
             LockIndex(F_SETLKW, F_UNLCK);
#endif
#ifdef HAVE_EXCEPTIONS
		 throw xbEoFException();
#else
	   return XB_EOF;
#endif
   }


   /* this logic assumes that interior nodes have n+1 left node no's where */
   /* n is the number of keys in the node                                  */

   /* pop up one node to the interior node level & free the leaf node      */

   TempNodeLink = CurNode;
   CurNode = CurNode->PrevNode;
   CurNode->NextNode = NULL;
   ReleaseNodeMemory( TempNodeLink );

   /* while no more right keys && not head node, pop up one node */
   while(( CurNode->CurKeyNo >= CurNode->Leaf.NoOfKeysThisNode ) &&
          ( CurNode->NodeNo != HeadNode.StartNode ))
   {
      TempNodeLink = CurNode;
      CurNode = CurNode->PrevNode;
      CurNode->NextNode = NULL;
      ReleaseNodeMemory( TempNodeLink );
   }

   /* if head node && right most key, return end-of-file */
   if(( HeadNode.StartNode == CurNode->NodeNo ) &&
      ( CurNode->CurKeyNo >= CurNode->Leaf.NoOfKeysThisNode ))
   {
#ifdef XB_LOCKING_ON
          if( dbf->GetAutoLock() )
             LockIndex(F_SETLKW, F_UNLCK);
#endif
#ifdef HAVE_EXCEPTIONS
		 throw xbEoFException();
#else
	   return XB_EOF;
#endif
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
#ifdef XB_LOCKING_ON
          if( dbf->GetAutoLock() )
             LockIndex(F_SETLKW, F_UNLCK);
#endif
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
xbShort xbNtx::GetLastKey( xbLong NodeNo, xbShort RetrieveSw )
{
/* This routine returns 0 on success and sets CurDbfRec to the record  */
/* corresponding to the last index pointer                             */

/* If NodeNo = 0, start at head node, otherwise start at NodeNo        */

   xbLong TempNodeNo;
   xbShort rc;

// TODO
// NTX files keep no TotalNode count.   
//   if( NodeNo < 0 || NodeNo > HeadNode.TotalNodes )
//      return XB_INVALID_NODE_NO;

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
   if( NodeNo == 0L )
      if(( rc = GetHeadNode()) != 0 )
      { 
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         CurDbfRec = 0L;
         return rc;
      }

   /* get a node and add it to the link */

   if( NodeNo == 0L )
   {
      if(( rc = GetLeafNode( HeadNode.StartNode, 1 )) != 0 )
      {
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         CurDbfRec = 0L;
         return rc;
      }
   }
   else
   {
      if(( rc = GetLeafNode( NodeNo, 1 )) != 0 )
      {
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         CurDbfRec = 0L;
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
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         CurDbfRec = 0L;
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
xbShort xbNtx::GetPrevKey( xbShort RetrieveSw )
{
/* This routine returns 0 on success and sets CurDbfRec to the record  */
/* corresponding to the previous index pointer                         */

   xbNodeLink * TempNodeLink;

   xbLong TempNodeNo;
   xbShort rc;
    
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
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      CurDbfRec = 0L;
      return GetFirstKey( RetrieveSw );
   }

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

   if( !CurNode->PrevNode )       /* michael - make sure prev node exists */
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
#ifdef HAVE_EXCEPTIONS
		 throw xbEoFException();
#else
	   return XB_EOF;
#endif
   }


   TempNodeLink = CurNode;
   CurNode = CurNode->PrevNode;
   CurNode->NextNode = NULL;
   ReleaseNodeMemory( TempNodeLink );

   /* while no more left keys && not head node, pop up one node */
   while(( CurNode->CurKeyNo == 0 ) && 
          ( CurNode->NodeNo != HeadNode.StartNode ))
   {
      TempNodeLink = CurNode;
      CurNode = CurNode->PrevNode;
      CurNode->NextNode = NULL;
      ReleaseNodeMemory( TempNodeLink );
   }

   /* if head node && left most key, return end-of-file */
   if(( HeadNode.StartNode == CurNode->NodeNo ) &&
      ( CurNode->CurKeyNo == 0 ))
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
#ifdef HAVE_EXCEPTIONS
		 throw xbEoFException();
#else
	   return XB_EOF;
#endif
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
      CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode -1;

/* traverse down the right side of the tree */
   while( GetLeftNodeNo( 0, CurNode ))		/* while interior node */
   {
      TempNodeNo = GetLeftNodeNo( CurNode->Leaf.NoOfKeysThisNode, CurNode );     
      if(( rc = GetLeafNode( TempNodeNo, 1 )) != 0 )
      {
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         CurDbfRec = 0L;
         return rc;     
      }
      if( GetLeftNodeNo( 0, CurNode ))	/* if interior node */
         CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode;
      else					/* leaf node */
         CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode -1;
   }
   CurDbfRec = GetDbfNo( CurNode->Leaf.NoOfKeysThisNode -1, CurNode );
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
xbShort xbNtx::CompareKey( const char * Key1, const char * Key2, xbShort Klen )
{
/*   if key1 = key2  --> return 0      */
/*   if key1 > key2  --> return 1      */
/*   if key1 < key2  --> return 2      */

    const char   *k1, *k2;
    xbShort  i;

    if( Klen > HeadNode.KeyLen ) Klen = HeadNode.KeyLen;

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

}
/***********************************************************************/
xbShort xbNtx::CompareKey( const char * Key1, const char * Key2)
{
/*   if key1 = key2  --> return 0      */
/*   if key1 > key2  --> return 1      */
/*   if key1 < key2  --> return 2      */

    int rc;
    rc = strcmp(Key1, Key2);
    if (rc < 0)
        return 2;
    else if (rc > 0)
        return 1;
    else
        return 0;
}

/***********************************************************************/
xbULong xbNtx::GetLeafFromInteriorNode( const char * Tkey, xbShort Klen )
{
   /* This function scans an interior node for a key and returns the   */
   /* correct interior leaf node no                                    */

   xbShort p;

   /* if Tkey > any keys in node, return right most key */
   p = CurNode->Leaf.NoOfKeysThisNode -1 ;
   if( CompareKey( Tkey, GetKeyData( p, CurNode )) == 1 )
   {
      CurNode->CurKeyNo = CurNode->Leaf.NoOfKeysThisNode;
      return GetLeftNodeNo( CurNode->Leaf.NoOfKeysThisNode, CurNode );
   }
   
   /* otherwise, start at the beginning and scan up */
#if 0
   p = 0;
   while( p < CurNode->Leaf.NoOfKeysThisNode)
   {
       rc = CompareKey( Tkey, GetKeyData( p, CurNode ) );
       if (rc == 2) break;
       else if (rc == 0)
       {
           CurNode->CurKeyNo = p;
           CurDbfRec = GetDbfNo( p, CurNode );
           return 0;
       }
       p++;
   }
#else
   p = 0;
   while( p < CurNode->Leaf.NoOfKeysThisNode &&
          ( CompareKey( Tkey, GetKeyData( p, CurNode ), Klen ) == 1 ))
      p++;
#endif

   CurNode->CurKeyNo = p;
   return GetLeftNodeNo( p, CurNode );
}
/***********************************************************************/
xbShort xbNtx::KeyExists( xbDouble d )
{
   char buf[9];
   memset( buf, 0x00, 9 );
   dbf->xbase->PutDouble( buf, d );
   return FindKey( buf, 8, 0 );
}
/***********************************************************************/
xbShort xbNtx::FindKey( xbDouble d )
{
   char buf[9];
   memset( buf, 0x00, 9 );
   dbf->xbase->PutDouble( buf, d );
   return FindKey( buf, 8, 1 );
}
/***********************************************************************/
xbShort xbNtx::FindKey( const char * Key )
{
   return FindKey( Key, strlen( Key ), 1 );
}
/***********************************************************************/
xbShort xbNtx::FindKey( const char * Tkey, xbLong DbfRec )
{
   /* find a key with a specifc xbDbf record number */
   xbShort rc;

   xbLong CurDbfRecNo;
   xbLong CurNtxDbfNo;

#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      if((rc = LockIndex(F_SETLKW, F_RDLCK)) != 0)
        return rc;
#endif

   /* if we are already on the correct key, return XB_FOUND */
   if( CurNode )
   {
      CurDbfRecNo = dbf->GetCurRecNo();
      CurNtxDbfNo = GetDbfNo( CurNode->CurKeyNo, CurNode );
      if( CurDbfRecNo == CurNtxDbfNo ) 
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
xbShort xbNtx::FindKey( void )
{
   /* if no paramaters given, use KeyBuf */
   return( FindKey( KeyBuf, HeadNode.KeyLen, 0 ));
}
/***********************************************************************/
xbShort xbNtx::FindKey( const char * Tkey, xbShort Klen, xbShort RetrieveSw )
{
   /* This routine sets the current key to the found key */
 
   /* if RetrieveSw is true, the method positions the dbf record */

   xbShort rc,i;
   xbLong TempNodeNo;

#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      if((rc = LockIndex(F_SETLKW, F_RDLCK)) != 0)
        return rc;
#endif

   if( NodeChain )
   {
      ReleaseNodeMemory( NodeChain );
      NodeChain = NULL;
   }

   if(( rc = GetHeadNode()) != 0 )
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
      CurDbfRec = 0L;
      return rc;
   }

   // If the index is empty
   if ( HeadNode.StartNode == 0)
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
       return XB_NOT_FOUND;
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

#if 0
      // GetLeafFromInteriorNode will return 0 if the key is found on
      // an inode. But the leftNodeNo will not be 0. 
      if (TempNodeNo == 0 &&
          GetLeftNodeNo( 0, CurNode ) != 0)
      {
#ifdef XB_LOCKING_ON
          if( dbf->GetAutoLock() )
              LockIndex(F_SETLKW, F_UNLCK);
#endif
          if( RetrieveSw ) dbf->GetRecord( CurDbfRec );
          return XB_FOUND;
      }
#endif      
      
      if(( rc = GetLeafNode( TempNodeNo, 1 )) != 0 )
      {
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         CurDbfRec = 0L;
         return rc;
      }
   }

   /* leaf level */
   for( i = 0; i < CurNode->Leaf.NoOfKeysThisNode; i++ )
   {
      rc = CompareKey(  Tkey, GetKeyData( i, CurNode ) );
         
      if( rc == 0 ) 
      {
         CurNode->CurKeyNo = i;
         CurDbfRec = GetDbfNo( i, CurNode );
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         if( RetrieveSw ) dbf->GetRecord( CurDbfRec );
         return XB_FOUND;
      }
      else if( rc == 2 )
      {
         CurNode->CurKeyNo = i;
         CurDbfRec = GetDbfNo( i, CurNode );
         if( RetrieveSw ) dbf->GetRecord( CurDbfRec );
         // If key is lessthan, without length involved,
         // Check to see if the substring match
#ifdef XB_LOCKING_ON
         if( dbf->GetAutoLock() )
            LockIndex(F_SETLKW, F_UNLCK);
#endif
         if (CompareKey(  Tkey, GetKeyData( i, CurNode ), Klen ) == 0)
             return XB_FOUND;
         else
             return XB_NOT_FOUND;
      }
   }
   CurNode->CurKeyNo = i;
   CurDbfRec = GetDbfNo( i, CurNode );
#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
   if( RetrieveSw ) dbf->GetRecord( CurDbfRec );
   return XB_NOT_FOUND;
}
/***********************************************************************/
xbShort xbNtx::CalcKeyLen( void )
{
   xbShort rc;
   xbExpNode * TempNode;
   char FieldName[11];
   char Type;

   TempNode = dbf->xbase->GetFirstTreeNode( ExpressionTree );

   if( !TempNode ) return 0;
   if( TempNode->Type == 'd' ) return TempNode->ResultLen;
   if( TempNode->Type == 'D' )
   {
      memset( FieldName, 0x00, 11 );
      memcpy( FieldName, TempNode->NodeText, TempNode->Len );
      Type = dbf->GetFieldType( dbf->GetFieldNo( FieldName ));
      if( Type == 'N' || Type == 'F' )
         return TempNode->ResultLen;
   }

   if(( rc = dbf->xbase->ProcessExpression( ExpressionTree )) != XB_NO_ERROR )
      return 0;

   TempNode = (xbExpNode *) dbf->xbase->Pop();
   if( !TempNode ) return 0;
   rc = TempNode->DataLen;

   if( !TempNode->InTree ) dbf->xbase->FreeExpNode( TempNode );
   return rc;
}
/***********************************************************************/
xbShort xbNtx::CreateIndex(const char * IxName, const char * Exp, xbShort Unique, xbShort Overlay )
{
   xbShort i, NameLen, KeyLen, rc;


   IndexStatus = XB_CLOSED;
   if( strlen( Exp ) > 255 ) xb_error( XB_INVALID_KEY_EXPRESSION);
   if( dbf->GetDbfStatus() == 0 ) xb_error( XB_NOT_OPEN);

   /* Get the index file name and store it in the class */
   NameLen = strlen( IxName ) + 1;

   if(( rc = dbf->NameSuffixMissing( 4, IxName )) > 0 )
      NameLen +=4;

/*
   if(( IndexName = (char *) malloc( NameLen )) == NULL )
   {
#ifdef HAVE_EXCEPTIONS
		 throw xbOutOfMemoryException();
#else
	   return XB_NO_MEMORY;
#endif
   }
*/           
               

   /* copy the name to the class variable */
   IndexName = IxName;
   
   if( rc == 1 )
       IndexName += ".ntx";
   else if( rc == 2 )
       IndexName += ".NTX";

   /* check if the file already exists */
   if (((indexfp = fopen( IndexName, "r" )) != NULL ) && !Overlay )
   {
      fclose( indexfp );
      xb_io_error(XB_FILE_EXISTS, IndexName);
   }
   else if( indexfp ) fclose( indexfp );

   if(( indexfp = fopen( IndexName, "w+b" )) == NULL )
   {
      return XB_OPEN_ERROR;
   }

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
      return rc;
   ExpressionTree = dbf->xbase->GetTree();
   dbf->xbase->SetTreeToNull(); 

   /* build the header record */
   memset( &HeadNode, 0x00, sizeof( NtxHeadNode ));
   HeadNode.Signature = 0x6;   // Clipper 5.x
   HeadNode.Version = 1;
   HeadNode.StartNode  = 1024L;

   KeyLen = CalcKeyLen();


   // TODO
   // What is the Clipper key length limit?
   if( KeyLen == 0 || KeyLen > 100 )       /* 100 byte key length limit */
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
       xb_error(XB_INVALID_KEY);
   }
   else
   { 
      HeadNode.KeyLen = KeyLen; 
   }

   // This is not the algorithm that Clipper uses. I cant figure out
   // what they use from looking at the examples.
   // This is correct tho.
   HeadNode.KeysPerNode = (xbUShort)
       (( XB_NTX_NODE_SIZE - (2 * sizeof( xbUShort ))) / (HeadNode.KeyLen + 10 )) - 1;
   if (HeadNode.KeysPerNode % 2)
       HeadNode.KeysPerNode--;

   HeadNode.HalfKeysPerNode = (xbUShort) HeadNode.KeysPerNode / 2;

   HeadNode.KeySize = HeadNode.KeyLen + 8;
//   while(( HeadNode.KeySize % 4 ) != 0 ) HeadNode.KeySize++;  /* multiple of 4*/
   HeadNode.Unique = Unique;
   strncpy( HeadNode.KeyExpression, Exp, 255 );
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
   for( i = 0; i < XB_NTX_NODE_SIZE; i++ )
   {
      if(( fwrite( "\x00", 1, 1, indexfp )) != 1 )
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

   if ((rc = GetLeafNode(HeadNode.StartNode, 1)) != 0)
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
       return rc;
   }
   
   for (i = 0; i < HeadNode.KeysPerNode + 1; i++)
   {
       CurNode->offsets[i] = (i * HeadNode.KeySize) +
           2 + (2 * (HeadNode.KeysPerNode + 1));
   }
   
   if ((rc = PutLeafNode(HeadNode.StartNode, CurNode )) != 0)
   {
#ifdef XB_LOCKING_ON
      if( dbf->GetAutoLock() )
         LockIndex(F_SETLKW, F_UNLCK);
#endif
       return rc;
   }

#ifdef XB_LOCKING_ON
   if( dbf->GetAutoLock() )
      LockIndex(F_SETLKW, F_UNLCK);
#endif
   return dbf->AddIndexToIxList( index, IndexName );  
}
/***********************************************************************/
xbShort xbNtx::PutLeftNodeNo( xbShort RecNo, xbNodeLink *n, xbLong NodeNo )
{
   /* This routine sets n node's leftnode number */
   NtxLeafNode *temp;
   char *p;
   xbUShort itemOffset;
   if( !n ) xb_error( XB_INVALID_NODELINK);
   temp = &n->Leaf;
   if( RecNo < 0 || RecNo > HeadNode.KeysPerNode)
       xb_error(XB_INVALID_KEY);
   p = temp->KeyRecs;
   itemOffset = GetItemOffset(RecNo, n, 1);
   p += itemOffset;
   dbf->xbase->PutLong( p, NodeNo );
   return XB_NO_ERROR;
}
/***********************************************************************/
xbShort xbNtx::PutDbfNo( xbShort RecNo, xbNodeLink *n, xbLong DbfNo )
{
   /* This routine sets n node's dbf number */

   NtxLeafNode *temp;
   char *p;
   xbUShort itemOffset;
   if( !n ) xb_error( XB_INVALID_NODELINK);
   temp = &n->Leaf;
   if( RecNo < 0 || RecNo > (HeadNode.KeysPerNode))
       xb_error(XB_INVALID_KEY);
   itemOffset = GetItemOffset(RecNo, n, 1);
   p = temp->KeyRecs;
   p += itemOffset;
   p += 4;
   dbf->xbase->PutLong( p, DbfNo );
   return XB_NO_ERROR;
}
/************************************************************************/
xbShort xbNtx::PutLeafNode( xbLong l, xbNodeLink *n )
{
    NtxLeafNode *temp;
    char *p;
    
    if(( fseek( indexfp, l , SEEK_SET )) != 0 )
    {
        fclose( indexfp );
        xb_io_error( XB_SEEK_ERROR, IndexName );
    }

    temp = &n->Leaf;
    p = temp->KeyRecs;
    dbf->xbase->PutShort( p, temp->NoOfKeysThisNode );

    // The offsets at the head of each leaf are not necessarly in order.
    p += 2;
    for (int i = 0; i < HeadNode.KeysPerNode + 1; i++)
    {
        dbf->xbase->PutShort( p, n->offsets[i] );
        p += 2;
    }
   
    
    if(( fwrite( &n->Leaf.KeyRecs, XB_NTX_NODE_SIZE, 1, indexfp )) != 1 )
    {
        fclose( indexfp );
        xb_io_error(XB_WRITE_ERROR, IndexName);
    }

    PutHeadNode(&HeadNode, indexfp, 1);
    return 0;   
}
/************************************************************************/
xbShort xbNtx::PutHeadNode( NtxHeadNode * Head, FILE * f, xbShort UpdateOnly )
{
    char buf[4];
    char *p;
 
    if(( fseek( f, 0L, SEEK_SET )) != 0 )
    {
        fclose( f );
        xb_io_error( XB_SEEK_ERROR, IndexName );
    }

    memset( buf, 0x00, 2 );
    dbf->xbase->PutUShort( buf, Head->Signature );
    if(( fwrite( &buf, 2, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error(XB_WRITE_ERROR, IndexName);
    }

    memset( buf, 0x00, 2 );
    dbf->xbase->PutUShort( buf, Head->Version );
    if(( fwrite( &buf, 2, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }

    memset( buf, 0x00, 4 );
    dbf->xbase->PutULong( buf, Head->StartNode );
    if(( fwrite( &buf, 4, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }
    
    memset( buf, 0x00, 4 );
    dbf->xbase->PutULong( buf, Head->UnusedOffset );
    if(( fwrite( &buf, 4, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }

    if( UpdateOnly )
    {
        fflush(indexfp);
        xb_io_error( XB_NO_ERROR, IndexName );
    }

    memset( buf, 0x00, 2 );
    dbf->xbase->PutUShort( buf, Head->KeySize );
    if(( fwrite( &buf, 2, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }

    memset( buf, 0x00, 2 );
    dbf->xbase->PutUShort( buf, Head->KeyLen );
    if(( fwrite( &buf, 2, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }

    memset( buf, 0x00, 2 );
    dbf->xbase->PutUShort( buf, Head->DecimalCount );
    if(( fwrite( &buf, 2, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }

    memset( buf, 0x00, 2 );
    dbf->xbase->PutUShort( buf, Head->KeysPerNode );
    if(( fwrite( &buf, 2, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }

    memset( buf, 0x00, 2 );
    dbf->xbase->PutUShort( buf, Head->HalfKeysPerNode );
    if(( fwrite( &buf, 2, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }

    p = HeadNode.KeyExpression;
    while (*p)
    {
        *p = tolower(*p);
        p++;
    }

    if(( fwrite( &Head->KeyExpression, 256, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }
    
    memset( buf, 0x00, 1 );
    buf[0] = Head->Unique;
    if(( fwrite( &buf, 1, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }


    if(( fwrite( &Head->NotUsed, 745, 1, f )) != 1 )
    {
        fclose( f );
        xb_io_error( XB_WRITE_ERROR, IndexName );
    }
    return 0;   
}

xbShort xbNtx::TouchIndex( void )
{
    xbShort rc;
    if (( rc = GetHeadNode()) != XB_NO_ERROR) return rc;
    HeadNode.Version++;
    if (( rc = PutHeadNode(&HeadNode, indexfp, 1)) != XB_NO_ERROR) return rc;
    return XB_NO_ERROR;
}

/************************************************************************/
xbShort xbNtx::PutKeyData( xbShort RecNo, xbNodeLink *n )
{
   /* This routine copies the KeyBuf data into xbNodeLink n */
   NtxLeafNode *temp;
   char *p;
   xbShort i;
   xbUShort itemOffset;
   if( !n ) xb_error( XB_INVALID_NODELINK );
   temp = &n->Leaf;
   if( RecNo < 0 || RecNo > (HeadNode.KeysPerNode))
       xb_error( XB_INVALID_KEY );
   itemOffset = GetItemOffset(RecNo, n, 1);
   p = temp->KeyRecs;
   p += itemOffset;
   p += 8;
   for( i = 0; i < HeadNode.KeyLen; i++ )
   {
      *p = KeyBuf[i];
      p++;
   }
   return XB_NO_ERROR;
}
/************************************************************************/
xbShort xbNtx::PutKeyInNode( xbNodeLink * n, xbShort pos, xbLong d, xbLong l, xbShort w )
{

    /* check the node */
    if( !n ) xb_error( XB_INVALID_NODELINK );
    if( pos < 0 || pos > HeadNode.KeysPerNode ) xb_error( XB_INVALID_RECORD );
    if( n->Leaf.NoOfKeysThisNode >= HeadNode.KeysPerNode ) xb_error( XB_NODE_FULL );

    InsertKeyOffset(pos, n);

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
xbShort xbNtx::SplitLeafNode( xbNodeLink *n1, xbNodeLink *n2, xbShort pos, xbLong d )
{
   xbShort i,j,rc;
   xbShort temp;
   xbShort start;
   xbShort end;
   xbShort length;

   if( !n1 || !n2 ) xb_error( XB_INVALID_NODELINK );
   if( pos < 0 || pos > HeadNode.KeysPerNode ) xb_error( XB_INVALID_RECORD );

   length = strlen(KeyBuf);

   // If the new key goes in the first node.
   if (pos < HeadNode.HalfKeysPerNode)
   {
       // Setup key to insert into parent
       memcpy(PushItem.Key,
              GetKeyData(HeadNode.HalfKeysPerNode -1, n1),
              HeadNode.KeyLen);
       PushItem.RecordNumber = GetDbfNo(HeadNode.HalfKeysPerNode -1, n1);
       PushItem.Node =  0L; 
       start = pos;

       end = HeadNode.HalfKeysPerNode - 1;

       temp  = n1->offsets[end];
       for( i = end; i > start; i--)
       {
           n1->offsets[i] = n1->offsets[i-1];
       }
       n1->offsets[start] = temp;

       // Insert new key
       PutKeyData( start , n1 );
       PutDbfNo  ( start , n1, d );

   }
   else 
   {
       // If the passed-in key IS median key, just copy it.
       if (pos == HeadNode.HalfKeysPerNode)
       {
           memcpy(PushItem.Key,
                  KeyBuf,
                  HeadNode.KeyLen);
           PushItem.RecordNumber = d;
           start = pos;
           end = pos;

       }
       else
       {
           // Otherwise, the median key will be middle key becasue the
           // new key will be inserted somewhere above the middle.
           memcpy(PushItem.Key,
                  GetKeyData(HeadNode.HalfKeysPerNode,  n1),
                  HeadNode.KeyLen);
           PushItem.RecordNumber = GetDbfNo(HeadNode.HalfKeysPerNode, n1);
           start = HeadNode.HalfKeysPerNode ;
           end = pos -1;
       }
       temp  = n1->offsets[start];
       for( i = start; i < end; i++)
       {
           n1->offsets[i] = n1->offsets[i+1];
       }
       n1->offsets[end] = temp;

       // Insert new key
       PutKeyData( pos -1 , n1 );
       PutDbfNo  ( pos -1 , n1, d );

   }
   

      // Dup the node data
   memcpy(n2->Leaf.KeyRecs, n1->Leaf.KeyRecs, XB_NTX_NODE_SIZE);

   // Dup the offsets
   for ( i = 0; i < HeadNode.KeysPerNode +1; i++)
   {
       n2->offsets[i] = n1->offsets[i];
   }

   // Setup the second node
   for (j = 0, i = HeadNode.HalfKeysPerNode; i < HeadNode.KeysPerNode; i++, j++ )
   {
       temp = n2->offsets[j];
       n2->offsets[j] = n2->offsets[i];
       n2->offsets[i] = temp;
   }

   // Get the last offset for both nodes
   temp = n2->offsets[j];
   n2->offsets[j] = n2->offsets[HeadNode.KeysPerNode];
   n2->offsets[HeadNode.KeysPerNode] = temp;

   // Set the new count of both nodes
   n2->Leaf.NoOfKeysThisNode = HeadNode.HalfKeysPerNode;
   n1->Leaf.NoOfKeysThisNode = HeadNode.HalfKeysPerNode;


   if(( rc = PutLeafNode( n1->NodeNo, n1 )) != 0 )
       return rc;
   if(( rc = PutLeafNode( n2->NodeNo, n2 )) != 0 )
       return rc;
   return 0;
}
/************************************************************************/
xbShort xbNtx::SplitINode( xbNodeLink *n1, xbNodeLink *n2, xbLong t )
                   /* parent, tempnode, tempnodeno */
{
   xbShort i,j,rc;
   xbShort temp;
   
   xbShort pos = n1->CurKeyNo;
   xbShort start;
   xbShort end;

   NtxItem oldPushItem;
   oldPushItem.Node = PushItem.Node;
   oldPushItem.RecordNumber = PushItem.RecordNumber;
   memcpy(oldPushItem.Key, PushItem.Key, sizeof(PushItem.Key));
   
   //   n2->NodeNo = HeadNode.TotalNodes++;
   n2->NodeNo = GetNextNodeNo();


   // If the new key goes in the first node.
   if (pos < HeadNode.HalfKeysPerNode)
   {
       
      // Setup key to insert into parent
       memcpy(PushItem.Key,
              GetKeyData(HeadNode.HalfKeysPerNode -1, n1),
              HeadNode.KeyLen);
       PushItem.RecordNumber = GetDbfNo(HeadNode.HalfKeysPerNode -1, n1);
       PushItem.Node =  n2->NodeNo;
       start = pos;
       end = HeadNode.HalfKeysPerNode - 1;
   }
   
   else
   {
       // If the passed-in key IS median key, just copy it.
       if (pos == HeadNode.HalfKeysPerNode)
       {
           PutLeftNodeNo(0, n2, oldPushItem.Node);
           // PushItem should remain the same, except for its left pointer
           PushItem.Node = n2->NodeNo;
           start = pos;
           end = pos;
       }
       else 
       {
           // Otherwise, the median key will be middle key becasue the
           // new key will be inserted somewhere above the middle.
           memcpy(PushItem.Key,
                  GetKeyData(HeadNode.HalfKeysPerNode,  n1),
                  HeadNode.KeyLen);
           PushItem.RecordNumber = GetDbfNo(HeadNode.HalfKeysPerNode, n1);
           PushItem.Node = n2->NodeNo;
           
           start = HeadNode.HalfKeysPerNode + 1;
           end = pos;
       }
   }

   temp  = n1->offsets[end];
   for( i = end; i > start; i--)
   {
       n1->offsets[i] = n1->offsets[i-1];
   }
   n1->offsets[start] = temp;

   /* restore original key */
   memcpy( KeyBuf, oldPushItem.Key, HeadNode.KeyLen + 1);

   // Insert new key
   PutKeyData( pos, n1 );
   PutDbfNo  ( pos, n1, oldPushItem.RecordNumber);
   PutLeftNodeNo( pos, n1, GetLeftNodeNo (pos + 1, n1));
   PutLeftNodeNo( pos + 1  /* +1 ?*/, n1, oldPushItem.Node /* t */ );

   
   // Dup the node data
   memcpy(n2->Leaf.KeyRecs, n1->Leaf.KeyRecs, XB_NTX_NODE_SIZE);

   // Dup the offsets
   for ( i = 0; i < HeadNode.KeysPerNode +1; i++)
   {
       n2->offsets[i] = n1->offsets[i];
   }

   // Setup the second node
   for (j = 0, i = HeadNode.HalfKeysPerNode; i < HeadNode.KeysPerNode; i++, j++ )
   {
       temp = n2->offsets[j];
       n2->offsets[j] = n2->offsets[i];
       n2->offsets[i] = temp;
   }

   // Get the last offset for both nodes
   temp = n2->offsets[j];
   n2->offsets[j] = n2->offsets[HeadNode.KeysPerNode];
   n2->offsets[HeadNode.KeysPerNode] = temp;

      // Set the new count of both nodes
   n2->Leaf.NoOfKeysThisNode = HeadNode.HalfKeysPerNode;
   n1->Leaf.NoOfKeysThisNode = HeadNode.HalfKeysPerNode;

   
   if((rc = PutLeafNode( n1->NodeNo,n1 )) != 0) return rc;
   if((rc = PutLeafNode( n2->NodeNo,n2 )) != 0) return rc;
   return 0;
}
/************************************************************************/
xbShort xbNtx::CreateKey( xbShort RecBufSw, xbShort KeyBufSw )
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
   if( !TempNode ) xb_error( XB_INVALID_KEY );

   if( KeyBufSw )
   {
       memset( KeyBuf2, 0x00, HeadNode.KeyLen + 1 );
       memcpy( KeyBuf2, TempNode->Result, TempNode->DataLen );
   }
   else
   {
       memset( KeyBuf, 0x00, HeadNode.KeyLen + 1 );
       memcpy( KeyBuf, TempNode->Result, TempNode->DataLen );
   }
   if( !TempNode->InTree ) dbf->xbase->FreeExpNode( TempNode );
   return 0;
}
/************************************************************************/
xbShort  
xbNtx::GetCurrentKey(char *key)
{
  CreateKey(0, 0);
  memcpy(key, KeyBuf, HeadNode.KeyLen + 1);
    
  return 0;
}
/************************************************************************/
xbShort xbNtx::AddKey( xbLong DbfRec )
{
 /* This routine assumes KeyBuf contains the contents of the index to key */

   xbShort i,rc;
   xbNodeLink * TempNode;   
   xbNodeLink * Tparent;
   xbLong TempNodeNo = 0L;          /* new, unattached leaf node no */

   /* find node key belongs in */
   rc = FindKey( KeyBuf, HeadNode.KeyLen, 0 );
   if( rc == XB_FOUND && HeadNode.Unique )
       xb_error(XB_KEY_NOT_UNIQUE);

// 9/29/98
// these lines commented out - compatibity wins over efficiency for this library
//    /* 1.02 next statement moves to key match w/ space in nodes to reduce splits */
//    if( !HeadNode.Unique && rc == XB_FOUND &&
//         HeadNode.KeysPerNode == CurNode->Leaf.NoOfKeysThisNode )
//    {
//       if(( rc = CloneNodeChain()) != XB_NO_ERROR ) return rc;
//       if(( rc = GetNextKey( 0 )) != XB_NO_ERROR )
//          UncloneNodeChain();
//       while( HeadNode.KeysPerNode == CurNode->Leaf.NoOfKeysThisNode &&
//              rc == XB_NO_ERROR && 
//              (CompareKey( KeyBuf, GetKeyData( CurNode->CurKeyNo, CurNode ),
//                 HeadNode.KeyLen ) == 0 )
//            )
//          if(( rc = GetNextKey( 0 )) != XB_NO_ERROR )
//             UncloneNodeChain();
//    }

   /************************************************/
   /* section A - if room in node, add key to node */
   /************************************************/

   if( CurNode->Leaf.NoOfKeysThisNode < HeadNode.KeysPerNode )
   {
      if(( rc = PutKeyInNode( CurNode,CurNode->CurKeyNo,DbfRec,0L,1)) != 0)
         return rc;
      if(( rc = PutHeadNode( &HeadNode, indexfp, 1 )) != 0)
         return rc;
      return XB_NO_ERROR;
   }   

   /***********************************************************************/
   /* section B - split leaf node if full and put key in correct position */
   /***********************************************************************/

   TempNode = GetNodeMemory();
   // Create a new page
   TempNode->NodeNo = GetNextNodeNo();

   rc = SplitLeafNode( CurNode, TempNode, CurNode->CurKeyNo, DbfRec );
   if( rc ) return rc;

   /* TempNode is on disk, now we have to point someone above to
      that node. Keep the NodeNo of the on disk new node.
   */
   TempNodeNo = TempNode->NodeNo;
   ReleaseNodeMemory( TempNode );

   /*
     PushItem also contains the key to put into the parent
     PushItem should point at TempNode
    */
   PushItem.Node = TempNodeNo;

   /*****************************************************/
   /* section C go up tree splitting nodes as necessary */
   /*****************************************************/

   Tparent = CurNode->PrevNode;

   while( Tparent && 
          Tparent->Leaf.NoOfKeysThisNode >= HeadNode.KeysPerNode )
   {
      TempNode = GetNodeMemory();
      if( !TempNode )
#ifdef HAVE_EXCEPTIONS
          throw xbOutOfMemoryException();
#else
	   return XB_NO_MEMORY;
#endif


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
      if( !TempNode )
#ifdef HAVE_EXCEPTIONS
          throw xbOutOfMemoryException();
#else
	   return XB_NO_MEMORY;
#endif


      memcpy( KeyBuf, PushItem.Key, HeadNode.KeyLen );
      PutKeyData( 0, TempNode );
      
       PutDbfNo ( 0, TempNode, PushItem.RecordNumber );
       
      PutLeftNodeNo( 0, TempNode, CurNode->NodeNo );
      PutLeftNodeNo( 1, TempNode, PushItem.Node );

      TempNode->NodeNo = GetNextNodeNo();

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
   InsertKeyOffset(Tparent->CurKeyNo, Tparent);
   
   /* put key in parent */

   i = Tparent->CurKeyNo;
   memcpy( KeyBuf, PushItem.Key, HeadNode.KeyLen);
   PutKeyData( i, Tparent );

   PutDbfNo( i, Tparent, PushItem.RecordNumber);
   
   PutLeftNodeNo( i , Tparent, CurNode->NodeNo );
   PutLeftNodeNo( i + 1 , Tparent, TempNodeNo );

   Tparent->Leaf.NoOfKeysThisNode++;
   
   rc = PutLeafNode( Tparent->NodeNo, Tparent );
   if( rc ) return rc;
   rc = PutHeadNode( &HeadNode, indexfp, 1 );
   if( rc ) return rc;
 
   return XB_NO_ERROR;
}
/***********************************************************************/
xbShort xbNtx::UpdateParentKey( xbNodeLink * n )
{
/* this routine goes backwards thru the node chain looking for a parent
   node to update */

   xbNodeLink * TempNode;
   
   if( !n ) return XB_INVALID_NODELINK;
   if( !GetDbfNo( 0, n ))
   {
      cout << "Fatal index error - Not a leaf node" << n->NodeNo << "\n";
//      exit(0);
      return XB_NOT_LEAFNODE;
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
void xbNtx::UpdateDeleteList( xbNodeLink *n )
{
   n->NextNode = DeleteChain;
   DeleteChain = n;
}
/***********************************************************************/
/* Delete nodes from the node list - for now we leave the empty nodes  */
/* dangling in the file. Eventually we will remove nodes from the file */

void xbNtx::ProcessDeleteList( void )
{
   if( DeleteChain )
   {
      ReleaseNodeMemory( DeleteChain );
      DeleteChain = NULL;
   }
}
/***********************************************************************/
xbShort xbNtx::KeyWasChanged( void )
{
   CreateKey( 0, 0 );            /* use KeyBuf,  RecBuf    */
   CreateKey( 1, 1 );            /* use KeyBuf2, RecBuf2   */
   if( CompareKey( KeyBuf, KeyBuf2, HeadNode.KeyLen ) != 0 )
      return 1;
   else
      return 0;
}
// /***********************************************************************/
// xbNodeLink * xbNtx::LeftSiblingHasSpace( xbNodeLink * n )
// {
//    xbNodeLink * TempNode;
//    xbNodeLink * SaveCurNode;

//    /* returns a Nodelink to xbNodeLink n's left sibling if it has space */

//    /* if left most node in parent return NULL */
//    if( n->PrevNode->CurKeyNo == 0 )  
//       return NULL;

//    SaveCurNode = CurNode;
//    GetLeafNode( GetLeftNodeNo( n->PrevNode->CurKeyNo-1, n->PrevNode ), 2 ); 
//    if( CurNode->Leaf.NoOfKeysThisNode < HeadNode.KeysPerNode )
//    {
//       TempNode = CurNode;  
//       CurNode = SaveCurNode;
//       TempNode->PrevNode = n->PrevNode;
//       return TempNode;
//    }
//    else  /* node is already full */
//    {
//       ReleaseNodeMemory( CurNode );
//       CurNode = SaveCurNode;
//       return NULL;
//    }
//}
/***********************************************************************/
// xbNodeLink * xbNtx::RightSiblingHasSpace( xbNodeLink * n )
// {
//    /* returns a Nodelink to xbNodeLink n's right sibling if it has space */

//    xbNodeLink * TempNode;
//    xbNodeLink * SaveCurNode;

//    /* if left most node in parent return NULL */
//    if( n->PrevNode->CurKeyNo >= n->PrevNode->Leaf.NoOfKeysThisNode )  
//       return NULL;

//    SaveCurNode = CurNode;
//    /* point curnode to right sib*/
//    GetLeafNode( GetLeftNodeNo( n->PrevNode->CurKeyNo+1, n->PrevNode ), 2 ); 

//    if( CurNode->Leaf.NoOfKeysThisNode < HeadNode.KeysPerNode )
//    {
//       TempNode = CurNode;  
//       CurNode = SaveCurNode;
//       TempNode->PrevNode = n->PrevNode;
//       return TempNode;
//    }
//    else /* node is already full */
//    {
//       ReleaseNodeMemory( CurNode );
//       CurNode = SaveCurNode;
//       return NULL;
//    }
// }
// /***********************************************************************/
// xbShort xbNtx::MoveToRightNode( xbNodeLink * n, xbNodeLink * Right )
// {
//    xbShort j;
//    xbNodeLink * TempNode;
//    xbNodeLink * SaveCurNode;
//    xbNodeLink * SaveNodeChain;

//    if( n->CurKeyNo == 0 )
//    {
//       j = 1;
//       SaveNodeChain = NodeChain;
//       SaveCurNode = CurNode;
//       NodeChain = NULL;
//       GetLastKey( n->NodeNo, 0 ); 
//       memcpy( KeyBuf, GetKeyData( CurNode->CurKeyNo, CurNode),HeadNode.KeyLen);
//       ReleaseNodeMemory( NodeChain );
//       NodeChain = SaveNodeChain;
//       CurNode = SaveCurNode;
//    }
//    else
//    {
//       j = 0;
//       memcpy( KeyBuf, GetKeyData( j, n ), HeadNode.KeyLen);
//    }
//    PutKeyInNode( Right, 0, 0L, GetLeftNodeNo( j, n ), 1 );
//    ReleaseNodeMemory( Right );
//    TempNode = n;
//    CurNode = n->PrevNode;
//    n = n->PrevNode;
//    n->NextNode = NULL;
//    UpdateDeleteList( TempNode );
//    DeleteSibling( n );
//    return XB_NO_ERROR;
// }
// /***********************************************************************/
// xbShort xbNtx::MoveToLeftNode( xbNodeLink * n, xbNodeLink * Left )
// {
//    xbShort j, rc;
//    xbNodeLink * SaveNodeChain;
//    xbNodeLink * TempNode;

//    if( n->CurKeyNo == 0 )
//       j = 1;
//    else
//       j = 0;
   
//    /* save the original node chain */
//    SaveNodeChain = NodeChain;
//    NodeChain = NULL;

//    /* determine new right most key for left node */
//    GetLastKey( Left->NodeNo, 0 );
//    memcpy( KeyBuf, GetKeyData( CurNode->CurKeyNo, CurNode ), HeadNode.KeyLen);
//    ReleaseNodeMemory( NodeChain );
//    NodeChain = NULL;			/* for next GetLastKey */
//    PutKeyData( Left->Leaf.NoOfKeysThisNode, Left); 
//    PutLeftNodeNo( Left->Leaf.NoOfKeysThisNode+1, Left, GetLeftNodeNo( j,n ));
//    Left->Leaf.NoOfKeysThisNode++;
//    Left->CurKeyNo = Left->Leaf.NoOfKeysThisNode;
//    if(( rc = PutLeafNode( Left->NodeNo, Left )) != 0 ) return rc;

//    n->PrevNode->NextNode = NULL;
//    UpdateDeleteList( n );

//    /* get the new right most key for left to update parents */
//    GetLastKey( Left->NodeNo, 0 );
   
//    /* assemble the chain */
//    TempNode = Left->PrevNode;
//    TempNode->CurKeyNo--;
//    NodeChain->PrevNode = Left->PrevNode;
//    UpdateParentKey( CurNode );
//    ReleaseNodeMemory( NodeChain );
//    ReleaseNodeMemory( Left );
//    CurNode = TempNode;
//    NodeChain = SaveNodeChain;
//    TempNode->CurKeyNo++;    
//    DeleteSibling( TempNode );
//    return XB_NO_ERROR;
// }
// /***********************************************************************/
// xbShort xbNtx::DeleteSibling( xbNodeLink * n )
// {
//    xbNodeLink * Left;
//    xbNodeLink * Right;
//    xbNodeLink * SaveCurNode;
//    xbNodeLink * SaveNodeChain;
//    xbNodeLink * TempNode;
//    xbShort  rc;

//    /* this routine deletes sibling CurRecNo out of xbNodeLink n */
//    if( n->Leaf.NoOfKeysThisNode > 1 )
//    {  
//       RemoveKeyFromNode( n->CurKeyNo, n );
//       if( n->CurKeyNo == n->Leaf.NoOfKeysThisNode )
//       {
//          SaveNodeChain = NodeChain;
//          SaveCurNode = CurNode;
//          NodeChain = NULL;
//          GetLastKey( n->NodeNo, 0 );
//          /* assemble the node chain */
//          TempNode = NodeChain->NextNode;
//          NodeChain->NextNode = NULL;
//          ReleaseNodeMemory( NodeChain );
//          TempNode->PrevNode = n;
//          UpdateParentKey( CurNode );
//          /* take it back apart */
//          ReleaseNodeMemory( TempNode );
//          NodeChain = SaveNodeChain;
//          CurNode = SaveCurNode;
//       }
//    }
//    else if( n->NodeNo == HeadNode.StartNode )
//    {
//       /* get here if root node and only one child remains */
//       /* make remaining node the new root */
//       if( n->CurKeyNo == 0 )   
//          HeadNode.StartNode = GetLeftNodeNo( 1, n );
//       else
//          HeadNode.StartNode = GetLeftNodeNo( 0, n );
//       UpdateDeleteList( n );
//       NodeChain = NULL;
//       CurNode = NULL;
//    }
//    else if (( Left = LeftSiblingHasSpace( n )) != NULL )
//    {
//       return MoveToLeftNode( n, Left );
//    }
//    else if (( Right = RightSiblingHasSpace( n )) != NULL )
//    {
//       return MoveToRightNode( n, Right );
//    }
//    /* else if left sibling exists */   
//    else if( n->PrevNode->CurKeyNo > 0 )
//    {
//       /* move right branch from left sibling to this node */  
//       SaveCurNode = CurNode;
//       SaveNodeChain = NodeChain;
//       NodeChain = NULL;
//       GetLeafNode( GetLeftNodeNo( n->PrevNode->CurKeyNo-1, n->PrevNode ), 2 );
//       Left = CurNode;
//       Left->PrevNode = SaveCurNode->PrevNode;
//       GetLastKey( Left->NodeNo, 0 );
      
//       strncpy( KeyBuf, GetKeyData( CurNode->CurKeyNo,CurNode),HeadNode.KeyLen );
//       if( n->CurKeyNo == 1 )
//          PutLeftNodeNo( 1, n, GetLeftNodeNo( 0, n ));
//       PutKeyData( 0, n );
//       PutLeftNodeNo( 0, n, GetLeftNodeNo( Left->Leaf.NoOfKeysThisNode, Left ));
//       if(( rc = PutLeafNode( n->NodeNo, n )) != XB_NO_ERROR ) return rc;
//       SaveCurNode = n->PrevNode;
//       SaveCurNode->NextNode = NULL;
//       ReleaseNodeMemory( n );
//       Left->Leaf.NoOfKeysThisNode--;
//       if(( rc = PutLeafNode( Left->NodeNo, Left )) != XB_NO_ERROR ) return rc;
//       /* rebuild left side of tree */
//       GetLastKey( Left->NodeNo, 0 );
//       NodeChain->PrevNode = SaveCurNode;
//       SaveCurNode->CurKeyNo--;
//       UpdateParentKey( CurNode );
//       ReleaseNodeMemory( NodeChain );
//       ReleaseNodeMemory( Left );
//       CurNode = SaveCurNode;
//       NodeChain = SaveNodeChain;
//    }
//    /* right sibling must exist */
//    else if( n->PrevNode->CurKeyNo <= n->PrevNode->Leaf.NoOfKeysThisNode )
//    {
//       /* move left branch from left sibling to this node */   
//       SaveCurNode = CurNode;
//       SaveNodeChain = NodeChain;
//       NodeChain = NULL;

//       /* move the left node number one to the left if necessary */
//       if( n->CurKeyNo == 0 )
//       {
//          PutLeftNodeNo( 0, n, GetLeftNodeNo( 1, n ));
//          GetLastKey( GetLeftNodeNo( 0, n ), 0 );
//          memcpy(KeyBuf,GetKeyData(CurNode->CurKeyNo,CurNode),HeadNode.KeyLen);
//          PutKeyData( 0, n );
//          ReleaseNodeMemory( NodeChain );
//          NodeChain = NULL;
//       }
//       GetLeafNode( GetLeftNodeNo( n->PrevNode->CurKeyNo+1, n->PrevNode ), 2 );
      
//       /* put leftmost node number from right node in this node */
//       PutLeftNodeNo( 1, n, GetLeftNodeNo( 0, CurNode ));      
//       if(( rc = PutLeafNode( n->NodeNo, n )) != XB_NO_ERROR ) return rc;

//       /* remove the key from the right node */
//       RemoveKeyFromNode( 0, CurNode );
//       if(( rc = PutLeafNode( CurNode->NodeNo, CurNode )) != XB_NO_ERROR ) return rc;
//       ReleaseNodeMemory( CurNode );

//       /* update new parent key value */
//       GetLastKey( n->NodeNo, 0 );
//       NodeChain->PrevNode = n->PrevNode;
//       UpdateParentKey( CurNode );
//       ReleaseNodeMemory( NodeChain );
      
//       NodeChain = SaveNodeChain;
//       CurNode = SaveCurNode;
//    }
//    else
//    {
//       /* this should never be true-but could be if 100 byte limit is ignored*/
//       cout << "Fatal index error\n";
//       exit(0);
//    }
//    return XB_NO_ERROR;   
// }
/***********************************************************************/
xbShort xbNtx::DeleteKey( xbLong DbfRec )
{
/* this routine assumes the key to be deleted is in KeyBuf */

   xbShort rc;

   // FindKey will set CurNodeNo on evey page down to the
   // key being deleted. This is important. Plus we
   // need to be able to find the key to delete it.
   CurNode = NULL;
   if(( rc = FindKey( KeyBuf, DbfRec )) != XB_FOUND )
       return rc;

   // Then delete it
   rc = DeleteKeyFromNode( CurNode->CurKeyNo, CurNode );
   
   CurDbfRec = GetDbfNo( CurNode->CurKeyNo, CurNode );

   if(( rc = PutHeadNode( &HeadNode, indexfp, 1 )) != 0 )
      return rc;
   return XB_NO_ERROR;
}

xbShort
xbNtx::DeleteKeyFromNode(xbShort pos, xbNodeLink *n )
{
    xbNodeLink *TempNode;
    xbShort rc;
    
    // Check to see if this is an inode
   if (GetLeftNodeNo( 0 , n ) != 0)
   {
       // Copy the rightmost key from the left node.
       TempNode = n;
       GetLeafNode ( GetLeftNodeNo (n->CurKeyNo, n), 1);

       // Get the key Data
       strcpy (KeyBuf , GetKeyData( CurNode->Leaf.NoOfKeysThisNode -1, CurNode));
       PutKeyData( pos, TempNode );

       // Get the xbDbf no
       PutDbfNo (pos, TempNode, GetDbfNo( CurNode->Leaf.NoOfKeysThisNode -1, CurNode) );
       // We don't change the LeftNodeNo. determined later

       // Write the changed node
       PutLeafNode( TempNode->NodeNo, TempNode );

       // Now delete the key from the child
       TempNode = CurNode;

       if((rc = PutLeafNode( n->NodeNo,n )) != 0) return rc;       
       
       return DeleteKeyFromNode( TempNode->Leaf.NoOfKeysThisNode -1, TempNode);
   }
   else
   {
       return RemoveKeyFromNode(pos, n);
   }
   return XB_NO_ERROR;
}

xbShort xbNtx::RemoveKeyFromNode( xbShort pos, xbNodeLink *n )
{
    xbNodeLink *TempNode;
    xbNodeLink *sibling;
    xbNodeLink *parent;
    xbShort rc;
    xbLong newHeadNode = 0;
    bool harvest = false;
    // Here we are a leaf node..

    
    if ( n->NodeNo == HeadNode.StartNode && n->Leaf.NoOfKeysThisNode == 1)
    {
        // we are about to delete the last node from the head node.
        newHeadNode = GetLeftNodeNo( 0 , n );
    }

       // Remove the key from the current node.
    DeleteKeyOffset(pos, n);
    n->Leaf.NoOfKeysThisNode--;
   
    // Check to see if the number of keys left is less then
    // 1/2 KeysPerNode
    if ( ! ( n->NodeNo == HeadNode.StartNode )
         && n->Leaf.NoOfKeysThisNode < HeadNode.HalfKeysPerNode)
    {
        // This observed clipper behavior.
        // If less then 1/2 keys per node, then merge with right sibling.
        // If no right sibling, merge with left sibling.
        parent = n->PrevNode;
        // If the parents cur key is the last key, then take the left node
        if (parent->CurKeyNo == parent->Leaf.NoOfKeysThisNode)
        {
            TempNode = CurNode;
            GetLeafNode( GetLeftNodeNo(parent->CurKeyNo -1, parent), 2 );
            sibling = CurNode;
            CurNode = TempNode;

            JoinSiblings(parent, parent->CurKeyNo -1, sibling, n);
            
            // Harvest the empty node, if necessary
            // JoinSiblings will set NodeNo to -(NodeNo) to signal
            // that the node needs to be harvested. Clipper keeps the
            // old key count on the node, to we can't set it to 0
            if (sibling->NodeNo < 0)
            {
                sibling->NodeNo *= -1;
                harvest = true;
            }
            if((rc = PutLeafNode( n->NodeNo,n )) != 0) return rc;
            if((rc = PutLeafNode( sibling->NodeNo,sibling )) != 0) return rc;
            if((rc = PutLeafNode( parent->NodeNo,parent )) != 0) return rc;
            
            if (harvest)
            {

                HeadNode.UnusedOffset = n->NodeNo;
                // Save the empty xbNodeLink
//                ReleaseNodeMemory(n);
                // We may have to delete a node from the parent
                return RemoveKeyFromNode( parent->CurKeyNo, parent);
            }

        }
        else
        {
            // Take the right node
            TempNode = CurNode;
            GetLeafNode( GetLeftNodeNo(parent->CurKeyNo + 1, parent), 2 );
            sibling = CurNode;
            CurNode = TempNode;
            
            JoinSiblings(parent, parent->CurKeyNo, n, sibling);
            // Harvest the empty node, if necessary
            // JoinSiblings will set NodeNo to -(NodeNo) to signal
            // that the node needs to be harvested. Clipper keeps the
            // old key count on the node, to we can't set it to 0
            if (sibling->NodeNo < 0)
            {
                sibling->NodeNo *= -1;
                harvest = true;
            }
            if((rc = PutLeafNode( n->NodeNo,n )) != 0) return rc;
            if((rc = PutLeafNode( sibling->NodeNo,sibling )) != 0) return rc;
            if((rc = PutLeafNode( parent->NodeNo,parent )) != 0) return rc;
            
            if (harvest)
            {
                HeadNode.UnusedOffset = sibling->NodeNo;
                // Save the empty xbNodeLink
                ReleaseNodeMemory( sibling );

                // Now the parents->CurKeyNo+1 left pointer is empty, and
                // we are about to delete the parent. So move the left node no
                // from the parents->CurKeyNo+1 to the parent->CurNodeNo
                PutLeftNodeNo( parent->CurKeyNo +1 , parent,
                               GetLeftNodeNo( parent->CurKeyNo, parent ));
                // We may have to delete a node from the parent
                return RemoveKeyFromNode( parent->CurKeyNo, parent);  
            }
                        
        }

    }
    else
    {
        if ( n->NodeNo == HeadNode.StartNode && n->Leaf.NoOfKeysThisNode == 0 )
        {
            // we are about to delete the last node from the head node.
            HeadNode.UnusedOffset = HeadNode.StartNode;
            HeadNode.StartNode = newHeadNode;
        }

        
        if((rc = PutLeafNode( n->NodeNo,n )) != 0) return rc;
        // If more then 1/2 keys per node -> done.
        return XB_NO_ERROR;
    }
    return XB_NO_ERROR;
}

xbShort
xbNtx::JoinSiblings(xbNodeLink *parent, xbShort parentPos, xbNodeLink *n1, xbNodeLink* n2)
{
    // ASSUMES: keys in n1 are less then keys in n2
    //
    // Here, the contents of n1 need to be merged with n2. If n1 + parent_key
    // + n2 can all fit in n1, then leave n2 empty, and remove the key from the
    // parent.
    // Otherwise evenly distribute the keys from n1 and n2 over both, resetting
    // the parent.

    xbShort i, j;
//    xbLong rightPointerOffset;


    // if n1 has exactly (it will never have less) 1/2 keys per node
    // then put everything into n1.
    if ( (n1->Leaf.NoOfKeysThisNode + n2->Leaf.NoOfKeysThisNode + 1) <= HeadNode.KeysPerNode)
    {

        // Bring down the parent
        strcpy(KeyBuf, GetKeyData( parentPos, parent ));
        PutKeyData( n1->Leaf.NoOfKeysThisNode , n1);

        PutDbfNo ( n1->Leaf.NoOfKeysThisNode, n1, GetDbfNo( parentPos, parent ) );
        n1->Leaf.NoOfKeysThisNode++;

        
        // Copy over the rest of the keys
        for (i = n1->Leaf.NoOfKeysThisNode, j = 0; j < n2->Leaf.NoOfKeysThisNode; i++, j++)
        {
            strcpy(KeyBuf, GetKeyData( j, n2 ));
            PutKeyData( i, n1);

            PutLeftNodeNo( i, n1, GetLeftNodeNo( j, n2) );
            PutDbfNo ( i , n1, GetDbfNo( j, n2 ) );

        }
        n1->Leaf.NoOfKeysThisNode += j;

        // We need a way to signal that this node will be harvested.
        // Clipper keeps the KeyCount on harvested nodes, it does NOT
        // set them to 0.
        n2->NodeNo *= -1;
        return XB_NO_ERROR;
        
    }
    else
    {
        // Distribute the keys evenly. Of off by one, the extra
        // goes to n1.

        // If n1 contains the greater than keys, then at this point we
        // know that n1 has more than 1/2MKPN. therefore we copy
        // over untill we get to  median. All the while removing
        // keys from n2. Then 
        
        int totalKeys = n1->Leaf.NoOfKeysThisNode + n2->Leaf.NoOfKeysThisNode + 1;
        int median = (int) totalKeys/2;

        // If n1 has more keys then n2, then we need to copy the last keys
        // of n1 to the beginning of n2.
        // Leave HalfKeysPerNode+1 keys in n1, then the last key will
        // be copied up to the parent.
        if (n1->Leaf.NoOfKeysThisNode > HeadNode.HalfKeysPerNode)
        {
            // Bring down the parent
            InsertKeyOffset(0, n2);
            strcpy(KeyBuf, GetKeyData( parentPos, parent ));
            PutKeyData( 0 , n2);
            PutDbfNo ( 0, n2, GetDbfNo( parentPos, parent ) );
            n2->Leaf.NoOfKeysThisNode++;


            for (i = n1->Leaf.NoOfKeysThisNode -1; i > (median -1);  i--)
            {
                // Put the key in n2
                InsertKeyOffset(0, n2);
                strcpy(KeyBuf, GetKeyData( i, n1 ));
                PutKeyData( 0, n2);
                PutLeftNodeNo( 0, n2, GetLeftNodeNo( i, n1) );
                PutDbfNo ( 0 , n2, GetDbfNo( i, n1 ) );
            
                // Remove the key from the current node.
//                DeleteKeyOffset(i, n1);
                n1->Leaf.NoOfKeysThisNode--;
                n2->Leaf.NoOfKeysThisNode++;

            }
            // Copy up the last key from n1, that will become the new parent key.
            strcpy(KeyBuf, GetKeyData( n1->Leaf.NoOfKeysThisNode -1 , n1 ));
            PutKeyData( parentPos, parent);
            PutDbfNo ( parentPos , parent, GetDbfNo( n1->Leaf.NoOfKeysThisNode -1, n1) );
            n1->Leaf.NoOfKeysThisNode--;
        }
        else
        {
            // Bring down the parent
            strcpy(KeyBuf, GetKeyData( parentPos, parent ));
            PutKeyData( n1->Leaf.NoOfKeysThisNode , n1);
            PutDbfNo ( n1->Leaf.NoOfKeysThisNode, n1, GetDbfNo( parentPos, parent ) );
            n1->Leaf.NoOfKeysThisNode++;

            // Moving the median to the parent may have to occur
            // before moving the other keys to n1. This we would have
            // to calcualte the correct offset from the median

            // Copy up the first key from n2 (the median),
            // that will become the new parent key.
            strcpy(KeyBuf, GetKeyData( (n2->Leaf.NoOfKeysThisNode - median) -1, n2 ));
            PutKeyData( parentPos, parent);
            PutDbfNo ( parentPos , parent, GetDbfNo( (n2->Leaf.NoOfKeysThisNode - median) -1, n2 ) );

// Still investigating the -1 thing with clipper, If anyone has clues, 
// please let me know - bob@synxis.com            
//             if ( n1->Leaf.NoOfKeysThisNode  >=  (median - 1))
//             {
//                 // Clipper, don't know why
//                 PutLeftNodeNo(0, n2 , -1 );
//                 cout << "Clipper hack" << endl;
//             }

            DeleteKeyOffset(  (n2->Leaf.NoOfKeysThisNode - median) -1, n2);
            n2->Leaf.NoOfKeysThisNode--;


//            xbShort clipperMessedUpIndex = n1->Leaf.NoOfKeysThisNode;            
            for (i = n1->Leaf.NoOfKeysThisNode; i < median -1; i++)
            {

                cout << " i = " << i << endl;
                strcpy(KeyBuf, GetKeyData( 0, n2 ));
                PutKeyData( i, n1);
                cout << "KeyBuf = " << KeyBuf << endl;
                PutLeftNodeNo( i, n1, GetLeftNodeNo( 0, n2) );
                PutDbfNo ( i , n1, GetDbfNo( 0, n2 ) );

//                 if ( i == clipperMessedUpIndex)
//                 {
//                     // Clipper, don't know why
//                     PutLeftNodeNo(0, n2 , -1 );
//                     cout << "Clipper hack in loop i = " <<  i << endl;
//                 }
            
                // Remove the key from the current node.
                DeleteKeyOffset(0, n2);
                n2->Leaf.NoOfKeysThisNode--;
                n1->Leaf.NoOfKeysThisNode++;
            }

        }
    }
    
    return XB_NO_ERROR;
    
}



/************************************************************************/
#ifdef XBASE_DEBUG
xbShort xbNtx::CheckIndexIntegrity( const xbShort option )
{
   /* if option = 1, print out some stats */

   xbShort rc;
   xbLong ctr = 1L;

   if ( option ) cout << "Checking NTX " << IndexName << endl;
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

   if( option )
   {
       cout << "Exiting with rc = " << rc << "\n";
       cout << "\nTotal records checked = " << ctr << "\n";
   }

   return XB_NO_ERROR;
}
#endif
/***********************************************************************/
xbShort xbNtx::ReIndex(void (*statusFunc)(xbLong itemNum, xbLong numItems))
{
   /* this method assumes the index has been locked in exclusive mode */

   xbLong l;
   xbShort rc, i, NameLen, saveAutoLock;
   NtxHeadNode TempHead;
   FILE *t, *temp;
   xbString TempName;

   memcpy( &TempHead, &HeadNode, sizeof( struct NtxHeadNode ));

   TempHead.StartNode = 1024L;

   if(( rc = dbf->xbase->DirectoryExistsInName( IndexName )) > 0 )
       NameLen = rc + 13;
   else
       NameLen = 13;

   if (rc) {
       TempName.assign(IndexName, 0, rc);
       TempName += "TEMPFILE.NTX";
   } else
       TempName = "TEMPFILE.NTX";

 
   if(( t = fopen( TempName, "w+b" )) == NULL )
      return XB_OPEN_ERROR;
   
   if(( rc = PutHeadNode( &TempHead, t, 0 )) != 0 )
   {
      fclose( t );
      remove( TempName );
      return rc;
   }

   for( i = 0; i < XB_NTX_NODE_SIZE; i++ )
   {
      if(( fwrite( "\x00", 1, 1, t )) != 1 )
      {
         fclose( t );
         remove( TempName );
         xb_io_error( XB_WRITE_ERROR, TempName );
      }
   }

   temp = indexfp;
   indexfp = t;

   if ((rc = GetLeafNode(TempHead.StartNode, 1)) != 0)
       return rc;
   
   for (i = 0; i < TempHead.KeysPerNode; i++)
   {
       CurNode->offsets[i] = (i * HeadNode.KeySize) +
           2 + (2 * (HeadNode.KeysPerNode + 1));
   }

   
   if ((rc = PutLeafNode(TempHead.StartNode, CurNode )) != 0)
       return rc;

   indexfp = temp;


   if( fclose( indexfp ) != 0 )
       xb_io_error( XB_CLOSE_ERROR, IndexName);

   if( fclose( t ) != 0 )
       xb_io_error( XB_CLOSE_ERROR, TempName );

   if( remove( IndexName ) != 0 )
       xb_io_error( XB_CLOSE_ERROR, IndexName );
    
   if( rename( TempName, IndexName ) != 0 )
       xb_io_error( XB_WRITE_ERROR, IndexName );

   if(( indexfp = fopen( IndexName, "r+b" )) == NULL )
       xb_open_error( IndexName );

   saveAutoLock = dbf->GetAutoLock();
   dbf->AutoLockOff();

   for( l = 1; l <= dbf->NoOfRecords(); l++ )
   {
      if(statusFunc)
         statusFunc(l, dbf->NoOfRecords());
         
      if(( rc = dbf->GetRecord(l)) != XB_NO_ERROR )
         return rc;

      if(!dbf->GetRealDelete() || !dbf->RecordDeleted())
      {
         /* Create the key */
         CreateKey( 0, 0 );

         /* add key to index */
         if(( rc = AddKey( l )) != XB_NO_ERROR )
            return rc;
      }
   } 
   if(saveAutoLock)
      dbf->AutoLockOn();
   return XB_NO_ERROR;
}


xbULong
xbNtx::GetNextNodeNo()
{
    struct stat FileStat;
    int rc;
    xbULong FileSize;

    if(HeadNode.UnusedOffset != 0)
    {
        FileSize = HeadNode.UnusedOffset;
        HeadNode.UnusedOffset = 0;
        
        PutHeadNode(&HeadNode, indexfp, 1);
        
        return FileSize;
    }
    

    rc = fstat(fileno(indexfp), &FileStat);
    if (rc != 0)
    {
        return 0;
    }
    

    FileSize = (xbULong)FileStat.st_size;
    // File offset is zero based, so the file size will be the
    // offset of the next page.
    return FileSize;
}

void
xbNtx::GetExpression(char *buf, int len)
{
  memcpy(buf, HeadNode.KeyExpression, len < 256 ? len : 256);
}

#endif     /* XB_INDEX_NTX */
