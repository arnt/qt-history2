/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsocketdevice.cpp#4 $
**
** Implementation of QSocketDevice class
**
** Created : 990221
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qsocketdevice.h"
#include <string.h>

#if defined(_OS_WIN32_)
#include "qt_windows.h"
#endif

#if defined(UNIX)
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#endif


#if defined(_OS_WIN32_)
static void cleanupWinSock()
{
    winsock_init = FALSE;
    WSACleanup();
}

static bool initWinSock()
{
    static bool init = FALSE;
    if ( !init ) {
	init = TRUE;
	qAddPostRoutine( cleanupWinSock );
	WSAData wsadata;
	bool error = WSAStartup(MAKEWORD(1,1),&wsadata) != 0;
	if ( error ) {
#if defined(CHECK_NULL)
	    warning( "QSocketDevice: WinSock initialization failed" );
#endif
	    return FALSE;
	}
    }
    return TRUE;
}
#endif // _OS_WIN32_


#if defined(_OS_WIN32_)
/*!
  Windows only: This function initializes the winsock API.  You do not
  need to call this function if you use the standard QSocketDevice constructor
  which creates a socket for you.  Returns TRUE if the winsock API
  initialization was successful.
*/

bool QSocketDevice::initWinSock()
{
    return ::initWinSock();
}
#endif


/*!
  Creates a QSocketDevice object for a stream or datagram socket.

  The \a type argument must be either
  \c QSocketDevice::Stream for a reliable, connection-oriented TCP socket, or
  \c QSocketDevice::Datagram for an unreliable, connectionless UDP socket.

  The socket is made \link setNonblocking() nonblocking\endlink
  by default.
*/

QSocketDevice::QSocketDevice( Type type )
    : sock_fd(-1)
{
#if defined(_OS_WIN32_)
    ::initWinSock();
#endif
    int s;
    switch ( sock_type ) {			// create a socket
	case Stream:
	    s = ::socket( AF_INET, SOCK_STREAM, 0 );
	    break;
	case Datagram:
	    s = ::socket( AF_INET, SOCK_DGRAM, 0 );
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "QSocketDevice::QSocketDevice: Invalid socket type %d",
		     sock_type );
#endif
	    s = -1;
	    break;
    }
    if ( s != -1 ) {
	setSocket( s, type );
	setNonblocking( TRUE );
    }
}


/*!
  Creates a QSocketDevice object for an existing socket.

  The \a type argument must match the actual socket type;
  \c QSocketDevice::Stream for a reliable, connection-oriented TCP socket, or
  \c QSocketDevice::Datagram for an unreliable, connectionless UDP socket.
*/

QSocketDevice::QSocketDevice( int socket, Type type )
    : sock_fd(-1)
{
    setSocket( socket, type );
}


QSocketDevice::~QSocketDevice()
{
}


/*!
  \fn bool QSocketDevice::isValid() const

  Returns TRUE if this is a valid socket or FALSE if it is an invalid
  socket (socket() == -1).

  \sa socket()
*/

/*!
  \fn QSocketDevice::Type QSocketDevice::type() const

  Returns the socket type;
  \c QSocketDevice::Stream for a reliable, connection-oriented TCP socket, or
  \c QSocketDevice::Datagram for an unreliable, connectionless UDP socket.

  \sa socket()
*/

/*!
  \fn int QSocketDevice::socket() const

  Returns the socket number, or -1 if it is an invalid socket.

  \sa isValid(), type()
*/


/*!
  Sets an existing socket.

  The \a type argument must match the actual socket type;
  \c QSocketDevice::Stream for a reliable, connection-oriented TCP socket, or
  \c QSocketDevice::Datagram for an unreliable, connectionless UDP socket.

  If the previous socket is valid, this function will first call
  close() to close it.

  \sa isValid(), close()
*/

