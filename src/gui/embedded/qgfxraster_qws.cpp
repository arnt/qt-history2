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

#include "qgfxraster_qws.h"
#include "qpen.h"
#include "qcolormap.h"
#include "qpaintdevice.h"
#include "qmemorymanager_qws.h"
#include "qwsdisplay_qws.h"
#include "qgfxdriverfactory_qws.h"
#include "qpixmap.h"

#include <stdlib.h>
#include <math.h>

// Used for synchronisation with accelerated drivers (defined in qgfxrasterbase_qws.cpp)
extern volatile int *optype;
extern volatile int *lastop;

#if !defined(_OS_FREEBSD_) && !defined(Q_OS_MAC)
# include <endian.h>
# if __BYTE_ORDER == __BIG_ENDIAN
#  define QT_QWS_REVERSE_BYTE_ENDIANNESS
//#  define QWS_BIG_ENDIAN
# endif
#endif

//===========================================================================
// Utility macros and functions [start]


// Uncomment the following for 1bpp/4bpp displays with a different
// endianness than the processor. This is endianness *within* a byte,
// ie. whether 0x01 is the rightmost or the leftmost pixel on the
// screen.
// This code is still experimental. There are no known bugs, but it
// has not been extensively tested.
//#define QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS


// Uncomment the following to ensure the pixels in (1 pixel wide)
// polyline joins are only written once, i.e. XOR polyline joins work
// correctly.
//#define GFX_CORRECT_POLYLINE_JOIN


#ifdef GFX_CORRECT_POLYLINE_JOIN
    static QPoint *gfx_storedLineRd = 0;
    static QPoint *gfx_storedLineWr = 0;
    static bool gfx_storeLine = false;
    static int gfx_storedLineRead = 0;
    static int gfx_storedLineWrite = 0;
    static int gfx_storedLineDir = 1;
    static bool gfx_noLineOverwrite = false;
    static int gfx_storedLineBufferSize = 0;
    static bool gfx_doDraw = true;
#else
    static const bool gfx_storeLine = false;
#endif

static Q_GFX_INLINE unsigned char *find_pointer_4(unsigned char * base,int x,int y, int w,
                                                int linestep, int &astat, unsigned char &ahold,
                                                bool rev
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                                                , bool reverse_endian
#endif
   )
{
    int nbits;
    int nbytes;

    if (rev) {
        nbits = 1 - (x+w) % 2;
        nbytes = (x+w) / 2;
    } else {
        nbits = x % 2;
        nbytes = x / 2;
    }

    unsigned char *ret = base + (y*linestep) + nbytes;
    astat = nbits;
    ahold = *ret;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
    if (reverse_endian)
        ahold = (ahold & 0x0f) << 4 |  (ahold & 0xf0) >> 4;
#endif

    if (rev)
        ahold = ahold << (nbits*4);
    else
        ahold = ahold >> (nbits*4);

    return ret;
}

#ifdef GFX_CORRECT_POLYLINE_JOIN
static inline bool qt_inside_edge(const QPoint &p, const QRect &r, int edge)
{
    switch (edge) {
        case 0:
            return p.x() > r.left();
        case 1:
            return p.y() > r.top();
        case 2:
            return p.x() < r.right();
        case 3:
            return p.y() < r.bottom();
    }

    return false;
}

static inline QPoint qt_intersect_edge(const QPoint &p1, const QPoint &p2, const QRect &r, int edge)
{
    int x=0, y=0;
    int dy = p2.y() - p1.y();
    int dx = p2.x() - p1.x();

    switch (edge) {
        case 0:
            x = r.left();
            y = p1.y() + dy * qAbs(p1.x() - x) / qAbs(dx);
            break;
        case 1:
            y = r.top();
            x = p1.x() + dx * qAbs(p1.y() - y) / qAbs(dy);
            break;
        case 2:
            x = r.right();
            y = p1.y() + dy * qAbs(p1.x() - x) / qAbs(dx);
            break;
        case 3:
            y = r.bottom();
            x = p1.x() + dx * qAbs(p1.y() - y) / qAbs(dy);
            break;
    }

    return QPoint(x,y);
}

static bool qt_clipLine(int &x1, int &y1, int &x2, int &y2, const QRect &clip)
{
    if (clip.contains(x1, y1) && clip.contains(x2, y2))
        return true;

    for (int e = 0; e < 4; e++) {
        if (!qt_inside_edge(QPoint(x1, y1), clip, e) &&
                qt_inside_edge(QPoint(x2, y2), clip, e)) {
            QPoint i = qt_intersect_edge(QPoint(x1, y1), QPoint(x2, y2), clip, e);
            x1 = i.x();
            y1 = i.y();
        } else if (!qt_inside_edge(QPoint(x2, y2), clip, e) &&
                qt_inside_edge(QPoint(x1, y1), clip, e)) {
            QPoint i = qt_intersect_edge(QPoint(x1, y1), QPoint(x2, y2), clip, e);
            x2 = i.x();
            y2 = i.y();
        } else {
            return false;
        }
    }

    return true;
}
#endif // GFX_CORRECT_POLYLINE_JOIN


// Based on lines_intersect from Graphics Gems II, author: Mukesh Prasad
static QPoint intersection(const QPolygon& pa, const QPoint& p0, int p, int q)
{
    int x1 = p0.x();
    int x2 = pa[p+1].x();
    int y1 = p0.y();
    int y2 = pa[p+1].y();
    int x3 = pa[q].x();
    int x4 = pa[q+1].x();
    int y3 = pa[q].y();
    int y4 = pa[q+1].y();

    int a1 = y2 - y1;
    int b1 = x1 - x2;
    int c1 = x2 * y1 - x1 * y2;

    int a2 = y4 - y3;
    int b2 = x3 - x4;
    int c2 = x4 * y3 - x3 * y4;

    int denom = a1 * b2 - a2 * b1;
    if (denom == 0)
        return (p0+pa[q])/2;

    int offset = denom < 0 ? - denom / 2 : denom / 2;
    int num = b1 * c2 - b2 * c1;
    int x = (num < 0 ? num - offset : num + offset) / denom;

    num = a2 * c1 - a1 * c2;
    int y = (num < 0 ? num - offset : num + offset) / denom;

    return QPoint(x,y);
}

static void fix_mitre(QPolygon& pa, QPoint& pp, int i1, int i2, int i3, int penwidth)
{
    QPoint inter = intersection(pa, pp, i1, i3);
    pp = pa[i3];
    QPoint d2 = inter-pa[i2];
    int l2 = d2.x()*d2.x()+d2.y()*d2.y();
    if (l2 > penwidth*penwidth*8) {
        // Too sharp, leave it square
    } else {
        pa[i2] = inter;
        pa[i3] = inter;
    }
}

// Converts a thick polyline into a polygon which can be painted with
// the winding rule.
static QPolygon convertThickPolylineToPolygon(const QPolygon &points,int index, int npoints,
                                                 int penwidth, Qt::PenJoinStyle join, bool close)
{
    QPolygon pa(npoints*4+(close?2:-4));

    int cw=0; // clockwise cursor in pa
    int acw=pa.count()-1; // counterclockwise cursor in pa

    for (int i=0; i<npoints-(close?0:1); i++) {
        int x1 = points[index + i].x();
        int y1 = points[index + i].y();
        int x2 = points[index + (i==npoints-1 ? 0 : i+1)].x();
        int y2 = points[index + (i==npoints-1 ? 0 : i+1)].y();

        int dx = x2 - x1;
        int dy = y2 - y1;
        int w = qt_int_sqrt(dx*dx+dy*dy);
        int iy = w ? (penwidth * dy)/ w : dy ? 0 : penwidth;
        int ix = w ? (penwidth * dx)/ w : dx ? 0 : penwidth;

        // rounding dependent on sign
        int nix, niy;
        if (ix < 0) {
            nix = ix/2;
            ix = (ix-1)/2;
        } else {
            nix = (ix+1)/2;
            ix = ix/2;
        }
        if (iy < 0) {
            niy = iy/2;
            iy = (iy-1)/2;
        } else {
            niy = (iy+1)/2;
            iy = iy/2;
        }

        pa.setPoint(cw, x1+iy, y1-nix);
        pa.setPoint(acw, x1-niy, y1+ix);
        cw++; acw--;
        pa.setPoint(cw, x2+iy, y2-nix);
        pa.setPoint(acw, x2-niy, y2+ix);
        cw++; acw--;
    }
    if (close) {
        pa[cw] = pa[0];
        pa[acw] = pa[pa.count()-1];
    }
    if (npoints > 2 && join == Qt::MiterJoin) {
        if (close) {
            QPoint pp=pa[0];
            QPoint p1=pa[pa.count()-2];
            int i;
            for (i=0; i<cw-2; i+=2)
                fix_mitre(pa, pp, i, i+1, i+2, penwidth);
            fix_mitre(pa, pp, i, i+1, 0, penwidth);
            pp=p1;
            fix_mitre(pa, pp, pa.count()-2, acw, acw+1, penwidth);
            for (i=acw+1; i<(int)pa.count()-3; i+=2)
                fix_mitre(pa, pp, i, i+1, i+2, penwidth);

            pa[0] = pa[cw];
            pa[pa.count()-1] = pa[acw];
        } else {
            int i;
            QPoint pp=pa[0];
            for (i=0; i<cw-2; i+=2)
                fix_mitre(pa, pp, i, i+1, i+2, penwidth);
            pp=pa[acw+1];
            for (i=acw+1; i<(int)pa.count()-2; i+=2)
                fix_mitre(pa, pp, i, i+1, i+2, penwidth);
        }
    }

    return pa;
}

// Utility macros and functions [end]
//===========================================================================


#ifndef QT_NO_QWS_CURSOR
/*!
    \class QScreenCursor qgfx_qws.h
    \brief The QScreenCursor class manages the onscreen mouse cursor in
    Qt/Embedded.

    \internal (for now)

    \ingroup qws

    It provides an implementation of a software mouse cursor
    and can be subclassed by hardware drivers which support a hardware mouse
    cursor. There may only be one QScreenCursor at a time; it is constructed
    by QScreen or one of its descendants.

    This class is non-portable. It is available only in Qt/Embedded.
    It is also internal - this documentation is intended for those subclassing
    it in hardware drivers, not for application developers.
*/

extern bool qws_sw_cursor;
static QGfx * graphicsContext(QImage *img)
{
    qWarning("graphicsContext(QImage*) should not be called");
    QGfx * ret=0;
#if 1

    if(img->depth()) {
        int w = qt_screen->mapToDevice(QSize(img->width(),img->height())).width();
        int h = qt_screen->mapToDevice(QSize(img->width(),img->height())).height();
        ret=QGfx::createGfx(img->depth(),img->bits(),w,h,img->bytesPerLine());
    } else {
        qDebug("Trying to create image for null depth");
        return 0;
    }
    if(img->depth()<=8) {
        QRgb * tmp=const_cast<QRgb*>(img->colorTable().constData());
        int nc=img->numColors();
        if(tmp==0) {
            static QRgb table[2] = { qRgb(255,255,255), qRgb(0,0,0) };
            tmp=table;
            nc=2;
        }
        ret->setClut(tmp,nc);
    }
#endif
    return ret;

}

/*!
    \internal

    Constructs a screen cursor
*/
QScreenCursor::QScreenCursor() : gfx(0), gfxunder(0), imgunder(0), cursor(0)
{
}

/*!
    \internal

    Initialises a screen cursor - creates a Gfx to draw it with
    and an image to store the part of the screen stored under the cursor.
    Should not be called by hardware cursor descendants. \a da points
    to the location in framebuffer memory where the cursor saves information
    stored under it, \a init is true if the cursor is being initialized
    (i.e. if the program calling this is the Qt/Embedded server), false
    if another application has already initialized it.
*/
void QScreenCursor::init(SWCursorData *da, bool init)
{
    // initialise our gfx
    gfx = (QGfxRasterBase*)qt_screen->screenGfx();
    gfx->setClipRegion(QRect(0, 0, qt_screen->width(), qt_screen->height()), Qt::ReplaceClip);

    data = da;
    save_under = false;
    fb_start = qt_screen->base();
    fb_end = fb_start + gfx->pixelHeight() * gfx->linestep();

    if (init) {
        data->x = gfx->pixelWidth()/2;
        data->y = gfx->pixelHeight()/2;
        data->width = 0;
        data->height = 0;
        data->enable = true;
        data->bound = QRect(data->x - data->hotx, data->y - data->hoty,
                       data->width+1, data->height+1);
    }
    clipWidth = qt_screen->deviceWidth();
    clipHeight = qt_screen->deviceHeight();

    int d = gfx->bitDepth();
    int cols = gfx->bitDepth() == 1 ? 0 : 256;
    if (d == 4) {
        d = 8;
        cols = 16;
    }

    imgunder = new QImage(data->under, 64, 64, d, 0,
                cols, QImage::LittleEndian);
    if (d <= 8) {
        for (int i = 0; i < cols; i++)
            imgunder->setColor(i, qt_screen->clut()[i]);
    }
    gfxunder = (QGfxRasterBase*)graphicsContext(imgunder);
}

/*!
    \internal

    Destroys a screen cursor, deleting its gfxes, cursor image and
    under-cursor storage
*/
QScreenCursor::~QScreenCursor()
{
    delete gfx;
    delete gfxunder;
    delete imgunder;
}

/*!
    \internal

    Returns true if an alpha-blended cursor image is supported.
    This affects the type of QImage passed to the cursor - descendants
    returning true (as QScreenCursor does for bit depths of 8 and above.
    unless QT_NO_QWS_ALPHA_CURSOR is defined in qconfig.h) should be prepared
    to accept QImages with full 8-bit alpha channels
*/
bool QScreenCursor::supportsAlphaCursor()
{
#ifndef QT_NO_QWS_ALPHA_CURSOR
    return gfx->bitDepth() >= 8;
#else
    return false;
#endif
}

/*!
    \internal

    Hide the mouse cursor from the screen.
*/
void QScreenCursor::hide()
{
    if (data->enable) {
        if (restoreUnder(data->bound))
            QWSDisplay::ungrab();
        delete gfx;
        gfx = 0;
        data->enable = false;
    }
}

/*!
    \internal

    Show the mouse cursor again after it has been hidden. Note that hides
    and shows are not nested; show() should always re-display the cursor no
    matter how many hide()s preceded it.
*/
void QScreenCursor::show()
{
    if (!data->enable) {
        if (qws_sw_cursor)
            QWSDisplay::grab(true);
        data->enable = true;
        gfx = (QGfxRasterBase*)qt_screen->screenGfx();
        gfx->setClipRegion(QRect(0, 0, qt_screen->width(), qt_screen->height()), Qt::ReplaceClip);
        fb_start = qt_screen->base();
        fb_end = fb_start + qt_screen->deviceHeight() * gfx->linestep();
        clipWidth = qt_screen->deviceWidth();
        clipHeight = qt_screen->deviceHeight();
        saveUnder();
    }
}

/*!
    \internal

    Sets a mouse cursor to \a image. The QImage is 32 bit, with an alpha
    channel containing either only 255 or 0 (that is, display the pixel
    or not) or a full alpha channel, depending on what
    supportsAlphaCursor() returns. \a hotx and \a hoty are the point within
    the QImage where mouse events actually 'come from'.
*/
void QScreenCursor::set(const QImage &image, int hotx, int hoty)
{
#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
    QWSDisplay::grab(true);
#endif
    bool save = restoreUnder(data->bound);
    data->hotx = hotx;
    data->hoty = hoty;
    data->width = image.width();
    data->height = image.height();
    for (int r = 0; r < image.height(); r++)
        memcpy(data->cursor+data->width*r, image.scanLine(r), data->width);
    data->colors = image.numColors();
    int depth = qt_screen->depth();
    if (depth <= 8) {
        for (int i = 0; i < image.numColors(); i++) {
            int r = qRed(image.colorTable()[i]);
            int g = qGreen(image.colorTable()[i]);
            int b = qBlue(image.colorTable()[i]);
            data->translut[i] = QColormap::instance().pixel(QColor(r, g, b));
        }
    }
    for (int i = 0; i < image.numColors(); i++) {
        data->clut[i] = image.colorTable()[i];
    }
    data->bound = QRect(data->x - data->hotx, data->y - data->hoty,
                   data->width+1, data->height+1);
    if (save) saveUnder();
#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
    QWSDisplay::ungrab();
#endif
}

/*!
    \internal

    Move the mouse cursor to point (\a x, \a y) on the screen. This should be done
    in such a way that the hotspot of the cursor is at (x,y) - e.g. if the
    hotspot is at 5,5 within the image then the top left of the image should
    be at x-5,y-5
*/
void QScreenCursor::move(int x, int y)
{
    bool save = false;
    if (qws_sw_cursor) {
#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
        QWSDisplay::grab(true);
#endif
        save = restoreUnder(data->bound);
    }
    data->x = x;
    data->y = y;
    data->bound = QRect(data->x - data->hotx, data->y - data->hoty,
                        data->width+1, data->height+1);
    if (qws_sw_cursor) {
        if (save)
            saveUnder();
#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
        QWSDisplay::ungrab();
#endif
    }
}

