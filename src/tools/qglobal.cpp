/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"
#include <qstring.h>
#include <qhash.h>

#include <limits.h>
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>

#if defined(Q_CC_MSVC) && !defined(Q_OS_TEMP)
#include <crtdbg.h>
#endif


/*!
    \relates QApplication

    Returns the Qt version number as a string, for example, "2.3.0" or
    "3.0.5".

    The \c QT_VERSION define has the numeric value in the form:
    0xmmiibb (m = major, i = minor, b = bugfix). For example, Qt
    3.0.5's \c QT_VERSION is 0x030005.
*/

const char *qVersion()
{
    return QT_VERSION_STR;
}

bool qSharedBuild()
{
#ifdef QT_SHARED
    return true;
#else
    return false;
#endif
}

/*****************************************************************************
  System detection routines
 *****************************************************************************/

static bool si_alreadyDone = false;
static int  si_wordSize;
static bool si_bigEndian;

/*!
    \relates QApplication

    Obtains information about the system.

    The system's word size in bits (typically 32) is returned in \a
    *wordSize. The \a *bigEndian is set to true if this is a big-endian
    machine, or to false if this is a little-endian machine.

    In debug mode, this function calls qFatal() with a message if the
    computer is truly weird (i.e. different endianness for 16 bit and
    32 bit integers); in release mode it returns false.
*/

bool qSysInfo( int *wordSize, bool *bigEndian )
{
    Q_ASSERT( wordSize != 0 );
    Q_ASSERT( bigEndian != 0 );

    if ( si_alreadyDone ) {			// run it only once
	*wordSize  = si_wordSize;
	*bigEndian = si_bigEndian;
	return true;
    }

    si_wordSize = 0;
    Q_ULONG n = (Q_ULONG)(~0);
    while ( n ) {				// detect word size
	si_wordSize++;
	n /= 2;
    }
    *wordSize = si_wordSize;

    if ( *wordSize != 64 &&
	 *wordSize != 32 &&
	 *wordSize != 16 ) {			// word size: 16, 32 or 64
	qFatal( "qSysInfo: Unsupported system word size %d", *wordSize );
	return false;
    }
    if ( sizeof(Q_INT8) != 1 || sizeof(Q_INT16) != 2 || sizeof(Q_INT32) != 4 ||
	 sizeof(Q_ULONG)*8 != si_wordSize || sizeof(float) != 4 || sizeof(double) != 8 ) {
	qFatal( "qSysInfo: Unsupported system data type size" );
	return false;
    }

    bool  be16, be32;				// determine byte ordering
    short ns = 0x1234;
    int	  nl = 0x12345678;

    unsigned char *p = (unsigned char *)(&ns);	// 16-bit integer
    be16 = *p == 0x12;

    p = (unsigned char *)(&nl);			// 32-bit integer
    if ( p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78 )
	be32 = true;
    else
    if ( p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12 )
	be32 = false;
    else
	be32 = !be16;

    if ( be16 != be32 ) {			// strange machine!
	qFatal( "qSysInfo: Inconsistent system byte order" );
	return false;
    }

    *bigEndian = si_bigEndian = be32;
    si_alreadyDone = true;
    return true;
}

#if !defined(QWS) && defined(Q_OS_MAC)

#include "qt_mac.h"

int qMacVersion()
{
    static int macver = Qt::MV_Unknown;
    static bool first = true;
    if(first) {
	first = false;
	long gestalt_version;
	if(Gestalt(gestaltSystemVersion, &gestalt_version) == noErr) {
	    if(gestalt_version >= 0x1030 && gestalt_version < 0x1040)
		macver = Qt::MV_10_DOT_3;
	    else if(gestalt_version >= 0x1020 && gestalt_version < 0x1030)
		macver = Qt::MV_10_DOT_2;
	    else if(gestalt_version >= 0x1010 && gestalt_version < 0x1020)
		macver = Qt::MV_10_DOT_1;
	    else if(gestalt_version >= 0x1000 && gestalt_version < 0x1010)
		macver = Qt::MV_10_DOT_0;

	}
    }
    return macver;
}
Qt::MacintoshVersion qt_macver = (Qt::MacintoshVersion)qMacVersion();
#elif defined(Q_OS_WIN32) || defined(Q_OS_CYGWIN) || defined(Q_OS_TEMP)
bool qt_winunicode;
bool qt_winUnicode() { return qt_winunicode; }
# ifdef Q_OS_TEMP
  DWORD qt_cever = 0;
