/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qatomic.h"
#include "qbitmap.h"
#include "qbuffer.h"
#include "qimage.h"
#include "qpointarray.h"
#include "qregion.h"
#include "qt_windows.h"


QRegion::QRegionData QRegion::shared_empty = { Q_ATOMIC_INIT(1), 0 };

/*!
    Creates a new empty region.

    \sa isEmpty()
*/
QRegion::QRegion()
    : d(&shared_empty)
{
    ++d->ref;
}

/*!
    Creates a new region. If \a t is \c Rectangle the new region is a
    rectangle occupying the area of the rectangle \a r; if \a t is
    \c Ellipse the new region is the largest ellipse that fills the
    area occupied by the rectangle \a r.
*/
QRegion::QRegion(const QRect &r, RegionType t)
{
    if (r.isEmpty()) {
        d = &shared_empty;
        ++d->ref;
    } else {
        d = new QRegionData;
        d->ref = 1;
        if (t == Rectangle)
            d->rgn = CreateRectRgn(r.left(), r.top(),
                                   r.right() + 1,
                                   r.bottom() + 1);
#ifndef Q_OS_TEMP
        else if (t == Ellipse)
            d->rgn = CreateEllipticRgn(r.left(), r.top(),
                                       r.right() + 1,
                                       r.bottom() + 1);
#endif
    }
}

/*!
    Creates a new region occupying the area of the polygon defined by
    the points in the point array \a a. The shape of the polygon may
    differ depending on the value of \a winding which if true causes
    the shape to be filled using the Winding algorithm, and if false
    casues it to be filled using the Odd-Even algorithm.
*/
QRegion::QRegion(const QPointArray &a, Qt::FillRule rule)
{
    if (a.size() < 3) {
        d = &shared_empty;
        ++d->ref;
    } else {
        d = new QRegionData;
        d->ref = 1;
        d->rgn = CreatePolygonRgn(reinterpret_cast<const POINT*>(a.data()), a.size(),
                                  rule == Qt::OddEvenFill ? ALTERNATE : WINDING);
    }
}

/*!
    Creates a new region that is a copy of the region \a r.

    This is fast because QRegion is implicitly shared.
*/
QRegion::QRegion(const QRegion &r)
{
    d = r.d;
    ++d->ref;
}

HRGN qt_win_bitmapToRegion(const QBitmap& bitmap)
{
#ifndef Q_OS_TEMP
    HRGN region=0;
    QImage image = bitmap.toImage();
    const int MAXRECT = 256;
    struct RData {
        RGNDATAHEADER header;
        RECT rect[MAXRECT];
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
                if (region) { \
                    CombineRgn(region, region, r, RGN_OR); \
                    DeleteObject(r); \
                } else { \
                    region = r; \
                } \
                data.header.rcBound.top = y; \
        }

#define AddSpan \
        { \
            data.rect[n].left=prev1; \
            data.rect[n].top=y; \
            data.rect[n].right=x-1+1; \
            data.rect[n].bottom=y+1; \
            n++; \
            if (n == MAXRECT) { \
                FlushSpans \
                n=0; \
            } \
        }

    data.header.rcBound.top = 0;
    data.header.rcBound.left = 0;
    data.header.rcBound.right = image.width()-1;
    int n = 0;

    int zero = 0x00;

    int x, y;
    for (y = 0; y < image.height(); ++y) {
        uchar *line = image.scanLine(y);
        int w = image.width();
        uchar all=zero;
        int prev1 = -1;
        for (x = 0; x < w;) {
            uchar byte = line[x/8];
            if (x > w - 8 || byte != all) {
                for (int b = 8; b > 0 && x < w; --b) {
                    if (!(byte & 0x80) == !all) {
                        // More of the same
                    } else {
                        // A change.
                        if (all != zero) {
                            AddSpan;
                            all = zero;
                        } else {
                            prev1 = x;
                            all = ~zero;
                        }
                    }
                    byte <<= 1;
                    ++x;
                }
            } else {
                x += 8;
            }
        }
        if (all != zero) {
            AddSpan;
        }
    }
    if (n) {
        FlushSpans;
    }

    if (!region) {
        // Surely there is some better way.
        region = CreateRectRgn(0,0,1,1);
        CombineRgn(region, region, region, RGN_XOR);
    }
    return region;
#else
        return 0;
#endif
}


/*!
    Creates a new region based on the bitmap \a bm.
*/
QRegion::QRegion(const QBitmap &bm)
{
    if (bm.isNull()) {
        d = &shared_empty;
        ++d->ref;
    } else {
        d = new QRegionData;
        d->ref = 1;
        d->rgn = qt_win_bitmapToRegion(bm);
    }
}

void QRegion::cleanUp(QRegion::QRegionData *x)
{
    if (x->rgn)
        DeleteObject(x->rgn);
    delete x;
}

/*!
    Destroys this region.
*/
QRegion::~QRegion()
{
    if (!--d->ref)
        cleanUp(d);
}

/*!
    Assigns region \a r to this region and returns a reference to this
    region.
*/
QRegion &QRegion::operator=(const QRegion &r)
{
    QRegionData *x = r.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        cleanUp(x);
    return *this;
}


QRegion QRegion::copy() const
{
    QRegion r;
    QRegionData *x = new QRegionData;
    x->ref = 1;
    if (d->rgn) {
        x->rgn = CreateRectRgn(0, 0, 2, 2);
        CombineRgn(x->rgn, d->rgn, 0, RGN_COPY);
    } else {
        x->rgn = 0;
    }
    x = qAtomicSetPtr(&r.d, x);
    if (!--x->ref)
        cleanUp(x);
    return r;
}

