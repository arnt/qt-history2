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

// Uncomment the next line to enable the MIT Shared Memory extension
//
// WARNING:  This has some problems:
//
//    1. Consumes a 800x600 pixmap
//    2. Qt does not handle the ShmCompletion message, so you will
//        get strange effects if you xForm() repeatedly.
//
// #define QT_MITSHM

#if defined(Q_OS_WIN32) && defined(QT_MITSHM)
#undef QT_MITSHM
#endif

#include "qplatformdefs.h"

#include "qiodevice.h"
#include "qpixmap_p.h"
#include "qbitmap.h"
#include "qcolormap.h"
#include "qimage.h"
#include "qmatrix.h"
#include "qapplication.h"
#include "qcolormap.h"
#include <private/qpaintengine_x11_p.h>
#include <private/qt_x11_p.h>
#include "qx11info_x11.h"

#include <stdlib.h>

#if defined(Q_CC_MIPS)
#  define for if(0){}else for
#endif


// For thread-safety:
//   image->data does not belong to X11, so we must free it ourselves.

inline static void qSafeXDestroyImage(XImage *x)
{
    if (x->data) {
        free(x->data);
        x->data = 0;
    }
    XDestroyImage(x);
}


/*****************************************************************************
  MIT Shared Memory Extension support: makes xForm noticeably (~20%) faster.
 *****************************************************************************/

#if defined(QT_MITSHM)

static bool               xshminit = false;
static XShmSegmentInfo xshminfo;
static XImage              *xshmimg = 0;
static Pixmap               xshmpm  = 0;

static void qt_cleanup_mitshm()
{
    if (xshmimg == 0)
        return;
    Display *dpy = QX11Info::appDisplay();
    if (xshmpm) {
        XFreePixmap(dpy, xshmpm);
        xshmpm = 0;
    }
    XShmDetach(dpy, &xshminfo); xshmimg->data = 0;
    qSafeXDestroyImage(xshmimg); xshmimg = 0;
    shmdt(xshminfo.shmaddr);
    shmctl(xshminfo.shmid, IPC_RMID, 0);
}

static bool qt_create_mitshm_buffer(const QPaintDevice* dev, int w, int h)
{
    static int major, minor;
    static Bool pixmaps_ok;
    Display *dpy = dev->data->xinfo->display();
    int dd         = dev->x11Depth();
    Visual *vis         = (Visual*)dev->x11Visual();

    if (xshminit) {
        qt_cleanup_mitshm();
    } else {
        if (!XShmQueryVersion(dpy, &major, &minor, &pixmaps_ok))
            return false;                        // MIT Shm not supported
        qAddPostRoutine(qt_cleanup_mitshm);
        xshminit = true;
    }

    xshmimg = XShmCreateImage(dpy, vis, dd, ZPixmap, 0, &xshminfo, w, h);
    if (!xshmimg)
        return false;

    bool ok;
    xshminfo.shmid = shmget(IPC_PRIVATE,
                             xshmimg->bytes_per_line * xshmimg->height,
                             IPC_CREAT | 0777);
    ok = xshminfo.shmid != -1;
    if (ok) {
        xshmimg->data = (char*)shmat(xshminfo.shmid, 0, 0);
        xshminfo.shmaddr = xshmimg->data;
        ok = (xshminfo.shmaddr != (char*)-1);
    }
    xshminfo.readOnly = false;
    if (ok)
        ok = XShmAttach(dpy, &xshminfo);
    if (!ok) {
        qSafeXDestroyImage(xshmimg);
        xshmimg = 0;
        if (xshminfo.shmaddr)
            shmdt(xshminfo.shmaddr);
        if (xshminfo.shmid != -1)
            shmctl(xshminfo.shmid, IPC_RMID, 0);
        return false;
    }
    if (pixmaps_ok)
        xshmpm = XShmCreatePixmap(dpy, DefaultRootWindow(dpy), xshmimg->data,
                                   &xshminfo, w, h, dd);

    return true;
}

#else

// If extern, need a dummy.
//
// static bool qt_create_mitshm_buffer(QPaintDevice*, int, int)
// {
//     return false;
// }

#endif // QT_MITSHM


/*****************************************************************************
  Internal functions
 *****************************************************************************/

extern const uchar *qt_get_bitflip_array();                // defined in qimage.cpp

static uchar *flip_bits(const uchar *bits, int len)
{
    register const uchar *p = bits;
    const uchar *end = p + len;
    uchar *newdata = new uchar[len];
    uchar *b = newdata;
    const uchar *f = qt_get_bitflip_array();
    while (p < end)
        *b++ = f[*p++];
    return newdata;
}

// Returns position of highest bit set or -1 if none
static int highest_bit(uint v)
{
    int i;
    uint b = (uint)1 << 31;
    for (i=31; ((b & v) == 0) && i>=0;         i--)
        b >>= 1;
    return i;
}

// Returns position of lowest set bit in 'v' as an integer (0-31), or -1
static int lowest_bit(uint v)
{
    int i;
    ulong lb;
    lb = 1;
    for (i=0; ((v & lb) == 0) && i<32;  i++, lb<<=1);
    return i==32 ? -1 : i;
}

// Counts the number of bits set in 'v'
static uint n_bits(uint v)
{
    int i = 0;
    while (v) {
        v = v & (v - 1);
        i++;
    }
    return i;
}

static uint *red_scale_table   = 0;
static uint *green_scale_table = 0;
static uint *blue_scale_table  = 0;

static void cleanup_scale_tables()
{
    delete[] red_scale_table;
    delete[] green_scale_table;
    delete[] blue_scale_table;
}

/*
  Could do smart bitshifting, but the "obvious" algorithm only works for
  nBits >= 4. This is more robust.
*/
static void build_scale_table(uint **table, uint nBits)
{
    if (nBits > 7) {
        qWarning("build_scale_table: internal error, nBits = %i", nBits);
        return;
    }
    if (!*table) {
        static bool firstTable = true;
        if (firstTable) {
            qAddPostRoutine(cleanup_scale_tables);
            firstTable = false;
        }
        *table = new uint[256];
    }
    int   maxVal   = (1 << nBits) - 1;
    int   valShift = 8 - nBits;
    int i;
    for(i = 0 ; i < maxVal + 1 ; i++)
        (*table)[i << valShift] = i*255/maxVal;
}

static int defaultScreen = -1;

/*****************************************************************************
  QPixmap member functions
 *****************************************************************************/

/*!
  \internal
  Initializes the pixmap data.
*/

void QPixmap::init(int w, int h, int d, bool bitmap, Optimization optim)
{
    if (qApp->type() == QApplication::Tty) {
        qWarning("QPixmap: Cannot create a QPixmap when no GUI "
                 "is being used");
    }

    static int serial = 0;

    data = new QPixmapData;
    memset(data, 0, sizeof(QPixmapData));
    data->count  = 1;
    data->uninit = true;
    data->bitmap = bitmap;
    data->ser_no = ++serial;
    data->optim = optim;
    data->xft_hd = 0;

    if (defaultScreen >= 0 && defaultScreen != data->xinfo.screen()) {
        QX11InfoData* xd = data->xinfo.getX11Data(true);
        xd->screen = defaultScreen;
        xd->depth = QX11Info::appDepth(xd->screen);
        xd->cells = QX11Info::appCells(xd->screen);
        xd->colormap = QX11Info::appColormap(xd->screen);
        xd->defaultColormap = QX11Info::appDefaultColormap(xd->screen);
        xd->visual = (Visual *)QX11Info::appVisual(xd->screen);
        xd->defaultVisual = QX11Info::appDefaultVisual(xd->screen);
        data->xinfo.setX11Data(xd);
    }

    int dd = data->xinfo.depth();

    if (optim == DefaultOptim)                // use default optimization
        optim = defOptim;

    bool make_null = w == 0 || h == 0;                // create null pixmap
    if (d == 1)                                // monocrome pixmap
        data->d = 1;
    else if (d < 0 || d == dd)                // def depth pixmap
        data->d = dd;
    if (make_null || w < 0 || h < 0 || data->d == 0) {
        data->hd = 0;
        data->xft_hd = 0;
        if (!make_null)
            qWarning("QPixmap: Invalid pixmap parameters");
        return;
    }
    data->w = w;
    data->h = h;
    data->hd = (Qt::HANDLE)XCreatePixmap(data->xinfo.display(),
                                         RootWindow(data->xinfo.display(),
                                                    data->xinfo.screen()),
                                         w, h, data->d);

#ifndef QT_NO_XFT
    if (X11->has_xft) {
        if (data->d == 1) {
            data->xft_hd = (Qt::HANDLE) XftDrawCreateBitmap(data->xinfo.display(), data->hd);
        } else {
            data->xft_hd = (Qt::HANDLE) XftDrawCreate(data->xinfo.display(), data->hd,
                                                      (Visual *) data->xinfo.visual(),
                                                      data->xinfo.colormap());
        }
    }
#endif // QT_NO_XFT
}

