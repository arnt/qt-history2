/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnection.h#21 $
**
** Definition of QConnection class
**
** Created : 930417
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QCONNECTION_H
#define QCONNECTION_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H


typedef void (QObject::*QMember)();		// pointer to member function


class Q_EXPORT QConnection
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
#if defined(Q_DISABLE_COPY)
    QConnection( const QConnection & );
    QConnection &operator=( const QConnection & );
#endif
};


#endif // QCONNECTION_H
