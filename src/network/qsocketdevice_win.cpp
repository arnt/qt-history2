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

#include "qcoreapplication.h"
#include "qsocketdevice.h"
#include "qsocketdevice_p.h"
#include "qdatetime.h"

#include <string.h>

#if defined (QT_NO_IPV6)
#  include <windows.h>
#  include <winsock.h>
#else
#  if defined (Q_CC_BOR) || defined (Q_CC_GNU)
#    include <winsock2.h>
#  elif defined (Q_CC_INTEL)
#    include <winsock.h>
#  else
#    include <windows.h>
#  endif
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
#endif

#ifndef AF_INET6
#define AF_INET6        23              /* Internetwork Version 6 */
#endif

#ifndef NO_ERRNO_H
#include <errno.h>
#endif


#if defined(SOCKLEN_T)
#undef SOCKLEN_T
#endif

#define SOCKLEN_T int // Winsock 1.1

#define d d_func()
#define q q_func()

static int winsockVersion = 0x00;

static void cleanupWinSock() // post-routine
{
    WSACleanup();
    winsockVersion = 0x00;
}

static inline void qt_socket_getportaddr(struct sockaddr *sa, Q_UINT16 *port, QHostAddress *addr)
{
#if !defined (QT_NO_IPV6)
    if (sa->sa_family == AF_INET6) {
        qt_sockaddr_in6 *sa6 = (qt_sockaddr_in6 *)sa;
        Q_IPV6ADDR tmp;
        for (int i = 0; i < 16; ++i)
            tmp.c[i] = sa6->sin6_addr.qt_s6_addr[i];
        QHostAddress a(tmp);
        *addr = a;
        *port = ntohs(sa6->sin6_port);
        return;
    }
#endif
    struct sockaddr_in *sa4 = (struct sockaddr_in *)sa;
    *port = ntohs(sa4->sin_port);
    *addr = QHostAddress(ntohl(sa4->sin_addr.s_addr));
}

void QSocketDevicePrivate::init()
{
#if !defined(QT_NO_IPV6)
    if (!winsockVersion) {
        WSAData wsadata;
        // IPv6 requires Winsock v2.0 or better.
        if (WSAStartup(MAKEWORD(2,0), &wsadata) != 0) {
#  if defined(QSOCKETDEVICE_DEBUG)
            qDebug("QSocketDevice: WinSock v2.0 initialization failed, disabling IPv6 support.");
#  endif
        } else {
            qAddPostRoutine(cleanupWinSock);
            winsockVersion = 0x20;
            return;
        }
    }
#endif

    if (!winsockVersion) {
        WSAData wsadata;
        if (WSAStartup(MAKEWORD(1,1), &wsadata) != 0) {
#if defined(QT_CHECK_NULL)
            qWarning("QSocketDevice: WinSock initialization failed");
#endif
#if defined(QSOCKETDEVICE_DEBUG)
            qDebug("QSocketDevice: WinSock initialization failed" );
#endif
            return;
        }
        qAddPostRoutine(cleanupWinSock);
        winsockVersion = 0x11;
    }
}

QSocketDevice::Protocol QSocketDevicePrivate::getProtocol() const
{
    if (q->isValid()) {
#if !defined (QT_NO_IPV6)
        struct qt_sockaddr_storage sa;
#else
        struct sockaddr_in sa;
#endif
        memset(&sa, 0, sizeof(sa));
        SOCKLEN_T sz = sizeof(sa);
        if (!::getsockname(fd, (struct sockaddr *)&sa, &sz)) {
#if !defined (QT_NO_IPV6)
            switch (sa.ss_family) {
                case AF_INET:
                    return QSocketDevice::IPv4;
                case AF_INET6:
                    return QSocketDevice::IPv6;
                default:
                    return QSocketDevice::Unknown;
            }
#else
            switch (sa.sin_family) {
                case AF_INET:
                    return QSocketDevice::IPv4;
                default:
                    return QSocketDevice::Unknown;
            }
#endif
        }
    }
    return QSocketDevice::Unknown;
}

