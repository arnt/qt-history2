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
#include "qimageio.h"

#if defined(Q_WS_X11)
#include "qx11info_x11.h"
#endif

//#define QT_NO_PIXMAP_RETAIN
#ifndef QT_NO_PIXMAP_RETAIN
#include <qcache.h>
#include <qfileinfo.h>
#include <qdatetime.h>
class QRetainedPixmap : public QPixmap
{
public:
    QRetainedPixmap(const QPixmap &p) : QPixmap(p) { }
    virtual bool isOutOfDate() { return false; } //never goes out of date by default
};
Q_DECLARE_SHARED(QRetainedPixmap)

class QRetainedPixmapFile : public QRetainedPixmap
{
    QString file;
    QDateTime mtime;
public:
    QRetainedPixmapFile(const QString &f, const QPixmap &p) : QRetainedPixmap(p), file(f) {
        mtime = QFileInfo(file).lastModified();
    }
    virtual bool isOutOfDate() {
        return mtime != QFileInfo(file).lastModified();
    }
};

typedef QCache<QString, QRetainedPixmap> QRetainedPixmapCache;
Q_GLOBAL_STATIC_WITH_ARGS(QRetainedPixmapCache, qt_retained_pixmaps, (1024*1024))
#endif

/*!
    \class QPixmap qpixmap.h
    \brief The QPixmap class is an off-screen, pixel-based paint device.

    \ingroup multimedia
    \ingroup shared
    \mainclass

    QPixmap is one of the two classes Qt provides for dealing with
    images; the other is QImage. QPixmap is designed and optimized
    for drawing; QImage is designed and optimized for I/O and for
    direct pixel access/manipulation. There are (slow) functions to
    convert between QImage and QPixmap: convertToImage() and
    convertFromImage().

    One common use of the QPixmap class is to enable smooth updating
    of widgets. Whenever something complex needs to be drawn, you can
    use a pixmap to obtain flicker-free drawing, like this:

    \list 1
    \i Create a pixmap with the same size as the widget.
    \i Fill the pixmap with the widget background color.
    \i Paint the pixmap.
    \i QPainter::drawPixmap() the pixmap contents onto the widget.
    \endlist

    Pixel data in a pixmap is internal and is managed by the
    underlying window system. Pixels can be accessed only through
    QPainter functions and by converting the
    QPixmap to a QImage.

    You can easily display a QPixmap on the screen using
    QLabel::setPixmap(). For example, all the QButton subclasses
    support pixmap use.

    The QPixmap class uses \link shclass.html copy-on-write\endlink,
    so it is practical to pass QPixmap objects by value.

    You can retrieve the width(), height(), depth() and size() of a
    pixmap. The enclosing rectangle can be determined with rect().
    Pixmaps can be filled with fill() and resized with resize(). You
    can create and set a mask with createHeuristicMask() and setMask().
    Use selfMask() to see if the pixmap is identical to its mask.

    In addition to loading a pixmap from file using load() you can
    also loadFromData(). You can control optimization with
    setOptimization() and obtain a transformed version of the pixmap
    using xForm()

    Note regarding Windows 95 and 98: on Windows 9x the system crashes
    if you create more than about 1000 pixmaps, independent of the
    size of the pixmaps or installed RAM. Windows NT-systems (including
    2000, XP and following versions) do not have the same limitation,
    but depending on the graphics equipment the system will fail to
    allocate pixmap objects at some point (due to system running out of
    GDI resources).

    Qt tries to work around the resource limitation. If you set the
    pixmap optimization to \c QPixmap::MemoryOptim and the width of
    your pixmap is less than or equal to 128 pixels, Qt stores the
    pixmap in a way that is very memory-efficient when there are many
    pixmaps.

    If your application uses dozens or hundreds of pixmaps (for
    example on tool bar buttons and in popup menus), and you plan to
    run it on Windows 95 or Windows 98, we recommend using code like
    this:

    \code
        QPixmap::setDefaultOptimization(QPixmap::MemoryOptim);
        while (...) {
            // load tool bar pixmaps etc.
            QPixmap *pixmap = new QPixmap(fileName);
        }
        QPixmap::setDefaultOptimization(QPixmap::NormalOptim);
    \endcode

    In general it is recommended to make as much use of QPixmap's
    implicit sharing and the QPixmapCache as possible.

    \sa QBitmap, QImage, QImageIO, \link shclass.html Shared Classes\endlink
*/

/*!
    \enum QPixmap::ColorMode

    \compat

    This enum type defines the color modes that exist for converting
    QImage objects to QPixmap.

    \value Auto  Select \c Color or \c Mono on a case-by-case basis.
    \value Color Always create colored pixmaps.
    \value Mono  Always create bitmaps.
*/

