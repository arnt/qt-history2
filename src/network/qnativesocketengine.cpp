/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

//#define QNATIVESOCKETENGINE_DEBUG

/*! \class QNativeSocketEngine
    \internal

    \brief The QNativeSocketEngine class provides low level access to a socket.

    \reentrant
    \ingroup io
    \module network

    QtSocketLayer provides basic socket functionality provided by the
    operating system. It also keeps track of what state the socket is
    in, and which errors that occur.

    The classes QTcpSocket, QUdpSocket and QTcpServer provide a
    higher level API, and are in general more useful for the common
    application.

    There are two main ways of initializing the a QNativeSocketEngine; either
    create a new socket by passing the socket type (TcpSocket or
    UdpSocket) and network layer protocol (IPv4Protocol or
    IPv6Protocol) to initialize(), or pass an existing socket
    descriptor and have QNativeSocketEngine determine the type and protocol
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
    readDatagram(), and writeDatagram().

    Call state() to determine the state of the socket, for
    example, ListeningState or ConnectedState. socketType() tells
    whether the socket is a TCP socket or a UDP socket, or if the
    socket type is unknown. protocol() is used to determine the
    socket's network layer protocol.

    localAddress(), localPort() are called to find the address and
    port that are currently bound to the socket. If the socket is
    connected, peerAddress() and peerPort() determine the address and
    port of the connected peer.

    Finally, if any function should fail, error() and
    errorString() can be called to determine the cause of the error.
*/

#include "qnativesocketengine_p.h"
#include <qabstracteventdispatcher.h>
#include <qsocketnotifier.h>

#include <private/qthread_p.h>

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
#define Q_TR(a) QT_TRANSLATE_NOOP(QNativeSocketEngine, a)

/*! \internal
    Constructs the private class and initializes all data members.

    On Windows, WSAStartup is called "recursively" for every
    concurrent QNativeSocketEngine. This is safe, because WSAStartup and
    WSACleanup are reference counted.
*/
QNativeSocketEnginePrivate::QNativeSocketEnginePrivate()
{
    socketDescriptor = -1;
    readNotifier = 0;
    writeNotifier = 0;
    exceptNotifier = 0;
}

/*! \internal
    Destructs the private class.
*/
QNativeSocketEnginePrivate::~QNativeSocketEnginePrivate()
{
}

/*! \internal

    Sets the error and error string if not set already. The only
    interesting error is the first one that occurred, and not the last
    one.
*/
void QNativeSocketEnginePrivate::setError(QAbstractSocket::SocketError error, ErrorString errorString) const
{
    if (hasSetSocketError) {
        // Only set socket errors once for one engine; expect the
        // socket to recreate its engine after an error. Note: There's
        // one exception: SocketError(11) bypasses this as it's purely
        // a temporary internal error condition.
        return;
    }
    if (error != QAbstractSocket::SocketError(11))
        hasSetSocketError = true;

    socketError = error;

    switch (errorString) {
    case NonBlockingInitFailedErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Unable to initialize non-blocking socket"));
        break;
    case BroadcastingInitFailedErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Unable to initialize broadcast socket"));
        break;
    case NoIpV6ErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Attempt to use IPv6 socket on a platform with no IPv6 support"));
        break;
    case RemoteHostClosedErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "The remote host closed the connection"));
        break;
    case TimeOutErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Network operation timed out"));
        break;
    case ResourceErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Out of resources"));
        break;
    case OperationUnsupportedErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Unsupported socket operation"));
        break;
    case ProtocolUnsupportedErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Protocol type not supported"));
        break;
    case InvalidSocketErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Invalid socket descriptor"));
        break;
    case HostUnreachableErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Host unreachable"));
        break;
    case NetworkUnreachableErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Network unreachable"));
        break;
    case AccessErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Permission denied"));
        break;
    case ConnectionTimeOutErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Connection timed out"));
        break;
    case ConnectionRefusedErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Connection refused"));
        break;
    case AddressInuseErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "The bound address is already in use"));
        break;
    case AddressNotAvailableErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "The address is not available"));
        break;
    case AddressProtectedErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "The address is protected"));
        break;
    case DatagramTooLargeErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Datagram was too large to send"));
        break;
    case SendDatagramErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Unable to send a message"));
        break;
    case ReceiveDatagramErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Unable to receive a message"));
        break;
    case WriteErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Unable to write"));
        break;
    case ReadErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Network error"));
        break;
    case PortInuseErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Another socket is already listening on the same port"));
        break;
    case NotSocketErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Operation on non-socket"));
        break;
    case UnknownSocketErrorString:
        socketErrorString = QLatin1String(QT_TRANSLATE_NOOP("QNativeSocketEngine", "Unknown error"));
        break;
    }
}

