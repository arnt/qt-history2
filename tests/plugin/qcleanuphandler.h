#ifndef QCLEANUPHANDLER_H
#define QCLEANUPHANDLER_H

class QCleanUpHandler
{
public:
    typedef void (*CleanUpProc)();

    QCleanUpHandler() 
    {
	qDebug( "New Cleanup handler %p", this );
    }
    ~QCleanUpHandler() 
    { 
	qDebug( "Running cleanup...  %p (%d)", this, cleanUpRoutines.count() );
	while ( CleanUpProc* rt = cleanUpRoutines.first() ) {
	    qDebug( "\tcalling %p", rt );
	    (*rt)();
	    cleanUpRoutines.remove();
	    if ( cleanUpRoutines.first() == cleanUpRoutines.last() )
		break;
	}
    }

    void addCleanUpRoutine( CleanUpProc rt ) {
	cleanUpRoutines.append( (const CleanUpProc*)&rt );
    }

protected:
    QList<CleanUpProc> cleanUpRoutines;
};

static QCleanUpHandler cleanUpHandler;

#endif //QCLEANUPHANDLER_H