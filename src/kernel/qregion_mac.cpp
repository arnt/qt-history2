/****************************************************************************
**
** Implementation of QRegion class for mac.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qregion.h"
#include "qpointarray.h"
#include "qbuffer.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qt_mac.h"

#if defined( Q_WS_MACX )
#define RGN_CACHE_SIZE 200
static bool rgncache_init = false;
static int rgncache_used;
static RgnHandle rgncache[RGN_CACHE_SIZE];
static void qt_mac_cleanup_rgncache()
{
    rgncache_init = false;
    for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
	if(rgncache[i]) {
	    --rgncache_used;
	    DisposeRgn(rgncache[i]);
	    rgncache[i] = 0;
	}
    }
}

RgnHandle qt_mac_get_rgn()
{
    RgnHandle ret = 0;
    if(!rgncache_init) {
	rgncache_used = 0;
	rgncache_init = true;
	for(int i = 0; i < RGN_CACHE_SIZE; ++i)
	    rgncache[i] = 0;
	qAddPostRoutine(qt_mac_cleanup_rgncache);
    } else if(rgncache_used) {
	for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
	    if(rgncache[i]) {
		ret = rgncache[i];
		SetEmptyRgn(ret);
		rgncache[i] = 0;
		--rgncache_used;
		break;
	    }
	}
    }
    if(!ret)
	ret = NewRgn();
    return ret;
}

void qt_mac_dispose_rgn(RgnHandle r)
{
    if(rgncache_init && rgncache_used < RGN_CACHE_SIZE) {
	for(int i = 0; i < RGN_CACHE_SIZE; ++i) {
	    if(!rgncache[i]) {
		++rgncache_used;
		rgncache[i] = r;
		break;
	    }
	}
    } else {
	DisposeRgn(r);
    }
}

#else
RgnHandle qt_mac_get_rgn() { return NewRgn(); }
void qt_mac_dispose_rgn(RgnHandle r) { DisposeRgn(r); }
#endif

QRegion::QRegionData QRegion::shared_empty = { Q_ATOMIC_INIT(1), 0, 0 };

QRegion::QRegion() : d(&shared_empty)
{
    ++d->ref;
}

void QRegion::rectifyRegion()
{
    if(d->rgn) {
	Q_ASSERT(!d->rect);
	return;
    }
    detach();
    d->rgn = qt_mac_get_rgn();
    if(d->rect && !d->rect->isEmpty()) {
	Rect r;
	SetRect(&r, d->rect->x(), d->rect->y(), d->rect->right() + 1, d->rect->bottom() + 1);
	OpenRgn();
	FrameRect(&r);
	CloseRgn(d->rgn);
    }
    delete d->rect;
    d->rect = NULL;
}

/*!
    \internal
*/
RgnHandle QRegion::handle(bool require_rgn) const
{
    if(require_rgn && !d->rgn)
	const_cast<QRegion *>(this)->rectifyRegion();
    return d->rect ? 0 : d->rgn;
}

QRegion::QRegion(const QRect &r, RegionType t)
{
    if(r.isEmpty()) {
	d = &shared_empty;
	++d->ref;
	return;
    }
    QRect rr = r.normalize();
    d = new QRegionData;
    d->ref = 1;
    if(t == Rectangle) {
	d->rgn = NULL;
	d->rect = new QRect(r.x(), r.y(), r.width(), r.height());
    } else {
	d->rect = NULL;
	Rect rect;
	SetRect(&rect, rr.x(), rr.y(), rr.right() + 1, rr.bottom() + 1);
	d->rgn = qt_mac_get_rgn();
	OpenRgn();
	FrameOval(&rect);
	CloseRgn(d->rgn);
    }
}

//### We do not support winding yet, how do we do that?? --SAM
QRegion::QRegion(const QPointArray &a, bool)
{
    if(a.size() > 0) {
	d = new QRegionData;
	d->ref = 1;
	d->rect = NULL;
	d->rgn = qt_mac_get_rgn();
	OpenRgn();
	MoveTo(a[0].x(), a[0].y());
	for(int loopc = 1; loopc < a.size(); ++loopc) {
	    LineTo(a[loopc].x(), a[loopc].y());
	    MoveTo(a[loopc].x(), a[loopc].y());
	}
	LineTo(a[0].x(), a[0].y());
	CloseRgn(d->rgn);
    } else {
	d = &shared_empty;
	++d->ref;
    }
}

QRegion::QRegion(const QRegion &r)
{
    d = r.d;
    ++d->ref;
}