int QSocketDevicePrivate::createNewSocket()
{
#if !defined(QT_NO_IPV6)
    int s;
    // Support IPv6 for Winsock v2.0++
    s = ::socket(winsockVersion >= 0x20 && q->protocol() == QSocketDevice::IPv6
                 ? AF_INET6 : AF_INET,
                 t == QSocketDevice::Datagram ? SOCK_DGRAM : SOCK_STREAM, 0);
#else
    int s = ::socket(AF_INET, t == QSocketDevice::Datagram ? SOCK_DGRAM : SOCK_STREAM, 0);
#endif
    if (s == INVALID_SOCKET) {
        switch (WSAGetLastError()) {
            case WSANOTINITIALISED:
                e = QSocketDevice::Impossible;
                break;
            case WSAENETDOWN:
                e = QSocketDevice::NetworkFailure;
                break;
            case WSAEMFILE:
                e = QSocketDevice::NoFiles; // special case for this
                break;
            case WSAEINPROGRESS:
            case WSAENOBUFS:
                e = QSocketDevice::NoResources;
                break;
            case WSAEAFNOSUPPORT:
            case WSAEPROTOTYPE:
            case WSAEPROTONOSUPPORT:
            case WSAESOCKTNOSUPPORT:
                e = QSocketDevice::InternalError;
                break;
            default:
                e = QSocketDevice::UnknownError;
                break;
        }
    } else {
        return s;
    }
    return -1;
}


void QSocketDevice::close()
{
    if (d->fd == -1)                // already closed
        return;
    ::closesocket(d->fd);
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug("QSocketDevice::close: Closed socket %x", d->fd);
#endif
    d->fd = -1;
    d->fetchConnectionParameters();
    return;
}


bool QSocketDevice::blocking() const
{
    if (!isValid())
        return true;
    return true;
}


void QSocketDevice::setBlocking(bool enable)
{
#if defined(QSOCKETDEVICE_DEBUG)
    qDebug("QSocketDevice::setBlocking(%d)", enable);
#endif
    if (!isValid())
        return;

    unsigned long dummy = enable ? 0 : 1;
    ioctlsocket(d->fd, FIONBIO, &dummy);
}


int QSocketDevicePrivate::option(Option opt) const
{
    if (!q->isValid())
        return -1;
    int n = -1;
    int v = -1;
    switch (opt) {
#if 0
        case Broadcast:
            n = SO_BROADCAST;
            break;
#endif
        case ReceiveBuffer:
            n = SO_RCVBUF;
            break;
        case ReuseAddress:
            n = SO_REUSEADDR;
            break;
        case SendBuffer:
            n = SO_SNDBUF;
            break;
    }
    if (n != -1) {
        SOCKLEN_T len = sizeof(v);
        int r = ::getsockopt(fd, SOL_SOCKET, n, (char*)&v, &len);
        if (r != SOCKET_ERROR)
            return v;
        if (!e) {
            switch (WSAGetLastError()) {
                case WSANOTINITIALISED:
                    e = QSocketDevice::Impossible;
                    break;
                case WSAENETDOWN:
                    e = QSocketDevice::NetworkFailure;
                    break;
                case WSAEFAULT:
                case WSAEINVAL:
                case WSAENOPROTOOPT:
                    e = QSocketDevice::InternalError;
                    break;
                case WSAEINPROGRESS:
                    e = QSocketDevice::NoResources;
                    break;
                case WSAENOTSOCK:
                    e = QSocketDevice::Impossible;
                    break;
                default:
                    e = QSocketDevice::UnknownError;
                    break;
            }
        }
        return -1;
    }
    return v;
}


void QSocketDevicePrivate::setOption(Option opt, int v)
{
    if (!q->isValid())
        return;
    int n = -1; // for really, really bad compilers
    switch (opt) {
#if 0
        case Broadcast:
            n = SO_BROADCAST;
            break;
#endif
        case ReceiveBuffer:
            n = SO_RCVBUF;
            break;
        case ReuseAddress:
            n = SO_REUSEADDR;
            break;
        case SendBuffer:
            n = SO_SNDBUF;
            break;
        default:
            return;
    }
    int r = ::setsockopt(fd, SOL_SOCKET, n, (char*)&v, sizeof(v));
    if (r == SOCKET_ERROR && e == QSocketDevice::NoError) {
        switch(WSAGetLastError()) {
            case WSANOTINITIALISED:
                e = QSocketDevice::Impossible;
                break;
            case WSAENETDOWN:
                e = QSocketDevice::NetworkFailure;
                break;
            case WSAEFAULT:
            case WSAEINVAL:
            case WSAENOPROTOOPT:
                e = QSocketDevice::InternalError;
                break;
            case WSAEINPROGRESS:
                e = QSocketDevice::NoResources;
                break;
            case WSAENETRESET:
            case WSAENOTCONN:
                e = QSocketDevice::Impossible;
                break;
            case WSAENOTSOCK:
                e = QSocketDevice::Impossible;
                break;
            default:
                e = QSocketDevice::UnknownError;
                break;
        }
    }
}


