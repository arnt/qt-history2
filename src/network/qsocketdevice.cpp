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

#include "qsocketdevice.h"
#include "qsocketdevice_p.h"

#include <string.h>

#define d d_func()
#define q q_func()

//#define QSOCKETDEVICE_DEBUG

QSocketDevicePrivate::~QSocketDevicePrivate()
{
}

/*!
    \class QSocketDevice
    \brief The QSocketDevice class provides a platform-independent low-level socket API.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    QSocketDevice provides a low level API for working with sockets.
    Users of this class are assumed to have networking experience.
    For most users, the QSocket class provides a much easier and high
    level alternative, but certain things (like UDP) cannot be done
    with QSocket and if you need a platform-independent API for those,
    QSocketDevice is the right choice.

    The essential purpose of the class is to provide a QIODevice that
    works on sockets, wrapped in a platform-independent API.

    When calling connect() or bind(), QSocketDevice detects the
    protocol family (IPv4, IPv6) automatically. Passing the protocol
    family to QSocketDevice's constructor or to setSocket() forces
    creation of a socket device of a specific protocol. If not set, the
    protocol will be detected at the first call to connect() or bind().

    \sa QSocket, QSocketNotifier, QHostAddress
*/


/*!
    \enum QSocketDevice::Protocol

    This enum type describes the protocol family of the socket. Possible
    values are:

    \value IPv4 The socket is an IPv4 socket.
    \value IPv6 The socket is an IPv6 socket.
    \value Unknown The protocol family of the socket is not known. This can
           happen if you use QSocketDevice with an already existing socket; it
           tries to determine the protocol family, but this can fail if the
           protocol family is not known to QSocketDevice.

    \sa protocol() setSocket()
*/

/*!
    \enum QSocketDevice::Error

    This enum type describes the error states of QSocketDevice.

    \value NoError        No error occurred.

    \value AlreadyBound   The device is already bound, according to bind().

    \value Inaccessible   The operating system or firewall prohibited
                          the action.

    \value NoResources    The operating system ran out of a resource.

    \value InternalError  An internal error occurred in QSocketDevice.

    \value Impossible     An attempt was made to do something which makes
    no sense. For example:
    \code
    ::close(sd->socket());
    sd->writeBlock(someData, 42);
    \endcode
    The libc ::close() closes the socket, but QSocketDevice is not aware
    of this. So when you call writeBlock(), it attempts to do the
    impossible.

    \value NoFiles        The operating system will not let QSocketDevice
                          open another file.

    \value ConnectionRefused  A connection attempt was rejected by the peer.

    \value NetworkFailure  There is a network failure.

    \value UnknownError    The operating system did something unexpected.

    \omitvalue Bug
*/

/*!
    \enum QSocketDevice::Type

    This enum type describes the type of the socket:

    \value Stream    A stream socket (usually TCP).
    \value Datagram  A datagram socket (usually UDP).
*/


/*!
    Creates a QSocketDevice object for the existing \a socket.

    The \a type must match the actual socket type; use \c
    QSocketDevice::Stream for a reliable, connection-oriented TCP
    socket, or \c QSocketDevice::Datagram for an unreliable,
    connectionless UDP socket.
*/
QSocketDevice::QSocketDevice(int socket, Type type)
    : QIODevice(*new QSocketDevicePrivate)
{
    d->t = type;
    d->protocol = QSocketDevice::Unknown;
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug("QSocketDevice: Created QSocketDevice %p (socket %x, type %d)",
           this, socket, type);
#endif
    d->init();
    setSocket(socket, type);
}

/*!
    Creates a QSocketDevice object for a stream or datagram socket.

    The \a type argument must be either \c QSocketDevice::Stream for a
    reliable, connection-oriented TCP socket, or \c QSocketDevice::Datagram
    for a UDP socket.

    The socket is created as an IPv4 socket.

    \sa blocking() protocol()
*/
QSocketDevice::QSocketDevice(Type type)
    : QIODevice(*new QSocketDevicePrivate)
{
    d->t = type;
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug("QSocketDevice: Created QSocketDevice object %p, type %d",
            this, type);
#endif
#if defined(Q_OS_WIN32)
    d->init();
#endif
    setSocket(d->createNewSocket(), type);
}

/*!
    \fn QSocketDevice::QSocketDevice(Type type, Protocol protocol, int unused)

    Creates a QSocketDevice object for a stream or datagram socket.

    The \a type must be either \c QSocketDevice::Stream for a
    reliable, connection-oriented TCP socket, or \c QSocketDevice::Datagram
    for a UDP socket.

    The \a protocol indicates whether the socket should be of type IPv4
    or IPv6. Passing \c Unknown is not meaningful in this context and you
    should avoid using (it creates an IPv4 socket, but your code is not
    easily readable).

    ###
    The \a unused argument is necessary for compatibility with some
    compilers.

    \sa blocking() protocol()
*/
QSocketDevice::QSocketDevice(Type type, Protocol protocol, int)
    : QIODevice(*new QSocketDevicePrivate)
{
    d->t = type;
    d->protocol = protocol;
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug("QSocketDevice: Created QSocketDevice object %p, type %d",
            this, type);
#endif
    d->init();
    setSocket(d->createNewSocket(), type);
}

/*!
    Destroys the socket device, and closes the socket if it is open.
*/
QSocketDevice::~QSocketDevice()
{
    close();
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug("QSocketDevice: Destroyed QSocketDevice %p", this);
#endif
}


