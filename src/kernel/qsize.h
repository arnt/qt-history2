/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsize.h#1 $
**
** Definition of QSize class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
*****************************************************************************/

#ifndef QSIZE_H
#define QSIZE_H

#include "qpoint.h"


class QSize					// size class
{
public:
    QSize()	{}				// undefined init values
    QSize( QCOOT w, QCOOT h );			// set w=width, h=height

    bool   isNull()	const	{ return wd==0 && ht==0; }
    bool   isEmpty()	const	{ return wd<1 || ht<1; }
    bool   isValid()	const	{ return wd>=0 && ht>=0; }

    QCOOT &width()		{ return wd; }	// get/set width
    QCOOT &height()		{ return ht; }	// get/set height
    QCOOT  getWidth()	const	{ return wd; }	// width, when const is needed
    QCOOT  getHeight()	const	{ return ht; }	// height, when const is needed

    QSize  &operator*=( int c );		// multiply with scalar
    QSize  &operator*=( float c );		// multiply with scalar float
    QSize  &operator/=( int c );		// divide by scalar
    QSize  &operator/=( float c );		// divide by scalar float

    friend bool	  operator==( const QSize &, const QSize & );
    friend bool	  operator!=( const QSize &, const QSize & );
    friend QSize  operator*( const QSize &, int );
    friend QSize  operator*( int, const QSize & );
    friend QSize  operator*( const QSize &, float );
    friend QSize  operator*( float, const QSize & );
    friend QSize  operator/( const QSize &, int );
    friend QSize  operator/( const QSize &, float );

private:
    QCOOT wd;					// width
    QCOOT ht;					// height
};


// --------------------------------------------------------------------------
// QSize stream functions
//

QStream &operator<<( QStream &, const QSize & );
QStream &operator>>( QStream &, QSize & );


#if !(defined(QSIZE_C) || defined(DEBUG))

// --------------------------------------------------------------------------
// QSize member functions
//

inline QSize::QSize( QCOOT w, QCOOT h )
{
    wd=w; ht=h;
}

inline QSize &QSize::operator*=( int c )
{
    wd*=c; ht*=c; return *this;
}

inline QSize &QSize::operator*=( float c )
{
    wd=(QCOOT)(wd*c); ht=(QCOOT)(ht*c); return *this;
}

inline QSize &QSize::operator/=( int c )
{
    wd/=c; ht/=c; return *this;
}

inline QSize &QSize::operator/=( float c )
{
    wd=(QCOOT)(wd/c); ht=(QCOOT)(ht/c); return *this;
}

inline bool operator==( const QSize &s1, const QSize &s2 )
{
    return s1.wd == s2.wd && s1.ht == s2.ht;
}

inline bool operator!=( const QSize &s1, const QSize &s2 )
{
    return s1.wd != s2.wd || s1.ht != s2.ht;
}

inline QSize operator*( const QSize &s, int c )
{
    return QSize( s.wd*c, s.ht*c );
}

inline QSize operator*( int c, const QSize &s )
{
    return QSize( s.wd*c, s.ht*c );
}

inline QSize operator*( const QSize &s, float c )
{
    return QSize( (QCOOT)(s.wd*c), (QCOOT)(s.ht*c) );
}

inline QSize operator*( float c, const QSize &s )
{
    return QSize( (QCOOT)(s.wd*c), (QCOOT)(s.ht*c) );
}

inline QSize operator/( const QSize &s, int c )
{
    return QSize( s.wd/c, s.ht/c );
}

inline QSize operator/( const QSize &s, float c )
{
    return QSize( (QCOOT)(s.wd/c), (QCOOT)(s.ht/c) );
}

#endif // inline functions


#endif // QSIZE_H
