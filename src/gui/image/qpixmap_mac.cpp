/****************************************************************************
**
** Implementation of QFileInfo class.
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

#include "qpixmap.h"
#include "qimage.h"
#include "qpaintdevicemetrics.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qwmatrix.h"
#include "qpaintengine_mac.h"
#include "qt_mac.h"

#include <limits.h>
#include <string.h>

extern const uchar *qt_get_bitflip_array();                // defined in qimage.cpp
#define QMAC_PIXMAP_ALPHA

QPixmap::QPixmap(int w, int h, const uchar *bits, bool isXbitmap)
    : QPaintDevice(QInternal::Pixmap)
{
    init(w, h, 1, true, DefaultOptim);
    Q_ASSERT_X(data->hd, "QPixmap::QPixmap", "No handle");

    long *dptr = (long *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd))), *drow, q;
    unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
    for(int yy=0;yy<h;yy++) {
        drow = (long *)((char *)dptr + (yy * dbpr));
        int sy = yy * ((w+7)/8);
        for(int xx=0;xx<w;xx++) {
            char one_bit = *(bits + (sy + (xx / 8)));
            if(!isXbitmap)
                one_bit = one_bit >> (7 - (xx % 8));
            else
                one_bit = one_bit >> (xx % 8);
            q = 0;
            if(!(one_bit & 0x01))
                q = (255 << 16) | (255 << 8) | 255;
            *(drow + xx) = q;
        }
    }
}

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

bool QPixmap::convertFromImage(const QImage &img, int conversion_flags)
{
    if(img.isNull()) {
        qWarning("QPixmap::convertFromImage: Cannot convert a null image");
        return false;
    }

    QImage image = img;
    int    d     = image.depth();
    int    dd    = defaultDepth();
    bool force_mono = (dd == 1 || isQBitmap() ||
                       (conversion_flags & Qt::ColorMode_Mask)==Qt::MonoOnly);
    if(force_mono) {                         // must be monochrome
        if(d != 1) {
            image = image.convertDepth(1, conversion_flags);  // dither
            d = 1;
        }
    } else {                                    // can be both
        bool conv8 = false;
        if(d > 8 && dd <= 8) {               // convert to 8 bit
            if((conversion_flags & Qt::DitherMode_Mask) == Qt::AutoDither)
                conversion_flags = (conversion_flags & ~Qt::DitherMode_Mask)
                                   | Qt::PreferDither;
            conv8 = true;
        } else if((conversion_flags & Qt::ColorMode_Mask) == Qt::ColorOnly) {
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
            image = image.convertDepth(8, conversion_flags);
            d = 8;
        }
    }

    if(image.depth()==1) {
        image.setColor(0, qRgba(255,255,255,0));
        image.setColor(1, qRgba(0,0,0,0));
    }

    int w = image.width();
    int h = image.height();

    if(width() == w && height() == h && ((d == 1 && depth() == 1) ||
                                            (d != 1 && depth() != 1))) {
        // same size etc., use the existing pixmap
        detach();

        if(data->mask) {                     // get rid of the mask
            delete data->mask;
            data->mask = 0;
        }
        if(data->alphapm) {                     // get rid of the alpha
            delete data->alphapm;
            data->alphapm = 0;
        }
    } else {
        // different size or depth, make a new pixmap
        QPixmap pm(w, h, d == 1 ? 1 : -1);
        pm.data->bitmap = data->bitmap;         // keep is-a flag
        pm.data->optim  = data->optim;          // keep optimization flag
        *this = pm;
    }

    Q_ASSERT_X(data->hd, "QPixmap::convertFromImage", "No handle");

    long *dptr = (long *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd))), *drow;
    unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));

    QRgb q=0;
    int sdpt = image.depth();
    unsigned short sbpr = image.bytesPerLine();
    uchar *sptr = image.bits(), *srow;
    QImage::Endian sord = image.bitOrder();

    for(int yy=0;yy<h;yy++) {
        drow = (long *)((char *)dptr + (yy * dbpr));
        srow = sptr + (yy * sbpr);
        switch(sdpt) {
        case 1:
        {
            for(int xx=0;xx<w;xx++) {
                char one_bit = *(srow + (xx / 8));
                if(sord==QImage::BigEndian)
                    one_bit = one_bit >> (7 - (xx % 8));
                else
                    one_bit = one_bit >> (xx % 8);
                q = 0;
                if(!(one_bit & 0x01))
                    q = (255 << 16) | (255 << 8) | 255;
                *(drow + xx) = q;
            }
            break;
        }
        case 8:
            for(int xx=0;xx<w;xx++) {
                q = image.color(*(srow + xx));
                *(drow + xx) = q;
            }
            break;
        case 16:
            for(int xx=0;xx<w;xx++) {
                q = qt_conv16ToRgb(*(((ushort *)srow) + xx));
                *(drow + xx) = q;
            }
            break;
        case 32:
            for(int xx=0;xx<w;xx++) {
                q = *(((QRgb *)srow) + xx);
                *(drow + xx) = q;
            }
            break;
        default:
            qDebug("Qt: internal: Oops: Forgot a depth %s:%d", __FILE__, __LINE__);
            break;
        }
    }

    data->uninit = false;

    // get rid of alpha pixmap
    delete data->alphapm;
    data->alphapm = 0;
    if(img.hasAlphaBuffer()) {
        {
            QBitmap m;
            m = img.createAlphaMask(conversion_flags);
            setMask(m);
        }
#ifdef QMAC_PIXMAP_ALPHA
        bool alphamap = img.depth() == 32;
        if (img.depth() == 8) {
            const QRgb * const rgb = img.colorTable();
            for (int i = 0, count = img.numColors(); i < count; ++i) {
                const int alpha = qAlpha(rgb[i]);
                if (alpha != 0 && alpha != 0xff) {
                    alphamap = true;
                    break;
                }
            }
        }
        if (alphamap) {
            data->alphapm = new QPixmap(w, h, 32);
            long *dptr = (long *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->alphapm->data->hd))),
                 *drow;
            unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
            if (img.depth() == 32) {
                unsigned short sbpr = image.bytesPerLine();
                long *sptr = (long*)image.bits(), *srow;
                uchar clr;
                for(int yy=0; yy < h; yy++) {
                    drow = (long *)((char *)dptr + (yy * dbpr));
                    srow = (long *)((char *)sptr + (yy * sbpr));
                    for (int xx=0; xx < w; xx++) {
                        clr = ~(((*(srow + xx)) >> 24) & 0xFF);
                        *(drow + xx) = qRgba(clr, clr, clr, 0);
                    }
                }
            } else {
                const QRgb *const rgb = img.colorTable();
                for (int y = 0; y < h; ++y) {
                    const uchar *iptr = image.scanLine(y);
                    drow = (long *)((char *)dptr + (y * dbpr));
                    for (int x = 0; x < w; ++x) {
                        const int alpha = ~qAlpha(rgb[*iptr++]);
                        *(drow + x) = qRgba(alpha, alpha, alpha, 0);
                    }
                }
            }
        }
#endif //!QMAC_PIXMAP_ALPHA
    }
    return true;
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

QImage QPixmap::convertToImage() const
{
    if(!data->w || !data->h || !data->hd)
        return QImage(); // null image

    int w = data->w;
    int h = data->h;
    int d = data->d;
    int ncols = 2;

#if 0
    if(d > 1 && d <= 8) {                    // set to nearest valid depth
        d = 8;                                  //   2..7 ==> 8
        ncols = 256;
    } else if(d > 8) {
        d = 32;                                 //   > 8  ==> 32
        ncols = 0;
    }
#else
    if(d != 1) { //do we want to FIXME??? Might want to support indexed color modes?
        d = 32;
        ncols = 0;
    }
#endif

    QImage image(w, h, d, ncols, QImage::BigEndian);
    //first we copy the clut
    //handle bitmap case, what about other indexed depths?
    if(d == 1) {
        image.setNumColors(2);
        image.setColor(0, qRgba(255, 255, 255, 0));
        image.setColor(1, qRgba(0, 0, 0, 0));
    } else if(d == 8) {
        //figure out how to copy clut into image FIXME???
    }

    Q_ASSERT_X(data->hd, "QPixmap::convertToImage", "No handle");

    QRgb q;
    long *sptr = reinterpret_cast<long *>(GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)))),
         *srow,
         r;
    unsigned short sbpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
    long *aptr = 0, *arow = 0;
    unsigned short abpr = 0;
    if(data->alphapm) {
        image.setAlphaBuffer(true);
        aptr = reinterpret_cast<long *>(GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->alphapm->data->hd))));
        abpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->alphapm->data->hd)));
    }

    for(int yy=0;yy<h;yy++) {
        srow = (long *)((char *)sptr + (yy * sbpr));
        if(aptr)
            arow = (long *)((char *)aptr + (yy * abpr));
        for(int xx=0;xx<w;xx++) {
            r = *(srow + xx);
            q=qRgba((r >> 16) & 0xFF, (r >> 8) & 0xFF, r & 0xFF, (arow ? ~(*(arow + xx) & 0xFF): 0));
            if(d == 1)
                image.setPixel(xx, yy, q ? 0 : 1);
            else if(ncols)
                image.setPixel(xx, yy, get_index(&image,q));
            else
                image.setPixel(xx,yy,q);
        }
    }

    if(data->mask && !data->alphapm) {
        QImage alpha = data->mask->convertToImage();
        image.setAlphaBuffer(true);
        switch(d) {
        case 8: {
            int used[256];
            memset(used, 0, sizeof(int)*256);
            uchar* p = image.bits();
            int l = image.numBytes();
            while(l--)
                used[*p++]++;
            int trans=0;
            int bestn=INT_MAX;
            for(int i=0; i<256; i++) {
                if(used[i] < bestn) {
                    bestn = used[i];
                    trans = i;
                    if(!bestn)
                        break;
                }
            }
            image.setColor(trans, image.color(trans)&0x00ffffff);
            for(int y=0; y<image.height(); y++) {
                uchar* mb = alpha.scanLine(y);
                uchar* ib = image.scanLine(y);
                uchar bit = 0x80;
                int i=image.width();
                while(i--) {
                    if(!(*mb & bit))
                        *ib = trans;
                    bit /= 2;
                    if(!bit)
                        mb++,bit = 0x80; // ROL
                    ib++;
                }
            }
        } break;
        case 32: {
            for(int y=0; y<image.height(); y++) {
                uchar* mb = alpha.scanLine(y);
                QRgb* ib = (QRgb*)image.scanLine(y);
                uchar bit = 0x80;
                int i=image.width();
                while(i--) {
                    if(*mb & bit)
                        *ib |= 0xff000000;
                    else
                        *ib &= 0x00ffffff;
                    bit /= 2;
                    if(!bit)
                        mb++, bit = 0x80; // ROL
                    ib++;
                }
            }
        } break;
        }
    }

    return image;
}

void QPixmap::fill(const QColor &fillColor)
{
    if(!width() || !height())
        return;
    Q_ASSERT_X(data->hd, "QPixmap::fill", "No handle");

    //at the end of this function this will go out of scope and the destructor will restore the state
    QMacSavedPortInfo saveportstate(this);
    detach();                                        // detach other references
    if(depth() == 1 || depth() == 32) { //small optimization over QD
        ulong *dptr = (ulong *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
        int dbytes = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)))*height();
        Q_ASSERT_X(dptr && dbytes, "QPixmap::fill", "No dptr or no dbytes");
        QRgb colr = qRgba(fillColor.red(),fillColor.green(), fillColor.blue(), 0);
        if(depth() == 1 || !colr) {
            memset(dptr, colr ? 0xFF : 0x00, dbytes);
        } else if(depth() == 32) {
            for(int i = 0; i < (int)(dbytes/sizeof(ulong)); i++)
                *(dptr + i) = colr;
        }
    } else {
        Rect r;
        RGBColor rc;
        rc.red=fillColor.red()*256;
        rc.green=fillColor.green()*256;
        rc.blue=fillColor.blue()*256;
        RGBForeColor(&rc);
        SetRect(&r,0,0,width(),height());
        PaintRect(&r);
    }
}

void QPixmap::detach()
{
    if(data->uninit || data->count == 1)
        data->uninit = false;
    else
        *this = copy();
}

int QPixmap::metric(int m) const
{
    int val=0;
    switch(m) {
        case QPaintDeviceMetrics::PdmWidth:
            val = width();
            break;
        case QPaintDeviceMetrics::PdmHeight:
            val = height();
            break;
        case QPaintDeviceMetrics::PdmWidthMM:
        case QPaintDeviceMetrics::PdmHeightMM:
            break;
        case QPaintDeviceMetrics::PdmNumColors:
            val = 1 << depth();
            break;
        case QPaintDeviceMetrics::PdmDpiX: // ### fill with real values!
        case QPaintDeviceMetrics::PdmPhysicalDpiX:
        case QPaintDeviceMetrics::PdmDpiY:
        case QPaintDeviceMetrics::PdmPhysicalDpiY:
            val = 72;
            break;
        case QPaintDeviceMetrics::PdmDepth:
            val = depth();
            break;
        default:
            val = 0;
            qWarning("QPixmap::metric: Invalid metric command");
    }
    return val;
}

void QPixmap::deref()
{
    if(data && data->deref()) {     // Destroy image if last ref
        if(data->mask) {
            delete data->mask;
            data->mask = 0;
        }
        if(data->alphapm) {
            delete data->alphapm;
            data->alphapm = 0;
        }

        if(data->hd && qApp) {
            UnlockPixels(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
            CGContextRelease((CGContextRef)data->cg_hd);
            data->cg_hd = 0;
            DisposeGWorld(static_cast<GWorldPtr>(data->hd));
            data->hd = 0;
        }
        delete data;
        data = 0;
    }
}

QPixmap QPixmap::xForm(const QWMatrix &matrix) const
{
    int           w, h;                                // size of target pixmap
    int           ws, hs;                                // size of source pixmap
    uchar *dptr;                                // data in target pixmap
    int           dbpl, dbytes;                        // bytes per line/bytes total
    uchar *sptr;                                // data in original pixmap
    int           sbpl;                                // bytes per line in original
    int           bpp;                                        // bits per pixel

    if(isNull())                                // this is a null pixmap
        return copy();

    ws = width();
    hs = height();

    QWMatrix mat(matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(), 0., 0.);

    if(matrix.m12() == 0.0F  && matrix.m21() == 0.0F &&
         matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F) {
        if(mat.m11() == 1.0F && mat.m22() == 1.0F)
            return *this;                        // identity matrix
        h = qRound(mat.m22()*hs);
        w = qRound(mat.m11()*ws);
        h = QABS(h);
        w = QABS(w);
    } else {                                        // rotation or shearing
        QPointArray a(QRect(0,0,ws+1,hs+1));
        a = mat.map(a);
        QRect r = a.boundingRect().normalize();
        w = r.width()-1;
        h = r.height()-1;
    }

    mat = trueMatrix(mat, ws, hs); // true matrix

    bool invertible;
    mat = mat.invert(&invertible);                // invert matrix

    if(h == 0 || w == 0 || !invertible) {        // error, return null pixmap
        QPixmap pm;
        pm.data->bitmap = data->bitmap;
        pm.data->alphapm = data->alphapm;
        return pm;
    }

    sptr = (uchar *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
    sbpl = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
    ws=width();
    hs=height();

    QPixmap pm(w, h, depth(), optimization());
    dptr = (uchar *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(pm.handle())));
    dbpl = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(pm.handle())));
    bpp = 32;
    dbytes = dbpl*h;

    if(bpp == 1)
        memset(dptr, 0x00, dbytes);
    else if(bpp == 8)
        memset(dptr, QColor(white).pixel(), dbytes);
    else if(bpp == 32)
        pm.fill(0x00FFFFFF);
    else
        memset(dptr, 0xff, dbytes);

    int        xbpl = bpp == 1 ? ((w+7)/8) : ((w*bpp)/8);
    if(!qt_xForm_helper(mat, 0, QT_XFORM_TYPE_MSBFIRST, bpp,
                        dptr, xbpl, dbpl - xbpl, h, sptr, sbpl, ws, hs)){
        qWarning("Qt: QPixmap::xForm: display not supported (bpp=%d)",bpp);
        QPixmap pm;
        return pm;
    }

    if(depth() == 1) {
        if(data->mask) {
            if(data->selfmask)               // pixmap == mask
                pm.setMask(*((QBitmap*)(&pm)));
            else
                pm.setMask(data->mask->xForm(matrix));
        }
    } else if(data->mask) {
        pm.setMask(data->mask->xForm(matrix));
    }
    if(data->alphapm)
        pm.data->alphapm = new QPixmap(data->alphapm->xForm(matrix));
    return pm;
}

void QPixmap::init(int w, int h, int d, bool bitmap, Optimization optim)
{
    if (qApp->type() == QApplication::Tty)
        qWarning("QPixmap: Cannot create a QPixmap when no GUI "
                  "is being used");

    if(d != 32 && d != 1)
        d = 32; //magic number.. we always use a 32 bit depth for non-bitmaps

    static int serial = 0;

    data = new QPixmapData;
    data->hd = 0;
    data->cg_hd = 0;
    memset(data, 0, sizeof(QPixmapData));
    data->count=1;
    data->uninit=true;
    data->bitmap=bitmap;
    data->clut = 0;
    data->ser_no=++serial;
    data->optim=optim;

    int dd = 32; //magic number? 32 seems to be default?
    bool make_null = w == 0 || h == 0;                // create null pixmap
    if(d == 1)                                // monocrome pixmap
        data->d = 1;
    else if(d < 0 || d == dd)                // def depth pixmap
        data->d = dd;
    if(make_null || w < 0 || h < 0 || data->d == 0) {
        data->hd = 0;
        if(!make_null)
            qWarning("Qt: QPixmap: Invalid pixmap parameters");
        return;
    }

    if(w<1 || h<1)
        return;
    data->w=w;
    data->h=h;

    Rect rect;
    SetRect(&rect,0,0,w,h);
    QDErr e = -1;
    const int params = alignPix | stretchPix | newDepth;
#if 1
    if(optim == BestOptim) //try to get it into distant memory
        e = NewGWorld(reinterpret_cast<GWorldPtr *>(&data->hd), 32, &rect,
                      data->clut ? &data->clut : 0, 0, useDistantHdwrMem | params);
    if(e != noErr) //oh well I tried
#endif
        e = NewGWorld(reinterpret_cast<GWorldPtr *>(&data->hd), 32, &rect,
                      data->clut ? &data->clut : 0, 0, params);

    /* error? */
    if(e != noErr) {
        data->w = data->h = 0;
        data->cg_hd = data->hd = 0; //just to be sure
        qDebug("Qt: internal: QPixmap::init error (%d) (%d %d %d %d)", e, rect.left, rect.top, rect.right, rect.bottom);
    } else {
        bool locked = LockPixels(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
        Q_ASSERT(locked);
        data->w=w;
        data->h=h;
    }
}

