/*! \class QAbstractSocket

    \brief The QAbstractSocket class provides the basic functionality
    for socket classes.

\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif
    \reentrant
    \ingroup io
    \module network

    There are two common ways of using QAbstractSocket. The first is
    as a base pointer for hiding higher level details on the type of
    socket that is currently in use. For example, an application may
    choose to use either TCP or UDP to perform certain network
    operations. By making use of a QAbstractSocket pointer, the code
    which handles the specific operations can be left unchanged,
    regardless of whether the pointer points to a QTcpSocket or a
    QUdpSocket.

    The other common method is to use it as a base class for creating
    new socket classes.

    QAbstractSocket can operate in blocking or non-blocking mode, and
    it's either buffered or unbuffered. Since it inherits QIODevice,
    it can be used with QTextStream for reading or writing text. The
    QIODevice behavior works best when the socket is in blocking mode.

    Call connectToHost() to connect the socket to a host. isValid()
    and socketState() can be called to check the state of the socket.


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

//#define QABSTRACTSOCKET_DEBUG

#define d d_func()
#define q q_func()

/*! \internal

    Constructs a sliding window buffer of unlimited size.
*/
QSlidingWindowBuffer::QSlidingWindowBuffer()
{
    maximumBufferSize = 0;
    clear();
}

/*! \internal

    Returns the maximum size of the buffer, or 0 if the buffer has no
    size limit.
*/
Q_LLONG QSlidingWindowBuffer::maximumSize() const
{
    return maximumBufferSize;
}

/*! \internal

    Sets the maximum size of the buffer to \a size. The buffer's
    current contents are left unchanged, even if the buffer is already
    larger than \a size.
*/
void QSlidingWindowBuffer::setMaximumSize(Q_LLONG size)
{
    maximumBufferSize = size;
}

/*! \internal
*/
QByteArray QSlidingWindowBuffer::readAll()
{
    QByteArray tmp;
    tmp.resize(size());
    readFromFront(tmp.data(), size());
    return tmp;
}

/*! \internal
*/
bool QSlidingWindowBuffer::contains(char character) const
{
    if (headBuffer->indexOf(character, head) != -1)
        return true;
    return tailBuffer->contains(character);
}

/*! \internal
*/
int QSlidingWindowBuffer::indexOf(char character) const
{
    int i = headBuffer->indexOf(character, head);
    if (i != -1)
        return i - head;
    i = tailBuffer->indexOf(character);
    if (i != -1)
        return (headBuffer->size() - head) + i;
    return -1;
}

/*! \internal
*/
QByteArray QSlidingWindowBuffer::left(int len)
{
    QByteArray tmp;
    tmp.resize(len > size() ? size() : len);
    readFromFront(tmp.data(), tmp.size());
    return tmp;
}

/*! \internal

    Clears the buffer.
*/
void QSlidingWindowBuffer::clear()
{
    head = 0;
    tail = 0;
    headBuffer = &buffers[0];
    tailBuffer = &buffers[1];
    buffers[0].clear();
    buffers[1].clear();
}

/*! \internal

    Swaps the head and tail buffer. This is done before reading, if
    the head buffer is empty.
*/
void QSlidingWindowBuffer::swapBuffers()
{
    headBuffer->clear();
    QByteArray *tmp = headBuffer;
    headBuffer = tailBuffer;
    tailBuffer = tmp;
    tail = 0;
    head = 0;
}

/*! \internal

    Reads and removes at most \a maxLength bytes from the from of the
    buffer, and stores it in \a data. Returns the number of bytes
    read.
*/
Q_LLONG QSlidingWindowBuffer::readFromFront(char *data, Q_LLONG maxLength)
{
    // Truncate requested data to size of sliding window buffer.
    if (maxLength > size())
        maxLength = size();
    if (maxLength == 0)
        return 0;

    // Swap the buffers if data is to be read and there is nothing
    // available
    if (headBuffer->size() - head == 0)
        swapBuffers();

    int readSoFar = 0;
    int headBufferSize = headBuffer->size();

    // Check if we need to read more than what's in the head buffer
    if (maxLength > headBufferSize) {
        if (data) {
            if (headBufferSize == 1)
                *data = *headBuffer->data();
            else
                memcpy(data, headBuffer->data(), headBufferSize);
        }
        readSoFar = headBufferSize;
        maxLength -= headBufferSize;
        headBuffer->clear();
        head = 0;
        swapBuffers();
    }

    // Check if all data is in the current buffer.
    if (data) {
        if (maxLength == 1)
            data[readSoFar] = headBuffer->data()[head];
        else
            memcpy(data + readSoFar, headBuffer->data() + head, maxLength);
    }
    head += maxLength;
    return readSoFar + maxLength;
}

