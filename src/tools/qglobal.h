/****************************************************************************
** $Id: //depot/qt/main/src/tools/qglobal.h#171 $
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


#define QT_VERSION	300
#define QT_VERSION_STR	"3.0.0-snapshot"


//
// The operating system, must be one of: (Q_OS_x)
//
//   MACX	- Mac OS X
//   MAC9	- Mac OS 9
//   MSDOS	- MS-DOS and Windows
//   OS2	- OS/2
//   OS2EMX	- XFree86 on OS/2 (not PM)
//   WIN32	- Win32 (Windows 95/98 and Windows NT)
//   SUN	- SunOS
//   SOLARIS	- Sun Solaris
//   HPUX	- HP-UX
//   ULTRIX	- DEC Ultrix
//   LINUX	- Linux
//   FREEBSD	- FreeBSD
//   NETBSD	- NetBSD
//   OPENBSD    - OpenBSD
//   BSDI	- BSD/OS
//   IRIX	- SGI Irix
//   OSF	- Tru64 / Digital UNIX
//   UNIXWARE	- SCO UnixWare
//   SCO	- SCO of some sort
//   AIX	- AIX
//   GNU	- GNU Hurd
//   DGUX	- DG/UX
//   DYNIX	- DYNIX/ptx
//   RELIANT	- Reliant UNIX
//   QNX	- QNX
//   LYNXOS	- LynxOS
//   BSD4	- Any BSD 4.4 system
//   UNIX	- Any UNIX BSD/SYSV system
//
// The idea for configuring a UNIX platform is:
//
// 1) Import most recent Open Group interfaces available.
//    A. This is preferably done implictly on the following platforms
//       by defining
//       	_ALL_SOURCE   on AIX
//       	_GNU_SOURCE   on GNU systems
//       	_HPUX_SOURCE  on HP-UX
//       	_SGI_SOURCE   on Irix
//       This has the advantage that not only the latest Open Group
//    	 interfaces are imported, but also additional proprietary or
//       draft interfaces possibly used by Qt are imported as well.
//    B. This is done explicitly on systems such as Solaris by defining
//       	_XOPEN_SOURCE          to 600 for SUSv3
//       	_XOPEN_SOURCE          to 500 for SUSv2/XPG5
//       	_XOPEN_SOURCE_EXTENDED to 1   for SUS/XPG4v2
//       	_XOPEN_SOURCE          to 1   for plain XPG4
//       Be cautious however as specifying an Open Group standard might
//       cause proprietary or draft interfaces possibly used by Qt to be
//       left out on the basis that they are not part of the standard.
//       In such cases an additional macro is often available to import
//       additional interfaces as well. On Solaris use __EXTENSIONS__.
//
// 2) Draft POSIX and Open Group standards more recent than the current
//    Open Group standard must sometimes be specified separately. This
//    is the case for threads with POSIX semantics on Solaris and Large
//    File support on SUSv2/XPG5 platforms.
//    	_POSIX_C_SOURCE   to 199506L for IEEE Std 1003.1c (1995) / POSIX.1c
//                                   semantics on Solaris
//    	_FILE_OFFSET_BITS to 64      for Large File Support (draft 8)
//    	_POSIX_PII_SOCKET            for IEEE Std 1003.1g/D6.6 (March 1997)
//                                   sockets on otherwise XPG4v2 Tru64 4.0F
//    Some functions are not even in draft Open Group or POSIX standards.
//    However they are still made available either by including specific
//    platform-dependant header files (to be avoided) or by using
//    platform-dependant macros such as _BSD_SOURCE. Such exceptional things
//    should probably be defined near to the code that needs the specific
//    functions.
//
// 3) Only now may <unistd.h> be included, explicitly or not.
//    It will define some internal macros so that the system header files
//    that are included afterwards will import the correct interfaces.
//    You may also test for the availability of some specification.
//    Important examples are:
//    	_XOPEN_VERSION set to 500 for SUSv2/XPG5
//    	_XOPEN_UNIX    defined    for SUS/XPG4v2
//    	_XOPEN_XPG4    defined    for XPG4
//    	_XOPEN_XPG3    defined    for XPG3
//    	_POSIX_THREADS defined    for IEEE Std 1003.1c (1995) / POSIX.1c
//    Note that these macros never work as expected.  For example even
//    though recent releases of the GNU C library present SUSv2 (not to
//    say SUSv3) style interfaces, _XOPEN_VERSION is not defined to 500
//    unless you explicitly set _XOPEN_SOURCE to 500. The reason is that
//    _XOPEN_SOURCE should be defined to 500 only when the interfaces are
//    exact XPG5 interfaces - without extensions. On the other hand Irix
//    defines _XOPEN_UNIX although we do not explicitly specify XPG4v2 -
//    and then uses good ol' Berkeley-style sockets.
//    Do not blindly rely on these macros!
//
// This is mainly for SYSV platforms. BSD platforms are less of a problem,
// they have more stable interfaces.
//

#if defined(__APPLE__) && defined(__GNUC__)
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
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#  define Q_OS_WIN32
#elif defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#  define Q_OS_WIN32
#  define Q_OS_WIN64
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
#elif defined(reliantunix)
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
#  define Q_OS_LYNXOS
#elif defined(_UNIXWARE)
#  define Q_OS_UNIXWARE
#elif defined(__GNU__)
#  define Q_OS_GNU
#elif defined(DGUX)
#  define Q_OS_DGUX
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

#if defined(Q_OS_UNIX)
// BSDs have nice, stable interfaces...
#  if defined(Q_OS_BSD4)
// Do not specify Open Group standards on Irix! Only use the general
// _SGI_SOURCE macro which will automatically import the most recent
// Open Group interfaces automatically together with proprietary or
// draft interfaces not part of the current Open Group standard.
// Directly specifying Open Group standards will hide non-standard
// extension such as some DNS details we actually use in Qt.
// The MIPSpro and the GCC compilers define _SGI_SOURCE automatically
// so we don't redefine it unless it is hidden by a POSIX or Open Group
// standard request.
// See <standards.h> for more details.
#  elif defined(Q_OS_IRIX)
// On AIX use the general _ALL_SOURCE macro which will import the most
// recent interfaces automatically.
// The IBM and the GCC compilers define _ALL_SOURCE automatically
// so we don't redefine it unless it is hidden by a POSIX or Open Group
// standard request.
// See <standards.h> for more details.
#  elif defined(Q_OS_AIX)
// Use the general _GNU_SOURCE macro which will import the most recent
// which will automatically import the most recent Open Group interfaces
// together with  proprietary or draft interfaces not part of the current
// Open Group standard.  For example _GNU_SOURCE seems to be needed to
// import POSIX thread interfaces from the header files of pre-2.2 GNU C
// libraries.
// See <features.h> for more details.
#  elif defined(Q_OS_LINUX) || defined(Q_OS_GNU)
// Use the general _HPUX_SOURCE macro which will import the most recent
// which will automatically import the most recent Open Group interfaces
// together with  proprietary or draft interfaces not part of the current
// Open Group standard.  I'm not sure why defining _POSIX_C_SOURCE is
// needed in this context though.
// See stdsyms(5) and <sys/stdsyms.h> for more details.
#  elif defined(Q_OS_HPUX)
// There is an _OSF_SOURCE macro on Tru64, but it seems to behave somehow
// differently - possibly better?
// It does not import most recent Open Group interfaces.  Instead you must
// explicitly specify them using the relevant _XOPEN_SOURCE* macros, while
// _OSF_SOURCE imports additional proprietary or draft interfaces not part
// of the specified OpenGroup standard.  This is probably more flexible...
// See standards(5) and <standards.h> for more details.
#  elif defined(Q_OS_OSF)
/*
// ### Need to take a closer look at this issue...
#    if !defined(_OSF_SOURCE)
#      define _OSF_SOURCE
#    endif
*/
#    define _XOPEN_SOURCE 500          // import SUSv2/XPG5
#    define _XOPEN_SOURCE_EXTENDED 1   // fall back on SUS/XPG4v2
#    if defined(QT_THREAD_SUPPORT)
#      define _POSIX_C_SOURCE 199506L  // IEEE Std 1003.1c (1995) / POSIX.1c
#    endif
// There is no _*_SOURCE on Solaris, there's an __EXTENSIONS__ macro instead.
// This is very similar to Tru64.  This macro does not import most recent
// Open Group interfaces.  Instead you must explicitly specify them using the
// relevant _XOPEN_SOURCE* macros, while __EXTENSIONS__ imports additional
// proprietary or draft interfaces not part of the specified OpenGroup
// standard.
// Note that defining _POSIX_C_SOURCE to 199506L is mandatory to specify
// POSIX thread semantics as opposed to _REENTRANT which specifies Solaris
// thread semantics.
// Finally BSD_COMP must be defined for <sys/ioctl.h> to include <sys/filio.h>
// which in turn will define FIONREAD.
// See standards(5) and <sys/feature_tests.h> for more details.
// See also the note in <sys/ioctl.h> for more details on BSD_COMP.
#  elif defined(Q_OS_SOLARIS)
#    if !defined(BSD_COMP)
#      define BSD_COMP
#    endif
// BSD_COMP must be defined for <sys/ioctl.h> to define FIONREAD.
#  elif defined(Q_OS_UNIXWARE7)
#    if !defined(BSD_COMP)
#      define BSD_COMP
#    endif
// Don't know about other SYSV systems...
// Please send info on relevant macros to qt-bugs@trolltech.com.
// You could send the relevant header files, usually <unistd.h> and
// the first header file included by <unistd.h>.
#  endif
// Done with the general UNIX configuration. Now the details...
/*
// ### implement in 3.0
#  define _FILE_OFFSET_BITS 64   // X/Open Large File Support (draft 8)
*/
#endif


