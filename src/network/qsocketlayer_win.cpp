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

#include <winsock2.h>

#include "qsocketlayer.h"
#include "qsocketlayer_p.h"

//#define QSOCKETLAYER_DEBUG

#if defined(QSOCKETLAYER_DEBUG)

#include <qstring.h>
#include <qbytearray.h>

void verboseWSErrorDebug(int r)
{
    switch (r) {
        case WSANOTINITIALISED : qDebug("WSA error : WSANOTINITIALISED"); break;
	case WSAEINTR: qDebug("WSA error : WSAEINTR"); break;
	case WSAEBADF: qDebug("WSA error : WSAEBADF"); break;
	case WSAEACCES: qDebug("WSA error : WSAEACCES"); break;
	case WSAEFAULT: qDebug("WSA error : WSAEFAULT"); break;
	case WSAEINVAL: qDebug("WSA error : WSAEINVAL"); break;
	case WSAEMFILE: qDebug("WSA error : WSAEMFILE"); break;
	case WSAEWOULDBLOCK: qDebug("WSA error : WSAEWOULDBLOCK"); break;
	case WSAEINPROGRESS: qDebug("WSA error : WSAEINPROGRESS"); break;
	case WSAEALREADY: qDebug("WSA error : WSAEALREADY"); break;
	case WSAENOTSOCK: qDebug("WSA error : WSAENOTSOCK"); break;
	case WSAEDESTADDRREQ: qDebug("WSA error : WSAEDESTADDRREQ"); break;
	case WSAEMSGSIZE: qDebug("WSA error : WSAEMSGSIZE"); break;
	case WSAEPROTOTYPE: qDebug("WSA error : WSAEPROTOTYPE"); break;
	case WSAENOPROTOOPT: qDebug("WSA error : WSAENOPROTOOPT"); break;
	case WSAEPROTONOSUPPORT: qDebug("WSA error : WSAEPROTONOSUPPORT"); break;
	case WSAESOCKTNOSUPPORT: qDebug("WSA error : WSAESOCKTNOSUPPORT"); break;
	case WSAEOPNOTSUPP: qDebug("WSA error : WSAEOPNOTSUPP"); break;
	case WSAEPFNOSUPPORT: qDebug("WSA error : WSAEPFNOSUPPORT"); break;
	case WSAEAFNOSUPPORT: qDebug("WSA error : WSAEAFNOSUPPORT"); break;
	case WSAEADDRINUSE: qDebug("WSA error : WSAEADDRINUSE"); break;
	case WSAEADDRNOTAVAIL: qDebug("WSA error : WSAEADDRNOTAVAIL"); break;
	case WSAENETDOWN: qDebug("WSA error : WSAENETDOWN"); break;
	case WSAENETUNREACH: qDebug("WSA error : WSAENETUNREACH"); break;
	case WSAENETRESET: qDebug("WSA error : WSAENETRESET"); break;
	case WSAECONNABORTED: qDebug("WSA error : WSAECONNABORTED"); break;
	case WSAECONNRESET: qDebug("WSA error : WSAECONNRESET"); break;
	case WSAENOBUFS: qDebug("WSA error : WSAENOBUFS"); break;
	case WSAEISCONN: qDebug("WSA error : WSAEISCONN"); break;
	case WSAENOTCONN: qDebug("WSA error : WSAENOTCONN"); break;
	case WSAESHUTDOWN: qDebug("WSA error : WSAESHUTDOWN"); break;
	case WSAETOOMANYREFS: qDebug("WSA error : WSAETOOMANYREFS"); break;
	case WSAETIMEDOUT: qDebug("WSA error : WSAETIMEDOUT"); break;
	case WSAECONNREFUSED: qDebug("WSA error : WSAECONNREFUSED"); break;
	case WSAELOOP: qDebug("WSA error : WSAELOOP"); break;
	case WSAENAMETOOLONG: qDebug("WSA error : WSAENAMETOOLONG"); break;
	case WSAEHOSTDOWN: qDebug("WSA error : WSAEHOSTDOWN"); break;
	case WSAEHOSTUNREACH: qDebug("WSA error : WSAEHOSTUNREACH"); break;
	case WSAENOTEMPTY: qDebug("WSA error : WSAENOTEMPTY"); break;
	case WSAEPROCLIM: qDebug("WSA error : WSAEPROCLIM"); break;
	case WSAEUSERS: qDebug("WSA error : WSAEUSERS"); break;
	case WSAEDQUOT: qDebug("WSA error : WSAEDQUOT"); break;
	case WSAESTALE: qDebug("WSA error : WSAESTALE"); break;
	case WSAEREMOTE: qDebug("WSA error : WSAEREMOTE"); break;
	case WSAEDISCON: qDebug("WSA error : WSAEDISCON"); break;
	default: qDebug("WSA error : Unknown"); break;
    }
}

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
            out += tmp.latin1();
        }
    }

    if (len < maxLength)
        out += "...";

    return out;
}


