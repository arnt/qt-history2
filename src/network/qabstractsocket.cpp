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

//#define QABSTRACTSOCKET_DEBUG

/*! \class QAbstractSocket

    \brief The QAbstractSocket class provides the base functionality
    common to all socket types.

    \reentrant
    \ingroup io
    \module network

    QAbstractSocket is the base class for QTcpSocket and QUdpSocket
    and contains all common functionality of these two classes. If
    you need a socket, you have two options:

    \list
    \i  Instantiate QTcpSocket or QUdpSocket.
    \i  Create a native socket descriptor, instantiate
	QAbstractSocket, and call setSocketDescriptor() to wrap the
	native socket.
    \endlist

    TCP (Transmission Control Protocol) is a reliable,
    stream-oriented, connection-oriented transport protocol. UDP
    (User Datagram Protocol) is an unreliable, datagram-oriented,
    connectionless protocol. In practice, this means that TCP is
    better suited for continuous transmission of data, whereas the
    more lightweight UDP can be used when reliability isn't
    important.

    QAbstractSocket's API unifies most of the differences between the
    two protocols. For example, although UDP is connectionless,
    connectToHost() establishes a virtual connection for UDP sockets,
    enabling you to use QAbstractSocket in more or less the same way
    regardless of the underlying protocol. Internally,
    QAbstractSocket remembers the address and port passed to
    connectToHost(), and functions like read() and write() use these
    values.

    At any time, QAbstractSocket has a state (returned by
    state()). The initial state is QAbstractSocket::UnconnectedState. After
    calling connectToHost(), the socket first enters
    QAbstractSocket::HostLookupState. If the host is found, QAbstractSocket enters
    QAbstractSocket::ConnectingState and emits the hostFound() signal. When the
    connection has been established, it enters QAbstractSocket::ConnectedState and
    emits connected(). If an error occurs at any stage, error() is
    emitted. Whenever the state changes, stateChanged() is emitted.
    For convenience, isValid() returns true if the socket is ready for
    reading and writing.

    Read or write data by calling read() or write(), or use the
    convenience functions readLine() and readAll(). QAbstractSocket
    also inherits getChar(), putChar(), and ungetChar() from
    QIODevice, which work on single bytes. For every chunk of data
    that has been written to the socket, the bytesWritten() signal is
    emitted.

    The readyRead() signal is emitted every time a new chunk of data
    has arrived. bytesAvailable() then returns the number of bytes
    that are available for reading. Typically, you would connect the
    readyRead() signal to a slot and read all available data there.
    If you don't read all the data at once, the remaining data will
    still be available later, and any new incoming data will be
    appended to QAbstractSocket's internal read buffer. To limit the
    size of the read buffer, call setReadBufferSize().

    To close the socket, call close(). QAbstractSocket enters
    QAbstractSocket::ClosingState, then emits closing(). After all pending data
    has been written to the socket, QAbstractSocket actually closes
    the socket, enters QAbstractSocket::ClosedState, and emits closed(). If you
    want to abort a connection immediately, discarding all pending
    data, call abort() instead.

    The port and address of the connected peer is fetched by calling
    peerPort() and peerAddress(). peerName() returns the host name of
    the peer, as passed to connectToHost(). localPort() and
    localAddress() return the port and address of the local socket.

    QAbstractSocket provides a set of functions that suspend the
    calling thread until certain signals are emitted. These functions
    can be used to implement blocking sockets:

    \list
    \o waitForConnected() blocks until a connection has been established.

    \o waitForReadyRead() blocks until new data is available for
    reading.

    \o waitForBytesWritten() blocks until one payload of data has been
    written to the socket.

    \o waitForClosed() blocks until the connection has closed.
    \endlist

    Programming with a blocking socket is radically different from
    programming with a non-blocking socket. A blocking socket doesn't
    require an event loop and typically leads to simpler code.
    However, in a GUI application, blocking sockets should only be
    used in non-GUI threads, to avoid freezing the user interface.
    See the \l network/fortuneclient and \l network/blockingfortuneclient
    examples for an overview of both approaches.

    QAbstractSocket can be used with QTextStream and QDataStream's
    stream operators (operator<<() and operator>>()). There is one
    issue to be aware of, though: You must make sure that enough data
    is available before attempting to read it using operator>>().

    \sa QFtp, QHttp, QTcpServer
*/

/*!
    \fn void QAbstractSocket::hostFound()

    This signal is emitted after connectToHost() has been called and
    the host lookup has succeeded.

    \sa connected()
*/

/*!
    \fn void QAbstractSocket::connected()

    This signal is emitted after connectToHost() has been called and
    a connection has been successfully established.

    \sa connectToHost(), connectionClosed()
*/

/*!
    \fn void QAbstractSocket::closing()

    This signal is emitted when the connection is closing, before any
    pending data has been written to the network.

    QAbstractSocket stops receiving data (and no longer emits
    readyRead()) after closing() has been emitted. Any pending data is
    still available, and can still be read, but no more data will be
    read from the network.

    \sa closed()
*/

/*!
    \fn void QAbstractSocket::closed()

    This signal is emitted when the connection has been closed.

    \sa connectToHost(), close()
*/

/*!
    \fn void QAbstractSocket::error(int socketError)

    This signal is emitted after an error occurred. The \a
    socketError parameter is a \l QAbstractSocket::SocketError value.

    \sa error(), errorString()
*/

/*!
    \fn void QAbstractSocket::stateChanged(int socketState)

    This signal is emitted whenever QAbstractSocket's state changes.
    The \a socketState parameter is a \l QAbstractSocket::SocketState value.

    \sa state()
*/

/*! \internal
    \enum QAbstractSocket::NetworkLayerProtocol

    This enum describes the network layer protocol values used in Qt.

    \value IPv4Protocol IPv4
    \value IPv6Protocol IPv6
    \value UnknownNetworkLayerProtocol Other than IPv4 and IPv6

    \sa QSocketLayer::protocol()
*/

/*! \enum QAbstractSocket::SocketType

    This enum describes the transport layer protocol.

    \value TcpSocket TCP
    \value UdpSocket UDP
    \value UnknownSocketType Other than TCP and UDP

    \sa QAbstractSocket::socketType()
*/

