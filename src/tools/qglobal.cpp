/****************************************************************************
** $Id: //depot/qt/main/src/tools/qglobal.cpp#73 $
**
** Global functions
**
** Created : 920604
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#include "qglobal.h"
#include "qasciidict.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#if defined(Q_OS_WIN32)
#define vsnprintf _vsnprintf
#else
#include <errno.h>
#endif

// NOT REVISED

/*!
  \relates QApplication
  Returns the Qt version number for the library, typically "1.30"
  or "2.1.0".
*/

const char *qVersion()
{
    return QT_VERSION_STR;
}


/*****************************************************************************
  System detection routines
 *****************************************************************************/

static bool si_alreadyDone = FALSE;
static int  si_wordSize;
static bool si_bigEndian;

/*!
  \relates QApplication
  Obtains information about the system.

  The system's word size in bits (typically 32) is returned in \a wordSize.
  The \a bigEndian is set to TRUE if this is a big-endian machine,
  or to FALSE if this is a little-endian machine.

  In debug mode, this function calls qFatal() with a message if the computer is 
  truly weird (i.e. different endianness for 16 bit and 32 bit integers), in 
  release mode it returns FALSE.
*/

bool qSysInfo( int *wordSize, bool *bigEndian )
{
#if defined(QT_CHECK_NULL)
    Q_ASSERT( wordSize != 0 );
    Q_ASSERT( bigEndian != 0 );
#endif

    if ( si_alreadyDone ) {			// run it only once
	*wordSize  = si_wordSize;
	*bigEndian = si_bigEndian;
	return TRUE;
    }
    si_alreadyDone = TRUE;

    si_wordSize = 0;
    uint n = (uint)(~0);
    while ( n ) {				// detect word size
	si_wordSize++;
	n /= 2;
    }
    *wordSize = si_wordSize;

    if ( *wordSize != 64 &&
	 *wordSize != 32 &&
	 *wordSize != 16 ) {			// word size: 16, 32 or 64
#if defined(QT_CHECK_RANGE)
	qFatal( "qSysInfo: Unsupported system word size %d", *wordSize );
#endif
	return FALSE;
    }
    if ( sizeof(Q_INT8) != 1 || sizeof(Q_INT16) != 2 || sizeof(Q_INT32) != 4 ||
	 sizeof(float) != 4 || sizeof(double) != 8 ) {
#if defined(QT_CHECK_RANGE)
	qFatal( "qSysInfo: Unsupported system data type size" );
#endif
	return FALSE;
    }

    bool  be16, be32;				// determine byte ordering
    short ns = 0x1234;
    int	  nl = 0x12345678;

    unsigned char *p = (unsigned char *)(&ns);	// 16-bit integer
    be16 = *p == 0x12;

    p = (unsigned char *)(&nl);			// 32-bit integer
    if ( p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78 )
	be32 = TRUE;
    else
    if ( p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12 )
	be32 = FALSE;
    else
	be32 = !be16;

    if ( be16 != be32 ) {			// strange machine!
#if defined(QT_CHECK_RANGE)
	qFatal( "qSysInfo: Inconsistent system byte order" );
#endif
	return FALSE;
    }

    *bigEndian = si_bigEndian = be32;
    return TRUE;
}

#if defined(Q_OS_WIN32)
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

    static int winver = Qt::WV_NT;
    static int t=0;
    if ( !t ) {
	t=1;
	OSVERSIONINFOA osver;
	osver.dwOSVersionInfoSize = sizeof(osver);
	GetVersionExA( &osver );
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
    return winver;
}
#endif


/*****************************************************************************
  Debug output routines
 *****************************************************************************/

/*!
  \fn void qDebug( const char *msg, ... )

  \relates QApplication
  Prints a debug message \a msg, or calls the message handler (if it has been
  installed).

  This function takes a format string and a list of arguments, similar to
  the C printf() function.

  Example:
  \code
    qDebug( "my window handle = %x", myWidget->id() );
  \endcode

  Under X11, the text is printed to stderr.  Under Windows, the text is
  sent to the debugger.

  \warning The internal buffer is limited to 8196 bytes (including the
  0-terminator).

  \sa qWarning(), qFatal(), qInstallMsgHandler(),
  \link debug.html Debugging\endlink
*/

