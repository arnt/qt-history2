/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qsocketdevice_unix.cpp#9 $
**
** Implementation of Network Extension Library
**
** Created : 970521
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qsocketdevice.h"
#include "qwindowdefs.h"
#include <string.h>


#if defined(_OS_WIN32_)
#include <windows.h>
#endif

#if defined(_OS_UNIX_)
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>
#include <errno.h>
#endif

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX    108
#endif

#ifdef __MIPSEL__
 #ifndef SOCK_DGRAM
  #define SOCK_DGRAM 1
 #endif
 #ifndef SOCK_STREAM
  #define SOCK_STREAM 2
 #endif
#endif
//#define QSOCKETDEVICE_DEBUG


#if defined(_OS_SOLARIS_) || defined(_OS_UNIXWARE7_)
// this should perhaps be included for all unixware versions?
#include <sys/filio.h>
#endif
#if defined(_OS_SOLARIS_) || defined(_OS_UNIXWARE7_) || defined(_OS_OS2EMX_)
// and this then?  unixware?
#ifndef FNDELAY
#define FNDELAY O_NDELAY
#endif
#endif

// this mess (it's not yet a mess but I'm sure it'll be one before
// it's done) defines SOCKLEN_T to socklen_t or whatever else, for
// this system.  Single Unix 2 says it's to be socklen_t, classically
// it's int, who knows what it might be on different modern unixes.

#if defined(SOCKLEN_T)
#undef SOCKLEN_T
#endif

#if defined(_XOPEN_UNIX)

// new linux, at least
#define SOCKLEN_T socklen_t

#else

// caldera 1.3, for example... probably all linux-libc5 systems
#define SOCKLEN_T int

#endif


// test that EAGAIN exists, and that if EWOULDBLOCK also exists, that
// they have the same value.

#if !defined(EAGAIN)
#error "requires EAGAIN (mostly for simplicity)"
#elif defined(EWOULDBLOCK) && ( EAGAIN != EWOULDBLOCK )
#error "does not support EWOULDBLOCK that is different from EAGAIN"
#endif




static void cleanupWinSock() // post-routine
{
#if defined(_OS_WIN32_)
    WSACleanup();
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice: WinSock cleanup" );
#endif
#endif
}

void QSocketDevice::init()
{
#if defined(_OS_WIN32_)
    static bool init = FALSE;
    if ( !init ) {
	WSAData wsadata;
	bool error = WSAStartup(MAKEWORD(1,1),&wsadata) != 0;
	if ( error ) {
#if defined(CHECK_NULL)
	    qWarning( "QSocketDevice: WinSock initialization failed" );
#endif
	    return FALSE;
	}
	qAddPostRoutine( cleanupWinSock );
#if defined(QSOCKETDEVICE_DEBUG) || defined(QSOCKETDEVICE_DEBUG)
	qDebug( "QSocketDevice: WinSock initialization %s",
		(error ? "failed" : "OK") );
#endif
	init = TRUE;
    }
#endif
}




/*!
  Creates a QSocketDevice object for a stream or datagram socket.

  The \a type argument must be either \c QSocketDevice::Stream for a
  reliable, connection-oriented TCP socket, or \c
  QSocketDevice::Datagram for an unreliable UDP socket.

  \sa blocking()
*/

QSocketDevice::QSocketDevice( Type type, bool inet )
    : fd( -1 ), t( Stream ), p( 0 ), pp( 0 ), e( NoError ), d( 0 )
{
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice: Created QSocketDevice object %p, type %d",
	    this, type );
#endif
    init();
    int s = ::socket( inet ? AF_INET : AF_UNIX,
	    type==Datagram?SOCK_DGRAM:SOCK_STREAM, 0 );
    if ( s < 0 ) {
	// leave fd at -1 but set the type
	t = type;
	switch( errno ) {
	case EPROTONOSUPPORT:
	    e = Bug; // 0 is supposed to work for both types
	    break;
	case ENFILE:
	    e = NoFiles; // special case for this
	    break;
	case EACCES:
	    e = Inaccessible;
	    break;
	case ENOBUFS:
	case ENOMEM:
	    e = NoResources;
	    break;
	case EINVAL:
	    e = Impossible;
	    break;
	default:
	    e = UnknownError;
	    break;
	}
    } else {
	setSocket( s, type );
    }
}


