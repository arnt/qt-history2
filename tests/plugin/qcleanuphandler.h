#ifndef QCLEANUPHANDLER_H
#define QCLEANUPHANDLER_H

template<class Type>
class QCleanUpHandler
{
public:
    QCleanUpHandler() 
    {
	qDebug( "New Cleanup handler %p", this );
    }
    ~QCleanUpHandler()
    { 
	qDebug( "Running cleanup...  %p (%d)", this, cleanUpObjects.count() );
	QListIterator<Type> it( cleanUpObjects );
	while ( it.current() ) {
	    Type* object = it.current();
	    ++it;
	    qDebug( "\tDeleting object %p", object );
	    delete object;
	}
    }

    void addCleanUp( Type* object ) {
	qDebug( "\tCleanup object %p added", object );
	cleanUpObjects.insert( 0, object );
    }

protected:
    QList<Type> cleanUpObjects;
};

#endif //QCLEANUPHANDLER_H