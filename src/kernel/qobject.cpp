/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.cpp#59 $
**
** Implementation of QObject class
**
** Author  : Haavard Nord
** Created : 930418
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qobject.h"
#include "qobjcoll.h"
#include "qregexp.h"
#include <ctype.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qobject.cpp#59 $";
#endif


/*----------------------------------------------------------------------------
  \class QObject qobject.h
  \brief The QObject class is the base class of all Qt objects that can
  deal with signals, slots and events.

  Qt provides a very powerful mechanism for seamless object
  communication; \link metaobjects.html signal/slot
  connections. \endlink The signal/slot mechanism is an advanced way
  of making traditional callback routines.

  Example:
  \code
    //
    // MyObject must inherit QObject to use signals and slots.
    //

    class MyObject : public QObject
    {
	Q_OBJECT				// required for signals/slots
    public:
	MyObject( QObject *parent=0, const char *name );
    signals:
	void	minuteTimeout();
    private slots:
	void	secondTimeout();
    private:
	QTimer *timer;
	int	secs;
    };

    //
    // Initializes MyObject and sets up a timer.
    // The QTimer::timeout() signal is emitted at a specified millisecond
    // interval (here: 1 second).
    // This timeout signal is connected to the internal secondTimeout()
    // slot of MyObject.
    //

    MyObject::MyObject( QObject *parent=0, const char *name )
	: QObject( parent, name )
    {
	secs = 0;				// 0 seconds so far
	timer = new QTimer;
	connect( timer, SIGNAL(timeout()), SLOT(secondTimeout()) );
	timer->start( 1000 );			// start 1 second timer
    }

    //
    // secondTimeout() is activated by the QTimer::timeout() signal
    //

    void MyObject::secondTimeout()
    {
	debug( "Timer activation" );
	if ( ++secs == 60 ) {			// 60 seconds elapsed
	    secs = 0;				// reset counter, and
	    emit minuteTimeout();		// emit a new signal
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

  All Qt widgets inherit QObject and they make good use of signals and
  slots.  A QScrollBar, for example, emits \link
  QScrollBar::valueChanged() valueChanged()\endlink whenever the
  scroll bar value changes.

  Meta objects are useful for doing more than connecting signals to slots.
  They also allow the programmer to obtain information about the class to
  which an object is instantiated from (see isA() and inherits()) or to
  produce a list of child objects that inherit a particular class
  (see queryList()).
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  \class QSenderObject qobject.h
  \brief Internal object used for sending signals.

  \internal
 ----------------------------------------------------------------------------*/


//
// Remove white space from SIGNAL and SLOT names.
// Internal for QObject::connect() and QObject::disconnect()
//

static QString rmWS( const char *src )
{
    QString tmp( strlen( src ) + 1 );
    register char *d = tmp.data();
    register char *s = (char *)src;
    while( *s && isspace(*s) )
	s++;
    while ( *s ) {
	while( *s && !isspace(*s) )
	    *d++ = *s++;
	while( *s && isspace(*s) )
	    s++;
	if ( *s && (isalpha(*s) || *s == '_') )
	    *d++ = ' ';
    }
    tmp.truncate( d - tmp.data() );
    return tmp;
}

//
// Checks whether two functions have compatible arguments.
// Returns TRUE if the arguments are compatible, otherwise FALSE.
// Internal for QObject::connect()
//
// TRUE:	"signal(<anything>)",	"member()"
// TRUE:	"signal(a,b,c)",	"member(a,b,c)"
// TRUE:	"signal(a,b,c)",	"member(a,b)", "member(a)" etc.
// FALSE:	"signal(const a)",	"member(a)"
// FALSE:	"signal(a)",		"member(const a)"
// FALSE:	"signal(a)",		"member(b)"
// FALSE:	"signal(a)",		"member(a,b)"
//

static bool checkCompatArgs( const char *signal, const char *member )
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


// Event functions, implemented in qapp_xxx.cpp

int   qStartTimer( long interval, QObject *obj );
bool  qKillTimer( int id );
bool  qKillTimer( QObject *obj );

void  qRemovePostedEvents( QObject * );


declare(QListM,QConnection);			// dictionary of connections
declare(QListIteratorM,QConnection);		// dictionary iterator
declare(QDictM,QListM(QConnection));
declare(QDictIteratorM,QListM(QConnection));

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


/*****************************************************************************
  QObject member functions
 *****************************************************************************/