/*!
    \internal

    This is relevant only to the software mouse cursor and should be
    reimplemented as a null method in hardware cursor drivers. It redraws
    what was under the mouse cursor when the cursor is moved. \a r
    is the rectangle that needs updating,
*/
bool QScreenCursor::restoreUnder(const QRect &r)
{
    if (!qws_sw_cursor)
        return false;

    if (!data || !data->enable) {
        return false;
    }

    if (!r.intersects(data->bound)) {
        return false;
    }

    if (!save_under) {
#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
        QWSDisplay::grab(true);
#endif
        int depth = gfx->bitDepth();

        int x = data->x - data->hotx;
        int y = data->y - data->hoty;

        if (depth < 8) {
            if (data->width && data->height) {
                gfx->gfx_swcursor = false;   // prevent recursive call from blt
                QSize s(qt_screen->deviceWidth(), qt_screen->deviceHeight());
                QRect r(x,y,data->width,data->height);
                r = qt_screen->mapFromDevice(r, s);
                gfx->setSource(imgunder);
                gfx->setAlphaType(QGfx::IgnoreAlpha);
                gfx->blt(r.x(), r.y(), r.width(), r.height(),0,0);
                gfx->gfx_swcursor = true;
            }
        } else {
            // This is faster than the above - at least until blt is
            // better optimized.
            int linestep = gfx->linestep();
            int startCol = x < 0 ? qAbs(x) : 0;
            int startRow = y < 0 ? qAbs(y) : 0;
            int endRow = y + data->height > clipHeight ? clipHeight - y : data->height;
            int endCol = x + data->width > clipWidth ? clipWidth - x : data->width;

            int srcLineStep = data->width * depth/8;
            unsigned char *dest = fb_start + (y + startRow) * linestep
                                    + (x + startCol) * depth/8;
            unsigned char *src = data->under;

            if (endCol > startCol) {
                int bytes;
                if (depth < 8)
                    bytes = (x + endCol - 1)*depth/8 - (x + startCol)*depth/8 + 1;
                else
                    bytes = (endCol - startCol) * depth / 8;
                if (depth == 1) bytes++;
                for (int row = startRow; row < endRow; row++)
                {
                    memcpy(dest, src, bytes);
                    src += srcLineStep;
                    dest += linestep;
                }
            }
        }
        save_under = true;
        return true;
    }

    return false;
}

/*!
    \internal

    This saves the area under the mouse pointer - it should be reimplemented
    as a null method by hardware drivers.
*/
void QScreenCursor::saveUnder()
{
    if (!qws_sw_cursor)
        return;

    int depth = gfx->bitDepth();
    int x = data->x - data->hotx;
    int y = data->y - data->hoty;

    if (depth < 8) {
        gfxunder->gfx_swcursor = false;   // prevent recursive call from blt
        gfxunder->setAlphaType(QGfx::IgnoreAlpha);
        gfxunder->srclinestep = gfx->linestep();
        gfxunder->srcdepth = gfx->bitDepth();
        gfxunder->srcbits = gfx->buffer;
        gfxunder->srctype = QGfx::SourceImage;
        gfxunder->srcpixeltype = QScreen::NormalPixel;
        gfxunder->srcwidth = qt_screen->width();
        gfxunder->srcheight = qt_screen->height();
        gfxunder->setSourceWidgetOffset(0, 0);
        gfxunder->src_normal_palette = true;
        QSize s(qt_screen->deviceWidth(), qt_screen->deviceHeight());
        QRect r(x, y, data->width, data->height);
        r = qt_screen->mapFromDevice(r, s);
        gfxunder->blt(0,0,data->width,data->height,r.x(), r.y());
        gfxunder->gfx_swcursor = true;
    } else {
        // This is faster than the above - at least until blt is
        // better optimized.
        int linestep = gfx->linestep();
        int startRow = y < 0 ? qAbs(y) : 0;
        int startCol = x < 0 ? qAbs(x) : 0;
        int endRow = y + data->height > clipHeight ? clipHeight - y : data->height;
        int endCol = x + data->width > clipWidth ? clipWidth - x : data->width;

        int destLineStep = data->width * depth / 8;

        unsigned char *src = fb_start + (y + startRow) * linestep
                                + (x + startCol) * depth/8;
        unsigned char *dest = data->under;

        if (endCol > startCol) {
            int bytes;
            if (depth < 8)
                bytes = (x + endCol - 1)*depth/8 - (x + startCol)*depth/8 + 1;
            else
                bytes = (endCol - startCol) * depth / 8;
            for (int row = startRow; row < endRow; row++)
            {
                memcpy(dest, src, bytes);
                src += linestep;
                dest += destLineStep;
            }
        }
    }

    drawCursor();

    save_under = false;

#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
    QWSDisplay::ungrab();
#endif
}

/*!
    \internal

    This draws the software cursor. It should be reimplemented as a null
    method by hardware drivers
*/
void QScreenCursor::drawCursor()
{
    // We could use blt, but since cursor redraw speed is critical it
    // is all handled here.  Whether this is significantly faster is
    // questionable.
    int x = data->x - data->hotx;
    int y = data->y - data->hoty;

    /* ### experimental
    if (data->width != cursor->width() || data->height != cursor->height()) {
        delete cursor;
        cursor = new QImage(data->cursor, data->width, data->height, 8,
                         data->clut, data->colors, QImage::IgnoreEndian);
    }
    if (data->width && data->height) {
        qt_sw_cursor = false;   // prevent recursive call from blt
        gfx->setSource(cursor);
        gfx->setAlphaType(QGfx::InlineAlpha);
        gfx->blt(x,y,data->width,data->height,0,0);
        qt_sw_cursor = true;
    }

    return;
    */

    int linestep = gfx->linestep();
    int depth = gfx->bitDepth();

    // clipping
    int startRow = y < 0 ? qAbs(y) : 0;
    int startCol = x < 0 ? qAbs(x) : 0;
    int endRow = y + data->height > clipHeight ? clipHeight - y : data->height;
    int endCol = x + data->width > clipWidth ? clipWidth - x : data->width;

    unsigned char *dest = fb_start + (y + startRow) * linestep
                            + x * depth/8;
    unsigned const char *srcptr = data->cursor + startRow * data->width;

    QRgb *clut = data->clut;

#ifndef QT_NO_QWS_DEPTH_32
    if (depth == 32)
    {
        unsigned int *dptr = (unsigned int *)dest;
        unsigned int srcval;
        int av,r,g,b;
        for (int row = startRow; row < endRow; row++)
        {
            for (int col = startCol; col < endCol; col++)
            {
                srcval = clut[*(srcptr+col)];
                av = srcval >> 24;
                if (av == 0xff) {
                    *(dptr+col) = srcval;
                }
# ifndef QT_NO_QWS_ALPHA_CURSOR
                else if (av != 0) {
                    r = (srcval & 0xff0000) >> 16;
                    g = (srcval & 0xff00) >> 8;
                    b = srcval & 0xff;
                    unsigned int hold = *(dptr+col);
                    int sr=(hold & 0xff0000) >> 16;
                    int sg=(hold & 0xff00) >> 8;
                    int sb=(hold & 0xff);

                    r = ((r-sr) * av) / 256 + sr;
                    g = ((g-sg) * av) / 256 + sg;
                    b = ((b-sb) * av) / 256 + sb;

                    *(dptr+col) = (r << 16) | (g << 8) | b;
                }
# endif
            }
            srcptr += data->width;
            dptr += linestep/4;
        }
        return;
    }
#endif
#ifndef QT_NO_QWS_DEPTH_24
    if (depth == 24)
    {
        unsigned int srcval;
        int av,r,g,b;
        for (int row = startRow; row < endRow; row++)
        {
            unsigned char *dptr = dest + (row-startRow) * linestep + startCol * 3;
            for (int col = startCol; col < endCol; col++, dptr += 3)
            {
                srcval = clut[*(srcptr+col)];
                av = srcval >> 24;
                if (av == 0xff) {
                    gfxSetRgb24(dptr, srcval);
                }
# ifndef QT_NO_QWS_ALPHA_CURSOR
                else if (av != 0) {
                    r = (srcval & 0xff0000) >> 16;
                    g = (srcval & 0xff00) >> 8;
                    b = srcval & 0xff;
                    unsigned int hold = gfxGetRgb24(dptr);
                    int sr=(hold & 0xff0000) >> 16;
                    int sg=(hold & 0xff00) >> 8;
                    int sb=(hold & 0xff);

                    r = ((r-sr) * av) / 256 + sr;
                    g = ((g-sg) * av) / 256 + sg;
                    b = ((b-sb) * av) / 256 + sb;

                    gfxSetRgb24(dptr, r, g, b);
                }
# endif
            }
            srcptr += data->width;
        }
        return;
    }
#endif
#ifndef QT_NO_QWS_DEPTH_16
    if (depth == 16)
    {
        unsigned short *dptr = (unsigned short *)dest;
        unsigned int srcval;
        int av,r,g,b;
        for (int row = startRow; row < endRow; row++)
        {
            for (int col = startCol; col < endCol; col++)
            {
                srcval = clut[*(srcptr+col)];
                av = srcval >> 24;
                if (av == 0xff) {
                    *(dptr+col) = qt_convRgbTo16(srcval);
                }
# ifndef QT_NO_QWS_ALPHA_CURSOR
                else if (av != 0) {
                    // This is absolutely silly - but we can so we do.
                    r = (srcval & 0xff0000) >> 16;
                    g = (srcval & 0xff00) >> 8;
                    b = srcval & 0xff;

                    int sr;
                    int sg;
                    int sb;
                    qt_conv16ToRgb(*(dptr+col),sr,sg,sb);

                    r = ((r-sr) * av) / 256 + sr;
                    g = ((g-sg) * av) / 256 + sg;
                    b = ((b-sb) * av) / 256 + sb;

                    *(dptr+col) = qt_convRgbTo16(r,g,b);
                }
# endif
            }
            srcptr += data->width;
            dptr += linestep/2;
        }
        return;
    }
#endif
#if !defined(QT_NO_QWS_DEPTH_8)
    if (depth == 8) {
        unsigned char *dptr = (unsigned char *)dest;
        unsigned int srcval;
        int av,r,g,b;
        QRgb * screenclut=qt_screen->clut();
        simple_8bpp_alloc=true;
        for (int row = startRow; row < endRow; row++)
        {
            for (int col = startCol; col < endCol; col++)
            {
                srcval = clut[*(srcptr+col)];
                av = srcval >> 24;
                if (av == 0xff) {
                    *(dptr+col) = data->translut[*(srcptr+col)];
                }
# ifndef QT_NO_QWS_ALPHA_CURSOR
                else if (av != 0) {
                    // This is absolutely silly - but we can so we do.
                    r = (srcval & 0xff0000) >> 16;
                    g = (srcval & 0xff00) >> 8;
                    b = srcval & 0xff;

                    unsigned char hold = *(dptr+col);
                    int sr,sg,sb;
                    sr=qRed(screenclut[hold]);
                    sg=qGreen(screenclut[hold]);
                    sb=qBlue(screenclut[hold]);

                    r = ((r-sr) * av) / 256 + sr;
                    g = ((g-sg) * av) / 256 + sg;
                    b = ((b-sb) * av) / 256 + sb;

                    *(dptr+col) = GFX_CLOSEST_PIXEL_CURSOR(r,g,b);
                }
# endif
            }
            srcptr += data->width;
            dptr += linestep;
        }
        simple_8bpp_alloc=false;
    }
#endif
#ifndef QT_NO_QWS_DEPTH_4
    if (depth == 4) {
        unsigned int srcval;
        int av;
        for (int row = startRow; row < endRow; row++)
        {
            unsigned char *dp = fb_start + (y + row) * linestep;
            for (int col = startCol; col < endCol; col++)
            {
                srcval = clut[*(srcptr+col)];
                av = srcval >> 24;
                if (av == 0xff) {
                    int tx = x + col;
                    unsigned char *dptr = dp + (tx>>1);
                    int val = data->translut[*(srcptr+col)];
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                    int s = (~tx & 1) << 2;
#else
                    int s = (tx & 1) << 2;
#endif
                    *dptr = (*dptr & MASK4BPP(s)) | (val << s);
                }
            }
            srcptr += data->width;
        }
    }
#endif
#ifndef QT_NO_QWS_DEPTH_1
    if (depth == 1) {
        unsigned int srcval;
        int av;
        for (int row = startRow; row < endRow; row++)
        {
            unsigned char *dp = fb_start + (y + row) * linestep;
            int x1 = x+startCol;
            int x2 = x+endCol-1;
            dp += x1/8;
            int skipbits = x1%8;
            int col = startCol;
            for (int b = x1/8; b <= x2/8; b++) {
                unsigned char m = *dp;
                for (int i = 0; i < 8 && col < endCol; i++) {
                    if (skipbits)
                        skipbits--;
                    else {
                        srcval = clut[*(srcptr+col)];
                        av = srcval >> 24;
                        if (av == 0xff) {
                            unsigned char val = data->translut[*(srcptr+col)];
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                            if (val)
                                m |= 0x80 >> i;
                            else
                                m &= ~(0x80 >> i);
#else
                            if (val)
                                m |= 1 << i;
                            else
                                m &= ~(1 << i);
#endif
                        }
                        col++;
                    }
                }
                *(dp++) = m;
            }
            srcptr += data->width;
        }
    }
#endif
}
#endif // QT_NO_QWS_CURSOR



/*!
    \class QGfxRaster qgfxraster_qws.h
    \brief The QGfxRaster class is QGfxRasterBase specialized for a particular bit
    depth, specified by the depth parameter of the template. The type field
    is currently not used. In future versions, it may be used to specify the
    pixel storage format.

    \internal (for now)

    \ingroup qws

    Many operations with QGfxRaster are specified along the lines of

    if(depth==32) {
    ...
    } else if(depth==16) {
    ...

    The intention is that the compiler will realise when instantiating a template
    for a particular depth that it need only include the code for that depth -
    so if you never use an 8-bit gfx, for example, the 8-bit code will not
    be included in your executable. The actual drawing code for software-only
    rendering is all included in this class; it should be subclassed by hardware
    drivers so that they can fall back to it for anything that hardware can't
    handle.
*/

/*!
    \fn QGfxRaster::QGfxRaster(unsigned char *b, int w, int h)

    \internal

    Constructs a QGfxRaster for a particular depth with a framebuffer pointed
    to by \a b, with a width and height of \a w and \a h (specified in
    pixels, not bytes)
*/
template <const int depth, const int type>
QGfxRaster<depth,type>::QGfxRaster(unsigned char *b, int w, int h)
    : QGfxRasterBase(b,w,h)
{
    setLineStep((depth*width+7)/8);
    if (depth == 1) {
        setPen(QColor(Qt::color1));
        setBrush(QColor(Qt::color0));
    }
}

/*!
    \fn QGfxRaster::~QGfxRaster()

    \internal

    Destroys a QGfxRaster.
*/
template <const int depth, const int type>
QGfxRaster<depth,type>::~QGfxRaster()
{
}

/*!
    \fn void QGfxRaster<depth,type>::calcPacking(void *m, int x1, int x2, int &frontadd, int &backadd, int &count)

    \internal

    This is an internal method used by methods which pack writes to the
    framebuffer for optimisation reasons. It takes longer to write
    80 8-bit values over the PCI or AGP bus to a graphics card than
    it does 20 32-bit values, as long as those 32-bit values are aligned
    on a 32-bit boundary. Therefore the code writes individual pixels
    up to a boundary, writes 32-bit values until it reaches the last boundary
    before the end of the line, and draws then individual pixels again.
    Given a pointer to a start of the line within the framebuffer \a m
    and starting and ending x coordinates \a x1 and \a x2, \a frontadd is filled
    with the number of individual pixels to write at the start of the line,
    \a count with the number of 32-bit values to write and \a backadd with the
    number of individual pixels to write at the end. This optimisation
    yields up to 60% drawing speed performance improvements when Memory
    Type Range Registers are not available, and still gives a few percent
    when write-combining on the framebuffer is enabled using those registers
    (which effectively causes the CPU to do a similar optimisation in hardware)
*/
template<const int depth,const int type>
Q_GFX_INLINE void QGfxRaster<depth,type>::calcPacking(void *m, int x1, int x2,
                                                    int &frontadd, int &backadd, int &count)
{
    // Calculate packing values for 32-bit writes
    int w = x2-x1+1;

#ifndef QWS_NO_WRITE_PACKING
    if (depth == 16) {
        if (w < 2)
            goto unpacked;

        unsigned short int * myptr=(unsigned short int *)m;
        frontadd=(((unsigned long)myptr)+(x1*2)) & 0x3;
        backadd=(((unsigned long)myptr)+((x2+1)*2)) & 0x3;
        if (frontadd)
            frontadd = 4 - frontadd;
        frontadd >>= 1;
        backadd >>= 1;
        count=(w-(frontadd+backadd));
        count >>= 1;
    } else if (depth == 8) {
        if (w < 4)
            goto unpacked;

        unsigned char * myptr=(unsigned char *)m;
        frontadd=(((unsigned long)myptr)+x1) & 0x3;
        backadd=(((unsigned long)myptr)+x2+1) & 0x3;
        if (frontadd)
            frontadd = 4 - frontadd;
        count = w-(frontadd+backadd);
        count >>= 2;
    } else {
        goto unpacked;
    }

    if(count<0)
        count=0;
    if(frontadd<0)
        frontadd=0;
    if(backadd<0)
        backadd=0;
    return;
#endif

unpacked:
    frontadd = w;
    backadd = 0;
    count = 0;
    if(frontadd<0)
        frontadd=0;
}