QPixmapData::~QPixmapData()
{
    delete mask;
    delete alphapm;
    if (ximage)
        qSafeXDestroyImage((XImage*)ximage);
    if (maskgc)
        XFreeGC(xinfo.display(), (GC)maskgc);
    if (qApp && hd) {

#ifndef QT_NO_XFT
        if (xft_hd) {
            XftDrawDestroy((XftDraw *) xft_hd);
            xft_hd = 0;
        }
#endif // QT_NO_XFT

        XFreePixmap(xinfo.display(), hd);
        hd = 0;
    }
    delete paintEngine;
}

/*!
    Constructs a monochrome pixmap, with width \a w and height \a h,
    that is initialized with the data in \a bits. The \a isXbitmap
    indicates whether the data is an X bitmap and defaults to false.
    This constructor is protected and used by the QBitmap class.
*/

QPixmap::QPixmap(int w, int h, const uchar *bits, bool isXbitmap)
    : QPaintDevice(QInternal::Pixmap)
{                                                // for bitmaps only
    init(0, 0, 0, false, defOptim);
    if (w <= 0 || h <= 0)                        // create null pixmap
        return;

    data->uninit = false;
    data->w = w;
    data->h = h;
    data->d = 1;
    uchar *flipped_bits;
    if (isXbitmap) {
        flipped_bits = 0;
    } else {                                        // not X bitmap -> flip bits
        flipped_bits = flip_bits(bits, ((w+7)/8)*h);
        bits = flipped_bits;
    }
    data->hd = (Qt::HANDLE)XCreateBitmapFromData(data->xinfo.display(),
					     RootWindow(data->xinfo.display(), data->xinfo.screen()),
					     (char *)bits, w, h);

#ifndef QT_NO_XFT
    if (X11->has_xft)
        data->xft_hd = (Qt::HANDLE) XftDrawCreateBitmap (data->xinfo.display (), data->hd);
#endif // QT_NO_XFT

    if (flipped_bits)                                // Avoid purify complaint
        delete [] flipped_bits;
}


/*!
    This is a special-purpose function that detaches the pixmap from
    shared pixmap data.

    A pixmap is automatically detached by Qt whenever its contents is
    about to change. This is done in all QPixmap member functions that
    modify the pixmap (fill(), resize(), convertFromImage(), load(),
    etc.), and in QPainter::begin() on a pixmap.

    It is possible to modify a pixmap without letting Qt know. You can
    first obtain the system-dependent handle() and then call
    system-specific functions (for instance, BitBlt under Windows)
    that modify the pixmap contents. In such cases, you can call
    detach() to cut the pixmap loose from other pixmaps that share
    data with this one.

    detach() returns immediately if there is just a single reference
    or if the pixmap has not been initialized yet.
*/

void QPixmap::detach()
{
    if ( data->count == 1 ) {
        data->uninit = FALSE;
        return;
    }
    *this = copy();
    data->uninit = FALSE;

    // reset the cache data
    if (data->ximage) {
        qSafeXDestroyImage((XImage*)data->ximage);
        data->ximage = 0;
    }
    if (data->maskgc) {
        XFreeGC(data->xinfo.display(), (GC)data->maskgc);
        data->maskgc = 0;
    }
}


/*!
    Returns the default pixmap depth, i.e. the depth a pixmap gets if
    -1 is specified.

    \sa depth()
*/

int QPixmap::defaultDepth()
{
    return QX11Info::appDepth();
}


/*!
    Sets pixmap drawing optimization for this pixmap.

    The \a optimization setting affects pixmap operations, in
    particular drawing of transparent pixmaps (bitBlt() a pixmap with
    a mask set) and pixmap transformations (the transform() function).

    Pixmap optimization involves keeping intermediate results in a
    cache buffer and using the cache to speed up
    QPainter::drawPixmap() and transform().  The cost is more memory
    consumption, up to twice as much as an unoptimized pixmap.

    Use the setDefaultOptimization() to change the default
    optimization for all new pixmaps.

    \sa optimization(), setDefaultOptimization(), defaultOptimization()
*/

void QPixmap::setOptimization(Optimization optimization)
{
    if (optimization == data->optim)
        return;
    detach();
    data->optim = optimization == DefaultOptim ? defOptim : optimization;
    if (data->optim == MemoryOptim && data->ximage) {
        qSafeXDestroyImage((XImage*)data->ximage);
        data->ximage = 0;
    }
}


/*!
    Fills the pixmap with the color \a fillColor.
*/

void QPixmap::fill(const QColor &fillColor)
{
    if (isNull())
        return;
    if (fillColor.alpha() != 255) {
        QImage im(width(), height(), 32);
        im.fill(fillColor.rgba());
        im.setAlphaBuffer(true);
        *this = im;
        return;
    } else {
        detach();
    }
    GC gc = qt_xget_temp_gc(data->xinfo.screen(), depth()==1);
    XSetForeground(data->xinfo.display(), gc, QColormap::instance(data->xinfo.screen()).pixel(fillColor));
    XFillRectangle(data->xinfo.display(), data->hd, gc, 0, 0, width(), height());
}


/*!
  Internal implementation of the virtual QX11Info::metric() function.

  \a m is the metric to get.
*/

int QPixmap::metric(PaintDeviceMetric m) const
{
    int val;
    if (m == PdmWidth)
        val = width();
    else if (m == PdmHeight) {
        val = height();
    } else {
        Display *dpy = data->xinfo.display();
        int scr = data->xinfo.screen();
        switch (m) {
            case PdmDpiX:
            case PdmPhysicalDpiX:
                val = QX11Info::appDpiX(scr);
                break;
            case PdmDpiY:
            case PdmPhysicalDpiY:
                val = QX11Info::appDpiY(scr);
                break;
            case PdmWidthMM:
                val = (DisplayWidthMM(dpy,scr)*width())/
                      DisplayWidth(dpy,scr);
                break;
            case PdmHeightMM:
                val = (DisplayHeightMM(dpy,scr)*height())/
                      DisplayHeight(dpy,scr);
                break;
            case PdmNumColors:
                val = 1 << depth();
                break;
            case PdmDepth:
                val = depth();
                break;
            default:
                val = 0;
                qWarning("QPixmap::metric: Invalid metric command");
        }
    }
    return val;
}

/*!
    Converts the pixmap to a QImage. Returns a null image if it fails.

    If the pixmap has 1-bit depth, the returned image will also be 1
    bit deep. If the pixmap has 2- to 8-bit depth, the returned image
    has 8-bit depth. If the pixmap has greater than 8-bit depth, the
    returned image has 32-bit depth.

    Note that for the moment, alpha masks on monochrome images are
    ignored.

    \sa convertFromImage()
*/

