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

#include "qlocalsocket.h"
#include "qlocalsocket_p.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qdebug.h>

QLocalSocketPrivate::QLocalSocketPrivate() : QIODevicePrivate(),
        delayConnect(0),
        connectingSocket(0),
        state(QLocalSocket::UnconnectedState)
{
}

void QLocalSocketPrivate::_q_stateChanged(QAbstractSocket::SocketState newState)
{
    Q_Q(QLocalSocket);
    QLocalSocket::LocalSocketState currentState = state;
    switch(newState) {
    case QAbstractSocket::UnconnectedState:
        state = QLocalSocket::UnconnectedState;
        break;
    case QAbstractSocket::ConnectingState:
        state = QLocalSocket::ConnectingState;
        break;
    case QAbstractSocket::ConnectedState:
        state = QLocalSocket::ConnectedState;
        break;
    case QAbstractSocket::ClosingState:
        state = QLocalSocket::ClosingState;
        break;
    default:
#if defined QLOCALSOCKET_DEBUG
        qWarning() << "QLocalSocket::Unhandled socket state change:" << newState;
#endif
        return;
    }
    if (currentState != state)
        emit q->stateChanged(state);
}

void QLocalSocketPrivate::errorOccured(QLocalSocket::LocalSocketError error,
                                       const QString &function)
{
    Q_Q(QLocalSocket);
    QString errorString;
    switch (error) {
    case QLocalSocket::ConnectionRefusedError:
        errorString = QLocalSocket::tr("%1: Connection refused").arg(function);
        unixSocket.setSocketError(QAbstractSocket::ConnectionRefusedError);
        break;
    case QLocalSocket::RemoteClosedError:
        errorString = QLocalSocket::tr("%1: Remote closed").arg(function);
        unixSocket.setSocketError(QAbstractSocket::RemoteHostClosedError);
        break;
    case QLocalSocket::NotFoundError:
        errorString = QLocalSocket::tr("%1: Invalid name").arg(function);
        unixSocket.setSocketError(QAbstractSocket::HostNotFoundError);
        break;
    case QLocalSocket::SocketAccessError:
        errorString = QLocalSocket::tr("%1: Socket access error").arg(function);
        unixSocket.setSocketError(QAbstractSocket::SocketAccessError);
        break;
    case QLocalSocket::SocketResourceError:
        errorString = QLocalSocket::tr("%1: Socket resource error").arg(function);
        unixSocket.setSocketError(QAbstractSocket::SocketResourceError);
        break;
    case QLocalSocket::SocketTimeoutError:
        errorString = QLocalSocket::tr("%: Socket operation timed out").arg(function);
        unixSocket.setSocketError(QAbstractSocket::SocketTimeoutError);
        break;
    case QLocalSocket::DatagramTooLargeError:
        errorString = QLocalSocket::tr("%: Datagram too large").arg(function);
        unixSocket.setSocketError(QAbstractSocket::DatagramTooLargeError);
        break;
    case QLocalSocket::ConnectionError:
        errorString = QLocalSocket::tr("%: Connection error").arg(function);
        unixSocket.setSocketError(QAbstractSocket::NetworkError);
        break;
    case QLocalSocket::UnsupportedSocketOperationError:
        errorString = QLocalSocket::tr("%1: The socket operation is not supported").arg(function);
        unixSocket.setSocketError(QAbstractSocket::UnsupportedSocketOperationError);
        break;
    case QLocalSocket::UnknownSocketError:
    default:
        errorString = QLocalSocket::tr("%1: Unknown error %2").arg(function).arg(errno);
        unixSocket.setSocketError(QAbstractSocket::UnknownSocketError);
#if defined QLOCALSOCKET_DEBUG
        qWarning() << errorString;
        perror("QLocalSocket");
#endif
    }

    q->setErrorString(errorString);
    unixSocket.setErrorString(errorString);

    // A big error, disconnect
    unixSocket.setSocketState(QAbstractSocket::UnconnectedState);
    bool stateChanged = (state != QLocalSocket::UnconnectedState);
    state = QLocalSocket::UnconnectedState;
    if (stateChanged)
        q->emit stateChanged(state);
}

