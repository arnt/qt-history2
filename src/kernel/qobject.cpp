/****************************************************************************
** $Id$
**
** Implementation of QObject class
**
** Created : 930418
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
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

#include "qvariant.h"
#include "qapplication.h"
#include "qobject.h"
#include "qobjectlist.h"
#include "qregexp.h"
#include "qmetaobject.h"
#include <private/qucom_p.h>
#include "qucomextra_p.h"
#include "qptrvector.h"
#include "qobjectlist.h"
#include "qguardedptr.h"

#ifdef QT_THREAD_SUPPORT
#include <qmutex.h>
#include <private/qmutexpool_p.h>
#endif

#include <ctype.h>
#include <limits.h>

#define GUARDED_SIGNAL INT_MIN

struct QObjectPrivate
{
    QObjectPrivate()
	:connections(0),
	 senders(0)
	{
#ifndef QT_NO_USERDATA
	    userData.setAutoDelete(true);
#endif
	}
#ifndef QT_NO_USERDATA
    QPtrVector<QObjectUserData> userData; // #### use new container
#endif

    // signal connections
    struct Connections {
	int count;
	struct Connection{
	    int signal;
	    union {
		QObject *receiver;
		QObject **guarded;
	    };
	    int member;
	} connections[1];
    };
    Connections *connections;
    void addConnection(int signal, QObject *receiver, int member);
    Connections::Connection *findConnection(int signal, int &i);
    void removeReceiver(QObject *receiver);

    // slot connections
    struct Senders
    {
	int ref;
	QObject *current;
	int count;
	struct Sender {
	    int ref;
	    QObject * sender;
	};
	Sender *senders;
	Sender stack[1];
    };
    Senders *senders;
    void refSender(QObject *sender);
    void derefSender(QObject *sender);
    void removeSender(QObject *sender);

    static QObject *setCurrentSender(Senders *senders, QObject *sender);
    static void resetCurrentSender(Senders *senders, QObject *sender);
    static void derefSenders(Senders *senders);
};

void QGuardedPtrData::add(QObject **ptr)
{
    if (!*ptr)
	return;
    (*ptr)->d->addConnection(GUARDED_SIGNAL, reinterpret_cast<QObject*>(ptr), 0);
}

void QGuardedPtrData::remove(QObject **ptr)
{
    if (!*ptr)
	return;
    (*ptr)->d->removeReceiver(reinterpret_cast<QObject*>(ptr));
}

void QGuardedPtrData::replace(QObject **ptr, QObject *o)
{
    remove(ptr);
    *ptr = o;
    add(ptr);
}

/*!
    \class Qt qnamespace.h

    \brief The Qt class is a namespace for miscellaneous identifiers
    that need to be global-like.

    \ingroup misc

    Normally, you can ignore this class. QObject and a few other
    classes inherit it, so all the identifiers in the Qt namespace are
    normally usable without qualification.

    However, you may occasionally need to say \c Qt::black instead of
    just \c black, particularly in static utility functions (such as
    many class factories).

*/

/*!
    \enum Qt::Orientation

    This type is used to signify an object's orientation.

    \value Horizontal
    \value Vertical

    Orientation is used with QScrollBar for example.
*/


/*!
    \class QObject qobject.h
    \brief The QObject class is the base class of all Qt objects.

    \ingroup objectmodel
    \mainclass
    \reentrant

    QObject is the heart of the \link object.html Qt object model.
    \endlink The central feature in this model is a very powerful
    mechanism for seamless object communication called \link
    signalsandslots.html signals and slots \endlink. You can
    connect a signal to a slot with connect() and destroy the
    connection with disconnect(). To avoid never ending notification
    loops you can temporarily block signals with blockSignals(). The
    protected functions connectNotify() and disconnectNotify() make it
    possible to track connections.

    QObjects organize themselves in object trees. When you create a
    QObject with another object as parent, the object will
    automatically do an insertChild() on the parent and thus show up
    in the parent's children() list. The parent takes ownership of the
    object i.e. it will automatically delete its children in its
    destructor. You can look for an object by name and optionally type
    using child() or queryList(), and get the list of tree roots using
    objectTrees().

    Every object has an object name() and can report its className()
    and whether it inherits() another class in the QObject inheritance
    hierarchy.

    When an object is deleted, it emits a destroyed() signal. You can
    catch this signal to avoid dangling references to QObjects. The
    QGuardedPtr class provides an elegant way to use this feature.

    QObjects can receive events through event() and filter the events
    of other objects. See installEventFilter() and eventFilter() for
    details. A convenience handler, childEvent(), can be reimplemented
    to catch child events.

    Last but not least, QObject provides the basic timer support in
    Qt; see QTimer for high-level support for timers.

    Notice that the Q_OBJECT macro is mandatory for any object that
    implements signals, slots or properties. You also need to run the
    \link moc.html moc program (Meta Object Compiler) \endlink on the
    source file. We strongly recommend the use of this macro in \e all
    subclasses of QObject regardless of whether or not they actually
    use signals, slots and properties, since failure to do so may lead
    certain functions to exhibit undefined behaviour.

    All Qt widgets inherit QObject. The convenience function
    isWidgetType() returns whether an object is actually a widget. It
    is much faster than inherits( "QWidget" ).

    Some QObject functions, e.g. children(), objectTrees() and
    queryList() return a QObjectList. A QObjectList is a QPtrList of
    QObjects. QObjectLists support the same operations as QPtrLists
    and have an iterator class, QObjectListIterator.
*/


//
// Remove white space from SIGNAL and SLOT names.
// Internal for QObject::connect() and QObject::disconnect()
//