/*! \reimp

  Closes the socket and sets the socket identifier to -1 (invalid).

  (This function ignores errors; if there are any then a file
  descriptor leakage might result.  As far as we know, the only error
  that can arise is EBADF, and that would of course not cause leakage.
  There may be OS-specfic errors that we haven't come across,
  however.)

  \sa open()
*/

void QSocketDevice::close()
{
    if ( fd == -1 || !isOpen() )		// already closed
	return;
    setFlags( IO_Sequential );
    setStatus( IO_Ok );
#if defined(_OS_WIN32_)
    ::closesocket( fd );
#elif defined(_OS_UNIX_)
    ::close( fd );
#else
    #error "This OS is not supported"
#endif
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::close: Closed socket %x", fd );
#endif
    fd = -1;
    fetchConnectionParameters();
}


/*!
  Returns TRUE if the socket is in blocking mode, or FALSE if it
  is in nonblocking mode or if the socket is invalid.

  Note that this function does not set error().

  \warning On Windows, this function always returns TRUE since the
  ioctlsocket() function is broken.

  \sa setBlocking(), isValid()
*/

bool QSocketDevice::blocking() const
{
    if ( !isValid() )
	return TRUE;
#if defined(_OS_WIN32_)
    return TRUE;
#elif defined(_OS_UNIX_)
    int s = fcntl(fd, F_GETFL, 0);
    return !(s >= 0 && ((s & FNDELAY) != 0));
#else
    #error "This OS is not supported"
#endif
}


/*!
  Makes the socket blocking if \a enable is TRUE or nonblocking if
  \a enable is FALSE.

  Sockets are blocking by default, but we recommend using
  nonblocking socket operations, especially for GUI programs that need
  to be responsive.

  \warning On Windows, this function does nothing since the
  ioctlsocket() function is broken.  Whenever you use a
  QSocketNotifier on Windows, the socket is immediately made
  nonblocking.

  \sa blocking(), isValid()
*/

void QSocketDevice::setBlocking( bool enable )
{
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::setBlocking( %d )", enable );
#endif
    if ( !isValid() )
	return;
#if defined(_OS_WIN32_)
    // Do nothing
#elif defined(_OS_UNIX_)
    int tmp = ::fcntl(fd, F_GETFL, 0);
    if ( tmp >= 0 )
	tmp = fcntl( fd, F_SETFL, enable ? (tmp&!FNDELAY) : (tmp|FNDELAY) );
    if ( tmp >= 0 )
	return;
    if ( e )
	return;
    switch( errno ) {
    case EACCES:
    case EBADF:
	e = Impossible;
	break;
    case EFAULT:
    case EAGAIN:
    case EDEADLK:
    case EINTR:
    case EINVAL:
    case EMFILE:
    case ENOLCK:
    case EPERM:
    default:
	e = UnknownError;
    }
#else
#error "This OS is not supported"
#endif
}


/*!
  Returns a socket option.
*/

int QSocketDevice::option( Option opt ) const
{
    if ( !isValid() )
	return -1;
    int n = -1;
    int v = -1;
    switch ( opt ) {
    case Broadcast:
	n = SO_BROADCAST;
	break;
    case ReceiveBuffer:
	n = SO_RCVBUF;
	break;
    case ReuseAddress:
	n = SO_REUSEADDR;
	break;
    case SendBuffer:
	n = SO_SNDBUF;
	break;
    }
    if ( n != -1 ) {
	SOCKLEN_T len = sizeof(v);
	int r = ::getsockopt( fd, SOL_SOCKET, n, (char*)&v, &len );
	if ( r >= 0 )
	    return v;
	if ( !e ) {
	    switch( errno ) {
	    case EBADF:
	    case ENOTSOCK:
		e = Impossible;
		break;
	    case EFAULT:
		e = Bug;
		break;
	    default:
		e = UnknownError;
		break;
	    }
	}
	return -1;
    }
    return v;
}


/*!
  Sets a socket option.
*/

void QSocketDevice::setOption( Option opt, int v )
{
    if ( !isValid() )
	return;
    int n = -1; // for really, really bad compilers
    switch ( opt ) {
    case Broadcast:
	n = SO_BROADCAST;
	break;
    case ReceiveBuffer:
	n = SO_RCVBUF;
	break;
    case ReuseAddress:
	n = SO_REUSEADDR;
	break;
    case SendBuffer:
	n = SO_SNDBUF;
	break;
    default:
	return;
    }
    if ( ::setsockopt( fd, SOL_SOCKET, n, (char*)&v, sizeof(v)) < 0 &&
	 e == NoError ) {
	switch( errno ) {
	case EBADF:
	case ENOTSOCK:
	    e = Impossible;
	    break;
	case EFAULT:
	    e = Bug;
	    break;
	default:
	    e = UnknownError;
	    break;
	}
    }
}


