/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobject.h#27 $
**
** Definition of QMetaObject class
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

#ifndef QMETAOBJECT_H
#define QMETAOBJECT_H

#ifndef QT_H
#include "qconnection.h"
#include "qstrlist.h"
#endif // QT_H


struct QMetaData				// member function meta data
{						//   for signal and slots
    const char *name;				// - member name
    QMember ptr;				// - member pointer
};


struct QMetaEnum 				// enumerator meta data
{						//  for properties
    ~QMetaEnum() { delete [] items; }
    const char *name;				// - enumerator name
    uint count;					// - number of values
    struct Item 				// - a name/value pair
    {
	const char *key;
	int value;
    };
    Item *items;				// - the name/value pairs
};

class QMetaProperty 				// property meta data
{
public:
    QMetaProperty()
	:t(0),n(0),
	 get(0),set(0),enumData(0),
	 gspec(Unspecified),sspec(Unspecified),
	 flags(0)
    {
    }

    const char 	*type() const { return t; }		// type of the property
    const char*	name() const { return n; }		// name of the property

    bool writeable() const { return set != 0; }
    bool isValid() const { return get != 0 && !testFlags( UnresolvedEnum) ; }

    bool isEnumType() const { return enumData != 0; }
    QStrList enumKeys() const;			// enumeration names

    const char* t;
    const char* n;
    QMember 	get;				// get-function or 0 ( 0 indicates an error )
    QMember 	set;				// set-function or 0
    QMetaEnum	*enumData; 			// a pointer to the enum specification or 0
    
    enum Specification  { Unspecified, Class, Reference, Pointer, ConstCharStar };
    Specification gspec;			// specification of the get-function
    Specification sspec;			// specification of the set-function

    enum Flags  {
	UnresolvedEnum = 0x00000001
    };

    inline bool testFlags( Flags f ) const
	{ return (flags & (uint)f) == (uint)f; }
    inline void setFlags( Flags f )
	{ flags |= (uint)f; }
    inline void clearFlags( Flags f )
	{ flags &= ~(uint)f; }

private:
    uint flags;
};

struct QClassInfo 				// class info meta data
{
    const char* name;				// - name of the info
    const char* value;				// - value of the info
};

class QMetaObjectPrivate;

class Q_EXPORT QMetaObject			// meta object class
{
public:
    QMetaObject( const char *class_name, const char *superclass_name,
		 QMetaData *slot_data,	int n_slots,
		 QMetaData *signal_data, int n_signals );
    QMetaObject( const char *class_name, const char *superclass_name,
		 QMetaData *slot_data,	int n_slots,
		 QMetaData *signal_data, int n_signals,
		 QMetaProperty *prop_data, int n_props,
		 QMetaEnum *enum_data, int n_enums,
		 QClassInfo *class_info, int n_info );


    virtual ~QMetaObject();

    const char	*className()		const { return classname; }
    const char	*superClassName()	const { return superclassname; }

    QMetaObject *superClass()		const { return superclass; }

    bool 	inherits( const char* clname ) const;

    int  	numSlots( bool super = FALSE ) const;
    int		numSignals( bool super = FALSE ) const;

    QMetaData	*slot( int index, bool super = FALSE ) const;
    QMetaData	*signal( int index, bool super = FALSE ) const;

    QMetaData	*slot( const char *, bool super = FALSE ) const;
    QMetaData	*signal( const char *, bool super = FALSE ) const;

    QStrList	slotNames( bool super = FALSE ) const;
    QStrList	signalNames( bool super = FALSE ) const;

    int		numClassInfo( bool super = FALSE ) const;
    QClassInfo 	*classInfo( int index, bool super = FALSE ) const;
    const char 	*classInfo( const char* name, bool super = FALSE ) const;

    const QMetaProperty	*property( const char* name, bool super = FALSE ) const;
    QStrList		propertyNames( bool super = FALSE ) const;
    void		resolveProperty( QMetaProperty* prop );

    // static wrappers around constructors, necessary to work around a
    // Windows-DLL limitation: objects can only be deleted within a
    // DLL if they were actually created within that DLL.
    static QMetaObject	*new_metaobject( const char *, const char *,
					QMetaData *, int,
					QMetaData *, int,
					QMetaProperty *prop_data, int n_props,
					QMetaEnum *enum_data, int n_enums,
					QClassInfo * class_info, int n_info );
    static QMetaObject	*new_metaobject( const char *, const char *,
					QMetaData *, int,
					QMetaData *, int );
    static QMetaData		*new_metadata( int );
    static QMetaEnum 		*new_metaenum( int );
    static QMetaEnum::Item 	*new_metaenum_item( int );
    static QMetaProperty 	*new_metaproperty( int );
    static QClassInfo 		*new_classinfo( int );

private:
    QMemberDict		*init( QMetaData *, int );
    QMetaData		*mdata( int code, const char *, bool ) const;
    QMetaData		*mdata( int code, int index, bool super ) const;

    const char		*classname;			// class name
    const char		*superclassname;		// super class name
    QMetaObject 	*superclass;			// super class meta object
    QMetaObjectPrivate	*d;				// private data for...
    void        	*reserved;			// ...binary compatibility
    QMetaData		*slotData;			// slot meta data
    QMemberDict 	*slotDict;			// slot dictionary
    QMetaData		*signalData;			// signal meta data
    QMemberDict 	*signalDict;			// signal dictionary
    QMetaEnum		*enumerator( const char* name, bool super = FALSE ) const;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMetaObject( const QMetaObject & );
    QMetaObject &operator=( const QMetaObject & );
#endif
};

#endif // QMETAOBJECT_H
