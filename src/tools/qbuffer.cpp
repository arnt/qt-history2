/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbuffer.cpp#1 $
**
** Implementation of QBuffer class
**
** Author  : Haavard Nord
** Created : 930812
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#include "qbuffer.h"
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qbuffer.cpp#1 $";
#endif


QBuffer::QBuffer()
{
    a_inc = 1024;				// default increment
    a_len = 0;
    ptr = 0;
}

QBuffer::QBuffer( QByteArray ba )
{
    a_inc = 1024;				// default increment
    setBuffer( ba );				// set buffer
}

QBuffer::~QBuffer()
{
}


bool QBuffer::setBuffer( QByteArray ba )	// set buffer
{
    ptr = 0;
    a = ba;
    a_len = a.size();
    return TRUE;
}


bool QBuffer::open( int mode  )			// open buffer
{
    if ( !setMode( mode ) )
	return FALSE;				// invalid mode
    ptr = 0;
    return TRUE;
}

bool QBuffer::close()				// close buffer
{
    smode = Stream_Null;
    ptr = 0;
    return TRUE;
}

bool QBuffer::flush()				// flush buffer
{
    return TRUE;				// nothing to do
}


long QBuffer::size()				// get buffer size
{
    return a.size();
}

bool QBuffer::at( long n )			// set buffer pointer
{
#if defined(CHECK_RANGE)
    if ( n > a_len ) {
	warning( "QBuffer::at: Pointer %lu out of range", n );
	return FALSE;
    }
#endif
    ptr = n;
    return TRUE;
}


QStream& QBuffer::_read( char *p, uint len )	// read data from buffer
{
#if defined(CHECK_STATE)
    if ( !(mode() & Stream_ReadOnly) ) {	// reading not permitted
	warning( "QBuffer::_read: Read operation illegal in this mode" );
	return *this;
    }
#endif
    if ( (uint)ptr + len > a.size() ) {		// overflow
#if defined(CHECK_RANGE)
	warning( "QBuffer::_read: Buffer read error" );
#endif
	return *this;
    }
    memcpy( p, a.data()+ptr, len );
    ptr += len;
    return *this;
}

QStream& QBuffer::_write(const char *p, uint len)// write data info buffer
{
#if defined(CHECK_STATE)
    if ( !(mode() & Stream_WriteOnly) ) {	// writing not permitted
	warning( "QBuffer::_write: Write operation illegal in this mode" );
	return *this;
    }
#endif
    if ( (uint)ptr + len >= a_len ) {		// overflow
	uint new_len = a_len + a_inc*(((uint)ptr+len-a_len)/a_inc+1);
	if ( !a.resize( (uint)new_len ) ) {	// could not resize
#if defined(CHECK_NULL)
	    warning( "QBuffer::_write: Memory allocation error" );
#endif
	    return *this;
	}
	a_len = new_len;
	a.p->len = (uint)ptr + len;
    }
    memcpy( a.data()+ptr, p, len );
    ptr += len;
    if ( a.p->len < (uint)ptr )
	a.p->len = (uint)ptr;			// fake (not alloc'd) length
    return *this;
}


int QBuffer::getch()				// get next char
{
    if ( (uint)ptr+1 > a.size() ) {		// overflow
#if defined(CHECK_RANGE)
	warning( "QBuffer::getch: Buffer read error" );
#endif
	return -1;
    }
    return *(a.data()+(uint)ptr++);
}

int QBuffer::putch( int ch )			// put char
{
#if defined(CHECK_STATE)
    if ( !(mode() & Stream_WriteOnly) ) {	// writing not permitted
	warning( "QFile::putch: Write operation illegal in this mode" );
	return -1;
    }
#endif
    if ( (uint)ptr + 1 >= a_len ) {		// overflow
	char buf[1];
	buf[0] = (char)ch;
	_write( buf, 1 );
    }
    else {
	*(a.data() + ptr++) = (char)ch;
	if ( a.p->len < (uint)ptr )
	    a.p->len = (uint)ptr;
    }
    return ch;
}

int QBuffer::ungetch( int ch )			// put back char
{
    if ( ptr )
	ptr--;
    else
	ch = -1;
    return ch;
}
