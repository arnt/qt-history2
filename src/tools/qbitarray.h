/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbitarray.h#12 $
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

    QBitArray &operator=( const QBitArray &a )
		{ return (QBitArray&)assign( a ); }

    uint    size() const { return ((bitarr_data*)sharedBlock())->nbits; }
    bool    resize( uint size );

    bool    fill( bool v, int size = -1 );

    void    detach();
    QBitArray copy() const;

    bool    testBit( uint i ) const;
    void    setBit( uint i );
    void    setBit( uint i, bool v )
		{ if ( v ) setBit(i); else clearBit(i); }
    void    clearBit( uint i );
    bool    toggleBit( uint i );

    bool    at( uint i ) const
		{ return testBit(i); }
    QBitVal operator[]( int i )
		{ return QBitVal( (QBitArray*)this, i ); }

    QBitArray &operator&=( const QBitArray & );
    QBitArray &operator|=( const QBitArray & );
    QBitArray &operator^=( const QBitArray & );
    QBitArray  operator~() const;

protected:
    struct bitarr_data : QGArray::array_data {
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