/*!
    Writes at most \a length bytes from \a data onto the end of the
    buffer. Returns the number of bytes written.
*/
Q_LLONG QSlidingWindowBuffer::writeToEnd(const char *data, Q_LLONG length)
{
    // Truncate length to fit the buffer.
    if (maximumBufferSize && length > maximumBufferSize - size())
        length = maximumBufferSize - size();
    if (length == 0)
        return 0;

    // Make sure the tail buffer is large enough
    if (tailBuffer->size() - tail < length)
        tailBuffer->resize(tail + length);

    if (length == 1)
        tailBuffer->data()[tail] = *data;
    else
        memcpy(tailBuffer->data() + tail, data, length);
    tail += length;
    return length;
}

/*!
    Returns the size of the buffer.
*/
Q_LLONG QSlidingWindowBuffer::size() const
{
    return (headBuffer->size() - head) + tailBuffer->size();
}

/*! \internal

    Constructs a QAbstractSocketPrivate. Initializes all members.
*/
QAbstractSocketPrivate::QAbstractSocketPrivate()
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
    if (!readBufferMaxSize || d->readBuffer.size() < d->readBufferMaxSize)
        if (d->readSocketNotifier)
            d->readSocketNotifier->setEnabled(true);

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
    stopWatch.start();
    do {
        // Attempt to write it all in one chunk.
        Q_LLONG written = socketLayer.write(writeBuffer.data(), writeBuffer.size());
        if (written < 0) {
            socketError = socketLayer.socketError();
            socketErrorString = socketLayer.errorString();
            emit q->error(socketError);
            resetSocketLayer();
            break;
        } else {
            // Remove what we wrote so far.
            writeBuffer.remove(0, written);
            emit q->bytesWritten(written);
        }
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
            socketError = socketLayer.socketError();
            socketErrorString = socketLayer.errorString();
            emit q->error(socketError);
            return;
        }

        if (timedOut) {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::connectToNextAddress(), connection timed out");
#endif
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
        QByteArray tmpBuffer;
        tmpBuffer.resize(bytesToRead);
        Q_LLONG readBytes = socketLayer.read(tmpBuffer.data(), bytesToRead);
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

        readBuffer.writeToEnd(tmpBuffer.data(), readBytes);

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
    Creates a new abstract socket of type \a socketType. The \a parent
    argument is passed to QObject's constructor.
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
    Destroys the abstract socket.
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
    Attempts to make a connection to \a hostName on the specified \a
    port. Returns true if the connection has been established;
    otherwise returns false.

    QAbstractSocket first enters Qt::HostLookupState, then performs a
    host name lookup of \a hostName. If the lookup suceeds,
    hostFound() is emitted and QAbstractSocket enters
    Qt::ConnectingState. It then attempts to connect to the address or
    addresses returned by the lookup. Finally, if a connection is
    established, QAbstractSocket enters Qt::ConnectedState and
    connected() is emitted. If there is an error at any point, it
    emits error().

    \a hostName may be an IP address in string form, or it may be a
    host name. QAbstractSocket will do a lookup only if required. Note
    that \a port is in native byte order, unlike some other libraries.

    \sa socketState()
*/
bool QAbstractSocket::connectToHost(const QString &hostName, Q_UINT16 port)
{
    if (d->state == Qt::ConnectingState || d->state == Qt::ConnectedState) {
        close();
    }

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

    Attempts to make a connection to \a address on the specified \a
    port.
*/
bool QAbstractSocket::connectToHost(const QHostAddress &address, Q_UINT16 port)
{
    return connectToHost(address.toString(), port);
}

/*!
    If \a blocking is true, the socket is set to blocking mode and the
    timeout to \a msec milliseconds; otherwise the socket is set to
    non-blocking mode and \a msec is ignored.

    When in blocking mode, connectToHost(), write(), read() and read()
    may suspend the program's execution until their task has been
    completed, or until the timeout has expired. Example:

    \code
        QTcpSocket socket;
        socket.setBlocking(true);
        if (socket.connectToHost("imap", 143)) {
            qDebug("Connected!");
        } else {
            qDebug("Unable to connect: %s", socket.errorString().latin1());
        }
    \encode

    Blocking mode is useful for applications that do not use the event
    loop.

    \sa isBlocking()
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
    bytes are written as the event loop is entered, or by calling
    flush().

    \sa bytesAvailable()
*/
Q_LLONG QAbstractSocket::bytesToWrite() const
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::bytesToWrite() == %i", d->writeBuffer.size());
#endif
    return (Q_LLONG) d->writeBuffer.size();
}

/*!
    Returns the number of incoming bytes that can be read, i.e. the
    size of the input buffer. Equivalent to size().

    \sa bytesToWrite()
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
    Returns the host port number of the local socket if available, in
    native byte order; otherwise returns 0.
*/
Q_UINT16 QAbstractSocket::localPort() const
{
    if (!d->socketLayer.isValid())
        return 0;
    return d->socketLayer.localPort();
}

/*!
    Returns the host address of the local socket if available. (This
    is normally the main IP address of the host, but can be
    e.g. 127.0.0.1 for connections to localhost.) Otherwise returns
    QHostAddress::NullAddress.
*/
QHostAddress QAbstractSocket::localAddress() const
{
    if (!d->socketLayer.isValid())
        return QHostAddress();
    return d->socketLayer.localAddress();
}

/*!
    If the socket is in ConnectedState, this function returns the port
    of the connected peer; otherwise 0 is returned.
*/
Q_UINT16 QAbstractSocket::peerPort() const
{
    if (!d->socketLayer.isValid())
        return 0;
    return d->socketLayer.peerPort();
}

/*!
    If the socket is in ConnectedState, this function returns the
    address of the connected peer; otherwise QHostAddress::NullAddress
    is returned.
*/
QHostAddress QAbstractSocket::peerAddress() const
{
    if (!d->socketLayer.isValid())
        return QHostAddress();
    return d->socketLayer.peerAddress();
}

/*!
    Returns the name of the peer as specified by connectToHost(), or
    an empty QString if this function has not been called.
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
    bool hasLine = d->readBuffer.contains('\n');
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::canReadLine() == %s", hasLine ? "true" : "false");
#endif
    return hasLine;
}

/*!
    Reads a line of ascii data from the socket and returns it as a
    QString. If no line can be read, an empty QString is returned.
    The line is assumed to be terminated by the ASCII line feed
    character '\n'.

    If no line can be read when QAbstractSocket is in blocking mode,
    this function blocks until a whole line can be read, or until the
    timeout expires. If the read buffer is full and no line could be
    read, an empty string is returned.

    \sa canReadLine()
*/
QByteArray QAbstractSocket::readLine()
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::readLine(), blocking ? %s",
           d->isBlocking ? "true" : "false");
