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

/*! \class QAbstractSocket

    \brief The QAbstractSocket class provides the base functionality
    common to all socket types.

\if defined(commercial)
    It is part of the \l{commercialeditions.html}{Qt Enterprise Edition}.
\endif
    \reentrant
    \ingroup io
    \module network

    QAbstractSocket is the base class for QTcpSocket and QUdpSocket
    and contains all common functionality of these two classes. If
    you need a socket, you have three options:

    \list
    \i  Instantiate QTcpSocket or QUdpSocket.
    \i  Create a native socket descriptor, instantiate
	QAbstractSocket, and call setSocketDescriptor() to wrap the
	native socket.
    \i  Subclass QAbstractSocket and implement your own socket type
	(e.g., for Unix domain sockets or Windows named pipes).
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

    By default, QAbstractSocket is non-blocking (asynchronous). This
    can be changed by calling setBlocking(true). In blocking mode,
    many functions (notably connectToHost(), read(), write(), and
    close()) do not return until their operation has completed; in
    non-blocking mode, all functions except waitForReadyRead() return
    immediately.

    Programming with a blocking socket is radically different from
    programming with a non-blocking socket. A blocking socket doesn't
    require an event loop and typically leads to simpler code.
    However, in a GUI application, blocking sockets should only be
    used in non-GUI threads, to avoid freezing the user interface.
    See the network/fortuneclient and network/blockingfortuneclient
    examples for an overview of both approaches.

    At any time, QAbstractSocket has a state (returned by
    socketState()). The initial state is Qt::UnconnectedState. After
    calling connectToHost(), the socket first enters
    Qt::HostLookupState. If the host is found, QAbstractSocket enters
    Qt::ConnectingState and emits the hostFound() signal. When the
    connection has been established, it enters Qt::ConnectedState and
    emits connected(). If an error occurs at any stage, error() is
    emitted. Whenever the state changes, stateChanged() is emitted.
    For convenience, isValid() returns true if the socket is ready
    for reading and writing.

    Read or write data by calling read() or write(), or use the
    convenience functions readLine() and readAll(). QAbstractSocket
    also inherits getch(), putch(), and ungetch() from QIODevice,
    which work on single bytes. For every chunk of data that has been
    written to the socket, the bytesWritten() signal is emitted.

    The readyRead() signal is emitted every time a new chunk of data
    has arrived. bytesAvailable() then returns the number of bytes
    that are available for reading. Typically, you would connect the
    readyRead() signal to a slot and read all available data there.
    If you don't read all the data at once, the remaining data will
    still be available later, and any new incoming data will be
    appended to QAbstractSocket's internal read buffer. To limit the
    size of the read buffer, call setReadBufferSize().

    To close the socket, call close(). QAbstractSocket enters
    Qt::ClosingState, then emits closing(). After all pending data
    has been written to the socket, QAbstractSocket actually closes
    the socket, enters Qt::ClosedState, and emits closed(). If you
    want to abort a connection immediately, discarding all pending
    data, call abort() instead.

    The port and address of the connected peer is fetched by calling
    peerPort() and peerAddress(). peerName() returns the host name of
    the peer, as passed to connectToHost(). localPort() and
    localAddress() return the port and address of the local socket.

    Since QAbstractSocket inherits QIODevice, it can be used with
    QTextStream and QDataStream's stream operators (operator<<() and
    operator>>()). There is one issue to be aware of, though: In
    non-blocking mode, you must make sure that enough data is
    available before attempting to read it using operator>>().

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

    \sa closed()
*/

/*!
    \fn void QAbstractSocket::closed()

    This signal is emitted when the connection has been closed.

    \sa connectToHost(), close()
*/

/*!
    \fn void QAbstractSocket::readyRead()

    This signal is emitted every time there is new incoming data.

    Bear in mind that new incoming data is only reported once; if you
    do not read any data, this class buffers the data and you can
    read it later, but no signal is emitted until new data arrives. A
    good practice is to read all data in the slot connected to this
    signal unless you are sure that you need to receive more data to
    be able to process it.

    \sa read(), readAll(), readLine(), bytesAvailable()
*/


/*!
    \fn void QAbstractSocket::bytesWritten(Q_LLONG numBytes)

    This signal is emitted when a payload of data has been written to
    the network. The \a numBytes parameter specifies how many bytes
    were written.

    \sa write(), bytesToWrite()
*/

/*!
    \fn void QAbstractSocket::error(int socketError)

    This signal is emitted after an error occurred. The parameter \a
    socketError is the \l Qt::SocketError value.

    \sa socketError(), errorString()
*/

/*!
    \fn void QAbstractSocket::stateChanged(int socketState)

    This signal is emitted whenever QAbstractSocket's state
    changes. The parameter \a socketState is the \l Qt::SocketState
    value.

    \sa socketState()
*/

/*! \internal
    \enum Qt::NetworkLayerProtocol

    This enum describes the network layer protocol values used in Qt.

    \value IPv4Protocol IPv4
    \value IPv6Protocol IPv6
    \value UnknownNetworkLayerProtocol Other than IPv4 and IPv6

    \sa QSocketLayer::protocol()
*/

/*! \enum Qt::SocketType

    This enum describes the transport layer protocol.

    \value TcpSocket TCP
    \value UdpSocket UDP
    \value UnknownSocketType Other than TCP and UDP

    \sa QAbstractSocket::socketType()
*/

/*! \enum Qt::SocketError

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
    \value UnsupportedSocketOperationError The socket operation was
           not supported by the local operating system (e.g., lack of
           IPv6 support).
    \value UnknownSocketError An unidentified error occurred.

    \sa QAbstractSocket::socketError()
*/

