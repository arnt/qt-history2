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
#ifdef QT_RASTER_PAINTENGINE
#  include <private/qpaintengine_raster_p.h>
#else
#  include <private/qpaintengine_mac_p.h>
#endif
#include <private/qt_mac_p.h>

#include <limits.h>
#include <string.h>

/*****************************************************************************
  Externals
 *****************************************************************************/
extern const uchar *qt_get_bitflip_array();                // defined in qimage.cpp
extern GrafPtr qt_mac_qd_context(const QPaintDevice *); //qpaintdevice_mac.cpp
extern RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
extern void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QRegion qt_mac_convert_mac_region(RgnHandle rgn); //qregion_mac.cpp

static int qt_pixmap_serial = 0;

static void qt_mac_cgimage_data_free(void *, const void *data, size_t)
{
    free(const_cast<void *>(data));
}

/*****************************************************************************
  QPixmap member functions
 *****************************************************************************/
QPixmap::QPixmap(int w, int h, const uchar *bits, bool isXbitmap)
    : QPaintDevice(QInternal::Pixmap)
{
    init(w, h, 1, true);

    uint *dptr = data->pixels, *drow, q;
    const uint bytesPerRow = data->nbytes / h;
    for(int yy=0;yy<h;yy++) {
        drow = (uint *)((char *)dptr + (yy * bytesPerRow));
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

    // different size or depth, make a new pixmap
    pixmap = QPixmap(w, h, d == 1 ? 1 : -1);

    uint *dptr = pixmap.data->pixels, *drow;
    const uint dbpr = pixmap.data->nbytes / h;

    QRgb q=0;
    int sdpt = image.depth();
    const unsigned short sbpr = image.bytesPerLine();
    uchar *sptr = image.bits(), *srow;
    QImage::Endian sord = image.bitOrder();

    for(int yy=0;yy<h;yy++) {
        drow = (uint*)((char *)dptr + (yy * dbpr));
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
    if(image.hasAlphaBuffer()) { //setup the alpha
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
    int d = data->d;
    int ncols = 2;

    if(d != 1) { //Doesn't support index color modes
        d = 32;
        ncols = 0;
    }

    QImage image(w, h, d, ncols, QImage::BigEndian);
    image.setAlphaBuffer(data->has_alpha);
    if(d == 1) {
        image.setNumColors(2);
        image.setColor(0, qRgba(255, 255, 255, 0));
        image.setColor(1, qRgba(0, 0, 0, 0));
    }

    QRgb q;
    uint *sptr = data->pixels, *srow, r;
    const uint bytesPerRow = data->nbytes / h;
    for(int yy=0;yy<h;yy++) {
        srow = (uint *)((char *)sptr + (yy * bytesPerRow));
        for(int xx=0;xx<w;xx++) {
            r = *(srow + xx);
            q=qRgba((r >> 16) & 0xFF, (r >> 8) & 0xFF, r & 0xFF, (r >> 24) & 0xFF);
            if(d == 1)
                image.setPixel(xx, yy, (q & RGB_MASK) ? 0 : 1);
            else if(ncols)
                image.setPixel(xx, yy, get_index(&image,q));
            else
                image.setPixel(xx, yy, q);
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
        uint *dptr = data->pixels;
        Q_ASSERT_X(dptr, "QPixmap::fill", "No dptr");
        QRgb colr = fillColor.rgba();
        if(!colr) {
            memset(dptr, colr, data->nbytes);
        } else {
            for(uint i = 0; i < data->nbytes/sizeof(uint); i++)
                *(dptr + i) = colr;
        }
    }
    data->macSetHasAlpha(fillColor.alpha() != 255);
}

QPixmap QPixmap::alphaChannel() const
{
    if (!data->has_alpha)
        return QPixmap();
    QPixmap alpha(width(), height(), 32);
    data->macGetAlphaChannel(&alpha);
    return alpha;
}

void QPixmap::setAlphaChannel(const QPixmap &alpha)
{
    if (data == alpha.data) // trying to alpha
        return;

    if (alpha.width() != width() || alpha.height() != height()) {
        qWarning("QPixmap::setAlphaChannel: The pixmap and the mask must have the same size");
        return;
    }
    detach();

    data->has_mask = false;
    if(alpha.isNull()) {
        data->has_mask = false;
        QPixmap opaque(width(), height());
        opaque.fill(QColor(255, 255, 255, 255));
        data->macSetAlphaChannel(&opaque);
    } else {
        data->has_mask = true;
        data->macSetAlphaChannel(&alpha);
    }
}

QBitmap QPixmap::mask() const
{
    if (!data->has_mask)
        return QBitmap();
    QBitmap mask(width(), height());
    data->macGetAlphaChannel(&mask);
    return mask;
}

void QPixmap::setMask(const QBitmap &newmask)
{
    if (data == newmask.data) // trying to selfmask
        return;

    if (newmask.width() != width() || newmask.height() != height()) {
        qWarning("QPixmap::setMask: The pixmap and the mask must have the same size");
        return;
    }
    detach();

    data->has_alpha = false;
    if(newmask.isNull()) {
        data->has_mask = false;
        QPixmap opaque(width(), height());
        opaque.fill(QColor(255, 255, 255, 255));
        data->macSetAlphaChannel(&opaque);
    } else {
        data->has_mask = true;
        data->macSetAlphaChannel(&newmask);
    }
}

void QPixmap::detach()
{
    if (data->count != 1) {
        *this = copy();
        data->qd_alpha = 0; //leave it behind
    }
    data->uninit = false;
    data->ser_no = ++qt_pixmap_serial;
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
    macQDDisposeAlpha();
    if(qd_data) {
        DisposeGWorld(qd_data);
        qd_data = 0;
    }
    if(cg_data) {
        CGImageRelease(cg_data);
        cg_data = 0;
        pixels = 0; //let the cgimage hang onto the pixels if it wants to
    }
}

void
QPixmapData::macSetAlphaChannel(const QPixmap *pix)
{
    uchar *dptr = (uchar*)pixels, *drow;
    const uint dbpr = nbytes / h;
    const unsigned short sbpr = pix->data->nbytes / pix->data->h;
    uchar *sptr = (uchar*)pix->data->pixels, *srow;
    for(int yy=0; yy < h; yy++) {
        drow = dptr + (yy * dbpr);
        srow = sptr + (yy * sbpr);
        for(int xx=0; xx < w*4; xx+=4)
            *(drow+xx) = *(srow+xx);
    }
    macSetHasAlpha(true);
}

void
QPixmapData::macGetAlphaChannel(QPixmap *pix) const
{
    uchar *dptr = (uchar*)pix->data->pixels, *drow;
    const uint dbpr = pix->data->nbytes / pix->data->h;
    const unsigned short sbpr = nbytes / h;
    uchar *sptr = (uchar*)pixels, *srow;
    for(int yy=0; yy < h; yy++) {
        drow = dptr + (yy * dbpr);
        srow = sptr + (yy * sbpr);
        for(int xx=0; xx < w*4; xx+=4)
            *(drow+xx) = *(srow+xx);
    }
}

void
QPixmapData::macSetHasAlpha(bool b)
{
    has_alpha = b;
#if 1
    macQDDisposeAlpha(); //let it get created lazily
#else
    macQDUpdateAlpha();
#endif
}

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
    for(int yy=0; yy < h; yy++) {
        drow = (int*)((char *)dptr + (yy * dbpr));
        srow = (int*)((char *)sptr + (yy * sbpr));
        for (int xx=0; xx < w; xx++) {
            clr = 255 - (((*(srow + xx)) >> 24) & 0xFF);
            *(drow + xx) = qRgba(clr, clr, clr, 0);
        }
    }
}

QPixmap QPixmap::transformed(const QMatrix &matrix, Qt::TransformationMode mode) const
{
    if (mode == Qt::SmoothTransformation) {
        // ###### do this efficiently! --Sam
        QImage image = toImage();
        return QPixmap(image.transformed(matrix, mode));
    }
    if(isNull())
        return copy();

    int w, h;  // size of target pixmap
    const int ws = width();
    const int hs = height();

    QMatrix mat(matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(), 0., 0.);
    if(matrix.m12() == 0.0F  && matrix.m21() == 0.0F &&
         matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F) {
        if(mat.m11() == 1.0F && mat.m22() == 1.0F) // identity matrix
            return *this;
        h = qRound(mat.m22()*hs);
        w = qRound(mat.m11()*ws);
        h = qAbs(h);
        w = qAbs(w);
    } else { // rotation or shearing
        QPolygon a(QRect(0,0,ws+1,hs+1));
        a = mat.map(a);
        QRect r = a.boundingRect().normalize();
        w = r.width()-1;
        h = r.height()-1;
    }
    mat = trueMatrix(mat, ws, hs);
    bool invertible;
    mat = mat.inverted(&invertible);
    if(!h || !w || !invertible) // error
        return QPixmap();

    //create destination
    QPixmap pm(w, h, depth());
    pm.fill(0x00FFFFFF);
    const uchar *sptr = (uchar *)data->pixels;
    uchar *dptr = (uchar *)pm.data->pixels;

    const int bpp = 32;
    const int xbpl = bpp == 1 ? ((w+7)/8) : ((w*bpp)/8);

    //do the transform
    if(!qt_xForm_helper(mat, 0, QT_XFORM_TYPE_MSBFIRST, bpp,
                        dptr, xbpl, (pm.data->nbytes / pm.data->h) - xbpl, h, sptr,
                        (data->nbytes / data->h), ws, hs)){
        qWarning("Qt: QPixmap::transform: display not supported (bpp=%d)",bpp);
        QPixmap pm;
        return pm;
    }

    //update the alpha
    pm.data->macSetHasAlpha(data->has_alpha);
    pm.data->has_mask = data->has_mask;
    return pm;
}


void QPixmap::init(int w, int h, int d, bool bitmap)
{
    if (qApp->type() == QApplication::Tty)
        qWarning("QPixmap: Cannot create a QPixmap when no GUI "
                  "is being used");

    if(d != 32 && d != 1)
        d = 32; //magic number.. we always use a 32 bit depth for non-bitmaps

    data = new QPixmapData;
    memset(data, 0, sizeof(QPixmapData));
    data->count = 1;
    data->uninit = true;
    data->bitmap = bitmap;
    data->ser_no = ++qt_pixmap_serial;

    int dd = 32; //magic number? 32 seems to be default?
    bool make_null = w == 0 || h == 0;                // create null pixmap
    if(d == 1)                                // monocrome pixmap
        data->d = 1;
    else if(d < 0 || d == dd)                // def depth pixmap
        data->d = dd;
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
    data->nbytes = (w*h*4) + (h*4); // ### testing for alignment --Sam
    data->pixels = (uint*)malloc(data->nbytes);

    //create the cg data
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef provider = CGDataProviderCreateWithData(0, data->pixels, data->nbytes, qt_mac_cgimage_data_free);
    data->cg_data = CGImageCreate(w, h, 8, 32, data->nbytes / h, colorspace,
                                  kCGImageAlphaFirst, provider, 0, 0, kCGRenderingIntentDefault);
    CGColorSpaceRelease(colorspace);
    CGDataProviderRelease(provider);

    //create the qd data
    Rect rect;
    SetRect(&rect, 0, 0, w, h);
    if(NewGWorldFromPtr(&data->qd_data, k32ARGBPixelFormat, &rect, 0, 0, 0, (char*)data->pixels,
                        data->nbytes / h) != noErr)
        qWarning("Qt: internal: QPixmap::init error (%d %d %d %d)", rect.left, rect.top, rect.right, rect.bottom);
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
        pm = QPixmap(w, h, 32);
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

Qt::HANDLE QPixmap::macQDHandle() const
{
    return data->qd_data;
}

Qt::HANDLE QPixmap::macQDAlphaHandle() const
{
    if(data->has_alpha || data->has_mask) {
        if(!data->qd_alpha) //lazily created
            data->macQDUpdateAlpha();
        return data->qd_alpha;
    }
    return 0;
}

Qt::HANDLE QPixmap::macCGHandle() const
{
    return data->cg_data;
}

bool QPixmap::hasAlpha() const
{
    return data->has_alpha || data->has_mask;
}

bool QPixmap::hasAlphaChannel() const
{
    return data->has_alpha;
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
            if(!px.isNull())
                in_pix = &px;

            const int x_rows = images[i].width * (images[i].depth/8), y_rows = images[i].height;
            Handle hdl = NewHandle(x_rows*y_rows);
            if(in_pix) {
                //make the image
                QImage im;
                im = in_pix->toImage();
                im = im.scaled(images[i].width, images[i].height,
                               Qt::IgnoreAspectRatio, Qt::SmoothTransformation)
                     .convertDepth(images[i].depth);
                //set handle bits
                if(images[i].mask) {
                    if(images[i].mac_type == kThumbnail8BitMask) {
                        for(int y = 0, h = 0; y < im.height(); y++) {
                            for(int x = 0; x < im.width(); x++)
                                *((*hdl)+(h++)) = im.pixel(x, y) ? 0 : 255;
                        }
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

/*! \internal */
QPaintEngine *QPixmap::paintEngine() const
{
    if (!data->paintEngine) {
#ifdef QT_RASTER_PAINTENGINE
        data->paintEngine = new QRasterPaintEngine();
#else
#if !defined(QMAC_NO_COREGRAPHICS)
        if(!qgetenv("QT_MAC_USE_QUICKDRAW"))
            data->paintEngine = new QCoreGraphicsPaintEngine();
        else
#endif
            data->paintEngine = new QQuickDrawPaintEngine();
#endif
    }
    return data->paintEngine;
}
