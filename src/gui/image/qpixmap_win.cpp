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

#include "qbitmap.h"
#include "qimage.h"
#include "qwidget.h"
#include "qpainter.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qapplication.h"
#include <private/qinternal_p.h>
#include "qevent.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qdatetime.h"
#include "qpixmapcache.h"
#include "qimagereader.h"
#include "qimagewriter.h"
#include "qdebug.h"

QPixmap QPixmap::grabWindow(WId winId, int x, int y, int w, int h )
{
    RECT r;
    GetClientRect(winId, &r);

    if (w < 0) w = r.right - r.left;
    if (h < 0) h = r.bottom - r.top;

    // Create and setup bitmap
    HDC bitmap_dc = CreateCompatibleDC(qt_win_display_dc());
    HBITMAP bitmap = CreateCompatibleBitmap(qt_win_display_dc(), w, h);
    HGDIOBJ null_bitmap = SelectObject(bitmap_dc, bitmap);

    // copy data
    HDC window_dc = GetDC(winId);
    BitBlt(bitmap_dc, 0, 0, w, h, window_dc, x, y, SRCCOPY);

    // clean up all but bitmap
    ReleaseDC(winId, window_dc);
    SelectObject(bitmap_dc, null_bitmap);
    DeleteDC(bitmap_dc);

    QPixmap pixmap = QPixmap::fromWinHBITMAP(bitmap);

    DeleteObject(bitmap);

    return pixmap;
}



/*!
    \enum QPixmap::HBitmapFormat

    This enum defines how the conversion between \c HBITMAP
    and QPixmap is performed.

    \warning This enum is only available on Windows.

    \value NoAlpha The alpha channel is ignored and always treated as
    being set to fully opaque. This is preferred if the \c HBITMAP is
    used with standard GDI calls, such as \c BitBlt().

    \value PremultipliedAlpha The \c HBITMAP is treated as having a
    alpha channel and premultiplied colors. This is preferred if the
    \c HBITMAP is accessed through the \c AlphaBlend() GDI function.

    \sa fromWinHBITMAP(), toWinHBITMAP()
*/

/*!
    Creates a \c HBITMAP equivalent to the QPixmap, based on the
    given \a format, and returns the \c HBITMAP handle.

    It is the caller's responsibility to free the \c HBITMAP data
    after use.

    \warning This function is only available on Windows.

    \sa fromWinHBITMAP()
*/
HBITMAP QPixmap::toWinHBITMAP(HBitmapFormat format) const
{
    int w = data->image.width();
    int h = data->image.height();

    HDC display_dc = qt_win_display_dc();

    // Define the header
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = w;
    bmi.bmiHeader.biHeight      = -h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = w * h * 4;

    // Create the pixmap
    uchar *pixels = 0;
    HBITMAP bitmap = CreateDIBSection(display_dc, &bmi, DIB_RGB_COLORS, (void **) &pixels, 0, 0);
    if (!bitmap) {
        qErrnoWarning("QPixmap::toWinHBITMAP(), failed to create dibsection");
        return 0;
    }
    if (!pixels) {
        qErrnoWarning("QPixmap::toWinHBITMAP(), did not allocate pixel data");
        return 0;
    }

    // Copy over the data
    QImage::Format imageFormat = format == NoAlpha
                                 ? QImage::Format_RGB32
                                 : QImage::Format_ARGB32_Premultiplied;
    const QImage image = data->image.convertToFormat(imageFormat);
    int bytes_per_line = w * 4;
    for (int y=0; y<h; ++y)
        memcpy(pixels + y * bytes_per_line, image.scanLine(y), bytes_per_line);

    return bitmap;
}

/*!
    Returns a QPixmap that is equivalent to the given \c bitmap
    which has the specified \a format.

    \sa toWinHBITMAP()
*/
QPixmap QPixmap::fromWinHBITMAP(HBITMAP bitmap, HBitmapFormat format)
{
    // Verify size
    BITMAP bitmap_info;
    memset(&bitmap_info, 0, sizeof(BITMAP));
    if (!GetObject(bitmap, sizeof(BITMAP), &bitmap_info)) {
        qErrnoWarning("QPixmap::fromWinHBITMAP(), failed to get bitmap info");
        return QPixmap();
    }
    int w = bitmap_info.bmWidth;
    int h = bitmap_info.bmHeight;

    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = w;
    bmi.bmiHeader.biHeight      = -h;
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = w * h * 4;

    QImage result;
    // Get bitmap bits
    uchar *data = (uchar *) qMalloc(bmi.bmiHeader.biSizeImage);
    if (GetDIBits(qt_win_display_dc(), bitmap, 0, h, data, &bmi, DIB_RGB_COLORS)) {

        QImage::Format imageFormat = QImage::Format_ARGB32_Premultiplied;
        uint mask = 0;
        if (format == NoAlpha) {
            imageFormat = QImage::Format_RGB32;
            mask = 0xff000000;
        }

        // Create image and copy data into image.
        QImage image(w, h, imageFormat);
        int bytes_per_line = w * sizeof(QRgb);
        for (int y=0; y<h; ++y) {
            QRgb *dest = (QRgb *) image.scanLine(y);
            const QRgb *src = (const QRgb *) (data + y * bytes_per_line);
            for (int x=0; x<w; ++x) {
                dest[x] = src[x] | mask;
            }
        }
        result = image;
    } else {
        qWarning("QPixmap::fromWinHBITMAP(), failed to get bitmap bits");
    }
    qFree(data);
    return fromImage(result);
}