/*!
    \fn void QGfxRaster<depth,type>::setSource(const QPaintDevice *p)

    \internal
    \overload

    This sets the gfx to use an arbitrary paintdevice \a p as the source for
    future data. It sets up a default alpha-blending value of IgnoreAlpha.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(const QPaintDevice *p)
{
    // if the source is 1bpp, the pen and brush currently active will be used
    srclinestep=((QPaintDevice *)p)->qwsBytesPerLine();
    srcdepth=p->depth();
    if(srcdepth==0)
        abort();
    srcbits=((QPaintDevice *)p)->qwsScanLine(0);
    srctype=SourceImage;
    srcpixeltype=QScreen::NormalPixel;
    setAlphaType(IgnoreAlpha);
    if (p->devType() == QInternal::Widget) {
        QWidget * w=(QWidget *)p;
        srcwidth=w->width();
        srcheight=w->height();
        QPoint hold;
        hold=w->mapToGlobal(hold);
        setSourceWidgetOffset(hold.x(), hold.y());
        if (srcdepth == 1) {
            buildSourceClut(0, 0);
        } else if(srcdepth <= 8) {
            src_normal_palette=true;
        }
    } else if (p->devType() == QInternal::Pixmap) {
        //still a bit ugly
        QPixmap *pix = (QPixmap*)p;
        srcwidth=pix->width();
        srcheight=pix->height();
        setSourceWidgetOffset(0, 0);
        if (srcdepth == 1) {
            buildSourceClut(0, 0);
        } else if(srcdepth <= 8) {
            src_normal_palette=true;
        }
    } else {
        // This is a bit ugly #### I'll say!
        //### We will have to find another way to do this
        setSourceWidgetOffset(0, 0);
        buildSourceClut(0,0);
    }

    src_little_endian=true;
}

/*!
    \fn QGfxRaster<depth,type>::setSource(const QImage *i)

    \internal

    This sets up future blt's to use a QImage \a i as a source - used by
    QPainter::drawImage()
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(const QImage *i)
{
    //qWarning("QGfxRaster::setSource(const QImage*)");
    srctype=SourceImage;
    srcpixeltype=QScreen::NormalPixel;
    srclinestep=i->bytesPerLine();
    srcdepth=i->depth();
    if(srcdepth==0)
        abort();
    srcbits=i->scanLine(0);
    src_little_endian=(i->bitOrder()==QImage::LittleEndian);
    QSize s = gfx_screen->mapToDevice(QSize(i->width(), i->height()));
    srcwidth=s.width();
    srcheight=s.height();
    setSourceWidgetOffset(0, 0);
    src_normal_palette=false;
    if (srcdepth == 1)
        buildSourceClut(0, 0);
    else  if(srcdepth<=8)
        buildSourceClut(const_cast<QRgb*>(i->colorTable().constData()),i->numColors());
}

/*!
    \fn QGfxRaster<depth,type>::setSource(unsigned char *c, int w, int h, int l, int d, QRgb *clut, int numcols)

    \internal

    This sets up future blt's to use \a c as source, and sets up necessary Cluts.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(unsigned char *c, int w, int h, int l, int d, QRgb *clut,
                                       int numcols)
{
    srctype=SourceImage;
    srcpixeltype=QScreen::NormalPixel;
    srclinestep=l;
    srcdepth=d;
    srcbits=c;
    src_little_endian=true;
    setSourceWidgetOffset(0,0);
    srcwidth=w;
    srcheight=h;
    src_normal_palette=false;
    if (srcdepth == 1)
        buildSourceClut(0, 0);
    else  if(srcdepth<=8)
        buildSourceClut(clut,numcols);
}

/*!
    \fn void QGfxRaster::buildSourceClut(const QRgb * cols,int numcols)
    \internal

    This is an internal method used to optimise blt's from paletted to paletted
    data, where the palettes are different for each. A lookup table
    indexed by the source value providing the destination value is
    filled in. \a cols is the source data's color lookup table,
    \a numcols the number of entries in it. If \a cols is 0 some default
    values are put in (this is for 1bpp sources which don't have
    a palette).
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::buildSourceClut(const QRgb * cols,int numcols)
{
    if (!cols) {
        pixel = brushPixel;
#if !defined(QT_NO_IMAGE_16_BIT) || !defined(QT_NO_QWS_DEPTH_16)
        if (qt_screen->depth() == 16 && depth==32) {
            srcclut[0]=qt_conv16ToRgb(pixel);
            transclut[0]=qt_conv16ToRgb(pixel);
        } else
#endif
        {
            srcclut[0]=pixel;
            transclut[0]=pixel;
        }
        pixel = penPixel;
#if !defined(QT_NO_IMAGE_16_BIT) || !defined(QT_NO_QWS_DEPTH_16)
        if (qt_screen->depth() == 16 && depth==32) {
            srcclut[1]=qt_conv16ToRgb(pixel);
            transclut[1]=qt_conv16ToRgb(pixel);
        } else
#endif
        {
            srcclut[1]=pixel;
            transclut[1]=pixel;
        }
        return;
    }

    int loopc;

    // Copy clut
    for(loopc=0;loopc<numcols;loopc++)
        srcclut[loopc] = cols[loopc];

    if(depth<=8) {
        // Now look for matches
        for(loopc=0;loopc<numcols;loopc++) {
            int r = qRed(srcclut[loopc]);
            int g = qGreen(srcclut[loopc]);
            int b = qBlue(srcclut[loopc]);
            transclut[loopc] = gfx_screen->alloc(r,g,b);
        }
    }
}

/*!
    \fn void QGfxRaster<depth,type>::drawPointUnclipped(int x, unsigned char *l)

    \internal

    This draws a point in the scanline pointed to by \a l, at the position \a x,
    without taking any notice of clipping. It's an internal method called
    by drawPoint(), and vLine().
*/
template <const int depth, const int type>
Q_GFX_INLINE void QGfxRaster<depth,type>::drawPointUnclipped(int x, unsigned char *l)
{
    //screen coordinates
    if (depth == 32)
        ((QRgb*)l)[x] = pixel;
    else if (depth == 24)
        gfxSetRgb24(l + x*3, pixel);
    else if (depth == 16)
        ((ushort*)l)[x] = (pixel & 0xffff);
    else if (depth == 8)
        l[x] = (pixel & 0xff);
    else if (depth == 4) {
        uchar *d = l + (x>>1);
        int s = (x & 1) << 2;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        if (is_screen_gfx)
            s = (~x & 1) << 2;
#endif
        *d = (*d & MASK4BPP(s)) | (pixel << s);
    } else if (depth == 1)
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        if (is_screen_gfx) {
        if (pixel)
            l[x/8] |= 0x80 >> (x%8);
        else
            l[x/8] &= ~(0x80 >> (x%8));
        } else
#endif
        if (pixel)
            l[x/8] |= 1 << (x%8);
        else
            l[x/8] &= ~(1 << (x%8));
}

/*!
    \fn void QGfxRaster<depth,type>::drawAlphaPointUnclipped(int x, unsigned char *l)

    \internal

    This draws an alpha point in the scanline pointed to by \a l, at the position
    \a x, without taking any notice of clipping. It's an internal method called
    by drawPoint(), and vLine();
*/
template <const int depth, const int type> //screen coordinates
Q_GFX_INLINE void QGfxRaster<depth,type>::drawAlphaPointUnclipped(int x, unsigned char *l)
{
    int alpha = qAlpha(penColor);
    if (depth == 32) {
        l += x * 4;
        QRgb *org = reinterpret_cast<QRgb *>(l);
        int srcR = qRed(penColor);
        int srcG = qGreen(penColor);
        int srcB = qBlue(penColor);
        int dstR = qRed(*org);
        int dstG = qGreen(*org);
        int dstB = qBlue(*org);
        dstR += ((srcR - dstR) * alpha) >> 8;
        dstG += ((srcG - dstG) * alpha) >> 8;
        dstB += ((srcB - dstB) * alpha) >> 8;
        *org = qRgba(dstR, dstG, dstB, qAlpha(*org));
    } else if (depth == 24) {
        l += x * 3;
        int dstR, srcR = qRed(penColor);
        int dstG, srcG = qGreen(penColor);
        int dstB, srcB = qBlue(penColor);
        gfxGetRgb24(l, dstR, dstG, dstB);
        dstR += ((srcR - dstR) * alpha) >> 8;
        dstG += ((srcG - dstG) * alpha) >> 8;
        dstB += ((srcB - dstB) * alpha) >> 8;
        gfxSetRgb24(l, dstR, dstG, dstB);
    } else if (depth == 16) {
        l += x * 2;
        unsigned short *org = reinterpret_cast<unsigned short *>(l);
        int dstR, srcR = qRed(penColor);
        int dstG, srcG = qGreen(penColor);
        int dstB, srcB = qBlue(penColor);
        qt_conv16ToRgb(*org, dstR, dstG, dstB);
        dstR += ((srcR - dstR) * alpha) >> 8;
        dstG += ((srcG - dstG) * alpha) >> 8;
        dstB += ((srcB - dstB) * alpha) >> 8;
        *org = qt_convRgbTo16(dstR, dstG, dstB);
    } else if (depth == 8) {
        l += x;
        QRgb conv32 = clut[*l];
        int dstR = qRed(conv32),   srcR = qRed(penColor);
        int dstG = qGreen(conv32), srcG = qGreen(penColor);
        int dstB = qBlue(conv32),  srcB = qBlue(penColor);
        dstR += ((srcR - dstR) * alpha) >> 8;
        dstG += ((srcG - dstG) * alpha) >> 8;
        dstB += ((srcB - dstB) * alpha) >> 8;
        *l = GFX_CLOSEST_PIXEL(dstR, dstG, dstB);
    } else if (depth == 4) {
        l += x >> 1;
        int bitShift = x & 1 ? 4 : 0;
        QRgb conv32 = clut[ (*l >> bitShift) & 0x0f ];
        int dstR = qRed(conv32),   srcR = qRed(penColor);
        int dstG = qGreen(conv32), srcG = qGreen(penColor);
        int dstB = qBlue(conv32),  srcB = qBlue(penColor);
        dstR += ((srcR - dstR) * alpha) >> 8;
        dstG += ((srcG - dstG) * alpha) >> 8;
        dstB += ((srcB - dstB) * alpha) >> 8;
        if (bitShift)
            *l = (*l & 0x0f) | GFX_CLOSEST_PIXEL(dstR, dstG, dstB) << 4;
        else
            *l = (*l & 0xf0) | GFX_CLOSEST_PIXEL(dstR, dstG, dstB);;
    } else if (depth == 1) {
        l += x >> 3;
        bool isPenBlack = qGray(penColor) < 0x80;
        uchar bitNumInL = 1 << (x & 0x07);
        if (!(*l & bitNumInL) != isPenBlack)
            *l ^= bitNumInL;
    }
}

/*!
    \fn void QGfxRaster<depth,type>::drawPoint(int x, int y)

    \internal

    Draw a point at \a x, \a y in the current pen color. As with most
    externally-called dawing methods x and y are relevant to the current gfx
    offset, stored in the variables xoffs and yoffs.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::drawPoint(int x, int y)
{
    if (!ncliprect)
        return;
    if(cpen.style()==Qt::NoPen)
        return;
    x += xoffs;
    y += yoffs;

    GFX_START(QRect(x,y,2,2))
    if((*gfx_optype))
        sync();
    (*gfx_optype)=0;
        pixel = penPixel;
    if (inClip(x,y)) {
        if (qAlpha(penColor) == 255)
            drawPointUnclipped(x, scanLine(y));
        else
            drawAlphaPointUnclipped(x, scanLine(y));
    }
    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::drawPoints(const QPolygon &pa, int index, int npoints)

    \internal

    Draw \a npoints points from position \a index in the array of points \a pa.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::drawPoints(const QPolygon &pa, int index, int npoints)
{
    if (!ncliprect)
        return;
    if(cpen.style()==Qt::NoPen)
        return;
    pixel = penPixel;
    QRect cr;
    bool in = false;
    bool foundone=(((*gfx_optype)==0) ? true : false);

    bool useAlpha = (qAlpha(penColor) != 255);
    GFX_START(clipbounds);
    while (npoints--) {
        int x = pa[index].x() + xoffs;
        int y = pa[index].y() + yoffs;
        if (!cr.contains(x,y)) {
            in = inClip(x,y,&cr);
        }
        if (in) {
            if(foundone==false) {
                sync();
                (*gfx_optype)=0;
                foundone=true;
            }
            if (useAlpha)
                drawAlphaPointUnclipped(x, scanLine(y));
            else
                drawPointUnclipped(x, scanLine(y));
        }
        ++index;
    }
    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::drawLine(int x1, int y1, int x2, int y2)

    \internal

    Draw a line in the current pen style from \a x1 \a y1 to \a x2 \a y2
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::drawLine(int x1, int y1, int x2, int y2)
{
    if (!ncliprect)
        return;
    if (cpen.style()==Qt::NoPen)
        return;

    if (cpen.width() > 1) {
        drawThickLine(x1, y1, x2, y2);
        return;
    }

    pixel = penPixel;
    setSourcePen();
    int alphaValue = qAlpha(penColor);
    setAlphaSource(alphaValue);
    setAlphaType(alphaValue == 255 ? IgnoreAlpha : SolidAlpha);


    x1+=xoffs;
    y1+=yoffs;
    x2+=xoffs;
    y2+=yoffs;

    if(x1>x2) {
        int x3;
        int y3;
        x3=x2;
        y3=y2;
        x2=x1;
        y2=y1;
        x1=x3;
        y1=y3;
#ifdef GFX_CORRECT_POLYLINE_JOIN
        if (gfx_noLineOverwrite) {
            if (x2-x1 > qAbs(y2-y1))
                gfx_storedLineRead -= x2-x1;
            else
                gfx_storedLineRead -= qAbs(y2-y1);
            gfx_storedLineDir = -gfx_storedLineDir;
        }
#endif
    }

    int dx=x2-x1;
    int dy=y2-y1;

    GFX_START(QRect(x1, y1 < y2 ? y1 : y2, dx+1, qAbs(dy)+1))

    if((*gfx_optype))
    sync();
    (*gfx_optype)=0;

    bool useAlpha = (qAlpha(penColor) != 255);
#ifdef QWS_EXPERIMENTAL_FASTPATH
    // Fast path
    if (!dashedLines && !gfx_storeLine) {
        if (y1 == y2) {
            if (ncliprect == 1) {
                if (x1 > cliprect[0].right() || x2 < cliprect[0].left()
                        || y1 < cliprect[0].top() || y1 > cliprect[0].bottom()) {
                    GFX_END
                    return;
                }
                x1 = x1 > cliprect[0].left() ? x1 : cliprect[0].left();
                x2 = x2 > cliprect[0].right() ? cliprect[0].right() : x2;
                unsigned char *l = scanLine(y1);
                hlineUnclipped(x1,x2,l);
            } else {
                hline(x1, x2, y1);
            }
            GFX_END
            return;
        }
        else if (x1 == x2) {
            vline(x1, y1, y2);
            GFX_END
            return;
        }
    }
#endif
    // Bresenham algorithm from Graphics Gems

    int ax=qAbs(dx)*2;
    int ay=qAbs(dy)*2;
    int sy=dy>0 ? 1 : -1;
    int x=x1;
    int y=y1;

    int d;
    bool doDraw = true;

    QRect cr;
    bool inside = inClip(x,y,&cr);
    if(ax>ay && !dashedLines && !gfx_storeLine) {
        unsigned char* l = scanLine(y);
        d=ay-(ax >> 1);
        int px=x;
        #define FLUSH(nx) \
                if (inside) \
                    hlineUnclipped(px,nx,l); \
                px = nx+1;
        for(;;) {
            if(x==x2) {
                FLUSH(x);
                GFX_END
                return;
            }
            if(d>=0) {
                FLUSH(x);
                y+=sy;
                d-=ax;
                l = scanLine(y);
                if (!cr.contains(x+1,y))
                    inside = inClip(x+1,y, &cr);
            } else if (!cr.contains(x+1,y)) {
                FLUSH(x);
                inside = inClip(x+1,y, &cr);
            }
            x++;
            d+=ay;
        }
    } else if (ax > ay) {
        // cannot use hline for dashed lines
        int di = 0;
        int dc = dashedLines ? dashes[0] : 0;
        d=ay-(ax >> 1);
        for(;;) {
            if (!cr.contains(x,y))
                inside = inClip(x,y, &cr);
#ifdef GFX_CORRECT_POLYLINE_JOIN
            if (gfx_storeLine) {
                doDraw = gfx_doDraw;
                QPoint pt(x, y);
                if (gfx_noLineOverwrite) {
                    if (gfx_storedLineRead >= 0 && pt == gfx_storedLineRd[gfx_storedLineRead]) {
                        // we drew this point last time.
                        doDraw = false;
                    }
                    gfx_storedLineRead += gfx_storedLineDir;
                    if (gfx_storedLineRead >= gfx_storedLineBufferSize)
                        gfx_noLineOverwrite = false;
                }
                gfx_storedLineWr[gfx_storedLineWrite++] = pt;
                if (gfx_storedLineWrite >= gfx_storedLineBufferSize)
                    gfx_storeLine = false;
            }
#endif
            if (doDraw && inside && (di&0x01) == 0) {
                if(useAlpha)
                    drawAlphaPointUnclipped(x, scanLine(y));
                else
                    drawPointUnclipped(x, scanLine(y));
            }
            if(x==x2) {
                GFX_END
                return;
            }
            if (dashedLines && --dc <= 0) {
                if (++di >= numDashes)
                    di = 0;
                dc = dashes[di];
            }
            if(d>=0) {
                y+=sy;
                d-=ax;
            }
            x++;
            d+=ay;
        }
    } else {
        int di = 0;
        int dc = dashedLines ? dashes[0] : 0;
        d=ax-(ay >> 1);
        for(;;) {
            // y is dominant so we can't optimise with hline
            if (!cr.contains(x,y))
                inside = inClip(x,y, &cr);
#ifdef GFX_CORRECT_POLYLINE_JOIN
            if (gfx_storeLine) {
                doDraw = gfx_doDraw;
                QPoint pt(x, y);
                if (gfx_noLineOverwrite) {
                    if (gfx_storedLineRead >= 0 && pt == gfx_storedLineRd[gfx_storedLineRead]) {
                        // we drew this point last time.
                        doDraw = false;
                    }
                    gfx_storedLineRead += gfx_storedLineDir;
                    if (gfx_storedLineRead >= gfx_storedLineBufferSize)
                        gfx_noLineOverwrite = false;
                }
                gfx_storedLineWr[gfx_storedLineWrite++] = pt;
                if (gfx_storedLineWrite >= gfx_storedLineBufferSize)
                    gfx_storeLine = false;
            }
#endif
            if (doDraw && inside && (di&0x01) == 0)
                if(useAlpha)
                    drawAlphaPointUnclipped(x, scanLine(y));
                else
                    drawPointUnclipped(x, scanLine(y));
            if(y==y2) {
                GFX_END
                return;
            }
            if (dashedLines && --dc <= 0) {
                if (++di >= numDashes)
                    di = 0;
                dc = dashes[di];
            }
            if(d>=0) {
                x++;
                d-=ay;
            }
            y+=sy;
            d+=ax;
        }
    }
    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::vline(int x, int y1, int y2)

    Draw a line at coordinate \a x from \a y1 to \a y2.

    Performs clipping.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::vline(int x, int y1, int y2) //screen coordinates, clipped
{
    if (y1 > y2) {
        int ty = y2;
        y2 = y1;
        y1 = ty;
    }

    bool useAlpha = (qAlpha(penColor) != 255);
    // gross clip.
    if (y1 > clipbounds.bottom() || y2 < clipbounds.top())
        return;
    if (y1 < clipbounds.top())
        y1 = clipbounds.top();
    if (y2 > clipbounds.bottom())
        y2 = clipbounds.bottom();
    /*
    qDebug("x %d, y1 %d, y2 %d, cliprects %d", x, y1, y2, ncliprect);
    for (int i = 0; i < ncliprect; i++) {
        QRect r = cliprect[i];
        qDebug("clip: %d, %d, %d, %d", r.left(), r.top(), r.right(), r.bottom());
    }
    */
    QRect cr;
    bool plot=inClip(x,y1,&cr);
    int y=y1;
    for (;;) {
        int yb = cr.bottom();
        if (yb >= y2) {
            if (plot) {
                unsigned char *sl = scanLine(y);
                for (int r = y; r <= y2; r++) {
                    if (useAlpha)
                        drawAlphaPointUnclipped(x, sl);
                    else
                        drawPointUnclipped(x, sl);
                    sl += lstep;
                }
            }
            break;
        } else {
//            qDebug("Y = %d, cl %d, cr %d, ct %d, cb %d, clipcursor %d", y, cr.left(), cr.right(), cr.top(), cr.bottom(), clipcursor);
            if (plot) {
                unsigned char *sl = scanLine(y);
                for (int r = y; r <= yb; r++) {
                    if (useAlpha)
                        drawAlphaPointUnclipped(x, sl);
                    else
                        drawPointUnclipped(x, sl);
                    sl += lstep;
                }
            }
            y=yb+1;
            plot=inClip(x,y,&cr,plot);
        }
    }
