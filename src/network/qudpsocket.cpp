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

/*! \class QUdpSocket

    \reentrant
    \brief The QUdpSocket class provides a UDP socket.

    \ingroup io
    \module network

    UDP (User Datagram Protocol) is a lightweight, unreliable,
    datagram-oriented, connectionless protocol. It can be used when
    reliability isn't important. QUdpSocket is a subclass of
    QAbstractSocket that allows you to send and receive UDP
    datagrams.

    The most common way to use this class is to bind to an address
    and port using bind(), then call writeDatagram() and
    readDatagram() to transfer data.

    The socket emits the bytesWritten() signal every time a datagram
    is written to the network. If you just want to send datagrams,
    you don't need to call bind().

    The readyRead() signal is emitted whenever datagrams arrive. In
    that case, hasPendingDatagrams() returns true. Call
    pendingDatagramSize() to obtain the size of the first pending
    datagram, and readDatagram() to read it.

    Example:

    \code
        void Server::initSocket()
        {
            udpSocket = new QUdpSocket(this);
            udpSocket->bind(QHostAddress::LocalHost, 7755);

            connect(udpSocket, SIGNAL(readyRead()),
                    this, SLOT(readPendingDatagrams()));
        }

        void Server::readPendingDatagrams()
        {
            while (udpSocket->hasPendingDatagrams()) {
                QByteArray datagram;
                datagram.resize(udpSocket->pendingDatagramSize());
                QHostAddress sender;
                Q_UINT16 senderPort;

                udpSocket->readDatagram(datagram.data(), datagram.size(),
                                        &sender, &senderPort);

                processTheDatagram(datagram);
            }
        }
    \endcode

    With QUdpSocket, you can also establish a virtual connection to a
    UDP server using connectToHost() and then use read() and write()
    to exchange datagrams without specifying the receiver for each
    datagram.

    The \l network/broadcastclient and \l network/broadcastserver examples
    illustrate how to use QUdpSocket in applications.

    \sa QTcpSocket
*/
#include "qdns.h"
#include "qhostaddress.h"
#include "qabstractsocket_p.h"
#include "qudpsocket.h"

#define d d_func()
#define q q_func()

// #define QUDPSOCKET_DEBUG

#if defined(QT_NO_IPV6)
#define QT_ENSURE_INITIALIZED(a) do { \
    QAbstractSocket::NetworkLayerProtocol proto = address.protocol(); \
    if (proto == QUdpSocket::IPv6Protocol) { \
        d->socketError = QUdpSocket::UnsupportedSocketOperationError; \
        setErrorString(tr("This platform does not support IPv6")); \
        return (a); \
    } \
    if (!d->socketLayer.isValid() || d->socketLayer.protocol() != proto) \
        if (!d->initSocketLayer(QUdpSocket::UdpSocket, proto)) \
            return (a); \
    } while (0)
#else
#define QT_ENSURE_INITIALIZED(a) do { \
    QAbstractSocket::NetworkLayerProtocol proto = address.protocol(); \
    if (!d->socketLayer.isValid() || d->socketLayer.protocol() != proto) \
        if (!d->initSocketLayer(QUdpSocket::UdpSocket, proto)) \
            return (a); \
    } while (0)
#endif
#define QT_CHECK_BOUND(function, a) do { \
    if (!isValid()) { \
        qWarning(function" called on a QUdpSocket when not in QUdpSocket::BoundState"); \
        return (a); \
    } } while (0)

class QUdpSocketPrivate : public QAbstractSocketPrivate
{
    Q_DECLARE_PUBLIC(QUdpSocket)
};

/*!
    Creates a QUdpSocket object.

    \a parent is passed to the QObject constructor.

    \sa socketType()
*/
QUdpSocket::QUdpSocket(QObject *parent)
    : QAbstractSocket(UdpSocket, *new QUdpSocketPrivate, parent)
{
    d->isBuffered = false;
}

/*!
    Destroys the socket, closing the connection if necessary.

    \sa close()
*/
QUdpSocket::~QUdpSocket()
{
}

