/****************************************************************************
**
** Implementation of QPixmap class for FB.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qglobal.h"

#include "qbitmap.h"
#include "qpaintdevicemetrics.h"
#include "qimage.h"
#include "qwmatrix.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qwsdisplay_qws.h"
#include "qgfx_qws.h"
#include "qhash.h"
#include <stdlib.h>
#include <limits.h>

#include "qmemorymanager_qws.h"
#include "qpaintengine_qws.h"


//#### HACK:
#include "private/qpainter_p.h"


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
/*
static int highest_bit(uint v)
{
    int i;
    uint b = (uint)1 << 31;
    for (i=31; ((b & v) == 0) && i>=0;         i--)
        b >>= 1;
    return i;
}
*/

// Returns position of lowest set bit in 'v' as an integer (0-31), or -1
/*
static int lowest_bit(uint v)
{
    int i;
    ulong lb;
    lb = 1;
    for (i=0; ((v & lb) == 0) && i<32;  i++, lb<<=1);
    return i==32 ? -1 : i;
}
*/

// Counts the number of bits set in 'v'
/*
static uint n_bits(uint v)
{
    int i = 0;
    while (v) {
        v = v & (v - 1);
        i++;
    }
    return i;
}
*/

/*
static uint *red_scale_table   = 0;
static uint *green_scale_table = 0;
static uint *blue_scale_table  = 0;

static void cleanup_scale_tables()
{
    delete[] red_scale_table;
    delete[] green_scale_table;
    delete[] blue_scale_table;
}
*/

/*
  Could do smart bitshifting, but the "obvious" algorithm only works for
  nBits >= 4. This is more robust.
*/
/*
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
*/

static QList<QSharedTemporary*> *qws_pixmapData = 0;
static bool qws_trackPixmapData = true;

class QwsPixmap : public QPixmap
{
public:
    QwsPixmap() : QPixmap() {}
    static void mapPixmaps(bool from);
    static QHash<QPixmapData*, QImage*> *images;
};

QHash<QPixmap::QPixmapData*, QImage*> *QwsPixmap::images = 0;

void QwsPixmap::mapPixmaps(bool from)
{
    if (!qws_pixmapData)
        return;
    if (!images)
        images = new QHash<QPixmapData*, QImage*>;
    qws_trackPixmapData = false;
    for (int i = 0; i < qws_pixmapData->size(); ++i) {
        QPixmapData *d = (QPixmapData*)qws_pixmapData->at(i);
        if (d->w && d->h) {
            if (from) {
                QwsPixmap p;
                QPixmapData *tmp = p.data;
                p.data = d;
                QImage *img = new QImage(p.convertToImage());
                images->insert(d, img);
                p.data = tmp;
            } else {
                QImage *img = images->take(d);
                if (img) {
                    if (d->clut)
                        delete [] d->clut;
                    if (memorymanager)
                        memorymanager->deletePixmap(d->id);
                    QwsPixmap p;
                    p.convertFromImage(*img);
                    int cnt = d->count-1;
                    p.data->mask = d->mask;
                    *d = *p.data;
                    while (cnt > 0) {
                        d->ref();
                        --cnt;
                    }
                    delete img;
                    delete p.data;
                    p.data = 0;
                }
            }
        }
    }
    if (!from)
        images->clear();
    qws_trackPixmapData = true;
}

void qws_mapPixmaps(bool from)
{
    QwsPixmap::mapPixmaps(from);
}

/*****************************************************************************
  QPixmap member functions
 *****************************************************************************/

void QPixmap::init(int w, int h, int d, bool bitmap, Optimization optim)
{
    static int serial = 0;
    int dd = defaultDepth();

    if (!qws_pixmapData)
        qws_pixmapData = new QList<QSharedTemporary*>;

    if (optim == DefaultOptim)                // use default optimization
        optim = defOptim;

    data = new QPixmapData;

    if (qws_trackPixmapData)
        qws_pixmapData->append(data);

    memset(data, 0, sizeof(QPixmapData));
    data->id=0;
    data->count  = 1;
    data->uninit = true;
    data->bitmap = bitmap;
    data->ser_no = ++serial;
    data->optim         = optim;
    data->clut=0;
    data->numcols = 0;
    data->hasAlpha = false;

    if (d > 0 && !qwsDisplay()->supportsDepth(d))
        d = dd; // caller asked for an unsupported depth

    bool make_null = w == 0 || h == 0;                // create null pixmap
    if (d == 1)                                // monocrome pixmap
        data->d = 1;
    else if (d < 0 || d == dd)                // def depth pixmap
        data->d = dd;
    else
        data->d = d;
    if (make_null || w < 0 || h < 0 || data->d == 0) {
        data->id = 0;
        data->w = 0;
        data->h = 0;
        if (!make_null) {
            qWarning("QPixmap: Invalid pixmap parameters, %d %d %d",w,h,data->d);
            abort();
        }
        return;
    }
    data->w = w;
    data->h = h;

    if(data->d<=8) {
        if (qt_screen->numCols()) {
            data->numcols = qt_screen->numCols();
            data->clut = new QRgb[qt_screen->numCols()];
            for (int i = 0; i < qt_screen->numCols(); i++)
                data->clut[i] = qt_screen->clut()[i];
        }
    }

    data->rw = qt_screen->mapToDevice(QSize(w,h)).width();
    data->rh = qt_screen->mapToDevice(QSize(w,h)).height();

    data->id=memorymanager->newPixmap(data->rw, data->rh, data->d, optim);
    if (data->id == 0)
        data->w = data->h = 0; // out of memory -- create null pixmap
}