int QPixmap::defaultDepth()
{
    GDHandle gd;
    gd=GetMainDevice();
    int wug=(**gd).gdCCDepth;
    if(wug) {
        return wug;
    } else {
        return 16;
    }
}

void QPixmap::setOptimization(Optimization)
{
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
        pm = QPixmap(w, h, 32);
        bitBlt(&pm, 0, 0, widget, x, y, w, h);
    }
    return pm;
}

/*!
    \internal
*/
Qt::HANDLE QPixmap::macCGHandle() const
{
    if(!data->cg_hd && data->hd) {
        CreateCGContextForPort(static_cast<CGrafPtr>(data->hd), reinterpret_cast<CGContextRef*>(&data->cg_hd));
        SyncCGContextOriginWithPort(static_cast<CGContextRef>(data->cg_hd), static_cast<CGrafPtr>(data->hd));
        CGContextTranslateCTM(static_cast<CGContextRef>(data->cg_hd), 0, height());
        CGContextScaleCTM(static_cast<CGContextRef>(data->cg_hd), 1, -1);
    }
    return data->cg_hd;
}

bool QPixmap::hasAlpha() const
{
    return data->alphapm || data->mask;
}

bool QPixmap::hasAlphaChannel() const
{
    return data->alphapm != 0;
}

