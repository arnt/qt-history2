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
    \class QUdpSocket
    \reentrant
    \brief The QUdpSocket class provides communication via Udp.
\if defined(commercial)
    It is part of the <a href="commercialeditions.html">Qt Enterprise Edition</a>.
\endif

    \ingroup io
    \module network

    The most common way to use this class is to bind to an address and
    port, then send messages with sendMessage() and receive messages
    with receiveMessage(). The readyRead() signal is emitted when a
    Udp message is ready for reading. If a message is available,
    hasMoreMessages() will return true. The size of the next incoming
    message is determined by calling nextMessageSize().
    bytesWritten() is emitted for every payload that is written to the
    network. If an error occurs, error() is emitted.

    \code
        QUdpSocket udpSocket;
        udpSocket.bind(QHostAddress("127.0.0.1"), 7755);

        ...
        ...

        while (udpSocket.hasMoreMessages()) {
            QByteArray message;
            message.resize(udpSocket.nextMessageSize());
            QHostAddress sender;
            Q_UINT16 senderPort;

            udpSocket.receiveMessage(message.data(), message.size(),
                                     &sender, &senderPort);
        }
    \endcode

    By connecting to a Udp recipient with connectToHost(), readBlock()
    and writeBlock() can be used to exchange messages without
    specifying the receiver for each message.

    Unlike QTcpSocket, QUdpSocket is unbuffered. This means that all
    sending and receiving of messages is performed immediately.

    setSocketDescriptor() is provided for those who wish to use the
    QUdpSocket interface on an existing native socket. This
    descriptor can also be retrieved with socketDescriptor().

    QUdpSocket inherits QIODevice. In general, you can treat it as a
    QIODevice for writing, but not for reading. The match isn't
    perfect, since the QIODevice API is designed for streaming devices
    that are controlled by the same machine, and a Udp peer-to-peer
    network connection is both message based and unreliable. This
    means that reads operations will either return a whole packet, or
    they will fail.

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
/*
bool QUdpSocket::connectToHost(const QString &hostName, Q_UINT16 port)
{
    d->hostName = hostName;
    d->port = port;
    d->state = Qt::HostLookupState;

    QDns::getHostByName(hostName, this, SLOT(startConnecting(const QDnsHostInfo &)));
    return false;
}
*/

/*!
    Binds this socket to the address \a address and the port \a port.
    When bound, the signal readyRead() is emitted when a Udp message
    arrives on the given address and port. This function is most often
    used to write Udp servers.

    On success, true is returned and the socket enters BoundState;
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
bool QUdpSocket::bind(Q_UINT16 port)
{
    return bind(QHostAddress::AnyAddress, port);
}

/*!
    Returns true if at least one Udp message is waiting to be read;
    otherwise returns false.
*/
bool QUdpSocket::hasPendingDatagram() const
{
// ### Connected or Bound    QT_ENSURE_INITIALIZED(false);
    return d->socketLayer.hasPendingDatagram();
}

/*!
    Returns the size of the first pending Udp message. If there is no
    message available, this function returns -1.
*/
Q_LLONG QUdpSocket::pendingDatagramSize() const
{
// ### Connected or Bound    QT_ENSURE_INITIALIZED(-1);
    return d->socketLayer.pendingDatagramSize();
}

/*!
    Sends the message \a data of length \a length to the host address
    \a address at port \a port. On success, the number of bytes sent
    is returned; otherwise this function returns -1.
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
    Receives a message no larger than \a maxLength and stores it in \a
    data. The sender's host address and port is stored in \a address
    and \a port unless they are 0.

    On success, the number of bytes received is returned; otherwise
    this function returns -1.
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
