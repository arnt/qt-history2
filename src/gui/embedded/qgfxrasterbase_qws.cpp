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

#include "qgfxrasterbase_qws.h"
//#include "qwsregionmanager_qws.h"
#include "qcolormap.h"

// Used for synchronisation with accelerated drivers
static int dummy_optype = 0;
static int dummy_lastop = 0;
volatile int *optype = &dummy_optype;
volatile int *lastop = &dummy_lastop;

struct _XRegion {
    int numRects;
    QVector<QRect> rects;
    // ... etc.
};

/*!
    \class QGfxRasterBase qgfxraster_qws.h
    \brief The QGfxRasterBase class is the base class of the
    QGfxRaster<depth> template and contains the non-depth-dependent code.

    \internal (for now)

    \ingroup qws

    The QGfxRaster class is used for drawing in software on raw
    framebuffers of varying depths and  is subclassed by hardware
    drivers. It handles clipping and a movable origin in order to
    support subwindows. It is available \e only in Qt/Embedded. QWidget
    and QPixmap both return a QGfxRaster via their respective
    graphicsContext() methods, already initialized with the appropriate
    origin, framebuffer and clip region. QGfxRasterBase and its
    template subclasses should effectively be considered as one class;
    a raw QGfxRasterBase is never used, it's simply a handy place to
    put some of the functionality.
*/

/*!
    \internal

    This constructed a QGfxRasterBase. \a b is the data buffer pointed to,
    \a w and \a h its width and height in pixels
*/
QGfxRasterBase::QGfxRasterBase(unsigned char *b, int w, int h) :
    cbrush(Qt::black), brushColor(0x00000000),
    buffer(b)
{
    // Buffers should always be aligned
    if(((unsigned long)b) & 0x3) {
        qDebug("QGfx buffer unaligned: %lx",(unsigned long)b);
    }

#ifdef QT_PAINTER_LOCKING
    QWSDisplay::grab();
#endif

    gfx_screen=qt_screen;
#ifndef QT_NO_QWS_CURSOR
    gfx_screencursor=qt_screencursor;
    gfx_swcursor=qt_sw_cursor;
#endif
    srcpixeltype = pixeltype = QScreen::NormalPixel;
    is_screen_gfx = buffer==qt_screen->base();
    width=w;
    height=h;

    QRect cr(0,0,w,h);
    cr = qt_screen->mapToDevice(cr, QSize(w, h));
    cliprgn = cr;
    cliprect = new QRect[1];
    cliprect[0] = cr;
    ncliprect = 1;
    clipbounds = cr;
    clipcursor = 0;

    srclinestep=0;
    srcbits=0;
    lstep=0;
    src_normal_palette=false;
    clutcols = 0;

    src_little_endian=true;
#if !defined(QT_NO_QWS_DEPTH_8)
    // default color map
    setClut(gfx_screen->clut(), gfx_screen->numCols());
#endif
}

/*!
    \internal

    Destroys a QGfxRaster
*/
QGfxRasterBase::~QGfxRasterBase()
{
#ifdef QT_PAINTER_LOCKING
    QWSDisplay::ungrab();
#endif
}

