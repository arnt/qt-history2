/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnection.cpp#4 $
**
** Implementation of QConnection class
**
** Author  : Haavard Nord
** Created : 930417
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qconnect.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qconnection.cpp#4 $";
#endif


QConnection::QConnection( const QObject *object, QMember member,
			  const char *memberName )
{
    obj = (QObject *)object;
    mbr = member;
    mbr_name = memberName;
}
