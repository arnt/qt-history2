/****************************************************************************
**
** Unix implementation of platform specifics in the QSocketLayer class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <time.h>
#include <errno.h>
#include <sys/types.h>

#include "qglobal.h"
#include "qhostaddress.h"
#include "qplatformdefs.h"
#include "qsocketlayer.h"
#include "qsocketlayer_p.h"

//#define QSOCKETLAYER_DEBUG

#if defined QSOCKETLAYER_DEBUG
#include <qstring.h>
#include <ctype.h>

/*
    Returns a human readable representation of the first \a len
    characters in \a data.
*/
static QByteArray qt_prettyDebug(const char *data, int len, int maxLength)
{
    if (!data) return "(null)";
    QByteArray out;
    for (int i = 0; i < len; ++i) {
        char c = data[i];
        if (isprint(c)) {
            out += c;
        } else switch (c) {
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            QString tmp;
            tmp.sprintf("\\%o", c);
            out += tmp;
        }
    }

    if (len < maxLength)
        out += "...";

    return out;
}
#endif

// Almost always the same. If not, specify in qplatformdefs.h.
#if !defined(QT_SOCKOPTLEN_T)
# define QT_SOCKOPTLEN_T QT_SOCKLEN_T
#endif

// Tru64 redefines accept -> _accept with _XOPEN_SOURCE_EXTENDED
static inline int qt_socket_accept(int s, struct sockaddr *addr, QT_SOCKLEN_T *addrlen)
{ return ::accept(s, addr, addrlen); }
#if defined(accept)
# undef accept
#endif

// Solaris redefines bind -> __xnet_bind with _XOPEN_SOURCE_EXTENDED
static inline int qt_socket_bind(int s, struct sockaddr *addr, QT_SOCKLEN_T addrlen)
{ return ::bind(s, addr, addrlen); }
#if defined(bind)
# undef bind
#endif

// Solaris redefines connect -> __xnet_connect with _XOPEN_SOURCE_EXTENDED
static inline int qt_socket_connect(int s, struct sockaddr *addr, QT_SOCKLEN_T addrlen)
{ return ::connect(s, addr, addrlen); }
#if defined(connect)
# undef connect
#endif

// UnixWare 7 redefines listen -> _listen
static inline int qt_socket_listen(int s, int backlog)
{ return ::listen(s, backlog); }
#if defined(listen)
# undef listen
#endif

// UnixWare 7 redefines socket -> _socket
static inline int qt_socket_socket(int domain, int type, int protocol)
{ return ::socket(domain, type, protocol); }
#if defined(socket)
# undef socket
#endif

static void qt_ignore_sigpipe()
{
    struct sigaction noaction;
    noaction.sa_handler = SIG_IGN;
    noaction.sa_flags = 0;
    ::sigaction(SIGPIPE, &noaction, 0);
}

/*! \internal

    Extracts the port and address from a sockaddr, and stores them in
    \a port and \a addr if they are non-null.
*/
static inline void qt_socket_getPortAndAddress(struct sockaddr *sa, Q_UINT16 *port, QHostAddress *addr)
{
#if !defined(QT_NO_IPV6)
    if (sa->sa_family == AF_INET6) {
        struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)sa;
        Q_IPV6ADDR tmp;
        memcpy(&tmp, &sa6->sin6_addr.s6_addr, sizeof(tmp));
        if (addr) {
            QHostAddress tmpAddress;
            tmpAddress.setAddress(tmp);
            *addr = tmpAddress;
        }
        if (port)
            *port = ntohs(sa6->sin6_port);
        return;
    }
#endif
    struct sockaddr_in *sa4 = (struct sockaddr_in *)sa;
    if (port)
        *port = ntohs(sa4->sin_port);
    if (addr) {
        QHostAddress tmpAddress;
        tmpAddress.setAddress(ntohl(sa4->sin_addr.s_addr));
        *addr = tmpAddress;
    }
}

