/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobject.h#7 $
**
** Definition of QMetaObject class
**
** Created : 930419
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QMETAOBJ_H
#define QMETAOBJ_H

#include "qconnect.h"


struct QMetaData				// member function meta data
{						//   for signal and slots
    char   *name;				// - member name
    QMember ptr;				// - member pointer
};


class QMetaObject				// meta object class
{
public:
    QMetaObject( const char *class_name, const char *superclass_name,
		 QMetaData *slot_data,	int n_slots,
		 QMetaData *signal_data, int n_signals );
   ~QMetaObject();

    const char	*className()		const { return classname; }
    const char	*superClassName()	const { return superclassname; }

    QMetaObject *superClass()		const { return superclass; }

    int		 nSlots( bool=FALSE )	const;
    int		 nSignals( bool=FALSE ) const;

    QMetaData	*slot( const char *, bool=FALSE )   const;
    QMetaData	*signal( const char *, bool=FALSE ) const;

    QMetaData	*slot( int index, bool=FALSE )	    const;
    QMetaData	*signal( int index, bool=FALSE )    const;

private:
    QMemberDict *init( QMetaData *, int );
    QMetaData	*mdata( int code, const char *, bool ) const;
    QMetaData	*mdata( int code, int, bool ) const;

    char	*classname;			// class name
    char	*superclassname;		// super class name
    QMetaObject *superclass;			// super class meta object
    QMetaData	*slotData;			// slot meta data
    QMemberDict *slotDict;			// slot dictionary
    QMetaData	*signalData;			// signal meta data
    QMemberDict *signalDict;			// signal dictionary

private:	// Disabled copy constructor and operator=
    QMetaObject( const QMetaObject & ) {}
    QMetaObject &operator=( const QMetaObject & ) { return *this; }
};


#endif // QMETAOBJ_H
