/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qrect.h#24 $
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

    int	   left()	const;
    int	   top()	const;
    int	   right()	const;
    int	   bottom()	const;
    int	   x()		const;
    int	   y()		const;
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

    void   moveTopLeft( const QPoint &p );
    void   moveBottomRight( const QPoint &p );
    void   moveTopRight( const QPoint &p );
    void   moveBottomLeft( const QPoint &p );
    void   moveCenter( const QPoint &p );
    void   moveBy( int dx, int dy );


    void   setRect( int x, int y, int w, int h );
    void   setCoords( int x1, int y1, int x2, int y2 );

    QSize  size()	const;
    int	   width()	const;
    int	   height()	const;
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
#if defined(OBSOLETE)
public:
    void   setTopLeft( const QPoint &p );
    void   setBottomRight( const QPoint &p );
    void   setTopRight( const QPoint &p );
    void   setBottomLeft( const QPoint &p );
    void   setCenter( const QPoint &p );
    void   translate( int dx, int dy );
    void   move( int dx, int dy );
#endif
};


/*****************************************************************************
  QRect stream functions
 *****************************************************************************/

QDataStream &operator<<( QDataStream &, const QRect & );
QDataStream &operator>>( QDataStream &, QRect & );


/*****************************************************************************
  QRect inline member functions
 *****************************************************************************/

inline QRect::QRect( int left, int top, int width, int height )
{
    x1 = (QCOORD)left;
    y1 = (QCOORD)top;
    x2 = (QCOORD)(left+width-1);
    y2 = (QCOORD)(top+height-1);
}

inline bool QRect::isNull() const
{ return x2 == x1-1 && y2 == y1-1; }

inline bool QRect::isEmpty() const
{ return x1 > x2 || y1 > y2; }

inline bool QRect::isValid() const
{ return x1 <= x2 && y1 <= y2; }

inline int QRect::left() const
{ return x1; }

inline int QRect::top()	const
{ return y1; }

inline int QRect::right() const
{ return x2; }

inline int QRect::bottom() const
{ return y2; }

inline int QRect::x() const
{ return x1; }

inline int QRect::y() const
{ return y1; }

inline void QRect::setLeft( int pos )
{ x1 = (QCOORD)pos; }

inline void QRect::setTop( int pos )
{ y1 = (QCOORD)pos; }

inline void QRect::setRight( int pos )
{ x2 = (QCOORD)pos; }

inline void QRect::setBottom( int pos )
{ y2 = (QCOORD)pos; }

inline void QRect::setX( int x )
{ x1 = (QCOORD)x; }

inline void QRect::setY( int y )
{ y1 = (QCOORD)y; }

inline QPoint QRect::topLeft() const
{ return QPoint(x1, y1); }

inline QPoint QRect::bottomRight() const
{ return QPoint(x2, y2); }

inline QPoint QRect::topRight() const
{ return QPoint(x2, y1); }

inline QPoint QRect::bottomLeft() const
{ return QPoint(x1, y2); }

inline QPoint QRect::center() const
{ return QPoint((x1+x2)/2, (y1+y2)/2); }

inline int QRect::width() const
{ return  x2 - x1 + 1; }

inline int QRect::height() const
{ return  y2 - y1 + 1; }

inline QSize QRect::size() const
{ return QSize(x2-x1+1, y2-y1+1); }

#if defined(OBSOLETE)
inline void   QRect::setTopLeft( const QPoint &p )
{ qObsolete("QRect", "setTopLeft" , "moveTopLeft" ); moveTopLeft( p ); }
inline void   QRect::setBottomRight( const QPoint &p )
{ 
    qObsolete("QRect", "setBottomRight" , "moveBottomRight" ); 
    moveBottomRight( p ); 
}
inline void   QRect::setTopRight( const QPoint &p )
{ qObsolete("QRect", "setTopRight" , "moveTopRight" ); moveTopRight( p ); }
inline void   QRect::setBottomLeft( const QPoint &p )
{ 
    qObsolete("QRect", "setBottomLeft" , "moveBottomLeft" ); 
    moveBottomLeft( p ); 
}
inline void   QRect::setCenter( const QPoint &p )
{ qObsolete("QRect", "setCenter" , "moveCenter" ); moveCenter( p ); }
inline void   QRect::translate( int dx, int dy )
{ qObsolete("QRect", "translate" , "moveBy" ); moveBy( dx, dy ); }
inline void   QRect::move( int dx, int dy ) 
{ qObsolete("QRect", "move" , "moveBy" ); moveBy( dx, dy ); }
#endif


#endif // QRECT_H
