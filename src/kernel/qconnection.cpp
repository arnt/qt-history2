/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnection.cpp#22 $
**
** Implementation of QConnection class
**
** Created : 930417
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qconnection.h"

/*!
  \class QConnection qconnection.h

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