Q_GUI_EXPORT void copyBlt(QPixmap *dst, int dx, int dy,
                       const QPixmap *src, int sx, int sy, int sw, int sh)
{
    if (! dst || ! src || sw == 0 || sh == 0 || dst->depth() != src->depth()) {
#ifdef QT_CHECK_0
        Q_ASSERT(dst != 0);
        Q_ASSERT(src != 0);
#endif
        return;
    }

    // copy pixel data
    bitBlt(dst, dx, dy, src, sx, sy, sw, sh, true);

    // copy mask data
    if (src->data->mask) {
        if (! dst->data->mask) {
            dst->data->mask = new QBitmap(dst->width(), dst->height());

            // new masks are fully opaque by default
            dst->data->mask->fill(Qt::color1);
        }

        bitBlt(dst->data->mask, dx, dy,
	       src->data->mask, sx, sy, sw, sh, true);
    }

#ifdef QMAC_PIXMAP_ALPHA
    // copy alpha data
    if (! src->data->alphapm)
        return;

    if (sw < 0)
        sw = src->width() - sx;
    else
        sw = qMin(src->width()-sx, sw);
    sw = qMin(dst->width()-dx, sw);

    if (sh < 0)
        sh = src->height() - sy ;
    else
        sh = qMin(src->height()-sy, sh);
    sh = qMin(dst->height()-dy, sh);

    if (sw <= 0 || sh <= 0)
        return;

    if (! dst->data->alphapm) {
        dst->data->alphapm = new QPixmap(dst->data->w, dst->data->h, 32);

        // new alpha pixmaps are fully opaque by default
        dst->data->alphapm->fill(Qt::black);
    }

    bitBlt(dst->data->alphapm, dx, dy,
            src->data->alphapm, sx, sy, sw, sh, true);
#endif // QMAC_PIXMAP_ALPHA
}

