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

#include "qpixmap.h"
#include "qpixmap_p.h"
#include "qimage.h"
#include "qapplication.h"
#include "qbitmap.h"
#include "qmatrix.h"
#include <private/qpaintengine_mac_p.h>
#include <private/qt_mac_p.h>

#include <limits.h>
#include <string.h>

/*****************************************************************************
  Externals
 *****************************************************************************/
extern const uchar *qt_get_bitflip_array();                // defined in qimage.cpp
extern GrafPtr qt_macQDHandle(const QPaintDevice *); //qpaintdevice_mac.cpp
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QRegion qt_mac_convert_mac_region(RgnHandle rgn); //qregion_mac.cpp

/*****************************************************************************
  QPixmap member functions
 *****************************************************************************/
QPixmap::QPixmap(int w, int h, const uchar *bits, bool isXbitmap)
    : QPaintDevice(QInternal::Pixmap)
{
    init(w, h, 1, true, DefaultOptim);
    Q_ASSERT_X(data->hd, "QPixmap::QPixmap", "No handle");

    int *dptr = (int *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd))), *drow, q;
    unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
    for(int yy=0;yy<h;yy++) {
        drow = (int *)((char *)dptr + (yy * dbpr));
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

bool QPixmap::fromImage(const QImage &img, Qt::ImageConversionFlags flags)
{
    if(img.isNull()) {
        qWarning("QPixmap::convertFromImage: Cannot convert a null image");
        return false;
    }

    QImage image = img;
    int    d     = image.depth();
    int    dd    = defaultDepth();
    bool force_mono = (dd == 1 || isQBitmap() ||
                       (flags & Qt::ColorMode_Mask)==Qt::MonoOnly);
    if(force_mono) {                         // must be monochrome
        if(d != 1) {
            image = image.convertDepth(1, flags);  // dither
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
            image = image.convertDepth(8, flags);
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

    int *dptr = (int *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd))), *drow;
    unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));

    QRgb q=0;
    int sdpt = image.depth();
    unsigned short sbpr = image.bytesPerLine();
    uchar *sptr = image.bits(), *srow;
    QImage::Endian sord = image.bitOrder();

    for(int yy=0;yy<h;yy++) {
        drow = (int *)((char *)dptr + (yy * dbpr));
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
            qWarning("Qt: internal: Oops: Forgot a depth %s:%d", __FILE__, __LINE__);
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
            m = img.createAlphaMask(flags);
            setMask(m);
        }
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
            int *dptr = (int *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->alphapm->data->hd))),
                 *drow;
            unsigned short dbpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
            if (img.depth() == 32) {
                unsigned short sbpr = image.bytesPerLine();
                int *sptr = (int*)image.bits(), *srow;
                uchar clr;
                for(int yy=0; yy < h; yy++) {
                    drow = (int*)((char *)dptr + (yy * dbpr));
                    srow = (int*)((char *)sptr + (yy * sbpr));
                    for (int xx=0; xx < w; xx++) {
                        clr = ~(((*(srow + xx)) >> 24) & 0xFF);
                        *(drow + xx) = qRgba(clr, clr, clr, 0);
                    }
                }
            } else {
                const QRgb *const rgb = img.colorTable();
                for (int y = 0; y < h; ++y) {
                    const uchar *iptr = image.scanLine(y);
                    drow = (int*)((char *)dptr + (y * dbpr));
                    for (int x = 0; x < w; ++x) {
                        const int alpha = ~qAlpha(rgb[*iptr++]);
                        *(drow + x) = qRgba(alpha, alpha, alpha, 0);
                    }
                }
            }
        }
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

