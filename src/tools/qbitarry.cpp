/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbitarry.cpp#22 $
**
** Implementation of QBitArray class
**
** Author  : Haavard Nord
** Created : 940118
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qbitarry.h"
#include "qdstream.h"

RCSTAG("$Id: //depot/qt/main/src/tools/qbitarry.cpp#22 $")


#define SHBLOCK  ((bitarr_data*)(sharedBlock()))


/*!
  \class QBitArray qbitarry.h
  \brief The QBitArray class provides an array of bits.

  \ingroup tools
  \ingroup shared

  A QBitArray is a special byte array that can access individual bits and
  perform bit-operations (AND, OR, XOR and NOT) on entire arrays or bits.

  Bits can be manipulated by the setBit() and clearBit() functions, but it
  is also possible to use the indexing [] operator to test and set
  individual bits. The [] operator is a little slower than the others,
  because some trics are required to implement bitwise assignments.

  Example:
  \code
    QBitArray a(3);
    a.setBit( 0 );
    a.clearBit( 1 );
    a.setBit( 2 );			// a = [1 0 1]

    QBitArray b(3);
    b[0] = 1;
    b[1] = 1;
    b[2] = 0;				// b = [1 1 0]

    QBitArray c;
    c = ~a & b;				// c = [0 1 0]
  \endcode
*/


/*!
  \class QBitVal qbitarry.h
  \brief The QBitVal class is an internal class, used with QBitArray.

  \internal

  The QBitVal is required by the indexing [] operator on bit arrays.
*/


/*!
  Constructs an empty bit array.
*/

QBitArray::QBitArray() : QByteArray( 0, 0 )
{
    bitarr_data *x = new bitarr_data;
    CHECK_PTR( x );
    x->nbits = 0;
    setSharedBlock( x );
}

/*!
  Constructs a bit array of \e size bits. The bits are uninitialized.
*/

QBitArray::QBitArray( uint size ) : QByteArray( 0, 0 )
{
    bitarr_data *x = new bitarr_data;
    CHECK_PTR( x );
    x->nbits = 0;
    setSharedBlock( x );
    resize( size );
}


void QBitArray::pad0()				// pad last byte with 0-bits
{
    uint sz = size();
    if ( !sz )
	return;
    uchar mask = 1 << (sz%8);
    if ( mask )
	mask--;
    *(data()+sz/8) &= mask;
}


/*!
  \fn uint QBitArray::size() const
  Returns the size (number of bits) of the bit array.
  \sa resize()
*/

/*!
  Resizes the bit array to \e size bits.
  Returns TRUE if the bit array could be resized.

  When expanding the bit array, the new bits will be uninitialized.

  \sa size()
*/

bool QBitArray::resize( uint size )
{
    uint s = this->size();
    if ( !QByteArray::resize( (size+7)/8 ) )
	return FALSE;				// cannot resize
    SHBLOCK->nbits = size;
    if ( size != 0 ) {				// not null array
	int ds = (int)(size+7)/8 - (int)(s+7)/8;// number of bytes difference
	if ( ds > 0 )				// expanding array
	    memset( data() + (s+7)/8, 0, ds );	//   reset new data
    }
    return TRUE;
}


/*!
  Fills the bit array with \e v (1's if \e v is TRUE, or 0's if \e v is FALSE).

  Will resize the bit array to \e size bits if \e size is nonnegative.

  Returns FALSE if a nonnegative \e size was specified and if the bit array
  could not be resized, otherwise returns TRUE.

  \sa resize()
*/

bool QBitArray::fill( bool v, int size )
{
    if ( size >= 0 ) {				// resize first
	if ( !resize( size ) )
	    return FALSE;			// cannot resize
    }
    else
	size = this->size();
    memset( data(), v ? 0xff : 0, (size+7)/8 ); // set many bytes, fast
    if ( v )
	pad0();
    return TRUE;
}


/*!
  Detaches from shared bit array data and makes sure that this bit array
  is the only one referring the data.

  If multiple bit arrays share common data, this bit array dereferences the
  data and gets a copy of the data. Nothing will be done if there is just
  a single reference.

  \sa copy()
*/

