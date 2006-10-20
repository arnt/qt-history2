/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
//#define QT_RASTER_PAINTENGINE

#include "qpixmap.h"
#include "qpixmap_p.h"
#include "qimage.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qmatrix.h"
#include "qtransform.h"
#include "qdebug.h"
#include <private/qdrawhelper_p.h>
#ifdef QT_RASTER_PAINTENGINE
#  include <private/qpaintengine_raster_p.h>
#endif
#include <private/qpaintengine_mac_p.h>
#include <private/qt_mac_p.h>

#include <limits.h>
#include <string.h>

/*****************************************************************************
  Externals
 *****************************************************************************/
extern const uchar *qt_get_bitflip_array(); //qimage.cpp
extern CGContextRef qt_mac_cg_context(const QPaintDevice *pdev); //qpaintdevice_mac.cpp
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QRegion qt_mac_convert_mac_region(RgnHandle rgn); //qregion_mac.cpp

static int qt_pixmap_serial = 0;

Q_GUI_EXPORT quint32 *qt_mac_pixmap_get_base(const QPixmap *pix) { return pix->data->pixels; }
Q_GUI_EXPORT int qt_mac_pixmap_get_bytes_per_line(const QPixmap *pix) { return pix->data->nbytes / pix->data->h; }

static void qt_mac_cgimage_data_free(void *memory, const void *, size_t)
{
    free(memory);
}

/*****************************************************************************
  QPixmap member functions
 *****************************************************************************/

static inline QRgb qt_conv16ToRgb(ushort c) {
    static const int qt_rbits = (565/100);
    static const int qt_gbits = (565/10%10);
    static const int qt_bbits = (565%10);
    static const int qt_red_shift = qt_bbits+qt_gbits-(8-qt_rbits);
    static const int qt_green_shift = qt_bbits-(8-qt_gbits);
    static const int qt_neg_blue_shift = 8-qt_bbits;
    static const int qt_blue_mask = (1<<qt_bbits)-1;
    static const int qt_green_mask = (1<<(qt_gbits+qt_bbits))-((1<<qt_bbits)-1);
    static const int qt_red_mask = (1<<(qt_rbits+qt_gbits+qt_bbits))-(1<<(qt_gbits+qt_bbits));

    const int r=(c & qt_red_mask);
    const int g=(c & qt_green_mask);
    const int b=(c & qt_blue_mask);
    const int tr = r >> qt_red_shift;
    const int tg = g >> qt_green_shift;
    const int tb = b << qt_neg_blue_shift;

    return qRgb(tr,tg,tb);
}