/*!
    Constructs a QNativeSocketEngine.

    \sa initialize()
*/
QNativeSocketEngine::QNativeSocketEngine(QObject *parent)
    : QAbstractSocketEngine(*new QNativeSocketEnginePrivate(), parent)
{
}

/*!
    Destructs a QNativeSocketEngine.
*/
QNativeSocketEngine::~QNativeSocketEngine()
{
    close();
}

/*!
    Initializes a QNativeSocketEngine by creating a new socket of type \a
    socketType and network layer protocol \a protocol. Returns true on
    success; otherwise returns false.

    If the socket was already initialized, this function closes the
    socket before reeinitializing it.

    The new socket is non-blocking, and for UDP sockets it's also
    broadcast enabled.
*/
bool QNativeSocketEngine::initialize(QAbstractSocket::SocketType socketType, QAbstractSocket::NetworkLayerProtocol protocol)
{
    Q_D(QNativeSocketEngine);
    if (isValid())
        close();

#if defined(QT_NO_IPV6)
    if (protocol == QAbstractSocket::IPv6Protocol) {
        d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                    QNativeSocketEnginePrivate::NoIpV6ErrorString);
        return false;
    }
#endif

    // Create the socket
    if (!d->createNewSocket(socketType, protocol)) {
#if defined (QNATIVESOCKETENGINE_DEBUG)
        QString typeStr = QLatin1String("UnknownSocketType");
        if (socketType == QAbstractSocket::TcpSocket) typeStr = QLatin1String("TcpSocket");
        else if (socketType == QAbstractSocket::UdpSocket) typeStr = QLatin1String("UdpSocket");
        QString protocolStr = QLatin1String("UnknownProtocol");
        if (protocol == QAbstractSocket::IPv4Protocol) protocolStr = QLatin1String("IPv4Protocol");
        else if (protocol == QAbstractSocket::IPv6Protocol) protocolStr = QLatin1String("IPv6Protocol");
        qDebug("QNativeSocketEngine::initialize(type == %s, protocol == %s) failed: %s",
               typeStr.toLatin1().constData(), protocolStr.toLatin1().constData(), d->socketErrorString.toLatin1().constData());
#endif
        return false;
    }

    // Make the socket nonblocking.
    if (!setOption(NonBlockingSocketOption, 1)) {
        d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                    QNativeSocketEnginePrivate::NonBlockingInitFailedErrorString);
        close();
        return false;
    }

    // Set the broadcasting flag if it's a UDP socket.
    if (socketType == QAbstractSocket::UdpSocket
        && !setOption(BroadcastSocketOption, 1)) {
        d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                    QNativeSocketEnginePrivate::BroadcastingInitFailedErrorString);
        close();
        return false;
    }

    // Make sure we receive out-of-band data
    if (socketType == QAbstractSocket::TcpSocket
        && !setOption(ReceiveOutOfBandData, 1)) {
        qWarning("QNativeSocketEngine::initialize unable to inline out-of-band data");
    }

    // Set the send and receive buffer sizes to a magic size, found
    // most optimal for our platforms.
    setReceiveBufferSize(49152);
    setSendBufferSize(49152);

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
bool QNativeSocketEngine::initialize(int socketDescriptor, QAbstractSocket::SocketState socketState)
{
    Q_D(QNativeSocketEngine);

    if (isValid())
        close();

    d->socketDescriptor = socketDescriptor;

    // determine socket type and protocol
    if (!d->fetchConnectionParameters()) {
#if defined (QNATIVESOCKETENGINE_DEBUG)
        qDebug("QNativeSocketEngine::initialize(socketDescriptor == %i) failed: %s",
               socketDescriptor, d->socketErrorString.toLatin1().constData());
#endif
        d->socketDescriptor = -1;
        return false;
    }

    if (d->socketType != QAbstractSocket::UnknownSocketType) {
        // Make the socket nonblocking.
        if (!setOption(NonBlockingSocketOption, 1)) {
            d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                QNativeSocketEnginePrivate::NonBlockingInitFailedErrorString);
            close();
            return false;
        }

        // Set the broadcasting flag if it's a UDP socket.
        if (d->socketType == QAbstractSocket::UdpSocket
            && !setOption(BroadcastSocketOption, 1)) {
            d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                QNativeSocketEnginePrivate::BroadcastingInitFailedErrorString);
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
bool QNativeSocketEngine::isValid() const
{
    Q_D(const QNativeSocketEngine);
    return d->socketDescriptor != -1;
}

/*!
    Returns the native socket descriptor. Any use of this descriptor
    stands the risk of being non-portable.
*/
int QNativeSocketEngine::socketDescriptor() const
{
    Q_D(const QNativeSocketEngine);
    return d->socketDescriptor;
}

/*!
    Connects to the IP address and port specified by \a address and \a
    port. If the connection is established, this function returns true
    and the socket enters ConnectedState. Otherwise, false is
    returned.

    If false is returned, state() should be called to see if the
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
        QNativeSocketEngine socketLayer;
        socketLayer.initialize(QAbstractSocket::TcpSocket, QAbstractSocket::IPv4Protocol);
        socketLayer.connectToHost(QHostAddress::LocalHost, 22);
        // returns false

        socketLayer.waitForWrite();
        socketLayer.connectToHost(QHostAddress::LocalHost, 22);
        // returns true
    \endcode

    Otherwise, error() should be called to determine the cause of the
    error.
*/
bool QNativeSocketEngine::connectToHost(const QHostAddress &address, quint16 port)
{
    Q_D(QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::connectToHost(), false);

#if defined (QT_NO_IPV6)
    if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                    QNativeSocketEnginePrivate::NoIpV6ErrorString);
        return false;
    }
