/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrect.h#19 $
**
** Definition of QRect class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QRECT_H
#define QRECT_H

#include "qsize.h"


class QRect					// rectangle class
{
public:
    QRect()	{}
    QRect( const QPoint &topleft, const QPoint &bottomright );
    QRect( const QPoint &topleft, const QSize &size );
    QRect( int left, int top, int width, int height );

    bool   isNull()	const;
    bool   isEmpty()	const;
    bool   isValid()	const;
    QRect  normalize()	const;

    int	   left()	const	{ return x1; }
    int	   top()	const	{ return y1; }
    int	   right()	const	{ return x2; }
    int	   bottom()	const	{ return y2; }
    int	   x()		const	{ return x1; }
    int	   y()		const	{ return y1; }
    void   setLeft( int pos );
    void   setTop( int pos );
    void   setRight( int pos );
    void   setBottom( int pos );
    void   setX( int x );
    void   setY( int y );

    QPoint topLeft()	 const;
    QPoint bottomRight() const;
    QPoint topRight()	 const;
    QPoint bottomLeft()	 const;
    QPoint center()	 const;
    void   rect( int *x, int *y, int *w, int *h ) const;
    void   coords( int *x1, int *y1, int *x2, int *y2 ) const;
    void   setTopLeft( const QPoint &p );
    void   setBottomRight( const QPoint &p );
    void   setTopRight( const QPoint &p );
    void   setBottomLeft( const QPoint &p );
    void   setCenter( const QPoint &p );
    void   translate( int dx, int dy );
#if defined(OBSOLETE)
    void   move( int dx, int dy ) { translate( dx, dy ); }
#endif
    void   setRect( int x, int y, int w, int h );
    void   setCoords( int x1, int y1, int x2, int y2 );

    QSize  size()	const;
    int	   width()	const	{ return x2 - x1 + 1; }
    int	   height()	const	{ return y2 - y1 + 1; }
    void   setWidth( int w );
    void   setHeight( int h );
    void   setSize( const QSize &s );

    bool   contains( const QPoint &p, bool proper=FALSE ) const;
    bool   contains( const QRect &r, bool proper=FALSE ) const;
    QRect  unite( const QRect &r ) const;
    QRect  intersect( const QRect &r ) const;
    bool   intersects( const QRect &r ) const;

    friend bool operator==( const QRect &, const QRect & );
    friend bool operator!=( const QRect &, const QRect & );

private:
#if defined(_OS_MAC_)
    QCOORD y1;
    QCOORD x1;
    QCOORD y2;
    QCOORD x2;
#else
    QCOORD x1;
    QCOORD y1;
    QCOORD x2;
    QCOORD y2;
#endif
};


// --------------------------------------------------------------------------
// QRect stream functions
//

QDataStream &operator<<( QDataStream &, const QRect & );
QDataStream &operator>>( QDataStream &, QRect & );


#if !(defined(QRECT_C) || defined(DEBUG))

// --------------------------------------------------------------------------
// QRect member functions
//

inline bool QRect::isNull() const
{
    return x2 == x1-1 && y2 == y1-1;
}

inline bool QRect::isEmpty() const
{
    return x1 > x2 || y1 > y2;
}

inline bool QRect::isValid() const
{
    return x1 <= x2 && y1 <= y2;
}

inline void QRect::setLeft( int pos )
{
    x1 = (QCOORD)pos;
}

inline void QRect::setTop( int pos )
{
    y1 = (QCOORD)pos;
}

inline void QRect::setRight( int pos )
{
    x2 = (QCOORD)pos;
}

inline void QRect::setBottom( int pos )
{
    y2 = (QCOORD)pos;
}

inline void QRect::setX( int x )
{
    x1 = (QCOORD)x;
}

inline void QRect::setY( int y )
{
    y1 = (QCOORD)y;
}

inline QPoint QRect::topLeft() const
{
    return QPoint( x1, y1 );
}

inline QPoint QRect::bottomRight() const
{
    return QPoint( x2, y2 );
}

inline QPoint QRect::topRight() const
{
    return QPoint( x2, y1 );
}

inline QPoint QRect::bottomLeft() const
{
    return QPoint( x1, y2 );
}

inline QPoint QRect::center() const
{
    return QPoint( (x1+x2)/2, (y1+y2)/2 );
}

inline QSize QRect::size() const
{
    return QSize( x2-x1+1, y2-y1+1 );
}

#endif // inline functions


#endif // QRECT_H
