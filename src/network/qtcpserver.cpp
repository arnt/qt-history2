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

//#define QTCPSERVER_DEBUG

/*! \class QTcpServer

    \brief The QTcpServer class provides a TCP-based server.

    \reentrant
    \ingroup io
    \module network

    This class makes it possible to accept incoming TCP connections.
    You can specify the port or have QTcpServer pick one
    automatically. You can listen on a specific address or on all the
    machine's addresses.

    Call listen() to have the server listen for incoming connections.
    The newConnection() signal is then emitted each time a client
    connects to the server.

    Call nextPendingConnection() to accept the pending connection as
    a connected QTcpSocket. The function returns a pointer to a
    QTcpSocket in QAbstractSocket::ConnectedState that you can use for
    communicating with the client.

    If an error occurs, serverError() returns the type of error, and
    errorString() can be called to get a human readable description of
    what happened.

    When listening for connections, the address and port on which the
    server is listening are available as serverAddress() and
    serverPort().

    Calling close() makes QTcpServer stop listening for incoming
    connections.

    Although QTcpServer is mostly designed for use with an event
    loop, it's possible to use it without one. In that case, you must
    use waitForNewConnection(), which blocks until either a
    connection is available or a timeout expires.

    The \l network/fortuneserver example illustrates how to use
    QTcpServer in an application.

    \sa QTcpSocket
*/

/*! \fn void QTcpServer::newConnection()

    This signal is emitted every time a new connection is available.

    \sa hasPendingConnections(), nextPendingConnection()
*/

#include "private/qobject_p.h"
#include "qalgorithms.h"
#include "qhostaddress.h"
#include "qlist.h"
#include "qpointer.h"
#include "qsocketlayer_p.h"
#include "qsocketnotifier.h"
#include "qtcpserver.h"
#include "qtcpsocket.h"

class QTcpServerPrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QTcpServer)
public:
    QTcpServerPrivate();
    ~QTcpServerPrivate();

    QList<QTcpSocket *> pendingConnections;

    quint16 port;
    QHostAddress address;

    QAbstractSocket::SocketState state;
    QSocketLayer socketLayer;

    QAbstractSocket::SocketError serverSocketError;
    QString serverSocketErrorString;

    int maxConnections;

    QSocketNotifier *readSocketNotifier;

    // private slots
    void processIncomingConnection(int socketDescriptor);
};

/*! \internal
*/
QTcpServerPrivate::QTcpServerPrivate()
{
    port = 0;
    maxConnections = 30;
    readSocketNotifier = 0;
}

/*! \internal
*/
QTcpServerPrivate::~QTcpServerPrivate()
{
}

/*! \internal
*/
void QTcpServerPrivate::processIncomingConnection(int)
{
    Q_Q(QTcpServer);
    for (;;) {
        if (pendingConnections.count() >= maxConnections) {
#if defined (QTCPSERVER_DEBUG)
            qDebug("QTcpServerPrivate::processIncomingConnection() too many connections");
#endif
            if (readSocketNotifier && readSocketNotifier->isEnabled())
                readSocketNotifier->setEnabled(false);
            return;
        }

        int descriptor = socketLayer.accept();
        if (descriptor == -1)
            break;
#if defined (QTCPSERVER_DEBUG)
        qDebug("QTcpServerPrivate::processIncomingConnection() accepted socket %i", descriptor);
#endif
        q->incomingConnection(descriptor);

        QPointer<QTcpServer> that = q;
        emit q->newConnection();
        if (!that || !q->isListening())
            return;
    }
}

/*!
    Constructs a QTcpServer object.

    \a parent is passed to the QObject constructor.

    \sa listen(), setSocketDescriptor()
*/
QTcpServer::QTcpServer(QObject *parent)
    : QObject(*new QTcpServerPrivate, parent)
{
    d_func()->state = QAbstractSocket::UnconnectedState;
}

/*!
    Destroys the QTcpServer object. If the server is listening for
    connections, the socket is automatically closed.

    Any client \l{QTcpSocket}s that are still connected must either
    disconnect or be reparented before the server is deleted.

    \sa close()
*/
QTcpServer::~QTcpServer()
{
    close();
}

