#ifndef QCLEANUPHANDLER_H
#define QCLEANUPHANDLER_H

#include <qlist.h>
#include <qguardedptr.h>
#include <qpixmap.h>

template<class Type>
class Q_EXPORT QCleanUpHandler
{
public:
    ~QCleanUpHandler()
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

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QList<QPixmap>;
// MOC_SKIP_END
#endif

class Q_EXPORT QCleanUpHandler<QPixmap>
{
public:
    ~QCleanUpHandler()
    {
	QListIterator<QPixmap> it( cleanUpObjects );
	while ( it.current() ) {
	    QPixmap* object = it.current();
	    ++it;
	    delete object;
	}
    }

    void addCleanUp( const QPixmap* pixmap ) 
    {
	cleanUpObjects.insert( 0, pixmap );
    }

    bool clean() 
    {
	return !cleanUpObjects.count();
    }

private:
    QList<QPixmap> cleanUpObjects;
};

#endif //QCLEANUPHANDLER_H
