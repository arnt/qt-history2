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

#include "qlocalserver.h"
#include "qlocalserver_p.h"
#include "qlocalsocket.h"

/*!
    \class QLocalServer
    \sense 4.4

    \brief The QLocalServer class provides a local socket based server.

    This class makes it possible to accept incoming local socket
    connections.

    Call \l QLocalServer::listen() to have the server start listening
    for incoming connections on a specified key.  The
    \l QLocalServer::newConnection() signal is then emitted each time a client
    connects to the server.

    Call \l nextPendingConnection() to accept the pending connection
    as a connected QLocalSocket.  The function returns a pointer to a
    QLocalSocket that can be used for communicating with the client.

    If an error occurs, serverError() returns the type of error, and
    errorString() can be called to get a human readable description
    of what happened.

    When listening for connections, the name which the server is
    listening on is available as serverName().

    Calling close() makes QLocalServer stop listening for incoming connections.

    Although QLocalServer is designed for use with an event loop, it's possible
    to use it without one. In that case, you must use waitForNewConnection(),
    which blocks until either a connection is available or a timeout expires.

    \sa QLocalSocket
*/

/*!
    Create a new Unix socket server with the given \a parent.

    \sa listen()
 */
QLocalServer::QLocalServer(QObject *parent)
        : QObject(*new QLocalServerPrivate, parent)
{
  Q_D(QLocalServer);
  d->init();
}

/*!
    Destructor will close the server if it is currently
    listening for connections.

    \sa close()
 */
QLocalServer::~QLocalServer()
{
    close();
}

/*!
    Stop listening for incoming connections.  Existing connections are not
    effected, but any new connections will be refused.

    \sa isListening(), listen()
 */
bool QLocalServer::close()
{
    Q_D(QLocalServer);
    if (!isListening())
        return false;
    d->serverName = QString();
    qDeleteAll(d->pendingConnections);
    d->pendingConnections.clear();
    return d->closeServer();
}

/*!
    Returns the human-readable message appropriate to the current error
    reported by error(). If no suitable string is available, an empty
    string is returned.

    \sa error()
 */
QString QLocalServer::errorString() const
{
    Q_D(const QLocalServer);
    return d->errorString;
}

/*!
    Returns true if the server has a pending connection; otherwise
    returns false.

    \sa nextPendingConnect(), setMaxPendingConnections()
 */
bool QLocalServer::hasPendingConnections() const
{
    Q_D(const QLocalServer);
    return !(d->pendingConnections.isEmpty());
}

/*!
    This virtual function is called by QLocalServer when a new connection
    is available. The socket argument is the native socket descriptor for
    the accepted connection.

    The base implementation creates a QLocalSocket, sets the socket descriptor
    and then stores the QLocalSocket in an internal list of pending
    connections. Finally newConnection() is emitted.

    Reimplement this function to alter the server's behavior
    when a connection is available.

    \sa newConnection(), nextPendingConnection()
 */
void QLocalServer::incomingConnection(int socketDescriptor)
{
    Q_D(QLocalServer);
    QLocalSocket *socket = new QLocalSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    d->pendingConnections.append(socket);
    emit newConnection();
}

/*!
    Returns true if the server is listening for incoming connections
    otherwise false.

    \sa listen(), close()
 */
bool QLocalServer::isListening() const
{
    Q_D(const QLocalServer);
    return !(d->serverName.isEmpty());
}

/*!
    Tells the server to listen for incoming connections on name.
    If the server is currently listening then it will first close().

    Return true on success otherwise false.

    \sa serverName(), isListening(), close()
 */
bool QLocalServer::listen(const QString &name)
{
    Q_D(QLocalServer);

    if (isListening())
        close();

    if (name.isEmpty()) {
        d->error = NameError;
        d->errorString = tr("%1: Name error").arg(QLatin1String("QLocalServer::listen"));
        return false;
    }

    if (!d->listen(name))
        return false;

    d->serverName = name;
    return true;
}

/*!
    Returns the maximum number of pending accepted connections.
    The default is 30.

    \sa setMaxPendingConnections(), hasPendingCOnnections()
 */
int QLocalServer::maxPendingConnections() const
{
    Q_D(const QLocalServer);
    return d->maxPendingConnections;
}

/*!
    \fn void QLocalServer::newConnection()

    This signal is emitted every time a new connection is available.

    \sa hasPendingConnections(), nextPendingConnections()
*/

/*!
    Returns the next pending connection as a connected QLocalSocket object.

    The socket is created as a child of the server, which means that it is
    automatically deleted when the QLocalServer object is destroyed. It is
    still a good idea to delete the object explicitly when you are done with
    it, to avoid wasting memory.

    0 is returned if this function is called when there are no pending
    connections.

    \sa hasPendingConnections(), newConnection()
 */
QLocalSocket *QLocalServer::nextPendingConnection()
{
    Q_D(QLocalServer);
    if (d->pendingConnections.isEmpty())
        return 0;
    return d->pendingConnections.takeFirst();
}

/*!
    Returns the name that the server is listening on.

    \sa listen()
 */
QString QLocalServer::serverName() const
{
    Q_D(const QLocalServer);
    return d->serverName;
}

/*!
  \enum QLocalServer::ServerError

  The ServerError enumeration represents the errors that can occur during server
  establishment.  The most recent error can be retrieved through a call to
  \l QLocalServer::serverError().

  \value NoError No error has occurred.
  \value KeyError Error with the local server key.
  \value UnknownError An unknown error has occurred.
 */

/*!
    Returns the type of error that occurred last or NoError.

    \sa errorString()
 */
QLocalServer::LocalServerError QLocalServer::serverError() const
{
    Q_D(const QLocalServer);
    return d->error;
}

/*!
    Sets the maximum number of pending accepted connections to
    \a numConnections.  QLocalServer will accept no more than
    \a numConnections incoming connections before nextPendingConnection()
    is called.  By default, the limit is 30 pending connections.

    Clients may still able to connect after the server has reached its maximum
    number of pending connections (i.e., QLocalSocket can still emit the
    connected() signal). QLocalServer will stop accepting the new connections,
    but the operating system may still keep them in queue.

    \sa maxPendingConnections(), hasPendingConnections()
 */
void QLocalServer::setMaxPendingConnections(int numConnections)
{
    Q_D(QLocalServer);
    d->maxPendingConnections = numConnections;
}

/*!
    Waits for at most \a msec milliseconds or until an incoming connection
    is available.  Returns true if a connection is available; otherwise
    returns false.  If the operation timed out and \a timedOut is not 0,
    *timedOut will be set to true.

    This is a blocking function call. Its use is disadvised in a
    single-threaded GUI application, since the whole application will stop
    responding until the function returns. waitForNewConnection() is mostly
    useful when there is no event loop available.

    The non-blocking alternative is to connect to the newConnection() signal.

    \sa hasPendingConnections(), nextPendingConnection()
 */
bool QLocalServer::waitForNewConnection(int msec, bool *timedOut)
{
    Q_D(QLocalServer);
    if (!isListening())
        return false;

    d->waitForNewConnection(msec, timedOut);

    return !d->pendingConnections.isEmpty();
}

#include "moc_qlocalserver.cpp"