/*! \internal

    Creates and returns a new socket descriptor of type \a socketType
    and \a socketProtocol.  Returns -1 on failure.
*/
bool QSocketLayerPrivate::createNewSocket(Qt::SocketType socketType,
                                         Qt::NetworkLayerProtocol socketProtocol)
{
    int protocol = (socketProtocol == Qt::IPv6Protocol) ? AF_INET6 : AF_INET;
    int type = (socketType == Qt::UdpSocket) ? SOCK_DGRAM : SOCK_STREAM;
    int socket = qt_socket_socket(protocol, type, 0);

    if (socket <= 0) {
        switch (errno) {
        case EPROTONOSUPPORT:
        case EINVAL:
            setError(Qt::UnsupportedSocketOperationError, "Protocol type not supported");
            break;
        case ENFILE:
        case EMFILE:
        case ENOBUFS:
        case ENOMEM:
            setError(Qt::SocketResourceError, "Out of resources");
            break;
        case EACCES:
            setError(Qt::SocketAccessError, "Permission denied");
            break;
        default:
            break;
        }

        return false;
    }

    socketDescriptor = socket;
    return true;
}


/*! \internal

    Returns the value of the socket option \a opt.
*/
int QSocketLayerPrivate::option(SocketOption opt) const
{
    if (!q->isValid())
        return -1;

    int n = -1;
    switch (opt) {
    case ReceiveBufferSocketOption:
        n = SO_RCVBUF;
        break;
    case SendBufferSocketOption:
        n = SO_SNDBUF;
        break;
    case NonBlockingSocketOption:
        break;
    case BroadcastSocketOption:
        break;
    }

    int v = -1;
    QT_SOCKOPTLEN_T len = sizeof(v);
    if (getsockopt(socketDescriptor, SOL_SOCKET, n, (char *) &v, &len) != -1)
        return v;
    return -1;
}


/*! \internal
    Sets the socket option \a opt to \a v.
*/
bool QSocketLayerPrivate::setOption(SocketOption opt, int v)
{
    if (!q->isValid())
        return false;

    int n = 0;
    switch (opt) {
    case ReceiveBufferSocketOption:
        n = SO_RCVBUF;
        break;
    case SendBufferSocketOption:
        n = SO_SNDBUF;
        break;
    case BroadcastSocketOption:
        n = SO_BROADCAST;
        break;
    case NonBlockingSocketOption:
        // Make the socket nonblocking.
        int flags = ::fcntl(socketDescriptor, F_GETFL, 0);
        return flags != -1 && ::fcntl(socketDescriptor, F_SETFL, flags | O_NONBLOCK) != -1;
    }

    return ::setsockopt(socketDescriptor, SOL_SOCKET, n, (char *) &v, sizeof(v)) == 0;
}

bool QSocketLayerPrivate::nativeConnect(const QHostAddress &addr, Q_UINT16 port)
{
    struct sockaddr_in sockAddrIPv4;
    struct sockaddr *sockAddrPtr;
    QT_SOCKLEN_T sockAddrSize;

#if !defined(QT_NO_IPV6)
    struct sockaddr_in6 sockAddrIPv6;

    if (addr.isIPv6Address()) {
        memset(&sockAddrIPv6, 0, sizeof(sockAddrIPv6));
        sockAddrIPv6.sin6_family = AF_INET6;
        sockAddrIPv6.sin6_port = htons(port);
        Q_IPV6ADDR ip6 = addr.toIPv6Address();
        memcpy(&sockAddrIPv6.sin6_addr.s6_addr, &ip6, sizeof(ip6));

        sockAddrSize = sizeof(sockAddrIPv6);
        sockAddrPtr = (struct sockaddr *) &sockAddrIPv6;
    } else
#if 0
    {}
#endif
#endif
    if (addr.isIPv4Address()) {
        memset(&sockAddrIPv4, 0, sizeof(sockAddrIPv4));
        sockAddrIPv4.sin_family = AF_INET;
        sockAddrIPv4.sin_port = htons(port);
        sockAddrIPv4.sin_addr.s_addr = htonl(addr.toIPv4Address());

        sockAddrSize = sizeof(sockAddrIPv4);
        sockAddrPtr = (struct sockaddr *) &sockAddrIPv4;
    } else {
        // unreachable
    }

    int connectResult = qt_socket_connect(socketDescriptor, sockAddrPtr, sockAddrSize);
    if (connectResult == -1) {
        switch (errno) {
        case EINVAL:
            setError(Qt::UnsupportedSocketOperationError, "Unsupported socket operation");
            break;
        case EISCONN:
            socketState = Qt::ConnectedState;
            break;
        case ECONNREFUSED:
            setError(Qt::ConnectionRefusedError, "Connection refused");
            break;
        case ETIMEDOUT:
            setError(Qt::NetworkError, "Connection timed out");
            break;
        case ENETUNREACH:
            setError(Qt::NetworkError, "Network unreachable");
            break;
        case EADDRINUSE:
            setError(Qt::NetworkError, "The bound address is already in use");
            break;
        case EINPROGRESS:
        case EALREADY:
            socketState = Qt::ConnectingState;
            break;
        case EAGAIN:
            setError(Qt::SocketResourceError, "Out of free local ports");
            break;
        case EACCES:
        case EPERM:
            setError(Qt::SocketAccessError, "Permission denied");
            break;
        case EAFNOSUPPORT:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
        default:
            break;
        }

        if (socketState != Qt::ConnectedState) {
#if defined (QSOCKETLAYER_DEBUG)
            qDebug("QSocketLayerPrivate::nativeConnect(%s, %i) == false (%s)",
                   addr.toString().latin1(), port,
                   socketState == Qt::ConnectingState
                   ? "Connection in progress" : socketErrorString.latin1());
#endif
            return false;
        }
    }

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeConnect(%s, %i) == true",
           addr.toString().latin1(), port);