/*!
    Returns true if this is a valid socket; otherwise returns false.

    \sa socket()
*/
bool QSocketDevice::isValid() const
{
    return d->fd != -1;
}


/*!
    \fn Type QSocketDevice::type() const

    Returns the socket type which is either \c QSocketDevice::Stream
    or \c QSocketDevice::Datagram.

    \sa socket()
*/
QSocketDevice::Type QSocketDevice::type() const
{
    return d->t;
}

/*!
    Returns the socket's protocol family, which is one of \c Unknown,
    \c IPv4, or \c IPv6.

    QSocketDevice either creates a socket with a well known protocol family,
    or it uses an already existing socket. In the first case, this function
    returns the protocol family it was constructed with. In the second case,
    it tries to determine the protocol family of the socket; if this fails,
    it returns \c Unknown.

    \sa Protocol setSocket()
*/
QSocketDevice::Protocol QSocketDevice::protocol() const
{
    if (d->protocol == Unknown)
        d->protocol = d->getProtocol();
    return d->protocol;
}

/*!
    Returns the socket number, or -1 if it is an invalid socket.

    \sa isValid(), type()
*/
int QSocketDevice::socket() const
{
    return d->fd;
}


/*!
    Sets the socket device to operate on the existing \a socket.

    The \a type argument must match the actual socket type; use \c
    QSocketDevice::Stream for a reliable, connection-oriented TCP
    socket, or \c QSocketDevice::Datagram for a connectionless UDP
    socket.

    Any existing socket is closed.

    \sa isValid(), close()
*/
void QSocketDevice::setSocket(int socket, Type type)
{
    if (d->fd != -1)                        // close any open socket
        close();
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug("QSocketDevice::setSocket: socket %x, type %d", socket, type);
#endif
    d->t = type;
    d->fd = socket;
    d->protocol = Unknown;
    d->e = NoError;
    setFlags(IO_Sequential);
    resetStatus();
    open(IO_ReadWrite);
    d->fetchConnectionParameters();
}


/*!
    \reimp

    The size is meaningless for a socket, therefore this function returns 0.
*/
Q_LLONG QSocketDevice::size() const
{
    return 0;
}


/*!
    \reimp

    The read/write index is meaningless for a socket, therefore this
    function returns 0.
*/
Q_LLONG QSocketDevice::at() const
{
    return 0;
}


/*!
    \reimp

    The read/write index is meaningless for a socket, therefore this
    function does nothing and returns true.
*/
bool QSocketDevice::seek(Q_LLONG)
{
    return true;
}


/*!
    Returns true if the address of this socket can be used by other
    sockets at the same time, and false if this socket claims
    exclusive ownership.

    \sa setAddressReusable()
*/
bool QSocketDevice::addressReusable() const
{
    return d->option(QSocketDevicePrivate::ReuseAddress);
}


/*!
    If \a enable is true, the address of this socket can be used by other
    sockets; otherwise the address is used exclusively by this socket.

    When a socket is reusable, other sockets can use the same port
    number (and IP address), which is generally useful. Of course
    other sockets cannot use the same
    (address,port,peer-address,peer-port) 4-tuple as this socket, so
    there is no risk of confusing the two TCP connections.

    \sa addressReusable()
*/
void QSocketDevice::setAddressReusable(bool enable)
{
    d->setOption(QSocketDevicePrivate::ReuseAddress, enable);
}


/*!
    Returns the size of the operating system receive buffer.

    \sa setReceiveBufferSize()
*/
int QSocketDevice::receiveBufferSize() const
{
    return d->option(QSocketDevicePrivate::ReceiveBuffer);
}


/*!
    Sets the size of the operating system receive buffer to the given
    \a size in bytes.

    The operating system receive buffer size effectively limits two
    things: how much data can be in transit at any one moment, and how
    much data can be received in one iteration of the main event loop.

    The default is operating system-dependent. A socket that receives
    large amounts of data is probably best with a buffer size of
    49152 bytes.
*/
void QSocketDevice::setReceiveBufferSize(uint size)
{
    d->setOption(QSocketDevicePrivate::ReceiveBuffer, size);
}


/*!
    Returns the size of the operating system send buffer.

    \sa setSendBufferSize()
*/
int QSocketDevice::sendBufferSize() const
{
    return d->option(QSocketDevicePrivate::SendBuffer);
}


/*!
    Sets the size of the operating system send buffer to the given \a size
    in bytes.

    The operating system send buffer size effectively limits how much
    data can be in transit at any one moment.

    The default is operating system-dependent. A socket that sends
    large amounts of data is probably best with a buffer size of
    49152 bytes.
*/
void QSocketDevice::setSendBufferSize(uint size)
{
    d->setOption(QSocketDevicePrivate::SendBuffer, size);
}


/*!
    Returns the port number of this socket device. This may be 0 initially,
    but is set to something sensible as soon as a sensible value is
    available.

    Note that Qt always uses native byte order; i.e. 67 is 67 in Qt.
    There is no need to call htons().
*/
Q_UINT16 QSocketDevice::port() const
{
    return d->p;
}


/*!
    Returns the address of this socket device. This may be 0.0.0.0
    initially, but is set to something sensible as soon as a sensible
    value is available.
*/
QHostAddress QSocketDevice::address() const
{
    return d->a;
}


/*!
    Returns the first error seen.
*/
QSocketDevice::Error QSocketDevice::error() const
{
    return d->e;
}


/*!
    \fn void QSocketDevice::setError(Error error)

    Allows subclasses to set the \a error state.
*/
void QSocketDevice::setError(Error err)
{
    d->e = err;
}