/*!
    Tells the server to listen for incoming connections on address \a
    address and port \a port. If \a port is 0, a port is chosed
    automatically. If \a address is QHostAddress::Any, the server
    will listen on all network interfaces.

    Returns true on success; otherwise returns false.

    \sa isListening()
*/
bool QTcpServer::listen(const QHostAddress &address, quint16 port)
{
    Q_D(QTcpServer);
    if (d->state == QAbstractSocket::ListeningState) {
        qWarning("QTcpServer::listen() called when already listening");
        return false;
    }

    QAbstractSocket::NetworkLayerProtocol proto = address.protocol();
#if defined(Q_NO_IPv6)
    if (proto == QAbstractSocket::IPv6Protocol) {
        // If we have no IPv6 support, then we will not be able to
        // listen on an IPv6 interface.
        d->serverSocketError = QAbstractSocket::SocketOperationUnsupportedError;
        d->serverSocketErrorString = tr("Socket operation unsupported");
        return false;
    }
#endif

    if (!d->socketLayer.initialize(QAbstractSocket::TcpSocket, proto)) {
        d->serverSocketError = d->socketLayer.error();
        d->serverSocketErrorString = d->socketLayer.errorString();
        return false;
    }

    if (!d->socketLayer.bind(address, port)) {
        d->serverSocketError = d->socketLayer.error();
        d->serverSocketErrorString = d->socketLayer.errorString();
        return false;
    }

    if (!d->socketLayer.listen()) {
        d->serverSocketError = d->socketLayer.error();
        d->serverSocketErrorString = d->socketLayer.errorString();
        return false;
    }

    d->readSocketNotifier = new QSocketNotifier(d->socketLayer.socketDescriptor(),
                                                QSocketNotifier::Read, this);
    connect(d->readSocketNotifier, SIGNAL(activated(int)), SLOT(processIncomingConnection(int)));

    d->state = QAbstractSocket::ListeningState;
    d->address = address;
    d->port = port;

#if defined (QTCPSERVER_DEBUG)
    qDebug("QTcpServer::listen(%i, \"%s\") == true (listening on port %i)", port,
           address.toString().toLatin1().constData(), d->socketLayer.localPort());
#endif
    return true;
}

/*!
    Returns true if the server is currently listening for incoming
    connections; otherwise returns false.

    \sa listen()
*/
bool QTcpServer::isListening() const
{
    return d_func()->socketLayer.state() == QAbstractSocket::ListeningState;
}

/*!
    Closes the server. The server will no longer listen for incoming
    connections.

    \sa listen()
*/
void QTcpServer::close()
{
    Q_D(QTcpServer);
    if (d->readSocketNotifier && d->readSocketNotifier->isEnabled())
        d->readSocketNotifier->setEnabled(false);
    delete d->readSocketNotifier;
    d->readSocketNotifier = 0;

    qDeleteAll(d->pendingConnections);
    d->pendingConnections.clear();

    if (d->socketLayer.isValid())
        d->socketLayer.close();

    d->state = QAbstractSocket::UnconnectedState;
}

/*!
    Returns the native socket descriptor the server uses to listen
    for incoming instructions, or -1 if the server is not listening.

    \sa setSocketDescriptor(), isListening()
*/
int QTcpServer::socketDescriptor() const
{
    return d_func()->socketLayer.socketDescriptor();
}

/*!
    Sets the socket descriptor this server should use when listening
    for incoming connections to \a socketDescriptor.

    The socket is assumed to be in listening state.

    \sa socketDescriptor(), isListening()
*/
bool QTcpServer::setSocketDescriptor(int socketDescriptor)
{
    Q_D(QTcpServer);
    if (isListening()) {
        qWarning("QTcpServer::setSocketDescriptor() called when already listening");
        return false;
    }

    if (!d->socketLayer.initialize(socketDescriptor, QAbstractSocket::ListeningState)) {
        d->serverSocketError = d->socketLayer.error();
        d->serverSocketErrorString = d->socketLayer.errorString();
#if defined (QTCPSERVER_DEBUG)
        qDebug("QTcpServer::setSocketDescriptor(%i) failed (%s)", socketDescriptor,
               d->serverSocketErrorString.toLatin1().constData());
#endif
        return false;
    }

    d->readSocketNotifier = new QSocketNotifier(d->socketLayer.socketDescriptor(),
                                                QSocketNotifier::Read, this);
    connect(d->readSocketNotifier, SIGNAL(activated(int)), SLOT(processIncomingConnection(int)));

    d->state = d->socketLayer.state();
    d->address = d->socketLayer.localAddress();
    d->port = d->socketLayer.localPort();

#if defined (QTCPSERVER_DEBUG)
    qDebug("QTcpServer::setSocketDescriptor(%i) succeeded.", socketDescriptor);
#endif
    return true;
}