/*!
    \enum QPixmap::Optimization

    QPixmap has the choice of optimizing for speed or memory in a few
    places; the best choice varies from pixmap to pixmap but can
    generally be derived heuristically. This enum type defines a
    number of optimization modes that you can set for any pixmap to
    tweak the speed/memory tradeoffs:

    \value DefaultOptim  Whatever QPixmap::defaultOptimization()
        returns. A pixmap with this optimization will have whatever
        the current default optimization is. If the default
        optimization is changed using setDefaultOptimization(), then
        this will not effect any pixmaps that have already been
        created.

    \value NoOptim  No optimization (currently the same as \c
        MemoryOptim).

    \value MemoryOptim  Optimize for minimal memory use on Windows
        9x and X11 systems.

    \value NormalOptim  Optimize for typical usage. Often uses more
        memory than \c MemoryOptim, and is often faster.

    \value BestOptim  Optimize for pixmaps that are drawn very often
        and where performance is critical. Generally uses more memory
        than \c NormalOptim and may provide a little more speed.

    \value LoadOptim  Optimize for pixmaps that are loaded from disk
                      rather than stored in memory.

    We recommend using \c DefaultOptim.

*/

QPixmap::Optimization QPixmap::defOptim = QPixmap::NormalOptim;


/*!
    \internal

    Private constructor that takes the bitmap flag, the optimization,
    and a screen.
*/

QPixmap::QPixmap(int w, int h, int depth, bool bitmap,
                  Optimization optimization)
    : QPaintDevice(QInternal::Pixmap)
{
    init(w, h, depth, bitmap, optimization);
}


/*!
    Constructs a null pixmap.

    \sa isNull()
*/

QPixmap::QPixmap()
    : QPaintDevice(QInternal::Pixmap)
{
    init(0, 0, 0, false, defOptim);
}

/*!
    Constructs a pixmap from the QImage \a image.

    \sa convertFromImage()
*/

QPixmap::QPixmap(const QImage& image)
    : QPaintDevice(QInternal::Pixmap)
{
    init(0, 0, 0, false, defOptim);
    fromImage(image);
}

/*!
    Constructs a pixmap with \a w width, \a h height and \a depth bits
    per pixel. The pixmap is optimized in accordance with the \a
    optimization value.

    The contents of the pixmap is uninitialized.

    The \a depth can be either 1 (monochrome) or the depth of the
    current video mode. If \a depth is negative, then the hardware
    depth of the current video mode will be used.

    If either \a w or \a h is zero, a null pixmap is constructed.

    \sa isNull() QPixmap::Optimization
*/

QPixmap::QPixmap(int w, int h, int depth, Optimization optimization)
    : QPaintDevice(QInternal::Pixmap)
{
    init(w, h, depth, false, optimization);
}

/*!
    \overload

    Constructs a pixmap of size \a size, \a depth bits per pixel,
    optimized in accordance with the \a optimization value.
*/

QPixmap::QPixmap(const QSize &size, int depth, Optimization optimization)
    : QPaintDevice(QInternal::Pixmap)
{
    init(size.width(), size.height(), depth, false, optimization);
}

#ifndef QT_NO_IMAGEIO
/*!
    Constructs a pixmap from the file with the given \a fileName. If the
    file does not exist or is of an unknown format, the pixmap becomes a
    null pixmap.

    The \a fileName, \a format and \a flags parameters are
    passed on to load(). This means that the data in \a fileName is
    not compiled into the binary. If \a fileName contains a relative
    path (e.g. the filename only) the relevant file must be found
    relative to the runtime working directory.

    The way the pixmap is handled is specified by \a optimization.

    If the image needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a
    flags to specify how you'd prefer this to happen.

    \sa Qt::ImageConversionFlags isNull() load() loadFromData() save() imageFormat() Optimization
*/

QPixmap::QPixmap(const QString& fileName, const char *format,
                 Qt::ImageConversionFlags flags, Optimization optimization)
    : QPaintDevice(QInternal::Pixmap)
{
    init(0, 0, 0, false, optimization);
    load(fileName, format, flags);
}
#endif //QT_NO_IMAGEIO

/*!
    Constructs a pixmap that is a copy of \a pixmap.
*/

QPixmap::QPixmap(const QPixmap &pixmap)
    : QPaintDevice(QInternal::Pixmap)
{
    if (pixmap.paintingActive()) {                // make a deep copy
        data = 0;
        operator=(pixmap.copy());
    } else {
        data = pixmap.data;
        data->ref();
        devFlags = pixmap.devFlags;                // copy QPaintDevice flags
    }
}


#ifndef QT_NO_IMAGEIO_XPM
// helper
extern void qt_read_xpm_image_or_array(QImageIO *, const char * const *, QImage &);
#endif // QT_NO_IMAGEIO_XPM

