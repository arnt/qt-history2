/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qsocketdevice.cpp#21 $
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

#include "qwssocket_qws.h"

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

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX    108
#endif
 

/***********************************************************************
 *
 * QWSSocketDevice
 *
 **********************************************************************/
QWSSocketDevice::QWSSocketDevice( Type type, bool inet )
    : QSocketDevice(
	    ::socket( inet?AF_INET:AF_UNIX, type==Datagram?SOCK_DGRAM:SOCK_STREAM, 0 ),
	    type )
{
}

QWSSocketDevice::QWSSocketDevice( int socket, Type type )
    : QSocketDevice( socket, type )
{
}

QWSSocketDevice::~QWSSocketDevice()
{
}

bool QWSSocketDevice::connect( const QString& localfilename )
{
    if ( !isValid() )
	return FALSE;

    struct sockaddr_un a;
    memset( &a, 0, sizeof(a) );
    a.sun_family = AF_UNIX;
    strncpy( a.sun_path, localfilename.local8Bit(), UNIX_PATH_MAX-1 );

    int r = ::connect( socket(), (struct sockaddr*)&a,
		       sizeof(struct sockaddr_un) );
    if ( r == 0 ) {
	return TRUE;
    }
    if ( errno == EISCONN || errno == EALREADY || errno == EINPROGRESS )
	return TRUE;
    if ( error() != NoError )
	return FALSE;
    switch( errno ) {
    case EBADF:
    case ENOTSOCK:
	setError( Impossible );
	break;
    case EFAULT:
    case EAFNOSUPPORT:
	setError( Bug );
	break;
    case ECONNREFUSED:
	setError( ConnectionRefused );
	break;
    case ETIMEDOUT:
    case ENETUNREACH:
	setError( NetworkFailure );
	break;
    case EADDRINUSE:
	setError( NoResources );
	break;
    case EACCES:
    case EPERM:
	setError( Inaccessible );
	break;
    case EAGAIN:
	// ignore that.  can retry.
	break;
    default:
	setError( UnknownError );
	break;
    }
    return FALSE;
}

bool QWSSocketDevice::bind( const QString& localfilename )
{
    if ( !isValid() )
	return FALSE;

    struct sockaddr_un a;
    memset( &a, 0, sizeof(a) );
    a.sun_family = AF_UNIX;
    strncpy( a.sun_path, localfilename.local8Bit(), UNIX_PATH_MAX-1 );

    int r = ::bind( socket(), (struct sockaddr*)&a, sizeof(struct sockaddr_un) );
    if ( r < 0 ) {
	switch( r ) {
	case EINVAL:
	    setError( AlreadyBound );
	    break;
	case EACCES:
	    setError( Inaccessible );
	    break;
	case ENOMEM:
	    setError( NoResources );
	    break;
	case EFAULT: // a was illegal
	case ENAMETOOLONG: // sz was wrong
	    setError( Bug );
	    break;
	case EBADF: // AF_UNIX only
	case ENOTSOCK: // AF_UNIX only
	case EROFS: // AF_UNIX only
	case ENOENT: // AF_UNIX only
	case ENOTDIR: // AF_UNIX only
	case ELOOP: // AF_UNIX only
	    setError( Impossible );
	    break;
	default:
	    setError( UnknownError );
	    break;
	}
	return FALSE;
    }
    return TRUE;
}


/***********************************************************************
 *
 * QWSSocket
 *
 **********************************************************************/
QWSSocket::QWSSocket( QObject *parent, const char *name )
    : QSocket( parent, name )
{
}

QWSSocket::~QWSSocket()
{
}

void QWSSocket::setSocket( int socket, bool inet )
{
    if ( inet == TRUE ) {
	setSocket( socket );
    } else {
	QWSSocketDevice *sd;
	if ( socket >= 0 ) {
	    sd = new QWSSocketDevice( socket, QWSSocketDevice::Stream );
	    setSocketDevice( sd, TRUE );
	} else {
	    // error?
	}
    }
}

void QWSSocket::connectToLocalFile( const QString &file )
{
    QWSSocketDevice *sd;
    sd = new QWSSocketDevice( QWSSocketDevice::Stream, FALSE );
    setSocketDevice( sd, sd->connect( file ) );
}


/***********************************************************************
 *
 * QWSServerSocket
 *
 **********************************************************************/
QWSServerSocket::QWSServerSocket( const QString& localfile, int backlog, QObject *parent, const char *name )
    : QServerSocket( parent, name )
{
    QWSSocketDevice *sd = new QWSSocketDevice( QSocketDevice::Stream, FALSE );
    if ( sd->bind( localfile ) )
    {
	setSocketDevice( sd, backlog );
    } else {
	delete sd;
    }
}

QWSServerSocket::~QWSServerSocket()
{
}
