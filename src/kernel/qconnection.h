/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnection.h#14 $
**
** Definition of QConnection class
**
** Created : 930417
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
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

    QObject *object() const  { return obj; }	// get object/member pointer
    QMember *member() const  { return (QMember*)&mbr; }
    const char *memberName() const { return mbr_name; }
    int	     numArgs() const { return nargs; }

private:
    QObject *obj;				// object connected to
    QMember  mbr;				// member connected to
    const char *mbr_name;
    int	     nargs;

private:	// Disabled copy constructor and operator=
    QConnection( const QConnection & );
    QConnection &operator=( const QConnection & );
};


#endif // QCONNECT_H