/*----------------------------------------------------------------------------
  Constructs an object with parent objects \e parent and a \e name.

  The parent of an object may be viewed as the object's owner. For
  instance, a \link QDialog dialog box\endlink is the parent of the
  "ok" and "cancel" buttons inside it.

  The destructor of a parent object destroys all child objects.

  Setting \e parent to 0 constructs an object with no parent.
  If the object is a widget, it will become a top-level window.

  \sa parent(), name()
 ----------------------------------------------------------------------------*/

QObject::QObject( QObject *parent, const char *name )
{
    if ( !objectDict )				// will create object dict
	initMetaObject();
    objname = name ? qstrdup(name) : 0;		// set object name
    parentObj = parent;				// set parent
    childObjects = 0;				// no children yet
    connections = 0;				// no connections yet
    senderObjects = 0;				// no signals connected yet
    eventFilters = 0;				// no filters installed
    sigSender = 0;				// no sender yet
    isSignal   = FALSE;				// assume not a signal object
    isWidget   = FALSE;				// assume not a widget object
    hiPriority = FALSE;				// normal priority
    pendTimer  = FALSE;				// no timers yet
    pendEvent  = FALSE;				// no events yet
    blockSig   = FALSE;				// not blocking signals
    if ( parentObj )				// add object to parent
	parentObj->insertChild( this );
}

/*----------------------------------------------------------------------------
  Destroys the object and all its children objects.

  All signals to and from the object are automatically disconnected.

  \warning \e All child objects are deleted.  If any of these objects
  are on the stack or global, your program will dump core.  If you have
  a pointer to a child object and try to use it later, you will lose.
  ----------------------------------------------------------------------------*/

QObject::~QObject()
{
    if ( objname )
	delete objname;
    if ( pendTimer )				// might be pending timers
	qKillTimer( this );
    if ( pendEvent )				// pending posted events
	qRemovePostedEvents( this );
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
    }
    if ( childObjects ) {			// delete children objects
	obj = childObjects->first();
	while ( obj ) {				// delete all child objects
	    obj->parentObj = 0;			// object should not remove
	    delete obj;				//   itself from the list
	    obj = childObjects->next();
	}
	delete childObjects;
    }
    delete eventFilters;
}


/*----------------------------------------------------------------------------
  \fn QMetaObject *QObject::metaObject() const
  Returns a pointer to the meta object of this object.

  A meta object contains information about a class that inherits QObject:
  class name, super class name, signals and slots. Every class that contains
  the \c Q_OBJECT macro will also have a meta object.

  The meta object information is required by the signal/slot connection
  mechanism.  The functions isA() and inherits() also make use of the
  meta object.

  The meta object is created by the initMetaObject() function, which is
  generated by the meta object compiler and called from the class constructor.

  \warning If this function returns 0, the constructor probably forgot
  to call initMetaObject().
  ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Returns the class name of this object.

  This function is generated by the \link metaobjects.html Meta Object
  Compiler. \endlink

  \warning This function will return an invalid name if the class
  constructor did not call initMetaObject() or if the class definition
  lacks the \c Q_OBJECT macro.

  \sa name()
 ----------------------------------------------------------------------------*/

const char *QObject::className() const
{
    return "QObject";
}


/*----------------------------------------------------------------------------
  Returns TRUE if this object is an instance of a specified class,
  otherwise FALSE.

  Example:
  \code
    QTimer *t = new QTimer;		// QTimer inherits QObject
    t->isA("QTimer");			// returns TRUE
    t->isA("QObject");			// returns FALSE
  \endcode

  \sa inherits(), metaObject()
 ----------------------------------------------------------------------------*/

bool QObject::isA( const char *clname ) const	// test if is-a class
{
    return strcmp(className(),clname) == 0;
}

/*----------------------------------------------------------------------------
  Returns TRUE if this object is an instance of a class that inherits
  \e clname.  (A class is considered to inherit itself.)

  Example:
  \code
    QTimer *t = new QTimer;		// QTimer inherits QObject
    t->inherits("QTimer");		// returns TRUE
    t->inherits("QObject");		// returns TRUE
    t->inherits("QButton");		// returns FALSE
  \endcode

  \sa isA(), metaObject()
 ----------------------------------------------------------------------------*/

bool QObject::inherits( const char *clname ) const
{						// test if inherits class
    QMetaObject *meta = queryMetaObject();
    while ( meta ) {
	if ( strcmp(clname,meta->className()) == 0 )
	    return TRUE;
	meta = meta->superClass();
    }
    return FALSE;
}


