/****************************************************************************
**
** Implementation of QDnsHostInfo class.
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

#include "qdns_p.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <qbytearray.h>

//#define QDNS_DEBUG

QDnsHostInfo QDnsAgent::getHostByName(const QString &hostName)
{
    QDnsHostInfo results;
    results.d->hostName = hostName;
#if !defined (QT_NO_GETADDRINFO)
    // Call getaddrinfo, and place all IPv4 addresses at the start and
    // the IPv6 addresses at the end of the address list in results.
    addrinfo *res = 0;
    int result = getaddrinfo(hostName.latin1(), 0, 0, &res);
    if (result == 0) {
        addrinfo *node = res;
        while (node) {
            if (node->ai_family == AF_INET) {
                QHostAddress addr(ntohl(((sockaddr_in *) node->ai_addr)->sin_addr.s_addr));
                if (!results.d->addrs.contains(addr))
                    results.d->addrs.prepend(addr);
            } else if (node->ai_family == AF_INET6) {
                QHostAddress addr(((sockaddr_in6 *) node->ai_addr)->sin6_addr.s6_addr);
                if (!results.d->addrs.contains(addr))
                    results.d->addrs.append(addr);
            } else {
                results.d->err = QDnsHostInfo::UnknownError;
                results.d->errorStr = tr("Unknown address type");
                break;
            }
            node = node->ai_next;
        }

        freeaddrinfo(res);
    } else if (result == EAI_NONAME) {
        results.d->err = QDnsHostInfo::HostNotFound;
        results.d->errorStr = tr("Host not found");
    } else {
        results.d->err = QDnsHostInfo::UnknownError;
        results.d->errorStr = QString::fromLocal8Bit(gai_strerror(result));
    }

#else
    // Fall back to the reentrant GNU extension gethostbyname_r for
    // platforms that don't define getaddrinfo. gethostbyname_r does
    // not support IPv6.
    char auxbuf[512];
    hostent ent;
    hostent *result;
    int err;
    if (gethostbyname_r(hostName.latin1(), &ent, auxbuf, sizeof(auxbuf), &result, &err) == 0) {
        if (ent.h_addrtype == AF_INET) {
            for (char **p = ent.h_addr_list; *p != 0; p++) {
                QHostAddress addr(ntohl(*((long *)*p)));
                if (!results.d->addresses.contains(addr))
                    results.d->addresses.prepend(addr);
            }
        } else {
            results.d->error = QDnsHostInfo::UnknownError;
            results.d->errorString = tr("Unknown address type");
        }
    } else if (h_errno == HOST_NOT_FOUND) {
        results.d->error = QDnsHostInfo::HostNotFound;
        results.d->errorString = tr("Host not found");
    } else if (h_errno != NO_DATA) {
        results.d->error = QDnsHostInfo::UnknownError;
        results.d->errorString = QString::fromLocal8Bit(hstrerror(h_errno));
    }
#endif //  !defined (QT_NO_GETADDRINFO)

#if defined(QDNS_DEBUG)
    if (results.d->err != QDnsHostInfo::NoError) {
        qDebug("QDnsAgent::run(%p): error #%d %s",
               this, h_errno, results.d->errorStr.latin1() );
    } else {
        QString tmp;
        for (int i = 0; i < results.d->addrs.count(); ++i) {
            if (i != 0) tmp += ", ";
            tmp += results.d->addrs.at(i).toString();
        }
        qDebug("QDnsAgent::run(%p): found %i entries for \"%s\": {%s}",
               this, results.d->addrs.count(), hostName.latin1(), tmp.latin1());
    }
#endif
    return results;
}