/*!
  \fn void qWarning( const char *msg, ... )

  \relates QApplication
  Prints a warning message \a msg, or calls the message handler (if it has been
  installed).

  This function takes a format string and a list of arguments, similar to
  the C printf() function.

  Example:
  \code
    void f( int c )
    {
	if ( c > 200 )
	    qWarning( "f: bad argument, c == %d", c );
    }
  \endcode

  Under X11, the text is printed to stderr.  Under Windows, the text is
  sent to the debugger.

  \warning The internal buffer is limited to 8196 bytes (including the
  0-terminator).

  \sa qDebug(), qFatal(), qInstallMsgHandler(),
  \link debug.html Debugging\endlink
*/

/*!
  \fn void qFatal( const char *msg, ... )

  \relates QApplication
  Prints a fatal error message \a msg and exits, or calls the message handler (if it
  has been installed).

  This function takes a format string and a list of arguments, similar to
  the C printf() function.

  Example:
  \code
    int divide( int a, int b )
    {
	if ( b == 0 )				// program error
	    qFatal( "divide: cannot divide by zero" );
	return a/b;
    }
  \endcode

  Under X11, the text is printed to stderr.  Under Windows, the text is
  sent to the debugger.

  \warning The internal buffer is limited to 8196 bytes (including the
  0-terminator).

  \sa qDebug(), qWarning(), qInstallMsgHandler(),
  \link debug.html Debugging\endlink
*/


static QtMsgHandler handler = 0;			// pointer to debug handler

#ifdef Q_OS_MAC
const unsigned char * p_str(const char * c)
{
    static unsigned char * ret=NULL;
    static int ret_len = 0;

    int len = qstrlen(c);
    if(len > ret_len) {
	delete ret;
	ret = new unsigned char[ret_len = (len+2)];
    }
    ret[0]=len;
    qstrcpy(((char *)ret)+1,c);
    return ret;
}

QCString p2qstring(const unsigned char *c) {
       char *arr = (char *)malloc(c[0] + 1);
       memcpy(arr, c+1, c[0]);
       arr[c[0]] = '\0';
       QCString ret = arr;
       delete arr;
       return ret;
} 
#endif

#ifdef Q_OS_MAC9

#include "qt_mac.h"

extern bool	  qt_is_gui_used;
static void mac_default_handler(const char *msg)
{
      if(qt_is_gui_used)
        DebugStr(p_str(msg));	
      else
         fprintf(stderr, msg);
}

#endif


void qDebug( const char *msg, ... )
{
    char buf[8196];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsprintf( buf, msg, ap );		// ### vsnprintf would be great here
	va_end( ap );
	(*handler)( QtDebugMsg, buf );
    } else {
#ifdef Q_OS_MAC9
	vsprintf( buf, msg, ap );		
	va_end( ap );
        mac_default_handler(buf);
#else
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
#endif
    }
}

// copied... this looks really bad.
void debug( const char *msg, ... )
{
    char buf[8196];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsnprintf( buf, 512, msg, ap );
	va_end( ap );
	(*handler)( QtDebugMsg, buf );
    } else {
#ifdef Q_OS_MAC9
	vsprintf( buf, msg, ap );		
	va_end( ap );
        mac_default_handler(buf);
#else
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
#endif
    }
}

void qWarning( const char *msg, ... )
{
    char buf[8196];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsnprintf( buf, 512, msg, ap );
	va_end( ap );
	(*handler)( QtWarningMsg, buf );
    } else {
#ifdef Q_OS_MAC9
	vsprintf( buf, msg, ap );		
	va_end( ap );
        mac_default_handler(buf);
#else
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
#endif
    }
}


// again, copied
void warning( const char *msg, ... )
{
    char buf[8196];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsnprintf( buf, 512, msg, ap );
	va_end( ap );
	(*handler)( QtWarningMsg, buf );
    } else {
#ifdef Q_OS_MAC9
	vsprintf( buf, msg, ap );		
	va_end( ap );
        mac_default_handler(buf);
#else
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
#endif
    }
}

void qFatal( const char *msg, ... )
{
    char buf[8196];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsnprintf( buf, 512, msg, ap );
	va_end( ap );
	(*handler)( QtFatalMsg, buf );
    } else {
#ifdef Q_OS_MAC9
	vsprintf( buf, msg, ap );		
	va_end( ap );
        mac_default_handler(buf);
#else
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
#endif    
#if defined(Q_OS_UNIX) && defined(QT_DEBUG)
	abort();				// trap; generates core dump
#else
	exit( 1 );				// goodbye cruel world
#endif
    }
}

