#include "qobjectcleanuphandler.h"
#include "qobjectlist.h"

QObjectCleanupHandler::QObjectCleanupHandler() : QObject(), cleanupObjects( 0 ) {}
QObjectCleanupHandler::~QObjectCleanupHandler() { clear(); }

QObject* QObjectCleanupHandler::add( QObject* object ) {
    if ( !cleanupObjects ) {
	cleanupObjects = new QObjectList;
 	cleanupObjects->setAutoDelete( TRUE );
    }
    connect( object, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)) );
    cleanupObjects->insert( 0, object );
    return object;
}

void QObjectCleanupHandler::remove( QObject *object ) {
    if ( !cleanupObjects )
	return;
    if ( cleanupObjects->findRef( object ) >= 0 ) {
	(void) cleanupObjects->take();
	disconnect( object, SIGNAL(destroyed( QObject* )), this, SLOT(objectDestroyed( QObject* )) );
    }
}

bool QObjectCleanupHandler::isEmpty() const {
    return cleanupObjects ? cleanupObjects->isEmpty() : TRUE;
}

void QObjectCleanupHandler::clear() {
    delete cleanupObjects;
    cleanupObjects = 0;
}

void QObjectCleanupHandler::objectDestroyed( QObject*object )
{
    if ( cleanupObjects )
	cleanupObjects->setAutoDelete( FALSE );

    remove( object );

    if ( cleanupObjects )
	cleanupObjects->setAutoDelete( TRUE );
}
