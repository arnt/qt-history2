/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobject.cpp#54 $
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

#ifdef QT_BUILDER
#include "qpixmap.h"
#include "qtl.h"
#endif

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

#ifdef QT_BUILDER
QMetaObject::QMetaObject( const char *class_name, const char *superclass_name,
			  QMetaData *slot_data,	  int n_slots,
			  QMetaData *signal_data, int n_signals,
			  QMetaProperty *prop_data, int n_props,
			  QMetaEnum *enum_data, int n_enums )
#else // QT_BUILDER
QMetaObject::QMetaObject( const char *class_name, const char *superclass_name,
			  QMetaData *slot_data,	  int n_slots,
			  QMetaData *signal_data, int n_signals )
#endif
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

#ifdef QT_BUILDER
    propData = prop_data;
    nPropData = n_props;
    enumData = enum_data;
    nEnumData = n_enums;
    inspectorFactory = 0;
    objectFactory = 0;
    commentData = 0;
    pixmapData = 0;
#endif // QT_BUILDER

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

#ifdef QT_BUILDER
int QMetaObject::nProperties( bool super ) const	// number of properties
{
    if ( !super )
	return nPropData;
    int n = 0;
    register QMetaObject *meta = (QMetaObject *)this;
    while ( meta ) {				// for all super classes...
      n += meta->nPropData;
      meta = meta->superclass;
    }
    return n;
}
#endif // QT_BUILDER

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

#ifdef QT_BUILDER
QMetaObject *QMetaObject::new_metaobject( const char *class_name,
					  const char *superclass_name,
					  QMetaData *slot_data,	int n_slots,
					  QMetaData *signal_data,int n_signals,
					  QMetaProperty *prop_data, int n_props,
					  QMetaEnum *enum_data, int n_enums )
{
    return new QMetaObject( class_name, superclass_name, slot_data, n_slots,
			    signal_data, n_signals, prop_data, n_props,
			    enum_data, n_enums );
}
#else // QT_BUILDER
QMetaObject *QMetaObject::new_metaobject( const char *class_name,
					  const char *superclass_name,
					  QMetaData *slot_data,	int n_slots,
					  QMetaData *signal_data,int n_signals)
{
    return new QMetaObject( class_name, superclass_name, slot_data, n_slots,
			    signal_data, n_signals );
}
#endif

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

#ifdef QT_BUILDER
void QMetaObject::fixProperty( QMetaProperty* prop, bool fix_enum_type )
{
  QMetaObject* super = superclass;
  while( super )
  {
    QMetaProperty* p = super->property( prop->name );
    if( p )
    {
      if ( prop->get == 0 )
      {
	if ( p->get )
	{
	  prop->get = p->get;
	  prop->getSpec = p->getSpec;
	}
      }
      if ( prop->set == 0 )
      {
	if ( p->set )
	{
	  prop->set = p->set;
	  prop->setSpec = p->setSpec;
	}
      }
    }
    if ( fix_enum_type && !prop->enumType )
    {
      QMetaEnum* e = super->enumerator( prop->type );
      if ( e )
	prop->enumType = e;
    }
    super = super->superClass();

    if ( prop->get && prop->set && ( prop->enumType || !fix_enum_type ) )
      return;
  }

  if ( prop->get == 0 )
  {
    fatal("In class %s the property %s has no get function. The same applies for its super classes.",
	  className(), prop->name );
  }
  if ( fix_enum_type && !prop->enumType )
  {
    fatal("In class %s the property %s has the illegal type %s.",
	  className(), prop->name, prop->type );
  }
}

QMetaEnum* QMetaObject::enumerator( const char* _name, bool _super ) const
{
  for( int i = 0; i < nEnumData; ++i )
    if ( strcmp( enumData[i].name, _name ) == 0 )
      return &enumData[i];

  if ( !_super )
    return 0;

  QMetaObject* super = superclass;
  while( super )
  {
    QMetaEnum* e = super->enumerator( _name );
    if ( e )
      return e;
    super = super->superClass();
  }

  return 0;
}