//    qDebug("Done");
}

/*!
    \fn void QGfxRaster<depth, type>::drawThickLine(int x1, int y1, int x2, int y2)

    \internal

    Draw a line with a thickness greater than one pixel from \a x1, \a y1
    to \a x2, \a y2. Called from drawLine when necessary.
*/
template <const int depth, const int type>
void QGfxRaster<depth, type>::drawThickLine(int x1, int y1, int x2, int y2)
{
    QPolygon pa(2);
    pa.setPoint(0,x1,y1);
    pa.setPoint(1,x2,y2);
    drawThickPolyline(pa,0,2);
}

/*!
    \fn void QGfxRaster<depth,type>::drawThickPolyline(const QPolygon &points, int index, int npoints)

    \internal

    Draw a series of lines of a thickness greater than one pixel - called
    from drawPolyline as necessary. \a points is the array of points to
    use, \a index is the offset at which to start in that array, \a npoints
    is the number of points to use.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::drawThickPolyline(const QPolygon &points, int index, int npoints)
{
    if (npoints < 2)
        return;
    bool close = points[index] == points[index+npoints-1];
    QPolygon pa = convertThickPolylineToPolygon(points, index,
        close ? npoints-1 : npoints,
        cpen.width(), cpen.joinStyle(), close);

    pixel = penPixel;
    setSourcePen();
    int alphaValue = qAlpha(penColor);
    setAlphaSource(alphaValue);
    setAlphaType(alphaValue == 255 ? IgnoreAlpha : SolidAlpha);

    GFX_START(clipbounds)
    if((*gfx_optype)!=0) {
        sync();
    }
    (*gfx_optype)=0;
    scan(pa, true, 0, pa.count(), false);
    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::hline(int x1, int x2, int y)

    \internal

    Draw a line at coordinate \a y from \a x1 to \a x2 - used by the polygon
    drawing code. Performs clipping.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::hline(int x1, int x2, int y) //screen coordinates, clipped, x1<=x2
{
    // gross clip.
    if (x1 > clipbounds.right() || x2 < clipbounds.left())
        return;
    if (x1 < clipbounds.left())
        x1 = clipbounds.left();
    if (x2 > clipbounds.right())
        x2 = clipbounds.right();

    QRect cr;
    unsigned char *l = scanLine(y);
    bool plot = inClip(x1, y, &cr);
    int x=x1;
    for (;;) {
        int xr = cr.right();
        if (xr >= x2) {
            if (plot)
                hlineUnclipped(x,x2,l);
            break;
        } else {
            if (plot)
                hlineUnclipped(x,xr,l);
            x=xr+1;
            plot=inClip(x,y,&cr,plot);
        }
    }
}

/*!
    \fn void QGfxRaster<depth,type>::hlineUnclipped(int x1, int x2, unsigned char *l)

    \internal

    Draws a line in the current pen color from \a x1 to \a x2 on scanline \a l,
    ignoring clipping. Used by anything that draws in solid colors - drawLine,
    fillRect, and drawPolygon.
*/
template <const int depth, const int type> //screen coordinates, unclipped, x1<=x2, x1>=0
Q_GFX_INLINE void QGfxRaster<depth,type>::hlineUnclipped(int x1, int x2, unsigned char *l)
{
    int w = x2-x1+1;
    // Use hAlphaLineUnclipped if we are drawing with an alpha pen
    if (alphatype == SolidAlpha && srctype == SourcePen && calpha != 255) {
//        printf("Using hAlphaLineUnclipped instead of hLineUnclipped!\n");
        alphabuf = new unsigned int[w];
        hAlphaLineUnclipped(x1, x2, l, 0, 0);
        delete[] alphabuf;
        alphabuf = 0;
        return;
    }

    if (depth == 32) {
        unsigned int *myptr=(unsigned int *)l + x1;
        while (w--)
            *(myptr++) = pixel;
    } else if (depth == 24) {
        unsigned char *myptr = l + x1*3;
        while (w--) {
            gfxSetRgb24(myptr, pixel);
            myptr += 3;
        }
    } else if (depth == 16) {
        unsigned short int *myptr=(unsigned short int *)l;
#ifdef QWS_NO_WRITE_PACKING
        myptr+=x1;
        while (w--)
            *(myptr++) = pixel;
#else
        int frontadd;
        int backadd;
        int count;
        calcPacking(myptr,x1,x2,frontadd,backadd,count);

        myptr+=x1;

        PackType put = pixel | (pixel << 16);

        while (frontadd--)
            *(myptr++)=pixel;
        // Duffs device.
        PackType *myptr2 = (PackType*)myptr;
        myptr += count * 2;
        PackType *end2 = (PackType*)myptr;
        switch(count%8){
            case 0:
                while (myptr2 != end2) {
                    *myptr2++ = put;
            case 7: *myptr2++ = put;
            case 6: *myptr2++ = put;
            case 5: *myptr2++ = put;
            case 4: *myptr2++ = put;
            case 3: *myptr2++ = put;
            case 2: *myptr2++ = put;
            case 1: *myptr2++ = put;
                }
        }
        while (backadd--)
            *(myptr++)=pixel;
#endif
    } else if (depth == 8) {
        unsigned char *myptr=l;
#ifdef QWS_NO_WRITE_PACKING
        myptr+=x1;
        while (w--)
            *(myptr++) = pixel;
#else
        int frontadd,backadd,count;
        calcPacking(myptr,x1,x2,frontadd,backadd,count);

        myptr+=x1;

        PackType put = pixel | (pixel << 8)
            | (pixel << 16) | (pixel << 24);

        while (frontadd--)
            *(myptr++)=pixel;
        // Duffs device.
        PackType *myptr2 = (PackType*)myptr;
        myptr += count * 4;
        PackType *end2 = (PackType*)myptr;
        switch(count%8){
            case 0:
                while (myptr2 != end2) {
                    *myptr2++ = put;
                    case 7: *myptr2++ = put;
                    case 6: *myptr2++ = put;
                    case 5: *myptr2++ = put;
                    case 4: *myptr2++ = put;
                    case 3: *myptr2++ = put;
                    case 2: *myptr2++ = put;
                    case 1: *myptr2++ = put;
                }
        }
        while (backadd--)
            *(myptr++)=pixel;
#endif
    } else if (depth == 4) {
        unsigned char *myptr=l;
        unsigned char *dptr = myptr + x1/2;
        if (x1&1) {
            drawPointUnclipped(x1, myptr);
            w--;
            dptr++;
        }

        unsigned char val = pixel | (pixel << 4);
        while (w > 1) {
            *dptr++ = val;
            w -= 2;
        }

        if (!(x2&1))
            drawPointUnclipped(x2, myptr);
    } else if (depth == 1) {
        //#### we need to use semaphore
        l += x1/8;
        if (x1/8 == x2/8) {
            // Same byte

            uchar mask = (0xff << (x1 % 8)) & (0xff >> (7 - x2 % 8));
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx)
            mask = (0xff >> (x1 % 8)) & (0xff << (7 - x2 % 8));
#endif
            if (pixel)
                *l |= mask;
            else
                *l &= ~mask;
        } else {
            volatile unsigned char *last = l + (x2/8-x1/8);
            uchar mask = 0xff << (x1 % 8);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx)
                mask = 0xff >> (x1 % 8);
#endif
            if (pixel)
                *l++ |= mask;
            else
                *l++ &= ~mask;
            unsigned char byte = pixel ? 0xff : 0x00;
            while (l < last)
                *l++ = byte;

            mask = 0xff >> (7 - x2 % 8);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx)
                mask = 0xff << (7 - x2 % 8);
#endif
            if (pixel)
                *l |= mask;
            else
                *l &= ~mask;
        }
    }
}

