/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobject.cpp#58 $
**
** Implementation of QMetaObject class
**
** Created : 930419
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
    QMetaEnum     *enumData;
    int		   nEnumData;
    QMetaProperty *propData;                    // property meta data
    int            nPropData;
    QMetaMetaProperty *metaPropData;
    int		   nMetaPropData;
    QObjectFactory     objectFactory;
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

    classname = (char *)class_name;		// set meta data
    superclassname = (char *)superclass_name;
    slotDict = init( slotData = slot_data, n_slots );
    signalDict = init( signalData = signal_data, n_signals );

    objectDict->insert( classname, this );	// insert into object dict

    superclass = objectDict->find( superclassname ); // get super class meta object
						

    d = new QMetaObjectPrivate;
    reserved = 0;

    d->enumData = 0;
    d->nEnumData = 0;
    d->propData = 0;
    d->nPropData = 0;
    d->metaPropData = 0;
    d->nMetaPropData = 0;
    d->objectFactory = 0;
}

QMetaObject::QMetaObject( const char *class_name, const char *superclass_name,
			  QMetaData *slot_data,	  int n_slots,
			  QMetaData *signal_data, int n_signals,
			  QMetaProperty *prop_data, int n_props,
			  QMetaEnum *enum_data, int n_enums,
			  QMetaMetaProperty* meta_prop_data, int n_meta_props )
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

    d = new QMetaObjectPrivate;
    reserved = 0;

    d->propData = prop_data;
    d->nPropData = n_props;
    d->enumData = enum_data;
    d->nEnumData = n_enums;
    d->metaPropData = meta_prop_data;
    d->nMetaPropData = n_meta_props;
    d->objectFactory = 0;

    objectDict->insert( classname, this );	// insert into object dict

    superclass = objectDict->find( superclassname ); // get super class meta object
						
}