/*!
  Connects to the IP address and port specified by \a addr.  Returns
  TRUE if it establishes a connection, and FALSE if not.  error()
  explains why.

  Note that error() commonly returns NoError for non-blocking sockets;
  this just means that you can call connect() again in a little while
  and it'll probably succeed.
*/

bool QSocketDevice::connect( const QHostAddress &addr, uint port )
{
    if ( !isValid() )
	return FALSE;

    struct sockaddr_in a;
    memset( &a, 0, sizeof(a) );
    a.sin_family = AF_INET;
    a.sin_port = htons( port );
    a.sin_addr.s_addr = htonl( addr.ip4Addr() );

    int r = ::connect( fd, (struct sockaddr*)&a,
		       sizeof(struct sockaddr_in) );
#if defined(_OS_WIN32_)
    if ( r == SOCKET_ERROR )
	return FALSE;
    fetchConnectionParameters();
    return TRUE;
#elif defined(_OS_UNIX_)
    if ( r == 0 ) {
	fetchConnectionParameters();
	return TRUE;
    }
    if ( errno == EISCONN || errno == EALREADY || errno == EINPROGRESS )
	return TRUE;
    if ( e != NoError )
	return FALSE;
    switch( errno ) {
    case EBADF:
    case ENOTSOCK:
	e = Impossible;
	break;
    case EFAULT:
    case EAFNOSUPPORT:
	e = Bug;
	break;
    case ECONNREFUSED:
	e = ConnectionRefused;
	break;
    case ETIMEDOUT:
    case ENETUNREACH:
	e = NetworkFailure;
	break;
    case EADDRINUSE:
	e = NoResources;
	break;
    case EACCES:
    case EPERM:
	e = Inaccessible;
	break;
    case EAGAIN:
	// ignore that.  can retry.
	break;
    default:
	e = UnknownError;
	break;
    }
    return FALSE;
#else
    #error "This OS is not supported"
#endif
}

bool QSocketDevice::connect( const QString &localfilename )
{
    if ( !isValid() )
	return FALSE;

    struct sockaddr_un a;
    memset( &a, 0, sizeof(a) );
    a.sun_family = AF_UNIX;
    strncpy(a.sun_path,localfilename.local8Bit(),UNIX_PATH_MAX-1);

    int r = ::connect( fd, (struct sockaddr*)&a,
		       sizeof(struct sockaddr_un) );
#if defined(_OS_WIN32_)
    if ( r == SOCKET_ERROR )
	return FALSE;
    fetchConnectionParameters();
    return TRUE;
#elif defined(_OS_UNIX_)
    if ( r == 0 ) {
	fetchConnectionParameters();
	return TRUE;
    }
    if ( errno == EISCONN || errno == EALREADY || errno == EINPROGRESS )
	return TRUE;
    if ( e != NoError )
	return FALSE;
    switch( errno ) {
    case EBADF:
    case ENOTSOCK:
	e = Impossible;
	break;
    case EFAULT:
    case EAFNOSUPPORT:
	e = Bug;
	break;
    case ECONNREFUSED:
	e = ConnectionRefused;
	break;
    case ETIMEDOUT:
    case ENETUNREACH:
	e = NetworkFailure;
	break;
    case EADDRINUSE:
	e = NoResources;
	break;
    case EACCES:
    case EPERM:
	e = Inaccessible;
	break;
    case EAGAIN:
	// ignore that.  can retry.
	break;
    default:
	e = UnknownError;
	break;
    }
    return FALSE;
#else
    #error "This OS is not supported"
#endif
}

/*!
  Assigns a name to an unnamed socket.  If the operation succeeds,
  bind() returns TRUE.  Otherwise, it returns FALSE without changing
  what port() and address() return.

  bind() is used by servers for setting up incoming connections.
  Call bind() before listen().
*/

