#include <qkernelapplication.h>
#include <qeventloop.h>
#include <qevent.h>
#include <qvector.h>

#ifdef QT_THREAD_SUPPORT
# include <qmutex.h>
# include <qthread.h>
#endif

#if defined(QT_ACCESSIBILITY_SUPPORT)
# include <qaccessible.h>
#endif

#include "qkernelapplication_p.h"
#define d d_func()
#define q q_func()

// Definitions for posted events
struct QPostEvent {
    QPostEvent():receiver(0),event(0){}
    QPostEvent( QObject *r, QEvent *e ): receiver( r ), event( e ) {}
    QObject  *receiver;
    QEvent   *event;
};
typedef QVector<QPostEvent> QPostEventList;
static uint postEventCounter = 0;

static QPostEventList postedEvents;	// list of posted events

uint qGlobalPostedEventsCount()
{
    return postedEvents.size();
}


bool	  QKernelApplication::is_app_running = FALSE;	// app starting up if FALSE
bool	  QKernelApplication::is_app_closing = FALSE;	// app closing down if TRUE
QEventLoop *QKernelApplication::eventloop = 0;	// application event loop

#ifdef QT_THREAD_SUPPORT
QMutex *QKernelApplication::qt_mutex		= 0;
static QMutex *postevent_mutex		= 0;
#endif // QT_THREAD_SUPPORT

QKernelApplication *QKernelApplication::self = 0;

QKernelApplicationPrivate::QKernelApplicationPrivate(int &aargc,  char **aargv)
    : QObjectPrivate(), argc(aargc), argv(aargv)
{
}

/*!\internal
 */
QKernelApplication::QKernelApplication(QKernelApplicationPrivate *p)
    : QObject(p, 0, 0)
{
    init();
}

/*!
  Constructs a Qt kernel application. Kernel applications are
  applications without graphical user interface, used for example in
  server processes.
*/
QKernelApplication::QKernelApplication( int &argc, char **argv )
    : QObject(new QKernelApplicationPrivate(argc, argv), 0, 0)
{
    init();
}

// ### move to QKernelApplicationPrivate constructor?
void QKernelApplication::init()
{
    is_app_closing = FALSE;

    if (self)
	qFatal("cannot construct to application objects");
    self = this;

#if defined(QT_THREAD_SUPPORT)
    QThread::initialize();
    qt_mutex = new QMutex( TRUE );
    postevent_mutex = new QMutex( TRUE );
#endif // QT_THREAD_SUPPORT
}

QKernelApplication::~QKernelApplication()
{
#ifdef QT_THREAD_SUPPORT
    delete qt_mutex;
    qt_mutex = 0;
    delete postevent_mutex;
    postevent_mutex = 0;
    QThread::cleanup();
#endif

removePostedEvents(this);
    self = 0;
    is_app_running = FALSE;

}

/*!
    Returns the application event loop. This function will return
    zero if called during and after destroying QKernelApplication.

    To create your own instance of QEventLoop or QEventLoop subclass create
    it before you create the QKernelApplication object.

    \sa QEventLoop
*/
QEventLoop *QKernelApplication::eventLoop()
{
    if (!eventloop && !is_app_closing)
	(void) new QEventLoop(self, 0);
    return eventloop;
}

/*!
  Sends event \a e to \a receiver: \a {receiver}->event(\a e).
  Returns the value that is returned from the receiver's event handler.

  For certain types of events (e.g. mouse and key events),
  the event will be propagated to the receiver's parent and so on up to
  the top-level object if the receiver is not interested in the event
  (i.e., it returns FALSE).

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
  see mouse events for \link QWidget::isEnabled() disabled widgets.

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

bool QKernelApplication::notify( QObject *receiver, QEvent *e )
{
    // no events are delivered after ~QKernelApplication() has started
    if ( is_app_closing )
	return TRUE;

    if ( receiver == 0 ) {			// serious error
	qWarning( "QKernelApplication::notify: Unexpected null receiver" );
	return TRUE;
    }

#ifndef QT_NO_COMPAT
    if (e->type() == QEvent::ChildRemoved && receiver->hasPostedChildInsertedEvents) {

#ifdef QT_THREAD_SUPPORT
	QMutexLocker locker( postevent_mutex );
#endif // QT_THREAD_SUPPORT

	// the QObject destructor calls QObject::removeChild, which calls
	// QKernelApplication::sendEvent() directly.  this can happen while the event
	// loop is in the middle of posting events, and when we get here, we may
	// not have any more posted events for this object.
	bool postedChildInsertEventsRemaining = false;
	// if this is a child remove event and the child insert
	// hasn't been dispatched yet, kill that insert
	QObject * c = ((QChildEvent*)e)->child();
	for (int i = 0; i < postedEvents.size(); ++i) {
	    const QPostEvent &pe = postedEvents.at(i);
	    if (pe.event && pe.receiver == receiver) {
		if (pe.event->type() == QEvent::ChildInserted
		    && ((QChildEvent*)pe.event)->child() == c ) {
		    pe.event->posted = false;
		    delete pe.event;
		    const_cast<QPostEvent &>(pe).event = 0;
		    const_cast<QPostEvent &>(pe).receiver = 0;
		} else {
		    postedChildInsertEventsRemaining = true;
		}
	    }
	    receiver->hasPostedChildInsertedEvents = postedChildInsertEventsRemaining;
	}
    }
#endif // QT_NO_COMPAT

    return receiver->isWidgetType() ? FALSE : notify_helper( receiver, e );
}

/*!\internal

  Helper function called by notify()
 */
