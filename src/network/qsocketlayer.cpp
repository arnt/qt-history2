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

/*! \class QSocketLayer

    \brief The QSocketLayer class provides low level access to a socket.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif
    \reentrant
    \ingroup io
    \module network

    QtSocketLayer provides basic socket functionality provided by the
    operating system. It also keeps track of what state the socket is
    in, and which errors that occur.

    The classes QTcpSocket, QUdpSocket and QTcpServer provide a
    higher level API, and are in general more useful for the common
    application.

    There are two main ways of initializing the a QSocketLayer; either
    create a new socket by passing the socket type (TcpSocket or
    UdpSocket) and network layer protocol (IPv4Protocol or
    IPv6Protocol) to initialize(), or pass an existing socket
    descriptor and have QSocketLayer determine the type and protocol
    itself. The native socket descriptor can later be fetched by
    calling socketDescriptor(). The socket is made non-blocking, but
    blocking behavior can still be achieved by calling waitForRead()
    and waitForWrite(). isValid() can be called to check if the socket
    has been successfully initialized and is ready to use.

    To connect to a host, determine its address and pass this and the
    port number to connectToHost(). The socket can then be used as a
    TCP or UDP client. Otherwise; bind(), listen() and accept() are
    used to have the socket function as a TCP or UDP server. Call
    close() to close the socket.

    bytesAvailable() is called to determine how much data is available
    for reading. read() and write() are used by both TCP and UDP
    clients to exchange data with the connected peer. UDP clients can
    also call hasMoreDatagrams(), nextDatagramSize(),
    receiveDatagram() and sendDatagram().

    Call socketState() to determine the state of the socket, for
    example, ListeningState or ConnectedState. socketType() tells
    whether the socket is a TCP socket or a UDP socket, or if the
    socket type is unknown. protocol() is used to determine the
    socket's network layer protocol.

    localAddress(), localPort() are called to find the address and
    port that are currently bound to the socket. If the socket is
    connected, peerAddress() and peerPort() determine the address and
    port of the connected peer.

    Finally, if any function should fail, socketError() and
    errorString() can be called to determine the cause of the error.
*/

#include "qsocketlayer.h"
#include "qsocketlayer_p.h"

//#define QSOCKETLAYER_DEBUG


#define Q_VOID


// Common constructs
#define Q_CHECK_VALID_SOCKETLAYER(function, returnValue) do { \
    if (!isValid()) { \
        qWarning(""#function" was called on an uninitialized socket device"); \
        return returnValue; \
    } } while (0)
#define Q_CHECK_INVALID_SOCKETLAYER(function, returnValue) do { \
    if (isValid()) { \
        qWarning(""#function" was called on an already initialized socket device"); \
        return returnValue; \
    } } while (0)
#define Q_CHECK_STATE(function, checkState, returnValue) do { \
    if (d->socketState != (checkState)) { \
        qWarning(""#function" was not called in "#checkState); \
        return (returnValue); \
    } } while (0)
#define Q_CHECK_NOT_STATE(function, checkState, returnValue) do { \
    if (d->socketState == (checkState)) { \
        qWarning(""#function" was called in "#checkState); \
        return (returnValue); \
    } } while (0)
#define Q_CHECK_STATES(function, state1, state2, returnValue) do { \
    if (d->socketState != (state1) && d->socketState != (state2)) { \
        qWarning(""#function" was called" \
                 " not in "#state1" or "#state2); \
        return (returnValue); \
    } } while (0)
#define Q_CHECK_TYPE(function, type, returnValue) do { \
    if (d->socketType != (type)) { \
        qWarning(#function" was called by a" \
                 " socket other than "#type""); \
        return (returnValue); \
    } } while (0)
#define Q_TR(a) QT_TRANSLATE_NOOP(QSocketLayer, a)

/*! \internal
    Constructs the private class and initializes all data members.

    On Windows, WSAStartup is called "recursively" for every
    concurrent QSocketLayer. This is safe, because WSAStartup and
    WSACleanup are reference counted.
*/
QSocketLayerPrivate::QSocketLayerPrivate()
{
    socketDescriptor = -1;
    socketState = Qt::UnconnectedState;
    socketType = Qt::UnknownSocketType;
    socketProtocol = Qt::UnknownNetworkLayerProtocol;
    socketError = Qt::UnknownSocketError;
    socketErrorString = Q_TR("Unknown error");

    peerPort = 0;
    localPort = 0;

#ifdef Q_OS_WIN
    winSockVersion = 0x00;
    startup();
#endif
}

