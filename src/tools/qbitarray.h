/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbitarray.h#1 $
**
** Definition of QBitArray class
**
** Author  : Haavard Nord
** Created : 940118
**
** Copyright (C) 1994 by Troll Tech as.	 All rights reserved.
**
*****************************************************************************/

#ifndef QBITARRY_H
#define QBITARRY_H

#include "qstring.h"


// --------------------------------------------------------------------------
// QBitVal class; a context class for QBitArray::operator[]
//

class QBitArray;

class QBitVal
{
private:
    QBitArray *array;
    uint    index;
public:
    QBitVal( QBitArray *a, uint i ) : array(a), index(i) {}
	    operator int();
    QBitVal &operator=( const QBitVal &v );
    QBitVal &operator=( int v );
};


// --------------------------------------------------------------------------
// QBitArray class
//

typedef ulong *qbitarraysz_t;

class QBitArray : public QByteArray
{
public:
    QBitArray() {}
    QBitArray( uint size );
    QBitArray( const QBitArray &a ) : QByteArray( a ) {}

    QBitArray &operator=( const QBitArray &a )	// shallow copy
	{ return (QBitArray&)assign( a ); }

    uint    size() const { return isNull() ? 0 : *((qbitarraysz_t)data()); }
    bool    resize( uint size );		// resize bit array

    bool    fill( bool v, int size = -1 );	// fill bit array with value

    QBitArray copy() const			// get deep copy
		{ QBitArray tmp; return *((QBitArray*)&tmp.duplicate(*this)); }

    bool    testBit( uint i ) const;		// test if bit set
    void    setBit( uint i );			// set bit
    void    setBit( uint i, bool v )		// set bit to value
		{ if ( v ) setBit(i); else clearBit(i); }
    void    clearBit( uint i );			// clear bit
    bool    toggleBit( uint i );		// toggle/invert bit

    bool    at( uint i ) const			// access bit
		{ return testBit(i); }
    QBitVal operator[]( int i )			// get/set bit
		{ return QBitVal( (QBitArray*)this, i ); }

    QBitArray &operator&=( const QBitArray & ); // AND bits
    QBitArray &operator|=( const QBitArray & ); // OR  bits
    QBitArray &operator^=( const QBitArray & ); // XOR bits
    QBitArray  operator~() const;		// NOT bits

private:
    void    pad0();
};


// --------------------------------------------------------------------------
// Misc. QBitArray operator functions
//

QBitArray operator&( const QBitArray &, const QBitArray & );
QBitArray operator|( const QBitArray &, const QBitArray & );
QBitArray operator^( const QBitArray &, const QBitArray & );


inline QBitVal::operator int()
{
    return array->testBit( index );
}

inline QBitVal &QBitVal::operator=( const QBitVal &v )
{
    array->setBit( index, v.array->testBit(v.index) );
    return *this;
}

inline QBitVal &QBitVal::operator=( int v )
{
    array->setBit( index, v );
    return *this;
}


// --------------------------------------------------------------------------
// QBitArray stream functions
//

QStream &operator<<( QStream &, const QBitArray & );
QStream &operator>>( QStream &, QBitArray & );


#endif // QBITARRY_H