#endif

    socketState = Qt::ConnectedState;
    return true;
}

bool QSocketLayerPrivate::nativeBind(const QHostAddress &address, Q_UINT16 port)
{
    struct sockaddr_in sockAddrIPv4;
    struct sockaddr *sockAddrPtr;
    QT_SOCKLEN_T sockAddrSize;

#if !defined(QT_NO_IPV6)
    struct sockaddr_in6 sockAddrIPv6;

    if (address.isIPv6Address()) {
        memset(&sockAddrIPv6, 0, sizeof(sockAddrIPv6));
        sockAddrIPv6.sin6_family = AF_INET6;
        sockAddrIPv6.sin6_port = htons(port);
        Q_IPV6ADDR tmp = address.toIPv6Address();
        memcpy(&sockAddrIPv6.sin6_addr.s6_addr, &tmp, sizeof(tmp));
        sockAddrSize = sizeof(sockAddrIPv6);
        sockAddrPtr = (struct sockaddr *) &sockAddrIPv6;
    } else
#endif
        if (address.isIPv4Address()) {
            memset(&sockAddrIPv4, 0, sizeof(sockAddrIPv4));
            sockAddrIPv4.sin_family = AF_INET;
            sockAddrIPv4.sin_port = htons(port);
            sockAddrIPv4.sin_addr.s_addr = htonl(address.toIPv4Address());
            sockAddrSize = sizeof(sockAddrIPv4);
            sockAddrPtr = (struct sockaddr *) &sockAddrIPv4;
        } else {
            // unreachable
        }

    int bindResult = qt_socket_bind(socketDescriptor, sockAddrPtr, sockAddrSize);
    if (bindResult < 0) {
        switch(errno) {
        case EADDRINUSE:
            setError(Qt::AddressInUseError, "The address is already bound");
            break;
        case EACCES:
            setError(Qt::SocketAccessError, "The address is protected");
            break;
        case EINVAL:
            setError(Qt::UnsupportedSocketOperationError, "Unsupported socket operation");
            break;
        default:
            break;
        }

#if defined (QSOCKETLAYER_DEBUG)
        qDebug("QSocketLayerPrivate::nativeBind(%s, %i) == false (%s)",
               address.toString().latin1(), port, socketErrorString.latin1());
#endif

        return false;
    }

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeBind(%s, %i) == true",
           address.toString().latin1(), port);
#endif
    socketState = Qt::BoundState;
    return true;
}

bool QSocketLayerPrivate::nativeListen(int backlog)
{
    if (qt_socket_listen(socketDescriptor, backlog) < 0) {
        switch (errno) {
        case EADDRINUSE:
            setError(Qt::AddressInUseError,
                     "Another socket is already listening on the same port");
            break;
        default:
            break;
        }

#if defined (QSOCKETLAYER_DEBUG)
        qDebug("QSocketLayerPrivate::nativeListen(%i) == false (%s)",
               backlog, socketErrorString.latin1());
#endif
        return false;
    }

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeListen(%i) == true", backlog);
#endif

    socketState = Qt::ListeningState;
    return true;
}

