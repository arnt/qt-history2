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
    \brief The QUdpSocket class provides a UDP socket for sending and
    receiving datagrams.

\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    The most common way to use this class is to bind to an address and
    port, then send messages with sendDatagram() and receive messages
    with receiveDatagram(). The readyRead() signal is emitted when a
    UDP message is ready for reading. If a datagram is available,
    hasPendingDatagram() will return true. The size of the next
    datagram is determined by calling pendingDatagramSize().
    bytesWritten() is emitted for every datagram that is written to
    the network. If an error occurs, error() is emitted.

    \code
        QUdpSocket udpSocket;
        udpSocket.bind(QHostAddress("127.0.0.1"), 7755);

        ...
        ...

        while (udpSocket.hasPendingDatagram()) {
            QByteArray message;
            message.resize(udpSocket.pendingDatagramSize());
            QHostAddress sender;
            Q_UINT16 senderPort;

            udpSocket.receiveDatagram(message.data(), message.size(),
                                      &sender, &senderPort);
        }
    \endcode

    By connecting to a UDP server with connectToHost(), read() and
    write() can be used to exchange datagrams without specifying the
    receiver for each message.

    All sending and receiving of datagrams is performed immediately.
    bytesToWrite() always returns 0.

    \sa QTcpSocket, QAbstractSocket
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
public:
    QUdpSocketPrivate();
    ~QUdpSocketPrivate();

    bool blockingConnect;
};

/*!
    Creates a QUdpSocket object. The \a parent argument is passed to
    QAbstractSocket's constructor.
*/
QUdpSocket::QUdpSocket(QObject *parent)
    : QAbstractSocket(Qt::UdpSocket, *new QUdpSocketPrivate, parent)
{
    setFlags(Sequential | Async);
    setState(Qt::UnconnectedState);
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
    arrives on the given address and port. This function is most often
    used to write UDP servers.

    On success, true is returned and the socket enters Qt::BoundState;
    otherwise false is returned.

    \sa receiveMessage()
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

    Binds to QHostAddress:AnyAddress on \a port.
*/
bool QUdpSocket::bind(Q_UINT16 port)
{
    return bind(QHostAddress::AnyAddress, port);
}

/*!
    Returns true if at least one datagram is waiting to be read;
    otherwise returns false.
*/
bool QUdpSocket::hasPendingDatagram() const
{
    QT_CHECK_BOUND("QUdpSocket::hasPendingDatagram()", false);
    return d->socketLayer.hasPendingDatagram();
}

/*!
    Returns the size of a pending UDP datagram. If there is no
    datagram available, this function returns -1.
*/
Q_LLONG QUdpSocket::pendingDatagramSize() const
{
    QT_CHECK_BOUND("QUdpSocket::pendingDatagramSize()", -1);
    return d->socketLayer.pendingDatagramSize();
}

/*!
    Sends the datagram at \a data of length \a length to the host
    address \a address at port \a port. On success, the number of
    bytes sent is returned; otherwise this function returns -1.

    Datagrams are always written in one block. The maximum size of a
    datagram is highly platform dependent, but if too large a datagram
    is sent in one go, this function will return -1 and socketError()
    will return Qt::DatagramTooLargeError.

    Sending large datagrams is in general disadvised, as even if they
    are sent successfully, they are likely to be fragmented before
    arriving at their destination.

    Experience has shown that it is in general safe to send datagrams
    no larger than 512 bytes.
*/
Q_LLONG QUdpSocket::sendDatagram(const char *data, Q_LLONG length,
                                 const QHostAddress &address, Q_UINT16 port)
{
#if defined QUDPSOCKET_DEBUG
    qDebug("QUdpSocket::sendMessage(%p, %llu, \"%s\", %i)", data, length,
           address.toString().latin1(), port);
#endif
    QT_ENSURE_INITIALIZED(-1);
    Q_LLONG sent = d->socketLayer.sendDatagram(data, length, address, port);
    if (sent > 0)
        emit bytesWritten(sent);
    return sent;
}

/*!
    Receives a datagram no larger than \a maxLength bytes and stores
    it in \a data. The sender's host address and port is stored in \a
    address and \a port. If either of these pointers is 0, the
    corresponding value is discarded.

    On success, the size of the received datagram is returned;
    otherwise this function returns -1.

    To avoid unnecessarily loss of data, call pendingDatagramSize() to
    determine the size of the pending message before reading it. If \a
    maxLength is too small, the rest of the datagram will be lost.

    \sa hasPendingDatagram()
*/
Q_LLONG QUdpSocket::receiveDatagram(char *data, Q_LLONG maxLength,
                                    QHostAddress *addr, Q_UINT16 *port)
{
#if defined QUDPSOCKET_DEBUG
    qDebug("QUdpSocket::receiveMessage(%p, %llu, %p, %p"
           data, maxLength, addr, port);
#endif
// ###    QT_ENSURE_INITIALIZED(-1);
    Q_LLONG readBytes = d->socketLayer.receiveDatagram(data, maxLength, addr, port);
    d->readSocketNotifier->setEnabled(true);
    return readBytes;
}

/*! \internal
*/
QUdpSocketPrivate::QUdpSocketPrivate()
{
}

/*! \internal
*/
QUdpSocketPrivate::~QUdpSocketPrivate()
{
}