/*!
    \fn void QGfxRaster<depth,type>::hImageLineUnclipped(int x1,int x2, unsigned char *l, unsigned const char *srcdata, bool reverse)

    \internal

    \a l points to the start of the destination line's data.
    \a x1 and \a x2 are the start and end pixels.
    \a srcdata points to the source's left pixel start byte if \a reverse is
    false. srcdata points to the source's right pixels's start byte if reverse
    is true. reverse will only be true if the source and destination are the same
    buffer and a mask is set.
    Image data comes from of the setSource calls (in which case the
    variable srcdata points to it) or as a solid value stored in srccol
    (if setSourcePen is used). This method is internal and called from blt and
    stretchBlt. Its complexity is caused by its need to deal with masks, copying
    from right to left or left to right (for overlapping blt's), packing writes
    for performance reasons and arbitrary source and destination depths.
*/
template <const int depth, const int type>
Q_GFX_INLINE void QGfxRaster<depth,type>::hImageLineUnclipped(int x1, int x2, unsigned char *l,
                                                            unsigned const char *srcdata,
                                                            bool reverse)
{
    int w = x2-x1+1;
    if (depth == 32) {
        unsigned int *myptr=(unsigned int *)l;
        int inc = 1;
        if(!reverse) {
            myptr+=x1;
        } else {
            myptr+=x2;
            inc = -1;
        }
        if (!ismasking) {
            uint gv = srccol;
            while (w--) {
                if (srctype==SourceImage)
                    gv = get_value_32(srcdepth,&srcdata);
                *(myptr++) = gv;
            }
        } else {
            //masked 32bpp blt...
            unsigned int gv = srccol;
            while (w--) {
                if (srctype == SourceImage)
                    gv = get_value_32(srcdepth, &srcdata, reverse);
                bool masked = true;
                GET_MASKED(reverse, w);
                if (!masked)
                    *(myptr) = gv;
                myptr += inc;
            }
        }
    } else if (depth == 24) {
        unsigned char *myptr = l;
        int inc = 3;
        if(!reverse) {
            myptr += x1*3;
        } else {
            myptr += x2*3;
            inc = -3;
        }
        if (!ismasking) {
            uint gv = srccol;
            while (w--) {
                if (srctype==SourceImage)
                    gv = get_value_24(srcdepth,&srcdata);
                gfxSetRgb24(myptr, gv);
                myptr += 3;
            }
        } else {
            //masked 32bpp blt...
            unsigned int gv = srccol;
            while (w--) {
                if (srctype == SourceImage)
                    gv = get_value_24(srcdepth, &srcdata, reverse);
                bool masked = true;
                GET_MASKED(reverse, w);
                if (!masked)
                    gfxSetRgb24(myptr, gv);
                myptr += inc;
            }
        }
    } else if (depth == 16) {
        unsigned short int *myptr=(unsigned short int *)l;
        int inc = 1;
        if(!reverse) {
            myptr+=x1;
        } else {
            myptr+=x2;
            inc = -1;
        }
        if(!ismasking) {
#ifdef QWS_NO_WRITE_PACKING
            while (w--) {
                *(myptr++)=get_value_16(srcdepth,&srcdata);
            }
#else
            // 32-bit writes
            int frontadd;
            int backadd;
            int count;

            calcPacking(myptr-x1,x1,x2,frontadd,backadd,count);

            PackType dput;
            while (frontadd--)
                *(myptr++)=get_value_16(srcdepth,&srcdata);
            PackType *myptr2 = (PackType*)myptr;
            myptr += count * 2;
            while (count--) {
# ifdef QT_QWS_REVERSE_BYTE_ENDIANNESS
                dput = (get_value_16(srcdepth,&srcdata) << 16);
                dput |= get_value_16(srcdepth,&srcdata);
# else
                dput = get_value_16(srcdepth,&srcdata);
                dput |= (get_value_16(srcdepth,&srcdata) << 16);
# endif
                *myptr2++ = dput;
            }
            while (backadd--)
                *(myptr++)=get_value_16(srcdepth,&srcdata);
#endif
        } else {
            // Probably not worth trying to pack writes if there's a mask
            unsigned short int gv = srccol;
            while (w--) {
                if (srctype==SourceImage)
                    gv = get_value_16(srcdepth, &srcdata, reverse);
                bool masked = true;
                GET_MASKED(reverse, w);
                if (!masked)
                    *(myptr) = gv;
                myptr += inc;
            }
        }
    } else if (depth == 8) {
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        // cursor code uses 8bpp backing store
        if (srcdepth == 4 && srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
        unsigned char *myptr=(unsigned char *)l;
        int inc = 1;
        if(!reverse) {
            myptr+=x1;
        } else {
            myptr+=x2;
            inc = -1;
        }
        if(!ismasking) {
#ifdef QWS_NO_WRITE_PACKING
          while (w--)
            *(myptr++)=get_value_8(srcdepth,&srcdata);
#else
            // 32-bit writes
            int frontadd;
            int backadd;
            int count;

            calcPacking(myptr-x1,x1,x2,frontadd,backadd,count);

            PackType dput;
            while (frontadd--)
                *(myptr++)=get_value_8(srcdepth,&srcdata);
            while (count--) {
#ifdef QT_QWS_REVERSE_BYTE_ENDIANNESS
                dput = (get_value_8(srcdepth,&srcdata) << 24);
                dput |= (get_value_8(srcdepth,&srcdata) << 16);
                dput |= (get_value_8(srcdepth,&srcdata) << 8);
                dput |= get_value_8(srcdepth,&srcdata);
#else
                dput = get_value_8(srcdepth,&srcdata);
                dput |= (get_value_8(srcdepth,&srcdata) << 8);
                dput |= (get_value_8(srcdepth,&srcdata) << 16);
                dput |= (get_value_8(srcdepth,&srcdata) << 24);
#endif
                *((PackType*)myptr) = dput;
                myptr += 4;
            }
            while (backadd--)
                *(myptr++)=get_value_8(srcdepth,&srcdata);
#endif
        } else {
            // Probably not worth trying to pack writes if there's a mask
            unsigned char gv = srccol;
            while (w--) {
                if (srctype==SourceImage)
                    gv = get_value_8(srcdepth, &srcdata, reverse);
                bool masked = true;
                GET_MASKED(reverse, w);
                if (!masked)
                    *(myptr) = gv;
                myptr += inc;
            }
        }
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //restore
        if (srcdepth == 4 && srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
    } else if (depth == 4) {
        unsigned char *dp = l;
        unsigned int gv = srccol;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //really ugly hack: screen is opposite endianness to everything else
        if (srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
        if (reverse) {
            dp += (x2/2);
            int x = x2;
            while (w--) {
                if (srctype==SourceImage)
                    gv = get_value_4(srcdepth, &srcdata, reverse);
                bool masked = true;
                if (ismasking) {
                    GET_MASKED(reverse, w);
                }
                if (!masked || !ismasking) {
                    int s = (x&1) << 2;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                    if (is_screen_gfx)
                        s = (~x&1) << 2;
#endif
                    *dp = (*dp & MASK4BPP(s)) | (gv << s);
                }
                if (!(x&1))
                    dp--;
                x--;
            }
        } else {
            dp += (x1/2);
            int x = x1;
            while (w--) {
                if (srctype==SourceImage)
                    gv = get_value_4(srcdepth, &srcdata, reverse);
                bool masked = true;
                if (ismasking) {
                    GET_MASKED(reverse, w);
                }
                if (!masked || !ismasking) {
                    int s = (x&1) << 2;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                    if (is_screen_gfx)
                        s = (~x&1) << 2;
#endif
                    *dp = (*dp & MASK4BPP(s)) | (gv << s);
                }
                if (x&1)
                    dp++;
                x++;
            }
        }
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //end of really ugly hack; undo the damage
        if (srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
    } else if (depth == 1) {
        // General case only implemented.
        // Lots of optimisation can be performed for special cases.
        unsigned char * dp=l;
        unsigned int gv = srccol;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //really ugly hack: screen is opposite endianness to everything else
        if (srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
        if (reverse) {
            dp+=(x2/8);
            int skipbits = 7 - (x2%8);
            for (int b = x1/8; b <= x2/8; b++) {
                unsigned char m = *dp;
                for (int i = 0; i < 8 && w; i++) {
                    if (skipbits)
                        skipbits--;
                    else {
                        w--;
                        if (srctype == SourceImage)
                            gv = get_value_1(srcdepth, &srcdata, true);
                        bool masked = true;
                        if (ismasking) {
                            GET_MASKED(true, w);
                        }
                        if (!masked || !ismasking) {
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                            if (is_screen_gfx) {
                                if (gv)
                                    m |= 0x01 << i;
                                else
                                    m &= ~(0x01 << i);
                            } else
#endif
                                if (gv)
                                    m |= 0x80 >> i;
                                else
                                    m &= ~(0x80 >> i);
                        }
                    }
                }
                *(dp--) = m;
            }
        } else {
            dp+=(x1/8);
            int skipbits = x1%8;
            for (int b = x1/8; b <= x2/8; b++) {
                unsigned char m = *dp;
                for (int i = 0; i < 8 && w; i++) {
                    if (skipbits)
                        skipbits--;
                    else {
                        w--;
                        if (srctype == SourceImage)
                            gv = get_value_1(srcdepth, &srcdata, false);
                        bool masked = true;
                        if (ismasking) {
                            GET_MASKED(false, w);
                        }
                        if (!masked || !ismasking) {
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                            if (is_screen_gfx) {
                                if (gv)
                                    m |= 0x80 >> i;
                                else
                                    m &= ~(0x80 >> i);
                            } else
#endif
                                if (gv)
                                    m |= 1 << i;
                                else
                                    m &= ~(1 << i);
                        }
                    }
                }
                *(dp++) = m;
            }
        }
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //end of really ugly hack; undo the damage
        if (srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
    }
}

/*!
    \fn void QGfxRaster::hAlphaLineUnclipped(int x1, int x2, unsigned char *l,
                                             unsigned const char *srcdata,
                                             unsigned const char *alphas)
    \internal

    This is similar to hImageLineUnclipped but handles the more complex
    alpha blending modes (InlineAlpha, SeparateAlpha, SolidAlpha).
    Blending is a simple averaging between the source and destination r, g and b
    values using the 8-bit source alpha value that is, for each of r, g and b the
    result is

    (source - destination * alpha) / 256 + destination

    Note that since blending requires some per-pixel computation and a read-write
    access on the destination it tends to be slower than the simpler alpha
    blending modes. \a x1 and \a x2 specify where to draw in the destination,
    \a l is the pointer to the scanline to draw into, \a srcdata is the source
    pixel data and \a alphas is the alpha channel for the SeparateAlpha mode.
*/
template <const int depth, const int type>
Q_GFX_INLINE void QGfxRaster<depth,type>::hAlphaLineUnclipped(int x1, int x2, unsigned char *l,
                                                            unsigned const char *srcdata,
                                                            unsigned const char *alphas)
{
    int w=x2-x1+1;
    if (depth == 32) {
        // First read in the destination line
        unsigned int *myptr = reinterpret_cast<unsigned int *>(l);
        unsigned int *alphaptr = reinterpret_cast<unsigned int *>(alphabuf);
        unsigned const char *avp = alphas;
        int loopc;

        unsigned int *temppos = myptr+x1;
        for (int i = 0; i < w; i++)
            *(alphaptr++) = *(temppos++);

        // Now blend with source data
        unsigned const char * srcptr=srcdata;
        unsigned int srcval;

        if(srctype==SourceImage) {
            srcptr=srcdata;
            srcval=0; // Shut up compiler
        } else {
            // SourcePen
            srcval=srccol;
        }

        alphaptr = reinterpret_cast<unsigned int *>(alphabuf);
        for(loopc=0;loopc<w;loopc++) {
            int a,r,g,b;
            if(srctype==SourceImage)
                srcval=get_value_32(srcdepth,&srcptr);

            int av;
            if(alphatype==InlineAlpha) {
                av = srcval >> 24;
            } else if(alphatype==SolidAlpha) {
                av=calpha;
            } else {
                av=*(avp++);
            }

            r = (srcval & 0xff0000) >> 16;
            g = (srcval & 0xff00) >> 8;
            b = srcval & 0xff;

            unsigned int hold = alphabuf[loopc];
            if(av==255) {
                a = av;
                // Do nothing - we already have source values in r,g,b
            } else if(av==0) {
                a = (hold >> 24) & 0xff;
                r = (hold >> 16) & 0xff;
                g = (hold >> 8) & 0xff;
                b = hold & 0xff;
            } else {
                int tmp = (hold >> 24) & 0xff;
                a = av + tmp - av*tmp/255;
                tmp = (hold >> 16) & 0xff;
                r = ((r-tmp) * av) / 256 + tmp;
                tmp = (hold >> 8) & 0xff;
                g = ((g-tmp) * av) / 256 + tmp;
                tmp = hold & 0xff;
                b = ((b-tmp) * av) / 256 + tmp;
            }
            *(alphaptr++) = (a<<24) | (r << 16) | (g << 8) | b;
        }

        // Now write it all out
        alphaptr = reinterpret_cast<unsigned int *>(alphabuf);

        myptr += x1;
        while (w--)
            *(myptr++)=*(alphaptr++);
    } else if (depth == 24) {
        // First read in the destination line
        unsigned char *myptr = l;
        unsigned char *alphaptr = reinterpret_cast<unsigned char *>(alphabuf);
        unsigned const char *avp = alphas;
        int loopc;

        memcpy(alphabuf, myptr+x1*3, w*3);

        // Now blend with source data
        unsigned const char * srcptr=srcdata;
        unsigned int srcval;

        if(srctype==SourceImage) {
            srcptr=srcdata;
            srcval=0; // Shut up compiler
        } else {
            // SourcePen
            srcval=srccol;
        }

        alphaptr = reinterpret_cast<unsigned char *>(alphabuf);
        for(loopc=0;loopc<w;loopc++) {
            int r,g,b;
            if(srctype==SourceImage)
                srcval=get_value_32(srcdepth,&srcptr);

            int av;
            if(alphatype==InlineAlpha) {
                av = srcval >> 24;
            } else if(alphatype==SolidAlpha) {
                av=calpha;
            } else {
                av=*(avp++);
            }

            r = (srcval & 0xff0000) >> 16;
            g = (srcval & 0xff00) >> 8;
            b = srcval & 0xff;

            unsigned char *tmp = alphaptr;
            if (av == 255) {
                // Do nothing - we already have source values in r,g,b
            } else if (av == 0) {
                r = *(tmp+2);
                g = *(tmp+1);
                b = *(tmp+0);
            } else {
                r = ((r-*(tmp+2)) * av) / 256 + *(tmp+2);
                g = ((g-*(tmp+1)) * av) / 256 + *(tmp+1);
                b = ((b-*(tmp+0)) * av) / 256 + *(tmp+0);
            }
            gfxSetRgb24(alphaptr, r, g, b);
            alphaptr += 3;
        }

        // Now write it all out
        memcpy(myptr+x1*3, alphabuf, w*3);
#if !defined(QT_NO_IMAGE_16_BIT) || !defined(QT_NO_QWS_DEPTH_16)
    } else if (depth == 16) {
        // First read in the destination line
        unsigned short int *myptr = reinterpret_cast<unsigned short int *>(l);
        unsigned int *alphaptr = reinterpret_cast<unsigned int *>(alphabuf);
        int loopc;

#ifdef QWS_NO_WRITE_PACKING
        unsigned short int *temppos = myptr + x1;
        for (int i = 0; i < w; i++)
            *(alphaptr++) = get_value_32(16,reinterpret_cast<unsigned char **>(&temppos));
#else
        int frontadd, backadd, count;
        calcPacking(myptr,x1,x2,frontadd,backadd,count);
        myptr+=x1;

        unsigned const short int *temppos = myptr;

        int loopc2;
        for(loopc2=0;loopc2<frontadd;loopc2++)
            *(alphaptr++)=get_value_32(16,reinterpret_cast<unsigned const char **>(&temppos));

        volatile PackType temp2;
        volatile unsigned short int *cp;
        for(loopc2=0;loopc2<count;loopc2++) {
            temp2=*reinterpret_cast<const PackType *>(temppos);
            cp=reinterpret_cast<volatile unsigned short int*>(&temp2);
            *(alphaptr++)=qt_conv16ToRgb(*cp);
            cp++;
            *(alphaptr++)=qt_conv16ToRgb(*cp);
            temppos += 2;
        }

        for(loopc2=0;loopc2<backadd;loopc2++)
            *(alphaptr++)=get_value_32(16, reinterpret_cast<unsigned const char **>(&temppos));
#endif

        // Now blend with source data
        unsigned const char *srcptr=srcdata;
        unsigned int srcval;

        if(srctype==SourceImage) {
            srcptr=srcdata;
            srcval=0; // Shut up compiler
        } else {
            // SourcePen
            srcval=qt_conv16ToRgb(srccol);
        }

        int astype = 3;
        if (srctype==SourceImage && alphatype==InlineAlpha)
            astype = 0;
        else if (srctype==SourceImage && alphatype != SolidAlpha)
            astype = 1;
        else if (alphatype != SolidAlpha)
            astype = 2;

        int av = calpha;
        alphaptr = (unsigned int *)alphabuf;
        unsigned const char *avp = alphas;
        for (loopc=0;loopc<w;loopc++) {
            switch (astype) {
                case 0:
                    srcval=get_value_32(srcdepth,&srcptr);
                    av = srcval >> 24;
                    break;
                case 1:
                    srcval=get_value_32(srcdepth,&srcptr);
                    // FALLTHROUGH
                case 2:
                    av=*(avp++);
                    break;
            }

            int r,g,b;
            if (av == 255) {
                unsigned const char *stmp = reinterpret_cast<unsigned const char *>(&srcval);
#ifdef QT_QWS_REVERSE_BYTE_ENDIANNESS
                stmp++;
                r = *stmp++;
                g = *stmp++;
                b = *stmp;
#else
                b = *stmp++;
                g = *stmp++;
                r = *stmp;
#endif
            } else {
                unsigned const char *tmp=reinterpret_cast<unsigned const char *>(alphaptr);
#ifdef QT_QWS_REVERSE_BYTE_ENDIANNESS
                tmp++;
                r = *tmp++;
                g = *tmp++;
                b = *tmp;
#else
                b = *tmp++;
                g = *tmp++;
                r = *tmp;
#endif
                if (av) {
                    unsigned const char *stmp = reinterpret_cast<unsigned const char *>(&srcval);
#ifdef QT_QWS_REVERSE_BYTE_ENDIANNESS
                    stmp++;
                    r += ((*stmp++-r) * av) >> 8;
                    g += (((*stmp++)-g) * av) >> 8;
                    b += (((*stmp)-b) * av) >> 8;
#else
                    b += (((*stmp++)-b) * av) >> 8;
                    g += (((*stmp++)-g) * av) >> 8;
                    r += ((*stmp-r) * av) >> 8;
#endif
                }
            }
            *(alphaptr++) = qt_convRgbTo16(r,g,b);
        }

        // Now write it all out
        alphaptr = reinterpret_cast<unsigned int *>(alphabuf);

        // #### Consider having different pointer types to alphabuf, such that
        // #### the written 16bits are aligned and we may use memcpy instead
        // #### the hackish while loop/DUFF_WRITE_WORD below. Should be faster.
        // #### (Packing wouldn't be an issue either)
#ifdef QWS_NO_WRITE_PACKING
        myptr += x1;
        while (w--)
            *(myptr++)=*(alphaptr++);
#else
        myptr=reinterpret_cast<unsigned short int *>(l);
        calcPacking(myptr,x1,x2,frontadd,backadd,count);
        myptr+=x1;

        for (loopc2=0;loopc2<frontadd;loopc2++)
            *(myptr++)=*(alphaptr++);

        PackType put;
        // Duffs device.
#ifdef QT_QWS_REVERSE_BYTE_ENDIANNESS
        #define DUFF_WRITE_WORD put=(*(alphaptr++) << 16); put|=(*alphaptr++); \
                            *myptr2++ = put;
#else
        #define DUFF_WRITE_WORD put=*(alphaptr++); put|=(*(alphaptr++) << 16); \
                            *myptr2++ = put;
#endif
        PackType *myptr2 = reinterpret_cast<PackType*>(myptr);
        myptr += count * 2;
        PackType *end2 = reinterpret_cast<PackType*>(myptr);
        switch(count%8){
            case 0:
                while (myptr2 != end2) {
                    DUFF_WRITE_WORD
            case 7: DUFF_WRITE_WORD
            case 6: DUFF_WRITE_WORD
            case 5: DUFF_WRITE_WORD
            case 4: DUFF_WRITE_WORD
            case 3: DUFF_WRITE_WORD
            case 2: DUFF_WRITE_WORD
            case 1: DUFF_WRITE_WORD
                }
        }
        #undef DUFF_WRITE_WORD

        for (loopc2=0;loopc2<backadd;loopc2++)
            *(myptr++)=*(alphaptr++);
#endif
#endif
    } else if (depth == 8) {
        // First read in the destination line
        unsigned char * myptr;
        myptr=l;
        myptr+=x1;
        int loopc;

        unsigned const char * avp=alphas;

        unsigned char * tempptr=myptr;

        for(loopc=0;loopc<w;loopc++) {
            int val = *tempptr++;
            alphabuf[loopc] = clut[val];
        }

        // Now blend with source data

        unsigned const char * srcptr;
        unsigned int srcval = 0;
        if(srctype==SourceImage) {
            srcptr=srcdata;
        } else {
            // SourcePen
            QRgb mytmp=clut[srccol];
            srcval=qRed(mytmp) << 16 | qGreen(mytmp) << 8 | qBlue(mytmp);
        }
        simple_8bpp_alloc=true;
        for(loopc=0;loopc<w;loopc++) {
            int r,g,b;
            if(srctype==SourceImage)
                srcval=get_value_32(srcdepth,&srcptr);

            int av;
            if(alphatype==InlineAlpha) {
                av=srcval >> 24;
            } else if(alphatype==SolidAlpha) {
                av=calpha;
            } else {
                av=*(avp++);
            }

            r = (srcval & 0xff0000) >> 16;
            g = (srcval & 0xff00) >> 8;
            b = srcval & 0xff;

            unsigned char * tmp=reinterpret_cast<unsigned char *>(&alphabuf[loopc]);
            if(av==255) {
                *myptr = GFX_CLOSEST_PIXEL(r,g,b);
            } else if (av > 0) {
                r = ((r-*(tmp+2)) * av) / 256 + *(tmp+2);
                g = ((g-*(tmp+1)) * av) / 256 + *(tmp+1);
                b = ((b-*(tmp+0)) * av) / 256 + *(tmp+0);
                *myptr = GFX_CLOSEST_PIXEL(r,g,b);
            }
            myptr++;
        }
        simple_8bpp_alloc=false;
    } else if (depth == 4) {
        // First read in the destination line
        unsigned char *myptr = l;
        myptr+=x1/2;

        unsigned const char *avp=alphas;
        unsigned char *tempptr=myptr;

        int loopc = 0;
        if (x1&1) {
            int val = *tempptr++;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx)
                alphabuf[loopc++] = clut[(val & 0x0f)];
            else
#endif
            alphabuf[loopc++] = clut[(val & 0xf0) >> 4];
        }

        for (;loopc < w-1; loopc += 2) {
            int val = *tempptr++;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx) {
                alphabuf[loopc] = clut[val >> 4];
                alphabuf[loopc+1] = clut[val & 0x0f];
            } else
#endif
            {
            alphabuf[loopc] = clut[val & 0x0f];
            alphabuf[loopc+1] = clut[val >> 4];
            }
        }

        if (!(x2&1)) {
            int val = *tempptr;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx)
                alphabuf[w-1] = clut[(val & 0xf0) >> 4];
            else
#endif
            alphabuf[w-1] = clut[val & 0x0f];
        }

        // Now blend with source data

        unsigned const char * srcptr;
        unsigned int srcval = 0;
        if(srctype==SourceImage) {
            srcptr=srcdata;
        } else {
            // SourcePen
            QRgb mytmp=clut[srccol];
            srcval=qRed(mytmp) << 16 | qGreen(mytmp) << 8 | qBlue(mytmp);
        }
        for(loopc=0;loopc<w;loopc++) {
            int r,g,b;
            if(srctype==SourceImage)
                srcval=get_value_32(srcdepth,&srcptr);

            int av;
            if(alphatype==InlineAlpha) {
                av=srcval >> 24;
            } else if(alphatype==SolidAlpha) {
                av=calpha;
            } else {
                av=*(avp++);
            }

            r = (srcval & 0xff0000) >> 16;
            g = (srcval & 0xff00) >> 8;
            b = srcval & 0xff;

            unsigned char *tmp = reinterpret_cast<unsigned char *>(&alphabuf[loopc]);

            if(av==255) {
                alphabuf[loopc] = qRgb(r,g,b);
            } else {
                r = ((r-*(tmp+2)) * av) / 256 + *(tmp+2);
                g = ((g-*(tmp+1)) * av) / 256 + *(tmp+1);
                b = ((b-*(tmp+0)) * av) / 256 + *(tmp+0);
                alphabuf[loopc] = qRgb(r,g,b);
            }
        }

        loopc = 0;
        if (x1&1) {
            QRgb rgb = alphabuf[loopc++];
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx)
                *myptr++ = (*myptr & 0xf0) |
                (gfx_screen->alloc(qRed(rgb), qGreen(rgb), qBlue(rgb)) & 0x0f);
            else
#endif
            *myptr++ = (*myptr & 0x0f) |
                (gfx_screen->alloc(qRed(rgb), qGreen(rgb), qBlue(rgb)) << 4);
        }

        for (;loopc < w-1; loopc += 2) {
            QRgb rgb1 = alphabuf[loopc];
            QRgb rgb2 = alphabuf[loopc+1];
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx) {
                rgb2 = alphabuf[loopc];
                rgb1 = alphabuf[loopc+1];
            }
#endif
            *myptr++ = gfx_screen->alloc(qRed(rgb1), qGreen(rgb1), qBlue(rgb1)) |
                (gfx_screen->alloc(qRed(rgb2), qGreen(rgb2), qBlue(rgb2)) << 4);
        }

        if (!(x2&1)) {
            QRgb rgb = alphabuf[w-1];
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
            if (is_screen_gfx)
                *myptr++ = (*myptr & 0x0f) |
                     (gfx_screen->alloc(qRed(rgb), qGreen(rgb), qBlue(rgb)) << 4);
            else
#endif
            *myptr = (*myptr & 0xf0) |
                gfx_screen->alloc(qRed(rgb), qGreen(rgb), qBlue(rgb));
        }

    } else if (depth == 1) {
        if (srctype==SourceImage) {
            static int warn;
            if (warn++ < 5)
                qDebug("bitmap alpha-image not implemented");
            hImageLineUnclipped(x1, x2, l, srcdata, false);
        } else {
            bool black = qGray(clut[srccol]) < 128;
            for (int x=x1; x<=x2; x++) {
                if (*alphas++ >= 64) { // ### could be configurable (monoContrast)
                    uchar* lx = l+(x>>3);
                    uchar b = 1<<(x&0x7);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                    if (is_screen_gfx)
                        b = 0x80>>(x&0x7);
#endif
                    if (!(*lx&b) != black)
                        *lx ^= b;
                }
            }
        }
    }
}

/*!
    \fn void QGfxRaster<depth,type>::fillRect(int rx, int ry, int w, int h)

    \internal

    Draw a filled rectangle in the current brush color from \a rx,\a ry to \a w,
    \a h.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::fillRect(int rx,int ry,int w,int h) //widget coordinates
{
    if (!ncliprect)
        return;
    GFX_START(QRect(rx+xoffs, ry+yoffs, w+1, h+1))

    if((*gfx_optype))
        sync();
    (*gfx_optype)=0;
    setAlphaType(IgnoreAlpha);
    if (w <= 0 || h <= 0) {
        GFX_END
        return;
    }

#ifdef QWS_EXPERIMENTAL_FASTPATH
    // ### fix for 8bpp
    // This seems to be reliable now, at least for 16bpp

    bool useAlpha = (qAlpha(brushColor) != 255);
    if (ncliprect == 1 && cbrush.style()==Qt::SolidPattern && !useAlpha) {
        // Fast path
        if(depth==16) {
            pixel = brushPixel;
            int x1,y1,x2,y2;
            rx+=xoffs;
            ry+=yoffs;
            x2=rx+w-1;
            y2=ry+h-1;
            if(rx>cliprect[0].right() || ry>cliprect[0].bottom() ||
               x2<cliprect[0].left() || y2<cliprect[0].top()) {
                GFX_END
                return;
            }
            x1=cliprect[0].left() > rx ? cliprect[0].left() : rx;
            y1=cliprect[0].top() > ry ? cliprect[0].top() : ry;
            x2=cliprect[0].right() > x2 ? x2 : cliprect[0].right();
            y2=cliprect[0].bottom() > y2 ? y2 : cliprect[0].bottom();
            w=(x2-x1)+1;
            h=(y2-y1)+1;

            if(w<1 || h<1) {
                GFX_END
                return;
            }

            unsigned short int * myptr=(unsigned short int *)scanLine(y1);

        // 64-bit writes make a /big/ difference from 32-bit ones,
        // at least on my (fast AGP) hardware - 856 rects/second as opposed
        // to 550, although MTRR makes this difference much less
            int frontadd;
            int backadd;
            int count;
            calcPacking(myptr,x1,x2,frontadd,backadd,count);

            int loopc,loopc2;

            PackType put;
            put = pixel | (pixel << 16);
            int add=linestep()/2;
            add-=(frontadd+(count * 2)+backadd);

            myptr=((unsigned short int *)scanLine(y1))+x1;
            for(loopc=0;loopc<h;loopc++) {
                for(loopc2=0;loopc2<frontadd;loopc2++)
                    *(myptr++)=pixel;
                // Duffs device.
                PackType *myptr2 = (PackType*)myptr;
                myptr += count * 2;
                PackType *end2 = (PackType*)myptr;
                switch(count%8) {
                case 0:
                    while (myptr2 != end2) {
                        *myptr2++ = put;
                case 7: *myptr2++ = put;
                case 6: *myptr2++ = put;
                case 5: *myptr2++ = put;
                case 4: *myptr2++ = put;
                case 3: *myptr2++ = put;
                case 2: *myptr2++ = put;
                case 1: *myptr2++ = put;
                                           }
                }
                for(loopc2=0;loopc2<backadd;loopc2++)
                    *(myptr++)=pixel;
                myptr+=add;
            }
            GFX_END
            return;
        } else if(depth==32) {
            pixel = brushPixel;
            int x1,y1,x2,y2;
            rx+=xoffs;
            ry+=yoffs;
            x2=rx+w-1;
            y2=ry+h-1;
            if(rx>cliprect[0].right() || ry>cliprect[0].bottom() ||
               x2<cliprect[0].left() || y2<cliprect[0].top()) {
                GFX_END
                return;
            }
            x1=cliprect[0].left() > rx ? cliprect[0].left() : rx;
            y1=cliprect[0].top() > ry ? cliprect[0].top() : ry;
            x2=cliprect[0].right() > x2 ? x2 : cliprect[0].right();
            y2=cliprect[0].bottom() > y2 ? y2 : cliprect[0].bottom();
            w=(x2-x1)+1;
            h=(y2-y1)+1;

            if(w<1 || h<1) {
                GFX_END
                return;
            }

            unsigned int * myptr=(unsigned int *)scanLine(y1);

            int frontadd;
            int backadd;
            int count;
            calcPacking(myptr,x1,x2,frontadd,backadd,count);

            int loopc,loopc2;
            PackType put;
            unsigned int * sp=(unsigned int *)&put;
            *sp=pixel;
            int add=linestep()/4;
            add-=(frontadd+(count * 2)+backadd);

            myptr=((unsigned int *)scanLine(y1))+x1;

            for(loopc=0;loopc<h;loopc++) {
                for(loopc2=0;loopc2<frontadd;loopc2++)
                    *(myptr++)=pixel;
                for(loopc2=0;loopc2<count;loopc2++) {
                    *((PackType *)myptr)=put;
                    myptr+=2;
                }
                for(loopc2=0;loopc2<backadd;loopc2++)
                    *(myptr++)=pixel;
                myptr+=add;
            }
            GFX_END
            return;
        } else if(depth==8) {
            pixel = brushPixel;
            int x1,y1,x2,y2;
            rx+=xoffs;
            ry+=yoffs;
            x2=rx+w-1;
            y2=ry+h-1;
            if(rx>cliprect[0].right() || ry>cliprect[0].bottom() ||
               x2<cliprect[0].left() || y2<cliprect[0].top()) {
                GFX_END
                return;
            }
            x1=cliprect[0].left() > rx ? cliprect[0].left() : rx;
            y1=cliprect[0].top() > ry ? cliprect[0].top() : ry;
            x2=cliprect[0].right() > x2 ? x2 : cliprect[0].right();
            y2=cliprect[0].bottom() > y2 ? y2 : cliprect[0].bottom();
            w=(x2-x1)+1;
            h=(y2-y1)+1;

            if(w<1 || h<1) {
                GFX_END
                return;
            }

            unsigned char * myptr=(unsigned char *)scanLine(y1);

            int frontadd;
            int backadd;
            int count;
            calcPacking(myptr,x1,x2,frontadd,backadd,count);

            int loopc,loopc2;
            PackType put;
            if (count)
                put = pixel | (pixel<<8) | (pixel<<16) | (pixel<<24);

            int add=linestep();
            add-=(frontadd+(count * 4)+backadd);

            myptr=((unsigned char *)scanLine(y1))+x1;

            for(loopc=0;loopc<h;loopc++) {
                for(loopc2=0;loopc2<frontadd;loopc2++)
                    *(myptr++)=pixel;
                for(loopc2=0;loopc2<count;loopc2++) {
                    *((PackType *)myptr)=put;
                    myptr+=4;
                }
                for(loopc2=0;loopc2<backadd;loopc2++)
                    *(myptr++)=pixel;
                myptr+=add;
            }
            GFX_END
            return;
        } else {

        }
    }
#endif // QWS_EXPERIMENTAL_FASTPATH
    if((cbrush.style()!=Qt::NoBrush) &&
        (cbrush.style()!=Qt::SolidPattern)) {
        srcwidth=cbrushpixmap.width();
        srcheight=cbrushpixmap.height();
        if(cbrushpixmap.depth()==1) {
            if(opaque) {
                setSource(&cbrushpixmap);
                setAlphaType(IgnoreAlpha);
                pixel = brushPixel;
                srcclut[1]=pixel;
                transclut[1]=pixel;
                QBrush tmp=cbrush;
                cbrush=QBrush(backcolor);
                pixel = brushPixel;
                srcclut[0]=pixel;
                transclut[0]=pixel;
                cbrush=tmp;
            } else {
                pixel = brushPixel;
                srccol=pixel;
                srctype=SourcePen;
                setAlphaType(LittleEndianMask);
                setAlphaSource(const_cast<uchar*>(cbrushpixmap.qwsScanLine(0)),
                               cbrushpixmap.qwsBytesPerLine());
            }
        } else {
            setSource(&cbrushpixmap);
            setAlphaType(IgnoreAlpha);
        }
        tiledBlt(rx,ry,w,h);
    } else if(cbrush.style()!=Qt::NoBrush) {
        pixel = brushPixel;
        if (useAlpha) {
            pixel = brushPixel;
            srccol = brushPixel;
            srctype = SourcePen;
            int alphaValue = qAlpha(brushColor);
            setAlphaSource(alphaValue);
            setAlphaType(alphaValue == 255 ? IgnoreAlpha : SolidAlpha);
        }
        rx += xoffs;
        ry += yoffs;
        // Gross clip
        if (rx < clipbounds.left()) {
            w -= clipbounds.left()-rx;
            rx = clipbounds.left();
        }
        if (ry < clipbounds.top()) {
            h -= clipbounds.top()-ry;
            ry = clipbounds.top();
        }
        if (rx+w-1 > clipbounds.right())
            w = clipbounds.right()-rx+1;
        if (ry+h-1 > clipbounds.bottom())
            h = clipbounds.bottom()-ry+1;
        if (w > 0 && h > 0)
            for (int j=0; j<h; j++,ry++) {
                hline(rx,rx+w-1,ry); }
    }
    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::drawPolyline(const QPolygon &a,int index, int npoints)

    \internal

    Draw a series of lines specified by \a npoints coordinates from array \a a,
    starting from \a index in the array.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::drawPolyline(const QPolygon &a,int index, int npoints)
{
    if (!ncliprect)
        return;
    if(cpen.style()==Qt::NoPen)
        return;
    if (cpen.width() > 1) {
        drawThickPolyline(a, index, npoints);
        return;
    }

#ifndef QT_NO_QWS_CURSOR
    GFX_START(a.boundingRect())
#else
    GFX_START(clipbounds)
#endif

    if((*gfx_optype))
        sync();
    (*gfx_optype)=0;
    //int m=qMin(index+npoints-1, int(a.size())-1);

    int loopc;
    int end;
    end=(index+npoints) > (int)a.size() ? a.size() : index+npoints;
#ifdef GFX_CORRECT_POLYLINE_JOIN
    if (myrop != CopyROP && npoints > 1) {
        gfx_storedLineBufferSize = qMax(clipbounds.height(),clipbounds.width());
        gfx_storedLineBufferSize = qMax(gfx_storedLineBufferSize,10);
        gfx_storedLineRd = new QPoint [gfx_storedLineBufferSize];
        gfx_storedLineWr = new QPoint [gfx_storedLineBufferSize];
        gfx_storeLine = true;
        gfx_storedLineWrite = 0;
        gfx_storedLineRead = 0;
        gfx_noLineOverwrite = false;
        if (a[index] == a[end-1]) {
            // initialize rd buffer
            gfx_doDraw = false;
            int x1 = a[end-2].x();
            int y1 = a[end-2].y();
            int x2 = a[end-1].x();
            int y2 = a[end-1].y();
            QRect cr = clipbounds;
            cr.moveBy(-xoffs, -yoffs);
            qt_clipLine(x1, y1, x2, y2, cr);
            drawLine(x1, y1, x2, y2);
            gfx_storedLineDir = x1 > x2 ? 1 : -1;
            gfx_storedLineRead = gfx_storedLineDir > 0 ? 0 : gfx_storedLineWrite - 1;
            gfx_noLineOverwrite = true;
            QPoint *tmp = gfx_storedLineWr;
            gfx_storedLineWr = gfx_storedLineRd;
            gfx_storedLineRd = tmp;
            gfx_doDraw = true;
        }
    }
    for(loopc=index+1;loopc<end;loopc++) {
        int x1 = a[loopc-1].x();
        int y1 = a[loopc-1].y();
        int x2 = a[loopc].x();
        int y2 = a[loopc].y();
        if (gfx_storeLine) {
            QRect cr = clipbounds;
            cr.moveBy(-xoffs, -yoffs);
            qt_clipLine(x1, y1, x2, y2, cr);
            gfx_storedLineWrite = 0;
        }
        drawLine(x1, y1, x2, y2);
        if (gfx_storeLine) {
            gfx_storedLineDir = x1 > x2 ? 1 : -1;
            gfx_storedLineRead = gfx_storedLineDir > 0 ? 0 : gfx_storedLineWrite - 1;
            gfx_noLineOverwrite = true;
            QPoint *tmp = gfx_storedLineWr;
            gfx_storedLineWr = gfx_storedLineRd;
            gfx_storedLineRd = tmp;
        }
    }
    if (gfx_storedLineRd)
        delete [] gfx_storedLineRd;
    if (gfx_storedLineWr)
        delete [] gfx_storedLineWr;
    gfx_storedLineRd = 0;
    gfx_storedLineWr = 0;
    gfx_storeLine = false;
#else
    for(loopc=index+1;loopc<end;loopc++) {
        drawLine(a[loopc-1].x(),a[loopc-1].y(),
                 a[loopc].x(),a[loopc].y());
    }
#endif
    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::drawPolygon(const QPolygon &pa, bool winding, int index, int npoints)

    \internal

    Draw a filled polygon in the current brush style, with a border in the current
    pen style. The polygon is specified in array \a pa by \a npoints points from
    \a index. \a winding specifies whether to use the winding fill algorithm
    or the even-odd (alternative) fill algorithm.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::drawPolygon(const QPolygon &pa, bool winding, int index,
                                         int npoints)
{
    if (!ncliprect)
        return;
    pixel = brushPixel;
    GFX_START(clipbounds)
    if((*gfx_optype)!=0) {
        sync();
    }
    (*gfx_optype)=0;
    if (cbrush.style()!=Qt::NoBrush) {
        if (cbrush.style()!=Qt::SolidPattern) {
            srcwidth=cbrushpixmap.width();
            srcheight=cbrushpixmap.height();
            if(cbrushpixmap.depth()==1) {
                if(opaque) {
                    setSource(&cbrushpixmap);
                    setAlphaType(IgnoreAlpha);
                    pixel = brushPixel;
                    srcclut[1]=pixel;
                    transclut[1]=pixel;
                    QBrush tmp=cbrush;
                    cbrush=QBrush(backcolor);
                    pixel = brushPixel;
                    srcclut[0]=pixel;
                    transclut[0]=pixel;
                    cbrush=tmp;
                } else {
                    pixel = brushPixel;
                    srccol=pixel;
                    srctype=SourcePen;
                    setAlphaType(LittleEndianMask);
                    setAlphaSource(const_cast<uchar*>(cbrushpixmap.qwsScanLine(0)),
                                   cbrushpixmap.qwsBytesPerLine());
                }
            } else {
                setSource(&cbrushpixmap);
                setAlphaType(IgnoreAlpha);
            }
        } else { // SolidPattern
            pixel = brushPixel;
            srccol = pixel;
            srctype = SourcePen;
            int alphaValue = qAlpha(brushColor);
            setAlphaSource(alphaValue);
            setAlphaType(alphaValue == 255 ? IgnoreAlpha : SolidAlpha);
        }
        scan(pa,winding,index,npoints,stitchedges);
    }

    pixel = penPixel;
    setSourcePen();
    int alphaValue = qAlpha(penColor);
    setAlphaSource(alphaValue);
    setAlphaType(alphaValue == 255 ? IgnoreAlpha : SolidAlpha);

    drawPolyline(pa, index, npoints);
    if (pa[index] != pa[index+npoints-1]) {
        drawLine(pa[index].x(), pa[index].y(),
                pa[index+npoints-1].x(),pa[index+npoints-1].y());
    }
    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::processSpans(int n, QPoint *point, int *width)

    \internal

    This is used internally by drawPolygon (via scan()) to draw the individual
    scanlines of a polygon by calling hline. \a point is an array of points
    describing the horizontal lines in this scanline of the polygon,
    \a n the array of points to draw, \a width the width of the scanline.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::processSpans(int n, QPoint *point, int *width) // widget coords
{
    while (n--) {
        if (*width > 0) {
            if (patternedbrush && srcwidth != 0 && srcheight != 0) {
                unsigned char * savealphabits=alphabits;
                int offx = srcwidgetoffs.x() + point->x() - brushorig.x() ;
                int offy = srcwidgetoffs.y() + point->y() - brushorig.y();

                // from qpainter_qws.cpp
                if (offx < 0)
                    offx = srcwidth - -offx % srcwidth;
                else
                    offx = offx % srcwidth;
                if (offy < 0)
                    offy = srcheight - -offy % srcheight;
                else
                    offy = offy % srcheight;

                int rx = point->x();
                int w = *width;
                int xPos = rx;
                while (xPos < rx + w - 1) {
                    int drawW = srcwidth - offx; // Cropping first column
                    if (xPos + drawW > rx + w)    // Cropping last column
                        drawW = rx + w - xPos;
                    blt(xPos, point->y(), drawW, 1, offx, offy);
                    alphabits=savealphabits;
                    xPos += drawW;
                    offx = 0;
                }
            } else {
                int x=point->x()+xoffs;
                hline(x,x+*width-1,point->y()+yoffs); // ######## Make it clip by itself!
            }
        }
        point++;
        width++;
    }
}


/*!
    \fn void QGfxRaster<depth,type>::scroll(int rx, int ry, int w, int h, int sx, int sy)

    \internal

    This is intended for hardware optimisation - it handles the common case
    of blting a rectangle a small distance within the same drawing surface
    (for example when scrolling a listbox). \a rx and \a ry are the X and Y
    coordinates to which the rectangle should be moved, \a sx and \a sy
    are its source coordinates and \a w and \a h are its width and height.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::scroll(int rx,int ry,int w,int h,int sx, int sy)
{
    if (!w || !h || !ncliprect)
        return;

    int dy = sy - ry;
    int dx = sx - rx;

    if (dx == 0 && dy == 0)
        return;

    GFX_START(QRect(qMin(rx+xoffs,sx+xoffs), qMin(ry+yoffs,sy+yoffs), w+qAbs(dx)+1, h+qAbs(dy)+1))

    srcbits=buffer;
    srclinestep=linestep();
    srcdepth=depth;
    srcwidth=w;
    srcheight=h;
    if(srcdepth==0)
        abort();
    srctype=SourceImage;
    setAlphaType(IgnoreAlpha);
    setSourceWidgetOffset(xoffs, yoffs);
    src_normal_palette = true;
    blt(rx,ry,w,h,sx,sy);

    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::blt(int rx, int ry, int w, int h, int sx, int sy)

    \internal

    This corresponds to QPixmap::drawPixmap (into a QPainter with no transformation
    other than a translation) or bitBlt. The source is set up using
    setSource and setSourceWidgetOffset before the blt. \a rx and \a ry are the
    destination coordinates, \a w and \a h the size of the rectangle to blt,
    \a sx and \a sy the source coordinates relative to the source's widget offset.
    In the case of a pen source sx and sy are ignored. Source and destination
    can overlap and can be of arbitrary (different) depths.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::blt(int rx,int ry,int w,int h, int sx, int sy)
{
    if (!w || !h) return;
    int osrcdepth=srcdepth;
    if(srctype==SourcePen) {
        srclinestep=0;//w;
        srcdepth=0;
        pixel = penPixel;
    }

    rx += xoffs;
    ry += yoffs;

    QRect cursRect(rx, ry, w+1, h+1);
    GFX_START(cursRect&clipbounds);

    // Very gross clip
    if (!clipbounds.intersects(QRect(rx,ry,w,h))) {
        GFX_END
        return;
    }

    //slightly tighter clip
    int leftd = clipbounds.x() - rx;
    if (leftd > 0) {
        rx += leftd;
        sx += leftd;
        w -= leftd;
    }
    int topd =  clipbounds.y() - ry;
    if (topd > 0) {
        ry += topd;
        sy += topd;
        h -= topd;
    }
    int rightd = rx + w - 1 - clipbounds.right();
    if (rightd > 0)
        w -= rightd;
    int botd = ry + h - 1 - clipbounds.bottom();
    if (botd > 0)
        h -= botd;

    // have we already clipped away everything necessary
    bool mustclip = ncliprect != 1;

    if((*gfx_optype)!=0)
        sync();
    (*gfx_optype)=0;

    QPoint srcoffs = srcwidgetoffs + QPoint(sx, sy);

    int dl = linestep();
    int sl = srclinestep;
    int dj = 1;
    int dry = 1;
    int tj;
    int j;
    if (srcbits == buffer && srcoffs.y() < ry) {
        // Back-to-front
        dj = -dj;
        dl = -dl;
        sl = -sl;
        dry = -dry;
        j = h-1;
        ry=(ry+h)-1;
        tj = -1;
    } else {
        j = 0;
        tj = h;
    }

    bool xrev = (srcbits == buffer && srcoffs.x() < rx);

    QRect cr(rx, ry, w, h);

    unsigned char *l = scanLine(ry);
    unsigned const char *srcline = srcScanLine(j+srcoffs.y());
    int right = rx+w-1;

    // Fast path for 8/16/32 bit same-depth opaque blit. (ie. the common case)
    if (srcdepth == depth && alphatype == IgnoreAlpha
        && pixeltype == srcpixeltype
        && (depth > 8 || (depth == 8 && src_normal_palette))) {
        int bytesPerPixel = depth/8;
        if (mustclip) {
            if (xrev) {
                for (; j!=tj; j+=dj,ry+=dry,l+=dl,srcline+=sl) {
                    bool plot = inClip(right,ry,&cr);
                    int x2=right;
                    for (;;) {
                        int x = cr.left();
                        if (x < rx) {
                            x = rx;
                            if (x2 < x) break;
                        }
                        if (plot) {
                            unsigned const char *srcptr=srcline+(x-rx+srcoffs.x())*bytesPerPixel;
                            unsigned char *destptr = l + x*bytesPerPixel;
                            memmove(destptr, srcptr, (x2-x+1) * bytesPerPixel);
                        }
                        if (x <= rx)
                            break;
                        x2=x-1;
                        plot=inClip(x2,ry,&cr,plot);
                    }
                }
            } else {
                for (; j!=tj; j+=dj,ry+=dry,l+=dl,srcline+=sl) {
                    bool plot = inClip(rx,ry,&cr);
                    int x=rx;
                    for (;;) {
                        int x2 = cr.right();
                        if (x2 > right) {
                            x2 = right;
                            if (x2 < x) break;
                        }
                        if (plot) {
                            unsigned const char *srcptr=srcline+(x-rx+srcoffs.x())*bytesPerPixel;
                            unsigned char *destptr = l + x*bytesPerPixel;
                            memmove(destptr, srcptr, (x2-x+1) * bytesPerPixel);
                        }
                        x=x2+1;
                        if (x > right)
                            break;
                        plot=inClip(x,ry,&cr,plot);
                    }
                }
            }
        } else {
            unsigned const char *srcptr = srcline + srcoffs.x()*bytesPerPixel;
            unsigned char *destptr = l + rx*bytesPerPixel;
            int bytes = w * bytesPerPixel;
            for (; j!=tj; j+=dj,destptr+=dl,srcptr+=sl) {
                memmove(destptr, srcptr, bytes);
            }
        }
    } else {
        if (alphatype == InlineAlpha || alphatype == SolidAlpha ||
             alphatype == SeparateAlpha) {
            alphabuf = new unsigned int[w];
        }

        // reverse will only ever be true if the source and destination
        // are the same buffer.
        bool reverse = srcoffs.y()==ry && rx>srcoffs.x() &&
                        srctype==SourceImage && srcbits == buffer;

        if (alphatype == LittleEndianMask || alphatype == BigEndianMask) {
            // allows us to optimise GET_MASK a little
            amonolittletest = false;
            if((alphatype==LittleEndianMask && !reverse) ||
                (alphatype==BigEndianMask && reverse)) {
                amonolittletest = true;
            }
        }

        unsigned const char *srcptr = 0;
        for (; j!=tj; j+=dj,ry+=dry,l+=dl,srcline+=sl) {
            bool plot = mustclip ? inClip(rx,ry,&cr) : true;
            int x=rx;
            for (;;) {
                int x2 = cr.right();
                if (x2 > right) {
                    x2 = right;
                    if (x2 < x) break;
                }
                if (plot) {
                    if (srctype == SourceImage) {
                        if (srcdepth == 1) {
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                        if  (srcbits == qt_screen->base())
                                 src_little_endian =  !src_little_endian;
#endif
                            srcptr=find_pointer(srcbits,(x-rx)+srcoffs.x(),
                                         j+srcoffs.y(), x2-x, srclinestep,
                                         monobitcount, monobitval,
                                         !src_little_endian, reverse);
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                        if  (srcbits == qt_screen->base())
                                 src_little_endian =  !src_little_endian;
#endif
                        } else if (srcdepth == 4) {
                            srcptr = find_pointer_4(const_cast<unsigned char*>(srcbits),(x-rx)+srcoffs.x(),
                                         j+srcoffs.y(), x2-x, srclinestep,
                                         monobitcount, monobitval, reverse
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                                                    , srcbits == qt_screen->base()
#endif
                               );
                        } else if (reverse)
                            srcptr = srcline + (x2-rx+srcoffs.x())*srcdepth/8;
                        else
                            srcptr = srcline + (x-rx+srcoffs.x())*srcdepth/8;
                    }
                    switch (alphatype) {
                      case LittleEndianMask:
                      case BigEndianMask:
                        maskp=find_pointer(alphabits,(x-rx)+srcoffs.x(),
                                           j+srcoffs.y(), x2-x, alphalinestep,
                                           amonobitcount,amonobitval,
                                           alphatype==BigEndianMask, reverse);
                        // Fall through
                      case IgnoreAlpha:
                        hImageLineUnclipped(x,x2,l,srcptr,reverse);
                        break;
                      case InlineAlpha:
                      case SolidAlpha:
                        hAlphaLineUnclipped(x,x2,l,srcptr,0);
                        break;
                      case SeparateAlpha:
                        // Separate alpha table
                        unsigned char * alphap=alphabits
                                                +((j+srcoffs.y())*alphalinestep)
                                                +(x-rx)+srcoffs.x();
                        hAlphaLineUnclipped(x,x2,l,srcptr,alphap);
                    }
                }
                x=x2+1;
                if (x > right)
                    break;
                if (mustclip)
                    plot=inClip(x,ry,&cr,plot);
            }
        }
        if (alphabuf) {
            delete [] alphabuf;
            alphabuf = 0;
        }
    }

    srcdepth=osrcdepth;
    GFX_END
}

/*!
    \fn void QGfxRaster<depth,type>::stretchBlt(int rx, int ry, int w, int h, int sw, int sh)

    \internal

    This is similar to blt() but allows the source rectangle to be a different
    size to the destination - the source is expanded or shrunk as necessary
    to fit the destination. The source and destination cannot overlap.
    Note that since the software implementation uses floating point it will
    be slow on embedded processors without an FPU. Qt/Embedded uses
    stretchBlt to speed up QPixmap::xForm. \a rx, \a ry, \a w and \a h
    specify the destination rectangle, \a sw and \a sh specify the size
    of the source; its x and y position are assumed to be 0.
*/
#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS) || !defined(QT_NO_PIXMAP_TRANSFORMATION)
template <const int depth, const int type>
void QGfxRaster<depth,type>::stretchBlt(int rx,int ry,int w,int h,
                                         int sw,int sh)
{
    QRect cr;
    unsigned const char * srcptr;
    unsigned char * data = new unsigned char [(w*depth)/8];
    rx+=xoffs;
    ry+=yoffs;
    //int sy=0;
    unsigned char * l=scanLine(ry);
    unsigned char * sl=data;
    double xfac=sw;
    xfac=xfac/((double)w);
    double yfac=sh;
    yfac=yfac/((double)h);

    int loopc;

    // We don't allow overlapping stretchblt src and destination

    int mulfac;
    if(srcdepth==32) {
        mulfac=4;
    } else if(srcdepth==24) {
        mulfac=3;
    } else if(srcdepth==16) {
        mulfac=2;
    } else if(srcdepth==8) {
        mulfac=1;
    } else {
        mulfac=0;
        qDebug("Can't cope with stretchblt source depth %d",mulfac);
        return;
    }

    QPoint srcoffs = srcwidgetoffs; // + QPoint(sx, sy);

    QRect cursRect(rx, ry, w+1, h+1);
    /* ???
    if (buffer_offset >= 0 && src_buffer_offset >= 0) {
        cursRect = QRect(qMin(rx,srcoffs.x()), qMin(ry,srcoffs.y()),
                        qMax(w, sw)+qAbs(rx - srcoffs.x())+1,
                        qMax(h, sh)+qAbs(ry - srcoffs.y())+1);
    } else if (src_buffer_offset >= 0) {
        cursRect = QRect(srcoffs.x(), srcoffs.y(), sw+1, sh+1);
    }
    */

    GFX_START(cursRect);
    if((*gfx_optype))
        sync();
    (*gfx_optype)=0;
    int osrcdepth=srcdepth;
    int pyp=-1;

    for(int j=0;j<h;j++,ry++,l+=linestep()) {
        bool plot=inClip(rx,ry,&cr);
        int x=rx;

        int yp=(int) (((double) j)*yfac);

        if(yp!=pyp) {
            for(loopc=0;loopc<w;loopc++) {
                int sp=(int) (((double) loopc)*xfac);
                unsigned const char * p=srcScanLine(yp)+(sp*mulfac);
                if(depth==32) {
                    unsigned int val=get_value_32(srcdepth,&p);
                    unsigned int * dp=(unsigned int *)data;
                    *(dp+loopc)=val;
                } else if(depth==24) {
                    unsigned int val=get_value_32(srcdepth,&p);
                    unsigned char* dp=(unsigned char *)data;
                    gfxSetRgb24(dp+loopc*3, val);
                } else if(depth==16) {
                    unsigned int val=get_value_16(srcdepth,&p);
                    unsigned short int * dp=(unsigned short int *)data;
                    *(dp+loopc)=val;
                } else if(depth==8) {
                    unsigned int val=get_value_8(srcdepth,&p);
                    *(data+loopc)=val;
                } else {
                    qDebug("Can't cope with stretchblt depth %d",depth);
                    GFX_END
                    delete [] data;
                    return;
                }
            }
            pyp=yp;
        }

        srcdepth=depth;
        for (;;) {
            int x2 = cr.right();
            if (x2 >= rx+w-1) {
                srcptr=sl;
                srcptr+=(((x-rx)+srcoffs.x())*mulfac);
                if (plot) {
                    hImageLineUnclipped(x,rx+w-1,l,srcptr,false);
                }
                break;
            } else {
                srcptr=sl;
                srcptr+=(((x-rx)+(srcoffs.x()))*mulfac);
                if (plot) {
                        hImageLineUnclipped(x,x2,l,srcptr,false);
                }
                x=x2+1;
                plot=inClip(x,ry,&cr,plot);
            }
        }
        srcdepth=osrcdepth;
    }
    delete [] data;
    GFX_END
}
#endif

/*!
    \fn void QGfxRaster<depth,type>::tiledBlt(int rx,int ry,int w,int h)

    \internal

    Like scroll(), this is intended as a candidate for hardware acceleration
    - it's a special case of blt where the source can be a different size
    to the destination and is tiled across the destination. \a rx and \a ry
    specify the x and y position of the rectangle to fill, \a w and \a h
    its size.
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::tiledBlt(int rx,int ry,int w,int h)
{
    if (srcwidth == 0 || srcheight == 0)
        return;
    GFX_START(QRect(rx+xoffs, ry+yoffs, w+1, h+1))

    pixel = brushPixel;
    unsigned char * savealphabits=alphabits;

    int offx = srcwidgetoffs.x() + rx - brushorig.x();
    int offy = srcwidgetoffs.y() + ry - brushorig.y();

    // from qpainter_qws.cpp
    if (offx < 0)
        offx = srcwidth - -offx % srcwidth;
    else
        offx = offx % srcwidth;
    if (offy < 0)
        offy = srcheight - -offy % srcheight;
    else
        offy = offy % srcheight;

    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = ry;
    yOff = offy;
    while(yPos < ry + h) {
        drawH = srcheight - yOff;    // Cropping first row
        if (yPos + drawH > ry + h)        // Cropping last row
            drawH = ry + h - yPos;
        xPos = rx;
        xOff = offx;
        while(xPos < rx + w) {
            drawW = srcwidth - xOff; // Cropping first column
            if (xPos + drawW > rx + w)    // Cropping last column
                drawW = rx + w - xPos;
            blt(xPos, yPos, drawW, drawH,xOff,yOff);
            alphabits=savealphabits;
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
    GFX_END
}

/*!
  \class QScreen qgfx_qws.h
  \brief The QScreen class and its descendants manage the framebuffer and
  palette.

  \ingroup qws

  QScreens act as factories for the screen cursor and QGfx's. QLinuxFbScreen
  manages a Linux framebuffer; accelerated drivers subclass QLinuxFbScreen.
  There can only be one screen in a Qt/Embedded application.
*/

/*!
     \enum QScreen::PixelType

     \value NormalPixel
     \value BGRPixel
*/

/*!
\fn QScreen::initDevice()
This function is called by the Qt/Embedded server when initializing
the framebuffer. Accelerated drivers use it to set up the graphics card.
*/

/*!
\fn QScreen::connect(const QString &displaySpec)
This function is called by every Qt/Embedded application on startup.
It maps in the framebuffer and in the accelerated drivers the graphics
card control registers. \a displaySpec has the following syntax:
<p>
<tt>[gfx driver][:driver specific options][:display number]</tt>
<p>
for example if you want to use the mach64 driver on fb1 as display 2:
<p>
<tt>Mach64:/dev/fb1:2</tt>
<p>
\a displaySpec is passed in via the QWS_DISPLAY environment variable
or the -display command line parameter.
*/

/*!
\fn QScreen::disconnect()
This function is called by every Qt/Embedded application just
before exitting; it's normally used to unmap the framebuffer.
*/

/*!
    \fn QScreen::setMode(int width, int height, int depth)

    This function can be used to set the framebuffer \a width, \a
    height, and \a depth. It's currently unused.
*/

/*!
\fn QScreen::blank(bool on)
If \a on is true, blank the screen. Otherwise unblank it.
*/

/*!
\fn QScreen::pixmapOffsetAlignment()
Returns the value in bits to which the start address of pixmaps held in
graphics card memory should be aligned. This is only useful for accelerated
drivers. By default the value returned is 64 but it can be overridden
by individual accelerated drivers.
*/

/*!
\fn QScreen::pixmapLinestepAlignment()
Returns the value in bits to which individual scanlines of pixmaps held in
graphics card memory should be aligned. This is only useful for accelerated
drivers. By default the value returned is 64 but it can be overridden
by individual accelerated drivers.
*/

/*!
\fn QScreen::width() const
Gives the width in pixels of the framebuffer.
*/

/*!
\fn QScreen::height() const
Gives the height in pixels of the framebuffer.
*/

/*!
\fn QScreen::depth() const
Gives the depth in bits per pixel of the framebuffer. This is the number
of bits each pixel takes up rather than the number of significant bits,
so 24bpp and 32bpp express the same range of colors (8 bits of
red, green and blue)
*/

/*!
\fn QScreen::pixmapDepth() const
Gives the preferred depth for pixmaps. By default this is the same
as the screen depth, but for the VGA16 driver it's 8bpp.
*/

/*!
\fn QScreen::linestep() const
Returns the length in bytes of each scanline of the framebuffer.
*/

/*!
  \fn QScreen::deviceWidth() const
Gives the full width of the framebuffer device, as opposed to the
width which Qt/Embedded will actually use. These can differ if the
display is centered within the framebuffer.
*/

/*!
  \fn QScreen::deviceHeight() const
Gives the full height of the framebuffer device, as opposed to the
height which Qt/Embedded will actually use. These can differ if the
display is centered within the framebuffer.
*/

/*!
  \fn QScreen::base() const
Returns a pointer to the start of the framebuffer.
*/

/*!
    \fn QScreen::cache(int,int)

    \internal

    This function is used to store pixmaps in graphics memory for the
    use of the accelerated drivers. See QLinuxFbScreen (where the
    cacheing is implemented) for more information.
*/

/*!
    \fn QScreen::uncache(uchar *)

    \internal

    This function is called on pixmap destruction to remove them from
    graphics card memory.
*/

/*!
  \fn QScreen::screenSize() const
Returns the size in bytes of the screen. This is always located at
the beginning of framebuffer memory (i.e. at base()).
*/

/*!
  \fn QScreen::totalSize() const
Returns the size in bytes of available graphics card memory, including the
screen. Offscreen memory is only used by the accelerated drivers.
*/

// Unaccelerated screen/driver setup. Can be overridden by accelerated
// drivers

/*!
  \fn QScreen::QScreen(int display_id)
  Create a screen; the \a display_id is the number of the Qt/Embedded server
  to connect to.
*/

/*!
  \fn QScreen::clut()
  Returns the screen's color lookup table (color palette). This is only
  valid in paletted modes (8bpp and lower).
*/

/*!
  \fn QScreen::numCols()
  Returns the number of entries in the color table returned by clut().
*/

QScreen::QScreen(int display_id)
{
    pixeltype=NormalPixel;
    data = 0;
    displayId = display_id;
    initted=false;
    entryp=0;
    clearCacheFunc = 0;
    grayscale = false;
    screencols = 0;
}

/*!
  Destroys a QScreen
*/

QScreen::~QScreen()
{
}

/*!
  Called by the Qt/Embedded server on shutdown; never called by
  a Qt/Embedded client. This is intended to support graphics card specific
  shutdown; the unaccelerated implementation simply hides the mouse cursor.
*/

void QScreen::shutdownDevice()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
#endif
}

