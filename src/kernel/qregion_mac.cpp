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
#include "qt_mac.h"

#define OLD_FALSE FALSE
#undef FALSE
#define FALSE (bool)OLD_FALSE

// NOT REVISED

static QRegion *empty_region = 0;

static void cleanup_empty_region()
{
    delete empty_region;
    empty_region = 0;
}

QRegion::QRegion()
{
    if ( !empty_region ) {			// avoid too many allocs
	qAddPostRoutine( cleanup_empty_region );
	empty_region = new QRegion( TRUE );
	Q_CHECK_PTR( empty_region );
    }
    data = empty_region->data;
    data->ref();
}

QRegion::QRegion(RgnHandle rgn)
{
    data = new QRegionData;
    Q_CHECK_PTR(data);
    data->is_null = FALSE;
    data->rgn = NewRgn();
    CopyRgn(rgn, data->rgn);
}    

QRegion::QRegion( bool is_null )
{
    data = new QRegionData;
    Q_CHECK_PTR( data );
    data->rgn = NewRgn();
    data->is_null = is_null;
}

QRegion::QRegion( const QRect &r, RegionType t )
{
    QRect rr = r.normalize();
    data = new QRegionData;
    Q_CHECK_PTR( data );
    data->is_null = FALSE;
    data->rgn = NewRgn();

    Rect rect;
    SetRect(&rect, rr.x(), rr.y(), rr.right()+1, rr.bottom()+1);
    OpenRgn();
    if ( t == Rectangle )			// rectangular region
	FrameRect(&rect);
    else if ( t == Ellipse )		// elliptic region
	FrameOval(&rect);
    CloseRgn(data->rgn);
}



QRegion::QRegion( const QPointArray &a, bool winding)
{
    data = new QRegionData;
    Q_CHECK_PTR( data );
    data->is_null = FALSE;
    data->rgn = NewRgn();

    OpenRgn();
    MoveTo( a[0].x(), a[0].y() );
    for ( unsigned int loopc = 1; loopc < a.size(); loopc++ ) {
	LineTo( a[loopc].x(), a[loopc].y() );
	MoveTo( a[loopc].x(), a[loopc].y() );
    }
    LineTo( a[0].x(), a[0].y() );
    CloseRgn(data->rgn);
}


QRegion::QRegion( const QRegion &r )
{
    data = r.data;
    data->ref();
}

//OPTIMIZATION FIXME, I think quickdraw can do this, this is just to get something going..
static RgnHandle qt_mac_bitmapToRegion(const QBitmap& bitmap)
{
    QImage image = bitmap.convertToImage();

    RgnHandle region = NewRgn(), rr;

#define AddSpan \
	{ \
    	   Rect rect; \
	   SetRect(&rect, prev1, y, (x-1)+1, (y+1)); \
	   rr = NewRgn(); \
    	   OpenRgn(); \
	   FrameRect(&rect); \
	   CloseRgn(rr); \
	   UnionRgn( rr, region, region ); \
	   DisposeRgn(rr); \
	}

    const int zero=0;
    bool little = image.bitOrder() == QImage::LittleEndian;

    int x, y;
    for (y=0; y<image.height(); y++) {
	uchar *line = image.scanLine(y);
	int w = image.width();
	uchar all=zero;
	int prev1 = -1;
	for (x=0; x<w; ) {
	    uchar byte = line[x/8];
	    if ( x>w-8 || byte!=all ) {
		if ( little ) {
		    for ( int b=8; b>0 && x<w; b-- ) {
			if ( !(byte&0x01) == !all ) {
			    // More of the same
			} else {
			    // A change.
			    if ( all!=zero ) {
				AddSpan
				all = zero;
			    } else {
				prev1 = x;
				all = ~zero;
			    }
			}
			byte >>= 1;
			x++;
		    }
		} else {
		    for ( int b=8; b>0 && x<w; b-- ) {
			if ( !(byte&0x80) == !all ) {
			    // More of the same
			} else {
			    // A change.
			    if ( all!=zero ) {
				AddSpan
				all = zero;
			    } else {
				prev1 = x;
				all = ~zero;
			    }
			}
			byte <<= 1;
			x++;
		    }
		}
	    } else {
		x+=8;
	    }
	}
	if ( all != zero ) {
	    AddSpan
	}
    }
    return region;
}