QImage QPixmap::toImage() const
{
    QImage image;
    if (isNull())
        return image; // null image

    int            w  = width();
    int            h  = height();
    int            d  = depth();
    bool    mono = d == 1;
    Visual *visual = (Visual *) data->xinfo.visual();
    bool    trucol = (visual->c_class >= TrueColor) && !mono && d > 8;

    if (d > 1 && d <= 8)                        // set to nearest valid depth
        d = 8;                                        //   2..8 ==> 8
    // we could run into the situation where d == 8 AND trucol is true, which can
    // cause problems when converting to and from images.  in this case, always treat
    // the depth as 32... from Klaus Schmidinger and qt-bugs/arc-15/31333.
    if (d > 8 || trucol)
        d = 32;                                        //   > 8  ==> 32

    XImage *xi = (XImage *)data->ximage;        // any cached ximage?
    if (!xi)                                        // fetch data from X server
        xi = XGetImage(data->xinfo.display(), data->hd, 0, 0, w, h, AllPlanes,
                        mono ? XYPixmap : ZPixmap);

    Q_CHECK_PTR(xi);
    if (!xi)
        return image;

    QImage::Endian bitOrder = QImage::IgnoreEndian;
    if (mono)
        bitOrder = xi->bitmap_bit_order == LSBFirst ? QImage::LittleEndian : QImage::BigEndian;

    image.create(w, h, d, 0, bitOrder);
    if (image.isNull())                        // could not create image
        return image;

    const QPixmap* msk = mask();
    const QPixmap *alf = data->alphapm;

    QImage alpha;
    if (alf) {
        XImage *axi = XGetImage(data->xinfo.display(), alf->data->hd, 0, 0, w, h, AllPlanes, ZPixmap);

        if (axi) {
            image.setAlphaBuffer(true);
            alpha.create(w, h, 8);

            // copy each scanline
            char *src = axi->data;
            int bpl = qMin(alpha.bytesPerLine(), axi->bytes_per_line);
            for (int y = 0; y < h; y++) {
                memcpy(alpha.scanLine(y), src, bpl);
                src += axi->bytes_per_line;
            }

            qSafeXDestroyImage(axi);
        }
    } else if (msk) {
        image.setAlphaBuffer(true);
        alpha = msk->toImage();
    }
    bool ale = alpha.bitOrder() == QImage::LittleEndian;

    if (trucol) {                                // truecolor
        const uint red_mask         = (uint)visual->red_mask;
        const uint green_mask         = (uint)visual->green_mask;
        const uint blue_mask         = (uint)visual->blue_mask;
        const int  red_shift         = highest_bit(red_mask)   - 7;
        const int  green_shift = highest_bit(green_mask) - 7;
        const int  blue_shift         = highest_bit(blue_mask)  - 7;

        const uint red_bits    = n_bits(red_mask);
        const uint green_bits  = n_bits(green_mask);
        const uint blue_bits   = n_bits(blue_mask);

        static uint red_table_bits   = 0;
        static uint green_table_bits = 0;
        static uint blue_table_bits  = 0;

        if (red_bits < 8 && red_table_bits != red_bits) {
            build_scale_table(&red_scale_table, red_bits);
            red_table_bits = red_bits;
        }
        if (blue_bits < 8 && blue_table_bits != blue_bits) {
            build_scale_table(&blue_scale_table, blue_bits);
            blue_table_bits = blue_bits;
        }
        if (green_bits < 8 && green_table_bits != green_bits) {
            build_scale_table(&green_scale_table, green_bits);
            green_table_bits = green_bits;
        }

        int  r, g, b;

        QRgb  *dst;
        uchar *src;
        uint   pixel;
        int    bppc = xi->bits_per_pixel;

        if (bppc > 8 && xi->byte_order == LSBFirst)
            bppc++;

        for (int y=0; y<h; y++) {
            uchar* asrc = alf || msk ? alpha.scanLine(y) : 0;
            dst = (QRgb *)image.scanLine(y);
            src = (uchar *)xi->data + xi->bytes_per_line*y;
            for (int x=0; x<w; x++) {
                switch (bppc) {
                case 8:
                    pixel = *src++;
                    break;
                case 16:                        // 16 bit MSB
                    pixel = src[1] | (ushort)src[0] << 8;
                    src += 2;
                    break;
                case 17:                        // 16 bit LSB
                    pixel = src[0] | (ushort)src[1] << 8;
                    src += 2;
                    break;
                case 24:                        // 24 bit MSB
                    pixel = src[2] | (ushort)src[1] << 8 |
                            (uint)src[0] << 16;
                    src += 3;
                    break;
                case 25:                        // 24 bit LSB
                    pixel = src[0] | (ushort)src[1] << 8 |
                            (uint)src[2] << 16;
                    src += 3;
                    break;
                case 32:                        // 32 bit MSB
                    pixel = src[3] | (ushort)src[2] << 8 |
                            (uint)src[1] << 16 | (uint)src[0] << 24;
                    src += 4;
                    break;
                case 33:                        // 32 bit LSB
                    pixel = src[0] | (ushort)src[1] << 8 |
                            (uint)src[2] << 16 | (uint)src[3] << 24;
                    src += 4;
                    break;
                default:                        // should not really happen
                    x = w;                        // leave loop
                    y = h;
                    pixel = 0;                // eliminate compiler warning
                    qWarning("QPixmap::convertToImage: Invalid depth %d",
                              bppc);
                }
                if (red_shift > 0)
                    r = (pixel & red_mask) >> red_shift;
                else
                    r = (pixel & red_mask) << -red_shift;
                if (green_shift > 0)
                    g = (pixel & green_mask) >> green_shift;
                else
                    g = (pixel & green_mask) << -green_shift;
                if (blue_shift > 0)
                    b = (pixel & blue_mask) >> blue_shift;
                else
                    b = (pixel & blue_mask) << -blue_shift;

                if (red_bits < 8)
                    r = red_scale_table[r];
                if (green_bits < 8)
                    g = green_scale_table[g];
                if (blue_bits < 8)
                    b = blue_scale_table[b];

                if (alf) {
                    *dst++ = qRgba(r, g, b, asrc[x]);
                } else if (msk) {
                    if (ale) {
                        *dst++ = (asrc[x >> 3] & (1 << (x & 7)))
                                 ? qRgba(r, g, b, 0xff) : qRgba(r, g, b, 0x00);
                    } else {
                        *dst++ = (asrc[x >> 3] & (1 << (7 -(x & 7))))
                                 ? qRgba(r, g, b, 0xff) : qRgba(r, g, b, 0x00);
                    }
                } else {
                    *dst++ = qRgb(r, g, b);
                }
            }
        }
    } else if (xi->bits_per_pixel == d) {        // compatible depth
        char *xidata = xi->data;                // copy each scanline
        int bpl = qMin(image.bytesPerLine(),xi->bytes_per_line);
        for (int y=0; y<h; y++) {
            memcpy(image.scanLine(y), xidata, bpl);
            xidata += xi->bytes_per_line;
        }
    } else {
        /* Typically 2 or 4 bits display depth */
        qWarning("QPixmap::convertToImage: Display not supported (bpp=%d)",
                  xi->bits_per_pixel);
        image.reset();
        return image;
    }

    if (mono) {                                // bitmap
        image.setNumColors(2);
        image.setColor(0, qRgb(255,255,255));
        image.setColor(1, qRgb(0,0,0));
    } else if (!trucol) {                        // pixmap with colormap
        register uchar *p;
        uchar *end;
        uchar  use[256];                        // pixel-in-use table
        uchar  pix[256];                        // pixel translation table
        int    ncols, i, bpl;
        memset(use, 0, 256);
        memset(pix, 0, 256);
        bpl = image.bytesPerLine();

        if (msk) {                                // which pixels are used?
            for (i=0; i<h; i++) {
                uchar* asrc = alpha.scanLine(i);
                p = image.scanLine(i);
                for (int x = 0; x < w; x++) {
                    if (ale) {
                        if (asrc[x >> 3] & (1 << (x & 7)))
                            use[*p] = 1;
                    } else {
                        if (asrc[x >> 3] & (1 << (7 -(x & 7))))
                            use[*p] = 1;
                    }
                    ++p;
                }
            }
        } else {
            for (i=0; i<h; i++) {
                p = image.scanLine(i);
                end = p + bpl;
                while (p < end)
                    use[*p++] = 1;
            }
        }
        ncols = 0;
        for (i=0; i<256; i++) {                // build translation table
            if (use[i])
                pix[i] = ncols++;
        }
        for (i=0; i<h; i++) {                        // translate pixels
            p = image.scanLine(i);
            end = p + bpl;
            while (p < end) {
                *p = pix[*p];
                p++;
            }
        }
        if (msk) {
            int trans;
            if (ncols < 256) {
                trans = ncols++;
                image.setNumColors(ncols);        // create color table
                image.setColor(trans, 0x00000000);
            } else {
                image.setNumColors(ncols);        // create color table
                // oh dear... no spare "transparent" pixel.
                // use first pixel in image (as good as any).
                trans = image.scanLine(i)[0];
            }
            for (i=0; i<h; i++) {
                uchar* asrc = alpha.scanLine(i);
                p = image.scanLine(i);
                for (int x = 0; x < w; x++) {
                    if (ale) {
                        if (!(asrc[x >> 3] & (1 << (x & 7))))
                            *p = trans;
                    } else {
                        if (!(asrc[x >> 3] & (1 << (7 -(x & 7)))))
                            *p = trans;
                    }
                    ++p;
                }
            }
        } else {
            image.setNumColors(ncols);        // create color table
        }
        QVector<QColor> colors = QColormap::instance(data->xinfo.screen()).colormap();
        int j = 0;
        for (i=0; i<colors.size(); i++) {                // translate pixels
            if (use[i])
                image.setColor(j++, (msk ? 0xff000000 : 0) | (colors.at(i).rgb() & 0x00ffffff));
        }
    }
    if (data->optim != BestOptim) {                // throw away image data
        qSafeXDestroyImage(xi);
        ((QPixmap*)this)->data->ximage = 0;
    } else                                        // keep ximage data
        ((QPixmap*)this)->data->ximage = xi;

    return image;
}


