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

#include "qpixmap_p.h"
#include "qbitmap.h"
#include "qimage.h"
#include "qmatrix.h"
#include "qcolormap.h"
#include "qapplication.h"
#include <private/qapplication_p.h>
#include "qt_windows.h"
#include <limits.h>

#ifdef QT_RASTER_PAINTENGINE
#include "private/qpaintengine_raster_p.h"
#else
#include <private/qpaintengine_win_p.h>
#endif

extern const uchar *qt_get_bitflip_array();                // defined in qimage.cpp

#define DATA_MCPI               data->mcpi
#define DATA_MCPI_MCP           data->mcpi->mcp
#define DATA_MCPI_OFFSET        data->mcpi->offset

static bool mcp_system_unstable = false;

/*
  The QMultiCellPixmap class is strictly internal and used to
  implement the setOptimization(MemoryOptim) feature for Win9x.
*/

class QMultiCellPixmap {
public:
    struct FreeNode {
        FreeNode() : offset(0), size(0) {}
        FreeNode(short o, short s) : offset(o), size(s) {}
        FreeNode(const FreeNode &o) : offset(o.offset), size(o.size) {}
        FreeNode &operator =(const FreeNode &o) { offset = o.offset; size = o.size; return *this; }
        bool operator==(const FreeNode &o) { return (offset == o.offset && size == o.size); }
        short offset;
        short size;
    };
    typedef QList<FreeNode> FreeList;


    QMultiCellPixmap(int width, int depth, int maxHeight);
   ~QMultiCellPixmap();
    bool isEmpty() const
    {
        if(free_list.isEmpty())
            return true;
        FreeNode n = free_list.last();
        return n.offset == 0 && n.size == max_height;
    }
    QPixmap *sharedPixmap() const { return pixmap; }
//    HDC             handle()            const { return pixmap->winHDC(); }
    HBITMAP  hbm()            const { return pixmap->hbm(); }
    int             allocCell(int height);
    void     freeCell(int offset, int height);
    void     debugger(); // for debugging during development
private:
    QPixmap          *pixmap;
    int                   max_height;
    FreeList free_list;

public:
    QPixmapData::MemDC mem_dc;
};

struct QMCPI { // mem optim for win9x
    QMultiCellPixmap *mcp;
    int offset;
};

inline HBITMAP QPixmapData::bm() const { return mcp ? mcpi->mcp->hbm() : hbm; }

void QPixmap::initAlphaPixmap(uchar *bytes, int length, BITMAPINFO *bmi)
{
    if (data->mcp)
        QPixmapData::freeCell(data, true);
    DeleteObject(data->bm());

    HDC hdc = GetDC(0);
    data->hbm = CreateDIBSection(hdc, bmi, DIB_RGB_COLORS, (void**)&data->realAlphaBits, NULL, 0);
    ReleaseDC(0, hdc);
    if (bytes)
        memcpy(data->realAlphaBits, bytes, length);
}

static int qt_pixmap_serial = 0;

void QPixmap::init(int w, int h, int d, bool bitmap, Optimization optim)
{
    if (qApp->type() == QApplication::Tty) {
        qWarning("QPixmap: Cannot create a QPixmap when no GUI "
                  "is being used");
    }

    int dd = defaultDepth();

    if (optim == DefaultOptim)                // use default optimization
        optim = defOptim;

    data = new QPixmapData;

    memset(data, 0, sizeof(QPixmapData));
    data->count  = 1;
    data->uninit = true;
    data->bitmap = bitmap;
    data->ser_no = ++qt_pixmap_serial;
    data->optim         = optim;

    bool make_null = w == 0 || h == 0;                // create null pixmap
    if (d == 1)                                // monocrome pixmap
        data->d = 1;
    else if (d < 0 || d == dd)                // compatible pixmap
        data->d = dd;
    if (make_null || w < 0 || h < 0 || data->d == 0) {
        data->hbm = 0;
        if (!make_null)                        // invalid parameters
            qWarning("QPixmap: Invalid pixmap parameters");
        return;
    }
    data->w = w;
    data->h = h;
    if (data->optim == MemoryOptim && (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)) {
        if (data->allocCell(this) >= 0)                        // successful
            return;
    }

#ifndef Q_OS_TEMP
    if (data->d == dd)                        // compatible bitmap
        data->hbm = CreateCompatibleBitmap(qt_win_display_dc(), w, h);
    else                                        // monocrome bitmap
        data->hbm = CreateBitmap(w, h, 1, 1, 0);
#else
        // WinCE must use DIBSections instead of Compatible Bitmaps
        // so it's possible to get the colortable at a later point.

        // For 16bpp or 32bpp non-palettized images, the color table
        // must be three entries long; the entries must specify the
        // values of the red, green, and blue bitmasks. Also, the
        // biCompression field in the BITMAPINFOHEADER structure
        // should be set to BI_BITFIELDS. BI_RBG is not supported
        // for these bit depths.
        int   ncols           = data->d <= 8 ? 1<<data->d : 0;
        int   bmi_data_len    = sizeof(BITMAPINFO) + sizeof(RGBQUAD) * (data->d > 8 ? 3 : ncols);
        char *bmi_data        = new char[bmi_data_len];
        memset(bmi_data, 0, bmi_data_len);
        BITMAPINFO       *bmi = (BITMAPINFO*)bmi_data;
        BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi_data;
        bmh->biSize              = sizeof(BITMAPINFOHEADER);
        bmh->biWidth              = w;
        bmh->biHeight              = -h; // top-down bitmap
        bmh->biPlanes              = 1;
        bmh->biBitCount              = data->d;
#if defined(_WIN32_WCE_EMULATION) && defined(WIN32_PLATFORM_PSPC) && (WIN32_PLATFORM_PSPC < 310)
        // Old CE 30 emulator doesn't handle BI_BITFIELDS correctly
        bmh->biCompression    = BI_RGB;
#else
        bmh->biCompression    = (data->d > 8 ? BI_BITFIELDS : BI_RGB);
#endif
        bmh->biSizeImage      = 0;
        bmh->biClrUsed              = ncols;
        bmh->biClrImportant   = 0;
        QRgb *coltbl = (QRgb*)(bmi_data + sizeof(BITMAPINFOHEADER));
        if (ncols == 2) {
            coltbl[0] = 0x0;
            coltbl[1] = 0xffffff;
        } else if (ncols == 0 && data->d > 8) {
            switch(data->d) {
            case 16:
                coltbl[0] = 0x00F800; // R 1111100000000000
                coltbl[1] = 0x0007E0; // G 0000011111100000
                coltbl[2] = 0x00001F; // B 0000000000011111
                break;
            case 24:
                coltbl[0] = 0x0;      // R
                coltbl[1] = 0x0;      // G
                coltbl[2] = 0x0;      // B
                break;
            case 32:
                coltbl[0] = 0xff0000; // R
                coltbl[1] = 0x00ff00; // G
                coltbl[2] = 0x0000ff; // B
                break;
            }
        }
        DATA_HBM = CreateDIBSection(qt_win_display_dc(),
                                     bmi,
                                     DIB_RGB_COLORS,
                                     (void**)&(data->ppvBits),
                                     NULL,
                                     0);
        delete [] bmi_data;
#endif
    if (!data->hbm) {
        qErrnoWarning("QPixmap::init(): Pixmap allocation of size %d, %d failed", data->w, data->h);
        data->w = 0;
        data->h = 0;

        return;
    }
}

