/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qserversocket.h"
#include "qsocketnotifier.h"

class QServerSocketPrivate {
public:
    QServerSocketPrivate(): s(0), n(0) {}
    ~QServerSocketPrivate() { delete n; delete s; }
    QSocketDevice *s;
    QSocketNotifier *n;
};


/*!
    \class QServerSocket qserversocket.h
    \brief The QServerSocket class provides a TCP-based server.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    This class is a convenience class for accepting incoming TCP
    connections. You can specify the port or have QServerSocket pick
    one, and listen on just one address, or on all the machine's
    addresses.

    Using the API is very simple: subclass QServerSocket, call the
    constructor of your choice, and implement newConnection() to
    handle new incoming connections. There is nothing more to do.

    (Note that due to lack of support in the underlying APIs,
    QServerSocket cannot accept or reject connections conditionally.)

    \sa QSocket, QSocketDevice, QHostAddress, QSocketNotifier
*/


/*!
    Creates a server socket object to serve the given \a port
    on all the addresses of this host. If \a port is 0, QServerSocket
    will pick a suitable port in a system-dependent manner. Use \a
    backlog to specify how many pending connections the server can
    have.

    The \a parent argument is passed on to the QObject constructor.

    \warning On Tru64 Unix systems a value of 0 for \a backlog means
    that you don't accept any connections at all; you should specify a
    value larger than 0.
*/

QServerSocket::QServerSocket(Q_UINT16 port, int backlog, QObject *parent)
    : QObject(parent)
{
    d = new QServerSocketPrivate;
    init(QHostAddress(), port, backlog);
}


/*!
    Creates a server socket object to serve the given \a port
    only on the given \a address. Use \a backlog to specify how many
    pending connections the server can have.

    The \a parent argument is passed on to the QObject constructor.

    \warning On Tru64 Unix systems a value of 0 for \a backlog means
    that you don't accept any connections at all; you should specify a
    value larger than 0.
*/

QServerSocket::QServerSocket(const QHostAddress & address, Q_UINT16 port,
                              int backlog, QObject *parent)
    : QObject(parent)
{
    d = new QServerSocketPrivate;
    init(address, port, backlog);
}


/*!
    Construct an empty server socket with the given \a parent.

    This constructor, in combination with setSocket(), allows us to
    use the QServerSocket class as a wrapper for other socket types,
    such as Unix Domain Sockets.

    \sa setSocket()
*/

QServerSocket::QServerSocket(QObject *parent)
    : QObject(parent)
{
    d = new QServerSocketPrivate;
}


#ifdef QT_COMPAT
QServerSocket::QServerSocket(Q_UINT16 port, int backlog,
                              QObject *parent, const char *name)
    : QObject(parent)
{
    setObjectName(name);
    d = new QServerSocketPrivate;
    init(QHostAddress(), port, backlog);
}

QServerSocket::QServerSocket(const QHostAddress & address, Q_UINT16 port,
                              int backlog,
                              QObject *parent, const char *name)
    : QObject(parent)
{
    setObjectName(name);
    d = new QServerSocketPrivate;
    init(address, port, backlog);
}

QServerSocket::QServerSocket(QObject *parent, const char *name)
    : QObject(parent)
{
    setObjectName(name);
    d = new QServerSocketPrivate;
}
#endif

/*!
    Returns true if the socket is ready to use; otherwise returns false.
*/
bool QServerSocket::ok() const
{
    return !!d->s;
}

/*
  The common bit of the constructors.
 */
void QServerSocket::init(const QHostAddress & address, Q_UINT16 port, int backlog)
{
    d->s = new QSocketDevice(QSocketDevice::Stream, address.isIPv4Address()
                              ? QSocketDevice::IPv4 : QSocketDevice::IPv6, 0);
#if !defined(Q_OS_WIN32)
    // Under Unix, we want to be able to use the port, even if a socket on the
    // same address-port is in TIME_WAIT. Under Windows this is possible anyway
    // -- furthermore, the meaning of reusable is different: it means that you
    // can use the same address-port for multiple listening sockets.
    d->s->setAddressReusable(true);
#endif
    if (d->s->bind(address, port)
      && d->s->listen(backlog))
    {
        d->n = new QSocketNotifier(d->s->socket(), QSocketNotifier::Read, this);
        d->n->setObjectName("accepting new connections");
        connect(d->n, SIGNAL(activated(int)),
                 this, SLOT(incomingConnection(int)));
    } else {
        qWarning("QServerSocket: failed to bind or listen to the socket");
        delete d->s;
        d->s = 0;
    }
}


/*!
    Destroys the socket.

    All backlogged connections are closed; this includes connections that
    have reached the host, but have not yet been set up by a call to
    QSocketDevice::accept().

    Existing connections continue to exist; this only affects the
    ability of the server to accept new connections.
*/
QServerSocket::~QServerSocket()
{
    delete d;
}


/*!
    \fn void QServerSocket::newConnection(int socket)

    This pure virtual function is responsible for setting up a new
    incoming connection. The \a socket given is the fd (file descriptor)
    for the newly accepted connection.
*/


void QServerSocket::incomingConnection(int)
{
    int fd = d->s->accept();
    if (fd >= 0)
        newConnection(fd);
}


/*!
    Returns the port number on which this server socket listens. This
    is always non-zero; if you specify 0 in the constructor,
    QServerSocket will pick a non-zero port itself. ok() must be true
    before this function is called.

    \sa address() QSocketDevice::port()
*/
Q_UINT16 QServerSocket::port() const
{
    if (!d || !d->s)
        return 0;
    return d->s->port();
}


/*!
    Returns the operating system socket.
*/
int QServerSocket::socket() const
{
    if (!d || !d->s)
        return -1;

    return d->s->socket();
}

/*!
    Returns the address on which this object listens, or 0.0.0.0 if
    this object listens on more than one address. ok() must be true
    before calling this function.

    \sa port() QSocketDevice::address()
*/
QHostAddress QServerSocket::address() const
{
    if (!d || !d->s)
        return QHostAddress();

    return d->s->address();
}


/*!
    Returns a pointer to the internal socket device. The returned
    pointer is 0 if there is no connection or pending connection.

    There is normally no need to manipulate the socket device directly
    since this class does all the necessary setup for most client or
    server socket applications.
*/
QSocketDevice *QServerSocket::socketDevice()
{
    if (!d)
        return 0;

    return d->s;
}


/*!
    Sets the \a socket to use for the server. bind() and listen() should
    already have been called for the \a socket given.

    This allows us to use the QServerSocket class as a wrapper for
    other socket types, such as Unix Domain Sockets.
*/
void QServerSocket::setSocket(int socket)
{
    delete d;
    d = new QServerSocketPrivate;
    d->s = new QSocketDevice(socket, QSocketDevice::Stream);
    d->n = new QSocketNotifier(d->s->socket(), QSocketNotifier::Read, this);
    d->n->setObjectName("accepting new connections");
    connect(d->n, SIGNAL(activated(int)),
             this, SLOT(incomingConnection(int)));
}