/*!
    Converts image \a img and sets this pixmap. Returns true if
    successful; otherwise returns false.

    The \a flags argument is a bitwise-OR of the
    \l{Qt::ImageConversionFlags}. Passing 0 for \a flags sets all the
    default options.

    Note that even though a QPixmap with depth 1 behaves much like a
    QBitmap, isQBitmap() returns false.

    If a pixmap with depth 1 is painted with Qt::color0 and Qt::color1 and
    converted to an image, the pixels painted with Qt::color0 will produce
    pixel index 0 in the image and those painted with Qt::color1 will
    produce pixel index 1.

    \sa convertToImage(), isQBitmap(), QImage::convertDepth(),
    defaultDepth(), QImage::hasAlphaBuffer()
*/

bool QPixmap::fromImage(const QImage &img, Qt::ImageConversionFlags flags)
{
    if (img.isNull()) {
        qWarning("QPixmap::convertFromImage: Cannot convert a null image");
        return false;
    }
    detach();                                        // detach other references
    QImage  image = img;
    const uint         w   = image.width();
    const uint         h   = image.height();
    int         d   = image.depth();
    const int         dd  = data->xinfo.depth();
    bool force_mono = (dd == 1 || isQBitmap() ||
                       (flags & Qt::ColorMode_Mask)==Qt::MonoOnly);

    // get rid of the mask
    delete data->mask;
    data->mask = 0;

    // get rid of alpha pixmap
    delete data->alphapm;
    data->alphapm = 0;

    // must be monochrome
    if (force_mono) {
        if (d != 1) {
            // dither
            image = image.convertDepth(1, flags);
            d = 1;
        }
    } else {                                        // can be both
        bool conv8 = false;
        if (d > 8 && dd <= 8) {                // convert to 8 bit
            if ((flags & Qt::DitherMode_Mask) == Qt::AutoDither)
                flags = (flags & ~Qt::DitherMode_Mask)
                                   | Qt::PreferDither;
            conv8 = true;
        } else if ((flags & Qt::ColorMode_Mask) == Qt::ColorOnly) {
            conv8 = d == 1;                        // native depth wanted
        } else if (d == 1) {
            if (image.numColors() == 2) {
                QRgb c0 = image.color(0);        // Auto: convert to best
                QRgb c1 = image.color(1);
                conv8 = qMin(c0,c1) != qRgb(0,0,0) || qMax(c0,c1) != qRgb(255,255,255);
            } else {
                // eg. 1-color monochrome images (they do exist).
                conv8 = true;
            }
        }
        if (conv8) {
            image = image.convertDepth(8, flags);
            d = 8;
        }
    }

    if (d == 1) {                                // 1 bit pixmap (bitmap)
        if (data->hd) {                                // delete old X pixmap

#ifndef QT_NO_XFT
            if (data->xft_hd) {
                XftDrawDestroy((XftDraw *) data->xft_hd);
                data->xft_hd = 0;
            }
#endif // QT_NO_XFT

            XFreePixmap(data->xinfo.display(), data->hd);
        }

        // make sure image.color(0) == Qt::color0 (white) and image.color(1) == Qt::color1 (black)
        const QRgb c0 = QColor(Qt::white).rgb();
        const QRgb c1 = QColor(Qt::black).rgb();
        if (image.color(0) == c1 && image.color(1) == c0) {
            image.invertPixels();
            image.setColor(0, c0);
            image.setColor(1, c1);
        }

        char  *bits;
        uchar *tmp_bits;
        int    bpl = (w+7)/8;
        int    ibpl = image.bytesPerLine();
        if (image.bitOrder() == QImage::BigEndian || bpl != ibpl) {
            tmp_bits = new uchar[bpl*h];
            bits = (char *)tmp_bits;
            uchar *p, *b, *end;
            uint y, count;
            if (image.bitOrder() == QImage::BigEndian) {
                const uchar *f = qt_get_bitflip_array();
                b = tmp_bits;
                for (y=0; y<h; y++) {
                    p = image.scanLine(y);
                    end = p + bpl;
                    count = bpl;
                    while (count > 4) {
                        *b++ = f[*p++];
                        *b++ = f[*p++];
                        *b++ = f[*p++];
                        *b++ = f[*p++];
                        count -= 4;
                    }
                    while (p < end)
                        *b++ = f[*p++];
                }
            } else {                                // just copy
                b = tmp_bits;
                p = image.scanLine(0);
                for (y=0; y<h; y++) {
                    memcpy(b, p, bpl);
                    b += bpl;
                    p += ibpl;
                }
            }
        } else {
            bits = (char *)image.bits();
            tmp_bits = 0;
        }
        data->hd = (Qt::HANDLE)XCreateBitmapFromData(data->xinfo.display(),
						 RootWindow(data->xinfo.display(), data->xinfo.screen()),
						 bits, w, h);

#ifndef QT_NO_XFT
        if (X11->has_xft)
            data->xft_hd = (Qt::HANDLE) XftDrawCreateBitmap(data->xinfo.display(), data->hd);
#endif // QT_NO_XFT

        if (tmp_bits)                                // Avoid purify complaint
            delete [] tmp_bits;
        data->w = w;  data->h = h;  data->d = 1;

        if (image.hasAlphaBuffer()) {
            QBitmap m;
            m = image.createAlphaMask(flags);
            setMask(m);
        }
        return true;
    }

    Display *dpy   = data->xinfo.display();
    Visual *visual = (Visual *) data->xinfo.visual();
    XImage *xi           = 0;
    bool    trucol = (visual->c_class >= TrueColor);
    uint    nbytes = image.numBytes();
    uchar  *newbits= 0;

    if (trucol) {                                // truecolor display
        QRgb  pix[256];                                // pixel translation table
        const bool  d8 = d == 8;
        const uint  red_mask          = (uint)visual->red_mask;
        const uint  green_mask  = (uint)visual->green_mask;
        const uint  blue_mask          = (uint)visual->blue_mask;
        const int   red_shift          = highest_bit(red_mask)   - 7;
        const int   green_shift = highest_bit(green_mask) - 7;
        const int   blue_shift  = highest_bit(blue_mask)  - 7;
        const uint  rbits = highest_bit(red_mask) - lowest_bit(red_mask) + 1;
        const uint  gbits = highest_bit(green_mask) - lowest_bit(green_mask) + 1;
        const uint  bbits = highest_bit(blue_mask) - lowest_bit(blue_mask) + 1;

        if (d8) {                                // setup pixel translation
            QRgb *ctable = image.colorTable();
            for (int i=0; i<image.numColors(); i++) {
                int r = qRed  (ctable[i]);
                int g = qGreen(ctable[i]);
                int b = qBlue (ctable[i]);
                r = red_shift        > 0 ? r << red_shift   : r >> -red_shift;
                g = green_shift > 0 ? g << green_shift : g >> -green_shift;
                b = blue_shift        > 0 ? b << blue_shift  : b >> -blue_shift;
                pix[i] = (b & blue_mask) | (g & green_mask) | (r & red_mask)
                         | ~(blue_mask | green_mask | red_mask);
            }
        }

        xi = XCreateImage(dpy, visual, dd, ZPixmap, 0, 0, w, h, 32, 0);
        Q_CHECK_PTR(xi);
        newbits = (uchar *)malloc(xi->bytes_per_line*h);
        Q_CHECK_PTR(newbits);
        if (!newbits)                                // no memory
            return false;
        int    bppc = xi->bits_per_pixel;

        if (bppc > 8 && xi->byte_order == LSBFirst)
            bppc++;

        bool contig_bits = n_bits(red_mask) == rbits &&
                           n_bits(green_mask) == gbits &&
                           n_bits(blue_mask) == bbits;
        bool dither_tc =
            // Want it?
            (flags & Qt::Dither_Mask) != Qt::ThresholdDither &&
            (flags & Qt::DitherMode_Mask) != Qt::AvoidDither &&
            // Need it?
            bppc < 24 && !d8 &&
            // Can do it? (Contiguous bits?)
            contig_bits;

        static bool init=false;
        static int D[16][16];
        if (dither_tc && !init) {
            // I also contributed this code to XV - WWA.
            /*
              The dither matrix, D, is obtained with this formula:

              D2 = [0 2]
              [3 1]


              D2*n = [4*Dn       4*Dn+2*Un]
              [4*Dn+3*Un  4*Dn+1*Un]
            */
            int n,i,j;
            init=1;

            /* Set D2 */
            D[0][0]=0;
            D[1][0]=2;
            D[0][1]=3;
            D[1][1]=1;

            /* Expand using recursive definition given above */
            for (n=2; n<16; n*=2) {
                for (i=0; i<n; i++) {
                    for (j=0; j<n; j++) {
                        D[i][j]*=4;
                        D[i+n][j]=D[i][j]+2;
                        D[i][j+n]=D[i][j]+3;
                        D[i+n][j+n]=D[i][j]+1;
                    }
                }
            }
            init=true;
        }

        for (uint y=0; y<h; y++) {
            uchar* src = image.scanLine(y);
            uchar* dst = newbits + xi->bytes_per_line*y;
            QRgb* p = (QRgb *)src;

#define GET_RGB \
                int r = qRed  (*p); \
                int g = qGreen(*p); \
                int b = qBlue (*p++); \
                r = red_shift   > 0 \
                    ? r << red_shift   : r >> -red_shift; \
                g = green_shift > 0 \
                    ? g << green_shift : g >> -green_shift; \
                b = blue_shift  > 0 \
                    ? b << blue_shift  : b >> -blue_shift;

#define GET_PIXEL \
                int pixel; \
                if (d8) pixel = pix[*src++]; \
                else { \
                    GET_RGB \
                    pixel = (b & blue_mask)|(g & green_mask) | (r & red_mask) \
                            | ~(blue_mask | green_mask | red_mask); \
                }

#define GET_PIXEL_DITHER_TC \
                int r = qRed  (*p); \
                int g = qGreen(*p); \
                int b = qBlue (*p++); \
                const int thres = D[x%16][y%16]; \
                if (r <= (255-(1<<(8-rbits))) && ((r<<rbits) & 255) \
                        > thres) \
                    r += (1<<(8-rbits)); \
                if (g <= (255-(1<<(8-gbits))) && ((g<<gbits) & 255) \
                        > thres) \
                    g += (1<<(8-gbits)); \
                if (b <= (255-(1<<(8-bbits))) && ((b<<bbits) & 255) \
                        > thres) \
                    b += (1<<(8-bbits)); \
                r = red_shift   > 0 \
                    ? r << red_shift   : r >> -red_shift; \
                g = green_shift > 0 \
                    ? g << green_shift : g >> -green_shift; \
                b = blue_shift  > 0 \
                    ? b << blue_shift  : b >> -blue_shift; \
                int pixel = (b & blue_mask)|(g & green_mask) | (r & red_mask);

            if (dither_tc) {
                uint x;
                switch (bppc) {
                case 16:                        // 16 bit MSB
                    for (x=0; x<w; x++) {
                        GET_PIXEL_DITHER_TC
                        *dst++ = (pixel >> 8);
                        *dst++ = pixel;
                    }
                    break;
                case 17:                        // 16 bit LSB
                    for (x=0; x<w; x++) {
                        GET_PIXEL_DITHER_TC
                        *dst++ = pixel;
                        *dst++ = pixel >> 8;
                    }
                    break;
                default:
                    qFatal("Logic error");
                }
            } else {
                uint x;
                switch (bppc) {
                case 8:                        // 8 bit
                    for (x=0; x<w; x++) {
                        int pixel = pix[*src++];
                        *dst++ = pixel;
                    }
                    break;
                case 16:                        // 16 bit MSB
                    for (x=0; x<w; x++) {
                        GET_PIXEL
                        *dst++ = (pixel >> 8);
                        *dst++ = pixel;
                    }
                    break;
                case 17:                        // 16 bit LSB
                    for (x=0; x<w; x++) {
                        GET_PIXEL
                        *dst++ = pixel;
                        *dst++ = pixel >> 8;
                    }
                    break;
                case 24:                        // 24 bit MSB
                    for (x=0; x<w; x++) {
                        GET_PIXEL
                        *dst++ = pixel >> 16;
                        *dst++ = pixel >> 8;
                        *dst++ = pixel;
                    }
                    break;
                case 25:                        // 24 bit LSB
                    for (x=0; x<w; x++) {
                        GET_PIXEL
                        *dst++ = pixel;
                        *dst++ = pixel >> 8;
                        *dst++ = pixel >> 16;
                    }
                    break;
                case 32:                        // 32 bit MSB
                    for (x=0; x<w; x++) {
                        GET_PIXEL
                        *dst++ = pixel >> 24;
                        *dst++ = pixel >> 16;
                        *dst++ = pixel >> 8;
                        *dst++ = pixel;
                    }
                    break;
                case 33:                        // 32 bit LSB
                    for (x=0; x<w; x++) {
                        GET_PIXEL
                        *dst++ = pixel;
                        *dst++ = pixel >> 8;
                        *dst++ = pixel >> 16;
                        *dst++ = pixel >> 24;
                    }
                    break;
                default:
                    qFatal("Logic error 2");
                }
            }
        }
        xi->data = (char *)newbits;
    }

    if (d == 8 && !trucol) {                        // 8 bit pixmap
        int  pop[256];                                // pixel popularity

        if (image.numColors() == 0)
            image.setNumColors(1);

        memset(pop, 0, sizeof(int)*256);        // reset popularity array
        uint i;
        for (i=0; i<h; i++) {                        // for each scanline...
            uchar* p = image.scanLine(i);
            uchar *end = p + w;
            while (p < end)                        // compute popularity
                pop[*p++]++;
        }

        newbits = (uchar *)malloc(nbytes);        // copy image into newbits
        Q_CHECK_PTR(newbits);
        if (!newbits)                                // no memory
            return false;
        uchar* p = newbits;
        memcpy(p, image.bits(), nbytes);        // copy image data into newbits

        /*
         * The code below picks the most important colors. It is based on the
         * diversity algorithm, implemented in XV 3.10. XV is (C) by John Bradley.
         */

        struct PIX {                                // pixel sort element
            uchar r,g,b,n;                        // color + pad
            int          use;                                // popularity
            int          index;                        // index in colormap
            int          mindist;
        };
        int ncols = 0;
        for (i=0; i< (uint) image.numColors(); i++) { // compute number of colors
            if (pop[i] > 0)
                ncols++;
        }
        for (i=image.numColors(); i<256; i++) // ignore out-of-range pixels
            pop[i] = 0;

        // works since we make sure above to have at least
        // one color in the image
        if (ncols == 0)
            ncols = 1;

        PIX pixarr[256];                        // pixel array
        PIX pixarr_sorted[256];                        // pixel array (sorted)
        memset(pixarr, 0, ncols*sizeof(PIX));
        PIX *px                   = &pixarr[0];
        int  maxpop = 0;
        int  maxpix = 0;
        uint j = 0;
        QRgb* ctable = image.colorTable();
        for (i=0; i<256; i++) {                // init pixel array
            if (pop[i] > 0) {
                px->r = qRed  (ctable[i]);
                px->g = qGreen(ctable[i]);
                px->b = qBlue (ctable[i]);
                px->n = 0;
                px->use = pop[i];
                if (pop[i] > maxpop) {        // select most popular entry
                    maxpop = pop[i];
                    maxpix = j;
                }
                px->index = i;
                px->mindist = 1000000;
                px++;
                j++;
            }
        }
        pixarr_sorted[0] = pixarr[maxpix];
        pixarr[maxpix].use = 0;

        for (i=1; i< (uint) ncols; i++) {                // sort pixels
            int minpix = -1, mindist = -1;
            px = &pixarr_sorted[i-1];
            int r = px->r;
            int g = px->g;
            int b = px->b;
            int dist;
            if ((i & 1) || i<10) {                // sort on max distance
                for (int j=0; j<ncols; j++) {
                    px = &pixarr[j];
                    if (px->use) {
                        dist = (px->r - r)*(px->r - r) +
                               (px->g - g)*(px->g - g) +
                               (px->b - b)*(px->b - b);
                        if (px->mindist > dist)
                            px->mindist = dist;
                        if (px->mindist > mindist) {
                            mindist = px->mindist;
                            minpix = j;
                        }
                    }
                }
            } else {                                // sort on max popularity
                for (int j=0; j<ncols; j++) {
                    px = &pixarr[j];
                    if (px->use) {
                        dist = (px->r - r)*(px->r - r) +
                               (px->g - g)*(px->g - g) +
                               (px->b - b)*(px->b - b);
                        if (px->mindist > dist)
                            px->mindist = dist;
                        if (px->use > mindist) {
                            mindist = px->use;
                            minpix = j;
                        }
                    }
                }
            }
            pixarr_sorted[i] = pixarr[minpix];
            pixarr[minpix].use = 0;
        }

        QColormap cmap = QColormap::instance(data->xinfo.screen());
        uint pix[256];                                // pixel translation table
        px = &pixarr_sorted[0];
        for (i=0; i< (uint) ncols; i++) {                // allocate colors
            QColor c(px->r, px->g, px->b);
            pix[px->index] = cmap.pixel(c);
            px++;
        }

        p = newbits;
        for (i=0; i< (uint) nbytes; i++) {                // translate pixels
            *p = pix[*p];
            p++;
        }
    }

    if (!xi) {                                // X image not created
        xi = XCreateImage(dpy, visual, dd, ZPixmap, 0, 0, w, h, 32, 0);
        if (xi->bits_per_pixel == 16) {        // convert 8 bpp ==> 16 bpp
            ushort *p2;
            int            p2inc = xi->bytes_per_line/sizeof(ushort);
            ushort *newerbits = (ushort *)malloc(xi->bytes_per_line * h);
            Q_CHECK_PTR(newerbits);
            if (!newerbits)                                // no memory
                return false;
            uchar* p = newbits;
            for (uint y=0; y<h; y++) {                // OOPS: Do right byte order!!
                p2 = newerbits + p2inc*y;
                for (uint x=0; x<w; x++)
                    *p2++ = *p++;
            }
            free(newbits);
            newbits = (uchar *)newerbits;
        } else if (xi->bits_per_pixel != 8) {
            qWarning("QPixmap::convertFromImage: Display not supported "
                      "(bpp=%d)", xi->bits_per_pixel);
        }
        xi->data = (char *)newbits;
    }

    if (data->hd && (width() != (int)w || height() != (int)h || this->depth() != dd)) {

#ifndef QT_NO_XFT
        if (data->xft_hd) {
            XftDrawDestroy((XftDraw *) data->xft_hd);
            data->xft_hd = 0;
        }
#endif // QT_NO_XFT

        XFreePixmap(dpy, data->hd);                        // don't reuse old pixmap
        data->hd = 0;
    }
    if (!data->hd) {                                        // create new pixmap
        data->hd = (Qt::HANDLE)XCreatePixmap(data->xinfo.display(),
					 RootWindow(data->xinfo.display(), data->xinfo.screen()),
					 w, h, dd);

#ifndef QT_NO_XFT
        if (X11->has_xft) {
            if (data->d == 1) {
                data->xft_hd = (Qt::HANDLE) XftDrawCreateBitmap(data->xinfo.display (), data->hd);
            } else {
                data->xft_hd = (Qt::HANDLE) XftDrawCreate(data->xinfo.display (), data->hd,
                                                          (Visual *) data->xinfo.visual(),
                                                          data->xinfo.colormap());
            }
        }
#endif // QT_NO_XFT

    }

    XPutImage(dpy, data->hd, qt_xget_readonly_gc(data->xinfo.screen(), false ),
	      xi, 0, 0, 0, 0, w, h);

    if (data->optim != BestOptim) {                // throw away image
        qSafeXDestroyImage(xi);
        data->ximage = 0;
    } else {                                        // keep ximage that we created
        data->ximage = xi;
    }
    data->w = w;
    data->h = h;
    data->d = dd;

    if (image.hasAlphaBuffer()) {
        QBitmap m;
        m = image.createAlphaMask(flags);
        setMask(m);

#ifndef QT_NO_XFT
        // does this image have an alphamap (and not just a 1bpp mask)?
        bool alphamap = image.depth() == 32;
        if (image.depth() == 8) {
            const QRgb * const rgb = image.colorTable();
            for (int i = 0, count = image.numColors(); i < count; ++i) {
                const int alpha = qAlpha(rgb[i]);
                if (alpha != 0 && alpha != 0xff) {
                    alphamap = true;
                    break;
                }
            }
        }

        if (X11->use_xrender && X11->has_xft && alphamap) {
            data->alphapm = new QPixmap; // create a null pixmap

            // setup pixmap data
            data->alphapm->data->w = w;
            data->alphapm->data->h = h;
            data->alphapm->data->d = 8;

            // create 8bpp pixmap and render picture
            data->alphapm->data->hd =
                XCreatePixmap(data->xinfo.display(), RootWindow(data->xinfo.display(), data->xinfo.screen()),
                              w, h, 8);

            data->alphapm->data->xft_hd =
                (Qt::HANDLE)
                XftDrawCreateAlpha(data->xinfo.display(), data->alphapm->data->hd, 8);

            XImage *axi = XCreateImage(data->xinfo.display(), (Visual *) data->xinfo.visual(),
                                       8, ZPixmap, 0, 0, w, h, 8, 0);

            if (axi) {
                // the data is deleted by qSafeXDestroyImage
                axi->data = (char *) malloc(h * axi->bytes_per_line);
                Q_CHECK_PTR(axi->data);
                char *aptr = axi->data;

                if (image.depth() == 32) {
                    const int *iptr = (const int *) image.bits();
                    int max = w * h;
                    while (max--)
                        *aptr++ = *iptr++ >> 24; // squirt
                } else if (image.depth() == 8) {
                    const QRgb * const rgb = image.colorTable();
                    for (uint y = 0; y < h; ++y) {
                        const uchar *iptr = image.scanLine(y);
                        for (uint x = 0; x < w; ++x)
                            *aptr++ = qAlpha(rgb[*iptr++]);
                    }
                }

                GC gc = XCreateGC(data->xinfo.display(), data->alphapm->data->hd, 0, 0);
                XPutImage(dpy, data->alphapm->data->hd, gc, axi, 0, 0, 0, 0, w, h);
                XFreeGC(data->xinfo.display(), gc);
                qSafeXDestroyImage(axi);
            }
        }
#endif // QT_NO_XFT
    }

    return true;
}


