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
#include "qwsregionmanager_qws.h"
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
    cpen(Qt::black), cbrush(Qt::black), penColor(Qt::black), brushColor(Qt::black),
    backcolor(Qt::black), buffer(b)
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
    xoffs=0;
    yoffs=0;

    srctype=SourcePen;
    dashedLines = false;
    dashes = 0;
    numDashes = 0;

    patternedbrush=false;
    srccol = 0;
    cbrushpixmap=0;

    regionClip=false;
    QRect wr(0,0,w,h);
    wr = qt_screen->mapToDevice(wr, QSize(w, h));
    widgetrgn = wr;
    cliprect = new QRect[1];
    cliprect[0] = wr;
    ncliprect = 1;
    clipbounds = wr;
    clipcursor = 0;
    clipDirty = false;

    penx=0;
    peny=0;
    alphatype=IgnoreAlpha;
    alphabuf = 0;
    ismasking=false;
    srclinestep=0;
    srcbits=0;
    lstep=0;
    calpha=255;
    opaque=false;
    globalRegionRevision = 0;
    src_normal_palette=false;
    clutcols = 0;
    gfx_lastop=lastop;
    gfx_optype=optype;
    stitchedges=QPolygonScanner::Edge(QPolygonScanner::Left+QPolygonScanner::Top);

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
    delete [] dashes;
    delete [] cliprect;
}

void* QGfxRasterBase::beginTransaction(const QRect &r)
{
    GFX_START(r);
#ifndef QT_NO_QWS_CURSOR
    return (void*)swc_do_save;
#else
    return (void*)0;
#endif
}

void QGfxRasterBase::endTransaction(void *data)
{
    bool swc_do_save = !!data;
    GFX_END;
}

/*!
    \internal

    This does very little in a purely-software QGfxRasterBase (simply
    records that the last operation was a software one). Hardware drivers
    should reimplement this to wait for graphics engine idle in order to
    allow software and hardware drawing to synchronize properly.
*/
void QGfxRasterBase::sync()
{
    (*gfx_optype)=0;
}