static inline bool isIdentChar( char x )
{						// Avoid bug in isalnum
    return x == '_' || (x >= '0' && x <= '9') ||
	 (x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z');
}

static inline bool isSpace( char x )
{
#if defined(Q_CC_BOR)
  /*
    Borland C++ 4.5 has a weird isspace() bug.
    isspace() usually works, but not here.
    This implementation is sufficient for our internal use: rmWS()
  */
    return (uchar) x <= 32;
#else
    return isspace( (uchar) x );
#endif
}

// Event functions, implemented in qapplication_xxx.cpp

int   qStartTimer( int interval, QObject *obj );
bool  qKillTimer( int id );
bool  qKillTimer( QObject *obj );

/*!
    \relates QObject

    Returns a pointer to the object named \a name that inherits \a
    type and with a given \a parent.

    Returns 0 if there is no such child.

    \code
	QListBox *c = (QListBox *) qt_find_obj_child( myWidget, "QListBox",
						      "my list box" );
	if ( c )
	    c->insertItem( "another string" );
    \endcode
*/

void *qt_find_obj_child( QObject *parent, const char *type, const char *name )
{
    const QObjectList *list = parent->children();
    if ( list ) {
	QObjectListIterator it( *list );
	QObject *obj;
	while ( (obj = it.current()) ) {
	    ++it;
	    if ( qstrcmp(name,obj->name()) == 0 &&
		 obj->inherits(type) )
		return obj;
	}
    }
    return 0;
}

static QObjectList* object_trees = 0;

#ifdef QT_THREAD_SUPPORT
static QMutex *obj_trees_mutex = 0;
#endif

static void cleanup_object_trees()
{
    delete object_trees;
    object_trees = 0;
#ifdef QT_THREAD_SUPPORT
    delete obj_trees_mutex;
    obj_trees_mutex = 0;
#endif
}

static void ensure_object_trees()
{
    object_trees = new QObjectList;
    qAddPostRoutine( cleanup_object_trees );
}

static void insert_tree( QObject* obj )
{
#ifdef QT_THREAD_SUPPORT
    if ( !obj_trees_mutex )
	obj_trees_mutex = new QMutex();
    QMutexLocker locker( obj_trees_mutex );
#endif
    if ( !object_trees )
	ensure_object_trees();
    object_trees->insert(0, obj );
}

static void remove_tree( QObject* obj )
{
    if ( object_trees ) {
#ifdef QT_THREAD_SUPPORT
	QMutexLocker locker( obj_trees_mutex );
#endif
	object_trees->removeRef( obj );
    }
}


/*****************************************************************************
  QObject member functions
 *****************************************************************************/

/*!
    Constructs an object called \a name with parent object, \a parent.

    The parent of an object may be viewed as the object's owner. For
    instance, a \link QDialog dialog box\endlink is the parent of the
    "OK" and "Cancel" buttons it contains.

    The destructor of a parent object destroys all child objects.

    Setting \a parent to 0 constructs an object with no parent. If the
    object is a widget, it will become a top-level window.

    The object name is some text that can be used to identify a
    QObject. It's particularly useful in conjunction with \link
    designer-manual.book <i>Qt Designer</i>\endlink. You can find an
    object by name (and type) using child(). To find several objects
    use queryList().

    \sa parent(), name(), child(), queryList()
*/

QObject::QObject( QObject *parent, const char *name )
    :
    isSignal( FALSE ),				// assume not a signal object
    isWidget( FALSE ), 				// assume not a widget object
    pendTimer( FALSE ),				// no timers yet
    blockSig( FALSE ),      			// not blocking signals
    wasDeleted( FALSE ),       			// double-delete catcher
    isTree( FALSE ), 				// no tree yet
    objname( name ? qstrdup(name) : 0 ),        // set object name
    parentObj( 0 ),				// no parent yet. It is set by insertChild()
    childObjects( 0 ), 				// no children yet
    eventFilters( 0 ), 				// no filters installed
    postedEvents( 0 ), 				// no events posted
    d( new QObjectPrivate )
{
    if ( parent ) {				// add object to parent
	parent->insertChild( this );
    } else {
	insert_tree( this );
	isTree = TRUE;
    }
}


/*!
    Destroys the object, deleting all its child objects.

    All signals to and from the object are automatically disconnected.

    \warning All child objects are deleted. If any of these objects
    are on the stack or global, sooner or later your program will
    crash. We do not recommend holding pointers to child objects from
    outside the parent. If you still do, the QObject::destroyed()
    signal gives you an opportunity to detect when an object is
    destroyed.

    \warning Deleting a QObject while pending events are waiting to be
    delivered can cause a crash.  You must not delete the QObject
    directly from a thread that is not the GUI thread.  Use the
    QObject::deleteLater() method instead, which will cause the event
    loop to delete the object after all pending events have been
    delivered to the object.
*/

QObject::~QObject()
{
    if ( wasDeleted ) {
#if defined(QT_DEBUG)
	qWarning( "Double QObject deletion detected." );
#endif
	return;
    }
    wasDeleted = 1;

    blockSig = 0; // unblock signals so we always emit destroyed()
    emit destroyed( this );

    // reset all guarded pointers
    int i = 0;
    QObjectPrivate::Connections::Connection *c;
    while ((c = d->findConnection(GUARDED_SIGNAL, i))) {
	*c->guarded = 0;
	c->guarded = 0;
    }

    if ( objname )
	delete [] (char*)objname;
    objname = 0;
    if ( pendTimer )				// might be pending timers
	qKillTimer( this );
    QApplication::removePostedEvents( this );
    if ( isTree ) {
	remove_tree( this );		// remove from global root list
	isTree = FALSE;
    }
    if ( parentObj )				// remove it from parent object
	parentObj->removeChild( this );

    // disconnect senders
    if (d->senders) {
	for (int i = 0; i < d->senders->count; ++i) {
	    QObjectPrivate::Senders::Sender &sender = d->senders->senders[i];
	    if (sender.sender)
		sender.sender->d->removeReceiver(this);
	}
 	QObjectPrivate::derefSenders(d->senders);
	d->senders = 0;
    }

    // disconnect receivers
    if (d->connections) {
	for (int i = 0; i < d->connections->count; ++i) {
	    QObjectPrivate::Connections::Connection &c
		= d->connections->connections[i];
	    if (c.receiver)
		c.receiver->d->removeSender(this);
	}
	qFree(d->connections);
	d->connections = 0;
    }

    if ( eventFilters ) {
	delete eventFilters;
	eventFilters = 0;
    }
    if ( childObjects ) {			// delete children objects
	QObject *obj;
	QObjectListIterator it(*childObjects);
	while ( (obj=it.current()) ) {
	    ++it;
	    obj->parentObj = 0;
	    childObjects->removeRef( obj );
	    delete obj;
	}
	delete childObjects;
    }

    delete d;
}


/*!
    \fn QMetaObject *QObject::metaObject() const

    Returns a pointer to the meta object of this object.

    A meta object contains information about a class that inherits
    QObject, e.g. class name, superclass name, properties, signals and
    slots. Every class that contains the Q_OBJECT macro will also have
    a meta object.

    The meta object information is required by the signal/slot
    connection mechanism and the property system. The functions isA()
    and inherits() also make use of the meta object.
*/

/*!
    Returns the class name of this object.

    \warning This function depends on the \link metaobjects.html Meta
    Object Compiler. \endlink It will return the wrong name if the
    class definion lacks the Q_OBJECT macro.
*/
const char *QObject::className() const
{
    return metaObject()->className();
}

/*!
    \fn bool QObject::isA( const char *clname ) const

    \obsolete
    Returns TRUE if this object is an instance of the class \a clname;
    otherwise returns FALSE.

  Example:
  \code
    QTimer *t = new QTimer; // QTimer inherits QObject
    t->isA( "QTimer" );     // returns TRUE
    t->isA( "QObject" );    // returns FALSE
  \endcode

  \sa inherits() metaObject()
*/


/*!
    \fn bool QObject::inherits( const char *clname ) const

    \obsolete
    Returns TRUE if this object is an instance of a class that
    inherits \a clname, and \a clname inherits QObject; otherwise
    returns FALSE.

    A class is considered to inherit itself.

    Example:
    \code
	QTimer *t = new QTimer;         // QTimer inherits QObject
	t->inherits( "QTimer" );        // returns TRUE
	t->inherits( "QObject" );       // returns TRUE
	t->inherits( "QButton" );       // returns FALSE

	// QScrollBar inherits QWidget and QRangeControl
	QScrollBar *s = new QScrollBar( 0 );
	s->inherits( "QWidget" );       // returns TRUE
	s->inherits( "QRangeControl" ); // returns FALSE
    \endcode

    (\l QRangeControl is not a QObject.)

    \sa isA(), metaObject()
*/

/*!
    \property QObject::name

    \brief the name of this object

    You can find an object by name (and type) using child(). You can
    find a set of objects with queryList().

    The object name is set by the constructor or by the setName()
    function. The object name is not very useful in the current
    version of Qt, but will become increasingly important in the
    future.

    If the object does not have a name, the name() function returns
    "unnamed", so printf() (used in qDebug()) will not be asked to
    output a null pointer. If you want a null pointer to be returned
    for unnamed objects, you can call name( 0 ).

    \code
	qDebug( "MyClass::setPrecision(): (%s) invalid precision %f",
		name(), newPrecision );
    \endcode

    \sa className(), child(), queryList()
*/

const char * QObject::name() const
{
    // If you change the name here, the builder will be broken
    return objname ? objname : "unnamed";
}

/*!
    Sets the object's name to \a name.
*/
void QObject::setName( const char *name )
{
    if ( objname )
	delete [] (char*) objname;
    objname = name ? qstrdup(name) : 0;
}

/*!
    \overload

    Returns the name of this object, or \a defaultName if the object
    does not have a name.
*/

const char * QObject::name( const char * defaultName ) const
{
    return objname ? objname : defaultName;
}


/*!
    Searches the children and optionally grandchildren of this object,
    and returns a child that is called \a objName that inherits \a
    inheritsClass. If \a inheritsClass is 0 (the default), any class
    matches.

    If \a recursiveSearch is TRUE (the default), child() performs a
    depth-first search of the object's children.

    If there is no such object, this function returns 0. If there are
    more than one, the first one found is retured; if you need all of
    them, use queryList().
*/
QObject* QObject::child( const char *objName, const char *inheritsClass,
			 bool recursiveSearch )
{
    const QObjectList *list = children();
    if ( !list )
	return 0;

    bool onlyWidgets = ( inheritsClass && qstrcmp( inheritsClass, "QWidget" ) == 0 );
    QObjectListIterator it( *list );
    QObject *obj;
    while ( ( obj = it.current() ) ) {
	++it;
	if ( onlyWidgets ) {
	    if ( obj->isWidgetType() && ( !objName || qstrcmp( objName, obj->name() ) == 0 ) )
		break;
	} else if ( ( !inheritsClass || obj->inherits(inheritsClass) ) && ( !objName || qstrcmp( objName, obj->name() ) == 0 ) )
	    break;
	if ( recursiveSearch && (obj = obj->child( objName, inheritsClass, recursiveSearch ) ) )
	    break;
    }
    return obj;
}

/*!
    \fn bool QObject::isWidgetType() const

    Returns TRUE if the object is a widget; otherwise returns FALSE.

    Calling this function is equivalent to calling
    inherits("QWidget"), except that it is much faster.
*/

/*!
    \fn bool QObject::highPriority() const

    Returns TRUE if the object is a high-priority object, or FALSE if
    it is a standard-priority object.

    High-priority objects are placed first in QObject's list of
    children on the assumption that they will be referenced very
    often.
*/


/*!
    This virtual function receives events to an object and should
    return TRUE if the event \a e was recognized and processed.

    The event() function can be reimplemented to customize the
    behavior of an object.

    \sa installEventFilter(), timerEvent(), QApplication::sendEvent(),
    QApplication::postEvent(), QWidget::event()
*/

bool QObject::event( QEvent *e )
{
#if defined(QT_CHECK_NULL)
    if ( e == 0 )
	qWarning( "QObject::event: Null events are not permitted" );
#endif
    if ( eventFilters ) {			// try filters
	if ( activate_filters(e) )		// stopped by a filter
	    return TRUE;
    }

    switch ( e->type() ) {
    case QEvent::Timer:
	timerEvent( (QTimerEvent*)e );
	return TRUE;

    case QEvent::ChildInserted:
    case QEvent::ChildRemoved:
	childEvent( (QChildEvent*)e );
	return TRUE;

    case QEvent::DeferredDelete:
	delete this;
	return TRUE;

    default:
	if ( e->type() >= QEvent::User ) {
	    customEvent( (QCustomEvent*) e );
	    return TRUE;
	}
	break;
    }
    return FALSE;
}

/*!
    This event handler can be reimplemented in a subclass to receive
    timer events for the object.

    QTimer provides a higher-level interface to the timer
    functionality, and also more general information about timers.

    \sa startTimer(), killTimer(), killTimers(), event()
*/

void QObject::timerEvent( QTimerEvent * )
{
}


/*!
    This event handler can be reimplemented in a subclass to receive
    child events.

    Child events are sent to objects when children are inserted or
    removed.

    Note that events with QEvent::type() \c QEvent::ChildInserted are
    posted (with \l{QApplication::postEvent()}) to make sure that the
    child's construction is completed before this function is called.

    If a child is removed immediately after it is inserted, the \c
    ChildInserted event may be suppressed, but the \c ChildRemoved
    event will always be sent. In such cases it is possible that there
    will be a \c ChildRemoved event without a corresponding \c
    ChildInserted event.

    If you change state based on \c ChildInserted events, call
    QWidget::constPolish(), or do
    \code
	QApplication::sendPostedEvents( this, QEvent::ChildInserted );
    \endcode
    in functions that depend on the state. One notable example is
    QWidget::sizeHint().

    \sa event(), QChildEvent
*/

void QObject::childEvent( QChildEvent * )
{
}

/*!
    This event handler can be reimplemented in a subclass to receive
    custom events. Custom events are user-defined events with a type
    value at least as large as the "User" item of the \l QEvent::Type
    enum, and is typically a QCustomEvent or QCustomEvent subclass.

    \sa event(), QCustomEvent
*/
void QObject::customEvent( QCustomEvent * )
{
}



/*!
    Filters events if this object has been installed as an event
    filter for the \a watched object.

    In your reimplementation of this function, if you want to filter
    the event \a e, out, i.e. stop it being handled further, return
    TRUE; otherwise return FALSE.

    Example:
    \code
    class MyMainWindow : public QMainWindow
    {
    public:
	MyMainWindow( QWidget *parent = 0, const char *name = 0 );

    protected:
	bool eventFilter( QObject *obj, QEvent *ev );

    private:
	QTextEdit *textEdit;
    };

    MyMainWindow::MyMainWindow( QWidget *parent, const char *name )
	: QMainWindow( parent, name )
    {
	textEdit = new QTextEdit( this );
	setCentralWidget( textEdit );
	textEdit->installEventFilter( this );
    }

    bool MyMainWindow::eventFilter( QObject *obj, QEvent *ev )
    {
	if ( obj == textEdit ) {
	    if ( e->type() == QEvent::KeyPress ) {
		qDebug( "Ate key press %d", k->key() );
		return TRUE;
	    } else {
		return FALSE;
	    }
	} else {
	    // pass the event on to the parent class
	    return QMainWindow::eventFilter( obj, ev );
	}
    }
    \endcode

    Notice in the example above that unhandled events are passed to
    the base class's eventFilter() function, since the base class
    might have reimplemented eventFilter() for its own internal
    purposes.

    \warning If you delete the receiver object in this function, be
    sure to return TRUE. Otherwise, Qt will forward the event to the
    deleted object and the program might crash.

    \sa installEventFilter()
*/

bool QObject::eventFilter( QObject * /* watched */, QEvent * /* e */ )
{
    return FALSE;
}


/*!
  \internal
  Activates all event filters for this object.
  This function is normally called from QObject::event() or QWidget::event().
*/

bool QObject::activate_filters( QEvent *e )
{
    if ( !eventFilters )			// no event filter
	return FALSE;
    QObjectListIterator it( *eventFilters );
    register QObject *obj = it.current();
    while ( obj ) {				// send to all filters
	++it;					//   until one returns TRUE
	if ( obj->eventFilter(this,e) ) {
	    return TRUE;
	}
	obj = it.current();
    }
    return FALSE;				// don't do anything with it
}


/*!
    \fn bool QObject::signalsBlocked() const

    Returns TRUE if signals are blocked; otherwise returns FALSE.

    Signals are not blocked by default.

    \sa blockSignals()
*/

/*!
    Blocks signals if \a block is TRUE, or unblocks signals if \a
    block is FALSE.

    Emitted signals disappear into hyperspace if signals are blocked.
    Note that the destroyed() signals will be emitted even if the signals
    for this object have been blocked.
*/

bool QObject::blockSignals( bool block )
{
    bool previous = blockSig;
    blockSig = block;
    return previous;
}


//
// The timer flag hasTimer is set when startTimer is called.
// It is not reset when killing the timer because more than
// one timer might be active.
//

/*!
    Starts a timer and returns a timer identifier, or returns zero if
    it could not start a timer.

    A timer event will occur every \a interval milliseconds until
    killTimer() or killTimers() is called. If \a interval is 0, then
    the timer event occurs once every time there are no more window
    system events to process.

    The virtual timerEvent() function is called with the QTimerEvent
    event parameter class when a timer event occurs. Reimplement this
    function to get timer events.

    If multiple timers are running, the QTimerEvent::timerId() can be
    used to find out which timer was activated.

    Example:
    \code
    class MyObject : public QObject
    {
	Q_OBJECT
    public:
	MyObject( QObject *parent = 0, const char *name = 0 );

    protected:
	void timerEvent( QTimerEvent * );
    };

    MyObject::MyObject( QObject *parent, const char *name )
	: QObject( parent, name )
    {
	startTimer( 50 );    // 50-millisecond timer
	startTimer( 1000 );  // 1-second timer
	startTimer( 60000 ); // 1-minute timer
    }

    void MyObject::timerEvent( QTimerEvent *e )
    {
	qDebug( "timer event, id %d", e->timerId() );
    }
    \endcode

    There is practically no upper limit for the interval value (more
    than one year is possible). Note that QTimer's accuracy depends on
    the underlying operating system and hardware. Most platforms
    support an accuracy of 20ms; some provide more. If Qt is unable to
    deliver the requested number of timer clicks, it will silently
    discard some.

    The QTimer class provides a high-level programming interface with
    one-shot timers and timer signals instead of events.

    \sa timerEvent(), killTimer(), killTimers()
*/

int QObject::startTimer( int interval )
{
    pendTimer = TRUE;				// set timer flag
    return qStartTimer( interval, (QObject *)this );
}

/*!
    Kills the timer with timer identifier, \a id.

    The timer identifier is returned by startTimer() when a timer
    event is started.

    \sa timerEvent(), startTimer(), killTimers()
*/

void QObject::killTimer( int id )
{
    qKillTimer( id );
}

/*!
    Kills all timers that this object has started.

    \warning Using this function can cause hard-to-find bugs: it kills
    timers started by sub- and superclasses as well as those started
    by you, which is often not what you want. We recommend using a
    QTimer or perhaps killTimer().

    \sa timerEvent(), startTimer(), killTimer()
*/

void QObject::killTimers()
{
    qKillTimer( this );
}

static void objSearch( QObjectList *result,
		       QObjectList *list,
		       const char  *inheritsClass,
		       bool onlyWidgets,
		       const char  *objName,
		       QRegExp	   *rx,
		       bool	    recurse )
{
    if ( !list || list->isEmpty() )		// nothing to search
	return;
    QObject *obj = list->first();
    while ( obj ) {
	bool ok = TRUE;
	if ( onlyWidgets )
	    ok = obj->isWidgetType();
	else if ( inheritsClass && !obj->inherits(inheritsClass) )
	    ok = FALSE;
	if ( ok ) {
	    if ( objName )
		ok = ( qstrcmp(objName,obj->name()) == 0 );
#ifndef QT_NO_REGEXP
	    else if ( rx )
		ok = ( rx->search(QString::fromLatin1(obj->name())) != -1 );
#endif
	}
	if ( ok )				// match!
	    result->append( obj );
	if ( recurse && obj->children() )
	    objSearch( result, (QObjectList *)obj->children(), inheritsClass,
		       onlyWidgets, objName, rx, recurse );
	obj = list->next();
    }
}

/*!
    \fn QObject *QObject::parent() const

    Returns a pointer to the parent object.

    \sa children()
*/

/*!
    \fn const QObjectList *QObject::children() const

    Returns a list of child objects, or 0 if this object has no
    children.

    The QObjectList class is defined in the \c qobjectlist.h header
    file.

    The first child added is the \link QPtrList::first() first\endlink
    object in the list and the last child added is the \link
    QPtrList::last() last\endlink object in the list, i.e. new
    children are appended at the end.

    Note that the list order changes when QWidget children are \link
    QWidget::raise() raised\endlink or \link QWidget::lower()
    lowered.\endlink A widget that is raised becomes the last object
    in the list, and a widget that is lowered becomes the first object
    in the list.

    \sa child(), queryList(), parent(), insertChild(), removeChild()
*/


/*!
    Returns a pointer to the list of all object trees (their root
    objects), or 0 if there are no objects.

    The QObjectList class is defined in the \c qobjectlist.h header
    file.

    The most recent root object created is the \link QPtrList::first()
    first\endlink object in the list and the first root object added
    is the \link QPtrList::last() last\endlink object in the list.

    \sa children(), parent(), insertChild(), removeChild()
*/
const QObjectList *QObject::objectTrees()
{
    return object_trees;
}


/*!
    Searches the children and optionally grandchildren of this object,
    and returns a list of those objects that are named or that match
    \a objName and inherit \a inheritsClass. If \a inheritsClass is 0
    (the default), all classes match. If \a objName is 0 (the
    default), all object names match.

    If \a regexpMatch is TRUE (the default), \a objName is a regular
    expression that the objects's names must match. The syntax is that
    of a QRegExp. If \a regexpMatch is FALSE, \a objName is a string
    and object names must match it exactly.

    Note that \a inheritsClass uses single inheritance from QObject,
    the way inherits() does. According to inherits(), QMenuBar
    inherits QWidget but not QMenuData. This does not quite match
    reality, but is the best that can be done on the wide variety of
    compilers Qt supports.

    Finally, if \a recursiveSearch is TRUE (the default), queryList()
    searches \e{n}th-generation as well as first-generation children.

    If all this seems a bit complex for your needs, the simpler
    child() function may be what you want.

    This somewhat contrived example disables all the buttons in this
    window:
    \code
    QObjectList *l = topLevelWidget()->queryList( "QButton" );
    QObjectListIterator it( *l ); // iterate over the buttons
    QObject *obj;

    while ( (obj = it.current()) != 0 ) {
	// for each found object...
	++it;
	((QButton*)obj)->setEnabled( FALSE );
    }
    delete l; // delete the list, not the objects
    \endcode

    The QObjectList class is defined in the \c qobjectlist.h header
    file.

    \warning Delete the list as soon you have finished using it. The
    list contains pointers that may become invalid at almost any time
    without notice (as soon as the user closes a window you may have
    dangling pointers, for example).

    \sa child() children(), parent(), inherits(), name(), QRegExp
*/

QObjectList *QObject::queryList( const char *inheritsClass,
				 const char *objName,
				 bool regexpMatch,
				 bool recursiveSearch ) const
{
    QObjectList *list = new QObjectList;
    Q_CHECK_PTR( list );
    bool onlyWidgets = ( inheritsClass && qstrcmp(inheritsClass, "QWidget") == 0 );
#ifndef QT_NO_REGEXP
    if ( regexpMatch && objName ) {		// regexp matching
	QRegExp rx(QString::fromLatin1(objName));
	objSearch( list, (QObjectList *)children(), inheritsClass, onlyWidgets,
		   0, &rx, recursiveSearch );
    } else
#endif
	{
	objSearch( list, (QObjectList *)children(), inheritsClass, onlyWidgets,
		   objName, 0, recursiveSearch );
    }
    return list;
}


/*!
    Inserts an object \a obj into the list of child objects.

    \warning This function cannot be used to make one widget the child
    widget of another widget. Child widgets can only be created by
    setting the parent widget in the constructor or by calling
    QWidget::reparent().

    \sa removeChild(), QWidget::reparent()
*/

void QObject::insertChild( QObject *obj )
{
    if ( obj->isTree ) {
	remove_tree( obj );
	obj->isTree = FALSE;
    }
    if ( obj->parentObj && obj->parentObj != this ) {
#if defined(QT_CHECK_STATE)
	if ( obj->parentObj != this && obj->isWidgetType() )
	    qWarning( "QObject::insertChild: Cannot reparent a widget, "
		     "use QWidget::reparent() instead" );
#endif
	obj->parentObj->removeChild( obj );
    }

    if ( !childObjects ) {
	childObjects = new QObjectList;
	Q_CHECK_PTR( childObjects );
    } else if ( obj->parentObj == this ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QObject::insertChild: Object %s::%s already in list",
		 obj->className(), obj->name( "unnamed" ) );
#endif
	return;
    }
    obj->parentObj = this;
    childObjects->append( obj );

    QChildEvent *e = new QChildEvent( QEvent::ChildInserted, obj );
    QApplication::postEvent( this, e );
}

