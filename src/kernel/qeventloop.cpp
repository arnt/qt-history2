/****************************************************************************
** $Id$
**
** Implementation of QEventLoop class
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qeventloop_p.h" // includes qplatformdefs.h
#include "qeventloop.h"
#include "qdatetime.h"

static QEventLoop *INSTANCE = 0;


/*!
    Creates a QEventLoop object.
*/
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

/*!
    Destructs the QEventLoop object.
*/
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

/*!
    Enters the main event loop and waits until exit() is called or the
    main widget is destroyed, and returns the value that was passed to
    exit() (which is zero if exit() is called via quit()).
*/
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

/*!
    Tells the event loop to exit with a return code.
 */
void QEventLoop::exit( int retcode )
{
    if ( d->quitnow ) // preserve existing quitcode
	return;
    d->quitcode = retcode;
    d->quitnow = true;
    d->exitloop = true;
}

/*!
    This function enters the event loop (recursively).  Do not call
    this function unless you really know what you are doing.
 */
int QEventLoop::enterLoop()
{
    // save the current exitloop state
    bool old_exitloop = d->exitloop;
    d->exitloop = FALSE;

    d->looplevel++;
    while ( ! d->exitloop )
	processNextEvent( AllEvents, TRUE );
    d->looplevel--;

    // restore the exitloop state, but if quitnow is true, we need to keep
    // exitloop set so that all other event loops drop out.
    d->exitloop = old_exitloop || d->quitnow;

    return d->looplevel;
}

/*!
    This function exits from a recursive call to the event loop.  Do
    not call this function unless you really know what you are doing.
 */
void QEventLoop::exitLoop()
{
    d->exitloop = TRUE;
}

/*!

 */
int QEventLoop::loopLevel() const
{
    return d->looplevel;
}

/*!
  process one, and exactly one event, waiting for an event if necessary

  calls processNextEvent( flags, TRUE );
*/
void QEventLoop::processOneEvent( ProcessEventsFlags flags )
{
    (void) processNextEvent( flags, TRUE );
}

/*!

   process pending events that match \a eventTypes for a maximum of \a
   maxtime milliseconds, or until there are no more events to process,
   which ever is shorter. if this function is called without any
   arguments, then all event types are processed for a maximum of 3
   seconds (3000 milliseconds).

*/
void QEventLoop::processEvents( ProcessEventsFlags flags, int maxTime )
{
    QTime start = QTime::currentTime();
    QTime now;
    while ( ! d->quitnow && processNextEvent( flags, FALSE ) ) {
	now = QTime::currentTime();
	if ( start.msecsTo( now ) > maxTime )
	    break;
    }
}

/*! \fn bool QEventLoop::hasPendingEvents()

    Returns true if there is an event waiting, otherwise it returns false.
*/

/*! \fn void QEventLoop::registerSocketNotifier( QSocketNotifier *notifier )

    registers the given socket notifier with the event loop.  subclasses
    need to reimplement this method to tie a socket notifier into another
    event loop.  reimplementations MUST call the base implementation.
*/

/*! \fn void QEventLoop::unregisterSocketNotifier( QSocketNotifier *notifier )

    unregisters the given socket notifier from the event loop.  subclasses
    need to reimplement this method to tie a socket notifier into another
    event loop.  reimplementations MUST call the base implementation.
*/

/*! \fn void QEventLoop::setSocketNotifierPending( QSocketNotifier *notifier )

    marks the given socket notifier as pending.  the socket notifier will
    be activated the next time activateSocketNotifiers() is called.
*/

/*! \fn int QEventLoop::activateSocketNotifiers()

    Activates all pending socket notifiers and returns the number of socket
    notifiers that were activated.
*/

/*! \fn int QEventLoop::activateTimers()

    Activates all Qt timers and returns the number of timers that were activated.

    QEventLoop subclasses that do their own timer handling need to call
    this after the time returned by timeToWait() has elapsed.
*/

/*! \fn int QEventLoop::timeToWait() const

    Returns the number of milliseconds that Qt needs to handle its timers or
    -1 if there are no timers running.

    QEventLoop subclasses that do their own timer handling need to
    use this to make sure that Qt's timers continue to work. returns
*/

/*! \fn int QEventLoop::exec()

    Enters the main event loop and waits until exit() is called, and
    returns the value that was set to exit().

    It is necessary to call this function to start event handling. The
    main event loop receives events from the window system and
    dispatches these to the application widgets.

    Generally speaking, no user interaction can take place before
    calling exec(). As a special case, modal widgets like QMessageBox
    can be used before calling exec(), because modal widgets call
    exec() to start a local event loop.

    To make your application perform idle processing, i.e. executing a
    special function whenever there are no pending events, use a
    QTimer with 0 timeout. More advanced idle processing schemes can
    be achieved using processEvents().

    \sa quit(), exit(), processEvents()
*/

/*! \fn void QEventLoop::exit( int retcode = 0 )

    Tells the event loop to exit with a return code.

    After this function has been called, the event loop returns from
    the call to exec(). The exec() function returns \a retcode.

    By convention, a \a retcode of 0 means success, and any non-zero
    value indicates an error.

    Note that unlike the C library function of the same name, this
    function \e does return to the caller -- it is event processing that
    stops.

    \sa quit(), exec()
*/

/*! \fn int QEventLoop::enterLoop()

    This function enters the main event loop (recursively). Do not call
    it unless you really know what you are doing.
 */

/*! \fn void QEventLoop::exitLoop()

    This function exits from a recursive call to the main event loop.
    Do not call it unless you really know what you are doing.
*/

/*! \fn void QEventLoop::loopLevel() const

    Returns the current loop level.
*/


/*! \fn void QEventLoop::wakeUp()

    Wakes up the event loop.  This function is thread safe, and can be
    called by any running thread.
*/

/*! \fn QMutex *QEventLoop::mutex() const

    Returns the Qt library mutex.  Do not call this function unless you
    really know what you are doing.

*/

/*! \fn void QEventLoop::awake()

    This signal is emitted just before the event loop begins processing
    newly arrived events.
*/

/*! \fn void QEventLoop::aboutToBlock()

    This signal is emitted just before the event loop calls a function
    that could block.
*/

/*! \fn bool QEventLoop::processNextEvent( ProcessEventsFlags flags, bool canWait )

    Processes the next event received that matches \a flags. If no
    events matching \a flags are available, this function will wait
    for the next event if \a canWait is true, otherwise it returns
    immediately.

    Returns TRUE if an event was processed, otherwise returns FALSE.
*/

/*! \fn int QEventLoop::macHandleSelect( timeval * )
    \internal
*/

/*! \fn void QEventLoop::macHandleTimer( TimerInfo * )
    \internal
*/
