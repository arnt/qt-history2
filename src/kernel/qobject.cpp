/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.cpp#17 $
**
** Implementation of QObject class
**
** Author  : Haavard Nord
** Created : 930418
**
** Copyright (C) 1993,1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qobject.h"
#include "qobjcoll.h"
#include <ctype.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qobject.cpp#17 $";
#endif


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


QObject::QObject( QObject *parent, const char *name )
{
    if ( !objectDict )				// will create object dict
	initMetaObject();
    if ( name )
	objname = name;				// set object name
    parentObj = parent;				// set parent
    childObjects = 0;				// no children yet
    connections = 0;				// no connections yet
    senderObjects = 0;				// no signals connected yet
    eventFilters = 0;				// no filters installed
    sigSender = 0;				// no sender yet
    isSignal = FALSE;				// assume not a signal object
    isWidget = FALSE;				// assume not a widget object
    pendTimer = FALSE;				// no timers yet
    pendEvent = FALSE;				// no events yet
    blockSig = FALSE;				// not blocking signals
    if ( parentObj )				// add object to parent
	parentObj->insertChild( this );
}

QObject::~QObject()
{
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


bool QObject::isA( const char *clname ) const	// test if is-a class
{
    return strcmp(className(),clname) == 0;
}

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


const char *QObject::className() const		// get name of class
{
    return "QObject";
}


void QObject::setName( const char *name )	// set object name
{
    objname = name;
}


bool QObject::event( QEvent *e )		// receive event
{
    return activate_filters( e );
}

bool QObject::eventFilter( QObject *, QEvent * )// filter event
{
    return FALSE;				// don't do anything with it
}


bool QObject::activate_filters( QEvent *e )	// activate event filters
{
    register QObject *obj = eventFilters ? eventFilters->first() : 0;
    bool stop = FALSE;
    while ( obj ) {				// send to all filters
	stop = obj->eventFilter( this, e );	//   until one returns TRUE
	if ( stop )
	    break;
	obj = eventFilters->next();
    }
    return stop;				// don't do anything with it
}


void QObject::blockSignals( bool b )
{
    blockSig = b;
}


//
// The timer flag hasTimer is set when startTimer is called.
// It is not reset when killing the timer because more than
// one timer might be active.
//
int QObject::startTimer( long interval )	// start timer events
{
    pendTimer = TRUE;				// set timer flag
    return qStartTimer( interval, (QObject *)this );
}

void QObject::killTimer( int id )		// kill timer events
{
    qKillTimer( id );
}

void QObject::killTimers()			// kill all timers for object
{
    qKillTimer( this );
}


QConnectionList *QObject::receivers( const char *signal ) const
{						// get receiver
    return connections ? connections->find( signal ) : 0;
}


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
    childObjects->append( obj );
}

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


void QObject::installEventFilter( const QObject *obj )
{						// add event filter object
    if ( !eventFilters ) {
	eventFilters = new QObjectList;
	CHECK_PTR( eventFilters );
    }
    eventFilters->insert( obj );
}

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


// ---------------------------------------------------------------------------
// Signal connection management.
//

static char *rmWS( char *dest, const char *src )// remove white space
{
    register char *d = dest;
    register char *s = (char *)src;
    while ( *s ) {
	if ( !isspace(*s) )
	    *d++ = *s;
	s++;
    }
    *d = '\0';
    return dest;
}


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
	case SLOT_CODE:   type = "slot";   break;
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


bool QObject::connect( QObject *sender,         const char *signal,
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
    rmWS( signal_tmp.data(), signal );		// strip white space
    signal = signal_tmp;
    rmWS( member_tmp.data(), member );
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
    const char *s1 = signal;			// check if compatible args
    const char *s2 = member;
    while ( *s1++ != '(' ) ;
    while ( *s2++ != '(' ) ;
    if ( !(*s2 == ')' || strcmp(s1,s2) == 0) ) {
	int s1len = strlen(s1);
	int s2len = strlen(s2);
	if ( !(s2len < s1len && !strncmp(s1,s2,s2len-1) && s1[s2len-1]==',') )
	    warning( "QObject::connect: Incompatible sender/receiver arguments"
		     "\n\t%s::%s --> %s::%s",
		     sender->className(), signal,
		     r->className(), member );
    }
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


bool QObject::disconnect( QObject *sender, const char *signal,
			  const QObject *receiver, const char *member )
{
#if defined(CHECK_NULL)
    if ( sender == 0 || (receiver == 0 && member != 0) ) {
	warning( "QObject::disconnect: Unexpected NULL parameter" );
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
	rmWS( member_tmp.data(), member );
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
	    case SLOT_CODE:   rm = rmeta->slot( member, TRUE );   break;
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
	rmWS( signal_tmp.data(), signal );
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
		warning( "QObject::diconnect: No such signal %s::%s",
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


// ---------------------------------------------------------------------------
// Signal activation with the most frequently used parameter/argument types.
// All other combinations are generated by the meta object compiler.
//

void QObject::activate_signal( const char *signal )
{
    QConnectionList *clist;
    if ( !connections || signalsBlocked() ||
	 !(clist=connections->find(signal)) )
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
    }
}

#define ACTIVATE_SIGNAL_WITH_PARAM(TYPE)				      \
void QObject::activate_signal( const char *signal, TYPE param )		      \
{									      \
    QConnectionList *clist;						      \
    if ( !connections || signalsBlocked() ||				      \
	 !(clist=connections->find(signal)) )				      \
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
    }									      \
}

// We don't want to duplicate too much text...

ACTIVATE_SIGNAL_WITH_PARAM( short )
ACTIVATE_SIGNAL_WITH_PARAM( int )
ACTIVATE_SIGNAL_WITH_PARAM( long )
ACTIVATE_SIGNAL_WITH_PARAM( const char * )


// ---------------------------------------------------------------------------
// QObject debugging output routines; to be removed before real version 1.0.
//

static void dumpRecursive( int level, QObject *object )
{
#if defined(DEBUG)
    if ( object ) {
	QString buf;
	buf.fill( '\t', level );
	const char *name = object->name() ? object->name() : "????";
	debug( "%s%s::%s", (char*)buf, object->className(), name );
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
