/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qsocketdevice.cpp#21 $
**
** Implementation of QWSSocket and related classes.
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qwssocket_qws.h"

#ifndef QT_NO_QWS_MULTIPROCESS

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <netdb.h>
#include <errno.h>

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX    108
#endif
 
#ifdef __MIPSEL__
# ifndef SOCK_DGRAM
#  define SOCK_DGRAM 1
# endif
# ifndef SOCK_STREAM
#  define SOCK_STREAM 2
# endif
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

    if ( chmod( file.local8Bit(), 0600 ) < 0 ) {
	qWarning( "Could not set permissions of %s", file.latin1() );
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
#endif  //QT_NO_QWS_MULTIPROCESS