/*!
    Grabs the contents of the window \a window and makes a pixmap out
    of it. Returns the pixmap.

    The arguments (\a{x}, \a{y}) specify the offset in the window,
    whereas (\a{w}, \a{h}) specify the width and height of the area to
    be copied.

    If \a w is negative, the function copies everything to the right
    border of the window. If \a h is negative, the function copies
    everything to the bottom of the window.

    Note that grabWindow() grabs pixels from the screen, not from the
    window. If there is another window partially or entirely over the
    one you grab, you get pixels from the overlying window, too.

    Note also that the mouse cursor is generally not grabbed.

    The reason we use a window identifier and not a QWidget is to
    enable grabbing of windows that are not part of the application,
    window system frames, and so on.

    \warning Grabbing an area outside the screen is not safe in
    general. This depends on the underlying window system.

    \warning X11 only: If \a window is not the same depth as the root
    window and another window partially or entirely obscures the one
    you grab, you will \e not get pixels from the overlying window.
    The contests of the obscured areas in the pixmap are undefined and
    uninitialized.

    \sa grabWidget()
*/

QPixmap QPixmap::grabWindow(WId window, int x, int y, int w, int h)
{
    if (w == 0 || h == 0)
        return QPixmap();

    Display *dpy = X11->display;
    XWindowAttributes window_attr;
    if (! XGetWindowAttributes(dpy, window, &window_attr))
        return QPixmap();

    if (w < 0)
        w = window_attr.width - x;
    if (h < 0)
        h = window_attr.height - y;

    // determine the screen
    int scr;
    for (scr = 0; scr < ScreenCount(dpy); ++scr) {
        if (window_attr.root == RootWindow(dpy, scr))        // found it
            break;
    }
    if (scr >= ScreenCount(dpy))                // sanity check
        return QPixmap();


    // get the depth of the root window
    XWindowAttributes root_attr;
    if (! XGetWindowAttributes(dpy, window_attr.root, &root_attr))
        return QPixmap();

    if (window_attr.depth == root_attr.depth) {
        // if the depth of the specified window and the root window are the
        // same, grab pixels from the root window (so that we get the any
        // overlapping windows and window manager frames)

        // map x and y to the root window
        WId unused;
        if (! XTranslateCoordinates(dpy, window, window_attr.root, x, y,
                                      &x, &y, &unused))
            return QPixmap();

       window = window_attr.root;
    }

    QPixmap pm(w, h);
    pm.data->uninit = false;
    pm.x11SetScreen(scr);

    GC gc = qt_xget_temp_gc(scr, false);
    XSetSubwindowMode(dpy, gc, IncludeInferiors);
    XCopyArea(dpy, window, pm.handle(), gc, x, y, w, h, 0, 0);
    XSetSubwindowMode(dpy, gc, ClipByChildren);

    return pm;
}