/*!
    Constructs a pixmap from \a xpm, which must be a valid XPM image.

    Errors are silently ignored.

    Note that it's possible to squeeze the XPM variable a little bit
    by using an unusual declaration:

    \code
        static const char * const start_xpm[]={
            "16 15 8 1",
            "a c #cec6bd",
        ....
    \endcode

    The extra \c const makes the entire definition read-only, which is
    slightly more efficient (for example, when the code is in a shared
    library) and ROMable when the application is to be stored in ROM.

    In order to use that sort of declaration you must cast the
    variable back to \c{const char **} when you create the QPixmap.
*/

QPixmap::QPixmap(const char * const xpm[])
    : QPaintDevice(QInternal::Pixmap)
{
    init(0, 0, 0, false, defOptim);
    QImage image;
#ifndef QT_NO_IMAGEIO_XPM
    qt_read_xpm_image_or_array(0, xpm, image);
#else
    // We use a qFatal rather than disabling the whole function, as this
    // constructor may be ambiguous.
    qFatal("XPM not supported");
#endif
    if (!image.isNull())
        fromImage(image);
}



/*!
    Destroys the pixmap.
*/

QPixmap::~QPixmap()
{
    deref();
}

/*!
    Returns a \link shclass.html deep copy\endlink of the pixmap.

    \sa operator=()
*/

QPixmap QPixmap::copy(bool) const
{
#if defined(Q_WS_X11)
    int old = x11SetDefaultScreen(data->xinfo.screen());
#endif // Q_WS_X11

    QPixmap pm(data->w, data->h, data->d, data->bitmap, data->optim);

    QPainter painter(&pm);
    painter.drawPixmap(QPoint(0, 0), *this, Qt::CopyPixmap);
    painter.end();
#if defined(Q_WS_QWS)
    pm.data->hasAlpha = data->hasAlpha;
#endif
#if defined(Q_WS_X11)
    x11SetDefaultScreen(old);
#endif // Q_WS_X11

    return pm;
}


/*!
    Assigns the pixmap \a pixmap to this pixmap and returns a
    reference to this pixmap.
*/

QPixmap &QPixmap::operator=(const QPixmap &pixmap)
{
    if (paintingActive()) {
        qWarning("QPixmap::operator=: Cannot assign to pixmap during painting");
        return *this;
    }
    pixmap.data->ref();                                // avoid 'x = x'
    deref();
    if (pixmap.paintingActive()) {                // make a deep copy
        init(pixmap.width(), pixmap.height(), pixmap.depth(),
              pixmap.data->bitmap, pixmap.data->optim);
        data->uninit = false;
        if (!isNull()) {
            QPainter p(this);
            p.drawPixmap(0, 0, pixmap);
        }
        pixmap.data->deref();
    } else {
        data = pixmap.data;
        devFlags = pixmap.devFlags;                // copy QPaintDevice flags
    }
    return *this;
}


/*!
    \overload

    Converts the image \a image to a pixmap that is assigned to this
    pixmap. Returns a reference to the pixmap.

    \sa convertFromImage().
*/

QPixmap &QPixmap::operator=(const QImage &image)
{
    fromImage(image);
    return *this;
}

/*!
    \fn bool QPixmap::operator!() const

    Returns true if this is a null pixmap; otherwise returns false.

    \sa isNull()
*/

/*!
    \fn QPixmap::operator QImage() const

    \internal

    Returns the pixmap as a QImage.
*/

/*!
    Returns the actual matrix used for transforming a pixmap with \a w
    width and \a h height and matrix \a m.

    When transforming a pixmap with xForm(), the transformation matrix
    is internally adjusted to compensate for unwanted translation,
    i.e. xForm() returns the smallest pixmap containing all
    transformed points of the original pixmap.

    This function returns the modified matrix, which maps points
    correctly from the original pixmap into the new pixmap.

    \sa xForm(), QMatrix
*/
#ifndef QT_NO_PIXMAP_TRANSFORMATION
QMatrix QPixmap::trueMatrix(const QMatrix &m, int w, int h)
{
    return QImage::trueMatrix(m, w, h);
}
#endif

/*!
    \fn bool QPixmap::isQBitmap() const

    Returns true if this is a QBitmap; otherwise returns false.
*/
bool QPixmap::isQBitmap() const
{
    return data->bitmap;
}

/*!
    \fn bool QPixmap::isNull() const

    Returns true if this is a null pixmap; otherwise returns false.

    A null pixmap has zero width, zero height and no contents. You
    cannot draw in a null pixmap.

    Resizing an existing pixmap to (0, 0) makes a pixmap into a null
    pixmap.

    \sa resize()
*/
bool QPixmap::isNull() const
{
    return data->w == 0;
}

