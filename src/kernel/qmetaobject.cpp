/****************************************************************************
** $Id$
**
** Implementation of QMetaObject class
**
** Created : 930419
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qmetaobject.h"
#include "qasciidict.h"

/*!
  \class QMetaData qmetaobject.h

  \brief The QMetaData struct provides information about  a member function that is known to the meta object system.

  \internal

  The struct consists of three members, \e name, \e method  and \e access:

  \code
    const char *name;				// - member name
    const QUMethod* method;			// - detailed method description
    enum Access { Private, Protected, Public };
    Access access;				// - access permission
  \endcode
 */

/*!
  \class QClassInfo qmetaobject.h

  \brief The QClassInfo class provides a struct that stores some basic information about a single class.

  \internal

  The class information is a simple \e name - \e value pair:

  \code
    const char* name;
    const char* value;
  \endcode

 */


/*!
  \class QMetaObject qmetaobject.h

  \brief The QMetaObject class contains meta information about Qt objects.

  \ingroup objectmodel

  The Meta Object System in Qt is responsible for the signal/slot
  mechanism for communication between objects, runtime type
  information and the property system. All meta information in Qt is
  kept in a single instance of QMetaObject per class.

  In general, you will not have to use this class directly in any
  application program. However, if you write meta applications such
  as scripting engines or GUI builders, you might find these
  functions useful:

  \list
  \i className() to get the name of a class.
  \i superClassName() to get the name of the superclass.
  \i inherits(), the function called by QObject::inherits().
  \i superClass() to access the meta object of the superclass.
  \i numSlots(), numSignals(), slotNames(), and  signalNames() to get
      information about a class's signals and slots.
  \i property() and propertyNames() to receive information about a
      class's properties.
  \i classInfo() and numClassInfo() to access additional class information.
  \endlist

*/


/*****************************************************************************
  The private object.
 *****************************************************************************/


static QAsciiDict<QMetaObject> *qt_metaobjects = 0;

class QMetaObjectPrivate
{
public:
    QMetaObjectPrivate() :
#ifndef QT_NO_PROPERTIES
	enumData(0), numEnumData(0),
	propData(0),numPropData(0),
#endif
	classInfo(0), numClassInfo(0) {}
#ifndef QT_NO_PROPERTIES
    const QMetaEnum     *enumData;			// enumeration types
    int		   numEnumData;
    const QMetaProperty *propData;                    // property meta data
    int            numPropData;
#endif
    const QClassInfo    *classInfo;			// class information
    int            numClassInfo;

};


/*****************************************************************************
  Internal dictionary for fast access to class members
 *****************************************************************************/

#if defined(Q_CANNOT_DELETE_CONSTANT)
typedef QMetaData QConstMetaData;
#else
typedef const QMetaData QConstMetaData;
#endif

