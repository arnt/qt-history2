/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.cpp#233 $
**
** Implementation of QObject class
**
** Created : 930418
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qapplication.h"
#include "qobject.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qsignalslotimp.h"
#include "qregexp.h"
#include <ctype.h>

#ifdef QT_BUILDER
#include "qvariant.h"
#include "qpixmap.h"
#include "qiconset.h"
#include "qdom.h"
#endif // QT_BUILDER

// NOT REVISED
/*! \class Qt qnamespace.h

  \brief The Qt class is a namespace for miscellaneous identifiers
  that need to be global-like.

  Normally, you can ignore this class.  QObject and a few other
  classes inherit it, so that all the identifiers in the Qt namespace
  are visible to you without qualification.

  However, occasionally you may need to say \c Qt::black instead just
  \c black, particularly in static utility functions (such as many
  class factories).

*/

/*! \enum Qt::Orientation

  This type is used to signify whether an object should be \c
  Horizontal or \c Vertical (for example in QScrollBar).
*/


/*!
  \class QObject qobject.h
  \brief The QObject class is the base class of all Qt objects that can
  deal with signals, slots and events.

  \ingroup kernel

  Qt provides a very powerful mechanism for seamless object
  communication; \link metaobjects.html signal/slot
  connections\endlink. The signal/slot mechanism is an advanced way
  of making traditional callback routines.

  Example:
  \code
    //
    // The Mandelbrot class uses a QTimer to calculate the mandelbrot
    // set one scanline at a time without blocking the CPU.
    // It inherits QObject to use signals and slots.
    // Calling start() starts the calculation. The done() signal is
    // emitted when it has finished.
    // Note that this example is not complete. Feel free to complete it.
    //

    class Mandelbrot : public QObject
    {
	Q_OBJECT				// required for signals/slots
    public:
	Mandelbrot( QObject *parent=0, const char *name );
	...
    public slots:
	void	start();
    signals:
	void	done();
    private slots:
	void	calculate();
    private:
	QTimer	timer;
	...
    };

    //
    // Constructs and initializes a Mandelbrot object.
    //

    Mandelbrot::Mandelbrot( QObject *parent=0, const char *name )
	: QObject( parent, name )
    {
	connect( &timer, SIGNAL(timeout()), SLOT(calculate()) );
	...
    }

    //
    // Starts the calculation task. The internal calculate() slot
    // will be activated every 10 milliseconds.
    //

    void Mandelbrot::start()
    {
	if ( !timer.isActive() )		// not already running
	    timer.start( 10 );			// timeout every 10 ms
    }

    //
    // Calculates one scanline at a time.
    // Emits the done() signal when finished.
    //

    void Mandelbrot::calculate()
    {
	...			// perform the calculation for a scanline
	if ( finished ) {	// no more scanlines
	   timer.stop();
	   emit done();
	}
    }
  \endcode

  When an object has changed in some way that might be interesting for
  the outside world, it emits a signal to tell whoever is listening.
  All slots that are connected to this signal will be activated
  (called).  It is even possible to connect a signal directly to
  another signal.  (This will emit the second signal immediately
  whenever the first is emitted.)

  There is no limitation on how many slots that can be connected to
  a signal.  The slots will be activated in the order they were connected
  to the signal.

  Notice that the \c Q_OBJECT macro is mandatory for any object that
  implement signals or slots.  You also need to run the \link
  metaobjects.html moc program (Meta Object Compiler) \endlink on the
  source file.

  The signal/slot mechanism allows objects to easily reused, because
  the object that emits a signal does not need to know what the
  signals are connected to.

  All Qt widgets inherit QObject and use signals and slots.  A
  QScrollBar, for example, emits \link QScrollBar::valueChanged()
  valueChanged()\endlink whenever the scroll bar value changes.

  Meta objects are useful for doing more than connecting signals to slots.
  They also allow the programmer to obtain information about the class to
  which an object is instantiated from (see isA() and inherits()) or to
  produce a list of child objects that inherit a particular class
  (see queryList()).
*/


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

/*
  \fn void QSenderObject::setSender (QObject* s)

  Internal function, used in signal-slot connections.
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
#if defined(_CC_BOR_)
  /*
    Borland C++ 4.5 has a weird isspace() bug.
    isspace() usually works, but not here.
    This implementation is sufficient for our internal use: rmWS()
  */
    return (uchar)x <= 32;
#else
    return isspace( x );
#endif
}

static QCString qt_rmWS( const char *src )
{
    QCString result( strlen(src)+1 );
    char *d = result.data();
    char *s = (char *)src;
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


// Event functions, implemented in qapp_xxx.cpp

int   qStartTimer( int interval, QObject *obj );
bool  qKillTimer( int id );
bool  qKillTimer( QObject *obj );

void  qRemovePostedEvents( QObject* );


QMetaObject *QObject::metaObj = 0;


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

  Returns a pointer to the child named \a name of QObject \a parent
  which inherits type \a type.

  Returns 0 if there is no such child.

  \code
    QListBox * c = (QListBox *)::qt_find_obj_child(myWidget,QListBox,
						   "listboxname");
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
	while ( (obj=it.current()) ) {
	    ++it;
	    if ( qstrcmp(name,obj->name()) == 0 &&
		obj->inherits(type) )
		return obj;
	}
    }
    return 0;
}


/*****************************************************************************
  QObject member functions
 *****************************************************************************/

/*!
  Constructs an object with the parent object \e parent and a \e name.

  The parent of an object may be viewed as the object's owner. For
  instance, a \link QDialog dialog box\endlink is the parent of the
  "ok" and "cancel" buttons inside it.

  The destructor of a parent object destroys all child objects.

  Setting \e parent to 0 constructs an object with no parent.
  If the object is a widget, it will become a top-level window.

  The object name is a text that can be used to identify this QObject.
  It is not very useful in the current version of Qt, but it will become
  increasingly important in the future.

  The queryList() function searches the object tree for objects that
  matches a particular object name.

  \sa parent(), name(), queryList()
*/

QObject::QObject( QObject *parent, const char *name )
{
    if ( !objectDict )				// will create object dict
	initMetaObject();
    objname       = name ? qstrdup(name) : 0;   // set object name
    parentObj     = 0;				// parent set by insertChild()
    childObjects  = 0;				// no children yet
    connections   = 0;				// no connections yet
    senderObjects = 0;				// no signals connected yet
    eventFilters  = 0;				// no filters installed
    sigSender     = 0;				// no sender yet
    isSignal   = FALSE;				// assume not a signal object
    isWidget   = FALSE;				// assume not a widget object
    pendTimer  = FALSE;				// no timers yet
    pendEvent  = FALSE;				// no events yet
    blockSig   = FALSE;				// not blocking signals
    wasDeleted = FALSE;				// double-delete catcher
    if ( parent )				// add object to parent
	parent->insertChild( this );
}



/*!
  Destroys the object and all its children objects.

  All signals to and from the object are automatically disconnected.

  \warning \e All child objects are deleted.  If any of these objects are
  on the stack or global, your program will sooner or later crash.  We do
  not recommend holding pointers to child objects from outside the parent.
  If you still do, the QObject::destroyed() signal gives you an
  opportunity to detect when an object is destroyed.
*/

QObject::~QObject()
{
    if ( wasDeleted ) {
#if defined(DEBUG)
	qWarning( "Double QObject deletion detected." );
#endif
	return;
    }
    wasDeleted = 1;
    emit destroyed();
    if ( objname )
	delete [] (char*)objname;
    objname = 0;
    if ( pendTimer )				// might be pending timers
	qKillTimer( this );
    if ( pendEvent )
	QApplication::removePostedEvents( this );
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
    }
    if ( connections ) {			// disconnect receivers
	QSignalDictIt it(*connections);
	QConnectionList *clist;
	while ( (clist=it.current()) ) {	// for each signal...
	    ++it;
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
	    delete obj;
	    if ( !childObjects )		// removeChild resets it
		break;
	}
	delete childObjects;
    }
}