/*! \internal
    Destructs the private class.
*/
QSocketLayerPrivate::~QSocketLayerPrivate()
{
#ifdef Q_OS_WIN
    cleanup();
#endif
}

/*! \internal

    Sets the error and error string if not set already. The only
    interesting error is the first one that occurred, and not the last
    one.
*/
void QSocketLayerPrivate::setError(Qt::SocketError error,
                                   const QString &errorString) const
{
    if (socketError != Qt::UnknownSocketError)
        return;

    socketError = error;
    socketErrorString = errorString;
}

/*!
    Constructs a QSocketLayer.

    \sa initialize()
*/
QSocketLayer::QSocketLayer()
{
    d = new QSocketLayerPrivate;
    d->q = this;
}

/*!
    Destructs a QSocketLayer.
*/
QSocketLayer::~QSocketLayer()
{
    if (d->socketDescriptor != -1)
        close();
    delete d;
}

/*!
    Initializes a QSocketLayer by creating a new socket of type \a
    socketType and network layer protocol \a protocol. Returns true on
    success; otherwise returns false.

    If the socket was already initialized, this function closes the
    socket before reeinitializing it.

    The new socket is non-blocking, and for UDP sockets it's also
    broadcast enabled.
*/
bool QSocketLayer::initialize(Qt::SocketType socketType, Qt::NetworkLayerProtocol protocol)
{
    if (isValid())
        close();

#if defined(QT_NO_IPV6)
    if (protocol == IPv6Protocol) {
        d->setError(Qt::UnsupportedSocketOperationError,
                    Q_TR("Attempt to create an IPv6 socket on"
                         " a platform with no IPv6 support"));
        return false;
    }
#endif

    // Create the socket
    if (!d->createNewSocket(socketType, protocol)) {
#if defined (QSOCKETLAYER_DEBUG)
        QString typeStr = "UnknownSocketType";
        if (type == Qt::TcpSocket) typeStr = "TcpSocket";
        else if (type == Qt::UdpSocket) typeStr = "UdpSocket";
        QString protocolStr = "UnknownProtocol";
        if (protocol == Qt::IPv4Protocol) protocolStr = "IPv4Protocol";
        else if (protocol == Qt::IPv6Protocol) protocolStr = "IPv6Protocol";
        qDebug("QSocketLayer::initialize(type == %s, protocol == %s) failed: %s",
               typeStr.latin1(), protocolStr.latin1(), d->socketErrorString.latin1());
#endif
        return false;
    }

    // Make the socket nonblocking.
    if (!d->setOption(QSocketLayerPrivate::NonBlockingSocketOption, 1)) {
        d->setError(Qt::UnsupportedSocketOperationError,
                    Q_TR("Unable to initialize a non-blocking socket"));
        close();
        return false;
    }

    // Set the broadcasting flag if it's a UDP socket.
    if (socketType == Qt::UdpSocket
        && !d->setOption(QSocketLayerPrivate::BroadcastSocketOption, 1)) {
        d->setError(Qt::UnsupportedSocketOperationError,
                    Q_TR("Unable to initialize broadcasting socket"));
        close();
        return false;
    }

    d->socketType = socketType;
    d->socketProtocol = protocol;
    return true;
}

/*! \overload

    Initializes the socket using \a socketDescriptor instead of
    creating a new one. The socket type and network layer protocol are
    determined automatically. The socket's state is set to \a
    socketState.

    If the socket type is either TCP or UDP, it is made non-blocking.
    UDP sockets are also broadcast enabled.
 */
