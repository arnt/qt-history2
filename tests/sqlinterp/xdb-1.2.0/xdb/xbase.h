/*  $Id: xbase.h,v 1.10 1999/03/19 10:56:34 willy Exp $

    Xbase project source code

    This file contains a header file for the xbXBase class, which is the
    base class for using the Xbase DBMS library.

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
    V 1.5    1/2/98     - Added memo field support
    V 1.6a   4/1/98     - Added expression support
    V 1.6b   4/8/98     - Numeric index keys
    V 1.8.0a 1/27/99    - Added DisplayError method
*/

#ifndef __XB_XBASE_H__
#define __XB_XBASE_H__

#include <xdb/xbconfig.h>

#include <string.h>

#if defined(__WIN32__)

#include "windows.h"

// ripped from wxWindows

// _declspec works in BC++ 5 and later, as well as VC++
#if defined(__VISUALC__) || defined(__BORLANDC__)
#  ifdef XBMAKINGDLL
#    define XBDLLEXPORT __declspec( dllexport )
#    define XBDLLEXPORT_DATA(type) __declspec( dllexport ) type
#    define XBDLLEXPORT_CTORFN
#  elif defined(XBUSINGDLL)
#    define XBDLLEXPORT __declspec( dllimport )
#    define XBDLLEXPORT_DATA(type) __declspec( dllimport ) type
#    define XBDLLEXPORT_CTORFN
#  else
#    define XBDLLEXPORT
#    define XBDLLEXPORT_DATA(type) type
#    define XBDLLEXPORT_CTORFN
#  endif

#elif defined(__GNUC__)

#  ifdef XBMAKINGDLL
#    define XBDLLEXPORT __declspec( dllexport )
#    define XBDLLEXPORT_DATA(type) __declspec( dllexport ) type
#    define XBDLLEXPORT_CTORFN
#  elif defined(XBUSINGDLL)
#    define XBDLLEXPORT __declspec( dllimport )
#    define XBDLLEXPORT_DATA(type) __declspec( dllimport ) type
#    define XBDLLEXPORT_CTORFN
#  else
#    define XBDLLEXPORT
#    define XBDLLEXPORT_DATA(type) type
#    define XBDLLEXPORT_CTORFN
#  endif

#else

#  define XBDLLEXPORT
#  define XBDLLEXPORT_DATA(type) type
#  define XBDLLEXPORT_CTORFN
#endif

#else // !Windows
#  define XBDLLEXPORT
#  define XBDLLEXPORT_DATA(type) type
#  define XBDLLEXPORT_CTORFN
#endif // Win/!Win

#include <xdb/xtypes.h>
#include <xdb/retcodes.h>
#include <xdb/xdate.h>

#include <xdb/xbstring.h>

#if defined(XB_EXPRESSIONS)
#include <xdb/exp.h>
#endif

class XBDLLEXPORT xbDbf;

struct XBDLLEXPORT xbDbList{
   xbDbList * NextDbf;
   char * DbfName;
   xbDbf  * dbf;
};

#if defined(XB_EXPRESSIONS)
class XBDLLEXPORT xbXBase : public xbExpn {
#else
class XBDLLEXPORT xbXBase : public xbDate {
#endif

   xbDbList * DbfList;
   xbDbList * FreeDbfList;
   xbShort EndianType;                     /* B = Big Endian, L = Little Endian */

public:
   ~xbXBase();
   xbXBase();
   xbShort  AddDbfToDbfList(xbDbf *d, const char *DatabaseName);
   xbDbf *  GetDbfPtr( const char *Name );
   xbShort  DirectoryExistsInName( const char *Name );
   xbShort  GetEndianType( void ) { return EndianType; }
   void     DisplayError( const xbShort ErrorCode ) const;
   static const char* GetErrorMessage( const xbShort ErrorNo );

   /* next 6 routines handle both big endian and little endian machines */
   xbDouble GetDouble( const char *p );
   xbLong   GetLong  ( const char *p );
   xbULong  GetULong ( const char *p );
   xbShort  GetShort ( const char *p );
   void   PutLong  ( char *p, const xbLong   l );
   void   PutShort ( char *p, const xbShort  s );
   void   PutULong ( char *p, const xbULong  l );
   void   PutUShort( char *p, const xbUShort s );
   void   PutDouble( char *p, const xbDouble d );

   xbShort  RemoveDbfFromDbfList( xbDbf * );
};

#include <xdb/dbf.h>

#if defined(XB_INDEX_ANY)
#include <xdb/index.h>
#endif

#ifdef XB_INDEX_NDX
#include <xdb/ndx.h>
#endif

#ifdef XB_INDEX_NTX
#include <xdb/ntx.h>
#endif

#ifdef XB_LOCKING_ON

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_LOCKING_H
#include <sys/locking.h>
#define F_SETLKW 0
#define F_SETLK  1
#define F_RDLCK  2
#define F_WRLCK  3
#define F_UNLCK  4
#endif

#else
enum { F_SETLKW = 0, F_WRLCK = 0 };
#endif

#ifdef XB_HTML
#include <xdb/html.h>
#endif

#endif		// __XB_XBASE_H__