void QBitArray::detach()
{
    int nbits = SHBLOCK->nbits;
    this->duplicate( *this );
    SHBLOCK->nbits = nbits;
}

/*!
  Returns a deep copy of the bit array.
  \sa detach()
*/

QBitArray QBitArray::copy() const
{
    QBitArray tmp;
    tmp.duplicate( *this );
    ((bitarr_data*)(tmp.sharedBlock()))->nbits = SHBLOCK->nbits;
    return tmp;
}


/*!
  Returns TRUE if the bit at position \e i is set.
  \sa setBit(), clearBit()
*/

bool QBitArray::testBit( uint i ) const
{
#if defined(CHECK_RANGE)
    if ( i >= size() ) {
	warning( "QBitArray::testBit: Index %d out of range", i );
	return FALSE;
    }
#endif
    return *(data()+(i>>3)) & (1 << (i & 7));
}

/*!
  Sets the bit at position \e i (sets it to 1).
  \sa clearBit(), toggleBit()
*/

void QBitArray::setBit( uint i )
{
#if defined(CHECK_RANGE)
    if ( i >= size() ) {
	warning( "QBitArray::setBit: Index %d out of range", i );
	return;
    }
#endif
    *(data()+(i>>3)) |= (1 << (i & 7));
}

/*!
  Clears the bit at position \e i (sets it to 0).
  \sa setBit(), toggleBit()
*/

void QBitArray::clearBit( uint i )
{
#if defined(CHECK_RANGE)
    if ( i >= size() ) {
	warning( "QBitArray::clearBit: Index %d out of range", i );
	return;
    }
#endif
    *(data()+(i>>3)) &= ~(1 << (i & 7));
}

/*!
  Toggles the bit at position \e i.

  If the previous value was 0, the new value will be 1.  If the previous
  value was 1, the new value will be 0.

  \sa setBit(), clearBit()
*/

bool QBitArray::toggleBit( uint i )
{
#if defined(CHECK_RANGE)
    if ( i >= size() ) {
	warning( "QBitArray::toggleBit: Index %d out of range", i );
	return FALSE;
    }
#endif
    register uchar *p = (uchar *)data() + (i>>3);
    uchar b = (1 << (i & 7));			// bit position
    uchar c = *p & b;				// read bit
    *p ^= b;					// toggle bit
    return c;
}


/*!
  Performs the AND operation between all bits in this bit array and \e a.
  Returns a reference to this bit array.

  The two bit arrays must have the same size.  The debug library will
  warn you if they aren't, the production library blithely ignores the
  problem.

  Example:
  \code
    QBitArray a(3), b(3);
    a[0] = 1;  a[1] = 0;	a[2] = 1;	// a = [1 0 1]
    b[0] = 0;  b[1] = 0;	b[2] = 1;	// b = [0 0 1]
    a &= b;					// a = [0 0 1]
  \endcode

  \sa operator|=(), operator^=(), operator~()
*/

QBitArray &QBitArray::operator&=( const QBitArray &a )
{
    if ( size() == a.size() ) {			// must have same length
	register uchar *a1 = (uchar *)data();
	register uchar *a2 = (uchar *)a.data();
	int n = QByteArray::size();
	while ( --n >= 0 )
	    *a1++ &= *a2++;
    }
#if defined(CHECK_RANGE)
    else
	warning( "QBitArray::operator&=: Bit arrays have different size" );
#endif
    return *this;
}

/*!
  Performs the OR operation between all bits in this bit array and \e a.
  Returns a reference to this bit array.

  The two bit arrays must have the same size.  The debug library will
  warn you if they aren't, the production library blithely ignores the
  problem.

  Example:
  \code
    QBitArray a(3), b(3);
    a[0] = 1;  a[1] = 0;	a[2] = 1;	// a = [1 0 1]
    b[0] = 0;  b[1] = 0;	b[2] = 1;	// b = [0 0 1]
    a |= b;					// a = [1 0 1]
  \endcode

  \sa operator&=(), operator^=(), operator~()
*/

