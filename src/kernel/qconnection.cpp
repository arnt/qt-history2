/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnection.cpp#12 $
**
** Implementation of QConnection class
**
** Created : 930417
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qconnect.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qconnection.cpp#12 $");


/*!
  \class QConnection qconnect.h

  \brief The QConnection class is an internal class, used in the
  signal/slot mechanism.

  Do not use this class directly in application programs.

  \internal
  QObject has a list of QConnection for each signal that is connected to the
  outside world.
*/


QConnection::QConnection( const QObject *object, QMember member,
			  const char *memberName )
{
    obj = (QObject *)object;
    mbr = member;
    mbr_name = memberName;
}
