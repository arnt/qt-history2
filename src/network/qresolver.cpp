#include "qresolver_p.h"

const int QResolverEvent::ResolverEventType=2323;

#if !defined(QT_NO_IPV6)
#if defined(Q_WS_WIN)
#    include <winsock2.h>
#    include <ws2tcpip.h>
#  else
#    include <sys/types.h>
#    include <sys/socket.h>
#    include <netdb.h>
#    include <netinet/in.h>
#  endif
#endif // !defined(QT_NO_IPV6)

/*****************************************************************
 *
 * QResolverSync
 *
 *****************************************************************/
#if !defined(QT_NO_IPV6)
void QResolverSync::run()
{
#if defined(QRESOLVER_DEBUG)
    qDebug( "QResolverSync::run( %p ): start DNS lookup", this );
#endif
    struct addrinfo *res;
    int err = ::getaddrinfo( hostName.latin1(), 0, 0, &res );
    QResolverEvent *event = new QResolverEvent();
    if ( err ) {
	switch ( err ) {
	    case EAI_NONAME:
		event->resData.error = QResolver::HostNotFound;
		break;
	    default:
		event->resData.error = QResolver::UnknownError;
		break;
	}
#if defined(Q_WS_WIN)
	QT_WA( {
	    event->resData.errorString = QString::fromUcs2( (ushort*)gai_strerrorW( err ) );
	} , {
	    event->resData.errorString = QString::fromLocal8Bit( gai_strerrorA( err ) );
	} );
#else
	event->resData.errorString = QString::fromLocal8Bit( gai_strerror( err ) );
#endif
#if defined(QRESOLVER_DEBUG)
	qDebug( "QResolverSync::run( %p ): error %d: %s",
		this, err, event->resData.errorString.latin1() );
#endif
    } else {
	QHostAddress *newAddress = 0;
	for ( struct addrinfo *p=res; p!=0; p = p->ai_next ) {
	    switch ( p->ai_family ) {
		case AF_INET:
		    newAddress = new QHostAddress( ntohl( ((sockaddr_in*)p->ai_addr)->sin_addr.s_addr ) );
#if defined(QRESOLVER_DEBUG)
		    qDebug( "QResolverSync::run( %p ): found IP4 address %s",
			    this, event->resData.addresses.last().toString().latin1() );
#endif
		    break;
		case AF_INET6:
		    newAddress = new QHostAddress( ((sockaddr_in6*)p->ai_addr)->sin6_addr.s6_addr );
#if defined(QRESOLVER_DEBUG)
		    qDebug( "QResolverSync::run( %p ): found IP6 address %s",
			    this, event->resData.addresses.last().toString().latin1() );
#endif
		    break;
		default:
		    event->resData.error = QResolver::UnknownError;
		    event->resData.errorString = "Unknown address type";
		    break;
	    }
	    if ( newAddress ) {
		if (!event->resData.addresses.contains( *newAddress ))
		    event->resData.addresses << *newAddress;
		delete newAddress;
		newAddress = 0;
	    }
	}
	freeaddrinfo( res );
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
#endif // !defined(QT_NO_IPV6)

/*****************************************************************
 *
 * QResolverManager
 *
 *****************************************************************/
static QResolverManager *globalManager = 0;

static void cleanupResolver()
{
    delete globalManager;
    globalManager = 0;
}

QResolverManager *QResolverManager::manager()
{
    if ( !globalManager ) {
        qAddPostRoutine(cleanupResolver);
	globalManager = new QResolverManager();
    }
    return globalManager;
}

void QResolverManager::addQuery( const QString& name, const QObject * receiver, const char *resultsReady )
{
    QResolverDispatcher *dispatcher = new QResolverDispatcher( this, name );
    QObject::connect( dispatcher, SIGNAL(resultsReady(const QResolver::HostInfo&)),
	    receiver, resultsReady );
    queries.append( dispatcher );
    if ( queries.count() == 1 )
	startNextQuery();
}

bool QResolverManager::event( QEvent *e )
{
#if defined(QRESOLVER_DEBUG)
    qDebug( "QResolverManager::event( %p %p ): event of type %d", this, e, e->type() );
#endif
    if (e->type() == QResolverEvent::ResolverEventType && queries.size() > 0) {
	QResolverDispatcher *dispatcher = queries.takeFirst();
	emit dispatcher->resultsReady( ((QResolverEvent*)e)->resData );
	delete dispatcher;
	startNextQuery();
	return TRUE;
    }
#if defined(QRESOLVER_DEBUG)
    qDebug( "QResolverManager::event( %p %p ): event not recognized, pass it to QObject::event()", this, e );
#endif
    return QObject::event( e );
}

/*****************************************************************
 *
 * QResolver
 *
 *****************************************************************/
void QResolver::getHostByName( const QString& name, const QObject * receiver, const char *resultsReady )
{
    QResolverManager::manager()->addQuery( name, receiver, resultsReady );
}