#define WS_ERROR_DEBUG verboseWSErrorDebug(WSAGetLastError());

#else

#define WS_ERROR_DEBUG

#endif

#if !defined (QT_NO_IPV6)

// Use our own defines and structs which we know are correct
#  define QT_SS_MAXSIZE 128
#  define QT_SS_ALIGNSIZE (sizeof(__int64))
#  define QT_SS_PAD1SIZE (QT_SS_ALIGNSIZE - sizeof (short))
#  define QT_SS_PAD2SIZE (QT_SS_MAXSIZE - (sizeof (short) + QT_SS_PAD1SIZE + QT_SS_ALIGNSIZE))
struct qt_sockaddr_storage {
      short ss_family;
      char __ss_pad1[QT_SS_PAD1SIZE];
      __int64 __ss_align;
      char __ss_pad2[QT_SS_PAD2SIZE];
};

// sockaddr_in6 size changed between old and new SDK
// Only the new version is the correct one, so always
// use this structure.
struct qt_in6_addr {
    u_char qt_s6_addr[16];
};
typedef struct {
    short   sin6_family;            /* AF_INET6 */
    u_short sin6_port;              /* Transport level port number */
    u_long  sin6_flowinfo;          /* IPv6 flow information */
    struct  qt_in6_addr sin6_addr;  /* IPv6 address */
    u_long  sin6_scope_id;          /* set of interfaces for a scope */
} qt_sockaddr_in6;

#else

typedef void * qt_sockaddr_in6 ;


#endif

#ifndef AF_INET6
#define AF_INET6        23              /* Internetwork Version 6 */
#endif

#ifndef SO_EXCLUSIVEADDRUSE
#define SO_EXCLUSIVEADDRUSE ((int)(~SO_REUSEADDR)) /* disallow local address reuse */
#endif

//###
#define QT_SOCKLEN_T int
#define QT_SOCKOPTLEN_T int


/*
    Extracts the port and address from a sockaddr, and stores them in
    \a port and \a addr if they are non-null.
*/
static inline void qt_socket_getPortAndAddress(SOCKET socketDescriptor, struct sockaddr *sa, Q_UINT16 *port, QHostAddress *address)
{
#if !defined (QT_NO_IPV6)
    if (sa->sa_family == AF_INET6) {
        qt_sockaddr_in6 *sa6 = (qt_sockaddr_in6 *)sa;
        Q_IPV6ADDR tmp;
        for (int i = 0; i < 16; ++i)
            tmp.c[i] = sa6->sin6_addr.qt_s6_addr[i];
        QHostAddress a;
	a.setAddress(tmp);
	if (address)
	    *address = a;
        if (port)
	    WSANtohs(socketDescriptor, sa6->sin6_port, port);
    } else
#endif
    if (sa->sa_family == AF_INET) {
        struct sockaddr_in *sa4 = (struct sockaddr_in *)sa;
        unsigned long addr;
        WSANtohl(socketDescriptor, sa4->sin_addr.s_addr, &addr);
        QHostAddress a;
	a.setAddress(addr);
	if (address)
	    *address = a;
        if (port)
	    WSANtohs(socketDescriptor, sa4->sin_port, port);
    }
}


