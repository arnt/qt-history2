#include "qresolver_p.h"
#include <qsocketdevice.h>
#include <qt_windows.h>

/*****************************************************************
 *
 * QResolverManager
 *
 *****************************************************************/
uint QResolverManager::winMsg = 0;

QResolverManager::QResolverManager()
    : QWidget( 0, "DNS manager" ),
#if !defined(QT_NO_IPV6)
      resolverThread(0),
#endif
      requestHandle(0),
      buf(0)
{
    QSocketDevice dev; // ### hack to make sure that WSAStartup() was called
}

QResolverManager::~QResolverManager()
{
#if !defined(QT_NO_IPV6)
    if ( resolverThread ) {
	if ( resolverThread->running() ) {
	    resolverThread->terminate();
	    resolverThread->wait();
	}
	delete resolverThread;
    }
#endif
    delete[] buf;
}

void QResolverManager::startNextQuery()
{
    QResolverDispatcher *dispatcher = queries.first();
    if ( dispatcher ) {
#if !defined(QT_NO_IPV6)
	if ( qWinVersion() == WV_XP ) {
	    if ( !resolverThread )
		resolverThread = new QResolverSync;
	    resolverThread->hostName = dispatcher->hostName;
#  ifdef QT_THREAD_SUPPORT
	    resolverThread->start();
#  else
	    resolverThread->run();
#  endif
	    return;
	}
#endif

	if ( !winMsg )
	    winMsg = RegisterWindowMessageA( "QtGetHostByNameEvent" );
	if ( !buf )
	    buf = new char[MAXGETHOSTSTRUCT];
	requestHandle = WSAAsyncGetHostByName( winId(), winMsg, dispatcher->hostName.latin1(), buf, MAXGETHOSTSTRUCT );
	if ( requestHandle == 0 ) {
	    QResolver::HostInfo resData;
	    resData.error = QResolver::UnknownError;
	    resData.errorString = tr( "Starting host name lookup failed" );
	    emit dispatcher->resultsReady( resData );
	    queries.removeFirst();
	    startNextQuery();
	    return;
	}
    }
}

bool QResolverManager::winEvent( MSG *msg )
{
    // ### The windows event handling could be rather done in qapplication like
    // for socket notifiers. This means that the same widget as for socket
    // notifiers could be used and that the QResolverManager does not need
    // to be a QWidget subclass.

#if defined(QRESOLVER_DEBUG)
    qDebug( "QResolverManager::winEvent( %p %p ): event of type %d (%d)", this, msg, msg->message, winMsg );
#endif
    if ( winMsg && msg->message == winMsg && (HANDLE)msg->wParam == requestHandle ) {
	QResolver::HostInfo hostInfo;
	if ( WSAGETASYNCERROR( msg->lParam ) == 0 ) {
	    char **p;
	    for ( p = ((hostent*)buf)->h_addr_list; *p!=0; p++ ) {
		long *ip4Addr = (long*)*p;
		hostInfo.addresses << QHostAddress( ntohl( *ip4Addr ) );
#if defined(QRESOLVER_DEBUG)
		qDebug( "QResolverSync::run( %p ): found IP4 address %s",
			this, hostInfo.addresses.last().toString().latin1() );
#endif
	    }
	} else {
	    if ( WSAGETASYNCERROR( msg->lParam ) == WSAHOST_NOT_FOUND ) {
		hostInfo.error = QResolver::HostNotFound;
		hostInfo.errorString = tr( "Host not found" );
	    } else {
		hostInfo.error = QResolver::UnknownError;
		hostInfo.errorString = tr( "Unknown error" );
	    }
	}
	QResolverDispatcher *dispatcher = queries.first();
	if ( dispatcher ) {
	    emit dispatcher->resultsReady( hostInfo );
	    queries.removeFirst();
	    startNextQuery();
	    return TRUE;
	}
    }
#if defined(QRESOLVER_DEBUG)
    qDebug( "QResolverManager::winEvent( %p %p ): event not recognized, pass it to QObject::event()", this, msg );
#endif
    return QWidget::winEvent( msg );
}
