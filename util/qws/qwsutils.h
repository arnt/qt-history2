/****************************************************************************
**
** Implementation of Qt/FB central server.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
** EDITIONS: PROFESSIONAL
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWSUTILS_H
#define QWSUTILS_H

#include "../../extensions/network/src/qsocket.h"

/********************************************************************
 *
 * Convinient socket functions
 *
 ********************************************************************/

static int qws_read_uint( QSocket *socket )
{
    if ( !socket || socket->size() < sizeof( int ) )
	return -1;

    int i;
    socket->readBlock( (char*)&i, sizeof( i ) );

    return i;
}

static void qws_write_uint( QSocket *socket, int i )
{
    if ( !socket )
	return;

    socket->writeBlock( (char*)&i, sizeof( i ) );
}

#endif
