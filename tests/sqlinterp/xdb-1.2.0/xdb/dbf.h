/*  $Id: dbf.h,v 1.14 1999/03/19 10:56:33 willy Exp $

    Xbase project source code

    This file contains the Class definition for a xbDBF object.

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

    V 1.0    10/10/97   - Initial release of software
    V 1.3    11/30/97   - Added memo field processing
    V 1.6a   4/1/98     - Added expression support
    V 1.6b   4/8/98     - Numeric index keys
    V 1.7.4d 10/28/98   - Added support for OS2/DOS/Win/NT locking
    V 1.8    11/29/98   - New class names and types 
*/


#ifndef __XB_DBF_H__
#define __XB_DBF_H__

#include <xdb/xbconfig.h>
#include <xdb/xtypes.h>
#include <xdb/xdate.h>

#include <iostream.h>
#include <stdio.h>

/*! \file dbf.h
*/

#if defined(XB_INDEX_ANY)
   class xbIndex;
   class xbNdx;
   class xbNtx;
#endif

/*****************************/
/* Field Types               */

#define XB_CHAR_FLD      'C'
#define XB_LOGICAL_FLD   'L'
#define XB_NUMERIC_FLD   'N'
#define XB_DATE_FLD      'D'
#define XB_MEMO_FLD      'M'
#define XB_FLOAT_FLD     'F'

/*****************************/
/* File Status Codes         */

#define XB_CLOSED  0
#define XB_OPEN    1
#define XB_UPDATED 2

/*****************************/
/* Other defines             */

#define XB_OVERLAY     1    
#define XB_DONTOVERLAY 0

/* This structure is used for defining a database */
struct xbSchema {
   char      FieldName[11];
   char      Type;
// xbUShort  FieldLen;       /* does not work */
// xbUShort  NoOfDecs;       /* does not work */
   unsigned  char FieldLen;  /* fields are stored as one byte on record*/
   unsigned  char NoOfDecs;
};

/* This structure defines field data as defined in an Xbase file header */
struct xbSchemaRec {
   char     FieldName[11];
   char     Type;            /* field type */
   char     *Address;        /* pointer to field in record buffer 1 */
// xbUShort FieldLen;        /* does not work */
// xbUShort NoOfDecs;        /* does not work */
   unsigned char FieldLen;   /* fields are stored as one byte on record */
   unsigned char NoOfDecs;
   char     *Address2;       /* pointer to field in record buffer 2 */
   char     *fp;             /* pointer to null terminated buffer for field */
                             /* see method GetString */
   xbShort  LongFieldLen;    /* to handle long field lengths */
};

struct xbIxList {
   xbIxList * NextIx;
   xbString IxName;
#if defined(XB_INDEX_ANY)
   xbIndex  * index;
   xbShort  Unique;
   xbShort  KeyUpdated;
#endif
};

#ifdef XB_MEMO_FIELDS
struct xbMH{                      /* memo header                    */
   xbLong  NextBlock;             /* pointer to next block to write */
   char    FileName[8];           /* name of dbt file               */
   char    Version;               /* not sure                       */
   xbShort BlockSize;             /* memo file block size           */
};
#endif

//! xbDbf class
/*!
  The xbDbf class encapsulates an xbase DBF database file.  It includes
  all dbf access, field access, and locking methods.
*/
    