/*! \enum Qt::SocketState

    This enum describes the different states in which a socket can be.

    \value UnconnectedState The socket is not connected.
    \value HostLookupState The socket is performing a host name lookup.
    \value ConnectingState The socket has started establishing a connection.
    \value ConnectedState A connection is established.
    \value BoundState The socket is bound to an address and port (for servers).
    \value ClosingState The socket is about to close (data may still
    be waiting to be written).

    \omitvalue ListeningState
    \omit This state is only used by QSocketLayer. \endomit

    \sa QAbstractSocket::socketState()
*/

#include "qabstractsocket.h"
#include "qabstractsocket_p.h"
#include "qdatetime.h"
#include "qhostaddress.h"
#include "qpointer.h"
#include "qsignal.h"
#include "qsocketlayer.h"
#include "qtimer.h"

#include <time.h>

#define QABSTRACTSOCKET_BUFFERSIZE 32768
//#define QABSTRACTSOCKET_BUFFERSIZE 10

//#define QABSTRACTSOCKET_DEBUG

#define d d_func()
#define q q_func()

/*! \internal

    Creates an empty ring buffer. The buffer will grow in steps of \a
    growth as data is written to it.
*/
QRingBuffer::QRingBuffer(int growth)
    : basicBlockSize(growth)
{
    buffers << QByteArray();
    clear();
}

/*! \internal

    Returns the number of bytes that can be read in one operation. The
    data is read from readPointer().
*/
int QRingBuffer::nextDataBlockSize() const
{
    return (tailBuffer == 0 ? tail : buffers.at(0).size()) - head;
}

/*! \internal

    Returns a pointer to where no more than nextDataBlockSize() bytes
    of data can be read. Call free() to remove data after reading.
*/
char *QRingBuffer::readPointer() const
{
    if (buffers.count() == 0)
        return 0;
    return const_cast<char *>(buffers[0].data()) + head;
}

/*! \internal

    Removes \a bytes bytes from the front of the buffer. If \a bytes
    is larger than the size of the buffer, the buffer is cleared.
*/
void QRingBuffer::free(int bytes)
{
    bufferSize -= bytes;
    if (bufferSize < 0)
        bufferSize = 0;

    for (;;) {
        int nextBlockSize = nextDataBlockSize();
        if (bytes < nextBlockSize) {
            head += bytes;
            if (head == tail && tailBuffer == 0)
                head = tail = 0;
            return;
        }

        bytes -= nextBlockSize;
        if (buffers.count() == 1) {
            if (buffers.at(0).size() != basicBlockSize)
                buffers[0].resize(basicBlockSize);
            head = tail = 0;
            tailBuffer = 0;
            return;
        }

        buffers.removeAt(0);
        --tailBuffer;
        head = 0;
    }
}

/*! \internal

    Reserves space in the buffer for \a bytes new bytes, and returns a
    pointer to the first byte.
*/
char *QRingBuffer::reserve(int bytes)
{
    bufferSize += bytes;

    // if there is already enough space, simply return.
    if (tail + bytes <= buffers.at(tailBuffer).size()) {
        char *writePtr = buffers[tailBuffer].data() + tail;
        tail += bytes;
        return writePtr;
    }

    // if our buffer isn't half full yet, simply resize it.
    if (tail < buffers.at(tailBuffer).size() / 2) {
        buffers[tailBuffer].resize(tail + bytes);
        char *writePtr = buffers[tailBuffer].data() + tail;
        tail += bytes;
        return writePtr;
    }

    // shrink this buffer to its current size
    buffers[tailBuffer].resize(tail);

    // create a new QByteArray with the right size
    buffers << QByteArray();
    ++tailBuffer;
    buffers[tailBuffer].resize(qMax(basicBlockSize, bytes));
    tail = bytes;
    return buffers[tailBuffer].data();
}

/*! \internal

    Removes \a bytes bytes from the end of the buffer. If \a bytes is
    larger than the buffer size, the buffer is cleared.
*/
void QRingBuffer::truncate(int bytes)
{
    bufferSize -= bytes;
    if (bufferSize < 0)
        bufferSize = 0;

    for (;;) {
        // special case: head and tail are in the same buffer
        if (tailBuffer == 0) {
            tail -= bytes;
            if (tail <= head)
                tail = head = 0;
            return;
        }

        if (bytes <= tail) {
            tail -= bytes;
            return;
        }

        bytes -= tail;
        buffers.removeAt(tailBuffer);

        --tailBuffer;
        tail = buffers.at(tailBuffer).size();
    }
}

/*! \internal

    Returns and removes the first character in the buffer. Returns -1
    if the buffer is empty.
*/
int QRingBuffer::getChar()
{
    if (isEmpty())
       return -1;
    char c = *readPointer();
    free(1);
    return c;
}

/*! \internal

    Appends the character \a c to the end of the buffer.
*/
void QRingBuffer::putChar(char c)
{
    char *ptr = reserve(1);
    *ptr = c;
}

/*! \internal

    Prepends the character \a c to the front of the buffer.
*/
void QRingBuffer::ungetChar(char c)
{
    --head;
    if (head < 0) {
        buffers.prepend(QByteArray());
        buffers[0].resize(basicBlockSize);
        head = basicBlockSize - 1;
        ++tailBuffer;
    }
    buffers[0][head] = c;
    ++bufferSize;
}

/*! \internal

    Returns the size of the buffer; e.g. the number of bytes
    currently in use.
*/
int QRingBuffer::size() const
{
    return bufferSize;
}