QPixmap QPixmap::fromImage(const QImage &img, Qt::ImageConversionFlags flags)
{
    QPixmap pixmap;
    if(img.isNull()) {
        qWarning("QPixmap::convertFromImage: Cannot convert a null image");
        return pixmap;
    }

    QImage image = img;
    int    d     = image.depth();
    int    dd    = defaultDepth();
    bool force_mono = (dd == 1 ||
                       (flags & Qt::ColorMode_Mask)==Qt::MonoOnly);
    if(force_mono) {                         // must be monochrome
        if(d != 1) {
            image = image.convertToFormat(QImage::Format_MonoLSB, flags);  // dither
            d = 1;
        }
    } else {                                    // can be both
        bool conv8 = false;
        if(d > 8 && dd <= 8) {               // convert to 8 bit
            if((flags & Qt::DitherMode_Mask) == Qt::AutoDither)
                flags = (flags & ~Qt::DitherMode_Mask)
                                   | Qt::PreferDither;
            conv8 = true;
        } else if((flags & Qt::ColorMode_Mask) == Qt::ColorOnly) {
            conv8 = d == 1;                     // native depth wanted
        } else if(d == 1) {
            if(image.numColors() == 2) {
                QRgb c0 = image.color(0);       // Auto: convert to best
                QRgb c1 = image.color(1);
#if 0
                conv8 = qMin(c0,c1) != qRgb(0,0,0) || qMax(c0,c1) != qRgb(255,255,255);
#else
                conv8 = ((c0 == qRgb(0,0,0)) && c1 == qRgb(255,255,255));
#endif
            } else {
                // eg. 1-color monochrome images (they do exist).
                conv8 = true;
            }
        }
        if(conv8) {
            image = image.convertToFormat(QImage::Format_Indexed8, flags);
            d = 8;
        }
    }

    if(image.depth()==1) {
        image.setColor(0, QColor(Qt::color0).rgba());
        image.setColor(1, QColor(Qt::color1).rgba());
    }

    int w = image.width();
    int h = image.height();

    // different size or depth, make a new pixmap
    if (d == 1)
        pixmap = QBitmap(w, h);
    else
        pixmap = QPixmap(w, h);

    quint32 *dptr = pixmap.data->pixels, *drow;
    const uint dbpr = pixmap.data->nbytes / h;

    const QImage::Format sfmt = image.format();
    const unsigned short sbpr = image.bytesPerLine();
    uchar *sptr = image.bits(), *srow;

    for(int y=0;y<h;y++) {
        drow = dptr + (y * (dbpr / 4));
        srow = sptr + (y * sbpr);
        switch(sfmt) {
        case QImage::Format_MonoLSB:
        case QImage::Format_Mono:{
            for(int x=0;x<w;++x) {
                char one_bit = *(srow + (x / 8));
                if(sfmt == QImage::Format_Mono)
                    one_bit = one_bit >> (7 - (x % 8));
                else
                    one_bit = one_bit >> (x % 8);
                if((one_bit & 0x01))
                    *(drow+x) = 0x00000000;
                else
                    *(drow+x) = 0xFFFFFFFF;
            }
            break; }
        case QImage::Format_Indexed8:
            for(int x=0;x<w;++x) {
                *(drow+x) = PREMUL(image.color(*(srow + x)));
            }
            break;
        case QImage::Format_RGB32:
            for(int x=0;x<w;++x)
                *(drow+x) = *(((quint32*)srow) + x) | 0xFF000000;
            break;
        case QImage::Format_ARGB32:
        case QImage::Format_ARGB32_Premultiplied:
            for(int x=0;x<w;++x) {
                if(sfmt == QImage::Format_RGB32)
                    *(drow+x) = 0xFF000000 | (*(((quint32*)srow) + x) & 0x00FFFFFF);
                else if(sfmt == QImage::Format_ARGB32_Premultiplied)
                    *(drow+x) = *(((quint32*)srow) + x);
                else
                    *(drow+x) = PREMUL(*(((quint32*)srow) + x));
            }
            break;
        default:
            qWarning("Qt: internal: Oops: Forgot a format [%d] %s:%d", sfmt,
                     __FILE__, __LINE__);
            break;
        }
    }
    if(sfmt != QImage::Format_RGB32) { //setup the alpha
        bool alphamap = image.depth() == 32;
        if (sfmt == QImage::Format_Indexed8) {
            const QVector<QRgb> rgb = image.colorTable();
            for (int i = 0, count = image.numColors(); i < count; ++i) {
                const int alpha = qAlpha(rgb[i]);
                if (alpha != 0xff) {
                    alphamap = true;
                    break;
                }
            }
        }
        pixmap.data->macSetHasAlpha(alphamap);
    }
    pixmap.data->uninit = false;
    return pixmap;
}

int get_index(QImage * qi,QRgb mycol)
{
    int loopc;
    for(loopc=0;loopc<qi->numColors();loopc++) {
        if(qi->color(loopc)==mycol)
            return loopc;
    }
    qi->setNumColors(qi->numColors()+1);
    qi->setColor(qi->numColors(),mycol);
    return qi->numColors();
}

QImage QPixmap::toImage() const
{
    if(!data->w || !data->h)
        return QImage(); // null image

    int w = data->w;
    int h = data->h;
    QImage::Format format = QImage::Format_MonoLSB;
    if(data->d != 1) //Doesn't support index color modes
        format = (data->has_alpha ? QImage::Format_ARGB32_Premultiplied :
                  QImage::Format_RGB32);

    QImage image(w, h, format);
    quint32 *sptr = data->pixels, *srow;
    const uint sbpr = data->nbytes / h;
    if(format == QImage::Format_MonoLSB) {
        image.fill(0);
        image.setNumColors(2);
        image.setColor(0, QColor(Qt::color0).rgba());
        image.setColor(1, QColor(Qt::color1).rgba());
        for (int y = 0; y < h; ++y) {
            uchar *scanLine = image.scanLine(y);
            srow = sptr + (y * (sbpr/4));
            for (int x = 0; x < w; ++x) {
                if (!(*(srow + x) & RGB_MASK))
                    scanLine[x >> 3] |= (1 << (x & 7));
            }
        }
    } else {
        for(int y=0;y<h;y++) {
            srow = sptr + (y * (sbpr/4));
            memcpy(image.scanLine(y), srow, w * 4);
        }

    }

    return image;
}