/*! \enum QAbstractSocket::SocketError

    This enum describes the socket errors that can occur.

    \value ConnectionRefusedError The connection was refused by the
           peer (or timed out).
    \value RemoteHostClosedError The remote host closed the
           connection.
    \value HostNotFoundError The host address was not found.
    \value SocketAccessError The socket operation failed because the
           application lacked the required privileges.
    \value SocketResourceError The local system ran out of resources
           (e.g., too many sockets).
    \value SocketTimeoutError The socket operation timed out.
    \value DatagramTooLargeError The datagram was larger than the
           operating system's limit (which can be as low as 8192
           bytes).
    \value NetworkError An error occurred with the network (e.g., the
           network cable was accidentally plugged out).
    \value AddressInUseError The address specified to QUdpSocket::bind() is
           already in use and was set to be exclusive.
    \value SocketAddressNotAvailableError The address specified to
           QUdpSocket::bind() does not belong to the host.
    \value UnsupportedSocketOperationError The requested socket operation is
           not supported by the local operating system (e.g., lack of
           IPv6 support).
    \value UnknownSocketError An unidentified error occurred.

    \sa QAbstractSocket::error()
*/

/*!
    \enum QAbstractSocket::SocketState

    This enum describes the different states in which a socket can be.

    \value UnconnectedState The socket is not connected.
    \value HostLookupState The socket is performing a host name lookup.
    \value ConnectingState The socket has started establishing a connection.
    \value ConnectedState A connection is established.
    \value BoundState The socket is bound to an address and port (for servers).
    \value ClosingState The socket is about to close (data may still
    be waiting to be written).
    \value ListeningState For internal use only.
    \omitvalue Idle
    \omitvalue HostLookup
    \omitvalue Connecting
    \omitvalue Connected
    \omitvalue Closing
    \omitvalue Connection

    \sa QAbstractSocket::state()
*/

#include "qabstractsocket.h"
#include "qabstractsocket_p.h"

#include <qabstracteventdispatcher.h>
#include <qdatetime.h>
#include <qhostaddress.h>
#include <qpointer.h>
#include <qsignal.h>
#include <qtimer.h>

#include <time.h>

#define QABSTRACTSOCKET_BUFFERSIZE 32768
#define QT_CONNECT_TIMEOUT 30000
#define QT_TRANSFER_TIMEOUT 120000

#if defined QABSTRACTSOCKET_DEBUG
#include <qstring.h>
#include <ctype.h>

/*
    Returns a human readable representation of the first \a len
    characters in \a data.
*/
static QByteArray qt_prettyDebug(const char *data, int len, int maxLength)
{
    if (!data) return "(null)";
    QByteArray out;
    for (int i = 0; i < len; ++i) {
        char c = data[i];
        if (isprint(c)) {
            out += c;
        } else switch (c) {
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            QString tmp;
            tmp.sprintf("\\%o", c);
            out += tmp.toLatin1();
        }
    }

    if (len < maxLength)
        out += "...";

    return out;
}
#endif

#define d d_func()
#define q q_func()

/*! \internal

    Constructs a QAbstractSocketPrivate. Initializes all members.
*/
QAbstractSocketPrivate::QAbstractSocketPrivate()
    : readBuffer(QABSTRACTSOCKET_BUFFERSIZE),
      writeBuffer(QABSTRACTSOCKET_BUFFERSIZE)
{
    port = 0;
    readSocketNotifier = 0;
    writeSocketNotifier = 0;
    readSocketNotifierCalled = false;
    isBuffered = false;
    blockingTimeout = 30000;
    state = QAbstractSocket::UnconnectedState;
    readBufferMaxSize = 0;
    socketError = QAbstractSocket::UnknownSocketError;
}

/*! \internal

    Destructs the QAbstractSocket. If the socket layer is open, it
    will be reset.
*/
QAbstractSocketPrivate::~QAbstractSocketPrivate()
{
    if (socketLayer.isValid())
        resetSocketLayer();
}

/*! \internal

    Resets the socket layer, clears the read and write buffers and
    deletes any socket notifiers.
*/
void QAbstractSocketPrivate::resetSocketLayer()
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::resetSocketLayer()");
#endif

    if (readSocketNotifier)
        readSocketNotifier->setEnabled(false);
    delete readSocketNotifier;
    readSocketNotifier = 0;

    if (writeSocketNotifier)
        writeSocketNotifier->setEnabled(false);
    delete writeSocketNotifier;
    writeSocketNotifier = 0;

    if (socketLayer.isValid())
        socketLayer.close();
}

/*! \internal

    Creates one read and one write socket notifier, and disables them
    both. Connects their signals to the respective private slots
    canReadNotification() and canWriteNotification().
*/
void QAbstractSocketPrivate::setupSocketNotifiers()
{
    readSocketNotifier = new QSocketNotifier(socketLayer.socketDescriptor(),
                                             QSocketNotifier::Read);
    writeSocketNotifier = new QSocketNotifier(socketLayer.socketDescriptor(),
                                              QSocketNotifier::Write);
    readSocketNotifier->setEnabled(false);
    writeSocketNotifier->setEnabled(false);

    QObject::connect(readSocketNotifier, SIGNAL(activated(int)),
                     q, SLOT(canReadNotification(int)));
    QObject::connect(writeSocketNotifier, SIGNAL(activated(int)),
                     q, SLOT(canWriteNotification(int)));
}

/*! \internal

    Initializes the socket layer to by of type \a type, using the
    network layer protocol \a protocol. Resets the socket layer first
    if it's already initialized. Sets up the socket notifiers.
*/
bool QAbstractSocketPrivate::initSocketLayer(QAbstractSocket::SocketType type,
                                            QAbstractSocket::NetworkLayerProtocol protocol)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    QString typeStr;
    if (type == QAbstractSocket::TcpSocket) typeStr = "TcpSocket";
    else if (type == QAbstractSocket::UdpSocket) typeStr = "UdpSocket";
    else typeStr = "UnknownSocketType";
    QString protocolStr;
    if (protocol == QAbstractSocket::IPv4Protocol) protocolStr = "IPv4Protocol";
    else if (protocol == QAbstractSocket::IPv6Protocol) protocolStr = "IPv6Protocol";
    else protocolStr = "UnknownNetworkLayerProtocol";