//
// The compiler, must be one of: (Q_CC_x)
//
//   SYM	- Symantec C++ for both PC and Macintosh
//   MPW	- MPW C++
//   MWERKS	- Metrowerks CodeWarrior
//   MSVC	- Microsoft Visual C/C++
//   BOR	- Borland/Turbo C++
//   WAT	- Watcom C++
//   GNU	- GNU C++
//   COMEAU	- Comeau C++
//   EDG	- Edison Design Group C++
//   OC		- CenterLine C++
//   SUN	- Sun C++
//   DEC	- DEC C++
//   HP		- HPUX C++
//   HPACC	- HPUX ANSI C++
//   USLC	- SCO UnixWare C++
//   CDS	- Reliant C++
//   KAI	- KAI C++
//   HIGHC	- MetaWare High C/C++
//

// Should be sorted most-authoritative to least-authoritative

#if defined(__SC__)
#  define Q_CC_SYM
#elif defined(applec)
#  define Q_CC_MPW
#  define Q_NO_BOOL_TYPE
#elif defined(__MWERKS__)
#  define Q_CC_MWERKS
#elif defined(_MSC_VER)
#  define Q_CC_MSVC
// proper support of bool for _MSC_VER >= 1100
#elif defined(__BORLANDC__) || defined(__TURBOC__)
#  define Q_CC_BOR
#  if __BORLANDC__ < 0x500
#    define Q_NO_BOOL_TYPE
#  endif
#elif defined(__WATCOMC__)
#  define Q_CC_WAT
#elif defined(__HIGHC__)
#  define Q_CC_HIGHC
#elif defined(__GNUC__)
#  define Q_CC_GNU
#  if __GNUC__ == 2 && __GNUC_MINOR__ <= 7
#    define Q_FULL_TEMPLATE_INSTANTIATION
#    define Q_SPURIOUS_NON_VOID_WARNING
#  endif
#  if __GNUC__ > 2 || __GNUC__ == 2 && __GNUC_MINOR__ >= 95
#    define Q_DELETING_VOID_UNDEFINED
#  endif
#  if (defined(__arm__) || defined(__ARMEL__)) && !defined(QT_MOC_CPP)
#    define Q_PACKED __attribute__ ((packed))
#  endif
#elif defined(__xlC__)
#  define Q_CC_XLC
#  define Q_FULL_TEMPLATE_INSTANTIATION
#  if __xlC__ < 0x400
#    define Q_NO_BOOL_TYPE
#  endif
#elif defined(__CDS__)
// does not seem to define __EDG__ or __EDG according to Reliant documentation
// but nevertheless uses EDG conventions like _BOOL
#  define Q_CC_EDG
#  define Q_CC_CDS
#elif defined(CENTERLINE_CLPP) || defined(OBJECTCENTER)
// don't know whether it defines __EDG__ or __EDG but an EDG compiler for sure
#  define Q_CC_EDG
#  define Q_CC_OC
#elif defined(__EDG) || defined(__EDG__) || defined(Q_CC_EDG)
// __EDG documented by SGI, observed on MIPSpro 7.3.1.1 and KAI C++ 4.0b
// __EDG__ documented in EDG online docs, observed on Compaq C++ V6.3-002
#  if !defined(Q_CC_EDG)
#    define Q_CC_EDG
#  endif
// the EDG documentation says that _BOOL is defined when the compiler has bool
// but Compaq seem to have disabled this, as observed on Compaq C++ V6.3-002.
#  if defined(__DECCXX)
#    define Q_CC_DEC
#    if __DECCXX_VER < 60060005
#      define Q_NO_BOOL_TYPE
#    endif
#  else
#    if !defined(_BOOL)
#      define Q_NO_BOOL_TYPE
#    endif
#    if defined(__COMO__)
#      define Q_CC_COMEAU
#      define Q_C_CALLBACKS
#    elif defined( __KCC )
#      define Q_CC_KAI
// the new UnixWare 7 compiler is based on EDG and defines __EDG__
#    elif defined(__USLC__)
#      define Q_CC_USLC
#    endif
#  endif
#elif defined(__USLC__)
// the older UnixWare compiler is not based on EDG
#  define Q_CC_USLC
#  define Q_NO_BOOL_TYPE
#elif defined(__SUNPRO_CC)
#  define Q_CC_SUN
#  if __SUNPRO_CC >= 0x500
     // 'bool' is enabled by default but it can still be disabled
     // using -features=nobool in which case _BOOL is not defined
     // which is the default in the 4.2 compatibility mode triggered
     // by -compat=4
