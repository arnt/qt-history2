/****************************************************************************
** $Id: //depot/qt/main/src/tools/qglobal.cpp#68 $
**
** Global functions
**
** Created : 920604
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qglobal.h"
#include "qdict.h"
#include "qstring.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

/*!
  \relates QApplication
  Returns the Qt version number for the library, typically "1.30"
  or "1.31".
*/

Q_EXPORT
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

  The system's word size in bits (typically 32) is returned in \e *wordSize.
  The \e *bigEndian is set to TRUE if this is a big-endian machine,
  or to FALSE if this is a little-endian machine.

  This function calls fatal() with a message if the computer is truly weird
  (i.e. different endianness for 16 bit and 32 bit integers).
*/

Q_EXPORT
bool qSysInfo( int *wordSize, bool *bigEndian )
{
#if defined(CHECK_NULL)
    ASSERT( wordSize != 0 );
    ASSERT( bigEndian != 0 );
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
#if defined(CHECK_RANGE)
	fatal( "qSysInfo: Unsupported system word size %d", *wordSize );
#endif
	return FALSE;
    }
    if ( sizeof(Q_INT8) != 1 || sizeof(Q_INT16) != 2 || sizeof(Q_INT32) != 4 ||
	 sizeof(float) != 4 || sizeof(double) != 8 ) {
#if defined(CHECK_RANGE)
	fatal( "qSysInfo: Unsupported system data type size" );
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
#if defined(CHECK_RANGE)
	fatal( "qSysInfo: Inconsistent system byte order" );
#endif
	return FALSE;
    }

    *bigEndian = si_bigEndian = be32;
    return TRUE;
}


/*****************************************************************************
  Debug output routines
 *****************************************************************************/

static msg_handler handler = 0;			// pointer to debug handler

/*!
  \relates QApplication
  Prints a debug message, or calls the message handler (if it has been
  installed).

  This function takes a format string and a stack arguments, similar to
  the C printf() function.

  Example:
  \code
    debug( "my window handle = %x", myWidget->id() );
  \endcode

  Under X11, the text is printed to stderr.  Under Windows, the text is
  sent to the debugger.

  Note: If DEBUG was not defined when the Qt library was built
  (i.e. NO_DEBUG was defined), this function does nothing.

  \warning The internal buffer is limited to 512 bytes (including the
  0-terminator).

  \sa warning(), fatal(), qInstallMsgHandler(),
  \link debug.html Debugging\endlink
*/

Q_EXPORT
void debug( const char *msg, ... )
{
//#if defined(TESTEAA)
    char buf[512];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsprintf( buf, msg, ap );
	va_end( ap );
	(*handler)( QtDebugMsg, buf );
    } else {
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
    }
//#else
    //Q_UNUSED( msg );
//#endif
}

/*!
  \relates QApplication
  Prints a warning message, or calls the message handler (if it has been
  installed).

  This function takes a format string and a stack arguments, similar to
  the C printf() function.

  Example:
  \code
    void f( int c )
    {
	if ( c > 200 )
	    warning( "f: bad argument, c == %d", c );
    }
  \endcode

  Under X11, the text is printed to stderr.  Under Windows, the text is
  sent to the debugger.

  \warning The internal buffer is limited to 512 bytes (including the
  0-terminator).

  \sa debug(), fatal(), qInstallMsgHandler(),
  \link debug.html Debugging\endlink
*/

Q_EXPORT
void warning( const char *msg, ... )
{
    char buf[512];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsprintf( buf, msg, ap );
	va_end( ap );
	(*handler)( QtWarningMsg, buf );
    } else {
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
    }
}


/*!
  \relates QApplication
  Prints a fatal error message and exits, or calls the message handler (if it
  has been installed).

  This function takes a format string and a stack arguments, similar to
  the C printf() function.

  Example:
  \code
    int divide( int a, int b )
    {
	if ( b == 0 )				// program error
	    fatal( "divide: cannot divide by zero" );
	return a/b;
    }
  \endcode

  Under X11, the text is printed to stderr.  Under Windows, the text is
  sent to the debugger.

  \warning The internal buffer is limited to 512 bytes (including the
  0-terminator).

  \sa debug(), warning(), qInstallMsgHandler(),
  \link debug.html Debugging\endlink
*/

