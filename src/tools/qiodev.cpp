/****************************************************************************
** $Id: //depot/qt/main/src/tools/qiodev.cpp#1 $
**
** Implementation of QIODevice class
**
** Author  : Haavard Nord
** Created : 940913
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qiodev.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qiodev.cpp#1 $";
#endif


// --------------------------------------------------------------------------
// QIODevice member functions
//

QIODevice::QIODevice()
{
    ioMode = 0;					// initial mode
    ioSt = IO_Ok;
    index = 0;
}

QIODevice::~QIODevice()
{
}


void QIODevice::setType( int t )		// set device type
{
#if defined(CHECK_RANGE)
    if ( (t & IO_TypeMask) != t )
	warning( "QIODevice::setType: Specified type out of range" );
#endif
    ioMode &= ~IO_TypeMask;			// reset type bits
    ioMode |= t;
}

void QIODevice::setMode( int m )		// set device mode
{
#if defined(CHECK_RANGE)
    if ( (m & IO_ModeMask) != m )
	warning( "QIODevice::setMode: Specified mode out of range" );
#endif
    ioMode &= ~IO_ModeMask;			// reset mode bits
    ioMode |= m;
}

void QIODevice::setState( int s )		// set device state
{
#if defined(CHECK_RANGE)
    if ( (s & IO_StateMask) != s )
	warning( "QIODevice::setState: Specified state out of range" );
#endif
    ioMode &= ~IO_StateMask;			// reset state bits
    ioMode |= s;
}

void QIODevice::setStatus( int s )		// set status
{
    ioSt = s;
}


long QIODevice::at() const			// get data index
{
    return index;
}

bool QIODevice::at( long n )			// set data index
{
#if defined(CHECK_RANGE)
    if ( n > size() ) {
	warning( "QIODevice::at: Index %lu out of range", n );
	return FALSE;
    }
#endif
    index = n;
    return TRUE;
}

bool QIODevice::atEnd() const			// at end of data
{
    return at() == size();
}
