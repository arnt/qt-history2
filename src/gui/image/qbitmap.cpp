/****************************************************************************
**
** Implementation of QBitmap class.
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

#include "qbitmap.h"
#include "qimage.h"
#include <qpainter.h>

/*!
    \class QBitmap qbitmap.h
    \brief The QBitmap class provides monochrome (1-bit depth) pixmaps.

    \ingroup multimedia

    The QBitmap class is a monochrome off-screen paint device used
    mainly for creating custom QCursor and QBrush objects, in
    QPixmap::setMask() and for QRegion.

    A QBitmap is a QPixmap with a \link QPixmap::depth() depth\endlink
    of 1. If a pixmap with a depth greater than 1 is assigned to a
    bitmap, the bitmap will be dithered automatically. A QBitmap is
    guaranteed to always have the depth 1, unless it is
    QPixmap::isNull() which has depth 0.

    When drawing in a QBitmap (or QPixmap with depth 1), we recommend
    using the QColor objects \c Qt::color0 and \c Qt::color1.
    Painting with \c Qt::color0 sets the bitmap bits to 0, and painting
    with \c Qt::color1 sets the bits to 1. For a bitmap, 0-bits indicate
    background (or transparent pixels) and 1-bits indicate foreground (or
    opaque pixels). Using the \c black and \c white QColor objects make no
    sense because the QColor::pixel() value is not necessarily 0 for
    black and 1 for white.

    The QBitmap can be transformed (translated, scaled, sheared, and
    rotated) using xForm().

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
{
    data->bitmap = true;
}


/*!
    \fn QBitmap::QBitmap(int width, int height, bool clear, QPixmap::Optimization optimization)

    Constructs a bitmap with the given \a width and \a height.

    The pixels in the bitmap are uninitialized if \a clear is false;
    otherwise it is filled with pixel value 0 (the QColor \c
    Qt::color0).

    The optional \a optimization argument specifies the optimization
    setting for the bitmap. The default optimization should be used in
    most cases. Games and other pixmap-intensive applications may
    benefit from setting this argument; see \l{QPixmap::Optimization}.

    \sa QPixmap::setOptimization(), QPixmap::setDefaultOptimization()
*/

QBitmap::QBitmap(int w, int h, bool clear,
                  QPixmap::Optimization optimization)
    : QPixmap(w, h, 1, optimization)
{
    data->bitmap = true;
    if (clear)
        fill(Qt::color0);
}


/*!
    \overload

    Constructs a bitmap with the given \a size.

    The pixels in the bitmap are uninitialized if \a clear is false;
    otherwise it is filled with pixel value 0 (the QColor \c
    Qt::color0).

    The optional \a optimization argument specifies the optimization
    setting for the bitmap. The default optimization should be used in
    most cases. Games and other pixmap-intensive applications may
    benefit from setting this argument; see \l{QPixmap::Optimization}.
*/

QBitmap::QBitmap(const QSize &size, bool clear,
                  QPixmap::Optimization optimization)
    : QPixmap(size, 1, optimization)
{
    data->bitmap = true;
    if (clear)
        fill(Qt::color0);
}


/*!
    \fn QBitmap::QBitmap(int width, int height, const uchar *bits, bool isXbitmap)

    Constructs a bitmap with the given \a width and \a height,
    and sets the contents to the \a bits supplied.

    The \a isXbitmap flag should be true if \a bits was generated by
    the X11 bitmap program. The X bitmap bit order is little endian.
    The QImage documentation discusses bit order of monochrome images.

    Example (creates an arrow bitmap):
    \code
        uchar arrow_bits[] = { 0x3f, 0x1f, 0x0f, 0x1f, 0x3b, 0x71, 0xe0, 0xc0 };
        QBitmap bm(8, 8, arrow_bits, true);
    \endcode
*/

QBitmap::QBitmap(int w, int h, const uchar *bits, bool isXbitmap)
    : QPixmap(w, h, bits, isXbitmap)
{
    data->bitmap = true;
}


