#include "qresolver_p.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <qlibrary.h>
#include <qsignal.h>
#include <qtimer.h>

//#define QRESOLVER_DEBUG

struct qt_addrinfo
{
    int                 ai_flags;       // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
    int                 ai_family;      // PF_xxx
    int                 ai_socktype;    // SOCK_xxx
    int                 ai_protocol;    // 0 or IPPROTO_xxx for IPv4 and IPv6
    size_t              ai_addrlen;     // Length of ai_addr
    char *              ai_canonname;   // Canonical name for nodename
    struct sockaddr    *ai_addr;        // Binary address
    struct qt_addrinfo *ai_next;        // Next structure in linked list
};

void QResolverAgent::run()
{
#if defined(QRESOLVER_DEBUG)
    qDebug("QResolverAgent::run(%p): start DNS lookup", this);
#endif

    // Attempt to resolve getaddrinfo(); without it we'll have to fall
    // back to gethostbyname(), which has no IPv6 support.
    typedef int (*getaddrinfoProto)(const char *, const char *, const qt_addrinfo *, qt_addrinfo **);
    typedef int (*freeaddrinfoProto)(qt_addrinfo *);
    getaddrinfoProto local_getaddrinfo;
    freeaddrinfoProto local_freeaddrinfo;
    local_getaddrinfo = (getaddrinfoProto) QLibrary::resolve("ws2_32.dll", "getaddrinfo");
    local_freeaddrinfo = (freeaddrinfoProto) QLibrary::resolve("ws2_32.dll", "freeaddrinfo");

    QResolverHostInfo results;

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
                    QHostAddress addr(ntohl(((sockaddr_in *) p->ai_addr)->sin_addr.s_addr));
                    if (!results.addresses.contains(addr))
                        results.addresses.prepend(addr);
                }
                    break;
                case AF_INET6: {
                    QHostAddress addr(((sockaddr_in6 *) p->ai_addr)->sin6_addr.s6_addr);
                    if (!results.addresses.contains(addr))
                        results.addresses.append(addr);
                }
                    break;
                default:
                    results.error = QResolver::UnknownError;
                    results.errorString = "Unknown address type";
                    break;
                }
            }
            local_freeaddrinfo(res);
        } else if (err == EAI_NONAME) {
            results.error = QResolver::HostNotFound;
            results.errorString = tr("Host not found");
        } else {
            results.error = QResolver::UnknownError;
            // Get the error messages returned by getaddrinfo's gai_strerror
            QT_WA( {
                results.errorString = QString::fromUtf16((ushort *) gai_strerrorW(err));
            } , {
                results.errorString = QString::fromLocal8Bit(gai_strerrorA(err));
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
		    results.addresses << QHostAddress(ntohl(*ip4Addr));
		}
		break;
	    default:
		results.error = QResolver::UnknownError;
		results.errorString = tr("Unknown address type");
		break;
            }
        } else if (WSAGetLastError() == 11001) {
            results.errorString = tr("Host not found");
            results.error = QResolver::HostNotFound;
        } else {
            results.errorString = tr("Unknown error");
            results.error = QResolver::UnknownError;
        }
    }
    
    emit resultsReady(results);

#if !defined QT_NO_THREAD
    connect(this, SIGNAL(terminated()), SLOT(deleteLater()));
#else
    deleteLater();
#endif
}

