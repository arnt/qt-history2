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

#include "qcoreapplication.h"
#include "qcoreapplication_p.h"

#include "qabstracteventdispatcher.h"
#include "qcoreevent.h"
#include "qeventloop.h"
#include <qdatastream.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qhash.h>
#include <private/qprocess_p.h>
#include <qtextcodec.h>
#include <qthread.h>
#include <qthreadstorage.h>
#include <private/qthread_p.h>
#include <qlibraryinfo.h>

#ifdef Q_OS_UNIX
#  include "qeventdispatcher_unix_p.h"
#endif
#ifdef Q_OS_WIN
#  include "qeventdispatcher_win_p.h"
#endif

#include <stdlib.h>

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
extern QString qAppFileName();
#endif

bool QCoreApplicationPrivate::checkInstance(const char *function)
{
    bool b = (QCoreApplication::self != 0);
    if (!b)
        qWarning("QApplication::%s() failed: please instantiate the QApplication object first.", function);
    return b;
}

// Support for introspection

QSignalSpyCallbackSet Q_CORE_EXPORT qt_signal_spy_callback_set = { 0, 0, 0, 0 };

void qt_register_signal_spy_callbacks(const QSignalSpyCallbackSet &callback_set)
{
    qt_signal_spy_callback_set = callback_set;
}

extern "C" void Q_CORE_EXPORT qt_startup_hook()
{
}

typedef QList<QtCleanUpFunction> QVFuncList;
Q_GLOBAL_STATIC(QVFuncList, postRList)

void qAddPostRoutine(QtCleanUpFunction p)
{
    QVFuncList *list = postRList();
    if (!list)
        return;
    list->prepend(p);
}

void qRemovePostRoutine(QtCleanUpFunction p)
{
    QVFuncList *list = postRList();
    if (!list)
        return;
    list->removeAll(p);
}

void Q_CORE_EXPORT qt_call_post_routines()
{
    QVFuncList *list = postRList();
    if (!list)
        return;
    while (!list->isEmpty())
        (list->takeFirst())();
}


// app starting up if false
bool QCoreApplicationPrivate::is_app_running = false;
 // app closing down if true
bool QCoreApplicationPrivate::is_app_closing = false;


Q_CORE_EXPORT uint qGlobalPostedEventsCount()
{
    QThread *currentThread = QThread::currentThread();
    if (!currentThread)
        return 0;
    return QThreadData::get(currentThread)->postEventList.size();
}


QCoreApplication *QCoreApplication::self = 0;
QAbstractEventDispatcher *QCoreApplicationPrivate::eventDispatcher = 0;

#ifdef Q_OS_UNIX
Qt::HANDLE qt_application_thread_id = 0;
#endif

// thread wrapper for the main() thread
class QCoreApplicationThread : public QThread
{
    Q_DECLARE_PRIVATE(QThread)
public:
    inline QCoreApplicationThread()
    {
        // thread should be running and not finished for the lifetime
        // of the application (even if QCoreApplication goes away)
        d_func()->running = true;
        d_func()->finished = false;
    }
    inline ~QCoreApplicationThread()
    {
        // avoid warning from QThread
        d_func()->running = false;
    }
private:
    inline void run()
    {
        // this function should never be called, it is implemented
        // only so that we can instantiate the object
        qFatal("QCoreApplicationThread: internal error");
    }
};
Q_GLOBAL_STATIC(QCoreApplicationThread, mainThread)

QCoreApplicationPrivate::QCoreApplicationPrivate(int &aargc, char **aargv)
    : QObjectPrivate(), argc(aargc), argv(aargv), eventFilter(0)
{
    static const char *const empty = "";
    if (argc == 0 || argv == 0) {
        argc = 0;
        argv = (char **)&empty; // ouch! careful with QCoreApplication::argv()!
    }
    QCoreApplicationPrivate::is_app_closing = false;

#ifdef Q_OS_UNIX
    qt_application_thread_id = QThread::currentThreadId();
#endif

    QThread *thr = mainThread();
    QThreadPrivate::setCurrentThread(thr);
    QObjectPrivate::thread = QThreadData::get(thr)->id;
}

QCoreApplicationPrivate::~QCoreApplicationPrivate()
{
    QThreadData *data = QThreadData::get(mainThread());
    QThreadStorageData::finish(data->tls);
    QThreadPrivate::setCurrentThread(0);

    // need to clear the state of the mainData, just in case a new QCoreApplication comes along.
    QMutexLocker locker(&data->postEventList.mutex);
    for (int i = 0; i < data->postEventList.size(); ++i) {
        const QPostEvent &pe = data->postEventList.at(i);
        if (pe.event) {
            --pe.receiver->d_func()->postedEvents;
#ifdef QT3_SUPPORT
            if (pe.event->type() == QEvent::ChildInserted)
                --pe.receiver->d_func()->postedChildInsertedEvents;
#endif
            pe.event->posted = false;
            delete pe.event;
        }
    }
    data->postEventList.clear();
    data->postEventList.recursion = 0;
    data->quitNow = false;
}

void QCoreApplicationPrivate::createEventDispatcher()
{
    Q_Q(QCoreApplication);
#if defined(Q_OS_UNIX)
    eventDispatcher = new QEventDispatcherUNIX(q);
#elif defined(Q_OS_WIN)
    eventDispatcher = new QEventDispatcherWin32(q);
#else
#  error "QEventDispatcher not yet ported to this platform"
#endif
}

void QCoreApplicationPrivate::moveToMainThread(QObject *o)
{
    if (!o || o->thread() == mainThread())
        return;
    Q_ASSERT(o->parent() == 0);
    if (o->thread() != 0)
        QCoreApplication::sendPostedEvents(o, 0);
    o->d_func()->thread = QThreadData::get(mainThread())->id;
}

QThread *QCoreApplicationPrivate::mainThread()
{ return ::mainThread(); }

#ifdef QT3_SUPPORT
void QCoreApplicationPrivate::removePostedChildInsertedEvents(QObject *receiver, QObject *child)
{
    QThread *currentThread = QThread::currentThread();
    if (currentThread) {
        QThreadData *data = QThreadData::get(currentThread);
        QMutexLocker locker(&data->postEventList.mutex);

        // the QObject destructor calls QObject::removeChild, which calls
        // QCoreApplication::sendEvent() directly.  this can happen while the event
        // loop is in the middle of posting events, and when we get here, we may
        // not have any more posted events for this object.

        // if this is a child remove event and the child insert
        // hasn't been dispatched yet, kill that insert
        for (int i = 0; i < data->postEventList.size(); ++i) {
            const QPostEvent &pe = data->postEventList.at(i);
            if (pe.event && pe.receiver == receiver) {
                if (pe.event->type() == QEvent::ChildInserted
                    && ((QChildEvent*)pe.event)->child() == child) {
                    --receiver->d_func()->postedEvents;
                    --receiver->d_func()->postedChildInsertedEvents;
                    Q_ASSERT(receiver->d_func()->postedEvents >= 0);
                    Q_ASSERT(receiver->d_func()->postedChildInsertedEvents >= 0);
                    pe.event->posted = false;
                    delete pe.event;
                    const_cast<QPostEvent &>(pe).event = 0;
                    const_cast<QPostEvent &>(pe).receiver = 0;
                }
            }
        }
    }
}
#endif