QPixmapData::~QPixmapData()
{
    if (mcp) {
        if (mcp_system_unstable) {        // all mcp's gone
            mcp = false;
            delete mcpi;
            mcpi = 0;
            hbm = 0;
        } else {
            freeCell(this, true);
        }
    }
    if (mask)
        delete mask;
    if (maskpm)
        delete maskpm;
    if (hbm) {
        DeleteObject(hbm);
        hbm = 0;
    }
    delete paintEngine;
}

QPixmap::QPixmap(int w, int h, const uchar *bits, bool isXbitmap)
    : QPaintDevice(QInternal::Pixmap)
{                                                // for bitmaps only
    init(0, 0, 0, false, NormalOptim);
    if (w <= 0 || h <= 0)                        // create null pixmap
        return;

    data->uninit = false;
    data->w = w;
    data->h = h;
    data->d = 1;

    int bitsbpl = (w+7)/8;                        // original # bytes per line

    // CreateBitmap data is word aligned, while
    // CreateDIBSection is doubleword aligned
#ifndef Q_OS_TEMP
    int bpl        = ((w+15)/16)*2;                // bytes per scanline
#else
    int bpl        = ((w+31)/32)*4;                // bytes per scanline
#endif
    uchar *newbits = new uchar[bpl*h];
    uchar *p        = newbits;
    int x, y, pad;
    pad = bpl - bitsbpl;

    if (isXbitmap) {                                // flip and invert
        const uchar *f = qt_get_bitflip_array();
        for (y=0; y<h; y++) {
            for (x=0; x<bitsbpl; x++)
                *p++ = ~f[*bits++];
            for (x=0; x<pad; x++)
                *p++ = 0;
        }
    } else {                                        // invert all bits
        for (y=0; y<h; y++) {
            for (x=0; x<bitsbpl; x++)
                *p++ = ~(*bits++);
            for (x=0; x<pad; x++)
                *p++ = 0;
        }
    }

#ifndef Q_OS_TEMP
    data->hbm = CreateBitmap(w, h, 1, 1, newbits);
#else
    // WinCE must use DIBSections instead of Compatible Bitmaps
    // so it's possible to get the colortable at a later point.
    int   ncols           = 2;
    int   bmi_data_len    = sizeof(BITMAPINFO) + sizeof(RGBQUAD)*ncols;
    char *bmi_data        = new char[bmi_data_len];
    memset(bmi_data, 0, bmi_data_len);
    BITMAPINFO       *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi_data;
    bmh->biSize           = sizeof(BITMAPINFOHEADER);
    bmh->biWidth          = w;
    bmh->biHeight         = -h; // top-down bitmap
    bmh->biPlanes         = 1;
    bmh->biBitCount       = 1;
    bmh->biCompression    = BI_RGB;
    bmh->biSizeImage      = 0;
    bmh->biClrUsed        = ncols;
    bmh->biClrImportant   = 0;
    QRgb *coltbl = (QRgb*)(bmi_data + sizeof(BITMAPINFOHEADER));
    coltbl[1] = 0xffffff;
    coltbl[0] = 0x0;

    DATA_HBM = CreateDIBSection(qt_win_display_dc(),
                                 bmi,
                                 DIB_RGB_COLORS,
                                 (void**)&(data->ppvBits),
                                 NULL,
                                 0);
    memcpy(data->ppvBits, newbits, bpl*h);
#endif

    delete [] newbits;
    if (defOptim != NormalOptim)
        setOptimization(defOptim);
}


void QPixmap::detach()
{
    if (data->count != 1)
        *this = copy();
    data->uninit = FALSE;
    if (data->maskpm) {
        delete data->maskpm;
        data->maskpm = 0;
    }
    data->ser_no = ++qt_pixmap_serial;
}


int QPixmap::defaultDepth()
{
    static int dd = 0;
    if (dd == 0)
        dd = GetDeviceCaps(qt_win_display_dc(), BITSPIXEL);
    return dd;
}


void QPixmap::setOptimization(Optimization optimization)
{
    if (optimization == data->optim)
        return;
    detach();
    data->optim = optimization == DefaultOptim ?
            defOptim : optimization;
    if (data->optim == MemoryOptim) {
        if (data->maskpm) {
            delete data->maskpm;
            data->maskpm = 0;
        }
        if (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)
            data->allocCell(this);
    } else {
        if (data->mcp)
            QPixmapData::freeCell(data);
    }
}


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

    HDC dc = getDC();
    if (fillColor == Qt::black) {
        PatBlt(dc, 0, 0, data->w, data->h, BLACKNESS);
    } else if (fillColor == Qt::white) {
        PatBlt(dc, 0, 0, data->w, data->h, WHITENESS);
    } else {
        HBRUSH hbrush = CreateSolidBrush(RGB(fillColor.red(), fillColor.green(), fillColor.blue()));
        HBRUSH hb_old = (HBRUSH)SelectObject(dc, hbrush);
        PatBlt(dc, 0, 0, data->w, data->h, PATCOPY);
        DeleteObject(SelectObject(dc, hb_old));
    }
    releaseDC(dc);
}