void QLocalSocketPrivate::init()
{
    Q_Q(QLocalSocket);
    // QIODevice signals
    q->connect(&unixSocket, SIGNAL(aboutToClose()), q, SIGNAL(aboutToClose()));
    q->connect(&unixSocket, SIGNAL(bytesWritten(qint64)),
               q, SIGNAL(bytesWritten(qint64)));
    q->connect(&unixSocket, SIGNAL(readyRead()), q, SIGNAL(readyRead()));
    // QAbstractSocket signals
    q->connect(&unixSocket, SIGNAL(connected()), q, SIGNAL(connected()));
    q->connect(&unixSocket, SIGNAL(disconnected()), q, SIGNAL(disconnected()));
    q->connect(&unixSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
               q, SLOT(_q_stateChanged(QAbstractSocket::SocketState)));
}

/*!
    Attempts to make a connection to \a name.

    The socket is opened in the given openMode and first enters ConnectingState.
    It then attempts to connect to the address or addresses returned by the lookup.
    Finally, if a connection is established, QLocalSocket enters ConnectedState
    and emits connected().

    At any point, the socket can emit error() to signal that an error occurred.

    See also state(), peerName(), and waitForConnected().
  */
void QLocalSocket::connectToName(const QString &name, OpenMode newOpenMode)
{
    Q_D(QLocalSocket);
    if (state() == ConnectedState || state() == ConnectingState)
        return;

    d->errorString = QString();
    d->unixSocket.setSocketState(QAbstractSocket::ConnectingState);
    d->state = ConnectingState;
    emit stateChanged(ConnectingState);

    if (name.isEmpty()) {
        d->errorOccured(NotFoundError, QLatin1String("QLocalSocket::connectToName"));
        return;
    }

    // create the socket
    d->connectingSocket = 0;
    if ((d->connectingSocket = qSocket(PF_UNIX, SOCK_STREAM, 0)) == -1) {
        d->errorOccured(UnsupportedSocketOperationError,
                        QLatin1String("QLocalSocket::connectToName"));
        return;
    }

    // set non blocking
    int flags = fcntl(d->connectingSocket, F_GETFL, 0);
    if (flags == -1 || fcntl(d->connectingSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        d->errorOccured(UnknownSocketError,
                QLatin1String("QLocalSocket::connectToName"));
        return;
    }
    d->connectingName = name;
    d->connectingOpenMode = newOpenMode;
    d->_q_connectToSocket();
}

void QLocalSocketPrivate::_q_connectToSocket()
{
    Q_Q(QLocalSocket);
    QString connectingPathName = QDir::tempPath() + QLatin1Char('/') + connectingName;
    struct sockaddr_un name;
    name.sun_family = PF_UNIX;
    ::memcpy(name.sun_path, connectingPathName.toLatin1().data(),
             connectingPathName.toLatin1().size() + 1);
    if (qConnect(connectingSocket, (struct sockaddr *)&name, sizeof(name)) == -1) {
        QString function = QLatin1String("QLocalSocket::connectToName");
        switch (errno)
        {
        case EINVAL:
        case ECONNREFUSED:
            errorOccured(QLocalSocket::ConnectionRefusedError, function);
            break;
        case ENOENT:
            errorOccured(QLocalSocket::NotFoundError, function);
            break;
        case EACCES:
        case EPERM:
            errorOccured(QLocalSocket::SocketAccessError, function);
            break;
        case ETIMEDOUT:
            errorOccured(QLocalSocket::SocketTimeoutError, function);
            break;
        case EAGAIN:
            // Try again later, all of the sockets listening are full
            if (!delayConnect) {
                delayConnect = new QSocketNotifier(connectingSocket, QSocketNotifier::Write);
                q->connect(delayConnect, SIGNAL(activated(int)), q, SLOT(_q_connectToSocket()));
            }
            delayConnect->setEnabled(true);
            break;
        default:
            errorOccured(QLocalSocket::UnknownSocketError, function);
        }
        return;
    }

    if (delayConnect) {
        delayConnect->setEnabled(false);
        delete delayConnect;
    }

    peerName = connectingName;
    if (unixSocket.setSocketDescriptor(connectingSocket, QAbstractSocket::ConnectedState, connectingOpenMode)) {
        q->setOpenMode(connectingOpenMode);
        q->emit connected();
    } else {
        errorOccured(QLocalSocket::UnknownSocketError, QLatin1String("QLocalSocket::connectToName"));
    }
    connectingSocket = 0;
    connectingName = QString();
    connectingOpenMode = 0;
}

/*!
    Initializes QLocalSocket with the native socket descriptor
    socketDescriptor. Returns true if socketDescriptor is accepted
    as a valid socket descriptor; otherwise returns false. The socket is
    opened in the mode specified by openMode, and enters the socket state
    specified by socketState.

    Note: It is not possible to initialize two local sockets with the same
    native socket descriptor.

    \sa socketDescriptor(), state(), openMode()
 */
bool QLocalSocket::setSocketDescriptor(int socketDescriptor, LocalSocketState socketState, OpenMode openMode)
{
    Q_D(QLocalSocket);
    QAbstractSocket::SocketState newSocketState = QAbstractSocket::UnconnectedState;
    switch (socketState) {
    case ConnectingState:
        newSocketState = QAbstractSocket::ConnectingState;
        break;
    case ConnectedState:
        newSocketState = QAbstractSocket::ConnectedState;
        break;
    case ClosingState:
        newSocketState = QAbstractSocket::ClosingState;
        break;
    case UnconnectedState:
        newSocketState = QAbstractSocket::UnconnectedState;
        break;
    }
    setOpenMode(openMode);
    d->state = socketState;
    return d->unixSocket.setSocketDescriptor(socketDescriptor, newSocketState, openMode);
}

/*!
    Returns the native socket descriptor of the QLocalSocket object if
    this is available; otherwise returns -1.

    The socket descriptor is not available when QLocalSocket
    is in UnconnectedState.

    \sa setSocketDescriptor()
 */
int QLocalSocket::socketDescriptor() const
{
    Q_D(const QLocalSocket);
    return d->unixSocket.socketDescriptor();
}

/*!
    \reimp
 */
qint64 QLocalSocket::readData(char *data, qint64 c)
{
    Q_D(QLocalSocket);
    return d->unixSocket.readData(data, c);
}

/*!
    \reimp
 */
qint64 QLocalSocket::writeData(const char *data, qint64 c)
{
    Q_D(QLocalSocket);
    return d->unixSocket.writeData(data, c);
}

/*!
    Aborts the current connection and resets the socket.
    Unlike disconnect(), this function immediately closes the socket, clearing
    any pending data in the write buffer.

    \sa disconnect(), close()
 */
void QLocalSocket::abort()
{
    Q_D(QLocalSocket);
    d->unixSocket.abort();
}

/*!
    \reimp
 */
qint64 QLocalSocket::bytesAvailable() const
{
    Q_D(const QLocalSocket);
    return d->unixSocket.bytesAvailable();
}

/*!
    \reimp
 */
qint64 QLocalSocket::bytesToWrite() const
{
    Q_D(const QLocalSocket);
    return d->unixSocket.bytesToWrite();
}

/*!
    \reimp
 */
bool QLocalSocket::canReadLine() const
{
    Q_D(const QLocalSocket);
    return d->unixSocket.canReadLine();
}

/*!
    \reimp
 */
void QLocalSocket::close()
{
    Q_D(QLocalSocket);
    return d->unixSocket.close();
}

/*!
    \reimp
 */
bool QLocalSocket::waitForBytesWritten(int msecs)
{
    Q_D(QLocalSocket);
    return d->unixSocket.waitForBytesWritten(msecs);
}

/*!
    This function writes as much as possible from the internal write buffer
    to the underlying network socket, without blocking. If any data was
    written, this function returns true; otherwise false is returned.

    Call this function if you need QLocalSocket to start sending buffered data
    immediately. The number of bytes successfully written depends on the
    operating system. In most cases, you do not need to call this function,
    because QLocalSocket will start sending data automatically once control
    goes back to the event loop. In the absence of an event loop, call
    waitForBytesWritten() instead.

    \sa write(), waitForBytesWritten()
 */
bool QLocalSocket::flush()
{
    Q_D(QLocalSocket);
    return d->unixSocket.flush();
}

/*!
    Attempts to close the socket. If there is pending data waiting to be
    written, QLocalSocket will enter ClosingState and wait until all data
    has been written. Eventually, it will enter UnconnectedState and emit
    the disconnected() signal.

    \sa connectToName()
*/
void QLocalSocket::disconnectFromName()
{
    Q_D(QLocalSocket);
    d->unixSocket.disconnectFromHost();
}

/*!
    Returns the type of error that last occurred.

    \sa state(), errorString()
*/
QLocalSocket::LocalSocketError QLocalSocket::error() const
{
    Q_D(const QLocalSocket);
    switch (d->unixSocket.error()) {
    case QAbstractSocket::ConnectionRefusedError:
        return QLocalSocket::ConnectionRefusedError;
    case QAbstractSocket::RemoteHostClosedError:
        return QLocalSocket::RemoteClosedError;
    case QAbstractSocket::HostNotFoundError:
        return QLocalSocket::NotFoundError;
    case QAbstractSocket::SocketAccessError:
        return QLocalSocket::SocketAccessError;
    case QAbstractSocket::SocketResourceError:
        return QLocalSocket::SocketResourceError;
    case QAbstractSocket::SocketTimeoutError:
        return QLocalSocket::SocketTimeoutError;
    case QAbstractSocket::DatagramTooLargeError:
        return QLocalSocket::DatagramTooLargeError;
    case QAbstractSocket::NetworkError:
        return QLocalSocket::ConnectionError;
    case QAbstractSocket::UnsupportedSocketOperationError:
        return QLocalSocket::UnsupportedSocketOperationError;
    case QAbstractSocket::UnknownSocketError:
        return QLocalSocket::UnknownSocketError;
    default:
#if defined QLOCALSOCKET_DEBUG
        qWarning() << "Error not handled" << d->unixSocket.error();
#endif
        break;
    }
    return UnknownSocketError;
}

/*!
    Returns true if the socket is valid and ready for use; otherwise
    returns false.
    Note: The socket's state must be ConnectedState before reading
    and writing can occur.

    \sa state()
 */
bool QLocalSocket::isValid() const
{
    Q_D(const QLocalSocket);
    return d->unixSocket.isValid();
}

/*!
    Returns the size of the internal read buffer. This limits the amount of
    data that the client can receive before you call read() or readAll().
    A read buffer size of 0 (the default) means that the buffer has no size
    limit, ensuring that no data is lost.

    \sa setReadBufferSize(), read()
 */
qint64 QLocalSocket::readBufferSize() const
{
    Q_D(const QLocalSocket);
    return d->unixSocket.readBufferSize();
}

/*!
    Sets the size of QLocalSocket's internal read buffer to be size bytes.

    If the buffer size is limited to a certain size, QLocalSocket won't
    buffer more than this size of data. Exceptionally, a buffer size of 0
    means that the read buffer is unlimited and all incoming data is buffered.
    This is the default.

    This option is useful if you only read the data at certain points in
    time (e.g., in a real-time streaming application) or if you want to
    protect your socket against receiving too much data, which may eventually
    cause your application to run out of memory.

    \sa readBufferSize(), read()
 */
void QLocalSocket::setReadBufferSize(qint64 size)
{
    Q_D(QLocalSocket);
    d->unixSocket.setReadBufferSize(size);
}

/*!
    Waits until the socket is connected, up to msecs milliseconds. If the
    connection has been established, this function returns true; otherwise
    it returns false. In the case where it returns false, you can call
    error() to determine the cause of the error.

    The following example waits up to one second for a connection
    to be established:

    \code
        socket->connectToName("market");
        if (socket->waitForConnected(1000))
            qDebug("Connected!");
    \endcode

    If msecs is -1, this function will not time out.

    \sa connectToName(), connected()
 */
bool QLocalSocket::waitForConnected(int msecs)
{
    Q_D(QLocalSocket);
    QTime stopWatch;
    stopWatch.start();
    while (state() == ConnectingState
            && (msecs == -1 || stopWatch.elapsed() < msecs)) {
        d->_q_connectToSocket();
    }

    return state() == ConnectedState;
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
        socket->disconnectFromName();
        if (socket->waitForDisconnected(1000))
            qDebug("Disconnected!");
    \endcode

    If msecs is -1, this function will not time out.

    \sa disconnectFromName(), close()
*/

bool QLocalSocket::waitForDisconnected(int msecs)
{
    Q_D(QLocalSocket);
    if (state() == UnconnectedState)
        return false;
    return d->unixSocket.waitForDisconnected(msecs);
}

/*!
    This function blocks until data is available for reading and the
    \l{QIODevice::}{readyRead()} signal has been emitted. The function
    will timeout after \a msecs milliseconds; the default timeout is
    3000 milliseconds.

    The function returns true if data is available for reading;
    otherwise it returns false (if an error occurred or the
    operation timed out).

    \sa waitForBytesWritten()
*/
bool QLocalSocket::waitForReadyRead(int msecs)
{
    Q_D(QLocalSocket);
    if (state() == QLocalSocket::UnconnectedState)
        return false;
    return d->unixSocket.waitForReadyRead(msecs);
}