bool QSocketDevice::bind( const QHostAddress &address, uint port )
{
    if ( !isValid() )
	return FALSE;

    struct sockaddr_in a;
    memset( &a, 0, sizeof(a) );
    a.sin_family = AF_INET;
    a.sin_port = htons( port );
    a.sin_addr.s_addr = htonl( address.ip4Addr() );

    int r = ::bind( fd, (struct sockaddr*)&a,sizeof(struct sockaddr_in) );
    if ( r < 0 ) {
	switch( errno ) {
	case EINVAL:
	    e = AlreadyBound;
	    break;
	case EACCES:
	    e = Inaccessible;
	    break;
	case ENOMEM:
	    e = NoResources;
	    break;
	case EFAULT: // a was illegal
	case ENAMETOOLONG: // sz was wrong
	    e = Bug;
	    break;
	case EBADF: // AF_UNIX only
	case ENOTSOCK: // AF_UNIX only
	case EROFS: // AF_UNIX only
	case ENOENT: // AF_UNIX only
	case ENOTDIR: // AF_UNIX only
	case ELOOP: // AF_UNIX only
	    e = Impossible;
	    break;
	default:
	    e = UnknownError;
	    break;
	}
	return FALSE;
    }
    fetchConnectionParameters();
    return TRUE;
}

bool QSocketDevice::bind( const QString & filename )
{
    if ( !isValid() )
	return FALSE;

    struct sockaddr_un a;
    memset( &a, 0, sizeof(a) );
    a.sun_family = AF_UNIX;
    strncpy(a.sun_path, filename.local8Bit(),107);

    int r = ::bind( fd, (struct sockaddr*)&a,sizeof(struct sockaddr_un) );
    if ( r < 0 ) {
	switch( r ) {
	case EINVAL:
	    e = AlreadyBound;
	    break;
	case EACCES:
	    e = Inaccessible;
	    break;
	case ENOMEM:
	    e = NoResources;
	    break;
	case EFAULT: // a was illegal
	case ENAMETOOLONG: // sz was wrong
	    e = Bug;
	    break;
	case EBADF: // AF_UNIX only
	case ENOTSOCK: // AF_UNIX only
	case EROFS: // AF_UNIX only
	case ENOENT: // AF_UNIX only
	case ENOTDIR: // AF_UNIX only
	case ELOOP: // AF_UNIX only
	    e = Impossible;
	    break;
	default:
	    e = UnknownError;
	    break;
	}
	return FALSE;
    }
    fetchConnectionParameters();
    return TRUE;
}


/*!
  Specifies how many pending connections a server socket can have.
  Returns TRUE if the operation was successful, otherwise FALSE.

  The listen() call only applies to sockets where type() is \c Stream,
  not \c Datagram sockets.  listen() must not be called before bind()
  or after accept().  It is common to use a \a backlog value of 50 on
  most Unix systems.

  \sa bind(), accept()
*/

bool QSocketDevice::listen( int backlog )
{
    if ( !isValid() )
	return FALSE;
    if ( ::listen( fd, backlog ) >= 0 )
	return TRUE;
    if ( !e )
	e = Impossible;
    return FALSE;
}


/*!
  Extracts the first connection from the queue of pending connections
  for this socket and returns a new socket identifier.  Returns -1
  if the operation failed.

  \sa bind(), listen()
*/

int QSocketDevice::accept()
{
    if ( !isValid() )
	return FALSE;
    struct sockaddr a;
    SOCKLEN_T l = sizeof(struct sockaddr);
    int s = ::accept( fd, (struct sockaddr*)&a, &l );
    // we'll blithely throw away the stuff accept() wrote to a
    if ( s < 0 && e == NoError ) {
	switch( errno ) {
	case EPROTO:
	case ENOPROTOOPT:
	case EHOSTDOWN:
	case EOPNOTSUPP:
	case ENONET:
	case EHOSTUNREACH:
	case ENETDOWN:
	case ENETUNREACH:
	case ETIMEDOUT:
	    // in all these cases, an error happened during connection
	    // setup.  we're not interested in what happened, so we
	    // just treat it like the client-closed-quickly case.
	case EPERM:
	    // firewalling wouldn't let us accept.  we treat it like
	    // the client-closed-quickly case.
	case EAGAIN:
	    // the client closed the connection before we got around
	    // to accept()ing it.
	    break;
	case EBADF:
	case ENOTSOCK:
	    e = Impossible;
	    break;
	case EFAULT:
	    e = Bug;
	    break;
	case ENOMEM:
	case ENOBUFS:
	    e = NoResources;
	    break;
	default:
	    e = UnknownError;
	    break;
	}
    }
    return s;
}


/*!
  Returns the number of bytes available for reading, or -1 if an
  error occurred.

  \warning On Microsoft Windows, we use the ioctlsocket() function
  to determine the number of bytes queued on the socket.
  According to Microsoft (KB Q125486), ioctlsocket() sometimes
  return an incorrect number.  The only safe way to determine the
  amount of data on the socket is to read it using readBlock().
  QSocket has workarounds to deal with this problem.
*/

