/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.cpp#8 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qobject.cpp#8 $";
#endif


// Timer functions, implemented in qapp_xxx.cpp

int   qStartTimer( long interval, QObject *obj );
bool  qKillTimer( int id );
bool  qKillTimer( QObject *obj );


declare(QDictM,QConnection);			// dictionary of connections
declare(QDictIteratorM,QConnection);		// dictionary iterator

QMetaObject *QObject::metaObj = 0;


static void removeObjFromList( QObjectList *objList, const QObject *obj )
{
    int i = objList->findRef( obj );
    while ( i >= 0 ) {
	objList->remove();
	i = objList->findNextRef( obj );
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
    sigSender = 0;				// no sender yet
    isSignal = FALSE;				// assume not a signal object
    isWidget = FALSE;				// assume not a widget object
    hasTimer = FALSE;				// no timers yet
    blockSig = FALSE;				// not blocking signals
    if ( parentObj )				// add object to parent
	parentObj->insertChild( this );
}

QObject::~QObject()
{
    if ( hasTimer )				// might be pending timers
	qKillTimer( (QObject *)this );
    if ( parentObj )				// remove it from parent object
	parentObj->removeChild( this );
    register QObject *obj;
    if ( senderObjects ) {			// disconnect from senders
	obj = senderObjects->first();
	while ( obj ) {
	    obj->disconnect( this );
	    obj = senderObjects->next();
	}
	delete senderObjects;
	senderObjects = 0;			// in case of auto-connections
    }
    if ( connections ) {			// disconnect receivers
	QDictIteratorM(QConnection) it(*connections);
	QConnection *c;
	while ( (c=it.current()) ) {
	    if ( (obj=c->object()) && obj->senderObjects )
		removeObjFromList( obj->senderObjects, this );
	    ++it;
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
}


const char *QObject::className() const		// get name of class
{
    return "QObject";
}


void QObject::setName( const char *name )	// set object name
{
    objname = name;
}


bool QObject::event( QEvent * )			// receive event
{
    return FALSE;				// don't do anything with it
}


void QObject::blockSignals( bool b )
{
    blockSig = b;
}


// The timer flag, hasTimer, is set when startTimer is called.
// It is not reset when killing the timer because there might
// be multiple timers.

int QObject::startTimer( long interval )	// start timer events
{
    hasTimer = TRUE;				// set timer flag
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


QConnection *QObject::receiver( const char *signal ) const
{						// get receiver
    return connections ? connections->find( signal ) : 0;
}


void QObject::insertChild( QObject *obj )	// add object object
{
    if ( !childObjects ) {
	childObjects = new QObjectList;
	CHECK_PTR( childObjects );
    }
#if defined(CHECK_RANGE)
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
    if ( childObjects && childObjects->findRef(obj) )
	childObjects->remove();			// does not delete object
}



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


bool QObject::connect( QObject *sender, const char *signal,
		       const QObject *receiver, const char *member )
{						// connect signal to method
#if defined(CHECK_NULL)
    if ( sender == 0 || receiver == 0 || signal == 0 || member == 0 ) {
	warning( "QObject::connect: Unexpected NULL parameter" );
	return FALSE;
    }
#endif
    char *s=strdup(signal), *m=strdup(member);
    rmWS( s, signal );
    rmWS( m, member );
    bool result = sender->bind( s, receiver, m );
    if ( result ) {				// ok
	register QObject *x = (QObject *)receiver; // override const
	if ( !x->senderObjects ) {		// create list of senders
	    x->senderObjects = new QObjectList;
	    CHECK_PTR( x->senderObjects );
	}
	x->senderObjects->append( sender );	// add sender to list
    }
    delete s;
    delete m;
    return result;
}

bool QObject::disconnect( QObject *sender, const char *signal,
			  const QObject *receiver, const char *member )
{
#if defined(CHECK_NULL)
    if ( sender == 0 || receiver == 0 || (signal == 0 && member != 0) ) {
	warning( "QObject::disconnect: Unexpected NULL parameter" );
	return FALSE;
    }
#endif
    if ( !sender->connections )			// sender has no signals
	return FALSE;
    if ( signal == 0 && member == 0 ) {		// remove all signals to recv
	QDictIteratorM(QConnection) it(*sender->connections);	
	register QConnection *c;
	while ( (c=it.current()) ) {
	    if ( c->object() == receiver )
		sender->connections->remove( it.currentKey() );
	    else
		++it;
	}
    }
    else if ( member == 0 ) {			// remove signal to recv
	sender->connections->remove( signal );	// ONLY ONE RECEIVER!!!
    }
    else {
	sender->connections->remove( signal );	// ONLY ONE RECEIVER!!!
    }
    return TRUE;
}


bool QObject::bind( const char *signal, const QObject *object,
		    const char *member )
{
    QMetaObject *meta = metaObject();
    if ( !meta ) {				// has no meta object
	initMetaObject();
	meta = metaObject();
    }
    if ( !meta ) {				// has no meta object
#if defined(CHECK_NULL)
	warning( "QObject::bind: Object %s::%s has no meta object",
		 className(), name() );
#endif
	return FALSE;
    }
#if defined(CHECK_RANGE)
    int sigcode = (int)(*signal) - '0';
    if ( sigcode != SIGNAL_CODE ) {
	if ( sigcode == METHOD_CODE || sigcode == SLOT_CODE )
	    warning( "QObject::bind: Attempt to bind non-signal %s::%s",
		     className(), signal+1 );
	else
	    warning( "QObject::bind: Use the SIGNAL() macro to bind %s::%s",
		     className(), signal );
	return FALSE;
    }
#endif
    signal++;					// skip member type code
    if ( meta->signal( signal, TRUE ) == 0 ) {	// no such find signal
#if defined(CHECK_RANGE)
	if ( strchr(signal,')') == 0 )		// was common typing mistake
	    warning( "QObject::bind: Parentheses expected for signal %s::%s",
		     className(), signal );
	else
	    warning( "QObject::bind: No such signal %s::%s",
		     className(), signal );
#endif
	return FALSE;
    }

    int membcode = member[0] - '0';		// get member code
    QObject *r = object ?			// set receiver object
		 (QObject *)object : parentObj;
#if defined(CHECK_RANGE)
    if ( membcode != METHOD_CODE && membcode != SLOT_CODE &&
	 membcode != SIGNAL_CODE ) {
	warning( "QObject::bind: Use the SLOT/SIGNAL macro to connect %s::%s",
		 r->className(), member );
        return FALSE;
    }
#endif
    member++;					// skip code
    QMember *m = 0;
    QMetaObject *rmeta = r->metaObject();
    if ( !rmeta ) {				// receiver has no meta object
	r->initMetaObject();			// then create it
	rmeta = r->metaObject();
    }
    if ( !rmeta ) {
#if defined(CHECK_NULL)
	warning( "QObject::bind: Object %s::%s has no meta object",
		 r->className(), r->name() );
#endif
	return FALSE;
    }
    switch ( membcode ) {			// get receiver member
	case METHOD_CODE: m = rmeta->method( member, TRUE ); break;
	case SLOT_CODE:	  m = rmeta->slot( member, TRUE );   break;
	case SIGNAL_CODE: m = rmeta->signal( member, TRUE ); break;
    }
    if ( !m ) {					// no such member
#if defined(CHECK_RANGE)
	char *memberType = 0;
	switch ( membcode ) {			// set member type string
	    case METHOD_CODE: memberType = "method"; break;
	    case SLOT_CODE:   memberType = "slot";   break;
	    case SIGNAL_CODE: memberType = "signal"; break;
	}
	if ( strchr(member,')') == 0 )		// was common typing mistake
	    warning( "QObject::bind: Parentheses expected for %s %s::%s",
		     memberType, r->className(), member );
	else
	    warning( "QObject::bind: No such %s: %s::%s", memberType,
		     r->className(), member );
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
	if ( !(s2len < s1len && !strncmp(s1,s2,s2len-1) && s1[s2len-1]==',')) {
	    warning( "QObject::bind: Incompatible sender/receiver arguments"
		     "\n\t%s::%s --> %s::%s", className(), signal,
		     r->className(), member );
	    return FALSE;
	}
    }
#endif
    if ( !connections ) {
	connections = new QConnections;		// create connections dict
	CHECK_PTR( connections );
	connections->setAutoDelete( TRUE );
    }
    QConnection *c = connections->find( signal );
    if ( c )					// override last connection
	c->connect( r, *m );
    else					// create new connection
	connections->insert( signal, new QConnection( r, *m ) );
    return TRUE;
}


void QObject::initMetaObject()			// initialize meta object
{
    metaObj = new QMetaObject( "QObject", "", 0, 0, 0, 0, 0, 0 );
}


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