/*----------------------------------------------------------------------------
  \fn const char *QObject::name() const
  Returns the name of this object.

  The object name is set by the constructor or by the setName() function.
  The object name is not very useful in the current version of Qt, but
  will become increasingly important in the future.

  The queryList() function searches the object tree for objects that
  matches a particular object name.

  \sa setName(), className()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Sets the name of this object to \e name.

  The object name is not very useful in the current version of Qt, but
  it will become increasingly important in the future.

  The queryList() function searches the object tree for objects that
  matches a particular object name.

  \sa name()
 ----------------------------------------------------------------------------*/

void QObject::setName( const char *name )
{
    if ( objname )
	delete objname;
    objname = name ? qstrdup(name) : 0;
}

/*----------------------------------------------------------------------------
  \fn bool QObject::isWidgetType() const
  Returns TRUE if the object is a widget, or FALSE if not.

  Calling this function is equivalent to calling inherits("QWidget"),
  except that it is much faster.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn bool QObject::highPriority() const
  Returns TRUE if the object is a high priority object, or FALSE if it is a
  standard priority object.

  High priority objects are placed first in list of children,
  on the assumption that they will be referenced very often.

  Currently only QAccel objects are high priority.
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
  This virtual function receives events to an object and must
  return TRUE if the event was recognized and processed.

  The event() function can be reimplemented to customize the behavior
  of an object.
  \sa QWidget::event(), installEventFilter()
 ----------------------------------------------------------------------------*/

bool QObject::event( QEvent *e )		// receive event
{
    return activate_filters( e );
}

/*----------------------------------------------------------------------------
  Filters events if this object has been installed as an event filter for
  another object.

  The reimplementation of this virtual function must return TRUE if the
  event should be stopped, or FALSE if the event should be dispatched normally.

  \sa installEventFilter()
 ----------------------------------------------------------------------------*/

bool QObject::eventFilter( QObject *, QEvent * )
{
    return FALSE;				// don't do anything with it
}


bool QObject::activate_filters( QEvent *e )	// activate event filters
{
    if ( !eventFilters )			// no event filter
	return FALSE;
    QObjectListIt it( *eventFilters );
    register QObject *obj = it.current();
    while ( obj ) {				// send to all filters
	++it;					//   until one returns TRUE
	if ( obj->eventFilter(this,e) )
	    return TRUE;
	obj = it.current();
    }
    return FALSE;				// don't do anything with it
}

/*----------------------------------------------------------------------------
  \fn bool QObject::signalsBlocked() const
  Returns TRUE if signals are blocked, or FALSE if signals are not blocked.

  Signals are not blocked by default.
  \sa blockSignals()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Blocks signals if \e block is TRUE, or unblocks signals if \e block is FALSE.

  Emitted signals disappear into hyperspace if signals are blocked.
 ----------------------------------------------------------------------------*/

void QObject::blockSignals( bool block )
{
    blockSig = block;
}


//
// The timer flag hasTimer is set when startTimer is called.
// It is not reset when killing the timer because more than
// one timer might be active.
//

/*----------------------------------------------------------------------------
  Starts a timer and returns a timer identifier.

  A timer event will occur every \e interval milliseconds until killTimer()
  or killTimers() is called.
  If \e interval is 0, then timer event occurs as often as possible.

  The virtual event() function is called with the QTimerEvent event parameter
  class when a timer event occurs.
  Widgets dispatch timer events to the QWidget::timerEvent() event handler.
  Reimplement this virtual function to get timer events if your object is
  a widget.  If your object is not a widget, you must reimplement the event()
  function to get timer events.

  The QTimer class provides a high-level programming interface with one-shot
  timers and timer signals instead of events.

  \sa QTimerEvent, killTimer(), killTimers(), QWidget::timerEvent()
 ----------------------------------------------------------------------------*/

int QObject::startTimer( long interval )
{
    pendTimer = TRUE;				// set timer flag
    return qStartTimer( interval, (QObject *)this );
}

/*----------------------------------------------------------------------------
  Kills the timer with the identifier \e id.

  The timer identifer is returned by startTimer() when a
  timer event is started.

  \sa startTimer(), killTimers(), QWidget::timerEvent()
 ----------------------------------------------------------------------------*/

