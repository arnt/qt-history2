#include "qobjectcleanuphandler.h"
#include "qobjectlist.h"

QObjectCleanupHandler::QObjectCleanupHandler() : QObject(), cleanupObjects( 0 ) {}
QObjectCleanupHandler::~QObjectCleanupHandler() { clear(); }

QObject* QObjectCleanupHandler::add( QObject* object ) {
    if ( !cleanupObjects ) {
	cleanupObjects = new QObjectList;
 	cleanupObjects->setAutoDelete( TRUE );
    }
    connect( object, SIGNAL(destroyed()), this, SLOT(objectDestroyed()) );
    cleanupObjects->insert( 0, object );
    return object;
}

void QObjectCleanupHandler::remove( QObject *object ) {
    if ( !cleanupObjects )
	return;
    if ( cleanupObjects->findRef( object ) >= 0 )
	(void) cleanupObjects->take();
}

bool QObjectCleanupHandler::isEmpty() const {
    return cleanupObjects ? cleanupObjects->isEmpty() : TRUE;
}

void QObjectCleanupHandler::clear() {
    delete cleanupObjects;
    cleanupObjects = 0;
}

void QObjectCleanupHandler::objectDestroyed()
{
    if ( cleanupObjects )
	cleanupObjects->setAutoDelete( FALSE );

    QObject *object = (QObject*)sender();
    remove( object );

    if ( cleanupObjects )
	cleanupObjects->setAutoDelete( TRUE );
}