#endif

    resetSocketLayer();

    if (!socketLayer.initialize(type, protocol)) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::initSocketLayer(%s, %s) failed (%s)",
               typeStr.latin1(), protocolStr.latin1(),
               socketLayer.errorString().latin1());
#endif
        socketError = socketLayer.error();
	q->setErrorString(socketLayer.errorString());
        return false;
    }

    if (QAbstractEventDispatcher::instance(q->thread()))
        setupSocketNotifiers();

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::initSocketLayer(%s, %s) success",
           typeStr.latin1(), protocolStr.latin1());
#endif
    return true;
}

/*! \internal

    Slot connected to the read socket notifier. This slot is called
    when new data is available for reading, or when the socket has
    been closed. Handles recursive calls.
*/
bool QAbstractSocketPrivate::canReadNotification(int)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::canReadNotification()");
#endif

    // Prevent recursive calls
    if (readSocketNotifierCalled) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::canReadNotification() recursive call detected.");
#endif
        return false;
    }
    readSocketNotifierCalled = true;

    // If buffered, read data from the socket into the read buffer
    if (isBuffered) {
        // Return if there is no space in the buffer
        if (readBufferMaxSize && readBuffer.size() >= readBufferMaxSize) {
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::canReadNotification() buffer is full");
#endif
            readSocketNotifierCalled = false;
            return false;
        }

        // If reading from the socket fails after getting a read
        // notification, close the socket.
        if (!d->readFromSocket()) {
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::canReadNotification() disconnecting socket");
#endif
            q->disconnectFromHost();
            readSocketNotifierCalled = false;
            return false;
        }

        // If read buffer is full, disable the read socket notifier.
        if (readBufferMaxSize && readBuffer.size() == readBufferMaxSize) {
            if (d->readSocketNotifier)
                readSocketNotifier->setEnabled(false);
        }
    }

    // If we were closed as a result of the readyRead() signal,
    // return.
    if (state == QAbstractSocket::UnconnectedState || state == QAbstractSocket::ClosingState) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::canReadNotification() socket is closing - returning");
#endif
        readSocketNotifierCalled = false;
        return true;
    }

    readSocketNotifierCalled = false;
    emit q->readyRead();
    return true;
}

/*! \internal

    Slot connected to the write socket notifier. It's called during a
    delayed connect or when the socket is ready for writing.
*/
bool QAbstractSocketPrivate::canWriteNotification(int)
{
#if defined (Q_OS_WIN)
    if (writeSocketNotifier && writeSocketNotifier->isEnabled())
        writeSocketNotifier->setEnabled(false);
#endif

    // If in connecting state, check if the connection has been
    // established, otherwise flush pending data.
    if (state == QAbstractSocket::ConnectingState) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::canWriteNotification() testing connection");
#endif
        testConnection();
        return false;
    }

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::canWriteNotification() flushing");
#endif
    int tmp = writeBuffer.size();
    flush();

#if defined (Q_OS_WIN)
    if (!writeBuffer.isEmpty() && writeSocketNotifier)
        writeSocketNotifier->setEnabled(true);
#else
    if (writeBuffer.isEmpty() && writeSocketNotifier)
        writeSocketNotifier->setEnabled(false);
#endif

    return (writeBuffer.size() < tmp);
}

/*! \internal

    Writes pending data in the write buffers to the socket. The
    function writes as much as it can without blocking.

    It is usually invoked by canWriteNotification after one or more
    calls to write().

    Emits bytesWritten().
*/
bool QAbstractSocketPrivate::flush()
{
    if (!socketLayer.isValid() || writeBuffer.isEmpty()) {
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::flush() nothing to do: valid ? %s, writeBuffer.isEmpty() ? %s",
           socketLayer.isValid() ? "yes" : "no", writeBuffer.isEmpty() ? "yes" : "no");
#endif
        return false;
    }

    int nextSize = writeBuffer.nextDataBlockSize();
    char *ptr = writeBuffer.readPointer();

    // Attempt to write it all in one chunk.
    Q_LONGLONG written = socketLayer.write(ptr, nextSize);
    if (written < 0) {
        socketError = socketLayer.error();
        q->setErrorString(socketLayer.errorString());
        emit q->error(socketError);
        // an unexpected error so close the socket.
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::flush() write error, aborting.");
#endif
        q->abort();
        return false;
    }

#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::flush() %lld bytes written to the network",
               written);
#endif

    // Remove what we wrote so far.
    writeBuffer.free(written);
    if (written > 0)
        emit q->bytesWritten(written);

    if (writeBuffer.isEmpty() && d->writeSocketNotifier && d->writeSocketNotifier->isEnabled())
        d->writeSocketNotifier->setEnabled(false);
    if (state == QAbstractSocket::ClosingState)
        q->close();

    return true;
}

/*! \internal

    Slot connected to QDns::getHostByName() in connectToHost(). This
    function starts the process of connecting to any number of
    candidate IP addresses for the host, if it was found. Calls
    connectToNextAddress().
*/
void QAbstractSocketPrivate::startConnecting(const QDnsHostInfo &hostInfo)
{
    addresses = hostInfo.addresses();

#if defined(QABSTRACTSOCKET_DEBUG)
    QString s = "{";
    for (int i = 0; i < addresses.count(); ++i) {
        if (i != 0) s += ", ";
        s += addresses.at(i).toString();
    }
    s += "}";
    qDebug("QAbstractSocketPrivate::startConnecting(hostInfo == %s)", s.latin1());
#endif

    // Try all addresses twice.
    addresses += addresses;

    // If there are no addresses in the host list, report this to the
    // user.
    if (addresses.isEmpty()) {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::startConnecting(), host not found");
#endif
        state = QAbstractSocket::UnconnectedState;
        socketError = QAbstractSocket::HostNotFoundError;
        q->setErrorString(QT_TRANSLATE_NOOP(QAbstractSocket, "Host not found"));
        emit q->stateChanged(state);
        emit q->error(QAbstractSocket::HostNotFoundError);
        return;
    }

    // Enter Connecting state (see also sn_write, which is called by
    // the write socket notifier after connect())
    state = QAbstractSocket::ConnectingState;
    emit q->stateChanged(state);

    // Report the successful host lookup
    emit q->hostFound();

    // Reset the total time spent connecting.
    connectTimeElapsed = 0;

    // The addresses returned by the lookup will be tested one after
    // another by connectToNextAddress().
    connectToNextAddress();
}

