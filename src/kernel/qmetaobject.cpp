/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobject.cpp#58 $
**
** Implementation of QMetaObject class
**
** Created : 930419
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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

#include "qmetaobject.h"
#include "qobjectlist.h"
#include "qobjectdict.h"
#include "qstrlist.h"
#include "qtl.h"

// NOT REVISED

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
  The private object.
 *****************************************************************************/


class QMetaObjectPrivate
{
public:
    QMetaEnum     *enumData;			// enumeration types
    int		   numEnumData;
    QMetaProperty *propData;                    // property meta data
    int            numPropData;
    QClassInfo    *classInfo;			// class information
    int            numClassInfo;
};


/*****************************************************************************
  Internal dictionary for fast access to class members
 *****************************************************************************/

#if defined(QT_DLL)
template class Q_EXPORT QAsciiDict<QMetaData>;
#endif

class Q_EXPORT QMemberDict : public QAsciiDict<QMetaData>
{
public:
    QMemberDict(int size=17,bool cs=TRUE,bool ck=TRUE) :
	QAsciiDict<QMetaData>(size,cs,ck) {}
    QMemberDict( const QMemberDict &dict ) : QAsciiDict<QMetaData>(dict) {}
    ~QMemberDict() { clear(); }
    QMemberDict &operator=(const QMemberDict &dict)
    { return (QMemberDict&)QAsciiDict<QMetaData>::operator=(dict); }
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

// ## To disappear in Qt 3.0
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

    classname = class_name;			// set meta data
    superclassname = superclass_name;
    slotDict = init( slotData = slot_data, n_slots );
    signalDict = init( signalData = signal_data, n_signals );

    objectDict->insert( classname, this );	// insert into object dict

    superclass = objectDict->find( superclassname ); // get super class meta object
						

    d = new QMetaObjectPrivate;
    reserved = 0;

    d->enumData = 0;
    d->numEnumData = 0;
    d->propData = 0;
    d->numPropData = 0;
    d->classInfo = 0;
    d->numClassInfo = 0;
}

QMetaObject::QMetaObject( const char *class_name, const char *superclass_name,
			  QMetaData *slot_data,	  int n_slots,
			  QMetaData *signal_data, int n_signals,
			  QMetaProperty *prop_data, int n_props,
			  QMetaEnum *enum_data, int n_enums,
			  QClassInfo *class_info, int n_info )
{
    if ( !objectDict ) {			// first meta object created
	objectDict
	    = new QObjectDictionary( 211,
				     TRUE,	// no copying of keys
				     FALSE );	// case sensitive
	CHECK_PTR( objectDict );
	objectDict->setAutoDelete( TRUE );	// use as master dict
    }

    classname = class_name;			// set meta data
    superclassname = superclass_name;
    slotDict = init( slotData = slot_data, n_slots );
    signalDict = init( signalData = signal_data, n_signals );

    d = new QMetaObjectPrivate;
    reserved = 0;

    d->propData = prop_data;
    d->numPropData = n_props;
    d->enumData = enum_data;
    d->numEnumData = n_enums;
    d->classInfo = class_info;
    d->numClassInfo = n_info;

    objectDict->insert( classname, this );	// insert into object dict

    superclass = objectDict->find( superclassname ); // get super class meta object
						
}

QMetaObject::~QMetaObject()
{
    if ( slotData )
	delete [] slotData;			// delete arrays created in
    if ( signalData )
	delete [] signalData;			//   initMetaObject()
    if ( d->enumData )
	delete [] d->enumData;
    if ( d->propData )
	delete [] d->propData;
    if ( d->classInfo )
	delete [] d->classInfo;
    delete slotDict;				// delete dicts
    delete signalDict;
    delete d;
    delete reserved;
}


int QMetaObject::numSlots( bool super ) const	// number of slots
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

int QMetaObject::numSignals( bool super ) const	// number of signals
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

QMetaObject *QMetaObject::new_metaobject( const char *classname,
					  const char *superclassname,
					  QMetaData *slot_data,	int n_slots,
					  QMetaData *signal_data,int n_signals,
					  QMetaProperty *prop_data, int n_props,
					  QMetaEnum *enum_data, int n_enums,
					  QClassInfo * class_info, int n_info )
{
    return new QMetaObject( classname, superclassname, slot_data, n_slots,
			    signal_data, n_signals, prop_data, n_props,
			    enum_data, n_enums, class_info, n_info );
}