QPixmap QPixmap::alphaChannel() const
{
    if (!hasAlphaChannel())
        return QPixmap();
    // ################### PIXMAP
}

void setAlphaChannel(const QPixmap &alpha)
{
    // ############ PIXMAP
}

QBitmap QPixmap::mask() const
{
    return data->mask ? *data->mask : QBitmap();
}

void QPixmap::setMask(const QBitmap &newmask)
{
    // ##################### PIXMAP
#if 0
    const QPixmap *tmp = &newmask;                // dec cxx bug
    if (data == tmp->data) {
        QPixmap m = tmp->copy();
        setMask(*((QBitmap*)&m));
        data->selfmask = true;                        // mask == pixmap
        return;
    }

    if (newmask.isNull()) {                        // reset the mask
        if (data->mask) {
            detach();
            data->selfmask = false;

            delete data->mask;
            data->mask = 0;
        }
        return;
    }

    detach();
    data->selfmask = false;

    if (newmask.width() != width() || newmask.height() != height()) {
        qWarning("QPixmap::setMask: The pixmap and the mask must have the same size");
        return;
    }
#if defined(Q_WS_MAC)
    // when setting the mask, we get rid of the alpha channel completely
    data->macQDDisposeAlpha();
#endif

    delete data->mask;
    QBitmap* newmaskcopy;
    if (newmask.mask())
        newmaskcopy = (QBitmap*)new QPixmap(tmp->copy());
    else
        newmaskcopy = new QBitmap(newmask);
#ifdef Q_WS_X11
    newmaskcopy->x11SetScreen(data->xinfo.screen());
#endif
    data->mask = newmaskcopy;
#endif
}

int QPixmap::metric(PaintDeviceMetric m) const
{
    if (m == PdmWidth) {
        return width();
    } else if (m == PdmHeight) {
        return height();
    } else {
        int val;
        HDC dc = GetDC(0);
        QPixmap *spm;
        if (data->mcp) {
            spm = DATA_MCPI_MCP->sharedPixmap();
        } else {
            spm = 0;
        }
        switch (m) {
            case PdmDpiX:
                val = GetDeviceCaps(dc, LOGPIXELSX);
                break;
            case PdmDpiY:
                val = GetDeviceCaps(dc, LOGPIXELSY);
                break;
            case PdmWidthMM:
                val = width()
                        * GetDeviceCaps(dc, HORZSIZE)
                        / GetDeviceCaps(dc, HORZRES);
                if (spm)
                    val = val * width() / spm->width();
                break;
            case PdmHeightMM:
                val = height()
                        * GetDeviceCaps(dc, VERTSIZE)
                        / GetDeviceCaps(dc, VERTRES);
                if (spm)
                    val = val * height() / spm->height();
                break;
            case PdmNumColors:
                if (GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE)
                    val = GetDeviceCaps(dc, SIZEPALETTE);
                else {
                    int bpp = GetDeviceCaps(dc, BITSPIXEL);
                    if(bpp==32)
                        val = INT_MAX;
                    else if(bpp<=8)
                        val = GetDeviceCaps(dc, NUMCOLORS);
                    else
                        val = 1 << (bpp * GetDeviceCaps(dc, PLANES));
                }
                break;
            case PdmDepth:
                val = depth();
                break;
            default:
                val = 0;
                qWarning("QPixmap::metric: Invalid metric command");
        }
        ReleaseDC(0, dc);
        return val;
    }
}


