/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qserversocket.cpp#2 $
**
** Implementation of QServerSocket class
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

#include "qserversocket.h"


class QServerSocketPrivate {
public:
    QServerSocketPrivate() { socket=0; }
    QSocketDevice *socket;
    QSocketAddress addr;
};


/*!
  \class QServerSocket qserversocket.h
  \brief The QServerSocket class provides a TCP-based server.

  \ingroup kernel

  This class is not yet documented.

  \sa QSocket, QSocketDevice, QSocketAddress, QSocketNotifier
*/


/*!
  Creates a server socket object, but does not start any server yet.

  The \a parent and \a name arguments are passed on as usual
  to the QObject constructor.
*/

QServerSocket::QServerSocket( QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QServerSocketPrivate;    
}


/*!
  Creates a server socket object, ands starts a server on the
  given \a port.

  The \a parent and \a name arguments are passed on as usual
  to the QObject constructor.
*/

QServerSocket::QServerSocket( int port, QObject *parent,
			      const char *name )
    : QObject( parent, name )
{
    d = new QServerSocketPrivate;
    d->socket = new QSocketDevice;
    d->addr = QSocketAddress( port );    
}


int QServerSocket::port() const
{
    return d->addr.port();
}


void QServerSocket::setPort( int port )
{
}


void QServerSocket::newConnection( int socket )
{
    QSocketDevice s(socket,QSocketDevice::Stream);
    s.close();
}


QSocketDevice *QServerSocket::socketDevice()
{
    return d->socket;
}


bool QServerSocket::accept( const QSocketAddress & ) const
{
    return TRUE;
}


void QServerSocket::incomingConnection( int socket )
{
}