QMetaObject *QMetaObject::new_metaobject( const char *classname,
					  const char *superclassname,
					  QMetaData *slot_data,	int n_slots,
					  QMetaData *signal_data,int n_signals)
{
    return new QMetaObject( classname, superclassname, slot_data, n_slots,
			    signal_data, n_signals );
}

QMetaData *QMetaObject::new_metadata( int numEntries )
{
    return numEntries > 0 ? new QMetaData[numEntries] : 0;
}


QMetaEnum *QMetaObject::new_metaenum( int numEntries )
{
    return numEntries > 0 ? new QMetaEnum[numEntries] : 0;
}

QMetaEnum::Item *QMetaObject::new_metaenum_item( int numEntries )
{
    return numEntries > 0 ? new QMetaEnum::Item[numEntries] : 0;
}

QMetaProperty *QMetaObject::new_metaproperty( int numEntries )
{
    return numEntries > 0 ? new QMetaProperty[numEntries] : 0;
}

QClassInfo *QMetaObject::new_classinfo( int numEntries )
{
    return numEntries > 0 ? new QClassInfo[numEntries] : 0;
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
#if !defined(Q_NO_DEAD_CODE)
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
		index -= n;
		meta = meta->superclass;
		if ( !meta )			// there is no superclass
		    return 0;
		continue;
	    }
	}
	if ( index >= 0 && index < n ) {
	    switch ( code ) {			// find member
	    case SLOT_CODE:	  d = meta->slotData;	break;
	    case SIGNAL_CODE: d = meta->signalData; break;
	    default:	  d = 0;	// eliminates compiler warning
	    }
	    if ( d )
		return &(d[n-index-1]);
	} else {				// bad index
	    return 0;
	}
    }
#if !defined(Q_NO_DEAD_CODE)
    return 0;
#endif
}


int QMetaObject::numClassInfo( bool super ) const
{
    return d->numClassInfo + (super && superclass)?superclass->numClassInfo(super):0;
}

QClassInfo* QMetaObject::classInfo( int index, bool super ) const
{
    if ( index < 0 )
	return 0;
    if ( index < d->numClassInfo )
	return &(d->classInfo[ index ]);
    if ( !super || !superclass )
	return 0;
    return superclass->classInfo( index - d->numClassInfo, super );
}

const char* QMetaObject::classInfo( const char* name, bool super ) const
{
    for( int i = 0; i < d->numClassInfo; ++i ) {
	if ( strcmp( d->classInfo[i].name, name ) == 0 )
	    return d->classInfo[i].value;
    }
    if ( !super || !superclass )
	return 0;
    return superclass->classInfo( name, super );
}


void QMetaObject::resolveProperty( QMetaProperty* prop )
{
    QMetaObject* super = superclass;
    while ( super ) {
	const QMetaProperty* p = super->property( prop->n );
	if( p ) {
	    if ( strcmp( prop->type(), p->type() ) != 0 )
		qDebug( "Attempt to override property type: %s %s::%s clashes with %s %s::%s", p->type(), super->className(), p->name(),
			prop->type(), className(), prop->name() );
	    if ( prop->get == 0 ) {
		if ( p->get ) {
		    prop->get = p->get;
		    prop->gspec = p->gspec;
		}
	    }
	    if ( prop->set == 0 ) {
		if ( p->set ) {
		    prop->set = p->set;
		    prop->sspec = p->sspec;
		}
	    }

	    if ( prop->testFlags( QMetaProperty::UnresolvedStored ) )
	    {
		if ( !p->testFlags( QMetaProperty::UnresolvedStored ) )
		{
		    prop->clearFlags( QMetaProperty::UnresolvedStored );
		    if ( p->testFlags( QMetaProperty::NotStored ) )
			prop->setFlags( QMetaProperty::NotStored );
		    prop->store = p->store;
		}
	    }
	    if ( prop->testFlags( QMetaProperty::UnresolvedDesignable ) )
	    {
		if ( !p->testFlags( QMetaProperty::UnresolvedDesignable ) )
		{
		    prop->clearFlags( QMetaProperty::UnresolvedDesignable );
		    if ( p->testFlags( QMetaProperty::NotDesignable ) )
			prop->setFlags( QMetaProperty::NotDesignable );
		}
	    }
	}
	if ( prop->testFlags( QMetaProperty::UnresolvedEnum | QMetaProperty::UnresolvedSet | QMetaProperty::UnresolvedEnumOrSet ) ) {
	    QMetaEnum* e = super->enumerator( prop->t);
	    if ( e && e->set ) {
		if ( !prop->testFlags( QMetaProperty::UnresolvedSet | QMetaProperty::UnresolvedEnumOrSet ) )
		    qDebug("The property %s %s::%s assumed that '%s' was listed in Q_ENUMS, but it was listed in Q_SETS",
			   prop->type(), className(), prop->name(), prop->type() );
		prop->enumData = e;
		prop->clearFlags( QMetaProperty::UnresolvedEnum );
	    }
	    else if ( e && !e->set ) {
		if ( !prop->testFlags( QMetaProperty::UnresolvedEnum | QMetaProperty::UnresolvedEnumOrSet ) )
		    qDebug("The property %s %s::%s assumed that '%s' was listed in Q_SETS, but it was listed in Q_ENUMS",
			   prop->type(), className(), prop->name(), prop->type() );
		prop->enumData = e;
		prop->clearFlags( QMetaProperty::UnresolvedEnum );
	    }
	}
	super = super->superclass;
    }
	
    if ( !prop->isValid() )
	qDebug("Could not resolve property %s::%s. Property not available.", className(), prop->name() );
}