/*! \internal

    Removes all data from the buffer and resets its size to 0.
*/
void QRingBuffer::clear()
{
    QByteArray tmp = buffers[0];
    buffers.clear();
    buffers << tmp;

    if (buffers.at(0).size() != basicBlockSize)
        buffers[0].resize(basicBlockSize);

    head = tail = 0;
    tailBuffer = 0;
    bufferSize = 0;
}

/*! \internal

    Returns true if the buffer is empty; otherwise returns false.
*/
bool QRingBuffer::isEmpty() const
{
    return tailBuffer == 0 && tail == 0;
}

/*! \internal

    Returns the index of the first occurrence of the character \a c in
    the buffer. In no such character is found, -1 is returned.
*/
int QRingBuffer::indexOf(char c) const
{
    int index = 0;
    for (int i = 0; i < buffers.size(); ++i) {
        int start = 0;
        int end = buffers.at(i).size();

        if (i == 0)
            start = head;
        if (i == tailBuffer)
            end = tail;
        const char *ptr = buffers.at(i).data() + start;
        for (int j = start; j < end; ++j) {
            if (*ptr++ == c)
                return index;
            ++index;
        }
    }

    return -1;
}

/*! \internal

    Reads one line of data (all data up to and including the '\n'
    character), no longer than \a maxLength bytes, and stores it in \a
    data. If the line is too long, or if no line could be read, -1 is
    returned.
*/
int QRingBuffer::readLine(char *data, int maxLength)
{
    int index = indexOf('\n');
    if (index == -1)
        return -1;

    if (index > maxLength)
        return -1;

    int readSoFar = 0;
    while (readSoFar < index) {
        int bytesToRead = qMin((index + 1) - readSoFar, nextDataBlockSize());
        memcpy(data + readSoFar, readPointer(), bytesToRead);
        readSoFar += bytesToRead;
        free(bytesToRead);
    }

    return readSoFar;
}

/*! \internal

    Returns true if a line can be read from the buffer; otherwise
    returns false.
*/
bool QRingBuffer::canReadLine() const
{
    return indexOf('\n') != -1;
}

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
    isBlocking = false;
    state = Qt::UnconnectedState;
    readBufferMaxSize = 0;
    socketError = Qt::UnknownSocketError;
    socketErrorString = QT_TRANSLATE_NOOP(QAbstractSocket, "Unknown error");
}

/*! \internal

    Destructs the QAbstractSocket. If the socket layer is open, it
    will be reset.
*/
QAbstractSocketPrivate::~QAbstractSocketPrivate()
{
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

    readBuffer.clear();
    writeBuffer.clear();

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
bool QAbstractSocketPrivate::initSocketLayer(Qt::SocketType type,
                                            Qt::NetworkLayerProtocol protocol)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    QString typeStr;
    if (type == Qt::TcpSocket) typeStr = "TcpSocket";
    else if (type == Qt::UdpSocket) typeStr = "UdpSocket";
    else typeStr = "UnknownSocketType";
    QString protocolStr;
    if (protocol == Qt::IPv4Protocol) protocolStr = "IPv4Protocol";
    else if (protocol == Qt::IPv6Protocol) protocolStr = "IPv6Protocol";
    else protocolStr = "UnknownNetworkLayerProtocol";
#endif

    resetSocketLayer();

    if (!socketLayer.initialize(type, protocol)) {
#if defined (QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::initSocketLayer(%s, %s) failed (%s)",
               typeStr.latin1(), protocolStr.latin1(),
               socketLayer.errorString().latin1());
#endif
        d->socketError = socketLayer.socketError();
	d->socketErrorString = socketLayer.errorString();
        return false;
    }

    setupSocketNotifiers();

#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::initSocketLayer(%s, %s) succeess",
           typeStr.latin1(), protocolStr.latin1());
#endif
    return true;
}

/*! \internal

    Slot connected to the read socket notifier. This slot is called
    when new data is available for reading, or when the socket has
    been closed. Handles recursive calls.
*/
void QAbstractSocketPrivate::canReadNotification(int)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::canReadNotification()");
#endif

    // Prevent recursive calls
    if (readSocketNotifierCalled)
        return;
    readSocketNotifierCalled = true;

    // Prevent notifier from getting fired more times
    readSocketNotifier->setEnabled(false);

    // If buffered, read data from the socket into the read buffer
    if (isBuffered) {
        // Return if there is no space in the buffer
        if (readBufferMaxSize && readBuffer.size() >= readBufferMaxSize) {
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::canReadNotification() buffer is full");
#endif
            readSocketNotifierCalled = false;
            return;
        }

        // If reading from the socket fails after getting a read
        // notification, close the socket.
        int oldBufferSize = d->readBuffer.size();
        if (!d->readFromSocket()) {
            q->close();
            readSocketNotifierCalled = false;
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::canReadNotification() closing socket");
#endif
            return;
        }

        // If the buffer size is unchanged after reading from the
        // socket, close the socket.
        if (oldBufferSize == d->readBuffer.size()) {
            q->close();
            readSocketNotifierCalled = false;
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::canReadNotification() unchanged buffer: closing socket");
#endif
            return;
        }
    }

    // Emit readyRead(). Anything might have happened in whatever is
    // connected to the readyRead() slot, so check that we weren't
    // deleted to avoid a crash.
    QPointer<QAbstractSocket> that = q;
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::canReadNotification() emitting readyRead()");
#endif
    emit q->readyRead();
    if (!that)
        return;

    // If we were closed as a result of the readyRead() signal,
    // return.
    if (state == Qt::UnconnectedState || state == Qt::ClosingState)
        return;

    // If there is still space in the buffer, reenable the read socket
    // notifier.
    if (!readBufferMaxSize || d->readBuffer.size() < d->readBufferMaxSize) {
        if (d->readSocketNotifier)
            d->readSocketNotifier->setEnabled(true);
    }

    readSocketNotifierCalled = false;
}