// yet again, copied
void fatal( const char *msg, ... )
{
    char buf[8196];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsnprintf( buf, 512, msg, ap );
	va_end( ap );
	(*handler)( QtFatalMsg, buf );
    } else {
#ifdef Q_OS_MAC9
	vsprintf( buf, msg, ap );		
	va_end( ap );
        mac_default_handler(buf);
#else
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
#endif    
#if defined(Q_OS_UNIX) && defined(QT_DEBUG)
	abort();				// trap; generates core dump
#else
	exit( 1 );				// goodbye cruel world
#endif
    }
}

/*!
  \relates QApplication

  Prints the message \a msg and uses \a code to get a system 
  specific error message. When \a code is -1 (default), the system's last 
  error code will be used if possible.
  Use this method to handle failures in platform specific API calls.

  This function does nothing when Qt is built with Q_NO_DEBUG
  defined.
*/
void qSystemWarning( const char* msg, int code )
{
#ifndef Q_NO_DEBUG
#if defined(Q_OS_WIN32)
    if ( code == -1 )
	code = GetLastError();

    if ( !code )
	return;

    char* string;

    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
			  NULL,
			  code,
			  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			  (char*)&string,
			  0,
			  NULL );

    qWarning( "%s\n\tError code %d - %s", msg, code, (const char*)string );

    LocalFree( (HLOCAL)string );
#else
    if ( code != -1 )
	qWarning( "%s\n\tError code %d - %s", msg, code, strerror( code ) );
    else
	qWarning( msg );
#endif
#endif
}

/*!
  \fn void Q_ASSERT( bool test )
  \relates QApplication
  Prints a warning message containing the source code file name and line number
  if \a test is FALSE.

  This is really a macro defined in qglobal.h.

  Q_ASSERT is useful for testing required conditions in your program.

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

  If \c b is zero, the Q_ASSERT statement will output the following message
  using the qWarning() function:
  \code
    ASSERT: "b == 0" in div.cpp (9)
  \endcode

  \sa qWarning(), \link debug.html Debugging\endlink
*/


/*!
  \fn void Q_CHECK_PTR( void *p )
  \relates QApplication
  If \a p is null, a fatal messages says that the program ran out of memory
  and exits.  If \e p is not null, nothing happens.

  This is really a macro defined in qglobal.h.

  Example:
  \code
    int *a;
    Q_CHECK_PTR( a = new int[80] );	// never do this!
      // do this instead:
    a = new int[80];
    Q_CHECK_PTR( a );			// this is fine
  \endcode

  \sa qFatal(), \link debug.html Debugging\endlink
*/


//
// The Q_CHECK_PTR macro calls this function to check if an allocation went ok.
//

bool qt_check_pointer( bool c, const char *n, int l )
{
    if ( c )
	qFatal( "In file %s, line %d: Out of memory", n, l );
    return TRUE;
}


static bool firstObsoleteWarning(const char *obj, const char *oldfunc )
{
    static QAsciiDict<int> *obsoleteDict = 0;
    if ( !obsoleteDict ) {			// first time func is called
	obsoleteDict = new QAsciiDict<int>;
#if defined(QT_DEBUG)
	qDebug(
      "You are using obsolete functions in the Qt library. Call the function\n"
      "qSuppressObsoleteWarnings() to suppress obsolete warnings.\n"
	     );
#endif
    }
    QCString s( obj );
    s += "::";
    s += oldfunc;
    if ( obsoleteDict->find(s.data()) == 0 ) {
	obsoleteDict->insert( s.data(), (int*)1 );	// anything different from 0
	return TRUE;
    }
    return FALSE;
}

static bool suppressObsolete = FALSE;

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
  Installs a Qt message handler \a h.  Returns a pointer to the message handler
  previously defined.

  The message handler is a function that prints out debug messages,
  warnings and fatal error messages.  The Qt library (debug version)
  contains hundreds of warning messages that are printed when internal
  errors (usually invalid function arguments) occur.  If you implement
  your own message handler, you get total control of these messages.

  The default message handler prints the message to the standard output
  under X11 or to the debugger under Windows.  If it is a fatal message,
  the application aborts immediately.

  Only one message handler can be defined, since this is usually done on
  an application-wide basis to control debug output.

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
	    case QtWarningMsg:
		fprintf( stderr, "Warning: %s\n", msg );
		break;
	    case QtFatalMsg:
		fprintf( stderr, "Fatal: %s\n", msg );
		abort();			// dump core on purpose
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


#ifdef Q_WS_WIN
bool qt_winunicode=FALSE;
#endif