QMetaObject::~QMetaObject()
{
    if ( slotData )
	delete [] slotData;			// delete arrays created in
    if ( signalData )
	delete [] signalData;			//   initMetaObject()
    delete slotDict;				// delete dicts
    delete signalDict;
    delete d;
    delete reserved;
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

int QMetaObject::nProperties( bool super ) const	// number of properties
{
    if ( !super )
	return d->nPropData;
    int n = 0;
    register QMetaObject *meta = (QMetaObject *)this;
    while ( meta ) {				// for all super classes...
	n += meta->d->nPropData;
	meta = meta->superclass;
    }
    return n;
}

int QMetaObject::nMetaProperties( bool super ) const	// number of meta properties
{
    if ( !super )
	return d->nMetaPropData;
    int n = 0;
    register QMetaObject *meta = (QMetaObject *)this;
    while ( meta ) {				// for all super classes...
	n += meta->d->nMetaPropData;
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
					  QMetaMetaProperty* meta_prop_data, int n_meta_props )
{
    return new QMetaObject( classname, superclassname, slot_data, n_slots,
			    signal_data, n_signals, prop_data, n_props,
			    enum_data, n_enums, meta_prop_data, n_meta_props );
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

void QMetaObject::resolveProperty( QMetaProperty* prop )
{
    QMetaObject* super = superclass;
    while ( super ) {
	QMetaProperty* p = super->property( prop->name );
	if( p ) {
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
	}
	if ( prop->testState( QMetaProperty::UnresolvedEnum ) ) {
	    QMetaEnum* e = super->enumerator( prop->type );
	    if ( e ) {
		prop->enumType = e;
		prop->clearState( QMetaProperty::UnresolvedEnum );
	    }
	}
	super = super->superClass();
    }
}

QMetaEnum* QMetaObject::enumerator( const char* name, bool superclassname ) const
{
    for( int i = 0; i < d->nEnumData; ++i )
	if ( strcmp( d->enumData[i].name, name ) == 0 )
	    return &(d->enumData[i]);

    if ( !superclassname )
	return 0;

    QMetaObject* super = superclass;
    while( super )
	{
	    QMetaEnum* e = super->enumerator( name, superclassname );
	    if ( e )
		return e;
	    super = super->superClass();
	}

    return 0;
}

QMetaProperty* QMetaObject::property( int index ) const
{
    if ( index >= d->nPropData || index < 0 )
	return 0;

    return &(d->propData[ index ]);
}

QMetaProperty* QMetaObject::property( const char* name, bool super ) const
{
    for( int i = 0; i < d->nPropData; ++i )
	if ( strcmp( d->propData[i].name, name ) == 0 )
	    return &(d->propData[i]);

    if ( !super )
	return 0;

    QMetaObject* meta = superclass;
    while ( meta ) {
	QMetaProperty* p = meta->property( name, super );
	if ( p )
	    return p;
	meta = meta->superClass();
    }

    return 0;
}

QObjectFactory QMetaObject::factory() const
{
    return d->objectFactory;
}

void QMetaObject::setFactory( QObjectFactory f )
{
    d->objectFactory = f;
}

QStringList QMetaObject::propertyNames( bool super ) const
{
    QStringList l;
    for( int i = 0; i < d->nPropData; ++i )
	l.append( d->propData[i].name );

    if ( superclass && super )
	l += superclass->propertyNames( super );

    if ( l.count() < 2 )
	return l;

    qBubbleSort( l );

    // Remove dups
    QStringList::Iterator it = l.begin();
    QString old = *(it++);
    while( it != l.end() ) {
	if ( old == *it ) {
	    it = l.remove( it );
	}
	else {
	    old = *it;
	    ++it;
	}
    }
    return l;
}

bool QMetaObject::inherits( const char* clname ) const
{
    const QMetaObject *meta = this;
    while ( meta ) {
	if ( strcmp(clname, meta->className()) == 0 )
	    return TRUE;
	meta = meta->superClass();
    }
    return FALSE;
}

QStringList QMetaEnum::enumeratorNames()
{
    QStringList l;
    for( int i = 0; i < nEnumerators; ++i )
	l.append( enumerators[i].name );

    return l;
}

QMetaMetaProperty* QMetaObject::metaProperty( int index ) const
{
    if ( index >= d->nMetaPropData || index < 0 )
	return 0;

    return &(d->metaPropData[ index ]);
}

const char* QMetaObject::metaProperty( const char* name, bool super ) const
{
    for( int i = 0; i < d->nMetaPropData; ++i )
	if ( strcmp( d->metaPropData[i].name, name ) == 0 )
	    return d->metaPropData[i].value;

    if ( !super )
	return 0;

    QMetaObject* meta = superclass;
    while( meta ) {
	const char* p = meta->metaProperty( name, super );
	if ( p )
	    return p;
	meta = meta->superClass();
    }

    return 0;
}

QStringList QMetaObject::metaPropertyNames( bool super ) const
{
    QStringList l;
    for( int i = 0; i < d->nMetaPropData; ++i )
	l.append( d->metaPropData[i].name );

    if ( superclass && super )
	l += superclass->metaPropertyNames( super );

    if ( l.count() < 2 )
	return l;

    qBubbleSort( l );

    return l;
}


struct QMetaInitFunction {
    QMetaInitFunction( QMetaObject*(*fn)(), QMetaInitFunction* n ) : f(fn), f_old(0), next(n) { }
    // ## To disappear in Qt 3.0
    QMetaInitFunction( void(*fn)(), QMetaInitFunction* n ) : f(0), f_old(fn), next(n) { }
    ~QMetaInitFunction() { delete next; }
    QMetaObject*(*f)();
    // ## To disappear in Qt 3.0
    void(*f_old)();
    QMetaInitFunction* next;
};

static QMetaInitFunction* functions_head = 0;

static QMetaObject **meta_list = 0;
static int n_meta_list = 0;

/* not documented
  \class QMetaObjectInit qmetaobject.h

  \brief The QMetaObjectInit class is an internal class used for the meta
  object system.

  It is generally a very bad idea to use this class directly in
  application programs except you are interested in writing
  a GUI-Builder or something compareable.

  \internal

  This class is not yet documented.  Our <a
  href="http://www.troll.no">home page</a> contains a pointer to the
  current version of Qt.
*/

/*!
  Creates a new QMetaObjectInit instance. It will add the
  meta object factory pointer to a list. Once init() is called
  all factories are called to produce their QMetaObject. After
  that the list of factories will be deleted again to save memory.
*/
QMetaObjectInit::QMetaObjectInit(QMetaObject*(*f)())
{
    functions_head = new QMetaInitFunction(f,functions_head);
}

// ## To disappear in Qt 3.0
QMetaObjectInit::QMetaObjectInit(void(*f)())
{
    functions_head = new QMetaInitFunction(f,functions_head);
}

/*!
  Creates a list of all QMetaObject instances. This function is
  called from all the other static member functions of QMetaObjectInit.
  Calling it twice does not harm. The function returns the number of
  QMetaObjects.
*/
int QMetaObjectInit::init()
{
    typedef QMetaObject* QMetaObjectPtr;

    // Did we do init() already ?
    if ( functions_head == 0 )
	return n_meta_list;

    // Count the classes
    int i = 0;
    QMetaInitFunction* f;
    for ( f = functions_head; f; f = f->next)
	if ( f->f ) // This line is to disappear in Qt 3.0
	    i++;
    // Create a list of all meta objects
    meta_list = new QMetaObjectPtr[ i ];

    // Create all metaobjects
    i = 0;
    for ( f = functions_head; f; f = f->next) {
	if ( f->f )       // This line is to disappear in Qt 3.0
	    meta_list[i++] = (*(f->f))();
	else              // This line is to disappear in Qt 3.0
	    (*(f->f_old))();  // This line is to disappear in Qt 3.0
    }
    n_meta_list = i;

    delete functions_head;
    // This will remember us that we have done init() already
    functions_head = 0;
    return n_meta_list;
}

/*!
  Returns the meta object for a certain class or 0 if there is no
  QMetaObject registered for that class.
*/
QMetaObject* QMetaObjectInit::metaObject( const char* classname )
{
    // Perhaps someone did not call init
    init();

    for( int i = 0; i < n_meta_list; ++i ) 
	if ( strcmp( meta_list[i]->className(), classname ) == 0 )
	    return meta_list[i];

    return 0;
}

/*!
  Returns the meta object at a certain position in the index.
  Together with the function nMetaObjects() this function can be
  used to iterate over all QMetaObjects.
*/
QMetaObject* QMetaObjectInit::metaObject( int index )
{
    // Perhaps someone did not call init
    init();

    if ( index > n_meta_list )
	return 0;

    return meta_list[ index ];
}

/*!
  Returns the amount of registered QMetaObjects.
*/
int QMetaObjectInit::nMetaObjects()
{
    return init();
}