/*!
    Returns a copy of the pixmap that is transformed using \a matrix.
    The original pixmap is not changed.

    The transformation \a matrix is internally adjusted to compensate
    for unwanted translation, i.e. transform() returns the smallest image
    that contains all the transformed points of the original image.

    This function is slow because it involves transformation to a
    QImage, non-trivial computations and a transformation back to a
    QPixmap.

    \sa trueMatrix(), QMatrix, QPainter::setWorldMatrix() QImage::transform()
*/

QPixmap QPixmap::transform(const QMatrix &matrix, Qt::TransformationMode mode) const
{
    int           w = 0;
    int           h = 0;                                // size of target pixmap
    int           ws, hs;                                // size of source pixmap
    uchar *dptr;                                // data in target pixmap
    int           dbpl, dbytes;                        // bytes per line/bytes total
    uchar *sptr;                                // data in original pixmap
    int           sbpl;                                // bytes per line in original
    int           bpp;                                        // bits per pixel
    bool   depth1 = depth() == 1;
    Display *dpy = data->xinfo.display();

    if (isNull())                                // this is a null pixmap
        return copy();

    ws = width();
    hs = height();

    QMatrix mat(matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(), 0., 0.);

    if (matrix.m12() == 0.0F && matrix.m21() == 0.0F) {
        if (matrix.m11() == 1.0F && matrix.m22() == 1.0F)
            return *this;                        // identity matrix
        h = qRound(matrix.m22()*hs);
        w = qRound(matrix.m11()*ws);
        h = qAbs(h);
        w = qAbs(w);
    } else {                                        // rotation or shearing
        QPolygon a(QRect(0,0,ws+1,hs+1));
        a = mat.map(a);
        QRect r = a.boundingRect().normalize();
        w = r.width()-1;
        h = r.height()-1;
    }

    mat = trueMatrix(mat, ws, hs); // true matrix


    bool invertible;
    mat = mat.invert(&invertible);                // invert matrix

    if (h == 0 || w == 0 || !invertible) {        // error, return null pixmap
        QPixmap pm;
        pm.data->bitmap = data->bitmap;
        return pm;
    }

    // ### enable this when the X server manages to do it quicker than we can
#if 0 //!defined(QT_NO_XFT) && !defined(QT_NO_XRENDER)
    if (X11->has_xft && X11->use_xrender && data->xft_hd) {
        // server side pixmap transformations using RENDER
        QPixmap result(w, h, depth());
        XTransform xtransform = {{
                { XDoubleToFixed(mat.m11()), XDoubleToFixed(mat.m21()), XDoubleToFixed(mat.dx()) },
                { XDoubleToFixed(mat.m12()), XDoubleToFixed(mat.m22()), XDoubleToFixed(mat.dy()) },
                { 0, 0, XDoubleToFixed(1.) }
            }};
        XTransform xunity = {{
                { XDoubleToFixed(1.), 0, 0 },
                { 0, XDoubleToFixed(1.), 0 },
                { 0, 0, XDoubleToFixed(1.) }
            }};

        ::Picture src = XftDrawPicture((XftDraw *) data->xft_hd);
        ::Picture dst = result.xftPictureHandle();
        XRenderSetPictureTransform (dpy, src, &xtransform);
        XRenderSetPictureFilter(dpy, src,
                                (char *)(mode == Qt::SmoothTransformation
                                         ? "bilinear"
                                         : "nearest"),
                                0, 0);
        XRenderComposite(dpy, PictOpSrc, src, XNone, dst, 0, 0, 0, 0, 0, 0, w, h);
        XRenderSetPictureTransform(dpy, src, &xunity);

        // create an alpha pixmap
        result.data->alphapm = new QPixmap;

        // setup pixmap data
        result.data->alphapm->data->w = w;
        result.data->alphapm->data->h = h;
        result.data->alphapm->data->d = 8;

        // create 8bpp pixmap and render picture
        result.data->alphapm->data->hd =
            XCreatePixmap(dpy, RootWindow(dpy, result.data->xinfo.screen()), w, h, 8);
        result.data->alphapm->data->xft_hd =
            (Qt::HANDLE) XftDrawCreateAlpha(dpy, result.data->alphapm->data->hd, 8);

        ::Picture adst = result.data->alphapm->xftPictureHandle();

        XRenderColor clear = { 0x0000, 0x0000, 0x0000, 0x0000 };
        XRenderFillRectangle(dpy, PictOpSrc, adst, &clear, 0, 0, w, h);

        Pixmap alphamap;
        XftDraw *xftdraw;
        ::Picture asrc;
        if (data->alphapm) {
            // transform existing alpha map
            asrc = data->alphapm->xftPictureHandle();
        } else {
            // create temporary alpha map
            alphamap =
                XCreatePixmap(dpy, RootWindow(dpy, result.data->xinfo.screen()), ws, hs, 8);
            xftdraw = XftDrawCreateAlpha(dpy, alphamap, 8);
            asrc = XftDrawPicture(xftdraw);

            if (data->mask) {
                // use existing mask as a starting point
                ::Picture mask = data->mask->xftPictureHandle();
                XRenderComposite(dpy, PictOpSrc, mask, 0, asrc, 0, 0, 0, 0, 0, 0, ws, hs);
            } else {
                // all pixels are fully opaque
                XRenderColor set = { 0xffff, 0xffff, 0xffff, 0xffff };
                XRenderFillRectangle(dpy, PictOpSrc, asrc, &set, 0, 0, ws, hs);
            }
        }

        XRenderSetPictureTransform (dpy, asrc, &xtransform);
        XRenderSetPictureFilter(dpy, asrc,
                                (char *)(mode == Qt::SmoothTransformation
                                         ? "bilinear"
                                         : "nearest"),
                                0, 0);
        XRenderComposite(dpy, PictOpSrc, asrc, XNone, adst, 0, 0, 0, 0, 0, 0, w, h);
        XRenderSetPictureTransform(dpy, asrc, &xunity);

        if (!data->alphapm) {
            // free temporary alpha map
            XftDrawDestroy(xftdraw);
            XFreePixmap(dpy, alphamap);
        }

        return result;
    }
#endif // !QT_NO_XFT && !QT_NO_XRENDER

    if (mode == Qt::SmoothTransformation) {
        QImage image = toImage();
        return QPixmap(image.transform(matrix, mode));
    }


#if defined(QT_MITSHM)
    static bool try_once = true;
    if (try_once) {
        try_once = false;
        if (!xshminit)
            qt_create_mitshm_buffer(this, 800, 600);
    }

    bool use_mitshm = xshmimg && !depth1 &&
    xshmimg->width >= w && xshmimg->height >= h;
#endif
    XImage *xi = (XImage*)data->ximage;                // any cached ximage?
    if (!xi)
        xi = XGetImage(data->xinfo.display(), handle(), 0, 0, ws, hs, AllPlanes,
                       depth1 ? XYPixmap : ZPixmap);

    if (!xi) {                                // error, return null pixmap
        QPixmap pm;
        pm.data->bitmap = data->bitmap;
        pm.data->alphapm = data->alphapm;
        return pm;
    }

    sbpl = xi->bytes_per_line;
    sptr = (uchar *)xi->data;
    bpp         = xi->bits_per_pixel;

    if (depth1)
        dbpl = (w+7)/8;
    else
        dbpl = ((w*bpp+31)/32)*4;
    dbytes = dbpl*h;

#if defined(QT_MITSHM)
    if (use_mitshm) {
        dptr = (uchar *)xshmimg->data;
        uchar fillbyte = bpp == 8 ? white.pixel() : 0xff;
        for (int y=0; y<h; y++)
            memset(dptr + y*xshmimg->bytes_per_line, fillbyte, dbpl);
    } else {
#endif
        dptr = (uchar *)malloc(dbytes);        // create buffer for bits
        Q_CHECK_PTR(dptr);
        if (depth1)                                // fill with zeros
            memset(dptr, 0, dbytes);
        else if (bpp == 8)                        // fill with background color
            memset(dptr, WhitePixel(data->xinfo.display(), data->xinfo.screen()), dbytes);
        else
            memset(dptr, 0xff, dbytes);
#if defined(QT_MITSHM)
    }
#endif

    // #define QT_DEBUG_XIMAGE
#if defined(QT_DEBUG_XIMAGE)
    qDebug("----IMAGE--INFO--------------");
    qDebug("width............. %d", xi->width);
    qDebug("height............ %d", xi->height);
    qDebug("xoffset........... %d", xi->xoffset);
    qDebug("format............ %d", xi->format);
    qDebug("byte order........ %d", xi->byte_order);
    qDebug("bitmap unit....... %d", xi->bitmap_unit);
    qDebug("bitmap bit order.. %d", xi->bitmap_bit_order);
    qDebug("depth............. %d", xi->depth);
    qDebug("bytes per line.... %d", xi->bytes_per_line);
    qDebug("bits per pixel.... %d", xi->bits_per_pixel);
#endif

    int type;
    if (xi->bitmap_bit_order == MSBFirst)
        type = QT_XFORM_TYPE_MSBFIRST;
    else
        type = QT_XFORM_TYPE_LSBFIRST;
    int        xbpl, p_inc;
    if (depth1) {
        xbpl  = (w+7)/8;
        p_inc = dbpl - xbpl;
    } else {
        xbpl  = (w*bpp)/8;
        p_inc = dbpl - xbpl;
#if defined(QT_MITSHM)
        if (use_mitshm)
            p_inc = xshmimg->bytes_per_line - xbpl;
#endif
    }

    if (!qt_xForm_helper(mat, xi->xoffset, type, bpp, dptr, xbpl, p_inc, h, sptr, sbpl, ws, hs)){
        qWarning("QPixmap::transform: display not supported (bpp=%d)",bpp);
        QPixmap pm;
        return pm;
    }

    if (data->optim == NoOptim) {                // throw away ximage
        qSafeXDestroyImage(xi);
        data->ximage = 0;
    } else {                                        // keep ximage that we fetched
        data->ximage = xi;
    }

    if (depth1) {                                // mono bitmap
        QPixmap pm(w, h, dptr, QImage::systemBitOrder() != QImage::BigEndian);
        pm.data->bitmap = data->bitmap;
        free(dptr);
        if (data->mask) {
            if (data->selfmask)                // pixmap == mask
                pm.setMask(*((QBitmap*)(&pm)));
            else
                pm.setMask(data->mask->transform(matrix));
        }
        return pm;
    } else {                                        // color pixmap
        GC gc = qt_xget_readonly_gc(data->xinfo.screen(), false);
        QPixmap pm(w, h);
        pm.data->uninit = false;
        pm.x11SetScreen(data->xinfo.screen());
#if defined(QT_MITSHM)
        if (use_mitshm) {
            XCopyArea(dpy, xshmpm, pm.handle(), gc, 0, 0, w, h, 0, 0);
        } else {
#endif
            xi = XCreateImage(dpy, (Visual *) data->xinfo.visual(), data->xinfo.depth(),
                              ZPixmap, 0, (char *)dptr, w, h, 32, 0);
            XPutImage(dpy, pm.handle(), gc, xi, 0, 0, 0, 0, w, h);
            qSafeXDestroyImage(xi);
#if defined(QT_MITSHM)
        }
#endif

        if (data->mask) // xform mask, too
            pm.setMask(data->mask->transform(matrix));

#ifndef QT_NO_XFT
        if (X11->use_xrender && X11->has_xft && data->alphapm) { // xform the alpha channel
            XImage *axi = 0;
            if ((axi = XGetImage(data->xinfo.display(), data->alphapm->handle(),
                                 0, 0, ws, hs, AllPlanes, ZPixmap))) {
                sbpl = axi->bytes_per_line;
                sptr = (uchar *) axi->data;
                bpp  = axi->bits_per_pixel;
                dbytes = dbpl * h;
                dptr = (uchar *) malloc(dbytes);
                Q_CHECK_PTR(dptr);
                memset(dptr, 0, dbytes);
                if (axi->bitmap_bit_order == MSBFirst)
                    type = QT_XFORM_TYPE_MSBFIRST;
                else
                    type = QT_XFORM_TYPE_LSBFIRST;

                if (qt_xForm_helper(mat, axi->xoffset, type, bpp, dptr, w,
                                    0, h, sptr, sbpl, ws, hs)) {
                    delete pm.data->alphapm;
                    pm.data->alphapm = new QPixmap; // create a null pixmap

                    // setup pixmap data
                    pm.data->alphapm->data->w = w;
                    pm.data->alphapm->data->h = h;
                    pm.data->alphapm->data->d = 8;

                    // create 8bpp pixmap and render picture
                    pm.data->alphapm->data->hd =
                        XCreatePixmap(data->xinfo.display(),
                                      RootWindow(data->xinfo.display(), data->xinfo.screen()),
                                      w, h, 8);

                    pm.data->alphapm->data->xft_hd =
                        (Qt::HANDLE) XftDrawCreateAlpha(data->xinfo.display(),
                                                        pm.data->alphapm->data->hd, 8);

                    XImage *axi2 = XCreateImage(data->xinfo.display(),
                                                (Visual *) data->xinfo.visual(),
                                                8, ZPixmap, 0, (char *)dptr, w, h, 8, 0);

                    if (axi2) {
                        // the data is deleted by qSafeXDestroyImage
                        GC gc = XCreateGC(data->xinfo.display(), pm.data->alphapm->data->hd, 0, 0);
                        XPutImage(dpy, pm.data->alphapm->data->hd, gc, axi2, 0, 0, 0, 0, w, h);
                        XFreeGC(data->xinfo.display(), gc);
                        qSafeXDestroyImage(axi2);
                    }
                }
                qSafeXDestroyImage(axi);
            }
        }
#endif // QT_NO_XFT

        return pm;
    }
}


