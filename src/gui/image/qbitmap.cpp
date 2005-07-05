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

#include "qbitmap.h"
#include "qpixmap_p.h"
#include "qimage.h"
#include "qvariant.h"
#include <qpainter.h>
#if defined(Q_WS_X11)
#include <private/qt_x11_p.h>
#endif

/*!
    \class QBitmap qbitmap.h
    \brief The QBitmap class provides monochrome (1-bit depth) pixmaps.

    \ingroup multimedia

    The QBitmap class is a monochrome off-screen paint device used mainly for
    creating custom QCursor and QBrush objects, settings masks for \link
    QPixmap::setMask() pixmaps \endlink and \link QWidget::setMask() widgets
    \endlink, and for QRegion.

    A QBitmap is a QPixmap with a \link QPixmap::depth() depth\endlink
    of 1. If a pixmap with a depth greater than 1 is assigned to a
    bitmap, the bitmap will be dithered automatically. A QBitmap is
    guaranteed to always have the depth 1, unless it is
    \link QPixmap::isNull() null \endlink since null pixmaps have depth 0.

    When drawing in a QBitmap (or QPixmap with depth 1), we recommend
    using the QColor objects Qt::color0 and Qt::color1.
    Painting with Qt::color0 sets the bitmap bits to 0, and painting
    with Qt::color1 sets the bits to 1. For a bitmap, 0-bits indicate
    background (or transparent pixels) and 1-bits indicate foreground (or
    opaque pixels). Using the Qt::black and Qt::white colors make no
    sense because the QColor::pixel() value is not necessarily 0 for
    black and 1 for white.

    The QBitmap can be transformed (translated, scaled, sheared, and
    rotated) using transform().

    Just like the QPixmap class, QBitmap is optimized by the use of
    \link shclass.html implicit sharing\endlink, so it is very
    efficient to pass QBitmap objects as arguments.

    \sa QPixmap, QPainter::drawPixmap(), bitBlt(), \link shclass.html Shared Classes\endlink
*/


/*!
    Constructs a null bitmap.

    \sa QPixmap::isNull()
*/

QBitmap::QBitmap()
    : QPixmap(QSize(0, 0), BitmapType)
{
}

/*!
    \fn QBitmap::QBitmap(int width, int height)

    Constructs a bitmap with the given \a width and \a height. The pixels
    inside are uninitialized.

*/

QBitmap::QBitmap(int w, int h)
    : QPixmap(QSize(w, h), BitmapType)
{
}

/*!
    \fn QBitmap::clear()

    Clear the bitmap to Qt::color0.
*/

/*!
    \overload

    Constructs a bitmap with the given \a size.

    The pixels in the bitmap are uninitialized.
*/

QBitmap::QBitmap(const QSize &size)
    : QPixmap(size, BitmapType)
{
}

/*!
    Constructs a bitmap that is a copy of the \a pixmap given.

    Dithering will be performed if the pixmap has a QPixmap::depth()
    greater than 1.
*/

QBitmap::QBitmap(const QPixmap &pixmap)
{
    QBitmap::operator=(pixmap);
}

/*!
  \fn QBitmap::QBitmap(const QImage &image)

    Constructs a bitmap that is a copy of the \a image given.

    Dithering will be performed if the image has a QImage::depth()
    greater than 1.
*/

/*!
    Constructs a bitmap from the file referred to by \a fileName. If the
    file does not exist, or is of an unknown format, the bitmap becomes a
    null bitmap.

    The parameters \a fileName and \a format are passed on to
    QPixmap::load(). Dithering will be performed if the file format
    uses more than 1 bit per pixel.

    \sa QPixmap::isNull(), QPixmap::load(), QPixmap::loadFromData(),
    QPixmap::save(), QImageReader::imageFormat()
*/

QBitmap::QBitmap(const QString& fileName, const char *format)
    : QPixmap(QSize(0, 0), BitmapType)
{
    load(fileName, format, Qt::MonoOnly);
}