//OPTIMIZATION FIXME, I think quickdraw can do this, this is just to get something going..
static RgnHandle qt_mac_bitmapToRegion(const QBitmap& bitmap)
{
    QImage image = bitmap.convertToImage();
    RgnHandle region = qt_mac_get_rgn(), rr;
#define AddSpan \
	{ \
	   Rect rect; \
	   SetRect(&rect, prev1, y, (x-1)+1, (y+1)); \
	   rr = qt_mac_get_rgn(); \
	   OpenRgn(); \
	   FrameRect(&rect); \
	   CloseRgn(rr); \
	   UnionRgn(rr, region, region); \
	   DisposeRgn(rr); \
	}

    int x, y;
    const int zero = 0;
    bool little = image.bitOrder() == QImage::LittleEndian;
    for(y = 0; y < image.height(); ++y) {
	uchar *line = image.scanLine(y);
	int w = image.width();
	uchar all = zero;
	int prev1 = -1;
	for(x = 0; x < w; ) {
	    uchar byte = line[x / 8];
	    if(x > w - 8 || byte != all) {
		if(little) {
		    for(int b = 8; b > 0 && x < w; --b) {
			if(!(byte & 0x01) == !all) {			    // More of the same
			} else {
			    // A change.
			    if(all != zero) {
				AddSpan
				all = zero;
			    } else {
				prev1 = x;
				all = ~zero;
			    }
			}
			byte >>= 1;
			++x;
		    }
		} else {
		    for(int b = 8; b > 0 && x < w; --b) {
			if(!(byte & 0x80) == !all) {			    // More of the same
			} else {			    // A change.
			    if(all != zero) {
				AddSpan
				all = zero;
			    } else {
				prev1 = x;
				all = ~zero;
			    }
			}
			byte <<= 1;
			++x;
		    }
		}
	    } else {
		x += 8;
	    }
	}
	if(all != zero) {
	    AddSpan
	}
    }
    return region;
}

QRegion::QRegion(const QBitmap &bm)
{
    if( bm.isNull() ) {
	d = &shared_empty;
	++d->ref;
    } else {
	d = new QRegionData;
	d->ref = 1;
	d->rect = NULL;
#if 0 //this should work, but didn't
	d->rgn = qt_mac_get_rgn();
	BitMapToRegion(d->rgn, (BitMap *)*GetGWorldPixMap((GWorldPtr)bm.handle()));
#else
	d->rgn = qt_mac_bitmapToRegion(bm);
#endif
    }
}

void QRegion::cleanUp(QRegion::QRegionData *x)
{
    if(x == &shared_empty)
	return;
    if(x->rgn)
	qt_mac_dispose_rgn(x->rgn);
    delete x;
}

QRegion::~QRegion()
{
    if(!--d->ref)
	cleanUp(d);
}

QRegion &QRegion::operator=(const QRegion &r)
{
    if(r.d == d)
	return *this;
    QRegionData *x = r.d;
    ++x->ref;
    QRegionData *old = qAtomicSetPtr(&d, x);
    if(!--old->ref)
	cleanUp(old);
    return *this;
}

QRegion QRegion::copy() const
{
    QRegion r;
    QRegionData *x = new QRegionData;
    x->ref = 1;
    if(d == &shared_empty) {
	x->rgn = NULL;
	x->rect = new QRect(0, 0, 0, 0);
    } else if(d->rect) {
	x->rgn = NULL;
	x->rect = new QRect(d->rect->x(), d->rect->y(), d->rect->width(), d->rect->height());
    } else {
	x->rect = NULL;
	x->rgn = qt_mac_get_rgn();
	CopyRgn(d->rgn, x->rgn);
    }
    QRegionData *old = qAtomicSetPtr(&r.d, x);
    if(--old->ref) 
	cleanUp(old);
    return r;
}

bool QRegion::isEmpty() const
{
    if(d == &shared_empty)
	return true;
    if(d->rect)
	return d->rect->isEmpty();
    return EmptyRgn(d->rgn);
}

bool QRegion::contains(const QPoint &p) const
{
    if(d == &shared_empty)
	return false;
    if(d->rect)
	return d->rect->contains(p);
    Point point;
    point.h = p.x();
    point.v = p.y();
    return PtInRgn(point, d->rgn);
}

bool QRegion::contains(const QRect &r) const
{
    if(d == &shared_empty)
	return false;
    if(d->rect)
	return d->rect->intersects(r);
    Rect rect;
    SetRect(&rect, r.x(), r.y(), r.x() + r.width(), r.y() + r.height());
    return RectInRgn(&rect, d->rgn);
}

void QRegion::translate(int x, int y)
{
    if(d == &shared_empty)
	return;
    detach();
    if(d->rect) 
	d->rect->moveBy(x, y);
    else 
	OffsetRgn(d->rgn, x, y);
}