void QPixmap::fill(const QColor &fillColor)

{
    if(!width() || !height())
        return;

    detach();
    { //we don't know what backend to use so we cannot paint here
        quint32 *dptr = data->pixels;
        Q_ASSERT_X(dptr, "QPixmap::fill", "No dptr");
        const quint32 colr = PREMUL(fillColor.rgba());
        if(!colr) {
            memset(dptr, 0, data->nbytes);
        } else {
            for(uint i = 0; i < data->nbytes/sizeof(quint32); ++i)
                *(dptr + i) = colr;
        }
    }
    data->macSetHasAlpha(fillColor.alpha() != 255);
}

QPixmap QPixmap::alphaChannel() const
{
    if (!data->has_alpha)
        return QPixmap();
    QPixmap alpha(width(), height());
    data->macGetAlphaChannel(&alpha, false);
    return alpha;
}

void QPixmap::setAlphaChannel(const QPixmap &alpha)
{
    if (paintingActive()) {
        qWarning("QPixmap::setAlphaChannel: Should not set alpha channel while pixmap is being painted on");
    }

    if (data == alpha.data) // trying to selfalpha
        return;

    if (alpha.width() != width() || alpha.height() != height()) {
        qWarning("QPixmap::setAlphaChannel: The pixmap and the alpha must have the same size");
        return;
    }

    detach();
    data->has_mask = true;
    data->macSetAlphaChannel(&alpha, false);
}

QBitmap QPixmap::mask() const
{
    if (!data->has_mask && !data->has_alpha)
        return QBitmap();
    QBitmap mask(width(), height());
    data->macGetAlphaChannel(&mask, true);
    return mask;
}

void QPixmap::setMask(const QBitmap &newmask)
{
    if (paintingActive()) {
        qWarning("QPixmap::setMask: Should not set mask while pixmap is being painted on");
    }

    if (data == newmask.data) // trying to selfmask
        return;

    if(newmask.isNull()) {
        detach();
        QPixmap opaque(width(), height());
        opaque.fill(QColor(255, 255, 255, 255));
        data->macSetAlphaChannel(&opaque, true);
        data->has_alpha = data->has_mask = false;
        return;
    }

    if (newmask.width() != width() || newmask.height() != height()) {
        qWarning("QPixmap::setMask: The pixmap and the mask must have the same size");
        return;
    }
    detach();
    data->has_alpha = false;
    data->has_mask = true;
    data->macSetAlphaChannel(&newmask, true);
}

void QPixmap::detach()
{
    ++data->detach_no;
    if(data->cg_mask) {
        CGImageRelease(data->cg_mask);
        data->cg_mask = 0;
    }
    if (data->count != 1) {
        *this = copy();
#ifndef QT_MAC_NO_QUICKDRAW
        data->qd_alpha = 0; //leave it behind
#endif
        data->ser_no = ++qt_pixmap_serial;
    }
    data->uninit = false;
}

int QPixmap::metric(PaintDeviceMetric m) const
{
    int val=0;
    switch (m) {
        case PdmWidth:
            val = width();
            break;
        case PdmHeight:
            val = height();
            break;
        case PdmWidthMM:
        case PdmHeightMM:
            break;
        case PdmNumColors:
            val = 1 << depth();
            break;
        case PdmDpiX:
        case PdmPhysicalDpiX:
        case PdmDpiY:
        case PdmPhysicalDpiY: {
            if(GDHandle gd = GetMainDevice()) {
                if (m == PdmDpiX || m == PdmPhysicalDpiX)
                    val = Fix2Long((**(**gd).gdPMap).hRes);
                else
                    val = Fix2Long((**(**gd).gdPMap).vRes);
            }
            break; }
        case PdmDepth:
            val = depth();
            break;
        default:
            val = 0;
            qWarning("QPixmap::metric: Invalid metric command");
    }
    return val;
}

QPixmapData::~QPixmapData()
{
#ifndef QT_MAC_NO_QUICKDRAW
    macQDDisposeAlpha();
    if(qd_data) {
        DisposeGWorld(qd_data);
        qd_data = 0;
    }
#endif
    if(cg_mask) {
        CGImageRelease(cg_mask);
        cg_mask = 0;
    }
    if(cg_data) {
        CGImageRelease(cg_data);
        cg_data = 0;
        pixels = 0; //let the cgimage hang onto the pixels if it wants to
    }
    delete paintEngine;
}