# endif // Q_OS_TEMP

#include "qt_windows.h"

int qWinVersion()
{
#ifndef VER_PLATFORM_WIN32s
#define VER_PLATFORM_WIN32s	    0
#endif
#ifndef VER_PLATFORM_WIN32_WINDOWS
#define VER_PLATFORM_WIN32_WINDOWS  1
#endif
#ifndef VER_PLATFORM_WIN32_NT
#define VER_PLATFORM_WIN32_NT	    2
#endif
#ifndef VER_PLATFORM_WIN32_CE
#define VER_PLATFORM_WIN32_CE	    3
#endif

    static int winver = Qt::WV_NT;
    static int t=0;
    if ( !t ) {
	t=1;
#ifndef Q_OS_TEMP
	OSVERSIONINFOA osver;
	osver.dwOSVersionInfoSize = sizeof(osver);
	GetVersionExA( &osver );
#else
	OSVERSIONINFOW osver;
	osver.dwOSVersionInfoSize = sizeof(osver);
	GetVersionEx( &osver );
	qt_cever = osver.dwMajorVersion * 100;
	qt_cever += osver.dwMinorVersion * 10;
#endif
	switch ( osver.dwPlatformId ) {
	case VER_PLATFORM_WIN32s:
	    winver = Qt::WV_32s;
	    break;
	case VER_PLATFORM_WIN32_WINDOWS:
	    // We treat Windows Me (minor 90) the same as Windows 98
	    if ( ( osver.dwMinorVersion == 10 ) || ( osver.dwMinorVersion == 90 ) )
		winver = Qt::WV_98;
	    else
		winver = Qt::WV_95;
	    break;
#ifdef Q_OS_TEMP
	case VER_PLATFORM_WIN32_CE:
#ifdef Q_OS_TEMP

	    if ( qt_cever >= 400 )
		winver = Qt::WV_CENET;
	    else
#endif
		winver = Qt::WV_CE;
	    break;
#endif
	default: // VER_PLATFORM_WIN32_NT
	    if ( osver.dwMajorVersion < 5 ) {
		winver = Qt::WV_NT;
	    } else if ( osver.dwMinorVersion == 0 ) {
		winver = Qt::WV_2000;
	    } else {
		winver = Qt::WV_XP;
	    }
	}
    }

#if defined(UNICODE)
    if ( winver & Qt::WV_NT_based )
	qt_winunicode = true;
    else
#endif
	qt_winunicode = false;

    return winver;
}

Qt::WindowsVersion qt_winver = (Qt::WindowsVersion)qWinVersion();
#endif


/*****************************************************************************
  Debug output routines
 *****************************************************************************/

/*!
    \fn void qDebug( const char *msg, ... )

    \relates QApplication

    Calls the message handler with the debug message \a msg.  If no
    message handler has been installed, the message is printed to
    stderr. Under Windows, the message is sent to the debugger.  This
    function does nothing if \c QT_NO_DEBUG was defined during
    compilation.

    This function takes a format string and a list of arguments,
    similar to the C printf() function.

    Example:
    \code
	qDebug( "my window handle = %x", myWidget->id() );
    \endcode

    \warning The internal buffer is limited to 8196 bytes (including
    the '\0'-terminator).

    \warning Passing (const char *)0 as argument to qDebug might lead
    to crashes on certain platforms due to the platforms printf implementation.

    \sa qWarning(), qFatal(), qInstallMsgHandler(),
	\link debug.html Debugging\endlink
*/

/*!
    \fn void qWarning( const char *msg, ... )

    \relates QApplication

    Calls the message handler with the warning message \a msg. If no
    message handler has been installed, the message is printed to
    stderr. Under Windows, the message is sent to the debugger.  This
    function does nothing if \c QT_NO_DEBUG was defined during
    compilation; it exits if the environment variable \c
    QT_FATAL_WARNINGS is defined.

    This function takes a format string and a list of arguments,
    similar to the C printf() function.

    Example:
    \code
	void f( int c )
	{
	    if ( c > 200 )
		qWarning( "f: bad argument, c == %d", c );
	}
    \endcode

    \warning The internal buffer is limited to 8196 bytes (including
    the '\0'-terminator).

    \warning Passing (const char *)0 as argument to qWarning might lead
    to crashes on certain platforms due to the platforms printf implementation.

    \sa qDebug(), qFatal(), qSystemWarning(), qInstallMsgHandler(),
    \link debug.html Debugging\endlink
*/


