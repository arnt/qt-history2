/****************************************************************************
** $Id: $
**
** Definition of QMetaObject class
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

#ifndef QMETAOBJECT_H
#define QMETAOBJECT_H

#ifndef QT_H
#include "qconnection.h"
#include "qstrlist.h"
#endif // QT_H

#ifndef Q_MOC_OUTPUT_REVISION
#define Q_MOC_OUTPUT_REVISION 16
#endif

class QObject;
struct QUMethod;
class QMetaObjectPrivate;

struct QMetaData				// - member function meta data
{						//   for signal and slots
    const char *name;				// - member name
    const QUMethod* method;			// - detailed method description
    enum Access { Private, Protected, Public };
    Access access;				// - access permission
};

#ifndef QT_NO_PROPERTIES
struct QMetaEnum				// enumerator meta data
{						//  for properties
    const char *name;				// - enumerator name
    uint count;					// - number of values
    struct Item					// - a name/value pair
    {
	const char *key;
	int value;
    };
    const Item *items;				// - the name/value pairs
    bool set;					// whether enum has to be treated as a set
};
#endif

#ifndef QT_NO_PROPERTIES

class Q_EXPORT QMetaProperty			// property meta data
{
public:
    QMetaProperty();
    ~QMetaProperty();

    const char*	type() const { return t; }	// type of the property
    const char*	name() const { return n; }	// name of the property

    bool writable() const;
    bool isValid() const;

    bool isSetType() const;
    bool isEnumType() const;
    QStrList enumKeys() const;			// enumeration names

    int keyToValue( const char* key ) const;	// enum and set conversion functions
    const char* valueToKey( int value ) const;
    int keysToValue( const QStrList& keys ) const;
    QStrList valueToKeys( int value ) const;

    bool designable( QObject* ) const;
    bool scriptable( QObject* ) const;
    bool stored( QObject* ) const;

    bool reset( QObject* ) const;

    const char* t;			//internal
    const char* n;			//internal
    int id;				//internal
    const QMetaProperty* p;		//internal
    const QMetaEnum *enumData;	// internal

    enum Flags  {
	Readable		= 0x00000001,
	Writable		= 0x00000002,
	EnumOrSet		= 0x00000004,
	StdSet			= 0x00000100
    };

    bool testFlags( uint f ) const;
    void setFlags( uint f );
    void copyFlags( uint mask );
    void clearFlags( uint f );

private:
    uint flags;
};
#endif // QT_NO_PROPERTIES

struct QClassInfo				// class info meta data
{
    const char* name;				// - name of the info
    const char* value;				// - value of the info
};

class Q_EXPORT QMetaObject			// meta object class
{
public:
    QMetaObject( const char *class_name, QMetaObject *superclass,
		 const QMetaData * const slot_data, int n_slots,
		 const QMetaData * const signal_data, int n_signals,
#ifndef QT_NO_PROPERTIES
		 const QMetaProperty *const prop_data, int n_props,
		 const QMetaEnum *const enum_data, int n_enums,
#endif
		 const QClassInfo *const class_info, int n_info );


    virtual ~QMetaObject();

    const char	*className()		const { return classname; }
    const char	*superClassName()	const { return superclassname; }

    QMetaObject *superClass()		const { return superclass; }

    bool	inherits( const char* clname ) const;

    int	numSlots( bool super = FALSE ) const;
    int		numSignals( bool super = FALSE ) const;

    int		findSlot( const char *, bool super = FALSE ) const;
    int		findSignal( const char *, bool super = FALSE ) const;

    const QMetaData 	*slot( int index, bool super = FALSE ) const;
    const QMetaData 	*signal( int index, bool super = FALSE ) const;

    QStrList	slotNames( bool super = FALSE ) const;
    QStrList	signalNames( bool super = FALSE ) const;

    int		slotOffset() const;
    int		signalOffset() const;
    int		propertyOffset() const;

    int		numClassInfo( bool super = FALSE ) const;
    const QClassInfo	*classInfo( int index, bool super = FALSE ) const;
    const char	*classInfo( const char* name, bool super = FALSE ) const;

#ifndef QT_NO_PROPERTIES
    const QMetaProperty	*property( int index, bool super = FALSE ) const;
    int findProperty( const char *name, bool super = FALSE ) const;
    QStrList		propertyNames( bool super = FALSE ) const;
    int		numProperties( bool super = FALSE ) const;
#endif

    // static wrappers around constructors, necessary to work around a
    // Windows-DLL limitation: objects can only be deleted within a
    // DLL if they were actually created within that DLL.
    static QMetaObject	*new_metaobject( const char *, QMetaObject *,
					const QMetaData *const, int,
					const QMetaData *const, int,
#ifndef QT_NO_PROPERTIES
					const QMetaProperty *const prop_data, int n_props,
					const QMetaEnum *const enum_data, int n_enums,
#endif
					const QClassInfo *const  class_info, int n_info );
#ifndef QT_NO_PROPERTIES
    const QMetaEnum		*enumerator( const char* name, bool super = FALSE ) const;
#endif

    static QMetaObject *metaObject( const char *class_name );

private:
    QMemberDict		*init( const QMetaData *, int );

    const char		*classname;			// class name
    const char		*superclassname;		// super class name
    QMetaObject	*superclass;			// super class meta object
    QMetaObjectPrivate	*d;			// private data for...
    void	*reserved;			// ...binary compatibility
    const QMetaData		*slotData;			// slot meta data
    QMemberDict	*slotDict;			// slot dictionary
    const QMetaData		*signalData;			// signal meta data
    QMemberDict	*signalDict;			// signal dictionary
    int signaloffset;
    int slotoffset;
#ifndef QT_NO_PROPERTIES
    int propertyoffset;
#endif

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMetaObject( const QMetaObject & );
    QMetaObject &operator=( const QMetaObject & );
#endif
};

inline int QMetaObject::slotOffset() const
{ return slotoffset; }

inline int QMetaObject::signalOffset() const
{ return signaloffset; }

#ifndef QT_NO_PROPERTIES
inline int QMetaObject::propertyOffset() const
{ return propertyoffset; }

inline bool QMetaProperty::testFlags( uint f ) const
{ return (flags & (uint)f) != (uint)0; }
inline bool QMetaProperty::writable() const
{ return testFlags( Writable ); }
inline bool QMetaProperty::isValid() const
{ return testFlags( Readable ); }
inline bool QMetaProperty::isSetType() const
{ return ( enumData != 0 && enumData->set ); }
inline bool QMetaProperty::isEnumType() const
{ return ( enumData != 0 ); }
inline void QMetaProperty::setFlags( uint f )
{ flags |= f; }
inline void QMetaProperty::copyFlags( uint mask )
{ if (p) {flags &= ~mask; flags |= ( p->flags & mask ); }}
inline void QMetaProperty::clearFlags( uint f )
{ flags &= ~f; }
#endif

class Q_EXPORT QMetaObjectCleanUp
{
public:
    QMetaObjectCleanUp();
    ~QMetaObjectCleanUp();

    void setMetaObject( QMetaObject *&mo );

private:
    QMetaObject **metaObject;
};

#endif // QMETAOBJECT_H
