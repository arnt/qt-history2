/****************************************************************************
** $Id: //depot/qt/main/src/tools/qglobal.cpp#14 $
**
** Global functions
**
** Author  : Haavard Nord
** Created : 920604
**
** Copyright (C) 1992-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qglobal.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

RCSTAG("$Id: //depot/qt/main/src/tools/qglobal.cpp#14 $")


const char *qVersion() { return "0.93"; }

// --------------------------------------------------------------------------
// System detection routines
//

static bool si_alreadyDone = FALSE;
static int  si_wordSize;
static bool si_bigEndian;

bool qSysInfo( int *wordSize, bool *bigEndian )
{
    if ( si_alreadyDone ) {			// run it only once
	*wordSize = si_wordSize;
	*bigEndian = si_bigEndian;
	return TRUE;
    }
    si_alreadyDone = TRUE;

    si_wordSize = 1;
    uint n = (uint)(~0);
    while ( n >>= 1 )				// detect word size
	si_wordSize++;
    *wordSize = si_wordSize;

    if ( *wordSize != 32 && *wordSize != 16 ) { // word size should be 16 or 32
#if defined(CHECK_RANGE)
	fatal( "qSysInfo: Unsupported system word size %d", *wordSize );
#endif
	return FALSE;
    }
    if ( sizeof(INT8) != 1 || sizeof(INT16) != 2 || sizeof(INT32) != 4 ||
	 sizeof(float) != 4 || sizeof(double) != 8 ) {
#if defined(CHECK_RANGE)
	fatal( "qSysInfo: Unsupported system data type size" );
#endif
	return FALSE;
    }

    bool be16, be32;				// determine byte ordering
    short ns = 0x1234;
    long nl = 0x12345678;

    unsigned char *p = (unsigned char *)(&ns);	// 16-bit integer
    be16 = *p == 0x12;

    p = (unsigned char *)(&nl);			// 32-bit integer
    if ( p[0] == 0x12 && p[1] == 0x34 && p[2] == 0x56 && p[3] == 0x78 )
	be32 = TRUE;
    else
    if ( p[0] == 0x78 && p[1] == 0x56 && p[2] == 0x34 && p[3] == 0x12 )
	be32 = FALSE;
    else
	be32 = !be16;

    if ( be16 != be32 ) {			// strange machine!
#if defined(CHECK_RANGE)
	fatal( "qSysInfo: Inconsistent system byte order" );
#endif
	return FALSE;
    }

    *bigEndian = si_bigEndian = be32;
    return TRUE;
}


// --------------------------------------------------------------------------
// Debug output routines
//

static dbg_handler handler = 0;			// pointer to debug handler

void warning( const char *msg, ... )		// print message
{
    char buf[600];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsprintf( buf, msg, ap );
	(*handler)( buf );
    }
    else {
	vfprintf( stderr, msg, ap );
	fprintf( stderr, "\n" );		// add newline
    }
    va_end( ap );
}

void fatal( const char *msg, ... )		// print message and exit
{
    char buf[600];
    va_list ap;
    va_start( ap, msg );			// use variable arg list
    if ( handler ) {
	vsprintf( buf, msg, ap );
	(*handler)( buf );
    }
    else {
	vfprintf( stderr, msg, ap );
	fprintf( stderr, "\n" );		// add newline
    }
    va_end( ap );
#if defined(UNIX) && defined(DEBUG)
    abort();					// trap; generates core dump
#else
    exit( 1 );					// goodbye cruel world
#endif
}


bool chk_pointer( bool c, const char *n, int l )// fatal error if c is TRUE
{
    if ( c )
	fatal( "In file %s, line %d: Out of memory", n, l );
    return TRUE;
}


dbg_handler installDebugHandler( dbg_handler h )// install debug handler
{
    dbg_handler old = handler;
    handler = h;
    return old;
}
