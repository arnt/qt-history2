/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnect.h#1 $
**
** Definition of QConnection class
**
** Author  : Haavard Nord
** Created : 930417
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
** --------------------------------------------------------------------------
** The QConnection class contains a signal coupling. A signal can be connected
** to a slot in the parent part or a method in a sibling object.
*****************************************************************************/

#ifndef QCONNECT_H
#define QCONNECT_H

#include "qobject.h"


typedef void (QObject::*QMember)();		// pointer to member function


class QConnection				// signal coupling
{
public:
    QConnection( const QObject *, QMember );
   ~QConnection() {}

    bool     isConnected() const { return obj != 0; }

    QObject *object() const { return obj; }	// get object/member pointer
#if defined(_CC_SUN_)
    QMember *member() const			// avoid warning
			{ QConnection *c=(QConnection*)this; return &c->mbr; }
#else
    QMember *member() const { return (QMember*)&mbr; }
#endif

    bool     connect( const QObject *, QMember);// connect object/member
    bool     disconnect();			// disconnect current coupling

private:
    QObject *obj;				// object connected to
    QMember  mbr;				// member connected to
};


#endif // QCONNECT_H
