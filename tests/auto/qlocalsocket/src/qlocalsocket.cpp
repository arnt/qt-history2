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

#include <qdatetime.h>

/*!
    \class QLocalSocket
    \since 4.4

    \brief The QLocalSocket class provides a local socket.

    This class makes it possible to accept incoming local socket
    connections.

    If an error occurs, socketError() returns the type of error, and
    errorString() can be called to get a human readable description
    of what happened.

    Although QLocalSocket is designed for use with an event loop, it's possible
    to use it without one. In that case, you must use waitForConnected(),
    waitForReadyRead(), waitForBytesWritten(), and waitForDisconnected()
    which blocks until either a connection is available or a timeout expires.

    \sa QLocalServer
*/

/*!
    \fn void QLocalSocket::connected()

    This signal is emitted after connectToName() has been called and
    a connection has been successfully established.

    \sa connectToName(), disconnected()
*/

/*!
    \fn void QLocalSocket::disconnected()

    This signal is emitted when the socket has been disconnected.

    \sa connectToName(), disconnect(), abort()
*/

/*!
    \fn void QLocalSocket::error(QLocalSocket::LocalSocketError socketError)

    This signal is emitted after an error occurred. The \a socketError
    parameter describes the type of error that occurred.

    QLocalSocket::LocalSocketError is not a registered metatype, so for queued
    connections, you will have to register it with Q_REGISTER_METATYPE.

    \sa error(), errorString()
*/

/*!
    \fn void QLocalSocket::stateChanged(QLocalSocket::LocalSocketState socketState)

    This signal is emitted whenever QLocalSocket's state changes.
    The \a socketState parameter is the new state.

    QLocalSocket::SocketState is not a registered metatype, so for queued
    connections, you will have to register it with Q_REGISTER_METATYPE.

    \sa state()
*/

/*!
    Creates a new local socket. The \a parent argument is passed to QObject's constructor.
 */
QLocalSocket::QLocalSocket(QObject * parent) : QIODevice(*new QLocalSocketPrivate, parent)
{
    Q_D(QLocalSocket);
    d->init();
}

/*!
    Destroys the socket.
 */
QLocalSocket::~QLocalSocket()
{
    close();
}

/*!
    Returns the name of the peer as specified by connectToName(),
    or an empty QString if connectToName() has not been called.

    \sa connectToName()
 */
QString QLocalSocket::peerName() const
{
    Q_D(const QLocalSocket);
    return d->peerName;
}

/*!
    Returns the state of the socket.

    \sa error()
 */
QLocalSocket::LocalSocketState QLocalSocket::state() const
{
    Q_D(const QLocalSocket);
    return d->state;
}

/*!
  \enum QLocalSocket::LocalSocketError

  The LocalServerError enumeration represents the errors that can occur.
  The most recent error can be retrieved through a call to
  \l QLocalSocket::error().

  \value ConnectionRefusedError The connection was refused by the peer (or timed out).
  \value RemoteClosedError  The remote socket closed the connection. Note that the client socket (i.e., this socket) will be closed after the remote close notification has been sent.
  \value NotFoundError  The local socket name was not found.
  \value SocketAccessError The socket operation failed because the application lacked the required privileges.
  \value SocketResourceError The local system ran out of resources (e.g., too many sockets).
  \value SocketTimeoutError The socket operation timed out.
  \value DatagramTooLargeError The datagram was larger than the operating system's limit (which can be as low as 8192 bytes).
  \value ConnectionError An error occurred with the connection.
  \value UnsupportedSocketOperationError The requested socket operation is not supported by the local operating system.
  \value UnknownSocketError An unidentified error occurred.
 */

/*!
  \enum QLocalSocket::LocalSocketState

  This enum describes the different states in which a socket can be.
  \sa QLocalSocket::state()

  \value UnconnectedState The socket is not connected.
  \value ConnectingState The socket has started establishing a connection.
  \value ConnectedState A connection is established.
  \value ClosingState The socket is about to close (data may still be waiting to be written).
 */

#include "moc_qlocalsocket.cpp"
