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
#endif // QT_H


struct QMetaData				// member function meta data
{						//   for signal and slots
    const char *name;				// - member name
    QMember ptr;				// - member pointer
};

class QPixmap;
// Takes a pointer to the parent as parameter
typedef QObject* (*QObjectFactory)( QObject* );

struct QMetaEnumerator
{
    char	*name;
    int		 value;
};

struct QMetaEnum
{
    char	*name;
    int		 nEnumerators;
    QMetaEnumerator *enumerators;

    QStringList enumeratorNames();
};

struct QMetaProperty
{
    QMetaProperty()
	:name(0),set(0),get(0),type(0),enumType(0),
	 sspec(Unspecified),gspec(Unspecified),state(0)
    {
    }
    
    char        *name;
    QMember      set;
    QMember      get;
    char        *type;
    QMetaEnum   *enumType;
    bool readOnly() { return get != 0 && set == 0; }
    bool writeOnly() { return get == 0 && set != 0; }
    bool readWrite() { return get != 0 && set != 0; }
    bool isValid() { return !testState( UnresolvedEnum) ; }
    
    enum Specification  { Unspecified, Class, Reference, Pointer, ConstCharStar };
    
    Specification sspec;
    Specification gspec;
    
    enum State  {
	UnresolvedEnum = 0x00000001
    };
    
    inline bool testState( State s ) const
	{ return ((uint)state & s) == (uint)s; }
    inline void setState( State s )
	{ state |= s; }
    inline void clearState( State s )
	{ state &= ~s; }
    
    uint state;
};

struct QMetaMetaProperty
{
    const char *name;
    const char *value;
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
		 QMetaEnum *enum_data, int n_enums,
		 QMetaMetaProperty* meta_prop_data, int n_meta_props );

    virtual ~QMetaObject();

    const char	*className()		const { return classname; }
    const char	*superClassName()	const { return superclassname; }

    QMetaObject *superClass()		const { return superclass; }

    int		 nSlots( bool=FALSE )	const;
    int		 nSignals( bool=FALSE ) const;

    QMetaData	*slot( const char *, bool=FALSE )   const;
    QMetaData	*signal( const char *, bool=FALSE ) const;

    QMetaData	*slot( int index, bool=FALSE )	    const;
    QMetaData	*signal( int index, bool=FALSE )    const;

    bool inherits( const char* clname ) const;

    int            nProperties( bool=FALSE ) const;
    QMetaProperty *property( int index ) const;
    QMetaProperty *property( const char* name, bool super = FALSE ) const;
    QStringList    propertyNames( bool=TRUE ) const;
    QMetaEnum     *enumerator( const char* name, bool super = FALSE ) const;

    int		       nMetaProperties( bool=FALSE ) const;
    QMetaMetaProperty *metaProperty( int index ) const;
    const char        *metaProperty( const char* name, bool super = FALSE ) const;
    QStringList        metaPropertyNames( bool=TRUE ) const;

    void setFactory( QObjectFactory f );
    QObjectFactory factory() const;
    void resolveProperty( QMetaProperty* prop );

    static QMetaObject *new_metaobject( const char *, const char *,
					QMetaData *, int,
					QMetaData *, int,
					QMetaProperty *prop_data, int n_props,
					QMetaEnum *enum_data, int n_enums,
					QMetaMetaProperty* meta_prop_data, int n_meta_props );

    static QMetaObject *new_metaobject( const char *, const char *,
					QMetaData *, int,
					QMetaData *, int );
    static QMetaData   *new_metadata( int );

private:
    QMemberDict *init( QMetaData *, int );
    QMetaData	*mdata( int code, const char *, bool ) const;
    QMetaData	*mdata( int code, int, bool ) const;

    char	*classname;			// class name
    char	*superclassname;		// super class name
    QMetaObject *superclass;			// super class meta object
    QMetaObjectPrivate   *d;
    void        *reserved;
    QMetaData	*slotData;			// slot meta data
    QMemberDict *slotDict;			// slot dictionary
    QMetaData	*signalData;			// signal meta data
    QMemberDict *signalDict;			// signal dictionary

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMetaObject( const QMetaObject & );
    QMetaObject &operator=( const QMetaObject & );
#endif
};

class Q_EXPORT QMetaObjectInit
{
public:
    // ## To disappear in Qt 3.0
    QMetaObjectInit(void(*f)());
    QMetaObjectInit(QMetaObject*(*f)());

    static int init();
    static QMetaObject* metaObject( const char* classname );
    static QMetaObject* metaObject( int index );
    static int          nMetaObjects();
};

#endif // QMETAOBJECT_H