QBitArray &QBitArray::operator|=( const QBitArray &a )
{
    if ( size() == a.size() ) {			// must have same length
	register uchar *a1 = (uchar *)data();
	register uchar *a2 = (uchar *)a.data();
	int n = QByteArray::size();
	while ( --n >= 0 )
	    *a1++ |= *a2++;
    }
#if defined(CHECK_RANGE)
    else
	warning( "QBitArray::operator|=: Bit arrays have different size" );
#endif
    return *this;
}

/*!
  Performs the XOR operation between all bits in this bit array and \e a.
  Returns a reference to this bit array.

  The two bit arrays must have the same size.  The debug library will
  warn you if they aren't, the production library blithely ignores the
  problem.

  Example:
  \code
    QBitArray a(3), b(3);
    a[0] = 1;  a[1] = 0;	a[2] = 1;	// a = [1 0 1]
    b[0] = 0;  b[1] = 0;	b[2] = 1;	// b = [0 0 1]
    a ^= b;					// a = [1 0 0]
  \endcode

  \sa operator&=(), operator|=(), operator~()
*/

QBitArray &QBitArray::operator^=( const QBitArray &a )
{
    if ( size() == a.size() ) {			// must have same length
	register uchar *a1 = (uchar *)data();
	register uchar *a2 = (uchar *)a.data();
	int n = QByteArray::size();
	while ( --n >= 0 )
	    *a1++ ^= *a2++;
    }
#if defined(CHECK_RANGE)
    else
	warning( "QBitArray::operator^=: Bit arrays have different size" );
#endif
    return *this;
}

/*!
  Returns a bit array which contains the inverted bits of this bit array.

  Example:
  \code
    QBitArray a(3);
    a[0] = 1;  a[1] = 0; a[2] = 1;	// a = [1 0 1]
    QBitArray b = ~a;			// b = [0 1 0]
  \endcode
*/

QBitArray QBitArray::operator~() const
{
    QBitArray a( size() );
    register uchar *a1 = (uchar *)data();
    register uchar *a2 = (uchar *)a.data();
    int n = QByteArray::size();
    while ( n-- )
	*a2++ = ~*a1++;
    a.pad0();
    return a;
}


/*!
  \relates QBitArray
  Returns the AND result between the bit arrays \e a1 and \e a2.
  \sa QBitArray::operator&=()
*/

QBitArray operator&( const QBitArray &a1, const QBitArray &a2 )
{
    QBitArray tmp = a1.copy();
    tmp &= a2;
    return tmp;
}

/*!
  \relates QBitArray
  Returns the OR result between the bit arrays \e a1 and \e a2.
  \sa QBitArray::operator|=()
*/

QBitArray operator|( const QBitArray &a1, const QBitArray &a2 )
{
    QBitArray tmp = a1.copy();
    tmp |= a2;
    return tmp;
}

/*!
  \relates QBitArray
  Returns the XOR result between the bit arrays \e a1 and \e a2.
  \sa QBitArray::operator^()
*/

QBitArray operator^( const QBitArray &a1, const QBitArray &a2 )
{
    QBitArray tmp = a1.copy();
    tmp ^= a2;
    return tmp;
}


// --------------------------------------------------------------------------
// QBitArray stream functions
//

/*!
  \relates QBitArray
  Writes a bit array to a stream.
*/

QDataStream &operator<<( QDataStream &s, const QBitArray &a )
{
    UINT32 len = a.size();
    s << len;					// write size of array
    if ( len > 0 )				// write data
	s.writeRawBytes( a.data(), a.QByteArray::size() );
    return s;
}

/*!
  \relates QBitArray
  Reads a bit array from a stream.
*/

QDataStream &operator>>( QDataStream &s, QBitArray &a )
{
    UINT32 len;
    s >> len;					// read size of array
    if ( !a.resize( (uint)len ) ) {		// resize array
#if defined(CHECK_NULL)
	warning( "QDataStream: Not enough memory to read QBitArray" );
#endif
	len = 0;
    }
    if ( len > 0 )				// read data
	s.readRawBytes( a.data(), a.QByteArray::size() );
    return s;
}