void QObject::killTimer( int id )		// kill timer events
{
    qKillTimer( id );
}

/*----------------------------------------------------------------------------
  Kills all timers that this object has started.

  \sa killTimer(), startTimer(), QWidget::timerEvent()
 ----------------------------------------------------------------------------*/

void QObject::killTimers()			// kill all timers for object
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
		ok = strcmp(objName,obj->name()) == 0;
	    else if ( rx )
		ok = rx->match(obj->name()) >= 0;
	}
	if ( ok )				// match!
	    result->append( obj );
	if ( recurse && obj->children() )
	    objSearch( result, (QObjectList *)obj->children(), inheritsClass,
		       objName, rx, recurse );
	obj = list->next();
    }
}


/*----------------------------------------------------------------------------
  \fn QObject *QObject::parent() const
  Returns a pointer to the parent object.
  \sa children()
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn const QObjectList *QObject::children() const
  Returns a list of child objects.

  This function is not for the inexperienced.

  \sa queryList(), parent(), insertChild(), removeChild()
 ----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------
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
    QObjectList	 *list = myWidget->queryList( "QButton" );
    QObjectListIt it( *list );			// iterate over the buttons
    QFont	  newFont( "Courier", 24 );
    while ( it.current() ) {
	it.current()->setFont( newFont );
	++it;
    }
    delete list;				// delete the search results
  \endcode

  The QObjectList class is defined in the qobjcoll.h header file.

  \warning
  Throw the list away as soon you have finished using it.
  You can get in serious trouble if you for instance try to access
  an object that has been deleted.
 ----------------------------------------------------------------------------*/

QObjectList *QObject::queryList( const char *inheritsClass,
				 const char *objName,
				 bool regexpMatch,
				 bool recursiveSearch )
{
    QObjectList *result = new QObjectList;

    if ( regexpMatch && objName ) {		// regexp matching
	QRegExp rx = objName;
	objSearch( result, (QObjectList *)children(), inheritsClass,
		   0, &rx, recursiveSearch );
    }
    else {
	objSearch( result, (QObjectList *)children(), inheritsClass,
		   objName, 0, recursiveSearch );
    }
    return result;
}


/*----------------------------------------------------------------------------
  Returns a list of objects/slot pairs that are connected to the
  signal, or 0 if nothing is connected to it.

  This function is for internal use.
 ----------------------------------------------------------------------------*/

QConnectionList *QObject::receivers( const char *signal ) const
{						// get receiver
    if ( connections && signal ) {
	if ( *signal == '2' ) {
	    QString s = rmWS( signal+1 );
	    return connections->find( s );
	}
	else
	    return connections->find( signal );
    }
    return 0;
}


/*----------------------------------------------------------------------------
  Inserts an object \e obj into the list of child objects.

  \warning
  This function cannot be used to make a widget a child widget of
  another.  Child widgets can only be created by setting the parent
  widget in the constructor.
 ----------------------------------------------------------------------------*/

void QObject::insertChild( QObject *obj )	// add object object
{
    if ( !childObjects ) {
	childObjects = new QObjectList;
	CHECK_PTR( childObjects );
    }
#if defined(CHECK_STATE)
    else if ( childObjects->findRef(obj) >= 0 ) {
	warning( "QObject::insertChild: Object %s::%s already in list",
		 obj->className(), obj->name() );
	return;
    }
#endif
    obj->parentObj = this;
    if ( obj->hiPriority )
	childObjects->insert( 0, obj );		// high priority inserts
    else
	childObjects->append( obj );		// normal priority appends
}

/*----------------------------------------------------------------------------
  Removes the child object \e obj from the list of children.

  \warning
  This function will not remove a child widget from the screen.
  It will only remove it from the parent widget's list of children.
 ----------------------------------------------------------------------------*/

void QObject::removeChild( QObject *obj )	// remove child object
{
    if ( childObjects && childObjects->findRef(obj) >= 0 ) {
	childObjects->remove();			// remove object from list
	if ( childObjects->isEmpty() ) {	// list becomes empty
	    delete childObjects;
	    childObjects = 0;			// reset children list
	}
    }
}

/*----------------------------------------------------------------------------
  Adds an event filter object for this object.

  An event filter is another object that receives all events that are
  sent to this object via the eventFilter() function.
  The event filter returns TRUE if the event should be stopped, or
  FALSE if the event can be dispatched normally.

  \sa removeEventFilter(), eventFilter(), event()
 ----------------------------------------------------------------------------*/