void QCoreApplicationPrivate::checkReceiverThread(QObject *receiver)
{
    QThread *currentThread = QThread::currentThread();
    QThread *thr = receiver->thread();
    Q_ASSERT_X(currentThread == thr || !thr,
               "QCoreApplication::sendEvent",
               QString::fromLatin1("Cannot send events to objects owned by a different thread. "
                                   "Current thread %1. Receiver '%2' (of type '%3') was created in thread %4")
               .arg(QString::number((ulong) currentThread, 16))
               .arg(receiver->objectName())
               .arg(QLatin1String(receiver->metaObject()->className()))
               .arg(QString::number((ulong) thr, 16))
               .toLocal8Bit().data());
    Q_UNUSED(currentThread);
    Q_UNUSED(thr);
}

/*!
    \class QCoreApplication
    \brief The QCoreApplication class provides an event loop for console Qt
    applications.

    \ingroup application
    \mainclass

    This class is used by non-GUI applications to provide their event
    loop. For non-GUI application that uses Qt, there should be exactly
    one QCoreApplication object. For GUI applications, see
    QApplication.

    QCoreApplication contains the main event loop, where all events
    from the operating system (e.g., timer and network events) and
    other sources are processed and dispatched. It also handles the
    application's initialization and finalization, as well as
    system-wide and application-wide settings.

    The command line arguments which QCoreApplication's constructor
    should be called with are accessible using argc() and argv(). The
    event loop is started with a call to exec(). Long running
    operations can call processEvents() to keep the application
    responsive.

    Some Qt classes (e.g., QString) can be used without a
    QCoreApplication object. However, in general, we recommend that
    you create a QCoreApplication or a QApplication object in your \c
    main() function as early as possible.

    An application has an applicationDirPath() and an
    applicationFilePath(). Translation files can be added or removed
    using installTranslator() and removeTranslator(). Application
    strings can be translated using translate(). The QObject::tr()
    and QObject::trUtf8() functions are implemented in terms of
    translate().

    The class provides a quit() slot and an aboutToQuit() signal.

    Several static convenience functions are also provided. The
    QCoreApplication object is available from instance(). Events can
    be sent or posted using sendEvent(), postEvent(), and
    sendPostedEvents(). Pending events can be removed with
    removePostedEvents() or flushed with flush(). Library paths (see
    QLibrary) can be retrieved with libraryPaths() and manipulated by
    setLibraryPaths(), addLibraryPath(), and removeLibraryPath().

    \sa QApplication, QAbstractEventDispatcher, QEventLoop
*/

/*!
    \fn static QCoreApplication *QCoreApplication::instance()

    Returns a pointer to the application's QCoreApplication (or
    QApplication) instance.
*/

/*!\internal
 */
QCoreApplication::QCoreApplication(QCoreApplicationPrivate &p)
    : QObject(p, 0)
{
    init();
    // note: it is the subclasses' job to call
    // QCoreApplicationPrivate::eventDispatcher->startingUp();
}

/*!
    Flushes the platform specific event queues.

    If you are doing graphical changes inside a loop that does not
    return to the event loop on asynchronous window systems like X11
    or double buffered window systems like Mac OS X, and you want to
    visualize these changes immediately (e.g. Splash Screens), call
    this function.

    \sa sendPostedEvents()
*/
void QCoreApplication::flush()
{
    if (self && self->d_func()->eventDispatcher)
        self->d_func()->eventDispatcher->flush();
}

/*!
    Constructs a Qt kernel application. Kernel applications are
    applications without a graphical user interface. These type of
    applications are used at the console or as server processes.

    The \a argc and \a argv arguments are available from argc() and
    argv().
*/
QCoreApplication::QCoreApplication(int &argc, char **argv)
    : QObject(*new QCoreApplicationPrivate(argc, argv), 0)
{
    init();
    QCoreApplicationPrivate::eventDispatcher->startingUp();
}

extern void set_winapp_name();

// ### move to QCoreApplicationPrivate constructor?
void QCoreApplication::init()
{
    Q_D(QCoreApplication);
#ifdef Q_WS_WIN
    // Get the application name/instance if qWinMain() was not invoked
    set_winapp_name();
#endif

#ifndef QT_NO_COMPONENT
    d->app_libpaths = 0;
#endif

    Q_ASSERT_X(!self, "QCoreApplication", "there should be only one application object");
    QCoreApplication::self = this;

    QThread::initialize();

    if (!QCoreApplicationPrivate::eventDispatcher)
        d->createEventDispatcher();
    Q_ASSERT(QCoreApplicationPrivate::eventDispatcher != 0);
    QCoreApplicationPrivate::moveToMainThread(QCoreApplicationPrivate::eventDispatcher);

    QThreadData *data = QThreadData::get(mainThread());
    data->eventDispatcher = QCoreApplicationPrivate::eventDispatcher;

#ifdef Q_OS_UNIX
    // Make sure the process manager thread object is created in the main
    // thread.
    QProcessPrivate::initializeProcessManager();
#endif

    qt_startup_hook();
}

/*!
    Destroys the QCoreApplication object.
*/
QCoreApplication::~QCoreApplication()
{
    Q_D(QCoreApplication);
    qt_call_post_routines();

#ifndef QT_NO_COMPONENT
    delete d->app_libpaths;
    d->app_libpaths = 0;
#endif

    self = 0;
    QCoreApplicationPrivate::is_app_running = false;

    QThread::cleanup();

    QThreadData::get(mainThread())->eventDispatcher = 0;
    if (d->eventDispatcher)
        d->eventDispatcher->closingDown();
    d->eventDispatcher = 0;
}