void
QPixmapData::macSetAlphaChannel(const QPixmap *pix, bool asMask)
{
    if(!pixels || !h || !w || pix->data->w != w || pix->data->h != h)
        return;

    quint32 *dptr = pixels, *drow;
    const uint dbpr = nbytes / h;
    const unsigned short sbpr = pix->data->nbytes / pix->data->h;
    quint32 *sptr = pix->data->pixels, *srow;
    for (int y=0; y < h; ++y) {
        drow = dptr + (y * (dbpr/4));
        srow = sptr + (y * (sbpr/4));
        if(d == 1) {
            for (int x=0; x < w; ++x) {
                if((*(srow+x) & RGB_MASK))
                    *(drow+x) = 0xFFFFFFFF;
            }
        } else if(d == 8) {
            for (int x=0; x < w; ++x)
                *(drow+x) = (*(drow+x) & RGB_MASK) | (*(srow+x) << 24);
        } else if(asMask) {
            for (int x=0; x < w; ++x) {
                if(*(srow+x) & RGB_MASK)
                    *(drow+x) = (*(drow+x) & RGB_MASK);
                else
                    *(drow+x) = (*(drow+x) & RGB_MASK) | 0xFF000000;
                *(drow+x) = PREMUL(*(drow+x));
            }
        } else {
            for (int x=0; x < w; ++x) {
                const uchar alpha = qGray(qRed(*(srow+x)), qGreen(*(srow+x)), qBlue(*(srow+x)));
                const uchar destAlpha = qt_div_255(alpha * qAlpha(*(drow+x)));
#if 1
                *(drow+x) = (*(drow+x) & RGB_MASK) | (destAlpha << 24);
#else
                *(drow+x) = qRgba(qt_div_255(qRed(*(drow+x) * alpha)),
                                  qt_div_255(qGreen(*(drow+x) * alpha)),
                                  qt_div_255(qBlue(*(drow+x) * alpha)), destAlpha);
#endif
                *(drow+x) = PREMUL(*(drow+x));
            }
        }
    }
    macSetHasAlpha(true);
}

void
QPixmapData::macGetAlphaChannel(QPixmap *pix, bool asMask) const
{
    quint32 *dptr = pix->data->pixels, *drow;
    const uint dbpr = pix->data->nbytes / pix->data->h;
    const unsigned short sbpr = nbytes / h;
    quint32 *sptr = pixels, *srow;
    for(int y=0; y < h; ++y) {
        drow = dptr + (y * (dbpr/4));
        srow = sptr + (y * (sbpr/4));
        if(asMask) {
            for(int x = 0; x < w; ++x) {
                if(*(srow+x) & RGB_MASK)
                    *(drow+x) = 0x00000000;
                else
                    *(drow+x) = 0xFFFFFFFF;
            }
        } else {
            memcpy(drow, srow, w * 4);
        }
    }
}

void
QPixmapData::macSetHasAlpha(bool b)
{
    has_alpha = b;
#ifndef QT_MAC_NO_QUICKDRAW
    macQDDisposeAlpha(); //let it get created lazily
#endif
}

#ifndef QT_MAC_NO_QUICKDRAW
void
QPixmapData::macQDDisposeAlpha()
{
    if(qd_alpha) {
        DisposeGWorld(qd_alpha);
        qd_alpha = 0;
    }
}

void
QPixmapData::macQDUpdateAlpha()
{
    macQDDisposeAlpha(); // get rid of alpha pixmap
    if(!has_alpha && !has_mask)
        return;

    //setup
    Rect rect;
    SetRect(&rect, 0, 0, w, h);
    const int params = alignPix | stretchPix | newDepth;
    NewGWorld(&qd_alpha, 32, &rect, 0, 0, params);
    int *dptr = (int *)GetPixBaseAddr(GetGWorldPixMap(qd_alpha)), *drow;
    unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap(qd_alpha));
    const int *sptr = (int*)pixels, *srow;
    const uint sbpr = nbytes / h;
    uchar clr;
    for(int y=0; y < h; y++) {
        drow = (int*)((char *)dptr + (y * dbpr));
        srow = (int*)((char *)sptr + (y * sbpr));
        for (int x=0; x < w; x++) {
            clr = qAlpha(*(srow + x));
            *(drow + x) = qRgba(~clr, ~clr, ~clr, 0);
        }
    }
}
#endif

