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

#include "qplatformdefs.h"

#include "qdns_p.h"
#include "qiodevice.h"
#include <qbytearray.h>

extern "C" {
#include <netdb.h>
}

//#define QDNS_DEBUG

QDnsHostInfo QDnsAgent::getHostByName(const QString &hostName)
{
    QDnsHostInfo results;
    results.d->hostName = hostName;

#if defined(QDNS_DEBUG)
    qDebug("QDnsAgent::getHostByName(%s) looking up...", hostName.latin1());
#endif

#if !defined (QT_NO_GETADDRINFO)
    // Call getaddrinfo, and place all IPv4 addresses at the start and
    // the IPv6 addresses at the end of the address list in results.
    addrinfo *res = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;

    int result = getaddrinfo(hostName.latin1(), 0, &hints, &res);
    if (result == 0) {
        addrinfo *node = res;
        while (node) {
            if (node->ai_family == AF_INET) {
                QHostAddress addr;
                addr.setAddress(ntohl(((sockaddr_in *) node->ai_addr)->sin_addr.s_addr));
                if (!results.d->addrs.contains(addr))
                    results.d->addrs.prepend(addr);
            } else if (node->ai_family == AF_INET6) {
                QHostAddress addr;
                addr.setAddress(((sockaddr_in6 *) node->ai_addr)->sin6_addr.s6_addr);
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
    } else if (result == EAI_NONAME
#ifdef EAI_NODATA
	       // EAI_NODATA is deprecated in RFC 3493
	       || result == EAI_NODATA
#endif
	       ) {
        results.d->err = QDnsHostInfo::HostNotFound;
        results.d->errorStr = tr("Host not found");
    } else {
        results.d->err = QDnsHostInfo::UnknownError;
        results.d->errorStr = QString::fromLocal8Bit(gai_strerror(result));
    }

#else
    // Fall back to gethostbyname for platforms that don't define
    // getaddrinfo. gethostbyname does not support IPv6, and it's not
    // reentrant on all platforms. For now this is okay since we only
    // use one QDnsAgent, but if more agents are introduced, locking
    // must be provided.
    hostent *result = gethostbyname(hostName.latin1());
    if (result) {
        if (result->h_addrtype == AF_INET) {
            for (char **p = result->h_addr_list; *p != 0; p++) {
                QHostAddress addr;
                addr.setAddress(ntohl(*((Q_UINT32 *)*p)));
                if (!results.d->addrs.contains(addr))
                    results.d->addrs.prepend(addr);
            }
        } else {
            results.d->err = QDnsHostInfo::UnknownError;
            results.d->errorStr = tr("Unknown address type");
        }
    } else if (h_errno == HOST_NOT_FOUND || h_errno == NO_DATA
               || h_errno == NO_ADDRESS) {
        results.d->err = QDnsHostInfo::HostNotFound;
        results.d->errorStr = tr("Host not found");
    } else {
        results.d->err = QDnsHostInfo::UnknownError;
        results.d->errorStr = tr("Unknown error");
    }
#endif //  !defined (QT_NO_GETADDRINFO)

#if defined(QDNS_DEBUG)
    if (results.d->err != QDnsHostInfo::NoError) {
        qDebug("QDnsAgent::getHostByName(): error #%d %s",
               h_errno, results.d->errorStr.latin1());
    } else {
        QString tmp;
        for (int i = 0; i < results.d->addrs.count(); ++i) {
            if (i != 0) tmp += ", ";
            tmp += results.d->addrs.at(i).toString();
        }
        qDebug("QDnsAgent::getHostByName(): found %i entries for \"%s\": {%s}",
               results.d->addrs.count(), hostName.latin1(), tmp.latin1());
    }
#endif
    return results;
}