/*!
  Sends \a event to \a receiver: \a {receiver}->event(\a event).
  Returns the value that is returned from the receiver's event handler.

  For certain types of events (e.g. mouse and key events),
  the event will be propagated to the receiver's parent and so on up to
  the top-level object if the receiver is not interested in the event
  (i.e., it returns false).

  There are five different ways that events can be processed;
  reimplementing this virtual function is just one of them. All five
  approaches are listed below:
  \list 1
  \i Reimplementing this function. This is very powerful, providing
  complete control; but only one subclass can be active at a time.

  \i Installing an event filter on QCoreApplication::instance(). Such
  an event filter is able to process all events for all widgets, so
  it's just as powerful as reimplementing notify(); furthermore, it's
  possible to have more than one application-global event filter.
  Global event filters even see mouse events for
  \l{QWidget::isEnabled()}{disabled widgets}.

  \i Reimplementing QObject::event() (as QWidget does). If you do
  this you get Tab key presses, and you get to see the events before
  any widget-specific event filters.

  \i Installing an event filter on the object. Such an event filter
  gets all the events except Tab and Shift-Tab key presses.

  \i Reimplementing paintEvent(), mousePressEvent() and so
  on. This is the commonest, easiest and least powerful way.
  \endlist

  \sa QObject::event(), installEventFilter()
*/

bool QCoreApplication::notify(QObject *receiver, QEvent *event)
{
    Q_D(QCoreApplication);
    // no events are delivered after ~QCoreApplication() has started
    if (QCoreApplicationPrivate::is_app_closing)
        return true;

    if (receiver == 0) {                        // serious error
        qWarning("QCoreApplication::notify: Unexpected null receiver");
        return true;
    }

    d->checkReceiverThread(receiver);

#ifdef QT3_SUPPORT
    if (event->type() == QEvent::ChildRemoved && receiver->d_func()->postedChildInsertedEvents)
        d->removePostedChildInsertedEvents(receiver, static_cast<QChildEvent *>(event)->child());
#endif // QT3_SUPPORT

    return receiver->isWidgetType() ? false : d->notify_helper(receiver, event);
}

/*!\internal

  Helper function called by notify()
 */
bool QCoreApplicationPrivate::notify_helper(QObject *receiver, QEvent * event)
{
    Q_Q(QCoreApplication);
    // send to all application event filters
    for (int i = 0; i < eventFilters.size(); ++i) {
        register QObject *obj = eventFilters.at(i);
        if (obj && obj->eventFilter(receiver, event))
            return true;
    }

    // send to all receiver event filters
    if (receiver != q) {
        for (int i = 0; i < receiver->d_func()->eventFilters.size(); ++i) {
            register QObject *obj = receiver->d_func()->eventFilters.at(i);
            if (obj && obj->eventFilter(receiver, event))
                return true;
        }
    }

    return receiver->event(event);
}

/*!
  Returns true if an application object has not been created yet;
  otherwise returns false.

  \sa closingDown()
*/

bool QCoreApplication::startingUp()
{
    return !QCoreApplicationPrivate::is_app_running;
}

/*!
  Returns true if the application objects are being destroyed;
  otherwise returns false.

  \sa startingUp()
*/

bool QCoreApplication::closingDown()
{
    return QCoreApplicationPrivate::is_app_closing;
}


/*!
    Processes all pending events according to the specified \a flags until
    there are no more events to process.

    You can call this function occasionally when your program is busy
    performing a long operation (e.g. copying a file).

    \sa exec(), QTimer, QEventLoop::processEvents()
*/
void QCoreApplication::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    QThread *currentThread = QThread::currentThread();
    if (!currentThread)
        return;
    QThreadData::get(currentThread)->eventDispatcher->processEvents(flags);
}

/*!
    \overload

    Processes pending events for \a maxtime milliseconds or until
    there are no more events to process, whichever is shorter.

    You can call this function occasionally when you program is busy
    doing a long operation (e.g. copying a file).

    \sa exec(), QTimer, QEventLoop::processEvents()
*/
void QCoreApplication::processEvents(QEventLoop::ProcessEventsFlags flags, int maxtime)
{
    QThread *currentThread = QThread::currentThread();
    if (!currentThread)
        return;
    QThreadData *data = QThreadData::get(currentThread);
    QTime start;
    start.start();
    while (data->eventDispatcher->processEvents(flags & ~QEventLoop::WaitForMoreEvents)) {
        if (start.elapsed() > maxtime)
            break;
    }
}

/*****************************************************************************
  Main event loop wrappers
 *****************************************************************************/

/*!
    Enters the main event loop and waits until exit() is called.
    Returns the value that was set to exit() (which is 0 if exit() is
    called via quit()).

    It is necessary to call this function to start event handling. The
    main event loop receives events from the window system and
    dispatches these to the application widgets.

    To make your application perform idle processing, i.e. executing a
    special function whenever there are no pending events, use a
    QTimer with 0 timeout. More advanced idle processing schemes can
    be achieved using processEvents().

    \sa quit(), exit(), processEvents(), QApplication::exec()
*/
int QCoreApplication::exec()
{
    if (!QCoreApplicationPrivate::checkInstance("exec"))
        return -1;
    QThread *currentThread = QThread::currentThread();
    if (currentThread != self->thread()) {
        qWarning("QApplication::exec() failed: must be called from the main thread.");
        return -1;
    }
    QThreadData *data = QThreadData::get(currentThread);
    if (!data->eventLoops.isEmpty()) {
        qWarning("QApplication::exec() failed: the event loop is already running.");
        return -1;
    }
    QEventLoop eventLoop;
    int returnCode = eventLoop.exec();
    if (self) {
        emit self->aboutToQuit();
        sendPostedEvents(0, QEvent::DeferredDelete);
    }
    return returnCode;
}

/*!
  Tells the application to exit with a return code.

  After this function has been called, the application leaves the main
  event loop and returns from the call to exec(). The exec() function
  returns \a returnCode.

  By convention, a \a returnCode of 0 means success, and any non-zero
  value indicates an error.

  Note that unlike the C library function of the same name, this
  function \e does return to the caller -- it is event processing that
  stops.

  \sa quit(), exec()
*/
void QCoreApplication::exit(int returnCode)
{
    if (!self)
        return;
    QThreadData *data = QThreadData::get(self->thread());
    data->quitNow = true;
    for (int i = 0; i < data->eventLoops.size(); ++i) {
        QEventLoop *eventLoop = data->eventLoops.at(i);
        eventLoop->exit(returnCode);
    }
}

/*****************************************************************************
  QCoreApplication management of posted events
 *****************************************************************************/

/*!
    \fn bool QCoreApplication::sendEvent(QObject *receiver, QEvent *event)

    Sends event \a event directly to receiver \a receiver, using the
    notify() function. Returns the value that was returned from the
    event handler.

    The event is \e not deleted when the event has been sent. The normal
    approach is to create the event on the stack, for example:

    \code
        QMouseEvent event(QEvent::MouseButtonPress, pos, 0, 0);
        QApplication::sendEvent(mainWindow, &event);
    \endcode

    \sa postEvent(), notify()
*/

