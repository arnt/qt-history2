#ifndef QCLEANUPHANDLER_H
#define QCLEANUPHANDLER_H

#ifndef QT_H
#include <qptrlist.h>
#endif // QT_H

template<class Type>
class Q_EXPORT QCleanupHandler
{
public:
    QCleanupHandler() : cleanupObjects( 0 ) {}
    ~QCleanupHandler() { clear(); }

    Type* add( Type* object ) {
	if ( !cleanupObjects ) {
	    cleanupObjects = new QPtrList<Type>;
 	    cleanupObjects->setAutoDelete( TRUE );
	}
	cleanupObjects->insert( 0, object );
	return object;
    }

    void remove( Type *object ) {
	if ( !cleanupObjects )
	    return;
	if ( cleanupObjects->findRef( object ) >= 0 )
	    (void) cleanupObjects->take();
    }

    bool isEmpty() const {
	return cleanupObjects ? cleanupObjects->isEmpty() : TRUE;
    }

    void clear() {
	delete cleanupObjects;
	cleanupObjects = 0;
    }

private:
    QPtrList<Type> *cleanupObjects;
};

#endif //QCLEANUPHANDLER_H