QImage QPixmap::toImage() const
{
    if(!data->w || !data->h || !data->hd)
        return QImage(); // null image

    int w = data->w;
    int h = data->h;
    int d = data->d;
    int ncols = 2;

    if(d != 1) { //Doesn't support index color modes
        d = 32;
        ncols = 0;
    }

    QImage image(w, h, d, ncols, QImage::BigEndian);
    if(d == 1) {
        image.setNumColors(2);
        image.setColor(0, qRgba(255, 255, 255, 0));
        image.setColor(1, qRgba(0, 0, 0, 0));
    }

    Q_ASSERT_X(data->hd, "QPixmap::convertToImage", "No handle");

    QRgb q;
    int *sptr = reinterpret_cast<int*>(GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)))),
                *srow, r;
    unsigned short sbpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));

    int *aptr = 0, *arow = 0;
    unsigned short abpr = 0;
    if(data->alphapm) {
        image.setAlphaBuffer(true);
        aptr = reinterpret_cast<int*>(GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->alphapm->data->hd))));
        abpr = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->alphapm->data->hd)));
    }

    for(int yy=0;yy<h;yy++) {
        srow = (int *)((char *)sptr + (yy * sbpr));
        if(aptr)
            arow = (int *)((char *)aptr + (yy * abpr));
        for(int xx=0;xx<w;xx++) {
            r = *(srow + xx);
            q=qRgba((r >> 16) & 0xFF, (r >> 8) & 0xFF, r & 0xFF, (arow ? ~(*(arow + xx) & 0xFF) : 0xFF));
            if(d == 1)
                image.setPixel(xx, yy, (q & RGB_MASK) ? 0 : 1);
            else if(ncols)
                image.setPixel(xx, yy, get_index(&image,q));
            else
                image.setPixel(xx, yy, q);
        }
    }

    if(data->mask && !data->alphapm) {
        QImage alpha = data->mask->toImage();
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

    QMacSavedPortInfo saveportstate(this);
    detach();                                        // detach other references
    { //we don't know what backend to use, and we cannot paint here
        uint *dptr = (uint *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
        int dbytes = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)))*height();
        Q_ASSERT_X(dptr && dbytes, "QPixmap::fill", "No dptr or no dbytes");
        QRgb colr = fillColor.rgba();
        if(!colr) {
            memset(dptr, colr, dbytes);
        } else {
            for(int i = 0; i < (int)(dbytes/sizeof(uint)); i++)
                *(dptr + i) = colr;
        }
    }
    if(fillColor.alpha() == 255) {
        delete data->alphapm;
        data->alphapm = 0;
    } else { 
        if(!data->alphapm)
            data->alphapm = new QPixmap(data->w, data->h, 32);
        uint *aptr = (uint *)GetPixBaseAddr(GetGWorldPixMap(static_cast<GWorldPtr>(data->alphapm->data->hd)));
        int abytes = GetPixRowBytes(GetGWorldPixMap(static_cast<GWorldPtr>(data->alphapm->data->hd)))*height();
        Q_ASSERT_X(aptr && abytes, "QPixmap::fill", "No aptr or no abytes");
        QRgb colr = qRgba(255-fillColor.alpha(), 255-fillColor.alpha(), 255-fillColor.alpha(), 0);
        if(!colr) {
            memset(aptr, colr, abytes);
        } else {
            for(int i = 0; i < (int)(abytes/sizeof(uint)); i++)
                *(aptr + i) = colr;
        }
    }
}

