#include <qkernelapplication.h>
#include <qeventloop.h>
#include <qkernelevent.h>
#include <qfile.h>
#include <qtextcodec.h>

#ifdef Q_WS_QWS
#include <qevent.h>
#endif

#if defined(QT_THREAD_SUPPORT)
#  include <qmutex.h>
#  include <qthread.h>
#endif

#include "qkernelapplication_p.h"
#include "qeventloop_p.h"

#define d d_func()
#define q q_func()


#ifdef Q_WS_WIN
extern const char *qAppFileName(); // Declared in qapplication_win.cpp
#endif


// from qeventloop.cpp
extern QEventLoopPrivate *qt_find_eventloop_private(Qt::HANDLE thread);


typedef void (*VFPTR)();
typedef QList<VFPTR> QVFuncList;
static QVFuncList *postRList = 0;		// list of post routines

void qAddPostRoutine(QtCleanUpFunction p)
{
    if ( !postRList )
	postRList = new QVFuncList;
    postRList->prepend( p );
}

void qRemovePostRoutine(QtCleanUpFunction p)
{
    if ( !postRList ) return;
    QVFuncList::Iterator it = postRList->begin();
    while ( it != postRList->end() ) {
	if ( *it == p ) {
	    postRList->remove( it );
	    it = postRList->begin();
	} else {
	    ++it;
	}
    }
}


// app starting up if FALSE
bool QKernelApplication::is_app_running = FALSE;
 // app closing down if TRUE
bool QKernelApplication::is_app_closing = FALSE;


uint qGlobalPostedEventsCount()
{
#ifdef QT_THREAD_SUPPORT
    QEventLoopPrivate *p = qt_find_eventloop_private(QThread::currentThread());
#else
    QEventLoopPrivate *p = qt_find_eventloop_private(0);
#endif
    return p->postedEvents->size();
}


QKernelApplication *QKernelApplication::self = 0;


QKernelApplicationPrivate::QKernelApplicationPrivate(int &aargc,  char **aargv)
    : QObjectPrivate(), argc(aargc), argv(aargv)
{
    static const char *empty = "";
    if ( argc == 0 || argv == 0 ) {
	argc = 0;
	argv = (char **)&empty; // ouch! careful with QApplication::argv()!
    }
}

/*!\internal
 */
QKernelApplication::QKernelApplication(QKernelApplicationPrivate *p, QEventLoop *e)
    : QObject(p, 0, 0)
{
    init();
    e->setParent(this);
}

/*!
    Flushes the platform specific event queues.

    If you are doing graphical changes inside a loop that does not
    return to the event loop on asynchronous window systems like X11
    or double buffered window systems like MacOS X, and you want to
    visualize these changes immediately (e.g. Splash Screens), call
    this function.

    \sa sendPostedEvents() QPainter::flush()
*/
void QKernelApplication::flush()
{
    if(self)
	self->eventLoop()->flush();
}

/*!
    Constructs a Qt kernel application. Kernel applications are
    applications without a graphical user interface. These type of
    applications are used at the console or as server processes.

    The \a argc and \a argv arguments are available from argc() and
    argv().
*/
QKernelApplication::QKernelApplication( int &argc, char **argv )
    : QObject(new QKernelApplicationPrivate(argc, argv), 0, 0)
{
    init();
}

extern void set_winapp_name();

// ### move to QKernelApplicationPrivate constructor?
void QKernelApplication::init()
{
#ifdef Q_WS_WIN
    // Get the application name/instance if qWinMain() was not invoked
    set_winapp_name();
#endif

    is_app_closing = FALSE;

#ifndef QT_NO_COMPONENT
    d->app_libpaths = 0;
#endif

    Q_ASSERT_X(!self, "QKernelApplication", "there should be only one application object.");
    self = this;

#if defined(QT_THREAD_SUPPORT)
    QThread::initialize();
#endif // QT_THREAD_SUPPORT

    QEventLoop *eventloop = QEventLoop::instance();
    if (!eventloop) (void) new QEventLoop(self, 0);
}