/*! \internal

    Called by a queued or direct connection from startConnecting() or
    testConnection(), this function takes the first address of the
    pending addresses list and tries to connect to it. If the
    connection succeeds, QAbstractSocket will emit
    connected(). Otherwise, error(ConnectionRefusedError) or
    error(SocketTimeoutError) is emitted.
*/
void QAbstractSocketPrivate::connectToNextAddress()
{
    do {
        // Check for more pending addresses
        if (addresses.isEmpty()) {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::connectToNextAddress(), all addresses failed.");
#endif
            state = QAbstractSocket::UnconnectedState;
            socketError = QAbstractSocket::ConnectionRefusedError;
            q->setErrorString(QT_TRANSLATE_NOOP(QAbstractSocket,
                                                "Connection refused"));
            emit q->error(QAbstractSocket::ConnectionRefusedError);
            return;
        }

        // Pick the first host address candidate
        host = addresses.takeFirst();
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::connectToNextAddress(), connecting to %s:%i",
               host.toString().latin1(), port);
#endif

        // Determine its protocol.
        QAbstractSocket::NetworkLayerProtocol protocol = host.protocol();
#if defined(Q_NO_IPv6)
        if (protocol == QAbstractSocket::IPv4Protocol) {
            // If we have no IPv6 support, then we will not be able to
            // connect. So we just pretend we didn't see this address.
            continue;
        }
#endif

        // Perhaps reinitialize the socket layer if its protocol
        // doesn't match the address.
        if (!socketLayer.isValid() || socketLayer.protocol() != protocol
            || socketLayer.state() != QAbstractSocket::UnconnectedState) {
            // ### might fail
            initSocketLayer(q->socketType(), protocol);
        }

        // Tries to connect to the address. If it succeeds immediately
        // (localhost address on BSD or any UDP connect), emit
        // connected() and return.
        if (socketLayer.connectToHost(host, port)) {
            state = QAbstractSocket::ConnectedState;
            emit q->stateChanged(state);
            if (d->readSocketNotifier)
                readSocketNotifier->setEnabled(true);
            if (d->writeSocketNotifier && !d->writeBuffer.isEmpty())
                writeSocketNotifier->setEnabled(true);
            emit q->connected();
            return;
        }

        // Check that we're in delayed connection state. If not, an
        // error has occurred.
        if (socketLayer.state() != QAbstractSocket::ConnectingState) {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::connectToNextAddress(), connection failed (%s)",
                   socketLayer.errorString().latin1());
#endif
            socketError = socketLayer.error();
            q->setErrorString(socketLayer.errorString());
            emit q->error(socketError);
            return;
        }

        // Start the connect timer.
        if (QAbstractEventDispatcher::instance(q->thread()))
            d->connectTimer.start(QT_CONNECT_TIMEOUT);

        // Wait for a write notification that will eventually call
        // testConnection().
        if (d->writeSocketNotifier)
            d->writeSocketNotifier->setEnabled(true);
        break;
    } while (state != QAbstractSocket::ConnectedState);
}

/*! \internal

    Tests if a connection has been established. If it has, connected()
    is emitted. Otherwise, connectToNextAddress() is invoked.
*/
void QAbstractSocketPrivate::testConnection()
{
    if (QAbstractEventDispatcher::instance(q->thread()))
        connectTimer.stop();

    if (socketLayer.state() == QAbstractSocket::ConnectedState || socketLayer.connectToHost(host, port)) {
        state = QAbstractSocket::ConnectedState;
        emit q->stateChanged(state);

        if (d->readSocketNotifier)
            readSocketNotifier->setEnabled(true);
        if (d->writeSocketNotifier)
            writeSocketNotifier->setEnabled(true);

        emit q->connected();
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::testConnection() connection to %s:%i established",
               host.toString().latin1(), port);
#endif
        return;
    }

#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::testConnection() connection failed,"
           " checking for alternative addresses");
#endif
    connectToNextAddress();
}

/*! \internal

    This function is called after a certain number of seconds has
    passed while waiting for a connection. It simply tests the
    connection, and continues to the next address if the connection
    failed.
*/
void QAbstractSocketPrivate::abortConnectionAttempt()
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::abortConnectionAttempt() (timed out)");
#endif
    if (d->writeSocketNotifier)
        d->writeSocketNotifier->setEnabled(false);

    testConnection();
}

/*! \internal

    Reads data from the socket layer into the read buffer. Returns
    true on success; otherwise false.
*/
bool QAbstractSocketPrivate::readFromSocket()
{
    // Find how many bytes we can read from the socket layer.
    Q_LONGLONG bytesToRead = socketLayer.bytesAvailable();
    if (readBufferMaxSize && bytesToRead > (readBufferMaxSize - readBuffer.size()))
        bytesToRead = readBufferMaxSize - readBuffer.size();

#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::readFromSocket() about to read %d bytes",
           int(bytesToRead));
#endif

    // Read from the socket, store data in the read buffer.
    char *ptr = d->readBuffer.reserve(bytesToRead);
    Q_LONGLONG readBytes = socketLayer.read(ptr, bytesToRead);
    if (readBytes > 0)
        d->readBuffer.truncate((int) (bytesToRead - readBytes));

    if (!socketLayer.isValid()) {
        socketError = socketLayer.error();
        q->setErrorString(socketLayer.errorString());
        emit q->error(socketError);
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::readFromSocket() read failed: %s",
               q->errorString().latin1());
#endif
        d->resetSocketLayer();
        return false;
    }

    return true;
}

