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
#include <qtextcodec.h>
#include <qthread.h>
#include <qthreadstorage.h>
#include <private/qthread_p.h>

#ifdef Q_OS_UNIX
#  include "qeventdispatcher_unix.h"
#endif
#ifdef Q_OS_WIN
#  include "qeventdispatcher_win.h"
#endif

#include <stdlib.h>

#define d d_func()
#define q q_func()

#if defined(Q_WS_WIN) || defined(Q_WS_MAC)
extern const char *qAppFileName(); // Declared in qapplication_win.cpp
#endif


typedef void (*VFPTR)();
typedef QList<VFPTR> QVFuncList;
static QVFuncList *postRList = 0;                // list of post routines

void qAddPostRoutine(QtCleanUpFunction p)
{
    if (!postRList)
        postRList = new QVFuncList;
    postRList->prepend(p);
}

void qRemovePostRoutine(QtCleanUpFunction p)
{
    if (!postRList) return;
    QVFuncList::Iterator it = postRList->begin();
    while (it != postRList->end()) {
        if (*it == p) {
            postRList->erase(it);
            it = postRList->begin();
        } else {
            ++it;
        }
    }
}

void Q_CORE_EXPORT qt_call_post_routines()
{
    if (postRList) {
        QVFuncList::Iterator it = postRList->begin();
        while (it != postRList->end()) {        // call post routines
            (**it)();
            postRList->erase(it);
            it = postRList->begin();
        }
        delete postRList;
        postRList = 0;
    }
}


// app starting up if false
bool QCoreApplication::is_app_running = false;
 // app closing down if true
bool QCoreApplication::is_app_closing = false;


Q_CORE_EXPORT uint qGlobalPostedEventsCount()
{
    QThreadData *data = QThreadData::current();
    return data ? data->postEventList.size() : 0;
}


QCoreApplication *QCoreApplication::self = 0;

#ifdef Q_OS_UNIX
Qt::HANDLE qt_application_thread_id = 0;
#endif

// thread data for the main() thread
Q_GLOBAL_STATIC(QThreadData, mainData)

QCoreApplicationPrivate::QCoreApplicationPrivate(int &aargc,  char **aargv)
    : QObjectPrivate(), argc(aargc), argv(aargv), eventDispatcher(0), eventFilter(0)
{
    static const char *empty = "";
    if (argc == 0 || argv == 0) {
        argc = 0;
        argv = (char **)&empty; // ouch! careful with QApplication::argv()!
    }
    QCoreApplication::is_app_closing = false;

#ifdef Q_OS_UNIX
    qt_application_thread_id = QThread::currentThreadId();
#endif
    QThreadData *data = mainData();
    data->id = 0;
    QThreadData::setCurrent(data);
}

QCoreApplicationPrivate::~QCoreApplicationPrivate()
{
    QThreadData *data = mainData();

    QThreadStorageData::finish(data->tls);
    QThreadData::setCurrent(0);

    // need to clear the state of the mainData, just in case a new QCoreApplication comes along.
    QMutexLocker locker(&data->postEventList.mutex);
    data->postEventList.clear();
    data->postEventList.offset = 0;
    data->id = -1;
}

void QCoreApplicationPrivate::createEventDispatcher()
{
#if defined(Q_OS_UNIX)
    eventDispatcher = new QEventDispatcherUNIX;
#elif defined(Q_OS_WIN)
    eventDispatcher = new QEventDispatcherWin32;
#else
#  error "QEventDispatcher not yet ported to this platform"
#endif
}

void QCoreApplicationPrivate::moveToMainThread(QObject *o)
{
    if (!o || !o->thread())
        return;
    Q_ASSERT(o->parent() == 0);

    QCoreApplication::sendPostedEvents(o, 0);
    o->d->thread = 0;
}

