/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qserversocket.cpp#2 $
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

/*!
  Destructs the socket.
  \internal
     ##### what happens to connection?
*/
QServerSocket::~QServerSocket()
{
    delete d;
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


/*!
  Returns the QSocketDevice used by this server.
*/
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