/*!
    Adds the event \a event with the object \a receiver as the receiver of the
    event, to an event queue and returns immediately.

    The event must be allocated on the heap since the post event queue
    will take ownership of the event and delete it once it has been posted.
    It is \e {not safe} to modify or delete the event after it has been posted.

    When control returns to the main event loop, all events that are
    stored in the queue will be sent using the notify() function.

    \threadsafe

    \sa sendEvent(), notify()
*/

void QCoreApplication::postEvent(QObject *receiver, QEvent *event)
{
    if (receiver == 0) {
        qWarning("QCoreApplication::postEvent: Unexpected null receiver");
        delete event;
        return;
    }

    /*
      avoid a deadlock when trying to create the mainThread() when
      posting the very first event in an application with a
      QCoreApplication
    */
    (void) mainThread();

    QReadLocker locker(QObjectPrivate::readWriteLock());
    if (!QObjectPrivate::isValidObject(receiver)) {
        qWarning("QCoreApplication::postEvent: Receiver is not a valid QObject");
        delete event;
        return;
    }

    QThread *thread = receiver->thread();
    if (!thread)
        thread = mainThread();
    if (!thread) {
        // posting during destruction? just delete the event to prevent a leak
        delete event;
        return;
    }
    QThreadData *data = QThreadData::get(thread);

    {
        QMutexLocker locker(&data->postEventList.mutex);

        // if this is one of the compressible events, do compression
        if (receiver->d_func()->postedEvents
            && self && self->compressEvent(event, receiver, &data->postEventList)) {
            delete event;
            return;
        }

        event->posted = true;
        ++receiver->d_func()->postedEvents;
#ifdef QT3_SUPPORT
        if (event->type() == QEvent::ChildInserted)
            ++receiver->d_func()->postedChildInsertedEvents;
#endif
        if (event->type() == QEvent::DeferredDelete) {
            // remember the current eventloop
            if (!data->eventLoops.isEmpty())
                event->d = reinterpret_cast<QEventPrivate *>(data->eventLoops.top());
        }
        data->postEventList.append(QPostEvent(receiver, event));
    }

    if (data->eventDispatcher)
        data->eventDispatcher->wakeUp();
}

/*!
  \internal
  Returns true if \a event should be blocked and deleted
*/
bool QCoreApplication::compressEvent(QEvent *event, QObject *receiver, QPostEventList *postedEvents)
{
#ifdef Q_WS_WIN
    Q_ASSERT(event);
    Q_ASSERT(receiver);
    Q_ASSERT(postedEvents);

    // compress posted timers to this object.
    if (event->type() == QEvent::Timer && receiver->d_func()->postedEvents > 0) {
        int timerId = ((QTimerEvent *) event)->timerId();
        for (int i=0; i<postedEvents->size(); ++i) {
            const QPostEvent &e = postedEvents->at(i);
            if (e.receiver == receiver && e.event && e.event->type() == QEvent::Timer
                && ((QTimerEvent *) e.event)->timerId() == timerId)
                return true;
        }
    }
#else
    Q_UNUSED(event);
    Q_UNUSED(receiver);
    Q_UNUSED(postedEvents);
#endif

    return false;
}

/*!
  \fn void QCoreApplication::sendPostedEvents()
  \overload

    Dispatches all posted events, i.e. empties the event queue.
*/

/*!
  Immediately dispatches all events which have been previously queued
  with QCoreApplication::postEvent() and which are for the object \a receiver
  and have the event type \a event_type.

  Note that events from the window system are \e not dispatched by this
  function, but by processEvents().

  If \a receiver is null, the events of \a event_type are sent for all
  objects. If \a event_type is 0, all the events are sent for \a receiver.
*/

void QCoreApplication::sendPostedEvents(QObject *receiver, int event_type)
{
    QThread *currentThread = QThread::currentThread();
    if (self) {
        // allow sendPostedEvents() to be called when QCoreApplication
        // is not instantiated
        Q_ASSERT_X(currentThread != 0, "QCoreApplication::sendPostedEvents",
                   "Posted events can only be send from threads started with QThread");
        if (!currentThread)
            return;
    }

    if (receiver) {
        QThread *thr = receiver->thread();
        Q_ASSERT_X(thr == currentThread || !thr, "QCoreApplication::sendPostedEvents",
                   "Cannot send posted events for object created in another thread");
        if (thr == 0 || thr != currentThread)
            return;
    }

    QThreadData *data = QThreadData::get(currentThread);

    ++data->postEventList.recursion;
    // the allowDeferredDelete flag is set to true in
    // QEventLoop::exec(), just before each call to processEvents()
    //
    // Note: we leave it set to false when returning
    bool allowDeferredDelete = data->allowDeferredDelete || event_type == QEvent::DeferredDelete;
    data->allowDeferredDelete = false;

#ifdef QT3_SUPPORT
    // optimize sendPostedEvents(w, QEvent::ChildInserted) calls away
    if (receiver && event_type == QEvent::ChildInserted
        && !receiver->d_func()->postedChildInsertedEvents) {
        --data->postEventList.recursion;
        return;
    }
    // Make sure the object hierarchy is stable before processing events
    // to avoid endless loops
    if (receiver == 0 && event_type == 0)
        sendPostedEvents(0, QEvent::ChildInserted);
#endif

    QMutexLocker locker(&data->postEventList.mutex);

    if (data->postEventList.size() == 0 || (receiver && !receiver->d_func()->postedEvents)) {
        --data->postEventList.recursion;
        return;
    }

    // okay. here is the tricky loop. be careful about optimizing
    // this, it looks the way it does for good reasons.
    int i = 0;
    const int s = data->postEventList.size();
    while (i < data->postEventList.size()) {
        // avoid live-lock
        if (i >= s)
            break;

        const QPostEvent &pe = data->postEventList.at(i);
        ++i;

        if (!pe.event)
            continue;
        if (receiver && receiver != pe.receiver)
            continue;
        if (event_type && event_type != pe.event->type())
            continue;

        if (pe.event->type() == QEvent::DeferredDelete) {
            const QEventLoop *const savedEventLoop = reinterpret_cast<QEventLoop *>(pe.event->d);
            const QEventLoop *const currentEventLoop =
                data->eventLoops.isEmpty() ? 0 : data->eventLoops.top();
            if (!allowDeferredDelete
                || (savedEventLoop != 0 && currentEventLoop != 0 &&  savedEventLoop != currentEventLoop)) {
                // cannot send deferred delete
                if (!event_type && !receiver) {
                    // don't lose the event
                    data->postEventList.append(pe);
                    const_cast<QPostEvent &>(pe).event = 0;
                }
                continue;
            }
        }

        // first, we diddle the event so that we can deliver
        // it, and that noone will try to touch it later.
        pe.event->posted = false;
        QEvent * e = pe.event;
        QObject * r = pe.receiver;

        --r->d_func()->postedEvents;
        Q_ASSERT(r->d_func()->postedEvents >= 0);
#ifdef QT3_SUPPORT
        if (e->type() == QEvent::ChildInserted)
            --r->d_func()->postedChildInsertedEvents;
        Q_ASSERT(r->d_func()->postedChildInsertedEvents >= 0);
#endif

        // next, update the data structure so that we're ready
        // for the next event.
        const_cast<QPostEvent &>(pe).event = 0;

        locker.unlock();
        // after all that work, it's time to deliver the event.
        QCoreApplication::sendEvent(r, e);
        locker.relock();

        delete e;
        // careful when adding anything below this point - the
        // sendEvent() call might invalidate any invariants this
        // function depends on.
    }

    --data->postEventList.recursion;
    // clear the global list, i.e. remove everything that was
    // delivered.
    if (!data->postEventList.recursion && !event_type && !receiver) {
        const QPostEventList::iterator it = data->postEventList.begin();
        data->postEventList.erase(it, it + i);
    }
}