void QSocketDevice::setSocket( int socket, Type type )
{
    if ( sock_fd != -1 )			// close any open socket
	close();
    sock_type = type;
    sock_fd = socket;
    setFlags( IO_Sequential );
    setStatus( IO_Ok );
    open( IO_ReadWrite );
}


/*!
  Opens the socket using the specified QIODevice file mode.  This function
  is called from the QSocketDevice constructors and from the setSocket() function
  and you should not call it yourself.

  \sa close().
*/

bool QSocketDevice::open( int mode )
{
    if ( isOpen() || !isValid() )
	return FALSE;
    setMode( mode & IO_ReadWrite );
    setState( IO_Open );
    return TRUE;
}


/*!
  Closes the socket and sets the socket identifier to -1 (invalid).

  \sa open()
*/

void QSocketDevice::close()
{
    if ( sock_fd == -1 || !isOpen() )		// already closed
	return;
    setFlags( IO_Sequential );
    setStatus( IO_Ok );
#if defined(_OS_WIN32_)
    ::closesocket( sock_fd );
#else
    ::close( sock_fd );
#endif
    sock_fd = -1;
}


/*!
  Implementation of the abstract virtual QIODevice::flush() function.
  This implementation is a no-op.
*/

void QSocketDevice::flush()
{
}


/*!
  Implementation of the abstract virtual QIODevice::size() function.
  The size is meaningless for a socket, therefore this function returns 0.
*/

uint QSocketDevice::size() const
{
    return 0;
}


/*!
  Implementation of the abstract virtual QIODevice::at() function.
  The read/write index is meaningless for a socket, therefore
  this function returns 0.
*/

int QSocketDevice::at() const
{
    return 0;
}


/*!
  Implementation of the abstract virtual QIODevice::at(int) function.
  The read/write index is meaningless for a socket, therefore
  this function does nothing and returns TRUE.
*/

bool QSocketDevice::at( int )
{
    return TRUE;
}


/*!
  Implementation of the abstract virtual QIODevice::atEnd() function.
  The read/write index is meaningless for a socket, therefore
  this always returns FALSE.
*/

bool QSocketDevice::atEnd() const
{
    return FALSE;
}


/*!
  Returns TRUE if the socket is in nonblocking mode, or FALSE if it
  is in blocking mode or if the socket is invalid.

  \warning On Windows, this function always returns FALSE since the
  ioctlsocket() function is broken.

  \sa setNonblocking(), isValid()
*/

bool QSocketDevice::nonblocking() const
{
    if ( !isValid() )
	return FALSE;
#if defined(_OS_WIN32_)
    return FALSE;
#else
    int s = fcntl(sock_fd, F_GETFL, 0);
    return s >= 0 && ((s & FNDELAY) != 0);
#endif
}


/*!
  Makes the socket nonblocking if \a enable is TRUE or blocking if
  \a enable is FALSE.

  Sockets are blocking by default, but you are recommended to enable
  nonblocking socket operations, especially for GUI programs that need
  to be responsive.

  \warning On Windows, this function does nothing since the
  ioctlsocket() function is broken.  Whenever you use a QSocketDeviceNotifier
  on Windows, the socket is immediately made nonblocking automatically.

  \sa nonblocking(), isValid()
*/