/*!
    \overload

    Constructs a bitmap with the given \a size, and sets the contents to
    the \a bits supplied.

    The \a isXbitmap flag should be true if \a bits was generated by
    the X11 bitmap program. The X bitmap bit order is little endian.
    The QImage documentation discusses bit order of monochrome images.
*/

QBitmap::QBitmap(const QSize &size, const uchar *bits, bool isXbitmap)
    : QPixmap(size.width(), size.height(), bits, isXbitmap)
{
    data->bitmap = true;
}


/*!
    Constructs a bitmap that is a copy of the \a bitmap given.
*/

QBitmap::QBitmap(const QBitmap &bitmap)
    : QPixmap(bitmap)
{
}


/*!
    Constructs a bitmap that is a copy of the \a pixmap given.

    Dithering will be performed if the pixmap has a QPixmap::depth()
    greater than 1.
*/

QBitmap::QBitmap(const QPixmap &pixmap)
{
    data->bitmap = true;
    QBitmap::operator=(pixmap);
}

/*!
    Constructs a bitmap that is a copy of the \a image given.

    Dithering will be performed if the image has a QImage::depth()
    greater than 1.
*/

QBitmap::QBitmap(const QImage &image)
{
    data->bitmap = true;
    QBitmap::operator=(image);
}


#ifndef QT_NO_IMAGEIO
/*!
    Constructs a bitmap from the file referred to by \a fileName. If the
    file does not exist, or is of an unknown format, the bitmap becomes a
    null bitmap.

    The parameters \a fileName and \a format are passed on to
    QPixmap::load(). Dithering will be performed if the file format
    uses more than 1 bit per pixel.

    \sa QPixmap::isNull(), QPixmap::load(), QPixmap::loadFromData(),
    QPixmap::save(), QPixmap::imageFormat()
*/

QBitmap::QBitmap(const QString& fileName, const char *format)
    : QPixmap() // Will set bitmap to null bitmap, explicit call for clarity
{
    data->bitmap = true;
    load(fileName, format, Mono);
}
#endif

/*!
    Assigns the \a bitmap to this bitmap, and returns a
    reference to it.
*/

QBitmap &QBitmap::operator=(const QBitmap &bitmap)
{
    QPixmap::operator=(bitmap);
    Q_ASSERT(data->bitmap);
    return *this;
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
        QBitmap bm(0, 0, false, pixmap.optimization());
        QBitmap::operator=(bm);
    } else if (pixmap.depth() == 1) {                // 1-bit pixmap
        if (pixmap.isQBitmap()) {                // another QBitmap
            QPixmap::operator=(pixmap);                // shallow assignment
        } else {                                // not a QBitmap, but 1-bit
            QBitmap bm(pixmap.size(), false, pixmap.optimization());
            QPainter p(&bm);
            p.drawPixmap(0, 0, pixmap);
            p.end();
            QBitmap::operator=(bm);
        }
    } else {                                        // n-bit depth pixmap
        QImage image;
        image = pixmap;                                // convert pixmap to image
        *this = image;                                // will dither image
    }
    return *this;
}


/*!
    \overload

    Converts the image \a image to a bitmap, and assigns the result to
    this bitmap. Returns a reference to the bitmap.

    Dithering will be performed if the image has a QImage::depth()
    greater than 1.
*/

QBitmap &QBitmap::operator=(const QImage &image)
{
    convertFromImage(image);
    return *this;
}


#ifndef QT_NO_PIXMAP_TRANSFORMATION
/*!
    Returns a transformed copy of this bitmap using the \a matrix given.

    This function does exactly the same as QPixmap::xForm(), except
    that it returns a QBitmap instead of a QPixmap.

    \sa QPixmap::xForm()
*/

QBitmap QBitmap::xForm(const QWMatrix &matrix) const
{
    QPixmap pm = QPixmap::xForm(matrix);
    QBitmap bm;
    // Here we fake the pixmap to think it's a QBitmap. With this trick,
    // the QBitmap::operator=(const QPixmap&) will just refer the
    // pm.data and we do not need to perform a bitBlt.
    pm.data->bitmap = true;
    bm = pm;
    return bm;
}
#endif // QT_NO_TRANSFORMATIONS