int QSocketDevice::bytesAvailable() const
{
    if ( !isValid() )
	return -1;
#if defined(_OS_WIN32_)
    u_long nbytes = 0;
    if ( ::ioctlsocket(fd, FIONREAD, &nbytes) < 0 )
	return -1;
    return nbytes;
#elif defined(_OS_UNIX_)
    // gives shorter than true amounts on Unix domain sockets.
    int nbytes = 0;
    if ( ::ioctl(fd, FIONREAD, (char*)&nbytes) < 0 )
	return -1;
    return nbytes;
#else
    #error "This OS is not supported"
#endif
}


/*!
  Wait upto \a msecs milliseconds for more data to be available.
  If \a msecs is -1 the call will block indefinitely.
  This is a blocking call and should be avoided in event driven
  applications.
  Returns the number of bytes available for reading, or -1 if an
  error occurred.
  \sa bytesAvailable()
*/
int QSocketDevice::waitForMore( int msecs )
{
    if ( !isValid() )
	return -1;

    fd_set fds;
    struct timeval tv;

    FD_ZERO( &fds );
    FD_SET( fd, &fds );

    tv.tv_sec = msecs / 1000;
    tv.tv_usec = (msecs % 1000) * 1000;

    int rv = select( fd+1, &fds, 0, 0, msecs < 0 ? 0 : &tv );

    if ( rv < 0 )
	return -1;

    return bytesAvailable();
}


/*!
  Reads max \a maxlen bytes from the socket into \a data and returns
  the number of bytes read.  Returns -1 if an error occurred.
*/

int QSocketDevice::readBlock( char *data, uint maxlen )
{
#if defined(CHECK_NULL)
    if ( data == 0 && maxlen != 0 ) {
	qWarning( "QSocketDevice::readBlock: Null pointer error" );
    }
#endif
#if defined(CHECK_STATE)
    if ( !isValid() ) {
	qWarning( "QSocketDevice::readBlock: Invalid socket" );
	return -1;
    }
    if ( !isOpen() ) {
	qWarning( "QSocketDevice::readBlock: Device is not open" );
	return -1;
    }
    if ( !isReadable() ) {
	qWarning( "QSocketDevice::readBlock: Read operation not permitted" );
	return -1;
    }
#endif
    bool done = FALSE;
    int r = 0;
    while ( done == FALSE ) {
	if ( t == Datagram ) {
	    struct sockaddr_in a;
	    memset( &a, 0, sizeof(a) );
	    SOCKLEN_T sz;
	    sz = sizeof( a );
	    r = ::recvfrom( fd, data, maxlen, 0,
			    (struct sockaddr *)&a, &sz );
	    pp = ntohs( a.sin_port );
	    pa = QHostAddress( ntohl( a.sin_addr.s_addr ) );
	} else {
#if defined(_OS_WIN32_)
	    r = ::recv( fd, data, maxlen, 0 );
#elif defined(_OS_UNIX_)
	    r = ::read( fd, data, maxlen );
#else
#error "This OS is not supported"
#endif
	}
	done = TRUE;
	if ( r >= 0 || errno == EAGAIN ) {
	    // nothing
	} else if ( errno == EINTR ) {
	    done = FALSE;
	} else if ( e == NoError ) {
	    switch( errno ) {
	    case EIO:
	    case EISDIR:
	    case EBADF:
	    case EINVAL:
	    case EFAULT:
	    case ENOTCONN:
	    case ENOTSOCK:
		e = Impossible;
		break;
	    case ENONET:
	    case EHOSTUNREACH:
	    case ENETDOWN:
	    case ENETUNREACH:
	    case ETIMEDOUT:
		e = NetworkFailure;
		break;
	    default:
		e = UnknownError;
		break;
	    }
	}
    }
    return r;
}


/*!
  Writes \a len bytes from the socket from \a data and returns
  the number of bytes written.  Returns -1 if an error occurred.
*/