#ifdef QT_COMPAT
void QCoreApplicationPrivate::removePostedChildInsertedEvents(QObject *receiver, QObject *child)
{
    QThreadData *data = QThreadData::current();
    if (data) {
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
                    --receiver->d->postedEvents;
                    --receiver->d->postedChildInsertedEvents;
                    Q_ASSERT(receiver->d->postedEvents >= 0);
                    Q_ASSERT(receiver->d->postedChildInsertedEvents >= 0);
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



/*!
    \class QCoreApplication
    \brief The QCoreApplication class provides an event loop for Qt
    applications.

    \ingroup application
    \mainclass

    This class is used by non-GUI applications to provide their event
    loop. For GUI applications see QApplication.

    The command line arguments which QCoreApplication's constructor
    should be called with are accessible using argc() and argv(). The
    event loop is started with a call to exec(). Long running
    operations can call processEvents() to keep the application
    responsive. An application has an applicationDirPath() and an
    applicationFilePath(). Translators can be added or removed using
    installTranslator() and removeTranslator().

    The class provides one signal, aboutToQuit(), and one slot,
    quit().

    Several static convenience functions are also provided. The
    QCoreApplication object is available from instance(), and the
    event loop from eventLoop(). Events can be sent or posted using
    sendEvent(), postEvent(), and sendPostedEvents(). Pending events
    can be removed with removePostedEvents() or flushed with flush().
    Library paths (see QLibrary) can be retrieved with libraryPaths()
    and manipulated by setLibraryPaths(), addLibraryPath(), and
    removeLibraryPath(). Application strings can be translated using
    translate().
*/

/*!
    \fn static QCoreApplication *QCoreApplication::instance()

    Returns a pointer to the application's QCoreApplication.
*/

/*!\internal
 */
QCoreApplication::QCoreApplication(QCoreApplicationPrivate &p)
    : QObject(p, 0)
{ init(); }

/*!
    Flushes the platform specific event queues.

    If you are doing graphical changes inside a loop that does not
    return to the event loop on asynchronous window systems like X11
    or double buffered window systems like MacOS X, and you want to
    visualize these changes immediately (e.g. Splash Screens), call
    this function.

    \sa sendPostedEvents()
*/
void QCoreApplication::flush()
{
    if (self && self->d->eventDispatcher)
        self->d->eventDispatcher->flush();
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
}

extern void set_winapp_name();

// ### move to QCoreApplicationPrivate constructor?
void QCoreApplication::init()
{
#ifdef Q_WS_WIN
    // Get the application name/instance if qWinMain() was not invoked
    set_winapp_name();
#endif

#ifndef QT_NO_COMPONENT
    d->app_libpaths = 0;
#endif

    Q_ASSERT_X(!self, "QCoreApplication", "there should be only one application object.");
    self = this;

    QThread::initialize();

    if (!d->eventDispatcher)
        d->createEventDispatcher();
    Q_ASSERT(d->eventDispatcher != 0);
    d->eventDispatcher->setParent(this);
}

/*!
    Destructor.
*/
QCoreApplication::~QCoreApplication()
{
    qt_call_post_routines();

#ifndef QT_NO_COMPONENT
    delete d->app_libpaths;
    d->app_libpaths = 0;
#endif

    self = 0;
    is_app_running = false;

    QThread::cleanup();

    mainData()->eventDispatcher = 0;
    d->eventDispatcher = 0;
}

/*!
  Sends event \a e to \a receiver: \a {receiver}->event(\a e).
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
  complete control; but only one subclass can be qApp.

  \i Installing an event filter on qApp. Such an event filter is able
  to process all events for all widgets, so it's just as powerful as
  reimplementing notify(); furthermore, it's possible to have more
  than one application-global event filter. Global event filters even
  see mouse events for \link QWidget::isEnabled() disabled
  widgets\endlink.

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

bool QCoreApplication::notify(QObject *receiver, QEvent *e)
{
    // no events are delivered after ~QCoreApplication() has started
    if (is_app_closing)
        return true;

    if (receiver == 0) {                        // serious error
        qWarning("QCoreApplication::notify: Unexpected null receiver");
        return true;
    }

    Q_ASSERT_X(QThread::currentThread() == receiver->thread(),
               "QCoreApplication::sendEvent",
               QString::fromLatin1("Cannot send events to objects owned by a different thread "
                                   "(%1). Receiver '%2' (of type '%3') was created in thread %4")
               .arg(QString::number((ulong) QThread::currentThread(), 16))
               .arg(receiver->objectName())
               .arg(QLatin1String(receiver->metaObject()->className()))
               .arg(QString::number((ulong) receiver->thread(), 16))
               .latin1());

#ifdef QT_COMPAT
  if (e->type() == QEvent::ChildRemoved && receiver->d->postedChildInsertedEvents)
      d->removePostedChildInsertedEvents(receiver, static_cast<QChildEvent *>(e)->child());
#endif // QT_COMPAT

    return receiver->isWidgetType() ? false : notify_helper(receiver, e);
}

/*!\internal

  Helper function called by notify()
 */
bool QCoreApplication::notify_helper(QObject *receiver, QEvent * e)
{
    // send to all application event filters
    for (int i = 0; i < d->eventFilters.size(); ++i) {
        register QObject *obj = d->eventFilters.at(i);
        if (obj && obj->eventFilter(receiver,e))
            return true;
    }

    // send to all receiver event filters
    if (receiver != this) {
        for (int i = 0; i < receiver->d->eventFilters.size(); ++i) {
            register QObject *obj = receiver->d->eventFilters.at(i);
            if (obj && obj->eventFilter(receiver,e))
                return true;
        }
    }

    return receiver->event(e);
}

/*!
  Returns true if an application object has not been created yet;
  otherwise returns false.

  \sa closingDown()
*/

bool QCoreApplication::startingUp()
{
    return !is_app_running;
}

/*!
  Returns true if the application objects are being destroyed;
  otherwise returns false.

  \sa startingUp()
*/

bool QCoreApplication::closingDown()
{
    return is_app_closing;
}


/*!
    Processes all pending events until there are no more events to
    process.

    You can call this function occasionally when your program is busy
    performing a long operation (e.g. copying a file).

    \sa exec(), QTimer, QEventLoop::processEvents()
*/
void QCoreApplication::processEvents(QEventLoop::ProcessEventsFlags flags)
{
    QThreadData *data = QThreadData::current();
    if (!data)
        return;
    data->eventDispatcher->processEvents(flags);
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
    QThreadData *data = QThreadData::current();
    if (!data)
        return;
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
    Enters the main event loop and waits until exit() is called or the
    main widget is destroyed, and returns the value that was set to
    exit() (which is 0 if exit() is called via quit()).

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

    \sa quit(), exit(), processEvents(), QApplication::setMainWidget()
*/
int QCoreApplication::exec()
{
    if (!self) {
        qWarning("QApplication::exec() failed: create instantiate QApplication first");
        return -1;
    }
    QThreadData *data = QThreadData::current();
    if (data != mainData()) {
        qWarning("QApplication::exec() failed: must be called from the main thread.");
        return -1;
    }
    if (!data->eventLoops.isEmpty()) {
        qWarning("QApplication::exec() failed: the event loop is already running.");
        return -1;
    }
    QEventLoop eventLoop;
    int returnCode = eventLoop.exec();
    if (self)
        emit self->aboutToQuit();
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
    QThreadData *data = QThreadData::current();
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
    approach is to create the event on the stack, e.g.
    \code
    QMouseEvent me(QEvent::MouseButtonPress, pos, 0, 0);
    QApplication::sendEvent(mainWindow, &me);
    \endcode
    If you create the event on the heap you must delete it.

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

    QThread *thread = receiver->thread();
    QThreadData *data = thread ? QThreadData::get(thread) : mainData();
    Q_ASSERT_X(data != 0, "QCoreApplication::postEvent",
               "Cannot post events to threads without an event loop");

    {
        QMutexLocker locker(&data->postEventList.mutex);

        // if this is one of the compressible events, do compression
        if (receiver->d->postedEvents
            && self && self->compressEvent(event, receiver, &data->postEventList)) {
            delete event;
            return;
        }

        event->posted = true;
        ++receiver->d->postedEvents;
#ifdef QT_COMPAT
        if (event->type() == QEvent::ChildInserted)
            ++receiver->d->postedChildInsertedEvents;
#endif
        data->postEventList.append(QPostEvent(receiver, event));
    }

    if (data->eventDispatcher)
        data->eventDispatcher->wakeUp();
}

/*!
  \internal
  Returns true if \a event should be blocked and deleted
*/
bool QCoreApplication::compressEvent(QEvent *, QObject *, QPostEventList *)
{
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
    QThread *thread = receiver ? receiver->thread() : 0;
    QThreadData *data = thread ? QThreadData::get(thread) : QThreadData::current();
    Q_ASSERT_X(data != 0, "QCoreApplication::sendPostedEvents",
               "Cannot send posted events without an event loop");

#ifdef QT_COMPAT
    // optimize sendPostedEvents(w, QEvent::ChildInserted) calls away
    if (receiver && event_type == QEvent::ChildInserted
        && !receiver->d->postedChildInsertedEvents) {
        return;
    }
    // Make sure the object hierarchy is stable before processing events
    // to avoid endless loops
    if (receiver == 0 && event_type == 0)
        sendPostedEvents(0, QEvent::ChildInserted);
#endif

    QMutexLocker locker(&data->postEventList.mutex);

    if (data->postEventList.size() == 0 || (receiver && !receiver->d->postedEvents))
        return;

    // okay. here is the tricky loop. be careful about optimizing
    // this, it looks the way it does for good reasons.
    int i = data->postEventList.offset;
    const int s = data->postEventList.size();
    while (i < data->postEventList.size()) {
        // avoid live-lock
        if (i >= s)
            break;

        const QPostEvent &pe = data->postEventList.at(i);
        ++i;

        // optimize for recursive calls. In the no-receiver
        // no-event-type case we know that we process all events.
        if (!receiver && !event_type)
            data->postEventList.offset = i;

        if (// event hasn't been sent
            pe.event
            // we send to all receivers
            && (receiver == 0
                // we send to a specific receiver
                || receiver == pe.receiver)
            // we send all types
            && (event_type == 0
                // we send a specific type
                || event_type == pe.event->type())) {
            // first, we diddle the event so that we can deliver
            // it, and that noone will try to touch it later.
            pe.event->posted = false;
            QEvent * e = pe.event;
            QObject * r = pe.receiver;

            --r->d->postedEvents;
            Q_ASSERT(r->d->postedEvents >= 0);
#ifdef QT_COMPAT
            if (e->type() == QEvent::ChildInserted)
                --r->d->postedChildInsertedEvents;
            Q_ASSERT(r->d->postedChildInsertedEvents >= 0);
#endif

            // next, update the data structure so that we're ready
            // for the next event.
            const_cast<QPostEvent &>(pe).event = 0;

            locker.unlock();
            // after all that work, it's time to deliver the event.
            QCoreApplication::sendEvent(r, e);
            locker.relock();

            // update the offset, in case events have been added ore
            // removed
            i = data->postEventList.offset;

            delete e;
            // careful when adding anything below this point - the
            // sendEvent() call might invalidate any invariants this
            // function depends on.
        }
    }

    // clear the global list, i.e. remove everything that was
    // delivered.
    if (!event_type) {
        if (!receiver) {
            const QPostEventList::iterator it = data->postEventList.begin();
            data->postEventList.erase(it, it + i);
            data->postEventList.offset = 0;
        }
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
    if (!receiver) return;
    QThread *thread = receiver->thread();
    QThreadData *data = thread ? QThreadData::get(thread) : QThreadData::current();
    if (!data) return;

    QMutexLocker locker(&data->postEventList.mutex);

    // the QObject destructor calls this function directly.  this can
    // happen while the event loop is in the middle of posting events,
    // and when we get here, we may not have any more posted events
    // for this object.
    if (!receiver->d->postedEvents) return;

    // iterate over the posted event list and delete the events.
    // leave the QPostEvent objects; they'll be deleted by
    // sendPostedEvents().

    int n = data->postEventList.size();
    int j = 0;

    for (int i = 0; i < n; ++i) {
        if (data->postEventList.offset == i)
            data->postEventList.offset = j;

        const QPostEvent &pe = data->postEventList.at(i);
        if (pe.receiver == receiver) {
            if (pe.event) {
                --receiver->d->postedEvents;
#ifdef QT_COMPAT
                if (pe.event->type() == QEvent::ChildInserted)
                    --receiver->d->postedChildInsertedEvents;
#endif
                pe.event->posted = false;
                delete pe.event;
            }
        } else {
            if (i != j)
                data->postEventList.swap(i, j);
            ++j;
        }
    }

    Q_ASSERT(!receiver->d->postedEvents);
#ifdef QT_COMPAT
    Q_ASSERT(!receiver->d->postedChildInsertedEvents);
#endif

    if (data->postEventList.offset == n)
        data->postEventList.offset = j;

    while (j++ < n)
        data->postEventList.removeLast();
}


/*!
  Removes \a event from the queue of posted events, and emits a
  warning message if appropriate.

  \warning This function can be \e really slow. Avoid using it, if
  possible.

  \threadsafe
*/

void QCoreApplication::removePostedEvent(QEvent * event)
{
    if (!event || !event->posted)
        return;

    QThreadData *data = QThreadData::current();
    if (!data) return;

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
                     pe.receiver ? pe.receiver->objectName().local8Bit() : "object");
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

  \value DefaultCodec - the encoding specified by
  QTextCodec::codecForTr() (Latin1 if none has been set)
  \value UnicodeUTF8 - UTF-8

  \sa QObject::tr(), QObject::trUtf8(), QString::fromUtf8()
*/



/*!
  Tells the application to exit with return code 0 (success).
  Equivalent to calling QApplication::exit(0).

  It's common to connect the QApplication::lastWindowClosed() signal
  to quit(), and you also often connect e.g. QAbstractButton::clicked() or
  signals in QAction, QMenu, or QMenuBar to it.

  Example:
  \code
    QPushButton *quitButton = new QPushButton("Quit");
    connect(quitButton, SIGNAL(clicked()), qApp, SLOT(quit()));
  \endcode

  \sa exit() aboutToQuit() QApplication::lastWindowClosed() QAction
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

    d->translators.prepend(mf);

#ifndef QT_NO_TRANSLATION_BUILDER
    if (mf->isEmpty())
        return;
#endif

    QEvent ev(QEvent::LanguageChange);
    QCoreApplication::sendEvent(this, &ev);
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

    if (d->translators.removeAll(mf) && !self->closingDown()) {
        QEvent ev(QEvent::LanguageChange);
        QCoreApplication::sendEvent(this, &ev);
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

  \sa QObject::tr() installTranslator() defaultCodec()
*/

QString QCoreApplication::translate(const char *context, const char *sourceText,
                                    const char *comment, Encoding encoding)
{
    if (!sourceText)
        return QString();

    if (self && !self->d->translators.isEmpty()) {
        QList<QTranslator*>::ConstIterator it;
        QTranslator *mf;
        QString result;
        for (it = self->d->translators.constBegin(); it != self->d->translators.constEnd(); ++it) {
            mf = *it;
            result = mf->findMessage(context, sourceText, comment).translation();
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

#ifndef QT_NO_TEXTCODEC
/*!
    \obsolete

    Sets the codec to be used for tr() calls to \a codec. Use
    QTextCodec::setCodecForTr() instead.
*/
void QCoreApplication::setDefaultCodec(QTextCodec* codec)
{
    QTextCodec::setCodecForTr(codec);
}

/*! \obsolete
  Returns QTextCodec::codecForTr().
*/
QTextCodec* QCoreApplication::defaultCodec() const
{
    return QTextCodec::codecForTr();
}
#endif //QT_NO_TEXTCODEC
#endif //QT_NO_TRANSLATE

/*!
    Returns the directory that contains the application executable.

    For example, if you have installed Qt in the \c{C:\Trolltech\Qt}
    directory, and you run the \c{demo} example, this function will
    return "C:/Trolltech/Qt/examples/demo".

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
    return QFileInfo(applicationFilePath()).path();
}

#ifndef QT_NO_DIR
/*!
    Returns the file path of the application executable.

    For example, if you have installed Qt in the \c{C:\Trolltech\Qt}
    directory, and you run the \c{demo} example, this function will
    return "C:/Trolltech/Qt/examples/demo/demo.exe".

    \warning On Unix, this function assumes that argv[0] contains the file
    name of the executable (which it normally does). It also assumes that
    the current directory hasn't been changed by the application.

    \sa applicationDirPath()
*/
QString QCoreApplication::applicationFilePath()
{
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
    return fi.exists() ? fi.canonicalFilePath() : QString::null;
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
        char *pEnv = qgetenv("PATH");
        QDir currentDir = QDir::current();
        QStringList paths = QString::fromLocal8Bit(pEnv).split(QLatin1String(":"));
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
    return fi.exists() ? fi.canonicalFilePath() : QString::null;
#endif
}
#endif // QT_NO_DIR

/*!
    Returns the number of command line arguments.

    The documentation for argv() describes how to process command line
    arguments.

    \sa argv(), QApplication::QApplication()
*/
int QCoreApplication::argc() const
{
    return d->argc;
}


/*!
    Returns the command-line argument array.

    argv()[0] is the program name, argv()[1] is the first
    argument, and argv()[argc() - 1] is the last argument.

    A QApplication object is constructed by passing \e argc and \e
    argv from the \c main() function. Some of the arguments may be
    recognized as Qt options and removed from the argument vector. For
    example, the X11 version of Qt knows about \c -display, \c -font,
    and a few more options.

    Example:
    \code
        // showargs.cpp - displays program arguments in a list box

        #include <qapplication.h>
        #include <qlistbox.h>

        int main(int argc, char **argv)
        {
            QApplication a(argc, argv);
            QListBox b;
            a.setMainWidget(&b);
            for (int i = 0; i < a.argc(); i++)  // a.argc() == argc
                b.insertItem(a.argv()[i]);      // a.argv()[i] == argv[i]
            b.show();
            return a.exec();
        }
    \endcode

    If you run \c{showargs -display unix:0 -font 9x15bold hello world}
    under X11, the list box contains the three strings "showargs",
    "hello", and "world".

    Qt provides a global pointer, \c qApp, that points to the
    QApplication object, and through which you can access argc() and
    argv() in functions other than main().

    \sa argc(), QApplication::QApplication()
*/
char **QCoreApplication::argv() const
{
    return d->argv;
}


/*!
  \property QCoreApplication::organizationDomain
    \brief the Internet domain of the organization that wrote this application

    The value is used by the QSettings class when it is constructed
    using the empty constructor. This saves having to repeat this
    information each time a QSettings object is created.

    \sa applicationName
*/
void QCoreApplication::setOrganizationDomain(const QString &organization)
{
    self->d->organization = organization;
}

QString QCoreApplication::organizationDomain()
{
    return self ? self->d->organization : QString();
}

/*!
  \property QCoreApplication::applicationName
    \brief the name of this application

    The value is used by the QSettings class when it is constructed
    using the empty constructor. This saves having to repeat this
    information each time a QSettings object is created.

    \sa organizationDomain
*/
void QCoreApplication::setApplicationName(const QString &application)
{
    self->d->application = application;
}


QString QCoreApplication::applicationName()
{
    return self ? self->d->application : QString();
}



#ifndef QT_NO_COMPONENT

/*!
  Returns a list of paths that the application will search when
  dynamically loading libraries.
  The installation directory for plugins is the only entry if no
  paths have been set.  The default installation directory for plugins
  is \c INSTALL/plugins, where \c INSTALL is the directory where Qt was
  installed. The directory of the application executable (NOT the
  working directory) is also added to the plugin paths.

  If you want to iterate over the list, you should iterate over a
  copy, e.g.
    \code
    QStringList list = app.libraryPaths();
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode

  See the \link plugins-howto.html plugins documentation\endlink for a
  description of how the library paths are used.

  \sa setLibraryPaths(), addLibraryPath(), removeLibraryPath(), QLibrary
*/
QStringList QCoreApplication::libraryPaths()
{
    if (!self)
        return QStringList();
    if (!self->d->app_libpaths) {
        QStringList *app_libpaths = self->d->app_libpaths = new QStringList;
        QString installPathPlugins = QString::fromLocal8Bit(qInstallPathPlugins());
        if (QFile::exists(installPathPlugins)) {
#ifdef Q_WS_WIN
            installPathPlugins.replace(QLatin1Char('\\'), QLatin1Char('/'));
#endif
            app_libpaths->append(installPathPlugins);
        }

        QString app_location(self->applicationFilePath());
        app_location.truncate(app_location.lastIndexOf(QLatin1Char('/')));
        if (app_location != QString::fromLocal8Bit(qInstallPathPlugins()) && QFile::exists(app_location))
            app_libpaths->append(app_location);
    }
    return *self->d->app_libpaths;
}



/*!
  Sets the list of directories to search when loading libraries to \a paths.
  All existing paths will be deleted and the path list will consist of the
  paths given in \a paths.

  \sa libraryPaths(), addLibraryPath(), removeLibraryPath(), QLibrary
 */
void QCoreApplication::setLibraryPaths(const QStringList &paths)
{
    delete self->d->app_libpaths;
    self->d->app_libpaths = new QStringList(paths);
}

/*!
  Append \a path to the end of the library path list. If \a path is
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

    if (!self->d->app_libpaths->contains(path))
        self->d->app_libpaths->prepend(path);
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

    self->d->app_libpaths->removeAll(path);
}

#endif //QT_NO_COMPONENT

/*!
    Sets the event filter \a filter. Returns a pointer to the filter
    function previously defined.

    The event filter is a function that is called for every message
    received in all threads. This does \e not include messages to
    objects that are not handled by Qt.

    The function can return true to stop the event to be processed by
    Qt, or false to continue with the standard event processing.

    Only one filter can be defined, but the filter can use the return
    value to call the previously set event filter. By default, no
    filter is set (ie.  the function returns 0).
*/
QCoreApplication::EventFilter
QCoreApplication::setEventFilter(QCoreApplication::EventFilter filter)
{
    EventFilter old = d->eventFilter;
    d->eventFilter = filter;
    return old;
}

/*!
    Sends \a message through the event filter that was set by
    setEventFilter().  If no event filter has been set, this function
    returns false; otherwise, this function returns the result of the
    event filter function.

    \sa setEventFilter()
*/
bool QCoreApplication::filterEvent(void *message, long *result)
{
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

#ifdef QT_COMPAT
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

    See \l lock() for details.
*/

/*! \fn bool QCoreApplication::tryLock()

    This function does nothing. It is there to keep old code working.
    It always returns false.

    See \l lock() for details.
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

    This function returns true if there are pending events; otherwise
    returns false. Pending events can be either from the window system
    or posted events using QApplication::postEvent().
*/
bool QCoreApplication::hasPendingEvents()
{
    QAbstractEventDispatcher *eventDispatcher = QAbstractEventDispatcher::instance();
    if (eventDispatcher)
        return eventDispatcher->hasPendingEvents();
    return false;
}

/*! \obsolete

    This function enters the main event loop (recursively). Do not call
    it unless you really know what you are doing.
*/
int QCoreApplication::enter_loop()
{
    QThreadData *data = QThreadData::current();
    if (data != mainData()) {
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
    QThreadData *data = QThreadData::current();
    if (data != mainData()) {
        qWarning("QApplication::enter_loop() failed: must be called from the main thread.");
        return;
    }
    if (!data->eventLoops.isEmpty())
        data->eventLoops.top()->exit();
}

/*! \obsolete

    Returns the current loop level.
*/
int QCoreApplication::loopLevel()
{
    QThreadData *data = QThreadData::current();
    if (!data)
        return -1;
    return data->eventLoops.size();
}
#endif
