/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmetaobject.h#1 $
**
** Definition of QMetaObject class
**
** Author  : Haavard Nord
** Created : 930419
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
** --------------------------------------------------------------------------
** The QMetaObject class contains meta information about existing objects.
*****************************************************************************/

#ifndef QMETAOBJ_H
#define QMETAOBJ_H

#include "qconnect.h"


struct QMetaData				// member function meta data
{						//   for methods, signal, slots
    char   *name;				// - member name
    QMember ptr;				// - member pointer
};

class QStrList;					// defined in: qstrlist.h


class QMetaObject				// meta object class
{
friend class QObject;
public:
    QMetaObject( const char *class_name, const char *superclass_name,
		 QMetaData *method_data, int n_methods,
		 QMetaData *slot_data,	int n_slots,
		 QMetaData *signal_data, int n_signals );
   ~QMetaObject();

    const char *getClassName()	       const { return className; }
    const char *getSuperclassName()    const { return superclassName; }

    int		nMethods( bool=FALSE ) const;
    int		nSlots( bool=FALSE )   const;
    int		nSignals( bool=FALSE ) const;

    QMember    *method( const char *, bool=FALSE )  const;
    QMember    *slot( const char *, bool=FALSE )    const;
    QMember    *signal( const char *, bool=FALSE )  const;

    void	getMethods( QStrList *, bool=FALSE) const;
    void	getSlots( QStrList *,bool=FALSE )   const;
    void	getSignals( QStrList *, bool=FALSE) const;

private:
    QMemberDict*init( QMetaData *, int );
    QMember    *member( int code, const char *, bool ) const;
    void	getNames( int code, QStrList *, bool ) const;

    char       *className;			// class name
    char       *superclassName;			// super class name
    QMetaObject*superMetaObject;		// super meta object
    QMetaData  *methodData;			// method meta data
    QMemberDict*methodDict;			// method dictionary
    QMetaData  *slotData;			// slot meta data
    QMemberDict*slotDict;			// slot dictionary
    QMetaData  *signalData;			// signal meta data
    QMemberDict*signalDict;			// signal dictionary
};


#endif // QMETAOBJ_H