QImage QPixmap::toImage() const
{
    if (isNull())
        return QImage(); // null image

    int        w = width();
    int        h = height();
    const QBitmap *m = data->realAlphaBits ? 0 : mask();
#ifndef Q_OS_TEMP
    int        d = depth();
    int        ncols = 2;
#else
    DIBSECTION      ds;
    DWORD dwSize = GetObject(DATA_HBM, sizeof(DIBSECTION), &ds);
    Q_ASSERT(dwSize == sizeof(ds)); // Failing means HBitmap
    int d = ds.dsBmih.biBitCount;
    int ncols = ds.dsBmih.biClrUsed;
#endif

    if (d > 1 && d <= 8 || d == 1 && m) {        // set to nearest valid depth
        d = 8;                                        //   2..7 ==> 8
        ncols = 256;
    } else if (d > 8) {
        d = 32;                                        //   > 8  ==> 32
        ncols = 0;
    }

    QImage image(w, h, d, ncols, QImage::BigEndian);
    if (data->realAlphaBits) {
#ifndef Q_OS_TEMP
        GdiFlush();
#endif
        memcpy(image.bits(), data->realAlphaBits, image.numBytes());
        image.setAlphaBuffer(true);

        // Windows has premultiplied alpha, so revert it
        uchar *p = image.bits();
        uchar *end = p + image.numBytes();
        uchar alphaByte;
        while (p < end) {
            alphaByte = *(p+3);
            if (alphaByte == 0) {
                *p = 255;
                ++p;
                *p = 255;
                ++p;
                *p = 255;
                ++p;
                ++p;
            } else if (alphaByte == 255) {
                p += 4;
            } else {
                uchar alphaByte2 = alphaByte / 2;
                *p = ((int)(*p) * 255 + alphaByte2) / alphaByte;
                ++p;
                *p = ((int)(*p) * 255 + alphaByte2) / alphaByte;
                ++p;
                *p = ((int)(*p) * 255 + alphaByte2) / alphaByte;
                ++p;
                ++p;
            }
        }
        return image;
    }

    int          bmi_data_len = sizeof(BITMAPINFO)+sizeof(RGBQUAD)*ncols;
    char *bmi_data = new char[bmi_data_len];
    memset(bmi_data, 0, bmi_data_len);
    BITMAPINFO             *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize                  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth          = w;
    bmh->biHeight          = -h;                        // top-down bitmap
    bmh->biPlanes          = 1;
    bmh->biBitCount          = d;
    bmh->biCompression          = BI_RGB;
    bmh->biSizeImage          = image.numBytes();
    bmh->biClrUsed          = ncols;
    bmh->biClrImportant          = 0;
    QRgb *coltbl = (QRgb*)(bmi_data + sizeof(BITMAPINFOHEADER));
#ifndef Q_OS_TEMP
    bool mcp = data->mcp;
    if (mcp)                                        // disable multi cell
        QPixmapData::freeCell(data);

    GetDIBits(qt_win_display_dc(), data->bm(), 0, h, image.bits(), bmi, DIB_RGB_COLORS);

    // Opaque images need to have alpha channel set to 0xff. Windows ignores
    // this, but we need it for platform consistancy. (OpenGL conversion
    // does not work without it).
    if (d == 32 && !image.hasAlphaBuffer()) {
        int i = 0;
        while (i < image.height()) {
            uint *p = (uint *) image.scanLine(i);
            uint *end = p + image.width();
            while (p < end) {
                *p |= 0xff000000;
                ++p;
            }
            ++i;
        }
    }

    if (mcp)
        data->allocCell(this);
#else
    memcpy(image.bits(), data->ppvBits, image.numBytes());
    qt_GetDIBColorTable(data->hd, &ds, 0, ncols, (RGBQUAD*)coltbl);
#endif

    for (int i=0; i<ncols; i++) {                // copy color table
        RGBQUAD *r = (RGBQUAD*)&coltbl[i];
        image.setColor(i, qRgb(r->rgbRed, r->rgbGreen, r->rgbBlue));
    }

    if (m) {
        image.setAlphaBuffer(true);
        QImage msk = m->toImage();

        switch (d) {
          case 8: {
            int used[256];
            memset(used, 0, sizeof(int)*256);
            uchar* p = image.bits();
            int l = image.numBytes();
            while (l--) {
                used[*p++]++;
            }
            int trans=0;
            int bestn=INT_MAX;
            for (int i=0; i<256; i++) {
                if (used[i] < bestn) {
                    bestn = used[i];
                    trans = i;
                    if (!bestn)
                        break;
                }
            }
            image.setColor(trans, image.color(trans)&0x00ffffff);
            for (int y=0; y<image.height(); y++) {
                uchar* mb = msk.scanLine(y);
                uchar* ib = image.scanLine(y);
                uchar bit = 0x80;
                int i=image.width();
                while (i--) {
                    if (!(*mb & bit))
                        *ib = trans;
                    bit /= 2; if (!bit) mb++,bit = 0x80; // ROL
                    ib++;
                }
            }
          } break;
          case 32: {
            for (int y=0; y<image.height(); y++) {
                uchar* mb = msk.scanLine(y);
                QRgb* ib = (QRgb*)image.scanLine(y);
                uchar bit = 0x80;
                int i=image.width();
                while (i--) {
                    if (*mb & bit)
                        *ib |= 0xff000000;
                    else
                        *ib &= 0x00ffffff;
                    bit /= 2; if (!bit) mb++,bit = 0x80; // ROL
                    ib++;
                }
            }
          } break;
        }
    }
    if (d == 1) {
        // Make image bit 0 come from Qt::color0, image bit 1 come form Qt::color1
        image.invertPixels();
        QRgb c0 = image.color(0);
        image.setColor(0,image.color(1));
        image.setColor(1,c0);
    }
    delete [] bmi_data;
    return image;
}