int QSocketLayerPrivate::nativeAccept()
{
    int acceptedDescriptor = qt_socket_accept(socketDescriptor, 0, 0);
#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeAccept() == %i", acceptedDescriptor);
#endif
    return acceptedDescriptor;
}

Q_LLONG QSocketLayerPrivate::nativeBytesAvailable() const
{
    /*
      Apparently, there is not consistency among different operating
      systems on how to use FIONREAD.

      FreeBSD, Linux and Solaris all expect the 3rd argument to
      ioctl() to be an int, which is normally 32-bit even on 64-bit
      machines.

      IRIX, on the other hand, expects a size_t, which is 64-bit on
      64-bit machines.

      So, the solution is to use size_t initialized to zero to make
      sure all bits are set to zero, preventing underflow with the
      FreeBSD/Linux/Solaris ioctls.
    */
    size_t nbytes = 0;
    // gives shorter than true amounts on Unix domain sockets.
    Q_LLONG available = 0;
    if (::ioctl(socketDescriptor, FIONREAD, (char *) &nbytes) >= 0)
        available = (Q_LLONG) *((int *) &nbytes);

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeBytesAvailable() == %lli", available);
#endif
    return available;
}

bool QSocketLayerPrivate::nativeHasPendingDatagram() const
{
    // Create a sockaddr struct and reset its port number.
#if !defined(QT_NO_IPV6)
    struct sockaddr_storage storage;
    sockaddr_in6 *storagePtrIPv6 = reinterpret_cast<sockaddr_in6 *>(&storage);
    storagePtrIPv6->sin6_port = 0;
#else
    struct sockaddr storage;
#endif
    sockaddr *storagePtr = reinterpret_cast<sockaddr *>(&storage);
    storagePtr->sa_family = 0;

    sockaddr_in *storagePtrIPv4 = reinterpret_cast<sockaddr_in *>(&storage);
    storagePtrIPv4->sin_port = 0;
    QT_SOCKLEN_T storageSize = sizeof(storage);

    // Peek 0 bytes into the next message. The size of the message may
    // well be 0, so we can't check recvfrom's return value.
    int readBytes;
    do {
        readBytes = ::recvfrom(socketDescriptor, 0, 0, MSG_PEEK, storagePtr, &storageSize);
    } while (readBytes == -1 && errno == EINTR);

    // If the port was set in the sockaddr structure, then a new message is available.
    bool result = false;
#if !defined(QT_NO_IPV6)
    if (storagePtr->sa_family == AF_INET6)
        result = (storagePtrIPv6->sin6_port != 0);
    else
#endif
    result = (storagePtrIPv4->sin_port != 0);

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeHasPendingDatagram() == %s",
           result ? "true" : "false");
#endif
    return result;
}

Q_LLONG QSocketLayerPrivate::nativePendingDatagramSize() const
{
    int recvResult;
    do {
        // the data written to udpMessagePeekBuffer is discarded, so
        // this function is still reentrant although it might not look
        // so.
        static char udpMessagePeekBuffer[8192];
        recvResult = ::recv(socketDescriptor, udpMessagePeekBuffer,
                            sizeof(udpMessagePeekBuffer), MSG_TRUNC | MSG_PEEK);
    } while (recvResult == -1 && errno == EINTR);

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativePendingDatagramSize() == %i", recvResult);
#endif

    return recvResult;
}