/*!
    Binds this socket to the address \a address and the port \a port.
    When bound, the signal readyRead() is emitted whenever a UDP
    datagram arrives on the specified address and port. This function
    is useful to write UDP servers.

    On success, the functions returns true and the socket enters
    BoundState; otherwise it returns false.

    \sa readDatagram()
*/
bool QUdpSocket::bind(const QHostAddress &address, Q_UINT16 port)
{
    QT_ENSURE_INITIALIZED(false);

    bool result = d->socketLayer.bind(address, port);
    if (!result) {
        d->socketError = d->socketLayer.error();
        setErrorString(d->socketLayer.errorString());
        emit error(d->socketError);
        return false;
    }

    d->state = BoundState;
    d->readSocketNotifier->setEnabled(true);
    return true;
}

/*! \overload

    Binds to QHostAddress:Any on port \a port.
*/
bool QUdpSocket::bind(Q_UINT16 port)
{
    return bind(QHostAddress::Any, port);
}

/*!
    Returns true if at least one datagram is waiting to be read;
    otherwise returns false.

    \sa pendingDatagramSize(), readDatagram()
*/
bool QUdpSocket::hasPendingDatagrams() const
{
    QT_CHECK_BOUND("QUdpSocket::hasPendingDatagrams()", false);
    return d->socketLayer.hasPendingDatagrams();
}

/*!
    Returns the size of the first pending UDP datagram. If there is
    no datagram available, this function returns -1.

    \sa hasPendingDatagrams(), readDatagram()
*/
Q_LONGLONG QUdpSocket::pendingDatagramSize() const
{
    QT_CHECK_BOUND("QUdpSocket::pendingDatagramSize()", -1);
    return d->socketLayer.pendingDatagramSize();
}

/*!
    Sends the datagram at \a data of size \a size to the host
    address \a address at port \a port. Returns the number of
    bytes sent on success; otherwise returns -1.

    Datagrams are always written as one block. The maximum size of a
    datagram is highly platform-dependent, but can be as low as 8192
    bytes. If the datagram is too large, this function will return -1
    and error() will return DatagramTooLargeError.

    Sending datagrams larger than 512 bytes is in general disadvised,
    as even if they are sent successfully, they are likely to be
    fragmented by the IP layer before arriving at their final
    destination.

    \sa readDatagram()
*/
Q_LONGLONG QUdpSocket::writeDatagram(const char *data, Q_LONGLONG size, const QHostAddress &address,
                                  Q_UINT16 port)
{
#if defined QUDPSOCKET_DEBUG
    qDebug("QUdpSocket::writeDatagram(%p, %llu, \"%s\", %i)", data, size,
           address.toString().latin1(), port);
#endif
    QT_ENSURE_INITIALIZED(-1);
    Q_LONGLONG sent = d->socketLayer.writeDatagram(data, size, address, port);
    if (sent >= 0) {
        emit bytesWritten(sent);
    } else {
        d->socketError = d->socketLayer.error();
        setErrorString(d->socketLayer.errorString());
        emit error(d->socketError);
    }
    return sent;
}

/*! \fn Q_LONGLONG QUdpSocket::writeDatagram(const QByteArray &datagram,
                                             const QHostAddress &host, Q_UINT16 port)
    \overload

    Sends the datagram \a datagram to the host address \a host and at
    port \a port.
*/

/*!
    Receives a datagram no larger than \a maxSize bytes and stores
    it in \a data. The sender's host address and port is stored in
    *\a address and *\a port (unless the pointers are 0).

    Returns the size of the datagram on success; otherwise returns
    -1.

    If \a maxSize is too small, the rest of the datagram will be
    lost. To avoid loss of data, call pendingDatagramSize() to
    determine the size of the pending datagram before attempting to
    read it. If \a maxSize is 0, the datagram will be discarded.

    \sa writeDatagram(), hasPendingDatagrams(), pendingDatagramSize()
*/
Q_LONGLONG QUdpSocket::readDatagram(char *data, Q_LONGLONG maxSize, QHostAddress *address,
                                    Q_UINT16 *port)
{
#if defined QUDPSOCKET_DEBUG
    qDebug("QUdpSocket::readDatagram(%p, %llu, %p, %p)", data, maxSize, address, port);
#endif
    QT_CHECK_BOUND("QUdpSocket::readDatagram()", -1);
    Q_LONGLONG readBytes = d->socketLayer.readDatagram(data, maxSize, address, port);
    if (readBytes < 0) {
        d->socketError = d->socketLayer.error();
        setErrorString(d->socketLayer.errorString());
        emit error(d->socketError);
    }
    d->readSocketNotifier->setEnabled(true);
    return readBytes;
}