/*! \internal

    Slot connected to the write socket notifier. It's called during a
    delayed connect or when the socket is ready for writing.
*/
void QAbstractSocketPrivate::canWriteNotification(int)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::canWriteNotification()");
#endif
    // Prevent the write socket notifier from being called more times
    writeSocketNotifier->setEnabled(false);

    // If in connecting state, check if the connection has been
    // established, otherwise flush pending data.
    if (state == Qt::ConnectingState)
        testConnection();
    else
        flush();
}

/*! \internal

    Writes pending data in the write buffers to the socket. When in
    blocking mode, this function blocks until the buffers are empty;
    otherwise the function writes as much as it can without blocking.

    It is usually invoked by canWriteNotification after one or more
    calls to write().

    Emits bytesWritten().
*/
void QAbstractSocketPrivate::flush()
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::flush(), pending: %i bytes",
           writeBuffer.size());
#endif
    if (!socketLayer.isValid() || writeBuffer.isEmpty())
        return;

    bool timedOut = false;
    int timeout = blockingTimeout;
    QTime stopWatch;
    if (isBlocking)
        stopWatch.start();
    do {
        int nextSize = writeBuffer.nextDataBlockSize();
        char *ptr = writeBuffer.readPointer();

        // Attempt to write it all in one chunk.
        Q_LLONG written = socketLayer.write(ptr, nextSize);
        if (written < 0) {
            socketError = socketLayer.socketError();
            socketErrorString = socketLayer.errorString();
            emit q->error(socketError);
            resetSocketLayer();
            break;
        }

        // Remove what we wrote so far.
        writeBuffer.free(written);
        emit q->bytesWritten(written);
    } while (isBlocking && !writeBuffer.isEmpty()
             && socketLayer.waitForWrite(timeout - stopWatch.elapsed(), &timedOut));

    if (timedOut) {
        socketError = socketLayer.socketError();
        socketErrorString = socketLayer.errorString();
        return;
    }

    if (!writeBuffer.isEmpty()) {
        if (d->writeSocketNotifier)
            d->writeSocketNotifier->setEnabled(true);
    } else if (state == Qt::ClosingState) {
        q->close();
    }
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
    QString s;
    for (int i = 0; i < addresses.count(); ++i) {
        if (i != 0) s += ", ";
        s += addresses.at(i).toString();
    }
    if (addresses.count() > 1)
        s = "{" + s + "}";
    qDebug("QAbstractSocketPrivate::startConnecting(%s)", s.latin1());
#endif

    // If there are no addresses in the host list, report this to the
    // user.
    if (addresses.isEmpty()) {
        state = Qt::UnconnectedState;
        d->socketError = Qt::HostNotFoundError;
        d->socketErrorString = QT_TRANSLATE_NOOP(QAbstractSocket, "Host not found");
        emit q->stateChanged(state);
        emit q->error(Qt::HostNotFoundError);
        return;
    }

    // Enter Connecting state (see also sn_write, which is called by
    // the write socket notifier after connect())
    state = Qt::ConnectingState;
    emit q->stateChanged(state);

    // Report the successful host lookup
    emit q->hostFound();

    // The addresses returned by the lookup will be tested one after
    // another by the connectToNextAddress() slot.
    qInvokeMetaMember(q, "connectToNextAddress",
                      isBlocking ? Qt::DirectConnection
                      : Qt::QueuedConnection);
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
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::connectToNextAddress()");
#endif

    do {
        // Check for more pending addresses
        if (addresses.isEmpty()) {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::connectToNextAddress(), all addresses failed.");
#endif
            state = Qt::UnconnectedState;
            socketError = Qt::ConnectionRefusedError;
            socketErrorString = QT_TRANSLATE_NOOP(QAbstractSocket,
                                                  "Connection refused");
            emit q->error(Qt::ConnectionRefusedError);
            return;
        }

        // Pick the first host address candidate
        host = addresses.takeFirst();
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::connectToNextAddress(), connecting to %s:%i",
               host.toString().latin1(), port);
#endif

        // Determine its protocol.
        Qt::NetworkLayerProtocol protocol;
        if (host.isIPv4Address()) {
            protocol = Qt::IPv4Protocol;
        } else {
#if defined(Q_NO_IPv6)
            // If we have no IPv6 support, then we will not be able to
            // connect. So we just pretend we didn't see this address.
            continue;
#endif
            protocol = Qt::IPv6Protocol;
        }

        // Perhaps reinitialize the socket layer if its protocol
        // doesn't match the address.
        if (!socketLayer.isValid() || socketLayer.protocol() != protocol)
            initSocketLayer(q->socketType(), protocol);

        // Tries to connect to the address. If it succeeds immediately
        // (localhost address on BSD or any UDP connect), emit
        // connected() and return.
        if (socketLayer.connectToHost(host, port)) {
            state = Qt::ConnectedState;
            q->setFlags(q->flags() | QIODevice::Open | QIODevice::ReadWrite);
            emit q->stateChanged(state);
            if (d->readSocketNotifier)
                readSocketNotifier->setEnabled(true);
            emit q->connected();
            return;
        }

        // Check that we're in delayed connection state. If not, an
        // error has occurred.
        if (socketLayer.socketState() != Qt::ConnectingState) {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::connectToNextAddress(), connection failed (%s)",
                   socketLayer.errorString().latin1());