bool QSocketLayer::initialize(int socketDescriptor, Qt::SocketState socketState)
{
    if (isValid())
        close();

    d->socketDescriptor = socketDescriptor;

    // determine socket type and protocol
    if (!d->fetchConnectionParameters()) {
#if defined (QSOCKETLAYER_DEBUG)
        qDebug("QSocketLayer::initialize(socketDescriptor == %i) failed: %s",
               socketDescriptor, d->socketErrorString.latin1());
#endif
        d->socketDescriptor = -1;
        return false;
    }

    if (d->socketType != Qt::UnknownSocketType) {
        // Make the socket nonblocking.
        if (!d->setOption(QSocketLayerPrivate::NonBlockingSocketOption, 1)) {
            d->setError(Qt::UnsupportedSocketOperationError,
                        Q_TR("Unable to initialize a non-blocking socket"));
            close();
            return false;
        }

        // Set the broadcasting flag if it's a Udp socket.
        if (d->socketType == Qt::UdpSocket
            && !d->setOption(QSocketLayerPrivate::BroadcastSocketOption, 1)) {
            d->setError(Qt::UnsupportedSocketOperationError,
                        Q_TR("Unable to initialize broadcasting socket"));
            close();
            return false;
        }
    }

    d->socketState = socketState;
    return true;
}

/*!
    Returns true if the socket is valid; otherwise returns false. A
    socket is valid if it has not been successfully initialized, or if
    it has been closed.
*/
bool QSocketLayer::isValid() const
{
    return d->socketDescriptor != -1;
}

/*!
    Returns the socket's state.

    \sa QSocketLayer::SocketState
*/
Qt::SocketState QSocketLayer::socketState() const
{
    return d->socketState;
}

/*!
    Returns the socket's type.

    \sa QSocketLayer::SocketType
*/
Qt::SocketType QSocketLayer::socketType() const
{
    return d->socketType;
}

/*!
    Returns the socket's network layer protocol.

    \sa QSocketLayer::NetworkLayerProtocol
*/
Qt::NetworkLayerProtocol QSocketLayer::protocol() const
{
    return d->socketProtocol;
}

/*!
    Returns the native socket descriptor. Any use of this descriptor
    stands the risk of being non-portable.
*/
int QSocketLayer::socketDescriptor() const
{
    return d->socketDescriptor;
}

/*!
    If the socket is in BoundState or ListeningState, this function
    returns the local address that the socket is bound to; otherwise
    QHostAddress::NullAddress is returned.
*/
QHostAddress QSocketLayer::localAddress() const
{
    return d->localAddress;
}

/*!
    If the socket is in BoundState or ListeningState, this function
    returns the local port that the socket is bound to; otherwise 0 is
    returned.
*/
Q_UINT16 QSocketLayer::localPort() const
{
    return d->localPort;
}

/*!
    If the socket is in ConnectedState, this function returns the
    address of the connected peer; otherwise QHostAddress::NullAddress
    is returned.
*/
QHostAddress QSocketLayer::peerAddress() const
{
    return d->peerAddress;
}

/*!
    If the socket is in ConnectedState, this function returns the port
    of the connected peer; otherwise 0 is returned.
*/
Q_UINT16 QSocketLayer::peerPort() const
{
    return d->peerPort;
}

/*!
    Connects to the IP address and port specified by \a address and \a
    port. If the connection is established, this function returns true
    and the socket enters ConnectedState. Otherwise, false is
    returned.

    If false is returned, socketState() should be called to see if the
    socket is in ConnectingState. If so, a delayed TCP connection is
    taking place, and connectToHost() must be called again later to
    determine if the connection was established successfully or
    not. The second connection attempt must be made when the socket is
    ready for writing. This state can be determined either by
    connecting a QSocketNotifier to the socket descriptor returned by
    socketDescriptor(), or by calling the blocking function
    waitForWrite().

    Example:
    \code
        QSocketLayer socketLayer;
        socketLayer.initialize(Qt::TcpSocket, Qt::IPv4Protocol);
        socketLayer.connectToHost(QHostAddress::LocalhostAddress, 22);
        // returns false

        socketLayer.waitForWrite();
        socketLayer.connectToHost(QHostAddress::LocalhostAddress, 22);
        // returns true
    \endcode

    Otherwise, error() should be called to determine the cause of the
    error.
*/
bool QSocketLayer::connectToHost(const QHostAddress &address, Q_UINT16 port)
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::connectToHost(), false);

