#include "qeventloop.h"
#include "qeventloop_p.h"

static QEventLoop *INSTANCE = 0;


QEventLoop::QEventLoop( QObject *parent, const char *name )
    : QObject( parent, name )
{
#if defined(QT_CHECK_STATE)
    if ( INSTANCE )
	qFatal( "QEventLoop: there should only be one event loop object." );
    // for now ;)
#endif // QT_CHECK_STATE

    d = new QEventLoopPrivate;

    init();

    INSTANCE = this;
}

QEventLoop::~QEventLoop()
{
    cleanup();
    delete d;
    INSTANCE = 0;
}

#if defined(QT_THREAD_SUPPORT)
QMutex *QEventLoop::mutex() const
{
    return &d->mutex;
}
#endif // QT_THREAD_SUPPORT

int QEventLoop::exec()
{
    d->reset();

    enterLoop();

    // cleanup
    d->looplevel = 0;
    d->quitnow = FALSE;
    d->exitloop = FALSE;
    // don't reset quitcode!

    return d->quitcode;
}

void QEventLoop::exit( int retcode )
{
    if ( d->quitnow ) // preserve existing quitcode
	return;
    d->quitcode = retcode;
    d->quitnow = true;
    d->exitloop = true;
}

int QEventLoop::enterLoop()
{
    // save the current exitloop state
    bool old_exitloop = d->exitloop;

    d->looplevel++;
    while ( ! d->exitloop )
	processNextEvent( All, TRUE );
    d->looplevel--;

    // restore the exitloop state, but if quitnow is true, we need to keep
    // exitloop set so that all other event loops drop out.
    d->exitloop = old_exitloop || d->quitnow;

    return d->looplevel;
}

void QEventLoop::exitLoop()
{
    d->exitloop = TRUE;
}

int QEventLoop::loopLevel() const
{
    return d->looplevel;
}

void QEventLoop::processOneEvent( int eventType )
{
    (void) processNextEvent( eventType, TRUE );
}