QPixmap QPixmap::transformed(const QMatrix &matrix, Qt::TransformationMode mode) const
{
    return transformed(QTransform(matrix), mode);
}

QPixmap QPixmap::transformed(const QTransform &transform, Qt::TransformationMode mode) const
{
    if(isNull())
        return copy();

    int w, h;  // size of target pixmap
    const int ws = width();
    const int hs = height();

    QTransform mat(transform.m11(), transform.m12(), transform.m21(), transform.m22(), 0., 0.);
    if(transform.m12() == 0.0F  && transform.m21() == 0.0F &&
         transform.m11() >= 0.0F  && transform.m22() >= 0.0F) {
        if(mat.m11() == 1.0F && mat.m22() == 1.0F) // identity transform
            return *this;
        h = int(qAbs(mat.m22()) * hs + 0.9999);
        w = int(qAbs(mat.m11()) * ws + 0.9999);
        h = qAbs(h);
        w = qAbs(w);
    } else { // rotation or shearing
        QPolygonF a(QRectF(0,0,ws+1,hs+1));
        a = mat.map(a);
        QRectF r = a.boundingRect().normalized();
        w = int(r.width() + 0.9999);
        h = int(r.height() + 0.9999);
    }
    mat = trueMatrix(mat, ws, hs);
    if(!h || !w)
        return QPixmap();

    //create destination
    QPixmap pm = depth() == 1 ? QPixmap(QBitmap(w, h)) : QPixmap(w, h);

    const quint32 *sptr = data->pixels;
    quint32 *dptr = pm.data->pixels;
    memset(dptr, 0, pm.data->nbytes);

    //do the transform
    if(mode == Qt::SmoothTransformation) {
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        p.setRenderHint(QPainter::SmoothPixmapTransform);
        p.setTransform(mat);
        p.drawPixmap(0, 0, *this);
    } else {
        bool invertible;
        mat = mat.inverted(&invertible);
        if(!invertible)
            return QPixmap();

        const int bpp = 32;
        const int xbpl = (w * bpp) / 8;
        if(!qt_xForm_helper(mat, 0, QT_XFORM_TYPE_MSBFIRST, bpp,
                            (uchar*)dptr, xbpl, (pm.data->nbytes / pm.data->h) - xbpl,
                            h, (uchar*)sptr, (data->nbytes / data->h), ws, hs)) {
            qWarning("Qt: QPixmap::transform: failure");
            return QPixmap();
        }
    }

    //update the alpha
    pm.data->macSetHasAlpha(true);
    return pm;
}


void QPixmap::init(int w, int h, Type type)
{
    if (!qApp) {
        qFatal("QPixmap: Must construct a QApplication before a QPaintDevice");
        return;
    }
    if (qApp->type() == QApplication::Tty)
        qWarning("QPixmap: Cannot create a QPixmap when no GUI "
                  "is being used");

    data = new QPixmapData;
    memset(data, 0, sizeof(QPixmapData));
    data->count = 1;
    data->uninit = true;
    data->ser_no = ++qt_pixmap_serial;
    data->detach_no = 0;
    data->type = type;
    data->cg_mask = 0;
#ifndef QT_MAC_NO_QUICKDRAW
    data->qd_data = 0;
    data->qd_alpha = 0;
#endif

    bool make_null = w == 0 || h == 0;                // create null pixmap
    data->d = (type == PixmapType) ? 32 : 1;
    if(make_null || w < 0 || h < 0 || data->d == 0) {
        if(!make_null)
            qWarning("Qt: QPixmap: Invalid pixmap parameters");
        return;
    }

    if(w<1 || h<1)
        return;
    data->w=w;
    data->h=h;

    //create the pixels
    data->nbytes = (w*h*sizeof(quint32));
    quint32 *base_pixels = (quint32*)malloc(data->nbytes);
    data->pixels = base_pixels;

    //create the cg data
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(base_pixels,
                                                              data->pixels, data->nbytes,
                                                              qt_mac_cgimage_data_free);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    uint cgflags = kCGImageAlphaPremultipliedFirst;
#ifdef kCGBitmapByteOrder32Host //only needed because CGImage.h added symbols in the minor version
    if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4)
        cgflags |= kCGBitmapByteOrder32Host;
