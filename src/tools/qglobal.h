/****************************************************************************
** $Id$
**
** Global type declarations and definitions
**
** Created : 920529
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QGLOBAL_H
#define QGLOBAL_H

#define QT_VERSION       310
#define QT_VERSION_STR   "3.1.0-snapshot"


/*
   The operating system, must be one of: (Q_OS_x)

     MACX	  - Mac OS X
     MAC9	  - Mac OS 9
     MSDOS  - MS-DOS and Windows
     OS2	  - OS/2
     OS2EMX - XFree86 on OS/2 (not PM)
     WIN32  - Win32 (Windows 95/98/ME and Windows NT/2000)
     SUN	  - SunOS
     SOLARIS	 - Sun Solaris
     HPUX	  - HP-UX
     ULTRIX - DEC Ultrix
     LINUX  - Linux
     FREEBSD	 - FreeBSD
     NETBSD - NetBSD
     OPENBSD    - OpenBSD
     BSDI	  - BSD/OS
     IRIX	  - SGI Irix
     OSF	  - Compaq Tru64
     UNIXWARE	 - SCO UnixWare
     SCO	  - SCO OpenServer
     AIX	  - AIX
     HURD	  - GNU Hurd
     DGUX	  - DG/UX
     DYNIX  - DYNIX/ptx
     RELIANT	 - Reliant UNIX
     QNX	  - QNX
	QNX6   - QNX RTP 6.1
     LYNX	  - LynxOS
     BSD4	  - Any BSD 4.4 system
     UNIX	  - Any UNIX BSD/SYSV system
*/

#if defined(__APPLE__) && defined(__GNUC__)
#  define Q_OS_MACX
#elif defined(__MACOSX__)
#  define Q_OS_MACX
#elif defined(macintosh)
#  define Q_OS_MAC9
#elif defined(MSDOS) || defined(_MSDOS) || defined(__MSDOS__)
#  define Q_OS_MSDOS
#elif defined(OS2) || defined(_OS2) || defined(__OS2__)
#  if defined(__EMX__)
#    define Q_OS_OS2EMX
#  else
#    define Q_OS_OS2
#  endif
#elif defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#  define Q_OS_WIN32
#  define Q_OS_WIN64
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  define Q_OS_WIN32
#elif defined(__MWERKS__) && defined(__INTEL__)
#  define Q_OS_WIN32
#elif defined(sun) || defined(__sun) || defined(__sun__)
#  if defined(__SVR4)
#    define Q_OS_SOLARIS
#  else
#    define Q_OS_SUN
#    define Q_OS_BSD4
#  endif
#elif defined(hpux) || defined(__hpux) || defined(__hpux__)
#  define Q_OS_HPUX
#elif defined(ultrix) || defined(__ultrix) || defined(__ultrix__)
#  define Q_OS_ULTRIX
#elif defined(sinix)
#  define Q_OS_RELIANT
#elif defined(linux) || defined(__linux) || defined(__linux__)
#  define Q_OS_LINUX
#elif defined(__FreeBSD__)
#  define Q_OS_FREEBSD
#  define Q_OS_BSD4
#elif defined(__NetBSD__)
#  define Q_OS_NETBSD
#  define Q_OS_BSD4
#elif defined(__OpenBSD__)
#  define Q_OS_OPENBSD
#  define Q_OS_BSD4
#elif defined(bsdi) || defined(__bsdi__)
#  define Q_OS_BSDI
#  define Q_OS_BSD4
#elif defined(sgi) || defined(__sgi)
#  define Q_OS_IRIX
#elif defined(__osf__)
#  define Q_OS_OSF
#elif defined(_AIX)
#  define Q_OS_AIX
#elif defined(__Lynx__)
#  define Q_OS_LYNX
#elif defined(_UNIXWARE)
#  define Q_OS_UNIXWARE
#elif defined(__GNU__)
#  define Q_OS_HURD
#elif defined(DGUX)
#  define Q_OS_DGUX
#elif defined(__QNXNTO__)
#  define Q_OS_QNX6
#elif defined(__QNX__)
#  define Q_OS_QNX
#elif defined(_SCO_DS) || defined(M_UNIX) || defined(M_XENIX)
#  define Q_OS_SCO
#elif defined(sco) || defined(_UNIXWARE7)
#  define Q_OS_UNIXWARE7
#elif !defined(_SCO_DS) && defined(__USLC__) && defined(__SCO_VERSION__)
#  define Q_OS_UNIXWARE7
#elif defined(_SEQUENT_)
#  define Q_OS_DYNIX
#elif defined(__svr4__)
/*   generic fallback for the rest of svr4 systems, e.g. g++ on UnixWare7.*/
#  define Q_OS_SVR4
#else
#  error "Qt has not been ported to this OS - talk to qt-bugs@trolltech.com"
#endif

