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
#include "qwidget.h"


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
//####    qWarning("graphicsContext(QImage*) should not be called");
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
    gfx->setClipDeviceRegion(QRect(0, 0, qt_screen->width(), qt_screen->height()));

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
        gfx->setClipDeviceRegion(QRegion(0, 0, qt_screen->width(), qt_screen->height()));
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
        gfxunder->srclinestep = gfx->linestep();
        gfxunder->srcdepth = gfx->bitDepth();
        gfxunder->srcbits = gfx->buffer;
        gfxunder->srcpixeltype = QScreen::NormalPixel;
        gfxunder->srcwidth = qt_screen->width();
        gfxunder->srcheight = qt_screen->height();
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
    \fn QGfxRaster<depth,type>::setSource(const QImage *i)

    \internal

    This sets up future blt's to use a QImage \a i as a source - used by
    QPainter::drawImage()
*/
template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(const QImage *i)
{
    //qWarning("QGfxRaster::setSource(const QImage*)");
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
    src_normal_palette=false;
    if (srcdepth == 1)
        buildSourceClut(0, 0);
    else  if(srcdepth<=8)
        buildSourceClut(const_cast<QRgb*>(i->colorTable().constData()),i->numColors());
}


template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(const QPixmap *pix)
{
    setSource(&pix->toImage()); //###
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
    srcpixeltype=QScreen::NormalPixel;
    srclinestep=l;
    srcdepth=d;
    srcbits=c;
    src_little_endian=true;
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


static inline void drawPoint_4(int x, unsigned char *l, uint pixel)
{
    uchar *d = l + (x>>1);
    int s = (x & 1) << 2;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
    if (is_screen_gfx)
        s = (~x & 1) << 2;
#endif
    *d = (*d & MASK4BPP(s)) | (pixel << s);
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
            drawPoint_4(x1, myptr, pixel);
            w--;
            dptr++;
        }

        unsigned char val = pixel | (pixel << 4);
        while (w > 1) {
            *dptr++ = val;
            w -= 2;
        }

        if (!(x2&1))
            drawPoint_4(x2, myptr, pixel);
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
    variable srcdata points to it) or as a solid value stored in pixel
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

        while (w--) {
            uint gv = get_value_32(srcdepth,&srcdata);
            *(myptr++) = gv;
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

        while (w--) {
            uint gv = get_value_24(srcdepth,&srcdata);
            gfxSetRgb24(myptr, gv);
            myptr += 3;
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

#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //restore
        if (srcdepth == 4 && srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
    } else if (depth == 4) {
        unsigned char *dp = l;
        unsigned int gv = pixel;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
        //really ugly hack: screen is opposite endianness to everything else
        if (srcbits==qt_screen->base())
            src_little_endian = !src_little_endian;
#endif
        if (reverse) {
            dp += (x2/2);
            int x = x2;
            while (w--) {
                gv = get_value_4(srcdepth, &srcdata, reverse);
                int s = (x&1) << 2;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                if (is_screen_gfx)
                    s = (~x&1) << 2;
#endif
                *dp = (*dp & MASK4BPP(s)) | (gv << s);

                if (!(x&1))
                    dp--;
                x--;
            }
        } else {
            dp += (x1/2);
            int x = x1;
            while (w--) {
                gv = get_value_4(srcdepth, &srcdata, reverse);

                int s = (x&1) << 2;
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                if (is_screen_gfx)
                    s = (~x&1) << 2;
#endif
                *dp = (*dp & MASK4BPP(s)) | (gv << s);

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
        unsigned int gv = pixel;
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
                        gv = get_value_1(srcdepth, &srcdata, true);

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
                        gv = get_value_1(srcdepth, &srcdata, false);
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
    GFX_START(QRect(rx, ry, w+1, h+1))

    if (w <= 0 || h <= 0) {
        GFX_END
        return;
    }

#ifdef QWS_EXPERIMENTAL_FASTPATH
    // ### fix for 8bpp
    // This seems to be reliable now, at least for 16bpp

    if (ncliprect == 1) {
        // Fast path
        if(depth==16) {
            pixel = brushPixel;
            int x1,y1,x2,y2;
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
    if(cbrush.style()!=Qt::NoBrush) {
        pixel = brushPixel;
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

    QPoint srcoffs = QPoint(sx, sy);

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
    if (srcdepth == depth
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

        // reverse will only ever be true if the source and destination
        // are the same buffer.
        bool reverse = srcoffs.y()==ry && rx>srcoffs.x() &&
                       srcbits == buffer;

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
                    hImageLineUnclipped(x,x2,l,srcptr,reverse);
                }
                x=x2+1;
                if (x > right)
                    break;
                if (mustclip)
                    plot=inClip(x,ry,&cr,plot);
            }
        }
    }

    srcdepth=osrcdepth;
    GFX_END
}

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
    GFX_START(QRect(rx, ry, w+1, h+1))

    pixel = brushPixel;

    int offx =  rx;
    int offy = ry;

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