void QPixmap::deref()
{
    if (data && data->deref()) {                        // last reference lost
        if (qws_trackPixmapData)
            qws_pixmapData->remove(data);
        if (data->mask)
            delete data->mask;
        if (data->clut)
            delete[] data->clut;

        if (memorymanager)
            memorymanager->deletePixmap(data->id);
        delete data;
        data = 0;
    }
}


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
    data->rw = qt_screen->mapToDevice(QSize(w,h)).width();
    data->rh = qt_screen->mapToDevice(QSize(w,h)).height();
    data->hasAlpha = false;
    uchar *flipped_bits;
    if (isXbitmap) {
        flipped_bits = 0;
    } else {                                        // not X bitmap -> flip bits
        flipped_bits = flip_bits(bits, ((w+7)/8)*h);
        bits = flipped_bits;
    }

    if (qt_screen->isTransformed()) {
        int bpl = isXbitmap ? (w+7)/8 : ((w+31)/32)*4;
        QImage img((uchar *)bits, w, h, 1, bpl, 0, 0, QImage::LittleEndian);
        convertFromImage(img, Qt::MonoOnly);
        if (flipped_bits)
            delete [] flipped_bits;
        return;
    }

    data->id=memorymanager->newPixmap(data->rw,data->rh,data->d, optimization());
    if (data->id == 0) {
        // out of memory -- create null pixmap.
        data->w = data->h = 0;
        return;
    }
    uchar *dest;
    int xoffset,linestep;
    memorymanager->findPixmap(data->id,data->rw,data->d,&dest,&xoffset,&linestep);

    Q_ASSERT((xoffset&7) == 0); // if not, we need to fix this to do a bitblt
    dest += xoffset/8;

    uchar *src = (uchar*)bits;
    for (int row = 0; row < h; row++)
    {
        memcpy(dest, src, (w+7)/8);
        dest += linestep;
        src += (w+7)/8;
    }

    if (flipped_bits)                                // Avoid purify complaint
        delete [] flipped_bits;
}


void QPixmap::detach()
{
    if (data->uninit || data->count == 1)
        data->uninit = false;
    else
        *this = copy();
}


int QPixmap::defaultDepth()
{
    QWSDisplay *d = qwsDisplay();
    int dd = d ? d->pixmapDepth() : 16;
    return dd;
}


void QPixmap::setOptimization(Optimization)
{
}


void QPixmap::fill(const QColor &fillColor)
{
    if (isNull())
        return;
    QPainter p(this);
    p.fillRect(rect(),fillColor);
}


int QPixmap::metric(int m) const
{
    int val;
    if (m == QPaintDeviceMetrics::PdmWidth) {
        val = width();
    } else if (m == QPaintDeviceMetrics::PdmWidthMM) {
        // 75 dpi is 3dpmm
        val = (width()*100)/288;
    } else if (m == QPaintDeviceMetrics::PdmHeight) {
        val = height();
    } else if (m == QPaintDeviceMetrics::PdmHeightMM) {
        val = (height()*100)/288;
    } else if (m == QPaintDeviceMetrics::PdmDpiX || m == QPaintDeviceMetrics::PdmPhysicalDpiX) {
        return 72;
    } else if (m == QPaintDeviceMetrics::PdmDpiY || m == QPaintDeviceMetrics::PdmPhysicalDpiY) {
        return 72;
    } else if(m ==  QPaintDeviceMetrics::PdmDepth) {
        val=depth();
    } else {
        // XXX
        val = QPaintDevice::metric(m);
    }
    return val;
}