/*!
  Removes all events posted using postEvent() for \a receiver.

  The events are \e not dispatched, instead they are removed from the
  queue. You should never need to call this function. If you do call it,
  be aware that killing events may cause \a receiver to break one or
  more invariants.

  \threadsafe
*/

void QCoreApplication::removePostedEvents(QObject *receiver)
{
    if (!receiver)
        return;
    QThread *thr = receiver->thread();
    if (!thr)
        thr = mainThread();
    if (!thr)
        return;
    QThreadData *data = QThreadData::get(thr);

    QMutexLocker locker(&data->postEventList.mutex);

    // the QObject destructor calls this function directly.  this can
    // happen while the event loop is in the middle of posting events,
    // and when we get here, we may not have any more posted events
    // for this object.
    if (!receiver->d_func()->postedEvents) return;

    int n = data->postEventList.size();
    int j = 0;

    for (int i = 0; i < n; ++i) {
        const QPostEvent &pe = data->postEventList.at(i);
        if (pe.receiver == receiver) {
            if (pe.event) {
                --receiver->d_func()->postedEvents;
#ifdef QT3_SUPPORT
                if (pe.event->type() == QEvent::ChildInserted)
                    --receiver->d_func()->postedChildInsertedEvents;
#endif
                pe.event->posted = false;
                delete pe.event;
                const_cast<QPostEvent &>(pe).event = 0;
            }
        } else if (!data->postEventList.recursion) {
            if (i != j)
                data->postEventList.swap(i, j);
            ++j;
        }
    }

    Q_ASSERT(!receiver->d_func()->postedEvents);
#ifdef QT3_SUPPORT
    Q_ASSERT(!receiver->d_func()->postedChildInsertedEvents);
#endif
    if (!data->postEventList.recursion) {
        while (j++ < n)
            data->postEventList.removeLast();
    }
}


/*!
  Removes \a event from the queue of posted events, and emits a
  warning message if appropriate.

  \warning This function can be \e really slow. Avoid using it, if
  possible.

  \threadsafe
*/

void QCoreApplicationPrivate::removePostedEvent(QEvent * event)
{
    if (!event || !event->posted)
        return;

    QThread *thread = QThread::currentThread();
    if (!thread)
        return;
    QThreadData *data = QThreadData::get(thread);

    QMutexLocker locker(&data->postEventList.mutex);

    if (data->postEventList.size() == 0) {
#if defined(QT_DEBUG)
        qDebug("QCoreApplication::removePostedEvent: %p %d is posted: impossible",
                (void*)event, event->type());
        return;
#endif
    }

    for (int i = 0; i < data->postEventList.size(); ++i) {
        const QPostEvent & pe = data->postEventList.at(i);
        if (pe.event == event) {
#ifndef QT_NO_DEBUG
            qWarning("QEvent: Warning: event of type %d deleted while posted to %s %s",
                     event->type(),
                     pe.receiver ? pe.receiver->metaObject()->className() : "null",
                     pe.receiver ? pe.receiver->objectName().toLocal8Bit().data() : "object");
#endif
            --pe.receiver->d_func()->postedEvents;
#ifdef QT3_SUPPORT
            if (pe.event->type() == QEvent::ChildInserted)
                --pe.receiver->d_func()->postedChildInsertedEvents;
#endif
            pe.event->posted = false;
            delete pe.event;
            const_cast<QPostEvent &>(pe).event = 0;
            return;
        }
    }
}

/*!\reimp

*/
bool QCoreApplication::event(QEvent *e)
{
    if (e->type() == QEvent::Quit) {
        quit();
        return true;
    }
    return QObject::event(e);
}

/*! \enum QCoreApplication::Encoding

  This enum type defines the 8-bit encoding of character string
  arguments to translate():

  \value DefaultCodec  The encoding specified by
  QTextCodec::codecForTr() (Latin1 if none has been set)
  \value UnicodeUTF8  UTF-8

  \sa QObject::tr(), QObject::trUtf8(), QString::fromUtf8()
*/



/*!
    Tells the application to exit with return code 0 (success).
    Equivalent to calling QCoreApplication::exit(0).

    It's common to connect the QApplication::lastWindowClosed() signal
    to quit(), and you also often connect e.g. QAbstractButton::clicked() or
    signals in QAction, QMenu, or QMenuBar to it.

    Example:

    \code
        QPushButton *quitButton = new QPushButton("Quit");
        connect(quitButton, SIGNAL(clicked()), &app, SLOT(quit()));
    \endcode

    \sa exit(), aboutToQuit(), QApplication::lastWindowClosed()
*/

void QCoreApplication::quit()
{
    exit(0);
}

/*!
  \fn void QCoreApplication::aboutToQuit()

  This signal is emitted when the application is about to quit the
  main event loop, e.g. when the event loop level drops to zero.
  This may happen either after a call to quit() from inside the
  application or when the users shuts down the entire desktop session.

  The signal is particularly useful if your application has to do some
  last-second cleanup. Note that no user interaction is possible in
  this state.

  \sa quit()
*/

#ifndef QT_NO_TRANSLATION
/*!
  Adds the message file \a mf to the list of message files to be used
  for translations.

  Multiple message files can be installed. Translations are searched
  for in the last installed message file, then the one from last, and
  so on, back to the first installed message file. The search stops as
  soon as a matching translation is found.

  \sa removeTranslator() translate() QTranslator::load()
*/