bool QSocketDevice::connect(const QHostAddress &addr, Q_UINT16 port)
{
    if (!isValid())
        return false;

    d->pa = addr;
    d->pp = port;

    struct sockaddr_in a4;
    struct sockaddr *aa;
    SOCKLEN_T aalen;

#if !defined(QT_NO_IPV6)
    qt_sockaddr_in6 a6;

    if (winsockVersion >= 0x20 && addr.isIPv6Address()) {
        memset(&a6, 0, sizeof(a6));
        a6.sin6_family = AF_INET6;
        a6.sin6_port = htons(port);
        Q_IPV6ADDR ip6 = addr.toIPv6Address();
        memcpy(&a6.sin6_addr.qt_s6_addr, &ip6, sizeof(ip6));

        aalen = sizeof(a6);
        aa = (struct sockaddr *)&a6;
    } else
#endif
    if (addr.isIPv4Address()) {
        memset(&a4, 0, sizeof(a4));
        a4.sin_family = AF_INET;
        a4.sin_port = htons(port);
        a4.sin_addr.s_addr = htonl(addr.toIPv4Address());

        aalen = sizeof(a4);
        aa = (struct sockaddr *)&a4;
    } else {
        d->e = Impossible;
        return false;
    }

    int r = ::connect(d->fd, aa, aalen);

    if (r == SOCKET_ERROR) {
        switch(WSAGetLastError()) {
            case WSANOTINITIALISED:
                d->e = Impossible;
                break;
            case WSAENETDOWN:
                d->e = NetworkFailure;
                break;
            case WSAEADDRINUSE:
            case WSAEINPROGRESS:
            case WSAENOBUFS:
                d->e = NoResources;
                break;
            case WSAEINTR:
                d->e = UnknownError;
                break;
            case WSAEALREADY:
                break;
            case WSAEADDRNOTAVAIL:
                d->e = ConnectionRefused;
                break;
            case WSAEAFNOSUPPORT:
            case WSAEFAULT:
                d->e = InternalError;
                break;
            case WSAEINVAL:
                break;
            case WSAECONNREFUSED:
                d->e = ConnectionRefused;
                break;
            case WSAEISCONN:
                goto successful;
            case WSAENETUNREACH:
            case WSAETIMEDOUT:
                d->e = NetworkFailure;
                break;
            case WSAENOTSOCK:
                d->e = Impossible;
                break;
            case WSAEWOULDBLOCK:
                break;
            case WSAEACCES:
                d->e = Inaccessible;
                break;
            case 10107:
                // Workaround for a problem with the WinSock Proxy Server. See
                // also support/arc-12/25557 for details on the problem.
                goto successful;
            default:
                d->e = UnknownError;
                break;
        }
        return false;
    }

successful:
    d->fetchConnectionParameters();
    return true;
}


bool QSocketDevice::bind(const QHostAddress &address, Q_UINT16 port)
{
    if (!isValid())
        return false;
    int r;
    struct sockaddr_in a4;
#if !defined(QT_NO_IPV6)
    qt_sockaddr_in6 a6;

    if (winsockVersion >= 0x20 && address.isIPv6Address()) {
        memset(&a6, 0, sizeof(a6));
        a6.sin6_family = AF_INET6;
        a6.sin6_port = htons(port);
        Q_IPV6ADDR tmp = address.toIPv6Address();
        memcpy(&a6.sin6_addr.qt_s6_addr, &tmp, sizeof(tmp));

        r = ::bind(d->fd, (struct sockaddr *)&a6, sizeof(struct qt_sockaddr_storage));
    } else
#endif
    if (address.isIPv4Address()) {
        memset(&a4, 0, sizeof(a4));
        a4.sin_family = AF_INET;
        a4.sin_port = htons(port);
        a4.sin_addr.s_addr = htonl(address.toIPv4Address());

        r = ::bind(d->fd, (struct sockaddr*)&a4, sizeof(struct sockaddr_in));
    } else {
        d->e = Impossible;
        return false;
    }

    if (r == SOCKET_ERROR) {
        switch (WSAGetLastError()) {
            case WSANOTINITIALISED:
                d->e = Impossible;
                break;
            case WSAENETDOWN:
                d->e = NetworkFailure;
                break;
            case WSAEACCES:
                d->e = Inaccessible;
                break;
            case WSAEADDRNOTAVAIL:
                d->e = Inaccessible;
                break;
            case WSAEFAULT:
                d->e = InternalError;
                break;
            case WSAEINPROGRESS:
            case WSAENOBUFS:
                d->e = NoResources;
                break;
            case WSAEADDRINUSE:
            case WSAEINVAL:
                d->e = AlreadyBound;
                break;
            case WSAENOTSOCK:
                d->e = Impossible;
                break;
            default:
                d->e = UnknownError;
                break;
        }
        return false;
    }
    d->fetchConnectionParameters();
    return true;
}


