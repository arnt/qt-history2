/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpoint.cpp#6 $
**
** Implementation of QPoint class
**
** Author  : Haavard Nord
** Created : 931028
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#define QPOINT_C
#include "qpoint.h"
#include "qdstream.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpoint.cpp#6 $";
#endif

/*!
\class QPoint qpoint.h

\brief QPoint represents a point; the class defines the operations
that are commonly used on points.

QPoint consists of two QCOORD's, xp and yp, just like QSize does, but
the two classes aren't equivalent.  There are two main reasons why
they're not the same class: <ol><li> Structural equivalence doesn't
imply semantic equivalence, and a point is indeed different from a
size, and <li> Strict typing allows the compiler to catch more
errors. </dl>
*/

// --------------------------------------------------------------------------
// QPoint member functions
//

/*! Initializes the point from \e xpos and \e ypos. */
QPoint::QPoint( QCOORD xpos, QCOORD ypos )
{
    xp=xpos; yp=ypos;
}

/*! Adds (x+x, y+y) \e p to itself, and return itself afterwards */
QPoint &QPoint::operator+=( const QPoint &p )
{
    xp+=p.xp; yp+=p.yp; return *this;
}

/*! Subtracts (x-x, y-y) \e p from itself, and return itself afterwards. */
QPoint &QPoint::operator-=( const QPoint &p )
{
    xp-=p.xp; yp-=p.yp; return *this;
}

/*! Multiplies both of its own components by \e c, and return itself
afterwards. */ 
QPoint &QPoint::operator*=( int c )
{
    xp*=c; yp*=c; return *this;
}

/*! Multiplies both of its own components by \e c, and return itself
afterwards. */ 
QPoint &QPoint::operator*=( float c )
{
    xp=(QCOORD)(xp*c); yp=(QCOORD)(yp*c); return *this;
}

/*! Divides both of its own components by \e c, and return itself
afterwards.  Except in debug mode, division by zero is treated as
division by one. */ 
QPoint &QPoint::operator/=( int c )
{
    if ( c == 0 ) {
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
	return *this;
    }
    xp/=c; yp/=c; return *this;
}

/*! Divides both of its own components by \e c, and return itself
afterwards.  Except in debug mode, division by zero is treated as
division by one. */ 
QPoint &QPoint::operator/=( float c )
{
    if ( c == 0.0 ) {
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
	return *this;
    }
    xp=(QCOORD)(xp/c); yp=(QCOORD)(yp/c); return *this;
}

/*! Returns TRUE if both components of \e p1 are equal to
those of \e p2, FALSE otherwise. */
bool operator==( const QPoint & p1, const QPoint & p2 )
{
    return p1.xp == p2.xp && p1.yp == p2.yp;
}

/*! Returns FALSE if both components of \e p1 are equal to
those of \e p2, TRUE otherwise. */
bool operator!=( const QPoint & p1, const QPoint & p2 )
{
    return p1.xp != p2.xp || p1.yp != p2.yp;
}

/*! Returns the sum of \e p1 and \e p2; each component is added
separately. */
QPoint operator+( const QPoint & p1, const QPoint & p2 )
{
    return QPoint( p1.xp+p2.xp, p1.yp+p2.yp );
}

/*! Returns the difference of \e p1 and \e p2; each component is
subtracted separately. */
QPoint operator-( const QPoint & p1, const QPoint & p2 )
{
    return QPoint( p1.xp-p2.xp, p1.yp-p2.yp );
}

/*! Multiplies both of \e p's components by \e c and returns the
result. */ 
QPoint operator*( const QPoint &p, int c )
{
    return QPoint( p.xp*c, p.yp*c );
}

/*! Multiplies both of \e p's components by \e c and returns the
result. */ 
QPoint operator*( int c, const QPoint &p )
{
    return QPoint( p.xp*c, p.yp*c );
}

/*! Multiplies both of \e p's components by \e c and returns the
result. */ 
QPoint operator*( const QPoint &p, float c )
{
    return QPoint( (QCOORD)(p.xp*c), (QCOORD)(p.yp*c) );
}

/*! Multiplies both of \e p's components by \e c and returns the
result. */ 
QPoint operator*( float c, const QPoint &p )
{
    return QPoint( (QCOORD)(p.xp*c), (QCOORD)(p.yp*c) );
}

/*! Divides both of \e p's components by \e c and returns the
result. Division by zero is treated as the more common division by one
except in debug mode. */ 
QPoint operator/( const QPoint &p, int c )
{
    if ( c == 0 ) {
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
	return p;
    }
    return QPoint( p.xp/c, p.yp/c );
}

/*! Divides both of \e p's components by \e c and returns the
result. Division by zero is treated as the more common division by one
except in debug mode. */ 
QPoint operator/( const QPoint &p, float c )
{
    if ( c == 0.0 ) {
#if defined(CHECK_MATH)
	warning( "QPoint: Attempt to divide point by zero" );
#endif
	return p;
    }
    return QPoint( (QCOORD)(p.xp/c), (QCOORD)(p.yp/c) );
}

/*! Subtracts \e p from itself and returns its own value afterwards;
each component is subtracted separately. */
QPoint operator-( const QPoint &p )
{
    return QPoint( -p.xp, -p.yp );
}


// --------------------------------------------------------------------------
// QPoint stream functions
//

/*! Writes a QPoint to a QDataStream - actually writes first the x,
then the y component. */
QDataStream &operator<<( QDataStream &s, const QPoint &p )
{
    return s << (INT16)p.x() << (INT16)p.y();
}

/*! Reads a QPoint from a QDataStream.  First the x component, then
the y component is read. */
QDataStream &operator>>( QDataStream &s, QPoint &p )
{
    INT16 x, y;
    s >> x;  p.rx() = x;
    s >> y;  p.ry() = y;
    return s;
}