void QObject::installEventFilter( const QObject *obj )
{						// add event filter object
    if ( !eventFilters ) {
	eventFilters = new QObjectList;
	CHECK_PTR( eventFilters );
    }
    eventFilters->insert( 0, obj );
}

/*----------------------------------------------------------------------------
  Removes an event filter object from this object.
  \sa installEventFilter(), eventFilter(), event()
 ----------------------------------------------------------------------------*/

void QObject::removeEventFilter( const QObject *obj )
{						// remove event filter object
    if ( eventFilters && eventFilters->findRef(obj) >= 0 ) {
	eventFilters->remove();			// remove object from list
	if ( eventFilters->isEmpty() ) {
	    delete eventFilters;
	    eventFilters = 0;			// reset event filter list
	}
    }
}


/*****************************************************************************
  Signal connection management
 *****************************************************************************/

#if defined(CHECK_RANGE)

static bool check_signal_macro( QObject *sender, const char *signal,
				const char *func, const char *op )
{
    int sigcode = (int)(*signal) - '0';
    if ( sigcode != SIGNAL_CODE ) {
	if ( sigcode == SLOT_CODE )
	    warning( "QObject::%s: Attempt to %s non-signal %s::%s",
		     func, op, sender->className(), signal+1 );
	else
	    warning( "QObject::%s: Use the SIGNAL macro to %s %s::%s",
		     func, op, sender->className(), signal );
	return FALSE;
    }
    return TRUE;
}

static bool check_member_code( int code, QObject *object, const char *member,
			       const char *func )
{
    if ( code != SLOT_CODE && code != SIGNAL_CODE ) {
	warning( "QObject::%s: Use the SLOT or SIGNAL macro to "
		 "%s %s::%s", func, func, object->className(), member );
	return FALSE;
    }
    return TRUE;
}

static void err_member_notfound( int code, QObject *object, const char *member,
				 const char *func )
{
    const char *type = 0;
    switch ( code ) {
	case SLOT_CODE:	  type = "slot";   break;
	case SIGNAL_CODE: type = "signal"; break;
    }
    if ( strchr(member,')') == 0 )		// common typing mistake
	warning( "QObject::%s: Parentheses expected, %s %s::%s",
		 func, type, object->className(), member );
    else
	warning( "QObject::%s: No such %s %s::%s",
		 func, type, object->className(), member );
}

#endif // CHECK_RANGE


/*----------------------------------------------------------------------------
  \fn bool QObject::connect( QObject *sender, const char *signal, const char *member ) const

  Connects \e signal from object \e sender to \e member in this
  object.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \fn QObject *QObject::sender()
  Returns a pointer to the object that sent the last signal received by
  this object.

  Getting access to the sender might be very practical when lots
  of signals are connected to a single slot, however, it violates
  the object-oriented principle of modularity.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Connects \e signal from object \e sender to \e member in object \e
  receiver.  \sa disconnect()
  ----------------------------------------------------------------------------*/

bool QObject::connect( QObject *sender,		const char *signal,
		       const QObject *receiver, const char *member )
{
#if defined(CHECK_NULL)
    if ( sender == 0 || receiver == 0 || signal == 0 || member == 0 ) {
	warning( "QObject::connect: Unexpected NULL parameter" );
	return FALSE;
    }
#endif
    QString signal_tmp( strlen(signal)+1 );
    QString member_tmp( strlen(member)+1 );
    signal_tmp = rmWS( signal );		// strip white space
    signal = signal_tmp;
    member_tmp = rmWS( member );
    member = member_tmp;

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
#endif
	return FALSE;
    }
    signal = sm->name;				// use name from meta object

    int membcode = member[0] - '0';		// get member code
    QObject *r = (QObject *)receiver;		// set receiver object
#if defined(CHECK_RANGE)
    if ( !check_member_code( membcode, r, member, "connect" ) )
	return FALSE;
#endif
    member++;					// skip code
    QMetaData *rm = 0;
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
#endif
	return FALSE;
    }
#if defined(CHECK_RANGE)
    if ( !checkCompatArgs(signal,member) )
	warning( "QObject::connect: Incompatible sender/receiver arguments"
		 "\n\t%s::%s --> %s::%s",
		 sender->className(), signal,
		 r->className(), member );