void QCoreApplication::installTranslator(QTranslator * mf)
{
    if (!mf)
        return;

    if (!QCoreApplicationPrivate::checkInstance("installTranslator"))
        return;
    QCoreApplicationPrivate *d = self->d_func();
    d->translators.prepend(mf);

#ifndef QT_NO_TRANSLATION_BUILDER
    if (mf->isEmpty())
        return;
#endif

    QEvent ev(QEvent::LanguageChange);
    QCoreApplication::sendEvent(self, &ev);
}

/*!
  Removes the message file \a mf from the list of message files used by
  this application. (It does not delete the message file from the file
  system.)

  \sa installTranslator() translate(), QObject::tr()
*/

void QCoreApplication::removeTranslator(QTranslator * mf)
{
    if (!mf)
        return;
    if (!QCoreApplicationPrivate::checkInstance("removeTranslator"))
        return;
    QCoreApplicationPrivate *d = self->d_func();
    if (d->translators.removeAll(mf) && !self->closingDown()) {
        QEvent ev(QEvent::LanguageChange);
        QCoreApplication::sendEvent(self, &ev);
    }
}

/*! \reentrant
  Returns the translation text for \a sourceText, by querying the
  installed messages files. The message files are searched from the most
  recently installed message file back to the first installed message
  file.

  QObject::tr() and QObject::trUtf8() provide this functionality more
  conveniently.

  \a context is typically a class name (e.g., "MyDialog") and
  \a sourceText is either English text or a short identifying text, if
  the output text will be very long (as for help texts).

  \a comment is a disambiguating comment, for when the same \a
  sourceText is used in different roles within the same context. By
  default, it is null. \a encoding indicates the 8-bit encoding of
  character stings

  See the \l QTranslator documentation for more information about
  contexts and comments.

  If none of the message files contain a translation for \a
  sourceText in \a context, this function returns a QString
  equivalent of \a sourceText. The encoding of \a sourceText is
  specified by \e encoding; it defaults to \c DefaultCodec.

  This function is not virtual. You can use alternative translation
  techniques by subclassing \l QTranslator.

  \warning This method is reentrant only if all translators are
  installed \e before calling this method. Installing or removing
  translators while performing translations is not supported. Doing
  so will most likely result in crashes or other undesirable behavior.

  \sa QObject::tr() installTranslator() QTextCodec::codecForTr()
*/

QString QCoreApplication::translate(const char *context, const char *sourceText,
                                    const char *comment, Encoding encoding)
{
    if (!sourceText)
        return QString();

    if (self && !self->d_func()->translators.isEmpty()) {
        QList<QTranslator*>::ConstIterator it;
        QTranslator *mf;
        QString result;
        for (it = self->d_func()->translators.constBegin(); it != self->d_func()->translators.constEnd(); ++it) {
            mf = *it;
            result = mf->translate(context, sourceText, comment);
            if (!result.isEmpty())
                return result;
        }
    }
#ifndef QT_NO_TEXTCODEC
    if (encoding == UnicodeUTF8)
        return QString::fromUtf8(sourceText);
    else if (QTextCodec::codecForTr() != 0)
        return QTextCodec::codecForTr()->toUnicode(sourceText);
    else
#endif
        return QString::fromLatin1(sourceText);
}
#endif //QT_NO_TRANSLATE

/*!
    Returns the directory that contains the application executable.

    For example, if you have installed Qt in the \c{C:\Trolltech\Qt}
    directory, and you run the \c{launcher} example, this function will
    return "C:/Trolltech/Qt/examples/tools/launcher".

    On Mac OS X this will point to the directory actually containing the
    executable, which may be inside of an application bundle (if the
    application is bundled).

    \warning On Unix, this function assumes that argv[0] contains the file
    name of the executable (which it normally does). It also assumes that
    the current directory hasn't been changed by the application.

    \sa applicationFilePath()
*/
QString QCoreApplication::applicationDirPath()
{
    if (!self) {
        qWarning("QApplication::applicationDirPath() failed: please instantiate the QApplication object first");
        return QString();
    }
    return QFileInfo(applicationFilePath()).path();
}

#ifndef QT_NO_DIR
/*!
    Returns the file path of the application executable.

    For example, if you have installed Qt in the \c{/usr/local/qt}
    directory, and you run the \c{launcher} example, this function will
    return "/usr/local/qt/examples/tools/launcher".

    \warning On Unix, this function assumes that argv[0] contains the file
    name of the executable (which it normally does). It also assumes that
    the current directory hasn't been changed by the application.

    \sa applicationDirPath()
*/
QString QCoreApplication::applicationFilePath()
{
    if (!self) {
        qWarning("QApplication::applicationFilePath() failed: please instantiate the QApplication object first");
        return QString();
    }
#if defined( Q_WS_WIN )
    QFileInfo filePath;
    QT_WA({
        unsigned short module_name[256];
        GetModuleFileNameW(0, reinterpret_cast<wchar_t *>(module_name), sizeof(module_name));
        filePath = QString::fromUtf16(module_name);
    }, {
        char module_name[256];
        GetModuleFileNameA(0, module_name, sizeof(module_name));
        filePath = QString::fromLocal8Bit(module_name);
    });

    return filePath.filePath();
#elif defined(Q_WS_MAC)
    QFileInfo fi(qAppFileName());
    return fi.exists() ? fi.canonicalFilePath() : QString();
#else
    QString argv0 = QFile::decodeName(QByteArray(argv()[0]));
    QString absPath;

    if (!argv0.isEmpty() && argv0.at(0) == QLatin1Char('/')) {
        /*
          If argv0 starts with a slash, it is already an absolute
          file path.
        */
        absPath = argv0;
    } else if (argv0.contains(QLatin1Char('/'))) {
        /*
          If argv0 contains one or more slashes, it is a file path
          relative to the current directory.
        */
        absPath = QDir::current().absoluteFilePath(argv0);
    } else {
        /*
          Otherwise, the file path has to be determined using the
          PATH environment variable.
        */
        QByteArray pEnv = qgetenv("PATH");
        QDir currentDir = QDir::current();
        QStringList paths = QString::fromLocal8Bit(pEnv.constData()).split(QLatin1String(":"));
        for (QStringList::const_iterator p = paths.constBegin(); p != paths.constEnd(); ++p) {
            if ((*p).isEmpty())
                continue;
            QString candidate = currentDir.absoluteFilePath(*p + QLatin1Char('/') + argv0);
            if (QFile::exists(candidate)) {
                absPath = candidate;
                break;
            }
        }
    }

    absPath = QDir::cleanPath(absPath);

    QFileInfo fi(absPath);
    return fi.exists() ? fi.canonicalFilePath() : QString();
#endif
}
#endif // QT_NO_DIR