#if defined(Q_OS_MAC9) || defined(Q_OS_MACX)
#  define Q_OS_MAC
#endif

#if defined(Q_OS_MAC9) || defined(Q_OS_MSDOS) || defined(Q_OS_OS2) || defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#  undef Q_OS_UNIX
#elif !defined(Q_OS_UNIX)
#  define Q_OS_UNIX
#endif


/*
   The compiler, must be one of: (Q_CC_x)

     SYM	  - Symantec C++ for both PC and Macintosh
     MPW	  - MPW C++
     MWERKS - Metrowerks CodeWarrior
     MSVC	  - Microsoft Visual C/C++
     BOR	  - Borland/Turbo C++
     WAT	  - Watcom C++
     GNU	  - GNU C++
     COMEAU - Comeau C++
     EDG	  - Edison Design Group C++
     OC	  - CenterLine C++
     SUN	  - Sun C++
     MIPS	  - MIPSpro C++
     DEC	  - DEC C++
     HP	  - HPUX C++
     HPACC  - HPUX ANSI C++
     USLC	  - SCO UnixWare C++
     CDS	  - Reliant C++
     KAI	  - KAI C++
     HIGHC  - MetaWare High C/C++
     INTEL  - Intel C++

   Should be sorted most-authoritative to least-authoritative
*/

/* Symantec C++ is now Digital Mars? */
#if defined(__DMC__) || defined(__SC__) || defined(__ZTC__)
#  define Q_CC_SYM
/* "explicit" semantics implemented in 8.1e but keyword recognized since 7.5 */
#  if defined(__SC__) && __SC__ < 0x750
#    define Q_NO_EXPLICIT_KEYWORD
#  endif

#elif defined(applec)
#  define Q_CC_MPW
#  define Q_NO_BOOL_TYPE
#  define Q_NO_EXPLICIT_KEYWORD

#elif defined(__MWERKS__)
#  define Q_CC_MWERKS
/* "explicit" recognized since 4.0d1 */
#  define QMAC_PASCAL pascal

#elif defined(_MSC_VER)
#  define Q_CC_MSVC
/* proper support of bool for _MSC_VER >= 1100 */
#  define Q_CANNOT_DELETE_CONSTANT

#elif defined(__BORLANDC__) || defined(__TURBOC__)
#  define Q_CC_BOR
#  if __BORLANDC__ < 0x500
#    define Q_NO_BOOL_TYPE
#    define Q_NO_EXPLICIT_KEYWORD
#  endif

#elif defined(__WATCOMC__)
#  define Q_CC_WAT

/* Never tested! */
#elif defined(__HIGHC__)
#  define Q_CC_HIGHC

#elif defined(__GNUC__)
#  define Q_CC_GNU
#  define Q_C_CALLBACKS
#  if __GNUC__ == 2 && __GNUC_MINOR__ <= 7
#    define Q_FULL_TEMPLATE_INSTANTIATION
#  endif
#  if (defined(__arm__) || defined(__ARMEL__)) && !defined(QT_MOC_CPP)
#    define Q_PACKED __attribute__ ((packed))
#  endif