#    if !defined(_BOOL)
#      define Q_NO_BOOL_TYPE
#    endif
#    define Q_C_CALLBACKS
#  else
     // 4.2 compiler or older
#    define Q_NO_BOOL_TYPE
#  endif
#elif defined(Q_OS_HPUX)
// __HP_aCC was not defined in first aCC releases
#  if defined(__HP_aCC) || __cplusplus >= 199707L
#    define Q_CC_HPACC
#  else
#    define Q_CC_HP
#    define Q_NO_BOOL_TYPE
#    define Q_FULL_TEMPLATE_INSTANTIATION
#  endif
#else
#  error "Qt has not been tested with this compiler - talk to qt-bugs@trolltech.com"
#endif

#ifndef Q_PACKED
#  define Q_PACKED
#endif


//
// The window system, must be one of: (Q_WS_x)
//
//   MACX	- Mac OS X
//   MAC9	- Mac OS 9
//   QWS	- Qt/Embedded
//   WIN32	- Windows
//   X11	- X Window System
//   PM		- unsupported
//   WIN16	- unsupported
//

#if defined( Q_OS_MACX )
#  define Q_WS_MACX
#  define QMAC_PASCAL
#elif defined( Q_OS_MAC9 )
#  define Q_WS_MAC9
#  define QMAC_PASCAL pascal
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

