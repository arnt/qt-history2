/****************************************************************************
** $Id: $
**
** Implementation of QObject class
**
** Created : 930418
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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
#include "qsignalslotimp.h"
#include "qregexp.h"
#include "qmetaobject.h"
#include "qpixmap.h"
#include "qiconset.h"
#include "qimage.h"
#include "qregion.h"
#include "qbitmap.h"
#include "qpointarray.h"
#include "qcursor.h"
#include "qdatetime.h"
#include "qucom.h"

#include <ctype.h>

class QObjectPrivate
{
};


/*! \class Qt qnamespace.h

  \brief The Qt class is a namespace for miscellaneous identifiers
  that need to be global-like.

  \ingroup misc

  Normally, you can ignore this class.  QObject and a few other
  classes inherit it, so all the identifiers in the Qt namespace
  are usable without qualification.

  However, you may occasionally need to say \c Qt::black instead of just
  \c black, particularly in static utility functions (such as many
  class factories).

*/

/*! \enum Qt::Orientation

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

  QObject is the heart of the \link object.html Qt object model.
  \endlink The central feature in this model is a very powerful
  mechanism for seamless object communication called \link
  signalsandslots.html signals and slots \endlink. You can can connect
  a signal to a slot with connect() and destroy the connection with
  disconnect(). To avoid never ending notification loops you can
  temporarily block signals with blockSignals(). The protected
  functions connectNotify() and disconnectNotify() make it possible to
  track connections.

  QObjects organize themselves in object trees. When you create a
  QObject with another object as parent, the object will automatically
  do an insertChild() on the parent and thus show up in the parent's
  children() list. The parent takes ownership of the object i.e. it
  will automatically delete its children in its destructor. You can
  look for an object by name and optionally type using child() or
  queryList(), and get the list of tree roots using objectTrees().

  Every object has an object name() and can report its className() and
  whether it inherits() another class in the QObject inheritance
  hierarchy.

  When an object is deleted, it emits a destroyed() signal. You can
  catch this signal to avoid dangling references to QObjects. The
  QGuardedPtr class provides an elegant way to use this feature.

  QObjects can receive events through event() and filter the events of
  other objects. See installEventFilter() and eventFilter() for
  details. A convenience handler childEvent() can be reimplemented to
  catch child events.

  Last but not least, QObject provides the basic timer support in Qt;
  see QTimer for high-level support for timers.

  Notice that the Q_OBJECT macro is mandatory for any object that
  implements signals, slots or properties.  You also need to run the \link
  moc.html moc program (Meta Object Compiler) \endlink on the source file.
  We strongly recommend the use of this macro in \e all subclasses of QObject
  regardless of whether or not they actually use signals, slots and
  properties, since failure to do so may lead certain functions to
  exhibit undefined behaviour.

  All Qt widgets inherit QObject. The convenience function
  isWidgetType() returns whether an object is actually a widget.  It
  is much faster than inherits( "QWidget" ).

    Some QObject functions, e.g. children(), objectTrees() and
    queryList() return a QObjectList. A QObjectList is a QPtrList of
    QObjects. QObjectLists support the same operations as QPtrLists and
    have an iterator class, QObjectListIt.
*/


static QObject * sigSender = 0;

/* (no '!' on purpose since this is an internal class)
  \class QSenderObject qobject.h
  \brief Internal object used for sending signals.

  \internal

  It is generally a very bad idea to use this class directly in
  application programs.

  In particular, you cannot not use it to send signals from classes
  that do not inherit QObject. If you wish to do that, make an
  internal class that inherits QObject and has the necessary signals
  and slots.  Alternatively, you can use the QSignal class.
*/

/* \internal */

void QSenderObject::setSender( QObject *s )
{
    sigSender=s;
}


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

static QCString qt_rmWS( const char *s )
{
    QCString result( qstrlen(s)+1 );
    char *d = result.data();
    char last = 0;
    while( *s && isSpace(*s) )			// skip leading space
	s++;
    while ( *s ) {
	while ( *s && !isSpace(*s) )
	    last = *d++ = *s++;
	while ( *s && isSpace(*s) )
	    s++;
	if ( *s && isIdentChar(*s) && isIdentChar(last) )
	    last = *d++ = ' ';
    }
    result.truncate( (int)(d - result.data()) );
    int void_pos = result.find("(void)");
    if ( void_pos >= 0 )
	result.remove( void_pos+1, strlen("void") );
    return result;
}


// Event functions, implemented in qapplication_xxx.cpp

int   qStartTimer( int interval, QObject *obj );
bool  qKillTimer( int id );
bool  qKillTimer( QObject *obj );

void  qRemovePostedEvents( QObject* );

static void removeObjFromList( QObjectList *objList, const QObject *obj,
			       bool single=FALSE )
{
    if ( !objList )
	return;
    int index = objList->findRef( obj );
    while ( index >= 0 ) {
	objList->remove();
	if ( single )
	    return;
	index = objList->findNextRef( obj );
    }
}


/*!
  \relates QObject

  Returns a pointer to the object named \a name that inherits \a type
  and with a given \a parent.

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
	QObjectListIt it( *list );
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
static void cleanup_object_trees()
{
    delete object_trees;
    object_trees = 0;
}

static void ensure_object_trees()
{
    if ( object_trees )
	return;
    object_trees = new QObjectList;
    qAddPostRoutine( cleanup_object_trees );
}

static void insert_tree( QObject* obj )
{
    ensure_object_trees();
    object_trees->insert(0, obj );
}

static void remove_tree( QObject* obj )
{
    if ( object_trees )
	object_trees->removeRef( obj );
}


/*****************************************************************************
  QObject member functions
 *****************************************************************************/

/*!
  Constructs an object with the parent object \a parent and a \a name.

  The parent of an object may be viewed as the object's owner. For
  instance, a \link QDialog dialog box\endlink is the parent of the
  "OK" and "Cancel" buttons it contains.

  The destructor of a parent object destroys all child objects.

  Setting \a parent to 0 constructs an object with no parent.
  If the object is a widget, it will become a top-level window.

  The object name is some text that can be used to identify a QObject.
  It's particularly useful in conjunction with
    \link designer-manual.html <i>Qt Designer</i>\endlink.
  You can find an object by name (and type) using child(). To find
  several objects use queryList().

  \sa parent(), name(), child(), queryList()
*/