#if defined (QT_NO_IPV6)
    if (address.isIPv6Address()) {
        d->setError(Qt::UnsupportedSocketOperationError,
                    Q_TR("Attempt to connect to an IPv6 address on"
                         " a platform with no IPv6 support"));
        return false;
    }
#endif

    Q_CHECK_STATES(QSocketLayer::connectToHost(),
                   Qt::UnconnectedState, Qt::ConnectingState, false);

    bool connected = d->nativeConnect(address, port);
    if (connected)
        d->fetchConnectionParameters();

    return connected;
}

/*!
    Binds the socket to the address \a address and port \a
    port. Returns true on success; otherwise false is returned. The
    port may be 0, in which case an arbitrary unused port is assigned
    automatically by the operating system.

    Servers call this function to set up the server's address and
    port. TCP servers must in addition call listen() after bind().
*/
bool QSocketLayer::bind(const QHostAddress &address, Q_UINT16 port)
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::bind(), false);

#if defined (QT_NO_IPV6)
    if (address.isIPv6Address()) {
        d->setError(Qt::UnsupportedSocketOperationError,
                    Q_TR("Attempt to bind to an IPv6 address on"
                         " a platform with no IPv6 support"));
        return false;
    }
#endif

    Q_CHECK_STATE(QSocketLayer::bind(), Qt::UnconnectedState, false);

    if (!d->nativeBind(address, port))
        return false;

    d->fetchConnectionParameters();
    return true;
}

/*!
    Prepares a TCP server for accepting incoming connections. This
    function must be called after bind(), and only by TCP sockets.

    After this function has been called, pending client connections
    are detected by checking if the socket is ready for reading. This
    can be done by either creating a QSocketNotifier, passing the
    socket descriptor returned by socketDescriptor(), or by calling
    the blocking function waitForRead().

    Example:
    \code
        QSocketLayer socketLayer;
        socketLayer.bind(QHostAddress::AnyAddress, 4000);
        socketLayer.listen();
        if (socketLayer.waitForRead()) {
            int clientSocket = socketLayer.accept();
            // a client is connected
        }
    \encode

    \sa bind(), accept()
*/
bool QSocketLayer::listen()
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::listen(), false);
    Q_CHECK_STATE(QSocketLayer::listen(), Qt::BoundState, false);
    Q_CHECK_TYPE(QSocketLayer::listen(), Qt::TcpSocket, false);

    // We're using a backlog of 50. Most modern kernels support TCP
    // syncookies by default, and if they do, the backlog is ignored.
    // When there is no support for TCP syncookies, this value is
    // fine.
    return d->nativeListen(50);
}

/*!
    Accepts a pending connection from the socket, which must be in
    ListeningState, and returns its socket descriptor. If no pending
    connections are available, -1 is returned.

    \sa bind(), listen()
*/
int QSocketLayer::accept()
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::accept(), -1);
    Q_CHECK_STATE(QSocketLayer::accept(), Qt::ListeningState, false);
    Q_CHECK_TYPE(QSocketLayer::accept(), Qt::TcpSocket, false);

    return d->nativeAccept();
}

/*!
    Returns the number of bytes that are currently available for
    reading. On error, -1 is returned.

    For UDP sockets, this function returns the accumulated size of all
    pending datagrams, and it is therefore more useful for UDP sockets
    to call hasPendingDatagram() and pendingDatagramSize().
*/
Q_LLONG QSocketLayer::bytesAvailable() const
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::bytesAvailable(), -1);
    Q_CHECK_NOT_STATE(QSocketLayer::bytesAvailable(), Qt::UnconnectedState, false);

    return d->nativeBytesAvailable();
}

/*!
    Returns true if there is at least one datagram pending. This
    function is only called by UDP sockets, where a datagram can have
    a size of 0. TCP sockets call bytesAvailable().
*/
bool QSocketLayer::hasPendingDatagram() const
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::hasPendingDatagram(), false);
    Q_CHECK_NOT_STATE(QSocketLayer::hasPendingDatagram(), Qt::UnconnectedState, false);
    Q_CHECK_TYPE(QSocketLayer::hasPendingDatagram(), Qt::UdpSocket, false);

    return d->nativeHasPendingDatagram();
}