/* IBM compiler versions are a bit messy. There are actually two products:
   the C product, and the C++ product. The C++ compiler is always packaged
   with the latest version of the C compiler. Version numbers do not always
   match. This little table (I'm not sure it's accurate) should be helpful:

	   C++ product               C product

	    C Set 3.1              C Compiler 3.0
		 ...                       ...
	C++ Compiler 3.6.6         C Compiler 4.3
		 ...                       ...
	Visual Age C++ 4.0              ...
		 ...                       ...
	Visual Age C++ 5.0         C Compiler 5.0

   Now:
   __xlC__    is the version of the C compiler in hexadecimal notation
              - it's only an approximation of the C++ compiler version
   __IBMCPP__ is the version of the C++ compiler in decimal notation
              - but it's not defined on older compilers like C Set 3.1 */
#elif defined(__xlC__)
#  define Q_CC_XLC
#  define Q_FULL_TEMPLATE_INSTANTIATION
#  if __xlC__ < 0x400
#    define Q_NO_BOOL_TYPE
#    define Q_NO_EXPLICIT_KEYWORD
#    define Q_BROKEN_TEMPLATE_SPECIALIZATION
#    define Q_CANNOT_DELETE_CONSTANT
#  endif

/* Compilers with EDG front end are similar. To detect them we test:
   __EDG documented by SGI, observed on MIPSpro 7.3.1.1 and KAI C++ 4.0b
   __EDG__ documented in EDG online docs, observed on Compaq C++ V6.3-002 */
#elif defined(__EDG) || defined(__EDG__) || defined(Q_CC_EDG)
#  if !defined(Q_CC_EDG)
#    define Q_CC_EDG
#  endif
/* Compaq have disabled EDG's _BOOL macro - observed on Compaq C++ V6.3-002. */
#  if defined(__DECCXX)
#    define Q_CC_DEC
/* Compaq use _BOOL_EXISTS instead of _BOOL. */
#    if defined(_BOOL_EXISTS)
/* Well, at least macro _BOOL_EXISTS is documented for Compaq C++ V6.3.
   In any case versions prior to Compaq C++ V6.0-005 do not have bool. */
#    elif __DECCXX_VER < 60060005
#      define Q_NO_BOOL_TYPE
#    endif
/* Apart from Compaq, from the EDG documentation:
   _BOOL
    Defined in C++ mode when bool is a keyword. The name of this predefined
    macro is specified by a configuration flag. _BOOL is the default.
   __BOOL_DEFINED
    Defined in Microsoft C++ mode when bool is a keyword. */
#  else
#    if !defined(_BOOL) && !defined(__BOOL_DEFINED)
#      define Q_NO_BOOL_TYPE
#    endif
#    if defined(__COMO__)
#      define Q_CC_COMEAU
#      define Q_C_CALLBACKS
#    elif defined(__KCC)
#      define Q_CC_KAI
#    elif defined(__INTEL_COMPILER)
#      define Q_CC_INTEL
/* The new UnixWare 7 compiler is based on EDG and does define __EDG__ */
#    elif defined(__USLC__)
#      define Q_CC_EDG
#      define Q_CC_USLC
/* Never tested! */
#    elif defined(CENTERLINE_CLPP) || defined(OBJECTCENTER)
#      define Q_CC_OC
/* The MIPSpro compiler in o32 mode is based on EDG but disables features
   such as template specialization nevertheless */
#    elif defined(sgi) || defined(__sgi)
#      define Q_CC_MIPS
#      if defined(_MIPS_SIM) && ( _MIPS_SIM == _ABIO32) /* o32 ABI */
#        define Q_BROKEN_TEMPLATE_SPECIALIZATION
#      endif
#    endif
#  endif

