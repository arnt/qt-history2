/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobject.cpp#3 $
**
** Implementation of QObject class
**
** Author  : Haavard Nord
** Created : 930418
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qobject.h"
#include "qobjcoll.h"
#include "qpart.h"
#include "qview.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qobject.cpp#3 $";
#endif


declare(QDictM,QConnection);			// dictionary of connections

QMetaObject *QObject::metaObj = 0;


QObject::QObject( QObject *parent )		// create object with parent
{
    if ( !objectDict )				// will create object dict
	initMetaObject();
    parentObj = parent;				// set parent
    sender = 0;					// no sender yet
    connections = 0;				// no connections yet
    isPType = FALSE;				// assume not a parent type
    isWidget = FALSE;				// assume not a widget
    if ( parentObj ) {				// add object to parent
	if ( parentObj->isWidget )
	    ((QView*)parentObj)->insertObject( this );
	else
	    ((QPart*)parentObj)->insertObject( this );
    }
    hasTimer = FALSE;				// has no timer yet
}

QObject::~QObject()
{
    if ( hasTimer )				// might be pending timers
	qKillTimer( (QObject *)this );
    if ( parentObj ) {				// remove it from parent object
	if ( parentObj->isWidget )
	    ((QView*)parentObj)->removeObject( this );
	else
	    ((QPart*)parentObj)->removeObject( this );
    }
    delete connections;				// delete all connections
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
	fatal( "QObject::bind: No meta object for %s", className() );
#else
	return FALSE;
#endif
    }
    signal++;					// skip member type code
    if ( meta->signal( signal, TRUE ) == 0 ) {	// no such find signal
#if defined(CHECK_RANGE)
	if ( strchr(signal,')') == 0 )		// was common typing mistake
	    warning( "QObject::bind: Missing parentheses for signal '%s'",
		     signal );
	else
	    warning( "QObject::bind: Invalid signal '%s'", signal );
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
	fatal( "QObject::bind: No meta object for %s", r->className() );
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
	    warning( "QObject::bind: Missing parentheses for %s '%s'",
		     memberType, member );
	else
	    warning( "QObject::bind: Invalid %s: '%s'", memberType, member );
#endif
	return FALSE;
    }
#if defined(CHECK_RANGE)
    const char *s1 = signal;			// check if compatible args
    const char *s2 = member;
    while ( *s1++ != '(' ) ;
    while ( *s2++ != '(' ) ;
    if ( strcmp(s1,s2) != 0 ) {			// arguments differ
	warning( "QObject::bind: Incompatible sender/receiver arguments" );
	return FALSE;
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


char *QObject::className() const		// get name of class
{
    return "QObject";
}

void QObject::initMetaObject()			// initialize meta object
{
    metaObj = new QMetaObject( "QObject", "", 0, 0, 0, 0, 0, 0 );
}
