/****************************************************************************
**
** Implementation of QPixmap class for Win32.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qbitmap.h"
#include "qimage.h"
#include "qpaintdevicemetrics.h"
#include "qwmatrix.h"
#include "qapplication.h"
#include <private/qapplication_p.h>
#include "qt_windows.h"
#include <limits.h>

#include "qpaintengine_win.h"

extern const uchar *qt_get_bitflip_array();                // defined in qimage.cpp

#define DATA_HBM         data->hbm_or_mcpi.hbm
#define DATA_MCPI         data->hbm_or_mcpi.mcpi
#define DATA_MCPI_MCP         data->hbm_or_mcpi.mcpi->mcp
#define DATA_MCPI_OFFSET data->hbm_or_mcpi.mcpi->offset

static bool mcp_system_unstable = false;

/*!
  \class QPixmap::QMCPI
  \brief The QPixmap::QMCPI class is an internal class.
  \internal
*/

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
    HDC             handle()            const { return (HDC)pixmap->handle(); }
    HBITMAP  hbm()            const { return pixmap->hbm(); }
    int             allocCell(int height);
    void     freeCell(int offset, int height);
    void     debugger(); // for debugging during development
private:
    QPixmap          *pixmap;
    int                   max_height;
    FreeList free_list;
};


static inline HDC alloc_mem_dc(HBITMAP hbm, HBITMAP *old_hbm)
{
    HDC hdc = CreateCompatibleDC(qt_display_dc());
    if (!hdc) {
        qSystemWarning("alloc_mem_dc: CreateCompatibleDC failed");
        return hdc;
    }
    if (QColor::hPal()) {
        SelectPalette(hdc, QColor::hPal(), false);
        RealizePalette(hdc);
    }
    *old_hbm = (HBITMAP)SelectObject(hdc, hbm);
    return hdc;
}


void QPixmap::initAlphaPixmap(uchar *bytes, int length, BITMAPINFO *bmi)
{
    if (data->mcp)
        freeCell(true);
    if (!data->hd)
        data->hd = alloc_mem_dc(0, &data->old_hbm);

    HBITMAP hBitmap = CreateDIBSection((HDC)data->hd, bmi, DIB_RGB_COLORS, (void**)&data->realAlphaBits, NULL, 0);
    if (bytes)
        memcpy(data->realAlphaBits, bytes, length);

    DeleteObject(SelectObject((HDC)data->hd, data->old_hbm));
    data->old_hbm = (HBITMAP)SelectObject((HDC)data->hd, hBitmap);
    DATA_HBM = hBitmap;
}


void QPixmap::init(int w, int h, int d, bool bitmap, Optimization optim)
{
    if (qApp->type() == QApplication::Tty) {
        qWarning("QPixmap: Cannot create a QPixmap when no GUI "
                  "is being used");
    }

    static int serial = 0;
    int dd = defaultDepth();

    if (optim == DefaultOptim)                // use default optimization
        optim = defOptim;

    data = new QPixmapData;

    memset(data, 0, sizeof(QPixmapData));
    data->count  = 1;
    data->uninit = true;
    data->bitmap = bitmap;
    data->ser_no = ++serial;
    data->optim         = optim;

    bool make_null = w == 0 || h == 0;                // create null pixmap
    if (d == 1)                                // monocrome pixmap
        data->d = 1;
    else if (d < 0 || d == dd)                // compatible pixmap
        data->d = dd;
    if (make_null || w < 0 || h < 0 || data->d == 0) {
        data->hd = 0;
        DATA_HBM = 0;
        data->old_hbm = 0;
        if (!make_null)                        // invalid parameters
            qWarning("QPixmap: Invalid pixmap parameters");
        return;
    }
    data->w = w;
    data->h = h;
    if (data->optim == MemoryOptim && (QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based)) {
        data->hd = 0;
        if (allocCell() >= 0)                        // successful
            return;
    }

#ifndef Q_OS_TEMP
    if (data->d == dd)                        // compatible bitmap
        DATA_HBM = CreateCompatibleBitmap(qt_display_dc(), w, h);
    else                                        // monocrome bitmap
        DATA_HBM = CreateBitmap(w, h, 1, 1, 0);
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
        DATA_HBM = CreateDIBSection(qt_display_dc(),
                                     bmi,
                                     DIB_RGB_COLORS,
                                     (void**)&(data->ppvBits),
                                     NULL,
                                     0);
        delete [] bmi_data;
#endif
    if (!DATA_HBM) {
        data->w = 0;
        data->h = 0;
        data->hd = 0;
        qSystemWarning("QPixmap: Pixmap allocation failed");
        return;
    }
    data->hd = alloc_mem_dc(DATA_HBM, &data->old_hbm);
}