QMetaProperty* QMetaObject::property( int index ) const
{
    if ( index >= nPropData || index < 0 )
	return 0;
    
    return &propData[ index ];
}

QMetaProperty* QMetaObject::property( const char* _name, bool _super ) const
{
  for( int i = 0; i < nPropData; ++i )
    if ( strcmp( propData[i].name, _name ) == 0 )
      return &propData[i];

  if ( !_super )
    return 0;

  QMetaObject* super = superclass;
  while( super )
  {
    QMetaProperty* p = super->property( _name );
    if ( p )
      return p;
    super = super->superClass();
  }

  return 0;
}

QInspector* QMetaObject::inspector( QObject* _obj ) const
{
  if ( inspectorFactory )
    return inspectorFactory( _obj );
  return 0;
}

QString	QMetaObject::comment() const
{
  return QString( commentData );
}

QPixmap QMetaObject::pixmap() const
{
  return QPixmap( pixmapData );
}

QObjectFactory QMetaObject::factory() const
{
  return objectFactory;
}

void QMetaObject::setPixmap( const char* _pixmap[] )
{
  pixmapData = _pixmap;
}

void QMetaObject::setComment( const char* _comment )
{
  commentData = _comment;
}

void QMetaObject::setInspector( QInspectorFactory _ins )
{
  inspectorFactory = _ins;
}

void QMetaObject::setFactory( QObjectFactory f )
{
  objectFactory = f;
}

QStringList QMetaObject::propertyNames()
{
  QStringList l;
  for( int i = 0; i < nPropData; ++i )
    l.append( propData[i].name );

  if ( superclass )
  {
    QStringList super = superclass->propertyNames();
    l += super;
  }

  if ( l.count() < 2 )
    return l;

  qBubbleSort( l );

  // Remove dups
  QStringList::Iterator it = l.begin();
  QString old = *(it++);
  while( it != l.end() )
  {
    if ( old == *it )
      it = l.remove( it );
    else
    {
      old = *it;
      ++it;
    }
  }

  return l;
}

bool QMetaObject::inherits( const char* _class ) const
{
  if ( strcmp( _class, classname ) == 0 )
    return TRUE;

  if ( superClass() )
    return superClass()->inherits( _class );

  return 0;
}

QStringList QMetaEnum::enumeratorNames()
{
  QStringList l;
  for( int i = 0; i < nEnumerators; ++i )
    l.append( enumerators[i].name );

  return l;
}
#endif // QT_BUILDER

#ifdef QT_BUILDER

struct QMetaInitFunction {
    QMetaInitFunction( QMetaObject*(*fn)(), QMetaInitFunction* n ) : f(fn), next(n) { }
    ~QMetaInitFunction() { delete next; }
    QMetaObject*(*f)();
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
      i++;
    // Create a list of all meta objects
    meta_list = new QMetaObjectPtr[ i ];

    // Create all metaobjects
    i = 0;
    for ( f = functions_head; f; f = f->next)
      meta_list[i++] = (*(f->f))();
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
QMetaObject* QMetaObjectInit::metaObject( const char* _class_name )
{
    // Perhaps someone did not call init
    init();

    for( int i = 0; i < n_meta_list; ++i )
      if ( strcmp( meta_list[i]->className(), _class_name ) == 0 )
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

#else // QT_BUILDER

struct QMetaInitFunction {
    QMetaInitFunction( void(*fn)(), QMetaInitFunction* n ) : f(fn), next(n) { }
    ~QMetaInitFunction() { delete next; }
    void(*f)();
    QMetaInitFunction* next;
};

static QMetaInitFunction* functions_head = 0;

QMetaObjectInit::QMetaObjectInit(void(*f)())
{
    functions_head = new QMetaInitFunction(f,functions_head);
}

int QMetaObjectInit::init()
{
    int i=0;
    for (QMetaInitFunction* f = functions_head; f; f = f->next) {
	(*(f->f))();
	i++;
    }
    delete functions_head;
    functions_head = 0;
    return i;
}

#endif // QT_BUILDER