/* the older UnixWare compiler is not based on EDG */
#elif defined(__USLC__)
#  define Q_CC_USLC
#  define Q_NO_BOOL_TYPE
#  define Q_NO_EXPLICIT_KEYWORD

#elif defined(__SUNPRO_CC)
#  define Q_CC_SUN
/* 5.0 compiler or better
    'bool' is enabled by default but can be disabled using -features=nobool
    in which case _BOOL is not defined
        this is the default in 4.2 compatibility mode triggered by -compat=4 */
#  if __SUNPRO_CC >= 0x500
#    if !defined(_BOOL)
#      define Q_NO_BOOL_TYPE
#    endif
#    define Q_C_CALLBACKS
/* 4.2 compiler or older */
#  else
#    define Q_NO_BOOL_TYPE
#    define Q_NO_EXPLICIT_KEYWORD
#  endif

/* CDS++ does not seem to define __EDG__ or __EDG according to Reliant
   documentation but nevertheless uses EDG conventions like _BOOL */
#elif defined(sinix)
#  define Q_CC_EDG
#  define Q_CC_CDS
#  if !defined(_BOOL)
#    define Q_NO_BOOL_TYPE
#  endif

#elif defined(Q_OS_HPUX)
/* __HP_aCC was not defined in first aCC releases */
#  if defined(__HP_aCC) || __cplusplus >= 199707L
#    define Q_CC_HPACC
#  else
#    define Q_CC_HP
/* bool is a reserved keyword in later versions */
#    define bool int
#    define Q_FULL_TEMPLATE_INSTANTIATION
#    define Q_BROKEN_TEMPLATE_SPECIALIZATION
#    define Q_NO_EXPLICIT_KEYWORD
#  endif

#else
#  error "Qt has not been tested with this compiler - talk to qt-bugs@trolltech.com"
#endif

#ifndef Q_PACKED
#  define Q_PACKED
#endif


/*
   The window system, must be one of: (Q_WS_x)

     MACX	  - Mac OS X
     MAC9	  - Mac OS 9
     QWS	  - Qt/Embedded
     WIN32  - Windows
     X11	  - X Window System
     PM	  - unsupported
     WIN16  - unsupported
*/

#if defined( Q_OS_MACX )
#  define Q_WS_MACX
#elif defined( Q_OS_MAC9 )
#  define Q_WS_MAC9
#elif defined(Q_OS_MSDOS)
#  define Q_WS_WIN16
#  error "Qt requires Win32 and does not work with Windows 3.x"
#elif defined(_WIN32_X11_)
#  define Q_WS_X11
#elif defined(Q_OS_WIN32)
#  define Q_WS_WIN32
#  if defined(Q_OS_WIN64)
#    define Q_WS_WIN64
#  endif
#elif defined(Q_OS_OS2)
#  define Q_WS_PM
#  error "Qt does not work with OS/2 Presentation Manager or Workplace Shell"
#elif defined(Q_OS_UNIX)
#  ifdef QWS
#    define Q_WS_QWS
#  else
#    define Q_WS_X11
#  endif
#endif
#if defined(Q_OS_MAC) && !defined(QMAC_PASCAL)
#  define QMAC_PASCAL
#endif

#if defined(Q_WS_WIN16) || defined(Q_WS_WIN32)
#  define Q_WS_WIN
#endif

#if defined(Q_WS_MAC9) || defined(Q_WS_MACX)
#  define Q_WS_MAC
#endif


/*
   Some classes do not permit copies to be made of an object.
   These classes contains a private copy constructor and operator=
   to disable copying (the compiler gives an error message).
   Undefine Q_DISABLE_COPY to turn off this checking.
*/

#define Q_DISABLE_COPY

#if defined(__cplusplus)

//
// Useful type definitions for Qt
//

#if defined(Q_NO_BOOL_TYPE)
typedef int bool;
#endif

typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned	uint;
typedef unsigned long   ulong;
typedef char		   *pchar;
typedef uchar		   *puchar;
typedef const char     *pcchar;


//
// Constant bool values
//

#ifndef TRUE
const bool FALSE = 0;
const bool TRUE = !0;
#endif


//
// Use the "explicit" keyword on platforms that support it.
//

#if !defined(Q_NO_EXPLICIT_KEYWORD)
#  define Q_EXPLICIT explicit
#else
#  define Q_EXPLICIT
#endif


//
// Workaround for static const members on MSVC++.
//

#if defined(Q_CC_MSVC)
#  define QT_STATIC_CONST static
#  define QT_STATIC_CONST_IMPL
#else
#  define QT_STATIC_CONST static const
#  define QT_STATIC_CONST_IMPL const
#endif


//
// Utility macros and inline functions
//

#define QMAX(a, b)	((b) < (a) ? (a) : (b))
#define QMIN(a, b)	((a) < (b) ? (a) : (b))
#define QABS(a)	((a) >= 0  ? (a) : -(a))

inline int qRound( double d )
{
    return int( d >= 0.0 ? d + 0.5 : d - 0.5 );
}


//
// Size-dependent types (architechture-dependent byte order)
//

#if !defined(QT_CLEAN_NAMESPACE)
// source compatibility with Qt 1.x
typedef signed char	    INT8;		 // 8 bit signed
typedef unsigned char	   UINT8;		// 8 bit unsigned
typedef short		    INT16;	 // 16 bit signed
typedef unsigned short	   UINT16;	// 16 bit unsigned
typedef int		INT32;	  // 32 bit signed
typedef unsigned int	   UINT32;	// 32 bit unsigned
#endif

typedef signed char	    Q_INT8;	 // 8 bit signed
typedef unsigned char	   Q_UINT8;	// 8 bit unsigned
typedef short		    Q_INT16;	 // 16 bit signed
typedef unsigned short	   Q_UINT16;	// 16 bit unsigned
typedef int		Q_INT32;	  // 32 bit signed
typedef unsigned int	   Q_UINT32;	// 32 bit unsigned
#if defined(Q_OS_WIN64)
// LLP64 64-bit model on Windows
typedef __int64	    Q_LONG;	 // word up to 64 bit signed
typedef unsigned __int64	   Q_ULONG;	// word up to 64 bit unsigned
#else
// LP64 64-bit model on Linux
typedef long		    Q_LONG;
typedef unsigned long	   Q_ULONG;
#endif

#if !defined(QT_CLEAN_NAMESPACE)
#define Q_INT64	    Q_LONG
#define Q_UINT64	    Q_ULONG
#endif


//
// Data stream functions is provided by many classes (defined in qdatastream.h)
//

class QDataStream;


//
// Some platform specific stuff
//

#ifdef Q_WS_WIN
extern bool qt_winunicode;
#endif

//
// Feature subsetting
//
// Note that disabling some features will produce a libqt that is not
// compatible with other libqt builds. Such modifications are only
// supported on Qt/Embedded where reducing the library size is important
// and where the application-suite is often a fixed set.
//
#if !defined(QT_MOC)
#if defined(QCONFIG_LOCAL)
#include <qconfig-local.h>
#elif defined(QCONFIG_MINIMAL)
#include <qconfig-minimal.h>
#elif defined(QCONFIG_SMALL)
#include <qconfig-small.h>
#elif defined(QCONFIG_MEDIUM)
#include <qconfig-medium.h>
#elif defined(QCONFIG_LARGE)
#include <qconfig-large.h>
#else // everything...
#include <qconfig.h>
#endif
#endif