/*!
    Returns the number of command line arguments.

    The documentation for argv() describes how to process command line
    arguments.

    \sa argv()
*/
int QCoreApplication::argc()
{
    if (!self) {
        qWarning("QApplication::argc() failed: please instantiate the QApplication object first");
        return 0;
    }
    return self->d_func()->argc;
}


/*!
    Returns the command-line argument array.

    argv()[0] is the program name, argv()[1] is the first
    argument, and argv()[argc() - 1] is the last argument.

    A QCoreApplication object is constructed by passing \e argc and
    \e argv from the \c main() function. Some of the arguments may be
    recognized as Qt options and removed from the argument vector.
    For example, the X11 version of QApplication knows about \c
    -display, \c -font, and a few more options.

    QCoreApplication::instance() points to the QCoreApplication
    object, and through which you can access argc() and argv() in
    functions other than main().

    \sa argc()
*/
char **QCoreApplication::argv()
{
    if (!self) {
        qWarning("QApplication::argv() failed: please instantiate the QApplication object first");
        return 0;
    }
    return self->d_func()->argv;
}

/*!
    \property QCoreApplication::organizationName
    \brief the name of the organization that wrote this application

    The value is used by the QSettings class when it is constructed
    using the empty constructor. This saves having to repeat this
    information each time a QSettings object is created.

    On Mac, QSettings uses organizationDomain() as the organization
    if it's not an empty string; otherwise it uses
    organizationName(). On all other platforms, QSettings uses
    organizationName() as the organization.

    \sa organizationDomain applicationName
*/

void QCoreApplication::setOrganizationName(const QString &orgName)
{
    self->d_func()->orgName = orgName;
}

QString QCoreApplication::organizationName()
{
    return self ? self->d_func()->orgName : QString();
}

/*!
    \property QCoreApplication::organizationDomain
    \brief the Internet domain of the organization that wrote this application

    The value is used by the QSettings class when it is constructed
    using the empty constructor. This saves having to repeat this
    information each time a QSettings object is created.

    On Mac, QSettings uses organizationDomain() as the organization
    if it's not an empty string; otherwise it uses organizationName().
    On all other platforms, QSettings uses organizationName() as the
    organization.

    \sa organizationName applicationName
*/
void QCoreApplication::setOrganizationDomain(const QString &orgDomain)
{
    self->d_func()->orgDomain = orgDomain;
}

QString QCoreApplication::organizationDomain()
{
    return self ? self->d_func()->orgDomain : QString();
}

/*!
    \property QCoreApplication::applicationName
    \brief the name of this application

    The value is used by the QSettings class when it is constructed
    using the empty constructor. This saves having to repeat this
    information each time a QSettings object is created.

    \sa organizationName organizationDomain
*/
void QCoreApplication::setApplicationName(const QString &application)
{
    self->d_func()->application = application;
}


QString QCoreApplication::applicationName()
{
    return self ? self->d_func()->application : QString();
}



#ifndef QT_NO_COMPONENT

/*!
    Returns a list of paths that the application will search when
    dynamically loading libraries.
    The installation directory for plugins is the only entry if no
    paths have been set.  The default installation directory for plugins
    is \c INSTALL/plugins, where \c INSTALL is the directory where Qt was
    installed. The directory of the application executable (NOT the
    working directory) is also added to the plugin paths, as well as
    the colon separated entries of the QT_PLUGIN_PATH environment
    variable.

    If you want to iterate over the list, you can use the \l foreach
    pseudo-keyword:

    \code
        foreach (QString path, app.libraryPaths())
            do_something(path);
    \endcode

    \sa setLibraryPaths(), addLibraryPath(), removeLibraryPath(), QLibrary,
        {How to Create Qt Plugins}
*/
QStringList QCoreApplication::libraryPaths()
{
    if (!self)
        return QStringList();
    if (!self->d_func()->app_libpaths) {
        QStringList *app_libpaths = self->d_func()->app_libpaths = new QStringList;
        QString installPathPlugins =  QLibraryInfo::location(QLibraryInfo::PluginsPath);
        if (QFile::exists(installPathPlugins)) {
#ifdef Q_WS_WIN
            installPathPlugins.replace(QLatin1Char('\\'), QLatin1Char('/'));
#endif
            app_libpaths->append(installPathPlugins);
        }

        QString app_location(self->applicationFilePath());
        app_location.truncate(app_location.lastIndexOf(QLatin1Char('/')));
        if (app_location !=  QLibraryInfo::location(QLibraryInfo::PluginsPath) && QFile::exists(app_location))
            app_libpaths->append(app_location);

        const QByteArray libPathEnv = qgetenv("QT_PLUGIN_PATH");
        if (!libPathEnv.isEmpty())
            (*app_libpaths) += QString::fromLocal8Bit(libPathEnv.constData()).split(QLatin1String(":"), QString::SkipEmptyParts);
    }
    return *self->d_func()->app_libpaths;
}



/*!
  Sets the list of directories to search when loading libraries to \a paths.
  All existing paths will be deleted and the path list will consist of the
  paths given in \a paths.

  \sa libraryPaths(), addLibraryPath(), removeLibraryPath(), QLibrary
 */
void QCoreApplication::setLibraryPaths(const QStringList &paths)
{
    delete self->d_func()->app_libpaths;
    self->d_func()->app_libpaths = new QStringList(paths);
}

/*!
  Appends \a path to the end of the library path list. If \a path is
  empty or already in the path list, the path list is not changed.

  The default path list consists of a single entry, the installation
  directory for plugins.  The default installation directory for plugins
  is \c INSTALL/plugins, where \c INSTALL is the directory where Qt was
  installed.

  \sa removeLibraryPath(), libraryPaths(), setLibraryPaths()
 */
void QCoreApplication::addLibraryPath(const QString &path)
{
    if (path.isEmpty())
        return;

    // make sure that library paths is initialized
    libraryPaths();

    QString canonicalPath = QDir(path).canonicalPath();
    if (!self->d_func()->app_libpaths->contains(canonicalPath))
        self->d_func()->app_libpaths->prepend(canonicalPath);
}

/*!
    Removes \a path from the library path list. If \a path is empty or not
    in the path list, the list is not changed.

    \sa addLibraryPath(), libraryPaths(), setLibraryPaths()
*/
void QCoreApplication::removeLibraryPath(const QString &path)
{
    if (path.isEmpty())
        return;

    // make sure that library paths is initialized
    libraryPaths();

    self->d_func()->app_libpaths->removeAll(path);
}

