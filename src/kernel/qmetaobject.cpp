/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobject.cpp#34 $
**
** Implementation of QMetaObject class
**
** Created : 930419
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#include "qmetaobject.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qstrlist.h"


/* not documented
  \class QMetaObject qmetaobject.h

  \brief The QMetaObject class is an internal class used for the meta
  object system.

  It is generally a very bad idea to use this class directly in
  application programs.

  \internal

  This class is not yet documented.  Our <a
  href="http://www.troll.no">home page</a> contains a pointer to the
  current version of Qt.
*/


QObjectDictionary *objectDict = 0;		// global object dictionary


/*****************************************************************************
  Internal dictionary for fast access to class members
 *****************************************************************************/

template class Q_EXPORT QDict<QMetaData>;

class Q_EXPORT QMemberDict : public QDict<QMetaData>
{
public:
    QMemberDict(int size=17,bool cs=TRUE,bool ck=TRUE) :
	QDict<QMetaData>(size,cs,ck) {}
    QMemberDict( const QMemberDict &dict ) : QDict<QMetaData>(dict) {}
   ~QMemberDict() { clear(); }
    QMemberDict &operator=(const QMemberDict &dict)
	{ return (QMemberDict&)QDict<QMetaData>::operator=(dict); }
};

/*
  Calculate optimal dictionary size for n entries using prime numbers,
  and assuming there are no more than 40 entries.
*/

static int optDictSize( int n )
{
    if ( n < 6 )
	n = 5;
    else if ( n < 10 )
	n = 11;
    else if ( n < 14 )
	n = 17;
    else
	n = 23;
    return n;
}


/*****************************************************************************
  QMetaObject member functions
 *****************************************************************************/

QMetaObject::QMetaObject( const char *class_name, const char *superclass_name,
			  QMetaData *slot_data,	  int n_slots,
			  QMetaData *signal_data, int n_signals )
{
    if ( !objectDict ) {			// first meta object created
	objectDict
	    = new QObjectDictionary( 211,
				     TRUE,	// no copying of keys
				     FALSE );	// case sensitive
	CHECK_PTR( objectDict );
	objectDict->setAutoDelete( TRUE );	// use as master dict
    }

    classname = (char *)class_name;		// set meta data
    superclassname = (char *)superclass_name;
    slotDict = init( slotData = slot_data, n_slots );
    signalDict = init( signalData = signal_data, n_signals );

    objectDict->insert( classname, this );	// insert into object dict

    superclass =				// get super class meta object
	objectDict->find( superclassname );
}

QMetaObject::~QMetaObject()
{
    if ( slotData )
	delete [] slotData;			// delete arrays created in
    if ( signalData )
	delete [] signalData;			//   initMetaObject()
    delete slotDict;				// delete dicts
    delete signalDict;
}


int QMetaObject::nSlots( bool super ) const	// number of slots
{
    if ( !super )
	return slotDict ? slotDict->count() : 0;
    int n = 0;
    register QMetaObject *meta = (QMetaObject *)this;
    while ( meta ) {				// for all super classes...
	if ( meta->slotDict )
	    n += meta->slotDict->count();
	meta = meta->superclass;
    }
    return n;
}

int QMetaObject::nSignals( bool super ) const	// number of signals
{
    if ( !super )
	return signalDict ? signalDict->count() : 0;
    int n = 0;
    register QMetaObject *meta = (QMetaObject *)this;
    while ( meta ) {				// for all super classes...
	if ( meta->signalDict )
	    n += meta->signalDict->count();
	meta = meta->superclass;
    }
    return n;
}


QMetaData *QMetaObject::slot( const char *n, bool super ) const
{
    return mdata( SLOT_CODE, n, super );	// get slot meta data
}

QMetaData *QMetaObject::signal( const char *n, bool super ) const
{
    return mdata( SIGNAL_CODE, n, super );	// get signal meta data
}

QMetaData *QMetaObject::slot( int index, bool super ) const
{
    return mdata( SLOT_CODE, index, super );	// get slot meta data
}

QMetaData *QMetaObject::signal( int index, bool super ) const
{
    return mdata( SIGNAL_CODE, index, super );	// get signal meta data
}


QMemberDict *QMetaObject::init( QMetaData *data, int n )
{
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


QMetaData *QMetaObject::mdata( int code, const char *name, bool super ) const
{
    QMetaObject *meta = (QMetaObject *)this;
    QMemberDict *dict;
    while ( TRUE ) {
	switch ( code ) {			// find member
	    case SLOT_CODE:   dict = meta->slotDict;   break;
	    case SIGNAL_CODE: dict = meta->signalDict; break;
	    default:	      return 0;		// should not happen
	}
	
	if ( dict ) {
	    QMetaData *d = dict->find(name);
	    if ( d )
		return d;
	}
	if ( super && meta->superclass )	// try for super class
	    meta = meta->superclass;
	else					// not found
	    return 0;
    }
#if !defined(NO_DEADCODE)
    return 0;
#endif
}

QMetaData *QMetaObject::mdata( int code, int index, bool super ) const
{
    register QMetaObject *meta = (QMetaObject *)this;
    QMetaData *d;
    QMemberDict *dict;
    while ( TRUE ) {
	switch ( code ) {			// find member
	    case SLOT_CODE:   dict = meta->slotDict;   break;
	    case SIGNAL_CODE: dict = meta->signalDict; break;
	    default:	      return 0;		// should not happen
	}
	int n = dict ? dict->count() : 0;
	if ( super ) {
	    if ( index >= n ) {			// try the superclass
		index -= dict->count();
		meta = meta->superclass;
		if ( !meta )			// there is no superclass
		    return 0;
		continue;
	    }
	}
	if ( index >= 0 && index < n ) {
	    switch ( code ) {			// find member
		case SLOT_CODE:	  d = slotData;	  break;
		case SIGNAL_CODE: d = signalData; break;
		default:	  d = 0;	// eliminates compiler warning
	    }
	    return &d[n-index-1];
	} else {				// bad index
	    return 0;
	}
    }
#if !defined(NO_DEADCODE)
    return 0;
#endif
}
