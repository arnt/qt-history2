/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobj.h#2 $
**
** Definition of QMetaObject class
**
** Author  : Haavard Nord
** Created : 930419
**
** Copyright (C) 1993,1994 by Troll Tech AS.  All rights reserved.
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
friend class QObject;
public:
    QMetaObject( const char *class_name, const char *superclass_name,
		 QMetaData *slot_data,	int n_slots,
		 QMetaData *signal_data, int n_signals );
   ~QMetaObject();

    const char *getClassName()	       const { return className; }
    const char *getSuperclassName()    const { return superclassName; }

    int		nSlots( bool=FALSE )   const;
    int		nSignals( bool=FALSE ) const;

    QMetaData  *slot( const char *, bool=FALSE )   const;
    QMetaData  *signal( const char *, bool=FALSE ) const;

    QMetaData  *slot( int index, bool=FALSE )	   const;
    QMetaData  *signal( int index, bool=FALSE )	   const;

private:
    QMemberDict*init( QMetaData *, int );
    QMetaData  *mdata( int code, const char *, bool ) const;
    QMetaData  *mdata( int code, int, bool ) const;

    char       *className;			// class name
    char       *superclassName;			// super class name
    QMetaObject*superMetaObject;		// super meta object
    QMetaData  *slotData;			// slot meta data
    QMemberDict*slotDict;			// slot dictionary
    QMetaData  *signalData;			// signal meta data
    QMemberDict*signalDict;			// signal dictionary
};


#endif // QMETAOBJ_H