static QtMsgHandler handler = 0;		// pointer to debug handler
static const int QT_BUFFER_LENGTH = 8196;	// internal buffer length


#ifdef Q_OS_MAC
// This function has descended from Apple Source Code (FSpLocationFromFullPath),
// but changes have been made. [Creates a minimal alias from the full pathname]
OSErr qt_mac_create_fsspec(const QString &file, FSSpec *spec)
{
    FSRef fref;
    QByteArray utfs = file.utf8();
    OSErr ret = FSPathMakeRef((const UInt8 *)utfs.data(), &fref, NULL);
    if(ret == noErr)
	ret = FSGetCatalogInfo(&fref, kFSCatInfoNone, NULL, NULL, spec, NULL);
    return ret;
}


CFStringRef qstring2cfstring(const QString &str)
{
    return CFStringCreateWithCharacters(0, (UniChar *)str.unicode(), str.length());
}

QString cfstring2qstring(CFStringRef str)
{
    if(!str)
	return QString();

    CFIndex length = CFStringGetLength(str);
    if(const UniChar *chars = CFStringGetCharactersPtr(str))
	return QString((QChar *)chars, length);
    UniChar *buffer = (UniChar*)malloc(length * sizeof(UniChar));
    CFStringGetCharacters(str, CFRangeMake(0, length), buffer);
    QString ret((QChar *)buffer, length);
    free(buffer);
    return ret;
}

unsigned char * p_str(const char * c, int len=-1)
{
    const int maxlen = 255;
    if(len == -1)
	len = qstrlen(c);
    if(len > maxlen) {
	qWarning( "p_str len must never exceed %d", maxlen );
	len = maxlen;
    }
    unsigned char *ret = (unsigned char*)malloc(len+2);
    *ret=len;
    memcpy(((char *)ret)+1,c,len);
    *(ret+len+1) = '\0';
    return ret;
}

unsigned char * p_str(const QString &s)
{
    return p_str(s, s.length());
}

QByteArray p2qstring(const unsigned char *c) {
       char *arr = (char *)malloc(c[0] + 1);
       memcpy(arr, c+1, c[0]);
       arr[c[0]] = '\0';
       QByteArray ret( arr );
       delete arr;
       return ret;
}
#endif


#ifdef Q_CC_MWERKS

#include "qt_mac.h"

extern bool qt_is_gui_used;
static void mac_default_handler( const char *msg )
{
    if ( qt_is_gui_used ) {
	const char *p = p_str(msg);
	DebugStr(p);
	free(p);
    } else {
	fprintf( stderr, msg );
    }
}

#endif



/*!
    \fn void qFatal( const char *msg, ... )

    \relates QApplication

    Prints a fatal error message \a msg and exits, or calls the
    message handler (if it has been installed).

    This function takes a format string and a list of arguments,
    similar to the C printf() function.

    Example:
    \code
	int divide( int a, int b )
	{
	    if ( b == 0 )				// program error
		qFatal( "divide: cannot divide by zero" );
	    return a/b;
	}
    \endcode

    Under X11, the text is printed to stderr. Under Windows, the text
    is sent to the debugger.

    \warning The internal buffer is limited to 8196 bytes (including
    the '\0'-terminator).

    \warning Passing (const char *)0 as argument to qFatal might lead
    to crashes on certain platforms due to the platforms printf implementation.

    \sa qDebug(), qWarning(), qInstallMsgHandler(),
    \link debug.html Debugging\endlink
*/
void qFatal( const char *msg, ... )
{
    char buf[QT_BUFFER_LENGTH];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
#if defined(QT_VSNPRINTF)
	QT_VSNPRINTF( buf, QT_BUFFER_LENGTH, msg, ap );
#else
	vsprintf( buf, msg, ap );
#endif
	va_end( ap );
	(*handler)( QtFatalMsg, buf );
    } else {
	vsprintf( buf, msg, ap );		// ### is there no vsnprintf()?
	va_end( ap );
#if defined(Q_CC_MWERKS)
        mac_default_handler(buf);
#else
	fprintf( stderr, "%s\n", buf );		// add newline
#endif
    }

#if defined(Q_OS_UNIX) && defined(QT_DEBUG)
    abort();				// trap; generates core dump
#elif defined(Q_OS_TEMP) && defined(QT_DEBUG)
    QString fstr;
    fstr.sprintf( "%s:%s %s %s\n", __FILE__, __LINE__, QT_VERSION_STR, buf );
    OutputDebugString( fstr.ucs2() );