/*!
  \fn QMetaObject *QObject::metaObject() const
  Returns a pointer to the meta object of this object.

  A meta object contains information about a class that inherits QObject:
  class name, super class name, signals and slots. Every class that contains
  the \c Q_OBJECT macro will also have a meta object.

  The meta object information is required by the signal/slot connection
  mechanism.  The functions isA() and inherits() also make use of the
  meta object.

  The meta object is created by the initMetaObject() function, which
  is generated by the meta object compiler and called from the QObject
  constructor.
*/

/*!
  Returns the class name of this object.

  This function is generated by the \link metaobjects.html Meta Object
  Compiler. \endlink

  \warning This function will return an invalid name if the class
  definition lacks the \c Q_OBJECT macro.

  \sa name(), inherits(), isA(), isWidgetType()
*/

const char *QObject::className() const
{
    return "QObject";
}


/*!
  Returns TRUE if this object is an instance of a specified class,
  otherwise FALSE.

  Example:
  \code
    QTimer *t = new QTimer;		// QTimer inherits QObject
    t->isA("QTimer");			// returns TRUE
    t->isA("QObject");			// returns FALSE
  \endcode

  \sa inherits(), metaObject()
*/

bool QObject::isA( const char *clname ) const
{
    QMetaObject *meta = queryMetaObject();
    return meta ? strcmp(clname,meta->className()) == 0 : FALSE;
}

/*!
  Returns TRUE if this object is an instance of a class that inherits
  \e clname, and \a clname inherits QObject.

  (A class is considered to inherit itself.)

  Example:
  \code
    QTimer *t = new QTimer;		// QTimer inherits QObject
    t->inherits("QTimer");		// returns TRUE
    t->inherits("QObject");		// returns TRUE
    t->inherits("QButton");		// returns FALSE

    QScrollBar * s = new QScrollBar;	// inherits QWidget and QRangeControl
    s->inherits( "QWidget" );		// returns TRUE
    s->inherits( "QRangeControl" ); 	// returns FALSE
  \endcode

  \sa isA(), metaObject()
*/

bool QObject::inherits( const char *clname ) const
{
    QMetaObject *meta = queryMetaObject();
    while ( meta ) {
	if ( strcmp(clname,meta->className()) == 0 )
	    return TRUE;
	meta = meta->superClass();
    }
    return FALSE;
}

/*!
  Returns a list of all super classes. If \e includeThis is TRUE
  then the most derived class name of this object is added, too.
  The most derived classes appear first in the list. That means that
  QObject is always the last entry in the list.
*/

QStringList QObject::superClasses( bool includeThis ) const
{
  QStringList lst;

  QMetaObject *meta = queryMetaObject();
  if ( !includeThis )
    meta = meta->superClass();

  while ( meta ) {
    lst.append( QString::fromLatin1(meta->className()) );
    meta = meta->superClass();
  }

  return lst;
}


/*!
  \fn const char *QObject::name() const

  Returns the name of this object. If the object does not have a name,
  it will return "unnamed", so that printf() (used in qDebug()) will
  not be asked to output a null pointer.  If you want a null pointer
  to be returned for unnamed objects, you can call name( 0 ).

  \code
    qDebug( "MyClass::setPrecision(): (%s) unable to set precision to %f",
	    name(), newPrecision );
  \endcode

  The object name is set by the constructor or by the setName()
  function.  The object name is not very useful in the current version
  of Qt, but will become increasingly important in the future.

  The queryList() function searches the object tree for objects that
  matches a particular object name.

  \sa setName(), className(), queryList()
*/
const char * QObject::name() const
{
    // If you change the name here, the builder will be broken
    return objname ? objname : "unnamed";
}

/*!
  Returns the name of this object, or \a defaultName if the object
  does not have a name.
*/

const char * QObject::name( const char * defaultName ) const
{
    return objname ? objname : defaultName;
}


/*!
  Sets the name of this object to \e name.  The default name is the
  one assigned by the constructor.

  The object name is not very useful in the current version of Qt, but
  it will become increasingly important in the future.

  The queryList() function searches the object tree for objects that
  matches a particular object name.

  \sa name(), className(), queryList()
*/

void QObject::setName( const char *name )
{
    if ( objname )
	delete [] (char*) objname;
    objname = name ? qstrdup(name) : 0;
}

/*!
  Returns the pointer to a child widget with the required name and type or
  0 if no child matches. This function works recursive. That means it traverses
  the entire object tree to find the child. That in turn means that names have
  to be unique with regard to their toplevel window.

  If multiple widgets with the same name and type are found then it is undefined
  which one of them is returned.

  If \e type is set to 0 then the only criterion is the object's name.

  This function is useful if you need a widget of a dialog that was created from a ressource file.
*/

QObject* QObject::child( const char *name, const char *type )
{
    const QObjectList *list = children();
    if ( list ) {
	QObjectListIt it( *list );
	QObject *obj;
	while ( ( obj = it.current() ) ) {
	    ++it;
	    if ( ( !type || obj->inherits(type) ) && ( !name || qstrcmp( name, obj->name() ) == 0 ) )
		return obj;
	}

	// Recursion: Ask our children ...
	QObjectListIt it2( *list );
	while ( ( obj = it2.current() ) ) {
	    ++it2;
	    QObject* o = obj->child( name, type );
	    if ( o )
	      return o;
	}
    }

    return 0;
}

/*!
  \fn bool QObject::isWidgetType() const
  Returns TRUE if the object is a widget, or FALSE if not.

  Calling this function is equivalent to calling inherits("QWidget"),
  except that it is much faster.
*/

/*!
  \fn bool QObject::highPriority() const
  Returns TRUE if the object is a high priority object, or FALSE if it is a
  standard priority object.

  High priority objects are placed first in list of children,
  on the assumption that they will be referenced very often.
*/


/*!
  This virtual function receives events to an object and should return
  TRUE if the event was recognized and processed.

  The event() function can be reimplemented to customize the behavior of
  an object.

  \sa installEventFilter(), timerEvent(), QApplication::sendEvent(),
  QApplication::postEvent(), QWidget::event()
*/