/*! \internal

    Sets the port and address to a sockaddr. Requires that sa point to the IPv6 struct if the address is IPv6.
*/
static inline void qt_socket_setPortAndAddress(SOCKET socketDescriptor, sockaddr_in * sockAddrIPv4, qt_sockaddr_in6 * sockAddrIPv6,
                                               Q_UINT16 port, const QHostAddress & address, sockaddr ** sockAddrPtr, QT_SOCKLEN_T *sockAddrSize)
{
#if !defined(QT_NO_IPV6)
    if (address.isIPv6Address()) {
        memset(sockAddrIPv6, 0, sizeof(qt_sockaddr_in6));
        sockAddrIPv6->sin6_family = AF_INET6;
        WSAHtons(socketDescriptor, port, &(sockAddrIPv6->sin6_port));
        Q_IPV6ADDR tmp = address.toIPv6Address();
        memcpy(&(sockAddrIPv6->sin6_addr.qt_s6_addr), &tmp, sizeof(tmp));
        *sockAddrSize = sizeof(qt_sockaddr_in6);
        *sockAddrPtr = (struct sockaddr *) sockAddrIPv6;
    } else
#endif
    if (address.isIPv4Address()) {
        memset(sockAddrIPv4, 0, sizeof(sockaddr_in));
        sockAddrIPv4->sin_family = AF_INET;
        WSAHtons(socketDescriptor, port, &(sockAddrIPv4->sin_port));
        WSAHtonl(socketDescriptor, address.toIPv4Address(), &(sockAddrIPv4->sin_addr.s_addr));
        *sockAddrSize = sizeof(sockaddr_in);
        *sockAddrPtr = (struct sockaddr *) sockAddrIPv4;
    } else {
        // unreachable
    }
}


/*! \internal

*/
static inline Qt::NetworkLayerProtocol qt_socket_getProtocol(int socketDescriptor)
{

    if (socketDescriptor != -1) {
	WSAPROTOCOL_INFOW info;
	memset(&info, 0, sizeof(info));

	QT_SOCKLEN_T valueSize = sizeof(info);

	if (::getsockopt(socketDescriptor, SOL_SOCKET, SO_PROTOCOL_INFO, (char *) &info, &valueSize) != 0) {
	    WS_ERROR_DEBUG
	} else {
	    switch (info.iAddressFamily) {
                case AF_INET:
                    return Qt::IPv4Protocol;
                case AF_INET6:
                    return Qt::IPv6Protocol;
                default:
                    return Qt::UnknownNetworkLayerProtocol;
            }

	}
    }

    return Qt::UnknownNetworkLayerProtocol;
}


/*! \internal

*/
static inline Qt::SocketType qt_socket_getType(int socketDescriptor)
{
    int value = 0;
    QT_SOCKLEN_T valueSize = sizeof(value);
    if (::getsockopt(socketDescriptor, SOL_SOCKET, SO_TYPE, (char *) &value, &valueSize) != 0) {
	WS_ERROR_DEBUG
    } else {
        if (value == SOCK_STREAM)
            return Qt::TcpSocket;
        else if (value == SOCK_DGRAM)
            return Qt::UdpSocket;
    }
    return Qt::UnknownSocketType;
}


QWindowsSockInit::QWindowsSockInit()
:   version(0)
{
    //### should we try for 2.2 on all platforms ??
    WSAData wsadata;

    // IPv6 requires Winsock v2.0 or better.
    if (WSAStartup(MAKEWORD(2,0), &wsadata) != 0) {
	qWarning("QTcpSocketAPI: WinSock v2.0 initialization failed.");
    } else {
        version = 0x20;
    }
}

QWindowsSockInit::~QWindowsSockInit()
{
    WSACleanup();
}