QPixmap qt_pixmapForBrush(int brushStyle, bool invert); //in qbrush.cpp
/*!
    \internal

    This corresponds to QPainter::setBrush, and sets the gfx's brush to
    \a b.
*/
void QGfxRasterBase::setBrush(const QBrush &b)
{
    cbrush = b;
    brushColor = b.color().rgba();

#ifndef QT_NO_QWS_REPEATER
    if (isScreenGfx()) {
        int r = qRed(brushColor), g = qGreen(brushColor), b = qBlue(brushColor);
        int depth = bitDepth();
        if(depth==32||depth==24)
            brushPixel=(r << 16) | (g << 8) | b;
        else if(depth==16)
            brushPixel=((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        else if(depth==1)
            brushPixel=qGray(r, g, b) < 128 ? 1 : 0;
        else
            brushPixel=gfx_screen->alloc(r, g, b);
    } else
#endif
        brushPixel = QColormap::instance().pixel(brushColor) & 0x00ffffff;
}

void QGfxRasterBase::setClipDeviceRegion(const QRegion &r)
{
    cliprgn=r;
    _XRegion* cr = (_XRegion*) cliprgn.handle();

     if (cr->numRects==1) {
        // fastpath: just simple rectangles (90% of cases)
        QRect setrgn;
        setrgn=cr->rects[0];

        if (setrgn.isEmpty()) {
            ncliprect = 0;
            clipbounds = QRect();
        } else {
            // cache bounding rect
            QRect sr(QPoint(0,0), qt_screen->mapToDevice(QSize(width, height)));
            clipbounds = sr.intersect(setrgn);

            // Convert to simple array for speed
            if (ncliprect < 1) {
                delete [] cliprect;
                cliprect = new QRect[1];
            }
            cliprect[0] = setrgn;
            ncliprect = 1;
        }
    } else {
#ifdef QWS_EXTRA_DEBUG
        qDebug("QGfxRasterBase::update_clip");
#endif
        QRegion setrgn;
        setrgn=cliprgn;
        // cache bounding rect
        QRect sr(QPoint(0,0), gfx_screen->mapToDevice(QSize(width, height)));
        clipbounds = sr.intersect(setrgn.boundingRect());

        // Convert to simple array for speed
        _XRegion* cr = (_XRegion*) setrgn.handle();
        const QVector<QRect> &a = cr->rects;
        delete [] cliprect;
        cliprect = new QRect[cr->numRects];
        memcpy(cliprect, a.data(), cr->numRects*sizeof(QRect));
        ncliprect = cr->numRects;
    }
    clipcursor = 0;
}



/*!
    \internal

    Returns whether the point (\a x, \a y) is in the clip region.

    If \a cr is not null, \c *\cr is set to a rectangle containing
    the point, and within all of which the result does not change.
    If the result is true, \a cr is the widest rectangle for which
    the result remains true (so any point immediately to the left or
    right of \a cr will not be part of the clip region).

    Passing true for the \a known_to_be_outside allows optimizations,
    but the results are not defined it (\a x, \a y) is in the clip region.

    Using this, you can efficiently iterator over the clip region
    using:

    \code
        bool inside = inClip(x,y,&cr);
        while (change y, preferably by +1) {
            while (change x by +1 or -1) {
                if (!cr.contains(x,y))
                    inside = inClip(x,y,&cr,inside);
                if (inside) {
                    draw stuff
                }
            }
        }
    \endcode
*/
bool QGfxRasterBase::inClip(int x, int y, QRect *cr, bool known_to_be_outside)
{
    if (!ncliprect) {
        // No rectangles.
        if (cr)
            *cr = QRect(x-4000,y-4000,8000,8000);
        return false;
    }

//qDebug("Find %d,%d...%s",x,y,known_to_be_outside?" (outside)":"");
    bool search=false;
    const QRect *cursorRect = &cliprect[clipcursor];

//search=true;
    if (!known_to_be_outside) {
        if (cursorRect->contains(x,y)) {
            if (cr)
                *cr = *cursorRect;

//qDebug("found %d,%d at +0 in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
            return true;
        }
        if (clipcursor > 0) {
            if ((cursorRect-1)->contains(x,y)) {
                if (cr)
                    *cr = cliprect[--clipcursor];

//qDebug("found %d,%d at -1 in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
                return true;
            }
        } else if (clipcursor < (int)ncliprect-1) {
            if ((cursorRect+1)->contains(x,y)) {
                if (cr)
                    *cr = cliprect[++clipcursor];

//qDebug("found %d,%d at +1 in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
                return true;
            }
        }
        search=true;
    }

    // Optimize case where (x,y) is in the same band as the clipcursor,
    // and to its right.  eg. left-to-right, same-scanline cases.
    //
    if (cursorRect->right() < x
        && cursorRect->top() <= y
        && cursorRect->bottom() >= y)
    {
        // Move clipcursor right until it is after (x,y)
        for (;;) {
            if (clipcursor+1 < ncliprect &&
                 (cursorRect+1)->top()==cursorRect->top()) {
                // next clip rect is in this band too - move ahead
                clipcursor++;
                cursorRect++;
                if (cursorRect->left() > x) {
                    // (x,y) is between clipcursor-1 and clipcursor
                    if (cr)
                        cr->setCoords((cursorRect-1)->right()+1,
                                cursorRect->top(),
                                cursorRect->left()-1,
                                cursorRect->bottom());
                    return false;
                } else if (cursorRect->right() >= x) {
                    // (x,y) is in clipcursor
                    if (cr)
                        *cr = *cursorRect;

//qDebug("found %d,%d in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
                    return true;
                }
            } else {
                // (x,y) is after last rectangle on band
                if (cr)
                    cr->setCoords(cursorRect->right()+1,
                            cursorRect->top(),y+4000,
                            cursorRect->bottom());
                return false;
            }
        }
    } else {
        search=true;
    }

    // The "4000" below are infinitely large rectangles, made small enough
    // to let surrounding alrogithms work of small integers. It means that
    // in rare cases some extra calls may be made to this function, but that
    // will make no measurable difference in performance.

    /*
        (x,y) will be in one of these characteristic places:

        0. In a rectangle of the region
        1. Before the region
        2. To the left of the first rectangle in the first band
        3. To the left of the first rectangle in a non-first band
        4. Between two retcangles in a band
        5. To the right of the last rectangle in a non-last band
        6. Between the last two rectangles
        7. To the right of the last rectangle in the last band
        8. After the region
        9. Between the first two rectangles

                            1
                     2   BBBBBBB
                  3 BB0BBBB 4 BBBBBBBBB 5
                         BBBBBBB   6
                            7
    */


    if (search) {
//qDebug("Search for %d,%d",x,y);
        // binary search for rectangle which is before (x,y)
        int a=0;
        int l=ncliprect-1;
        int h;
        int m=-1;
        while (l>0) {
            h = l/2;
            m = a + h;
//            qDebug("l = %d, m = %d", l, m);
            const QRect& r = cliprect[m];
            if (r.bottom() < y || r.top() <= y && r.right() < x) {
                // m is before (x,y)
                a = m + 1;
                l = l - h - 1;
            } else
                l = h;
        }
        // Rectangle "a" is the rectangle containing (x,y), or the
        // closest rectangle to the right of (x,y).
        clipcursor = a;
        cursorRect = &cliprect[clipcursor];
        if (cursorRect->contains(x,y)) {
            // PLACE 0
//qDebug("found %d,%d in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
            if (cr)
                *cr = *cursorRect;
//qDebug("Found %d,%d in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
            return true;
        }
//qDebug("!found %d,%d in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
    }

    // At this point, (x,y) is outside the clip region and clipcursor is
    // the rectangle to the right/below of (x,y), or the last rectangle.

    if (cr) {
        const QRect &tcr = *cursorRect;
        if (y < tcr.top() && clipcursor == 0) {
            // PLACE 1
//qDebug("PLACE 1");
            cr->setCoords(x-4000,y-4000,x+4000,tcr.top()-1);
        } else if (clipcursor == (int)ncliprect-1 && y>tcr.bottom()) {
            // PLACE 7
//qDebug("PLACE 7");
            cr->setCoords(x-4000,tcr.bottom()+1,x+4000,y+4000);
        } else if (clipcursor == (int)ncliprect-1 && x > tcr.right()) {
            // PLACE 6
//qDebug("PLACE 6");
            cr->setCoords(tcr.right()+1,tcr.top(),x+4000,y+4000);
        } else if (clipcursor == 0) {
            // PLACE 2
//qDebug("PLACE 2");
            cr->setCoords(x-4000,y-4000,tcr.left()-1,tcr.bottom());
        } else {
            const QRect &prev_tcr = *(cursorRect-1);
            if (prev_tcr.bottom() < y && tcr.top() > y) {
                // found a new place
//qDebug("PLACE new");
                cr->setCoords(x-4000,prev_tcr.bottom()+1, x+4000,tcr.top()-1);
            } else if (prev_tcr.bottom() < y && tcr.left() > x) {
                // PLACE 3
//qDebug("PLACE 3");
                cr->setCoords(x-4000,tcr.top(), tcr.left()-1,tcr.bottom());
            } else {
                if (prev_tcr.y() == tcr.y()) {
                    // PLACE 4
//qDebug("PLACE 4");
                    cr->setCoords(prev_tcr.right()+1, tcr.y(),
                                       tcr.left()-1, tcr.bottom());
                } else {
                    // PLACE 5
//qDebug("PLACE 5");
                    cr->setCoords(prev_tcr.right()+1, prev_tcr.y(),
                                       prev_tcr.right()+4000, prev_tcr.bottom());
                }
            }
        }
    }

//qDebug("!found %d,%d in %d[%d..%d,%d..%d] nor [%d..%d,%d..%d]",x,y, clipcursor, cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom(), cr->left(),cr->right(),cr->top(),cr->bottom());
    return false;
}

/*!
    \fn QGfxRasterBase::get_value_32(int sdepth, unsigned const char **srcdata, bool reverse)

    \internal

    This converts a pixel in an arbitrary source depth (specified by \a
    sdepth, stored at *(*\a srcdata) to a 32 bit value; it's used by
    blt() where the source depth is less than 32 bits and the
    destination depth is 32 bits. *srcdata (the pointer to the data) is
    auto-incremented by the appropriate number of bytes, or decremented
    if \a reverse is true. If the source has a pixel size of less than
    a byte then auto-incrementing or decrementing will happen as
    necessary; the current position within the byte is stored in
    monobitcount (bit within the byte) and monobitval (value of the
    current byte). In the case of 8-bit source data lookups on the
    source's color table are performed.
*/

/*!
    \fn QGfxRasterBase::get_value_24(int sdepth, unsigned const char **srcdata, bool reverse)

    \internal

    This converts a pixel in an arbitrary source depth (specified by \a
    sdepth, stored at *(*\a srcdata) to a 24 bit value; it's used by
    blt() where the source depth is less than 24 bits and the
    destination depth is 24 bits. *srcdata (the pointer to the data) is
    auto-incremented by the appropriate number of bytes, or decremented
    if \a reverse is true. If the source has a pixel size of less than
    a byte then auto-incrementing or decrementing will happen as
    necessary; the current position within the byte is stored in
    monobitcount (bit within the byte) and monobitval (value of the
    current byte). In the case of 8-bit source data lookups on the
    source's color table are performed.

    \sa get_value_32()
*/

/*!
    \fn QGfxRasterBase::get_value_16(int sdepth, unsigned const char **srcdata, bool reverse)

    \internal

    This converts a pixel in an arbitrary source depth (specified by \a
    sdepth, stored at *(*\a srcdata) to a 16 bit value; it's used by
    blt() where the source depth is less than 16 bits and the
    destination depth is 16 bits. *srcdata (the pointer to the data) is
    auto-incremented by the appropriate number of bytes, or decremented
    if \a reverse is true. If the source has a pixel size of less than
    a byte then auto-incrementing or decrementing will happen as
    necessary; the current position within the byte is stored in
    monobitcount (bit within the byte) and monobitval (value of the
    current byte). In the case of 8-bit source data lookups on the
    source's color table are performed.

    \sa get_value_32()
*/


/*!
    \fn QGfxRasterBase::get_value_8(int sdepth, unsigned const char **srcdata, bool reverse)

    \internal

    This is similar to get_value_32(), but returns 8-bit values. Translation
    between different color palettes and from 32/24/16 bit data to the nearest
    match in the destination's color palette is performed.

    This converts a pixel in an arbitrary source depth (specified by \a sdepth,
    stored at *(*\a srcdata) to a 8 bit value; it's used by blt() where the
    source depth is less than 8 bits and the destination depth is 8 bits.
    *srcdata (the pointer to the data) is auto-incremented by the appropriate
    number of bytes, or decremented if \a reverse is true. If the source has
    a pixel size of less than a byte then auto-incrementing or decrementing
    will happen as necessary; the current position within the byte is stored in
    monobitcount (bit within the byte) and monobitval (value of the current
    byte). In the case of 8-bit source data lookups on the source's color
    table are performed.
*/

/*!
    \fn QGfxRasterBase::get_value_4(int sdepth, unsigned const char **srcdata, bool reverse)

    \internal

    This is similar to get_value_8, but returns 4-bit values.

    This converts a pixel in an arbitrary source depth (specified by \a sdepth,
    stored at *(*\a srcdata) to a 4 bit value; it's used by blt() where the
    source depth is less than 4 bits and the destination depth is 4 bits.
    *srcdata (the pointer to the data) is auto-incremented by the appropriate
    number of bytes, or decremented if \a reverse is true. If the source has
    a pixel size of less than a byte then auto-incrementing or decrementing
    will happen as necessary; the current position within the byte is stored in
    monobitcount (bit within the byte) and monobitval (value of the current
    byte). In the case of 8-bit source data lookups on the source's color
    table are performed.
*/

/*!
    \fn QGfxRasterBase::get_value_1(int sdepth, unsigned const char **srcdata, bool reverse)

    \internal

    This is similar to get_value_8, but returns 1-bit values. The number of depths
    that can be blt'd to a monochrome destination are limited - only monochrome
    or 32-bit sources are permitted.

    This converts a pixel in an arbitrary source depth (specified by \a sdepth,
    stored at *(*\a srcdata) to a 1 bit value; it's used by blt() where the
    source depth is less than 1 bit and the destination depth is 1 bit.
    *srcdata (the pointer to the data) is auto-incremented by the appropriate
    number of bytes, or decremented if \a reverse is true. If the source has
    a pixel size of less than a byte then auto-incrementing or decrementing
    will happen as necessary; the current position within the byte is stored in
    monobitcount (bit within the byte) and monobitval (value of the current
    byte). In the case of 8-bit source data lookups on the source's color
    table are performed.
*/