/*!
    \overload

    Assigns the \a pixmap to this bitmap and returns a
    reference to it.

    Dithering will be performed if the pixmap has a QPixmap::depth()
    greater than 1.
*/

QBitmap &QBitmap::operator=(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {                        // a null pixmap
        QBitmap bm(0, 0);
        QBitmap::operator=(bm);
    } else if (pixmap.depth() == 1) {                // 1-bit pixmap
        QPixmap::operator=(pixmap);                // shallow assignment
    } else {                                        // n-bit depth pixmap
        QImage image;
        image = pixmap.toImage();                                // convert pixmap to image
        *this = fromImage(image);                                // will dither image
    }
    return *this;
}


#ifdef QT3_SUPPORT
QBitmap::QBitmap(int w, int h, const uchar *bits, bool isXbitmap)
{
    *this = fromData(QSize(w, h), bits, isXbitmap ? QImage::Format_MonoLSB : QImage::Format_Mono);
}


QBitmap::QBitmap(const QSize &size, const uchar *bits, bool isXbitmap)
{
    *this = fromData(size, bits, isXbitmap ? QImage::Format_MonoLSB : QImage::Format_Mono);
}
#endif

/*!
  Destroys the bitmap.
*/
QBitmap::~QBitmap()
{
}

/*!
   Returns the bitmap as a QVariant.
*/
QBitmap::operator QVariant() const
{
    return QVariant(QVariant::Bitmap, this);
}

/*!
  \fn QBitmap &QBitmap::operator=(const QImage &image)

    \overload

    Converts the image \a image to a bitmap, and assigns the result to
    this bitmap. Returns a reference to the bitmap.

    Dithering will be performed if the image has a QImage::depth()
    greater than 1.
*/

/*!
    Returns a copy of the given \a image converted to a bitmap using the
    image conversion flags specified by \a flags.
*/
QBitmap QBitmap::fromImage(const QImage &image, Qt::ImageConversionFlags flags)
{
    if (image.isNull())
        return QBitmap();
    QImage img = image.convertToFormat(QImage::Format_MonoLSB, flags);
#if defined (Q_WS_WIN) || defined (Q_WS_QWS)
    QBitmap bm;
    bm.data->image = img;

    // Swap colors to match so that default config draws more correctly.
    // black bits -> black pen in QPainter
    if (image.numColors() == 2 && qGray(image.color(0)) < qGray(image.color(1))) {
        QRgb color0 = image.color(0);
        QRgb color1 = image.color(1);
        bm.data->image.setColor(0, color1);
        bm.data->image.setColor(1, color0);
        bm.data->image.invertPixels();
    }
    return bm;
#elif defined(Q_WS_X11)
    QBitmap bm;
    // make sure image.color(0) == Qt::color0 (white) and image.color(1) == Qt::color1 (black)
    const QRgb c0 = QColor(Qt::black).rgb();
    const QRgb c1 = QColor(Qt::white).rgb();
    if (img.color(0) == c0 && img.color(1) == c1) {
        img.invertPixels();
        img.setColor(0, c1);
        img.setColor(1, c0);
    }

    char  *bits;
    uchar *tmp_bits;
    int w = img.width();
    int h = img.height();
    int bpl = (w+7)/8;
    int ibpl = img.bytesPerLine();
    if (bpl != ibpl) {
        tmp_bits = new uchar[bpl*h];
        bits = (char *)tmp_bits;
        uchar *p, *b;
        int y;
        b = tmp_bits;
        p = img.scanLine(0);
        for (y = 0; y < h; y++) {
            memcpy(b, p, bpl);
            b += bpl;
            p += ibpl;
        }
    } else {
        bits = (char *)img.bits();
        tmp_bits = 0;
    }
    bm.data->hd = (Qt::HANDLE)XCreateBitmapFromData(bm.data->xinfo.display(),
                                                    RootWindow(bm.data->xinfo.display(), bm.data->xinfo.screen()),
                                                    bits, w, h);

#ifndef QT_NO_XRENDER
    if (X11->use_xrender)
        bm.data->picture = XRenderCreatePicture(X11->display, bm.data->hd,
                                                XRenderFindStandardFormat(X11->display, PictStandardA1), 0, 0);
#endif // QT_NO_XRENDER

    if (tmp_bits)                                // Avoid purify complaint
        delete [] tmp_bits;
    bm.data->w = w;  bm.data->h = h;  bm.data->d = 1;

    return bm;
#else
    return QBitmap(QPixmap::fromImage(img, flags|Qt::MonoOnly));
#endif
}