/*!
    Returns the size of the pending datagram, or -1 if no datagram is
    pending. A datagram size of 0 is perfectly valid. This function is
    called by UDP sockets before receiveMessage(). For TCP sockets,
    call bytesAvailable().
*/
Q_LLONG QSocketLayer::pendingDatagramSize() const
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::pendingDatagramSize(), -1);
    Q_CHECK_TYPE(QSocketLayer::pendingDatagramSize(), Qt::UdpSocket, false);

    return d->nativePendingDatagramSize();
}

/*!
    Reads up to \a maxLength bytes of a datagram from the socket,
    stores it in \a data and returns the number of bytes read. The
    address and port of the sender are stored in \a address and \a
    port. If either of these pointers is 0, the corresponding value is
    discarded.

    To avoid unnecessarily loss of data, call pendingDatagramSize() to
    determine the size of the pending message before reading it. If \a
    maxLength is too small, the rest of the datagram will be lost.

    Returns -1 if an error occurred.

    \sa hasPendingDatagram()
*/
Q_LLONG QSocketLayer::receiveDatagram(char *data, Q_LLONG maxLength,
                                      QHostAddress *address, Q_UINT16 *port)
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::receiveDatagram(), -1);
    Q_CHECK_TYPE(QSocketLayer::receiveDatagram(), Qt::UdpSocket, false);

    return d->nativeReceiveDatagram(data, maxLength, address, port);
}

/*!
    Writes a UDP datagram of size \a length bytes to the socket from
    \a data and returns the number of bytes written, or -1 if an error
    occurred.

    Only one datagram is sent, and if there is too much data to fit
    into a single datagram, the operation will fail and socketError()
    will return Qt::DatagramTooLargeError. Operating systems impose an
    upper limit to the size of a datagram, but this size is different
    on almost all platforms. Sending large datagrams is in general
    disadvised, as even if they are sent successfully, they are likely
    to be fragmented before arriving at their destination.

    Experience has shown that it is in general safe to send datagrams
    no larger than 512 bytes.

    \sa receiveDatagram()
*/
Q_LLONG QSocketLayer::sendDatagram(const char *data, Q_LLONG length,
                                    const QHostAddress &host, Q_UINT16 port)
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::sendDatagram(), -1);
    Q_CHECK_TYPE(QSocketLayer::sendDatagram(), Qt::UdpSocket, false);

    if (d->socketType != Qt::UdpSocket) {
        qWarning("QSocketLayer::sendDatagram() can only be "
                 "called on a UdpSocket socket device");
        return -1;
    }
    return d->nativeSendDatagram(data, length, host, port);
}

/*!
    Writes a block of \a length bytes from \a data to the socket.
    Returns the number of bytes written, or -1 if an error occurred.
*/
Q_LLONG QSocketLayer::write(const char *data, Q_LLONG length)
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::write(), -1);
    Q_CHECK_STATE(QSocketLayer::write(), Qt::ConnectedState, -1);
    return d->nativeWrite(data, length);
}

/*!
    Reads up to \a maxLength bytes into \a data from the socket.
    Returns the number of bytes read, or -1 if an error occurred.
*/
Q_LLONG QSocketLayer::read(char *data, Q_LLONG maxLength)
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::read(), -1);
    Q_CHECK_STATE(QSocketLayer::read(), Qt::ConnectedState, -1);

    Q_LLONG readBytes = d->nativeRead(data, maxLength);

    // Handle remote close
    if (readBytes == 0 && d->socketType == Qt::TcpSocket) {
        d->setError(Qt::RemoteHostClosedError,
                    Q_TR("The remote host closed the connection"));
        d->socketState = Qt::UnconnectedState;
        return -1;
    }
    return readBytes;
}

/*!
    Closes the socket. In order to use the socket again, initialize()
    must be called.
*/
void QSocketLayer::close()
{
    d->nativeClose();
    d->socketDescriptor = -1;
    d->socketState = Qt::UnconnectedState;
    d->localPort = 0;
    d->localAddress.clear();
    d->peerPort = 0;
    d->peerAddress.clear();
}

