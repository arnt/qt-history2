#include "qresolver_p.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <qlibrary.h>
#include <qsignal.h>

//#define QRESOLVER_DEBUG

#include <qtimer.h>

void QResolverAgent::run()
{
#if defined(QRESOLVER_DEBUG)
    qDebug("QResolverAgent::run(%p): start DNS lookup", this);
#endif

    QResolverHostInfo results;

    // Attempt to resolve getaddrinfo(); without it we'll have to fall
    // back to gethostbyname(), which has no IPv6 support.
    typedef int (*getaddrinfoProto)(const char *, const char *, const addrinfo *, addrinfo **);
    getaddrinfoProto local_getaddrinfo;
    local_getaddrinfo = (getaddrinfoProto) QLibrary::resolve("ws2_32.dll", "getaddrinfo");

    if (local_getaddrinfo) {
        addrinfo *res;
        int err = local_getaddrinfo(hostName.latin1(), 0, 0, &res);

        if (err) {
            switch ( err ) {
            case EAI_NONAME:
                results.error = QResolver::HostNotFound;
                break;
            default:
                results.error = QResolver::UnknownError;
                break;
            }
#if defined(Q_OS_WIN32)
            QT_WA( {
                results.errorString = QString::fromUtf16((ushort *) gai_strerrorW(err));
            } , {
                results.errorString = QString::fromLocal8Bit(gai_strerrorA(err));
            } );
#else
            results.errorString = QString::fromLocal8Bit(gai_strerror(err));
#endif
#if defined(QRESOLVER_DEBUG)
            qDebug("QResolverAgent::run(%p): error %d: %s",
                   this, err, results.errorString.latin1());
#endif
        } else {
            QHostAddress *newAddress = 0;
            for (addrinfo *p = res; p != 0; p = p->ai_next) {
                switch (p->ai_family) {
                case AF_INET:
                    newAddress = new QHostAddress(ntohl(((sockaddr_in *) p->ai_addr)->sin_addr.s_addr));
#if defined(QRESOLVER_DEBUG)
                    qDebug("QResolverAgent::run(%p): found IP4 address %s",
                           this, newAddress->toString().latin1());
#endif
                    break;
                case AF_INET6:
                    newAddress = new QHostAddress(((sockaddr_in6 *) p->ai_addr)->sin6_addr.s6_addr);
#if defined(QRESOLVER_DEBUG)
                    qDebug("QResolverAgent::run( %p ): found IP6 address %s",
                           this, newAddress->toString().latin1());
#endif
                    break;
                default:
                    results.error = QResolver::UnknownError;
                    results.errorString = "Unknown address type";
                    break;
                }

                if (newAddress) {
                    if (!results.addresses.contains(*newAddress))
                        results.addresses << *newAddress;
                    delete newAddress;
                    newAddress = 0;
                }
            }
            freeaddrinfo(res);
        }
    } else {
        // fall back to gethostbyname
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
	    case AF_INET6:
		for (p = ent->h_addr_list; *p != 0; p++)
		    results.addresses << QHostAddress((Q_UINT8 *) *p);
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