#endif
    if (!d->socketLayer.isValid())
        return QByteArray();

    if (d->isBlocking) {
        int timeout = d->blockingTimeout;
        while (!canReadLine()) {
            QTime stopWatch;
            stopWatch.start();
            int oldSize = d->readBuffer.size();
            if (!waitForReadyRead(timeout) || !d->readFromSocket() || oldSize == d->readBuffer.size()) {
                d->socketError = d->socketLayer.socketError();
                d->socketErrorString = d->socketLayer.errorString();
                return QByteArray();
            }

            timeout -= stopWatch.elapsed();
        }
    }

    int endOfLine = d->readBuffer.indexOf('\n');
    if (endOfLine == -1)
        return QByteArray();
    return d->readBuffer.left(endOfLine + 1);
}

/*!
    Returns the native socket descriptor of QAbstractSocket if this is
    available; otherwise -1 is returned.

    The socket descriptor is not available when QAbstractSocket is in
    Qt::UnconnectedState.
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

    Any use of this function stands the risk of being non-portable.
*/
bool QAbstractSocket::setSocketDescriptor(int socketDescriptor,
                                           Qt::SocketState socketState)
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
    Waits for up to \a msecs milliseconds or until there is data
    available for reading. Returns true if there is data available;
    otherwise returns false, in which case error() should be called to
    determine the cause of the error.

    This function is blocking regardless of the blocking/nonblocking
    behavior of QTcpSocket, but is most commonly used in a blocking
    environment to achieve a semi-nonblocking behavior. Example:

    \code
        // Wait for one second to see if we got a line of data yet.
        // If so, print it. Otherwise continue.
        if (socket->waitForReadyRead(1000) && socket->canReadLine())
            qDebug("Read a line: %s", socket->readLine().latin1());
    \endcode