/*!
  \internal
*/
int QPixmap::x11SetDefaultScreen(int screen)
{
    int old = defaultScreen;
    defaultScreen = screen;
    return old;
}

/*!
  \internal
*/
void QPixmap::x11SetScreen(int screen)
{
    if (screen < 0)
        screen = QX11Info::appScreen();

    if (screen == data->xinfo.screen())
        return; // nothing to do

    if (isNull()) {
        QX11InfoData* xd = data->xinfo.getX11Data(true);
        xd->screen = screen;
        xd->depth = QX11Info::appDepth(screen);
        xd->cells = QX11Info::appCells(screen);
        xd->colormap = QX11Info::appColormap(screen);
        xd->defaultColormap = QX11Info::appDefaultColormap(screen);
        xd->visual = (Visual *)QX11Info::appVisual(screen);
        xd->defaultVisual = QX11Info::appDefaultVisual(screen);
        data->xinfo.setX11Data(xd);
        return;
    }
#if 0
    qDebug("QPixmap::x11SetScreen for %p from %d to %d. Size is %d/%d", data, data->xinfo.screen(), screen, width(), height());
#endif

    QImage img = toImage();
    resize(0,0);
    QX11InfoData* xd = data->xinfo.getX11Data(true);
    xd->screen = screen;
    xd->depth = QX11Info::appDepth(screen);
    xd->cells = QX11Info::appCells(screen);
    xd->colormap = QX11Info::appColormap(screen);
    xd->defaultColormap = QX11Info::appDefaultColormap(screen);
    xd->visual = (Visual *)QX11Info::appVisual(screen);
    xd->defaultVisual = QX11Info::appDefaultVisual(screen);
    data->xinfo.setX11Data(xd);
    fromImage(img);
}

