/****************************************************************************
** $Id$
**
** Implementation of QSocketDevice class.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition licenses for Windows
** may use this file in accordance with the Qt Commercial License Agreement
** provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qsocketdevice.h"
#include "qwindowdefs.h"
#include "qdatetime.h"

#include <string.h>
#include <windows.h>
#include <winsock.h>
#ifndef NO_ERRNO_H
#include <errno.h>
#endif


#if defined(SOCKLEN_T)
#undef SOCKLEN_T
#endif

#define SOCKLEN_T int // #### Winsock 1.1

static bool initialized = FALSE;

static void cleanupWinSock() // post-routine
{
    WSACleanup();
    initialized = FALSE;
}


void QSocketDevice::init()
{
    if ( !initialized ) {
	WSAData wsadata;
	if ( WSAStartup( MAKEWORD(1,1), &wsadata ) != 0 ) {
#if defined(QT_CHECK_NULL)
	    qWarning( "QSocketDevice: WinSock initialization failed" );
#endif
#if defined(QSOCKETDEVICE_DEBUG)
	    qDebug( "QSocketDevice: WinSock initialization failed"  );
#endif
	    return;
	}
	qAddPostRoutine( cleanupWinSock );
#if defined(QSOCKETDEVICE_DEBUG)
	qDebug( "QSocketDevice: WinSock initialization OK" );
#endif
	initialized = TRUE;
    }
}

int QSocketDevice::createNewSocket( )
{
    int s = ::socket( AF_INET, t==Datagram?SOCK_DGRAM:SOCK_STREAM, 0 );
    if ( s == INVALID_SOCKET ) {
	switch( WSAGetLastError() ) {
	    case WSANOTINITIALISED:
		e = Impossible;
		break;
	    case WSAENETDOWN:
		// ### what to use here?
		e = NetworkFailure;
		//e = Inaccessible;
		break;
	    case WSAEMFILE:
		e = NoFiles; // special case for this
		break;
	    case WSAEINPROGRESS:
	    case WSAENOBUFS:
		e = NoResources;
		break;
	    case WSAEAFNOSUPPORT:
	    case WSAEPROTOTYPE:
	    case WSAEPROTONOSUPPORT:
	    case WSAESOCKTNOSUPPORT:
		e = Bug;
		break;
	    default:
		e = UnknownError;
		break;
	}
    } else {
	return s;
    }
    return -1;
}


void QSocketDevice::close()
{
    if ( fd == -1 || !isOpen() )		// already closed
	return;
    setFlags( IO_Sequential );
    setStatus( IO_Ok );
    ::closesocket( fd ); // ### do we need error handling here?
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::close: Closed socket %x", fd );
#endif
    fd = -1;
    fetchConnectionParameters();
}


bool QSocketDevice::blocking() const
{
    if ( !isValid() )
	return TRUE;
    return TRUE;
}


void QSocketDevice::setBlocking( bool enable )
{
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug( "QSocketDevice::setBlocking( %d )", enable );
#endif
    if ( !isValid() )
	return;

    // ### error handling?
    unsigned long dummy;
    ioctlsocket( fd, FIONBIO, (enable?0:&dummy) );
}


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
	if ( r != SOCKET_ERROR )
	    return v;
	if ( !e ) {
            QSocketDevice *that = (QSocketDevice*)this; // mutable function
 	    switch( WSAGetLastError() ) {
		case WSANOTINITIALISED:
		    that->e = Impossible;
		    break;
		case WSAENETDOWN:
		    that->e = NetworkFailure;
		    break;
		case WSAEFAULT:
		case WSAEINVAL:
		case WSAENOPROTOOPT:
		    that->e = Bug;
		    break;
		case WSAEINPROGRESS:
		    that->e = NoResources;
		    break;
		case WSAENOTSOCK:
		    that->e = Impossible;
		    break;
		default:
		    that->e = UnknownError;
		    break;
	    }
	}
	return -1;
    }
    return v;
}


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
    int r = ::setsockopt( fd, SOL_SOCKET, n, (char*)&v, sizeof(v) );
    if ( r == SOCKET_ERROR && e == NoError ) {
	switch( WSAGetLastError() ) {
	    case WSANOTINITIALISED:
		e = Impossible;
		break;
	    case WSAENETDOWN:
		e = NetworkFailure;
		break;
	    case WSAEFAULT:
	    case WSAEINVAL:
	    case WSAENOPROTOOPT:
		e = Bug;
		break;
	    case WSAEINPROGRESS:
		e = NoResources;
		break;
	    case WSAENETRESET:
	    case WSAENOTCONN:
		e =  Impossible; // ### ?
		break;
	    case WSAENOTSOCK:
		e = Impossible;
		break;
	    default:
		e = UnknownError;
		break;
	}
    }
}


bool QSocketDevice::connect( const QHostAddress &addr, Q_UINT16 port )
{
    if ( !isValid() )
	return FALSE;

    if ( !addr.isIp4Addr() ) {
	qWarning( "QSocketDevice: IPv6 is not supported by this version" );
	e = Impossible;
	return FALSE;
    }
    struct sockaddr_in a;
    memset( &a, 0, sizeof(a) );
    a.sin_family = AF_INET;
    a.sin_port = htons( port );
    a.sin_addr.s_addr = htonl( addr.ip4Addr() );

    int r = ::connect( fd, (struct sockaddr*)&a, sizeof(struct sockaddr_in) );
    if ( r == SOCKET_ERROR )
    {
	switch( WSAGetLastError() ) {
	    case WSANOTINITIALISED:
		e = Impossible;
		break;
	    case WSAENETDOWN:
		e = NetworkFailure;
		break;
	    case WSAEADDRINUSE:
	    case WSAEINPROGRESS:
	    case WSAENOBUFS:
		e = NoResources;
		break;
	    case WSAEINTR:
		e = UnknownError; // ### ?
		break;
	    case WSAEALREADY:
		// ### ?
		break;
	    case WSAEADDRNOTAVAIL:
		e = ConnectionRefused; // ### ?
		break;
	    case WSAEAFNOSUPPORT:
	    case WSAEFAULT:
		e = Bug;
		break;
	    case WSAEINVAL:
		// I experienced that this should be not an error situation.
		break;
	    case WSAECONNREFUSED:
		e = ConnectionRefused;
		break;
	    case WSAEISCONN:
		//e = Impossible; // ### ?
		//break;
		goto successful;
	    case WSAENETUNREACH:
	    case WSAETIMEDOUT:
		e = NetworkFailure;
		break;
	    case WSAENOTSOCK:
		e = Impossible;
		break;
	    case WSAEWOULDBLOCK:
		// ### ???
		break;
	    case WSAEACCES:
		e = Inaccessible;
		break;
	    case 10107:
		// Workaround for a problem with the WinSock Proxy Server. See
		// also support/arc-12/25557 for details on the problem.
		goto successful;
	    default:
		e = UnknownError;
		break;
	}
	return FALSE;
    }
successful:
    fetchConnectionParameters();
    return TRUE;
}


bool QSocketDevice::bind( const QHostAddress &address, Q_UINT16 port )
{
    if ( !isValid() )
	return FALSE;

    if ( !address.isIp4Addr() ) {
	qWarning( "QSocketDevice: IPv6 is not supported by this version" );
	e = Impossible;
	return FALSE;
    }
    struct sockaddr_in a;
    memset( &a, 0, sizeof(a) );
    a.sin_family = AF_INET;
    a.sin_port = htons( port );
    a.sin_addr.s_addr = htonl( address.ip4Addr() );

    int r = ::bind( fd, (struct sockaddr*)&a,sizeof(struct sockaddr_in) );
    if ( r == SOCKET_ERROR ) {
	switch( WSAGetLastError() ) {
	    case WSANOTINITIALISED:
		e = Impossible;
		break;
	    case WSAENETDOWN:
		e = NetworkFailure;
		break;
	    case WSAEACCES:
		e = Inaccessible;
		break;
	    case WSAEADDRNOTAVAIL:
		e = Inaccessible;
		break;
	    case WSAEFAULT:
		e = Bug;
		break;
	    case WSAEINPROGRESS:
	    case WSAENOBUFS:
		e = NoResources;
		break;
	    case WSAEADDRINUSE:
	    case WSAEINVAL:
		e = AlreadyBound;
		break;
	    case WSAENOTSOCK:
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


int QSocketDevice::accept()
{
    if ( !isValid() )
	return FALSE;
    struct sockaddr a;
    SOCKLEN_T l = sizeof(struct sockaddr);
    int s = ::accept( fd, (struct sockaddr*)&a, &l );
    // we'll blithely throw away the stuff accept() wrote to a
    if ( s == INVALID_SOCKET && e == NoError ) {
	switch( WSAGetLastError() ) {
	    case WSANOTINITIALISED:
		e = Impossible;
		break;
	    case WSAENETDOWN:
	    case WSAEOPNOTSUPP:
		// in all these cases, an error happened during connection
		// setup.  we're not interested in what happened, so we
		// just treat it like the client-closed-quickly case.
		break;
	    case WSAEFAULT:
		e = Bug;
		break;
	    case WSAEINTR:
		// ### ?
		break;
	    case WSAEMFILE:
	    case WSAEINPROGRESS:
	    case WSAENOBUFS:
		e = NoResources;
		break;
	    case WSAEINVAL:
	    case WSAENOTSOCK:
		e = Impossible;
		break;
	    case WSAEWOULDBLOCK:
		// ### ?
		break;
	    default:
		e = UnknownError;
		break;
	}
    }
    return s;
}


Q_LONG QSocketDevice::bytesAvailable() const
{
    if ( !isValid() )
	return -1;
    u_long nbytes = 0;
    if ( ::ioctlsocket(fd, FIONREAD, &nbytes) < 0 )
	return -1;
    return nbytes;
}


Q_LONG QSocketDevice::waitForMore( int msecs, bool *timeout ) const
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

    if ( timeout ) {
	if ( rv == 0 )
	    *timeout = TRUE;
	else
	    *timeout = FALSE;
    }

    return bytesAvailable();
}


Q_LONG QSocketDevice::readBlock( char *data, Q_ULONG maxlen )
{
#if defined(QT_CHECK_NULL)
    if ( data == 0 && maxlen != 0 ) {
	qWarning( "QSocketDevice::readBlock: Null pointer error" );
    }
#endif
#if defined(QT_CHECK_STATE)
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
    Q_LONG r = 0;
    while ( done == FALSE ) {
	if ( t == Datagram ) {
	    struct sockaddr_in a;
	    memset( &a, 0, sizeof(a) );
	    SOCKLEN_T sz;
	    sz = sizeof( a );
	    r = ::recvfrom( fd, data, maxlen, 0, (struct sockaddr *)&a, &sz );
	    pp = ntohs( a.sin_port );
	    pa = QHostAddress( ntohl( a.sin_addr.s_addr ) );
	} else {
	    r = ::recv( fd, data, maxlen, 0 );
	}
	done = TRUE;
#if 0
	if ( r >= 0 || errno == EAGAIN ) {
	    // nothing
	} else if ( errno == EINTR ) {
	    done = FALSE;
	} else if ( e == NoError ) {
#endif
	if ( r == SOCKET_ERROR && e == NoError ) {
	    switch( WSAGetLastError() ) {
		case WSANOTINITIALISED:
		    e = Impossible;
		    break;
		case WSAENETDOWN:
		case WSAENETRESET:
		case WSAECONNABORTED:
		case WSAETIMEDOUT:
		case WSAECONNRESET:
		    e = NetworkFailure;
		    break;
		case WSAEFAULT:
		case WSAENOTCONN:
		case WSAESHUTDOWN:
		case WSAEINVAL:
		    e = Impossible;
		    break;
		case WSAEINTR:
		    // ### ?
		    break;
		case WSAEINPROGRESS:
		    e = NoResources;
		    break;
		case WSAENOTSOCK:
		    e = Impossible;
		    break;
		case WSAEOPNOTSUPP:
		    e = Bug; // ### ?
		    break;
		case WSAEWOULDBLOCK:
		    // ### ???
		    break;
		case WSAEMSGSIZE:
		    e = NoResources; // ### ?
		    break;
		case WSAEISCONN:
		    // ### ?
		    break;
		default:
		    e = UnknownError;
		    break;
	    }
	}
    }
    return r;
}


Q_LONG QSocketDevice::writeBlock( const char *data, Q_ULONG len )
{
    if ( data == 0 && len != 0 ) {
#if defined(QT_CHECK_NULL) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::writeBlock: Null pointer error" );
#endif
	return -1;
    }
    if ( !isValid() ) {
#if defined(QT_CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::writeBlock: Invalid socket" );
#endif
	return -1;
    }
    if ( !isOpen() ) {
#if defined(QT_CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::writeBlock: Device is not open" );
#endif
	return -1;
    }
    if ( !isWritable() ) {
#if defined(QT_CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::writeBlock: Write operation not permitted" );
#endif
	return -1;
    }
    bool done = FALSE;
    Q_LONG r = 0;
    while ( !done ) {
	// Don't write more than 64K (see Knowledge Base Q201213).
	r = ::send( fd, data, ( len>64*1024 ? 64*1024 : len ), 0 );
	done = TRUE;
	if ( r == SOCKET_ERROR && e == NoError ) {//&& errno != WSAEAGAIN ) {
	    switch( WSAGetLastError() ) {
		case WSANOTINITIALISED:
		    e = Impossible;
		    break;
		case WSAENETDOWN:
		case WSAEACCES:
		case WSAENETRESET:
		case WSAESHUTDOWN:
		case WSAEHOSTUNREACH:
		case WSAECONNABORTED:
		case WSAECONNRESET:
		    e = NetworkFailure;
		    break;
		case WSAEINTR:
		    done = FALSE;
		    break;
		case WSAEINPROGRESS:
		    e = NoResources;
		    // ### perhaps try it later?
		    break;
		case WSAEFAULT:
		case WSAEOPNOTSUPP:
		    e = Bug;
		    break;
		case WSAENOBUFS:
		    // ### try later?
		    break;
		case WSAEMSGSIZE:
		    e = NoResources;
		    break;
		case WSAENOTCONN:
		case WSAENOTSOCK:
		case WSAEINVAL:
		    e = Impossible;
		    break;
		case WSAEWOULDBLOCK:
		    // ### ???
		    break;
		default:
		    e = UnknownError;
		    break;
	    }
	}
    }
    return r;
}


Q_LONG QSocketDevice::writeBlock( const char * data, Q_ULONG len,
			       const QHostAddress & host, Q_UINT16 port )
{
    if ( t != Datagram ) {
#if defined(QT_CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::sendBlock: Not datagram" );
#endif
	return -1; // for now - later we can do t/tcp
    }

    if ( data == 0 && len != 0 ) {
#if defined(QT_CHECK_NULL) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::sendBlock: Null pointer error" );
#endif
	return -1;
    }
    if ( !isValid() ) {
#if defined(QT_CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::sendBlock: Invalid socket" );
#endif
	return -1;
    }
    if ( !isOpen() ) {
#if defined(QT_CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::sendBlock: Device is not open" );
#endif
	return -1;
    }
    if ( !isWritable() ) {
#if defined(QT_CHECK_STATE) || defined(QSOCKETDEVICE_DEBUG)
	qWarning( "QSocketDevice::sendBlock: Write operation not permitted" );
#endif
	return -1;
    }
    if ( !host.isIp4Addr() ) {
	qWarning( "QSocketDevice: IPv6 is not supported by this version" );
	e = Impossible;
	return FALSE;
    }
    struct sockaddr_in a;
    memset( &a, 0, sizeof(a) );
    a.sin_family = AF_INET;
    a.sin_port = htons( port );
    a.sin_addr.s_addr = htonl( host.ip4Addr() );

    // we'd use MSG_DONTWAIT + MSG_NOSIGNAL if Stevens were right.
    // but apparently Stevens and most implementors disagree
    bool done = FALSE;
    Q_LONG r = 0;
    while ( !done ) {
	r = ::sendto( fd, data, len, 0,
		      (struct sockaddr *)(&a), sizeof(sockaddr_in) );
	done = TRUE;
	if ( r == SOCKET_ERROR && e == NoError ) {//&& e != EAGAIN ) {
	    switch( WSAGetLastError() ) {
		case WSANOTINITIALISED:
		    e = Impossible;
		    break;
		case WSAENETDOWN:
		case WSAEACCES:
		case WSAENETRESET:
		case WSAESHUTDOWN:
		case WSAEHOSTUNREACH:
		case WSAECONNABORTED:
		case WSAECONNRESET:
		case WSAEADDRNOTAVAIL:
		case WSAENETUNREACH:
		case WSAETIMEDOUT:
		    e = NetworkFailure;
		    break;
		case WSAEINTR:
		    done = FALSE;
		    break;
		case WSAEINPROGRESS:
		    e = NoResources;
		    // ### perhaps try it later?
		    break;
		case WSAEFAULT:
		case WSAEOPNOTSUPP:
		case WSAEAFNOSUPPORT:
		    e = Bug;
		    break;
		case WSAENOBUFS:
		case WSAEMSGSIZE:
		    e = NoResources;
		    break;
		case WSAENOTCONN:
		case WSAENOTSOCK:
		case WSAEINVAL:
		case WSAEDESTADDRREQ:
		    e = Impossible;
		    break;
		case WSAEWOULDBLOCK:
		    // ### ???
		    break;
		default:
		    e = UnknownError;
		    break;
	    }
	}
    }
    return r;
}


void QSocketDevice::fetchConnectionParameters()
{
    if ( !isValid() ) {
	p = 0;
	a = QHostAddress();
	pp = 0;
	pa = QHostAddress();
	return;
    }
    struct sockaddr_in sa;
    memset( &sa, 0, sizeof(sa) );
    SOCKLEN_T sz;
    sz = sizeof( sa );
    if ( !::getsockname( fd, (struct sockaddr *)(&sa), &sz ) ) {
	p = ntohs( sa.sin_port );
	a = QHostAddress( ntohl( sa.sin_addr.s_addr ) );
    }
    pp = 0;
    pa = QHostAddress();
}


void QSocketDevice::fetchPeerConnectionParameters()
{
    // do the getpeername() lazy on Windows (sales/arc-18/37759 claims that
    // there will be problems otherwise if you use MS Proxy server)
    struct sockaddr_in sa;
    memset( &sa, 0, sizeof(sa) );
    SOCKLEN_T sz;
    sz = sizeof( sa );
    if ( !::getpeername( fd, (struct sockaddr *)(&sa), &sz ) ) {
	pp = ntohs( sa.sin_port );
	pa = QHostAddress( ntohl( sa.sin_addr.s_addr ) );
    }
}

Q_UINT16 QSocketDevice::peerPort() const
{
    if ( pp==0 && isValid() ) {
	QSocketDevice *that = (QSocketDevice*)this; // mutable
	that->fetchPeerConnectionParameters();
    }
    return pp;
}


QHostAddress QSocketDevice::peerAddress() const
{
    if ( pp==0 && isValid() ) {
	QSocketDevice *that = (QSocketDevice*)this; // mutable
	that->fetchPeerConnectionParameters();
    }
    return pa;
}
