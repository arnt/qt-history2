/*! \class QAbstractSocket

*/


// ### Are the state of the socket api and the socket ever different?

#include "qabstractsocket.h"
#include "qabstractsocket_p.h"
#include "qdatetime.h"
#include "qsocketlayer.h"
#include "qhostaddress.h"
#include "qsignal.h"

#include <time.h>

#define QABSTRACTSOCKET_DEFAULT_BUFFER_SIZE 16384

//#define QABSTRACTSOCKET_DEBUG

#define d d_func()
#define q q_func()

QAbstractSocketPrivate::QAbstractSocketPrivate()
{
    port = 0;
    readSocketNotifier = 0;
    writeSocketNotifier = 0;
    readSocketNotifierCalled = false;
    isBuffered = false;
    isBlocking = false;
    state = Qt::UnconnectedState;
    readBufferMaxSize = QABSTRACTSOCKET_DEFAULT_BUFFER_SIZE;
    socketError = Qt::UnknownSocketError;
    socketErrorString = QT_TRANSLATE_NOOP(QAbstractSocket, "Unknown error");
}

QAbstractSocketPrivate::~QAbstractSocketPrivate()
{
    close();
}

void QAbstractSocketPrivate::close()
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::close()");
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

void QAbstractSocketPrivate::setupSocketNotifiers()
{
    readSocketNotifier = new QSocketNotifier(socketLayer.socketDescriptor(),
                                             QSocketNotifier::Read);
    writeSocketNotifier = new QSocketNotifier(socketLayer.socketDescriptor(),
                                              QSocketNotifier::Write);
    writeSocketNotifier->setEnabled(false);

    QObject::connect(readSocketNotifier, SIGNAL(activated(int)),
                     q, SLOT(canReadNotification(int)));
    QObject::connect(writeSocketNotifier, SIGNAL(activated(int)),
                     q, SLOT(canWriteNotification(int)));
}

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

    close();

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

void QAbstractSocketPrivate::canReadNotification(int)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::canReadNotification()");
#endif

    // prevent recursive calls
    if (readSocketNotifierCalled)
        return;

    readSocketNotifierCalled = true;

    // prevent notifier from getting fired more times
    readSocketNotifier->setEnabled(false);

    Q_LLONG nbytes = socketLayer.bytesAvailable();

    // read data from the socket into the read buffer
    if (isBuffered) {
        // return if there is no space in the buffer
        if (readBuffer.size() == readBufferMaxSize) {
            readSocketNotifierCalled = false;
            return;
        }

        // if reading from the socket fails after getting a read
        // notification, close the socket.
        if (!d->readFromSocket()) {
            q->close();
            readSocketNotifierCalled = false;
#if defined (QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::canReadNotification() closing socket");
#endif
            return;

        }
    }

    emit q->readyRead();

    // Anything might have happened in whatever is connected to the
    // readyRead() slot.
    if (!this)
        return;

    // if bytesAvailable is the same after readRead was emitted, don't
    // reenable the socket notifier. one notification is enough.
    if (nbytes != q->bytesAvailable())
        readSocketNotifier->setEnabled(true);
    readSocketNotifierCalled = false;
}

void QAbstractSocketPrivate::canWriteNotification(int)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::canWriteNotification()");
#endif
    writeSocketNotifier->setEnabled(false);

    // if in connecting state, check if the connection has been
    // established.
    if (state == Qt::ConnectingState)
        testConnection();
    else
        flush();
}

/*! \internal

Writes pending data in the write buffers to the socket. When in
blocking mode, this function blocks until the buffers are empty;
otherwise the function writes as much as it can without blocking.

It is usually invoked by canWriteNotification after one or more calls to
write().
*/
void QAbstractSocketPrivate::flush()
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::flush(), pending: %i bytes",
           writeBuffer.size());