bool QSocketLayerPrivate::createNewSocket(Qt::SocketType socketType, Qt::NetworkLayerProtocol socketProtocol)
{

    //### no ip6 support on winsocket 1.1 but we will try not to use this !!!!!!!!!!!!1
    /*
    if (winsockVersion < 0x20 && socketProtocol == Qt::IPv6Protocol) {
        //### no ip6 support
        return -1;
    }
    */

    int protocol = (socketProtocol == Qt::IPv6Protocol) ? AF_INET6 : AF_INET;
    int type = (socketType == Qt::UdpSocket) ? SOCK_DGRAM : SOCK_STREAM;
    SOCKET socket = ::WSASocket(protocol, type, 0, NULL, 0, 0);

    if (socket == INVALID_SOCKET) {
        WS_ERROR_DEBUG
        switch (WSAGetLastError()) {
        case WSANOTINITIALISED:
            //###
            break;
        case WSAEAFNOSUPPORT:
        case WSAESOCKTNOSUPPORT:
        case WSAEPROTOTYPE:
        case WSAEINVAL:
            setError(Qt::UnsupportedSocketOperationError, "Protocol type not supported");
            break;
        case WSAEMFILE:
        case WSAENOBUFS:
            setError(Qt::SocketResourceError, "Out of resources");
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
    case BroadcastSocketOption:
        n = SO_BROADCAST;
        break;
    case NonBlockingSocketOption:
        unsigned long buf = 0;
        if (WSAIoctl(socketDescriptor, FIONBIO, 0,0, &buf, sizeof(buf), 0,0,0) == 0)
            return buf;
        else
            return -1;
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
        unsigned long buf = v;
        unsigned long outBuf;
        DWORD sizeWritten = 0;
        if (::WSAIoctl(socketDescriptor, FIONBIO, &buf, sizeof(unsigned long), &outBuf, sizeof(unsigned long), &sizeWritten, 0,0) == SOCKET_ERROR) {
            WS_ERROR_DEBUG
            return false;
        }
        return true;
        break;
    }

    if (::setsockopt(socketDescriptor, SOL_SOCKET, n, (char*)&v, sizeof(v)) != 0) {
        WS_ERROR_DEBUG
        return false;
    }
    return true;
}

/*!
    Fetches information about both ends of the connection: whatever is
    available.
*/
bool QSocketLayerPrivate::fetchConnectionParameters()
{
    localPort = 0;
    localAddress.clear();
    peerPort = 0;
    peerAddress.clear();

    if (socketDescriptor == -1)
       return false;

#if !defined (QT_NO_IPV6)
    struct qt_sockaddr_storage sa;
#else
    struct sockaddr_in sa;
#endif
    struct sockaddr *pSa = (struct sockaddr *) &sa;

    QT_SOCKLEN_T sz = sizeof(sa);

    memset(&sa, 0, sizeof(sa));
    if (::getsockname(socketDescriptor, pSa, &sz) == 0) {
        qt_socket_getPortAndAddress(socketDescriptor, pSa, &localPort, &localAddress);
    } else {
	WS_ERROR_DEBUG
	if (WSAGetLastError() == WSAENOTSOCK) {
	    setError(Qt::UnsupportedSocketOperationError, "Invalid socket descriptor");
            return false;
	}
    }

    memset(&sa, 0, sizeof(sa));
    if (::getpeername(socketDescriptor, pSa, &sz) == 0) {
        qt_socket_getPortAndAddress(socketDescriptor, pSa, &peerPort, &peerAddress);
    } else {
	WS_ERROR_DEBUG
    }

    socketProtocol = qt_socket_getProtocol(socketDescriptor);

    socketType = qt_socket_getType(socketDescriptor);

#if defined (QSOCKETLAYER_DEBUG)
    QString socketProtocolStr = "UnknownProtocol";
    if (socketProtocol == Qt::IPv4Protocol) socketProtocolStr = "IPv4Protocol";
    else if (socketProtocol == Qt::IPv6Protocol) socketProtocolStr = "IPv6Protocol";

    QString socketTypeStr = "UnknownSocketType";
    if (socketType == Qt::TcpSocket) socketTypeStr = "TcpSocket";
    else if (socketType == Qt::UdpSocket) socketTypeStr = "UdpSocket";

    qDebug("QSocketLayerPrivate::fetchConnectionParameters() localAddress == %s, localPort = %i, peerAddress == %s, peerPort = %i, socketProtocol == %s, socketType == %s", localAddress.toString().latin1(), localPort, peerAddress.toString().latin1(), peerPort, socketProtocolStr.latin1(), socketTypeStr.latin1());
#endif

    return true;
}


bool QSocketLayerPrivate::nativeConnect(const QHostAddress &address, Q_UINT16 port)
{
    struct sockaddr_in sockAddrIPv4;
    qt_sockaddr_in6 sockAddrIPv6;
    struct sockaddr *sockAddrPtr;
    QT_SOCKLEN_T sockAddrSize;

    qt_socket_setPortAndAddress(socketDescriptor, &sockAddrIPv4, &sockAddrIPv6, port, address, &sockAddrPtr, &sockAddrSize);

    int connectResult = ::WSAConnect(socketDescriptor, sockAddrPtr, sockAddrSize, 0,0,0,0);
    if (connectResult == SOCKET_ERROR) {
        WS_ERROR_DEBUG
        switch (WSAGetLastError()) {
        case WSANOTINITIALISED:
            //###
            break;
        case WSAEINVAL: //### this should not be needed but untill all of Qt uses only ws2_32.lib it should be there
            if (socketState != Qt::ConnectingState)
                break;
        case WSAEISCONN:
            socketState = Qt::ConnectedState;
            break;
        case WSAEINPROGRESS:
        case WSAEALREADY:
        case WSAEWOULDBLOCK:
            socketState = Qt::ConnectingState;
            break;
        case WSAEADDRINUSE:
            setError(Qt::NetworkError, "The bound address is already in use");
            break;
        case WSAECONNREFUSED:
            setError(Qt::ConnectionRefusedError, "Connection refused");
            break;
        case WSAETIMEDOUT:
            setError(Qt::NetworkError, "Connection timed out");
            break;
        case WSAEACCES:
            setError(Qt::SocketAccessError, "Permission denied");
            break;
        case WSAENETUNREACH:
            setError(Qt::NetworkError, "Network unreachable");
            break;
        default:
            break;
        }

        if (socketState != Qt::ConnectedState) {
#if defined (QSOCKETLAYER_DEBUG)
            qDebug("QSocketLayerPrivate::nativeConnect(%s, %i) == false (%s)",
                   address.toString().latin1(), port,
                   socketState == Qt::ConnectingState
                   ? "Connection in progress" : socketErrorString.latin1());
#endif
            return false;
        }
    }

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeConnect(%s, %i) == true",
           address.toString().latin1(), port);
#endif

    socketState = Qt::ConnectedState;
    return true;
}