/*!
    Returns true if this region is empty; otherwise returns false.
*/
bool QRegion::isEmpty() const
{
    return d == &shared_empty || d->rgn == 0;
}


/*!
    Returns true if this region contains the point \a p;
    otherwise returns false.
*/
bool QRegion::contains(const QPoint &p) const
{
    return d->rgn ? PtInRegion(d->rgn, p.x(), p.y()) : false;
}

/*!
    Returns true if this region wholly contains the region \a r;
    otherwise returns false.
*/
bool QRegion::contains(const QRect &r) const
{
    if (!d->rgn)
        return false;
    RECT rect;
    SetRect(&rect, r.left(), r.top(), r.right(), r.bottom());
    return RectInRegion(d->rgn, &rect);
}


/*!
    Translates the region by \a dx on the x axis and by \a dy on the
    y axis.
*/
void QRegion::translate(int dx, int dy)
{
    if (!d->rgn)
        return;
    detach();
    OffsetRgn(d->rgn, dx, dy);
}


#define RGN_NOP -1

/*
  Performs the actual OR, AND, SUB and XOR operation between regions.
  Sets the resulting region handle to 0 to indicate an empty region.
*/

QRegion QRegion::winCombine(const QRegion &r, int op) const
{
    int both=RGN_NOP,
        left=RGN_NOP,
        right=RGN_NOP;
    switch (op) {
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
            qWarning("QRegion: Internal error in winCombine");
    }

    int allCombineRgnResults = NULLREGION;
    QRegion result;
    result.detach();
    result.d->rgn = CreateRectRgn(0, 0, 0, 0);
    if (d->rgn && r.d->rgn)
        allCombineRgnResults = CombineRgn(result.d->rgn, d->rgn, r.d->rgn, both);
    else if (d->rgn && left != RGN_NOP)
        allCombineRgnResults = CombineRgn(result.d->rgn, d->rgn, d->rgn, left);
    else if (r.d->rgn && right != RGN_NOP)
        allCombineRgnResults = CombineRgn(result.d->rgn, r.d->rgn, r.d->rgn, right);

    if (allCombineRgnResults == NULLREGION || allCombineRgnResults == ERROR)
        result = QRegion();

    //##### do not delete this. A null pointer is different from an empty region in SelectClipRgn in qpainter_win! (M)
//     if (allCombineRgnResults == NULLREGION) {
//         if (result.data->rgn)
//             DeleteObject(result.data->rgn);
//         result.data->rgn = 0;                        // empty region
//     }
    return result;
}

/*!
    Returns a region that is the result of performing a union of this
    region with the region \a r.
*/
QRegion QRegion::unite(const QRegion &r) const
{
    return winCombine(r, QRGN_OR);
}

/*!
    Returns a region that is the result of intersecting this
    region with the region \a r.
*/
QRegion QRegion::intersect(const QRegion &r) const
{
     return winCombine(r, QRGN_AND);
}

/*!
    Returns a region that is the result of subtracting
    the region \a r from this region.
*/
QRegion QRegion::subtract(const QRegion &r) const
{
    return winCombine(r, QRGN_SUB);
}

/*!
    Returns a region that is the result of exclusive-OR-ing this
    region with the region \a r.
*/
QRegion QRegion::eor(const QRegion &r) const
{
    return winCombine(r, QRGN_XOR);
}


/*!
    Returns the smallest bounding rectangle that contains this region.
*/
QRect QRegion::boundingRect() const
{
    RECT r;
    int result = d->rgn ? GetRgnBox(d->rgn, &r) : 0;
    if (result == 0 || result == NULLREGION)
        return QRect(0, 0, 0, 0);
    else
        return QRect(r.left, r.top, r.right - r.left, r.bottom - r.top);
}


/*!
    Returns a (possibly empty) list of the rectangles that constitute
    this region.
*/
QVector<QRect> QRegion::rects() const
{
    QVector<QRect> a;
    if (d->rgn == 0)
        return a;

    int numBytes = d->rgn ? GetRegionData(d->rgn, 0, 0) : 0;
    if (numBytes == 0)
        return a;

    char *buf = new char[numBytes];
    if (buf == 0)
        return a;

    RGNDATA *rd = reinterpret_cast<RGNDATA*>(buf);
    if (GetRegionData(d->rgn, numBytes, rd) == 0) {
        delete [] buf;
        return a;
    }

    a = QVector<QRect>(rd->rdh.nCount);
    RECT *r = reinterpret_cast<RECT*>(rd->Buffer);
    for (int i = 0; i < a.size(); ++i) {
        a[i].setCoords(r->left, r->top, r->right - 1, r->bottom - 1);
        ++r;
    }

    delete [] buf;
    return a;
}

/*!
    \internal

    Sets this region to be the area covered by the first \a num
    rectangles in the \a rects array.
*/
void QRegion::setRects(const QRect *rects, int num)
{
    *this = QRegion();
    for (int i = 0; i < num; ++i)
        *this |= rects[i];
}


/*!
    Returns true if this region is the same as the region \a r;
    otherwise returns false.
*/
bool QRegion::operator==(const QRegion &r) const
{
    if (d == r.d)
        return true;
    if ((d->rgn == 0) ^ (r.d->rgn == 0)) // one is empty, not both
        return false;
    return d->rgn == 0 ? true // both empty
                       : EqualRgn(d->rgn, r.d->rgn); // both non-empty
}