QRegion::QRegion( const QBitmap &bm )
{
    data = new QRegionData;
    Q_CHECK_PTR( data );
    data->is_null = FALSE;
    data->rgn = qt_mac_bitmapToRegion(bm);
}


QRegion::~QRegion()
{
    if ( data->deref() ) {
	DisposeRgn( data->rgn );
	delete data;
    }
}


QRegion &QRegion::operator=( const QRegion &r )
{
    r.data->ref();				// beware of r = r
    if ( data->deref() ) {
	DisposeRgn( data->rgn );
	delete data;
    }
    data = r.data;
    return *this;
}


QRegion QRegion::copy() const
{
    QRegion r( data->is_null );
    UnionRgn( data->rgn, r.data->rgn, r.data->rgn);
    return r;
}

bool QRegion::isNull() const
{
    return data->is_null;
}


bool QRegion::isEmpty() const
{
    return data->is_null || EmptyRgn(data->rgn);
}


bool QRegion::contains( const QPoint &p ) const
{
    Point point;
    point.h = p.x();
    point.v = p.y();
    return PtInRgn(point, data->rgn);
}

bool QRegion::contains( const QRect &r ) const
{
    Rect rect;
    SetRect(&rect, r.x(), r.y(), r.x() + r.width(), r.y() + r.height());
    return RectInRgn( &rect, data->rgn );
}


void QRegion::translate( int x, int y )
{
    if ( data == empty_region->data )
	return;
    detach();
    OffsetRgn( data->rgn, x, y);
}


QRegion QRegion::unite( const QRegion &r ) const
{
    QRegion result( FALSE );
    UnionRgn(data->rgn, r.data->rgn, result.data->rgn);
    return result;
}

QRegion QRegion::intersect( const QRegion &r ) const
{
    QRegion result( FALSE );
    SectRgn( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}


QRegion QRegion::subtract( const QRegion &r ) const
{
    QRegion result( FALSE );
    DiffRgn( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}


QRegion QRegion::eor( const QRegion &r ) const
{
    QRegion result( FALSE );
    XorRgn( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}


QRect QRegion::boundingRect() const
{
    Rect r;
    GetRegionBounds(data->rgn, &r);
    return QRect(r.left, r.top, (r.right - r.left), (r.bottom - r.top)); 
}

typedef QValueList<QRect> RectList;
static OSStatus mac_get_rgn_rect(UInt16 msg, RgnHandle, const Rect *rect, void *myd)
{
    if(msg == kQDRegionToRectsMsgParse) {
	RectList *rl = (RectList *)myd;
	rl->append(QRect(rect->left, rect->top, rect->right - rect->left, 
			 rect->bottom - rect->top));
    }
    return noErr;
}

QArray<QRect> QRegion::rects() const
{
    //get list
    RectList rl;
    OSStatus oss;
    RegionToRectsUPP cbk = NewRegionToRectsUPP(mac_get_rgn_rect);
    oss = QDRegionToRects(data->rgn, kQDParseRegionFromTopLeft, cbk, (void *)&rl);
    DisposeRegionToRectsUPP(cbk);

    //check for error
    if(oss != noErr) 
	return QArray<QRect>(0);

    //turn list into array
    QArray<QRect> ret(rl.count());
    int cnt = 0;
    for(RectList::Iterator it = rl.begin(); it != rl.end(); ++it) 
	ret[cnt++] = (*it);

    return ret; //done
}

void QRegion::setRects( const QRect *rects, int num )
{
    // Could be optimized
    *this = QRegion();
    for (int i=0; i<num; i++)
	*this |= rects[i];
}

bool QRegion::operator==( const QRegion &r ) const
{
    return data == r.data ? TRUE : EqualRgn( data->rgn, r.data->rgn );
}