bool QSocketLayerPrivate::nativeBind(const QHostAddress &address, Q_UINT16 port)
{
    struct sockaddr_in sockAddrIPv4;
    qt_sockaddr_in6 sockAddrIPv6;
    struct sockaddr *sockAddrPtr;
    QT_SOCKLEN_T sockAddrSize;

    qt_socket_setPortAndAddress(socketDescriptor, &sockAddrIPv4, &sockAddrIPv6, port, address, &sockAddrPtr, &sockAddrSize);


    int bindResult = ::bind(socketDescriptor, sockAddrPtr, sockAddrSize);
    if (bindResult == SOCKET_ERROR) {
        WS_ERROR_DEBUG
        switch (WSAGetLastError()) {
        case WSANOTINITIALISED:
            //###
            break;
        case WSAEADDRINUSE:
        case WSAEINVAL:
            setError(Qt::AddressInUseError, "The address is already bound");
            break;
        case WSAEACCES:
            setError(Qt::SocketAccessError, "The address is protected");
            break;
        case WSAEADDRNOTAVAIL:
            setError(Qt::SocketAddressNotAvailableError, "The address is not available");
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
    if (::listen(socketDescriptor, backlog) == SOCKET_ERROR) {
        WS_ERROR_DEBUG
        switch (WSAGetLastError()) {
        case WSANOTINITIALISED:
            //###
            break;
        case WSAEADDRINUSE:
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
    int acceptedDescriptor = WSAAccept(socketDescriptor, 0,0,0,0);
#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeAccept() == %i", acceptedDescriptor);
#endif
    return acceptedDescriptor;
}


Q_LLONG QSocketLayerPrivate::nativeBytesAvailable() const
{
    unsigned long  nbytes = 0;
    unsigned long dummy = 0;
    DWORD sizeWritten = 0;
    if (::WSAIoctl(socketDescriptor, FIONREAD, &dummy, sizeof(dummy), &nbytes, sizeof(nbytes), &sizeWritten, 0,0) == SOCKET_ERROR) {
        WS_ERROR_DEBUG
        return -1;
    }

    // ioctlsocket sometimes reports 1 byte available for datagrams
    // while the following recvfrom returns -1 and claims connection
    // was reset (udp is connectionless). so we peek one byte to
    // catch this case and return 0 bytes available if recvfrom
    // fails.
    if (nbytes == 1 && socketType == Qt::UdpSocket) {
        char c;
        WSABUF buf;
        buf.buf = &c;
        buf.len = sizeof(c);
        DWORD flags = MSG_PEEK;
        if (::WSARecvFrom(socketDescriptor, &buf, 1, 0, &flags, 0,0,0,0) == SOCKET_ERROR)
            return 0;
    }
    return nbytes;
}


bool QSocketLayerPrivate::nativeHasPendingDatagrams() const
{
    // Create a sockaddr struct and reset its port number.
#if !defined(QT_NO_IPV6)
    qt_sockaddr_in6 storage;
    qt_sockaddr_in6 *storagePtrIPv6 = reinterpret_cast<qt_sockaddr_in6 *>(&storage);
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
    DWORD flags = MSG_PEEK;
    ::WSARecvFrom(socketDescriptor, 0,0,0, &flags, storagePtr, &storageSize,0,0);

    // If the port was set in the sockaddr structure, then a new message is available.
    bool result = false;
#if !defined(QT_NO_IPV6)
    if (storagePtr->sa_family == AF_INET6)
        result = (storagePtrIPv6->sin6_port != 0);
    else
#endif
    result = (storagePtrIPv4->sin_port != 0);

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeHasPendingDatagrams() == %s",
           result ? "true" : "false");
#endif
    return result;
}


Q_LLONG QSocketLayerPrivate::nativePendingDatagramSize() const
{
    int recvResult = 0;
    Q_LONG msgSize = 0;
    DWORD flags;
    do {
        // the data written to udpMessagePeekBuffer is discarded, so
        // this function is still reentrant although it might not look
        // so.
        static char udpMessagePeekBuffer[8192];

        WSABUF buf;
        buf.buf = udpMessagePeekBuffer;
        buf.len = sizeof(udpMessagePeekBuffer);
        flags = MSG_PEEK;
        DWORD bytesRead = 0;
        recvResult = ::WSARecvFrom(socketDescriptor, &buf, 1, &bytesRead, &flags, 0,0,0,0);
        msgSize += bytesRead;
    } while (recvResult != SOCKET_ERROR && flags & MSG_PARTIAL);

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativePendingDatagramSize() == %i", msgSize);
#endif

    return msgSize;
}


Q_LLONG QSocketLayerPrivate::nativeReceiveDatagram(char *data, Q_LLONG maxLength,
                                                   QHostAddress *address, Q_UINT16 *port)
{
#if !defined(QT_NO_IPV6)
    qt_sockaddr_storage aa;
#else
    struct sockaddr_in aa;
#endif
    memset(&aa, 0, sizeof(aa));
    QT_SOCKLEN_T sz;
    sz = sizeof(aa);
    WSABUF buf;
    buf.buf = data;
    buf.len = maxLength;
    DWORD flags = 0;
    DWORD bytesRead = 0;
    if (::WSARecvFrom(socketDescriptor, &buf, 1, &bytesRead, &flags, (struct sockaddr *) &aa, &sz,0,0) == SOCKET_ERROR) {
        WS_ERROR_DEBUG
            setError(Qt::NetworkError, "Unable to receive a message");
    }

    qt_socket_getPortAndAddress(socketDescriptor, (struct sockaddr *) &aa, port, address);

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeReceiveDatagram(%p \"%s\", %li, %s, %i) == %li",
           data, qt_prettyDebug(data, qMin((Q_LONG)bytesRead, 16), bytesRead).data(), maxLength,
           address ? address->toString().latin1() : "(nil)",
           port ? *port : 0, (Q_LONG) bytesRead);
#endif

    return bytesRead;
}


Q_LLONG QSocketLayerPrivate::nativeSendDatagram(const char *data, Q_LLONG len,
                                                 const QHostAddress &address, Q_UINT16 port)
{
    struct sockaddr_in sockAddrIPv4;
    qt_sockaddr_in6 sockAddrIPv6;
    struct sockaddr *sockAddrPtr;
    QT_SOCKLEN_T sockAddrSize;

    qt_socket_setPortAndAddress(socketDescriptor, &sockAddrIPv4, &sockAddrIPv6, port, address, &sockAddrPtr, &sockAddrSize);

    WSABUF buf;
    buf.buf = (char*)data;
    buf.len = len;
    DWORD flags = 0;
    DWORD bytesSent = 0;
    if (::WSASendTo(socketDescriptor, &buf, 1, &bytesSent, flags, sockAddrPtr, sockAddrSize, 0,0) ==  SOCKET_ERROR) {
        WS_ERROR_DEBUG
        bytesSent = -1;
        setError(Qt::NetworkError, "Unable to send a message");
    }

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeSendDatagram(%p \"%s\", %lu, \"%s\", %i) == %li", data,
           qt_prettyDebug(data, qMin(len, 16), len).data(), len, address.toString().latin1(),
           port, (Q_LONG) bytesSent);
#endif

    return bytesSent;
}


Q_LLONG QSocketLayerPrivate::nativeWrite(const char *data, Q_LLONG len)
{
    Q_LLONG ret = -1;
    WSABUF buf;
    buf.buf = (char*)data;
    buf.len = len;
    DWORD flags = 0;
    DWORD bytesWritten = 0;
    if (::WSASend(socketDescriptor, &buf, 1, &bytesWritten, flags, 0,0) ==  SOCKET_ERROR) {
        WS_ERROR_DEBUG
        switch (WSAGetLastError()) {
        case WSAECONNRESET:
        case WSAECONNABORTED:
            ret = -1;
            setError(Qt::NetworkError, "Unable to write");
            q->close();
            break;
        default:
            break;
        }
    } else {
        ret = Q_LLONG(bytesWritten);
    }

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeWrite(%p \"%s\", %lu) == %i",
           data, qt_prettyDebug(data, qMin((int) bytesWritten, 16),
                                (int) bytesWritten).data(), len, (int) ret);
#endif

    return ret;
}


Q_LLONG QSocketLayerPrivate::nativeRead(char *data, Q_LLONG maxLength)
{
    Q_LLONG ret = -1;
    WSABUF buf;
    buf.buf = data;
    buf.len = maxLength;
    DWORD flags = 0;
    DWORD bytesRead = 0;
    if (::WSARecv(socketDescriptor, &buf, 1, &bytesRead, &flags, 0,0) ==  SOCKET_ERROR) {
        WS_ERROR_DEBUG
        switch (WSAGetLastError()) {
        case WSAEBADF:
        case WSAEINVAL:
            setError(Qt::NetworkError, "Network error");
            break;
        default:
            break;
        }

    } else {
	ret = Q_LLONG(bytesRead);
    }

#if defined (QSOCKETLAYER_DEBUG)
    qDebug("QSocketLayerPrivate::nativeRead(%p \"%s\", %lu) == %i",
           data, qt_prettyDebug(data, qMin((int)bytesRead, 16), bytesRead).data(), maxLength, bytesRead);
#endif

    return ret;
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
        return select(0, &fds, 0, 0, timeout < 0 ? 0 : &tv);
    else
        return select(0, 0, &fds, 0, timeout < 0 ? 0 : &tv);
}


void QSocketLayerPrivate::nativeClose()
{
#if defined (QTCPSOCKETENGINE_DEBUG)
    qDebug("Qt::nativeClose()");
#endif
    ::closesocket(socketDescriptor);
}