#endif //QT_NO_COMPONENT

/*!
    \typedef QCoreApplication::EventFilter

    A function with the following signature that can be used as an
    event filter:

    \code
        bool myEventFilter(void *message, long *result);
    \endcode

    \sa setEventFilter()
*/

/*!
    \fn EventFilter QCoreApplication::setEventFilter(EventFilter filter)

    Sets the event filter \a filter. Returns a pointer to the filter
    function previously defined.

    The event filter is a function that is called for every message
    received in all threads. This does \e not include messages to
    objects that are not handled by Qt.

    The function can return true to stop the event to be processed by
    Qt, or false to continue with the standard event processing.

    Only one filter can be defined, but the filter can use the return
    value to call the previously set event filter. By default, no
    filter is set (i.e., the function returns 0).

    \sa installEventFilter()
*/
QCoreApplication::EventFilter
QCoreApplication::setEventFilter(QCoreApplication::EventFilter filter)
{
    Q_D(QCoreApplication);
    EventFilter old = d->eventFilter;
    d->eventFilter = filter;
    return old;
}

/*!
    Sends \a message through the event filter that was set by
    setEventFilter(). If no event filter has been set, this function
    returns false; otherwise, this function returns the result of the
    event filter function in the \a result parameter.

    \sa setEventFilter()
*/
bool QCoreApplication::filterEvent(void *message, long *result)
{
    Q_D(QCoreApplication);
    if (result)
        *result = 0;
    if (d->eventFilter)
        return d->eventFilter(message, result);
#ifdef Q_OS_WIN
    return winEventFilter(reinterpret_cast<MSG *>(message), result);
#else
    return false;
#endif
}

/*!
    This function returns true if there are pending events; otherwise
    returns false. Pending events can be either from the window
    system or posted events using postEvent().

    \sa QAbstractEventDispatcher::hasPendingEvents()
*/
bool QCoreApplication::hasPendingEvents()
{
    QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance();
    if (eventDispatcher)
        return eventDispatcher->hasPendingEvents();
    return false;
}

#ifdef QT3_SUPPORT
/*! \fn void QCoreApplication::lock()

    In Qt 3, this function locked the Qt library mutex, allowing
    non-GUI threads to perform basic printing operations using
    QPainter.

    In Qt 4, this is no longer supported, since painting is only
    supported from within a paint event handler. This function does
    nothing.

    \sa QWidget::paintEvent()
*/

/*! \fn void QCoreApplication::unlock(bool wakeUpGui)

    In Qt 3, this function unlocked the Qt library mutex. The mutex
    allowed non-GUI threads to perform basic printing operations
    using QPainter.

    In Qt 4, this is no longer supported, since painting is only
    supported from within a paint event handler. This function does
    nothing.
*/

/*! \fn bool QCoreApplication::locked()

    This function does nothing. It is there to keep old code working.
    It always returns false.

    See lock() for details.
*/

/*! \fn bool QCoreApplication::tryLock()

    This function does nothing. It is there to keep old code working.
    It always returns false.

    See lock() for details.
*/

/*! \fn void QCoreApplication::processOneEvent()
    \obsolete

    Waits for an event to occur, processes it, then returns.

    This function is useful for adapting Qt to situations where the
    event processing must be grafted onto existing program loops.

    Using this function in new applications may be an indication of design
    problems.

    \sa processEvents(), exec(), QTimer
*/

/*! \obsolete

    This function enters the main event loop (recursively). Do not call
    it unless you really know what you are doing.
*/
int QCoreApplication::enter_loop()
{
    QThread *currentThread = QThread::currentThread();
    if (currentThread != mainThread()) {
        qWarning("QApplication::enter_loop() failed: must be called from the main thread.");
        return -1;
    }
    QEventLoop eventLoop;
    int returnCode = eventLoop.exec();
    return returnCode;
}

/*! \obsolete

    This function exits from a recursive call to the main event loop.
    Do not call it unless you are an expert.
*/
void QCoreApplication::exit_loop()
{
    QThread *currentThread = QThread::currentThread();
    if (currentThread != mainThread()) {
        qWarning("QApplication::exit_loop() failed: must be called from the main thread.");
        return;
    }
    QThreadData *data = QThreadData::get(currentThread);
    if (!data->eventLoops.isEmpty())
        data->eventLoops.top()->exit();
}

/*! \obsolete

    Returns the current loop level.
*/
int QCoreApplication::loopLevel()
{
    QThread *thr = mainThread();
    if (!thr)
        return -1;
    QThreadData *data = QThreadData::get(thr);
    return data->eventLoops.size();
}
#endif

/*!
    \fn void QCoreApplication::watchUnixSignal(int signal, bool watch)
    \internal
*/

/*!
    \fn void QCoreApplication::unixSignal(int number)

    This signal is emitted whenever a Unix signal is received by the
    application. The Unix signal received is specified by its \a number.
*/

/*!
    \fn qAddPostRoutine(QtCleanUpFunction ptr)
    \relates QCoreApplication

    Adds a global routine that will be called from the QApplication
    destructor. This function is normally used to add cleanup routines
    for program-wide functionality.

    The function specified by \a ptr should take no arguments and should
    return nothing. For example:

    \code
        static int *global_ptr = 0;

        static void cleanup_ptr()
        {
            delete [] global_ptr;
            global_ptr = 0;
        }

        void init_ptr()
        {
            global_ptr = new int[100];      // allocate data
            qAddPostRoutine(cleanup_ptr);   // delete later
        }
    \endcode

    Note that for an application- or module-wide cleanup,
    qAddPostRoutine() is often not suitable. For example, if the
    program is split into dynamically loaded modules, the relevant
    module may be unloaded long before the QApplication destructor is
    called.

    For modules and libraries, using a reference-counted
    initialization manager or Qt's parent-child deletion mechanism may
    be better. Here is an example of a private class which uses the
    parent-child mechanism to call a cleanup function at the right
    time:

    \code
        class MyPrivateInitStuff : public QObject
        {
        public:
            static MyPrivateInitStuff *initStuff(QObject *parent)
            {
                if (!p)
                    p = new MyPrivateInitStuff(parent);
                return p;
            }

            ~MyPrivateInitStuff()
            {
                // cleanup goes here
            }

        private:
            MyPrivateInitStuff(QObject *parent)
                : QObject(parent)
            {
                // initialization goes here
            }

            MyPrivateInitStuff *p;
        };
    \endcode

    By selecting the right parent object, this can often be made to
    clean up the module's data at the exactly the right moment.
*/