/*!
    Removes the child object \a obj from the list of children.

    \warning This function will not remove a child widget from the
    screen. It will only remove it from the parent widget's list of
    children.

    \sa insertChild(), QWidget::reparent()
*/

void QObject::removeChild( QObject *obj )
{
    if ( childObjects && childObjects->removeRef(obj) ) {
	obj->parentObj = 0;
	if ( !obj->wasDeleted ) {
	    insert_tree( obj );			// it's a root object now
	    obj->isTree = TRUE;
	}
	if ( childObjects->isEmpty() ) {
	    delete childObjects;		// last child removed
	    childObjects = 0;			// reset children list
	}

	// remove events must be sent, not posted!!!
	QChildEvent ce( QEvent::ChildRemoved, obj );
	QApplication::sendEvent( this, &ce );
    }
}


/*!
    \fn void QObject::installEventFilter( const QObject *filterObj )

    Installs an event filter \a filterObj on this object. For example:
    \code
    monitoredObj->installEventFilter( filterObj );
    \endcode

    An event filter is an object that receives all events that are
    sent to this object. The filter can either stop the event or
    forward it to this object. The event filter \a filterObj receives
    events via its eventFilter() function. The eventFilter() function
    must return TRUE if the event should be filtered, (i.e. stopped);
    otherwise it must return FALSE.

    If multiple event filters are installed on a single object, the
    filter that was installed last is activated first.

    Here's a \c KeyPressEater class that eats the key presses of its
    monitored objects:
    \code
    class KeyPressEater : public QObject
    {
	...
    protected:
	bool eventFilter( QObject *o, QEvent *e );
    };

    bool KeyPressEater::eventFilter( QObject *o, QEvent *e )
    {
	if ( e->type() == QEvent::KeyPress ) {
	    // special processing for key press
	    QKeyEvent *k = (QKeyEvent *)e;
	    qDebug( "Ate key press %d", k->key() );
	    return TRUE; // eat event
	} else {
	    // standard event processing
	    return FALSE;
	}
    }
    \endcode

    And here's how to install it on two widgets:
    \code
	KeyPressEater *keyPressEater = new KeyPressEater( this );
	QPushButton *pushButton = new QPushButton( this );
	QListView *listView = new QListView( this );

	pushButton->installEventFilter( keyPressEater );
	listView->installEventFilter( keyPressEater );
    \endcode

    The QAccel class, for example, uses this technique to intercept
    accelerator key presses.

    \warning If you delete the receiver object in your eventFilter()
    function, be sure to return TRUE. If you return FALSE, Qt sends
    the event to the deleted object and the program will crash.

    \sa removeEventFilter(), eventFilter(), event()
*/

