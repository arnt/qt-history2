/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qsocketdevice.cpp#6 $
**
** Implementation of Network Extension Library
**
** Created : 970521
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
#include <sys/file.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#endif


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

// caldera 1.3, for example
#define SOCKLEN_T int

#endif



//#define QSOCKETDEVICE_DEBUG



/*!
  \class QSocketDevice qsocketdevice.h
  \brief QSocketDevice provides a platform-independent low-level socket API.

  \ingroup kernel

  This class is not really meant for use outside Qt.  It can be used
  for to achieve some things that QSocket does not provide, but it's
  not particularly easy to understand or use.

  The basic purpose of the class is to provide a QIODevice that works
  on sockets.  As such, it reimplements the

  \sa QSocket, QSocketNotifier, QHostAddress
*/


static void cleanupWinSock() // post-routine
{
#if defined(_OS_WIN32_)
    WSACleanup();
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice: WinSock cleanup" );
#endif
#endif
}

static void initWinSock()
{
#if defined(_OS_WIN32_)
    // ### NOTE if it's harmful to call WSAStartup several times Qt
    // and the application program may conflict with each other.
    static bool init = FALSE;
    if ( !init ) {
	WSAData wsadata;
	bool error = WSAStartup(MAKEWORD(1,1),&wsadata) != 0;
	if ( error ) {
#if defined(CHECK_NULL)
	    qWarning( "QSocketDevice: WinSock initialization failed" );
#endif
	    return;
	}
	qAddPostRoutine( cleanupWinSock );
#if defined(QSOCKETDEVICE_DEBUG)
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

QSocketDevice::QSocketDevice( Type type )
    : sock_fd(-1)
{
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice: Created QSocketDevice object %p, type %d",
	    this, type );
#endif
    initWinSock();
    setSocket( ::socket( AF_INET, type==Datagram?SOCK_DGRAM:SOCK_STREAM, 0 ),
	       type );
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
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice: Created QSocketDevice %p (socket %x, type %d)",
	   this, socket, type );
#endif
    initWinSock();
    setSocket( socket, type );
}


/*!
  Destroys the socket device and closes the socket if it is open.
*/

QSocketDevice::~QSocketDevice()
{
    close();
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice: Destroyed QSocketDevice %p", this );
#endif
}


/*!
  \fn bool QSocketDevice::isValid() const

  Returns TRUE if this is a valid socket or FALSE if it is an invalid
  socket.  This is actually a shortcut for socket() == -1.

  \sa socket()
*/

/*!
  \fn QSocketDevice::Type QSocketDevice::type() const

  Returns the socket type; \c QSocketDevice::Stream for a reliable,
  connection-oriented TCP socket, or \c QSocketDevice::Datagram for an
  unreliable UDP socket.

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

  Any existing socket is closed.

  \sa isValid(), close()
*/

void QSocketDevice::setSocket( int socket, Type type )
{
    if ( sock_fd != -1 )			// close any open socket
	close();
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::setSocket: socket %x, type %d", socket, type );
#endif
    sock_type = type;
    sock_fd = socket;
    setFlags( IO_Sequential );
    setStatus( IO_Ok );
    open( IO_ReadWrite );
}


/*! \reimp

  Opens the socket using the specified QIODevice file \a mode.  This function
  is called from the QSocketDevice constructors and from the setSocket()
  function.  You should not call it yourself.

  \sa close().
*/

bool QSocketDevice::open( int mode )
{
    if ( isOpen() || !isValid() )
	return FALSE;
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::open: mode %x", mode );
#endif
    setMode( mode & IO_ReadWrite );
    setState( IO_Open );
    return TRUE;
}


/*! \reimp

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
#elif defined(UNIX)
    ::close( sock_fd );
#else
    #error "This OS is not supported"
#endif
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::close: Closed socket %x", sock_fd );
#endif
    sock_fd = -1;
}


/*! \reimp

  QSocket does not buffer at all, so this is a no-op.
*/

void QSocketDevice::flush()
{
}


/*! \reimp

  The size is meaningless for a socket, therefore this function returns 0.
*/

uint QSocketDevice::size() const
{
    return 0;
}


/*! \reimp

  The read/write index is meaningless for a socket, therefore
  this function returns 0.
*/

int QSocketDevice::at() const
{
    return 0;
}


/*! \reimp

  The read/write index is meaningless for a socket, therefore
  this function does nothing and returns TRUE.
*/

bool QSocketDevice::at( int )
{
    return TRUE;
}


