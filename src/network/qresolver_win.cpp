#include "qresolver_p.h"
#include <winsock2.h>
#include <ws2tcpip.h>

//#define QRESOLVER_DEBUG

#include <qtimer.h>

void QResolverAgent::run()
{
#if defined(QRESOLVER_DEBUG)
    qDebug("QResolverAgent::run(%p): start DNS lookup", this);
#endif
    struct addrinfo *res;
    int err = getaddrinfo(hostName.latin1(), 0, 0, &res);
    QResolverHostInfo results;

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
	for (struct addrinfo *p=res; p != 0; p = p->ai_next) {
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

    emit resultsReady(results);

#if !defined QT_NO_THREAD
    connect(this, SIGNAL(terminated()), SLOT(deleteLater()));
#else
    deleteLater();
#endif
}

