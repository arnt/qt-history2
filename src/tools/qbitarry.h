/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbitarry.h#11 $
**
** Definition of QBitArray class
**
** Author  : Haavard Nord
** Created : 940118
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
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

class QBitArray : public QByteArray
{
public:
    QBitArray();
    QBitArray( uint size );
    QBitArray( const QBitArray &a ) : QByteArray( a ) {}

    QBitArray &operator=( const QBitArray &a )	// shallow copy
	{ return (QBitArray&)assign( a ); }

    uint    size() const { return ((bitarr_data*)sharedBlock())->nbits; }
    bool    resize( uint size );		// resize bit array

    bool    fill( bool v, int size = -1 );	// fill bit array with value

    void    detach();				// detach bit array
    QBitArray copy() const;			// get deep copy

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

protected:
    struct bitarr_data : QGArray::array_data {	// shared bit array
	uint   nbits;
    };
    array_data *newData()		    { return new bitarr_data; }
    void	deleteData( array_data *d ) { delete (bitarr_data*)d; }
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

QDataStream &operator<<( QDataStream &, const QBitArray & );
QDataStream &operator>>( QDataStream &, QBitArray & );


#endif // QBITARRY_H
