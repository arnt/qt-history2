/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qregion_win.cpp#46 $
**
** Implementation of QRegion class for Win32
**
** Created : 940801
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit Professional Edition.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qregion.h"
#include "qpointarray.h"
#include "qbuffer.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qt_windows.h"


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
	CHECK_PTR( empty_region );
    }
    data = empty_region->data;
    data->ref();
}

QRegion::QRegion( bool is_null )
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->rgn = 0;
    data->is_null = is_null;
}

QRegion::QRegion( const QRect &r, RegionType t )
{
    QRect rr = r.normalize();
    data = new QRegionData;
    CHECK_PTR( data );
    data->is_null = FALSE;
    if ( t == Rectangle ) {			// rectangular region
	data->rgn = CreateRectRgn( rr.left(),	 rr.top(),
				   rr.right()+1, rr.bottom()+1 );
    } else if ( t == Ellipse ) {		// elliptic region
	data->rgn = CreateEllipticRgn( rr.left(),    rr.top(),
				       rr.right()+1, rr.bottom()+1 );
    }
}

QRegion::QRegion( const QPointArray &a, bool winding )
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->is_null = FALSE;
    data->rgn = CreatePolygonRgn( (POINT*)a.data(), a.size(),
				  winding ? WINDING : ALTERNATE );
}

QRegion::QRegion( const QRegion &r )
{
    data = r.data;
    data->ref();
}

HRGN qt_win_bitmapToRegion(const QBitmap& bitmap)
{
    HRGN region=0;
    QImage image = bitmap.convertToImage();
    const int maxrect=256;
    struct RData {
	RGNDATAHEADER header;
	RECT rect[maxrect];
    };
    RData data;

#define FlushSpans \
    { \
		data.header.dwSize = sizeof(RGNDATAHEADER); \
		data.header.iType = RDH_RECTANGLES; \
		data.header.nCount = n; \
		data.header.nRgnSize = 0; \
		data.header.rcBound.bottom = y; \
		HRGN r = ExtCreateRegion(0, \
		    sizeof(RGNDATAHEADER)+n*sizeof(RECT),(RGNDATA*)&data); \
		if ( region ) { \
		    CombineRgn(region, region, r, RGN_OR); \
		    DeleteObject( r ); \
		} else { \
		    region = r; \
		} \
		data.header.rcBound.top = y; \
		n=0; \
	}

#define AddSpan \
	{ \
	    data.rect[n].left=prev1; \
	    data.rect[n].top=y; \
	    data.rect[n].right=x-1+1; \
	    data.rect[n].bottom=y+1; \
	    n++; \
	    if ( n == maxrect ) { \
		FlushSpans \
	    } \
	}

    data.header.rcBound.top = 0;
    data.header.rcBound.left = 0;
    data.header.rcBound.right = image.width()-1;
    int n=0;

    // deal with 0<->1 problem (not on Windows anymore)
    int zero=0x00; //(qGray(image.color(0)) < qGray(image.color(1))
	    //? 0x00 : 0xff);

    int x, y;
    for (y=0; y<image.height(); y++) {
	uchar *line = image.scanLine(y);
	int w = image.width();
	uchar all=zero;
	int prev1 = -1;
	for (x=0; x<w; ) {
	    uchar byte = line[x/8];
	    if ( x>w-8 || byte!=all ) {
		for ( int b=8; b>0 && x<w; b-- ) {
		    if ( !(byte&0x80) == !all ) {
			// More of the same
		    } else {
			// A change.
			if ( all!=zero ) {
			    AddSpan;
			    all = zero;
			} else {
			    prev1 = x;
			    all = ~zero;
			}
		    }
		    byte <<= 1;
		    x++;
		}
	    } else {
		x+=8;
	    }
	}
	if ( all != zero ) {
	    AddSpan;
	}
    }
    if ( n ) {
	FlushSpans;
    }

    if ( !region ) {
	// Surely there is some better way.
	region = CreateRectRgn(0,0,1,1);
	CombineRgn(region, region, region, RGN_XOR);
    }

    return region;
}


QRegion::QRegion( const QBitmap & bm )
{
    data = new QRegionData;
    CHECK_PTR( data );
    data->is_null = FALSE;
    data->rgn = qt_win_bitmapToRegion(bm);
}


QRegion::~QRegion()
{
    if ( data->deref() ) {
	if ( data->rgn )
	    DeleteObject( data->rgn );
	delete data;
    }
}

QRegion &QRegion::operator=( const QRegion &r )
{
    r.data->ref();				// beware of r = r
    if ( data->deref() ) {
	if ( data->rgn )
	    DeleteObject( data->rgn );
	delete data;
    }
    data = r.data;
    return *this;
}