int QSocketDevice::writeBlock( const char *data, uint len )
{
    if ( data == 0 && len != 0 ) {
#if defined(CHECK_NULL) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::writeBlock: Null pointer error" );
#endif
	return -1;
    }
    if ( !isValid() ) {
#if defined(CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::writeBlock: Invalid socket" );
#endif
	return -1;
    }
    if ( !isOpen() ) {
#if defined(CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::writeBlock: Device is not open" );
#endif
	return -1;
    }
    if ( !isWritable() ) {
#if defined(CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::writeBlock: Write operation not permitted" );
#endif
	return -1;
    }
    bool done = FALSE;
    int r = 0;
    while ( !done ) {
#if defined(_OS_WIN32_)
	r = ::send( fd, data, len, 0 );
#elif defined(_OS_UNIX_)
	r = ::write( fd, data, len );
#else
#error "This OS is not supported"
#endif
	done = TRUE;
	if ( r < 0 && e == NoError && errno != EAGAIN ) {
	    switch( errno ) {
	    case EINTR: // signal - call read() or whatever again
		done = FALSE;
		break;
	    case ENOSPC:
	    case EPIPE:
	    case EIO:
	    case EISDIR:
	    case EBADF:
	    case EINVAL:
	    case EFAULT:
	    case ENOTCONN:
	    case ENOTSOCK:
		e = Impossible;
		break;
	    case ENONET:
	    case EHOSTUNREACH:
	    case ENETDOWN:
	    case ENETUNREACH:
	    case ETIMEDOUT:
		e = NetworkFailure;
		break;
	    default:
		e = UnknownError;
		break;
	    }
	}
    }
    return r;
}


int QSocketDevice::writeBlock( const char * data, uint len,
			       const QHostAddress & host, uint port )
{
    if ( t != Datagram ) {
#if defined(CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::sendBlock: Not datagram" );
#endif
	return -1; // for now - later we can do t/tcp
    }

    if ( data == 0 && len != 0 ) {
#if defined(CHECK_NULL) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::sendBlock: Null pointer error" );
#endif
	return -1;
    }
    if ( !isValid() ) {
#if defined(CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::sendBlock: Invalid socket" );
#endif
	return -1;
    }
    if ( !isOpen() ) {
#if defined(CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::sendBlock: Device is not open" );
#endif
	return -1;
    }
    if ( !isWritable() ) {
#if defined(CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::sendBlock: Write operation not permitted" );
#endif
	return -1;
    }
#if defined(_OS_WIN32_) || defined(_OS_UNIX_)
    struct sockaddr_in a;
    memset( &a, 0, sizeof(a) );
    a.sin_family = AF_INET;
    a.sin_port = htons( port );
    a.sin_addr.s_addr = htonl( host.ip4Addr() );

    // we'd use MSG_DONTWAIT + MSG_NOSIGNAL if Stevens were right.
    // but apparently Stevens and most implementors disagree
    bool done = FALSE;
    int r = 0;
    while ( !done ) {
	r = ::sendto( fd, data, len, 0,
		      (struct sockaddr *)(&a), sizeof(sockaddr_in) );
	done = TRUE;
	if ( r < 0 && e != EAGAIN && e == NoError ) {
	    switch( errno ) {
	    case EINTR: // signal - call read() or whatever again
		done = FALSE;
		break;
	    case ENOSPC:
	    case EPIPE:
	    case EIO:
	    case EISDIR:
	    case EBADF:
	    case EINVAL:
	    case EFAULT:
	    case ENOTCONN:
	    case ENOTSOCK:
		e = Impossible;
		break;
	    case ENONET:
	    case EHOSTUNREACH:
	    case ENETDOWN:
	    case ENETUNREACH:
	    case ETIMEDOUT:
		e = NetworkFailure;
		break;
	    default:
		e = UnknownError;
		break;
	    }
	}
    }
#else
#error "This OS is not supported"
#endif
    return r;
}




/*!  Fetches information about both ends of the connection - whatever
  is available. */

void QSocketDevice::fetchConnectionParameters()
{
    if ( !isValid() ) {
	p = 0;
	a = QHostAddress();
	pp = 0;
	pa = QHostAddress();
	return;
    }
#if defined(_OS_UNIX_)
    struct sockaddr_in sa;
    memset( &sa, 0, sizeof(sa) );
    SOCKLEN_T sz;
    sz = sizeof( sa );
    if ( !::getsockname( fd, (struct sockaddr *)(&sa), &sz ) ) {
	p = ntohs( sa.sin_port );
	a = QHostAddress( ntohl( sa.sin_addr.s_addr ) );
    }
    if ( !::getpeername( fd, (struct sockaddr *)(&sa), &sz ) ) {
	pp = ntohs( sa.sin_port );
	pa = QHostAddress( ntohl( sa.sin_addr.s_addr ) );
    }
#else
    // uhm.
#endif
}