#endif
    if ( !sender->connections ) {		// create connections dict
	sender->connections = new QSignalDict( 7, TRUE, FALSE );
	CHECK_PTR( sender->connections );
	sender->connections->setAutoDelete( TRUE );
    }
    QConnectionList *clist = sender->connections->find( signal );
    if ( !clist ) {				// create receiver list
	clist = new QConnectionList;
	CHECK_PTR( clist );
	clist->setAutoDelete( TRUE );
	sender->connections->insert( signal, clist );
    }
    clist->append( new QConnection( r, rm->ptr, rm->name ) );
    if ( !r->senderObjects ) {			// create list of senders
	r->senderObjects = new QObjectList;
	CHECK_PTR( r->senderObjects );
    }
    r->senderObjects->append( sender );		// add sender to list
    return TRUE;
}


/*----------------------------------------------------------------------------
  \overload bool QObject::disconnect( const char *signal, const QObject *receiver, const char *member )

  Disconnects \e signal from \e member of \e receiver.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.
  ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  \overload bool QObject::disconnect( const QObject *receiver, const char *member )

  Disconnects all signals in this object from \e member of \e receiver.

  A signal-slot connection is removed when either of the objects
  involved are destroyed.
 ----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
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
 ----------------------------------------------------------------------------*/

bool QObject::disconnect( QObject *sender, const char *signal,
			  const QObject *receiver, const char *member )
{
#if defined(CHECK_NULL)
    if ( sender == 0 || (receiver == 0 && member != 0) ) {
	warning( "QObject::disconnect: Unexpected null parameter" );
	return FALSE;
    }
#endif
    if ( !sender->connections )			// no connected signals
	return FALSE;
    QString signal_tmp;
    QString member_tmp;
    QMetaData *rm = 0;
    QObject *r = (QObject *)receiver;
    if ( member ) {
	member_tmp.resize( strlen(member)+1 );
	member_tmp = rmWS( member );
	member = member_tmp.data();
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
#endif
	    return FALSE;
	}
    }

    QConnectionList *clist;
    register QConnection *c;
    if ( signal == 0 ) {			// any/all signals
	QSignalDictIt it(*(sender->connections));
	while ( (clist=it.current()) ) {	// for all signals...
	    const char *curkey = it.currentKey();
	    ++it;
	    c = clist->first();
	    while ( c ) {			// for all receivers...
		if ( r == 0 ) {			// remove all receivers
		    removeObjFromList( c->object()->senderObjects, sender );
		    c = clist->next();
		}
		else if ( r == c->object() && (member == 0 ||
					       strcmp(member,c->memberName()) == 0) ) {
		    removeObjFromList( c->object()->senderObjects, sender );
		    clist->remove();
		    c = clist->current();
		}
		else
		    c = clist->next();
	    }
	    if ( r == 0 )			// disconnect all receivers
		sender->connections->remove( curkey );
	}
    }

    else {					// specific signal
	signal_tmp.resize( strlen(signal)+1 );
	signal_tmp = rmWS( signal );
	signal = signal_tmp.data();
#if defined(CHECK_RANGE)
	if ( !check_signal_macro( sender, signal, "disconnect", "unbind" ) )
	    return FALSE;
#endif
	signal++;
	clist = sender->connections->find( signal );
	if ( !clist ) {
#if defined(CHECK_RANGE)
	    QMetaObject *smeta = sender->queryMetaObject();
	    if ( !smeta )			// no meta object
		return FALSE;
	    if ( !smeta->signal(signal,TRUE) )
		warning( "QObject::disconnect: No such signal %s::%s",
			 sender->className(), signal );
#endif
	    return FALSE;
	}
	c = clist->first();
	while ( c ) {				// for all receivers...
	    if ( r == 0 ) {			// remove all receivers
		removeObjFromList( c->object()->senderObjects, sender, TRUE );
		c = clist->next();
	    }
	    else if ( r == c->object() && (member == 0 ||
					   strcmp(member,c->memberName()) == 0) ) {
		removeObjFromList( c->object()->senderObjects, sender, TRUE );
		clist->remove();
		c = clist->current();
	    }
	    else
		c = clist->next();
	}
	if ( r == 0 )				// disconnect all receivers
	    sender->connections->remove( signal );
    }
    return TRUE;
}


/*----------------------------------------------------------------------------
  \internal
  Returns the meta object for this object. If there is no meta object,
  it calls initMetaObject() and makes another try.
  \sa metaObject()
 ----------------------------------------------------------------------------*/

