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

#include "qhostinfo_p.h"
#include "qiodevice.h"
#include <qbytearray.h>

extern "C" {
#include <netdb.h>
}

#if defined (QT_NO_GETADDRINFO)
#include <qmutex.h>
Q_GLOBAL_STATIC(QMutex, getHostByNameMutex)
#endif

//#define QHOSTINFO_DEBUG

QHostInfo QHostInfoAgent::fromName(const QString &hostName)
{
    QHostInfo results;
    results.setHostName(hostName);

#if defined(QHOSTINFO_DEBUG)
    qDebug("QHostInfoAgent::fromName(%s) looking up...",
           hostName.toLatin1().constData());
#endif

#if !defined (QT_NO_GETADDRINFO)
    // Call getaddrinfo, and place all IPv4 addresses at the start and
    // the IPv6 addresses at the end of the address list in results.
    addrinfo *res = 0;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_UNSPEC;

    int result = getaddrinfo(hostName.toLatin1().constData(), 0, &hints, &res);
    if (result == 0) {
        addrinfo *node = res;
        QList<QHostAddress> addresses;
        while (node) {
            if (node->ai_family == AF_INET) {
                QHostAddress addr;
                addr.setAddress(ntohl(((sockaddr_in *) node->ai_addr)->sin_addr.s_addr));
                if (!addresses.contains(addr))
                    addresses.prepend(addr);
            } else if (node->ai_family == AF_INET6) {
                QHostAddress addr;
                addr.setAddress(((sockaddr_in6 *) node->ai_addr)->sin6_addr.s6_addr);
                if (!addresses.contains(addr))
                    addresses.append(addr);
            } else {
                results.setError(QHostInfo::UnknownError);
                results.setErrorString(tr("Unknown address type"));
                break;
            }
            node = node->ai_next;
        }
        results.setAddresses(addresses);
        freeaddrinfo(res);
    } else if (result == EAI_NONAME
#ifdef EAI_NODATA
	       // EAI_NODATA is deprecated in RFC 3493
	       || result == EAI_NODATA
#endif
	       ) {
        results.setError(QHostInfo::HostNotFound);
        results.setErrorString(tr("Host not found"));
    } else {
        results.setError(QHostInfo::UnknownError);
        results.setErrorString(QString::fromLocal8Bit(gai_strerror(result)));
    }

#else
    // Fall back to gethostbyname for platforms that don't define
    // getaddrinfo. gethostbyname does not support IPv6, and it's not
    // reentrant on all platforms. For now this is okay since we only
    // use one QHostInfoAgent, but if more agents are introduced, locking
    // must be provided.
    QMutexLocker locker(::getHostByNameMutex());
    hostent *result = gethostbyname(hostName.toLatin1().constData());
    if (result) {
        if (result->h_addrtype == AF_INET) {
            QList<QHostAddress> addresses;
            for (char **p = result->h_addr_list; *p != 0; p++) {
                QHostAddress addr;
                addr.setAddress(ntohl(*((quint32 *)*p)));
                if (!addresses.contains(addr))
                    addresses.prepend(addr);
            }
            results.setAddresses(addresses);
        } else {
            results.setError(QHostInfo::UnknownError);
            results.setErrorString(tr("Unknown address type"));
        }
    } else if (h_errno == HOST_NOT_FOUND || h_errno == NO_DATA
               || h_errno == NO_ADDRESS) {
        results.setError(QHostInfo::HostNotFound);
        results.setErrorString(tr("Host not found"));
    } else {
        results.setError(QHostInfo::UnknownError);
        results.setErrorString(tr("Unknown error"));
    }
#endif //  !defined (QT_NO_GETADDRINFO)

#if defined(QHOSTINFO_DEBUG)
    if (results.error() != QHostInfo::NoError) {
        qDebug("QHostInfoAgent::fromName(): error #%d %s",
               h_errno, results.errorString().toLatin1().constData());
    } else {
        QString tmp;
        QList<QHostAddress> addresses = results.addresses();
        for (int i = 0; i < addresses.count(); ++i) {
            if (i != 0) tmp += ", ";
            tmp += addresses.at(i).toString();
        }
        qDebug("QHostInfoAgent::fromName(): found %i entries for \"%s\": {%s}",
               addresses.count(), hostName.toLatin1().constData(),
               tmp.toLatin1().constData());
    }
#endif
    return results;
}

QString QHostInfo::localHostName()
{
    char hostName[512];
    if (gethostname(hostName, sizeof(hostName)) == -1)
        return QString::null;
    hostName[sizeof(hostName) - 1] = '\0';
    return QString::fromLocal8Bit(hostName);
}