/*! \internal

    Constructs a new abstract socket of type \a socketType. The \a
    parent argument is passed to QObject's constructor.
*/
QAbstractSocket::QAbstractSocket(SocketType socketType,
                                 QAbstractSocketPrivate &dd, QObject *parent)
    : QIODevice(dd, parent)
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::QAbstractSocket(%sSocket, QAbstractSocketPrivate == %p, parent == %p)",
           socketType == TcpSocket ? "Tcp" : socketType == UdpSocket
           ? "Udp" : "Unknown", &dd, parent);
#endif
    d->socketType = socketType;

    QObject::connect(&d->connectTimer, SIGNAL(timeout()), SLOT(abortConnectionAttempt()));
}

/*!
    Creates a new abstract socket of type \a socketType. The \a
    parent argument is passed to QObject's constructor.

    \sa socketType(), QTcpSocket, QUdpSocket
*/
QAbstractSocket::QAbstractSocket(SocketType socketType, QObject *parent)
    : QIODevice(*new QAbstractSocketPrivate, parent)
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::QAbstractSocket(%p)", parent);
#endif
    d->socketType = socketType;

    QObject::connect(&d->connectTimer, SIGNAL(timeout()), SLOT(abortConnectionAttempt()));
}

/*!
    Destroys the socket.
*/
QAbstractSocket::~QAbstractSocket()
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::~QAbstractSocket()");
#endif
    if (d->state != UnconnectedState)
        abort();
}

/*!
    Returns true if the socket is valid and ready for use; otherwise
    returns false.

    \sa state()
*/
bool QAbstractSocket::isValid() const
{
    return d->socketLayer.isValid();
}

/*!
    Attempts to make a connection to \a hostName on the given \a port.

    The socket is opened in the given \a openMode and first enters
    HostLookupState, then performs a host name lookup of \a hostName.
    If the lookup succeeds, hostFound() is emitted and QAbstractSocket
    enters ConnectingState. It then attempts to connect to the address
    or addresses returned by the lookup. Finally, if a connection is
    established, QAbstractSocket enters ConnectedState and
    emits connected().

    At any point, the socket can emit error() to signal that an error
    occurred.

    \a hostName may be an IP address in string form (e.g.,
    "43.195.83.32"), or it may be a host name (e.g.,
    "www.trolltech.com"). QAbstractSocket will do a lookup only if
    required. \a port is in native byte order.

    \sa state(), peerName(), peerAddress(), peerPort(), waitForConnected()
*/
void QAbstractSocket::connectToHost(const QString &hostName, Q_UINT16 port,
                                    OpenMode openMode)
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::connectToHost(\"%s\", %i, %i)...", hostName.latin1(), port,
           (int) openMode);
#endif

    if (d->state == ConnectedState || d->state == ConnectingState) {
        qWarning("QAbstractSocket::connectToHost() called when already connecting/connected");
        return;
    }

    d->hostName = hostName;
    d->port = port;
    d->state = HostLookupState;
    d->readBuffer.clear();
    d->writeBuffer.clear();
    d->closeCalled = false;

    setOpenMode(openMode);
    emit stateChanged(d->state);

    QHostAddress temp;
    if (temp.setAddress(hostName)) {
        d->startConnecting(QDns::getHostByName(hostName));
    } else {
        if (QAbstractEventDispatcher::instance(q->thread()))
            QDns::getHostByName(hostName, this, SLOT(startConnecting(const QDnsHostInfo &)));
    }

#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::connectToHost(\"%s\", %i) == %s%s", hostName.latin1(), port,
           (d->state == ConnectedState) ? "true" : "false",
           (d->state == ConnectingState || d->state == HostLookupState)
           ? " (connection in progress)" : "");
#endif
}

/*! \overload

    Attempts to make a connection to \a address on port \a port.
*/
void QAbstractSocket::connectToHost(const QHostAddress &address, Q_UINT16 port,
                                    OpenMode openMode)
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::connectToHost([%s], %i, %i)...",
           address.toString().latin1(), port, (int) openMode);
#endif
    connectToHost(address.toString(), port, openMode);
}

/*!
    Returns the number of bytes that are waiting to be written. The
    bytes are written when control goes back to the event loop or
    when flush() is called.

    \sa bytesAvailable(), flush()
*/
Q_LONGLONG QAbstractSocket::bytesToWrite() const
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::bytesToWrite() == %i", d->writeBuffer.size());
#endif
    return (Q_LONGLONG)d->writeBuffer.size();
}

/*!
    Returns the number of incoming bytes that are waiting to be read.

    \sa bytesToWrite(), read()
*/
Q_LONGLONG QAbstractSocket::bytesAvailable() const
{
    Q_LONGLONG available = 0;
    if (d->isBuffered)
        available = (Q_LONGLONG) d->readBuffer.size();
    else if (d->socketLayer.isValid())
        available = d->socketLayer.bytesAvailable();
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::bytesAvailable() == %llu", available);
#endif
    return available;
}

/*!
    Returns the host port number (in native byte order) of the local
    socket if available; otherwise returns 0.

    \sa localAddress(), peerPort()
*/
Q_UINT16 QAbstractSocket::localPort() const
{
    if (!d->socketLayer.isValid())
        return 0;
    return d->socketLayer.localPort();
}

/*!
    Returns the host address of the local socket if available;
    otherwise returns QHostAddress::Null.

    This is normally the main IP address of the host, but can be
    QHostAddress::LocalHost (127.0.0.1) for connections to the
    local host.

    \sa localPort(), peerAddress()
*/
QHostAddress QAbstractSocket::localAddress() const
{
    if (!d->socketLayer.isValid())
        return QHostAddress();
    return d->socketLayer.localAddress();
}

/*!
    Returns the port of the connected peer if the socket is in
    ConnectedState; otherwise returns 0.

    \sa peerAddress(), localPort()
*/
Q_UINT16 QAbstractSocket::peerPort() const
{
    if (!d->socketLayer.isValid())
        return 0;
    return d->socketLayer.peerPort();
}

