/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qconnection.cpp#25 $
**
** Implementation of QConnection class
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

#include "qconnection.h"

// REVISED we will not include this in the external doc any more
/*! \class QConnection qconnection.h

  \internal

  \brief The QConnection class is an internal class, used in the
  signal/slot mechanism.

  Do not use this class directly in application programs.

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
