/****************************************************************************
** $Id: //depot/qt/main/src/tools/qglobal.h#9 $
**
** Global type declarations and definitions
**
** Author  : Haavard Nord
** Created : 920529
**
** Copyright (C) 1992-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGLOBAL_H
#define QGLOBAL_H


// Operating system, must be one of these: (as _OS_x_ ), * = auto detected
//
//   MAC    -	Macintosh (*)
//   MSDOS  -	MS-DOS and Windows (*)
//   OS2    -	OS/2 ver 2.0 or newer (*)
//   WINNT  -	Windows NT (*)
//   SUN    -	SunOS 4.X (*)
//   SOLARIS-	Sun Solaris (*)
//   HPUX   -	HP-UX (*)
//   ULTRIX -	DEC Ultrix (*)
//   LINUX  -	Linux (*)
//   UNIX   -	Any UNIX bsd/sysv system

#if defined(macintosh)
#define _OS_MAC_
#elif defined(__MSDOS__) || defined(_MSDOS) || defined(MSDOS)
#define _OS_MSDOS_
#elif defined(__OS2__)
#define _OS_OS2_
#elif defined(__NT__) || defined(_WIN32)
#define _OS_WINNT_
#elif defined(sun) || defined(__sun) || defined(__sun__)
#define _OS_SUN_
#if defined(solaris)
#define _OS_SOLARIS_
#endif
#elif defined(hpux) || defined(__hpux) || defined(__hpux__)
#define _OS_HPUX_
#elif defined(ultrix) || defined(__ultrix) || defined(__ultrix__)
#define _OS_ULTRIX_
#elif defined(linux)
#define _OS_LINUX_
#else
#define _OS_???_				/* Here: _OS_x_ */
#endif

#if defined(_OS_SUN_) || defined(_OS_SOLARIS_) || defined(_OS_HPUX_) || defined(_OS_ULTRIX_) || defined(_OS_LINUX_)
#if !defined(UNIX)
#define UNIX
#endif
#endif


// What compiler is used (AT&T version 2.1 or greater), * = auto detected
//
//   MPW    -	MPW C++ (* - if mac and not symantec c++)
//   SYM    -	Symantec C++ (*)
//   MSC    -	MSC C/C++ (*)
//   BOR    -	Borland/Turbo C++ (*)
//   ZOR    -	Zortech C++ (*)
//   SUN    -	Sun C++ (* - if no other compiler for Sun)
//   OC	    -	CenterLine ObjectCenter C++
//   GNU    -	GNU C++ (*)

#if defined(__SC__)
#define _CC_SYM_
#elif defined(applec)
#define _CC_MPW_
#elif defined(_MSC_VER)
#define _CC_MSC_
#elif defined(__TURBOC__)
#define _CC_BOR_
#elif defined(__ZTC__)
#define _CC_ZOR_
#elif defined(__GNUC__)
#define _CC_GNU_
#elif defined(OBJECTCENTER) || defined(CENTERLINE_CLPP)
#define _CC_OC_
#elif defined(_OS_SUN_)
#define _CC_SUN_
#else
#define _CC_???_				/* Here: _CC_x_ */
#endif


// Specify to use macro or template classes (if not set from cmd line)

#define USE_MACROCLASS				/* always use macro classes */
#define USE_TEMPLATECLASS			/* use template classes */


// Some compilers don't support templates

#if defined(_CC_MPW_) || (defined(_CC_MSC_) && _MSC_VER < 900) || defined(_CC_SUN_)
#define NO_TEMPLATECLASS
#endif

#if defined(NO_TEMPLATECLASS)
#undef	USE_TEMPLATECLASS			/* templates not wanted */
#endif


// Smart setting/checking of DEFAULT_ flag

#if !defined(DEFAULT_MACROCLASS) && !defined(DEFAULT_TEMPLATECLASS)
#define DEFAULT_MACROCLASS
#endif

#if !defined(USE_TEMPLATECLASS) && defined(DEFAULT_TEMPLATECLASS)
// #error Can't use templates as default when USE_TEMPLATECLASS is not defined!
#endif

#if defined(DEFAULT_MACROCLASS) && defined(DEFAULT_TEMPLATECLASS)
// #error Define DEFAULT_MACROCLASS or DEFAULT_TEMPLATECLASS, not both!
#endif


// Useful types

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 6)
#define bool		int			/* bool is built-in */
#else
typedef int		bool;
#endif
typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned	uint;
typedef unsigned long	ulong;
typedef char	       *pchar;
typedef uchar	       *puchar;
typedef const char     *pcchar;


// Constant bool values

#ifndef TRUE
const bool FALSE = 0;
const bool TRUE = !FALSE;
#endif


// Size-dependent types (architechture-dependent byte order)

typedef char		INT8;			// 8 bit signed
typedef unsigned char	UINT8;			// 8 bit unsigned
typedef short		INT16;			// 16 bit signed
typedef unsigned short	UINT16;			// 16 bit unsigned
typedef long		INT32;			// 32 bit signed
typedef unsigned long	UINT32;			// 32 bit unsigned


// Data stream functions is provided by many classes (defined in qdstream.h)

class QDataStream;


// System information

bool qSysInfo( int *wordSize, bool *bigEndian );


// Debugging and error handling

#if !defined(NO_CHECK)
#define CHECK_STATE				// check state of objects etc.
#define CHECK_RANGE				// check range of indexes etc.
#define CHECK_NULL				// check null pointers
#define CHECK_MATH				// check math functions
#endif

#if !defined(NO_DEBUG)
#define DEBUG					// display debug messages
#endif

#if !defined(CC_WARNINGS)
#define NO_WARNINGS				// comment to get cc warnings
#endif
#if defined(NO_WARNINGS)
#if defined(_CC_MSC_)
#pragma warning(disable: 4759)
#elif defined(_CC_BOR_)
#pragma warn -pia
#pragma warn -ccc
#pragma warn -rch
#pragma warn -inl
#endif
#endif // NO_WARNINGS

void warning( const char *, ... );		// print message
void fatal( const char *, ... );		// print message and exit

#define debug	   warning
#define location() warning("In file %s, line %d:",__FILE__,__LINE__)

#if defined(_CC_SUN_)
#define ASSERT(x)  if ( !(x) )\
	warning("ASSERT: \"%s\" in %s (%d)","x",__FILE__,__LINE__)
#else
#define ASSERT(x)  if ( !(x) )\
	warning("ASSERT: \"%s\" in %s (%d)",#x,__FILE__,__LINE__)
#endif

bool chk_pointer( bool c, const char *, int );	// fatal error if c is TRUE

#if defined(CHECK_NULL)
#define CHECK_PTR(p) (chk_pointer((p)==0,__FILE__,__LINE__))
#else
#define CHECK_PTR(p)
#endif

typedef void (*dbg_handler)(char *);
dbg_handler installDebugHandler( dbg_handler ); // install debug handler


#if defined(CHECK_MEMORY)
#include "qmemchk.h"
#endif


#endif // QGLOBAL_H