/*! \reimp

  The read/write index is meaningless for a socket, therefore
  this always returns FALSE.
*/

bool QSocketDevice::atEnd() const
{
    return FALSE;
}


/*!
  Returns TRUE if the socket is in blocking mode, or FALSE if it
  is in nonblocking mode or if the socket is invalid.

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
#elif defined(UNIX)
    int s = fcntl(sock_fd, F_GETFL, 0);
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
  ioctlsocket() function is broken.  Whenever you use a QSocketDeviceNotifier
  on Windows, the socket is immediately made nonblocking automatically.

  \sa blocking(), isValid()
*/

void QSocketDevice::setBlocking( bool enable )
{
    if ( !isValid() )
	return;
#if defined(_OS_WIN32_)
    // Do nothing
#elif defined(UNIX)
    int tmp = ::fcntl(sock_fd, F_GETFL, 0);
    if ( tmp >= 0 )
	fcntl( sock_fd, F_SETFL, enable ? (tmp&!FNDELAY) : (tmp|FNDELAY) );
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
	if ( ::getsockopt( sock_fd, SOL_SOCKET, n, (void*)&v, &len ) < 0 )
	    return -1;			// error
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
    ::setsockopt( sock_fd, SOL_SOCKET, n, (char*)&v, sizeof(v));
}


/*!
  Connects to the IP address and port specified by \a addr.
  Returns FALSE if a connection could not be established.
*/

bool QSocketDevice::connect( const QHostAddress &addr, uint port )
{
    if ( !isValid() )
	return FALSE;

    struct sockaddr_in a;
    memset( &a, 0, sizeof(a) );
    a.sin_family = AF_INET;
    a.sin_port = ntohs( port );
    a.sin_addr.s_addr = ntohl( addr.ip4Addr() );

    int r = ::connect( sock_fd, (struct sockaddr*)&a,
		       sizeof(struct sockaddr_in) );
#if defined(_OS_WIN32_)
    return r != SOCKET_ERROR;
#elif defined(UNIX)
    return r == 0 || errno == EISCONN ||
       errno == EWOULDBLOCK || errno == EINPROGRESS;
#else
    #error "This OS is not supported"
#endif
}


/*!
  Assigns a name to an unnamed socket.  Returns TRUE if the operation
  was successful, otherwise FALSE.

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
    a.sin_port = ntohs( port );
    a.sin_addr.s_addr = ntohl( address.ip4Addr() );

    return ::bind( sock_fd, (struct sockaddr*)&a,
		   sizeof(struct sockaddr_in) ) == 0;
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
    return ::listen( sock_fd, backlog );
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
    int s = ::accept( sock_fd, (struct sockaddr*)&a, &l );
    // we'll blithely throw away the stuff accept() wrote to a
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
#if defined(_OS_WIN32_)
    u_long nbytes = 0;
    if ( ::ioctlsocket(sock_fd, FIONREAD, &nbytes) < 0 )
	return -1;
    return nbytes;
#elif defined(UNIX)
    int nbytes = 0;
    if ( ::ioctl(sock_fd, FIONREAD, (char*)&nbytes) < 0 )
	return -1;
    return nbytes;
#else
    #error "This OS is not supported"
#endif
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
#if defined(_OS_WIN32_)
    return ::recv( sock_fd, data, maxlen, 0 );
#elif defined(UNIX)
    if ( sock_type == Datagram ) {
	// ### need to keep the source of the data here
	return ::recvfrom( sock_fd, data, maxlen, 0, 0, 0 );
    }
    return ::read( sock_fd, data, maxlen );
#else
    #error "This OS is not supported"
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
	qWarning( "QSocketDevice::writeBlock: Null pointer error" );
#endif
	return -1;
    }
    if ( !isValid() ) {
#if defined(CHECK_STATE)
	qWarning( "QSocketDevice::writeBlock: Invalid socket" );
#endif
	return -1;
    }
    if ( !isOpen() ) {
#if defined(CHECK_STATE)
	qWarning( "QSocketDevice::writeBlock: Device is not open" );
#endif
	return -1;
    }
    if ( !isWritable() ) {
#if defined(CHECK_STATE)
	qWarning( "QSocketDevice::writeBlock: Write operation not permitted" );
#endif
	return -1;
    }
#if defined(_OS_WIN32_)
    return ::send( sock_fd, data, len, 0 );
#elif defined(UNIX)
    return ::write( sock_fd, data, len );
#else
    #error "This OS is not supported"
#endif
}


int QSocketDevice::writeBlock( const char * data, uint len,
			       const QHostAddress & host, uint port )
{
    if ( sock_type != Datagram ) {
#if defined(CHECK_STATE)
	qWarning( "QSocketDevice::sendBlock: Not datagram" );
#endif
	return -1; // for now - later we can do t/tcp
    }
	
    if ( data == 0 && len != 0 ) {
#if defined(CHECK_NULL)
	qWarning( "QSocketDevice::sendBlock: Null pointer error" );
#endif
	return -1;
    }
    if ( !isValid() ) {
#if defined(CHECK_STATE)
	qWarning( "QSocketDevice::sendBlock: Invalid socket" );
#endif
	return -1;
    }
    if ( !isOpen() ) {
#if defined(CHECK_STATE)
	qWarning( "QSocketDevice::sendBlock: Device is not open" );
#endif
	return -1;
    }
    if ( !isWritable() ) {
#if defined(CHECK_STATE)
	qWarning( "QSocketDevice::sendBlock: Write operation not permitted" );
#endif
	return -1;
    }
#if defined(_OS_WIN32_)
#warningerror "This OS is not supported"
    // haavard?
#elif defined(UNIX)
    struct sockaddr_in a;
    memset( &a, 0, sizeof(a) );
    a.sin_family = AF_INET;
    a.sin_port = ntohs( port );
    a.sin_addr.s_addr = ntohl( host.ip4Addr() );

    // we'd use MSG_DONTWAIT + MSG_NOSIGNAL if Stevens were right.
    // but apparently Stevens and whoever wrote glibc disagree.
    return ::sendto( sock_fd, data, len, 0,
		     (struct sockaddr *)(&a), sizeof(sockaddr_in) );
#else
#error "This OS is not supported"
#endif
    
}


/*! \reimp

  \warning getch() is implemented as a one-byte readBlock(), so it
  may be very slow if you call it more than a few times.

  \sa putch() readBlock()
*/