*/
bool QAbstractSocket::waitForReadyRead(int msecs)
{
    if (socketState() != Qt::ConnectedState) {
        qWarning("%s", tr("QAbstractSocket::waitForReadyRead() is only"
                          " allowed in connected state.").latin1());
        return false;
    }

    bool timedOut = false;
    if (!d->socketLayer.waitForRead(msecs, &timedOut) || !d->readFromSocket()) {
        d->socketError = d->socketLayer.socketError();
        d->socketErrorString = d->socketLayer.errorString();
        return false;
    }

    return true;
}

/*!
    Aborts the current connection and resets QAbstractSocket. As
    opposed to close(), this function immediately closes the socket,
    and any unwritten pending data is lost.

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

/*! \reimp
*/
Q_LLONG QAbstractSocket::size() const
{
    return bytesAvailable();
}

/*! \reimp
*/
Q_LLONG QAbstractSocket::at() const
{
    return 0;
}

/*!
    Writes any pending outgoing data to the socket, and returns
    immediately.

    If QAbstractSocket is in blocking mode, this function blocks until
    all data has been written, or until the timeout has expired.
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

    d->readBuffer.readFromFront(data, endOfLine + 1);
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
    return d->readBuffer.readAll();
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
    char c;
    d->readBuffer.readFromFront(&c, 1);
    if (d->readSocketNotifier)
        d->readSocketNotifier->setEnabled(true);
    return c;
}

/*! \reimp
*/
int QAbstractSocket::ungetch(int character)
{
    // ### Size of read buffer
    qWarning("QAbstractSocket::ungetch() unimplemented");
//    d->readBuffer.prepend(character);
    return character;
}

/*! \reimp
*/
int QAbstractSocket::putch(int character)
{
    d->writeBuffer += (char) character;
    d->writeSocketNotifier->setEnabled(true);
    return character;
}

/*!
    Reads at most \a maxLength bytes from the socket into \a data.
    Returns the number of bytes read, or -1 if an error occurred.

    If QAbstractSocket is in blocking mode and no data is available
    (e.g. bytesAvailable() returns 0), this function will block until
    data is available, or until the timeout expires.
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
        return d->readBuffer.readFromFront(data, maxLength);
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
    return d->readBuffer.readFromFront(data, maxLength);
}

/*!
    Writes \a length bytes to the socket from \a data. Returns the
    number of bytes written, or -1 if an error occurred.

    This function always returns immediately, even if QAbstractSocket
    is in blocking mode. To force the data to be written, call
    flush().
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

    Q_LLONG oldSize = d->writeBuffer.size();
    d->writeBuffer.resize(oldSize + length);
    memcpy(d->writeBuffer.data() + oldSize, data, length);

    if (d->writeSocketNotifier)
        d->writeSocketNotifier->setEnabled(true);
    return length;
}

/*!
    Attempts to close the socket. If there is pending data waiting to
    be written, QAbstractSocket will enter Qt::ClosingState and wait
    until all data has been written. Eventuall, it will enter
    Qt::ClosedState.

    When the socket has been closed, the closed() signal is emitted.
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
    readAll() is called. A read buffer size of 0 means that the buffer
    has no size limit.
*/
Q_LLONG QAbstractSocket::readBufferSize() const
{
    return d->readBufferMaxSize;
}

/*!
    Sets the size of QAbstractSocket's internal read buffer to \a bufSize.

    Usually QAbstractSocket reads all data that is available from the
    operating system's socket. If the buffer size is limited to a
    certain size, this means that QAbstractSocket doesn't buffer more
    than this size of data.

    If the size of the read buffer is 0, the read buffer is unlimited
    and all incoming data is buffered. This is the default.

    This option is useful if you only read the data at certain points
    in time, like in a realtime streaming application, or if you wish
    to protect your socket against receiving too much data, which may
    eventually cause your application to run out of memory.

    \sa readBufferSize()
*/
void QAbstractSocket::setReadBufferSize(Q_LLONG bufSize)
{
    d->readBufferMaxSize = bufSize;
}

/*!
    Returns the state of the socket.
*/
Qt::SocketState QAbstractSocket::socketState() const
{
    return d->state;
}

/*!
    Returns the socket type, also referred to as the transport layer
    protocol.
*/
Qt::SocketType QAbstractSocket::socketType() const
{
    return d->socketType;
}

/*!
    Returns the type of error that last occurred.

    \sa SocketError, errorString()
*/
Qt::SocketError QAbstractSocket::socketError() const
{
    return d->socketError;
}

/*!
    Returns the type of error that last occurred as a
    human readable string.

    \sa socketError()
*/
QString QAbstractSocket::errorString() const
{
    return d->socketErrorString;
}

#include "moc_qabstractsocket.cpp"