void QObject::installEventFilter( const QObject *obj )
{
    if ( !obj )
	return;
    if ( eventFilters ) {
	int c = eventFilters->findRef( obj );
	if ( c >= 0 )
	    eventFilters->take( c );
	disconnect( obj, SIGNAL(destroyed(QObject*)),
		    this, SLOT(cleanupEventFilter(QObject*)) );
    } else {
	eventFilters = new QObjectList;
	Q_CHECK_PTR( eventFilters );
    }
    eventFilters->insert( 0, obj );
    connect( obj, SIGNAL(destroyed(QObject*)), this, SLOT(cleanupEventFilter(QObject*)) );
}

/*!
    Removes an event filter object \a obj from this object. The
    request is ignored if such an event filter has not been installed.

    All event filters for this object are automatically removed when
    this object is destroyed.

    It is always safe to remove an event filter, even during event
    filter activation (i.e. from the eventFilter() function).

    \sa installEventFilter(), eventFilter(), event()
*/

void QObject::removeEventFilter( const QObject *obj )
{
    if ( eventFilters && eventFilters->removeRef(obj) ) {
	if ( eventFilters->isEmpty() ) {	// last event filter removed
	    delete eventFilters;
	    eventFilters = 0;			// reset event filter list
	}
	disconnect( obj,  SIGNAL(destroyed(QObject*)),
		    this, SLOT(cleanupEventFilter(QObject*)) );
    }
}