int QSocketDevice::getch()
{
    char buf[2];
    return  readBlock(buf,1) == 1 ? buf[0] : -1;
}


/*! \reimp

  \warning putch() is implemented as a one-byte writeBlock(), so it
  may be very slow if you call it more than a few times.

  \sa getch()
*/

int QSocketDevice::putch( int ch )
{
    char buf[2];
    buf[0] = ch;
    return writeBlock(buf, 1) == 1 ? ch : -1;
}


/*! \reimp

  This implementation of ungetch -1 (error).  A socket is a sequential
  device and does not allow any ungetch operation.
*/

int QSocketDevice::ungetch( int )
{
    return -1;
}


/*!  Returns TRUE if the address of this socket can be used by other
  sockets at the same time, and FALSE if this socket claims exclusive
  ownership.

  \sa setAddressReusable()
*/

bool QSocketDevice::addressReusable() const
{
    return option( ReuseAddress );
}


/*!  Sets the address of this socket to be usable by other sockets too
  if \a enable is TRYE, and to be used exclusively by this socket if
  \a enable is FALSE.

  When a socket is reusable, other sockets can use the same port
  number (and IP address), which is in general good.  Of course other
  sockets cannot use the same (address,port,peer-address,peer-port)
  4-tuple as this socket, so there is no risk of confusing the two TCP
  connections.

  \sa addressReusable()
*/

void QSocketDevice::setAddressReusable( bool enable )
{
    setOption( ReuseAddress, enable );
}


/*!  Returns the size of the OS receive buffer.

  \sa setReceiveBufferSize()
*/

int QSocketDevice::receiveBufferSize() const
{
    return option( ReceiveBuffer );
}


/*!  Sets the size of the OS receive buffer to \a size.

  The OS receive buffer size effectively limits two things: How much
  data can be in transit at any one moment, and how much data can be
  received in one iteration of the main event loop.

  The default is OS-dependent.  A socket that receives large amounts
  of data is probably best off with a buffer size of 49152.
*/

void QSocketDevice::setReceiveBufferSize( uint size )
{
    setOption( ReceiveBuffer, size );
}


/*!  Returns the size of the OS send buffer.

  \sa setSendBufferSize()
*/

int QSocketDevice::sendBufferSize() const
{
    return option( SendBuffer );
}


/*!  Sets the size of the OS send buffer to \a size.

  The OS send buffer size effectively limits how much data can be in
  transit at any one moment.

  The default is OS-dependent.  A socket that sends large amounts of
  data is probably best off with a buffer size of 49152.
*/

void QSocketDevice::setSendBufferSize( uint size )
{
    setOption( SendBuffer, size );
}
