/****************************************************************************
** $Id$
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

#include "qserversocket.h"


class QServerSocketPrivate {
public:
    QServerSocketPrivate(): s(0), n(0) {}
    ~QServerSocketPrivate() { delete s; delete n; }
    QSocketDevice *s;
    QSocketNotifier *n;
};


/*!
  \class QServerSocket qserversocket.h
  \brief The QServerSocket class provides a TCP-based server.

  \ingroup kernel

  This class is a convenience class for accepting incoming TCP
  connections.  You can specify port or have QSocketServer pick one,
  and listen on just one address or on all the addresses of the
  machine.

  The API is very simple: Subclass it, call the constructor of your
  choice, and implement newConnection() to handle new incoming
  connections.  There is nothing more to do.

  (Note that due to lack of support in the underlying APIs,
  QServerSocket cannot accept or reject connections conditionally.)

  \sa QSocket, QSocketDevice, QHostAddress, QSocketNotifier
*/


/*!
  Creates a server socket object, that will serve the given \a port on
  all the addresses of this host.  If \a port is 0, QServerSocket
  picks a suitable port in in a system-dependent manner.

  The \a parent and \a name arguments are passed on as usual
  to the QObject constructor.
*/

QServerSocket::QServerSocket( int port, int backlog,
			      QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QServerSocketPrivate;
    init( QHostAddress(), port, backlog );
}


/*!
  Creates a server socket object, that will serve the given \a port
  on just \a address.

  The \a parent and \a name arguments are passed on as usual
  to the QObject constructor.
*/

QServerSocket::QServerSocket( const QHostAddress & address, int port,
			      int backlog,
			      QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QServerSocketPrivate;
    init( address, port, backlog );
}


/*!
  Tests that the construction succeeded.
*/
bool QServerSocket::ok() const
{
    return d->s;
}

/*!  The common bit of the constructors. */

void QServerSocket::init( const QHostAddress & address, int port, int backlog )
{
    d->s = new QSocketDevice;
    if ( d->s->bind( address, port )
      && d->s->listen( backlog ) )
    {
	d->n = new QSocketNotifier( d->s->socket(), QSocketNotifier::Read,
				    this, "accepting new connections" );
	connect( d->n, SIGNAL(activated(int)),
		 this, SLOT(incomingConnection(int)) );
    } else {
	delete d->s;
	d->s = 0;
    }
}


/*!
  Destructs the socket.

  This brutally severes any backlogged connections (connections that
  have reached the host, but not yet been completely set up by calling
  QSocketDevice::accept()).

  Existing connections continue to exist; this only affects acceptance
  of new connections.
*/

QServerSocket::~QServerSocket()
{
    delete d;
}


/*! \fn void QServerSocket::newConnection( int socket )

  This pure virtual function is responsible for setting up a new
  incoming connection.  \a socket is the fd of the newly accepted
  connection.
*/


void QServerSocket::incomingConnection( int )
{
    int fd = d->s->accept();
    if ( fd >= 0 )
	newConnection( fd );
}


/*!  Returns the port number on which this object listens.  This is
always non-zero; if you specify 0 in the constructor, QServerSocket
picks a port itself and port() returns its number. ok() must be TRUE before
calling this function.

\sa address() QSocketDevice::port()
*/

uint QServerSocket::port() const
{
    return d->s->port();
}


/*! Returns the operating system socket.
*/

int QServerSocket::socket() const
{
    return d->s->socket();
}

/*!  Returns the address on which this object listens, or 0.0.0.0 if
this object listens on more than one address. ok() must be TRUE before
calling this function.

\sa port() QSocketDevice::address()
*/

QHostAddress QServerSocket::address() const
{
    return d->s->address();
}


/*! Returns a pointer to the internal socket device. The returned pointer is
  null if there is no connection or pending connection. 
 
  There is normally no need to manipulate the socket device directly since this
  class does all the necessary setup for most client or server socket
  applications.
*/

QSocketDevice *QServerSocket::socketDevice()
{
    return d->s;
}


/*!
  Construct an empty server socket. This makes rarely sense.

  \sa setSocketDevice()
*/

QServerSocket::QServerSocket( QObject *parent, const char *name )
    : QObject( parent, name )
{
    d = new QServerSocketPrivate;
}


/*!
  Set the socket device of the server socket to \a sd. This is a very low-level
  interface to the class and it is only useful when you want to implement a
  server socket that should listen to sockets that are bound to something
  exotic.

  If you call this function, take care that the socket device \a sd is already
  bound. This function will handle the rest. It returns TRUE on success,
  otherwise FALSE.

  Attention: This class will delete the socket device \a sd when it does not
  need it any more!
*/

bool QServerSocket::setSocketDevice( QSocketDevice *sd, int backlog )
{
    d->s = sd;
    if ( d->s->listen( backlog ) )
    {
	d->n = new QSocketNotifier( d->s->socket(), QSocketNotifier::Read,
				    this, "accepting new connections" );
	connect( d->n, SIGNAL(activated(int)),
		 this, SLOT(incomingConnection(int)) );
    } else {
	delete d->s;
	d->s = 0;
	return FALSE;
    }
    return TRUE;
}