#elif defined(Q_CC_MSVC) && defined(QT_DEBUG) && defined(_DEBUG)
    _CrtDbgReport( _CRT_ERROR, __FILE__, __LINE__, QT_VERSION_STR, buf );
#else
    exit( 1 );				// goodbye cruel world
#endif
}

/*!
  \fn void qSystemWarning( const char *msg, ... )
  \relates QApplication

  Prints the message \a msg together with the system's last error
  message (if available), or calls the message handler (if it has been
  installed). Use this method to notify the user of failures that are
  outside the control of the application.

  This function takes a format string and a list of arguments, similar
  to the C printf() function.

  Under X11, the text is printed to stderr. Under Windows, the text is
  sent to the debugger.

  \warning The internal buffer is limited to 8196 bytes (including the
  '\0'-terminator).

  \warning Passing (const char *)0 as argument to qSystemWarning might
  lead to crashes on certain platforms due to the platforms printf
  implementation.

  \sa qDebug(), qFatal(), qWarning(), qInstallMsgHandler(), \link
  debug.html Debugging\endlink

*/
void qSystemWarning( const char *msg, ... )
{
    QByteArray sys;

#if defined(Q_OS_WIN32)
    int code = GetLastError();
    unsigned short *string = 0;
    if (code) {
	QT_WA( {
	    FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
			   NULL,
			   code,
			   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			   (LPTSTR)&string,
			   0,
			   NULL );
	    sys = QString::fromUcs2(string).latin1();
	}, {
	    FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
			    NULL,
			    code,
			    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			    (char*)&string,
			    0,
			    NULL );

	    sys = (const char*) string;
	} );
	LocalFree( (HLOCAL)string );
    }
#else
    if (errno)
	sys = strerror(errno);
#endif

    QByteArray buf;
    buf.reserve(QT_BUFFER_LENGTH);
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
#if defined(QT_VSNPRINTF)
	int n = QT_VSNPRINTF( buf.data(), buf.capacity(), msg, ap );
#else
	int n = vsprintf( buf.data(), msg, ap );
#endif
	va_end( ap );
	buf.resize(n);
	if (!!sys) {
	    buf += ": ";
	    buf += sys;
	}
	(*handler)( QtSystemMsg, buf );
    } else {
	vsprintf( buf.data(), msg, ap );		// ### is there no vsnprintf()?
	va_end( ap );
#if defined(Q_CC_MWERKS)
        mac_default_handler(buf);
#elif defined(Q_OS_TEMP)
	QString fstr( buf );
	OutputDebugString( (fstr + "\n").ucs2() );
#else
	fprintf( stderr, "%s\n", buf.data() );		// add newline
#endif
    }
}

/*!
    \fn void Q_ASSERT( bool test )

    \relates QApplication

    Prints a warning message containing the source code file name and
    line number if \a test is false.

    This is really a macro defined in \c qglobal.h.

    Q_ASSERT is useful for testing pre- and post-conditions during
    development. It does nothing if \c QT_NO_DEBUG was defined during
    compilation.

    Example:
    \code
	//
	// File: div.cpp
	//

	#include <qglobal.h>

	int divide( int a, int b )
	{
	    Q_ASSERT( b != 0 );			// this is line 9
	    return a/b;
	}
    \endcode

    If \c b is zero, the Q_ASSERT statement will output the following
    message using the qFatal() function:
    \code
    ASSERT: "b == 0" in file div.cpp, line 9
    \endcode

    \sa Q_ASSERT_X(), qFatal(), \link debug.html Debugging\endlink
*/

/*!
    \fn void Q_ASSERT_X( bool test, const char *msg )

    \relates QApplication

    Prints the message \a msg together with the source file name and
    line number if \a test is false.

    This is really a macro defined in \c qglobal.h.

    Q_ASSERT_X is useful for testing pre- and post-conditions during
    development. It does nothing if \c QT_NO_DEBUG was defined during
    compilation.

    Example:
    \code
	//
	// File: div.cpp
	//

	#include <qglobal.h>

	int divide( int a, int b )
	{
	    Q_ASSERT_X( b != 0, "division by zero" );			// this is line 9
	    return a/b;
	}
    \endcode

    If \c b is zero, the Q_ASSERT_X statement will output the following
    message using the qFatal() function:
    \code
    ASSERT failure: "division by zero" in file div.cpp, line 9
    \endcode

    \sa Q_ASSERT(), qFatal(), \link debug.html Debugging\endlink
*/

