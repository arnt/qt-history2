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

/*!
    \class QTcpSocket
    \reentrant
    \brief The QTcpSocket class provides a Tcp connection.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    The most common way to use this class is to call connectToHost()
    passing a host name and a port number. The connected() signal is
    emitted when the connection has been established. The readyRead()
    signal is emitted when data is available for
    reading. bytesAvailable() then returns the number of bytes we can
    read with readBlock(). After writing data to the socket with
    writeBlock(), the bytesWritten() signal is emitted after every
    payload of data has been transmitted. To close the socket, call
    close(). Example:

    \code
        IMAPSocket::IMAPSocket(QObject *parent)
        {
            socket = new QTcpSocket(parent);
            connect(socket, SIGNAL(connected()), SLOT(socketConnected()));
            connect(socket, SIGNAL(readyRead()), SLOT(socketReadData()));
            connect(socket, SIGNAL(closed()), SLOT(socketClosed()));
            socket->connectToHost("imap", 143);
        }

        void socketConnected()
        {
            qDebug("Connected!");
        }

        void socketReadData()
        {
            QByteArray buffer;
            buffer.resize(socket->bytesAvailable());
            socket->readBlock(buffer.data(), buffer.size());
            socket->close();
        }

        void socketClosed()
        {
            qDebug("Closed.");
        }
    \endcode

    The class has two distinctive behaviors: blocking mode and
    non-blocking mode. This behavior is toggled with setBlocking(). By
    default, QTcpSocket is in non-blocking mode. When calling a function
    in blocking mode, the program's execution is suspended until the
    function has completed its task. For example, writeBlock() will
    not return until the block of data has been completely written. In
    non-blocking mode, all functions return immediately and the task
    is performed later. Blocking mode is useful for applications that
    do not use the event loop.

    waitForRead() and waitForWrite() are both \i blocking
    functions. They suspend the execution of the program until data is
    either available for reading or for writing. Example:

    \code
        int main(int argc, char *argv[])
        {
            QApplication app(argc, argv, false);

            QTcpSocket socket;
            socket.setBlocking(true);

            if (!socket.connectToHost("imap", 143)) {
                qDebug("Connection failed");
                return 1;
            }

            if (socket.waitForRead()) {
                int nbytes = socket.bytesAvailable();
                QByteArray array(nbytes, '\0');
                socket.readBlock(array.data(), array.size());
                qDebug("Read data: %s", array.trimmed().data());
            } else {
                qDebug("No data to read!");
            }

            socket.close();
            return 0;
        }
    \encode

    setSocketDescriptor() is provided for those who wish to use the
    QTcpSocket interface on an existing native socket. This descriptor
    can also be retrieved with socketDescriptor().

    QTcpSocket buffers outgoing data. flush() is used to force QTcpSocket
    to transmit any buffered data. In blocking mode, flush() returns
    when all data has been written, or if an error occurred. In
    non-blocking mode, flush() only writes as much as it can without
    blocking.

    QTcpSocket is a QIODevice, which means that it can be safely used
    with QTextStream for reading and writing. Reading with QTextStream
    is only reliable when in blocking mode.

    port() and address() are used to find the port and address of the
    host we have connected to.

    QTcpSocket::hasIPv6Support() is used to check whether the operating
    system supports creating IPv6 sockets.

    \sa QTcpSocketEngine, QUdpSocket, QTcpServer
*/
#include "qdns.h"
#include "qlist.h"
#include "qsignal.h"
#include "qabstractsocket_p.h"
#include "qtcpsocket.h"
#include "qhostaddress.h"

//#define QTCPSOCKET_DEBUG

#define d d_func()
#define q q_func()

class QTcpSocketPrivate : public QAbstractSocketPrivate
{
    Q_DECLARE_PUBLIC(QTcpSocket)

public:
    QTcpSocketPrivate();
    virtual ~QTcpSocketPrivate();

    void close();
};

/*!
    Creates a QTcpSocket object in \c QTcpSocket::Unconnected state.

    The \a parent argument is passed on to the QObject
    constructor.
*/
QTcpSocket::QTcpSocket(QObject *parent) : QAbstractSocket(*new QTcpSocketPrivate, parent)
{
#if defined(QTCPSOCKET_DEBUG)
    qDebug("QTcpSocket::QTcpSocket()");
#endif
    setFlags(Sequential | Async);
    setState(Qt::UnconnectedState);
    d->isBuffered = true;
    setSocketType(Qt::TcpSocket);
}

/*!
    Destroys the socket. Closes the connection if necessary.

    \sa close()
*/

QTcpSocket::~QTcpSocket()
{
#if defined(QTCPSOCKET_DEBUG)
    qDebug("QTcpSocket::~QTcpSocket()");
#endif
    if (state() != Qt::UnconnectedState)
        close();
}

/*!
    Attempts to make a connection to \a host on the specified \a port.

    Any connection or pending connection is closed immediately, and
    QTcpSocket goes into the \c HostLookup state. When the lookup
    succeeds, it emits hostFound(), starts a Tcp connection and goes
    into the \c Connecting state. Finally, when the connection
    succeeds, it emits connected() and goes into the \c Connected
    state. If there is an error at any point, it emits error().

    \a host may be an IP address in string form, or it may be a DNS
    name. QTcpSocket will do a normal DNS lookup if required. Note that
    \a port is in native byte order, unlike some other libraries.

    Returns true if the connection was established; otherwise returns
    false, in which case state() should be called to see if the
    connection is in progress.

    \sa state()
*/
/*
bool QTcpSocket::connectToHost(const QString &host, Q_UINT16 port)
{
    return QAbstractSocket::connectToHost(host, port);
}
*/

/*!
    \fn void QTcpSocket::hostFound()

    This signal is emitted after connectToHost() has been called and
    the host lookup has succeeded.

    \sa connected()
*/

/*!
    \fn void QTcpSocket::connected()

    This signal is emitted after connectToHost() has been called and a
    connection has been successfully established.

    \sa connectToHost(), connectionClosed()
*/

/*!
    \fn void QTcpSocket::closed()

    This signal is emitted when the other end has closed the
    connection. The read buffers may contain buffered input data which
    you can read after the connection was closed.

    \sa connectToHost(), close()
*/

/*!
    \fn void QTcpSocket::readyRead()

    This signal is emitted every time there is new incoming data.

    Bear in mind that new incoming data is only reported once; if you
    do not read any data, this class buffers the data and you can read
    it later, but no signal is emitted unless new data arrives. A good
    practice is to read all data in the slot connected to this signal
    unless you are sure that you need to receive more data to be able
    to process it.

    \sa readBlock(), readLine(), bytesAvailable()
*/


/*!
    \fn void QTcpSocket::bytesWritten(int nbytes)

    This signal is emitted when a payload of data has been written to
    the network.  The \a nbytes parameter specifies how many bytes
    were written.

    The bytesToWrite() function is often used in the same context; it
    indicates how many buffered bytes there are left to write.

    \sa writeBlock(), bytesToWrite()
*/

/*!
    \fn void QSocket::error(int)

    This signal is emitted after an error occurred. The parameter is
    the \l Error value.
*/

QTcpSocketPrivate::QTcpSocketPrivate()
{
    port = 0;
}

QTcpSocketPrivate::~QTcpSocketPrivate()
{
}