/*!
    \fn QObject::destroyed()

    This signal is emitted when the object is being destroyed.

    Note that the signal is emitted by the QObject destructor, so
    the object's virtual table is already degenerated at this point,
    and it is not safe to call any functions on the object emitting
    the signal. This signal can not be blocked.

    All the objects's children are destroyed immediately after this
    signal is emitted.
*/

/*!
    \overload QObject::destroyed( QObject* obj)

    This signal is emitted immediately before the object \a obj is
    destroyed, and can not be blocked.

    All the objects's children are destroyed immediately after this
    signal is emitted.
*/

/*!
    Performs a deferred deletion of this object.

    Instead of an immediate deletion this function schedules a
    deferred delete event for processing when Qt returns to the main
    event loop.
*/
void QObject::deleteLater()
{
    QApplication::postEvent( this, new QEvent( QEvent::DeferredDelete) );
}

/*!
    This slot is connected to the destroyed() signal of other objects
    that have installed event filters on this object. When the other
    object, \a obj, is destroyed, we want to remove its event filter.
*/

void QObject::cleanupEventFilter(QObject* obj)
{
    removeEventFilter( obj );
}

/*!
    \fn QString Object::tr( const char *sourceText, const char * comment )
    \reentrant

    Returns a translated version of \a sourceText, or \a sourceText
    itself if there is no appropriate translated version. The
    translation context is Object with \a comment (0 by default).
    All Object subclasses using the Q_OBJECT macro automatically have
    a reimplementation of this function with the subclass name as
    context.

    \warning This method is reentrant only if all translators are
    installed \e before calling this method. Installing or removing
    translators while performing translations is not supported. Doing
    so will probably result in crashes or other undesirable behavior.

    \sa trUtf8() QApplication::translate()
	\link i18n.html Internationalization with Qt\endlink
*/

/*!
    \fn QString Object::trUtf8( const char *sourceText,
                                 const char *comment )
    \reentrant

    Returns a translated version of \a sourceText, or
    QString::fromUtf8(\a sourceText) if there is no appropriate
    version. It is otherwise identical to tr(\a sourceText, \a
    comment).

    \warning This method is reentrant only if all translators are
    installed \e before calling this method. Installing or removing
    translators while performing translations is not supported. Doing
    so will probably result in crashes or other undesirable behavior.

    \sa tr() QApplication::translate()
*/


/*****************************************************************************
  Signals and slots
 *****************************************************************************/

/*
    Adds one reference to sender in this object's senders list. If
    sender has not yet an entry, creates one. Called from connect().
*/
void QObjectPrivate::refSender(QObject *sender)
{
    int i = 0;
    if (!senders) {
	senders = (Senders*)qMalloc(sizeof(Senders));
	senders->senders = senders->stack;
	senders->ref = 1;
	senders->count = 1;
	senders->current = 0;
    } else {
	while (i < senders->count) {
	    if (senders->senders[i].sender == sender) {
		++senders->senders[i].ref;
		return;
	    }
	    ++i;
	}
	i = 0;
	while (i < senders->count && senders->senders[i].sender)
	    ++i;
	if (i == senders->count) {
	    if (senders->senders != senders->stack) {
		senders->senders =
		    (Senders::Sender*)
		    qRealloc(senders->senders, (i+1)*sizeof(Senders::Sender));
	    } else if (senders->ref > 1) { // we cannot realloc
		senders->senders =
		    (Senders::Sender*)
		    qMalloc((i+1)*sizeof(Senders::Sender));
		::memcpy(senders->senders, senders->stack,
			 i*sizeof(Senders::Sender));
	    } else {
		senders = (Senders*)qRealloc(senders, sizeof(Senders)
					      + i * sizeof(Senders::Sender));
		senders->senders = senders->stack;
	    }
	    ++senders->count;
	}
    }
    senders->senders[i].sender = sender;
    senders->senders[i].ref = 1;
}