class XBDLLEXPORT xbDbf {

public:
   xbDbf( xbXBase * );
   xbXBase  *xbase;               /* linkage to main base class */
   char EofChar[10];

/* datafile methods */
#if defined(XB_INDEX_ANY)
   xbShort   AddIndexToIxList(xbIndex *, const char *IndexName);
   xbShort   RemoveIndexFromIxList( xbIndex * );
#endif
   xbShort   AppendRecord( void );
   xbShort   BlankRecord( void );
   xbLong    CalcCheckSum( void );
   xbShort   CloseDatabase(bool deleteIndexes = 0);
   xbShort   CopyDbfStructure( const char *, xbShort );
   xbShort   CreateDatabase( const char * Name, xbSchema *, const xbShort Overlay );
   xbLong    DbfTell( void ) { return ftell( fp ); }
   xbShort   DeleteAllRecords( void ) { return DeleteAll(0); }
   xbShort   DeleteRecord( void );
#ifdef XBASE_DEBUG
   xbShort   DumpHeader( xbShort );
#endif
   xbShort   DumpRecord( xbULong );
   xbLong    FieldCount( void ) { return NoOfFields; }
   xbShort   GetDbfStatus( void ) { return DbfStatus; }
   xbShort   GetFirstRecord( void );
   xbShort   GetLastRecord( void );
   xbShort   GetNextRecord( void );
   xbShort   GetPrevRecord( void );
   xbLong    GetCurRecNo( void ) { return CurRec; }
   xbShort   GetRecord( xbULong );
   char *    GetRecordBuf( void ) { return RecBuf; }
   xbShort   GetRecordLen( void ) { return RecordLen; }
   xbShort   NameSuffixMissing( xbShort, const char * );
   xbLong    NoOfRecords( void );
   xbLong    PhysicalNoOfRecords(void);
   xbShort   OpenDatabase( const char * );
   xbShort   PackDatabase(xbShort LockWaitOption,
                          void (*packStatusFunc)(xbLong itemNum, xbLong numItems) = 0,
                          void (*indexStatusFunc)(xbLong itemNum, xbLong numItems) = 0);
   xbShort   PutRecord(void); // Put record to current position
   xbShort   PutRecord(xbULong);
   xbShort   RebuildAllIndices(void (*statusFunc)(xbLong itemNum, xbLong numItems) = 0);
   xbShort   RecordDeleted( void );
   void      ResetNoOfRecs( void ) { NoOfRecs = 0L; }
   xbShort   SetVersion( const xbShort );
   xbShort   UndeleteAllRecords( void ) { return DeleteAll(1); }
   xbShort   UndeleteRecord( void );
   xbShort   Zap( xbShort );

/* field methods */
   const char *GetField(xbShort FieldNo) const; // Using internal static buffer
   const char *GetField(const char *Name) const;
   xbShort   GetField(const xbShort FieldNo, char *Buf) const;
   xbShort   GetField(const xbShort FieldNo, char *Buf,
		      const xbShort RecBufSw) const;
   xbShort   GetField(const char *Name, char *Buf) const;
   xbShort   GetField(const char *Name, char *Buf,
		      const xbShort RecBufSw) const;
   xbShort   GetFieldDecimal( const xbShort );
   xbShort   GetFieldLen( const xbShort );
   char *    GetFieldName( const xbShort );
   xbShort   GetFieldNo( const char * FieldName ) const;
   char      GetFieldType( const xbShort FieldNo ) const;
   xbShort   GetLogicalField( const xbShort FieldNo );
   xbShort   GetLogicalField( const char * FieldName );

   char *    GetStringField( const xbShort FieldNo );
   char *    GetStringField( const char * FieldName );

   xbShort   PutField( const xbShort, const char * );
   xbShort   PutField( const char *Name, const char *buf);
   xbShort   ValidLogicalData( const char * );
   xbShort   ValidNumericData( const char * );

   xbLong    GetLongField( const char *FieldName) const;
   xbLong    GetLongField( const xbShort FieldNo) const;
   xbShort   PutLongField( const xbShort, const xbLong );
   xbShort   PutLongField( const char *, const xbLong);

   xbFloat   GetFloatField( const char * FieldName );
   xbFloat   GetFloatField( const xbShort FieldNo );
   xbShort   PutFloatField( const char *, const xbFloat);
   xbShort   PutFloatField( const xbShort, const xbFloat);

   xbDouble  GetDoubleField(const char *);
   xbDouble  GetDoubleField(const xbShort);
   xbShort   PutDoubleField(const char *, const xbDouble);
   xbShort   PutDoubleField(const xbShort, const xbDouble);

#ifdef XB_LOCKING_ON
   xbShort   LockDatabase( const xbShort, const xbShort, const xbULong );
   xbShort   ExclusiveLock( const xbShort );
   xbShort   ExclusiveUnlock( void );

#ifndef HAVE_FCNTL
   xbShort   UnixToDosLockCommand( const xbShort WaitOption,
             const xbShort LockType ) const;
#endif

#else
   xbShort   LockDatabase( const xbShort, const xbShort, const xbLong )
     { return XB_NO_ERROR; }
   xbShort   ExclusiveLock( const xbShort ) { return XB_NO_ERROR; };
   xbShort   ExclusiveUnlock( void )      { return XB_NO_ERROR; };
#endif

   void    AutoLockOn( void )  { AutoLock = 1; }
   void    AutoLockOff( void ) { AutoLock = 0; }
   xbShort GetAutoLock(void) { return AutoLock; }

#ifdef XB_MEMO_FIELDS
   xbShort   GetMemoField( const xbShort FieldNo,const xbLong len,
             char * Buf, const xbShort LockOption );
   xbLong    GetMemoFieldLen( const xbShort FieldNo );
   xbShort   UpdateMemoData( const xbShort FieldNo, const xbLong len,
			     const char * Buf, const xbShort LockOption );
   xbShort   MemoFieldExists( const xbShort FieldNo ) const;
   xbShort   LockMemoFile( const xbShort WaitOption, const xbShort LockType );
   xbShort   MemoFieldsPresent( void ) const;
   xbLong    CalcLastDataBlock();
   xbShort   FindBlockSetInChain( const xbLong BlocksNeeded, const xbLong
               LastDataBlock, xbLong & Location, xbLong &PreviousNode );
   xbShort   GetBlockSetFromChain( const xbLong BlocksNeeded, const xbLong
               Location, const xbLong PreviousNode );

#ifdef XBASE_DEBUG
   xbShort   DumpMemoFreeChain( void );
   void      DumpMemoHeader( void ) const;
   void      DumpMemoBlock( void ) const;
#endif
#endif

