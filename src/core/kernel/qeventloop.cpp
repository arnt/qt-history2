/****************************************************************************
**
** Implementation of QEventLoop class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qeventloop.h"
#include "qeventloop_p.h"
#include "qcoreapplication.h"

#include <qdatetime.h>
#include <qhash.h>
#include <qthread.h>
#include <private/qspinlock_p.h>

#define d d_func()
#define q q_func()

static QStaticSpinLock spinlock = 0;
static int eventloop_count = 0;
static QHash<Qt::HANDLE, QEventLoop *> eventloops;

QEventLoopPrivate::QEventLoopPrivate()
    : QObjectPrivate()
{
    reset();
#if defined(Q_WS_X11)
    xfd = -1;
#endif // Q_WS_X11

    process_event_handler = 0;
    event_filter = 0;
}


// in qcoreapplication.cpp
extern void qt_setEventLoop(QObject *object, QEventLoop *p);


/*!
    \class QEventLoop
    \brief The QEventLoop class manages Qt's event queue.

    \ingroup application
    \ingroup events

    It receives events from the window system and other sources. It
    then sends them to QApplication for processing and delivery.
    QEventLoop provides fine-grained control over event delivery.

    For simple control of event processing use
    QApplication::processEvents().

    For finer control of the application's event loop call
    QApplication::eventLoop() and call functions on the QEventLoop
    object that is returned. If you want to use your own instance of
    QEventLoop or of a QEventLoop subclass, you must create your
    instance \e before you create the QApplication object.

    The event loop is started by calling exec(), and stopped by
    calling exit().

    Programs that perform long operations can call processEvents()
    with various \c ProcessEvents values OR'ed together to control
    which events should be delivered.

    QEventLoop also allows the integration of an external event loop
    with the Qt event loop. For example, the Motif Extension included
    with Qt
\if defined(commercial)
\link commercialeditions.html Enterprise Edition\endlink
\endif
    includes a reimplementation of QEventLoop that merges Qt and Motif
    events together.

*/

/*!
    \enum QEventLoop::ProcessEventHandler
    \internal
*/

/*!
    \enum QEventLoop::EventFilter
    \internal
*/

/*! \enum QEventLoop::ProcessEvents

    This enum controls the types of events processed by the
    processEvents() functions.

    \value AllEvents - All events are processed
    \value ExcludeUserInput - Do not process user input events, such
            as ButtonPress and KeyPress.
    \value ExcludeSocketNotifiers - Do not process socket notifier
           events.
    \value WaitForMore - Wait for events if no pending events
           are available.

    \sa processEvents()
*/

/*! \enum QEventLoop::ProcessEventsFlags
    A \c typedef to allow various ProcessEvents values to be OR'ed together.

    \sa ProcessEvents
 */


/*!
    Creates a QEventLoop object. This object becomes the global event
    loop object. There can only be one event loop object. The
    QEventLoop is usually constructed by calling
    QApplication::eventLoop(). If you want to create your own event
    loop object you \e must create it before you instantiate the
    QApplication object.

    The \a parent argument is passed on to the QObject constructor.
*/
QEventLoop::QEventLoop(QObject *parent)
    : QObject(*new QEventLoopPrivate(), parent)
{
    {
        QSpinLockLocker locker(::spinlock);
        eventloops.ensure_constructed();
        const Qt::HANDLE thr = thread();
        Q_ASSERT_X(!eventloops.contains(thr), "QEventLoop",
                   "Cannot have more than one event loop per thread.");
        eventloop_count++;
        eventloops.insert(thr, this);
    }

    init();
}


/*! \internal
 */
QEventLoop::QEventLoop(QEventLoopPrivate &priv, QObject *parent)
    : QObject(priv, parent)
{
    {
        QSpinLockLocker locker(::spinlock);
        eventloops.ensure_constructed();
        const Qt::HANDLE thr = thread();
        Q_ASSERT_X(!eventloops.contains(thr), "QEventLoop",
                   "Cannot have more than one event loop per thread.");
        eventloop_count++;
        eventloops.insert(thr, this);
    }

    init();
}


/*!
    Destructs the QEventLoop object.
*/
QEventLoop::~QEventLoop()
{
    cleanup();

    {
        QSpinLockLocker locker(::spinlock);
        eventloop_count--;
        eventloops.remove(thread());
    }
}

/*!
    Returns a pointer to the event loop object for the specified \a
    thread. If \a thread is zero, the current thread is used. If no
    event loop exists for the specified \a thread, this function
    returns 0.

    Note: If Qt is built without thread support, the \a thread
    argument is ignored.

    \sa QApplication::eventLoop()
 */