void QPixmap::detach()
{
    if(data->uninit || data->count == 1) {
        data->uninit = false;
        if(data->cgimage) {
            CGImageRelease(data->cgimage);
            data->cgimage = 0;
        }
    } else {
        *this = copy();
    }
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
            if(GDHandle gd = GetGWorldDevice(static_cast<GWorldPtr>(handle()))) {
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
    if(mask)
        delete mask;
    if(alphapm)
        delete alphapm;

    if(cgimage)
        CGImageRelease(cgimage);

    if(hd && qApp) {
        UnlockPixels(GetGWorldPixMap(static_cast<GWorldPtr>(hd)));
        DisposeGWorld(static_cast<GWorldPtr>(hd));
    }
}

QPixmap QPixmap::transform(const QMatrix &matrix, Qt::TransformationMode mode) const
{
    if (mode == Qt::SmoothTransformation) {
        // ###### do this efficiently!
        QImage image = toImage();
        return QPixmap(image.transform(matrix, mode));
    }

    int w, h;                                // size of target pixmap
    int ws, hs;                                // size of source pixmap
    uchar *dptr;                                // data in target pixmap
    int dbpl, dbytes;                        // bytes per line/bytes total
    uchar *sptr;                                // data in original pixmap
    int sbpl;                                // bytes per line in original
    int bpp;                                        // bits per pixel

    if(isNull())                                // this is a null pixmap
        return copy();

    ws = width();
    hs = height();

    QMatrix mat(matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(), 0., 0.);

    if(matrix.m12() == 0.0F  && matrix.m21() == 0.0F &&
         matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F) {
        if(mat.m11() == 1.0F && mat.m22() == 1.0F)
            return *this;                        // identity matrix
        h = qRound(mat.m22()*hs);
        w = qRound(mat.m11()*ws);
        h = qAbs(h);
        w = qAbs(w);
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
    pm.fill(0x00FFFFFF);
    dptr = (uchar *)GetPixBaseAddr(GetGWorldPixMap(qt_macQDHandle(&pm)));
    dbpl = GetPixRowBytes(GetGWorldPixMap(qt_macQDHandle(&pm)));
    bpp = 32;
    dbytes = dbpl*h;

    int        xbpl = bpp == 1 ? ((w+7)/8) : ((w*bpp)/8);
    if(!qt_xForm_helper(mat, 0, QT_XFORM_TYPE_MSBFIRST, bpp,
                        dptr, xbpl, dbpl - xbpl, h, sptr, sbpl, ws, hs)){
        qWarning("Qt: QPixmap::transform: display not supported (bpp=%d)",bpp);
        QPixmap pm;
        return pm;
    }

    if(depth() == 1) {
        if(data->mask) {
            if(data->selfmask)               // pixmap == mask
                pm.setMask(*((QBitmap*)(&pm)));
            else
                pm.setMask(data->mask->transform(matrix));
        }
    } else if(data->mask) {
        pm.setMask(data->mask->transform(matrix));
    }
    if(data->alphapm)
        pm.data->alphapm = new QPixmap(data->alphapm->transform(matrix));
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
    data->cgimage = 0;
    data->hd = 0;
    memset(data, 0, sizeof(QPixmapData));
    data->count=1;
    data->uninit=true;
    data->bitmap=bitmap;
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
                      0, 0, useDistantHdwrMem | params);
    if(e != noErr) //oh well I tried
#endif
        e = NewGWorld(reinterpret_cast<GWorldPtr *>(&data->hd), 32, &rect,
                      0, 0, params);

    /* error? */
    if(e != noErr) {
        data->w = data->h = 0;
        data->hd = 0; //just to be sure
        qWarning("Qt: internal: QPixmap::init error (%d) (%d %d %d %d)", e, rect.left, rect.top, rect.right, rect.bottom);
    } else {
        bool locked = LockPixels(GetGWorldPixMap(static_cast<GWorldPtr>(data->hd)));
        Q_ASSERT(locked);
        Q_UNUSED(locked);
        data->w=w;
        data->h=h;
    }
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
        extern WindowPtr qt_mac_window_for(const QWidget *); // qwidget_mac.cpp
        const BitMap *windowPort = 0;
        if(widget->isDesktop()) {
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
        const BitMap *pixmapPort = GetPortBitMapForCopyBits(static_cast<GWorldPtr>(pm.handle()));
        Rect macSrcRect, macDstRect;
        SetRect(&macSrcRect, x, y, x + w, y + h);
        SetRect(&macDstRect, 0, 0, w, h);
        CopyBits(windowPort, pixmapPort, &macSrcRect, &macDstRect, srcCopy, 0);
    }
    return pm;
}

bool QPixmap::hasAlpha() const
{
    return data->alphapm || data->mask;
}

bool QPixmap::hasAlphaChannel() const
{
    return data->alphapm != 0;
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
                im = in_pix->toImage();
                im = im.scale(images[i].width, images[i].height,
                              Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                     .convertDepth(images[i].depth);
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
    Rect rect;
    SetRect(&rect, 0, 0, width, height);
    {
        QMacSavedPortInfo pi(&ret);
        PlotIconRef(&rect, kAlignNone, kTransformNone, kIconServicesNormalUsageFlag, icon);
    }
    if(!IsIconRefMaskEmpty(icon)) {
        QBitmap bitmap(width, height, true);
        {
            QPainter p(&bitmap);
            RgnHandle mask = qt_mac_get_rgn();
            IconRefToRgn(mask, &rect, kAlignNone, kIconServicesNormalUsageFlag, icon);
            p.setClipRegion(qt_mac_convert_mac_region(mask));
            qt_mac_dispose_rgn(mask);
            p.fillRect(0, 0, width, height, Qt::color1);
            p.end();
        }
        ret.setMask(bitmap);
    }
    return ret;
}

static void qt_mac_cgimage_data_free(void *, const void *data, size_t)
{
    free(const_cast<void *>(data));
}

CGImageRef qt_mac_create_cgimage(const QPixmap &px, Qt::PixmapDrawingMode mode, bool mask)
{
    if(px.isNull())
        return 0;
    if(CGImageRef cache_img = px.data->cgimage) {
        CGImageRetain(cache_img); //for the caller
        return cache_img;
    }

    const uint bpl = GetPixRowBytes(GetGWorldPixMap(qt_macQDHandle(&px)));
    char *addr = GetPixBaseAddr(GetGWorldPixMap(qt_macQDHandle(&px)));
    CGDataProviderRef provider = 0;
    CGImageRef image = 0;
    if(mask) {
        Q_ASSERT(px.isQBitmap());
        const int w = px.width(), h = px.height();
        char *out_addr = (char*)malloc(w*h);
        provider = CGDataProviderCreateWithData(0, out_addr, w*h, qt_mac_cgimage_data_free);

        const QRgb c0 = (QColor(Qt::color0).rgb() & 0xFFFFFF);
        for(int yy = 0; yy < h; yy++) {
            char *out_row = out_addr + (yy * px.width());
            uint *in_row = reinterpret_cast<uint*>(reinterpret_cast<char *>(addr) + (yy * bpl));
            for(int xx = 0; xx < w; xx++)
                *(out_row+xx) = ((*(in_row+xx) & c0) == c0) ? 255 : 0;
        }
        image = CGImageMaskCreate(px.width(), px.height(), 8, 8, px.width(), provider, 0, true);
    } else {
        char *out_addr = (char*)malloc(px.height()*bpl);
        memcpy(out_addr, addr, px.height()*bpl);
        if(mode == Qt::ComposePixmap) {
            if(const QPixmap *alpha = px.data->alphapm) {
                char *drow;
                int *aptr = reinterpret_cast<int*>(GetPixBaseAddr(GetGWorldPixMap(qt_macQDHandle(alpha)))),
                     *arow;
                unsigned short abpr = GetPixRowBytes(GetGWorldPixMap(qt_macQDHandle(alpha)));
                const int h = alpha->height(), w = alpha->width();
                for(int yy=0; yy<h; yy++) {
                    arow = reinterpret_cast<int*>(reinterpret_cast<char *>(aptr) + (yy * abpr));
                    drow = out_addr + (yy * bpl);
                    for(int xx=0;xx<w;xx++)
                        *(drow + (xx*4)) = 255-(*(arow + xx) & 0xFF);
                }
            } else if(const QBitmap *mask = px.mask()) {
                char *mptr = reinterpret_cast<char*>(GetPixBaseAddr(GetGWorldPixMap(qt_macQDHandle(mask))));
                unsigned short mbpr = GetPixRowBytes(GetGWorldPixMap(qt_macQDHandle(mask)));
                const int h = mask->height(), w = mask->width();
                const QRgb c0 = (QColor(Qt::color0).rgb() & 0xFFFFFF);
                for(int yy=0; yy<h; yy++) {
                    uint *mrow = reinterpret_cast<uint*>(mptr + (yy * mbpr));
                    char *drow = out_addr + (yy * bpl);
                    for(int xx=0;xx<w;xx++)
                        *(drow + (xx*4)) = ((*(mrow + xx) & c0) == c0) ? 0 : 255;
                }
            } else {
                mode = Qt::CopyPixmap; //there isn't really a "mask"
            }
        }
        CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
        provider = CGDataProviderCreateWithData(0, out_addr, bpl*px.height(),
                                                qt_mac_cgimage_data_free);
        image = CGImageCreate(px.width(), px.height(), 8, 32, bpl, colorspace,
                              mode == Qt::ComposePixmap ? kCGImageAlphaFirst : kCGImageAlphaNoneSkipFirst,
                              provider, 0, 0, kCGRenderingIntentDefault);
        CGColorSpaceRelease(colorspace);
    }
    CGDataProviderRelease(provider);
    {
        px.data->cgimage = image;
        CGImageRetain(px.data->cgimage);
    }
    return image;
}

/*! \internal */
QPaintEngine *QPixmap::paintEngine() const
{
    if (!data->paintEngine) {
#if !defined(QMAC_NO_COREGRAPHICS)
        if(!qgetenv("QT_MAC_USE_QUICKDRAW"))
            data->paintEngine = new QCoreGraphicsPaintEngine();
        else
#endif
            data->paintEngine = new QQuickDrawPaintEngine();
    }
    return data->paintEngine;
}