#endif
    if (!socketLayer.isValid() || writeBuffer.isEmpty())
        return;

    do {
        // attempt to write it all in one chunk.
        Q_LLONG written = socketLayer.write(writeBuffer.data(), writeBuffer.size());
        if (written < 0) {
            socketError = socketLayer.socketError();
            socketErrorString = socketLayer.errorString();
            emit q->error(socketError);
            close();
            break;
        } else {
            writeBuffer = writeBuffer.mid(written);
            emit q->bytesWritten(written);
        }
    } while (isBlocking && !writeBuffer.isEmpty() && socketLayer.waitForWrite());

    if (!writeBuffer.isEmpty()) {
        d->writeSocketNotifier->setEnabled(true);
    } else if (state == Qt::ClosingState) {
        q->close();
    }
}

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

    // if there are no addresses in the host list, report this to the
    // user.
    if (addresses.isEmpty()) {
        state = Qt::UnconnectedState;
        d->socketError = Qt::HostNotFoundError;
        d->socketErrorString = QT_TRANSLATE_NOOP(QAbstractSocket, "Host not found");
        emit q->error(Qt::HostNotFoundError);
        return;
    }

    // enter Connecting state (see also sn_write, which is called by
    // the write socket notifier after connect())
    state = Qt::ConnectingState;

    // report the successful host lookup
    emit q->hostFound();

    // the addresses returned by the lookup will be tested one after
    // another by the connectToNextAddress() slot.
    if (isBlocking) {
        connectToNextAddress();
    } else {
        qInvokeMetaMember(q, "connectToNextAddress", Qt::QueuedConnection);
    }
}

/*! \internal

Called by a queued connection from tryConnecting() and a signal
from QTcpSocketEngine, this function takes the first address of the
address list and tries to connect to it. If the connection
succeeds, QSocketEngine will emit connected(), which is connected
to socketEngineConnected(). Otherwise,
error(ConnectionRefusedError) is emitted.
*/
void QAbstractSocketPrivate::connectToNextAddress()
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocketPrivate::connectToNextAddress()");
#endif

    do {
        if (addresses.isEmpty()) {
#if defined(QABSTRACTSOCKET_DEBUG)
            qDebug("QAbstractSocketPrivate::connectToNextAddress(), all addresses failed.");
#endif
            q->setState(Qt::UnconnectedState);
            socketError = Qt::ConnectionRefusedError;
            socketErrorString = QT_TRANSLATE_NOOP(QAbstractSocket,
                                                  "Connection refused");
            emit q->error(Qt::ConnectionRefusedError);
            return;
        }

        host = addresses.takeFirst();
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::connectToNextAddress(), connecting to %s:%i",
               host.toString().latin1(), port);
#endif
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

        if (!socketLayer.isValid() || socketLayer.protocol() != protocol)
            initSocketLayer(q->socketType(), protocol);

        if (socketLayer.connectToHost(host, port)) {
            state = Qt::ConnectedState;
            q->setFlags(q->flags() | QIODevice::Open | QIODevice::ReadWrite);
            emit q->connected();
            return;
        }

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

        if (isBlocking) {
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

            testConnection();
        } else {
            // this will eventually call testConnection()
            d->writeSocketNotifier->setEnabled(true);
        }

    } while (state != Qt::ConnectedState && isBlocking);
}

void QAbstractSocketPrivate::testConnection()
{
    if (socketLayer.socketState() == Qt::ConnectedState || socketLayer.connectToHost(host, port)) {
        state = Qt::ConnectedState;
        q->setFlags(q->flags() | QIODevice::Open | QIODevice::ReadWrite);

        emit q->connected();
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::testConnection() connection to %s:%i established",
               host.toString().latin1(), port);
#endif
        return;
    }

#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::testConnection() connection failed");
#endif
    qInvokeMetaMember(q, SLOT(connectToNextAddress()));
}

/*! \internal
 */
bool QAbstractSocketPrivate::readFromSocket()
{
    // read data into the read buffer
    Q_LLONG bytesToRead = qMin(socketLayer.bytesAvailable(),
                               readBufferMaxSize - readBuffer.size());
    if (bytesToRead > 0) {
        Q_LLONG oldSize = readBuffer.size();
        readBuffer.reserve(oldSize + bytesToRead);

        Q_LLONG readBytes = socketLayer.read(readBuffer.data() + oldSize,
                                           bytesToRead);
        if (!socketLayer.isValid()) {
            socketError = socketLayer.socketError();
            socketErrorString = socketLayer.errorString();
            emit q->error(socketError);
            return false;
        }

        if (readBytes > 0)
            readBuffer.resize(oldSize + readBytes);

#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::readFromSocket() read %lli new bytes",
               readBytes);
#endif
    } else {
#if defined(QABSTRACTSOCKET_DEBUG)
        qDebug("QAbstractSocketPrivate::readFromSocket() not reading");
#endif
    }

    return true;
}