class Q_EXPORT QMemberDict : public QAsciiDict<QConstMetaData>
{
public:
    QMemberDict( int size = 17, bool cs = TRUE, bool ck = TRUE ) :
	QAsciiDict<QConstMetaData>(size,cs,ck) {}
    QMemberDict( const QMemberDict &dict ) : QAsciiDict<QConstMetaData>(dict) {}
    ~QMemberDict() { clear(); }
    QMemberDict &operator=(const QMemberDict &dict)
    { return (QMemberDict&)QAsciiDict<QConstMetaData>::operator=(dict); }
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

/*!\internal
 */
QMetaObject::QMetaObject( const char *const class_name, QMetaObject *super_class,
			  const QMetaData *const slot_data, int n_slots,
			  const QMetaData *const signal_data, int n_signals,
#ifndef QT_NO_PROPERTIES
			  const QMetaProperty *const prop_data, int n_props,
			  const QMetaEnum *const enum_data, int n_enums,
#endif
			  const QClassInfo *const class_info, int n_info )
{
    classname = class_name;			// set meta data
    superclass = super_class;
    superclassname = superclass ? superclass->className() : 0;
    slotDict = init( slotData = slot_data, n_slots );
    signalDict = init( signalData = signal_data, n_signals );

    d = new QMetaObjectPrivate;
    reserved = 0;

#ifndef QT_NO_PROPERTIES
    d->propData = prop_data;
    d->numPropData = n_props;
    d->enumData = enum_data;
    d->numEnumData = n_enums;
#endif
    d->classInfo = class_info;
    d->numClassInfo = n_info;

    signaloffset = superclass ? ( superclass->signalOffset() + superclass->numSignals() ) : 0;
    slotoffset = superclass ? ( superclass->slotOffset() + superclass->numSlots() ) : 0;
#ifndef QT_NO_PROPERTIES
    propertyoffset = superclass ? ( superclass->propertyOffset() + superclass->numProperties() ) : 0;
#endif
    if ( !qt_metaobjects )
	qt_metaobjects = new QAsciiDict<QMetaObject>( 257 );
    qt_metaobjects->replace( class_name, this );
}

/*!\internal
 */
QMetaObject::~QMetaObject()
{
    delete slotDict;				// delete dicts
    delete signalDict;
    delete d;
    if ( qt_metaobjects ) {
	qt_metaobjects->remove( classname );
	if ( qt_metaobjects->isEmpty() ) {
	    delete qt_metaobjects;
	    qt_metaobjects = 0;
	}
    }

    // delete reserved;				// Unused void*
}


/*! \fn const char *QMetaObject::className() const

  Returns the class name.

  \sa QObject::className(), superClassName()
*/

/*! \fn const char *QMetaObject::superClassName() const

  Returns the class name of the superclass or 0 if there is no superclass
  in the QObject hierachy.

  \sa className()
*/

/*! \fn QMetaObject *QMetaObject::superClass() const

  Returns the meta object of the super class or 0 if there is no such
  object.
 */

/*!
  Returns the number of slots for this class.

  If \a super is TRUE, inherited slots are included.

  \sa slotNames()
 */
int QMetaObject::numSlots( bool super ) const	// number of slots
{
    int n = slotDict ? slotDict->count() : 0;
    if ( !super || !superclass )
	return n;
    return n + superclass->numSlots( super );
}

/*!
  Returns the number of signals for this class.

  If \a super is TRUE, inherited signals are included.

  \sa signalNames()
 */
int QMetaObject::numSignals( bool super ) const	// number of signals
{
    int n = signalDict ? signalDict->count() : 0;
    if ( !super || !superclass )
	return n;
    return n + superclass->numSignals( super );
}


/*!  \internal

  Returns the meta data of the slot with the name \a n or 0 if no
  such slot exists.

  If  \a super is TRUE, inherited slots are included.
 */
const QMetaData* QMetaObject::slot( int index, bool super ) const
{
    int idx = index - ( super ? slotOffset() : 0 );
    if ( slotDict && idx >= 0 && idx < (int) slotDict->count() ) {
	return slotData + idx;
    }
    if ( !super || !superclass )
	return 0;
    return superclass->slot( index, super );
}

/*!  \internal

  Returns the meta data of the signal with the name \a n or 0 if no
  such signal exists.

  If  \a super is TRUE, inherited signals are included.
 */
const QMetaData* QMetaObject::signal( int index, bool super ) const
{
    int idx = index - ( super ? signalOffset() : 0 );
    if ( signalDict && idx >= 0 && idx < (int) signalDict->count() ) {
	return signalData + idx;
    }
    if ( !super || !superclass )
	return 0;
    return superclass->signal( index, super );
}


/*!
  \fn int QMetaObject::signalOffset() const

  \internal

  Returns the signal offset for this metaobject.

*/

/*!
  \fn int QMetaObject::propertyOffset() const

  \internal

  Returns the property offset for this metaobject.

*/

/*! \internal
  Returns the index of the signal with name \n or -1 if no such signal exists.

  If  \a super is TRUE, inherited signals are included.
 */
int QMetaObject::findSignal( const char* n, bool super ) const
{
    const QMetaData *md = signalDict ? signalDict->find( n ) : 0;
    if ( md )
	return signalOffset() + ( md - signalData );
    if ( !super || !superclass)
	return -1;
    return superclass->findSignal( n, super );
}

/*!
  \fn int QMetaObject::slotOffset() const

  \internal

  Returns the slot offset for this metaobject.

*/

/*! \internal
  Returns the index of the slot with name \n or -1 if no such slot exists.

  If  \a super is TRUE, inherited slots are included.
 */
int QMetaObject::findSlot( const char* n, bool super ) const
{
    const QMetaData *md = slotDict ? slotDict->find( n ) : 0;
    if ( md )
	return slotOffset() + ( md - slotData );
    if ( !super || !superclass)
	return -1;
    return superclass->findSlot( n, super );
}

/*!\internal
 */
QMetaObject *QMetaObject::new_metaobject( const char *classname,
					  QMetaObject *superclassobject,
					  const QMetaData *slot_data,	int n_slots,
					  const QMetaData *signal_data,int n_signals,
#ifndef QT_NO_PROPERTIES
					  const QMetaProperty *prop_data, int n_props,
					  const QMetaEnum *enum_data, int n_enums,
#endif
					  const QClassInfo * class_info, int n_info )
{
    return new QMetaObject( classname, superclassobject, slot_data, n_slots,
			    signal_data, n_signals,
#ifndef QT_NO_PROPERTIES
			    prop_data, n_props,
			    enum_data, n_enums,
#endif
			    class_info, n_info );
}


/*!\internal
 */
QMemberDict *QMetaObject::init( const QMetaData * data, int n )
{
    if ( n == 0 )				// nothing, then make no dict
	return 0;
    QMemberDict *dict = new QMemberDict( optDictSize(n), TRUE, FALSE );
    Q_CHECK_PTR( dict );
    while ( n-- ) {				// put all members into dict
	dict->insert( data->name, data );
	data++;
    }
    return dict;
}

/*!
  Returns the number of class information available for this class.

  If  \a super is TRUE, inherited class information is included.
 */
int QMetaObject::numClassInfo( bool super ) const
{
    return d->numClassInfo + ((super && superclass)?superclass->numClassInfo(super):0);
}

/*!
  Returns the class information with index \a index or 0 if no such
  information exists.

  If  \a super is TRUE,  inherited class information is included.
 */
const QClassInfo* QMetaObject::classInfo( int index, bool super ) const
{
    if ( index < 0 )
	return 0;
    if ( index < d->numClassInfo )
	return &(d->classInfo[ index ]);
    if ( !super || !superclass )
	return 0;
    return superclass->classInfo( index - d->numClassInfo, super );
}

/*!
    \overload
  Returns the class information with name \a name or 0 if no such
  information exists.

  If \a super is TRUE, inherited class information is included.
 */
const char* QMetaObject::classInfo( const char* name, bool super ) const
{
    for( int i = 0; i < d->numClassInfo; ++i ) {
	if ( qstrcmp( d->classInfo[i].name, name ) == 0 )
	    return d->classInfo[i].value;
    }
    if ( !super || !superclass )
	return 0;
    return superclass->classInfo( name, super );
}

#ifndef QT_NO_PROPERTIES

/*!
  Returns the number of properties for this class.

  If \a super is TRUE, inherited properties are included.

  \sa propertyNames()
 */
int QMetaObject::numProperties( bool super ) const	// number of signals
{
    int n = d->numPropData;
    if ( !super || !superclass )
	return n;
    return n + superclass->numProperties( super );
}

/*!
  Returns the property meta data for the property at index \a index
  or 0 if no such property exists.

  If \a super is TRUE, inherited properties are included.

  \sa propertyNames()
 */
const QMetaProperty* QMetaObject::property( int index, bool super ) const
{
    int idx = index - ( super ? propertyOffset() : 0 );
    if ( d->propData && idx >= 0 && idx < (int)d->numPropData )
	return d->propData + idx;
    if ( !super || !superclass )
	return 0;
    return superclass->property( index, super );
}


/*!
  Returns the property meta data for the property with name \a name
  or 0 if no such property exists.

  If \a super is TRUE, inherited properties are included.

  \sa property(), propertyNames()
*/

int QMetaObject::findProperty( const char *name, bool super ) const
{
    for( int i = 0; i < d->numPropData; ++i ) {
	if ( d->propData[i].isValid() && qstrcmp( d->propData[i].name(), name ) == 0 ) {
	    return ( super ? propertyOffset() : 0 ) + i;
	}
    }
    if ( !super || !superclass )
	return -1;
    return superclass->findProperty( name, super );
}

/*!
  Returns a list with the names of all properties for this class.

  If \a super is TRUE, inherited properties are included.

  \sa property()
 */
QStrList QMetaObject::propertyNames( bool super ) const
{
    QStrList l( FALSE );

    if ( superclass && super ) {
	QStrList sl = superclass->propertyNames( super );
	for ( QStrListIterator slit( sl ); slit.current(); ++slit )
	    l.append( slit.current() );
    }

    for( int i = 0; i < d->numPropData; ++i ) {
	if ( d->propData[i].isValid() )
	    l.append( d->propData[i].name() );
    }

    return l;
}

/*!
  Returns a list with the names of all signals for this class.

  If \a super is TRUE, inherited signals are included.

 */
QStrList QMetaObject::signalNames( bool super ) const
{
    QStrList l( FALSE );
    int n = numSignals( super );
    for( int i = 0; i < n; ++i ) {
	l.append( signal(i, super)->name );
    }
    return l;
}

/*!
  Returns a list with the names of all slots for this class.

  If \a super is TRUE, inherited slots are included.

  \sa numSlots()
 */
QStrList QMetaObject::slotNames( bool super ) const
{
    QStrList l( FALSE );
    int n = numSlots( super );
    for( int i = 0; i < n; ++i )
	l.append( slot( i, super)->name );
    return l;
}

/*!\internal
 */
const QMetaEnum* QMetaObject::enumerator( const char* name, bool super ) const
{
    for( int i = 0; i < d->numEnumData; ++i )
	if ( qstrcmp( d->enumData[i].name, name ) == 0 )
	    return &(d->enumData[i]);
    if ( !super || !superclass )
	return 0;
    return superclass->enumerator( name, super );
}

#endif // QT_NO_PROPERTIES


/*!
  Returns TRUE if this class inherits \a clname within the meta
  object inheritance chain.

  (A class is considered to inherit itself.)
 */
bool QMetaObject::inherits( const char* clname ) const
{
    const QMetaObject *meta = this;
    while ( meta ) {
	if ( qstrcmp(clname, meta->className()) == 0 )
	    return TRUE;
	meta = meta->superclass;
    }
    return FALSE;
}

/*! \internal */

QMetaObject *QMetaObject::metaObject( const char *class_name )
{
    if ( !qt_metaobjects )
	return 0;
    return qt_metaobjects->find( class_name );
}


#ifndef QT_NO_PROPERTIES
/*!
  \class QMetaProperty qmetaobject.h

  \brief The QMetaProperty class stores meta data about a property.
  \ingroup objectmodel

  Property meta data includes type(), name(), and whether a property is
  writable(), designable() and stored().

  The functions isSetType(), isEnumType() and enumKeys() provide
  further information about a property's type. The conversion
  functions keyToValue(), valueToKey(), keysToValue() and
  valueToKeys() allow conversion between the integer representation of
  an enumeration or set value and its literal representation.

  Actual property values are set and received through QObject's set
  and get functions.  See QObject::setProperty() and
  QObject::property() for details.

  You receive meta property data through an object's meta object. See
  QMetaObject::property() and QMetaObject::propertyNames() for details.
*/

/*!
  Returns the possible enumeration keys if this property is an
  enumeration type (or a set type).

  \sa isEnumType()
*/
QStrList QMetaProperty::enumKeys() const
{
     QStrList l( FALSE );
     if ( enumData != 0 ) {
	 for( uint i = 0; i < enumData->count; ++i )
	     l.append( enumData->items[i].key );
     }
     return l;
}

/*!
  Converts the enumeration key \a key to its integer
  value.

  For set types, use keysToValue().

\sa valueToKey(), isSetType(), keysToValue()
 */
int QMetaProperty::keyToValue( const char* key ) const
{
    if ( !isEnumType() )
	return -1;

    for( uint i = enumData->count; i > 0; --i ) {
	if ( !qstrcmp( key, enumData->items[i-1].key) )
	    return enumData->items[i-1].value;
    }
    return -1;
}

/*!
  Converts the enumeration value \a value to its literal key.

  For set types, use valueToKeys().

\sa valueToKey(), isSetType(), valueToKeys()
 */
const char* QMetaProperty::valueToKey( int value ) const
{
    if ( !isEnumType() )
	return 0;

    for( uint i = enumData->count; i > 0; --i ) {
	if ( value == enumData->items[i-1].value )
	    return enumData->items[i-1].key ;
    }
    return 0;
}

/*!
  Converts the list of keys \a keys to their combined integer
  value.

\sa isSetType(), valueToKey(), keysToValue()
 */
int QMetaProperty::keysToValue( const QStrList& keys ) const
{
    if ( !isEnumType() )
	return -1;

    int value = 0;
    for ( QStrListIterator it( keys ); it.current(); ++it ) {
	value |= keyToValue( it.current() );
    }
    return value;
}

/*!
  Converts the set value \a value to a list of keys.

\sa isSetType(), valueToKey(), valueToKeys()
 */
QStrList QMetaProperty::valueToKeys( int value ) const
{
    QStrList keys;

    if ( !isEnumType() )
	return keys;

    for( uint i = enumData->count; i > 0; --i ) {
	int k = enumData->items[i-1].value;
	if ( (value & k) == k ) {
	    value = value & ~k;
	    keys.append( enumData->items[i-1].key );
	}
    }
    return keys;
}



/*! \internal

  Constructs a meta property.
 */
QMetaProperty::QMetaProperty()
    :t(0),n(0),id(-1),p(0),
     enumData(0),flags(0)
{
}

/*! \internal

  Destroys a meta property.
 */
QMetaProperty::~QMetaProperty()
{
}

/*! \fn const char* QMetaProperty::type() const

  Returns the type of the property.
 */

/*! \fn const char* QMetaProperty::name() const

  Returns the name of the property.
 */

/*! \fn bool QMetaProperty::writable() const

  Returns whether the property is writable or not.

*/

/*! \fn bool QMetaProperty::isValid() const

  \internal

  Returns whether the property is valid.
*/

/*! \fn bool QMetaProperty::isEnumType() const

  Returns whether the property's type is an enumeration value.

  \sa isSetType(), enumKeys()
*/

/*! \fn bool QMetaProperty::isSetType() const

  Returns whether the property's type is an enumeration value that is
  used as set, i.e. whether the enumeration values can be OR'ed together.  A
  set type is implicitly also an enum type.

  \sa isEnumType(), enumKeys()
*/

/*! \fn bool QMetaProperty::testFlags( uint f ) const

  \internal
*/
/*! \fn void QMetaProperty::setFlags( uint f )

  \internal
 */
/*! \fn void QMetaProperty::clearFlags( uint f )

  \internal
*/
/*! \fn void QMetaProperty::copyFlags( uint mask )

  \internal
*/


/*!
  Returns TRUE if the property is designable for object \a o;
  otherwise returns FALSE.
 */
bool QMetaProperty::designable( QObject* o ) const
{
    if ( !o || !testFlags( Readable) || !testFlags(Writable) )
	return FALSE;
    return o->qt_property( this, 3, 0 );
}

/*!
  Returns TRUE if the property is scriptable for object \a o;
  otherwise returns FALSE.
 */
bool QMetaProperty::scriptable( QObject* o ) const
{
    return o->qt_property( this, 4, 0 );
}

/*!
  Returns TRUE if the property shall be stored for object \a o;
  otherwise returns FALSE.
 */
bool QMetaProperty::stored( QObject* o ) const
{
    if ( !o || !testFlags( Writable ) || !testFlags( Readable ) )
	return FALSE;
    return o->qt_property( this, 5, 0 );
}


/*!
  Tries to reset the property for object \a o with a reset method. On
  success, returns TRUE; otherwise returns FALSE.

  Reset methods are optional, usually only a few properties support
  them.
 */
bool QMetaProperty::reset( QObject* o ) const
{
    return o->qt_property( this, 2, 0 );
}

/*! \enum QMetaProperty::Flags

  \internal
*/

#endif // QT_NO_PROPERTIES

/*
 * QMetaObjectCleanUp is used as static global object in the moc-generated cpp
 * files and deletes the QMetaObject provided with setMetaObject. It sets the
 * QObject reference to the metaObj to NULL when it is destroyed.
 */
QMetaObjectCleanUp::QMetaObjectCleanUp()
: metaObject( 0 )
{
}

QMetaObjectCleanUp::~QMetaObjectCleanUp()
{
    if ( metaObject ) {
	delete *metaObject;
	*metaObject = 0;
	metaObject = 0;
    }
}

void QMetaObjectCleanUp::setMetaObject( QMetaObject *&mo )
{
#if defined(QT_CHECK_RANGE)
    if ( metaObject )
	qWarning( "QMetaObjectCleanUp::setMetaObject: Double use of QMetaObjectCleanUp!" );
#endif
    metaObject = &mo;
}