void QSocketDevice::setNonblocking( bool enable )
{
    if ( !isValid() )
	return;
#if defined(_OS_WIN32_)
    // Do nothing
#else
    int s = fcntl(sock_fd, F_GETFL, 0);
    if ( s >= 0 ) {
	if ( enable )
	    s |= FNDELAY;
	else
	    s &= ~FNDELAY;
	fcntl( sock_fd, F_SETFL, s );
    }
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
	case Debug:
	    n = SO_DEBUG;
	    break;
	case DontRoute:
	    n = SO_DONTROUTE;
	    break;
	case KeepAlive:
	    n = SO_KEEPALIVE;
	    break;
	case Linger:
	    n = SO_LINGER;
	    break;
	case OobInline:
	    n = SO_OOBINLINE;
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
	if ( n == SO_LINGER ) {			// special handling for linger
	    struct linger l;
	    int len = sizeof(l);
	    ::getsockopt( sock_fd, SOL_SOCKET, n, (char*)&l, (int*)&len );
	    if ( l.l_onoff )
		v = l.l_linger;
	    else
		v = -2;
	} else {
	    int len = sizeof(v);
	    if ( ::getsockopt(sock_fd,SOL_SOCKET,n,(char*)&v,(int*)&len) < 0 )
		return -1;			// error
	}
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
    int n = -1;
    switch ( opt ) {
	case Broadcast:
	    n = SO_BROADCAST;
	    break;
	case Debug:
	    n = SO_DEBUG;
	    break;
	case DontRoute:
	    n = SO_DONTROUTE;
	    break;
	case KeepAlive:
	    n = SO_KEEPALIVE;
	    break;
	case Linger:
	    n = SO_LINGER;
	    break;
	case OobInline:
	    n = SO_OOBINLINE;
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
	if ( n == SO_LINGER ) {			// special handling for linger
	    struct linger l;
	    if ( v >= 0 ) {
		l.l_onoff = TRUE;
		l.l_linger = v;
	    } else {
		l.l_onoff = FALSE;
		l.l_linger = 0;
	    }
	    ::setsockopt( sock_fd, SOL_SOCKET, n, (char*)&l, sizeof(l));
	} else {
	    ::setsockopt( sock_fd, SOL_SOCKET, n, (char*)&v, sizeof(v));
	}
    }
}


/*!
  ### TODO: Documentation.
*/

bool QSocketDevice::connect( const QSocketAddress &addr )
{
    if ( !isValid() )
	return FALSE;
    return ::connect(sock_fd, (struct sockaddr*)addr.data(),
		     addr.length()) == 0;
}


/*!
  Assigns a name to an unnamed socket.  Returns TRUE if the operation
  was successful, otherwise FALSE.

  bind() is used by servers for setting up incoming connections.
  Call bind() before listen(). 
*/

bool QSocketDevice::bind( const QSocketAddress &name )
{
    if ( !isValid() )
	return FALSE;
    return ::bind(sock_fd, (struct sockaddr*)name.data(),
		  name.length()) == 0;
}


/*!
  Specifies how many pending connections a server socket can have.
  Returns TRUE if the operation was successful, otherwise FALSE.

  The listen() call only applies to sockets of \link setType()
  type\endlink \c Stream, not \c Datagram sockets.  listen() must be
  called after bind() and before accept(). It is common to use a
  \a backlog value of 50 on most Unix systems.

  \sa bind(), accept()
*/

bool QSocketDevice::listen( int backlog )
{
    if ( !isValid() )
	return FALSE;
    return ::listen(sock_fd, backlog);
}


/*!
  Extracts the first connection from the queue of pending connections
  for this socket and returns a new socket identifier.  Returns -1
  if the operation failed.

  Returns the address of the connecting entity in \a addr.

  \sa bind(), listen()
*/

int QSocketDevice::accept( QSocketAddress *addr )
{
    if ( !isValid() )
	return FALSE;
    struct sockaddr a;
    int l = sizeof(struct sockaddr);
    int s = ::accept( sock_fd, (struct sockaddr*)&a, &l );
    addr->setData( &a, l );
    return s;
}


/*!
  Returns the number of bytes available for reading, or -1 if an
  error occurred.
*/

int QSocketDevice::bytesAvailable() const
{
    if ( !isValid() )
	return -1;
#if defined(UNIX)
    int nbytes = 0;
    if ( ::ioctl(sock_fd, FIONREAD, (char*)&nbytes) < 0 )
	return -1;
    return nbytes;
#endif
#if defined(_OS_WIN32_)
    u_long nbytes;
    if ( ::ioctlsocket(sock_fd, FIONREAD, &nbytes) < 0 )
	return -1;
    return nbytes;
#endif
}