/*!
  Creates a new abstract socket. The \a parent argument is passed to
  QObject's constructor.
*/
QAbstractSocket::QAbstractSocket(QAbstractSocketPrivate &p, QObject *parent)
    : QObject(parent), QIODevice(p)
{
    // The d_ptr member variable is necessary because we have two base
    // classes with a variable called d_ptr.
    d_ptr = static_cast<QAbstractSocketPrivate *>(QIODevice::d_ptr);
}

/*!
  Creates a new abstract socket. The \a parent argument is passed to
  QObject's constructor.
*/
QAbstractSocket::QAbstractSocket(QObject *parent)
    : QObject(parent), QIODevice(*new QAbstractSocketPrivate())
{
    d_ptr = static_cast<QAbstractSocketPrivate *>(QIODevice::d_ptr);
}

/*!
  Destroys the abstract socket.
*/
QAbstractSocket::~QAbstractSocket()
{
}

bool QAbstractSocket::isValid() const
{
    return d->socketLayer.isValid();
}

bool QAbstractSocket::connectToHost(const QHostAddress &address, Q_UINT16 port)
{
    return connectToHost(address.toString(), port);
}

bool QAbstractSocket::connectToHost(const QString &hostName, Q_UINT16 port)
{
    d->hostName = hostName;
    d->port = port;
    d->state = Qt::HostLookupState;

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

/*!
    Returns true if the socket is blocking; otherwise returns false.

    \sa setBlocking()
*/
bool QAbstractSocket::isBlocking() const
{
    return d->isBlocking;
}

/*!
    If \a blocking is true, the socket is set to blocking mode and the
    timeout to \a msec milliseconds; otherwise the socket is set to
    non-blocking mode and \a msec is ignored.

    When in blocking mode, connectToHost(), writeBlock(), readBlock()
    and readLine() will suspend the program's execution until their
    task has been completed, or until the timeout has expired. When in
    non-blocking mode, these functions return immediately, and their
    task is enqueued for future processing. Example:

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
*/
void QAbstractSocket::setBlocking(bool blocking, int msec)
{
    d->isBlocking = blocking;
    d->blockingTimeout = msec;

    // Set Raw or Async depending on the value of blocking.
    setMode(blocking ? (mode() & ~Async) : (mode() | Async));
}

Q_LLONG QAbstractSocket::bytesToWrite() const
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::bytesToWrite() == %i", d->writeBuffer.size());
#endif
    return (Q_LLONG) d->writeBuffer.size();
}

void QAbstractSocket::clearBuffers()
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::clearBuffers()");
#endif

    d->writeBuffer.clear();
    d->readBuffer.clear();
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
  Returns the host port number of this socket if available, in
  native byte order; otherwise returns 0.
*/
Q_UINT16 QAbstractSocket::localPort() const
{
    if (!d->socketLayer.isValid())
        return 0;
    return d->socketLayer.localPort();
}

/*!
  Returns the host address of this socket if available. (This is
  normally the main IP address of the host, but can be
  e.g. 127.0.0.1 for connections to localhost.) Otherwise returns an
  empty QHostAddress.
*/
QHostAddress QAbstractSocket::localAddress() const
{
    if (!d->socketLayer.isValid())
        return QHostAddress();
    return d->socketLayer.localAddress();
}

/*!
  Returns the host port number of this socket if available, in
  native byte order; otherwise returns 0.
*/
Q_UINT16 QAbstractSocket::peerPort() const
{
    if (!d->socketLayer.isValid())
        return 0;
    return d->socketLayer.peerPort();
}

/*!
  Returns the host address of this socket if available. (This is
  normally the main IP address of the host, but can be
  e.g. 127.0.0.1 for connections to localhost.) Otherwise returns an
  empty QHostAddress.
*/
QHostAddress QAbstractSocket::peerAddress() const
{
    if (!d->socketLayer.isValid())
        return QHostAddress();
    return d->socketLayer.peerAddress();
}

