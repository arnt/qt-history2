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
 
#ifdef __MIPSEL__
 #ifndef SOCK_DGRAM
  #define SOCK_DGRAM 1
 #endif
 #ifndef SOCK_STREAM
  #define SOCK_STREAM 2
 #endif
#endif



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

void QWSSocket::connectToLocalFile( const QString &file )
{
    // create socket
    int s = ::socket( AF_UNIX, SOCK_STREAM, 0 );

    // connect to socket
    struct sockaddr_un a;
    memset( &a, 0, sizeof(a) );
    a.sun_family = AF_UNIX;
    strncpy( a.sun_path, file.local8Bit(), UNIX_PATH_MAX-1 );
    int r = ::connect( s, (struct sockaddr*)&a, sizeof(struct sockaddr_un) );
    if ( r == 0 ) {
	setSocket( s );
    } else {
	::close( s );
	emit error( ErrConnectionRefused );
    }
}


/***********************************************************************
 *
 * QWSServerSocket
 *
 **********************************************************************/
QWSServerSocket::QWSServerSocket( const QString& file, int backlog, QObject *parent, const char *name )
    : QServerSocket( parent, name )
{
    // create socket
    int s = ::socket( AF_UNIX, SOCK_STREAM, 0 );

    // bind socket
    struct sockaddr_un a;
    memset( &a, 0, sizeof(a) );
    a.sun_family = AF_UNIX;
    strncpy( a.sun_path, file.local8Bit(), UNIX_PATH_MAX-1 );
    int r = ::bind( s, (struct sockaddr*)&a, sizeof(struct sockaddr_un) );
    if ( r < 0 ) {
	qWarning( "QWSServerSocket: could not bind to file %s", file.latin1() );
	::close( s );
	return;
    }

    // listen
    if ( ::listen( s, backlog ) == 0 ) {
	setSocket( s );
    } else {
	qWarning( "QWSServerSocket: could not listen to file %s", file.latin1() );
	::close( s );
    }
}

QWSServerSocket::~QWSServerSocket()
{
}