const QMetaProperty* QMetaObject::property( const char* name, bool super ) const
{
    for( int i = 0; i < d->numPropData; ++i ) {
	if ( d->propData[i].isValid() && strcmp( d->propData[i].name(), name ) == 0 )
	    return &(d->propData[i]);
    }
    if ( !super || !superclass )
	return 0;
    return superclass->property( name, super );
}

QStrList QMetaObject::propertyNames( bool super ) const
{
    QStrList l( FALSE );
    for( int i = 0; i < d->numPropData; ++i ) {
	if ( d->propData[i].isValid() )
	    l.inSort( d->propData[i].name() );
    }

    if ( superclass && super ) {
	QStrList sl = superclass->propertyNames( super );
	for ( QStrListIterator slit( sl ); slit.current(); ++slit )
	    l.inSort ( slit.current() );
    }

    if ( l.count() < 2 )
	return l;

    // Remove dups
    QStrListIterator it( l );
    const char* old = it.current();
    ++it;
    while( it.current() ) {
	if ( strcmp( old, it.current() ) == 0 ) {
	    l.removeRef( old );
	}
	old = it.current();
	++it;
    }
    return l;
}

QStrList QMetaObject::signalNames( bool super ) const
{
    QStrList l( FALSE );
    int n = numSignals( super );
    for( int i = 0; i < n; ++i ) {
	l.inSort( signal(i, super)->name );
    }
    if ( l.count() < 2 )
	return l;

    // Remove dups
    QStrListIterator it( l );
    const char* old = it.current();
    ++it;
    while( it.current() ) {
	if ( strcmp( old, it.current() ) == 0 ) {
	    l.removeRef( old );
	}
	old = it.current();
	++it;
    }
    return l;
}

QStrList QMetaObject::slotNames( bool super ) const
{
    QStrList l( FALSE );
    int n = numSlots( super );
    for( int i = 0; i < n; ++i ) {
	l.inSort( slot(i, super)->name );
    }
    if ( l.count() < 2 )
	return l;

    // Remove dups
    QStrListIterator it( l );
    const char* old = it.current();
    ++it;
    while( it.current() ) {
	if ( strcmp( old, it.current() ) == 0 ) {
	    l.removeRef( old );
	}
	old = it.current();
	++it;
    }
    return l;
}


QMetaEnum* QMetaObject::enumerator( const char* name, bool super ) const
{
    for( int i = 0; i < d->numEnumData; ++i )
	if ( strcmp( d->enumData[i].name, name ) == 0 )
	    return &(d->enumData[i]);
    if ( !super || !superclass )
	return 0;
    return superclass->enumerator( name, super );
}

bool QMetaObject::inherits( const char* clname ) const
{
    const QMetaObject *meta = this;
    while ( meta ) {
	if ( strcmp(clname, meta->className()) == 0 )
	    return TRUE;
	meta = meta->superclass;
    }
    return FALSE;
}


QStrList QMetaProperty::enumKeys() const
{
     QStrList l( FALSE );
     if ( enumData != 0 ) {
	 for( uint i = 0; i < enumData->count; ++i )
	     l.append( enumData->items[i].key );
     }
     return l;
}

bool QMetaProperty::stored( QObject* o ) const
{
    if ( !isValid() || set == 0 || testFlags( NotStored | UnresolvedStored ) )
	return FALSE;

    if ( store == 0 )
	return TRUE;

    typedef bool (QObject::*ProtoBool)() const;	
    ProtoBool m;
    m = *((ProtoBool*)&store);

    return (o->*m)();
}