/*!
    Returns the address of the connected peer if the socket is in
    ConnectedState; otherwise returns QHostAddress::Null.

    \sa peerName(), peerPort(), localAddress()
*/
QHostAddress QAbstractSocket::peerAddress() const
{
    if (!d->socketLayer.isValid())
        return QHostAddress();
    return d->socketLayer.peerAddress();
}

/*!
    Returns the name of the peer as specified by connectToHost(), or
    an empty QString if connectToHost() has not been called.

    \sa peerAddress(), peerPort()
*/
QString QAbstractSocket::peerName() const
{
    return d->hostName;
}

/*!
    Returns true if a line of data can be read from the socket;
    otherwise returns false.

    \sa readLine()
*/
bool QAbstractSocket::canReadLine() const
{
    bool hasLine = d->readBuffer.canReadLine();
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::canReadLine() == %s", hasLine ? "true" : "false");
#endif
    return hasLine;
}

/*!
    Returns the native socket descriptor of QAbstractSocket if this is
    available; otherwise returns -1.

    The socket descriptor is not available when QAbstractSocket is in
    UnconnectedState.

    \sa setSocketDescriptor()
*/
int QAbstractSocket::socketDescriptor() const
{
    return d->socketLayer.socketDescriptor();
}

/*!
    Initializes QAbstractSocket with the native socket descriptor \a
    socketDescriptor. Returns true if \a socketDescriptor is accepted
    as a valid socket descriptor; otherwise returns false.
    The socket is opened in the mode specified by \a openMode, and
    enters the socket state specified by \a socketState.

    \sa socketDescriptor()
*/
bool QAbstractSocket::setSocketDescriptor(int socketDescriptor, SocketState socketState,
                                          OpenMode openMode)
{
    bool result = d->socketLayer.initialize(socketDescriptor, socketState);
    if (!result) {
        d->socketError = d->socketLayer.error();
        setErrorString(d->socketLayer.errorString());
        return false;
    }

    if (QAbstractEventDispatcher::instance(thread()))
        d->setupSocketNotifiers();

    setOpenMode(openMode);

    if (d->state != socketState) {
        d->state = socketState;
        emit stateChanged(d->state);
    }

    if (d->readSocketNotifier)
        d->readSocketNotifier->setEnabled(true);
    return true;
}

/*!
    Waits until the socket is connected, up to \a msecs
    milliseconds. If the connection has been established, this
    function returns true; otherwise it returns false. In the case
    where it returns false, you can call error() to determine
    the cause of the error.

    The following example waits up to one second for a connection
    to be established:

    \code
        socket->connectToHost("imap", 143);
        if (socket->waitForConnected(1000))
            qDebug("Connected!");
    \endcode

    If msecs is -1, this function will not time out.

    \sa connectToHost(), connected()
*/
bool QAbstractSocket::waitForConnected(int msecs)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::waitForConnected(%i)", msecs);
#endif

    if (state() == ConnectedState) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::waitForConnected(%i) already connected", msecs);
#endif
        return true;
    }

    QTime stopWatch;
    stopWatch.start();

    if (state() == HostLookupState) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::waitForConnected(%i) doing host name lookup", msecs);
#endif
        d->startConnecting(QDns::getHostByName(d->hostName));
        if (state() == UnconnectedState)
            return false;
    }

    bool timedOut = true;
#if defined (QABSTRACTSOCKET_DEBUG)
    int attempt = 1;
#endif
    while (state() == ConnectingState && (msecs == -1 || stopWatch.elapsed() < msecs)) {
        int timeout = msecs == -1 ? msecs : qMin(QT_CONNECT_TIMEOUT, msecs - stopWatch.elapsed());
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::waitForConnected(%i) waiting %.2f secs for connection attempt #%i",
               msecs, timeout / 1000.0, attempt++);
#endif
        timedOut = false;
        d->socketLayer.waitForWrite(timeout, &timedOut);
        d->testConnection();
    }

    if (timedOut && state() != ConnectedState) {
        d->socketError = SocketTimeoutError;
        setSocketState(UnconnectedState);
        d->resetSocketLayer();
        setErrorString(tr("Socket operation timed out"));
    }

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::waitForConnected(%i) == %s", msecs,
           state() == ConnectedState ? "true" : "false");
#endif
    return state() == ConnectedState;
}

/*! \reimp
*/
bool QAbstractSocket::waitForReadyRead(int msecs)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::waitForReadyRead(%i)", msecs);
#endif

    // require calling connectToHost() before waitForReadyRead()
    if (state() == UnconnectedState) {
        qWarning("QAbstractSocket::waitForReadyRead() is not allowed in UnconnectedState");
        return false;
    }

    QTime stopWatch;
    stopWatch.start();

    // handle a socket in connecting state
    if (state() == HostLookupState || state() == ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    forever {
        bool readyToRead = false;
        if (!d->socketLayer.waitForReadOrWrite(&readyToRead, true, !d->writeBuffer.isEmpty(),
                                               msecs == -1 ? msecs : msecs - stopWatch.elapsed())) {
            d->socketError = d->socketLayer.error();
            setErrorString(d->socketLayer.errorString());
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::waitForReadyRead(%i) failed (%i, %s)",
                   msecs, d->socketError, errorString().latin1());
#endif
            emit error(d->socketError);
            if (d->socketError != SocketTimeoutError)
                close();
            return false;
        }

        if (readyToRead) {
            if (d->canReadNotification(0))
                return true;
        } else {
            d->canWriteNotification(0);
        }

        if (state() != ConnectedState)
            return false;
    }
}

/*! \reimp
 */
