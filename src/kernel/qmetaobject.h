/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobject.h#23 $
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

#ifdef QT_BUILDER

class QPixmap;
class QInspector;
typedef QInspector* (*QInspectorFactory)( QObject* );
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
    char        *name;
    QMember      set;
    QMember      get;
    char        *type;
    QMetaEnum   *enumType;
    bool         readonly;
    char         getSpec; // 'c' = class, 'r' = reference, 'p' = pointer, '!' = QObject::name exception
    char         setSpec; // 'c' = class, 'r' = reference, '!' = QObject::setName exception
};

#endif // QT_BUILDER

class Q_EXPORT QMetaObject				// meta object class
{
public:
    QMetaObject( const char *class_name, const char *superclass_name,
		 QMetaData *slot_data,	int n_slots,
		 QMetaData *signal_data, int n_signals );
#ifdef QT_BUILDER
    QMetaObject( const char *class_name, const char *superclass_name,
		 QMetaData *slot_data,	int n_slots,
		 QMetaData *signal_data, int n_signals,
		 QMetaProperty *prop_data = 0, int n_props = 0,
		 QMetaEnum *enum_data = 0, int n_enums = 0 );
#endif // QT_BUILDER

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

#ifdef QT_BUILDER
    bool inherits( const char* _class ) const;
    int            nProperties( bool=FALSE ) const;
    QMetaProperty *property( const char* name, bool super = FALSE ) const;
    QStringList    propertyNames();
    QMetaEnum     *enumerator( const char* name, bool super = FALSE ) const;
    QInspector    *inspector( QObject* ) const;
    QString	   comment() const;
    QPixmap	   pixmap() const;
    QObjectFactory factory() const;
    void setPixmap( const char* _pixmap[] );
    void setComment( const char* _comment );
    void setInspector( QInspectorFactory f );
    void setFactory( QObjectFactory f );
    void fixProperty( QMetaProperty* prop, bool fix_enum_type = FALSE );

    static QMetaObject *new_metaobject( const char *, const char *,
					QMetaData *, int,
					QMetaData *, int,
					QMetaProperty *prop_data = 0, int n_props = 0,
					QMetaEnum *enum_data = 0, int n_enums = 0 );
#else // QT_BUILDER
    static QMetaObject *new_metaobject( const char *, const char *,
					QMetaData *, int,
					QMetaData *, int );
#endif // QT_BUILDER
    static QMetaData   *new_metadata( int );

private:
    QMemberDict *init( QMetaData *, int );
    QMetaData	*mdata( int code, const char *, bool ) const;
    QMetaData	*mdata( int code, int, bool ) const;

    char	*classname;			// class name
    char	*superclassname;		// super class name
    QMetaObject *superclass;			// super class meta object
    void        *reservedForPropData;		// todo
    void        *reservedForPropDict;		// todo
    QMetaData	*slotData;			// slot meta data
    QMemberDict *slotDict;			// slot dictionary
    QMetaData	*signalData;			// signal meta data
    QMemberDict *signalDict;			// signal dictionary

#ifdef QT_BUILDER
    QMetaEnum     *enumData;
    int		   nEnumData;
    QMetaProperty *propData;                    // property meta data
    int            nPropData;
    QInspectorFactory  inspectorFactory;
    QObjectFactory     objectFactory;
    const char	*commentData;
    const char **pixmapData;
#endif // QT_BUILDER

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QMetaObject( const QMetaObject & );
    QMetaObject &operator=( const QMetaObject & );
#endif
};

#ifdef QT_BUILDER
class Q_EXPORT QMetaObjectInit
{
public:
    QMetaObjectInit(QMetaObject*(*f)());

    static int init();
    static QMetaObject* metaObject( const char* _class_name );
    static QMetaObject* metaObject( int index );
    static int          nMetaObjects();
};
#else // QT_BUILDER
class Q_EXPORT QMetaObjectInit {
public:
    QMetaObjectInit(void(*f)());
    static int init();
};
#endif // QT_BUILDER

#endif // QMETAOBJECT_H
