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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
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

#ifdef Q_WS_MACX
#define RGN_CACHE_SIZE 200
static bool rgncache_init=FALSE;
static int rgncache_used;
static RgnHandle rgncache[RGN_CACHE_SIZE];
void cleanup_rgncache() {
    for(int i = 0; i < RGN_CACHE_SIZE; i++) {
	if(rgncache[i]) {
	    rgncache_used--;
	    DisposeRgn(rgncache[i]);
	    rgncache[i] = 0;
	}
    }
}
inline RgnHandle get_rgn() {
    RgnHandle ret = NULL;
    if(!rgncache_init) {
	rgncache_used = 0;
	rgncache_init = TRUE;
	for(int i = 0; i < RGN_CACHE_SIZE; i++) 
	    rgncache[i] = 0;
	qAddPostRoutine(cleanup_rgncache);
    } else if(rgncache_used) {
	for(int i = 0; i < RGN_CACHE_SIZE; i++) {
	    if(rgncache[i]) {
		ret = rgncache[i];
		SetEmptyRgn(ret);
		rgncache[i] = NULL;
		rgncache_used--;
		break;
	    }
	}
    }
    if(!ret) 
	ret = NewRgn();
    return ret;
}
inline void dispose_rgn(RgnHandle r) {
    if(rgncache_init && rgncache_used < RGN_CACHE_SIZE) {
	for(int i = 0; i < RGN_CACHE_SIZE; i++) {
	    if(!rgncache[i]) {
		rgncache_used++;
		rgncache[i] = r;
		break;
	    }
	}
    } else {
	DisposeRgn(r);
    }
}
#else
#define get_rgn NewRgn
#define dispose_rgn DisposeRgn
#endif

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
    data->is_rect = FALSE;
    data->rgn = get_rgn();
    CopyRgn(rgn, data->rgn);
}    

void 
QRegion::rectifyRegion()
{
    if(!data->is_rect)
	return;
    if(data->is_null) {
	detach();
	data->is_null = FALSE;
    }
    data->is_rect = FALSE;
    data->rgn = get_rgn();
    if(!data->rect.isEmpty()) {
	Rect rect;
	SetRect(&rect, data->rect.x(), data->rect.y(), 
		data->rect.right()+1, data->rect.bottom()+1);
	OpenRgn();
	FrameRect(&rect);
	CloseRgn(data->rgn);
    }
}

void 
*QRegion::handle(bool require_rgn) const
{
    if(require_rgn) {
	if(data->is_rect)
	    ((QRegion *)this)->rectifyRegion();
	return (void *)data->rgn;
    }
    if(data->is_rect)
	return (void *)&data->rect;
    return (void *)data->rgn;
}

QRegion::QRegion( bool is_null )
{
    data = new QRegionData;
    Q_CHECK_PTR( data );
    if((data->is_null = is_null)) {
	data->is_rect = TRUE;
	data->rect = QRect();
    } else {
	data->is_rect = FALSE;
	data->rgn = get_rgn();
    }
}

QRegion::QRegion( const QRect &r, RegionType t )
{
    QRect rr = r.normalize();
    data = new QRegionData;
    Q_CHECK_PTR( data );
    data->is_null = FALSE;
    if(t == Rectangle )	{		// rectangular region
	data->is_rect = TRUE;
	data->rect = r;
    } else {
	Rect rect;
	SetRect(&rect, rr.x(), rr.y(), rr.right()+1, rr.bottom()+1);
	data->is_rect = FALSE;
	data->rgn = get_rgn();
	OpenRgn();
	FrameOval(&rect);
	CloseRgn(data->rgn);
    }
}