#endif
#else
    CGImageAlphaInfo cgflags = kCGImageAlphaPremultipliedFirst;
#endif
    data->cg_data = CGImageCreate(w, h, 8, 32, data->nbytes / h, colorspace,
                                  cgflags, provider, 0, 0, kCGRenderingIntentDefault);

    CGColorSpaceRelease(colorspace);
    CGDataProviderRelease(provider);
}

int QPixmap::defaultDepth()
{
    int ret = 32;
    if(GDHandle gd = GetMainDevice()) {
        if((**gd).gdCCDepth)
            ret = (**gd).gdCCDepth;
    }
    return ret;
}

QPixmap QPixmap::grabWindow(WId window, int x, int y, int w, int h)
{
    QPixmap pm;
    QWidget *widget = QWidget::find(window);
    if(widget) {
        if(w == -1)
            w = widget->width() - x;
        if(h == -1)
            h = widget->height() - y;
        pm = QPixmap(w, h);
        extern WindowPtr qt_mac_window_for(const QWidget *); // qwidget_mac.cpp
        const BitMap *windowPort = 0;
        if((widget->windowType() == Qt::Desktop)) {
	    GDHandle gdh;
#if 0
	    if(GetWindowGreatestAreaDevice((WindowPtr)w->handle(), kWindowStructureRgn, &gdh, NULL) || !gdh)
		qDebug("Qt: internal: Unexpected condition reached: %s:%d", __FILE__, __LINE__);
#else
	    if(!(gdh=GetMainDevice()))
		qDebug("Qt: internal: Unexpected condition reached: %s:%d", __FILE__, __LINE__);
#endif
	    windowPort = (BitMap*)(*(*gdh)->gdPMap);
        } else {
            windowPort = GetPortBitMapForCopyBits(GetWindowPort(qt_mac_window_for(widget)));
        }
        const BitMap *pixmapPort = GetPortBitMapForCopyBits(static_cast<GWorldPtr>(pm.macQDHandle()));
        Rect macSrcRect, macDstRect;
        SetRect(&macSrcRect, x, y, x + w, y + h);
        SetRect(&macDstRect, 0, 0, w, h);
        CopyBits(windowPort, pixmapPort, &macSrcRect, &macDstRect, srcCopy, 0);
    }
    return pm;
}

/*! \internal

    Returns the QuickDraw CGrafPtr of the pixmap. 0 is returned if it can't
    be obtained. Do not hold the pointer around for long as it can be
    relocated.

    \warning This function is only available on Mac OS X.
*/

Qt::HANDLE QPixmap::macQDHandle() const
{
#ifndef QT_MAC_NO_QUICKDRAW
    if(!data->qd_data) { //create the qd data
        Rect rect;
        SetRect(&rect, 0, 0, data->w, data->h);
        unsigned long qdformat = k32ARGBPixelFormat;
        GWorldFlags qdflags = 0;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
        if(QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
            //we play such games so we can use the same buffer in CG as QD this
            //makes our merge much simpler, at some point the hacks will go away
            //because QD will be removed, but until that day this keeps them coexisting
            if(QSysInfo::ByteOrder == QSysInfo::LittleEndian)
                qdformat = k32BGRAPixelFormat;
#if 0
            qdflags |= kNativeEndianPixMap;
#endif
        }
#endif
        if(NewGWorldFromPtr(&data->qd_data, qdformat, &rect, 0, 0, qdflags,
                            (char*)data->pixels, data->nbytes / data->h) != noErr)
            qWarning("Qt: internal: QPixmap::init error (%d %d %d %d)", rect.left, rect.top, rect.right, rect.bottom);
    }
    return data->qd_data;
#else
    return 0;
#endif
}

/*! \internal

    Returns the QuickDraw CGrafPtr of the pixmap's alpha channel. 0 is
    returned if it can't be obtained. Do not hold the pointer around for
    long as it can be relocated.

    \warning This function is only available on Mac OS X.
*/

Qt::HANDLE QPixmap::macQDAlphaHandle() const
{
#ifndef QT_MAC_NO_QUICKDRAW
    if(data->has_alpha || data->has_mask) {
        if(!data->qd_alpha) //lazily created
            data->macQDUpdateAlpha();
        return data->qd_alpha;
    }
#endif
    return 0;
}