/*!
    Returns true this pixmap has an alpha channel or a mask.

    \sa hasAlphaChannel() mask()
*/
bool QPixmap::hasAlpha() const
{
    return data->alphapm || data->mask;
}

/*!
    Returns true if the pixmap has an alpha channel; otherwise it
    returns false.

    NOTE: If the pixmap has a mask but not alpha channel, this
    function returns false.

    \sa hasAlpha() mask()
*/
bool QPixmap::hasAlphaChannel() const
{
    return data->alphapm != 0;
}

/*!
    Returns a pointer to a QX11Info object. This pointer is owned by
    QPixmap and should not be deleted.
*/
const QX11Info &QPixmap::x11Info() const
{
    return data->xinfo;
}

QPaintEngine *QPixmap::paintEngine() const
{
    if (!data->paintEngine)
        data->paintEngine = new QX11PaintEngine();
    return data->paintEngine;
}

/*!
    Returns the Xft picture handle of the pixmap for XRender
    support. Use of this function is not portable. This function will
    return 0 if XRender support is not compiled into Qt, if the
    XRender extension is not supported on the X11 display, or if the
    handle could not be created.
*/

Qt::HANDLE QPixmap::xftPictureHandle() const
{
#ifndef QT_NO_XFT
    return data->xft_hd ? XftDrawPicture((XftDraw *) data->xft_hd) : 0;
#else
    return 0;
#endif // QT_NO_XFT
}

/*!
    Returns the Xft draw handle of the pixmap for XRender support. Use
    of this function is not portable. This function will return 0 if
    XRender support is not compiled into Qt, if the XRender extension
    is not supported on the X11 display, or if the handle could not be
    created.
*/

Qt::HANDLE QPixmap::xftDrawHandle() const
{
    return data->xft_hd;
}
