#include "qresolver_p.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <qlibrary.h>

void QResolverAgent::run()
{
#if defined(QRESOLVER_DEBUG)
    qDebug("QResolverAgent::run(%p): start DNS lookup", this);
#endif

    QResolverHostInfo results;

#if !defined (QT_NO_GETADDRINFO)
    addrinfo hints;
    addrinfo *res = 0;
    int result = getaddrinfo(hostName.latin1(), 0, 0, &res);
    if (result == 0) {
        addrinfo *node = res;
        while (node) {
            if (node->ai_family == AF_INET) {
                sockaddr_in *ip4 = (sockaddr_in *)node->ai_addr;
                QHostAddress addr(ntohl(ip4->sin_addr.s_addr));
                if (!results.addresses.contains(addr))
                    results.addresses.prepend(addr);
            } else if (node->ai_family == AF_INET6) {
                sockaddr_in6 *ip6 = (sockaddr_in6 *)node->ai_addr;
                Q_IPV6ADDR tmp;
                memcpy(&tmp, &ip6->sin6_addr.s6_addr, sizeof(tmp));
                QHostAddress addr(tmp);
                if (!results.addresses.contains(addr))
                    results.addresses.append(addr);
            } else {
		results.error = QResolver::UnknownError;
		results.errorString = tr("Unknown address type");
		break;
            }
            node = node->ai_next;
        }
       
        freeaddrinfo(res);
    } else {
        switch (result) {
        case EAI_NONAME:
            results.errorString = tr("Host not found");
            results.error = QResolver::HostNotFound;
            break;          
        case EAI_NODATA:
            // no error
            break;
        default:
            results.errorString = tr("Unknown error");
            results.error = QResolver::UnknownError;
            break;
        };
    }
#else
    char auxbuf[512];    
    hostent ent;
    hostent *result;
    int err;
    if (gethostbyname_r(hostName.latin1(), &ent, auxbuf, sizeof(auxbuf), &result, &err) == 0) {
        char **p;
        switch (ent.h_addrtype) {
            case AF_INET:
		for (p = ent.h_addr_list; *p != 0; p++) {
		    long *ip4Addr = (long *) *p;
		    results.addresses << QHostAddress(ntohl(*ip4Addr));
		}
		break;
	    case AF_INET6:
		for (p = ent.h_addr_list; *p != 0; p++)
		    results.addresses << QHostAddress((Q_UINT8 *) *p);
		break;
	    default:
		results.error = QResolver::UnknownError;
		results.errorString = tr("Unknown address type");
		break;
	}
    } else {
       switch (h_errno) {
	    case HOST_NOT_FOUND:
		results.error = QResolver::HostNotFound;
                results.errorString = tr("Host not found");
		break;
	    case NO_DATA:
		// no error
		break;
	    default:
		results.error = QResolver::UnknownError;
                results.errorString = tr("Unknown error");
		break;
	}
	results.errorString = QString::fromLocal8Bit(hstrerror(h_errno));
#if defined(QRESOLVER_DEBUG)
	qDebug( "QResolverAgent::run( %p ): error #%d %s",
		this, h_errno, results.errorString.latin1() );
#endif
    }
#endif

    emit resultsReady(results);    
    
#if !defined QT_NO_THREAD
    connect(this, SIGNAL(terminated()), SLOT(deleteLater()));
#else
    deleteLater();
#endif

}
