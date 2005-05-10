/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qabstracteventdispatcher.h"
#include "qabstracteventdispatcher_p.h"

#include "qthread.h"
#include <private/qthread_p.h>
#include <private/qcoreapplication_p.h>


static QBasicAtomic timerId = Q_ATOMIC_INIT(1);
static int nextTimerId()
{
    register int id;
    forever {
        id = timerId;
        if (timerId.testAndSet(id, id + 1))
            break;
    }
    Q_ASSERT_X(id >= 1, "QTimer", "timer id overflow, please contact qt-bugs@trolltech.com");
    return id;
}


void QAbstractEventDispatcherPrivate::init()
{
    Q_Q(QAbstractEventDispatcher);
    QThread *thread = q->thread();
    if (!thread) {
        if (!QCoreApplicationPrivate::eventDispatcher) {
            QCoreApplicationPrivate::eventDispatcher = q;
        } else {
            Q_ASSERT_X(false, "QAbstractEventDispatcher",
                       "An event dispatcher has already been created for the application");
        }
    } else {
        QThreadData *data = QThreadData::get(thread);
        Q_ASSERT_X(!data->eventDispatcher, "QAbstractEventDispatcher",
                   "An event dispatcher has already been created for this thread");
        Q_UNUSED(data);
    }
}


/*!
    \class QAbstractEventDispatcher
    \brief The QAbstractEventDispatcher class manages Qt's event queue, excluding GUI-related events.

    \ingroup application
    \ingroup events

    It receives events from the window system and other sources. It
    then sends them to the QCoreApplication or QApplication instance
    for processing and delivery. QAbstractEventDispatcher provides
    fine-grained control over event delivery.

    For simple control of event processing use
    QCoreApplication::processEvents().

    For finer control of the application's event loop, call
    instance() and call functions on the QAbstractEventDispatcher
    object that is returned. If you want to use your own instance of
    QAbstractEventDispatcher or of a QAbstractEventDispatcher
    subclass, you must create your instance \e before you create the
    QApplication object.

    The main event loop is started by calling
    QCoreApplication::exec(), and stopped by calling
    QCoreApplication::exit(). Local event loops can be created using
    QEventLoop.

    Programs that perform long operations can call processEvents()
    with various QEventLoop::ProcessEventsFlag values OR'ed together to
    control which events should be delivered.

    QAbstractEventDispatcher also allows the integration of an
    external event loop with the Qt event loop. For example, the Motif
    Extension included with Qt includes a reimplementation of
    QAbstractEventDispatcher that merges Qt and Motif events together.

    \sa QEventLoop, QCoreApplication
*/

/*!
    Constructs a new event dispatcher with the given \a parent.
*/
QAbstractEventDispatcher::QAbstractEventDispatcher(QObject *parent)
    : QObject(*new QAbstractEventDispatcherPrivate, parent)
{
    Q_D(QAbstractEventDispatcher);
    d->init();
}

/*!
    \internal
*/
QAbstractEventDispatcher::QAbstractEventDispatcher(QAbstractEventDispatcherPrivate &dd,
                                                   QObject *parent)
    : QObject(dd, parent)
{
    Q_D(QAbstractEventDispatcher);
    d->init();
}

/*!
    Destroys the event dispatcher.
*/
QAbstractEventDispatcher::~QAbstractEventDispatcher()
{ }

/*!
    Returns a pointer to the event dispatcher object for the specified
    \a thread. If \a thread is zero, the current thread is used. If no
    event dispatcher exists for the specified \a thread, this function
    returns 0.

    Note: If Qt is built without thread support, the \a thread
    argument is ignored.
 */
QAbstractEventDispatcher *QAbstractEventDispatcher::instance(QThread *thread)
{
    if (!thread)
        thread = QThread::currentThread();
    if (!thread)
        return 0;
    return QThreadData::get(thread)->eventDispatcher;
}

/*! \fn bool QAbstractEventDispatcher::processEvents(QEventLoop::ProcessEventsFlags flags)

    Processes pending events that match \a flags until there are no
    more events to process.

    This function is especially useful if you have a long running
    operation and want to show its progress without allowing user
    input, i.e. by using the QEventLoop::ExcludeUserInputEvents flag.

    If the QEventLoop::WaitForMoreEvents flag is set in \a flags, the
    behavior of this function is as follows:

    \list

    \i If events are available, this function returns after processing
    them.

    \i If no events are available, this function will wait until more
    are available and return after processing newly available events.

    \endlist

    If the QEventLoop::WaitForMoreEvents flag is not set in \a flags,
    and no events are available, this function will return
    immediately.

    Note: This function does not process events continuously; it
    returns after all available events are processed.

    This function returns true if an event was processed; otherwise it
    returns false.

    \sa ProcessEventsFlags hasPendingEvents()
*/

