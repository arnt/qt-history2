/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobject.h#20 $
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
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
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
    char   *name;				// - member name
    QMember ptr;				// - member pointer
};


class Q_EXPORT QMetaObject				// meta object class
{
public:
    QMetaObject( const char *class_name, const char *superclass_name,
		 QMetaData *slot_data,	int n_slots,
		 QMetaData *signal_data, int n_signals );
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
    void        *reservedForPropData;		// todo
    void        *reservedForPropDict;		// todo
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

class Q_EXPORT QMetaObjectInit {
public:
    QMetaObjectInit(void(*f)());
    static int init();
};


#endif // QMETAOBJECT_H