Q_LLONG QSocketLayerPrivate::nativeReceiveDatagram(char *data, Q_LLONG maxLength,
                                                    QHostAddress *address, Q_UINT16 *port)
{
#if !defined(QT_NO_IPV6)
    struct sockaddr_storage aa;
#else
    struct sockaddr_in aa;
#endif
    memset(&aa, 0, sizeof(aa));
    QT_SOCKLEN_T sz;
    sz = sizeof(aa);

    ssize_t recvFromResult = 0;
    do {
        recvFromResult = ::recvfrom(socketDescriptor, data, maxLength,
                                    0, (struct sockaddr *)&aa, &sz);
    } while (recvFromResult == -1 && errno == EINTR);

    if (recvFromResult == -1) {
        setError(Qt::NetworkError, "Unable to receive a message");
    } else if (port || address) {
        qt_socket_getPortAndAddress((struct sockaddr *) &aa, port, address);
    }

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeReceiveDatagram(%p \"%s\", %lli, %s, %i) == %lli",
           data, qt_prettyDebug(data, qMin(recvFromResult, 16), recvFromResult).data(), maxLength,
           address ? address->toString().latin1() : "(nil)",
           port ? *port : 0, (Q_LLONG) recvFromResult);
#endif

    return recvFromResult;
}

Q_LLONG QSocketLayerPrivate::nativeSendDatagram(const char *data, Q_LLONG len,
                                                 const QHostAddress &host, Q_UINT16 port)
{
    struct sockaddr_in sockAddrIPv4;
    struct sockaddr *sockAddrPtr;
    QT_SOCKLEN_T sockAddrSize;
#if !defined(QT_NO_IPV6)
    struct sockaddr_in6 sockAddrIPv6;
    if (host.isIPv6Address()) {
	memset(&sockAddrIPv6, 0, sizeof(sockAddrIPv6));
	sockAddrIPv6.sin6_family = AF_INET6;
	sockAddrIPv6.sin6_port = htons(port);

	Q_IPV6ADDR tmp = host.toIPv6Address();
	memcpy(&sockAddrIPv6.sin6_addr.s6_addr, &tmp, sizeof(tmp));
	sockAddrSize = sizeof(sockAddrIPv6);
	sockAddrPtr = (struct sockaddr *)&sockAddrIPv6;
    } else
#endif
    if (host.isIPv4Address()) {
	memset(&sockAddrIPv4, 0, sizeof(sockAddrIPv4));
	sockAddrIPv4.sin_family = AF_INET;
	sockAddrIPv4.sin_port = htons(port);
	sockAddrIPv4.sin_addr.s_addr = htonl(host.toIPv4Address());
	sockAddrSize = sizeof(sockAddrIPv4);
	sockAddrPtr = (struct sockaddr *)&sockAddrIPv4;
    }

    ssize_t sentBytes;
    do {
        sentBytes = ::sendto(socketDescriptor, data, len,
                             MSG_NOSIGNAL, sockAddrPtr, sockAddrSize);
    } while (sentBytes == -1 && errno == EINTR);

    if (sentBytes < 0)
        setError(Qt::NetworkError, "Unable to receive a message");

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayer::sendDatagram(%p \"%s\", %lli, \"%s\", %i) == %lli", data,
           qt_prettyDebug(data, qMin(len, 16), len).data(), len, host.toString().latin1(),
           port, (Q_LLONG) sentBytes);
#endif

    return sentBytes;
}