#endif
            socketError = socketLayer.socketError();
            socketErrorString = socketLayer.errorString();
            emit q->error(socketError);
            return;
        }

        // Wait for a write notification.
        // ### proper timeout handling.
        if (!isBlocking) {
            // this will eventually call testConnection()
            if (d->writeSocketNotifier)
                d->writeSocketNotifier->setEnabled(true);
            return;
        }

        // If blocking, wait until the connection has been
        // established. ### proper timeout handling.
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::connectToNextAddress(), waiting for connection...");
#endif
        bool timedOut = false;
        if (!socketLayer.waitForWrite(d->blockingTimeout, &timedOut) && !timedOut) {
            state = Qt::UnconnectedState;
            socketError = socketLayer.socketError();
            socketErrorString = socketLayer.errorString();
            emit q->error(socketError);
            return;
        }

        if (timedOut) {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::connectToNextAddress(), connection timed out");
#endif
            state = Qt::UnconnectedState;
            socketError = Qt::SocketTimeoutError;
            socketErrorString = QT_TRANSLATE_NOOP(QAbstractSocket,
                                                  "Network operation timed out");
            emit q->error(socketError);
            return;
        }

        // Check if the connection has been established.
        testConnection();
    } while (state != Qt::ConnectedState && isBlocking);
}

/*! \internal

    Tests if a connection has been established. If it has, connected()
    is emitted. Otherwise, connectToNextAddress() is invoked.
*/
void QAbstractSocketPrivate::testConnection()
{
    if (socketLayer.socketState() == Qt::ConnectedState || socketLayer.connectToHost(host, port)) {
        state = Qt::ConnectedState;
        q->setFlags(q->flags() | QIODevice::Open | QIODevice::ReadWrite);
        emit q->stateChanged(state);

        if (d->readSocketNotifier)
            readSocketNotifier->setEnabled(true);

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
    // Invoke connectToNextAddress(). If we're in blocking mode, we'll
    // reenter this function on return.
    if (!isBlocking)
        qInvokeMetaMember(q, "connectToNextAddress");
}

/*! \internal

    Reads data from the socket layer into the read buffer. Returns
    true on success; otherwise false.
*/
bool QAbstractSocketPrivate::readFromSocket()
{
    // Find how many bytes we can read from the socket layer.
    Q_LLONG bytesToRead;
    if (readBufferMaxSize) {
        if (readBuffer.size() >= readBufferMaxSize) {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::readFromSocket() buffer full, can't read");
#endif
            return true;
        }

        bytesToRead = qMin(socketLayer.bytesAvailable(),
                           readBufferMaxSize - readBuffer.size());
    } else {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::readFromSocket() socketLayer.bytesAvailable() == %lli",
               socketLayer.bytesAvailable());
#endif

        bytesToRead = socketLayer.bytesAvailable();
    }

    // Read from the socket, store data in the read buffer.
    if (bytesToRead > 0) {
        char *ptr = d->readBuffer.reserve(bytesToRead);
        Q_LLONG readBytes = socketLayer.read(ptr, bytesToRead);
        d->readBuffer.truncate((int) (bytesToRead - readBytes));

        if (!socketLayer.isValid()) {
            socketError = socketLayer.socketError();
            socketErrorString = socketLayer.errorString();
            emit q->error(socketError);
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::readFromSocket() read failed: %s",
                   socketErrorString.latin1());
#endif
            d->resetSocketLayer();
            return false;
        }

        // If there is still space in the buffer, reenabled the read
        // socket notifier.
        if (!readBufferMaxSize || readBuffer.size() < readBufferMaxSize) {
            if (d->readSocketNotifier)
                readSocketNotifier->setEnabled(true);
        }
    } else {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::readFromSocket() nothing to read!");
#endif
    }

    return true;
}

/*! \internal

    Constructs a new abstract socket of type \a socketType. The \a
    parent argument is passed to QObject's constructor.
*/
QAbstractSocket::QAbstractSocket(Qt::SocketType socketType,
                                 QAbstractSocketPrivate &p, QObject *parent)
    : QObject(parent), QIODevice(p)
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::QAbstractSocket(%p, %p)", &p, parent);
#endif
    // The d_ptr member variable is necessary because we have two base
    // classes with a variable called d_ptr.
    d_ptr = static_cast<QAbstractSocketPrivate *>(QIODevice::d_ptr);
    d->socketType = socketType;
}

/*!
    Creates a new abstract socket of type \a socketType. The \a
    parent argument is passed to QObject's constructor.

    \sa socketType(), QTcpSocket, QUdpSocket
*/
QAbstractSocket::QAbstractSocket(Qt::SocketType socketType, QObject *parent)
    : QObject(parent), QIODevice(*new QAbstractSocketPrivate())
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::QAbstractSocket(%p)", parent);
#endif
    d_ptr = static_cast<QAbstractSocketPrivate *>(QIODevice::d_ptr);
    d->socketType = socketType;
}

/*!
    Destroys the socket.
*/
QAbstractSocket::~QAbstractSocket()
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::~QAbstractSocket()");
#endif
    abort();
}

/*!
    Returns true if the socket is valid and ready for use; otherwise
    returns false.

    \sa socketState()
*/
bool QAbstractSocket::isValid() const
{
    return d->socketLayer.isValid();
}