bool QSocketDevice::listen(int backlog)
{
    if (!isValid())
        return false;
    if (::listen(d->fd, backlog) >= 0)
        return true;
    if (!d->e)
        d->e = Impossible;
    return false;
}


int QSocketDevice::accept()
{
    if (!isValid())
        return -1;
#if !defined(QT_NO_IPV6)
    struct qt_sockaddr_storage a;
#else
    struct sockaddr a;
#endif
    SOCKLEN_T l = sizeof(a);
    bool done;
    int s;
    do {
        s = ::accept(d->fd, (struct sockaddr*)&a, &l);
        // we'll blithely throw away the stuff accept() wrote to a
        done = true;
        if (s == INVALID_SOCKET && d->e == NoError) {
            switch(WSAGetLastError()) {
                case WSAEINTR:
                    done = false;
                    break;
                case WSANOTINITIALISED:
                    d->e = Impossible;
                    break;
                case WSAENETDOWN:
                case WSAEOPNOTSUPP:
                    // in all these cases, an error happened during connection
                    // setup.  we're not interested in what happened, so we
                    // just treat it like the client-closed-quickly case.
                    break;
                case WSAEFAULT:
                    d->e = InternalError;
                    break;
                case WSAEMFILE:
                case WSAEINPROGRESS:
                case WSAENOBUFS:
                    d->e = NoResources;
                    break;
                case WSAEINVAL:
                case WSAENOTSOCK:
                    d->e = Impossible;
                    break;
                case WSAEWOULDBLOCK:
                    break;
                default:
                    d->e = UnknownError;
                    break;
            }
        }
    } while (!done);
    return s;
}


Q_LONG QSocketDevice::bytesAvailable() const
{
    if (!isValid())
        return -1;
    u_long nbytes = 0;
    if (::ioctlsocket(d->fd, FIONREAD, &nbytes) < 0)
        return -1;

    // ioctlsocket sometimes reports 1 byte available for datagrams
    // while the following recvfrom returns -1 and claims connection
    // was reset (udp is connectionless). so we peek one byte to
    // catch this case and return 0 bytes available if recvfrom
    // fails.
    if (d->t == Datagram) {
        char c;
        if (::recvfrom(d->fd, &c, sizeof(c), MSG_PEEK, 0, 0) == SOCKET_ERROR)
            return 0;
    }

    return nbytes;
}


Q_LONG QSocketDevice::waitForMore(int msecs, bool *timeout) const
{
    if (!isValid())
        return -1;

    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(d->fd, &fds);

    tv.tv_sec = msecs / 1000;
    tv.tv_usec = (msecs % 1000) * 1000;

    int rv = select(d->fd+1, &fds, 0, 0, msecs < 0 ? 0 : &tv);

    if (rv < 0)
        return -1;

    if (timeout) {
        if (rv == 0)
            *timeout = true;
        else
            *timeout = false;
    }

    return bytesAvailable();
}