// prune to local config
#include "qmodules.h"
#ifndef QT_MODULE_ICONVIEW
# define QT_NO_ICONVIEW
#endif
#ifndef QT_MODULE_WORKSPACE
# define QT_NO_WORKSPACE
#endif
#ifndef QT_MODULE_NETWORK
#define QT_NO_NETWORK
#endif
#ifndef QT_MODULE_CANVAS
# define QT_NO_CANVAS
#endif
#ifndef QT_MODULE_TABLE
#define QT_NO_TABLE
#endif
#ifndef QT_MODULE_XML
# define QT_NO_XML
#endif
#ifndef QT_MODULE_OPENGL
# define QT_NO_OPENGL
#endif
#if !defined(QT_MODULE_SQL)
# define QT_NO_SQL
#endif

#ifdef Q_WS_MAC9
//No need for menu merging
#  ifndef QMAC_QMENUBAR_NO_MERGE
#    define QMAC_QMENUBAR_NO_MERGE
#  endif
//Mac9 does not use quartz
#  ifndef QMAC_NO_QUARTZ
#    define QMAC_NO_QUARTZ
#  endif
#endif
#ifdef Q_WS_MACX //for no nobody uses quartz, just putting in first level hooks
#  ifndef QMAC_NO_QUARTZ
#    define QMAC_NO_QUARTZ
#  endif
#endif

#ifndef QT_H
#include <qfeatures.h>
#endif // QT_H


//
// Create Qt DLL if QT_DLL is defined (Windows only)
//

#if defined(Q_OS_WIN32) || defined(Q_OS_WIN64)
#  if defined(QT_NODLL)
#    undef QT_MAKEDLL
#    undef QT_DLL
#  elif defined(QT_MAKEDLL) /* create a Qt DLL library */
#    if defined(QT_DLL)
#      undef QT_DLL
#    endif
#    define Q_EXPORT  __declspec(dllexport)
#    define Q_TEMPLATEDLL
#    undef  Q_DISABLE_COPY  /* avoid unresolved externals */
#  elif defined(QT_DLL)	   /* use a Qt DLL library */
#    define Q_EXPORT  __declspec(dllimport)
#    define Q_TEMPLATEDLL
#    undef  Q_DISABLE_COPY  /* avoid unresolved externals */
#  endif
#else
#  undef QT_MAKEDLL	    /* ignore these for other platforms */
#  undef QT_DLL
#endif

#ifndef Q_EXPORT
#  define Q_EXPORT
#endif


//
// System information
//

Q_EXPORT const char *qVersion();
Q_EXPORT bool qSysInfo( int *wordSize, bool *bigEndian );
#if defined(Q_WS_WIN)
Q_EXPORT int qWinVersion();
#endif


//
// Avoid some particularly useless warnings from some stupid compilers.
// To get ALL C++ compiler warnings, define QT_CC_WARNINGS or comment out
// the line "#define QT_NO_WARNINGS"
//

#if !defined(QT_CC_WARNINGS)
#  define QT_NO_WARNINGS
#endif
#if defined(QT_NO_WARNINGS)
#  if defined(Q_CC_MSVC)
#    pragma warning(disable: 4244) // 'conversion' conversion from 'type1' to 'type2', possible loss of data
#    pragma warning(disable: 4275) // non - DLL-interface classkey 'identifier' used as base for DLL-interface classkey 'identifier'
#    pragma warning(disable: 4514) // unreferenced inline/local function has been removed
#    pragma warning(disable: 4800) // 'type' : forcing value to bool 'true' or 'false' (performance warning)
#    pragma warning(disable: 4097) // typedef-name 'identifier1' used as synonym for class-name 'identifier2'
#    pragma warning(disable: 4706) // assignment within conditional expression
#    pragma warning(disable: 4786) // truncating debug info after 255 characters
#    pragma warning(disable: 4660) // template-class specialization 'identifier' is already instantiated
#    pragma warning(disable: 4355) // 'this' : used in base member initializer list
#  elif defined(Q_CC_BOR)
#    pragma option -w-inl
#    pragma option -w-aus
#    pragma warn -inl
#    pragma warn -pia
#    pragma warn -ccc
#    pragma warn -rch
#    pragma warn -sig
#  elif defined(Q_CC_MWERKS)
#    pragma warn_possunwant off
#  endif
#endif