/*
    Removes one reference to sender from this object's senders
    list. If this was the last reference, removes the entire
    entry. Called from disconnect().
*/
void QObjectPrivate::derefSender(QObject *sender)
{
    for (int i = 0; i < senders->count; ++i)
	if (senders->senders[i].sender == sender) {
	    if (!--senders->senders[i].ref) {
		senders->senders[i].sender = 0;
		if (senders->current == sender)
		    senders->current = 0;
	    }
	    break;
	}
}

/*
    Removes sender from this object's list of senders. Called from
    sender's destructor.
*/
void QObjectPrivate::removeSender(QObject *sender)
{
    if (!senders)
	return;
    for (int i = 0; i < senders->count; ++i)
	if (senders->senders[i].sender == sender)
	    senders->senders[i].sender = 0;
    if (senders->current == sender)
	senders->current = 0;
}

/*
    Dereferences senders list, and frees it no longer referenced.
 */
void QObjectPrivate::derefSenders(Senders *senders)
{
    if (senders && !--senders->ref) {
	if (senders->senders != senders->stack)
	    qFree(senders->senders);
	qFree(senders);
    }
}

/*
    Sets current sender, references senders list, and returns the
    previous current sender.
*/
QObject *QObjectPrivate::setCurrentSender(Senders *senders, QObject *sender)
{
    if (!senders)
	return 0;
    QObject *previous = senders->current;
    senders->current = sender;
    ++senders->ref;
    return previous;
}

/*
    Resets current sender, dererences senders list, and frees it no
    longer referenced.
 */
void QObjectPrivate::resetCurrentSender(Senders *senders, QObject *sender)
{
    if (!senders)
	return;
    senders->current = 0;
    if (sender) {
        // only if the new current sender is still in the list
	for (int i = 0; i < senders->count; ++i) {
	    if (senders->senders[i].sender == sender) {
		senders->current = sender;
		break;
	    }
	}
    }
    derefSenders(senders);
}


#if defined(QT_CHECK_RANGE)

static bool check_signal_macro(const QObject *sender, const char *signal,
				const char *func, const char *op)
{
    int sigcode = (int)(*signal) - '0';
    if (sigcode != QSIGNAL_CODE) {
	if (sigcode == QSLOT_CODE)
	    qWarning("Object::%s: Attempt to %s non-signal %s::%s",
		     func, op, sender->className(), signal+1);
	else
	    qWarning("Object::%s: Use the SIGNAL macro to %s %s::%s",
		     func, op, sender->className(), signal);
	return false;
    }
    return true;
}

static bool check_member_code(int code, const QObject *object,
			       const char *member, const char *func)
{
    if (code != QSLOT_CODE && code != QSIGNAL_CODE) {
	qWarning("Object::%s: Use the SLOT or SIGNAL macro to "
		 "%s %s::%s", func, func, object->className(), member);
	return false;
    }
    return true;
}

static void err_member_notfound(int code, const QObject *object,
				 const char *member, const char *func)
{
    const char *type = 0;
    switch (code) {
	case QSLOT_CODE:   type = "slot";   break;
	case QSIGNAL_CODE: type = "signal"; break;
    }
    if (strchr(member,')') == 0)		// common typing mistake
	qWarning("Object::%s: Parentheses expected, %s %s::%s",
		 func, type, object->className(), member);
    else
	qWarning("Object::%s: No such %s %s::%s",
		 func, type, object->className(), member);
}


static void err_info_about_objects(const char * func,
				    const QObject * sender,
				    const QObject * receiver)
{
    const char * a = sender->name(), * b = receiver->name();
    if (a)
	qWarning("Object::%s:  (sender name:   '%s')", func, a);
    if (b)
	qWarning("Object::%s:  (receiver name: '%s')", func, b);
}

#endif // QT_CHECK_RANGE


void QObjectPrivate::addConnection(int signal, QObject *receiver, int member)
{
    int i = 0;
    if ( !connections) {
	connections = (Connections*)qMalloc(sizeof(Connections));
	connections->count = 1;
    } else {
	while (i < connections->count && connections->connections[i].receiver)
	    ++i;
	if (i == connections->count) {
	    ++connections->count;
	    connections = (Connections*)
			  qRealloc(connections, sizeof(Connections)
				    + i * sizeof(Connections::Connection));
	}
    }
    Connections::Connection &c = connections->connections[i];
    c.signal = signal;
    c.receiver = receiver;
    c.member = member;
}

QObjectPrivate::Connections::Connection *QObjectPrivate::findConnection(int signal, int &i)
{
    if (connections) {
	while (i < connections->count) {
	    if (connections->connections[i].receiver
		&& connections->connections[i].signal == signal)
		return &connections->connections[i++];
	    ++i;
	}
    }
    return 0;
}

void QObjectPrivate::removeReceiver(QObject *receiver)
{
    if (connections) {
	for (int i = 0; i < connections->count; ++i)
	    if (connections->connections[i].receiver == receiver)
		connections->connections[i].receiver = 0;
    }
}


/*!
    Returns a pointer to the object that sent the signal, if called in
    a slot activated by a signal; otherwise it returns 0. The pointer
    is valid only during the execution of the slot that calls this
    function.

    The pointer returned by this function becomes invalid if the
    sender is destroyed, or if the slot is disconnected from the
    sender's signal.

    \warning This function violates the object-oriented principle of
     modularity. However, getting access to the sender might be useful
     when many signals are connected to a single slot. The sender is
     undefined if the slot is called as a normal C++ function.
*/

const QObject *QObject::sender()
{
    if (d->senders)
	return d->senders->current;
    return 0;
}

/*!
    Returns the number of receivers connect to the \a signal.

    \warning This function violates the object-oriented principle of
    modularity. However, it might be useful when you need to perform
    expensive initialization only if something is connected to a
    signal.
*/

int QObject::receivers(const char *signal) const
{
    int receivers = 0;
    if ( d->connections && signal ) {
	QByteArray signal_name = QMetaObject::normalizeSignature(signal);
	signal = signal_name;
#if defined(QT_CHECK_RANGE)
	if (!check_signal_macro(this, signal, "receivers", "bind"))
	    return 0;
#endif
	signal++; // skip code
	const QMetaObject *smeta = this->metaObject();
	int signal_index = smeta->findSignal(signal);
	if (signal_index < 0) {
#if defined(QT_CHECK_RANGE)
	    err_member_notfound(QSIGNAL_CODE, this, signal, "receivers");
#endif
	    return false;
	}
	int i = 0;
	QObjectPrivate::Connections::Connection *c;
	while((c = d->findConnection(signal_index, i)))
	    ++receivers;
    }
    return receivers;
}


/*!
    Connects \a signal from the \a sender object to \a member in object
    \a receiver, and returns true if the connection succeeds; otherwise
    returns false.

    You must use the SIGNAL() and SLOT() macros when specifying the \a signal
    and the \a member, for example:
    \code
    QLabel     *label  = new QLabel;
    QScrollBar *scroll = new QScrollBar;
    QObject::connect(scroll, SIGNAL(valueChanged(int)),
                      label,  SLOT(setNum(int)));
    \endcode

    This example ensures that the label always displays the current
    scroll bar value. Note that the signal and slots parameters must not
    contain any variable names, only the type. E.g. the following would
    not work and return false:
    QObject::connect(scroll, SIGNAL(valueChanged(int v)),
                      label,  SLOT(setNum(int v)));

    A signal can also be connected to another signal:

    \code
    class MyWidget : public QWidget
    {
	Q_OBJECT
    public:
	MyWidget();

    signals:
	void myUsefulSignal();

    private:
	QPushButton *aButton;
    };

    MyWidget::MyWidget()
    {
	aButton = new QPushButton(this);
	connect(aButton, SIGNAL(clicked()), SIGNAL(myUsefulSignal()));
    }
    \endcode

    In this example, the MyWidget constructor relays a signal from a
    private member variable, and makes it available under a name that
    relates to MyWidget.

    A signal can be connected to many slots and signals. Many signals
    can be connected to one slot.

    If a signal is connected to several slots, the slots are activated
    in an arbitrary order when the signal is emitted.

    The function returns true if it successfully connects the signal
    to the slot. It will return false if it cannot create the
    connection, for example, if QObject is unable to verify the
    existence of either \a signal or \a member, or if their signatures
    aren't compatible.

    A signal is emitted for \e{every} connection you make, so if you
    duplicate a connection, two signals will be emitted. You can
    always break a connection using \c{disconnect()}.

    \sa disconnect()
*/