/*!
    Constructs a bitmap with the given \a size, and sets the contents to
    the \a bits supplied.

    The bitmap data has to be byte aligned and provided in in the bit
    order specified by \a monoFormat. The mono format must be either
    QImage::Format_Mono or QImage::Format_MonoLSB.

    Use QImage::Format_Mono to specify data on the XBM format.

*/
QBitmap QBitmap::fromData(const QSize &size, const uchar *bits, QImage::Format monoFormat)
{
    Q_ASSERT(monoFormat == QImage::Format_Mono || monoFormat == QImage::Format_MonoLSB);

    QImage image(size, monoFormat);
    image.setColor(0, Qt::color0);
    image.setColor(1, Qt::color1);

    // Need to memcpy each line separatly since QImage is 32bit aligned and
    // this data is only byte aligned...
    int bytesPerLine = (size.width() + 7) / 8;
    for (int y = 0; y < size.height(); ++y)
        memcpy(image.scanLine(y), bits + bytesPerLine * y, bytesPerLine);
    return QBitmap::fromImage(image);
}


/*!
    Returns a transformed copy of this bitmap using the \a matrix given.

    This function does exactly the same as QPixmap::transform(), except
    that it returns a QBitmap instead of a QPixmap.

    \sa QPixmap::transformed()
*/

QBitmap QBitmap::transformed(const QMatrix &matrix) const
{
    QBitmap bm = QPixmap::transformed(matrix);
    return bm;
}

#ifdef QT3_SUPPORT
/*!
    \fn QBitmap QBitmap::xForm(const QMatrix &matrix) const

    Use transform(\a matrix) instead.
*/

/*!
    \fn QBitmap::QBitmap(const QSize &size, bool clear)

    Use the QBitmap(\a size) constructor. If \a clear is true, call
    clear() afterward.

    \sa clear()
*/

/*!
    \fn QBitmap::QBitmap(int w, int h, bool clear)

    Use the QBitmap(\a w, \a h) constructor. If \a clear is true, call
    clear() afterward.

    \sa clear()
*/

/*!
    \fn QBitmap::QBitmap(int width, int height, const uchar *bits, bool isXbitmap)

    Constructs a bitmap with the given \a width and \a height,
    and sets the contents to the \a bits supplied.

    The \a isXbitmap flag should be true if \a bits was generated by
    the X11 bitmap program. The X bitmap bit order is little endian.
    The QImage documentation discusses bit order of monochrome
    images. Opposed to QImage, the data has to be byte aligned.

    Example (creates an arrow bitmap):
    \code
        uchar arrow_bits[] = { 0x3f, 0x1f, 0x0f, 0x1f, 0x3b, 0x71, 0xe0, 0xc0 };
        QBitmap bm(8, 8, arrow_bits, true);
    \endcode
*/


/*!
  \fn QBitmap::QBitmap(const QSize &size, const uchar *bits, bool isXbitmap)

    \overload

    Constructs a bitmap with the given \a size, and sets the contents to
    the \a bits supplied.

    The \a isXbitmap flag should be true if \a bits was generated by
    the X11 bitmap program. The X bitmap bit order is little endian.
    The QImage documentation discusses bit order of monochrome images.
*/
#endif
