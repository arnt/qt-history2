/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.cpp#7 $
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
#include "qview.h"
#include <ctype.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qobject.cpp#7 $";
#endif


// Timer functions, implemented in qapp_xxx.cpp

int   qStartTimer( long interval, QObject *obj );
bool  qKillTimer( int id );
bool  qKillTimer( QObject *obj );


declare(QDictM,QConnection);			// dictionary of connections

QMetaObject *QObject::metaObj = 0;

QObject::QObject( QObject *parent, const char *name )
{
    if ( !objectDict )				// will create object dict
	initMetaObject();
    parentObj = parent;				// set parent
    if ( name )
	objname = name;				// set object name
    sender = 0;					// no sender yet
    connections = 0;				// no connections yet
    childObjects = 0;				// no children yet
    isWidget = FALSE;				// assume not a widget
    if ( parentObj )				// add object to parent
	parentObj->insertChild( this );
    hasTimer = FALSE;				// has no timer yet
}

QObject::~QObject()
{
    if ( hasTimer )				// might be pending timers
	qKillTimer( (QObject *)this );
    if ( parentObj )				// remove it from parent object
	parentObj->removeChild( this );
    delete connections;				// delete all connections
    if ( childObjects ) {
	register QObject *obj = childObjects->first();
	while ( obj ) {				// delete all child objects
	    obj->parentObj = 0;			// object should not remove
	    delete obj;				//   itself from the list
	    obj = childObjects->next();
	}
	delete childObjects;
    }
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
    blockSig = b ? TRUE : FALSE;
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
	warning( "QObject::insertChild: Duplicate %ss in list",
		 obj->className() );
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


bool QObject::connect( QObject *sigobj, const char *signal,
		       const QObject *obj, const char *member )
{						// connect signal to method
    char *s=strdup(signal), *m=strdup(member);
    rmWS( s, signal );
    rmWS( m, member );
    bool result = sigobj->bind( s, obj, m );
    delete s;
    delete m;
    return result;
}

bool QObject::disconnect( QObject *obj, const char *signal )
{
    char *s = strdup(signal);
    rmWS( s, signal );
    bool result = obj->unbind( s );
    delete s;
    return result;
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
	fatal( "QObject::bind: No meta object for %s (WHY?)", className() );
#else
	return FALSE;
#endif
    }
#if defined(CHECK_RANGE)
    int sigcode = (int)(*signal) - '0';
    if ( sigcode != SIGNAL_CODE ) {
	if ( sigcode == METHOD_CODE || sigcode == SLOT_CODE )
	    warning( "QObject::bind: Attempt to bind non-signal '%s::%s'",
		     className(), signal+1 );
	else
	    warning( "QObject::bind: Use the SIGNAL() macro to bind '%s::%s'",
		     className(), signal );
	return FALSE;
    }
#endif
    signal++;					// skip member type code
    if ( meta->signal( signal, TRUE ) == 0 ) {	// no such find signal
#if defined(CHECK_RANGE)
	if ( strchr(signal,')') == 0 )		// was common typing mistake
	    warning( "QObject::bind: Parentheses expected for signal '%s::%s'",
		     className(), signal );
	else
	    warning( "QObject::bind: No such signal '%s::%s'",
		     className(), signal );
#endif
	return FALSE;
    }

    int memberCode = member[0] - '0';		// get member code
    member++;					// skip code
    QObject *r = object ?			// set receiver object
		 (QObject *)object : parentObj;
    QMember *m = 0;
    QMetaObject *rmeta = r->metaObject();
    if ( !rmeta ) {				// receiver has no meta object
	r->initMetaObject();			// then create it
	rmeta = r->metaObject();
    }
    if ( !rmeta ) {
#if defined(CHECK_NULL)
	fatal( "QObject::bind: No meta object for %s (WHY?)", r->className() );
#else
	return FALSE;
#endif
    }
    switch ( memberCode ) {			// get receiver member
	case METHOD_CODE: m = rmeta->method( member, TRUE ); break;
	case SLOT_CODE:	  m = rmeta->slot( member, TRUE );   break;
	case SIGNAL_CODE: m = rmeta->signal( member, TRUE ); break;
    }
    if ( !m ) {					// no such member
#if defined(CHECK_RANGE)
	char *memberType;
	switch ( memberCode ) {			// set member type string
	    case METHOD_CODE: memberType = "method"; break;
	    case SLOT_CODE:   memberType = "slot";   break;
	    case SIGNAL_CODE: memberType = "signal"; break;
	    default:	      memberType = "(unspecified)";
	}
	if ( strchr(member,')') == 0 )		// was common typing mistake
	    warning( "QObject::bind: Parentheses expected for %s '%s::%s'",
		     memberType, r->className(), member );
	else
	    warning( "QObject::bind: No such %s: '%s::%s'", memberType,
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
		     "\n\t'%s::%s' --> '%s::%s'", className(), signal,
		     r->className(), member );
	    return FALSE;
	}
    }
#endif
    if ( !connections ) {
	connections = new QConnections;		// create connections dict
	connections->setAutoDelete( TRUE );
    }
    QConnection *c = connections->find( signal );
    if ( c )					// override last connection
	c->connect( r, *m );
    else					// create new connection
	connections->insert( signal, new QConnection( r, *m ) );
    return TRUE;
}

bool QObject::unbind( const char *signal )	// disconnect from signal
{
    if ( !connections )
	return FALSE;
    return connections->remove( signal );
}


const char *QObject::className() const		// get name of class
{
    return "QObject";
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