/*!
    Attempts to make a connection to \a hostName on port \a port.
    Returns true if the connection has been established; otherwise
    returns false.

    QAbstractSocket first enters Qt::HostLookupState, then performs a
    host name lookup of \a hostName. If the lookup suceeds,
    hostFound() is emitted and QAbstractSocket enters
    Qt::ConnectingState. It then attempts to connect to the address
    or addresses returned by the lookup. Finally, if a connection is
    established, QAbstractSocket enters Qt::ConnectedState and
    emits connected().

    At any point, the socket can emit error() to signal that an error
    occurred.

    \a hostName may be an IP address in string form (e.g.,
    "43.195.83.32"), or it may be a host name (e.g.,
    "www.trolltech.com"). QAbstractSocket will do a lookup only if
    required. \a port is in native byte order.

    \sa socketState(), peerName(), peerAddress(), peerPort()
*/
bool QAbstractSocket::connectToHost(const QString &hostName, Q_UINT16 port)
{
    if (d->state == Qt::ConnectingState || d->state == Qt::ConnectedState)
        close();

    d->hostName = hostName;
    d->port = port;
    d->state = Qt::HostLookupState;
    emit stateChanged(d->state);

    if (d->isBlocking)
        d->startConnecting(QDns::getHostByName(hostName));
    else
        QDns::getHostByName(hostName, this, SLOT(startConnecting(const QDnsHostInfo &)));


#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::connectToHost(\"%s\", %i) == %s", hostName.latin1(), port,
           d->state == Qt::ConnectedState ? "true" : "false");
#endif
    return (d->state == Qt::ConnectedState);
}

/*! \overload

    Attempts to make a connection to \a address on port \a port.
*/
bool QAbstractSocket::connectToHost(const QHostAddress &address, Q_UINT16 port)
{
    return connectToHost(address.toString(), port);
}

/*!
    If \a blocking is true, sets the socket to blocking mode and the
    timeout to \a msec milliseconds (the default timeout is 30
    seconds); otherwise, sets the socket to non-blocking mode (\a
    msec is ignored).

    In blocking mode, connectToHost(), read(), write(), readLine(),
    getch(), putch(), and close() suspend the program's execution
    until their task has been completed or until the timeout has
    expired. For example:

    \code
        QTcpSocket socket;
        socket.setBlocking(true);
        if (socket.connectToHost("imap", 143)) {
            qDebug("Connected!");
        } else {
            qDebug("Unable to connect: %s", socket.errorString().latin1());
        }
    \endcode

    Blocking mode is the only option in applications that do not have
    an event loop.

    \sa isBlocking(), waitForReadyRead()
*/
void QAbstractSocket::setBlocking(bool blocking, int msec)
{
    d->isBlocking = blocking;
    d->blockingTimeout = msec;

    // Set Raw or Async depending on the value of blocking.
    setMode(blocking ? (mode() & ~Async) : (mode() | Async));
}

/*!
    Returns true if the socket is blocking; otherwise returns false.

    \sa setBlocking()
*/
bool QAbstractSocket::isBlocking() const
{
    return d->isBlocking;
}

/*!
    Returns the number of bytes that are waiting to be written. The
    bytes are written when control goes back to the event loop or
    when flush() is called.

    \sa bytesAvailable(), flush()
*/
Q_LLONG QAbstractSocket::bytesToWrite() const
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::bytesToWrite() == %i", d->writeBuffer.size());
#endif
    return (Q_LLONG)d->writeBuffer.size();
}

/*!
    Returns the number of incoming bytes that are waiting to be read.

    \sa bytesToWrite(), read()
*/
Q_LLONG QAbstractSocket::bytesAvailable() const
{
    Q_LLONG available = 0;
    if (d->isBuffered)
        available = (Q_LLONG) d->readBuffer.size();
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
    Qt::ConnectedState; otherwise returns 0.

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
    Qt::ConnectedState; otherwise returns QHostAddress::Null.

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
    Reads a '\\n'-terminated line of ASCII data from the socket and
    returns it as a QByteArray (including the '\\n'). If no line can be
    read, it returns an empty QByteArray.

    If the socket is in blocking mode, this function will block until
    a whole line can be read or until the timeout expires.

    \sa canReadLine(), setBlocking()
*/
QByteArray QAbstractSocket::readLine()
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::readLine(), blocking? %s", d->isBlocking ? "true" : "false");
#endif
    if (!d->socketLayer.isValid())
        return QByteArray();

    if (d->isBlocking) {
        int timeout = d->blockingTimeout;
        while (!canReadLine()) {
            QTime stopWatch;
            stopWatch.start();
            int oldSize = d->readBuffer.size();
            if (!waitForReadyRead(timeout) || !d->readFromSocket()
                    || oldSize == d->readBuffer.size()) {
                d->socketError = d->socketLayer.socketError();
                d->socketErrorString = d->socketLayer.errorString();
                return QByteArray();
            }

            timeout -= stopWatch.elapsed();
        }
    }

    int endOfLine = d->readBuffer.indexOf('\n');
    QByteArray tmp;
    if (endOfLine == -1)
        return tmp;

    tmp.resize(endOfLine + 1);
    d->readBuffer.readLine(tmp.data(), endOfLine + 1);
    return tmp;
}

/*!
    Returns the native socket descriptor of QAbstractSocket if this is
    available; otherwise returns -1.

    The socket descriptor is not available when QAbstractSocket is in
    Qt::UnconnectedState.

    \sa setSocketDescriptor()
*/
int QAbstractSocket::socketDescriptor() const
{
    return d->socketLayer.socketDescriptor();
}

/*!
    Initializes QAbstractSocket with the native socket descriptor \a
    socketDescriptor. Returns true on success (e.g. \a
    socketDescriptor is accepted as a valid socket descriptor);
    otherwise returns false. QAbstractSocket enters the socket state
    specified by \a socketState.

    \sa socketDescriptor()
*/
bool QAbstractSocket::setSocketDescriptor(int socketDescriptor, Qt::SocketState socketState)
{
    bool result = d->socketLayer.initialize(socketDescriptor, socketState);
    if (!result) {
        d->socketError = d->socketLayer.socketError();
        d->socketErrorString = d->socketLayer.errorString();
        return false;
    }

    d->setupSocketNotifiers();

    setFlags(flags() | Open | ReadWrite);

    if (d->state != socketState) {
        d->state = socketState;
        emit stateChanged(d->state);
    }

    if (d->readSocketNotifier)
        d->readSocketNotifier->setEnabled(true);
    return true;
}