extern bool qws_accel; //in qapplication_qws.cpp

/*!
  Returns a QGfx (normally a QGfxRaster) initialized to point to the screen,
  with an origin at 0,0 and a clip region covering the whole screen.
*/

QGfx * QScreen::screenGfx()
{
    QGfx * ret=createGfx(data,w,h,d,lstep);
    if(d<=8) {
        ret->setClut(clut(),numCols());
    }
    return ret;
}

/*!
  \fn PixelType QScreen::pixelType() const
  Returns  the pixel storage format of the screen.
 */

/*!
  Given an RGB value \a r \a g \a b, return an index which is the closest
  match to it in the screen's palette. Used in paletted modes only.
*/

int QScreen::alloc(unsigned int r,unsigned int g,unsigned int b)
{
    int ret = 0;
    if (d == 8) {
        if (grayscale)
            return qGray(r, g, b);
        // First we look to see if we match a default color
        QRgb myrgb=qRgb(r,g,b);
        int pos= (r + 25) / 51 * 36 + (g + 25) / 51 * 6 + (b + 25) / 51;
        if (simple_8bpp_alloc || screenclut[pos] == myrgb || !initted) {
            return pos;
        }

        // search for nearest color
        unsigned int mindiff = 0xffffffff;
        unsigned int diff;
        int dr,dg,db;

        for (int loopc = 0; loopc < 256; loopc++) {
            dr = qRed(screenclut[loopc]) - r;
            dg = qGreen(screenclut[loopc]) - g;
            db = qBlue(screenclut[loopc]) - b;
            diff = dr*dr + dg*dg + db*db;

            if (diff < mindiff) {
                ret = loopc;
                if (!diff)
                    break;
                mindiff = diff;
            }
        }
    } else if (d == 4) {
        ret = qGray(r, g, b) >> 4;
    } else if (d == 1 && simple_8bpp_alloc) {
        ret = qGray(r, g, b) > 128;
    } else {
        qFatal("cannot alloc %dbpp color", d);
    }

    return ret;
}

