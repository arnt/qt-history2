/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbitarry.cpp#1 $
**
** Implementation of QBitArray class
**
** Author  : Haavard Nord
** Created : 940118
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
** --------------------------------------------------------------------------
** The size of a bit array is stored in the beginning of the actual array,
** which complicates the implementation. It's probably nicer to use the
** QShared class, and we might reimplement it with QShared later.
*****************************************************************************/

#include "qbitarry.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qbitarry.cpp#1 $";
#endif


// --------------------------------------------------------------------------
// QBitArray class
//

#define BA_SIZE	  *((qbitarraysz_t)data())
#define SZ_SIZE	  sizeof(qbitarraysz_t)
#define BA_DATA	  (data()+SZ_SIZE)

QBitArray::QBitArray( uint size ) : QByteArray( SZ_SIZE + size/8 + 1 )
{
    BA_SIZE = size;				// set number of bits
    memset( BA_DATA, 0, size/8 + 1 );		// set all bits to zero
}


void QBitArray::pad0()				// pad last byte with 0-bits
{
    uint sz = size();
    if ( !sz )
	return;
    uchar mask = 1 << (sz%8);
    if ( mask )
	mask--;
    *(BA_DATA+sz/8) &= mask;
}


bool QBitArray::resize( uint sz )		// resize bit array
{
    uint s = size();
    if ( !QByteArray::resize( SZ_SIZE + sz/8 + 1 ) )
	return FALSE;				// cannot resize
    if ( sz != 0 ) {				// not null array
	BA_SIZE = sz;
	sz = sz/8 - s/8;
	if ( sz > 0 ) {
	    if ( !s )				// was null
		*BA_DATA = 0;
	    memset( BA_DATA + s/8 + 1, 0, sz );
	}
    }
    return TRUE;
}


bool QBitArray::fill( bool v, int sz )		// fill bit array with value
{
    if ( sz != -1 ) {				// resize first
	if ( !resize( sz ) )
	    return FALSE;			// cannot resize
    }
    else
	sz = size();
    memset( BA_DATA, v ? 0xff : 0, sz/8 );	// set many bytes, fast
    pad0();
    return TRUE;
}


bool QBitArray::testBit( uint i ) const		// test if bit set
{
#if defined(CHECK_RANGE)
    if ( i >= size() ) {
	warning( "QBitArray::testBit: Index %d out of range", i );
	return FALSE;
    }
#endif
    return *((pcchar)BA_DATA+(i>>3)) & (1 << (i & 7));
}

void QBitArray::setBit( uint i )		// set bit
{
#if defined(CHECK_RANGE)
    if ( i >= size() ) {
	warning( "QBitArray::setBit: Index %d out of range", i );
	return;
    }
#endif
    *(BA_DATA+(i>>3)) |= (1 << (i & 7));
}

void QBitArray::clearBit( uint i )		// clear bit
{
#if defined(CHECK_RANGE)
    if ( i >= size() ) {
	warning( "QBitArray::clearBit: Index %d out of range", i );
	return;
    }
#endif
    *(BA_DATA+(i>>3)) &= ~(1 << (i & 7));
}

bool QBitArray::toggleBit( uint i )		// toggle/invert bit
{
#if defined(CHECK_RANGE)
    if ( i >= size() ) {
	warning( "QBitArray::toggleBit: Index %d out of range", i );
	return FALSE;
    }
#endif
    register char *p = BA_DATA + (i>>3);
    uchar b = (1 << (i & 7));			// bit position
    uchar c = *p & b;				// read bit
    *p ^= b;					// toggle bit
    return c;
}


QBitArray &QBitArray::operator&=( const QBitArray &a )
{						// AND bits
    if ( size() == a.size() ) {			// must have same length
	register uchar *a1 = (uchar *)data()+SZ_SIZE;
	register uchar *a2 = (uchar *)a.data()+SZ_SIZE;
	int n = QByteArray::size() - SZ_SIZE;
	while ( --n >= 0 )
	    *a1++ &= *a2++;
    }
#if defined(CHECK_RANGE)
    else
	warning( "QBitArray::operator&=: Bit arrays have different size" );
#endif
    return *this;
}

QBitArray &QBitArray::operator|=( const QBitArray &a )
{						// OR bits
    if ( size() == a.size() ) {			// must have same length
	register uchar *a1 = (uchar *)data()+SZ_SIZE;
	register uchar *a2 = (uchar *)a.data()+SZ_SIZE;
	int n = QByteArray::size() - SZ_SIZE;
	while ( --n >= 0 )
	    *a1++ |= *a2++;
    }
#if defined(CHECK_RANGE)
    else
	warning( "QBitArray::operator|=: Bit arrays have different size" );
#endif
    return *this;
}

QBitArray &QBitArray::operator^=( const QBitArray &a )
{						// XOR bits
    if ( size() == a.size() ) {			// must have same length
	register uchar *a1 = (uchar *)data()+SZ_SIZE;
	register uchar *a2 = (uchar *)a.data()+SZ_SIZE;
	int n = QByteArray::size() - SZ_SIZE;
	while ( --n >= 0 )
	    *a1++ ^= *a2++;
    }
#if defined(CHECK_RANGE)
    else
	warning( "QBitArray::operator^=: Bit arrays have different size" );
#endif
    return *this;
}

QBitArray QBitArray::operator~() const		// NOT bits
{
    QBitArray a( size() );
    register uchar *a1 = (uchar *)data()+SZ_SIZE;
    register uchar *a2 = (uchar *)a.data()+SZ_SIZE;
    int n = QByteArray::size() - SZ_SIZE;
    while ( n-- )
	*a2++ = ~*a1++;
    a.pad0();
    return a;
}


QBitArray operator&( const QBitArray &a1, const QBitArray &a2 )
{
    QBitArray tmp = a1.copy();
    tmp &= a2;
    return tmp;
}

QBitArray operator|( const QBitArray &a1, const QBitArray &a2 )
{
    QBitArray tmp = a1.copy();
    tmp |= a2;
    return tmp;
}

QBitArray operator^( const QBitArray &a1, const QBitArray &a2 )
{
    QBitArray tmp = a1.copy();
    tmp ^= a2;
    return tmp;
}


// --------------------------------------------------------------------------
// QBitArray stream functions
//

#include "qstream.h"

QStream &operator<<( QStream &s, const QBitArray &a )
{
    UINT32 len = a.size();
    s << len;					// write size of array
    if ( len )					// write data
	s.writeBytes( a.data()+SZ_SIZE, (uint)(len/8+1) );
    return s;
}

QStream &operator>>( QStream &s, QBitArray &a )
{
    UINT32 len;
    s >> len;					// read size of array
    if ( !a.resize( (uint)len ) ) {		// resize array
#if defined(CHECK_NULL)
	warning( "QStream: Not enough memory to read QBitArray" );
#endif
	len = 0;
    }
    if ( len > 0 )				// read data
	s.readBytes( a.data()+SZ_SIZE, (uint)(len/8+1) );
    return s;
}