/*!
    Waits until there is data available for reading, or up to \a
    msecs milliseconds. Returns true if there is data available;
    otherwise returns false, in which case error() should be called
    to determine the cause of the error.

    This function is blocking regardless of whether setBlocking() was
    called or not, but is most commonly used in blocking mode to
    achieve a semi-non-blocking behavior.

    The following example waits up to one second for a line of data
    to arrive:

    \code
        if (socket->waitForReadyRead(1000) && socket->canReadLine())
            qDebug("Read a line: %s", socket->readLine().ascii());
    \endcode

    \sa setBlocking()
*/
bool QAbstractSocket::waitForReadyRead(int msecs)
{
    if (socketState() != Qt::ConnectedState && socketState() != Qt::BoundState) {
        qWarning("%s", tr("QAbstractSocket::waitForReadyRead() is only"
                          " allowed in connected state.").latin1());
        return false;
    }

    bool timedOut = false;
    if (!d->socketLayer.waitForRead(msecs, &timedOut) || (d->isBuffered && !d->readFromSocket())) {
        d->socketError = d->socketLayer.socketError();
        d->socketErrorString = d->socketLayer.errorString();
        emit error(d->socketError);
        return false;
    }

    emit readyRead();
    return true;
}

/*!
    Aborts the current connection and resets the socket. Unlike
    close(), this function immediately closes the socket, losing any
    pending data in the write buffer.

    \sa close()
*/
void QAbstractSocket::abort()
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::abort()");
#endif
    d->writeBuffer.clear();
    close();
}

/*! \reimp
*/
bool QAbstractSocket::open(int)
{
    qWarning("QAbstractSocket::open(int): Sockets must be connected");
    return false;
}

/*!
    Same as bytesAvailable().
*/
Q_LLONG QAbstractSocket::size() const
{
    return bytesAvailable();
}

/*!
    Always returns 0.
*/
Q_LLONG QAbstractSocket::at() const
{
    return 0;
}

/*!
    Writes any pending outgoing data to the socket.

    If the socket is in blocking mode, this function blocks until all
    data has been written or until the timeout has expired;
    otherwise, the function writes as much as it can without
    blocking.

    \sa setBlocking()
*/
void QAbstractSocket::flush()
{
    d->flush();
}

/*! \reimp
*/
Q_LLONG QAbstractSocket::readLine(char *data, Q_LLONG maxLength)
{
    int endOfLine = d->readBuffer.indexOf('\n');
    if (endOfLine == -1)
        return -1;

     if (endOfLine + 1 > maxLength)
         endOfLine = maxLength - 1;

    d->readBuffer.readLine(data, endOfLine + 1);
    if (d->readSocketNotifier)
        d->readSocketNotifier->setEnabled(true);
    return endOfLine + 1;
}

/*! \reimp
*/
QByteArray QAbstractSocket::readAll()
{
    if (d->readSocketNotifier)
        d->readSocketNotifier->setEnabled(true);
    QByteArray tmp;
    tmp.resize(d->readBuffer.size());
    int readSoFar = 0;
    while (!d->readBuffer.isEmpty()) {
        int nextSize = d->readBuffer.nextDataBlockSize();
        memcpy(tmp.data() + readSoFar, d->readBuffer.readPointer(), nextSize);
        readSoFar += nextSize;
        d->readBuffer.free(nextSize);
    }
    return tmp;
}

/*! \reimp
*/
bool QAbstractSocket::seek(Q_LLONG off)
{
    return read(0, off) != -1;
}

/*! \reimp
*/
int QAbstractSocket::getch()
{
    return d->readBuffer.getChar();
}

/*! \reimp
*/
int QAbstractSocket::ungetch(int character)
{
    d->readBuffer.ungetChar((char) character);
    return character;
}

/*! \reimp
*/
int QAbstractSocket::putch(int character)
{
    d->writeBuffer.putChar(character);
    return character;
}

/*!
    Reads at most \a maxLength bytes from the socket into \a data.
    Returns the number of bytes read, or -1 if an error occurred.

    If the socket is in blocking mode and no data is available (e.g.
    bytesAvailable() returns 0), this function will block until data
    is available or until the timeout expires.

    \sa readLine(), readAll(), getch(), write()
*/
Q_LLONG QAbstractSocket::read(char *data, Q_LLONG maxLength)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::read(%p, %lli), readBuffer.size() == %lli",
           data, maxLength, d->readBuffer.size());
