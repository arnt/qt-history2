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

\if defined(commercial)
    It is part of the \l{commercialeditions.html}{Qt Enterprise Edition}.
\endif

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

    The network/broadcastclient and network/broadcastserver examples
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
    Qt::NetworkLayerProtocol proto; \
    if (address.isIPv4Address()) { \
        proto = Qt::IPv4Protocol; \
    } else { \
        d->socketError = Qt::UnsupportedSocketOperationError; \
        d->socketErrorStr = tr("This platform does not support IPv6"); \
        return (a); \
    } \
    if (!d->socketLayer.isValid() || d->socketLayer.protocol() != proto) \
        if (!d->initSocketLayer(Qt::UdpSocket, proto)) \
            return (a); \
    } while (0)
#else
#define QT_ENSURE_INITIALIZED(a) do { \
    Qt::NetworkLayerProtocol proto; \
    if (address.isIPv4Address()) { \
        proto = Qt::IPv4Protocol; \
    } else { \
        proto = Qt::IPv6Protocol; \
    } \
    if (!d->socketLayer.isValid() || d->socketLayer.protocol() != proto) \
        if (!d->initSocketLayer(Qt::UdpSocket, proto)) \
            return (a); \
    } while (0)
#endif
#define QT_CHECK_BOUND(function, a) do { \
    if (!isValid()) { \
        qWarning(function" called on a QUdpSocket when not in Qt::BoundState"); \
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
    : QAbstractSocket(Qt::UdpSocket, *new QUdpSocketPrivate, parent)
{
    setFlags(Sequential | Async);
    d->isBuffered = false;
}

/*!
    Destroys the socket.
*/
QUdpSocket::~QUdpSocket()
{
}

/*!
    Binds this socket to the address \a address and the port \a port.
    When bound, the signal readyRead() is emitted when a UDP datagram
    arrives on the specified address and port. This function is
    useful to write UDP servers.

    On success, it returns true and the socket enters Qt::BoundState;
    otherwise it returns false.

    \sa readDatagram()
*/
bool QUdpSocket::bind(const QHostAddress &address, Q_UINT16 port)
{
    QT_ENSURE_INITIALIZED(false);

    bool result = d->socketLayer.bind(address, port);
    if (!result) {
        d->socketError = d->socketLayer.socketError();
        d->socketErrorString = d->socketLayer.errorString();
        emit error(d->socketError);
        return false;
    }

    d->state = Qt::BoundState;
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
Q_LLONG QUdpSocket::pendingDatagramSize() const
{
    QT_CHECK_BOUND("QUdpSocket::pendingDatagramSize()", -1);
    return d->socketLayer.pendingDatagramSize();
}

/*!
    Sends the datagram at \a data of size \a size to the host
    address \a address at port \a port. On success, the number of
    bytes sent is returned; otherwise this function returns -1.

    Datagrams are always written in one block. The maximum size of a
    datagram is highly platform dependent, but if too large a datagram
    is sent in one go, this function will return -1 and socketError()
    will return Qt::DatagramTooLargeError.

    Sending large datagrams is in general disadvised, as even if they
    are sent successfully, they are likely to be fragmented before
    arriving at their destination.

    Experience has shown that it is usually safe to send datagrams of
    sizes up to 512 bytes.

    \sa readDatagram()
*/
Q_LLONG QUdpSocket::writeDatagram(const char *data, Q_LLONG size, const QHostAddress &address,
                                  Q_UINT16 port)
{
#if defined QUDPSOCKET_DEBUG
    qDebug("QUdpSocket::writeDatagram(%p, %llu, \"%s\", %i)", data, size,
           address.toString().latin1(), port);
#endif
    QT_ENSURE_INITIALIZED(-1);
    Q_LLONG sent = d->socketLayer.writeDatagram(data, size, address, port);
    if (sent >= 0) {
        emit bytesWritten(sent);
    } else {
        d->socketError = d->socketLayer.socketError();
        d->socketErrorString = d->socketLayer.errorString();
        emit error(d->socketError);
    }
    return sent;
}

/*!
    Receives a datagram no larger than \a maxLength bytes and stores
    it in \a data. The sender's host address and port is stored in
    *\a address and *\a port (unless the pointers are 0).

    Returns the size of the received datagram on success; otherwise
    returns -1.

    If \a maxLength is too small, the rest of the datagram will be
    lost. To avoid loss of data, call pendingDatagramSize() to
    determine the size of the pending datagram before reading it.

    \sa writeDatagram(), hasPendingDatagrams(), pendingDatagramSize()
*/
Q_LLONG QUdpSocket::readDatagram(char *data, Q_LLONG maxLength, QHostAddress *address,
                                 Q_UINT16 *port)
{
#if defined QUDPSOCKET_DEBUG
    qDebug("QUdpSocket::readDatagram(%p, %llu, %p, %p)", data, maxLength, address, port);
#endif
    QT_CHECK_BOUND("QUdpSocket::readDatagram()", -1);
    Q_LLONG readBytes = d->socketLayer.readDatagram(data, maxLength, address, port);
    if (readBytes < 0) {
        d->socketError = d->socketLayer.socketError();
        d->socketErrorString = d->socketLayer.errorString();
        emit error(d->socketError);
    }
    d->readSocketNotifier->setEnabled(true);
    return readBytes;
}