IconRef qt_mac_create_iconref(const QPixmap &px)
{
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
//            { kHuge32BitData,       48,  48, 32, false },
//            { kLarge32BitData,      32,  32, 32, false },
//            { kSmall32BitData,      16,  16, 32, false },
            //masks
            { kThumbnail8BitMask, 128, 128, 8, true },
//            { kHuge1BitMask,       48,  48, 1, true },
//            { kLarge1BitMask,      32,  32, 1, true },
//            { kSmall1BitMask,      16, 16,  1, true },
            { 0, 0, 0, 0, false } };
        for(int i = 0; images[i].mac_type; i++) {
            const QPixmap *in_pix = 0;
            if(images[i].mask)
                in_pix = px.mask();
            else if(!px.isNull())
                in_pix = &px;

            const int x_rows = images[i].width * (images[i].depth/8), y_rows = images[i].height;
            Handle hdl = NewHandle(x_rows*y_rows);
            if(in_pix) {
                //make the image
                QImage im;
                im = *in_pix;
                im = im.smoothScale(images[i].width, images[i].height).convertDepth(images[i].depth);
                //set handle bits
                if(images[i].mac_type == kThumbnail8BitMask) {
                    for(int y = 0, h = 0; y < im.height(); y++) {
                        for(int x = 0; x < im.width(); x++)
                            *((*hdl)+(h++)) = im.pixel(x, y) ? 0 : 255;
                    }
                } else {
                    for(int y = 0; y < y_rows; y++)
                        memcpy((*hdl)+(y*x_rows), im.scanLine(y), x_rows);
                }
            } else {
                memset((*hdl), 0xFF, x_rows*y_rows);
            }
            OSStatus set = SetIconFamilyData(iconFamily, images[i].mac_type, hdl);
            if(set != noErr)
                qWarning("%s: %d -- Something went very wrong!! %ld", __FILE__, __LINE__, set);
            DisposeHandle(hdl);
        }
    }

    //acquire and cleanup
    IconRef ret;
    const OSType kQtCreator = 'CUTE';
    RegisterIconRefFromIconFamily(kQtCreator, (OSType)px.serialNumber(), iconFamily, &ret);
    AcquireIconRef(ret);
    UnregisterIconRef(kQtCreator, (OSType)px.serialNumber());
    DisposeHandle(reinterpret_cast<Handle>(iconFamily));
    return ret;

}

