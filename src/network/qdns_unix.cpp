#include "qdns_p.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <qlibrary.h>
#include <netinet/in.h>
#include <qsignal.h>
#include <qbytearray.h>

// #define QDNS_DEBUG

/*
    Performs a blocking call to gethostbyname or getaddrinfo, stores
    the results in a QDnsHostInfo structure and emits the
    resultsReady() signal.
*/
void QDnsAgent::run()
{
    for (;;) {
        QDnsQuery query;
        {
            QMutexLocker locker(&mutex);
            if (queries.isEmpty())
                break;
            query = queries.takeFirst();
        }

        if (!query.receiver)
            continue;

        QString hostName = query.hostName;

#if defined(QDNS_DEBUG)
        qDebug("QDnsAgent::run(%p): looking up \"%s\"", this, hostName.latin1());
#endif

        QDnsHostInfo results;

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
                    if (!results.addrs.contains(addr))
                        results.addrs.prepend(addr);
                } else if (node->ai_family == AF_INET6) {
                    QHostAddress addr(((sockaddr_in6 *) node->ai_addr)->sin6_addr.s6_addr);
                    if (!results.addrs.contains(addr))
                        results.addrs.append(addr);
                } else {
                    results.err = QDnsHostInfo::UnknownError;
                    results.errorStr = tr("Unknown address type");
                    break;
                }
                node = node->ai_next;
            }

            freeaddrinfo(res);
        } else if (result == EAI_NONAME) {
            results.err = QDnsHostInfo::HostNotFound;
            results.errorStr = tr("Host not found");
        } else {
            results.err = QDnsHostInfo::UnknownError;
            results.errorStr = QString::fromLocal8Bit(gai_strerror(result));
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
                    if (!results.addresses.contains(addr))
                        results.addresses.prepend(addr);
                }
            } else {
                results.error = QDnsHostInfo::UnknownError;
                results.errorString = tr("Unknown address type");
            }
        } else if (h_errno == HOST_NOT_FOUND) {
            results.error = QDnsHostInfo::HostNotFound;
            results.errorString = tr("Host not found");
        } else if (h_errno != NO_DATA) {
            results.error = QDnsHostInfo::UnknownError;
            results.errorString = QString::fromLocal8Bit(hstrerror(h_errno));
        }
#endif //  !defined (QT_NO_GETADDRINFO)

#if defined(QDNS_DEBUG)
        if (results.err != QDnsHostInfo::NoError) {
            qDebug("QDnsAgent::run(%p): error #%d %s",
                   this, h_errno, results.errorStr.latin1() );
        } else {
            QString tmp;
            for (int i = 0; i < results.addrs.count(); ++i) {
                if (i != 0) tmp += ", ";
                tmp += results.addrs.at(i).toString();
            }
            qDebug("QDnsAgent::run(%p): found %i entries for \"%s\": {%s}",
                   this, results.addrs.count(), hostName.latin1(), tmp.latin1());
        }
#endif

        if (query.receiver) {
            QByteArray arr(query.member + 1);
            arr.resize(arr.indexOf('('));
            qInvokeSlot(query.receiver, arr, Qt::QueuedConnection,
                        QGenericArgument("QDnsHostInfo", &results));
        }
    }
}