bool QAbstractSocket::waitForBytesWritten(int msecs)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::waitForBytesWritten(%i)", msecs);
#endif

    // require calling connectToHost() before waitForBytesWritten()
    if (state() == UnconnectedState) {
        qWarning("QAbstractSocket::waitForBytesWritten() is not allowed in UnconnectedState");
        return false;
    }

    if (d->writeBuffer.isEmpty())
        return false;

    QTime stopWatch;
    stopWatch.start();

    // handle a socket in connecting state
    if (state() == HostLookupState || state() == ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    forever {
        bool readyToRead = false;
        if (!d->socketLayer.waitForReadOrWrite(&readyToRead, true, !d->writeBuffer.isEmpty(),
                                               msecs == -1 ? msecs : msecs - stopWatch.elapsed())) {
            d->socketError = d->socketLayer.error();
            setErrorString(d->socketLayer.errorString());
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::waitForBytesWritten(%i) failed (%i, %s)",
                   msecs, d->socketError, errorString().latin1());
#endif
            if (d->socketError != SocketTimeoutError)
                emit error(d->socketError);
            close();
            return false;
        }

        if (readyToRead) {
            d->canReadNotification(0);
        } else {
            if (d->canWriteNotification(0))
                return true;
        }

        if (state() != ConnectedState)
            return false;
    }
}

/*!
    Waits until the socket has disconnected, up to \a msecs
    milliseconds. If the connection has been disconnected, this
    function returns true; otherwise it returns false. In the case
    where it returns false, you can call error() to determine
    the cause of the error.

    The following example waits up to one second for a connection
    to be closed:

    \code
        socket->disconnect();
        if (socket->waitForDisconnected(1000))
            qDebug("Disconnected!");
    \endcode

    If msecs is -1, this function will not time out.

    \sa disconnect(), close()
*/
bool QAbstractSocket::waitForDisconnected(int msecs)
{
    // require calling connectToHost() before waitForDisconnected()
    if (state() == UnconnectedState) {
        qWarning("QAbstractSocket::waitForDisconnected() is not allowed in UnconnectedState");
        return false;
    }

    QTime stopWatch;
    stopWatch.start();

    // handle a socket in connecting state
    if (state() == HostLookupState || state() == ConnectingState) {
        if (!waitForConnected(msecs))
            return false;
    }

    forever {
        bool readyToRead = false;
        if (!d->socketLayer.waitForReadOrWrite(&readyToRead, state() == ConnectedState,
                                               !d->writeBuffer.isEmpty(),
                                               msecs == -1 ? msecs : msecs - stopWatch.elapsed())) {
            d->socketError = d->socketLayer.error();
            setErrorString(d->socketLayer.errorString());
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocket::waitForReadyRead(%i) failed (%i, %s)",
                   msecs, d->socketError, errorString().latin1());
#endif
            emit error(d->socketError);
            if (d->socketError != SocketTimeoutError)
                close();
            return false;
        }

        if (readyToRead) {
            d->canReadNotification(0);
        } else {
            d->canWriteNotification(0);
        }

        if (state() == UnconnectedState)
            return true;
    }
}

/*!
    Aborts the current connection and resets the socket. Unlike
    close(), this function immediately closes the socket, clearing
    any pending data in the write buffer.

    \sa close()
*/
void QAbstractSocket::abort()
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::abort()");
#endif
    if (d->state == UnconnectedState)
        return;

    d->writeBuffer.clear();
    close();
}

/*! \reimp
*/
bool QAbstractSocket::isSequential() const
{
    return true;
}

/*! \reimp
*/
Q_LONGLONG QAbstractSocket::readData(char *data, Q_LONGLONG maxSize)
{
    if (!d->isBuffered) {
        Q_LONGLONG readBytes = d->socketLayer.read(data, maxSize);
        if (readBytes < 0) {
            d->socketError = d->socketLayer.error();
            q->setErrorString(d->socketLayer.errorString());
        }
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::readData(%p \"%s\", %lli) == %lld",
               data, qt_prettyDebug(data, 32, readBytes).data(), maxSize,
               readBytes);
#endif
        return readBytes;
    }

    if (d->readBuffer.isEmpty())
        return Q_LONGLONG(0);

    if (d->readSocketNotifier && !d->readSocketNotifier->isEnabled())
        d->readSocketNotifier->setEnabled(true);

    // If readFromSocket() read data, copy it to its destination.
    if (maxSize == 1) {
        *data = d->readBuffer.getChar();
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::readData(%p '%c (0x%.2x)', 1) == 1",
               data, isprint(*data) ? *data : '?', *data);
#endif
        return 1;
    }

    Q_LONGLONG bytesToRead = qMin(Q_LONGLONG(d->readBuffer.size()), maxSize);
    Q_LONGLONG readSoFar = 0;
    while (readSoFar < bytesToRead) {
        char *ptr = d->readBuffer.readPointer();
        int bytesToReadFromThisBlock = qMin(int(bytesToRead - readSoFar),
                                            d->readBuffer.nextDataBlockSize());
        memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
        readSoFar += bytesToReadFromThisBlock;
        d->readBuffer.free(bytesToReadFromThisBlock);
    }

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::readData(%p \"%s\", %lli) == %lld",
           data, qt_prettyDebug(data, qMin(32, readSoFar), readSoFar).data(),
           maxSize, readSoFar);
#endif
    return readSoFar;
}

/*! \reimp
*/
Q_LONGLONG QAbstractSocket::writeData(const char *data, Q_LONGLONG size)
{
    if (!d->isBuffered) {
        Q_LONGLONG written = d->socketLayer.write(data, size);
        if (written < 0) {
            d->socketError = d->socketLayer.error();
            q->setErrorString(d->socketLayer.errorString());
        } else if (d->writeSocketNotifier && !d->writeBuffer.isEmpty()) {
            d->writeSocketNotifier->setEnabled(true);
        }

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::writeData(%p \"%s\", %lli) == %lli", data,
           qt_prettyDebug(data, qMin((int)size, 32), size).data(),
           size, written);
#endif
        if (written >= 0)
            emit bytesWritten(written);
        return written;
    }

    char *ptr = d->writeBuffer.reserve(size);
    if (size == 1)
        *ptr = *data;
    else
        memcpy(ptr, data, size);

    Q_LONGLONG written = size;

    if (d->writeSocketNotifier && !d->writeBuffer.isEmpty())
        d->writeSocketNotifier->setEnabled(true);

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::writeData(%p \"%s\", %lli) == %lli", data,
           qt_prettyDebug(data, qMin((int)size, 32), size).data(),
           size, written);
#endif
    return written;
}