bool QObject::connect(const QObject *sender, const char *signal,
		     const QObject *receiver, const char *member)
{
#if defined(QT_CHECK_NULL)
    if (sender == 0 || receiver == 0 || signal == 0 || member == 0) {
	qWarning("Object::connect: Cannot connect %s::%s to %s::%s",
		 sender ? sender->className() : "(null)",
		 signal ? signal+1 : "(null)",
		 receiver ? receiver->className() : "(null)",
		 member ? member+1 : "(null)");
	return false;
    }
#endif
    QByteArray signal_name = QMetaObject::normalizeSignature(signal);
    signal = signal_name;

#if defined(QT_CHECK_RANGE)
    if (!check_signal_macro(sender, signal, "connect", "bind"))
	return false;
#endif
    signal++; // skip code

    const QMetaObject *smeta = sender->metaObject();
    int signal_index = smeta->findSignal(signal);
    if (signal_index < 0) {
#if defined(QT_CHECK_RANGE)
	err_member_notfound(QSIGNAL_CODE, sender, signal, "connect");
	err_info_about_objects("connect", sender, receiver);
#endif
	return false;
    }

    QByteArray member_name = QMetaObject::normalizeSignature(member);
    member = member_name;
    int membcode = member[0] - '0';

#if defined(QT_CHECK_RANGE)
    if (!check_member_code(membcode, receiver, member, "connect"))
	return false;
#endif
    member++; // skip code

    const QMetaObject *rmeta = receiver->metaObject();
    int member_index = -1;
    switch (membcode) {
	case QSLOT_CODE:
	    member_index = rmeta->findSlot(member);
	    break;
	case QSIGNAL_CODE:
	    member_index = rmeta->findSignal(member);
	    break;
    }
    if (member_index < 0) {
#if defined(QT_CHECK_RANGE)
	err_member_notfound(membcode, receiver, member, "connect");
	err_info_about_objects("connect", sender, receiver);
#endif
	return false;
    }
#if defined(QT_CHECK_RANGE)
    if (!QMetaObject::checkConnectArgs(signal, member)) {
	qWarning("Object::connect: Incompatible sender/receiver arguments"
		 "\n\t%s::%s --> %s::%s",
		 sender->className(), signal,
		 receiver->className(), member);
	return false;
    }
#endif
    QMetaObject::connect(sender, signal_index, receiver, membcode, member_index);
    const_cast<QObject*>(sender)->connectNotify(signal_name);
    return true;
}


/*!
    \overload bool QObject::connect(const QObject *sender, const char *signal, const char *member) const

    Connects \a signal from the \a sender object to this object's \a
    member.

    Equivalent to: \c{Object::connect(sender, signal, this, member)}.

    \sa disconnect()
*/

/*!
    Disconnects \a signal in object \a sender from \a member in object
    \a receiver.

    A signal-slot connection is removed when either of the objects
    involved are destroyed.

    disconnect() is typically used in three ways, as the following
    examples demonstrate.
    \list 1
    \i Disconnect everything connected to an object's signals:
       \code
       disconnect(myObject, 0, 0, 0);
       \endcode
       equivalent to the non-static overloaded function
       \code
       myObject->disconnect();
       \endcode
    \i Disconnect everything connected to a specific signal:
       \code
       disconnect(myObject, SIGNAL(mySignal()), 0, 0);
       \endcode
       equivalent to the non-static overloaded function
       \code
       myObject->disconnect(SIGNAL(mySignal()));
       \endcode
    \i Disconnect a specific receiver:
       \code
       disconnect(myObject, 0, myReceiver, 0);
       \endcode
       equivalent to the non-static overloaded function
       \code
       myObject->disconnect(myReceiver);
       \endcode
    \endlist

    0 may be used as a wildcard, meaning "any signal", "any receiving
    object", or "any slot in the receiving object", respectively.

    The \a sender may never be 0. (You cannot disconnect signals from
    more than one object in a single call.)

    If \a signal is 0, it disconnects \a receiver and \a member from
    any signal. If not, only the specified signal is disconnected.

    If \a receiver is 0, it disconnects anything connected to \a
    signal. If not, slots in objects other than \a receiver are not
    disconnected.

    If \a member is 0, it disconnects anything that is connected to \a
    receiver. If not, only slots named \a member will be disconnected,
    and all other slots are left alone. The \a member must be 0 if \a
    receiver is left out, so you cannot disconnect a
    specifically-named slot on all objects.

    \sa connect()
*/
bool QObject::disconnect(const QObject *sender, const char *signal,
			 const QObject *receiver, const char *member)
{
#if defined(QT_CHECK_NULL)
    if (sender == 0 || (receiver == 0 && member != 0)) {
	qWarning("Object::disconnect: Unexpected null parameter");
	return false;
    }
#endif
    if (!sender->d->connections)
	return false;

    QByteArray signal_name;
    bool signal_found = false;
    if (signal) {
	signal_name = QMetaObject::normalizeSignature(signal);
	signal = signal_name;
#if defined(QT_CHECK_RANGE)
	if (!check_signal_macro(sender, signal, "disconnect", "unbind"))
	    return false;
#endif
	signal++; // skip code
    }

    QByteArray member_name;
    int membcode = -1;
    bool member_found = false;
    if (member) {
	member_name = QMetaObject::normalizeSignature(member);
	member = member_name;
	membcode = member[0] - '0';
#if defined(QT_CHECK_RANGE)
	if (!check_member_code(membcode, receiver, member, "disconnect"))
	    return false;
#endif
	member++; // skip code
    }

    /* We now iterate through all the sender's and receiver's meta
     * objects in order to also disconnect possibly shadowed signals
     * and slots with the same signature.
    */
    bool res = false;
    const QMetaObject *smeta = sender->metaObject();
    do {
	int signal_index = -1;
	if (signal) {
	    signal_index = smeta->findSignal(signal);
	    if (signal_index < smeta->signalOffset())
		continue;
	    signal_found = true;
	}

	if (!member) {
	    res |= QMetaObject::disconnect(sender,
					  signal_index,
					  receiver,
					  -1, -1);
	} else {
	    const QMetaObject *rmeta = receiver->metaObject();
	    do {
		int member_index = -1;
		switch (membcode) {
		case QSLOT_CODE:
		    member_index = rmeta->findSlot(member);
		    if (member_index >= 0)
			while (member_index < rmeta->slotOffset())
			    rmeta = rmeta->superClass();
		    break;
		case QSIGNAL_CODE:
		    member_index = rmeta->findSignal(member);
		    if (member_index >= 0)
			while (member_index < rmeta->signalOffset())
			    rmeta = rmeta->superClass();
		    break;
		}
		if (member_index < 0)
		    break;
		res |= QMetaObject::disconnect(sender,
					      signal_index,
					      receiver,
					      membcode, member_index);
		member_found = true;
	    } while ((rmeta = rmeta->superClass()));
	}
    } while (signal && (smeta = smeta->superClass()));

#if defined(QT_CHECK_RANGE)
    if (signal && !signal_found ) {
	err_member_notfound(QSIGNAL_CODE, sender, signal, "disconnect");
	err_info_about_objects("disconnect", sender, receiver);
    } else if (member && !member_found) {
	err_member_notfound(membcode, receiver, member, "disconnect");
	err_info_about_objects("disconnect", sender, receiver);
    }
#endif
    if (res)
	const_cast<QObject*>(sender)->disconnectNotify(signal);
    return res;
}


