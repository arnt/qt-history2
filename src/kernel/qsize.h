/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsize.h#7 $
**
** Definition of QSize class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSIZE_H
#define QSIZE_H

#include "qpoint.h"


class QSize					// size class
{
public:
    QSize()	{}				// undefined init values
    QSize( int w, int h );			// set w=width, h=height

    bool   isNull()	const	{ return wd==0 && ht==0; }
    bool   isEmpty()	const	{ return wd<1 || ht<1; }
    bool   isValid()	const	{ return wd>=0 && ht>=0; }

    int	   width()	const	{ return wd; }	// get width
    int	   height()	const	{ return ht; }	// get height
    void   setWidth( int w )	{ wd=(QCOORD)w;}// set width
    void   setHeight( int h )	{ ht=(QCOORD)h;}// set height

    QCOORD &rwidth()		{ return wd; }	// get reference to width
    QCOORD &rheight()		{ return ht; }	// get reference to height

    QSize &operator+=( const QSize & );		// add size
    QSize &operator-=( const QSize & );		// subtract size
    QSize &operator*=( int c );			// multiply with scalar
    QSize &operator*=( float c );		// multiply with scalar float
    QSize &operator/=( int c );			// divide by scalar
    QSize &operator/=( float c );		// divide by scalar float

    friend bool	  operator==( const QSize &, const QSize & );
    friend bool	  operator!=( const QSize &, const QSize & );
    friend QSize  operator+( const QSize &, const QSize & );
    friend QSize  operator-( const QSize &, const QSize & );
    friend QSize  operator*( const QSize &, int );
    friend QSize  operator*( int, const QSize & );
    friend QSize  operator*( const QSize &, float );
    friend QSize  operator*( float, const QSize & );
    friend QSize  operator/( const QSize &, int );
    friend QSize  operator/( const QSize &, float );

private:
    QCOORD wd;					// width
    QCOORD ht;					// height
};


// --------------------------------------------------------------------------
// QSize stream functions
//

QDataStream &operator<<( QDataStream &, const QSize & );
QDataStream &operator>>( QDataStream &, QSize & );


#if !(defined(QSIZE_C) || defined(DEBUG))

// --------------------------------------------------------------------------
// QSize member functions
//

inline QSize::QSize( int w, int h )
{
    wd=(QCOORD)w; ht=(QCOORD)h;
}

inline QSize &QSize::operator*=( int c )
{
    wd*=(QCOORD)c; ht*=(QCOORD)c; return *this;
}

inline QSize &QSize::operator*=( float c )
{
    wd=(QCOORD)(wd*c); ht=(QCOORD)(ht*c); return *this;
}

inline QSize &QSize::operator/=( int c )
{
    wd/=(QCOORD)c; ht/=(QCOORD)c; return *this;
}

inline QSize &QSize::operator/=( float c )
{
    wd=(QCOORD)(wd/c); ht=(QCOORD)(ht/c); return *this;
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
    return QSize( (QCOORD)(s.wd*c), (QCOORD)(s.ht*c) );
}

inline QSize operator*( float c, const QSize &s )
{
    return QSize( (QCOORD)(s.wd*c), (QCOORD)(s.ht*c) );
}

inline QSize operator/( const QSize &s, int c )
{
    return QSize( s.wd/c, s.ht/c );
}

inline QSize operator/( const QSize &s, float c )
{
    return QSize( (QCOORD)(s.wd/c), (QCOORD)(s.ht/c) );
}

#endif // inline functions


#endif // QSIZE_H
