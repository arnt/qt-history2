#ifndef QCLEANUPHANDLER_H
#define QCLEANUPHANDLER_H

#include <qlist.h>
#include <qguardedptr.h>

template<class Type>
class Q_EXPORT QGuardedCleanUpHandler
{
public:
    ~QGuardedCleanUpHandler()
    {
	QListIterator<QGuardedPtr<Type> > it( cleanUpObjects );
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    ++it;
	    delete (Type*)*guard;
	}
    }

    void addCleanUp( Type* object ) 
    {
	cleanUpObjects.insert( 0, new QGuardedPtr<Type>(object) );
    }

    bool clean() 
    {
	QListIterator<QGuardedPtr<Type> > it( cleanUpObjects );
	while ( it.current() ) {
	    QGuardedPtr<Type>* guard = it.current();
	    ++it;
	    if ( *guard )
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
	while ( it.current() ) {
	    Type* object = it.current();
	    ++it;
	    qDebug("Cleaning up object %p", object );
	    delete object;
	}
    }

    void addCleanUp( Type* object ) 
    {
	cleanUpObjects.insert( 0, object );
	qDebug("Cleanup object %p added", object );
    }

    bool clean() 
    {
	QListIterator<Type> it( cleanUpObjects );
	while ( it.current() ) {
	    Type* object = it.current();
	    ++it;
	    if ( *guard )
		return FALSE;
	}
	return TRUE;
    }

private:
    QList<Type> cleanUpObjects;
};

#endif //QCLEANUPHANDLER_H