/*!
    \overload bool QObject::disconnect(const char *signal, const QObject *receiver, const char *member)

    Disconnects \a signal from \a member of \a receiver.

    A signal-slot connection is removed when either of the objects
    involved are destroyed.
*/

/*!
    \overload bool QObject::disconnect(const QObject *receiver, const char *member)

    Disconnects all signals in this object from \a receiver's \a
    member.

    A signal-slot connection is removed when either of the objects
    involved are destroyed.
*/


/*!
    \fn void QObject::connectNotify(const char *signal)

    This virtual function is called when something has been connected
    to \a signal in this object.

    \warning This function violates the object-oriented principle of
    modularity. However, it might be useful when you need to perform
    expensive initialization only if something is connected to a
    signal.

    \sa connect(), disconnectNotify()
*/

void QObject::connectNotify(const char *)
{
}

/*!
    \fn void QObject::disconnectNotify(const char *signal)

    This virtual function is called when something has been
    disconnected from \a signal in this object.

    \warning This function violates the object-oriented principle of
    modularity. However, it might be useful for optimizing access to
    expensive resources.

    \sa disconnect(), connectNotify()
*/

void QObject::disconnectNotify(const char *)
{
}

/*!\internal
*/
bool QMetaObject::connect(const QObject *sender, int signal_index,
			  const QObject *receiver,
			  int membcode, int member_index)
{
    if (membcode != QSLOT_CODE && membcode != QSIGNAL_CODE)
	return false;

    QObject *s = const_cast<QObject*>(sender);
    QObject *r = const_cast<QObject*>(receiver);

    s->d->addConnection(signal_index, r, (member_index<<1)+membcode-1);
    r->d->refSender(s);
    return true;
}


/*!\internal
 */
bool QMetaObject::disconnect(const QObject *sender, int signal_index,
			    const QObject *receiver,
			    int membcode, int member_index)
{
    if ( !sender || !sender->d->connections
	 || ( member_index >= 0
	      && (membcode != QSLOT_CODE && membcode != QSIGNAL_CODE) ) )
	return false;
    QObject *s = const_cast<QObject*>(sender);
    QObject *r = const_cast<QObject*>(receiver);

    bool success = false;
    for (int i = 0; i < s->d->connections->count; ++i) {
	QObjectPrivate::Connections::Connection &c = s->d->connections->connections[i];
	if (c.receiver && c.signal != GUARDED_SIGNAL && (signal_index < 0 || signal_index == c.signal)
	    && (r == 0 || (c.receiver == r
			   && (member_index < 0
			       || (member_index<<1)+membcode-1
			       == c.member)))) {
	    c.receiver->d->derefSender(s);
	    c.receiver = 0;
	    success = true;
	}
    }
    return success;
}

/*!\internal
 */
void QMetaObject::activate(QObject *obj, int signal_index, void **argv)
{
    if (obj->blockSig)
	return;
    int i = 0;
    QObjectPrivate::Connections::Connection *c, *nc;
    void *static_argv[] = { 0 };
    if (!argv)
	argv = static_argv;
    if ((c = obj->d->findConnection(signal_index, i))) do {
	nc = obj->d->findConnection(signal_index, i);
	QObjectPrivate::Senders *senders = c->receiver->d->senders;
	QObject *sender = QObjectPrivate::setCurrentSender(senders, obj);
	c->receiver->qt_metacall(
	    (Call)((c->member & 1) + 1),
	    c->member >> 1,
	    argv);
	QObjectPrivate::resetCurrentSender(senders, sender);
    } while ((c = nc));
}

/*!\internal
 */
void QMetaObject::activate(QObject *obj, const QMetaObject *m, int local_signal_index, void **argv)
{
    activate(obj, m->signalOffset() + local_signal_index, argv);
}

/*****************************************************************************
  Properties
 *****************************************************************************/

#ifndef QT_NO_PROPERTIES

/*!
    Sets the value of the object's \a name property to \a value.

    Returns true if the operation was successful; otherwise returns
    false.

    Information about all available properties is provided through the
    metaObject().

    \sa property(), metaObject()
*/
bool QObject::setProperty(const char *name, const QVariant &value)
{
    const QMetaObject* meta = metaObject();
    if (!value.isValid() || !meta)
	return false;

    int id = meta->findProperty(name);
    QMetaProperty p = meta->property(id);
#if defined(QT_CHECK_RANGE)
    if (!p.isWritable())
	qWarning( "%s::setProperty( \"%s\", value ) failed: property invalid,"
		  " read-only or does not exist", className(), name );
#endif
    return p.write(this, value);
}

/*!
    Returns the value of the object's \a name property.

    If no such property exists, the returned variant is invalid.

    Information about all available properties is provided through the
    metaObject().

    \sa setProperty(), QVariant::isValid(), metaObject()
*/
QVariant QObject::property(const char *name) const
{
    const QMetaObject* meta = metaObject();
    if (!name || !meta)
	return QVariant();

    int id = meta->findProperty(name);
    QMetaProperty p = meta->property(id);
#if defined(QT_CHECK_RANGE)
    if (!p.isReadable())
	qWarning( "%s::property( \"%s\" ) failed:"
		  " property invalid or does not exist",
		  className(), name );
#endif
    return p.read(this);
}

#endif // QT_NO_PROPERTIES


/*****************************************************************************
  QObject debugging output routines.
 *****************************************************************************/

static void dumpRecursive( int level, QObject *object )
{
#if defined(QT_DEBUG)
    if ( object ) {
	QString buf;
	buf.fill( '\t', level/2 );
	if ( level % 2 )
	    buf += "    ";
	const char *name = object->name();
	QString flags="";
	if ( qApp->focusWidget() == object )
	    flags += 'F';
	if ( object->isWidgetType() ) {
	    QWidget * w = (QWidget *)object;
	    if ( w->isVisible() ) {
		QString t( "<%1,%2,%3,%4>" );
		flags += t.arg(w->x()).arg(w->y()).arg(w->width()).arg(w->height());
	    } else {
		flags += 'I';
	    }
	}
	qDebug( "%s%s::%s %s", (const char*)buf, object->className(), name,
	    flags.latin1() );
	if ( object->children() ) {
	    QObjectListIterator it(*object->children());
	    QObject * c;
	    while ( (c=it.current()) != 0 ) {
		++it;
		dumpRecursive( level+1, c );
	    }
	}
    }
#else
    Q_UNUSED( level )
    Q_UNUSED( object )
#endif
}

/*!
    Dumps a tree of children to the debug output.

    This function is useful for debugging, but does nothing if the
    library has been compiled in release mode (i.e. without debugging
    information).
*/

void QObject::dumpObjectTree()
{
    dumpRecursive( 0, this );
}

/*!
    Dumps information about signal connections, etc. for this object
    to the debug output.

    This function is useful for debugging, but does nothing if the
    library has been compiled in release mode (i.e. without debugging
    information).
*/

void QObject::dumpObjectInfo()
{
#if defined(QT_DEBUG)
    qDebug( "OBJECT %s::%s", className(), name( "unnamed" ) );
    //#### signals and slots info missing
#endif
}

#ifndef QT_NO_USERDATA
/*!\internal
 */
uint QObject::registerUserData()
{
    static int user_data_registration = 0;
    return user_data_registration++;
}

/*!\internal
 */
QObjectUserData::~QObjectUserData()
{
}

/*!\internal
 */
void QObject::setUserData( uint id, QObjectUserData* data)
{
    if (id >= d->userData.size())
	d->userData.resize(id+1);
    d->userData.insert(id, data);
}

/*!\internal
 */
QObjectUserData* QObject::userData( uint id ) const
{
    if (id < d->userData.size())
	return d->userData.at(id);
    return 0;
}

#endif // QT_NO_USERDATA