bool QKernelApplication::notify_helper( QObject *receiver, QEvent * e)
{
    bool consumed = false;

    // send to all application event filters
    for (int i = 0; i < d->eventFilters.size(); ++i) {
	register QObject *obj = d->eventFilters.at(i);
	if ( obj && obj->eventFilter(receiver,e) ) {
	    consumed = true;
	    goto handled;
	}
    }

    // send to all receiver event filters
    if (receiver != this) {
	for (int i = 0; i < receiver->d->eventFilters.size(); ++i) {
	    register QObject *obj = receiver->d->eventFilters.at(i);
	    if ( obj && obj->eventFilter(receiver,e) ) {
		consumed = true;
		goto handled;
	    }
	}
    }

    consumed = receiver->event( e );

 handled:
    e->spont = false;
    return consumed;
}

/*!
  Returns TRUE if an application object has not been created yet;
  otherwise returns FALSE.

  \sa closingDown()
*/

bool QKernelApplication::startingUp()
{
    return !is_app_running;
}

/*!
  Returns TRUE if the application objects are being destroyed;
  otherwise returns FALSE.

  \sa startingUp()
*/

bool QKernelApplication::closingDown()
{
    return is_app_closing;
}


/*!
  \fn void QKernelApplication::processEvents()

    Processes pending events, for 3 seconds or until there are no more
    events to process, whichever is shorter.

    You can call this function occasionally when your program is busy
    performing a long operation (e.g. copying a file).

    \sa exec(), QTimer, QEventLoop::processEvents()
*/

/*!
    \overload

    Processes pending events for \a maxtime milliseconds or until
    there are no more events to process, whichever is shorter.

    You can call this function occasionally when you program is busy
    doing a long operation (e.g. copying a file).

    \sa exec(), QTimer, QEventLoop::processEvents()
*/
void QKernelApplication::processEvents( int maxtime )
{
    eventLoop()->processEvents( QEventLoop::AllEvents, maxtime );
}

/*! \obsolete
  Waits for an event to occur, processes it, then returns.

  This function is useful for adapting Qt to situations where the
  event processing must be grafted onto existing program loops.

  Using this function in new applications may be an indication of design
  problems.

  \sa processEvents(), exec(), QTimer
*/

