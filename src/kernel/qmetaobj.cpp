/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobj.cpp#1 $
**
** Implementation of QMetaObject class
**
** Author  : Haavard Nord
** Created : 930419
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#define	 NO_WARNINGS
#include "qmetaobj.h"
#include "qobjcoll.h"
#include "qstrlist.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qmetaobj.cpp#1 $";
#endif


QObjectDictionary *objectDict = 0;		// global object dictionary


// --------------------------------------------------------------------------
// Internal dictionary for fast access to class members
//

declare(QDictM,QMetaData);


// --------------------------------------------------------------------------
// Calculate optimal dictionary size for n entries using prime numbers,
// and assuming there are no more than 40 entries.
//

static int optDictSize( int n )
{
    if ( n < 6 )
	n = 5;
    else
    if ( n < 10 )
	n = 11;
    else
    if ( n < 14 )
	n = 17;
    else
	n = 23;
    return n;
}


// --------------------------------------------------------------------------
// QMetaObject member functions
//

QMetaObject::QMetaObject( const char *class_name, const char *superclass_name,
			  QMetaData *method_data, int n_methods,
			  QMetaData *slot_data,	  int n_slots,
			  QMetaData *signal_data, int n_signals )
{
    if ( !objectDict ) {			// first meta object created
	objectDict = new
		QObjectDictionary( 211,		// suitable prime number
				   TRUE,	// case sensitive
				   FALSE );	// no copying of keys
	CHECK_PTR( objectDict );
	objectDict->setAutoDelete( TRUE );	// use as master dict
    }

    className = (char *)class_name;		// copy in data
    superclassName = (char *)superclass_name;
    methodDict = init( methodData = method_data, n_methods );
    slotDict = init( slotData = slot_data, n_slots );
    signalDict = init( signalData = signal_data, n_signals );

    objectDict->insert( className, this );	// insert into object dict

    superMetaObject =				// get super class meta object
	objectDict->find( superclassName );
}

QMetaObject::~QMetaObject()
{
    delete methodData;				// delete arrays created in
    delete slotData;				//   initMetaObject()
    delete signalData;
    delete methodDict;				// delete dicts
    delete slotDict;
    delete signalDict;
}


int QMetaObject::nMethods( bool super ) const	// number of methods
{
    if ( !super )
	return methodDict ? methodDict->count() : 0;
    int n = 0;
    QMetaObject *meta = (QMetaObject *)this;	// for all classes
    while ( meta ) {
	if ( meta->methodDict )
	    n += meta->methodDict->count();
	meta = meta->superMetaObject;
    }
    return n;
}

int QMetaObject::nSlots( bool super ) const	// number of slots
{
    if ( !super )
	return slotDict ? slotDict->count() : 0;
    int n = 0;
    QMetaObject *meta = (QMetaObject *)this;	// for all classes
    while ( meta ) {
	if ( meta->slotDict )
	    n += meta->slotDict->count();
	meta = meta->superMetaObject;
    }
    return n;
}

int QMetaObject::nSignals( bool super ) const	// number of signals
{
    if ( !super )
	return signalDict ? signalDict->count() : 0;
    int n = 0;
    QMetaObject *meta = (QMetaObject *)this;	// for all classes
    while ( meta ) {
	if ( meta->signalDict )
	    n += meta->signalDict->count();
	meta = meta->superMetaObject;
    }
    return n;
}


QMember *QMetaObject::method( const char *n, bool super ) const
{
    return member( METHOD_CODE, n, super );	// get method member
}

QMember *QMetaObject::slot( const char *n, bool super ) const
{
    return member( SLOT_CODE, n, super );	// get slot member
}

QMember *QMetaObject::signal( const char *n, bool super ) const
{
    return member( SIGNAL_CODE, n, super );	// get signal member
}


void QMetaObject::getMethods( QStrList *list, bool super ) const
{
    getNames( METHOD_CODE, list, super );	// get method names into list
}

void QMetaObject::getSlots( QStrList *list, bool super ) const
{
    getNames( SLOT_CODE, list, super );		// get method names into list
}

void QMetaObject::getSignals( QStrList *list, bool super ) const
{
    getNames( SIGNAL_CODE, list, super );	// get method names into list
}


QMemberDict *QMetaObject::init( QMetaData *data, int n )
{						// initialize meta data
    if ( n == 0 )				// nothing, then make no dict
	return 0;
    QMemberDict *dict = new QMemberDict( optDictSize(n), TRUE, FALSE );
    CHECK_PTR( dict );
    while ( n-- ) {				// put all members into dict
	dict->insert( data->name, data );
	data++;
    }
    return dict;
}

QMember *QMetaObject::member( int code, const char *name, bool super ) const
{						// get member pointer
    QMetaObject *meta = (QMetaObject *)this;
    QMetaData *d;
    QMemberDict *dict;
    while ( TRUE ) {
	switch ( code ) {			// find member
	    case METHOD_CODE: dict = meta->methodDict; break;
	    case SLOT_CODE:   dict = meta->slotDict;   break;
	    case SIGNAL_CODE: dict = meta->signalDict; break;
	    default:
#if defined(CHECK_RANGE)
		warning( "QMetaObject::member: Invalid code for %s", name );
#endif
		dict = meta->methodDict;	// recover
	}
	if ( dict && (d=dict->find(name)) )	// found it
	    return &d->ptr;
	if ( super && meta->superMetaObject )	// try for super class
	    meta = meta->superMetaObject;
	else					// not found
	    return 0;
    }
}

void QMetaObject::getNames( int code, QStrList *list, bool super ) const
{
    if ( super && superMetaObject )		// first get from super class
	superMetaObject->getNames( code, list, TRUE );
    int n = 0;
    switch ( code ) {				// get # members
	case METHOD_CODE: n = nMethods(); break;
	case SLOT_CODE:	  n = nSlots();	  break;
	case SIGNAL_CODE: n = nSignals(); break;
    }
    for ( int i=0; i<n; i++ ) {			// get member names
	char *name = 0;
	switch ( code ) {			// member type
	    case METHOD_CODE: name = methodData[i].name; break;
	    case SLOT_CODE:   name = slotData[i].name;	 break;
	    case SIGNAL_CODE: name = signalData[i].name; break;
	}
	list->append( name );
    }
}