/*!
    \internal

    This corresponds to QPainter::setPen - it tells QGfxRaster
    what line color and style to use, i.e. to use pen \a p.
*/
void QGfxRasterBase::setPen(const QPen &p)
{
    static char dash_line[]         = { 7, 3 };
    static char dot_line[]          = { 1, 3 };
    static char dash_dot_line[]     = { 7, 3, 2, 3 };
    static char dash_dot_dot_line[] = { 7, 3, 2, 3, 2, 3 };

    cpen=p;
    penColor = p.color().rgba();
    switch (cpen.style()) {
        case Qt::DashLine:
            setDashes(dash_line, sizeof(dash_line));
            setDashedLines(true);
            break;
        case Qt::DotLine:
            setDashes(dot_line, sizeof(dot_line));
            setDashedLines(true);
            break;
        case Qt::DashDotLine:
            setDashes(dash_dot_line, sizeof(dash_dot_line));
            setDashedLines(true);
            break;
        case Qt::DashDotDotLine:
            setDashes(dash_dot_dot_line, sizeof(dash_dot_dot_line));
            setDashedLines(true);
            break;
        default:
            setDashedLines(false);
            break;
    }

#ifndef QT_NO_QWS_REPEATER
    if (isScreenGfx) {
        int r = penColor.red(), g = penColor.green(), b = penColor.blue();
        if(depth==32||depth==24)
            penPixel=(r << 16) | (g << 8) | b;
        else if(depth==16)
            penPixel=((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
        else if(depth==1)
            penPixel=qGray(r, g, b) < 128 ? 1 : 0;
        else
            penPixel=gfx_screen->alloc(r, g, b);
    } else
#endif
        penPixel = QColormap::instance().pixel(penColor) & 0x00ffffff;
}

/*!
    \internal

    This corresponds to QPainter::setBrush, and sets the gfx's brush to
    \a b.
*/
void QGfxRasterBase::setBrush(const QBrush &b)
{
    cbrush = b;
    brushColor = b.color().rgba();
    if((cbrush.style() != Qt::NoBrush) && (cbrush.style() != Qt::SolidPattern))
        patternedbrush = true;
    else
        patternedbrush = false;

#ifndef QT_NO_QWS_REPEATER
    if (isScreenGfx) {
        int r = brushColor.red(), g = brushColor.green(), b = brushColor.blue();
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
    srccol = brushPixel; // #### Apparently, and only for now. Will be removed
}

/*!
    \internal

    This sets the offset of a pattern when drawing with a patterned
    brush to (\a x, \a y). This is needed when clipping means the start
    position for drawing doesn't correspond with the start position
    requested by QPainter, for example.
*/
void QGfxRasterBase::setBrushOrigin(int x, int y)
{
    brushorig = QPoint(x, y);
}

/*!
    \internal

    This sets the clipping region for the QGfx to region \a r. All
    drawing outside of the region is not displayed. The clip region is
    defined relative to the QGfx's origin at the time the clip region
    is set, and consists of an array of rectangles stored in the array
    cliprect. Note that changing the origin after the clip region is
    set will not change the position of the clip region within the
    buffer. Hardware drivers should use this to set their clipping
    scissors when drawing. Note also that this is the user clip region
    as set by QPainter; it is combined (via an intersection) with the
    widget clip region to provide the actual clipping region.
*/
void QGfxRasterBase::setClipRegion(const QRegion &r, Qt::ClipOperation op)
{
    regionClip= op != Qt::NoClip;
    QRegion mr = r;
    mr.translate(xoffs,yoffs);
    mr = qt_screen->mapToDevice(mr, QSize(width, height));

    switch (op) {
    case Qt::ReplaceClip:
        cliprgn=mr;
        break;
    case Qt::IntersectClip:
        cliprgn &= mr;
        break;
    case Qt::UniteClip:
        cliprgn |= mr;
    case Qt::NoClip:
        break;
    default:
        cliprgn = mr;
        break;
    }
    update_clip();

#ifdef QWS_EXTRA_DEBUG
    qDebug("QGfxRasterBase::setClipRegion");
    for (int i=0; i< ncliprect; i++) {
        QRect r = cliprect[i];
        qDebug("   cliprect[%d] %d,%d %dx%d", i, r.x(), r.y(),
               r.width(), r.height());
    }
#endif
}

void QGfxRasterBase::setClipDeviceRegion(const QRegion &r)
{
    regionClip=true;
    cliprgn=r;
    update_clip();
}

/*!
    \internal

    If \a b is true then clipping is switched enabled; otherwise clipping
    is disabled.

    If clipping is not enabled then drawing will access the whole buffer.
    This will be reflected in the cliprect array, which will consist of
    one rectangle of buffer width and height. The variable regionClip
    defines whether to clip or not.
*/
void QGfxRasterBase::setClipping(bool b)
{
    if(regionClip!=b) {
        regionClip=b;
        update_clip();
    } else if (clipDirty) {
        update_clip();
    }
}

/*!
    \internal

    This defines the (\a x, \a y) origin of the gfx. For instance, if
    the origin is set to (100, 100) and a line is then drawn from (0,
    0) to (10, 10) the line will be (relative to the top left of the
    buffer) from 100,100 to (110, 110). This is used to support windows
    within the buffer.
*/
void QGfxRasterBase::setOffset(int x, int y)
{
    xoffs=x;
    yoffs=y;
}

/*!
    \internal

    This is a special case of setWidgetRegion for widgets which are not
    shaped and not occluded by any other widgets. The rectangle is
    defined by position \a(x, y), width \a w and height \a h.
*/
void QGfxRasterBase::setWidgetRect(int x, int y, int w, int h)
{
    setWidgetRegion(QRegion(x,y,w,h));
}

/*!
    \internal

    This sets the widget's region clip to \a r, which is combined with
    the user clip to determine the widget's drawable region onscreen.
    It's a combination of the widget's shape (if it's a shaped widget)
    and the area not obscured by windows on top of it.
*/
void QGfxRasterBase::setWidgetRegion(const QRegion &r)
{
    widgetrgn = qt_screen->mapToDevice(r, QSize(width, height));
    clipDirty = true;
    hsync(r.boundingRect().bottom());
}

void QGfxRasterBase::setWidgetDeviceRegion(const QRegion &r)
{
    widgetrgn = r;
    clipDirty = true;
    hsync(r.boundingRect().bottom());
}

void QGfxRasterBase::setGlobalRegionIndex(int idx)
{
    globalRegionIndex = idx;
    globalRegionRevision = qt_fbdpy->regionManager()->revision(idx);
    currentRegionRevision = *globalRegionRevision;
}

/*!
    \internal

    If \a d is true then the gfx should draw dashed lines; otherwise it
    should draw solid lines. It's called by setPen so there is no need
    to call it directly.
*/

void QGfxRasterBase::setDashedLines(bool d)
{
    dashedLines = d;
}

/*!
    \internal

    This defines the pattern for dashed lines. It's called by setPen
    so there is no need to call it directly. \a dashList is a 1 bit
    per pixel specification of the dashes in the line, \a n is the
    number of bytes in the dashList.
*/
void QGfxRasterBase::setDashes(char *dashList, int n)
{
    if (dashes) delete [] dashes;
    dashes = new char [n];
    memcpy(dashes, dashList, n);
    numDashes = n;
}

void QGfxRasterBase::fixClip()
{
    currentRegionRevision = *globalRegionRevision;
    QRegion rgn = qt_fbdpy->regionManager()->region(globalRegionIndex);
    widgetrgn &= rgn;
    update_clip();
}

/*!
    \internal

    This combines the currently set widget and user clips
    and caches the result in an array of QRects, cliprect,
    the size of which is stored in ncliprect. It's called whenever
    the widget or user clips are changed.
*/
void QGfxRasterBase::update_clip()
{
    _XRegion* wr = (_XRegion*) widgetrgn.handle();
    _XRegion* cr = (_XRegion*) cliprgn.handle();

    if (wr->numRects==0) {
        // Widget not visible
        ncliprect = 0;
        delete [] cliprect;
        cliprect = 0;
        clipbounds = QRect();
    } else if (wr->numRects==1 && (!regionClip || cr->numRects==1)) {
        // fastpath: just simple rectangles (90% of cases)
        QRect setrgn;

        if(regionClip) {
            setrgn=wr->rects[0].intersect(cr->rects[0]);
        } else {
            setrgn=wr->rects[0];
        }

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
        if(regionClip) {
            setrgn=widgetrgn.intersect(cliprgn);
        } else {
            setrgn=widgetrgn;
        }

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
    clipDirty = false;
}

/*!
    \internal

    This is the counterpart to QPainter::moveTo. It simply stores the
    \a x and \a y values passed to it until a lineTo.
*/
void QGfxRasterBase::moveTo(int x, int y)
{
    penx=x;
    peny=y;
}

/*!
    \internal

    This draws a line from the last values passed to moveTo to (\a x,
    \a y). It calls drawLine so there is no need to reimplement it in
    an accelerated driver.
*/
void QGfxRasterBase::lineTo(int x, int y)
{
    drawLine(penx,peny,x,y);
    penx=x;
    peny=y;
}

/*!
    \internal

    Returns the current position of the pen.
*/
QPoint QGfxRasterBase::pos() const
{
    return QPoint(penx, peny);
}

/*!
    \internal

    This stores the offset from the screen framebuffer of the widget
    from which a blt() is being performed - this is added to the source
    \a x and \a y coordinates from a bitBlt to produce the source
    screen position of the blt
*/
void QGfxRasterBase::setSourceWidgetOffset(int x, int y)
{
    srcwidgetoffs = QPoint(x,y);
}

/*!
    \internal

    This sets one of several alpha channel types for the next
    blt operation to \a a:

    \list
    \i IgnoreAlpha - Always draw source pixels as-is (no alpha blending)
    \i InlineAlpha - An 8-bit alpha value is in the highest byte of the (32-bit) source data
    \i SeparateAlpha - A separate 8-bit alpha channel buffer is provided (used for anti-aliased text)
    \i LittleEndianMask - A separate little-bit-endian mask is provided
    \i BigEndianMask - A separate big-bit-endian mask is provided
    \i SolidAlpha - A single 8-bit alpha value is to be applied to all pixels
    \endlist

    The alpha channel buffer/value is provided by setAlphaSource
*/
void QGfxRasterBase::setAlphaType(AlphaType a)
{
    alphatype=a;
    if(a==LittleEndianMask || a==BigEndianMask) {
        ismasking=true;
    } else {
        ismasking=false;
    }
}

/*!
    \internal

    This is used in conjunction with LittleEndianMask, BigEndianMask or
    SeparateAlpha alpha channels. \a b is a pointer to the bytes
    containing the alpha values, \a l is the linestep (length in bytes
    per horizontal line of data)
*/
void QGfxRasterBase::setAlphaSource(unsigned char *b, int l)
{
    alphabits=b;
    alphalinestep=l;
}

/*!
    \internal
    \overload

    This is used by the SolidAlpha alpha channel type and sets a single
    alpha value to be used in blending all of the source data. The
    value is between 0 (draw nothing) and 255 (draw solid source data)
    - a value of 128 would draw a 50% blend between the source and
    destination data. Normally called with just one argument, where \a
    i is the alpha value to use. If \a i2, \a i3 and \a i4 are used
    they specify different alpha values for the corners of an
    alpha-blended rectangle; this is only used by some experimental
    hardware-accelerated alpha-blending code.
*/
void QGfxRasterBase::setAlphaSource(int i, int i2, int i3, int i4)
{
    calpha=i;
    if(i2==-1)
        i2=i;
    if(i3==-1)
        i3=i;
    if(i4==-1)
        i4=i;
    calpha2=i2;
    calpha3=i3;
    calpha4=i4;
    setAlphaType(SolidAlpha);
}

/*!
    \internal

    This saves the current brush and pen state to temporary variables.
    This is used internally in QGfxRaster when a temporary pen or brush
    is needed for something. This is not a stack; a save() followed by
    a save() will obliterate the previously saved brush and pen.
*/
void QGfxRasterBase::save()
{
    savepen=cpen;
    savebrush=cbrush;
}

/*!
    \internal

    Restores the brush and pen from a previous save().
*/
void QGfxRasterBase::restore()
{
    setPen(savepen);
    setBrush(savebrush);
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
    \internal

    This tells blt()s that instead of image data a single solid value
    should be used as the source, taken from the current pen color. You
    could reproduce a fillRect() using a pen source and the IgnoreAlpha
    alpha type, but this would be both pointless and slower than
    fillRect; its normal use is for anti-aliased text, where the text
    color is that of the pen and a separate alpha channel produces the
    shape of the glyphs.
*/
void QGfxRasterBase::setSourcePen()
{
#ifndef QT_NO_QWS_REPEATER
    if (is_screen_gfx && qt_screen != gfx_screen) {
        QScreen * tmp=qt_screen;
        qt_screen=gfx_screen;
        QColor tmpcol=cpen.color();
        srccol = tmpcol.alloc();
        qt_screen=tmp;
    } else
#endif
    srccol = cpen.color().rgb();
    src_normal_palette = true;
    srctype = SourcePen;
    setSourceWidgetOffset(0, 0);
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