Q_LLONG QSocketDevice::read(char *data, Q_LLONG maxlen)
{
    if (!isValid()) {
        qWarning("QSocketDevice::readBlock: Invalid socket");
        return -1;
    }
    Q_LONG r = 0;
    if (d->t == QSocketDevice::Datagram) {
#if !defined(QT_NO_IPV6)
        // With IPv6 support, we must be prepared to receive both IPv4
        // and IPv6 packets. The generic SOCKADDR_STORAGE (struct
        // sockaddr_storage on unix) replaces struct sockaddr.
        struct qt_sockaddr_storage a;
#else
        struct sockaddr_in a;
#endif
        memset(&a, 0, sizeof(a));
        SOCKLEN_T sz;
        sz = sizeof(a);
        r = ::recvfrom(d->fd, data, maxlen, 0, (struct sockaddr *)&a, &sz);
        qt_socket_getportaddr((struct sockaddr *)(&a), &d->pp, &d->pa);
    } else {
        r = ::recv(d->fd, data, maxlen, 0);
    }
    if (r == SOCKET_ERROR && d->e == QSocketDevice::NoError) {
        switch (WSAGetLastError()) {
            case WSANOTINITIALISED:
                d->e = QSocketDevice::Impossible;
                break;
            case WSAECONNABORTED:
                close();
                r = 0;
                break;
            case WSAETIMEDOUT:
            case WSAECONNRESET:
                /*
                From msdn doc:
                On a UDP datagram socket this error would indicate that a previous
                send operation resulted in an ICMP "Port Unreachable" message.

                So we should not close this socket just because one sendto failed.
                */
                if (d->t != QSocketDevice::Datagram)
                    close(); // connection closed
                r = 0;
                break;
            case WSAENETDOWN:
            case WSAENETRESET:
                d->e = QSocketDevice::NetworkFailure;
                break;
            case WSAEFAULT:
            case WSAENOTCONN:
            case WSAESHUTDOWN:
            case WSAEINVAL:
                d->e = QSocketDevice::Impossible;
                break;
            case WSAEINTR:
                r = 0;
                break;
            case WSAEINPROGRESS:
                d->e = QSocketDevice::NoResources;
                break;
            case WSAENOTSOCK:
                d->e = QSocketDevice::Impossible;
                break;
            case WSAEOPNOTSUPP:
                d->e = QSocketDevice::InternalError;
                break;
            case WSAEWOULDBLOCK:
                break;
           case WSAEMSGSIZE:
                d->e = QSocketDevice::NoResources;
                break;
            case WSAEISCONN:
                r = 0;
                break;
            default:
                d->e = QSocketDevice::UnknownError;
                break;
        }
    }
    return r;
}


Q_LLONG QSocketDevice::write(const char *data, Q_LLONG len)
{
    if (!isValid()) {
        qWarning("QSocketDevice::writeBlock: Invalid socket");
        return -1;
    }
    bool done = false;
    Q_LONG r = 0;
    while (!done) {
        // Don't write more than 64K (see Knowledge Base Q201213).
        r = ::send(d->fd, data, (len>64*1024 ? 64*1024 : len), 0);
        done = true;
        if (r == SOCKET_ERROR && d->e == QSocketDevice::NoError) {//&& errno != WSAEAGAIN) {
            switch(WSAGetLastError()) {
                case WSANOTINITIALISED:
                    d->e = QSocketDevice::Impossible;
                    break;
                case WSAENETDOWN:
                case WSAEACCES:
                case WSAENETRESET:
                case WSAESHUTDOWN:
                case WSAEHOSTUNREACH:
                    d->e = QSocketDevice::NetworkFailure;
                    break;
                case WSAECONNABORTED:
                case WSAECONNRESET:
                    // connection closed
                    close();
                    r = 0;
                    break;
                case WSAEINTR:
                    done = false;
                    break;
                case WSAEINPROGRESS:
                    d->e = QSocketDevice::NoResources;
                    break;
                case WSAEFAULT:
                case WSAEOPNOTSUPP:
                    d->e = QSocketDevice::InternalError;
                    break;
                case WSAENOBUFS:
                    break;
                case WSAEMSGSIZE:
                    d->e = QSocketDevice::NoResources;
                    break;
                case WSAENOTCONN:
                case WSAENOTSOCK:
                case WSAEINVAL:
                    d->e = QSocketDevice::Impossible;
                    break;
                case WSAEWOULDBLOCK:
                    r = 0;
                    break;
                default:
                    d->e = QSocketDevice::UnknownError;
                    break;
            }
        }
    }
    return r;
}