QObject::QObject( QObject *parent, const char *name )
    : d( 0 )
{
    if ( !metaObj )				// will create object dict
	(void) staticMetaObject();
    objname       = name ? qstrdup(name) : 0;   // set object name
    childObjects  = 0;				// no children yet
    connections   = 0;				// no connections yet
    senderObjects = 0;				// no signals connected yet
    eventFilters  = 0;				// no filters installed
    postedEvents  = 0;				// no events posted
    isSignal   = FALSE;				// assume not a signal object
    isWidget   = FALSE;				// assume not a widget object
    pendTimer  = FALSE;				// no timers yet
    blockSig   = FALSE;				// not blocking signals
    wasDeleted = FALSE;				// double-delete catcher
    isTree = FALSE;				// no tree yet
    parentObj  = parent;			// to avoid root checking in insertChild()
    if ( parent ) {				// add object to parent
	parent->insertChild( this );
    } else {
	insert_tree( this );
	isTree = TRUE;
    }

    // null out sigSender... this may be wrong.
    sigSender = 0;
}


/*!
  Destroys the object, deleting all its child objects.

  All signals to and from the object are automatically disconnected.

  \warning All child objects are deleted.  If any of these objects are
  on the stack or global, sooner or later your program will crash.  We do
  not recommend holding pointers to child objects from outside the parent.
  If you still do, the QObject::destroyed() signal gives you an
  opportunity to detect when an object is destroyed.
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
    emit destroyed( this );
    emit destroyed();
    if ( objname )
	delete [] (char*)objname;
    objname = 0;
    if ( pendTimer )				// might be pending timers
	qKillTimer( this );
    if ( postedEvents && qApp )
	QApplication::removePostedEvents( this );
    if ( isTree ) {
	remove_tree( this );		// remove from global root list
	isTree = FALSE;
    }
    if ( parentObj )				// remove it from parent object
	parentObj->removeChild( this );
    register QObject *obj;
    if ( senderObjects ) {			// disconnect from senders
	QObjectList *tmp = senderObjects;
	senderObjects = 0;
	obj = tmp->first();
	while ( obj ) {				// for all senders...
	    obj->disconnect( this );
	    obj = tmp->next();
	}
	delete tmp;
	senderObjects = 0;
    }
    if ( connections ) {			// disconnect receivers
	for ( int i = 0; i < (int) connections->size(); i++ ) {
	    QConnectionList* clist = (*connections)[i]; // for each signal...
	    if ( !clist )
		continue;
	    register QConnection *c;
	    QConnectionListIt cit(*clist);
	    while( (c=cit.current()) ) {	// for each connected slot...
		++cit;
		if ( (obj=c->object()) )
		    removeObjFromList( obj->senderObjects, this );
	    }
	}
	delete connections;
	connections = 0;
    }
    if ( eventFilters ) {
	delete eventFilters;
	eventFilters = 0;
    }
    if ( childObjects ) {			// delete children objects
	QObjectListIt it(*childObjects);
	while ( (obj=it.current()) ) {
	    ++it;
	    obj->parentObj = 0;
	    childObjects->removeRef( obj );
	    delete obj;
	}
	delete childObjects;
    }
}


/*!
  \fn QMetaObject *QObject::metaObject() const
  Returns a pointer to the meta object of this object.

  A meta object contains information about a class that inherits
  QObject, e.g. class name, superclass name, properties, signals and
  slots. Every class that contains the Q_OBJECT macro will also
  have a meta object.

  The meta object information is required by the signal/slot
  connection mechanism and the property system.  The functions isA()
  and inherits() also make use of the meta object.
*/

/*!\fn const char *QObject::className() const

  Returns the class name of this object.

  This function is generated by the \link metaobjects.html Meta Object
  Compiler. \endlink

  \warning This function will return a wrong name if the class
  definition lacks the Q_OBJECT macro.

  \sa name(), inherits(), isA(), isWidgetType()
*/

/*!
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

bool QObject::isA( const char *clname ) const
{
    return qstrcmp( clname, className() ) == 0;
}

/*!
  Returns TRUE if this object is an instance of a class that inherits
  \a clname, and \a clname inherits QObject; otherwise returns FALSE.

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

bool QObject::inherits( const char *clname ) const
{
    return metaObject()->inherits( clname );
}

/*!
  \property QObject::name

  \brief the name of this object

  You can find an object by name (and type) using child(). You can
  find a set of objects with queryList().

  The object name is set by the constructor or by the setName()
  function. The object name is not very useful in the current version
  of Qt, but will become increasingly important in the future.

  If the object does not have a name, the name() function returns
  "unnamed", so printf() (used in qDebug()) will not be asked to output a null
  pointer. If you want a null pointer to be returned for unnamed objects, you
  can call name( 0 ).

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

/*! \overload

  Returns the name of this object, or \a defaultName if the object
  does not have a name.
*/

const char * QObject::name( const char * defaultName ) const
{
    return objname ? objname : defaultName;
}