/*!
    Waits for \a msecs milliseconds or until the socket is ready for
    reading. If \a timedOut is not 0 and \a msecs milliseconds have
    passed, the value of \a timedOut is set to true.

    Returns true if data is available for reading; otherwise returns
    false.

    This is a blocking function call; its use is disadvised in a
    single threaded application, as the whole thread will stop
    responding until the function returns. waitForRead() is most
    useful when there is no event loop available. The general approach
    is to create a QSocketNotifier, passing the socket descriptor
    returned by socketDescriptor() to its constructor.
*/
bool QSocketLayer::waitForRead(int msecs, bool *timedOut) const
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::waitForRead(), false);
    Q_CHECK_NOT_STATE(QSocketLayer::waitForRead(),
                      Qt::UnconnectedState, false);

    int ret = d->nativeSelect(msecs, true);
    if (ret == 0) {
        if (timedOut)
            *timedOut = true;
        d->setError(Qt::SocketTimeoutError, "Network operation timed out");
        return false;
    }

    return ret > 0;
}

/*!
    Waits for \a msecs milliseconds or until the socket is ready for
    writing. If \a timedOut is not 0 and \a msecs milliseconds have
    passed, the value of \a timedOut is set to true.

    Returns true if data is available for writing; otherwise returns
    false.

    This is a blocking function call; its use is disadvised in a
    single threaded application, as the whole thread will stop
    responding until the function returns. waitForWrite() is most
    useful when there is no event loop available. The general approach
    is to create a QSocketNotifier, passing the socket descriptor
    returned by socketDescriptor() to its constructor.
*/
bool QSocketLayer::waitForWrite(int msecs, bool *timedOut) const
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::waitForWrite(), false);
    Q_CHECK_NOT_STATE(QSocketLayer::waitForWrite(),
                      Qt::UnconnectedState, false);

    int ret = d->nativeSelect(msecs, false);
    if (ret == 0) {
        if (timedOut)
            *timedOut = true;
        d->setError(Qt::SocketTimeoutError, "Network operation timed out");
        return false;
    }

    return ret > 0;
}

/*!
    Returns the size of the operating system's socket receive
    buffer. Depending on the operating system, this size may be
    different from what has been set earlier with
    setReceiveBufferSize().
*/
Q_LLONG QSocketLayer::receiveBufferSize() const
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::receiveBufferSize(), -1);
    return d->option(QSocketLayerPrivate::ReceiveBufferSocketOption);
}

/*!
    Sets the size of the operating system receive buffer to \a size.

    For clients, this should be set before connectToHost() is called;
    otherwise it will have no effect. For servers, it should be called
    before listen().

    The operating system receive buffer size effectively limits two
    things: how much data can be in transit at any one moment, and how
    much data can be received in one iteration of the main event loop.
    Setting the size of the receive buffer may have an impact on the
    socket's performance.

    The default value is operating system-dependent.
*/
void QSocketLayer::setReceiveBufferSize(Q_LLONG size)
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::setReceiveBufferSize(), Q_VOID);
    d->setOption(QSocketLayerPrivate::ReceiveBufferSocketOption, size);
}

/*!
    Returns the size of the operating system send buffer. Depending on
    the operating system, this size may be different from what has
    been set earlier with setSendBufferSize().
*/
Q_LLONG QSocketLayer::sendBufferSize() const
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::setSendBufferSize(), -1);
    return d->option(QSocketLayerPrivate::SendBufferSocketOption);
}

/*!
    Sets the size of the operating system send buffer to \a size.

    The operating system send buffer size effectively limits how much
    data can be in transit at any one moment. Setting the size of the
    send buffer may have an impact on the socket's performance.

    The default value is operating system-dependent.
*/
void QSocketLayer::setSendBufferSize(Q_LLONG size)
{
    Q_CHECK_VALID_SOCKETLAYER(QSocketLayer::setSendBufferSize(), Q_VOID);
    d->setOption(QSocketLayerPrivate::SendBufferSocketOption, size);
}

/*!
    Returns the type of error that last occurred.

    \sa SocketError
*/
Qt::SocketError QSocketLayer::socketError() const
{
    return d->socketError;
}

/*!
    Returns a human readable description of the last error that
    occurred.

    \sa socketError()
*/
QString QSocketLayer::errorString() const
{
    return d->socketErrorString;
}