void QKernelApplication::processOneEvent()
{
    eventLoop()->processEvents( QEventLoop::AllEvents |
				QEventLoop::WaitForMore );
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

    \sa quit(), exit(), processEvents(), setMainWidget()
*/
int QKernelApplication::exec()
{
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::setRootObject(this);
#endif
    return eventLoop()->exec();
}

/*!
  Tells the application to exit with a return code.

  After this function has been called, the application leaves the main
  event loop and returns from the call to exec(). The exec() function
  returns \a retcode.

  By convention, a \a retcode of 0 means success, and any non-zero
  value indicates an error.

  Note that unlike the C library function of the same name, this
  function \e does return to the caller -- it is event processing that
  stops.

  \sa quit(), exec()
*/
void QKernelApplication::exit( int retcode )
{
    eventLoop()->exit( retcode );
}

/*!
    \obsolete

    This function enters the main event loop (recursively). Do not call
    it unless you really know what you are doing.
*/
int QKernelApplication::enter_loop()
{
    return eventLoop()->enterLoop();
}

/*!
    \obsolete

    This function exits from a recursive call to the main event loop.
    Do not call it unless you are an expert.
*/
void QKernelApplication::exit_loop()
{
    eventLoop()->exitLoop();
}

/*!
    \obsolete

    Returns the current loop level.
*/
int QKernelApplication::loopLevel() const
{
    return eventLoop()->loopLevel();
}


/*****************************************************************************
  QKernelApplication management of posted events
 *****************************************************************************/

/*!
  Adds the event \a event with the object \a receiver as the receiver of the
  event, to an event queue and returns immediately.

  The event must be allocated on the heap since the post event queue
  will take ownership of the event and delete it once it has been posted.

  When control returns to the main event loop, all events that are
  stored in the queue will be sent using the notify() function.

  \threadsafe

  \sa sendEvent(), notify()
*/

void QKernelApplication::postEvent( QObject *receiver, QEvent *event )
{
    if ( receiver == 0 ) {
	qWarning( "QKernelApplication::postEvent: Unexpected null receiver" );
	delete event;
	return;
    }

#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker( postevent_mutex );
#endif // QT_THREAD_SUPPORT

    postedEvents.ensure_constructed();

    // if this is one of the compressible events, do compression
    if (receiver->hasPostedEvents
	&& (event->type() == QEvent::UpdateRequest
#ifndef QT_NO_COMPAT
	    || event->type() == QEvent::LayoutHint
#endif
	    || event->type() == QEvent::LayoutRequest
	    || event->type() == QEvent::Resize
	    || event->type() == QEvent::Move
	    || event->type() == QEvent::LanguageChange) ) {
	for (int i = 0; i < postedEvents.size(); ++i) {
	    const QPostEvent &cur = postedEvents.at(i);
	    if (cur.receiver != receiver || cur.event == 0 || cur.event->type() != event->type() )
		continue;
	    if ( cur.event->type() == QEvent::LayoutRequest
#ifndef QT_NO_COMPAT
			|| cur.event->type() == QEvent::LayoutHint
#endif
			|| cur.event->type() == QEvent::UpdateRequest ) {
		;
	    } else if ( cur.event->type() == QEvent::Resize ) {
		((QResizeEvent *)(cur.event))->s = ((QResizeEvent *)event)->s;
	    } else if ( cur.event->type() == QEvent::Move ) {
		((QMoveEvent *)(cur.event))->p = ((QMoveEvent *)event)->p;
	    } else if ( cur.event->type() == QEvent::LanguageChange ) {
		;
	    } else {
		continue;
	    }
	    delete event;
	    return;
	};
    }

    event->posted = TRUE;
    receiver->hasPostedEvents = true;
#ifndef QT_NO_COMPAT
    if (event->type() == QEvent::ChildInserted)
	receiver->hasPostedChildInsertedEvents = true;
#endif
    postedEvents.append( QPostEvent( receiver, event ) );
    ++postEventCounter;

    if (eventloop)
	eventloop->wakeUp();
}


/*!
  \fn void QKernelApplication::sendPostedEvents()
  \overload

    Dispatches all posted events, i.e. empties the event queue.
*/



/*!
  Immediately dispatches all events which have been previously queued
  with QKernelApplication::postEvent() and which are for the object \a receiver
  and have the event type \a event_type.

  Note that events from the window system are \e not dispatched by this
  function, but by processEvents().

  If \a receiver is null, the events of \a event_type are sent for all
  objects. If \a event_type is 0, all the events are sent for \a receiver.
*/

void QKernelApplication::sendPostedEvents( QObject *receiver, int event_type )
{
    static int skipSafely = 0;

    if ( !postedEvents || ( receiver && !receiver->hasPostedEvents ) )
	return;

#ifndef QT_NO_COMPAT
    // optimize sendPostedEvents(w, QEvent::ChildInserted) calls away
    if (receiver && event_type == QEvent::ChildInserted && !receiver->hasPostedChildInsertedEvents)
	return;
    // Make sure the object hierarchy is stable before processing events
    // to avoid endless loops
    if ( receiver == 0 && event_type == 0 )
	sendPostedEvents( 0, QEvent::ChildInserted );
#endif

#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker( postevent_mutex );
#endif

    // okay. here is the tricky loop. be careful about optimizing
    // this, it looks the way it does for good reasons.
    int i = skipSafely;
    while (i < postedEvents.size()) {
	const QPostEvent &pe = postedEvents.at(i);
	++i;

	// optimize for recursive calls. In the no-receiver
	// no-event-type case we know that we process all events.
	if (!receiver && !event_type)
	    skipSafely = i;

	if ( pe.event // hasn't been sent yet
	     && ( receiver == 0 // we send to all receivers
		  || receiver == pe.receiver ) // we send to THAT receiver
	     && ( event_type == 0 // we send all types
		  || event_type == pe.event->type() ) ) { // we send THAT type
	    // first, we diddle the event so that we can deliver
	    // it, and that noone will try to touch it later.
	    pe.event->posted = FALSE;
	    QEvent * e = pe.event;
	    QObject * r = pe.receiver;

	    // next, update the data structure so that we're ready
	    // for the next event.
	    const_cast<QPostEvent &>(pe).event = 0;

	    // remember postEventCounter, so we know when events get
	    // posted or removed.
	    uint backup = postEventCounter;
#ifdef QT_THREAD_SUPPORT
	    if ( locker.mutex() ) locker.mutex()->unlock();
#endif // QT_THREAD_SUPPORT
	    // after all that work, it's time to deliver the event.
	    if ( e->type() == QEvent::PolishRequest) {
		r->ensurePolished();
	    } else {
		QKernelApplication::sendEvent( r, e );
	    }
#ifdef QT_THREAD_SUPPORT
	    if ( locker.mutex() ) locker.mutex()->lock();
#endif // QT_THREAD_SUPPORT
	    if (backup != postEventCounter) // events got posted or removed ...
		i = skipSafely; // ... so start all over again.

	    delete e;
	    // careful when adding anything below this point - the
	    // sendEvent() call might invalidate any invariants this
	    // function depends on.
	}
    }

    // clear the global list, i.e. remove everything that was
    // delivered and update the hasPostedEvents cache.
    if (!event_type) {
	if (!receiver) {
	    for (i = 0; i < postedEvents.size(); ++i)
		if ((receiver = postedEvents.at(i).receiver)) {
		    receiver->hasPostedEvents = false;
#ifndef QT_NO_COMPAT
		    receiver->hasPostedChildInsertedEvents = false;
#endif
		}
	    postedEvents.clear();
	    postEventCounter = 0;
	    skipSafely = 0;
	} else {
	    receiver->hasPostedEvents = false;
#ifndef QT_NO_COMPAT
	    receiver->hasPostedChildInsertedEvents = false;
#endif
	}
    }
#ifndef QT_NO_COMPAT
    else if (event_type == QEvent::ChildInserted) {
	if (!receiver) {
	    for (i = 0; i < postedEvents.size(); ++i)
		if ((receiver = postedEvents.at(i).receiver))
		    receiver->hasPostedChildInsertedEvents = false;
	} else {
	    receiver->hasPostedChildInsertedEvents = false;
	}
    }
#endif
}

/*!
  Removes all events posted using postEvent() for \a receiver.

  The events are \e not dispatched, instead they are removed from the
  queue. You should never need to call this function. If you do call it,
  be aware that killing events may cause \a receiver to break one or
  more invariants.

  \threadsafe
*/

void QKernelApplication::removePostedEvents( QObject *receiver )
{
    if ( !receiver )
	return;

#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker( postevent_mutex );
#endif // QT_THREAD_SUPPORT

    // the QObject destructor calls this function directly.  this can
    // happen while the event loop is in the middle of posting events,
    // and when we get here, we may not have any more posted events
    // for this object.
    if ( !receiver->hasPostedEvents )
 	return;

    // iterate over the object-specific list and delete the events.
    // leave the QPostEvent objects; they'll be deleted by
    // sendPostedEvents().
    receiver->hasPostedEvents = false;
    for (int i = 0; i < postedEvents.size(); ++i) {
	const QPostEvent &pe = postedEvents.at(i);
	if (pe.receiver == receiver) {
	    if (pe.event) {
		pe.event->posted = false;
		delete pe.event;
		const_cast<QPostEvent &>(pe).event = 0;
	    }
	    const_cast<QPostEvent &>(pe).receiver = 0;
	}
    }
}


/*!
  Removes \a event from the queue of posted events, and emits a
  warning message if appropriate.

  \warning This function can be \e really slow. Avoid using it, if
  possible.

  \threadsafe
*/

void QKernelApplication::removePostedEvent( QEvent * event )
{
    if ( !event || !event->posted )
	return;

    if ( !postedEvents ) {
#if defined(QT_DEBUG)
	qDebug( "QKernelApplication::removePostedEvent: %p %d is posted: impossible",
		(void*)event, event->type() );
	return;
#endif
    }

#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker( postevent_mutex );
#endif // QT_THREAD_SUPPORT

    for (int i = 0; i < postedEvents.size(); ++i) {
	const QPostEvent & pe = postedEvents.at(i);
	if ( pe.event == event ) {
#if defined(QT_DEBUG)
	    const char *n;
	    switch ( event->type() ) {
	    case QEvent::Timer:
		n = "Timer";
		break;
	    case QEvent::MouseButtonPress:
		n = "MouseButtonPress";
		break;
	    case QEvent::MouseButtonRelease:
		n = "MouseButtonRelease";
		break;
	    case QEvent::MouseButtonDblClick:
		n = "MouseButtonDblClick";
		break;
	    case QEvent::MouseMove:
		n = "MouseMove";
		break;
#ifndef QT_NO_WHEELEVENT
	    case QEvent::Wheel:
		n = "Wheel";
		break;
#endif
	    case QEvent::KeyPress:
		n = "KeyPress";
		break;
	    case QEvent::KeyRelease:
		n = "KeyRelease";
		break;
	    case QEvent::FocusIn:
		n = "FocusIn";
		break;
	    case QEvent::FocusOut:
		n = "FocusOut";
		break;
	    case QEvent::Enter:
		n = "Enter";
		break;
	    case QEvent::Leave:
		n = "Leave";
		break;
	    case QEvent::Paint:
		n = "Paint";
		break;
	    case QEvent::Move:
		n = "Move";
		break;
	    case QEvent::Resize:
		n = "Resize";
		break;
	    case QEvent::Create:
		n = "Create";
		break;
	    case QEvent::Destroy:
		n = "Destroy";
		break;
	    case QEvent::Close:
		n = "Close";
		break;
	    case QEvent::Quit:
		n = "Quit";
		break;
	    default:
		n = "<other>";
		break;
	    }
	    qWarning("QEvent: Warning: %s event deleted while posted to %s %s",
		     n,
		     pe.receiver ? pe.receiver->className() : "null",
		     pe.receiver ? pe.receiver->name() : "object" );
	    // note the beautiful uglehack if !pe->receiver :)
#endif
	    pe.event->posted = false;
	    delete pe.event;
	    const_cast<QPostEvent &>(pe).event = 0;
	    return;
	}
    }
}

/*!
    This function returns TRUE if there are pending events; otherwise
    returns FALSE. Pending events can be either from the window system
    or posted events using QApplication::postEvent().
*/
bool QKernelApplication::hasPendingEvents()
{
    return eventLoop()->hasPendingEvents();
}

/*!\reimp

*/
bool QKernelApplication::event( QEvent *e )
{
    if (e->type() == QEvent::Quit) {
	quit();
	return TRUE;
    }
    return QObject::event(e);
}

/*!
  Tells the application to exit with return code 0 (success).
  Equivalent to calling QApplication::exit( 0 ).

  It's common to connect the lastWindowClosed() signal to quit(), and
  you also often connect e.g. QButton::clicked() or signals in
  QAction, QPopupMenu or QMenuBar to it.

  Example:
  \code
    QPushButton *quitButton = new QPushButton( "Quit" );
    connect( quitButton, SIGNAL(clicked()), qApp, SLOT(quit()) );
  \endcode

  \sa exit() aboutToQuit() lastWindowClosed() QAction
*/

void QKernelApplication::quit()
{
    exit( 0 );
}


/*! \fn void QKernelApplication::lock()

  Lock the Qt Library Mutex. If another thread has already locked the
  mutex, the calling thread will block until the other thread has
  unlocked the mutex.

  \sa unlock() locked() \link threads.html Thread Support in Qt\endlink
*/


/*! \fn void QKernelApplication::unlock(bool wakeUpMainThread)

  Unlock the Qt Library Mutex. If \a wakeUpMainThread is TRUE (the default),
  then the main thread will be woken up.

  \sa lock(), locked() \link threads.html Thread Support in Qt\endlink
*/


/*! \fn bool QKernelApplication::locked()

  Returns TRUE if the Qt Library Mutex is locked by a different thread;
  otherwise returns FALSE.

  \warning Due to different implementations of recursive mutexes on
  the supported platforms, calling this function from the same thread
  that previously locked the mutex will give undefined results.

  \sa lock() unlock() \link threads.html Thread Support in Qt\endlink
*/

/*! \fn bool QKernelApplication::tryLock()

  Attempts to lock the Qt Library Mutex, and returns immediately. If
  the lock was obtained, this function returns TRUE. If another thread
  has locked the mutex, this function returns FALSE, instead of
  waiting for the lock to become available.

  The mutex must be unlocked with unlock() before another thread can
  successfully lock it.

  \sa lock(), unlock() \link threads.html Thread Support in Qt\endlink
*/

#if defined(QT_THREAD_SUPPORT)
void QKernelApplication::lock()
{
    qt_mutex->lock();
}

void QKernelApplication::unlock(bool wakeUpMainThread)
{
    qt_mutex->unlock();

    if (wakeUpMainThread)
	eventloop->wakeUp();
}

bool QKernelApplication::locked()
{
    return qt_mutex->isLocked();
}

bool QKernelApplication::tryLock()
{
    return qt_mutex->tryLock();
}
#endif