QRegion::QRegion( const QPointArray &a, bool winding)
{
    data = new QRegionData;
    Q_CHECK_PTR( data );
    data->is_null = FALSE;
    data->is_rect = FALSE;
    data->rgn = get_rgn();

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

    RgnHandle region = get_rgn(), rr;

#define AddSpan \
	{ \
    	   Rect rect; \
	   SetRect(&rect, prev1, y, (x-1)+1, (y+1)); \
	   rr = get_rgn(); \
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
    data->is_rect = FALSE;
#if 0 //this should work, but didn't
    data->rgn = get_rgn();
    BitMapToRegion(data->rgn, (BitMap *)*GetGWorldPixMap((GWorldPtr)bm.handle()));
#else
    data->rgn = qt_mac_bitmapToRegion(bm);
#endif
}


QRegion::~QRegion()
{
    if ( data->deref() ) {
	if(!data->is_rect)
	    dispose_rgn( data->rgn );
	delete data;
    }
}


QRegion &QRegion::operator=( const QRegion &r )
{
    r.data->ref();				// beware of r = r
    if ( data->deref() ) {
	if(!data->is_rect)
	    dispose_rgn( data->rgn );
	delete data;
    }
    data = r.data;
    return *this;
}


QRegion QRegion::copy() const
{
    if(data->is_null) 
	return QRegion(TRUE);
     else if(data->is_rect) 
	return QRegion(data->rect);

    QRegion r( FALSE );
    CopyRgn( data->rgn, r.data->rgn);
    return r;
}

bool QRegion::isNull() const
{
    return data->is_null;
}

bool QRegion::isEmpty() const
{
    if(data->is_null)
	return TRUE;
    if(data->is_rect)
	return data->rect.isEmpty();
    return EmptyRgn(data->rgn);
}

bool QRegion::contains( const QPoint &p ) const
{
    if(data->is_null)
	return FALSE;
    else if(data->is_rect) 
	return data->rect.contains(p);

    Point point;
    point.h = p.x();
    point.v = p.y();
    return PtInRgn(point, data->rgn);
}

bool QRegion::contains( const QRect &r ) const
{
    if(data->is_null)
	return FALSE;
    else if(data->is_rect) 
	return data->rect.intersects(r);

    Rect rect;
    SetRect(&rect, r.x(), r.y(), r.x() + r.width(), r.y() + r.height());
    return RectInRgn( &rect, data->rgn );
}

void QRegion::translate( int x, int y )
{
    if ( data == empty_region->data )
	return;
    detach();
    if(data->is_rect) 
	data->rect.moveBy(x, y);
    else
	OffsetRgn( data->rgn, x, y);
}

QRegion QRegion::unite( const QRegion &r ) const
{
    if(data->is_null || r.data->is_null ) 
	return (!data->is_null ? this : &r)->copy();

    if(data->is_rect && r.data->is_rect && data->rect.contains(r.data->rect))
	return copy();

    if(data->is_rect) 
	((QRegion *)this)->rectifyRegion();
    if(r.data->is_rect)
	((QRegion *)&r)->rectifyRegion();
    QRegion result( FALSE );
    UnionRgn(data->rgn, r.data->rgn, result.data->rgn);
    return result;
}

QRegion QRegion::intersect( const QRegion &r ) const
{
    if(data->is_null || r.data->is_null ) 
	return QRegion();

    if(data->is_rect && r.data->is_rect) 
	return QRegion(data->rect & r.data->rect);

    if(data->is_rect) 
	((QRegion *)this)->rectifyRegion();
    if(r.data->is_rect) 
	((QRegion *)&r)->rectifyRegion();
    QRegion result( FALSE );
    SectRgn( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}

QRegion QRegion::subtract( const QRegion &r ) const
{
    if(data->is_null || r.data->is_null ) 
	return (!data->is_null ? this : &r)->copy();

    if(data->is_rect) 
	((QRegion *)this)->rectifyRegion();
    if(r.data->is_rect)
	((QRegion *)&r)->rectifyRegion();
    QRegion result( FALSE );
    DiffRgn( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}

QRegion QRegion::eor( const QRegion &r ) const
{
    if(data->is_null || r.data->is_null ) 
	return (!data->is_null ? this : &r)->copy();

    if(data->is_rect) 
	((QRegion *)this)->rectifyRegion();
    if(r.data->is_rect)
	((QRegion *)&r)->rectifyRegion();
    QRegion result( FALSE );
    XorRgn( data->rgn, r.data->rgn, result.data->rgn );
    return result;
}

QRect QRegion::boundingRect() const
{
    if(data->is_rect)
	return data->rect;
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
    if(data->is_rect) {
	QArray<QRect> ret(1);
	ret[0] = data->rect;
	return ret;
    }

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
    if(num == 1) {
	*this = QRegion(*rects);
	return;
    }
    *this = QRegion();
    for (int i=0; i<num; i++)
	*this |= rects[i];
}

bool QRegion::operator==( const QRegion &r ) const
{
    if(data == r.data)
	return TRUE;
    if(data->is_rect || r.data->is_rect) {
	if(data->is_rect && r.data->is_rect)
	    return data->rect == r.data->rect;
	return FALSE;
    }
    return EqualRgn( data->rgn, r.data->rgn );
}