QRegion QRegion::unite(const QRegion &r) const
{
    if(d == &shared_empty)
	return r;
    if(r.d == &shared_empty)
	return *this;
    if(d->rect && r.d->rect) {
	if(d->rect->contains(*r.d->rect))
	    return copy();
    }
    if(!d->rgn)
	const_cast<QRegion *>(this)->rectifyRegion();
    if(!r.d->rgn)
	const_cast<QRegion &>(r).rectifyRegion();
    QRegion result;
    result.detach();
    delete result.d->rect;
    result.d->rect = NULL;
    result.d->rgn = qt_mac_get_rgn();
    UnionRgn(d->rgn, r.d->rgn, result.d->rgn);
    return result;
}

QRegion QRegion::intersect(const QRegion &r) const
{
    if(d == &shared_empty || r.d == &shared_empty)
	return QRegion();
    if(d->rect && r.d->rect)
	return QRegion(*d->rect & *r.d->rect);
    if(!d->rgn)
	const_cast<QRegion *>(this)->rectifyRegion();
    if(!r.d->rgn)
	const_cast<QRegion &>(r).rectifyRegion();
    QRegion result;
    result.detach();
    delete result.d->rect;
    result.d->rect = NULL;
    result.d->rgn = qt_mac_get_rgn();
    SectRgn(d->rgn, r.d->rgn, result.d->rgn);
    return result;
}

QRegion QRegion::subtract(const QRegion &r) const
{
    if(d == &shared_empty || r.d == &shared_empty)
	return *this;
    if(!d->rgn)
	const_cast<QRegion *>(this)->rectifyRegion();
    if(!r.d->rgn)
	const_cast<QRegion &>(r).rectifyRegion();
    QRegion result;
    result.detach();
    delete result.d->rect;
    result.d->rect = NULL;
    result.d->rgn = qt_mac_get_rgn();
    DiffRgn(d->rgn, r.d->rgn, result.d->rgn);
    return result;
}

QRegion QRegion::eor(const QRegion &r) const
{
    if(d == &shared_empty)
	return r;
    if(r.d == &shared_empty)
	return *this;
    if(!d->rgn)
	const_cast<QRegion *>(this)->rectifyRegion();
    if(!r.d->rgn)
	const_cast<QRegion &>(r).rectifyRegion();
    QRegion result;
    result.detach();
    delete result.d->rect;
    result.d->rect = NULL;
    result.d->rgn = qt_mac_get_rgn();
    XorRgn(d->rgn, r.d->rgn, result.d->rgn);
    return result;
}

QRect QRegion::boundingRect() const
{
    if(d == &shared_empty)
	return QRect(0, 0, 0, 0);
    if(d->rect)
	return *d->rect;
    Rect r;
    GetRegionBounds(d->rgn, &r);
    return QRect(r.left, r.top, (r.right - r.left), (r.bottom - r.top));
}

typedef QList<QRect> RectList;
static OSStatus qt_mac_get_rgn_rect(UInt16 msg, RgnHandle, const Rect *rect, void *myd)
{
    if(msg == kQDRegionToRectsMsgParse) {
	RectList *rl = static_cast<RectList *>(myd);
	QRect rct(rect->left, rect->top, (rect->right - rect->left), (rect->bottom - rect->top));
	if(!rct.isEmpty())
	    rl->append(rct);
    }
    return noErr;
}

QVector<QRect> QRegion::rects() const
{
    if(d == &shared_empty)
	return QVector<QRect>(0);
    if(d->rect) {
	if(d->rect->isEmpty())
	    return QVector<QRect>(0);
	QVector<QRect> ret(1);
	ret[0] = *d->rect;
	return ret;
    }

    RectList rl;
    OSStatus oss;
    RegionToRectsUPP cbk = NewRegionToRectsUPP(qt_mac_get_rgn_rect);
    oss = QDRegionToRects(d->rgn, kQDParseRegionFromTopLeft, cbk, static_cast<void *>(&rl));
    DisposeRegionToRectsUPP(cbk);

    if(oss != noErr)
	return QVector<QRect>(0);

    QVector<QRect> ret(rl.count());
    for(int cnt = 0; cnt < rl.count(); ++cnt )
	ret[cnt] = rl.at(cnt);

    return ret;
}

void QRegion::setRects(const QRect *rects, int num)
{
    // Could be optimized
    if(num == 1) {
	*this = QRegion(*rects);
	return;
    }
    *this = QRegion();
    for(int i = 0; i < num; ++i)
	*this |= rects[i];
}

bool QRegion::operator==(const QRegion &r) const
{
    if(d == r.d)
	return true;
    if(r.d == &shared_empty || d == &shared_empty)
	return false;
    if(d->rect && r.d->rect)
	return *d->rect == *r.d->rect;
    if(!d->rgn)
	const_cast<QRegion *>(this)->rectifyRegion();
    if(!r.d->rgn)
	const_cast<QRegion &>(r).rectifyRegion();
    return EqualRgn(d->rgn, r.d->rgn);
}