bool QSocketLayerPrivate::fetchConnectionParameters()
{
    localPort = 0;
    localAddress.clear();
    peerPort = 0;
    peerAddress.clear();

    if (socketDescriptor == -1)
        return false;

#if !defined(QT_NO_IPV6)
    struct sockaddr_storage sa;
#else
    struct sockaddr_in sa;
#endif
    struct sockaddr *sockAddrPtr = (struct sockaddr *) &sa;
    QT_SOCKLEN_T sockAddrSize = sizeof(sa);

    // Determine local address
    memset(&sa, 0, sizeof(sa));
    if (::getsockname(socketDescriptor, sockAddrPtr, &sockAddrSize) == 0) {
        qt_socket_getPortAndAddress(sockAddrPtr, &localPort, &localAddress);

        // Determine protocol family
        switch (sockAddrPtr->sa_family) {
        case AF_INET:
            socketProtocol = Qt::IPv4Protocol;
            break;
#if !defined (QT_NO_IPV6)
        case AF_INET6:
            socketProtocol = Qt::IPv6Protocol;
            break;
#endif
        default:
            socketProtocol = Qt::UnknownNetworkLayerProtocol;
            break;
        }

    } else if (errno == EBADF) {
        setError(Qt::UnsupportedSocketOperationError, "Invalid socket descriptor");
        return false;
    }

    // Determine the remote address
    if (!::getpeername(socketDescriptor, sockAddrPtr, &sockAddrSize))
        qt_socket_getPortAndAddress(sockAddrPtr, &peerPort, &peerAddress);

    // Determine the socket type (UDP/TCP)
    int value = 0;
    QT_SOCKLEN_T valueSize;
    if (::getsockopt(socketDescriptor, SOL_SOCKET, SO_TYPE, &value, &valueSize) == 0) {
        if (value == SOCK_STREAM)
            socketType = Qt::TcpSocket;
        else if (value == SOCK_DGRAM)
            socketType = Qt::UdpSocket;
        else
            socketType = Qt::UnknownSocketType;
    }
#if defined (QSOCKETLAYER_DEBUG)
    QString socketProtocolStr = "UnknownProtocol";
    if (socketProtocol == Qt::IPv4Protocol) socketProtocolStr = "IPv4Protocol";
    else if (socketProtocol == Qt::IPv6Protocol) socketProtocolStr = "IPv6Protocol";

    QString socketTypeStr = "UnknownSocketType";
    if (socketType == Qt::TcpSocket) socketTypeStr = "TcpSocket";
    else if (socketType == Qt::UdpSocket) socketTypeStr = "UdpSocket";

    qDebug("QSocketLayerPrivate::fetchConnectionParameters() localAddress == %s,"
           " localPort = %i, peerAddress == %s, peerPort = %i, socketProtocol == %s,"
           " socketType == %s", localAddress.toString().latin1(), localPort,
           peerAddress.toString().latin1(), peerPort, socketProtocolStr.latin1(),
           socketTypeStr.latin1());
#endif
    return true;
}

void QSocketLayerPrivate::nativeClose()
{
#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayer::nativeClose()");
#endif
    ::close(socketDescriptor);
}

Q_LLONG QSocketLayerPrivate::nativeWrite(const char *data, Q_LLONG len)
{
    // ignore the SIGPIPE signal
    qt_ignore_sigpipe();

    // loop while ::write() returns -1 and errno == EINTR, in case
    // of an interrupting signal.
    ssize_t writtenBytes;
    do {
        writtenBytes = ::write(socketDescriptor, data, len);
    } while (writtenBytes < 0 && errno == EINTR);

    if (writtenBytes < 0) {
        switch (errno) {
        case EPIPE:
        case ECONNRESET:
            writtenBytes = -1;
            setError(Qt::RemoteHostClosedError, "Remote host closed");
            q->close();
            break;
        case EAGAIN:
            writtenBytes = 0;
            break;
        case EMSGSIZE:
            setError(Qt::DatagramTooLargeError, "Datagram too large");
            break;
        default:
            break;
        }
    }

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeWrite(%p \"%s\", %llu) == %i",
           data, qt_prettyDebug(data, qMin((int) writtenBytes, 16),
                                (int) writtenBytes).data(), len, (int) writtenBytes);
#endif

    return (Q_LLONG) writtenBytes;
}
/*
*/
Q_LLONG QSocketLayerPrivate::nativeRead(char *data, Q_LLONG maxLength)
{
    if (!q->isValid()) {
        qWarning("QSocketLayer::unbufferedRead: Invalid socket");
        return -1;
    }

    int r = 0;
    do {
        r = ::read(socketDescriptor, data, maxLength);
    } while (r == -1 && errno == EINTR);

    if (r < 0) {
        switch (errno) {
        case EAGAIN:
            // No data was available for reading
            return 0;
        case EBADF:
        case EINVAL:
        case EIO:
            setError(Qt::NetworkError, "Network error");
            break;
        default:
            break;
        }

        r = -1;
    }

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeRead(%p \"%s\", %llu) == %i",
           data, qt_prettyDebug(data, qMin(r, 16), r).data(), maxLength, r);
#endif

    return r;
}

int QSocketLayerPrivate::nativeSelect(int timeout, bool selectForRead) const
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(socketDescriptor, &fds);

    struct timeval tv;
    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    if (selectForRead)
        return select(socketDescriptor + 1, &fds, 0, 0, timeout < 0 ? 0 : &tv);
    else
        return select(socketDescriptor + 1, 0, &fds, 0, timeout < 0 ? 0 : &tv);
}