/*!
    \fn void Q_CHECK_PTR( void *p )

    \relates QApplication

    If \a p is 0, prints a warning message containing the source code file
    name and line number, saying that the program ran out of memory.

    This is really a macro defined in \c qglobal.h.

    Q_CHECK_PTR does nothing if \c QT_NO_DEBUG was defined during
    compilation.

    Example:
    \code
	int *a;

	Q_CHECK_PTR( a = new int[80] );  // WRONG!

	a = new (nothrow) int[80];       // Right
	Q_CHECK_PTR( a );
    \endcode

    \sa qWarning(), \link debug.html Debugging\endlink
*/


/*
  The Q_CHECK_PTR macro calls this function if an allocation check
  fails.
*/
void qt_check_pointer(const char *n, int l)
{
    qWarning( "In file %s, line %d: Out of memory", n, l );
}

/*
  The Q_ASSERT macro calls this this function when the test fails.
*/
void qt_assert(const char *assertion, const char *file, int line)
{
    qFatal("ASSERT: \"%s\" in file %s, line %d", assertion, file, line);
}

/*
  The Q_ASSERT_X macro calls this this function when the test fails.
*/
void qt_assert_x(const char *where, const char *what, const char *file, int line)
{
    qFatal("ASSERT failure in %s: \"%s\", file %s, line %d", where, what, file, line);
}


static bool firstObsoleteWarning(const char *obj, const char *oldfunc )
{
    static QHash<QString,int*> *obsoleteDict = 0;
    if ( !obsoleteDict ) {			// first time func is called
	obsoleteDict = new QHash<QString,int*>;
#if defined(QT_DEBUG)
	qDebug(
      "You are using obsolete functions in the Qt library. Call the function\n"
      "qSuppressObsoleteWarnings() to suppress obsolete warnings.\n"
	     );
#endif
    }
    QByteArray s( obj );
    s += "::";
    s += oldfunc;
    if ( obsoleteDict->find(s) == 0 ) {
	obsoleteDict->insert( s, (int*)1 );	// anything different from 0
	return true;
    }
    return false;
}

static bool suppressObsolete = false;

void qSuppressObsoleteWarnings( bool suppress )
{
    suppressObsolete = suppress;
}

void qObsolete(	 const char *obj, const char *oldfunc, const char *newfunc )
{
    if ( suppressObsolete )
	return;
    if ( !firstObsoleteWarning(obj, oldfunc) )
	return;
    if ( obj )
	qDebug( "%s::%s: This function is obsolete, use %s instead.",
	       obj, oldfunc, newfunc );
    else
	qDebug( "%s: This function is obsolete, use %s instead.",
	       oldfunc, newfunc );
}

void qObsolete(	 const char *obj, const char *oldfunc )
{
    if ( suppressObsolete )
	return;
    if ( !firstObsoleteWarning(obj, oldfunc) )
	return;
    if ( obj )
	qDebug( "%s::%s: This function is obsolete.", obj, oldfunc );
    else
	qDebug( "%s: This function is obsolete.", oldfunc );
}

void qObsolete(	 const char *message )
{
    if ( suppressObsolete )
	return;
    if ( !firstObsoleteWarning( "Qt", message) )
	return;
    qDebug( "%s", message );
}


/*!
    \relates QApplication

    Installs a Qt message handler \a h. Returns a pointer to the
    message handler previously defined.

    The message handler is a function that prints out debug messages,
    warnings and fatal error messages. The Qt library (debug version)
    contains hundreds of warning messages that are printed when
    internal errors (usually invalid function arguments) occur. If you
    implement your own message handler, you get total control of these
    messages.

    The default message handler prints the message to the standard
    output under X11 or to the debugger under Windows. If it is a
    fatal message, the application aborts immediately.

    Only one message handler can be defined, since this is usually
    done on an application-wide basis to control debug output.

    To restore the message handler, call \c qInstallMsgHandler(0).

    Example:
    \code
	#include <qapplication.h>
	#include <stdio.h>
	#include <stdlib.h>

	void myMessageOutput( QtMsgType type, const char *msg )
	{
	    switch ( type ) {
		case QtDebugMsg:
		    fprintf( stderr, "Debug: %s\n", msg );
		    break;
		case QtSystemMsg:
		    fprintf( stderr, "Warning: %s\n", msg );
		    break;
		case QtWarningMsg:
		    fprintf( stderr, "Warning: %s\n", msg );
		    break;
		case QtFatalMsg:
		    fprintf( stderr, "Fatal: %s\n", msg );
		    abort();			// deliberately core dump
	    }
	}

	int main( int argc, char **argv )
	{
	    qInstallMsgHandler( myMessageOutput );
	    QApplication a( argc, argv );
	    ...
	    return a.exec();
	}
    \endcode

    \sa qDebug(), qWarning(), qFatal(), \link debug.html Debugging\endlink
*/

