/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnect.h#5 $
**
** Definition of QConnection class
**
** Author  : Haavard Nord
** Created : 930417
**
** Copyright (C) 1993,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QCONNECT_H
#define QCONNECT_H

#include "qobject.h"


typedef void (QObject::*QMember)();		// pointer to member function


class QConnection				// signal coupling
{
public:
    QConnection( const QObject *, QMember, const char *memberName );
   ~QConnection() {}

    bool     isConnected() const { return obj != 0; }

    QObject *object() const { return obj; }	// get object/member pointer
    QMember *member() const;
    const char *memberName() const { return mbr_name; }

private:
    QObject *obj;				// object connected to
    QMember  mbr;				// member connected to
    const char *mbr_name;
};

inline QMember *QConnection::member() const {
#if defined(_CC_SUN_)
    QConnection *c=(QConnection*)this;		// avoid warning
    return &c->mbr;
#else
    return (QMember*)&mbr;
#endif
}

#endif // QCONNECT_H
