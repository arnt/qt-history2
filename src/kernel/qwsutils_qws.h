/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwscommand_qws.cpp#3 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
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
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QWSUTILS_H
#define QWSUTILS_H

#ifndef QT_H
#include <qwssocket_qws.h>
#endif // QT_H

/********************************************************************
 *
 * Convinient socket functions
 *
 ********************************************************************/

inline int qws_read_uint( QWSSocket *socket )
{
    if ( !socket || socket->size() < sizeof( int ) )
	return -1;

    int i;
    socket->readBlock( (char*)&i, sizeof( i ) );

    return i;
}

inline void qws_write_uint( QWSSocket *socket, int i )
{
    if ( !socket )
	return;

    socket->writeBlock( (char*)&i, sizeof( i ) );
}

#endif