/*!
    \fn int QPixmap::width() const

    Returns the width of the pixmap.

    \sa height(), size(), rect()
*/
int QPixmap::width() const
{
    return data->w;
}

/*!
    \fn int QPixmap::height() const

    Returns the height of the pixmap.

    \sa width(), size(), rect()
*/
int QPixmap::height() const
{
    return data->h;
}

/*!
    \fn QSize QPixmap::size() const

    Returns the size of the pixmap.

    \sa width(), height(), rect()
*/
QSize QPixmap::size() const
{
    return QSize(data->w,data->h);
}

/*!
    \fn QRect QPixmap::rect() const

    Returns the enclosing rectangle (0,0,width(),height()) of the pixmap.

    \sa width(), height(), size()
*/
QRect QPixmap::rect() const
{
    return QRect(0,0,data->w,data->h);
}

/*!
    \fn int QPixmap::depth() const

    Returns the depth of the pixmap.

    The pixmap depth is also called bits per pixel (bpp) or bit planes
    of a pixmap. A null pixmap has depth 0.

    \sa defaultDepth(), isNull(), QImage::convertDepth()
*/
int QPixmap::depth() const
{
    return data->d;
}

/*!
    \fn void QPixmap::resize(const QSize &size)
    \overload

    Resizes the pixmap to size \a size.
*/
void QPixmap::resize(const QSize &s)
{
    resize(s.width(), s.height());
}

/*!
    Resizes the pixmap to \a w width and \a h height. If either \a w
    or \a h is 0, the pixmap becomes a null pixmap.

    If both \a w and \a h are greater than 0, a valid pixmap is
    created. New pixels will be uninitialized (random) if the pixmap
    is expanded.
*/

void QPixmap::resize(int w, int h)
{
    if (w < 1 || h < 1) {                        // becomes null
        QPixmap pm(0, 0, 0, data->bitmap, data->optim);
        *this = pm;
        return;
    }
    int d;
    if (depth() > 0)
        d = depth();
    else
        d = isQBitmap() ? 1 : -1;
    // Create new pixmap
    QPixmap pm(w, h, d, data->bitmap, data->optim);
#ifdef Q_WS_X11
    pm.x11SetScreen(data->xinfo.screen());
#endif // Q_WS_X11
    if (!data->uninit && !isNull()) {                // has existing pixmap
        // Copy old pixmap
        QPainter p(&pm);
        p.drawPixmap(0, 0, *this, 0, 0, qMin(width(), w), qMin(height(), h));
    }
#if defined(Q_WS_MAC)
    if(data->alphapm) {
        data->alphapm->resize(w, h);
    } else
#elif defined(Q_WS_X11) && !defined(QT_NO_XFT)
    if (data->alphapm)
        qWarning("QPixmap::resize: TODO: resize alpha data");
    else
#endif // Q_WS_X11
        if (data->mask) {                                // resize mask as well
            if (data->selfmask) {                        // preserve self-mask
                pm.setMask(*((QBitmap*)&pm));
            } else {                                // independent mask
                QBitmap m = *data->mask;
                m.resize(w, h);
                pm.setMask(m);
            }
        }
    *this = pm;
}


/*!
    \fn const QBitmap *QPixmap::mask() const

    Returns the mask bitmap, or 0 if no mask has been set.

    \sa setMask(), QBitmap, hasAlpha()
*/
const QBitmap *QPixmap::mask() const
{
    return data->mask;
}


/*!
    Sets a mask bitmap.

    The \a newmask bitmap defines the clip mask for this pixmap. Every
    pixel in \a newmask corresponds to a pixel in this pixmap. Pixel
    value 1 means opaque and pixel value 0 means transparent. The mask
    must have the same size as this pixmap.

    \warning Setting the mask on a pixmap will cause any alpha channel
    data to be cleared. For example:
    \code
        QPixmap alpha("image-with-alpha.png");
        QPixmap alphacopy = alpha;
        alphacopy.setMask(*alphacopy.mask());
    \endcode
    Now, alpha and alphacopy are visually different.

    Setting a \link isNull() null\endlink mask resets the mask.

    \sa mask(), createHeuristicMask(), QBitmap
*/

