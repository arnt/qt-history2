/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsignal.cpp#1 $
**
** Implementation of QSignal class
**
** Author  : Haavard Nord
** Created : 941201
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qsignal.h"
#include "qobjcoll.h"
#include <ctype.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qsignal.cpp#1 $";
#endif


declare(QListM,QConnection);			// list of connections
declare(QListIteratorM,QConnection);		// list iterator


QSignal::QSignal( const char *name ) : QObject( 0, name )
{
    isSignal = TRUE;
    connlist = 0;
}

QSignal::~QSignal()
{
    delete connlist;
}


const char *QSignal::className() const		// get class name
{
    return "QSignal";
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


bool QSignal::connect( const QObject *receiver, const char *memberName )
{						// connect signal to method
#if defined(CHECK_NULL)
    if ( receiver == 0 || memberName == 0 ) {
	warning( "QSignal::connect: Unexpected NULL parameter" );
	return FALSE;
    }
#endif
    QString memberStr = memberName;
    char *member = memberStr.data();
    rmWS( member, memberName );
    int membcode = member[0] - '0';		// get member code
    QObject *r = (QObject *)receiver;		// receiver object
#if defined(CHECK_RANGE)
    if ( membcode != METHOD_CODE && membcode != SLOT_CODE &&
	 membcode != SIGNAL_CODE ) {
	warning( "QSignal::connect: Use the SLOT/SIGNAL macro to connect %s::%s",
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
	warning( "QSignal::connect: Object %s::%s has no meta object",
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
	    warning( "QSignal::connect: Parentheses expected for %s %s::%s",
		     memberType, r->className(), member );
	else
	    warning( "QSignal::connect: No such %s: %s::%s", memberType,
		     r->className(), member );
#endif
	return FALSE;
    }
#if defined(CHECK_RANGE)
    const char *s = member;			// check if zero args
    while ( *s++ != '(' ) ;
    if ( *s != ')' ) {
	warning( "QSignal::connect: Receiver %s::%s cannot take parameters",
		 r->className(), member );
	return FALSE;
    }
#endif
    if ( !connlist ) {
	connlist = new QConnectionList;
	CHECK_PTR( connlist );
	connlist->setAutoDelete( TRUE );
    }
    connlist->append( new QConnection( r, *m ) );
    return TRUE;
}

bool QSignal::disconnect( const QObject *receiver, const char *memberName )
{
#if defined(CHECK_NULL)
    if ( receiver == 0 ) {
	warning( "QSignal::disconnect: Unexpected NULL parameter" );
	return FALSE;
    }
#endif
    if ( !connlist )				// no connections
	return FALSE;
    QString memberStr = memberName;
    char *member = memberStr.data();
    if ( member )
	rmWS( member, memberName );
    register QConnection *c = connlist->first();
    while ( c ) {
	if ( c->object() == receiver ) {
	    if ( !member /* !!! || strcmp(member,c->member())==0 */ ) {
		connlist->remove();
		c = connlist->current();
		continue;
	    }
	}
	c = connlist->next();
    }
    return TRUE;
}


void QSignal::activate()
{
    if ( !connlist || blocked() )		// cannot activate signal
	return;
    typedef void (QObject::*RT)();
    typedef RT *PRT;
    QListIteratorM(QConnection) it(*connlist);
    register QConnection *c;
    RT r;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;					// be safe
	r = *((PRT)(c->member()));
	object = (QSenderObject*)c->object();
	object->setSender( this );
	(object->*r)();				// fire!
    }
}
