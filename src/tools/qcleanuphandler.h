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

    Type* add( Type **object ) {
	if ( !cleanupObjects )
	    cleanupObjects = new QPtrList<Type*>;
	cleanupObjects->insert( 0, object );
	return *object;
    }

    void remove( Type **object ) {
	if ( !cleanupObjects )
	    return;
	if ( cleanupObjects->findRef( object ) >= 0 )
	    (void) cleanupObjects->take();
    }

    bool isEmpty() const {
	return cleanupObjects ? cleanupObjects->isEmpty() : TRUE;
    }

    void clear() {
	if ( !cleanupObjects )
	    return;
	QPtrListIterator<Type*> it( *cleanupObjects );
	Type **object;
	while ( ( object = it.current() ) ) {
	    delete *object;
	    *object = 0;
	    cleanupObjects->remove( object );
	}
	delete cleanupObjects;
	cleanupObjects = 0;
    }

private:
    QPtrList<Type*> *cleanupObjects;
};

template<class Type>
class Q_EXPORT QSingleCleanupHandler
{
public:
    QSingleCleanupHandler() : object( 0 ) {}
    ~QSingleCleanupHandler() {
	if ( object ) {
	    delete *object;
	    *object = 0;
	}
    }
    Type* set( Type **o ) {
	object = o;
	return *object;
    }
    void reset() { object = 0; }
private:
    Type **object;
};

template<class Type>
class Q_EXPORT QSharedCleanupHandler
{
public:
    QSharedCleanupHandler() : object( 0 ) {}
    ~QSharedCleanupHandler() {
	if ( object ) {
	    if ( (*object)->deref() )
		delete *object;
	    *object = 0;
	}
    }
    Type* set( Type **o ) {
	object = o;
	(*object)->ref();
	return *object;
    }
    void reset() { object = 0; }
private:
    Type **object;
};

#endif //QCLEANUPHANDLER_H