Q_LLONG QSocketDevice::write(const char *data, Q_LLONG len, const QHostAddress & host, Q_UINT16 port)
{
    if (d->t != Datagram) {
        qWarning("QSocketDevice::sendBlock: Not datagram");
        return -1; // for now - later we can do t/tcp
    }

    if (data == 0 && len != 0) {
        qWarning("QSocketDevice::sendBlock: Null pointer error");
        return -1;
    }
    if (!isValid()) {
        qWarning("QSocketDevice::sendBlock: Invalid socket");
        return -1;
    }
    if (!isOpen()) {
        qWarning("QSocketDevice::sendBlock: Device is not open");
        return -1;
    }
    if (!isWritable()) {
        qWarning("QSocketDevice::sendBlock: Write operation not permitted");
        return -1;
    }
    struct sockaddr_in a4;
    struct sockaddr *aa;
    SOCKLEN_T slen;
#if !defined(QT_NO_IPV6)
    qt_sockaddr_in6 a6;
    if (winsockVersion >= 0x20 && host.isIPv6Address()) {
        memset(&a6, 0, sizeof(a6));
        a6.sin6_family = AF_INET6;
        a6.sin6_port = htons(port);

        Q_IPV6ADDR tmp = host.toIPv6Address();
        memcpy(&a6.sin6_addr.qt_s6_addr, &tmp, sizeof(tmp));
        slen = sizeof(a6);
        aa = (struct sockaddr *)&a6;
    } else
#endif
    if (host.isIPv4Address()) {

        memset(&a4, 0, sizeof(a4));
        a4.sin_family = AF_INET;
        a4.sin_port = htons(port);
        a4.sin_addr.s_addr = htonl(host.toIPv4Address());
        slen = sizeof(a4);
        aa = (struct sockaddr *)&a4;
    } else {
        d->e = Impossible;
        return -1;
    }

    // we'd use MSG_DONTWAIT + MSG_NOSIGNAL if Stevens were right.
    // but apparently Stevens and most implementors disagree
    bool done = false;
    Q_LONG r = 0;
    while (!done) {
        r = ::sendto(d->fd, data, len, 0, aa, slen);
        done = true;
        if (r == SOCKET_ERROR && d->e == NoError) {//&& e != EAGAIN) {
            switch (WSAGetLastError()) {
                case WSANOTINITIALISED:
                    d->e = Impossible;
                    break;
                case WSAENETDOWN:
                case WSAEACCES:
                case WSAENETRESET:
                case WSAESHUTDOWN:
                case WSAEHOSTUNREACH:
                case WSAECONNABORTED:
                case WSAECONNRESET:
                case WSAEADDRNOTAVAIL:
                case WSAENETUNREACH:
                case WSAETIMEDOUT:
                    d->e = NetworkFailure;
                    break;
                case WSAEINTR:
                    done = false;
                    break;
                case WSAEINPROGRESS:
                    d->e = NoResources;
                    break;
                case WSAEFAULT:
                case WSAEOPNOTSUPP:
                case WSAEAFNOSUPPORT:
                    d->e = InternalError;
                    break;
                case WSAENOBUFS:
                case WSAEMSGSIZE:
                    d->e = NoResources;
                    break;
                case WSAENOTCONN:
                case WSAENOTSOCK:
                case WSAEINVAL:
                case WSAEDESTADDRREQ:
                    d->e = Impossible;
                    break;
                case WSAEWOULDBLOCK:
                    r = 0;
                    break;
                default:
                    d->e = UnknownError;
                    break;
            }
        }
    }
    return r;
}


void QSocketDevicePrivate::fetchConnectionParameters() const
{
    if (!q->isValid()) {
        p = 0;
        a.clear();
        pp = 0;
        pa.clear();
        return;
    }
#if !defined (QT_NO_IPV6)
    struct qt_sockaddr_storage sa;
#else
    struct sockaddr_in sa;
#endif
    memset(&sa, 0, sizeof(sa));
    SOCKLEN_T sz;
    sz = sizeof(sa);
    if (!::getsockname(fd, (struct sockaddr *)(&sa), &sz))
        qt_socket_getportaddr((struct sockaddr *)(&sa), &p, &a);
    pp = 0;
    pa.clear();
}

void QSocketDevicePrivate::fetchPeerConnectionParameters() const
{
    // do the getpeername() lazy on Windows (sales/arc-18/37759 claims that
    // there will be problems otherwise if you use MS Proxy server)
#if !defined (QT_NO_IPV6)
    struct qt_sockaddr_storage sa;
#else
    struct sockaddr_in sa;
#endif
    memset(&sa, 0, sizeof(sa));
    SOCKLEN_T sz;
    sz = sizeof(sa);
    if (!::getpeername(fd, (struct sockaddr *)(&sa), &sz))
        qt_socket_getportaddr((struct sockaddr *)(&sa), &pp, &pa);
}

Q_UINT16 QSocketDevice::peerPort() const
{
    if (d->pp == 0 && isValid())
        d->fetchPeerConnectionParameters();
    return d->pp;
}

QHostAddress QSocketDevice::peerAddress() const
{
    if (d->pp == 0 && isValid())
        d->fetchPeerConnectionParameters();
    return d->pa;
}