/*! \internal

    Returns the CoreGraphics CGContextRef of the pixmap. 0 is returned if
    it can't be obtained. It is the caller's responsiblity to
    CGContextRelease the context when finished using it.

    \warning This function is only available on Mac OS X.
*/

Qt::HANDLE QPixmap::macCGHandle() const
{
    CGImageRef ret = (CGImageRef)data->cg_data;
    CGImageRetain(ret);
    return ret;
}

bool QPixmap::hasAlpha() const
{
    return data->has_alpha || data->has_mask;
}

bool QPixmap::hasAlphaChannel() const
{
    return data->has_alpha;
}

CGImageRef qt_mac_create_imagemask(const QPixmap &px, const QRectF &sr)
{
    if(px.data->cg_mask) {
        if(px.data->cg_mask_rect == sr) {
            CGImageRetain(px.data->cg_mask); //reference for the caller
            return px.data->cg_mask;
        }
        CGImageRelease(px.data->cg_mask);
        px.data->cg_mask = 0;
    }

    const int sx = qRound(sr.x()), sy = qRound(sr.y()), sw = qRound(sr.width()), sh = qRound(sr.height());
    const int sbpr = px.data->nbytes / px.data->h;
    const uint nbytes = sw * sh;
    quint8 *dptr = (quint8 *)malloc(nbytes);
    quint32 *sptr = px.data->pixels, *srow;
    for(int y = sy, offset=0; y < sh; ++y) {
        srow = sptr + (y * (sbpr / 4));
        for(int x = sx; x < sw; ++x)
            *(dptr+(offset++)) =  *(srow+x) ? 255 : 0;
    }
    QCFType<CGDataProviderRef> provider = CGDataProviderCreateWithData(dptr, dptr, nbytes, qt_mac_cgimage_data_free);
    px.data->cg_mask = CGImageMaskCreate(sw, sh, 8, 8, nbytes / sh, provider, 0, 0);
    px.data->cg_mask_rect = sr;
    CGImageRetain(px.data->cg_mask); //reference for the caller
    return px.data->cg_mask;
}

IconRef qt_mac_create_iconref(const QPixmap &px)
{
    if (px.isNull())
        return 0;
    QMacSavedPortInfo pi; //save the current state
    //create icon
    IconFamilyHandle iconFamily = reinterpret_cast<IconFamilyHandle>(NewHandle(0));
    //create data
    {
        struct {
            OSType mac_type;
            int width, height, depth;
            bool mask;
        } images[] = {
            { kThumbnail32BitData, 128, 128, 32, false },
            { kThumbnail8BitMask, 128, 128, 8, true },
            { 0, 0, 0, 0, false } //end marker
        };
        for(int i = 0; images[i].mac_type; i++) {
            //get QPixmap data
            QPixmap scaled_px = px.scaled(images[i].width, images[i].height);
            quint32 *sptr = scaled_px.data->pixels, *srow;
            const uint sbpr = scaled_px.data->nbytes / scaled_px.data->h;

            //get Handle data
            const int dbpr = images[i].width * (images[i].depth/8);
            Handle hdl = NewHandle(dbpr*images[i].height);
            if(!sptr) { //handle null pixmap
                memset((*hdl), '\0', dbpr*images[i].height);
            } else if(images[i].mask) {
                if(images[i].mac_type == kThumbnail8BitMask) {
                    for(int y = 0, hindex = 0; y < images[i].height; ++y) {
                        srow = sptr + (y * (sbpr/4));
                        for(int x = 0; x < images[i].width; ++x)
                            *((*hdl)+(hindex++)) = qAlpha(*(srow+x));
                    }
                }
            } else {
                char *dest = (*hdl);
#if defined(__i386__)
                if(images[i].depth == 32) {
                    for(int y = 0; y < images[i].height; ++y) {
                        uint *source = (uint*)((const uchar*)sptr+(sbpr*y));
                        for(int x = 0; x < images[i].width; ++x, dest += 4)
                            *((uint*)dest) = CFSwapInt32(*(source + x));
                    }
                } else
#endif
                {
                    for(int y = 0; y < images[i].height; ++y)
                        memcpy(dest+(y*dbpr), ((const uchar*)sptr+(sbpr*y)), dbpr);
                }
            }

            //set the family data to the Handle
            OSStatus set = SetIconFamilyData(iconFamily, images[i].mac_type, hdl);
            if(set != noErr)
                qWarning("%s: %d -- Something went very wrong!! %ld", __FILE__, __LINE__, long(set));
            DisposeHandle(hdl);
        }
    }

    //acquire and cleanup
    IconRef ret;
    static int counter = 0;
    const OSType kQtCreator = 'CUTE';
    RegisterIconRefFromIconFamily(kQtCreator, (OSType)counter, iconFamily, &ret);
    AcquireIconRef(ret);
    UnregisterIconRef(kQtCreator, (OSType)counter);
    DisposeHandle(reinterpret_cast<Handle>(iconFamily));
    counter++;
    return ret;

}