bool QObject::event( QEvent *e )
{
#if defined(CHECK_NULL)
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
      case QEvent::ChildInserted: case QEvent::ChildRemoved:
	childEvent( (QChildEvent*)e );
	return TRUE;
#ifdef QT_BUILDER
      case QEvent::Configure:
	  configureEvent( (QConfigureEvent*)e );
	  return TRUE;
#endif // QT_BUILDER
      default:
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

  \sa event(), QChildEvent
*/

void QObject::childEvent( QChildEvent * )
{
}


/*!
  Filters events if this object has been installed as an event filter for
  another object.

  The reimplementation of this virtual function must return TRUE if the
  event should be stopped, or FALSE if the event should be dispatched normally.

  \sa installEventFilter()
*/

bool QObject::eventFilter( QObject *, QEvent * )
{
    return FALSE;				// don't do anything with it
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
  Returns TRUE if signals are blocked, or FALSE if signals are not blocked.

  Signals are not blocked by default.
  \sa blockSignals()
*/

/*!
  Blocks signals if \e block is TRUE, or unblocks signals if \e block is FALSE.

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

  A timer event will occur every \e interval milliseconds until
  killTimer() or killTimers() is called.  If \e interval is 0, then
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
    public:
	MyObject( QObject *parent=0, const char *name=0 );
    protected:
	void  timerEvent( QTimerEvent * );
    };

    MyObject::MyObject( QObject *parent, const char *name )
	: QObject( parent, name )
    {
	startTimer( 50 );			// 50 millisecond timer
	startTimer( 1000 );			// 1 second timer
	startTimer( 60000 );			// 1 minute timer
    }

    void MyObject::timerEvent( QTimerEvent *e )
    {
	qDebug( "timer event, id=%d", e->timerId() );
    }
  \endcode

  There is practically no upper limit for the interval value (more than
  one year).  The accuracy depends on the underlying operating system.
  Windows 95 has 55 millisecond (18.2 times per second) accuracy; other
  systems that we have tested (UNIX X11, Windows NT and OS/2) can
  handle 1 millisecond intervals.

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
  Kills the timer with the identifier \e id.

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

  Note that using this function can cause hard-to-find bugs: It kills
  timers started by sub- and superclasses as well as those started by
  you, which is often not what you want.  Therefore, we recommend
  using a QTimer, or perhaps killTimer().

  \sa timerEvent(), startTimer(), killTimer()
*/

void QObject::killTimers()
{
    qKillTimer( this );
}


static void objSearch( QObjectList *result,
		       QObjectList *list,
		       const char  *inheritsClass,
		       const char  *objName,
		       QRegExp	   *rx,
		       bool	    recurse )
{
    if ( !list || list->isEmpty() )		// nothing to search
	return;
    QObject *obj = list->first();
    while ( obj ) {
	bool ok = TRUE;
	if ( inheritsClass && !obj->inherits(inheritsClass) )
	    ok = FALSE;
	if ( ok ) {
	    if ( objName )
		ok = qstrcmp(objName,obj->name()) == 0;
	    else if ( rx )
		ok = rx->match(QString::fromLatin1(obj->name())) >= 0;
	}
	if ( ok )				// match!
	    result->append( obj );
	if ( recurse && obj->children() )
	    objSearch( result, (QObjectList *)obj->children(), inheritsClass,
		       objName, rx, recurse );
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

  The QObjectList class is defined in the qobjcoll.h header file.

  The latest child added is the \link QList::first() first\endlink object
  in the list and the first child is added is the \link QList::last()
  last\endlink object in the list.

  Note that the list order might change when \link QWidget widget\endlink
  children are \link QWidget::raise() raised\endlink or \link
  QWidget::lower() lowered\endlink. A widget that is raised becomes the
  last object in the list.  A widget that is lowered becomes the first
  object in the list.

  \sa queryList(), parent(), insertChild(), removeChild()
*/


/*!
  Returns a list of child objects found by a query.

  The query is specified by:
  \arg \e inheritsClass is the name of the base class that an object should
  inherit. Any class will be matched if \e inheritsClass is 0.
  \arg \e objName is the object name to search for. Any object name will be
  matched if \e objName is 0.
  \arg \e regexpMatch specifies whether \e objName is a regular expression
  (default) or not.
  \arg \e recursiveSearch must be \c TRUE (default) if you want to search
  the entire object tree, or \c FALSE if you want the search to traverse
  just the 1st level child objects of this object.

  Example:
  \code
    //
    // Sets a Courier 24 point fonts for all children in myWidget that
    // inherit QButton (i.e. QPushButton, QCheckBox, QRadioButton).
    //
    QObjectList	*list = myWidget->queryList( "QButton" );
    QObjectListIt it( *list );		// iterate over the buttons
    QFont newFont( "Courier", 24 );
    QObject * obj;
    while ( (obj=it.current()) != 0 ) {	// for each found object...
	++it;
	((QButton*)obj)->setFont( newFont );
    }
    delete list;			// delete the list, not the objects
  \endcode

  The QObjectList class is defined in the qobjcoll.h header file.

  \warning
  Delete the list away as soon you have finished using it.
  You can get in serious trouble if you for instance try to access
  an object that has been deleted.

  \sa children(), parent(), inherits(), name(), QRegExp
*/

QObjectList *QObject::queryList( const char *inheritsClass,
				 const char *objName,
				 bool regexpMatch,
				 bool recursiveSearch )
{
    QObjectList *list = new QObjectList;
    CHECK_PTR( list );
    if ( regexpMatch && objName ) {		// regexp matching
	QRegExp rx(QString::fromLatin1(objName));
	objSearch( list, (QObjectList *)children(), inheritsClass,
		   0, &rx, recursiveSearch );
    } else {
	objSearch( list, (QObjectList *)children(), inheritsClass,
		   objName, 0, recursiveSearch );
    }
    return list;
}


/*!
  Returns a list of objects/slot pairs that are connected to the
  signal, or 0 if nothing is connected to it.

  This function is for internal use.
*/

QConnectionList *QObject::receivers( const char *signal ) const
{
    if ( connections && signal ) {
	if ( *signal == '2' ) {			// tag == 2, i.e. signal
	    QCString s = qt_rmWS( signal+1 );
	    return connections->find( (const char*)s );
	} else {
	    return connections->find( signal );
	}
    }
    return 0;
}


/*!
  Inserts an object \e obj into the list of child objects.

  \warning This function cannot be used to make a widget a child
  widget of another.  Child widgets can be created only by setting the
  parent widget in the constructor or by calling QWidget::reparent().

  \sa removeChild(), QWidget::reparent()
*/

void QObject::insertChild( QObject *obj )
{
    if ( obj->parentObj ) {
#if defined(CHECK_STATE)
	if ( obj->parentObj != this && obj->isWidgetType() )
	    qWarning( "QObject::insertChild: Cannot reparent a widget, "
		     "use QWidget::reparent() instead" );
#endif
	obj->parentObj->removeChild( obj );
    }

    if ( !childObjects ) {
	childObjects = new QObjectList;
	CHECK_PTR( childObjects );
    }
#if defined(CHECK_STATE)
    else if ( childObjects->findRef(obj) >= 0 ) {
	qWarning( "QObject::insertChild: Object %s::%s already in list",
		 obj->className(), obj->name( "unnamed" ) );
	return;
    }
#endif
    obj->parentObj = this;
    childObjects->append( obj );

    obj->pendEvent = TRUE;
    QChildEvent *e = new QChildEvent( QEvent::ChildInserted, obj );
    QApplication::postEvent( this, e );
}

/*!
  Removes the child object \e obj from the list of children.

  \warning
  This function will not remove a child widget from the screen.
  It will only remove it from the parent widget's list of children.

  \sa insertChild(), QWidget::reparent()
*/

void QObject::removeChild( QObject *obj )
{
    if ( childObjects && childObjects->removeRef(obj) ) {
	obj->parentObj = 0;
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
  Installs an event filter object for this object.

  An event filter is an object that receives all events that are sent to
  this object.	The filter can either stop the event or forward it to this
  object.  The event filter object receives events via the eventFilter()
  function.  The eventFilter() function must return TRUE if the event
  should be stopped, or FALSE if the event should be dispatched normally.

  If multiple event filters are installed for a single object, the
  filter that was installed last will be activated first.

  Example:
  \code
    #include <qwidget.h>

    class MyWidget : public QWidget
    {
    public:
	MyWidget::MyWidget( QWidget *parent=0, const char *name=0 );
    protected:
	bool  eventFilter( QObject *, QEvent * );
    };

    MyWidget::MyWidget( QWidget *parent, const char *name )
	: QWidget( parent, name )
    {
	if ( parent )				// has a parent widget
	    parent->installEventFilter( this ); // then install filter
    }

    bool MyWidget::eventFilter( QObject *, QEvent *e )
    {
	if ( e->type() == QEvent::KeyPress ) {	// key press
	    QKeyEvent *k = (QKeyEvent*)e;
	    qDebug( "Ate key press %d", k->key() );
	    return TRUE;			// eat event
	}
	return FALSE;				// standard event processing
    }
  \endcode

  The QAccel class was implemented using this technique.

  \warning
  If you delete the receiver object in your eventFilter() function, be
  sure to return TRUE. If you return FALSE, Qt sends the event to the
  deleted object and the program will crash.

  \sa removeEventFilter(), eventFilter(), event()
*/

void QObject::installEventFilter( const QObject *obj )
{
    if ( !eventFilters ) {
	eventFilters = new QObjectList;
	CHECK_PTR( eventFilters );
    }
    eventFilters->insert( 0, obj );
    connect( obj, SIGNAL(destroyed()), this, SLOT(cleanupEventFilter()) );
}

/*!
  Removes an event filter object \e obj from this object.
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
	disconnect( obj,  SIGNAL(destroyed()),
		    this, SLOT(cleanupEventFilter()) );
    }
}


/*****************************************************************************
  Signal connection management
 *****************************************************************************/

#if defined(CHECK_RANGE)

static bool check_signal_macro( const QObject *sender, const char *signal,
				const char *func, const char *op )
{
    int sigcode = (int)(*signal) - '0';
    if ( sigcode != SIGNAL_CODE ) {
	if ( sigcode == SLOT_CODE )
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
    if ( code != SLOT_CODE && code != SIGNAL_CODE ) {
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
	case SLOT_CODE:	  type = "slot";   break;
	case SIGNAL_CODE: type = "signal"; break;
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
	QMetaData *rm = 0;
	switch ( code ) {
	    case SLOT_CODE:   rm = mo->slot( newname, TRUE );	  break;
	    case SIGNAL_CODE: rm = mo->signal( newname, TRUE ); break;
	}
	if ( rm ) {
	    qWarning("QObject::%s:  Candidate: %s", func, newname.data());
	}
    }
}


#endif // CHECK_RANGE


/*!
  \fn const QObject *QObject::sender()
  Returns a pointer to the object that sent the last signal received by
  this object.

  \warning
  This function violates the object-oriented principle of modularity,
  However, getting access to the sender might be practical when many
  signals are connected to a single slot. The sender is undefined if
  the slot is called as a normal C++ function.
*/

/*!
  \fn void QObject::connectNotify( const char *signal )

  This virtual function is called when something has been connected to
  \e signal in this object.

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
  \e signal in this object.

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

  Returns TRUE if the \e signal and the \e member arguments are compatible,
  otherwise FALSE.

  \warning
  We recommend that you do not reimplement this function but use the default
  implementation.

  \internal
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
    while ( *s1++ != '(' ) ;			// scan to first '('
    while ( *s2++ != '(' ) ;
    if ( *s2 == ')' || strcmp(s1,s2) == 0 )	// member has no args or
	return TRUE;				//   exact match
    int s1len = strlen(s1);
    int s2len = strlen(s2);
    if ( s2len < s1len && strncmp(s1,s2,s2len-1)==0 && s1[s2len-1]==',' )
	return TRUE;				// member has less args
    return FALSE;
}


/* tmake ignore Q_OBJECT */
/* tmake ignore Q_OBJECT */

/*!
  Internal function, called from initMetaObject(). Used to emit a warning
  when a class containing the macro Q_OBJECT inherits from a class that
  does not contain it.
*/

void QObject::badSuperclassWarning( const char *className,
				    const char *superclassName )
{
#if defined(CHECK_NULL)
    qWarning(
    "%s::initMetaObject(): Warning:\n"
    "    The class \"%s\" contains the Q_OBJECT macro, but inherits from the\n"
    "    \"%s\" class, which does not contain the Q_OBJECT macro.\n"
    "    Signal/slot behavior is undefined.\n",
    className, className,
    superclassName );
#else
    Q_UNUSED( className )
    Q_UNUSED( superclassName )
#endif
}

/*!
  \overload bool QObject::connect( const QObject *sender, const char *signal, const char *member ) const

  Connects \e signal from the \e sender object to \e member in this object.

  Equivalent to: <code>QObject::connect(sender, signal, this, member)</code>.

  \sa disconnect()
*/

/*!
  Connects \e signal from the \e sender object to \e member in object \e
  receiver.

  You must use the SIGNAL() and SLOT() macros when specifying the \e signal
  and the \e member.

  Example:
  \code
    QLabel     *label  = new QLabel;
    QScrollBar *scroll = new QScrollBar;
    QObject::connect( scroll, SIGNAL(valueChanged(int)),
		      label,  SLOT(setNum(int)) );
  \endcode

  This example connects the scroll bar's \link QScrollBar::valueChanged()
  valueChanged()\endlink signal to the label's \link QLabel::setNum()
  setNum()\endlink slot. It makes the label always display the current
  scroll bar value.

  A signal can even be connected to another signal, i.e. \e member is
  a SIGNAL().

  \code
    class MyWidget : public QWidget
    {
    public:
	MyWidget();
    ...
    signals:
	void aSignal();
    ...
    private:
    ...
	QPushButton *aButton;
    };

    MyWidget::MyWidget()
    {
	aButton = new QPushButton( this );
	connect( aButton, SIGNAL(clicked()), SIGNAL(aSignal()) );
    }
  \endcode

  In its constructor, MyWidget creates a private button and connects the
  \link QButton::clicked() clicked()\endlink signal to relay clicked() to
  the outside world. You can achieve the same effect by connecting the
  clicked() signal to a private slot and emitting aSignal() in this slot,
  but that takes a few lines of extra code and is not quite as clear, of
  course.

  A signal can be connected to many slots/signals. Many signals can be
  connected to one slot.

  If a signal is connected to several slots, the slots are activated
  in arbitrary order when the signal is emitted.

  \sa disconnect()
*/

bool QObject::connect( const QObject *sender,	const char *signal,
		       const QObject *receiver, const char *member )
{
#if defined(CHECK_NULL)
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

    QMetaObject *smeta = sender->queryMetaObject();
    if ( !smeta )				// no meta object
	return FALSE;

#if defined(CHECK_RANGE)
    if ( !check_signal_macro( sender, signal, "connect", "bind" ) )
	return FALSE;
#endif
    signal++;					// skip member type code
    QMetaData *sm;
    if ( !(sm=smeta->signal(signal,TRUE)) ) {	// no such signal
#if defined(CHECK_RANGE)
	err_member_notfound( SIGNAL_CODE, sender, signal, "connect" );
	err_info_about_candidates( SIGNAL_CODE, smeta, signal, "connect" );
	err_info_about_objects( "connect", sender, receiver );
#endif
	return FALSE;
    }
    signal = sm->name;				// use name from meta object

    int membcode = member[0] - '0';		// get member code

    QObject *s = (QObject *)sender;		// we need to change them
    QObject *r = (QObject *)receiver;		//   internally

#if defined(CHECK_RANGE)
    if ( !check_member_code( membcode, r, member, "connect" ) )
	return FALSE;
#endif
    member++;					// skip code
    QMetaData   *rm = 0;
    QMetaObject *rmeta = r->queryMetaObject();
    if ( !rmeta )				// no meta object
	return FALSE;
    switch ( membcode ) {			// get receiver member
	case SLOT_CODE:	  rm = rmeta->slot( member, TRUE );   break;
	case SIGNAL_CODE: rm = rmeta->signal( member, TRUE ); break;
    }
    if ( !rm ) {				// no such member
#if defined(CHECK_RANGE)
	err_member_notfound( membcode, r, member, "connect" );
	err_info_about_candidates( membcode, rmeta, member, "connect" );
	err_info_about_objects( "connect", sender, receiver );
#endif
	return FALSE;
    }
#if defined(CHECK_RANGE)
    if ( !s->checkConnectArgs(signal,receiver,member) )
	qWarning( "QObject::connect: Incompatible sender/receiver arguments"
		 "\n\t%s::%s --> %s::%s",
		 s->className(), signal,
		 r->className(), member );
#endif
    if ( !s->connections ) {			// create connections dict
	s->connections = new QSignalDict( 7, TRUE, FALSE );
	CHECK_PTR( s->connections );
	s->connections->setAutoDelete( TRUE );
    }
    QConnectionList *clist = s->connections->find( signal );
    if ( !clist ) {				// create receiver list
	clist = new QConnectionList;
	CHECK_PTR( clist );
	clist->setAutoDelete( TRUE );
	s->connections->insert( signal, clist );
    }
    QConnection *c = new QConnection(r, rm->ptr, rm->name);
    CHECK_PTR( c );
    clist->append( c );
    if ( !r->senderObjects ) {			// create list of senders
	r->senderObjects = new QObjectList;
	CHECK_PTR( r->senderObjects );
    }
    r->senderObjects->append( s );		// add sender to list
    s->connectNotify( signal_name );
    return TRUE;
}


/*!
  \overload bool QObject::disconnect( const char *signal, const QObject *receiver, const char *member )

  Disconnects \e signal from \e member of \e receiver.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.
*/

/*!
  \overload bool QObject::disconnect( const QObject *receiver, const char *member )

  Disconnects all signals in this object from \e member of \e receiver.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.
*/

/*!
  Disconnects \e signal in object \e sender from \e member in object \e
  receiver.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.

  disconnect() is typically used in three ways, as the following examples
  show.

  <ol>
  <li> Disconnect everything connected to an object's signals:
  \code
    disconnect( myObject );
  \endcode
  <li> Disconnect everything connected to a signal:
  \code
    disconnect( myObject, SIGNAL(mySignal()) );
  \endcode
  <li> Disconnect a specific receiver.
  \code
    disconnect( myObject, 0, myReceiver, 0 );
  \endcode
  </ol>

  0 may be used as a wildcard in three of the four arguments, meaning
  "any signal", "any receiving object" or "any slot in the receiving
  object" respectively.

  The \e sender has no default and may never be 0.  (You cannot
  disconnect signals from more than one object.)

  If \e signal is 0, it disconnects \e receiver and \e member from any
  signal.  If not, only the specified signal is disconnected.

  If \e receiver is 0, it disconnects anything connected to \e signal.
  If not, slots in objects other than \e receiver are not disconnected.

  If \e member is 0, it disconnects anything that is connected to \e
  receiver.  If not, only slots named \e member will be disconnected,
  and all other slots are left alone.  The \e member must be 0 if \e
  receiver is left out, so you cannot disconnect a specifically-named
  slot on all objects.

  \sa connect()
*/

bool QObject::disconnect( const QObject *sender,   const char *signal,
			  const QObject *receiver, const char *member )
{
#if defined(CHECK_NULL)
    if ( sender == 0 || (receiver == 0 && member != 0) ) {
	qWarning( "QObject::disconnect: Unexpected null parameter" );
	return FALSE;
    }
#endif
    if ( !sender->connections )			// no connected signals
	return FALSE;
    QCString signal_name;
    QCString member_name;
    QMetaData *rm = 0;
    QObject *s = (QObject *)sender;
    QObject *r = (QObject *)receiver;
    if ( member ) {
	member_name = qt_rmWS( member );
	member = member_name.data();
	int membcode = member[0] - '0';
#if defined(CHECK_RANGE)
	if ( !check_member_code( membcode, r, member, "disconnect" ) )
	    return FALSE;
#endif
	member++;
	QMetaObject *rmeta = r->queryMetaObject();
	if ( !rmeta )				// no meta object
	    return FALSE;
	switch ( membcode ) {			// get receiver member
	    case SLOT_CODE:   rm = rmeta->slot( member, TRUE );	  break;
	    case SIGNAL_CODE: rm = rmeta->signal( member, TRUE ); break;
	}
	if ( !rm ) {				// no such member
#if defined(CHECK_RANGE)
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
	QSignalDictIt it(*(s->connections));
	while ( (clist=it.current()) ) {	// for all signals...
	    // Tricky hack to avoid UTF conversion.
	    const char *curkey = it.currentKey();
	    ++it;
	    c = clist->first();
	    while ( c ) {			// for all receivers...
		if ( r == 0 ) {			// remove all receivers
		    removeObjFromList( c->object()->senderObjects, s );
		    c = clist->next();
		} else if ( r == c->object() &&
			    (member == 0 ||
			     strcmp(member,c->memberName()) == 0) ) {
		    removeObjFromList( c->object()->senderObjects, s );
		    clist->remove();
		    c = clist->current();
		} else {
		    c = clist->next();
		}
	    }
	    if ( r == 0 )			// disconnect all receivers
		s->connections->remove( curkey );
	}
	s->disconnectNotify( 0 );
    }

    else {					// specific signal
	signal_name = qt_rmWS( signal );
	signal = signal_name.data();
#if defined(CHECK_RANGE)
	if ( !check_signal_macro( s, signal, "disconnect", "unbind" ) )
	    return FALSE;
#endif
	signal++;
	clist = s->connections->find( signal );
	if ( !clist ) {
#if defined(CHECK_RANGE)
	    QMetaObject *smeta = s->queryMetaObject();
	    if ( !smeta )			// no meta object
		return FALSE;
	    if ( !smeta->signal(signal,TRUE) )
		qWarning( "QObject::disconnect: No such signal %s::%s",
			 s->className(), signal );
#endif
	    return FALSE;
	}
	c = clist->first();
	while ( c ) {				// for all receivers...
	    if ( r == 0 ) {			// remove all receivers
		removeObjFromList( c->object()->senderObjects, s, TRUE );
		c = clist->next();
	    } else if ( r == c->object() && (member == 0 ||
				      strcmp(member,c->memberName()) == 0) ) {
		removeObjFromList( c->object()->senderObjects, s, TRUE );
		clist->remove();
		c = clist->current();
	    } else {
		c = clist->next();
	    }
	}
	if ( r == 0 )				// disconnect all receivers
	    s->connections->remove( signal );
	s->disconnectNotify( signal_name );
    }
    return TRUE;
}


/*!
  This signal is emitted immediately before the object is destroyed.

  All the objects's children are destroyed immediately after this signal
  is emitted.
*/

void QObject::destroyed()
{
    activate_signal( "destroyed()" );
}


/*!
  This slot is connected to the destroyed() signal of other objects
  that have installed event filters on this object. When the other
  object is destroyed, we want to remove its event filter.
*/

void QObject::cleanupEventFilter()
{
    removeEventFilter( sender() );
}


/*!
  \internal
  Returns the meta object for this object. If necessary, calls
  initMetaObject().
  \sa metaObject()
*/

QMetaObject *QObject::queryMetaObject() const
{
    register QObject *x = (QObject *)this;	// fake const
    QMetaObject *m = x->metaObject();
    if ( !m ) {					// not meta object
	x->initMetaObject();			//   then try to create it
	m = x->metaObject();
    }
#if defined(CHECK_NULL)
    if ( !m )					// still no meta object: error
	qWarning( "QObject: Object %s::%s has no meta object",
		 x->className(), x->name( "unnamed" ) );
#endif
    return m;
}

/*!
  Returns a translated version of \a text, or \a text if there is
  no appropriate translated version.  All QObject subclasses which use the
  \link metaobjects.html Q_OBJECT macro\endlink have an overridden
  version of this.

  \sa QApplication::translate()
*/

QString QObject::tr( const char *text )
{
    if ( qApp )
	return qApp->translate( "QObject", text );
    else
	return QString::fromLatin1(text);
}

/*!
  Initializes the \link metaObject() meta object\endlink of this
  object. This method is automatically executed on demand from the
  QObject constructor.
  \sa metaObject()
*/
void QObject::initMetaObject()
{
    staticMetaObject();
}


#ifdef QT_BUILDER
/*!
  The functionality of initMetaObject(), provided as a static function.
*/
QMetaObject* QObject::createMetaObject()
{
    if ( metaObj )
	return metaObj;

    typedef void(QObject::*m1_t0)();
    m1_t0 v1_0 = &QObject::cleanupEventFilter;
    QMetaData *slot_tbl = new QMetaData[1];
    slot_tbl[0].name = "cleanupEventFilter()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    typedef void(QObject::*m2_t0)();
    m2_t0 v2_0 = &QObject::destroyed;
    QMetaData *signal_tbl = new QMetaData[1];
    signal_tbl[0].name = "destroyed()";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    QMetaProperty *props_tbl = new QMetaProperty[1];
    typedef const char*(QObject::*m3_t0)()const;
    typedef void(QObject::*m3_t1)(const char*);
    m3_t0 v3_0 = &QObject::name;
    m3_t1 v3_1 = &QObject::setName;
    props_tbl[0].name = "name";
    props_tbl[0].get = *((QMember*)&v3_0);
    props_tbl[0].set = *((QMember*)&v3_1);
    props_tbl[0].readonly = FALSE;
    props_tbl[0].type = "QString";
    props_tbl[0].enumType = 0;
    props_tbl[0].getSpec = '!';
    props_tbl[0].setSpec = '!';
    metaObj = new QMetaObject( "QObject", "",
	slot_tbl, 1,
	signal_tbl, 1,
	props_tbl, 1,
	0, 0,
	0, 0 );
    return metaObj;
}

// ## To disappear in Qt 3.0
void QObject::staticMetaObject()
{
    createMetaObject();
}

#else

// ## To disappear in Qt 3.0
/*!
  The functionality of initMetaObject(), provided as a static function.
*/
void QObject::staticMetaObject()
{
    if ( metaObj )
	return;

    typedef void(QObject::*m1_t0)();
    m1_t0 v1_0 = &QObject::cleanupEventFilter;
    QMetaData *slot_tbl = new QMetaData[1];
    slot_tbl[0].name = "cleanupEventFilter()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    typedef void(QObject::*m2_t0)();
    m2_t0 v2_0 = &QObject::destroyed;
    QMetaData *signal_tbl = new QMetaData[1];
    signal_tbl[0].name = "destroyed()";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    metaObj = new QMetaObject( "QObject", "",
	slot_tbl, 1,
	signal_tbl, 1 );
}
#endif // QT_BUILDER

/*!
  \internal

  Signal activation with the most frequently used parameter/argument
  types.  All other combinations are generated by the meta object
  compiler.
*/
void QObject::activate_signal( const char *signal )
{
    if ( !connections )
	return;
    QConnectionList *clist = connections->find( signal );
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT)();
    typedef RT *PRT;
    RT r;
    QConnectionListIt it(*clist);
    register QConnection *c;
    register QObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = c->object();
	object->sigSender = this;
	r = *((PRT)(c->member()));
	(object->*r)();
    }
}

/*!
  \overload void QObject::activate_signal( const char *signal, short )
*/

/*!
  \overload void QObject::activate_signal( const char *signal, int )
*/

/*!
  \overload void QObject::activate_signal( const char *signal, long )
*/

/*!
  \overload void QObject::activate_signal( const char *signal, const char * )
*/


#define ACTIVATE_SIGNAL_WITH_PARAM(FNAME,TYPE)				      \
void QObject::FNAME( const char *signal, TYPE param )			      \
{									      \
    if ( !connections )							      \
	return;								      \
    QConnectionList *clist = connections->find( signal );		      \
    if ( !clist || signalsBlocked() )					      \
	return;								      \
    typedef void (QObject::*RT0)();					      \
    typedef RT0 *PRT0;							      \
    typedef void (QObject::*RT1)( TYPE );				      \
    typedef RT1 *PRT1;							      \
    RT0 r0;								      \
    RT1 r1;								      \
    QConnectionListIt it(*clist);					      \
    register QConnection *c;						      \
    register QObject *object;						      \
    while ( (c=it.current()) ) {					      \
	++it;								      \
	object = c->object();						      \
	object->sigSender = this;					      \
	if ( c->numArgs() ) {						      \
	    r1 = *((PRT1)(c->member()));				      \
	    (object->*r1)( param );					      \
	} else {							      \
	    r0 = *((PRT0)(c->member()));				      \
	    (object->*r0)();						      \
	}								      \
    }									      \
}

// We don't want to duplicate too much text so...

ACTIVATE_SIGNAL_WITH_PARAM( activate_signal, short )
ACTIVATE_SIGNAL_WITH_PARAM( activate_signal, int )
ACTIVATE_SIGNAL_WITH_PARAM( activate_signal, long )
ACTIVATE_SIGNAL_WITH_PARAM( activate_signal, const char * )
ACTIVATE_SIGNAL_WITH_PARAM( activate_signal_bool, bool )
ACTIVATE_SIGNAL_WITH_PARAM( activate_signal_string, QString )
ACTIVATE_SIGNAL_WITH_PARAM( activate_signal_strref, const QString & )

/*!
  \fn void QObject::activate_signal_bool (const char * signal, bool)
  \internal
*/
/*!
  \fn void QObject::activate_signal_string (const char * signal, QString)
  \internal
*/
/*!
  \fn void QObject::activate_signal_strref (const char * signal, const QString & )
  \internal
*/


/*****************************************************************************
  QObject debugging output routines.
 *****************************************************************************/

static void dumpRecursive( int level, QObject *object )
{
#if defined(DEBUG)
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
		QString t;
		t.sprintf( "<%d,%d,%d,%d>",
			   w->x(), w->y(), w->width(), w->height() );
		flags += t;
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

  This function is useful for debugging.
*/

void QObject::dumpObjectTree()
{
    dumpRecursive( 0, this );
}

/*!
  Dumps information about signal connections etc. for this object to the
  debug output.

  This function is useful for debugging.
*/

void QObject::dumpObjectInfo()
{
#if defined(DEBUG)
    qDebug( "OBJECT %s::%s", className(), name( "unnamed" ) );
    qDebug( "  SIGNALS OUT" );
    int n = 0;
    if ( connections ) {
	QSignalDictIt it(*connections);
	QConnectionList *clist;
	while ( (clist=it.current()) ) {
	    qDebug( "\t%s", it.currentKey() );
	    n++;
	    ++it;
	    register QConnection *c;
	    QConnectionListIt cit(*clist);
	    while ( (c=cit.current()) ) {
		++cit;
		qDebug( "\t  --> %s::%s %s", c->object()->className(),
		       c->object()->name( "unnamed" ), c->memberName() );
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


#ifdef QT_BUILDER

bool QObject::setProperty( const char *_name, const QVariant& _value )
{
  if ( _value.isEmpty() )
    return TRUE;

  typedef void (QObject::*Proto3)( const char* );

  typedef void (QObject::*ProtoString)( QString );
  typedef void (QObject::*RProtoString)( const QString&);

  typedef void (QObject::*ProtoInt)( int );
  typedef void (QObject::*RProtoInt)( const int& );

  typedef void (QObject::*ProtoDouble)( double );
  typedef void (QObject::*RProtoDouble)( const double& );

  typedef void (QObject::*ProtoBool)( bool );
  typedef void (QObject::*RProtoBool)( const bool& );

  typedef void (QObject::*ProtoFont)( QFont );
  typedef void (QObject::*RProtoFont)( const QFont& );

  typedef void (QObject::*ProtoMovie)( QMovie );
  typedef void (QObject::*RProtoMovie)( const QMovie& );

  typedef void (QObject::*ProtoPixmap)( QPixmap );
  typedef void (QObject::*RProtoPixmap)( const QPixmap& );

  typedef void (QObject::*ProtoBrush)( QBrush );
  typedef void (QObject::*RProtoBrush)( const QBrush& );

  typedef void (QObject::*ProtoRect)( QRect );
  typedef void (QObject::*RProtoRect)( const QRect& );

  typedef void (QObject::*ProtoSize)( QSize );
  typedef void (QObject::*RProtoSize)( const QSize& );

  typedef void (QObject::*ProtoColor)( QColor );
  typedef void (QObject::*RProtoColor)( const QColor& );

  typedef void (QObject::*ProtoPalette)( QPalette );
  typedef void (QObject::*RProtoPalette)( const QPalette& );

  typedef void (QObject::*ProtoColorGroup)( QColorGroup );
  typedef void (QObject::*RProtoColorGroup)( const QColorGroup& );

  typedef void (QObject::*ProtoIconSet)( QIconSet );
  typedef void (QObject::*RProtoIconSet)( const QIconSet& );

  QMetaProperty* p = queryMetaObject()->property( _name, TRUE );
  if ( !p )
  {
    debug("Attribute %s::%s does not exist", className(), _name );
    return FALSE;
  }

  if ( p->enumType )
  {
    if ( _value.type() != QVariant::String )
      return FALSE;
    for( int i = 0; i < p->enumType->nEnumerators; ++i )
    {
      if ( _value.stringValue() == (const char*)p->enumType->enumerators[i].name )
      {
	ProtoInt m;
	m = *((ProtoInt*)&p->set);
	(this->*m)( p->enumType->enumerators[i].value );
	return TRUE;
      }
    }
    return FALSE;
  }

  if ( _value.typeName() != p->type )
  {
    qDebug("Trying to set attribute %s with wrong type", _name );
    return FALSE;
  }


  if ( _value.type() == QVariant::String )
  {
    if ( strcmp( _name, "name" ) == 0 )
    {
      Proto3 m;
      m = *((Proto3*)&p->set);
      (this->*m)( _value.stringValue() );
    }
    else if ( p->setSpec == 'c' )
    {
      ProtoString m;
      m = *((ProtoString*)&p->set);
      (this->*m)( _value.stringValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoString m;
      m = *((RProtoString*)&p->set);
      (this->*m)( _value.stringValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::Font )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoFont m;
      m = *((ProtoFont*)&p->set);
      (this->*m)( _value.fontValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoFont m;
      m = *((RProtoFont*)&p->set);
      (this->*m)( _value.fontValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  /* else if ( _value.type() == QVariant::Movie )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoMovie m;
      m = *((ProtoMovie*)&p->set);
      (this->*m)( _value.movieValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoMovie m;
      m = *((RProtoMovie*)&p->set);
      (this->*m)( _value.movieValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
    } */
  else if ( _value.type() == QVariant::Pixmap )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoPixmap m;
      m = *((ProtoPixmap*)&p->set);
      (this->*m)( _value.pixmapValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoPixmap m;
      m = *((RProtoPixmap*)&p->set);
      (this->*m)( _value.pixmapValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::Brush )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoBrush m;
      m = *((ProtoBrush*)&p->set);
      (this->*m)( _value.brushValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoBrush m;
      m = *((RProtoBrush*)&p->set);
      (this->*m)( _value.brushValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::Rect )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoRect m;
      m = *((ProtoRect*)&p->set);
      (this->*m)( _value.rectValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoRect m;
      m = *((RProtoRect*)&p->set);
      (this->*m)( _value.rectValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::Size )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoSize m;
      m = *((ProtoSize*)&p->set);
      (this->*m)( _value.sizeValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoSize m;
      m = *((RProtoSize*)&p->set);
      (this->*m)( _value.sizeValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::Color )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoColor m;
      m = *((ProtoColor*)&p->set);
      (this->*m)( _value.colorValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoColor m;
      m = *((RProtoColor*)&p->set);
      (this->*m)( _value.colorValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::Palette )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoPalette m;
      m = *((ProtoPalette*)&p->set);
      (this->*m)( _value.paletteValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoPalette m;
      m = *((RProtoPalette*)&p->set);
      (this->*m)( _value.paletteValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::ColorGroup )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoColorGroup m;
      m = *((ProtoColorGroup*)&p->set);
      (this->*m)( _value.colorgroupValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoColorGroup m;
      m = *((RProtoColorGroup*)&p->set);
      (this->*m)( _value.colorgroupValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::IconSet )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoIconSet m;
      m = *((ProtoIconSet*)&p->set);
      (this->*m)( _value.iconsetValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoIconSet m;
      m = *((RProtoIconSet*)&p->set);
      (this->*m)( _value.iconsetValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::Int )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoInt m;
      m = *((ProtoInt*)&p->set);
      (this->*m)( _value.intValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoInt m;
      m = *((RProtoInt*)&p->set);
      (this->*m)( _value.intValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::Double )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoDouble m;
      m = *((ProtoDouble*)&p->set);
      (this->*m)( _value.doubleValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoDouble m;
      m = *((RProtoDouble*)&p->set);
      (this->*m)( _value.doubleValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( _value.type() == QVariant::Bool )
  {
    if ( p->setSpec == 'c' )
    {
      ProtoBool m;
      m = *((ProtoBool*)&p->set);
      (this->*m)( _value.boolValue() );
    }
    else if ( p->setSpec == 'r' )
    {
      RProtoBool m;
      m = *((RProtoBool*)&p->set);
      (this->*m)( _value.boolValue() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }

  return FALSE;
}

/*!
  Reserved for future developments.
*/
bool QObject::property( const char *_name, QVariant* _value ) const
{
  typedef QString (QObject::*ProtoString)() const;
  typedef const QString* (QObject::*PProtoString)() const;
  typedef const QString& (QObject::*RProtoString)() const;
  typedef const char* (QObject::*Proto3)(const char*) const;

  typedef int (QObject::*ProtoInt)() const;
  typedef const int* (QObject::*PProtoInt)() const;
  typedef const int& (QObject::*RProtoInt)() const;

  typedef double (QObject::*ProtoDouble)() const;
  typedef const double* (QObject::*PProtoDouble)() const;
  typedef const double& (QObject::*RProtoDouble)() const;

  typedef bool (QObject::*ProtoBool)() const;
  typedef const bool* (QObject::*PProtoBool)() const;
  typedef const bool& (QObject::*RProtoBool)() const;

  typedef QFont (QObject::*ProtoFont)() const;
  typedef const QFont* (QObject::*PProtoFont)() const;
  typedef const QFont& (QObject::*RProtoFont)() const;

  typedef QMovie (QObject::*ProtoMovie)() const;
  typedef const QMovie* (QObject::*PProtoMovie)() const;
  typedef const QMovie& (QObject::*RProtoMovie)() const;

  typedef QPixmap (QObject::*ProtoPixmap)() const;
  typedef const QPixmap* (QObject::*PProtoPixmap)() const;
  typedef const QPixmap& (QObject::*RProtoPixmap)() const;

  typedef QBrush (QObject::*ProtoBrush)() const;
  typedef const QBrush* (QObject::*PProtoBrush)() const;
  typedef const QBrush& (QObject::*RProtoBrush)() const;

  typedef QRect (QObject::*ProtoRect)() const;
  typedef const QRect* (QObject::*PProtoRect)() const;
  typedef const QRect& (QObject::*RProtoRect)() const;

  typedef QSize (QObject::*ProtoSize)() const;
  typedef const QSize* (QObject::*PProtoSize)() const;
  typedef const QSize& (QObject::*RProtoSize)() const;

  typedef QColor (QObject::*ProtoColor)() const;
  typedef const QColor* (QObject::*PProtoColor)() const;
  typedef const QColor& (QObject::*RProtoColor)() const;

  typedef QPalette (QObject::*ProtoPalette)() const;
  typedef const QPalette* (QObject::*PProtoPalette)() const;
  typedef const QPalette& (QObject::*RProtoPalette)() const;

  typedef QColorGroup (QObject::*ProtoColorGroup)() const;
  typedef const QColorGroup* (QObject::*PProtoColorGroup)() const;
  typedef const QColorGroup& (QObject::*RProtoColorGroup)() const;

  typedef QIconSet (QObject::*ProtoIconSet)() const;
  typedef const QIconSet* (QObject::*PProtoIconSet)() const;
  typedef const QIconSet& (QObject::*RProtoIconSet)() const;

  QMetaProperty* p = queryMetaObject()->property( _name, TRUE );
  if ( !p )
    return FALSE;

  if ( p->enumType )
  {
    ProtoInt m;
    m = *((ProtoInt*)&p->get);
    int x = (int) (this->*m)();
    for( int i = 0; i < p->enumType->nEnumerators; ++i )
      if ( x == p->enumType->enumerators[i].value )
      {
	_value->setValue( p->enumType->enumerators[i].name );
	return TRUE;
      }
    return FALSE;
  }
  else if ( strcmp( p->type, "QString" ) == 0 )
  {
    if ( strcmp( _name, "name" ) == 0 )
    {
      Proto3 m;
      m = *((Proto3*)&p->get);
      _value->setValue( QString( (this->*m)( "unnamed" ) ) );
    }
    else if ( p->getSpec == 'c' )
    {
      ProtoString m;
      m = *((ProtoString*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoString m;
      m = *((RProtoString*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoString m;
      m = *((PProtoString*)&p->get);
      const QString* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QString() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "QFont" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoFont m;
      m = *((ProtoFont*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoFont m;
      m = *((RProtoFont*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoFont m;
      m = *((PProtoFont*)&p->get);
      const QFont* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QFont() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  /* else if ( strcmp( p->type, "QMovie" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoMovie m;
      m = *((ProtoMovie*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoMovie m;
      m = *((RProtoMovie*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoMovie m;
      m = *((PProtoMovie*)&p->get);
      const QMovie* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QMovie() );
    }
    else
      ASSERT( 0 );
    return TRUE;
    } */
  else if ( strcmp( p->type, "QPixmap" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoPixmap m;
      m = *((ProtoPixmap*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoPixmap m;
      m = *((RProtoPixmap*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoPixmap m;
      m = *((PProtoPixmap*)&p->get);
      const QPixmap* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QPixmap() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "QBrush" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoBrush m;
      m = *((ProtoBrush*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoBrush m;
      m = *((RProtoBrush*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoBrush m;
      m = *((PProtoBrush*)&p->get);
      const QBrush* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QBrush() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "QRect" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoRect m;
      m = *((ProtoRect*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoRect m;
      m = *((RProtoRect*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoRect m;
      m = *((PProtoRect*)&p->get);
      const QRect* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QRect() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "QSize" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoSize m;
      m = *((ProtoSize*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoSize m;
      m = *((RProtoSize*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoSize m;
      m = *((PProtoSize*)&p->get);
      const QSize* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QSize() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "QColor" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoColor m;
      m = *((ProtoColor*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoColor m;
      m = *((RProtoColor*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoColor m;
      m = *((PProtoColor*)&p->get);
      const QColor* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QColor() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "QPalette" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoPalette m;
      m = *((ProtoPalette*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoPalette m;
      m = *((RProtoPalette*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoPalette m;
      m = *((PProtoPalette*)&p->get);
      const QPalette* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QPalette() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "QColorGroup" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoColorGroup m;
      m = *((ProtoColorGroup*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoColorGroup m;
      m = *((RProtoColorGroup*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoColorGroup m;
      m = *((PProtoColorGroup*)&p->get);
      const QColorGroup* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QColorGroup() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "QIconSet" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoIconSet m;
      m = *((ProtoIconSet*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoIconSet m;
      m = *((RProtoIconSet*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoIconSet m;
      m = *((PProtoIconSet*)&p->get);
      const QIconSet* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( QIconSet() );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "int" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoInt m;
      m = *((ProtoInt*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoInt m;
      m = *((RProtoInt*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoInt m;
      m = *((PProtoInt*)&p->get);
      const int *p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( 0 );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "double" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoDouble m;
      m = *((ProtoDouble*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoDouble m;
      m = *((RProtoDouble*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoDouble m;
      m = *((PProtoDouble*)&p->get);
      const double* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( 0.0 );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }
  else if ( strcmp( p->type, "bool" ) == 0 )
  {
    if ( p->getSpec == 'c' )
    {
      ProtoBool m;
      m = *((ProtoBool*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'r' )
    {
      RProtoBool m;
      m = *((RProtoBool*)&p->get);
      _value->setValue( (this->*m)() );
    }
    else if ( p->getSpec == 'p' )
    {
      PProtoBool m;
      m = *((PProtoBool*)&p->get);
      const bool* p = (this->*m)();
      if ( p )
	_value->setValue( *p );
      else
	_value->setValue( (bool)FALSE );
    }
    else
      ASSERT( 0 );
    return TRUE;
  }

  return FALSE;
}

/*!
  The configure function configures the object with the QDom tree \e element.
  Configuration means that properties specified in the QDom tree are set and
  child widgets might be generated from the QDom tree. The specific behaviour
  depends on the widgets implementation of the configureEvent() method.
  
  configure() just sends a QConfigureEvent to itself. This event triggers in turn
  the configureEvent() method of this object. So if your widget must understand special
  XML tags then you have to overloade configureEvent() and not configure().
  
  If the object is derived from QLayout, then you must call the QLayout::configure()
  method which takes a widget as an additional argument. Calling the QObject::configure()
  method of a QLayout directly would only set its properties but it would not create child
  widgets of the layout if any are described in the QDom tree.
*/
bool QObject::configure( const QDomElement& element )
{
    QDomElement e = element;
    QConfigureEvent ev( &e );
    QApplication::sendEvent( this, &ev );

    return ev.isAccepted();
}

/*!
  This function is triggered by the QConfigureEvent. It sets all
  properties of the object which are described in the QDom tree passed by
  the event.
  
  If your widget must understand special XML tags then you have to overload
  this method, but make shure that you call your super classes configureEvent()
  method unless you have good reasons for not calling it.
  
  If you want to configure an object, call configure(). This will send a QConfigureEvent
  event which triggers this function.
  
  If you intend to implement configuration of a class derived from QLayout, then you
  must overload QLayout::configure() which features a QWidget as an additional second parameter.
*/
void QObject::configureEvent( QConfigureEvent* ev )
{
    const QDomElement* element = ev->element();
    
    // Need a meta object to set teh properties
    QMetaObject* m = queryMetaObject();
    if ( !m )
    {
	ev->ignore();
	return;
    }
    
    // Process all known properties
    QStringList props = m->propertyNames();
    QStringList::Iterator it = props.begin();
    for( ; it != props.end(); ++it )
    {
	QMetaProperty* p = queryMetaObject()->property( *it, TRUE );
	if ( p && !p->readonly )
	    setProperty( p, *element );
    }

    QDomElement e = element->firstChild().toElement();
    for( ; !e.isNull(); e = e.nextSibling().toElement() )
    {
	if ( e.tagName() == "Connection" )
        {
	    QString signal = e.attribute( "signal" ).prepend("2");
	    QString slot;
	    if ( e.hasAttribute( "slot" ) )
		slot = e.attribute("slot").prepend("1");
	    else if ( e.hasAttribute( "transmitter" ) )
		slot = e.attribute("transmitter").prepend("2");
	    else
	    {
		ev->ignore();
		return;
	    }
	    
	    QString tmp = e.attribute( "sender" );
	    QObject *sender;
	    if ( tmp == name() )
		sender = this;
	    else if ( !tmp.isEmpty() )
		sender = child( tmp );
	    else
		sender = this;
	    if ( !sender )
		qDebug("Did not find sender object %s\n", tmp.ascii());

	    tmp = e.attribute( "receiver" );
	    QObject *receiver;
	    if ( tmp == name() )
		receiver = this;
	    else if ( !tmp.isEmpty() )
		receiver = child( tmp );
	    else
		receiver = this;
	    if ( !receiver )
		qDebug("Did not find receiver object %s\n", tmp.ascii());
	    
	    if ( !sender || !receiver )
	    {
		ev->ignore();
		return;
	    }
	    
	    connect( sender, signal, receiver, slot );
	}
    }
}

bool QObject::setProperty( const QMetaProperty* p, const QDomElement& element )
{
    QString name( p->name );

    QVariant::Type type = QVariant::String;
    if ( !p->enumType )
	type = QVariant::nameToType( p->type );

    QVariant prop = element.property( name, type );
    return setProperty( name, prop );
}

#if 0
QDomElement QObject::configuration( QDomDocument& doc, bool properties ) const
{
    QDomElement e = doc.createElement( className() );

    if ( properties )
    {
	QMetaObject* m = metaObject();
	if ( m )
	{
	    QStringList props = m->propertyNames();

	    QStringList::Iterator it = props.begin();
	    for( ; it != props.end(); ++it )
	    {
		QMetaProperty* p = m->property( *it, TRUE );
		QVariant prop;
		if ( property( p->name, &prop ) )
		{
		    // Save enums as attributes
		    if ( p->enumType )
			e.setAttribute( p->name, prop.stringValue() );
		    else
			e.setProperty( p->name, prop );
		}
	    }
	}
    }
    else
    {
	QString x;
	x.setNum( (long)this );
	e.setAttribute( "__ptr", x );
    }

    return e;
}
#endif 0

#endif // QT_BUILDER