void QPixmap::setMask(const QBitmap &newmask)
{
    const QPixmap *tmp = &newmask;                // dec cxx bug
    if (data == tmp->data) {
        QPixmap m = tmp->copy(true);
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
#if defined(Q_WS_MAC) || (defined(Q_WS_X11) && !defined(QT_NO_XFT))
    // when setting the mask, we get rid of the alpha channel completely
    delete data->alphapm;
    data->alphapm = 0;
#endif

    delete data->mask;
    QBitmap* newmaskcopy;
    if (newmask.mask())
        newmaskcopy = (QBitmap*)new QPixmap(tmp->copy(true));
    else
        newmaskcopy = new QBitmap(newmask);
#ifdef Q_WS_X11
    newmaskcopy->x11SetScreen(data->xinfo.screen());
#endif
    data->mask = newmaskcopy;
}


/*!
    \fn bool QPixmap::selfMask() const

    Returns true if the pixmap's mask is identical to the pixmap
    itself; otherwise returns false.

    \sa mask()
*/
bool QPixmap::selfMask() const
{
    return data->selfmask;
}

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
/*!
    Creates and returns a heuristic mask for this pixmap. It works by
    selecting a color from one of the corners and then chipping away
    pixels of that color, starting at all the edges.

    The mask may not be perfect but it should be reasonable, so you
    can do things such as the following:
    \code
    pm->setMask(pm->createHeuristicMask());
    \endcode

    This function is slow because it involves transformation to a
    QImage, non-trivial computations and a transformation back to a
    QBitmap.

    If \a clipTight is true the mask is just large enough to cover the
    pixels; otherwise, the mask is larger than the data pixels.

    \sa QImage::createHeuristicMask()
*/

QBitmap QPixmap::createHeuristicMask(bool clipTight) const
{
    QBitmap m;
    m.fromImage(toImage().createHeuristicMask(clipTight));
    return m;
}
#endif
#ifndef QT_NO_IMAGEIO
/*!
    Loads a pixmap from the file \a fileName at runtime. Returns true
    if successful; otherwise returns false.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    See the convertFromImage() documentation for a description of the
    \a flags argument.

    The QImageIO documentation lists the supported image formats and
    explains how to add extra formats.

    \sa loadFromData(), save(), imageFormat(), QImage::load(),
    QImageIO
*/

bool QPixmap::load(const QString &fileName, const char *format, Qt::ImageConversionFlags flags)
{
#ifndef QT_NO_PIXMAP_RETAIN
    const QString retain_key = QString("load_filename_") + fileName;
    if(QRetainedPixmap *retain = qt_retained_pixmaps()->object(retain_key)) {
        if(retain->isOutOfDate()) {
            qt_retained_pixmaps()->remove(retain_key);
            delete retain;
        } else {
            *this = *retain;
            return true;
        }
    }
#endif
    QImageIO io(fileName, format);
    bool result = io.read();
    if (result) {
        result = fromImage(io.image(), flags);
#ifndef QT_NO_PIXMAP_RETAIN
        if(result) {
            QRetainedPixmap *keeper = 0;
            if(fileName[0] != QLatin1Char(':'))
                keeper = new QRetainedPixmapFile(fileName, *this);
            if(!keeper)
                keeper = new QRetainedPixmap(*this);
            int cost = width()*height()*depth()/8;
            if(data->optim == LoadOptim)
                cost /= 4;
            qt_retained_pixmaps()->insert(retain_key, keeper, cost);
        }
#endif
    }
    return result;
}

/*!
    Loads a pixmap from the binary data in \a buf (\a len bytes).
    Returns true if successful; otherwise returns false.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    See the convertFromImage() documentation for a description of the
    \a flags argument.

    The QImageIO documentation lists the supported image formats and
    explains how to add extra formats.

    \sa load(), save(), imageFormat(), QImage::loadFromData(),
    QImageIO
*/

bool QPixmap::loadFromData(const uchar *buf, uint len, const char *format, Qt::ImageConversionFlags flags)
{
    QByteArray a = QByteArray::fromRawData(reinterpret_cast<const char *>(buf), len);
    QBuffer b(&a);
    b.open(QIODevice::ReadOnly);
    QImageIO io(&b, format);
    bool result = io.read();
    b.close();
    if (result)
        result = fromImage(io.image(), flags);
    return result;
}

/*!
    \overload
*/

bool QPixmap::loadFromData(const QByteArray &buf, const char *format,
                           Qt::ImageConversionFlags flags)
{
    return loadFromData((const uchar *)buf.constData(), buf.size(), format, flags);
}


/*!
    Saves the pixmap to the file \a fileName using the image file
    format \a format and a quality factor \a quality. \a quality must
    be in the range [0,100] or -1. Specify 0 to obtain small
    compressed files, 100 for large uncompressed files, and -1 to use
    the default settings. Returns true if successful; otherwise
    returns false.

    \sa load(), loadFromData(), imageFormat(), QImage::save(),
    QImageIO
*/

bool QPixmap::save(const QString &fileName, const char *format, int quality) const
{
    if (isNull())
        return false;                                // nothing to save
    QImageIO io(fileName, format);
    return doImageIO(&io, quality);
}

/*!
    \overload

    This function writes a QPixmap to the QIODevice, \a device. This
    can be used, for example, to save a pixmap directly into a
    QByteArray:
    \code
    QPixmap pixmap;
    QByteArray ba;
    QBuffer buffer(ba);
    buffer.open(QIODevice::WriteOnly);
    pixmap.save(&buffer, "PNG"); // writes pixmap into ba in PNG format
    \endcode
*/

bool QPixmap::save(QIODevice* device, const char* format, int quality) const
{
    if (isNull())
        return false;                                // nothing to save
    QImageIO io(device, format);
    return doImageIO(&io, quality);
}

/*! \internal
*/

bool QPixmap::doImageIO(QImageIO* io, int quality) const
{
    if (!io)
        return false;
    io->setImage(toImage());
    if (quality > 100  || quality < -1)
        qWarning("QPixmap::save: quality out of range [-1,100]");
    if (quality >= 0)
        io->setQuality(qMin(quality,100));
    return io->write();
}

#endif //QT_NO_IMAGEIO


// The implementation of QPixmap::fill(const QWidget *, const QPoint &)
// is in qwidget.cpp
/*!
 \fn void QPixmap::fill(const QWidget *widget, int xoff, int yoff)

 \overload

    Fills the pixmap with the \a widget's background color or pixmap.
    \a xoff, \a yoff is an offset in the widget.
*/


/*!
    \fn int QPixmap::serialNumber() const

    Returns a number that uniquely identifies the contents of this
    QPixmap object. This means that multiple QPixmap objects can have
    the same serial number as long as they refer to the same contents.

    An example of where this is useful is for caching QPixmaps.

    \sa QPixmapCache
*/
int QPixmap::serialNumber() const
{
    return data->ser_no;
}


/*!
    \fn QPixmap::Optimization QPixmap::optimization() const

    Returns the optimization setting for this pixmap.

    The default optimization setting is \c QPixmap::NormalOptim. You
    can change this setting in two ways:
    \list
    \i Call setDefaultOptimization() to set the default optimization
    for all new pixmaps.
    \i Call setOptimization() to set the optimization for individual
    pixmaps.
    \endlist

    \sa setOptimization(), setDefaultOptimization(), defaultOptimization()
*/

QPixmap::Optimization QPixmap::optimization() const
{
    return data->optim;
}


/*!
    Returns the default pixmap optimization setting.

    \sa setDefaultOptimization(), setOptimization(), optimization()
*/

QPixmap::Optimization QPixmap::defaultOptimization()
{
    return defOptim;
}

/*!
    Sets the default pixmap optimization.

    All \e new pixmaps that are created will use this default
    optimization. You may also set optimization for individual pixmaps
    using the setOptimization() function.

    The initial default \a optimization setting is \c QPixmap::Normal.

    \sa defaultOptimization(), setOptimization(), optimization()
*/

void QPixmap::setDefaultOptimization(Optimization optimization)
{
    if (optimization != DefaultOptim)
        defOptim = optimization;
}

/*
  fills \a buf with \a r in \a widget. Then blits \a buf on \a res at
  position \a offset
 */
static void grabWidget_helper(QWidget *widget, QPixmap &res, QPixmap &buf,
                              const QRect &r, const QPoint &offset)
{
    buf.fill(widget, r.topLeft());
    QPainter::setRedirected(widget, &buf, r.topLeft());
    QPaintEvent e(r & widget->rect());
    QApplication::sendEvent(widget, &e);
    QPainter::restoreRedirected(widget);
    {
        QPainter pt(&res);
        pt.drawPixmap(offset.x(), offset.y(), buf, 0, 0, r.width(), r.height());
    }

    const QObjectList children = widget->children();
    for (int i = 0; i < children.size(); ++i) {
        QWidget *child = static_cast<QWidget*>(children.at(i));
        if (!child->isWidgetType() || child->isTopLevel()
            || child->isHidden() || !child->geometry().intersects(r))
            continue;
        QRect cr = r & child->geometry();
        cr.translate(-child->pos());
        grabWidget_helper(child, res, buf, cr, offset + child->pos());
    }
}

/*!
    Creates a pixmap and paints \a widget in it.

    If the \a widget has any children, then they are also painted in
    the appropriate positions.

    If you specify \a x, \a y, \a w or \a h, only the rectangle you
    specify is painted. The defaults are 0, 0 (top-left corner) and
    -1,-1 (which means the entire widget).

    (If \a w is negative, the function copies everything to the right
    border of the window. If \a h is negative, the function copies
    everything to the bottom of the window.)

    If \a widget is 0, or if the rectangle defined by \a x, \a y, the
    modified \a w and the modified \a h does not overlap the \a
    {widget}->rect(), this function will return a null QPixmap.

    This function actually asks \a widget to paint itself (and its
    children to paint themselves). QPixmap::grabWindow() grabs pixels
    off the screen, which is a bit faster and picks up \e exactly
    what's on-screen. This function works by calling paintEvent() with
    painter redirection turned on. If there are overlaying windows,
    grabWindow() will see them, but not this function.

    If there is overlap, it returns a pixmap of the size you want,
    containing a rendering of \a widget. If the rectangle you ask for
    is a superset of \a widget, the areas outside \a widget are
    covered with the widget's background.

    If an error occurs when trying to grab the widget, such as the
    size of the widget being too large to fit in memory, an isNull()
    pixmap is returned.

    \sa grabWindow() QWidget::paintEvent()
*/

QPixmap QPixmap::grabWidget(QWidget * widget, int x, int y, int w, int h)
{
    QPixmap res, buf;

    if (!widget)
        return res;

    if (w < 0)
        w = widget->width() - x;
    if (h < 0)
        h = widget->height() - y;

    QRect r(x, y, w, h);
    if (!r.intersects(widget->rect()))
        return res;

    res.resize(r.size());
    buf.resize(r.size());
    if(!res || !buf)
        return res;

    grabWidget_helper(widget, res, buf, r, QPoint());
    return res;
}


#ifndef Q_WS_WIN
/*!
    Returns the pixmap's handle to the device context.

    \warning This function is Windows-specific; using it is
    non-portable.
*/

Qt::HANDLE QPixmap::handle() const
{
    return data->hd;
}
#endif


#ifdef QT_COMPAT
#ifndef QT_NO_IMAGEIO
static Qt::ImageConversionFlags colorModeToFlags(QPixmap::ColorMode mode)
{
    Qt::ImageConversionFlags flags = Qt::AutoColor;
    switch (mode) {
      case QPixmap::Color:
        flags |= Qt::ColorOnly;
        break;
      case QPixmap::Mono:
        flags |= Qt::MonoOnly;
        break;
      default:
        break;// Nothing.
    }
    return flags;
}

/*!
    Constructs a pixmap from the file \a fileName. If the file does
    not exist or is of an unknown format, the pixmap becomes a null
    pixmap.

    The \a fileName, \a format and \a mode parameters are passed on to
    load(). This means that the data in \a fileName is not compiled
    into the binary. If \a fileName contains a relative path (e.g. the
    filename only) the relevant file must be found relative to the
    runtime working directory.

    \sa QPixmap::ColorMode isNull(), load(), loadFromData(), save(), imageFormat()
*/

QPixmap::QPixmap(const QString& fileName, const char *format, ColorMode mode)
    : QPaintDevice(QInternal::Pixmap)
{
    init(0, 0, 0, false, defOptim);
    load(fileName, format, colorModeToFlags(mode));
}

/*!
    \overload

    Loads a pixmap from the file \a fileName at runtime.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    The \a mode is used to specify the color mode of the pixmap.

    \sa QPixmap::ColorMode
*/

bool QPixmap::load(const QString &fileName, const char *format, ColorMode mode)
{
    return load(fileName, format, colorModeToFlags(mode));
}

/*!
    \overload

    Loads a pixmap from the binary data in \a buf (\a len bytes) using
    color mode \a mode. Returns true if successful; otherwise returns
    false.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    \sa QPixmap::ColorMode
*/

bool QPixmap::loadFromData(const uchar *buf, uint len, const char *format, ColorMode mode)
{
    return loadFromData(buf, len, format, colorModeToFlags(mode));
}
#endif

/*!
    \fn bool QPixmap::convertFromImage(const QImage &image, ColorMode mode)
    \overload
    \compat

    Creates a pixmap from the given \a image with the color mode specified
    by \a mode.
    Returns true if successful; otherwise returns false.

    \sa QPixmap::ColorMode
*/

bool QPixmap::convertFromImage(const QImage &image, ColorMode mode)
{
    return fromImage(image, colorModeToFlags(mode));
}

#endif

/*****************************************************************************
  QPixmap stream functions
 *****************************************************************************/
#if !defined(QT_NO_DATASTREAM) && !defined(QT_NO_IMAGEIO)
/*!
    \relates QPixmap

    Writes the pixmap \a pixmap to the stream \a s as a PNG image.

    Note that writing the stream to a file will not produce a valid image file.

    \sa QPixmap::save()
    \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator<<(QDataStream &s, const QPixmap &pixmap)
{
    s << pixmap.toImage();
    return s;
}

/*!
    \relates QPixmap

    Reads a pixmap from the stream \a s into the pixmap \a pixmap.

    \sa QPixmap::load()
    \link datastreamformat.html Format of the QDataStream operators \endlink
*/

QDataStream &operator>>(QDataStream &s, QPixmap &pixmap)
{
    QImage img;
    s >> img;
    pixmap.fromImage(img);
    return s;
}

#endif //QT_NO_DATASTREAM

#ifdef QT_COMPAT
Q_GUI_EXPORT void copyBlt(QPixmap *dst, int dx, int dy,
                          const QPixmap *src, int sx, int sy, int sw, int sh)
{
    Q_ASSERT_X(dst, "::copyBlt", "Destination pixmap must be non null");
    Q_ASSERT_X(src, "::copyBlt", "Source pixmap must be non null");

    QPainter p(dst);
    p.drawPixmap(dx, dy, *src, sx, sy, sw, sh, Qt::CopyPixmap);
}
#endif

/*!
    \internal
*/

bool QPixmap::isDetached() const
{
    return data->count == 1;
}

void QPixmap::deref()
{
    if(data && data->deref()) { // Destroy image if last ref
        delete data;
        data = 0;
    }
}

/*!
    \fn QImage QPixmap::convertToImage() const
    \compat

    Converts the pixmap to a device-independent image and returns the result
    as a new image.
*/

/*!
    \fn bool QPixmap::convertFromImage(const QImage &image, Qt::ImageConversionFlags flags)
    \compat

    Creates a pixmap from the given \a image using the conversion flags
    specified by \a flags.
*/

/*!
    \fn QPixmap QPixmap::xForm(const QMatrix &matrix) const
    \compat

    Transforms the pixmap using the given \a matrix and returns the result
    as a new pixmap.
*/



/*!
    \fn QPixmap QPixmap::scale(int w, int h, Qt::AspectRatioMode aspectRatioMode,
                             Qt::TransformationMode transformMode) const

    Returns a copy of the pixmap scaled to a rectangle of width \a w
    and height \a h according to \a aspectRatioMode and \a transformMode.

    \list
    \i If \a aspectRatioMode is \c Qt::IgnoreAspectRatio, the pixmap
       is scaled to (\a w, \a h).
    \i If \a aspectRatioMode is \c Qt::KeepAspectRatio, the pixmap is
       scaled to a rectangle as large as possible inside (\a w, \a
       h), preserving the aspect ratio.
    \i If \a aspectRatioMode is \c Qt::KeepAspectRatioByExpanding,
       the pixmap is scaled to a rectangle as small as possible
       outside (\a w, \a h), preserving the aspect ratio.
    \endlist

    If either the width \a w or the height \a h is zero or negative,
    this function returns a \l{isNull()}{null} pixmap.

    \sa scaleWidth() scaleHeight() xForm()
*/

/*!
    \fn QPixmap QPixmap::scale(const QSize &size, Qt::AspectRatioMode aspectMode, Qt::TransformationMode transformMode) const

    \overload

    Scales the pixmap to the given \a size, using the aspect ratio and
    transformation modes specified by \a aspectMode and \a transformMode.
*/
QPixmap QPixmap::scale(const QSize& s, Qt::AspectRatioMode aspectMode, Qt::TransformationMode mode) const
{
    if (isNull()) {
        qWarning("QPixmap::scale: Pixmap is a null pixmap");
        return copy();
    }
    if (s.isEmpty())
        return QPixmap();

    QSize newSize = size();
    newSize.scale(s, aspectMode);
    if (newSize == size())
        return copy();

    QPixmap pix;
    QMatrix wm;
    wm.scale((double)newSize.width() / width(), (double)newSize.height() / height());
    pix = transform(wm, mode);
    return pix;
}

/*!
    Returns a scaled copy of the pixmap. The returned pixmap has a width
    of \a w pixels. This function automatically calculates the height
    of the pixmap so that the ratio of the pixmap is preserved.

    If \a w is 0 or negative a \link isNull() null\endlink pixmap is
    returned.

    \sa scale() scaleHeight() xForm()
*/
QPixmap QPixmap::scaleWidth(int w) const
{
    if (isNull()) {
        qWarning("QPixmap::scaleWidth: Pixmap is a null pixmap");
        return copy();
    }
    if (w <= 0)
        return QPixmap();

    QMatrix wm;
    double factor = (double) w / width();
    wm.scale(factor, factor);
    return transform(wm);
}

/*!
    Returns a scaled copy of the pixmap. The returned pixmap has a
    height of \a h pixels. This function automatically calculates the
    width of the pixmap so that the ratio of the pixmap is preserved.

    If \a h is 0 or negative a \link isNull() null\endlink pixmap is
    returned.

    \sa scale() scaleWidth() xForm()
*/
QPixmap QPixmap::scaleHeight(int h) const
{
    if (isNull()) {
        qWarning("QPixmap::scaleHeight: Pixmap is a null pixmap");
        return copy();
    }
    if (h <= 0)
        return QPixmap();

    QMatrix wm;
    double factor = (double) h / height();
    wm.scale(factor, factor);
    return transform(wm);
}