QImage QPixmap::convertToImage() const
{
    QImage image;
    if (isNull()) {
        qWarning("QPixmap::convertToImage: Cannot convert a null pixmap");
        return image;
    }

    int w  = qt_screen->mapToDevice(QSize(width(), height())).width();
    int h  = qt_screen->mapToDevice(QSize(width(), height())).height();
    int        d  = depth();
    bool mono = d == 1;

    const QBitmap* msk = mask();

    if(d == 15 || d == 16) {
#ifndef QT_NO_QWS_DEPTH_16
        d = 32;
        // Convert here because we may not have a 32bpp gfx
        image.create(w,h,d,0, QImage::IgnoreEndian);
        for (int y=0; y < h; y++) {     // for each scan line...
            register uint *p = (uint *)image.scanLine(y);
            ushort  *s = (ushort*)scanLine(y);
            uint *end = p + w;
            if (msk) {
                uchar* a = msk->scanLine(y);
                uchar bit = 1; // mask is LittleEndian
                while (p < end) {
                    uint rgb = qt_conv16ToRgb(*s++);
                    if (!(*a & bit))
                        rgb &= 0x00ffffff;
                    *p++ = rgb;
                    if (!(bit <<= 1)) {
                        ++a;
                        bit = 1;
                    }
                }
            } else {
                while (p < end)
                    *p++ = qt_conv16ToRgb(*s++);
            }
        }
        if (msk)
            image.setAlphaBuffer(true);
#endif
    } else {
        // We can only create little-endian pixmaps
        if (d == 4)
            image.create(w,h,8,0, QImage::IgnoreEndian);
        else if (d == 24)
            image.create(w,h,32,0, QImage::IgnoreEndian);
        else
            image.create(w,h,d,0, mono ? QImage::LittleEndian : QImage::IgnoreEndian);//####### endianness

        QGfx * mygfx=image.graphicsContext();
        if(mygfx) {
            mygfx->setSource(this);
            mygfx->setAlphaType(QGfx::IgnoreAlpha);
            mygfx->setLineStep(image.bytesPerLine());
            mygfx->blt(0,0,width(),height(),0,0);
        } else {
            qWarning("No image gfx for convertToImage!");
        }
        delete mygfx;
        image.setAlphaBuffer(data->hasAlpha);
    }

    if (mono) {                                // bitmap
        image.setNumColors(2);
        image.setColor(0, qRgb(255,255,255));
        image.setColor(1, qRgb(0,0,0));
    } else if (d <= 8) {
        image.setNumColors(numCols());
        for (int i = 0; i < numCols(); i++)
            image.setColor(i, clut()[i]);
        if (mask()) {                                // which pixels are used?
            QImage alpha = mask()->convertToImage();
            alpha = qt_screen->mapToDevice(alpha);
            bool ale = alpha.bitOrder() == QImage::LittleEndian;
            register uchar *p;
            int  used[256];
            memset(used, 0, sizeof(int)*256);
            for (int i=0; i<h; i++) {
                uchar* asrc = alpha.scanLine(i);
                p = image.scanLine(i);
                for (int x = 0; x < w; x++) {
                    if (ale) {
                        if (asrc[x >> 3] & (1 << (x & 7)))
                            used[*p]++;
                    } else {
                        if (asrc[x >> 3] & (1 << (7 -(x & 7))))
                            used[*p]++;
                    }
                    ++p;
                }
            }

            int trans=0;
            int bestn=INT_MAX;
            for (int i=0; i<numCols(); i++) {
                if (used[i] < bestn) {
                    bestn = used[i];
                    trans = i;
                    if (!bestn)
                        break;
                }
            }

            image.setColor(trans, image.color(trans)&0x00ffffff);

            for (int i=0; i<h; i++) {
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
            image.setAlphaBuffer(true);
        }
    } else if (d == 32 && mask()) {
        QImage alpha = mask()->convertToImage();
        bool ale = alpha.bitOrder() == QImage::LittleEndian;
        for (int i=0; i<h; i++) {
            uchar* asrc = alpha.scanLine(i);
            Q_UINT32 *p = (Q_UINT32 *)image.scanLine(i);
            for (int x = 0; x < w; x++) {
                if (ale) {
                    if (!(asrc[x >> 3] & (1 << (x & 7))))
                        *p = *p & 0x00ffffff;
                } else {
                    if (!(asrc[x >> 3] & (1 << (7 -(x & 7)))))
                        *p = *p & 0x00ffffff;
                }
                ++p;
            }
        }
        image.setAlphaBuffer(true);
    }

    image = qt_screen->mapFromDevice(image);

    return image;
}

bool QPixmap::convertFromImage(const QImage &img, int conversion_flags)
{
    if (img.isNull()) {
        qWarning("QPixmap::convertFromImage: Cannot convert a null image");
        return false;
    }

    QImage  image = img;
    int         w   = image.width();
    int         h   = image.height();
    int         d   = image.depth();        // source depth
    int         dd  = defaultDepth();        //destination depth
    bool force_mono = (dd == 1 || isQBitmap() ||
                       (conversion_flags & Qt::ColorMode_Mask)==Qt::MonoOnly);

    if (force_mono) {                                // must be monochrome
        if (d != 1) {
            image = image.convertDepth(1, conversion_flags);        // dither
            d = 1;
        }
    } else {                                        // can be both
        bool conv8 = false;
        if (d > 8 && dd <= 8) {                // convert to 8 bit
            if ((conversion_flags & Qt::DitherMode_Mask) == Qt::AutoDither)
                conversion_flags = (conversion_flags & ~Qt::DitherMode_Mask)
                                        | Qt::PreferDither;
            conv8 = true;
        } else if ((conversion_flags & Qt::ColorMode_Mask) == Qt::ColorOnly) {
            conv8 = d == 1;                        // native depth wanted
        } else if (d == 1) {
            if (image.numColors() == 2) {
                QRgb c0 = image.color(0);        // Auto: convert to best
                QRgb c1 = image.color(1);
                conv8 = qMin(c0,c1) != qRgb(0,0,0) || qMax(c0,c1) != qRgb(255,255,255);
                if (!conv8)
                    force_mono = true;
            } else {
                // eg. 1-color monochrome images (they do exist).
                conv8 = true;
            }
        }
        if (conv8) {
            image = image.convertDepth(8, conversion_flags);
            d = 8;
        }
    }

    if(image.depth()==1) {
        if(image.bitOrder()==QImage::BigEndian) {
            image=image.convertBitOrder(QImage::LittleEndian);
        }
    }

    if (force_mono)
        dd = 1;

    bool manycolors=(qt_screen->depth() > 8);

    bool partialalpha=false;

    QWSDisplay *dpy = qwsDisplay();
    if(image.hasAlphaBuffer() && dpy->supportsDepth(32) && dd>8 && manycolors) {
        if (image.depth()==8) {
            for (int i=0; i<image.numColors(); i++) {
                int t = qAlpha(image.color(i));
                if (t>0 && t<255) {
                    partialalpha = true;
                    break;
                }
            }
        } else if (image.depth()==32) {
            int loopc,loopc2;
            for (loopc=0;loopc<image.height();loopc++) {
                QRgb * tmp=(QRgb *)image.scanLine(loopc);
                for(loopc2=0;loopc2<image.width();loopc2++) {
                    int t=qAlpha(*tmp++);
                    if(t>0 && t<255) {
                        partialalpha=true;
                        loopc2=image.width();
                        loopc=image.height();
                    }
                }
            }
        }
        if (partialalpha)
            dd=32;
    }

    // for drivers that do not accelerate alpha blt
    Optimization optim = partialalpha ? NoOptim : defOptim;

    QImage rimg = qt_screen->mapToDevice(image);

    // detach other references and re-init()
    bool ibm = isQBitmap();
    detach();
    deref();
    init(w, h, dd, ibm, optim);

    //##### HACK to get to the gfx #####
    QPaintEngine *p = engine();
    QPainterState ps;
    p->begin(this, &ps);
    QGfx * mygfx=static_cast<QWSPaintEngine*>(p)->gfx();
    if (mygfx) {
        mygfx->setAlphaType(QGfx::IgnoreAlpha);
        mygfx->setSource(&rimg);
        mygfx->blt(0,0,data->w,data->h,0,0);
    }
    p->end();
    if (image.hasAlphaBuffer()) {
#ifndef QT_NO_IMAGE_DITHER_TO_1
        if (!partialalpha) {
            QBitmap m;
            m = image.createAlphaMask(conversion_flags);
            setMask(m);
        } else
#endif
            data->hasAlpha = true;
    }
    data->uninit = false;

    return true;
}


QPixmap QPixmap::grabWindow(WId window, int x, int y, int w, int h)
{
    //#############################
    QPixmap pm;
#ifdef QT_OLD_GFX

    QWidget *widget = QWidget::find(window);
    if (widget) {
        if (w <= 0 || h <= 0) {
            if (w == 0 || h == 0)
                return pm;
            if (w < 0)
                w = widget->width() - x;
            if (h < 0)
                h = widget->height() - y;
        }
        pm.resize(w, h);
        QGfx *gfx=pm.graphicsContext();
        if (gfx) {
            gfx->setAlphaType(QGfx::IgnoreAlpha);
            gfx->setSource(widget);
            gfx->blt(0,0,w,h,x,y);
            delete gfx;
        }
    }
#else
#warning "QPixmap::grabWindow"
#endif
    return pm;
}

#ifndef QT_NO_PIXMAP_TRANSFORMATION
QPixmap QPixmap::xForm(const QWMatrix &matrix) const
{
    i