   void      RealDeleteOn(void) { RealDelete = 1; ReadHeader(1); }
   void      RealDeleteOff(void) { RealDelete = 0; ReadHeader(1); }
   xbShort   GetRealDelete(void) { return RealDelete; }

#if defined(XB_INDEX_ANY)
   xbShort   IndexCount(void);
   xbIndex   *GetIndex(xbShort indexNum);
#endif

private:
   xbString DatabaseName;
   xbShort  XFV;                  /* xBASE file version            */
   xbShort  NoOfFields;
   char   DbfStatus;              /* 0 = closed
                                     1 = open
                                     2 = updates pending           */
   FILE   *fp;                    /* file pointer                  */
   xbSchemaRec *SchemaPtr;        /* Pointer to field data         */
   char   *RecBuf;                /* Pointer to record buffer      */
   char   *RecBuf2;               /* Pointer to original rec buf   */

#ifdef XB_MEMO_FIELDS
   FILE    *mfp;                  /* memo file pointer             */
   void    *mbb;                  /* memo block buffer             */
   xbMH     MemoHeader;           /* memo header structure         */

   xbShort  mfield1;              /* memo block field one FF       */
   xbShort  MStartPos;            /* memo start pos of data        */
   xbLong   MFieldLen;            /* memo length of data           */
   xbLong   NextFreeBlock;        /* next free block in free chain */
   xbLong   FreeBlockCnt;         /* count of free blocks this set */

   xbLong   MNextBlockNo;         /* free block chain              */
   xbLong   MNoOfFreeBlocks;      /* free block chain              */

   xbLong   CurMemoBlockNo;       /* Current block no loaded       */
#endif

/* Next seven variables are read directly off the database header */
/* Don't change the order of the following seven items            */
   char   Version;
   char   UpdateYY;
   char   UpdateMM;
   char   UpdateDD;
//   xbLong   NoOfRecs;
//   xbShort  HeaderLen;
//   xbShort  RecordLen;

   xbULong  NoOfRecs;
   xbUShort HeaderLen;
   xbUShort RecordLen;

//#ifdef XB_REAL_DELETE
   xbULong  FirstFreeRec;
   xbULong  RealNumRecs;
//#endif

   xbIxList * MdxList;
   xbIxList * NdxList;
   xbIxList * FreeIxList;
   xbULong  CurRec;               /* Current record or zero   */
   xbShort  AutoLock;             /* Auto update option 0 = off  */

//#ifdef XB_REAL_DELETE
   xbShort  RealDelete;           /* real delete option 0 = off */
//#endif

#ifdef XB_LOCKING_ON
   xbShort CurLockType;           /* current type of file lock */
   xbShort CurLockCount;          /* number of current file locks */
   xbULong CurLockedRecNo;        /* currently locked record no */
   xbShort CurRecLockType;        /* current type of rec lock held (F_RDLOCK or F_WRLCK) */
   xbShort CurRecLockCount;       /* number of current record locks */
   xbShort CurMemoLockType;       /* current type of memo lock */
   xbShort CurMemoLockCount;      /* number of current memo locks */
#endif

   xbShort   DeleteAll( xbShort );
   void    InitVars( void );
   xbShort   PackDatafiles(void (*statusFunc)(xbLong itemNum, xbLong numItems) = 0);
   xbShort   ReadHeader( xbShort );
   xbShort   WriteHeader( const xbShort );

#ifdef XB_MEMO_FIELDS
   xbShort   AddMemoData( const xbShort FieldNo, const xbLong Len, const char * Buf );
   xbShort   CreateMemoFile( void );
   xbShort   DeleteMemoField( const xbShort FieldNo );
   xbShort   GetDbtHeader( const xbShort Option );
   xbShort   GetMemoBlockSize( void ) { return MemoHeader.BlockSize; }
   xbShort   OpenMemoFile( void );
   xbShort   PutMemoData( const xbLong StartBlock, const xbLong BlocksNeeded,
             const xbLong Len, const char * Buf );
   xbShort   ReadMemoBlock( const xbLong BlockNo, const xbShort Option);
   xbShort   SetMemoBlockSize( const xbShort );
   xbShort   UpdateHeadNextNode( void ) const;
   xbShort   WriteMemoBlock( const xbLong BlockNo, const xbShort Option );
   xbShort   IsType3Dbt( void ) const { return( Version==(char)0x83 ? 1:0 ); }
   xbShort   IsType4Dbt( void ) const
            {return (( Version==(char)0x8B || Version==(char)0x8E ) ? 1:0 );}
#endif
};
#endif		// __XB_DBF_H__