Q_EXPORT
void fatal( const char *msg, ... )
{
    char buf[512];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsprintf( buf, msg, ap );
	va_end( ap );
	(*handler)( QtFatalMsg, buf );
    } else {
	vfprintf( stderr, msg, ap );
	va_end( ap );
	fprintf( stderr, "\n" );		// add newline
#if defined(UNIX) && defined(DEBUG)
	abort();				// trap; generates core dump
#else
	exit( 1 );				// goodbye cruel world
#endif
    }
}

/*!
  \fn void ASSERT( bool test )
  \relates QApplication
  Prints a warning message containing the source code file name and line number
  if \e test is FALSE.

  This is really a macro defined in qglobal.h.

  ASSERT is useful for testing required conditions in your program.

  Example:
  \code
    //
    // File: div.cpp
    //

    #include <qglobal.h>

    int divide( int a, int b )
    {
	ASSERT( b != 0 );			// this is line 9
	return a/b;
    }
  \endcode

  If \c b is zero, the ASSERT statement will output the following message
  using the warning() function:
  \code
    ASSERT: "b == 0" in div.cpp (9)
  \endcode

  \sa warning(), \link debug.html Debugging\endlink
*/


/*!
  \fn void CHECK_PTR( void *p )
  \relates QApplication
  If \e p is null, a fatal messages says that the program ran out of memory
  and exits.  If \e p is not null, nothing happens.

  This is really a macro defined in qglobal.h.

  Example:
  \code
    int *a;
    CHECK_PTR( a = new int[80] );	// never do this!
      // do this instead:
    a = new int[80];
    CHECK_PTR( a );			// this is fine
  \endcode

  \sa fatal(), \link debug.html Debugging\endlink
*/


//
// The CHECK_PTR macro calls this function to check if an allocation went ok.
//

Q_EXPORT
bool qt_check_pointer( bool c, const char *n, int l )
{
    if ( c )
	fatal( "In file %s, line %d: Out of memory", n, l );
    return TRUE;
}


static bool firstObsoleteWarning(const char *obj, const char *oldfunc )
{
    static QDict<int> *obsoleteDict = 0;
    if ( !obsoleteDict ) {			// first time func is called
	obsoleteDict = new QDict<int>;
#if defined(DEBUG)
	debug(
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

Q_EXPORT
void qSuppressObsoleteWarnings( bool suppress )
{
    suppressObsolete = suppress;
}

Q_EXPORT
void qObsolete(	 const char *obj, const char *oldfunc, const char *newfunc )
{
    if ( suppressObsolete )
	return;
    if ( !firstObsoleteWarning(obj, oldfunc) )
	return;
    debug( "%s::%s: This function is obsolete, use %s instead",
	   obj, oldfunc, newfunc );
}

Q_EXPORT
void qObsolete(	 const char *obj, const char *oldfunc )
{
    if ( suppressObsolete )
	return;
    if ( !firstObsoleteWarning(obj, oldfunc) )
	return;
    debug( "%s::%s: This function is obsolete.", obj, oldfunc );
}

Q_EXPORT
void qObsolete(	 const char *message )
{
    if ( suppressObsolete )
	return;
    if ( !firstObsoleteWarning( "Qt", message) )
	return;
    debug( "%s", message );
}


/*!
  \relates QApplication
  Installs a Qt message handler.  Returns a pointer to the message handler
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

  \sa debug(), warning(), fatal(), \link debug.html Debugging\endlink
*/

Q_EXPORT
msg_handler qInstallMsgHandler( msg_handler h )
{
    msg_handler old = handler;
    handler = h;
    return old;
}


#ifdef _WS_WIN_
bool qt_winunicode=FALSE;
#endif