//
// Use to avoid "unused parameter" warnings
//

#define Q_UNUSED(x) (void)x;


//
// Debugging and error handling
//

#if !defined(QT_NO_CHECK)
#  define QT_CHECK_STATE			 // check state of objects etc.
#  define QT_CHECK_RANGE			 // check range of indexes etc.
#  define QT_CHECK_NULL			 // check null pointers
#  define QT_CHECK_MATH			 // check math functions
#endif

#if !defined(QT_NO_DEBUG) && !defined(QT_DEBUG)
#  define QT_DEBUG			  // display debug messages
#  if !defined(QT_NO_COMPAT)
// source compatibility with Qt 2.x
#    if !defined(NO_DEBUG) && !defined(DEBUG)
#      if !defined(Q_OS_MACX)			// clash with MacOS X headers
#        define DEBUG
#      endif
#    endif
#  endif
#endif


Q_EXPORT void qDebug( const char *, ... )   // print debug message
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void qWarning( const char *, ... ) // print warning message
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void qFatal( const char *, ... )   // print fatal message and exit
#if defined(Q_CC_GNU)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void qSystemWarning( const char *, int code = -1 );

#if !defined(QT_CLEAN_NAMESPACE)
// source compatibility with Qt 1.x

Q_EXPORT void debug( const char *, ... )    // print debug message
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void warning( const char *, ... )  // print warning message
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void fatal( const char *, ... )    // print fatal message and exit
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

#endif // QT_CLEAN_NAMESPACE


#if !defined(Q_ASSERT)
#if defined(QT_CHECK_STATE)
#if defined(QT_FATAL_ASSERT)
#define Q_ASSERT(x)  ((x) ? (void)0 : qFatal("ASSERT: \"%s\" in %s (%d)",#x,__FILE__,__LINE__))
#else
#define Q_ASSERT(x)  ((x) ? (void)0 : qWarning("ASSERT: \"%s\" in %s (%d)",#x,__FILE__,__LINE__))
#endif
#else
#define Q_ASSERT(x)
#endif
#endif

#if !defined(QT_NO_COMPAT)
// source compatibility with Qt 2.x
#ifndef Q_OS_TEMP
#  if !defined(ASSERT)
#    define ASSERT(x) Q_ASSERT(x)
#  endif
#endif // Q_OS_TEMP
#endif // QT_NO_COMPAT


Q_EXPORT bool qt_check_pointer( bool c, const char *, int );

#if defined(QT_CHECK_NULL)
#  define Q_CHECK_PTR(p) (qt_check_pointer((p)==0,__FILE__,__LINE__))
#else
#  define Q_CHECK_PTR(p)
#endif

#if !defined(QT_NO_COMPAT)
// source compatibility with Qt 2.x
#  if !defined(CHECK_PTR)
#    define CHECK_PTR(x) Q_CHECK_PTR(x)
#  endif
#endif // QT_NO_COMPAT


enum QtMsgType { QtDebugMsg, QtWarningMsg, QtFatalMsg };

typedef void (*QtMsgHandler)(QtMsgType, const char *);
Q_EXPORT QtMsgHandler qInstallMsgHandler( QtMsgHandler );

#if !defined(QT_NO_COMPAT)
// source compatibility with Qt 2.x
typedef QtMsgHandler msg_handler;
#endif

Q_EXPORT void qSuppressObsoleteWarnings( bool = TRUE );

Q_EXPORT void qObsolete( const char *obj, const char *oldfunc,
		   const char *newfunc );
Q_EXPORT void qObsolete( const char *obj, const char *oldfunc );
Q_EXPORT void qObsolete( const char *message );

#endif // __cplusplus

#endif // QGLOBAL_H