QtMsgHandler qInstallMsgHandler( QtMsgHandler h )
{
    QtMsgHandler old = handler;
    handler = h;
    return old;
}


/*
    Dijkstra's bisection algorithm to find the square root as an integer.
    Deliberately not exported as part of the Qt API, but used in both
    qsimplerichtext.cpp and qgfxraster_qws.cpp
*/
Q_KERNEL_EXPORT unsigned int qt_int_sqrt( unsigned int n )
{
    // n must be in the range 0...UINT_MAX/2-1
    if ( n >= ( UINT_MAX>>2 ) ) {
	unsigned int r = 2 * qt_int_sqrt( n / 4 );
	unsigned int r2 = r + 1;
	return ( n >= r2 * r2 ) ? r2 : r;
    }
    uint h, p= 0, q= 1, r= n;
    while ( q <= n )
        q <<= 2;
    while ( q != 1 ) {
        q >>= 2;
        h= p + q;
        p >>= 1;
        if ( r >= h ) {
            p += q;
            r -= h;
        }
    }
    return p;
}


void *qMalloc(size_t size) { return ::malloc(size); }
void qFree(void *ptr) { ::free(ptr); }
void *qRealloc(void *ptr, size_t size) { return ::realloc(ptr, size); }
int qRand(void) { return ::rand(); }
void *qMemCopy(void *dest, const void *src, size_t n) { return ::memcpy(dest, src, n); }
void *qMemSet(void *dest, int c, size_t n) { return ::memset(dest, c, n); }

#undef qDebug
void qDebug( const char *msg, ... )
{
    char buf[QT_BUFFER_LENGTH];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
#if defined(QT_VSNPRINTF)
	QT_VSNPRINTF( buf, QT_BUFFER_LENGTH, msg, ap );
#else
	vsprintf( buf, msg, ap );
#endif
	va_end( ap );
	(*handler)( QtDebugMsg, buf );
    } else {
	vsprintf( buf, msg, ap );		// ### is there no vsnprintf()?
	va_end( ap );
#if defined(Q_CC_MWERKS)
        mac_default_handler(buf);
#elif defined(Q_OS_TEMP)
	QString fstr( buf );
	OutputDebugString( (fstr + "\n").ucs2() );
#else
	fprintf( stderr, "%s\n", buf );		// add newline
#endif
    }
}

#undef qWarning
void qWarning( const char *msg, ... )
{
    char buf[QT_BUFFER_LENGTH];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    static bool fatalWarnings = (getenv("QT_FATAL_WARNINGS") != 0);
    if ( handler ) {
#if defined(QT_VSNPRINTF)
	QT_VSNPRINTF( buf, QT_BUFFER_LENGTH, msg, ap );
#else
	vsprintf( buf, msg, ap );
#endif
	va_end( ap );
	(*handler)( QtWarningMsg, buf );
    } else {
	vsprintf( buf, msg, ap );		// ### is there no vsnprintf()?
	va_end( ap );
#if defined(Q_CC_MWERKS)
        mac_default_handler(buf);
#else
	fprintf( stderr, "%s\n", buf );		// add newline
#endif
#if defined(Q_OS_TEMP) && defined(QT_DEBUG)
	QString fstr;
	fstr.sprintf( "%s:%s %s %s\n", __FILE__, __LINE__, QT_VERSION_STR, buf );
	OutputDebugString( fstr.ucs2() );
#elif defined(Q_CC_MSVC) && defined(QT_DEBUG) && defined(_DEBUG)
	if (fatalWarnings)
	    _CrtDbgReport( _CRT_ERROR, __FILE__, __LINE__, QT_VERSION_STR, buf );
#endif
    }

    if (!fatalWarnings)
	return;

#if defined(Q_OS_UNIX) && defined(QT_DEBUG)
    abort();				// trap; generates core dump
#else
    exit( 1 );				// goodbye cruel world
#endif
}