QRegion QRegion::copy() const
{
    QRegion r( data->is_null );
    if ( data->rgn ) {
	r.data->rgn = CreateRectRgn( 0, 0, 2, 2 );
	CombineRgn( r.data->rgn, data->rgn, 0, RGN_COPY );
    }
    return r;
}


bool QRegion::isNull() const
{
    return data->is_null;
}

bool QRegion::isEmpty() const
{
    return data->is_null || data->rgn == 0;
}


bool QRegion::contains( const QPoint &p ) const
{
    return data->rgn ? PtInRegion(data->rgn, p.x(), p.y()) : FALSE;
}

bool QRegion::contains( const QRect &r ) const
{
    if ( !data->rgn )
	return FALSE;
    RECT rect;
    SetRect( &rect, r.left(), r.top(), r.right(), r.bottom() );
    return RectInRegion( data->rgn, &rect );
}


void QRegion::translate( int dx, int dy )
{
    if ( !data->rgn )
	return;
    detach();
    OffsetRgn( data->rgn, dx, dy );
}


#define RGN_NOP -1

/*!
  Performs the actual OR, AND, SUB and XOR operation between regions.
  Sets the resulting region handle to 0 to indicate an empty region.
*/

QRegion QRegion::winCombine( const QRegion &r, int op ) const
{
    int both=RGN_NOP, left=RGN_NOP, right=RGN_NOP;
    switch ( op ) {
	case QRGN_OR:
	    both = RGN_OR;
	    left = right = RGN_COPY;
	    break;
	case QRGN_AND:
	    both = RGN_AND;
	    break;
	case QRGN_SUB:
	    both = RGN_DIFF;
	    left = RGN_COPY;
	    break;
	case QRGN_XOR:
	    both = RGN_XOR;
	    left = right = RGN_COPY;
	    break;
	default:
#if defined(CHECK_RANGE)
	    warning( "QRegion: Internal error in winCombine" );
#endif
    }

    QRegion result( FALSE );
    result.data->rgn = CreateRectRgn( 0, 0, 0, 0 );
    int res = NULLREGION;
    if ( data->rgn && r.data->rgn )
	res = CombineRgn( result.data->rgn, data->rgn, r.data->rgn, both );
    else if ( data->rgn && left != RGN_NOP )
	res = CombineRgn( result.data->rgn, data->rgn, data->rgn, left ); // 3rd param not used, but must be non-0
    else if ( r.data->rgn && right != RGN_NOP )
	res = CombineRgn( result.data->rgn, r.data->rgn, r.data->rgn, right ); // 3rd param not used, but must be non-0
    //##### do not delete this. A null pointer is different from an empty region in SelectClipRgn in qpainter_win! (M)
//     if ( res == NULLREGION ) {
// 	if ( result.data->rgn )
// 	    DeleteObject( result.data->rgn );
// 	result.data->rgn = 0;			// empty region
//     }
    return result;
}

QRegion QRegion::unite( const QRegion &r ) const
{
    return winCombine( r, QRGN_OR );
}

QRegion QRegion::intersect( const QRegion &r ) const
{
     return winCombine( r, QRGN_AND );
}

QRegion QRegion::subtract( const QRegion &r ) const
{
    return winCombine( r, QRGN_SUB );
}

QRegion QRegion::eor( const QRegion &r ) const
{
    return winCombine( r, QRGN_XOR );
}


QRect QRegion::boundingRect() const
{
    RECT r;
    int result = GetRgnBox(data->rgn, &r);
    if ( result == 0 || result == NULLREGION )
	return QRect(0,0,0,0);
    else
	return QRect(r.left, r.top, r.right-r.left, r.bottom-r.top);
}


QArray<QRect> QRegion::rects() const
{
    QArray<QRect> a;
    if ( data->rgn == 0 )
	return a;

    int numBytes = GetRegionData( data->rgn, 0, 0 );
    if ( numBytes == 0 )
	return a;

    char *buf = new char[numBytes];
    if ( buf == 0 )
	return a;

    RGNDATA *rd = (RGNDATA*)buf;
    if ( GetRegionData(data->rgn, numBytes, rd) == 0 ) {
	delete [] buf;
	return a;
    }

    a = QArray<QRect>( rd->rdh.nCount );
    RECT *r = (RECT*)rd->Buffer;
    for ( int i=0; i<(int)a.size(); i++ ) {
	a[i].setCoords( r->left, r->top, r->right-1, r->bottom-1);
	r++;
    }

    delete [] buf;

    return a;
}


bool QRegion::operator==( const QRegion &r ) const
{
    if ( data == r.data )			// share the same data
	return TRUE;
    if ( (data->rgn == 0) ^ (r.data->rgn == 0)) // one is empty, not both
	return FALSE;
    return data->rgn == 0 ?
	TRUE :					// both empty
	EqualRgn( data->rgn, r.data->rgn );	// both non-empty
}