/*! \fn bool QAbstractEventDispatcher::hasPendingEvents()

    Returns true if there is an event waiting; otherwise returns
    false.
*/

/*!
    \fn void QAbstractEventDispatcher::registerSocketNotifier(QSocketNotifier *notifier)

    Registers \a notifier with the event loop. Subclasses must
    implement this method to tie a socket notifier into another
    event loop.
*/

/*! \fn void QAbstractEventDispatcher::unregisterSocketNotifier(QSocketNotifier *notifier)

    Unregisters \a notifier from the event dispatcher. Subclasses must
    reimplement this method to tie a socket notifier into another
    event loop. Reimplementations <b>must</b> call the base
    implementation.
*/

/*!
    \fn int QAbstractEventDispatcher::registerTimer(int interval, QObject *object)

    Register a timer with the specified \a interval for the given \a object.
*/
int QAbstractEventDispatcher::registerTimer(int interval, QObject *object)
{
    int id = nextTimerId();
    registerTimer(id, interval, object);
    return id;
}

/*!
    \fn void QAbstractEventDispatcher::registerTimer(int timerId, int interval, QObject *object)

    Register a timer with the specified \a timerId and \a interval for
    the given \a object.
*/

/*!
    \fn bool QAbstractEventDispatcher::unregisterTimer(int timerId)

    Unregister the timer with the given \a timerId.
*/

/*!
    \fn bool QAbstractEventDispatcher::unregisterTimers(QObject *object)

    Unregister all the timers associated with the given \a object.
*/

/*!
    \fn QList<TimerInfo> registeredTimers(QObject *object) const

    Returns a list of registered timers for \a object. The timer ID
    is the first member in each pair; the interval is the second.
*/

/*! \fn void QAbstractEventDispatcher::wakeUp()
    \threadsafe

    Wakes up the event loop.

    \sa awake()
*/

/*! \fn void QAbstractEventDispatcher::interrupt()

    Interrupts event dispatching, i.e. the event dispatcher will
    return from processEvents() as soon as possible.
*/

/*! \fn void QAbstractEventDispatcher::flush()

    Flushes the event queue. This normally returns almost
    immediately. Does nothing on platforms other than X11.
*/

// ### DOC: Are these called when the _application_ starts/stops or just
// when the current _event loop_ starts/stops?
/*! \internal */
void QAbstractEventDispatcher::startingUp()
{ }

/*! \internal */
void QAbstractEventDispatcher::closingDown()
{ }

/*!
    \typedef QAbstractEventDispatcher::TimerInfo

    Typedef for QPair<int, int>. The first component of
    the pair is the timer ID; the second component is
    the interval.

    \sa registeredTimers()
*/

/*!
    \typedef QAbstractEventDispatcher::EventFilter

    Typedef for a function with the signature

    \code
        bool myEventFilter(void *message);
    \endcode

    \sa setEventFilter(), filterEvent()
*/

/*!
    Sets the event filter \a filter. Returns a pointer to the filter
    function previously defined.

    The event filter is a function that receives all messages taken
    from the system event loop before the event is dispatched to the
    respective target. This includes messages that are not sent to Qt
    objects.

    The function can return true to stop the event to be processed by
    Qt, or false to continue with the standard event processing.

    Only one filter can be defined, but the filter can use the return
    value to call the previously set event filter. By default, no
    filter is set (i.e. the function returns 0).
*/
QAbstractEventDispatcher::EventFilter QAbstractEventDispatcher::setEventFilter(EventFilter filter)
{
    Q_D(QAbstractEventDispatcher);
    EventFilter oldFilter = d->event_filter;
    d->event_filter = filter;
    return oldFilter;
}

/*!
    Sends \a message through the event filter that was set by
    setEventFilter().  If no event filter has been set, this function
    returns false; otherwise, this function returns the result of the
    event filter function.

    Subclasses of QAbstractEventDispatcher \e must call this function
    for \e all messages received from the system to ensure
    compatibility with any extensions that may be used in the
    application.

    \sa setEventFilter()
*/
bool QAbstractEventDispatcher::filterEvent(void *message)
{
    Q_D(QAbstractEventDispatcher);
    if (d->event_filter)
        return d->event_filter(message);
    return false;
}

/*! \fn void QAbstractEventDispatcher::awake()

    This signal is emitted after the event loop returns from a
    function that could block.

    \sa wakeUp() aboutToBlock()
*/

/*! \fn void QAbstractEventDispatcher::aboutToBlock()

    This signal is emitted before the event loop calls a function that
    could block.

    \sa awake()
*/