/*!
    Returns the server's port if the server is listening for
    connections; otherwise returns 0.

    \sa serverAddress(), listen()
*/
quint16 QTcpServer::serverPort() const
{
    return d_func()->socketLayer.localPort();
}

/*!
    Returns the server's address if the server is listening for
    connections; otherwise returns QHostAddress::Null.

    \sa serverPort(), listen()
*/
QHostAddress QTcpServer::serverAddress() const
{
    return d_func()->socketLayer.localAddress();
}

/*!
    Waits for at most \a msec milliseconds or until an incoming
    connection is available. Returns true if a connection is
    available; otherwise returns false. If the operation timed out
    and \a timedOut is not 0, *\a timedOut will be set to true.

    This is a blocking function call. Its use is disadvised in a
    single-threaded GUI application, since the whole application will
    stop responding until the function returns.
    waitForNewConnection() is mostly useful when there is no event
    loop available.

    The non-blocking alternative is to connect to the newConnection()
    signal.

    \sa hasPendingConnections(), nextPendingConnection()
*/
bool QTcpServer::waitForNewConnection(int msec, bool *timedOut)
{
    Q_D(QTcpServer);
    if (d->state != QAbstractSocket::ListeningState)
        return false;

    if (!d->socketLayer.waitForRead(msec, timedOut)) {
        d->serverSocketError = d->socketLayer.error();
        d->serverSocketErrorString = d->socketLayer.errorString();
        return false;
    }

    if (timedOut && *timedOut)
        return false;

    d->processIncomingConnection(0);

    emit newConnection();
    return true;
}

/*!
    Returns true if the server has a pending connection; otherwise
    returns false.

    \sa nextPendingConnection(), setMaxPendingConnections()
*/
bool QTcpServer::hasPendingConnections() const
{
    return !d_func()->pendingConnections.isEmpty();
}

/*!
    Returns the next pending connection as a connected QTcpSocket
    object.

    The socket is created as a child of the server, which means that
    it is automatically deleted when the QTcpServer object is
    destroyed. It is still a good idea to delete the object
    explicitly when you are done with it, to avoid wasting memory.

    \sa hasPendingConnections()
*/
QTcpSocket *QTcpServer::nextPendingConnection()
{
    Q_D(QTcpServer);
    if (d->pendingConnections.isEmpty())
        return 0;

    if (d->readSocketNotifier && !d->readSocketNotifier->isEnabled())
        d->readSocketNotifier->setEnabled(true);

    return d->pendingConnections.takeFirst();
}

/*!
    This virtual function is called by QTcpServer when a new
    connection is available. The \a socketDescriptor argument is the
    native socket descriptor for the accepted connection.

    The base implementation creates a QTcpSocket, sets the socket
    descriptor and then stores the QTcpSocket in an internal list of
    pending connections. Finally newConnection() is emitted.

    Reimplement this function to alter the server's behavior when a
    connection is available.

    \sa newConnection(), nextPendingConnection()
*/
void QTcpServer::incomingConnection(int socketDescriptor)
{
#if defined (QTCPSERVER_DEBUG)
    qDebug("QTcpServer::incomingConnection(%i)", socketDescriptor);
#endif

    QTcpSocket *socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    d_func()->pendingConnections.append(socket);
}

/*!
    Sets the maximum number of pending accepted connections to \a
    numConnections. QTcpServer will accept no more than \a
    numConnections incoming connections before
    nextPendingConnection() is called. By default, the limit is 30
    pending connections.

    Clients that attempt to connect to the server after it has
    reached its maximum number of pending connections will either
    immediately fail to connect or time out.

    \sa maxPendingConnections(), hasPendingConnections()
*/
void QTcpServer::setMaxPendingConnections(int numConnections)
{
    d_func()->maxConnections = numConnections;
}

/*!
    Returns the maximum number of pending accepted connections. The
    default is 30.

    \sa setMaxPendingConnections(), hasPendingConnections()
*/
int QTcpServer::maxPendingConnections() const
{
    return d_func()->maxConnections;
}

/*!
    Returns an error code for the last error that occurred.

    \sa errorString()
*/
QAbstractSocket::SocketError QTcpServer::serverError() const
{
    return d_func()->serverSocketError;
}

/*!
    Returns a human readable description of the last error that
    occurred.

    \sa serverError()
*/
QString QTcpServer::errorString() const
{
    return d_func()->serverSocketErrorString;
}


#include "moc_qtcpserver.cpp"