QMetaObject *QObject::queryMetaObject() const	// get meta object
{
    register QObject *x = (QObject *)this;	// fake const
    QMetaObject *m = x->metaObject();
    if ( !m ) {					// not meta object
	x->initMetaObject();			//   then try to create it
	m = x->metaObject();
    }
#if defined(CHECK_NULL)
    if ( !m )					// still no meta object: error
	warning( "QObject: Object %s::%s has no meta object",
		 x->className(), x->name() );
#endif
    return m;
}


void QObject::initMetaObject()			// initialize meta object
{
    metaObj = new QMetaObject( "QObject", "", 0, 0, 0, 0 );
}


/*****************************************************************************
  Signal activation with the most frequently used parameter/argument types.
  All other combinations are generated by the meta object compiler.
 *****************************************************************************/

void QObject::activate_signal( const char *signal )
{
    if ( !connections )
	return;
    QConnectionList *clist = connections->find( signal );
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT)();
    typedef RT *PRT;
    QConnectionListIt it(*clist);
    RT r;
    register QConnection *c;
    register QObject *object;
    while ( (c=it.current()) ) {
	++it;
	r = *((PRT)(c->member()));
	object = c->object();
	object->sigSender = this;
	(object->*r)();
	object->sigSender = 0;
    }
}

#define ACTIVATE_SIGNAL_WITH_PARAM(TYPE)				      \
void QObject::activate_signal( const char *signal, TYPE param )		      \
{									      \
    if ( !connections )							      \
	return;								      \
    QConnectionList *clist = connections->find( signal );		      \
    if ( !clist || signalsBlocked() )					      \
	return;								      \
    typedef void (QObject::*RT)( TYPE );				      \
    typedef RT *PRT;							      \
    QConnectionListIt it(*clist);					      \
    RT r;								      \
    register QConnection *c;						      \
    register QObject *object;						      \
    while ( (c=it.current()) ) {					      \
	++it;								      \
	r = *((PRT)(c->member()));					      \
	object = c->object();						      \
	object->sigSender = this;					      \
	(object->*r)( param );						      \
	object->sigSender = 0;						      \
    }									      \
}

// We don't want to duplicate too much text so...

ACTIVATE_SIGNAL_WITH_PARAM( short )
ACTIVATE_SIGNAL_WITH_PARAM( int )
ACTIVATE_SIGNAL_WITH_PARAM( long )
ACTIVATE_SIGNAL_WITH_PARAM( const char * )


/*****************************************************************************
  QObject debugging output routines; to be removed before real version 1.0.
 *****************************************************************************/

static void dumpRecursive( int level, QObject *object )
{
#if defined(DEBUG)
    if ( object ) {
	QString buf;
	buf.fill( '\t', level );
	const char *name = object->name() ? object->name() : "????";
	debug( "%s%s::%s", (const char*)buf, object->className(), name );
	if ( object->children() ) {
	    QObjectListIt it(*object->children());
	    while ( it ) {
		dumpRecursive( level+1, it );
		++it;
	    }
	}
    }
#endif
}

/*----------------------------------------------------------------------------
  Dumps a tree of children to the debug device.	 Prints out
  all signal connections.
 ----------------------------------------------------------------------------*/

void QObject::dumpObjectTree()
{
    dumpRecursive( 0, this );
}

void QObject::dumpObjectInfo()
{
#if defined(DEBUG)
    debug( "OBJECT %s::%s", className(), name() );
    debug( "  SIGNALS OUT" );
    int n = 0;
    if ( connections ) {
	QSignalDictIt it(*connections);
	QConnectionList *clist;
	while ( (clist=it.current()) ) {
	    debug( "\t%s", it.currentKey() );
	    n++;
	    ++it;
	    register QConnection *c;
	    QConnectionListIt cit(*clist);
	    while ( (c=cit.current()) ) {
		++cit;
		debug( "\t  --> %s::%s %s", c->object()->className(),
		       c->object()->name(), c->memberName() );
	    }
	}
    }
    if ( n == 0 )
	debug( "\t<None>" );
    debug( "  SIGNALS IN" );
    n = 0;
    if ( senderObjects ) {
	QObject *sender = senderObjects->first();
	while ( sender ) {
	    debug( "\t%s::%s", sender->className(), sender->name() );
	    n++;
	    sender = senderObjects->next();
	}
    }
    if ( n == 0 )
	debug( "\t<None>" );
#endif
}

