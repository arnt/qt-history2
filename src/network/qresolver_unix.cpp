#include "qresolver_p.h"

#if defined QT_NO_IPV6
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netdb.h>
#endif

void QResolverAgent::run()
{
#if defined(QRESOLVER_DEBUG)
    qDebug("QResolverAgent::run(%p): start DNS lookup", this);
#endif
    hostent *ent = gethostbyname(hostName.latin1());
    QResolverHostInfo results;

    if (ent) {
        char **p;
        switch (ent->h_addrtype) {
            case AF_INET:
		for (p = ent->h_addr_list; *p != 0; p++) {
		    long *ip4Addr = (long *) *p;
		    results.addresses << QHostAddress(ntohl(*ip4Addr));
#if defined(QRESOLVER_DEBUG)
		    qDebug( "QResolverAgent::run( %p ): found IP4 address %s",
			    this, results.addresses.last().toString().latin1() );
#endif
		}
		break;
	    case AF_INET6:
		for (p = ent->h_addr_list; *p != 0; p++) {
		    results.addresses << QHostAddress((Q_UINT8 *) *p);
#if defined(QRESOLVER_DEBUG)
		    qDebug( "QResolverAgent::run( %p ): found IP6 address %s",
			    this, results.addresses.last().toString().latin1() );
#endif
		}
		break;
	    default:
		results.error = QResolver::UnknownError;
		results.errorString = "Unknown address type";
		break;
	}
    } else {
       switch (h_errno) {
	    case HOST_NOT_FOUND:
		results.error = QResolver::HostNotFound;
		break;
	    case NO_DATA:
		// no error
		break;
	    default:
		results.error = QResolver::UnknownError;
		break;
	}
	results.errorString = QString::fromLocal8Bit(hstrerror(h_errno));
#if defined(QRESOLVER_DEBUG)
	qDebug( "QResolverAgent::run( %p ): error #%d %s",
		this, h_errno, results.errorString.latin1() );
#endif
    }

    emit resultsReady(results);

#if !defined QT_NO_THREAD
    connect(this, SIGNAL(terminated()), SLOT(deleteLater()));
#else
    deleteLater();
#endif

}
