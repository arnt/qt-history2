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

#include <winsock2.h>

#include "qdns_p.h"
#include "qsocketlayer_p.h"
#include <ws2tcpip.h>
#include <qlibrary.h>
#include <qsignal.h>
#include <qtimer.h>
#include <qmutex.h>
#include <qsignal.h>

// #define QDNS_DEBUG

// Older SDKs do not include the addrinfo struct declaration, so we
// include a copy of it here.
struct qt_addrinfo
{
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    size_t ai_addrlen;
    char *ai_canonname;
    sockaddr *ai_addr;
    qt_addrinfo *ai_next;
};

typedef int (__stdcall *getaddrinfoProto)(const char *, const char *, const qt_addrinfo *, qt_addrinfo **);
typedef int (__stdcall *freeaddrinfoProto)(qt_addrinfo *);
static getaddrinfoProto local_getaddrinfo;
static freeaddrinfoProto local_freeaddrinfo;

void resolveLibrary()
{
    // Attempt to resolve getaddrinfo(); without it we'll have to fall
    // back to gethostbyname(), which has no IPv6 support.
    local_getaddrinfo = (getaddrinfoProto) QLibrary::resolve("ws2_32.dll", "getaddrinfo");
    local_freeaddrinfo = (freeaddrinfoProto) QLibrary::resolve("ws2_32.dll", "freeaddrinfo");
}

/*
    Performs a blocking call to gethostbyname or getaddrinfo, stores
    the results in a QDnsHostInfo structure and emits the
    resultsReady() signal.
*/
QDnsHostInfo QDnsAgent::getHostByName(const QString &hostName)
{
    QWindowsSockInit winSock;

    if (!local_getaddrinfo)
        resolveLibrary();

    QDnsHostInfo results;
    results.d->hostName = hostName;

#if defined(QDNS_DEBUG)
    qDebug("QDnsAgent::run(%p): looking up \"%s\" (IPv6 support is %s)",
           this, hostName.latin1(),
           (local_getaddrinfo && local_freeaddrinfo) ? "enabled" : "disabled");
#endif

    if (local_getaddrinfo && local_freeaddrinfo) {
        // Call getaddrinfo, and place all IPv4 addresses at the start
        // and the IPv6 addresses at the end of the address list in
        // results.
        qt_addrinfo *res;
        int err = local_getaddrinfo(hostName.latin1(), 0, 0, &res);
        if (err == 0) {
            for (qt_addrinfo *p = res; p != 0; p = p->ai_next) {
                switch (p->ai_family) {
                case AF_INET: {
                    QHostAddress addr;
		    addr.setAddress(ntohl(((sockaddr_in *) p->ai_addr)->sin_addr.s_addr));
                    if (!results.d->addrs.contains(addr))
                        results.d->addrs.prepend(addr);
                }
                    break;
                case AF_INET6: {
                    QHostAddress addr;
		    addr.setAddress(((sockaddr_in6 *) p->ai_addr)->sin6_addr.s6_addr);
                    if (!results.d->addrs.contains(addr))
                        results.d->addrs.append(addr);
                }
                    break;
                default:
                    results.d->err = QDnsHostInfo::UnknownError;
                    results.d->errorStr = "Unknown address type";
                    break;
                }
            }
            local_freeaddrinfo(res);
        } else if (err == WSAHOST_NOT_FOUND) {
            results.d->err = QDnsHostInfo::HostNotFound;
            results.d->errorStr = tr("Host not found");
        } else {
            results.d->err = QDnsHostInfo::UnknownError;
            results.d->errorStr = tr("Unknown error");
            // Get the error messages returned by getaddrinfo's gai_strerror
            QT_WA( {
                typedef char *(*gai_strerrorWProto)(int);
                gai_strerrorWProto local_gai_strerrorW;
                local_gai_strerrorW = (gai_strerrorWProto) QLibrary::resolve("ws2_32.dll", "gai_strerrorW");
                if (local_gai_strerrorW)
                    results.d->errorStr = QString::fromUtf16((ushort *) local_gai_strerrorW(err));
            } , {
                typedef char *(*gai_strerrorAProto)(int);
                gai_strerrorAProto local_gai_strerrorA;
                local_gai_strerrorA = (gai_strerrorAProto) QLibrary::resolve("ws2_32.dll", "gai_strerrorA");
                if (local_gai_strerrorA)
                    results.d->errorStr = QString::fromLocal8Bit(local_gai_strerrorA(err));
            } );
        }
    } else {
        // Fall back to gethostbyname, which only supports IPv4.
        hostent *ent = gethostbyname(hostName.latin1());
        if (ent) {
            char **p;
            switch (ent->h_addrtype) {
            case AF_INET:
                for (p = ent->h_addr_list; *p != 0; p++) {
                    long *ip4Addr = (long *) *p;
		    QHostAddress temp;
		    temp.setAddress(ntohl(*ip4Addr));
                    results.d->addrs << temp;
                }
                break;
            default:
                results.d->err = QDnsHostInfo::UnknownError;
                results.d->errorStr = tr("Unknown address type");
                break;
            }
        } else if (WSAGetLastError() == 11001) {
            results.d->errorStr = tr("Host not found");
            results.d->err = QDnsHostInfo::HostNotFound;
        } else {
            results.d->errorStr = tr("Unknown error");
            results.d->err = QDnsHostInfo::UnknownError;
        }
    }

#if defined(QDNS_DEBUG)
    if (results.d->err != QDnsHostInfo::NoError) {
        qDebug("QDnsAgent::run(%p): error (%s)",
               this, results.d->errorStr.latin1());
    } else {
        QString tmp;
        for (int i = 0; i < results.d->addrs.count(); ++i) {
            if (i != 0) tmp += ", ";
            tmp += results.d->addrs.at(i).toString();
        }
        qDebug("QDnsAgent::run(%p): found %i entries: {%s}",
               this, results.d->addrs.count(), tmp.latin1());
    }
#endif
    return results;
}