#if defined(Q_WS_WIN16) || defined(Q_WS_WIN32)
#  define Q_WS_WIN
#endif

#if defined(Q_WS_MAC9) || defined(Q_WS_MACX)
#  define Q_WS_MAC
#endif


//
// Some classes do not permit copies to be made of an object.
// These classes contains a private copy constructor and operator=
// to disable copying (the compiler gives an error message).
// Undefine Q_DISABLE_COPY to turn off this checking.
//

#define Q_DISABLE_COPY


//
// Useful type definitions for Qt
//

#if defined(Q_NO_BOOL_TYPE)
typedef int bool;
#endif

typedef unsigned char	uchar;
typedef unsigned short	ushort;
typedef unsigned	uint;
typedef unsigned long	ulong;
typedef char	       *pchar;
typedef uchar	       *puchar;
typedef const char     *pcchar;


//
// Constant bool values
//

#ifndef TRUE
const bool FALSE = 0;
const bool TRUE = !0;
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

#define QMAX(a,b)	((a) > (b) ? (a) : (b))
#define QMIN(a,b)	((a) < (b) ? (a) : (b))
#define QABS(a)		((a) >= 0  ? (a) : -(a))

inline int qRound( double d )
{
    return d > 0.0 ? int(d+0.5) : int(d-0.5);
}


//
// Size-dependent types (architechture-dependent byte order)
//

#if !defined(QT_CLEAN_NAMESPACE)
// source compatibility with Qt 1.x
typedef signed char		INT8;		// 8 bit signed
typedef unsigned char		UINT8;		// 8 bit unsigned
typedef short			INT16;		// 16 bit signed
typedef unsigned short		UINT16;		// 16 bit unsigned
typedef int			INT32;		// 32 bit signed
typedef unsigned int		UINT32;		// 32 bit unsigned
#endif

typedef signed char		Q_INT8;		// 8 bit signed
typedef unsigned char		Q_UINT8;	// 8 bit unsigned
typedef short			Q_INT16;	// 16 bit signed
typedef unsigned short		Q_UINT16;	// 16 bit unsigned
typedef int			Q_INT32;	// 32 bit signed
typedef unsigned int		Q_UINT32;	// 32 bit unsigned
#if defined(Q_OS_WIN64)
// LLP64 64-bit model on Windows
typedef	__int64			Q_LONG;		// word up to 64 bit signed
typedef unsigned __int64	Q_ULONG;	// word up to 64 bit unsigned
#else
// LP64 64-bit model on Linux
typedef long			Q_LONG;		
typedef unsigned long		Q_ULONG;
#endif

#if !defined(QT_CLEAN_NAMESPACE)
#define Q_INT64			Q_LONG
#define Q_UINT64		Q_ULONG
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