/*!
This is used to initialize the software cursor - \a end_of_location
points to the address after the area where the cursor image can be stored.
\a init is true for the first application this method is called from
(the Qt/Embedded server), false otherwise.
*/

int QScreen::initCursor(void* end_of_location, bool init)
{
    /*
      The end_of_location parameter is unusual: it's the address
      after the cursor data.
    */
#ifndef QT_NO_QWS_CURSOR
    qt_sw_cursor=true;
    // ### until QLumpManager works Ok with multiple connected clients,
    // we steal a chunk of shared memory
    SWCursorData *data = (SWCursorData *)end_of_location - 1;
    qt_screencursor=new QScreenCursor();
    qt_screencursor->init(data, init);
    return sizeof(SWCursorData);
#else
    return 0;
#endif
}

/*!
  Saves the state of the graphics card - used so that, for instance,
  the palette can be restored when switching between linux virtual
  consoles. Hardware QScreen descendants should save register state
  here if necessary if switching between virtual consoles (for
  example to/from X) is to be permitted.
*/

void QScreen::save()
{
}

/*!
  Restores the state of the graphics card from a previous save()
*/

void QScreen::restore()
{
}

void QScreen::blank(bool)
{
}

/*!
    \internal
*/

void QScreen::set(unsigned int, unsigned int, unsigned int, unsigned int)
{
}

