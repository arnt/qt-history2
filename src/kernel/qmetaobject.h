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


struct QMetaEnum
{
    const char *name;
    uint count;
    struct Item
    {
	const char *name;
	int value;
    };
    Item *items;
};

struct QMetaProperty
{
    QMetaProperty()
	:name(0),get(0),set(0),type(0),enumType(0),
	 gspec(Unspecified),sspec(Unspecified),state(0)
    {
    }

    const char*	name;
    QMember 	get;
    QMember 	set;
    const char 	*type;
    QMetaEnum	*enumType;

    bool readable() const { return get != 0; }
    bool writeable() const { return set != 0; }
    bool isValid() const { return !testState( UnresolvedEnum) ; }

    bool isEnumType() const { return enumType != 0; }
    QStrList enumNames() const;

    enum Specification  { Unspecified, Class, Reference, Pointer, ConstCharStar };

    Specification gspec;
    Specification sspec;

    enum State  {
	UnresolvedEnum = 0x00000001
    };

    inline bool testState( State s ) const
	{ return ((uint)state & s) == (uint)s; }
    inline void setState( State s )
	{ state |= s; }
    inline void clearState( State s )
	{ state &= ~s; }

private:
    uint state;
};

class QMetaObjectPrivate;

class Q_EXPORT QMetaObject				// meta object class
{
public:
    QMetaObject( const char *class_name, const char *superclass_name,
		 QMetaData *slot_data,	int n_slots,
		 QMetaData *signal_data, int n_signals );
    QMetaObject( const char *class_name, const char *superclass_name,
		 QMetaData *slot_data,	int n_slots,
		 QMetaData *signal_data, int n_signals,
		 QMetaProperty *prop_data, int n_props,
		 QMetaEnum *enum_data, int n_enums );


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

    QMetaProperty	*property( const char* name, bool super = FALSE ) const;
    QStrList		propertyNames( bool super = FALSE ) const;
    QMetaEnum		*enumerator( const char* name, bool super = FALSE ) const;

    static QMetaObject *new_metaobject( const char *, const char *,
					QMetaData *, int,
					QMetaData *, int,
					QMetaProperty *prop_data, int n_props,
					QMetaEnum *enum_data, int n_enums );

    static QMetaObject	*new_metaobject( const char *, const char *,
					QMetaData *, int,
					QMetaData *, int );
    static QMetaData	*new_metadata( int );

    void		resolveProperty( QMetaProperty* prop );

private:
    QMemberDict 	*init( QMetaData *, int );
    QMetaData		*mdata( int code, const char *, bool ) const;
    QMetaData		*mdata( int code, int index, bool super ) const;

    char		*classname;			// class name
    char		*superclassname;		// super class name
    QMetaObject 	*superclass;			// super class meta object
    QMetaObjectPrivate	*d;				// private data for...
    void        	*reserved;			// ...binary compatibility
    QMetaData		*slotData;			// slot meta data
    QMemberDict 	*slotDict;			// slot dictionary
    QMetaData		*signalData;			// signal meta data
    QMemberDict 	*signalDict;			// signal dictionary

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMetaObject( const QMetaObject & );
    QMetaObject &operator=( const QMetaObject & );
#endif
};

#endif // QMETAOBJECT_H