/*!
    Attempts to close the socket. If there is pending data waiting to
    be written, QAbstractSocket will enter ClosingState and wait
    until all data has been written. Eventually, it will enter
    UnconnectedState and emit the disconnected() signal.

    \sa abort()
*/
void QAbstractSocket::close()
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::close()");
#endif
    if (d->state != UnconnectedState) {
        d->closeCalled = true;
        disconnectFromHost();
    }
}

/*
*/
void QAbstractSocket::disconnectFromHost()
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::disconnectFromHost()");
#endif

    if (d->state == UnconnectedState) {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() was called on an unconnected socket");
#endif
        return;
    }

#ifdef QT_COMPAT
    emit connectionClosed(); // compat signal
#endif

    // Disable and delete read notification
    if (d->readSocketNotifier) {
        d->readSocketNotifier->setEnabled(false);
        delete d->readSocketNotifier;
        d->readSocketNotifier = 0;
    }

    // Perhaps emit closing()
    if (d->state != ClosingState) {
        d->state = ClosingState;
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() emits stateChanged()(ClosingState)");
#endif
        emit stateChanged(d->state);
    } else {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() return from delayed close");
#endif
    }

    // Wait for pending data to be written.
    if (d->writeBuffer.size() > 0) {
        if (d->writeSocketNotifier)
            d->writeSocketNotifier->setEnabled(true);

#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() delaying disconnect");
#endif
        return;
    } else {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() disconnecting immediately");
#endif
    }

    // Disable and delete write notification
    if (d->writeSocketNotifier) {
        d->writeSocketNotifier->setEnabled(false);
        delete d->writeSocketNotifier;
        d->writeSocketNotifier = 0;
    }

    d->resetSocketLayer();
    d->state = UnconnectedState;
    emit stateChanged(d->state);

#ifdef QT_COMPAT
    emit delayedCloseFinished(); // compat signal
#endif
    emit disconnected();

#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() disconnected!");
#endif

    if (d->closeCalled) {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocket::disconnectFromHost() closed!");
#endif
        d->readBuffer.clear();
        d->writeBuffer.clear();
        QIODevice::close();
    }
}

/*!
    Returns the size of the internal read buffer. This limits the
    amount of data that the client can receive before you call read()
    or readAll().

    A read buffer size of 0 (the default) means that the buffer has
    no size limit, ensuring that no data is lost.

    \sa setReadBufferSize(), read()
*/
Q_LONGLONG QAbstractSocket::readBufferSize() const
{
    return d->readBufferMaxSize;
}

/*!
    Sets the size of QAbstractSocket's internal read buffer to be \a
    size bytes.

    If the buffer size is limited to a certain size, QAbstractSocket
    won't buffer more than this size of data. Exceptionally, a buffer
    size of 0 means that the read buffer is unlimited and all
    incoming data is buffered. This is the default.

    This option is useful if you only read the data at certain points
    in time (e.g., in a real-time streaming application) or if you
    want to protect your socket against receiving too much data,
    which may eventually cause your application to run out of memory.

    \sa readBufferSize(), read()
*/
void QAbstractSocket::setReadBufferSize(Q_LONGLONG size)
{
    d->readBufferMaxSize = size;
}

/*!
    Returns the state of the socket.

    \sa error()
*/
QAbstractSocket::SocketState QAbstractSocket::state() const
{
    return d->state;
}

/*!
    Sets the state of the socket to \a state.

    \sa state()
*/
void QAbstractSocket::setSocketState(SocketState state)
{
    d->state = state;
}

/*!
    Returns the socket type (TCP, UDP, or other).

    \sa QTcpSocket, QUdpSocket
*/
QAbstractSocket::SocketType QAbstractSocket::socketType() const
{
    return d->socketType;
}

/*!
    Returns the type of error that last occurred.

    \sa state(), errorString()
*/
QAbstractSocket::SocketError QAbstractSocket::error() const
{
    return d->socketError;
}

/*!
    Sets the type of error that last occurred to \a socketError.

    \sa setSocketState(), setErrorString()
*/
void QAbstractSocket::setSocketError(SocketError socketError)
{
    d->socketError = socketError;
}

#ifdef QT_COMPAT
/*! \enum QAbstractSocket::Error
    \compat

    Use QAbstractSocket::SocketError instead.

    \value ErrConnectionRefused Use QAbstractSocket::ConnectionRefusedError instead.
    \value ErrHostNotFound Use QAbstractSocket::HostNotFoundError instead.
    \value ErrSocketRead Use QAbstractSocket::UnknownSocketError instead.
*/

/*! \enum QAbstractSocket::State
    \compat

    Use QAbstractSocket::SocketState instead.

    \value Idle Use QAbstractSocket::UnconnectedState instead.
    \value HostLookup Use QAbstractSocket::HostLookupState instead.
    \value Connecting Use QAbstractSocket::ConnectingState instead.
    \value Connected Use QAbstractSocket::ConnectedState instead.
    \value Closing Use QAbstractSocket::ClosingState instead.
    \value Connection Use QAbstractSocket::ConnectedState instead.
*/

/*!
    \fn int QAbstractSocket::socket() const

    Use socketDescriptor() instead.
*/

/*!
    \fn void QAbstractSocket::setSocket(int socket)

    Use setSocketDescriptor() instead.
*/

/*!
    \fn Q_ULONG QAbstractSocket::waitForMore(int msecs, bool *timeout = 0) const

    Use waitForReadyRead() instead.

    \oldcode
        bool timeout;
        Q_ULONG numBytes = socket->waitForMore(30000, &timeout);
    \newcode
        Q_LONGLONG numBytes = 0;
        if (socket->waitForReadyRead(msecs))
            numBytes = socket->bytesAvailable();
        bool timeout = (error() == QAbstractSocket::SocketTimeoutError);
    \endcode

    \sa waitForReadyRead(), bytesAvailable(), error(), SocketTimeoutError
*/

/*!
    \fn void QAbstractSocket::connectionClosed()

    Use closing() instead.
*/

/*!
    \fn void QAbstractSocket::delayedCloseFinished()

    Use closed() instead.
*/
#endif

#include "moc_qabstractsocket.cpp"