void QPixmap::deref()
{
    if (data && data->deref()) {                // last reference lost
        if (data->mcp) {
            if (mcp_system_unstable) {        // all mcp's gone
                data->mcp = false;
                delete DATA_MCPI;
                DATA_MCPI = 0;
                DATA_HBM  = 0;
            } else {
                freeCell(true);
            }
        }
        if (data->mask)
            delete data->mask;
        if (data->maskpm)
            delete data->maskpm;
        if (DATA_HBM) {
            DeleteObject((data->hd ? SelectObject((HDC)data->hd, data->old_hbm) : (HGDIOBJ)DATA_HBM));
            DATA_HBM = 0;
            data->old_hbm = 0;
        }
        if (data->hd) {
            DeleteDC((HDC)data->hd);
            data->hd = 0;
        }
        delete data->paintEngine;
        delete data;
    }
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
    DATA_HBM = CreateBitmap(w, h, 1, 1, newbits);
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

    DATA_HBM = CreateDIBSection(qt_display_dc(),
                                 bmi,
                                 DIB_RGB_COLORS,
                                 (void**)&(data->ppvBits),
                                 NULL,
                                 0);
    memcpy(data->ppvBits, newbits, bpl*h);
#endif

    data->hd = alloc_mem_dc(DATA_HBM, &data->old_hbm );
    delete [] newbits;
    if (defOptim != NormalOptim)
        setOptimization(defOptim);
}


void QPixmap::detach()
{
    if (data->uninit || data->count == 1)
        data->uninit = false;
    else
        *this = copy();
    if (data->maskpm) {
        delete data->maskpm;
        data->maskpm = 0;
    }
}


int QPixmap::defaultDepth()
{
    static int dd = 0;
    if (dd == 0)
        dd = GetDeviceCaps(qt_display_dc(), BITSPIXEL);
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
            allocCell();
    } else {
        if (data->mcp)
            freeCell();
    }
}


void QPixmap::fill(const QColor &fillColor)
{
    if (isNull())
        return;
    detach();                                        // detach other references
    HDC dc;
    int sy;
    if (data->mcp) {                                // multi-cell pixmap
        dc = DATA_MCPI_MCP->handle();
        sy = DATA_MCPI_OFFSET;
    } else {
        dc = (HDC)data->hd;
        sy = 0;
    }
    if (fillColor == Qt::black) {
        PatBlt(dc, 0, sy, data->w, data->h, BLACKNESS);
    } else if (fillColor == Qt::white) {
        PatBlt(dc, 0, sy, data->w, data->h, WHITENESS);
    } else {
        HBRUSH hbrush = CreateSolidBrush(fillColor.pixel());
        HBRUSH hb_old = (HBRUSH)SelectObject(dc, hbrush);
        PatBlt(dc, 0, sy, width(), height(), PATCOPY);
        DeleteObject(SelectObject(dc, hb_old));
    }
}