QKernelApplication::~QKernelApplication()
{
    if ( postRList ) {
	QVFuncList::Iterator it = postRList->begin();
	while ( it != postRList->end() ) {	// call post routines
	    (**it)();
	    postRList->remove( it );
	    it = postRList->begin();
	}
	delete postRList;
	postRList = 0;
    }

#ifndef QT_NO_COMPONENT
    delete d->app_libpaths;
    d->app_libpaths = 0;
#endif

    self = 0;
    is_app_running = FALSE;

#ifdef QT_THREAD_SUPPORT
    QThread::cleanup();
#endif
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
    if (!self) return 0;
#if defined(QT_THREAD_SUPPORT)
    return QEventLoop::instance(self->thread());
#else
    return QEventLoop::instance();
#endif
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

bool QKernelApplication::notify( QObject *receiver, QEvent *e )
{
    // no events are delivered after ~QKernelApplication() has started
    if ( is_app_closing )
	return TRUE;

    if ( receiver == 0 ) {			// serious error
	qWarning( "QKernelApplication::notify: Unexpected null receiver" );
	return TRUE;
    }

#if defined(QT_THREAD_SUPPORT)
    Q_ASSERT_X(QThread::currentThread() == receiver->thread(),
	       "QKernelApplication::sendEvent",
	       QString("Cannot send events to objects owned by a different thread (%1).  "
		       "Receiver '%2' (of type '%3') was created in thread %4")
	       .arg(QString::number((ulong) QThread::currentThread(), 16))
	       .arg(receiver->objectName())
	       .arg(receiver->className())
	       .arg(QString::number((ulong) receiver->thread(), 16)));
#endif

#ifndef QT_NO_COMPAT
    if (e->type() == QEvent::ChildRemoved && receiver->hasPostedChildInsertedEvents) {
	QEventLoop *eventloop = QEventLoop::instance();
	QPostEventList *postedEvents = eventloop->d->postedEvents;

#if defined(QT_THREAD_SUPPORT)
	QMutexLocker locker(&postedEvents->mutex);
#endif // QT_THREAD_SUPPORT

	// the QObject destructor calls QObject::removeChild, which calls
	// QKernelApplication::sendEvent() directly.  this can happen while the event
	// loop is in the middle of posting events, and when we get here, we may
	// not have any more posted events for this object.
	bool postedChildInsertEventsRemaining = false;
	// if this is a child remove event and the child insert
	// hasn't been dispatched yet, kill that insert
	QObject * c = ((QChildEvent*)e)->child();
	for (int i = 0; i < postedEvents->size(); ++i) {
	    const QPostEvent &pe = postedEvents->at(i);
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
    eventLoop()->processEvents( QEventLoop::AllEvents | QEventLoop::WaitForMore );
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

    /*
      work around a chicken-or-the-egg problem.  we have to have a
      QEventLoop object to be able to post events (since the post
      event list is stored in the QEventLoopPrivate).  however,
      QEventLoop is a QObject subclass, and the QObject constructor
      posts events before the QEventLoop constructor is called.

      solution: since the QEventLoopPrivate will be fully constructed
      BEFORE the QObject constructor is called, we get access to the
      QEventLoopPrivate directly using qt_find_eventloop_private().
    */
#ifdef QT_THREAD_SUPPORT
    QEventLoopPrivate *p = qt_find_eventloop_private(receiver->thread());
#else
    QEventLoopPrivate *p = qt_find_eventloop_private(0);
#endif
    Q_ASSERT_X(p && p->postedEvents, "QKernelApplication::postEvent",
	       "Cannot post events to threads without an event loop");
    QEventLoop *eventloop = p->initialized ? p->q : 0;
    QPostEventList *postedEvents = p->postedEvents;

#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker(&postedEvents->mutex);
#endif // QT_THREAD_SUPPORT

    // if this is one of the compressible events, do compression
    if (receiver->hasPostedEvents
	&& (event->type() == QEvent::UpdateRequest
#ifndef QT_NO_COMPAT
	    || event->type() == QEvent::LayoutHint
#endif
	    || event->type() == QEvent::LayoutRequest
	    || event->type() == QEvent::Resize
	    || event->type() == QEvent::Move
#ifdef Q_WS_QWS
	    || event->type() == QEvent::QWSUpdate
#endif
	    || event->type() == QEvent::LanguageChange) ) {
	for (int i = 0; i < postedEvents->size(); ++i) {
	    const QPostEvent &cur = postedEvents->at(i);
	    if (cur.receiver != receiver || cur.event == 0 || cur.event->type() != event->type() )
		continue;
	    if ( cur.event->type() == QEvent::LayoutRequest
#ifndef QT_NO_COMPAT
		 || cur.event->type() == QEvent::LayoutHint
#endif
		 || cur.event->type() == QEvent::UpdateRequest ) {
		;
		// 	    }
		// 	    else if ( cur.event->type() == QEvent::Resize ) {
		// 		((QResizeEvent *)(cur.event))->s = ((QResizeEvent *)event)->s;
		// 	    } else if ( cur.event->type() == QEvent::Move ) {
		// 		((QMoveEvent *)(cur.event))->p = ((QMoveEvent *)event)->p;
#ifdef Q_WS_QWS
	    } else if ( cur.event->type() == QEvent::QWSUpdate ) {
		QPaintEvent * p = (QPaintEvent*)(cur.event);
		p->reg = p->reg.unite( ((QPaintEvent *)event)->reg );
		p->rec = p->rec.unite( ((QPaintEvent *)event)->rec );
#endif
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
    postedEvents->append( QPostEvent( receiver, event ) );

    if (eventloop) eventloop->wakeUp();
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
    QEventLoop *eventloop = QEventLoop::instance();
    Q_ASSERT_X(eventloop && eventloop->d->postedEvents,
	       "QKernelApplication::sendPostedEvents",
	       "Cannot send events without an event loop");
    QPostEventList *postedEvents = eventloop->d->postedEvents;

#ifndef QT_NO_COMPAT
    // optimize sendPostedEvents(w, QEvent::ChildInserted) calls away
    if (receiver && event_type == QEvent::ChildInserted && !receiver->hasPostedChildInsertedEvents)
	return;
    // Make sure the object hierarchy is stable before processing events
    // to avoid endless loops
    if ( receiver == 0 && event_type == 0 )
	sendPostedEvents( 0, QEvent::ChildInserted );
#endif

#if defined(QT_THREAD_SUPPORT)
    QMutexLocker locker(&postedEvents->mutex);
#endif

    if (!*postedEvents || (receiver && !receiver->hasPostedEvents))
	return;

    // okay. here is the tricky loop. be careful about optimizing
    // this, it looks the way it does for good reasons.
    int i = postedEvents->offset;
    while (i < postedEvents->size()) {
	const QPostEvent &pe = postedEvents->at(i);
	++i;

	// optimize for recursive calls. In the no-receiver
	// no-event-type case we know that we process all events.
	if (!receiver && !event_type)
	    postedEvents->offset = i;

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
	    pe.event->posted = FALSE;
	    QEvent * e = pe.event;
	    QObject * r = pe.receiver;

	    // next, update the data structure so that we're ready
	    // for the next event.
	    const_cast<QPostEvent &>(pe).event = 0;

	    // remember postEventCounter, so we know when events get
	    // posted or removed.
	    int backup = postedEvents->size();
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
	    if (backup != postedEvents->size()) // events got posted or removed ...
		i = postedEvents->offset; // ... so start all over again.

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
	    for (i = 0; i < postedEvents->size(); ++i) {
		if ((receiver = postedEvents->at(i).receiver)) {
		    receiver->hasPostedEvents = false;
#ifndef QT_NO_COMPAT
		    receiver->hasPostedChildInsertedEvents = false;
#endif
		}
	    }
	    postedEvents->clear();
	    postedEvents->offset = 0;
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
	    for (i = 0; i < postedEvents->size(); ++i)
		if ((receiver = postedEvents->at(i).receiver))
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
    if (!receiver) return;

#if defined(QT_THREAD_SUPPORT)
    QEventLoop *eventloop = QEventLoop::instance(receiver->thread());
#else
    QEventLoop *eventloop = QEventLoop::instance();
#endif
    if (!eventloop || !eventloop->d->postedEvents) return;

    QPostEventList *postedEvents = eventloop->d->postedEvents;

#ifdef QT_THREAD_SUPPORT
    QMutexLocker locker(&postedEvents->mutex);
#endif // QT_THREAD_SUPPORT

    // the QObject destructor calls this function directly.  this can
    // happen while the event loop is in the middle of posting events,
    // and when we get here, we may not have any more posted events
    // for this object.
    if ( !receiver->hasPostedEvents ) return;

    // iterate over the object-specific list and delete the events.
    // leave the QPostEvent objects; they'll be deleted by
    // sendPostedEvents().
    receiver->hasPostedEvents = false;
    for (int i = 0; i < postedEvents->size(); ++i) {
	const QPostEvent &pe = postedEvents->at(i);
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

    QEventLoop *eventloop = QEventLoop::instance();
    if (!eventloop) return;
    QPostEventList *postedEvents = eventloop->d_func()->postedEvents;
    if (!postedEvents) return;

#if defined(QT_THREAD_SUPPORT)
    QMutexLocker locker(&postedEvents->mutex);
#endif

    if ( !*postedEvents ) {
#if defined(QT_DEBUG)
	qDebug( "QKernelApplication::removePostedEvent: %p %d is posted: impossible",
		(void*)event, event->type() );
	return;
#endif
    }

    for (int i = 0; i < postedEvents->size(); ++i) {
	const QPostEvent & pe = postedEvents->at(i);
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

/*! \enum QKernelApplication::Encoding

  This enum type defines the 8-bit encoding of character string
  arguments to translate():

  \value DefaultCodec - the encoding specified by
  QTextCodec::codecForTr() (Latin1 if none has been set)
  \value UnicodeUTF8 - UTF-8

  \sa QObject::tr(), QObject::trUtf8(), QString::fromUtf8()
*/



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
    \obsolete

  Lock the Qt Library Mutex. If another thread has already locked the
  mutex, the calling thread will block until the other thread has
  unlocked the mutex.

  \sa unlock() locked() \link threads.html Thread Support in Qt\endlink
*/


/*! \fn void QKernelApplication::unlock(bool wakeUpMainThread)
    \obsolete

  Unlock the Qt Library Mutex. If \a wakeUpMainThread is TRUE (the default),
  then the main thread will be woken up.

  \sa lock(), locked() \link threads.html Thread Support in Qt\endlink
*/


/*! \fn bool QKernelApplication::locked()
    \obsolete

  Returns TRUE if the Qt Library Mutex is locked by a different thread;
  otherwise returns FALSE.

  \warning Due to different implementations of recursive mutexes on
  the supported platforms, calling this function from the same thread
  that previously locked the mutex will give undefined results.

  \sa lock() unlock() \link threads.html Thread Support in Qt\endlink
*/

/*! \fn bool QKernelApplication::tryLock()
    \obsolete

  Attempts to lock the Qt Library Mutex, and returns immediately. If
  the lock was obtained, this function returns TRUE. If another thread
  has locked the mutex, this function returns FALSE, instead of
  waiting for the lock to become available.

  The mutex must be unlocked with unlock() before another thread can
  successfully lock it.

  \sa lock(), unlock() \link threads.html Thread Support in Qt\endlink
*/

#if defined(QT_THREAD_SUPPORT) && !defined(QT_NO_COMPAT)
void QKernelApplication::lock()
{
}

void QKernelApplication::unlock(bool)
{
}

bool QKernelApplication::locked()
{
    return false;
}

bool QKernelApplication::tryLock()
{
    return false;
}
#endif

/*!
  \fn void QKernelApplication::aboutToQuit()

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

void QKernelApplication::installTranslator( QTranslator * mf )
{
    if ( !mf )
	return;

    d->translators.prepend( mf );

#ifndef QT_NO_TRANSLATION_BUILDER
    if ( mf->isEmpty() )
	return;
#endif

    QEvent ev(QEvent::LanguageChange);
    QKernelApplication::sendEvent(this, &ev);
}

/*!
  Removes the message file \a mf from the list of message files used by
  this application. (It does not delete the message file from the file
  system.)

  \sa installTranslator() translate(), QObject::tr()
*/

void QKernelApplication::removeTranslator( QTranslator * mf )
{
    if (!mf)
	return;

    if ( d->translators.remove( mf ) && !self->closingDown() ) {
	QEvent ev(QEvent::LanguageChange);
	QKernelApplication::sendEvent(this, &ev);
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
  installed \e before calling this method.  Installing or removing
  translators while performing translations is not supported.  Doing
  so will most likely result in crashes or other undesirable behavior.

  \sa QObject::tr() installTranslator() defaultCodec()
*/

QString QKernelApplication::translate( const char * context, const char * sourceText,
				       const char * comment, Encoding encoding ) const
{
    if ( !sourceText )
	return QString::null;

    if (!d->translators.isEmpty()) {
	QList<QTranslator*>::ConstIterator it;
	QTranslator * mf;
	QString result;
	for ( it = d->translators.constBegin(); it != d->translators.constEnd(); ++it ) {
	    mf = *it;
	    result = mf->findMessage( context, sourceText, comment ).translation();
	    if ( !result.isNull() )
		return result;
	}
    }
#ifndef QT_NO_TEXTCODEC
    if ( encoding == UnicodeUTF8 )
	return QString::fromUtf8( sourceText );
    else if ( QTextCodec::codecForTr() != 0 )
	return QTextCodec::codecForTr()->toUnicode( sourceText );
    else
#endif
	return QString::fromLatin1( sourceText );
}

#ifndef QT_NO_TEXTCODEC
/*! \obsolete
  This is the same as QTextCodec::setCodecForTr().
*/
void QKernelApplication::setDefaultCodec( QTextCodec* codec )
{
    QTextCodec::setCodecForTr( codec );
}

/*! \obsolete
  Returns QTextCodec::codecForTr().
*/
QTextCodec* QKernelApplication::defaultCodec() const
{
    return QTextCodec::codecForTr();
}
#endif //QT_NO_TEXTCODEC
#endif //QT_NO_TRANSLATE

#ifndef QT_NO_COMPONENT

/*!
  Returns a list of paths that the application will search when
  dynamically loading libraries.
  The installation directory for plugins is the only entry if no
  paths have been set.  The default installation directory for plugins
  is \c INSTALL/plugins, where \c INSTALL is the directory where Qt was
  installed. On Windows, the directory of the application executable (NOT the
  working directory) is also added to the plugin paths.

  If you want to iterate over the list, you should iterate over a
  copy, e.g.
    \code
    QStringList list = app.libraryPaths();
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  See the \link plugins-howto.html plugins documentation\endlink for a
  description of how the library paths are used.

  \sa setLibraryPaths(), addLibraryPath(), removeLibraryPath(), QLibrary
*/
QStringList QKernelApplication::libraryPaths()
{
    if ( !self->d->app_libpaths ) {
	self->d->app_libpaths = new QStringList;
	if ( QFile::exists( qInstallPathPlugins() ) )
	    self->d->app_libpaths->append( qInstallPathPlugins() );
#ifdef Q_WS_WIN
	QString app_location = qAppFileName();
	app_location.truncate( app_location.findRev( '\\' ) );
	if ( app_location != qInstallPathPlugins() && QFile::exists( app_location ) )
	    self->d->app_libpaths->append( app_location );
#endif
    }
    return *self->d->app_libpaths;
}


/*!
  Sets the list of directories to search when loading libraries to \a paths.
  All existing paths will be deleted and the path list will consist of the
  paths given in \a paths.

  \sa libraryPaths(), addLibraryPath(), removeLibraryPath(), QLibrary
 */
void QKernelApplication::setLibraryPaths( const QStringList &paths )
{
    delete self->d->app_libpaths;
    self->d->app_libpaths = new QStringList( paths );
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
void QKernelApplication::addLibraryPath( const QString &path )
{
    if ( path.isEmpty() )
	return;

    // make sure that library paths is initialized
    libraryPaths();

    if ( !self->d->app_libpaths->contains( path ) )
	self->d->app_libpaths->prepend( path );
}

/*!
  Removes \a path from the library path list. If \a path is empty or not
  in the path list, the list is not changed.

  \sa addLibraryPath(), libraryPaths(), setLibraryPaths()
*/
void QKernelApplication::removeLibraryPath( const QString &path )
{
    if ( path.isEmpty() )
	return;

    // make sure that library paths is initialized
    libraryPaths();

    if ( self->d->app_libpaths->contains( path ) )
	self->d->app_libpaths->remove( path );
}
#endif //QT_NO_COMPONENT
