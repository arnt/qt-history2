/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrect.h#8 $
**
** Definition of QRect class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QRECT_H
#define QRECT_H

#include "qsize.h"


class QRect					// rectangle class
{
public:
    QRect()	{}				// undefined init values
    QRect( const QPoint &topleft, const QPoint &bottomright );
    QRect( const QPoint &topleft, const QSize &size );
    QRect( QCOOT left, QCOOT top, QCOOT width, QCOOT height );

    bool   isNull()	const;			// zero width and height
    bool   isEmpty()	const;			// zero or neg. width or height
    bool   isValid()	const;			// has valid coordinates
    void   fixup();				// fixup bad coordinates

    QCOOT  left()	 const	{ return x1; }	// get left position
    QCOOT  top()	 const	{ return y1; }	// get top position
    QCOOT  right()	 const	{ return x2; }	// get right position
    QCOOT  bottom()	 const	{ return y2; }	// get bottom position
    QCOOT  x()		 const	{ return x1; }	// same as left()
    QCOOT  y()		 const	{ return y1; }	// same as top()
    void   setLeft( QCOOT pos );		// set left position
    void   setTop( QCOOT pos );			// set top position
    void   setRight( QCOOT pos );		// set right position
    void   setBottom( QCOOT pos );		// set bottom position
    void   setX( QCOOT x );			// same as setLeft()
    void   setY( QCOOT y );			// same as setTop()

    QPoint topLeft()	 const;			// get top left point
    QPoint bottomRight() const;			// get bottom right point
    QPoint topRight()	 const;			// get top right point
    QPoint bottomLeft()	 const;			// get bottom left point
    QPoint center()	 const;			// get center point
    void   rect( int *x, int *y, int *w, int *h ) const;
    void   coords( int *x1, int *y1, int *x2, int *y2 ) const;
    void   setTopLeft( const QPoint &p );	// move; top left at p
    void   setBottomRight( const QPoint &p );	// move; bottom right at p
    void   setTopRight( const QPoint &p );	// move; top right at p
    void   setBottomLeft( const QPoint &p );	// move; bottom left at p
    void   setCenter( const QPoint &p );	// move; center at p
    void   move( int dx, int dy );		// move; displace by dx,dy
    void   setRect( int x, int y, int w, int h );
    void   setCoords( int x1, int y1, int x2, int y2 );

    QSize  size()	 const;			// get size
    QCOOT  width()	 const;			// get width
    QCOOT  height()	 const;			// get height
    void   setSize( const QSize &s );		// set size

    bool   contains( const QPoint &p, bool proper=FALSE ) const;
    bool   contains( const QRect &r, bool proper=FALSE ) const;
    QRect  unite( const QRect &r ) const;
    QRect  intersect( const QRect &r ) const;
    bool   intersects( const QRect &r ) const;

    friend bool operator==( const QRect &, const QRect & );
    friend bool operator!=( const QRect &, const QRect & );

private:
#if defined(_OS_MAC_)
    QCOOT y1;					// top position
    QCOOT x1;					// left position
    QCOOT y2;					// bottom position
    QCOOT x2;					// right position
#else
    QCOOT x1;					// left position
    QCOOT y1;					// top position
    QCOOT x2;					// right position
    QCOOT y2;					// bottom position
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

inline void QRect::setLeft( QCOOT pos )
{
    x1 = pos;
}

inline void QRect::setTop( QCOOT pos )
{
    y1 = pos;
}

inline void QRect::setX( QCOOT x )
{
    x1 = x;
}

inline void QRect::setY( QCOOT y )
{
    y1 = y;
}

inline void QRect::setRight( QCOOT pos )
{
    x2 = pos;
}

inline void QRect::setBottom( QCOOT pos )
{
    y2 = pos;
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

inline QSize QRect::size() const
{
    return QSize( x2-x1+1, y2-y1+1 );
}

inline QCOOT QRect::width() const
{
    return x2-x1+1;
}

inline QCOOT QRect::height() const
{
    return y2-y1+1;
}

inline void QRect::setSize( const QSize &s )
{
    x2 = x1+s.width()-1;
    y2 = y1+s.height()-1;
}

#endif // inline functions


#endif // QRECT_H