int QPixmap::metric(int m) const
{
    if (m == QPaintDeviceMetrics::PdmWidth) {
        return width();
    } else if (m == QPaintDeviceMetrics::PdmHeight) {
        return height();
    } else {
        int val;
        HDC dc;
        QPixmap *spm;
        if (data->mcp) {
            spm = DATA_MCPI_MCP->sharedPixmap();
            dc  = (HDC)spm->handle();
        } else {
            spm = 0;
            dc  = (HDC)handle();
        }
        switch (m) {
            case QPaintDeviceMetrics::PdmDpiX:
                val = GetDeviceCaps(dc, LOGPIXELSX);
                break;
            case QPaintDeviceMetrics::PdmDpiY:
                val = GetDeviceCaps(dc, LOGPIXELSY);
                break;
            case QPaintDeviceMetrics::PdmWidthMM:
                val = width()
                        * GetDeviceCaps(dc, HORZSIZE)
                        / GetDeviceCaps(dc, HORZRES);
                if (spm)
                    val = val * width() / spm->width();
                break;
            case QPaintDeviceMetrics::PdmHeightMM:
                val = height()
                        * GetDeviceCaps(dc, VERTSIZE)
                        / GetDeviceCaps(dc, VERTRES);
                if (spm)
                    val = val * height() / spm->height();
                break;
            case QPaintDeviceMetrics::PdmNumColors:
                if (GetDeviceCaps(dc, RASTERCAPS) & RC_PALETTE)
                    val = GetDeviceCaps(dc, SIZEPALETTE);
                else {
                    int bpp = GetDeviceCaps((HDC)data->hd, BITSPIXEL);
                    if(bpp==32)
                        val = INT_MAX;
                    else if(bpp<=8)
                        val = GetDeviceCaps((HDC)data->hd, NUMCOLORS);
                    else
                        val = 1 << (bpp * GetDeviceCaps((HDC)data->hd, PLANES));
                }
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
}


QImage QPixmap::convertToImage() const
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
        ((QPixmap*)this)->freeCell();

    GetDIBits(qt_display_dc(), DATA_HBM, 0, h, image.bits(), bmi, DIB_RGB_COLORS);

    if (mcp)
        ((QPixmap*)this)->allocCell();
#else
    memcpy(image.bits(), data->ppvBits, image.numBytes());
    qt_GetDIBColorTable(data->hd, &ds, 0, ncols, (RGBQUAD*)coltbl);
#endif

    for (int i=0; i<ncols; i++) {                // copy color table
        RGBQUAD *r = (RGBQUAD*)&coltbl[i];
        if (m)
            image.setColor(i, qRgba(r->rgbRed,
                               r->rgbGreen,
                               r->rgbBlue,255));
        else
            image.setColor(i, qRgb(r->rgbRed,
                               r->rgbGreen,
                               r->rgbBlue));
    }

    if (m) {
        image.setAlphaBuffer(true);
        QImage msk = m->convertToImage();

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


bool QPixmap::convertFromImage(const QImage &img, int conversion_flags)
{
    if (img.isNull()) {
        qWarning("QPixmap::convertFromImage: Cannot convert a null image");
        return false;
    }
    QImage image = img;
    int           d     = image.depth();
    int    dd    = defaultDepth();
    bool force_mono = (dd == 1 || isQBitmap() ||
                       (conversion_flags & Qt::ColorMode_Mask)==Qt::MonoOnly);

    if (force_mono) {                                // must be monochrome
        if (d != 1) {                                // dither
            image = image.convertDepth(1, conversion_flags);
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
                image = image.convertDepth(32, conversion_flags);
                d = image.depth();
            }
        } else if (image.depth() == 32) {
            int i = 0;
            while (i<image.height() && !hasRealAlpha) {
                uchar *p = image.scanLine(i);
                uchar *end = p + image.bytesPerLine();
                p += 3;
                while (p < end) {
                    if (*p!=0 && *p!=0xff) {
                        hasRealAlpha = true;
                        break;
                    }
                    p += 4;
                }
                ++i;
            }
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
    bool doAlloc = (QApplication::colorSpec() == QApplication::CustomColor
                     && QColor::hPal());
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
    bool doAlloc = (QApplication::colorSpec() == QApplication::CustomColor
                     && QColor::hPal());
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
        if (doAlloc) {
            QColor cl(c);
            cl.alloc();
        }
    }

    HDC dc;
    int sy;
    if (data->mcp) {
        dc = DATA_MCPI_MCP->handle();
        sy = DATA_MCPI_OFFSET;
    } else {
        dc = (HDC)handle();
        sy = 0;
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
        if (dc)
            StretchDIBits(dc, 0, sy, w, h, 0, 0, w, h,
                           image.bits(), bmi, DIB_RGB_COLORS, SRCCOPY);
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
        m = img.createAlphaMask(conversion_flags);
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
    HDC dc;
    int sy;
    if (pm.data->mcp) {
        dc = pm.DATA_MCPI_MCP->handle();
        sy = pm.DATA_MCPI_OFFSET;
    } else {
        dc = (HDC)pm.handle();
        sy = 0;
    }
    HDC src_dc = GetDC(window);
    BitBlt(dc, 0, sy, w, h, src_dc, x, y, SRCCOPY);
    ReleaseDC(window, src_dc);
    return pm;
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
    bool   depth1 = depth() == 1;

    if (isNull())                                // this is a null pixmap
        return copy();

    ws = width();
    hs = height();

    QWMatrix mat(matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(), 0., 0.);

    if (matrix.m12() == 0.0F  && matrix.m21() == 0.0F &&
         matrix.m11() >= 0.0F  && matrix.m22() >= 0.0F) {
        if (mat.m11() == 1.0F && mat.m22() == 1.0F)
            return *this;                        // identity matrix

        h = qRound(mat.m22()*hs);
        w = qRound(mat.m11()*ws);
        h = QABS(h);
        w = QABS(w);
        if (data->realAlphaBits == 0) {
            // we have to create the new pixmap before we query the handle of this,
            // as the handle might change if this is a multicell pixmap that gets
            // expanded by the constructor in the line below.
            QPixmap pm(w, h, depth(), optimization());
            HDC dc;
            int sy;
            if (data->mcp) {
                dc = DATA_MCPI_MCP->handle();
                sy = DATA_MCPI_OFFSET;
            } else {
                dc = (HDC)handle();
                sy = 0;
            }
            HDC pm_dc;
            int pm_sy;
            if (pm.data->mcp) {
                pm_dc = pm.multiCellHandle();
                pm_sy = pm.multiCellOffset();
            } else {
                pm_dc = (HDC)pm.handle();
                pm_sy = 0;
            }
#ifndef Q_OS_TEMP
            SetStretchBltMode(pm_dc, COLORONCOLOR);
#endif
            StretchBlt(pm_dc, 0, pm_sy, w, h,        // scale the pixmap
                    dc, 0, sy, ws, hs, SRCCOPY);
            if (data->mask) {
                QBitmap bm =
                    data->selfmask ? *((QBitmap*)(&pm)) :
                    data->mask->xForm(matrix);
                pm.setMask(bm);
            }
            return pm;
        }
    } else {
        // rotation or shearing
        QPointArray a(QRect(0,0,ws+1,hs+1));
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
        ((QPixmap*)this)->freeCell();
    int result;
#ifndef Q_OS_TEMP
    if (data->realAlphaBits) {
        GdiFlush();
        memcpy(sptr, data->realAlphaBits, sbpl*hs);
        result = 1;
    } else {
        result = GetDIBits(qt_display_dc(), DATA_HBM, 0, hs, sptr, bmi, DIB_RGB_COLORS);
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
        ((QPixmap*)this)->allocCell();

    if (!result) {                                // error, return null pixmap
        return QPixmap(0, 0, 0, data->bitmap, NormalOptim);
    }

    dbpl   = ((w*bpp+31)/32)*4;
    dbytes = dbpl*h;

    dptr = new uchar[dbytes];                        // create buffer for bits
    if (depth1)
        memset(dptr, 0xff, dbytes);
    else if (bpp == 8)
        memset(dptr, QColor(Qt::white).pixel(), dbytes);
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
        qWarning("QPixmap::xForm: display not supported (bpp=%d)",bpp);
        QPixmap pm;
        delete [] sptr;
        delete [] bmi_data;
        delete [] dptr;
        return pm;
    }

    delete [] sptr;

    QPixmap pm(w, h, depth(), data->bitmap, optimization());
    HDC pm_dc;
    int pm_sy;
    if (pm.data->mcp) {
        pm_dc = pm.multiCellHandle();
        pm_sy = pm.multiCellOffset();
    } else {
        pm_dc = (HDC)pm.handle();
        pm_sy = 0;
    }
    pm.data->uninit = false;
    bmh->biWidth  = w;
    bmh->biHeight = -h;
    bmh->biSizeImage = dbytes;
    if (data->realAlphaBits) {
        pm.initAlphaPixmap(dptr, dbytes, bmi);
    } else {
#ifndef Q_OS_TEMP
        SetDIBitsToDevice(pm_dc, 0, pm_sy, w, h, 0, 0, 0, h, dptr, bmi, DIB_RGB_COLORS);
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
        QBitmap bm = data->selfmask ? *((QBitmap*)(&pm)) :
                                     data->mask->xForm(matrix);
        pm.setMask(bm);
    }
    return pm;
}

/*!
  \fn HBITMAP QPixmap::hbm() const
  \internal
*/

/*!
  \fn bool QPixmap::isMultiCellPixmap() const
  \internal
*/

/*!
  \internal
*/
HDC QPixmap::multiCellHandle() const
{
    return data->mcp ? DATA_MCPI_MCP->handle() : 0;
}

/*!
  \internal
*/
HBITMAP QPixmap::multiCellBitmap() const
{
    return data->mcp ? DATA_MCPI_MCP->hbm() : 0;
}

/*!
  \internal
*/
int QPixmap::multiCellOffset() const
{
    return data->mcp ? DATA_MCPI_OFFSET : 0;
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
typedef QMultiCellPixmapList   *pQMultiCellPixmapList;

static const int mcp_num_lists  = 8;
static bool         mcp_lists_init = false;
static pQMultiCellPixmapList mcp_lists[mcp_num_lists];

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
int QPixmap::allocCell()
{
    if (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)                // only for NT based systems
        return -1;
    if (!mcp_lists_init)
        init_mcp();
    if (data->mcp)                                // cell already alloc'd
        freeCell();
    int s;
    int i = index_of_mcp_list(width(), depth() == 1, &s);
    if (i < 0)                                // too large width
        return -1;
    if (!mcp_lists[i])
        mcp_lists[i] = new QMultiCellPixmapList;

    QMultiCellPixmapList *list = mcp_lists[i];
    QMultiCellPixmap     *mcp  = list->first();
    int offset = -1;
    for(int i = 0; (i < list->size()) && (offset < 0); ++i) {
        offset = mcp->allocCell(height());
        if (offset < 0)
            mcp = list->at(i);
    }
    if (offset < 0) {                                // could not alloc
        mcp = new QMultiCellPixmap(s, depth(), 2048);
        offset = mcp->allocCell(height());
        if (offset < 0) {                        // height() > total height
            delete mcp;
            return offset;
        }
        list->append(mcp);
    }
    if (data->hd) {                                // copy into multi cell pixmap
        BitBlt(mcp->handle(), 0, offset, width(), height(), (HDC)data->hd,
               0, 0, SRCCOPY);
        DeleteDC((HDC)data->hd);
        data->hd = 0;
        DeleteObject(DATA_HBM);
        DATA_HBM = 0;
    }
    data->mcp = true;
    DATA_MCPI = new QMCPI;
    DATA_MCPI_MCP = mcp;
    DATA_MCPI_OFFSET = offset;
    return offset;
}


/*!
  \internal
*/
void QPixmap::freeCell(bool terminate)
{
    if (!mcp_lists_init || !data->mcp)
        return;
    QMultiCellPixmap *mcp = DATA_MCPI_MCP;
    int offset = DATA_MCPI_OFFSET;
    data->mcp = false;
    delete DATA_MCPI;
    DATA_MCPI = 0;
    Q_ASSERT(data->hd == 0);
    if (terminate) {                                // pixmap is being destroyed
        DATA_HBM = 0;
    } else {
        if (data->d == defaultDepth())
            DATA_HBM = CreateCompatibleBitmap(qt_display_dc(), data->w, data->h);
        else
            DATA_HBM = CreateBitmap(data->w, data->h, 1, 1, 0);
        data->hd = alloc_mem_dc(DATA_HBM, &data->old_hbm);
        BitBlt((HDC)data->hd, 0, 0, data->w, data->h, mcp->handle(), 0, offset, SRCCOPY);
    }
    mcp->freeCell(offset, data->h);
    if (mcp->isEmpty()) {                        // no more cells left
        int i = index_of_mcp_list(width(),depth()==1,0);
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

void QPixmap::convertToAlphaPixmap(bool initAlpha)
{
    char bmi_data[sizeof(BITMAPINFO)];
    memset(bmi_data, 0, sizeof(BITMAPINFO));
    BITMAPINFO             *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize                  = sizeof(BITMAPINFOHEADER);
    bmh->biWidth          = width();
    bmh->biHeight          = -height();                        // top-down bitmap
    bmh->biPlanes          = 1;
    bmh->biBitCount          = 32;
    bmh->biCompression          = BI_RGB;
    bmh->biSizeImage          = width() * height() * 4;
    bmh->biClrUsed          = 0;
    bmh->biClrImportant          = 0;

    QPixmap pm(width(), height(), -1);
    pm.initAlphaPixmap(0, 0, bmi);

#ifndef Q_OS_TEMP
    GetDIBits(qt_display_dc(), DATA_HBM, 0, height(), pm.data->realAlphaBits, bmi, DIB_RGB_COLORS);
    if (initAlpha) {
        // In bitBlt(), if the destination has an alpha channel and the source
        // doesn't have one, we bitBlt() the source with the destination's
        // alpha channel. In that case, there is no need to initialize the
        // alpha values.
        uchar *p = pm.data->realAlphaBits;
        uchar *pe = p + bmh->biSizeImage;
        if (mask()) {
            QImage msk = mask()->convertToImage();
            int i = 0;
            int w = width();
            int backgroundIndex = msk.color(0) == QColor(Qt::color0).rgb() ? 0 : 1;
            while (p < pe) {
                if (msk.pixelIndex(i%w, i/w) == backgroundIndex) {
                    *(p++) = 0x00;
                    *(p++) = 0x00;
                    *(p++) = 0x00;
                    *(p++) = 0x00;
                } else {
                    p += 3;
                    *(p++) = 0xff;
                }
                ++i;
            }
        } else {
            p += 3;
            while (p < pe) {
                *p = 0xff;
                p += 4;
            }
        }
    }
#else
    memcpy(pm.data->ppvBits, data->ppvBits, bmh->biSizeImage);
#endif
    if (mask())
        pm.setMask(*mask());

    *this = pm;
}

void QPixmap::bitBltAlphaPixmap(QPixmap *dst, int dx, int dy,
                                 const QPixmap *src, int sx, int sy,
                                 int sw, int sh, bool useDstAlpha)
{
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

#ifndef Q_OS_TEMP
    GdiFlush();
#endif
    uchar *sBits = src->data->realAlphaBits + (sy * src->width() + sx) * 4;
    uchar *dBits = dst->data->realAlphaBits + (dy * dst->width() + dx) * 4;
    int sw4 = sw * 4;
    int src4 = src->width() * 4;
    int dst4 = dst->width() * 4;
    if (useDstAlpha) {
        // Copy the source pixels premultiplied with the destination's alpha
        // channel. The alpha channel remains the destination's alpha channel.
        uchar *sCur;
        uchar *dCur;
        uchar alphaByte;
        for (int i=0; i<sh; i++) {
            sCur = sBits;
            dCur = dBits;
            for (int j=0; j<sw; j++) {
                alphaByte = *(dCur+3);
                if (alphaByte == 0 || (*(sCur+3)) == 0) {
                    dCur += 4;
                    sCur += 4;
                } else if (alphaByte == 255) {
                    *(dCur++) = *(sCur++);
                    *(dCur++) = *(sCur++);
                    *(dCur++) = *(sCur++);
                    dCur++;
                    sCur++;
                } else {
                    *(dCur++) = ((*(sCur++)) * (int)alphaByte + 127) / 255;
                    *(dCur++) = ((*(sCur++)) * (int)alphaByte + 127) / 255;
                    *(dCur++) = ((*(sCur++)) * (int)alphaByte + 127) / 255;
                    dCur++;
                    sCur++;
                }
            }
            sBits += src4;
            dBits += dst4;
        }
    } else {
        // Copy the source into the destination. Use the source's alpha
        // channel.
        for (int i=0; i<sh; i++) {
            memcpy(dBits, sBits, sw4);
            sBits += src4;
            dBits += dst4;
        }
    }
}

Q_GUI_EXPORT void copyBlt(QPixmap *dst, int dx, int dy,
                       const QPixmap *src, int sx, int sy, int sw, int sh)
{
    if (! dst || ! src || sw == 0 || sh == 0 || dst->depth() != src->depth()) {
        Q_ASSERT(dst != 0);
        Q_ASSERT(src != 0);
        return;
    }

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

    if (src->data->realAlphaBits) {
        if (!dst->data->realAlphaBits)
            dst->convertToAlphaPixmap();
        QPixmap::bitBltAlphaPixmap(dst, dx, dy, src, sx, sy, sw, sh, false);
    } else {
        // copy pixel data
        bitBlt(dst, dx, dy, src, sx, sy, sw, sh, true);
    }
}

QPaintEngine *QPixmap::paintEngine() const
{
    if (!data->paintEngine)
        data->paintEngine = new QWin32PaintEngine(const_cast<QPixmap*>(this));
    return data->paintEngine;
}