/*!
  Reads max \a maxlen bytes from the socket into \a data and returns
  the number of bytes read.  Returns -1 if an error occurred.
*/

int QSocketDevice::readBlock( char *data, uint maxlen )
{
    if ( data == 0 && maxlen != 0 ) {
#if defined(CHECK_NULL)
	warning( "QSocketDevice::readBlock: Null pointer error" );
#endif
	return -1;
    }
#if defined(CHECK_STATE)
    if ( !isValid() ) {
	warning( "QSocketDevice::readBlock: Invalid socket" );
	return -1;
    }
    if ( !isOpen() ) {
	warning( "QSocketDevice::readBlock: Device is not open" );
	return -1;
    }
    if ( !isReadable() ) {
	warning( "QSocketDevice::readBlock: Read operation not permitted" );
	return -1;
    }
#else
    if ( !isValid() || !isOpen() || !isReadable() )
	return -1;
#endif
#if defined(_OS_WIN32_)
    return ::recv( sock_fd, data, maxlen, 0 );
#endif
#if defined(UNIX)
    return ::read( sock_fd, data, maxlen );
#endif
}


/*!
  Writes \a len bytes from the socket from \a data and returns
  the number of bytes written.  Returns -1 if an error occurred.
*/

int QSocketDevice::writeBlock( const char *data, uint len )
{
    if ( data == 0 && len != 0 ) {
#if defined(CHECK_NULL)
	warning( "QSocketDevice::writeBlock: Null pointer error" );
#endif
	return -1;
    }
#if defined(CHECK_STATE)
    if ( !isValid() ) {
	warning( "QSocketDevice::writeBlock: Invalid socket" );
	return -1;
    }
    if ( !isOpen() ) {
	warning( "QSocketDevice::writeBlock: Device is not open" );
	return -1;
    }
    if ( !isWritable() ) {
	warning( "QSocketDevice::writeBlock: Write operation not permitted" );
	return -1;
    }
#else
    if ( !isValid() || !isOpen() || !isWritable() )
	return -1;
#endif
#if defined(_OS_WIN32_)
    return ::send( sock_fd, data, len, 0 );
#endif
#if defined(UNIX)
    return ::write( sock_fd, data, len );
#endif
}


/*****************************************************************************
  QSocketAddress member functions
 *****************************************************************************/

void QSocketAddress::setData( void *data, int size )
{
    if ( ptr )
	delete [] ptr;
    ptr = new char[size];
    len = size;
    memcpy( ptr, data, len );
}


QSocketAddress::QSocketAddress()
    : ptr(0), len(0)
{
    struct sockaddr_in a;
    memset( &a, 0, sizeof(a) );
    a.sin_family = AF_INET;
    setData( &a, sizeof(a) );
}


QSocketAddress::QSocketAddress( int port, uint ip4Addr )
    : ptr(0)
{
    struct sockaddr_in a;
    memset( &a, 0, sizeof(a) );
    a.sin_family = AF_INET;
    a.sin_port = ntohs( port );
    a.sin_addr.s_addr = ntohl( ip4Addr );
    setData( &a, sizeof(a) );
}


QSocketAddress::QSocketAddress( const QSocketAddress &a )
    : ptr(0)
{
    setData( a.ptr, a.len );
}


QSocketAddress::~QSocketAddress()
{
    if ( ptr )
	delete [] ptr;
}


QSocketAddress & QSocketAddress::operator=( const QSocketAddress &a )
{
    setData( a.ptr, a.len );
    return *this;
}


int QSocketAddress::port() const
{
    return htons(((struct sockaddr_in*)ptr)->sin_port);
}


uint QSocketAddress::ip4Addr() const
{
    return htonl(((struct sockaddr_in*)ptr)->sin_addr.s_addr);
}


bool QSocketAddress::operator==( const QSocketAddress &a )
{
    if ( a.len != len )
	return FALSE;
    return memcmp(ptr,a.ptr,len) == 0;
}