/*!
\fn bool QScreen::supportsDepth(int d) const
Returns true if the screen supports a particular color depth \a d.
Possible values are 1,4,8,16 and 32.
*/

bool QScreen::supportsDepth(int d) const
{
    if (false) {
        //Just to simplify the ifdeffery
#ifndef QT_NO_QWS_DEPTH_1
    } else if(d==1) {
        return true;
#endif
#ifndef QT_NO_QWS_DEPTH_4
    } else if(d==4) {
        return true;
#endif
#ifndef QT_NO_QWS_DEPTH_8
    } else if(d==8) {
        return true;
#endif
#ifndef QT_NO_QWS_DEPTH_16
    } else if(d==16) {
        return true;
#endif
#ifndef QT_NO_QWS_DEPTH_24
    } else if(d==24) {
        return true;
#endif
#ifndef QT_NO_QWS_DEPTH_32
    } else if(d==32) {
        return true;
#endif
    }
    return false;
}

// explicit template instaniation
#ifndef QT_NO_QWS_DEPTH_1
template class QGfxRaster<1,0>;
#endif
#ifndef QT_NO_QWS_DEPTH_4
template class QGfxRaster<4,0>;
#endif
#ifndef QT_NO_QWS_DEPTH_8
template class QGfxRaster<8,0>;
#endif
#ifndef QT_NO_QWS_DEPTH_16
template class QGfxRaster<16,0>;
#endif
#ifndef QT_NO_QWS_DEPTH_24
template class QGfxRaster<24,0>;
#endif
#ifndef QT_NO_QWS_DEPTH_32
template class QGfxRaster<32,0>;
#endif

/*!
    Creates a gfx on an arbitrary buffer \a bytes, width \a w and height \a h in
    pixels, depth \a d and \a linestep (length in bytes of each line in the
    buffer). Accelerated drivers can check to see if \a bytes points into
    graphics memory and create an accelerated Gfx.
*/

QGfx * QScreen::createGfx(unsigned char * bytes,int w,int h,int d, int linestep)
{
    QGfx* ret;
    if (false) {
        //Just to simplify the ifdeffery
#ifndef QT_NO_QWS_DEPTH_1
    } else if(d==1) {
        ret = new QGfxRaster<1,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_4
    } else if(d==4) {
        ret = new QGfxRaster<4,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8
    } else if(d==8) {
        ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_16
    } else if(d==16) {
        ret = new QGfxRaster<16,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_24
    } else if(d==24) {
        ret = new QGfxRaster<24,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_32
    } else if(d==32) {
        ret = new QGfxRaster<32,0>(bytes,w,h);
#endif
    } else {
        qFatal("Can't drive depth %d",d);
        ret = 0; // silence gcc
    }
    ret->setLineStep(linestep);
    return ret;
}

/*!
\fn bool QScreen::onCard(const unsigned char * p) const
Returns true if the buffer pointed to by \a p is within graphics card
memory, false if it's in main RAM.
*/

bool QScreen::onCard(const unsigned char * p) const
{
    long t=(unsigned long)p;
    long bmin=(unsigned long)data;
    if (t < bmin)
        return false;
    if(t >= bmin+mapsize)
        return false;
    return true;
}

/*!
\fn bool QScreen::onCard(const unsigned char * p, ulong& offset) const
\overload
This checks whether the buffer specified by \a p is on the card
(as per the other version of onCard) and returns an offset in bytes
from the start of graphics card memory in \a offset if it is.
*/

bool QScreen::onCard(const unsigned char * p, ulong& offset) const
{
    long t=(unsigned long)p;
    long bmin=(unsigned long)data;
    if (t < bmin)
        return false;
    long o = t - bmin;
    if (o >= mapsize)
        return false;
    offset = o;
    return true;
}

/*
#if !defined(QT_NO_QWS_REPEATER)
    { "Repeater", qt_get_screen_repeater, 0 },
#endif
#if defined(QT_QWS_EE)
    { "EE", qt_get_screen_ee, 0 },
#endif

*/

/*
Given a display_id (number of the Qt/Embedded server to connect to)
and a spec (e.g. Mach64:/dev/fb0) return a QScreen-descendant.
The QGfxDriverFactory is queried for a suitable driver and, if found,
asked to create a driver.
People writing new graphics drivers should either hook their own
QScreen-descendant into QGfxDriverFactory or use the QGfxDriverPlugin
to make a dynamically loadable driver.
*/

QScreen *qt_get_screen(int display_id, const char *spec)
{
    QString displaySpec(spec);
    QString driver = displaySpec;
    int colon = displaySpec.indexOf(':');
    if (colon >= 0)
        driver.truncate(colon);

    bool foundDriver = false;
    QString driverName = driver;

    QStringList driverList = QGfxDriverFactory::keys();
    QStringList::Iterator it;
    for (it = driverList.begin(); it != driverList.end(); ++it) {
        if (driver.isEmpty() || QString(*it) == driver) {
            driverName = *it;
            qt_screen = QGfxDriverFactory::create(driverName, display_id);
            if (qt_screen) {
                foundDriver = true;
                if (qt_screen->connect(spec)) {
                    return qt_screen;
                } else {
                    delete qt_screen;
                    qt_screen = 0;
                }
            }
        }
    }

    if (driver.isNull())
        qFatal("No suitable driver found");
    else if (foundDriver)
        qFatal("%s: driver cannot connect", driver.toLatin1().constData());
    else
        qFatal("%s: driver not found", driver.toLatin1().constData());

    return 0;
}