QEventLoop *QEventLoop::instance(Qt::HANDLE thread)
{
    if (thread == 0)
        thread = QThread::currentThread();
    QSpinLockLocker locker(::spinlock);
    if(!eventloop_count)
        return 0;
    eventloops.ensure_constructed();
    return eventloops.value(thread);
}

/*!
    Enters the main event loop and waits until exit() is called.
    Returns the value that was passed to exit().

    It is necessary to call this function to start event handling. The
    main event loop receives events from the window system and
    dispatches these to the application widgets.

    Generally speaking, no user interaction can take place before
    calling exec(). As a special case, modal widgets like QMessageBox
    can be used before calling exec(), because modal widgets call
    use their own local event loop.

    To make your application perform idle processing, i.e. executing a
    special function whenever there are no pending events, use a
    QTimer with 0 timeout. More sophisticated idle processing schemes
    can be achieved using processEvents().

    \sa QApplication::quit(), exit(), processEvents()
*/
int QEventLoop::exec()
{
    d->reset();

    enterLoop();

    // cleanup
    d->looplevel = 0;
    d->quitnow  = false;
    d->exitloop = false;
    d->shortcut = false;
    // don't reset quitcode!

    return d->quitcode;
}

/*! \fn void QEventLoop::exit(int retcode = 0)

    Tells the event loop to exit with a return code.

    After this function has been called, the event loop returns from
    the call to exec(). The exec() function returns \a retcode.

    By convention, a \a retcode of 0 means success, and any non-zero
    value indicates an error.

    Note that unlike the C library function of the same name, this
    function \e does return to the caller -- it is event processing that
    stops.

    \sa QApplication::quit(), exec()
*/
void QEventLoop::exit(int retcode)
{
    if (d->quitnow) // preserve existing quitcode
        return;
    d->quitcode = retcode;
    d->quitnow  = true;
    d->exitloop = true;
    d->shortcut = true;
}


/*! \fn int QEventLoop::enterLoop()

    This function enters the main event loop (recursively). Do not call
    it unless you really know what you are doing.
 */
int QEventLoop::enterLoop()
{
    // save the current exitloop state
    bool old_exitloop = d->exitloop;
    d->exitloop = false;
    d->shortcut = false;

    d->looplevel++;
    while (! d->exitloop)
        processEvents(AllEvents | WaitForMore);
    d->looplevel--;

    // restore the exitloop state, but if quitnow is true, we need to keep
    // exitloop set so that all other event loops drop out.
    d->exitloop = old_exitloop || d->quitnow;
    d->shortcut = d->quitnow;

    if (d->looplevel < 1) {
        d->quitnow  = false;
        d->exitloop = false;
        d->shortcut = false;
        if (this == QCoreApplication::eventLoop())
            emit QCoreApplication::instance()->aboutToQuit();

        // send deferred deletes
        QCoreApplication::sendPostedEvents(0, QEvent::DeferredDelete);
    }

    return d->looplevel;
}

/*! \fn void QEventLoop::exitLoop()

    This function exits from a recursive call to the main event loop.
    Do not call it unless you really know what you are doing.
*/
void QEventLoop::exitLoop()
{
    d->exitloop = true;
    d->shortcut = true;
}

/*! \fn void QEventLoop::loopLevel() const

    Returns the current loop level.
*/
int QEventLoop::loopLevel() const
{
    return d->looplevel;
}

/*!
    Process pending events that match \a flags for a maximum of \a
    maxTime milliseconds, or until there are no more events to
    process, whichever is shorter.

    This function is especially useful if you have a long running
    operation and want to show its progress without allowing user
    input, i.e. by using the \c ExcludeUserInput flag.

    Note: This function does not process events continuously; it
    returns after all available events are processed.

    Note: Specifying the \c WaitForMore flag makes no sense and will
    be ignored.
*/
void QEventLoop::processEvents(ProcessEventsFlags flags, int maxTime)
{
    QTime start;
    start.start();
    while (!d->quitnow && processEvents(flags & ~WaitForMore)) {
        if (start.elapsed() > maxTime)
            break;
    }
}

/*!
    \fn bool QEventLoop::processEvents(ProcessEventsFlags flags)
    \overload

    Processes pending events that match \a flags until there are no
    more events to process.

    This function is especially useful if you have a long running
    operation and want to show its progress without allowing user
    input, i.e. by using the \c ExcludeUserInput flag.

    If the \c WaitForMore flag is set in \a flags, the behavior of
    this function is as follows:

    \list

    \i If events are available, this function returns after processing
    them.

    \i If no events are available, this function will wait until more
    are available and return after processing newly available events.

    \endlist

    If the \c WaitForMore flag is \e not set in \a flags, and no
    events are available, this function will return immediately.

    Note: This function does not process events continuously; it
    returns after all available events are processed.

    This function returns true if an event was processed; otherwise it
    returns false.

    \sa ProcessEvents hasPendingEvents()
*/