#endif
    if (!isValid()) {
        qWarning("QAbstractSocket::read: Invalid socket");
        return -1;
    }

    if (!d->isBuffered) {
        Q_LLONG readBytes = d->socketLayer.read(data, maxLength);
        if (d->readSocketNotifier)
            d->readSocketNotifier->setEnabled(true);
        return readBytes;
    }

    // If readFromSocket() read data, copy it to its destination.
    if (d->readBuffer.size() > 0) {
        if (d->readSocketNotifier)
            d->readSocketNotifier->setEnabled(true);
        int bytesToRead = qMin(d->readBuffer.size(), (int) maxLength);
        int readSoFar = 0;
        while (readSoFar < bytesToRead) {
            char *ptr = d->readBuffer.readPointer();
            int bytesToReadFromThisBlock = qMin(bytesToRead - readSoFar, d->readBuffer.nextDataBlockSize());
            memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
            readSoFar += bytesToReadFromThisBlock;
            d->readBuffer.free(bytesToReadFromThisBlock);
        }

        return readSoFar;
    }

    // Nonblocking sockets are done after the first read.
    if (!d->isBlocking)
        return 0;

    // In blocking mode, wait for more data to read.
    if (!waitForReadyRead(d->blockingTimeout))
        return -1;

    // Read data into the read buffer.
    if (!d->readFromSocket())
        return -1;


    if (d->readSocketNotifier)
        d->readSocketNotifier->setEnabled(true);

    int bytesToRead = qMin(d->readBuffer.size(), (int) maxLength);
    int readSoFar = 0;
    while (readSoFar < bytesToRead) {
        char *ptr = d->readBuffer.readPointer();
        int bytesToReadFromThisBlock = qMin(bytesToRead - readSoFar, d->readBuffer.nextDataBlockSize());
        memcpy(data + readSoFar, ptr, bytesToReadFromThisBlock);
        readSoFar += bytesToReadFromThisBlock;
        d->readBuffer.free(bytesToReadFromThisBlock);
    }

    return readSoFar;
}

/*!
    Writes \a length bytes to the socket from \a data. Returns the
    number of bytes written, or -1 if an error occurred.

    This function always returns immediately, even if QAbstractSocket
    is in blocking mode. To force the data to be written, call
    flush().

    \sa putch()
*/
Q_LLONG QAbstractSocket::write(const char *data, Q_LLONG length)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::write(%p, %lli)", data, length);
#endif
    if (!isValid()) {
        qWarning("QAbstractSocket::write: Invalid socket");
        return -1;
    }

    if (!d->isBuffered) {
        Q_LLONG written = d->socketLayer.write(data, length);
        emit bytesWritten(written);
        return written;
    }

    char *ptr = d->writeBuffer.reserve(length);
    if (length == 1)
        *ptr = *data;
    else
        memcpy(ptr, data, length);

    Q_LLONG written = length;
    if (d->isBlocking) {
        flush();
        written -= d->writeBuffer.size();
    }

    if (d->writeSocketNotifier)
        d->writeSocketNotifier->setEnabled(true);
    return written;
}

/*!
    Attempts to close the socket. If there is pending data waiting to
    be written, QAbstractSocket will enter Qt::ClosingState and wait
    until all data has been written. Eventually, it will enter
    Qt::ClosedState.

    When the socket has been closed, the closed() signal is emitted.

    \sa abort()
*/
void QAbstractSocket::close()
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::close() %p", this);
#endif

    if (d->state == Qt::UnconnectedState) {
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::close() on an unconnected socket");
#endif
        return;
    }

    // Disable and delete read notification
    if (d->readSocketNotifier) {
        if (d->readSocketNotifier)
            d->readSocketNotifier->setEnabled(false);
        delete d->readSocketNotifier;
        d->readSocketNotifier = 0;
    }

    // Perhaps emit closing()
    if (d->state != Qt::ClosingState) {
        d->state = Qt::ClosingState;
        emit stateChanged(d->state);
        emit closing();
    } else {
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::close() return from delayed close");
#endif
    }

    // Wait for pending data to be written. In blocking mode the write
    // buffer list will always be empty.
    if (d->writeBuffer.size() > 0) {
        if (d->isBlocking) {
            bool timedOut = false;
            while (d->socketLayer.waitForWrite(d->blockingTimeout, &timedOut) && !timedOut)
                flush();
        } else {
            if (d->writeSocketNotifier)
                d->writeSocketNotifier->setEnabled(true);

#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::close() delaying close");
#endif
            return;
        }
    } else {
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::close() closing immediately");
#endif
    }

    // Disable and delete write notification
    if (d->writeSocketNotifier) {
        if (d->writeSocketNotifier)
            d->writeSocketNotifier->setEnabled(false);
        delete d->writeSocketNotifier;
        d->writeSocketNotifier = 0;
    }

    d->resetSocketLayer();
    d->state = Qt::UnconnectedState;
    setFlags(flags() & ~(Open | ReadWrite));

    emit stateChanged(d->state);
    emit closed();
}

/*!
    Returns the size of the internal read buffer. This limits the
    amount of data that can be sent to the client before read() or
    readAll() is called.

    A read buffer size of 0 (the default) means that the buffer has
    no size limit.

    \sa setReadBufferSize(), read()
*/
Q_LLONG QAbstractSocket::readBufferSize() const
{
    return d->readBufferMaxSize;
}

/*!
    Sets the size of QAbstractSocket's internal read buffer to be \a
    bufSize bytes.

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
void QAbstractSocket::setReadBufferSize(Q_LLONG bufSize)
{
    d->readBufferMaxSize = bufSize;
}

/*!
    Returns the state of the socket.

    \sa socketError()
*/
Qt::SocketState QAbstractSocket::socketState() const
{
    return d->state;
}

/*!
    Returns the socket type (TCP, UDP, or other).

    \sa QTcpSocket, QUdpSocket
*/
Qt::SocketType QAbstractSocket::socketType() const
{
    return d->socketType;
}

/*!
    Returns the type of error that last occurred.

    \sa socketState(), errorString()
*/
Qt::SocketError QAbstractSocket::socketError() const
{
    return d->socketError;
}

/*!
    Returns the type of error that last occurred as a human-readable
    string.

    \sa socketError()
*/
QString QAbstractSocket::errorString() const
{
    return d->socketErrorString;
}

#include "moc_qabstractsocket.cpp"
