#ifndef QCLEANUPHANDLER_H
#define QCLEANUPHANDLER_H

#ifndef QT_H
#include <qlist.h>
#include <qguardedptr.h>
#endif // QT_H

template<class Type>
class Q_EXPORT QGuardedCleanUpHandler
{
public:
    ~QGuardedCleanUpHandler()
    {
	QListIterator<QGuardedPtr<Type> > it( cleanUpObjects );
	it.toLast();
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    --it;
	    delete (Type*)*guard;
	    delete guard;
	}
    }

    void addCleanUp( Type* object )
    {
	cleanUpObjects.insert( 0, new QGuardedPtr<Type>(object) );
    }

    bool isClean()
    {
	QListIterator<QGuardedPtr<Type> > it( cleanUpObjects );
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    ++it;
	    if ( (Type*)*guard )
		return FALSE;
	}
	return TRUE;
    }

private:
    QList<QGuardedPtr<Type> > cleanUpObjects;
};

template<class Type>
class Q_EXPORT QCleanUpHandler
{
public:
    ~QCleanUpHandler()
    {
	QListIterator<Type> it( cleanUpObjects );
	it.toLast();
	while ( it.current() ) {
	    Type* object = it.current();
	    --it;
	    delete object;
	}
    }

    void addCleanUp( Type* object )
    {
	if ( object )
	    cleanUpObjects.insert( 0, object );
    }

    bool isClean()
    {
	return cleanUpObject.isEmpty();
    }

private:
    QList<Type> cleanUpObjects;
};

#endif //QCLEANUPHANDLER_H