/*! \fn bool QEventLoop::hasPendingEvents() const

    Returns true if there is an event waiting; otherwise returns
    false.
*/

/*! \fn void QEventLoop::registerSocketNotifier(QSocketNotifier *notifier)

    Registers \a notifier with the event loop. Subclasses must
    reimplement this method to tie a socket notifier into another
    event loop. Reimplementations <b>must</b> call the base
    implementation.
*/

/*! \fn void QEventLoop::unregisterSocketNotifier(QSocketNotifier *notifier)

    Unregisters \a notifier from the event loop. Subclasses must
    reimplement this method to tie a socket notifier into another
    event loop. Reimplementations <b>must</b> call the base
    implementation.
*/

/*! \fn void QEventLoop::setSocketNotifierPending(QSocketNotifier *notifier)

    Marks \a notifier as pending. The socket notifier will be
    activated the next time activateSocketNotifiers() is called.
*/

/*! \fn int QEventLoop::activateSocketNotifiers()

    Activates all pending socket notifiers and returns the number of
    socket notifiers that were activated.
*/

/*! \fn int QEventLoop::activateTimers()

    Activates all Qt timers and returns the number of timers that were
    activated.

    QEventLoop subclasses that do their own timer handling need to
    call this after the time returned by timeToWait() has elapsed.

    Note: This function is only useful on systems where \c select() is
    used to block the eventloop. On Windows, this function always
    returns 0. On Mac OS X, this function always returns 0 when the
    GUI is enabled. On Mac OS X, this function returns the documented
    value when the GUI is disabled.
*/

/*! \fn int QEventLoop::timeToWait() const

    Returns the number of milliseconds that Qt needs to handle its
    timers or -1 if there are no timers running.

    QEventLoop subclasses that do their own timer handling need to use
    this to make sure that Qt's timers continue to work.

    Note: This function is only useful on systems where \c select() is
    used to block the eventloop. On Windows, this function always
    returns -1. On MacOS X, this function always returns -1 when the
    GUI is enabled. On MacOS X, this function returns the documented
    value when the GUI is disabled.
*/

/*! \fn void QEventLoop::wakeUp()
    \threadsafe

    Wakes up the event loop.

    \sa awake()
*/

/*! \fn void QEventLoop::awake()

    This signal is emitted after the event loop returns from a
    function that could block.

    \sa wakeUp() aboutToBlock()
*/

/*! \fn void QEventLoop::aboutToBlock()

    This signal is emitted before the event loop calls a function that
    could block.

    \sa awake()
*/

// ### DOC: Are these called when the _application_ starts/stops or just
// when the current _event loop_ starts/stops?
/*!
   \internal
*/
void QEventLoop::appStartingUp()
{
}

/*!
    Flushes the event queue. This normally returns almost
    immediately. Does nothing on platforms other than X11.
*/
void QEventLoop::flush()
{
}

/*!
   \internal
*/
void QEventLoop::appClosingDown()
{
}

/*!
    Sets the process event handler \a handler. Returns a pointer to 
    the handler previously defined.

    The process event handler is a function that receives all messages
    taken from the system event loop before the event is dispatched to
    the respective target. This includes messages that are not sent to
    Qt objects.

    The function can return true to prevent the message from being dispatched,
    or false to pass the message back to the standard event processing.

    Only one handler can be defined, but the handler can use the return value
    to call the previously set event handler. By default, no handler is set (ie.
    the function returns 0).
*/
QEventLoop::ProcessEventHandler QEventLoop::setProcessEventHandler(ProcessEventHandler handler)
{
    ProcessEventHandler oldHandler = d->process_event_handler;
    d->process_event_handler = handler;
    return oldHandler;
}

/*!
    Sets the event filter \a filter. Returns a pointer to the filter function 
    previously defined.

    The event filter is a function that is called for every message received. This
    does \e not include messages to objects that are not handled by Qt.

    The function can return true to stop the event to be processed by Qt, or false
    to continue with the standard event processing.

    Only one filter can be defined, but the filter can use the return value
    to call the previously set event filter. By default, no filter is set (ie.
    the function returns 0).
*/
QEventLoop::EventFilter QEventLoop::setEventFilter(EventFilter filter)
{
    EventFilter oldFilter = d->event_filter;
    d->event_filter = filter;
    return oldFilter;
}
