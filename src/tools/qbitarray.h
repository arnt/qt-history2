/****************************************************************************
** $Id: //depot/qt/main/src/tools/qbitarray.h#29 $
**
** Definition of QBitArray class
**
** Created : 940118
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QBITARRAY_H
#define QBITARRAY_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H


/*****************************************************************************
  QBitVal class; a context class for QBitArray::operator[]
 *****************************************************************************/

class QBitArray;

class Q_EXPORT QBitVal
{
private:
    QBitArray *array;
    uint    index;
public:
    QBitVal( QBitArray *a, uint i ) : array(a), index(i) {}
    operator int();
    QBitVal &operator=( const QBitVal &v );
    QBitVal &operator=( bool v );
};


/*****************************************************************************
  QBitArray class
 *****************************************************************************/

class Q_EXPORT QBitArray : public QByteArray
{
public:
    QBitArray();
    QBitArray( uint size );
    QBitArray( const QBitArray &a ) : QByteArray( a ) {}

    QBitArray &operator=( const QBitArray & );

    uint    size() const;
    bool    resize( uint size );

    bool    fill( bool v, int size = -1 );

    void    detach();
    QBitArray copy() const;

    bool    testBit( uint index ) const;
    void    setBit( uint index );
    void    setBit( uint index, bool value );
    void    clearBit( uint index );
    bool    toggleBit( uint index );

    bool    at( uint index ) const;
    QBitVal operator[]( int index );

    QBitArray &operator&=( const QBitArray & );
    QBitArray &operator|=( const QBitArray & );
    QBitArray &operator^=( const QBitArray & );
    QBitArray  operator~() const;

protected:
    struct bitarr_data : public QGArray::array_data {
	uint   nbits;
    };
    array_data *newData()		    { return new bitarr_data; }
    void	deleteData( array_data *d ) { delete (bitarr_data*)d; }
private:
    void    pad0();
};


inline QBitArray &QBitArray::operator=( const QBitArray &a )
{ return (QBitArray&)assign( a ); }

inline uint QBitArray::size() const
{ return ((bitarr_data*)sharedBlock())->nbits; }

inline void QBitArray::setBit( uint index, bool value )
{ if ( value ) setBit(index); else clearBit(index); }

inline bool QBitArray::at( uint index ) const
{ return testBit(index); }

inline QBitVal QBitArray::operator[]( int index )
{ return QBitVal( (QBitArray*)this, index ); }


/*****************************************************************************
  Misc. QBitArray operator functions
 *****************************************************************************/

Q_EXPORT QBitArray operator&( const QBitArray &, const QBitArray & );
Q_EXPORT QBitArray operator|( const QBitArray &, const QBitArray & );
Q_EXPORT QBitArray operator^( const QBitArray &, const QBitArray & );


inline QBitVal::operator int()
{
    return array->testBit( index );
}

inline QBitVal &QBitVal::operator=( const QBitVal &v )
{
    array->setBit( index, v.array->testBit(v.index) );
    return *this;
}

inline QBitVal &QBitVal::operator=( bool v )
{
    array->setBit( index, v );
    return *this;
}


/*****************************************************************************
  QBitArray stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QBitArray & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QBitArray & );


#endif // QBITARRAY_H