QString QAbstractSocket::peerName() const
{
    return d->hostName;
}

/*!
  Returns true if a line of data can be read from the socket;
  otherwise returns false.
*/
bool QAbstractSocket::canReadLine() const
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::canReadLine()");
#endif
    return d->readBuffer.contains('\n');
}

/*!
  Reads a line of ascii data from the socket and returns it as a
  QString. If no line can be read, an empty QString is returned.

  When QTcpSocket is in blocking mode, this function blocks until a
  whole line can be read. If the read buffer is full and no line
  could be read, an empty string is returned.
*/
QByteArray QAbstractSocket::readLine()
{
    if (!d->socketLayer.isValid())
        return QByteArray();

    if (d->isBlocking) {
        int timeout = d->blockingTimeout;
        while (!canReadLine()) {
            QTime stopWatch;
            stopWatch.start();
            if (!waitForReadyRead(timeout) || !d->readFromSocket()) {
                d->socketError = d->socketLayer.socketError();
                d->socketErrorString = d->socketLayer.errorString();
                return QByteArray();
            }

            timeout -= stopWatch.elapsed();
        }
    }

    int endOfLine = d->readBuffer.find('\n');
    if (endOfLine == -1)
        return QByteArray();

    QByteArray tmp = d->readBuffer.left(endOfLine + 1);
    d->readBuffer.remove(0, endOfLine + 1);
    return tmp;
}

int QAbstractSocket::socketDescriptor() const
{
    return d->socketLayer.socketDescriptor();
}

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

    d->state = socketState;
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
  if (socket->waitForRead(1000) && socket->canReadLine())
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

void QAbstractSocket::abort()
{
    d->readBuffer.clear();
    d->writeBuffer.clear();
    close();
}

bool QAbstractSocket::open(int)
{
    // ### is this true?
    qWarning("QAbstractSocket::open(int): Sockets must be connected");
    return false;
}

Q_LLONG QAbstractSocket::size() const
{
    return bytesAvailable();
}

Q_LLONG QAbstractSocket::at() const
{
    return 0;
}

void QAbstractSocket::flush()
{
    d->flush();
}

Q_LLONG QAbstractSocket::readLine(char *data, Q_LLONG maxLength)
{
    int endOfLine = d->readBuffer.find('\n');
    if (endOfLine == -1 || endOfLine > maxLength)
        return -1;

    memcpy(data, d->readBuffer.data(), endOfLine + 1);
    d->readBuffer.remove(0, endOfLine + 1);
    return endOfLine + 1;
}

QByteArray QAbstractSocket::readAll()
{
    QByteArray tmp = d->readBuffer;
    d->readBuffer.clear();
    return tmp;
}

bool QAbstractSocket::seek(Q_LLONG off)
{
    return read(0, off) != -1;
}

int QAbstractSocket::getch()
{
    // ### Performance!!!
    char c = d->readBuffer.at(0);
    d->readBuffer = d->readBuffer.mid(1);
    return c;
}

int QAbstractSocket::ungetch(int character)
{
    // ### Size of read buffer
    d->readBuffer.prepend(character);
    return character;
}

int QAbstractSocket::putch(int character)
{
    d->writeBuffer += (char) character;
    d->writeSocketNotifier->setEnabled(true);
    return character;
}

/*
 */
Q_LLONG QAbstractSocket::read(char *data, Q_LLONG maxlen)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::read(%p, %lli), readBuffer.size() == %i",
           data, maxlen, d->readBuffer.size());