#endif

    Q_CHECK_STATES(QNativeSocketEngine::connectToHost(),
                   QAbstractSocket::UnconnectedState, QAbstractSocket::ConnectingState, false);

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
bool QNativeSocketEngine::bind(const QHostAddress &address, quint16 port)
{
    Q_D(QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::bind(), false);

#if defined (QT_NO_IPV6)
    if (address.protocol() == QAbstractSocket::IPv6Protocol) {
        d->setError(QAbstractSocket::UnsupportedSocketOperationError,
                    QNativeSocketEnginePrivate::NoIpV6ErrorString);
        return false;
    }
#endif

    Q_CHECK_STATE(QNativeSocketEngine::bind(), QAbstractSocket::UnconnectedState, false);

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
        QNativeSocketEngine socketLayer;
        socketLayer.bind(QHostAddress::Any, 4000);
        socketLayer.listen();
        if (socketLayer.waitForRead()) {
            int clientSocket = socketLayer.accept();
            // a client is connected
        }
    \endcode

    \sa bind(), accept()
*/
bool QNativeSocketEngine::listen()
{
    Q_D(QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::listen(), false);
    Q_CHECK_STATE(QNativeSocketEngine::listen(), QAbstractSocket::BoundState, false);
    Q_CHECK_TYPE(QNativeSocketEngine::listen(), QAbstractSocket::TcpSocket, false);

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
int QNativeSocketEngine::accept()
{
    Q_D(QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::accept(), -1);
    Q_CHECK_STATE(QNativeSocketEngine::accept(), QAbstractSocket::ListeningState, false);
    Q_CHECK_TYPE(QNativeSocketEngine::accept(), QAbstractSocket::TcpSocket, false);

    return d->nativeAccept();
}

/*!
    Returns the number of bytes that are currently available for
    reading. On error, -1 is returned.

    For UDP sockets, this function returns the accumulated size of all
    pending datagrams, and it is therefore more useful for UDP sockets
    to call hasPendingDatagrams() and pendingDatagramSize().
*/
qint64 QNativeSocketEngine::bytesAvailable() const
{
    Q_D(const QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::bytesAvailable(), -1);
    Q_CHECK_NOT_STATE(QNativeSocketEngine::bytesAvailable(), QAbstractSocket::UnconnectedState, false);

    return d->nativeBytesAvailable();
}

/*!
    Returns true if there is at least one datagram pending. This
    function is only called by UDP sockets, where a datagram can have
    a size of 0. TCP sockets call bytesAvailable().
*/
bool QNativeSocketEngine::hasPendingDatagrams() const
{
    Q_D(const QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::hasPendingDatagrams(), false);
    Q_CHECK_NOT_STATE(QNativeSocketEngine::hasPendingDatagrams(), QAbstractSocket::UnconnectedState, false);
    Q_CHECK_TYPE(QNativeSocketEngine::hasPendingDatagrams(), QAbstractSocket::UdpSocket, false);

    return d->nativeHasPendingDatagrams();
}

/*!
    Returns the size of the pending datagram, or -1 if no datagram is
    pending. A datagram size of 0 is perfectly valid. This function is
    called by UDP sockets before receiveMessage(). For TCP sockets,
    call bytesAvailable().
*/
qint64 QNativeSocketEngine::pendingDatagramSize() const
{
    Q_D(const QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::pendingDatagramSize(), -1);
    Q_CHECK_TYPE(QNativeSocketEngine::pendingDatagramSize(), QAbstractSocket::UdpSocket, false);

    return d->nativePendingDatagramSize();
}

/*!
    Reads up to \a maxSize bytes of a datagram from the socket,
    stores it in \a data and returns the number of bytes read. The
    address and port of the sender are stored in \a address and \a
    port. If either of these pointers is 0, the corresponding value is
    discarded.

    To avoid unnecessarily loss of data, call pendingDatagramSize() to
    determine the size of the pending message before reading it. If \a
    maxSize is too small, the rest of the datagram will be lost.

    Returns -1 if an error occurred.

    \sa hasPendingDatagrams()
*/
qint64 QNativeSocketEngine::readDatagram(char *data, qint64 maxSize, QHostAddress *address,
                                      quint16 *port)
{
    Q_D(QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::readDatagram(), -1);
    Q_CHECK_TYPE(QNativeSocketEngine::readDatagram(), QAbstractSocket::UdpSocket, false);

    return d->nativeReceiveDatagram(data, maxSize, address, port);
}

/*!
    Writes a UDP datagram of size \a size bytes to the socket from
    \a data to the address \a host on port \a port, and returns the
    number of bytes written, or -1 if an error occurred.

    Only one datagram is sent, and if there is too much data to fit
    into a single datagram, the operation will fail and error()
    will return QAbstractSocket::DatagramTooLargeError. Operating systems impose an
    upper limit to the size of a datagram, but this size is different
    on almost all platforms. Sending large datagrams is in general
    disadvised, as even if they are sent successfully, they are likely
    to be fragmented before arriving at their destination.

    Experience has shown that it is in general safe to send datagrams
    no larger than 512 bytes.

    \sa readDatagram()
*/
qint64 QNativeSocketEngine::writeDatagram(const char *data, qint64 size,
                                       const QHostAddress &host, quint16 port)
{
    Q_D(QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::writeDatagram(), -1);
    Q_CHECK_TYPE(QNativeSocketEngine::writeDatagram(), QAbstractSocket::UdpSocket, -1);
    return d->nativeSendDatagram(data, size, host, port);
}

/*!
    Writes a block of \a size bytes from \a data to the socket.
    Returns the number of bytes written, or -1 if an error occurred.
*/
qint64 QNativeSocketEngine::write(const char *data, qint64 size)
{
    Q_D(QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::write(), -1);
    Q_CHECK_STATE(QNativeSocketEngine::write(), QAbstractSocket::ConnectedState, -1);
    return d->nativeWrite(data, size);
}

/*!
    Reads up to \a maxSize bytes into \a data from the socket.
    Returns the number of bytes read, or -1 if an error occurred.
*/
qint64 QNativeSocketEngine::read(char *data, qint64 maxSize)
{
    Q_D(QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::read(), -1);
    Q_CHECK_STATES(QNativeSocketEngine::read(), QAbstractSocket::ConnectedState, QAbstractSocket::BoundState, -1);

    qint64 readBytes = d->nativeRead(data, maxSize);

    // Handle remote close
    if (readBytes == 0 && d->socketType == QAbstractSocket::TcpSocket) {
        d->setError(QAbstractSocket::RemoteHostClosedError,
                    QNativeSocketEnginePrivate::RemoteHostClosedErrorString);
        close();
        return -1;
    }
    return readBytes;
}

/*!
    Closes the socket. In order to use the socket again, initialize()
    must be called.
*/
void QNativeSocketEngine::close()
{
    Q_D(QNativeSocketEngine);
    if (d->readNotifier)
        d->readNotifier->setEnabled(false);
    if (d->writeNotifier)
        d->writeNotifier->setEnabled(false);
    if (d->exceptNotifier)
        d->exceptNotifier->setEnabled(false);

    if(d->socketDescriptor != -1) {
        d->nativeClose();
        d->socketDescriptor = -1;
    }
    d->socketState = QAbstractSocket::UnconnectedState;
    d->hasSetSocketError = false;
    d->localPort = 0;
    d->localAddress.clear();
    d->peerPort = 0;
    d->peerAddress.clear();
    if (d->readNotifier) {
        delete d->readNotifier;
        d->readNotifier = 0;
    }
    if (d->writeNotifier) {
        delete d->writeNotifier;
        d->writeNotifier = 0;
    }
    if (d->exceptNotifier) {
        delete d->exceptNotifier;
        d->exceptNotifier = 0;
    }
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
bool QNativeSocketEngine::waitForRead(int msecs, bool *timedOut) const
{
    Q_D(const QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::waitForRead(), false);
    Q_CHECK_NOT_STATE(QNativeSocketEngine::waitForRead(),
                      QAbstractSocket::UnconnectedState, false);

    if (timedOut)
        *timedOut = false;

    int ret = d->nativeSelect(msecs, true);
    if (ret == 0) {
        if (timedOut)
            *timedOut = true;
        d->setError(QAbstractSocket::SocketTimeoutError,
            QNativeSocketEnginePrivate::TimeOutErrorString);
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
bool QNativeSocketEngine::waitForWrite(int msecs, bool *timedOut) const
{
    Q_D(const QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::waitForWrite(), false);
    Q_CHECK_NOT_STATE(QNativeSocketEngine::waitForWrite(),
                      QAbstractSocket::UnconnectedState, false);

    if (timedOut)
        *timedOut = false;

    int ret = d->nativeSelect(msecs, false);
    if (ret == 0) {
        if (timedOut)
            *timedOut = true;
        d->setError(QAbstractSocket::SocketTimeoutError,
                    QNativeSocketEnginePrivate::TimeOutErrorString);
        return false;
    }

    return ret > 0;
}

bool QNativeSocketEngine::waitForReadOrWrite(bool *readyToRead, bool *readyToWrite,
                                      bool checkRead, bool checkWrite,
                                      int msecs, bool *timedOut) const
{
    Q_D(const QNativeSocketEngine);
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::waitForWrite(), false);
    Q_CHECK_NOT_STATE(QNativeSocketEngine::waitForReadOrWrite(),
                      QAbstractSocket::UnconnectedState, false);

    int ret = d->nativeSelect(msecs, checkRead, checkWrite, readyToRead, readyToWrite);
    if (ret == 0) {
        if (timedOut)
            *timedOut = true;
        d->setError(QAbstractSocket::SocketTimeoutError,
                    QNativeSocketEnginePrivate::TimeOutErrorString);
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
qint64 QNativeSocketEngine::receiveBufferSize() const
{
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::receiveBufferSize(), -1);
    return option(ReceiveBufferSocketOption);
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
void QNativeSocketEngine::setReceiveBufferSize(qint64 size)
{
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::setReceiveBufferSize(), Q_VOID);
    setOption(ReceiveBufferSocketOption, size);
}

/*!
    Returns the size of the operating system send buffer. Depending on
    the operating system, this size may be different from what has
    been set earlier with setSendBufferSize().
*/
qint64 QNativeSocketEngine::sendBufferSize() const
{
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::setSendBufferSize(), -1);
    return option(SendBufferSocketOption);
}

/*!
    Sets the size of the operating system send buffer to \a size.

    The operating system send buffer size effectively limits how much
    data can be in transit at any one moment. Setting the size of the
    send buffer may have an impact on the socket's performance.

    The default value is operating system-dependent.
*/
void QNativeSocketEngine::setSendBufferSize(qint64 size)
{
    Q_CHECK_VALID_SOCKETLAYER(QNativeSocketEngine::setSendBufferSize(), Q_VOID);
    setOption(SendBufferSocketOption, size);
}


/*!
    Sets the option \a option to the value \a value.
*/
bool QNativeSocketEngine::setOption(SocketOption option, int value)
{
    Q_D(QNativeSocketEngine);
    return d->setOption(option, value);
}

/*!
    Returns the value of the option \a socketOption.
*/
int QNativeSocketEngine::option(SocketOption socketOption) const
{
    Q_D(const QNativeSocketEngine);
    return d->option(socketOption);
}

bool QNativeSocketEngine::isReadNotificationEnabled() const
{
    Q_D(const QNativeSocketEngine);
    return d->readNotifier && d->readNotifier->isEnabled();
}

void QNativeSocketEngine::setReadNotificationEnabled(bool enable)
{
    Q_D(QNativeSocketEngine);
    if (d->readNotifier) {
        d->readNotifier->setEnabled(enable);
    } else if (enable && d->threadData->eventDispatcher) {
        d->readNotifier = new QSocketNotifier(d->socketDescriptor,
                                              QSocketNotifier::Read, this);
        QObject::connect(d->readNotifier, SIGNAL(activated(int)),
                         this, SLOT(readNotification()));
        d->readNotifier->setEnabled(true);
    }
}

bool QNativeSocketEngine::isWriteNotificationEnabled() const
{
    Q_D(const QNativeSocketEngine);
    return d->writeNotifier && d->writeNotifier->isEnabled();
}

void QNativeSocketEngine::setWriteNotificationEnabled(bool enable)
{
    Q_D(QNativeSocketEngine);
    if (d->writeNotifier) {
        d->writeNotifier->setEnabled(enable);
    } else if (enable && d->threadData->eventDispatcher) {
        d->writeNotifier = new QSocketNotifier(d->socketDescriptor,
                                              QSocketNotifier::Write, this);
        QObject::connect(d->writeNotifier, SIGNAL(activated(int)),
                         this, SLOT(writeNotification()));
        d->writeNotifier->setEnabled(true);
    }
}

bool QNativeSocketEngine::isExceptionNotificationEnabled() const
{
    Q_D(const QNativeSocketEngine);
    return d->exceptNotifier && d->exceptNotifier->isEnabled();
}

void QNativeSocketEngine::setExceptionNotificationEnabled(bool enable)
{
    Q_D(QNativeSocketEngine);
    if (d->exceptNotifier) {
        d->exceptNotifier->setEnabled(enable);
    } else if (enable && d->threadData->eventDispatcher) {
        d->exceptNotifier = new QSocketNotifier(d->socketDescriptor,
                                              QSocketNotifier::Exception, this);
        QObject::connect(d->exceptNotifier, SIGNAL(activated(int)),
                         this, SIGNAL(exceptionNotification()));
        d->exceptNotifier->setEnabled(true);
    }
}
