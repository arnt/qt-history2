#include "qresolver_p.h"

#if defined QT_NO_IPV6
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netdb.h>
#endif

/*****************************************************************
 *
 * QResolverSync
 *
 *****************************************************************/
#if defined QT_NO_IPV6
void QResolverSync::run()
{
#if defined(QRESOLVER_DEBUG)
    qDebug( "QResolverSync::run( %p ): start DNS lookup", this );
#endif
    hostent *ent = gethostbyname( hostName.latin1() );
    QResolverEvent *event = new QResolverEvent();
    if ( ent ) {
	char **p;
	switch ( ent->h_addrtype ) {
	    case AF_INET:
		for ( p = ent->h_addr_list; *p!=0; p++ ) {
		    long *ip4Addr = (long*)*p;
		    event->resData.addresses << QHostAddress( ntohl( *ip4Addr ) );
#if defined(QRESOLVER_DEBUG)
		    qDebug( "QResolverSync::run( %p ): found IP4 address %s",
			    this, event->resData.addresses.last().toString().latin1() );
#endif
		}
		break;
	    case AF_INET6:
		for ( p = ent->h_addr_list; *p!=0; p++ ) {
		    event->resData.addresses << QHostAddress( (Q_UINT8*)*p );
#if defined(QRESOLVER_DEBUG)
		    qDebug( "QResolverSync::run( %p ): found IP6 address %s",
			    this, event->resData.addresses.last().toString().latin1() );
#endif
		}
		break;
	    default:
		event->resData.error = QResolver::UnknownError;
		event->resData.errorString = "Unknown address type";
		break;
	}
    } else {
	switch ( h_errno ) {
	    case HOST_NOT_FOUND:
		event->resData.error = QResolver::HostNotFound;
		break;
	    case NO_DATA:
		// no error
		break;
	    default:
		event->resData.error = QResolver::UnknownError;
		break;
	}
	event->resData.errorString = QString::fromLocal8Bit( hstrerror( h_errno) );
#if defined(QRESOLVER_DEBUG)
	qDebug( "QResolverSync::run( %p ): error #%d %s",
		this, h_errno, event->resData.errorString.latin1() );
#endif
    }
#if defined(QRESOLVER_DEBUG)
    qDebug( "QResolverSync::run( %p ): posting results", this );
#endif

#ifdef QT_THREAD_SUPPORT
    QApplication::postEvent( QResolverManager::manager(), event );
#else
    QResolverManager::manager()->event( event );
#endif
}
#endif // QT_NO_IPV6

/*****************************************************************
 *
 * QResolverManager
 *
 *****************************************************************/
QResolverManager::QResolverManager()
    : QObject( 0, "DNS manager" )
{
    resolverThread = new QResolverSync;
}

QResolverManager::~QResolverManager()
{
#ifdef QT_THREAD_SUPPORT
    if (resolverThread->isRunning()) {
	resolverThread->terminate();
	resolverThread->wait();
    }
#endif
    delete resolverThread;
}

void QResolverManager::startNextQuery()
{
    if (queries.size() == 0)
	return;

    QResolverDispatcher *dispatcher = queries.first();
    if ( dispatcher ) {
	resolverThread->hostName = dispatcher->hostName;
#ifdef QT_THREAD_SUPPORT
	resolverThread->start();
#else
	resolverThread->run();
#endif
    }
}
