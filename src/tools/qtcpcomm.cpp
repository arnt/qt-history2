/****************************************************************************
** $Id: //depot/qt/main/src/tools/qtcpcomm.cpp#1 $
**
** Implementation of QTCPComm class
**
** Author  : Haavard Nord
** Created : 940913
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qtcpcomm.h"

#undef BSD_SOCKETS
#undef WIN_SOCKETS

#define HAS_SOCKET_API

#if defined(UNIX)
#define BSD_SOCKETS
#elif defined(_OS_WIN32_)
#define WIN_SOCKETS
#else
#undef	HAS_SOCKET_API
#endif

#if defined(HAS_SOCKET_API)
#include <errno.h>

#if defined(BSD_SOCKETS)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

#elif defined(WIN_SOCKETS)
#include <winsock.h>
#endif

RCSTAG("$Id: //depot/qt/main/src/tools/qtcpcomm.cpp#1 $")


#undef CLOSE

#if defined(BSD_SOCKETS)
#define CLOSE	::close
#if defined(_OS_ULTRIX_)
extern "C"			// ULTRIX 4.2 lacks these prototypes
{
    long  unsigned int inet_addr( const char * );
    short unsigned int htons( short unsigned int );
    short unsigned int ntohs( short unsigned int );
    long  unsigned int htonl( long unsigned int );
    long  unsigned int ntohl( long unsigned int );
}
#endif

#elif defined(WIN_SOCKETS)
#define CLOSE	closesocket
#endif


static int sock_count = 0;

QTCPComm::QTCPComm()
{
#if defined(WIN_SOCKETS)
    WSADATA wsadata;
    if ( sock_count++ == 0 ) {
	if ( WSAStartup(MAKEWORD(1,1), &wsadata) != 0 ) {
#if defined(CHECK_STATE)
	    warning( "QTCPComm: Cannot initialize winsockets" );
#endif
	}
    }
#endif
    lsockfd = -1;
}

QTCPComm::~QTCPComm()
{
    disconnect();
    hangup();
#if defined(WIN_SOCKETS)
    if ( --sock_count == 0 )
	WSACleanup();
#endif
}


bool QTCPComm::listen()				// wait for connection
{
    sockaddr_in serverAddr;
    if ( !isInactive() ) {
#if defined(CHECK_STATE)
	warning( "QTCPComm::listen: Must be idle" );
#endif
	return FALSE;
    }
    lsockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( lsockfd < 0 ) {
#if defined(CHECK_NULL)
	warning( "QTCPComm::listen: Cannot open stream socket" );
#endif
	return FALSE;
    }
    memset( (char *)&serverAddr, sizeof(serverAddr), 0 );
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(portNum());
    if ( bind(lsockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) != 0 ) {
#if defined(CHECK_NULL)
	warning( "QTCPComm::listen: Cannot bind local address" );
#endif
	return FALSE;
    }
    setState( IO_Listen );
    if ( ::listen( lsockfd, 5 ) < 0 ) {		// wait for call
	setState( 0 );
#if defined(CHECK_NULL)
	warning( "QTCPComm::listen: Listen command failed" );
#endif
	return FALSE;
    }
    sockaddr_in clientAddr;
    int clientLen = sizeof(clientAddr);
    sockfd = accept( lsockfd, (sockaddr*)&clientAddr, &clientLen );
    if ( sockfd < 0 ) {
	setState( 0 );
	return FALSE;
    }
    if ( isBuffered() )
	buffer.open( IO_ReadWrite );
    setState( IO_Connected );
    return TRUE;
}


void QTCPComm::hangup()				// hangup listen
{
    if ( lsockfd == -1 )
	return;
    shutdown( lsockfd, 2 );
    CLOSE( lsockfd );
    lsockfd = -1;
    setState( 0 );
}


bool QTCPComm::connect( int mode )		// connect to remote host
{
    sockaddr_in serverAddr;
    ulong	inAddr;
    hostent    *hp;
    if ( isConnected() )			// already connected
	return FALSE;
    memset( &serverAddr, 0, sizeof(serverAddr) );
    serverAddr.sin_family = AF_INET;
    inAddr = inet_addr( hostName() );
    if ( inAddr == INADDR_NONE ) {		// hostName() is name
	hp = gethostbyname( hostName() );
	if ( !hp )				// no such host
	    return FALSE;
	memcpy( (void*)&serverAddr.sin_addr, (void*)hp->h_addr,
		hp->h_length );
    }
    else					// hostName is ip address
	serverAddr.sin_addr.s_addr = inAddr;
    serverAddr.sin_port = htons( portNum() );
    lsockfd = -1;
    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sockfd < 0 ) {
#if defined(CHECK_NULL)
	warning( "QTCPComm::connect: Cannot open stream socket" );
#endif
	return FALSE;				// cannot open socket
    }
    if ( isRaw() )
	setType( IO_Sequential );
    else
	setType( IO_Combined );
    if ( ::connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0 )
	return FALSE;				// cannot connect to server
    setState( IO_Connected );
    if ( isBuffered() )
	buffer.open( IO_ReadWrite );
    return TRUE;
}


bool QTCPComm::disconnect()			// disconnect from remote host
{
    if ( !isConnected() )
	return FALSE;
    bool ok = CLOSE(sockfd) >= 0;
    setState( 0 );
    return ok;
}


void QTCPComm::abort()				// abort connection
{
    if ( !isConnected() )
	return;
    shutdown( sockfd, 2 );
    CLOSE( sockfd );
    setState( 0 );
}


bool QTCPComm::transmit()			// transmit buffered data
{
    if ( isRaw() )				// no flushing for raw device
	return TRUE;
    setMode( mode() | IO_Raw );			// fake mode
    bool res;
    res = writeBlock( buffer.buffer().data(), buffer.size() ) == buffer.size();
    setMode( mode() & ~IO_Raw );		// restore from fake mode
    return res;
}


int QTCPComm::readBlock( char *data, uint len )
{
    if ( !isConnected() )			// not connected
	return -1;
    int nleft = (int)len;			// number of bytes to read
    int nread;
    int ntotal = 0;
    while ( nleft > 0 ) {			// there's more to read
	nread = recv( sockfd, data, nleft, 0 );
	if ( nread == -1 ) {			// read error
	    setStatus( IO_ReadError );
	    return -1;
	}
	else if ( nread == 0 ) {		// connection broken (EOF)
	    // !!! Generate callback message
	    setStatus( IO_AbortError );
	    return -1;
	}
	nleft -= nread;
	data += nread;
	ntotal += nread;
	if ( !readComplete() )
	    return nread;
    }
    return ntotal;
}


int QTCPComm::writeBlock( const char *data, uint len )
{
    if ( !isConnected() )			// not connected
	return -1;
    if ( isBuffered() )
	return buffer.writeBlock( data, len );
    int nleft = (int)len;			// number of bytes to write
    int nwritten;
    while ( nleft > 0 ) {			// there's more to write
	nwritten = send( sockfd, data, nleft, 0 );
	if ( nwritten == -1 ) {
	    setStatus( IO_WriteError );
	    return -1;
	}
	nleft -= nwritten;
	data += nwritten;
    }
    return (int)len;
}


#endif // HAS_SOCKET_API