QPixmap qt_mac_convert_iconref(IconRef icon, int width, int height)
{
    QPixmap ret(width, height);
    Rect rect;
    SetRect(&rect, 0, 0, width, height);
    {
        QMacSavedPortInfo pi(&ret);
        PlotIconRef(&rect, kAlignNone, kTransformNone, kIconServicesNormalUsageFlag, icon);
    }
    if(!IsIconRefMaskEmpty(icon)) {
        QBitmap mask(width, height, true);
        QMacSavedPortInfo pi(&mask);
        PlotIconRef(&rect, kAlignNone, kTransformNone, kIconServicesNormalUsageFlag, icon);
        ret.setMask(mask);
    }
    return ret;
}

CGImageRef qt_mac_create_cgimage(const QPixmap &px, bool imask)
{
    if(px.isNull())
        return 0;

    const uint bpl = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(px.handle())));
    char *addr = GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(px.handle())));
    CGDataProviderRef provider = CGDataProviderCreateWithData(0, addr, bpl*px.height(), 0);
    CGImageRef image = 0;
    if(px.isQBitmap()) {
        image = CGImageMaskCreate(px.width(), px.height(), 1, 1, 1, provider, 0, false);
    } else {
        if(!imask) {
            if(const QPixmap *alpha = px.data->alphapm) {
                char *drow;
                long *aptr = reinterpret_cast<long *>(GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(alpha->handle())))),
                     *arow;
                unsigned short abpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(alpha->handle())));
                const int h = alpha->height(), w = alpha->width();
                for(int yy=0; yy<h; yy++) {
                    arow = reinterpret_cast<long *>(reinterpret_cast<char *>(aptr) + (yy * abpr));
                    drow = addr + (yy * bpl);
                    for(int xx=0;xx<w;xx++)
                        *(drow + (xx*4)) = 255-(*(arow + xx) & 0xFF);
                }
            } else if(const QBitmap *mask = px.mask()) {
                char *drow;
                long *mptr = reinterpret_cast<long *>(GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(mask->handle())))),
                *mrow;
                unsigned short mbpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(mask->handle())));
                const int h = mask->height(), w = mask->width();
                for(int yy=0; yy<h; yy++) {
                    mrow = reinterpret_cast<long *>(reinterpret_cast<char *>(mptr) + (yy * mbpr));
                    drow = addr + (yy * bpl);
                    for(int xx=0;xx<w;xx++) {
                        *(drow + (xx*4)) = (*(mrow + xx) & 0x01) ? 0 : 255;
                    }
                }
            } else {
                imask = true; //there isn't really a "mask"
            }
        }
        CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
        image = CGImageCreate(px.width(), px.height(), 8, 32, bpl, colorspace,
                              imask ? kCGImageAlphaNoneSkipFirst : kCGImageAlphaFirst,
                              provider, 0, 0, kCGRenderingIntentDefault);
        CGColorSpaceRelease(colorspace);
    }
    CGDataProviderRelease(provider);
    return image;
}

/*! \internal */
QPaintEngine *QPixmap::paintEngine() const
{
    if (!data->paintEngine) {
#if !defined(QMAC_NO_COREGRAPHICS)
        if(!getenv("QT_MAC_USE_QUICKDRAW"))
            data->paintEngine = new QCoreGraphicsPaintEngine(const_cast<QPixmap *>(this));
        else
#endif
            data->paintEngine = new QQuickDrawPaintEngine(const_cast<QPixmap*>(this));
    }
    return data->paintEngine;
}