#endif
    if (!isValid()) {
        qWarning("QAbstractSocket::read: Invalid socket");
        return -1;
    }

    Q_LLONG totalRead = 0;
    time_t timeLimit = time(0) * 1000 + d->blockingTimeout;
    bool isLooping = false;
    for (;;) {
        // Read data into the read buffer.
        if (d->readBuffer.size() + totalRead < maxlen) {
            int oldBufferSize = d->readBuffer.size();
            if (!d->readFromSocket())
                break;

            // Prevent infinite spinning when no data is read from the
            // socket.
            if (oldBufferSize == d->readBuffer.size()) {
                if (isLooping)
                    break;
            }
        }

        // If readFromSocket() read data, copy it to its destination.
        if (d->readBuffer.size() > 0) {
            Q_LONG oldSize = d->readBuffer.size();
            Q_LONG bytesToRead = qMin(maxlen - totalRead, oldSize);
            memcpy(data + totalRead, d->readBuffer.data(), bytesToRead);
            d->readBuffer.remove(0, bytesToRead);
            totalRead += bytesToRead;
        }

        if (totalRead == maxlen)
            break;

        // Nonblocking sockets are done after the first read.
        if (!d->isBlocking)
            break;

        // In blocking mode, wait for more data to read.
        if (!waitForReadyRead((timeLimit - time(0) * 1000)))
            break;

        isLooping = true;
    }

    return totalRead;
}

/*!
  Writes \a len bytes to the socket from \a data and returns the
  number of bytes written. Returns -1 if an error occurred.

  This is used for \c QAbstractSocket::Stream sockets.
*/
Q_LLONG QAbstractSocket::write(const char *data, Q_LLONG len)
{
#if defined (QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::write(%p, %lli)", data, len);
#endif
    if (!isValid()) {
        qWarning("QAbstractSocket::write: Invalid socket");
        return -1;
    }

    if (d->isBuffered) {
        Q_LLONG oldSize = d->writeBuffer.size();
        d->writeBuffer.resize(oldSize + len);
        memcpy(d->writeBuffer.data() + oldSize, data, len);

        // canWriteNotification will call flush()
        if (!d->writeSocketNotifier->isEnabled())
            d->writeSocketNotifier->setEnabled(true);
        return len;
    }

    return d->socketLayer.write(data, len);
}
/*!
  \reimp

  Closes the socket and sets the socket identifier to -1 (invalid).

  (This function ignores errors; if there are any then a file
  descriptor leakage might result. As far as we know, the only error
  that can arise is EBADF, and that would of course not cause
  leakage. There may be OS-specific errors that we haven't come
  across, however.)

  \sa open()
*/
void QAbstractSocket::close()
{
#if defined(QABSTRACTSOCKET_DEBUG)
    qDebug("QAbstractSocket::close()");
#endif
    // Disable and delete read notification
    if (d->readSocketNotifier) {
        d->readSocketNotifier->setEnabled(false);
        delete d->readSocketNotifier;
        d->readSocketNotifier = 0;
    }

    // Perhaps emit closing()
    if (d->state != Qt::ClosingState) {
        d->state = Qt::ClosingState;
        emit closing();
    }

    // Wait for pending data to be written. In blocking mode the write
    // buffer list will always be empty.
    if (d->writeBuffer.size() > 0) {
        if (d->isBlocking) {
            bool timedOut = false;
            while (d->socketLayer.waitForWrite(d->blockingTimeout, &timedOut) && !timedOut)
                flush();
        } else {
            d->writeSocketNotifier->setEnabled(true);
            return;
        }
    }

    // Disable and delete write notification
    if (d->writeSocketNotifier) {
        d->writeSocketNotifier->setEnabled(false);
        delete d->writeSocketNotifier;
        d->writeSocketNotifier = 0;
    }

    d->close();
    d->state = Qt::UnconnectedState;

    emit closed();
    return;
}

/*!
  Returns the size of the internal read buffer. This limits the
  amount of data that can be sent to the client before read()
  or readAll() is called.
*/
Q_LLONG QAbstractSocket::readBufferSize() const
{
    return d->readBufferMaxSize;
}

/*!
  Sets the size of the internal read buffer to \a bufSize.

  ### May truncate the buffer!
*/
void QAbstractSocket::setReadBufferSize(Q_LLONG bufSize)
{
    d->readBufferMaxSize = bufSize;
    if (d->readBuffer.size() > bufSize)
        d->readBuffer.resize(bufSize);
}

Qt::SocketState QAbstractSocket::socketState() const
{
    return d->state;
}

Qt::SocketType QAbstractSocket::socketType() const
{
    return d->socketType;
}
void QAbstractSocket::setSocketType(Qt::SocketType socketType)
{
    d->socketType = socketType;
}

/*!
  Returns the type of error that last occurred.

  \sa SocketError
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
