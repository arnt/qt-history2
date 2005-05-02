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

#include "qhostinfo_p.h"
#include "qsocketlayer_p.h"
#include <ws2tcpip.h>
#include <qlibrary.h>
#include <qtimer.h>
#include <qmutex.h>

//#define QHOSTINFO_DEBUG

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
static getaddrinfoProto local_getaddrinfo = 0;
static freeaddrinfoProto local_freeaddrinfo = 0;

void resolveLibrary()
{
    // Attempt to resolve getaddrinfo(); without it we'll have to fall
    // back to gethostbyname(), which has no IPv6 support.
    local_getaddrinfo = (getaddrinfoProto) QLibrary::resolve("ws2_32.dll", "getaddrinfo");
    local_freeaddrinfo = (freeaddrinfoProto) QLibrary::resolve("ws2_32.dll", "freeaddrinfo");
}

/*
    Performs a blocking call to gethostbyname or getaddrinfo, stores
    the results in a QHostInfo structure and emits the
    resultsReady() signal.
*/
QHostInfo QHostInfoAgent::fromName(const QString &hostName)
{
    QWindowsSockInit winSock;

    if (!local_getaddrinfo)
        resolveLibrary();

    QHostInfo results;
    results.setHostName(hostName);

#if defined(QHOSTINFO_DEBUG)
    qDebug("QHostInfoAgent::fromName(%p): looking up \"%s\" (IPv6 support is %s)",
           this, hostName.toLatin1().constData(),
           (local_getaddrinfo && local_freeaddrinfo) ? "enabled" : "disabled");
#endif

    if (local_getaddrinfo && local_freeaddrinfo) {
        // Call getaddrinfo, and place all IPv4 addresses at the start
        // and the IPv6 addresses at the end of the address list in
        // results.
        qt_addrinfo *res;
        int err = local_getaddrinfo(hostName.toLatin1().constData(), 0, 0, &res);
        if (err == 0) {
            QList<QHostAddress> addresses;
            for (qt_addrinfo *p = res; p != 0; p = p->ai_next) {
                switch (p->ai_family) {
                case AF_INET: {
                    QHostAddress addr;
		    addr.setAddress(ntohl(((sockaddr_in *) p->ai_addr)->sin_addr.s_addr));
                    if (!addresses.contains(addr))
                        addresses.prepend(addr);
                }
                    break;
                case AF_INET6: {
                    QHostAddress addr;
		    addr.setAddress(((sockaddr_in6 *) p->ai_addr)->sin6_addr.s6_addr);
                    if (!addresses.contains(addr))
                        addresses.append(addr);
                }
                    break;
                default:
                    results.setError(QHostInfo::UnknownError);
                    results.setErrorString(tr("Unknown address type"));
                }
            }
            results.setAddresses(addresses);
            local_freeaddrinfo(res);
        } else if (WSAGetLastError() == WSAHOST_NOT_FOUND || WSAGetLastError() == WSANO_DATA) {
            results.setError(QHostInfo::HostNotFound);
            results.setErrorString(tr("Host not found"));
        } else {
            results.setError(QHostInfo::UnknownError);
            results.setErrorString(tr("Unknown error"));
        }
    } else {
        // Fall back to gethostbyname, which only supports IPv4.
        hostent *ent = gethostbyname(hostName.toLatin1().constData());
        if (ent) {
            char **p;
            QList<QHostAddress> addresses;
            switch (ent->h_addrtype) {
            case AF_INET:
                for (p = ent->h_addr_list; *p != 0; p++) {
                    long *ip4Addr = (long *) *p;
		    QHostAddress temp;
		    temp.setAddress(ntohl(*ip4Addr));
                    addresses << temp;
                }
                break;
            default:
                results.setError(QHostInfo::UnknownError);
                results.setErrorString(tr("Unknown address type"));
                break;
            }
            results.setAddresses(addresses);
        } else if (WSAGetLastError() == 11001) {
            results.setErrorString(tr("Host not found"));
            results.setError(QHostInfo::HostNotFound);
        } else {
            results.setErrorString(tr("Unknown error"));
            results.setError(QHostInfo::UnknownError);
        }
    }

#if defined(QHOSTINFO_DEBUG)
    if (results.error() != QHostInfo::NoError) {
        qDebug("QHostInfoAgent::run(%p): error (%s)",
               this, results.errorString().toLatin1().constData());
    } else {
        QString tmp;
        QList<QHostAddress> addresses = results.addresses();
        for (int i = 0; i < addresses.count(); ++i) {
            if (i != 0) tmp += ", ";
            tmp += addresses.at(i).toString();
        }
        qDebug("QHostInfoAgent::run(%p): found %i entries: {%s}",
               this, addresses.count(), tmp.toLatin1().constData());
    }
#endif
    return results;
}

QString QHostInfo::localHostName()
{
    QWindowsSockInit winSock;

    char hostName[512];
    if (gethostname(hostName, sizeof(hostName)) == -1)
        return QString();
    hostName[sizeof(hostName) - 1] = '\0';
    return QString::fromLocal8Bit(hostName);
}