#ifndef QT_H
#include <qfeatures.h>
#endif // QT_H


//
// Create Qt DLL if QT_DLL is defined (Windows only)
//

#if defined(Q_OS_WIN32) || (Q_OS_WIN64)
#  if defined(QT_NODLL)
#    undef QT_MAKEDLL
#    undef QT_DLL
#  endif
#  ifdef QT_DLL
#    if defined(QT_MAKEDLL)	/* create a Qt DLL library */
#      undef QT_DLL
#      define Q_EXPORT  __declspec(dllexport)
#      define Q_TEMPLATEDLL
#      undef  Q_DISABLE_COPY	/* avoid unresolved externals */
#    endif
#  endif
#  if defined(QT_DLL)		/* use a Qt DLL library */
#    define Q_EXPORT  __declspec(dllimport)
#    define Q_TEMPLATEDLL
#    undef  Q_DISABLE_COPY	/* avoid unresolved externals */
#  endif
#else
#  undef QT_MAKEDLL		/* ignore these for other platforms */
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
#  define QT_CHECK_STATE			// check state of objects etc.
#  define QT_CHECK_RANGE			// check range of indexes etc.
#  define QT_CHECK_NULL				// check null pointers
#  define QT_CHECK_MATH				// check math functions
#endif

#if !defined(QT_NO_DEBUG) && !defined(QT_DEBUG)
#  define QT_DEBUG				// display debug messages
#  if !defined(QT_CLEAN_NAMESPACE)
// source compatibility with Qt 2.x
#    if !defined(NO_DEBUG) && !defined(DEBUG)
#      if !defined(Q_OS_MACX)			// clash with MacOS X headers
#        define DEBUG
#      endif
#    endif
#  endif
#endif


Q_EXPORT void qDebug( const char *, ... )	// print debug message
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void qWarning( const char *, ... )	// print warning message
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void qFatal( const char *, ... )	// print fatal message and exit
#if defined(Q_CC_GNU)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void qSystemWarning( const char *, int code = -1 );

#if !defined(QT_CLEAN_NAMESPACE)
// source compatibility with Qt 1.x

Q_EXPORT void debug( const char *, ... )	// print debug message
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void warning( const char *, ... )	// print warning message
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

Q_EXPORT void fatal( const char *, ... )	// print fatal message and exit
#if defined(Q_CC_GNU) && !defined(__INSURE__)
    __attribute__ ((format (printf, 1, 2)))
#endif
;

#endif // QT_CLEAN_NAMESPACE


#if !defined(Q_ASSERT)
#if defined(QT_CHECK_STATE)
#if defined(QT_FATAL_ASSERT)
#define Q_ASSERT(x)  if ( !(x) )\
	qFatal("ASSERT: \"%s\" in %s (%d)",#x,__FILE__,__LINE__)
#else
#define Q_ASSERT(x)  if ( !(x) )\
	qWarning("ASSERT: \"%s\" in %s (%d)",#x,__FILE__,__LINE__)
#endif
#else
#define Q_ASSERT(x)
#endif
#endif

#if !defined(QT_CLEAN_NAMESPACE)
// source compatibility with Qt 2.x
#  if !defined(ASSERT)
#    define ASSERT(x) Q_ASSERT(x)
#  endif
#endif // QT_CLEAN_NAMESPACE


Q_EXPORT bool qt_check_pointer( bool c, const char *, int );

#if defined(QT_CHECK_NULL)
#  define Q_CHECK_PTR(p) (qt_check_pointer((p)==0,__FILE__,__LINE__))
#else
#  define Q_CHECK_PTR(p)
#endif

#if !defined(QT_CLEAN_NAMESPACE)
// source compatibility with Qt 2.x
#  if !defined(ASSERT)
#    define CHECK_PTR(x) Q_CHECK_PTR(x)
#  endif
#endif // QT_CLEAN_NAMESPACE


enum QtMsgType { QtDebugMsg, QtWarningMsg, QtFatalMsg };

typedef void (*QtMsgHandler)(QtMsgType, const char *);
Q_EXPORT QtMsgHandler qInstallMsgHandler( QtMsgHandler );

#if !defined(QT_CLEAN_NAMESPACE)
// source compatibility with Qt 2.x
typedef QtMsgHandler msg_handler;
#endif

Q_EXPORT void qSuppressObsoleteWarnings( bool = TRUE );

Q_EXPORT void qObsolete( const char *obj, const char *oldfunc,
			 const char *newfunc );
Q_EXPORT void qObsolete( const char *obj, const char *oldfunc );
Q_EXPORT void qObsolete( const char *message );


#endif // QGLOBAL_H
