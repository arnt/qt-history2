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
    char *s = new char[ 8 ];
    sprintf( s, "%08X", i );
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
    return ( 16 * 16 * 16 * 16 * 16 * 16 * 16 * hex_ushort_to_int( array[ 0 ] ) +
	     16 * 16 * 16 * 16 * 16 * 16 * hex_ushort_to_int( array[ 1 ] ) +
	     16 * 16 * 16 * 16 * 16 * hex_ushort_to_int( array[ 2 ] ) + 
	     16 * 16 * 16 * 16 * hex_ushort_to_int( array[ 3 ] ) +
	     16 * 16 * 16 * hex_ushort_to_int( array[ 4 ] ) +
	     16 * 16 * hex_ushort_to_int( array[ 5 ] ) +
	     16 * hex_ushort_to_int( array[ 6 ] ) +
	     hex_ushort_to_int( array[ 7 ] ) );
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

    char i[ 8 ];
    socket->readBlock( i, 8 );

    return hex_to_int( i );
}

static void qws_write_uint( QSocket *socket, int i )
{
    if ( !socket )
	return;

    char *s = int_to_hex( i );
    socket->writeBlock( s, 8 );
    delete [] s;
}

#endif