QPixmap qt_mac_convert_iconref(IconRef icon, int width, int height)
{
    QPixmap ret(width, height);
    ret.fill(QColor(0, 0, 0, 0));

    CGRect rect = CGRectMake(0, 0, width, height);

    CGContextRef ctx = qt_mac_cg_context(&ret);
    CGAffineTransform old_xform = CGContextGetCTM(ctx);
    CGContextConcatCTM(ctx, CGAffineTransformInvert(old_xform));
    CGContextConcatCTM(ctx, CGAffineTransformIdentity);

    ::RGBColor b;
    b.blue = b.green = b.red = 255*255;
    PlotIconRefInContext(ctx, &rect, kAlignNone, kTransformNone, &b, kPlotIconRefNormalFlags, icon);
    CGContextRelease(ctx);
    return ret;
}

/*! \internal */
QPaintEngine *QPixmap::paintEngine() const
{
    if (!data->paintEngine) {
#ifdef QT_RASTER_PAINTENGINE
        if(qgetenv("QT_MAC_USE_COREGRAPHICS").isNull())
            data->paintEngine = new QRasterPaintEngine();
        else
            data->paintEngine = new QCoreGraphicsPaintEngine();
#else
        data->paintEngine = new QCoreGraphicsPaintEngine();
#endif
    }
    return data->paintEngine;
}

QPixmap QPixmap::copy(const QRect &rect) const
{
    QPixmap pm;
    if (data->type == BitmapType) {
        pm = QBitmap::fromImage(toImage().copy(rect));
    } else {
        if (rect.isNull()) {
            pm = QPixmap(size());
            memcpy(pm.data->pixels, data->pixels, data->nbytes);
        } else {
            int x = rect.x(), y = rect.y(), w = rect.width(), h = rect.height();
            if(x < 0) {
                w += x;
                x = 0;
            }
            if(y < 0) {
                h += y;
                y = 0;
            }
            if (w > 0 && h > 0 && x < data->w && y < data->h) {
                pm = QPixmap(w, h);
                if (x+w > data->w || y+h > data->h) {
                    pm.fill(Qt::color0);  // bitBlt will not cover entire image - clear it.
                    if(x+w > data->w)
                        w = data->w - x;
                    if(y+h > data->h)
                        h = data->h - y;
                }
                for (int i = 0; i < h; ++i)
                    memcpy(pm.data->pixels + i*pm.data->w,
                           data->pixels + (i + y)*data->w + x, w*4);
            }
        }
        pm.data->has_alpha = data->has_alpha;
        pm.data->has_mask = data->has_mask;
    }
    return pm;
}


/*!
    \since 4.2

    Creates a \c CGImageRef equivalent to the QPixmap. Returns the \c CGImageRef handle.

    It is the caller's responsibility to release the \c CGImageRef data
    after use.

    \warning This function is only available on Mac OS X.

    \sa fromMacCGImageRef()
*/
CGImageRef QPixmap::toMacCGImageRef() const
{
    return (CGImageRef)macCGHandle();
}

/*!
    \since 4.2

    Returns a QPixmap that is equivalent to the given \a image.

    \warning This function is only available on Mac OS X.

    \sa toMacCGImageRef(), {QPixmap#Pixmap Conversion}{Pixmap Conversion}
*/
QPixmap QPixmap::fromMacCGImageRef(CGImageRef image)
{
    const size_t w = CGImageGetWidth(image),
                 h = CGImageGetHeight(image);
    QPixmap ret(w, h);

    CGRect rect = CGRectMake(0, 0, w, h);
    CGContextRef ctx = qt_mac_cg_context(&ret);
    HIViewDrawCGImage(ctx, &rect, image);

    return ret;
}