/*!  Searches the children and optionally grandchildren of this object,
  and returns a child that is called \a objName that inherits \a
  inheritsClass. If \a inheritsClass is 0 (the default), any class
  matches.

  If \a recursiveSearch is TRUE (the default), child() performs a
  depth-first search of the object's children.

  If there is no such object, this function returns 0. If there are
  more than one, the first one found is retured; if you need
  all of them, use queryList().
*/
QObject* QObject::child( const char *objName, const char *inheritsClass,
			 bool recursiveSearch )
{
    const QObjectList *list = children();
    if ( !list )
	return 0;

    bool onlyWidgets = ( inheritsClass && qstrcmp( inheritsClass, "QWidget" ) == 0 );
    QObjectListIt it( *list );
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

  Calling this function is equivalent to calling inherits("QWidget"),
  except that it is much faster.
*/

/*!
  \fn bool QObject::highPriority() const
  Returns TRUE if the object is a high-priority object, or FALSE if it is a
  standard-priority object.

  High-priority objects are placed first in QObject's list of children
  on the assumption that they will be referenced very often.
*/


/*!
  This virtual function receives events to an object and should return
  TRUE if the event \a e was recognized and processed.

  The event() function can be reimplemented to customize the behavior of
  an object.

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

  QTimer provides a higher-level interface to the timer functionality,
  and also more general information about timers.

  \sa startTimer(), killTimer(), killTimers(), event()
*/

void QObject::timerEvent( QTimerEvent * )
{
}


/*!
  This event handler can be reimplemented in a subclass to receive
  child events.

  Child events are sent to objects when children are inserted or removed.

  Note that events with QEvent::type() \c QEvent::ChildInserted are
  posted (with \l{QApplication::postEvent()}) to make sure that the
  child's construction is completed before this function is called.

  Note that if a child is removed immediately after it is inserted, the
  \c ChildInserted event may be suppressed, but the \c ChildRemoved
  event will always be sent. In this case there will be a \c ChildRemoved
  event without a corresponding \c ChildInserted event.

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
  value at least as large as the "User" item of the \l QEvent::Type enum,
  and is typically a QCustomEvent or QCustomEvent subclass.

  \sa event(), QCustomEvent
*/
void QObject::customEvent( QCustomEvent * )
{
}



/*!
  Filters events if this object has been installed as an event filter for
  the \a watched object.

    In your reimplementation of this function, if you want to filter
    the event \a e, out, i.e. stop it being handled further, return TRUE;
    otherwise return FALSE.

  \warning
  If you delete the receiver object in this function, be sure to return TRUE.
  Otherwise, Qt will forward the event to the deleted object and the
  program might crash.

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
    QObjectListIt it( *eventFilters );
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
  Blocks signals if \a block is TRUE, or unblocks signals if \a block is FALSE.

  Emitted signals disappear into hyperspace if signals are blocked.
*/

void QObject::blockSignals( bool block )
{
    blockSig = block;
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
  killTimer() or killTimers() is called.  If \a interval is 0, then
  the timer event occurs once every time there are no more window system
  events to process.

  The virtual timerEvent() function is called with the QTimerEvent event
  parameter class when a timer event occurs.  Reimplement this function to
  get timer events.

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

  There is practically no upper limit for the interval value (more than one
  year is possible).  Note that QTimer's accuracy depends on the underlying
  operating system and hardware.  Most platforms support an accuracy of
  20ms; some provide more.  If Qt is unable to deliver the requested number
  of timer clicks, it will silently discard some.

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

  The timer identifier is returned by startTimer() when a timer event is
  started.

  \sa timerEvent(), startTimer(), killTimers()
*/

void QObject::killTimer( int id )
{
    qKillTimer( id );
}

/*!
  Kills all timers that this object has started.

  Note that using this function can cause hard-to-find bugs: it kills
  timers started by sub- and superclasses as well as those started by
  you, which is often not what you want.  We recommend
  using a QTimer or perhaps killTimer().

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
  Returns a list of child objects, or 0 if this object has no children.

  The QObjectList class is defined in the qobjectlist.h header file.

  The first child added is the \link QPtrList::first() first\endlink
  object in the list and the last child added is the \link
  QPtrList::last() last\endlink object in the list, i.e. new children
  are appended at the end.

  Note that the list order changes when QWidget children are \link
  QWidget::raise() raised\endlink or \link QWidget::lower()
  lowered.\endlink A widget that is raised becomes the last object in
  the list, and a widget that is lowered becomes the first object in
  the list.

  \sa child(), queryList(), parent(), insertChild(), removeChild()
*/


/*!
  Returns a pointer to the list of all object trees (their root objects),
  or 0 if there are no objects.

  The QObjectList class is defined in the qobjcoll.h header file.

  The latest root object created is the \link QPtrList::first()
  first\endlink object in the list and the first root object added is
  the \link QPtrList::last() last\endlink object in the list.

  \sa children(), parent(), insertChild(), removeChild()
 */
const QObjectList *QObject::objectTrees()
{
    return object_trees;
}


/*!  Searches the children and optionally grandchildren of this object,
  and returns a list of those objects that are named or that match \a
  objName and inherit \a inheritsClass.  If \a inheritsClass is 0 (the
  default), all classes match.  If \a objName is 0 (the default), all
  object names match.

  If \a regexpMatch is TRUE (the default), \a objName is a regular
  expression that the objects's names must match. The syntax is that
  of a QRegExp. If \a regexpMatch is FALSE, \a objName is a string
  and object names must match it exactly.

  Note that \a inheritsClass uses single inheritance from QObject, the
  way inherits() does.  According to inherits(), QMenuBar inherits
  QWidget but not QMenuData. This does not quite match reality, but is
  the best that can be done on the wide variety of compilers Qt
  supports.

  Finally, if \a recursiveSearch is TRUE (the default), queryList()
  searches \e{n}th-generation as well as first-generation children.

  If all this seems a bit complex for your needs, the simpler
  child() function may be what you want.

  This somewhat contrived example disables all the buttons in this
  window:
\code
    QObjectList *l = topLevelWidget()->queryList( "QButton" );
    QObjectListIt it( *l ); // iterate over the buttons
    QObject *obj;

    while ( (obj = it.current()) != 0 ) {
	// for each found object...
	++it;
	((QButton*)obj)->setEnabled( FALSE );
    }
    delete l; // delete the list, not the objects
\endcode

  \warning Delete the list as soon you have finished using it.
  The list contains pointers that may become invalid at almost any
  time without notice (as soon as the user closes a window you may
  have dangling pointers, for example).

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

/*! \internal

  Returns a list of objects/slot pairs that are connected to the
  \a signal, or 0 if nothing is connected to it.
*/

QConnectionList *QObject::receivers( const char* signal ) const
{
    if ( connections && signal ) {
	if ( *signal == '2' ) {			// tag == 2, i.e. signal
	    QCString s = qt_rmWS( signal+1 );
	    return receivers( metaObject()->findSignal( (const char*)s, TRUE ) );
	} else {
	    return receivers( metaObject()->findSignal(signal, TRUE ) );
	}
    }
    return 0;
}

/*! \internal

  Returns a list of objects/slot pairs that are connected to the
  signal, or 0 if nothing is connected to it.
*/

QConnectionList *QObject::receivers( int signal ) const
{
    if ( connections && signal >= 0 )
	return connections->at( signal );
    return 0;
}


/*!
  Inserts an object \a obj into the list of child objects.

  \warning This function cannot be used to make one widget the child
  widget of another widget.  Child widgets can only be created by
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
    }
#if defined(QT_CHECK_STATE)
    else if ( childObjects->findRef(obj) >= 0 ) {
	qWarning( "QObject::insertChild: Object %s::%s already in list",
		 obj->className(), obj->name( "unnamed" ) );
	return;
    }
#endif
    obj->parentObj = this;
    childObjects->append( obj );

    QChildEvent *e = new QChildEvent( QEvent::ChildInserted, obj );
    QApplication::postEvent( this, e );
}

/*!
  Removes the child object \a obj from the list of children.

  \warning
  This function will not remove a child widget from the screen.
  It will only remove it from the parent widget's list of children.

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
  Installs an event filter \a obj on this object.

  An event filter is an object that receives all events that are sent to
  this object.	The filter can either stop the event or forward it to this
  object.  The event filter \a obj receives events via its eventFilter()
  function.  The eventFilter() function must return TRUE if the event
  should be filtered, (i.e. stopped); otherwise it must return FALSE.

  If multiple event filters are installed on a single object, the
  filter that was installed last is activated first.

  Example:
\code
    #include <qwidget.h>

    class MyWidget : public QWidget
    {
	Q_OBJECT
    public:
	MyWidget( QWidget *parent = 0, const char *name = 0 );

    protected:
	bool eventFilter( QObject *, QEvent * );
    };

    MyWidget::MyWidget( QWidget *parent, const char *name )
	: QWidget( parent, name )
    {
	// install a filter on the parent (if any)
	if ( parent )
	    parent->installEventFilter( this );
    }

    bool MyWidget::eventFilter( QObject *o, QEvent *e )
    {
	if ( e->type() == QEvent::KeyPress ) {
	    // special processing for key press
	    QKeyEvent *k = (QKeyEvent *)e;
	    qDebug( "Ate key press %d", k->key() );
	    return TRUE; // eat event
	} else {
	    // standard event processing
	    return QWidget::eventFilter( o, e );
	}
    }
\endcode

  The QAccel class, for example, uses this technique to intercept
  accelerator key presses.

  \warning
  If you delete the receiver object in your eventFilter() function, be
  sure to return TRUE. If you return FALSE, Qt sends the event to the
  deleted object and the program will crash.

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
  Removes an event filter object \a obj from this object.
  The request is ignored if such an event filter has not been installed.

  All event filters for this object are automatically removed when this
  object is destroyed.

  It is always safe to remove an event filter, even during event filter
  activation (i.e. from the eventFilter() function).

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


/*****************************************************************************
  Signal connection management
 *****************************************************************************/

#if defined(QT_CHECK_RANGE)

static bool check_signal_macro( const QObject *sender, const char *signal,
				const char *func, const char *op )
{
    int sigcode = (int)(*signal) - '0';
    if ( sigcode != QSIGNAL_CODE ) {
	if ( sigcode == QSLOT_CODE )
	    qWarning( "QObject::%s: Attempt to %s non-signal %s::%s",
		     func, op, sender->className(), signal+1 );
	else
	    qWarning( "QObject::%s: Use the SIGNAL macro to %s %s::%s",
		     func, op, sender->className(), signal );
	return FALSE;
    }
    return TRUE;
}

static bool check_member_code( int code, const QObject *object,
			       const char *member, const char *func )
{
    if ( code != QSLOT_CODE && code != QSIGNAL_CODE ) {
	qWarning( "QObject::%s: Use the SLOT or SIGNAL macro to "
		 "%s %s::%s", func, func, object->className(), member );
	return FALSE;
    }
    return TRUE;
}

static void err_member_notfound( int code, const QObject *object,
				 const char *member, const char *func )
{
    const char *type = 0;
    switch ( code ) {
	case QSLOT_CODE:   type = "slot";   break;
	case QSIGNAL_CODE: type = "signal"; break;
    }
    if ( strchr(member,')') == 0 )		// common typing mistake
	qWarning( "QObject::%s: Parentheses expected, %s %s::%s",
		 func, type, object->className(), member );
    else
	qWarning( "QObject::%s: No such %s %s::%s",
		 func, type, object->className(), member );
}


static void err_info_about_objects( const char * func,
				    const QObject * sender,
				    const QObject * receiver )
{
    const char * a = sender->name(), * b = receiver->name();
    if ( a )
	qWarning( "QObject::%s:  (sender name:   '%s')", func, a );
    if ( b )
	qWarning( "QObject::%s:  (receiver name: '%s')", func, b );
}

static void err_info_about_candidates( int code,
				       const QMetaObject* mo,
				       const char* member,
				       const char *func	)
{
    if ( strstr(member,"const char*") ) {
	// porting help
	QCString newname = member;
	int p;
	while ( (p=newname.find("const char*")) >= 0 ) {
	    newname.replace(p, 11, "const QString&");
	}
	const QMetaData *rm = 0;
	switch ( code ) {
	case QSLOT_CODE:
	    rm = mo->slot( mo->findSlot( newname, TRUE ), TRUE );
	    break;
	case QSIGNAL_CODE:
	    rm = mo->signal( mo->findSignal( newname, TRUE ), TRUE );
	    break;
	}
	if ( rm ) {
	    qWarning("QObject::%s:  Candidate: %s", func, newname.data());
	}
    }
}


#endif // QT_CHECK_RANGE


/*! Returns a pointer to the object that sent the signal, if called in a
  slot before any function call or signal emission.  Returns an
  undefined value in all other cases.

  \warning This function will return something apparently correct in
  other cases as well.  However, its value may change during any function
  call, depending on what signal-slot connections are activated during
  that call.  In Qt 3.0 the value will change more often than in 2.x.

  \warning
  This function violates the object-oriented principle of modularity.
  However, getting access to the sender might be practical when many
  signals are connected to a single slot. The sender is undefined if
  the slot is called as a normal C++ function.
*/

const QObject *QObject::sender()
{
    return sigSender;
}


/*!
  \fn void QObject::connectNotify( const char *signal )

  This virtual function is called when something has been connected to
  \a signal in this object.

  \warning
  This function violates the object-oriented principle of modularity.
  However, it might be useful when you need to perform expensive
  initialization only if something is connected to a signal.

  \sa connect(), disconnectNotify()
*/

void QObject::connectNotify( const char * )
{
}

/*!
  \fn void QObject::disconnectNotify( const char *signal )

  This virtual function is called when something has been disconnected from
  \a signal in this object.

  \warning
  This function violates the object-oriented principle of modularity.
  However, it might be useful for optimizing access to expensive resources.

  \sa disconnect(), connectNotify()
*/

void QObject::disconnectNotify( const char * )
{
}


/*!
  \fn bool QObject::checkConnectArgs( const char *signal, const QObject *receiver, const char *member )

  Returns TRUE if the \a signal and the \a member arguments are compatible;
  otherwise returns FALSE. (The \a receiver argument is currently ignored.)

  \warning
  We recommend that you use the default implementation and do not
  reimplement this function.

  \omit
  TRUE:	 "signal(<anything>)",	"member()"
  TRUE:	 "signal(a,b,c)",	"member(a,b,c)"
  TRUE:	 "signal(a,b,c)",	"member(a,b)", "member(a)" etc.
  FALSE: "signal(const a)",	"member(a)"
  FALSE: "signal(a)",		"member(const a)"
  FALSE: "signal(a)",		"member(b)"
  FALSE: "signal(a)",		"member(a,b)"
*/

bool QObject::checkConnectArgs( const char    *signal,
				const QObject *,
				const char    *member )
{
    const char *s1 = signal;
    const char *s2 = member;
    while ( *s1++ != '(' ) { }			// scan to first '('
    while ( *s2++ != '(' ) { }
    if ( *s2 == ')' || qstrcmp(s1,s2) == 0 )	// member has no args or
	return TRUE;				//   exact match
    int s1len = qstrlen(s1);
    int s2len = qstrlen(s2);
    if ( s2len < s1len && qstrncmp(s1,s2,s2len-1)==0 && s1[s2len-1]==',' )
	return TRUE;				// member has less args
    return FALSE;
}

/*!
  Normlizes the signal or slot definition \a signalSlot by removing
  unnecessary whitespace.
*/

QCString QObject::normalizeSignalSlot( const char *signalSlot )
{
    if ( !signalSlot )
	return QCString();
    return  qt_rmWS( signalSlot );
}



/*!
  \overload bool QObject::connect( const QObject *sender, const char *signal, const char *member ) const

  Connects \a signal from the \a sender object to this object's \a member.

  Equivalent to: \c{QObject::connect(sender, signal, this, member)}.

  \sa disconnect()
*/

/*!  Connects \a signal from the \a sender object to \a member in object
  \a receiver, and returns TRUE if the connection succeeds; otherwise
  returns FALSE.

  You must use the SIGNAL() and SLOT() macros when specifying the \a signal
  and the \a member, for example:
\code
    QLabel     *label  = new QLabel;
    QScrollBar *scroll = new QScrollBar;
    QObject::connect( scroll, SIGNAL(valueChanged(int)),
                      label,  SLOT(setNum(int)) );
\endcode

  This example ensures that the label always displays the current scroll bar
  value.

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
	aButton = new QPushButton( this );
	connect( aButton, SIGNAL(clicked()), SIGNAL(myUsefulSignal()) );
    }
\endcode

  In this example, the MyWidget constructor relays a signal from a
  private member variable, and makes it available under a name that
  relates to MyWidget.

  A signal can be connected to many slots and signals. Many signals
  can be connected to one slot.

  If a signal is connected to several slots, the slots are activated
  in an arbitrary order when the signal is emitted.

  The function returns TRUE if it successfully connects the signal to
  the slot.  It will return FALSE if it cannot create the connection,
  for example, if QObject is unable to verify the existence of either
  \a signal or \a member, or if their signatures aren't compatible.

  \sa disconnect()
*/

bool QObject::connect( const QObject *sender,	const char *signal,
		       const QObject *receiver, const char *member )
{
#if defined(QT_CHECK_NULL)
    if ( sender == 0 || receiver == 0 || signal == 0 || member == 0 ) {
	qWarning( "QObject::connect: Cannot connect %s::%s to %s::%s",
		 sender ? sender->className() : "(null)",
		 signal ? signal+1 : "(null)",
		 receiver ? receiver->className() : "(null)",
		 member ? member+1 : "(null)" );
	return FALSE;
    }
#endif
    QCString signal_name = qt_rmWS( signal );	// white space stripped
    QCString member_name = qt_rmWS( member );
    signal = signal_name;
    member = member_name;

    QMetaObject *smeta = sender->metaObject();

#if defined(QT_CHECK_RANGE)
    if ( !check_signal_macro( sender, signal, "connect", "bind" ) )
	return FALSE;
#endif
    signal++;					// skip member type code

    int signal_index = smeta->findSignal( signal, TRUE );

    if ( signal_index < 0  ) {	// no such signal
#if defined(QT_CHECK_RANGE)
	err_member_notfound( QSIGNAL_CODE, sender, signal, "connect" );
	err_info_about_candidates( QSIGNAL_CODE, smeta, signal, "connect" );
	err_info_about_objects( "connect", sender, receiver );
#endif
	return FALSE;
    }
    const QMetaData *sm = smeta->signal( signal_index, TRUE );
    signal = sm->name;				// use name from meta object

    int membcode = member[0] - '0';		// get member code

    QObject *s = (QObject *)sender;		// we need to change them
    QObject *r = (QObject *)receiver;		//   internally

#if defined(QT_CHECK_RANGE)
    if ( !check_member_code( membcode, r, member, "connect" ) )
	return FALSE;
#endif
    member++;					// skip code

    QMetaObject *rmeta = r->metaObject();
    const QMetaData   *rm = 0;

    switch ( membcode ) {			// get receiver member
	case QSLOT_CODE:
	    rm = rmeta->slot( rmeta->findSlot( member, TRUE ), TRUE );
	    break;
	case QSIGNAL_CODE:
	    rm = rmeta->signal( rmeta->findSignal( member, TRUE ), TRUE );
	    break;
    }
    if ( !rm ) {
#if defined(QT_CHECK_RANGE)
	err_member_notfound( membcode, r, member, "connect" );
	err_info_about_candidates( membcode, rmeta, member, "connect" );
	err_info_about_objects( "connect", sender, receiver );
#endif
	return FALSE;
    }
#if defined(QT_CHECK_RANGE)
    if ( !s->checkConnectArgs(signal,receiver,member) )
	qWarning( "QObject::connect: Incompatible sender/receiver arguments"
		 "\n\t%s::%s --> %s::%s",
		 s->className(), signal,
		 r->className(), member );
#endif
    connectInternal( sender, signal_index, receiver, membcode, rm->ptr );
    s->connectNotify( signal_name );
    return TRUE;
}

/*! \internal */

void QObject::connectInternal( const QObject *sender, int signal_index, const QObject *receiver,
			       int membcode, int member_index )
{
    QObject *s = (QObject*)sender;
    QObject *r = (QObject*)receiver;

    if ( !s->connections ) {			// create connections dict
	s->connections = new QSignalVec( 7 );
	Q_CHECK_PTR( s->connections );
	s->connections->setAutoDelete( TRUE );
    }

    QConnectionList *clist = s->connections->at( signal_index );
    if ( !clist ) {				// create receiver list
	clist = new QConnectionList;
	Q_CHECK_PTR( clist );
	clist->setAutoDelete( TRUE );
	s->connections->insert( signal_index, clist );
    }

    QMetaObject *rmeta = r->metaObject();
    const QMetaData *rm = 0;

    switch ( membcode ) {			// get receiver member
	case QSLOT_CODE:
	    rm = rmeta->slot( member_index, TRUE );
	    break;
	case QSIGNAL_CODE:
	    rm = rmeta->signal( member_index, TRUE );
	    break;
    }

    QConnection *c = new QConnection( r, member_index, rm ? rm->name : "qt_invoke", membcode );
    Q_CHECK_PTR( c );
    clist->append( c );
    if ( !r->senderObjects ) {			// create list of senders
	r->senderObjects = new QObjectList;
	Q_CHECK_PTR( r->senderObjects );
    }
    r->senderObjects->append( s );		// add sender to list
}


/*!
  \overload bool QObject::disconnect( const char *signal, const QObject *receiver, const char *member )

  Disconnects \a signal from \a member of \a receiver.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.
*/

/*!
  \overload bool QObject::disconnect( const QObject *receiver, const char *member )

  Disconnects all signals in this object from \a receiver's \a member.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.
*/

/*!
  Disconnects \a signal in object \a sender from \a member in object \a
  receiver.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.

  disconnect() is typically used in three ways, as the following examples
  demonstrate.
  \list 1
  \i Disconnect everything connected to an object's signals:
  \code
    disconnect( myObject, 0, 0, 0 );
  \endcode
  equivalent to the non-static overloaded function
  \code
    myObject->disconnect();
  \endcode
  \i Disconnect everything connected to a specific signal:
  \code
    disconnect( myObject, SIGNAL(mySignal()), 0, 0 );
  \endcode
  equivalent to the non-static overloaded function
  \code
    myObject->disconnect( SIGNAL(mySignal()) );
  \endcode
  \i Disconnect a specific receiver:
  \code
    disconnect( myObject, 0, myReceiver, 0 );
  \endcode
  equivalent to the non-static overloaded function
  \code
    myObject->disconnect(  myReceiver );
  \endcode
  \endlist

  0 may be used as a wildcard, meaning "any signal", "any receiving
  object", or "any slot in the receiving object", respectively.

  The \a sender may never be 0.  (You cannot disconnect signals from
  more than one object.)

  If \a signal is 0, it disconnects \a receiver and \a member from any
  signal.  If not, only the specified signal is disconnected.

  If \a receiver is 0, it disconnects anything connected to \a signal.
  If not, slots in objects other than \a receiver are not disconnected.

  If \a member is 0, it disconnects anything that is connected to \a
  receiver.  If not, only slots named \a member will be disconnected,
  and all other slots are left alone.  The \a member must be 0 if \a
  receiver is left out, so you cannot disconnect a specifically-named
  slot on all objects.

  \sa connect()
*/

bool QObject::disconnect( const QObject *sender,   const char *signal,
			  const QObject *receiver, const char *member )
{
#if defined(QT_CHECK_NULL)
    if ( sender == 0 || (receiver == 0 && member != 0) ) {
	qWarning( "QObject::disconnect: Unexpected null parameter" );
	return FALSE;
    }
#endif
    if ( !sender->connections )			// no connected signals
	return FALSE;
    QCString signal_name;
    QCString member_name;
    const QMetaData *rm = 0;
    QObject *s = (QObject *)sender;
    QObject *r = (QObject *)receiver;
    if ( member ) {
	member_name = qt_rmWS( member );
	member = member_name.data();
	int membcode = member[0] - '0';
#if defined(QT_CHECK_RANGE)
	if ( !check_member_code( membcode, r, member, "disconnect" ) )
	    return FALSE;
#endif
	member++;
	QMetaObject *rmeta = r->metaObject();

	switch ( membcode ) {			// get receiver member
	    case QSLOT_CODE:
		rm = rmeta->slot( rmeta->findSlot( member, TRUE ), TRUE );
		break;
	    case QSIGNAL_CODE:
		rm = rmeta->signal( rmeta->findSignal( member, TRUE ), TRUE );
		break;
	}
	if ( !rm ) {				// no such member
#if defined(QT_CHECK_RANGE)
	    err_member_notfound( membcode, r, member, "disconnect" );
	    err_info_about_candidates( membcode, rmeta, member, "connect" );
	    err_info_about_objects( "disconnect", sender, receiver );
#endif
	    return FALSE;
	}
    }

    QConnectionList *clist;
    register QConnection *c;
    if ( signal == 0 ) {			// any/all signals
	for ( int i = 0; i < (int) s->connections->size(); i++ ) {
	    clist = (*s->connections)[i]; // for all signals...
	    if ( !clist )
		continue;
	    c = clist->first();
	    while ( c ) {			// for all receivers...
		if ( r == 0 ) {			// remove all receivers
		    removeObjFromList( c->object()->senderObjects, s );
		    c = clist->next();
		} else if ( r == c->object() &&
			    (member == 0 ||
			     qstrcmp(member,c->memberName()) == 0) ) {
		    removeObjFromList( c->object()->senderObjects, s );
		    clist->remove();
		    c = clist->current();
		} else {
		    c = clist->next();
		}
	    }
	    if ( r == 0 )			// disconnect all receivers
		s->connections->insert( i, 0 );
	}
	s->disconnectNotify( 0 );
    } else {					// specific signal
	signal_name = qt_rmWS( signal );
	signal = signal_name.data();
#if defined(QT_CHECK_RANGE)
	if ( !check_signal_macro( s, signal, "disconnect", "unbind" ) )
	    return FALSE;
#endif
	signal++;

	QMetaObject *smeta = s->metaObject();
	if ( !smeta )			// no meta object
	    return FALSE;
	int signal_index = smeta->findSignal( signal, TRUE );
	if ( signal_index < 0 ) {
#if defined(QT_CHECK_RANGE)
		qWarning( "QObject::disconnect: No such signal %s::%s",
			 s->className(), signal );
#endif
		return FALSE;
	}
	clist = s->connections->at( signal_index );
	if ( !clist )
	    return FALSE;

	c = clist->first();
	while ( c ) {				// for all receivers...
	    if ( r == 0 ) {			// remove all receivers
		removeObjFromList( c->object()->senderObjects, s, TRUE );
		c = clist->next();
	    } else if ( r == c->object() && (member == 0 ||
				      qstrcmp(member,c->memberName()) == 0) ) {
		removeObjFromList( c->object()->senderObjects, s, TRUE );
		clist->remove();
		c = clist->current();
	    } else {
		c = clist->next();
	    }
	}
	if ( r == 0 )				// disconnect all receivers
	    s->connections->insert( signal_index, 0 );
	s->disconnectNotify( signal_name );
    }
    return TRUE;
}


/*! \fn QObject::destroyed()

  This signal is emitted immediately before the object is destroyed.

  All the objects's children are destroyed immediately after this signal
  is emitted.
*/

/*! \fn QObject::destroyed( QObject* obj)

   \overload
   This signal is emitted immediately before the object \a obj is destroyed.

   All the objects's children are destroyed immediately after this signal
   is emitted.
*/

/*!  Delete this object deferred.

  Instead of an immediate deletion this function schedules a deferred
  delete event for processing when Qt returns to the main event loop.
 */
void QObject::deferredDelete()
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


/*! \fn const char *QObject::tr( const char *sourceText, const char * comment ) const

  Returns a translated version of \a sourceText, or \a sourceText
  itself if there is no appropriate translated version.  The
  translation context is QObject with \a comment (null by default).
  All QObject subclasses using the Q_OBJECT macro automatically have
  a reimplementation of this function with the subclass name as
  context.

  \sa trUtf8() QApplication::translate()
      \link i18n.html Internationalization with Qt\endlink
*/

/*! \fn const char *QObject::trUtf8( const char *sourceText,
				     const char *comment ) const

  Returns a translated version of \a sourceText, or
  QString::fromUtf8(\a sourceText) if there is no appropriate version.
  It is otherwise identical to tr(\a sourceText, \a comment).

  \sa tr() QApplication::translate()
*/

static QMetaObjectCleanUp cleanUp_Qt = QMetaObjectCleanUp();

QMetaObject* QObject::staticQtMetaObject()
{
    static QMetaObject* qtMetaObject = 0;
    if ( qtMetaObject )
	return qtMetaObject;

#ifndef QT_NO_PROPERTIES
    static const QMetaEnum::Item enum_0[] = {
	{ "AlignLeft",  (int) Qt::AlignLeft },
	{ "AlignRight",  (int) Qt::AlignRight },
	{ "AlignHCenter",  (int) Qt::AlignHCenter },
	{ "AlignTop",  (int) Qt::AlignTop },
	{ "AlignBottom",  (int) Qt::AlignBottom },
	{ "AlignVCenter",  (int) Qt::AlignVCenter },
	{ "AlignCenter", (int) Qt::AlignCenter },
	{ "AlignAuto", (int) Qt::AlignAuto },
	{ "AlignJustify", (int) Qt::AlignJustify },
	{ "WordBreak", (int) Qt::WordBreak }
    };

    static const QMetaEnum::Item enum_1[] = {
	{ "Horizontal", (int) Qt::Horizontal },
	{ "Vertical", (int) Qt::Vertical }
    };

    static const QMetaEnum::Item enum_2[] = {
	{ "PlainText", (int) Qt::PlainText },
	{ "RichText", (int) Qt::RichText },
	{ "AutoText", (int) Qt::AutoText }
    };

    static const QMetaEnum::Item enum_3[] = {
	{ "FixedColor",  (int) Qt::FixedColor },
	{ "FixedPixmap",  (int) Qt::FixedPixmap },
	{ "NoBackground",  (int) Qt::NoBackground },
	{ "PaletteForeground",  (int) Qt::PaletteForeground },
	{ "PaletteButton",  (int) Qt::PaletteButton },
	{ "PaletteLight",  (int) Qt::PaletteLight },
	{ "PaletteMidlight",  (int) Qt::PaletteMidlight },
	{ "PaletteDark",  (int) Qt::PaletteDark },
	{ "PaletteMid",  (int) Qt::PaletteMid },
	{ "PaletteText",  (int) Qt::PaletteText },
	{ "PaletteBrightText",  (int) Qt::PaletteBrightText },
	{ "PaletteBase",  (int) Qt::PaletteBase },
	{ "PaletteBackground",  (int) Qt::PaletteBackground },
	{ "PaletteShadow",  (int) Qt::PaletteShadow },
	{ "PaletteHighlight",  (int) Qt::PaletteHighlight },
	{ "PaletteHighlightedText",  (int) Qt::PaletteHighlightedText },
	{ "PaletteButtonText",  (int) Qt::PaletteButtonText },
	{ "PaletteLink", (int) Qt::PaletteLink },
	{ "PaletteLinkVisited", (int) Qt::PaletteLinkVisited },
	{ "X11ParentRelative",  (int) Qt::X11ParentRelative }
    };

    static const QMetaEnum::Item enum_4[] = {
	{ "TextDate", (int) Qt::TextDate },
	{ "ISODate", (int) Qt::ISODate },
	{ "LocalDate", (int) Qt::LocalDate }
    };


    static const QMetaEnum enum_tbl[] = {
	{ "Alignment", 10, enum_0, TRUE },
	{ "Orientation", 2, enum_1, FALSE },
	{ "TextFormat", 3, enum_2, FALSE },
	{ "BackgroundMode", 18, enum_3, FALSE },
	{ "DateFormat", 3, enum_4, FALSE }
    };
#endif

    qtMetaObject = new QMetaObject( "Qt", 0,
			  0, 0,
			  0, 0,
#ifndef QT_NO_PROPERTIES
			  0, 0,
			  enum_tbl, 5,
#endif
			  0, 0 );
    cleanUp_Qt.setMetaObject( qtMetaObject );

    return qtMetaObject;
}

/*!
  \internal

  Signal activation with the most frequently used parameter/argument
    types.  All other combinations are generated by the meta object
    compiler.
  */
void QObject::activate_signal( int signal )
{
    if ( !connections || signalsBlocked() || signal < 0 )
	return;
    QConnectionList *clist = connections->at( signal );
    if ( !clist )
	return;
    QUObject o[1];
    activate_signal( clist, o );
}

/*! \internal */

void QObject::activate_signal( QConnectionList *clist, QUObject *o )
{
    if ( !clist )
	return;

    QObject *object;
    QConnection *c;
    if ( clist->count() == 1 ) { // save iterator
	c = clist->first();
	object = c->object();
	sigSender = this;
	if ( c->memberType() == QSIGNAL_CODE )
	    object->qt_emit( c->member(), o );
	else
	    object->qt_invoke( c->member(), o );
    } else {
	QConnectionListIt it(*clist);
	while ( (c=it.current()) ) {
	    ++it;
	    object = c->object();
	    sigSender = this;
	    if ( c->memberType() == QSIGNAL_CODE )
		object->qt_emit( c->member(), o );
	    else
		object->qt_invoke( c->member(), o );
	}
    }
}

/*!
  \overload void QObject::activate_signal( int signal, int )
*/

/*!
  \overload void QObject::activate_signal( int signal, double )
*/

/*!
  \overload void QObject::activate_signal( int signal, QString )
*/

/*!
  \fn void QObject::activate_signal_bool( int signal, bool )
  \internal

  Like the above functions, but since bool is sometimes
  only a typedef it cannot be a simple overload.
*/

#define ACTIVATE_SIGNAL_WITH_PARAM(FNAME,TYPE)				      \
void QObject::FNAME( int signal, TYPE param )				      \
{									      \
    if ( !connections || signalsBlocked() || signal < 0 )		      \
	return;								      \
    QConnectionList *clist = connections->at( signal );			      \
    if ( !clist )							      \
	return;								      \
    QUObject o[2];							      \
    pQUType_##TYPE->set( o+1, param );					      \
    activate_signal( clist, o );					      \
}

// We don't want to duplicate too much text so...

ACTIVATE_SIGNAL_WITH_PARAM( activate_signal, int )
ACTIVATE_SIGNAL_WITH_PARAM( activate_signal, double )
ACTIVATE_SIGNAL_WITH_PARAM( activate_signal, QString )
ACTIVATE_SIGNAL_WITH_PARAM( activate_signal_bool, bool )


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
	    QObjectListIt it(*object->children());
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

  This function is useful for debugging, but does nothing if
  the library has been compiled in release mode (i.e. without debugging
  information).

*/

void QObject::dumpObjectTree()
{
    dumpRecursive( 0, this );
}

/*!
  Dumps information about signal connections, etc. for this object to the
  debug output.

  This function is useful for debugging, but does nothing if
  the library has been compiled in release mode (i.e. without debugging
  information).
*/

void QObject::dumpObjectInfo()
{
#if defined(QT_DEBUG)
    qDebug( "OBJECT %s::%s", className(), name( "unnamed" ) );
    int n = 0;
    qDebug( "  SIGNALS OUT" );
    if ( connections ) {
	QConnectionList *clist;
	for ( uint i = 0; i < connections->size(); i++ ) {
	    if ( ( clist = connections->at( i ) ) ) {
		qDebug( "\t%s", metaObject()->signal( i, TRUE )->name );
		n++;
		register QConnection *c;
		QConnectionListIt cit(*clist);
		while ( (c=cit.current()) ) {
		    ++cit;
		    qDebug( "\t  --> %s::%s %s", c->object()->className(),
			    c->object()->name( "unnamed" ), c->memberName() );
		}
	    }
	}
    }
    if ( n == 0 )
	qDebug( "\t<None>" );

    qDebug( "  SIGNALS IN" );
    n = 0;
    if ( senderObjects ) {
	QObject *sender = senderObjects->first();
	while ( sender ) {
	    qDebug( "\t%s::%s",
		   sender->className(), sender->name( "unnamed" ) );
	    n++;
	    sender = senderObjects->next();
	}
    }
    if ( n == 0 )
	qDebug( "\t<None>" );
#endif
}

#ifndef QT_NO_PROPERTIES

/*!
  Sets the object's property \a name to \a value.

  Returns TRUE if the operation was successful; otherwise returns FALSE.

  Information about all available properties is provided through the
  metaObject().

  \sa property(), metaObject(), QMetaObject::propertyNames(), QMetaObject::property()
*/
bool QObject::setProperty( const char *name, const QVariant& value )
{
    if ( !value.isValid() )
	return FALSE;

    QVariant v = value;

    QMetaObject* meta = metaObject();
    if ( !meta )
	return FALSE;
    const QMetaProperty* p = meta->property( meta->findProperty( name, TRUE ), TRUE );
    if ( !p || !p->writable() )
	return FALSE;

    if ( p->isEnumType() ) {
	if ( v.type() == QVariant::String || v.type() == QVariant::CString ) {
	    if ( p->isSetType() ) {
		QString s = value.toString();
		// QStrList does not support split, use QStringList for that.
		QStringList l = QStringList::split( '|', s );
		QStrList keys;
		for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
		    keys.append( (*it).stripWhiteSpace().latin1() );
		v = QVariant( p->keysToValue( keys ) );
	    } else {
		v = QVariant( p->keyToValue( value.toCString().data() ) );
	    }
	} else if ( v.type() != QVariant::Int && v.type() != QVariant::UInt ) {
	    return FALSE;
	}
	return qt_property( p, 0, &v );
    }

    QVariant::Type type = QVariant::nameToType( p->type() );
    if ( !v.canCast( type ) )
	return FALSE;
    return qt_property( p, 0, &v );
}

/*!
  Returns the value of the object's \a name property.

  If no such property exists, the returned variant is invalid.

  Information about all available properties are provided through the
  metaObject().

  \sa setProperty(), QVariant::isValid(), metaObject(),
  QMetaObject::propertyNames(), QMetaObject::property()

*/
QVariant QObject::property( const char *name ) const
{
    QVariant v;
    QMetaObject* meta = metaObject();
    if ( !meta )
	return v;
    const QMetaProperty* p = meta->property( meta->findProperty( name, TRUE ), TRUE );
    if ( !p )
	return v;
    QObject* that = (QObject*) this; // moc ensures constness for the qt_property call
    that->qt_property( p, 1, &v );
    return v;
}

#endif // QT_NO_PROPERTIES

