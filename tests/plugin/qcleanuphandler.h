#ifndef QCLEANUPHANDLER_H
#define QCLEANUPHANDLER_H

#include <qlist.h>
#include <qguardedptr.h>

template<class Type>
class QCleanUpHandler
{
public:
    ~QCleanUpHandler()
    {
	QListIterator<QGuardedPtr<Type> > it( cleanUpObjects );
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    ++it;
	    delete *guard;
	}
    }

    void addCleanUp( Type* object ) {
	cleanUpObjects.insert( 0, new QGuardedPtr<Type>(object) );
    }

    bool clean() {
	QListIterator<QGuardedPtr<Type> > it( cleanUpObjects );
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    ++it;
	    if ( *guard )
		return FALSE;
	}
	return TRUE;
    }

protected:
    QList<QGuardedPtr<Type> > cleanUpObjects;
};

#endif //QCLEANUPHANDLER_H
