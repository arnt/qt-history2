/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnection.cpp#2 $
**
** Implementation of QConnection class
**
** Author  : Haavard Nord
** Created : 930417
**
** Copyright (C) 1993,1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qconnect.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qconnection.cpp#2 $";
#endif


QConnection::QConnection( const QObject *object, QMember member )
{
    obj = (QObject *)object;
    mbr = member;
}


bool QConnection::connect( const QObject *object, QMember member )
{						// connect object/member
    obj = (QObject *)object;
    mbr = member;
    return TRUE;
}

bool QConnection::disconnect()			// disconnect from signal
{
    obj = 0;
    mbr = 0;
    return TRUE;
}
