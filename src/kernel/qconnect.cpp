/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnect.cpp#6 $
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
static char ident[] = "$Id: //depot/qt/main/src/kernel/qconnect.cpp#6 $";
#endif

/*! \class QConnection qconnect.h

  \brief The QConnection class is an internal class, used in the
  signal/slot mechanism.

  \internal

  Sex! Haavard se her! Fett YO! Eirik se her! */


QConnection::QConnection( const QObject *object, QMember member,
			  const char *memberName )
{
    obj = (QObject *)object;
    mbr = member;
    mbr_name = memberName;
}
