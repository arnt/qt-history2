/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion_mac.cpp
**
** Implementation of QRegion class for mac
**
** Created : 940729
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qregion.h"
#include "qpointarray.h"
#include "qbuffer.h"
#include "qimage.h"
#include "qbitmap.h"

// NOT REVISED

/*!
  Constructs a null region.
  \sa isNull()
*/

QRegion::QRegion()
{
    qDebug( "QRegion::QRegion" );
}

/*!
  Internal constructor that creates a null region.
*/

QRegion::QRegion( bool )
{
    qDebug( "QRegion::QRegion bool" );
}

/*!
\overload
 */

QRegion::QRegion( const QRect &, RegionType )
{
    qDebug( "QRegion::QRegion QRect" );
}


/*!
  Constructs a polygon region from the point array \a a.

  If \a winding is TRUE, the polygon
  region is filled using the winding algorithm, otherwise the default
  even-odd fill algorithm is used.

  This constructor may create complex regions that will slow
  down painting when used.
*/

QRegion::QRegion( const QPointArray &, bool )
{
    qDebug( "QRegion::QRegion QPointArray" );
}


/*!
  Constructs a new region which is equal to \a r.
*/

QRegion::QRegion( const QRegion & )
{
    qDebug( "QRegion::QRegion QRegion" );
}

/*!
  Constructs a region from the bitmap \a bm.

  The resulting region consists of the pixels in \a bm that are \c
  color1, as if each pixel was a 1 by 1 rectangle.

  This constructor may create complex regions that will slow
  down painting when used. Note that drawing masked pixmaps
  can be done much faster using QPixmap::setMask().

*/
QRegion::QRegion( const QBitmap & )
{
    qDebug( "QRegion::QRegion QBitmap" );
}

/*!
  Destructs the region.
*/

QRegion::~QRegion()
{
    qDebug( "QRegion::~QRegion" );
}


/*!
  Assigns \a r to this region and returns a reference to the
  region.

*/

QRegion &QRegion::operator=( const QRegion & )
{
    qDebug( "QRegion::operator" );
    return *this;
}


/*!
  Returns a \link shclass.html deep copy\endlink of the region.

  \sa detach()
*/

QRegion QRegion::copy() const
{
    qDebug( "QRegion::copy" );
    QRegion r( data->is_null );
    return r;
}

/*!
  Returns TRUE if the region is a null region, otherwise FALSE.

  A null region is a region that has not been initialized. A
  null region is always empty.

  \sa isEmpty()
*/

bool QRegion::isNull() const
{
    qDebug( "QRegion::isNull" );
    return data->is_null;
}


/*!
  Returns TRUE if the region is empty, or FALSE if it is non-empty.
  An empty region is a region that contains no points.

  Example:
  \code
    QRegion r1( 10, 10, 20, 20 );
    QRegion r2( 40, 40, 20, 20 );
    QRegion r3;
    r1.isNull();		// FALSE
    r1.isEmpty();		// FALSE
    r3.isNull();		// TRUE
    r3.isEmpty();		// TRUE
    r3 = r1.intersect( r2 );	// r3 = intersection of r1 and r2
    r3.isNull();		// FALSE
    r3.isEmpty();		// TRUE
    r3 = r1.unite( r2 );	// r3 = union of r1 and r2
    r3.isNull();		// FALSE
    r3.isEmpty();		// FALSE
  \endcode

  \sa isNull()
*/

bool QRegion::isEmpty() const
{
    qDebug( "QRegion::isEmpty" );
    return false;
}


/*!
  Returns TRUE if the region contains the point \a p, or FALSE if \a p is
  outside the region.
*/

bool QRegion::contains( const QPoint & ) const
{
    qDebug( "QRegion::contains" );
    return false;
}

/*!
  Returns TRUE if the region overlaps the rectangle \a r, or FALSE if \a r is
  completely outside the region.
*/

bool QRegion::contains( const QRect & ) const
{
    qDebug( "QRegion::contains QRect" );
    return false;
}


/*!
  Translates (moves) the region \a dx along the X axis and \a dy along the Y axis.
*/

void QRegion::translate( int, int )
{
    qDebug( "QRegion::translate" );
}


/*!
  Returns a region which is the union of this region and \a r.

    <img src=runion.png>

  The figure shows the union of two elliptical regions.
*/

QRegion QRegion::unite( const QRegion & ) const
{
    qDebug( "QRegion::unite" );
    QRegion result( FALSE );
    return result;
}

/*!
  Returns a region which is the intersection of this region and \a r.

  <img src=rintersect.png>

  The figure shows the intersection of two elliptical regions.
*/

QRegion QRegion::intersect( const QRegion & ) const
{
    qDebug( "QRegion::intersect" );
    QRegion result( FALSE );
    return result;
}

/*!
  Returns a region which is \a r subtracted from this region.

  <img src=rsubtract.png>

  The figure shows the result when the ellipse on the right is subtracted
  from the ellipse on the left. (\c left-right )
*/

QRegion QRegion::subtract( const QRegion & ) const
{
    qDebug( "QRegion::subtract" );
    QRegion result( FALSE );
    return result;
}

/*!
  Returns a region which is the exclusive or (XOR) of this region and \a r.

  <img src=rxor.png>

  The figure shows the exclusive or of two elliptical regions.
*/

QRegion QRegion::eor( const QRegion & ) const
{
    qDebug( "QRegion::eor" );
    QRegion result( FALSE );
    return result;
}


/*!
  Returns the bounding rectangle of this region.
  An empty region gives a rectangle that is  QRect::isNull().
*/

QRect QRegion::boundingRect() const
{
    qDebug( "QRegion::boundingRect" );
    return QRect();
}


/*
  This is how X represents regions internally.
*/

struct BOX {
    short x1, x2, y1, y2;
};

struct _XRegion {
    long size;
    long numRects;
    BOX *rects;
    BOX  extents;
};


/*!
  Returns an array of non-overlapping rectangles that make up the region.

  The union of all the rectangles is equal to the original region.
*/

QArray<QRect> QRegion::rects() const
{
    qDebug( "QRegion::rects" );
    QArray<QRect> a( (int)1 );
    return a;
}

/*!
  Sets the region to be the given set of rectangles.  The rectangles
  \e must be optimal Y-X sorted bands as follows:
   <ul>
    <li> The rectangles must not intersect
    <li> All rectangles with a given top coordinate must have the same height.
    <li> No two rectangles may abut horizontally (they should be combined
		into a single wider rectangle in that case).
    <li> The rectangles must be sorted ascendingly by Y as the major sort key
		and X as the minor sort key.
   </ul>
  \internal
  Only some platforms have that restriction (QWS).
*/
void QRegion::setRects( const QRect *rects, int num )
{
    qDebug( "QRegion::setRects" );
    // Could be optimized
    *this = QRegion();
    for (int i=0; i<num; i++)
	*this |= rects[i];
}

/*!
  Returns TRUE if the region is equal to \a r, or FALSE if the regions are
  different.
*/

bool QRegion::operator==( const QRegion & ) const
{
    qDebug( "QRegion::operator" );
    return false;
}

/*!
  \fn bool QRegion::operator!=( const QRegion &r ) const
  Returns TRUE if the region is different from \a r, or FALSE if the regions
  are equal.
*/