QPixmap QPixmap::fromImage(const QImage &img, Qt::ImageConversionFlags flags)
{
    QPixmap pixmap;
    if (img.isNull()) {
        qWarning("QPixmap::convertFromImage: Cannot convert a null image");
        return pixmap;
    }
    QImage image = img;
    int           d     = image.depth();
    int    dd    = defaultDepth();
    bool force_mono = (dd == 1 || isQBitmap() ||
                       (flags & Qt::ColorMode_Mask)==Qt::MonoOnly);

    if (force_mono) {                                // must be monochrome
        if (d != 1) {                                // dither
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
        image = image.convertBitOrder(QImage::BigEndian);
#ifdef Q_OS_TEMP
        if (image.color(0) == qRgb(255,255,255) && image.color(1) == qRgb(0,0,0)) {
            image.invertPixels();
            QRgb c0 = image.color(0);
            image.setColor(0,image.color(1));
            image.setColor(1,c0);
        }
#endif
    }

#ifndef Q_OS_TEMP
    bool hasRealAlpha = false;
    if (img.hasAlphaBuffer() &&
        (QSysInfo::WindowsVersion != QSysInfo::WV_95 &&
         QSysInfo::WindowsVersion != QSysInfo::WV_NT)) {
        if (image.depth() == 8) {
            const QRgb * const rgb = img.colorTable();
            for (int i = 0, count = img.numColors(); i < count; ++i) {
                const int alpha = qAlpha(rgb[i]);
                if (alpha != 0 && alpha != 0xff) {
                    hasRealAlpha = true;
                    break;
                }
            }
            if (hasRealAlpha) {
                image = image.convertDepth(32, flags);
                d = image.depth();
            }
        } else {
            hasRealAlpha = true;
        }
    }
#endif

    int w = image.width();
    int h = image.height();

    if (width() == w && height() == h && ((d == 1 && depth() == 1) ||
                                          (d != 1 && depth() != 1))) {
        if (data->realAlphaBits && !hasRealAlpha) {
            // pixmap uses a DIB section, but image has no alpha channel, so we
            // can't reuse the old pixmap
            QPixmap pm(w, h, d == 1 ? 1 : -1, data->bitmap, data->optim);
            *this = pm;
        } else {
            // same size etc., use the existing pixmap
            detach();
            if (data->mask) {                        // get rid of the mask
                delete data->mask;
                data->mask = 0;
            }
        }
    } else {
        // different size or depth, make a new pixmap
        QPixmap pm(w, h, d == 1 ? 1 : -1, data->bitmap, data->optim);
        *this = pm;
    }

    int          ncols           = image.numColors();
#ifndef Q_OS_TEMP
    char *bmi_data = new char[sizeof(BITMAPINFO)+sizeof(QRgb)*ncols];
    BITMAPINFO             *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize                  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth          = w;
    bmh->biHeight          = -h;
    bmh->biPlanes          = 1;
    bmh->biBitCount          = d;
    bmh->biCompression          = BI_RGB;
    bmh->biSizeImage          = image.numBytes();
    bmh->biXPelsPerMeter  = 0;
    bmh->biYPelsPerMeter  = 0;
    bmh->biClrUsed          = ncols;
    bmh->biClrImportant          = ncols;
    QRgb *coltbl = (QRgb*)(bmi_data + sizeof(BITMAPINFOHEADER));
#else
    int   bmi_data_len    = sizeof(BITMAPINFO) + sizeof(RGBQUAD) * (d > 8 ? 3 : ncols);
    char *bmi_data        = new char[bmi_data_len];
    memset(bmi_data, 0, bmi_data_len);
    BITMAPINFO             *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize                  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth          = w;
    bmh->biHeight          = -h;
    bmh->biPlanes          = 1;
    bmh->biBitCount          = d;
    bmh->biCompression    = (d > 8 ? BI_BITFIELDS : BI_RGB);
    bmh->biSizeImage          = image.numBytes();
    bmh->biXPelsPerMeter  = 0;
    bmh->biYPelsPerMeter  = 0;
    bmh->biClrUsed          = ncols;
    bmh->biClrImportant          = ncols;
    QRgb *coltbl = (QRgb*)(bmi_data + sizeof(BITMAPINFOHEADER));
    if (ncols == 0 && d > 8) {
        coltbl[0] = 0xff0000; // R
        coltbl[1] = 0x00ff00; // G
        coltbl[2] = 0x0000ff; // B
        if (d != 32)
            qWarning("QPixmap::convertFromImage(): Unsupported bitmap depth (%d)", d);
    }
#endif
    for (int i=0; i<ncols; i++) {                // copy color table
        RGBQUAD *r = (RGBQUAD*)&coltbl[i];
        QRgb         c = image.color(i);
        r->rgbBlue  = qBlue (c);
        r->rgbGreen = qGreen(c);
        r->rgbRed   = qRed  (c);
        r->rgbReserved = 0;
    }

#ifndef Q_OS_TEMP
    if (hasRealAlpha) {
        initAlphaPixmap(image.bits(), image.numBytes(), bmi);

        // Windows expects premultiplied alpha
        uchar *p = image.bits();
        uchar *b = data->realAlphaBits;
        uchar *end = p + image.numBytes();
        uchar alphaByte;
        while (p < end) {
            alphaByte = *(p+3);
            if (alphaByte == 0) {
                *(b++) = 0;
                *(b++) = 0;
                *(b++) = 0;
                b++;
                p += 4;
            } else if (alphaByte == 255) {
                b += 4;
                p += 4;
            } else {
                *(b++) = ((*(p++)) * (int)alphaByte + 127) / 255;
                *(b++) = ((*(p++)) * (int)alphaByte + 127) / 255;
                *(b++) = ((*(p++)) * (int)alphaByte + 127) / 255;
                b++;
                p++;
            }
        }
    }
#else
    data->realAlphaBits = 0;
#endif

    if (data->realAlphaBits == 0) {
#ifndef Q_OS_TEMP
        HDC dc = getDC();
        StretchDIBits(dc, 0, 0, w, h, 0, 0, w, h, image.bits(), bmi, DIB_RGB_COLORS, SRCCOPY);
        releaseDC(dc);
#else
        DeleteObject(DATA_HBM);
        HDC hdcSrc = handle();
        HBITMAP hBitmap = CreateDIBSection(hdcSrc, bmi, DIB_RGB_COLORS, (void**)&(data->ppvBits), NULL, 0);
        memcpy(data->ppvBits, image.bits(), image.numBytes());
        // Cannot use the return value of SelectObject, as it's a
        // true HBITMAP, and not a DIBSelection with a HBITMAP handle
        SelectObject(hdcSrc, hBitmap);
        DATA_HBM = hBitmap;
#endif
    }

    if (img.hasAlphaBuffer()) {
        QBitmap m;
        m = img.createAlphaMask(flags);
        setMask(m);
    }

    delete [] bmi_data;
    data->uninit = false;

    return true;
}


QPixmap QPixmap::grabWindow(WId window, int x, int y, int w, int h)
{
    if (w <= 0 || h <= 0) {
        if (w == 0 || h == 0) {
            QPixmap nullPixmap;
            return nullPixmap;
        }
        RECT r;
        GetClientRect(window, &r);
        if (w < 0)
            w = (r.right - r.left);
        if (h < 0)
            h = (r.bottom - r.top);
    }
    QPixmap pm(w, h);
    HDC dc = pm.getDC();

    HDC src_dc = GetDC(window);
    BitBlt(dc, 0, 0, w, h, src_dc, x, y, SRCCOPY);
    ReleaseDC(window, src_dc);

    pm.releaseDC(dc);
    return pm;
}


QPixmap QPixmap::transformed(const QMatrix &matrix, Qt::TransformationMode mode) const
{
    if (mode == Qt::SmoothTransformation) {
        // ###### do this efficiently!
        QImage image = toImage();
        return QPixmap(image.transform(matrix, mode));
    }

    int           w, h;                                // size of target pixmap
    int           ws, hs;                                // size of source pixmap
    uchar *dptr;                                // data in target pixmap
    int           dbpl, dbytes;                        // bytes per line/bytes total
    uchar *sptr;                                // data in original pixmap
    int           sbpl;                                // bytes per line in original
    int           bpp;                                        // bits per pixel
    bool   depth1 = depth() == 1;

    if (isNull())                                // this is a null pixmap
        return copy();

    ws = width();
    hs = height();

    QMatrix mat(matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(), 0., 0.);

    if (matrix.m12() == 0.0F  && matrix.m21() == 0.0F && matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F) {
        if (mat.m11() == 1.0F && mat.m22() == 1.0F)
            return *this;                        // identity matrix

        h = qRound(mat.m22()*hs);
        w = qRound(mat.m11()*ws);
        h = qAbs(h);
        w = qAbs(w);
        if (data->realAlphaBits == 0) {
            // we have to create the new pixmap before we query the handle of this,
            // as the handle might change if this is a multicell pixmap that gets
            // expanded by the constructor in the line below.
            QPixmap pm(w, h, depth(), optimization());
            HDC dc = getDC();
            HDC pm_dc = pm.getDC();

#ifndef Q_OS_TEMP
            SetStretchBltMode(pm_dc, COLORONCOLOR);
#endif
            StretchBlt(pm_dc, 0, 0, w, h, dc, 0, 0, ws, hs, SRCCOPY);
            if (data->mask) {
                QBitmap bm = data->selfmask ? *((QBitmap*)(&pm)) : data->mask->transform(matrix);
                pm.setMask(bm);
            }

            releaseDC(dc);
            pm.releaseDC(pm_dc);
            return pm;
        }
    } else {
        // rotation or shearing
        QPolygon a(QRect(0,0,ws+1,hs+1));
        a = mat.map(a);
        QRect r = a.boundingRect().normalize();
        w = r.width()-1;
        h = r.height()-1;
    }

    mat = trueMatrix(mat, ws, hs); // true matrix

    bool invertible;
    mat = mat.inverted(&invertible);                // invert matrix


    if (h == 0 || w == 0 || !invertible) {        // error, return null pixmap
        QPixmap pm;
        pm.data->bitmap = data->bitmap;
        return pm;
    }

    if (data->realAlphaBits) {
        bpp = 32;
    } else {
        bpp  = depth();
        if (bpp > 1 && bpp < 8)
            bpp = 8;
    }

    sbpl = ((ws*bpp+31)/32)*4;
    sptr = new uchar[hs*sbpl];
    int ncols;
    if (bpp <= 8) {
        ncols = 1 << bpp;
    } else {
        ncols = 0;
    }

    int          bmi_data_len = sizeof(BITMAPINFO)+sizeof(RGBQUAD)*ncols;
    char *bmi_data = new char[bmi_data_len];
    memset(bmi_data, 0, bmi_data_len);
    BITMAPINFO             *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize                  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth          = ws;
    bmh->biHeight          = -hs;                // top-down bitmap
    bmh->biPlanes          = 1;
    bmh->biBitCount          = bpp;
    bmh->biCompression          = BI_RGB;
    bmh->biSizeImage          = sbpl*hs;
    bmh->biClrUsed          = ncols;
    bmh->biClrImportant          = 0;

    bool mcp = data->mcp;
    if (mcp)
        QPixmapData::freeCell(data);
    int result;
#ifndef Q_OS_TEMP
    if (data->realAlphaBits) {
        GdiFlush();
        memcpy(sptr, data->realAlphaBits, sbpl*hs);
        result = 1;
    } else {
        result = GetDIBits(qt_win_display_dc(), data->bm(), 0, hs, sptr, bmi, DIB_RGB_COLORS);
    }
#else
    if (data->realAlphaBits) {
        memcpy(sptr, data->realAlphaBits, sbpl*hs);
        result = 1;
    } else {
        memcpy(sptr, data->ppvBits, sbpl*hs);
        result = 1;
    }
#endif

    if (mcp)
        data->allocCell(this);

    if (!result) {                                // error, return null pixmap
        return QPixmap(0, 0, 0, data->bitmap, NormalOptim);
    }

    dbpl   = ((w*bpp+31)/32)*4;
    dbytes = dbpl*h;

    dptr = new uchar[dbytes];                        // create buffer for bits
    if (depth1)
        memset(dptr, 0xff, dbytes);
    else if (bpp == 8)
        memset(dptr, QColormap::instance().pixel(QColor(Qt::white)), dbytes);
    else if (data->realAlphaBits)
        memset(dptr, 0x00, dbytes);
    else
        memset(dptr, 0xff, dbytes);

    int          xbpl, p_inc;
    if (depth1) {
        xbpl  = (w+7)/8;
        p_inc = dbpl - xbpl;
    } else {
        xbpl  = (w*bpp)/8;
        p_inc = dbpl - xbpl;
    }

    if (!qt_xForm_helper(mat, 0, QT_XFORM_TYPE_WINDOWSPIXMAP, bpp, dptr, xbpl, p_inc, h, sptr, sbpl, ws, hs)){
        qWarning("QPixmap::transform: display not supported (bpp=%d)",bpp);
        QPixmap pm;
        delete [] sptr;
        delete [] bmi_data;
        delete [] dptr;
        return pm;
    }

    delete [] sptr;

    QPixmap pm(w, h, depth(), data->bitmap, optimization());
    HDC pm_dc = pm.getDC();
    pm.data->uninit = false;
    bmh->biWidth  = w;
    bmh->biHeight = -h;
    bmh->biSizeImage = dbytes;
    if (data->realAlphaBits) {
        pm.initAlphaPixmap(dptr, dbytes, bmi);
    } else {
#ifndef Q_OS_TEMP
        SetDIBitsToDevice(pm_dc, 0, 0, w, h, 0, 0, 0, h, dptr, bmi, DIB_RGB_COLORS);
#else
        DeleteObject(pm.DATA_HBM);
        HDC hdcSrc = pm.handle();
        HBITMAP hBitmap = CreateDIBSection(hdcSrc, bmi, DIB_RGB_COLORS, (void**)&(data->ppvBits), NULL, 0);
        memcpy(data->ppvBits, dptr, dbytes);
        // Cannot use the return value of SelectObject, as it's a
        // true HBITMAP, and not a DIBSelection with a HBITMAP handle
        SelectObject(hdcSrc, hBitmap);
        pm.DATA_HBM = hBitmap;
#endif
    }

    delete [] bmi_data;
    delete [] dptr;
    if (data->mask) {
        QBitmap bm = data->selfmask ? *((QBitmap*)(&pm)) : data->mask->transform(matrix);
        pm.setMask(bm);
    }

    pm.releaseDC(pm_dc);
    return pm;
}

/*!
  \fn HBITMAP QPixmap::hbm() const
  \internal
*/
HBITMAP QPixmap::hbm() const
{
    return data->mcp ? 0 : data->hbm;
}


/*
  Implementation of internal QMultiCellPixmap class.
*/

QMultiCellPixmap::QMultiCellPixmap(int width, int depth, int maxHeight)
{
    // Start with a small pixmap first and expand as needed
    int height = width * 4;
    // Set def optim to normal to avoid recursion
    pixmap = new QPixmap(width, height, depth, QPixmap::NormalOptim);
    pixmap->detach();                                // clears uninit flag
    max_height = maxHeight;
    // The whole pixmap area can be allocated
    free_list.append(FreeNode(0,max_height));
}

QMultiCellPixmap::~QMultiCellPixmap()
{
    delete pixmap;
}

int QMultiCellPixmap::allocCell(int height)
{
    FreeNode n = free_list.first();
    for(int i = 0; (i < free_list.size()) && (n.size < height); ++i)
        n = free_list.at(i);                        // find free space
    if ((n == free_list.last())
         && (n.size < height))                        // not enough space
        return -1;
    int offset = n.offset;
    if (n.size > height) {                        // alloc part of free space
        n.offset += height;
        n.size -= height;
    } else {                                        // perfect fit, height == size
        Q_ASSERT(n.size == height);
        free_list.removeAll(n);                        // remove the node
    }
    int pm_height = pixmap->height();
    while (offset + height > pm_height)
        pm_height *= 2;
    if (pm_height > pixmap->height())                // expand pixmap
        pixmap->resize(pixmap->width(), pm_height);
    return offset;
}

void QMultiCellPixmap::freeCell(int offset, int size)
{
    FreeNode n = free_list.first();
    int i = 0;
    for(; (i < free_list.size()) && (free_list.at(i).offset < offset); ++i)
        ;
    if (i > 0 && free_list.at(i-1).offset + free_list.at(i-1).size == offset) {
        // The previous free node is adjacent to the cell we are freeing up,
        // then expand the size of the prev node to include this space.
        FreeNode &p = free_list[i-1];
        p.size += size;
        if (i < free_list.size() && p.offset + p.size == free_list.at(i).offset) {
            // If the next node comes after the prev node, collapse them.
            p.size += n.size;
            free_list.removeAt(i);        // removes the current node
        }
    } else if (i < free_list.size()) {
        FreeNode &n = free_list[i];
        // We have found the first free node after the cell.
        if (offset + size == n.offset) {
            // The next free node comes right after the freed up area, then
            // include this area.
            n.offset -= size;
            n.size   += size;
        } else {
            // Insert a new free node before this one.
            free_list.insert(i, FreeNode(offset,size));
        }
    } else {
        // n == 0, this means the free_list is empty or that the cell is
        // after the last free block (but still not adjacent to p),
        // then append a new free node.
        free_list.append(FreeNode(offset,size));
    }
}


/*
  We have internal lists of QMultiCellPixmaps; 4 for color pixmaps and
  4 for mono pixmaps/bitmaps.  There is one list for pixmaps with a
  width 1..16, 17..32, 33..64 and 65..128.
*/

typedef QList<QMultiCellPixmap*>  QMultiCellPixmapList;

static const int mcp_num_lists  = 8;
static bool         mcp_lists_init = false;
static QMultiCellPixmapList *mcp_lists[mcp_num_lists];

static void cleanup_mcp()
{
    if (mcp_lists_init) {
        mcp_system_unstable = true;                // tell QPixmap::deref()
        mcp_lists_init = false;
        for (int i=0; i<mcp_num_lists; i++) {
            qDeleteAll(*mcp_lists[i]);
            delete mcp_lists[i];
            mcp_lists[i] = 0;
        }
    }
}

static void init_mcp()
{
    if (!mcp_lists_init) {
        mcp_lists_init = true;
        for (int i=0; i<mcp_num_lists; i++)
            mcp_lists[i] = 0;
        qAddPostRoutine(cleanup_mcp);
    }
}

static int index_of_mcp_list(int width, bool mono, int *size=0)
{
    if (width <= 0)
        return -1;
    int i;
    int s = 16;
    for (i=0; i<4; i++) {                        // try s=16,32,64,128
        if (width <= s)                        // index= 0, 1, 2,  3
            break;
        s *= 2;
    }
    if (i == 4)                                // too big pixmap
        return -1;
    if (mono)                                        // mono: index 4, 5, ...
        i += 4;
    if (size)
        *size = s;
    return i;
}


/*!
  \internal
*/
int QPixmapData::allocCell(const QPixmap *p)
{
    if (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)                // only for NT based systems
        return -1;
    if (!mcp_lists_init)
        init_mcp();
    QPixmapData *data = p->data;
    if (data->mcp)                                // cell already alloc'd
        freeCell(data);
    int s;
    int i = index_of_mcp_list(data->w, (data->d == 1), &s);
    if (i < 0)                                // too large width
        return -1;
    if (!mcp_lists[i])
        mcp_lists[i] = new QMultiCellPixmapList;

    QMultiCellPixmapList *list = mcp_lists[i];
    QMultiCellPixmap     *mcp  = 0;
    int offset = -1;
    for(int i = 0; i < list->size(); ++i) {
        mcp = list->at(i);
        offset = mcp->allocCell(data->h);
        if (offset >= 0)
            break;
    }
    if (offset < 0) {                                // could not alloc
        mcp = new QMultiCellPixmap(s, data->d, 2048);
        offset = mcp->allocCell(data->h);
        if (offset < 0) {                        // height() > total height
            delete mcp;
            return offset;
        }
        list->append(mcp);
    }

    HDC dc = p->getDC();

    HDC mcp_dc = mcp->mem_dc.hdc;
    HGDIOBJ old_mcp_bm = INVALID_HANDLE_VALUE;
    if (!mcp->mem_dc.hdc) {
        mcp_dc = mcp->mem_dc.hdc ? mcp->mem_dc.hdc : CreateCompatibleDC(0);
        old_mcp_bm = SelectObject(mcp_dc, mcp->hbm());
    } else {
        SetViewportOrgEx(mcp_dc, 0, 0, NULL);
    }
    // copy into multi cell pixmap
    BitBlt(mcp_dc, 0, offset, data->w, data->h, dc, 0, 0, SRCCOPY);

    p->releaseDC(dc);
    if(!mcp->mem_dc.hdc) {
        SelectObject(mcp_dc, old_mcp_bm);
        DeleteDC(mcp_dc);
    }

    DeleteObject(data->hbm);
    data->hbm = 0;

    data->mcp = true;
    DATA_MCPI = new QMCPI;
    DATA_MCPI_MCP = mcp;
    DATA_MCPI_OFFSET = offset;
    return offset;
}


/*!
  \internal
*/
void QPixmapData::freeCell(QPixmapData *data, bool terminate)
{
    if (!mcp_lists_init || !data->mcp)
        return;
    QMultiCellPixmap *mcp = DATA_MCPI_MCP;
    int offset = DATA_MCPI_OFFSET;
    data->mcp = false;
    delete DATA_MCPI;
    DATA_MCPI = 0;
    if (terminate) {                                // pixmap is being destroyed
        data->hbm = 0;
    } else {
        if (data->d == QPixmap::defaultDepth())
            data->hbm = CreateCompatibleBitmap(qt_win_display_dc(), data->w, data->h);
        else
            data->hbm = CreateBitmap(data->w, data->h, 1, 1, 0);
        HDC hdc = data->mem_dc.hdc;
        HDC mcp_dc = mcp->mem_dc.hdc;
        HGDIOBJ old_mcp_bm = INVALID_HANDLE_VALUE;
        if (!mcp->mem_dc.hdc) {
            mcp_dc = mcp->mem_dc.hdc ? mcp->mem_dc.hdc : CreateCompatibleDC(0);
            old_mcp_bm = SelectObject(mcp_dc, mcp->hbm());
        } else {
            SetViewportOrgEx(mcp_dc, 0, 0, NULL);
        }
        BitBlt(hdc, 0, 0, data->w, data->h, mcp_dc, 0, offset, SRCCOPY);
        data->releaseDC(hdc);
        if(!mcp->mem_dc.hdc) {
            SelectObject(mcp_dc, old_mcp_bm);
            DeleteDC(mcp_dc);
        }
    }
    mcp->freeCell(offset, data->h);
    if (mcp->isEmpty()) {                        // no more cells left
        int i = index_of_mcp_list(data->w, (data->d == 1), 0);
        Q_ASSERT(i >= 0 && mcp_lists[i]);
        if (mcp_lists[i]->count() > 1) {        // don't remove the last one
            mcp_lists[i]->removeAll(mcp);
            if (mcp_lists[i]->isEmpty()) {
                delete mcp_lists[i];
                mcp_lists[i] = 0;
            }
        }
    }
}

void QPixmapData::releaseDC(HDC hdc) const
{
    QPixmapData::MemDC *memdc = const_cast<QPixmapData::MemDC*>(mcp ? &mcpi->mcp->mem_dc : &mem_dc);

    if(hdc != memdc->hdc) {
        qWarning("QPixmap::releaseDC(): releasing wrong DC");
        return;
    }
    --memdc->ref;
    if(memdc->ref == 0) {
        SelectObject(memdc->hdc, memdc->bm);
        DeleteDC(memdc->hdc);
        memdc->hdc = 0;
        memdc->bm = 0;
    }
}

void QMultiCellPixmap::debugger()
{
    qDebug("  Multi cell pixmap %d x %d x %d (%p)",
           pixmap->width(), max_height, pixmap->depth(), this);
    qDebug("    Actual pixmap height = %d", pixmap->height());
    qDebug("    Free List");
    for(int i = 0; i < free_list.size(); ++i) {
        FreeNode n = free_list.at(i);
        qDebug("      Offset %4d, Size %3d", n.offset, n.size);
    }
    qDebug("      Num free nodes = %d", free_list.count());
}


void qt_mcp_debugger()
{
    int i, s=16;
    const char *info = "pixmaps";
    bool nothing = true;
    qDebug("MCP DEBUGGER");
    qDebug("------------");
    for (i=0; i<mcp_num_lists; i++) {
        if (i == 5) {
            s = 16;
            info = "mono pixmaps";
        }
        if (mcp_lists[i]) {
            nothing = false;
            qDebug("Multi cell list %d, %s, size<=%d, number of lists = %d",
                   i, info, s, mcp_lists[i]->count());
            QMultiCellPixmap *mcp = mcp_lists[i]->first();
            for(int j = 0; j < mcp_lists[i]->size(); ++j) {
                mcp = mcp_lists[i]->at(j);
                mcp->debugger();
            }
        }
    }
    if (nothing)
        qDebug("No internal info");
    qDebug("MCP DONE\n");
}


bool QPixmap::hasAlpha() const
{
    return data->realAlphaBits || data->mask;
}

bool QPixmap::hasAlphaChannel() const
{
    return data->realAlphaBits != 0;
}

QPaintEngine *QPixmap::paintEngine() const
{
    if (!data->paintEngine) {
#ifdef QT_RASTER_PAINTENGINE
        data->paintEngine = new QRasterPaintEngine();
#else
        data->paintEngine = new QWin32PaintEngine();
#endif
    }
    return data->paintEngine;
}


/*!
    Returns the window system handle of the widget, for low-level
    access. Using this function is not portable.

    An HDC aquired with getDC() has to be released with releaseDC().
*/
HDC QPixmap::getDC() const
{
    QPixmapData::MemDC *mem_dc = data->mcp ? &data->mcpi->mcp->mem_dc : &data->mem_dc;

    if(!mem_dc->hdc) {
        mem_dc->hdc = CreateCompatibleDC(0);
        if (!mem_dc->hdc) {
            qErrnoWarning("QPixmap::getDC(): CreateCompatibleDC failed");
            return 0;
        }
        HPALETTE hpal = QColormap::hPal();
        if (hpal) {
            SelectPalette(mem_dc->hdc, hpal, false);
            RealizePalette(mem_dc->hdc);
        }
        mem_dc->bm = (HBITMAP)SelectObject(mem_dc->hdc, data->bm());
    }
    if (data->mcp)
        SetViewportOrgEx(mem_dc->hdc, 0, data->mcpi->offset, NULL);
    ++mem_dc->ref;
    return mem_dc->hdc;
}

/*!
    Releases the HDC aquired by a previous call to getDC().
    Using this function is not portable.
*/
void QPixmap::releaseDC(HDC hdc) const
{
    data->releaseDC(hdc);
}
