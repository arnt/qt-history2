/****************************************************************************
** $Id: //depot/qt/main/extensions/network/src/qserversocket.cpp#5 $
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
    QServerSocketPrivate() { socket=0; sn=0; }
    QSocketDevice *socket;
    QHostAddress addr;
    uint port;
    QSocketNotifier *sn;
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
  The server is not started until start() is called.

  The \a parent and \a name arguments are passed on as usual
  to the QObject constructor.
*/

QServerSocket::QServerSocket( QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QServerSocketPrivate;
}


/*!
  Creates a server socket object, that will server the given \a port.
  The server is not started until start() is called.

  The \a parent and \a name arguments are passed on as usual
  to the QObject constructor.
*/

QServerSocket::QServerSocket( int port, QObject *parent,
			      const char *name )
    : QObject( parent, name )
{
    d = new QServerSocketPrivate;
    d->socket = new QSocketDevice;
    d->addr = QHostAddress( 0 );
    d->port = port;
    
}

/*!
  Starts servicing the configured port(). \a backlog is the number of
  clients which may successfully attempt connect while the server is busy
  (the slower your server, and the more clients you are expecting, the
  higher this number must be if you wish to avoid failed connections).

  Returns TRUE if the server succeeds in binding to the required resources.
*/
bool QServerSocket::start( int backlog )
{
    if ( !d->socket->bind( d->addr, d->port ) )
	return FALSE;
    d->socket->listen( backlog );
    d->sn = new QSocketNotifier( d->socket->socket(), QSocketNotifier::Read, 
				 this, "accepting new connections" );
    connect( d->sn, SIGNAL(activated(int)),
	     this, SLOT(incomingConnection(int)) );
    return TRUE;
}


/*!
  Destructs the socket.

  This brutally severes any backlogged connections (connections that
  have reached the host, but not yet been completely set up by calling
  accept()).

  Existing connections continue to exist; this only affects acceptance
  of new connections.
*/

QServerSocket::~QServerSocket()
{
    delete d;
}


int QServerSocket::port() const
{
    return d->port;
}


void QServerSocket::setPort( int port )
{
}


void QServerSocket::newConnection( int socket )
{
    QSocketDevice s( socket, QSocketDevice::Stream );
    s.close();
}


/*!
  Returns the QSocketDevice used by this server.
*/
QSocketDevice *QServerSocket::socketDevice()
{
    return d->socket;
}


bool QServerSocket::accept() const
{
    return TRUE;
}


void QServerSocket::incomingConnection( int socket )
{
    int fd = d->socket->accept();
    newConnection( fd );
}
