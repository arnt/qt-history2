/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnection.cpp#16 $
**
** Implementation of QConnection class
**
** Created : 930417
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qconnect.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qconnection.cpp#16 $");


/*!
  \class QConnection qconnect.h

  \brief The QConnection class is an internal class, used in the
  signal/slot mechanism.

  Do not use this class directly in application programs.

  \internal
  QObject has a list of QConnection for each signal that is connected to the
  outside world.
*/

/*!
  \internal
*/
QConnection::QConnection( const QObject *object, QMember member,
			  const char *memberName )
{
    obj = (QObject *)object;
    mbr = member;
    mbr_name = memberName;
    nargs = 0;
    if ( strstr(memberName,"()") == 0 ) {
        const char *p = memberName;
	nargs++;
	while ( *p ) {
	    if ( *p++ == ',' )
		nargs++;
	}
    }
}

/*!
 \fn QConnection::~QConnection()
 \internal
*/

/*!
  \fn bool QConnection::isConnected() const
  \internal
*/

/*!
  \fn QObject *QConnection::object() const
  \internal
*/

/*!
  \fn QMember *QConnection::member() const
  \internal
*/

/*!
  \fn const char *QConnection::memberName() const
  \internal
*/

/*!
  \fn int QConnection::numArgs() const
  \internal
*/
