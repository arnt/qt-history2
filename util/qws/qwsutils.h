/****************************************************************************
** $Id: //depot/qt/main/util/qws/qwscommand.cpp#3 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QWSUTILS_H
#define QWSUTILS_H

#include "../../extensions/network/src/qsocket.h"

#include <stdlib.h>
#include <stdio.h>

/********************************************************************
 *
 * Helper functions
 *
 ********************************************************************/

static char *int_to_hex( int i )
{
    char *s = new char[ 4 ];
    sprintf( s, "%04X", i );
    return s;
}

static ushort hex_ushort_to_int( ushort c )
{
    if ( c >= 'A' && c <= 'F')
	return c - 'A' + 10;
    if ( c >= 'a' && c <= 'f')
	return c - 'a' + 10;
    if ( c >= '0' && c <= '9')
	return c - '0';
    return 0;
}

static int hex_to_int( char *array )
{
    return ( 16 * 16 * 16 * hex_ushort_to_int( array[ 0 ] ) +
	     16 * 16 * hex_ushort_to_int( array[ 1 ] ) +
	     16 * hex_ushort_to_int( array[ 2 ] ) +
	     hex_ushort_to_int( array[ 3 ] ) );
}

/********************************************************************
 *
 * Convinient socket functions
 *
 ********************************************************************/

static int qws_read_uint( QSocket *socket )
{
    if ( !socket )
	return -1;

    int i;
    socket->readBlock( (char*)&i, sizeof( int ) );

    return hex_to_int( (char*)&i );
}

static void qws_write_uint( QSocket *socket, int i )
{
    if ( !socket )
	return;

    char *s = int_to_hex( i );
    socket->writeBlock( s, strlen( s ) );
    delete [] s;
}

#endif
