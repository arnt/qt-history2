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


#include <qglobal.h>

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
#include "qpixmapcache.h"
#include "qdatetime.h"
#include "qimagereader.h"
#include "qimagewriter.h"

#if defined(Q_WS_X11)
#include "qx11info_x11.h"
#include <private/qt_x11_p.h>
#endif

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
    Constructs a null pixmap.

    \sa isNull()
*/

QPixmap::QPixmap()
    : QPaintDevice()
{
    init(0, 0);
}

/*!
    Constructs a pixmap with \a w width, \a h height.

    The content of the pixmap is uninitialized.

    If either \a w or \a h is zero, a null pixmap is constructed.

    \sa isNull()
*/

QPixmap::QPixmap(int w, int h)
    : QPaintDevice()
{
    init(w, h);
}

/*!
    \overload

    Constructs a pixmap of size \a size.
*/

QPixmap::QPixmap(const QSize &size)
    : QPaintDevice()
{
    init(size.width(), size.height());
}

/*!
  \internal
*/
QPixmap::QPixmap(const QSize &s, Type type)
{
    init(s.width(), s.height(), type);
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

    The file name can either refer to an actual file on disk or to
    one of the application's embedded resources. See the
    \l{resources.html}{Resource System} overview for details on how
    to embed images and other resource files in the application's
    executable.

    If the image needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a
    flags to specify how you'd prefer this to happen.

    \sa Qt::ImageConversionFlags, isNull(), load(), loadFromData(), save(),
    QImageReader::imageFormat()
*/

QPixmap::QPixmap(const QString& fileName, const char *format, Qt::ImageConversionFlags flags)
    : QPaintDevice()
{
    init(0, 0);
    load(fileName, format, flags);
}
#endif //QT_NO_IMAGEIO

/*!
    Constructs a pixmap that is a copy of \a pixmap.
*/

QPixmap::QPixmap(const QPixmap &pixmap)
    : QPaintDevice()
{
    if (pixmap.paintingActive()) {                // make a deep copy
        data = 0;
        operator=(pixmap.copy());
    } else {
        data = pixmap.data;
        data->ref();
    }
}

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
*/

QPixmap::QPixmap(const char * const xpm[])
    : QPaintDevice()
{
    init(0, 0);

    QImage image(xpm);
    if (!image.isNull()) {
        if (data->type == BitmapType)
            *this =  QBitmap::fromImage(image);
        else
            *this = fromImage(image);
    }
}



/*!
    Destroys the pixmap.
*/

QPixmap::~QPixmap()
{
    deref();
}

/*!
  \internal
*/
int QPixmap::devType() const
{
    return QInternal::Pixmap;
}

/*!
    \fn QPixmap QPixmap::copy(int x, int y, int w, int h) const

    \overload

    Returns a \link shclass.html deep copy\endlink of the subpart of
    the pixmap that is specified by the rectangle \a x, \a y, \a w \a
    h.

    If the rectangle is empty, the whole image is copied.
*/

/*!
    \fn QPixmap QPixmap::copy(const QRect &rect) const

    Returns a \link shclass.html deep copy\endlink of the subpart of
    the pixmap that is specified by \a rect.

    If \a rect is empty, the whole image is copied.

    \sa operator=()
*/

#if defined(Q_WS_WIN) || defined(Q_WS_QWS)
QPixmap QPixmap::copy(const QRect &rect) const
{
    QPixmap pm;
    if (data->type == BitmapType)
        pm = QBitmap::fromImage(toImage().copy(rect));
    else
        pm = fromImage(toImage().copy(rect));
    return pm;
}
#endif

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
    if (pixmap.paintingActive()) {                // make a deep copy
        *this = pixmap.copy();
    } else {
        pixmap.data->ref();                                // avoid 'x = x'
        deref();
        data = pixmap.data;
    }
    return *this;
}

/*!
   Returns the pixmap as a QVariant
*/
QPixmap::operator QVariant() const
{
    return QVariant(QVariant::Pixmap, this);
}

/*!
    \fn bool QPixmap::operator!() const

    Returns true if this is a null pixmap; otherwise returns false.

    \sa isNull()
*/

/*!
    \fn QPixmap::operator QImage() const

    Returns the pixmap as a QImage.

    This automatic conversion is disabled when \c QT3_SUPPORT is not
    defined. Use toImage() instead.
*/

/*!
    Returns the actual matrix used for transforming a pixmap with \a w
    width and \a h height and matrix \a m.

    When transforming a pixmap with transform(), the transformation matrix
    is internally adjusted to compensate for unwanted translation,
    i.e. transform() returns the smallest pixmap containing all
    transformed points of the original pixmap.

    This function returns the modified matrix, which maps points
    correctly from the original pixmap into the new pixmap.

    \sa transformed(), QMatrix
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
#ifdef QT3_SUPPORT
void QPixmap::resize_helper(const QSize &s)
{
    int w = s.width();
    int h = s.height();
    if (w < 1 || h < 1) {
        *this = QPixmap();
        return;
    }

    int d = data->d;
    // Create new pixmap
    QPixmap pm(QSize(w, h), d == 1 ? BitmapType : PixmapType);
#ifdef Q_WS_X11
    pm.x11SetScreen(data->xinfo.screen());
#endif // Q_WS_X11
    if (!data->uninit && !isNull()) {                // has existing pixmap
        // Copy old pixmap
        QPainter p(&pm);
        p.drawPixmap(0, 0, *this, 0, 0, qMin(width(), w), qMin(height(), h));
    }
#if defined(Q_WS_MAC)
    if(data->qd_alpha) {
        data->macQDUpdateAlpha();
    } else
#endif // Q_WS_X11
#ifdef Q_WS_X11
        if (data->x11_mask) {
            pm.data->x11_mask = (Qt::HANDLE)XCreatePixmap(X11->display, RootWindow(data->xinfo.display(), data->xinfo.screen()),
                                                          w, h, 1);
            GC gc = XCreateGC(X11->display, pm.data->x11_mask, 0, 0);
            XCopyArea(X11->display, data->x11_mask, pm.data->x11_mask, gc, 0, 0, qMin(width(), w), qMin(height(), h), 0, 0);
            XFreeGC(X11->display, gc);
        }
#elif !defined(Q_WS_MAC)
    if (data->mask) {
        QBitmap m = *data->mask;
        m.resize(w, h);
        pm.setMask(m);
    }
#endif
    *this = pm;
}
#endif

/*!
  \fn void QPixmap::resize(int w, int h)

    Resizes the pixmap to \a w width and \a h height. If either \a w
    or \a h is 0, the pixmap becomes a null pixmap.

    If both \a w and \a h are greater than 0, a valid pixmap is
    created. New pixels will be uninitialized (random) if the pixmap
    is expanded.
*/

/*!
    \fn bool QPixmap::selfMask() const

    \obsolete
*/

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

    If \a clipTight is true (the default) the mask is just large enough to cover the
    pixels; otherwise, the mask is larger than the data pixels.

    \sa QImage::createHeuristicMask()
*/
QBitmap QPixmap::createHeuristicMask(bool clipTight) const
{
    QBitmap m = QBitmap::fromImage(toImage().createHeuristicMask(clipTight));
    return m;
}
#endif

/*!
    Creates and returns a mask for this pixmap based on \a maskColor.

    This function is slow because it involves transformation to a
    QImage and a transformation back to a QBitmap.

    \sa createHeuristicMask()
*/
QBitmap QPixmap::createMaskFromColor(const QColor &maskColor) const
{
    QImage maskImage(size(), QImage::Format_MonoLSB);
    QImage image = toImage();
    QRgb mColor = maskColor.rgba();
    for (int w = 0; w < width(); w++) {
        for (int h = 0; h < height(); h++) {
            if (image.pixel(w, h) == mColor)
                maskImage.setPixel(w, h, Qt::color1);
            else
                maskImage.setPixel(w, h, Qt::color0);
        }
    }
    QBitmap m = QBitmap::fromImage(maskImage);
    return m;
}

#ifndef QT_NO_IMAGEIO
/*!
    Loads a pixmap from the file \a fileName at runtime. Returns true
    if successful; otherwise returns false.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    If the data needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a
    flags to specify how you'd prefer this to happen.

    The QImageReader documentation lists the supported image formats and
    explains how to add extra formats.

    The file name can either refer to an actual file on disk or to
    one of the application's embedded resources. See the
    \l{resources.html}{Resource System} overview for details on how
    to embed images and other resource files in the application's
    executable.

    \sa loadFromData(), save(), QImageReader::imageFormat(), QImage::load(), fromImage()
    QImageReader
*/

bool QPixmap::load(const QString &fileName, const char *format, Qt::ImageConversionFlags flags)
{
    if (fileName.isEmpty())
        return false;

    QFileInfo info(fileName);
    QString key = QLatin1String("qt_pixmap_") + info.absoluteFilePath() + QLatin1Char('_') + info.lastModified().toString()
                  + QString::number(data->type);

    if (QPixmapCache::find(key, *this))
            return true;
    QImage image = QImageReader(fileName, format).read();
    if (image.isNull())
        return false;
    QPixmap pm;
    if (data->type == BitmapType)
        pm = QBitmap::fromImage(image, flags);
    else
        pm = fromImage(image, flags);
    if (!pm.isNull()) {
        *this = pm;
        QPixmapCache::insert(key, *this);
        return true;
    }
    return false;
}

/*!
    Loads a pixmap from the binary data in \a buf (\a len bytes).
    Returns true if successful; otherwise returns false.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    If the data needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a
    flags to specify how you'd prefer this to happen.

    The QImageReader documentation lists the supported image formats and
    explains how to add extra formats.

    \sa load(), save(), QImageReader::imageFormat(), QImage::loadFromData(), fromImage(),
    QImageReader
*/

bool QPixmap::loadFromData(const uchar *buf, uint len, const char *format, Qt::ImageConversionFlags flags)
{
    QByteArray a = QByteArray::fromRawData(reinterpret_cast<const char *>(buf), len);
    QBuffer b(&a);
    b.open(QIODevice::ReadOnly);

    QImage image = QImageReader(&b, format).read();
    QPixmap pm;
    if (data->type == BitmapType)
        pm = QBitmap::fromImage(image, flags);
    else
        pm = fromImage(image, flags);
    if (!pm.isNull()) {
        *this = pm;
        return true;
    }
    return false;
}

/*!
  \fn bool QPixmap::loadFromData(const QByteArray &buf, const char *format, Qt::ImageConversionFlags flags)
    \overload

    Loads a pixmap from the binary data \a buf.

    If \a format is specified, the loader attempts to read the pixmap
    using the specified format. If \a format is not specified
    (default), the loader reads a few bytes from the header to guess
    the file's format.

    If the data needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a
    flags to specify how you'd prefer this to happen.
*/


/*!
    Saves the pixmap to the file \a fileName using the image file
    format \a format and a quality factor \a quality. \a quality must
    be in the range [0,100] or -1. Specify 0 to obtain small
    compressed files, 100 for large uncompressed files, and -1 to use
    the default settings. Returns true if successful; otherwise
    returns false.

    \sa load(), loadFromData(), QImageReader::imageFormat(), QImage::save(),
    QImageReader
*/

bool QPixmap::save(const QString &fileName, const char *format, int quality) const
{
    if (isNull())
        return false;                                // nothing to save
    QImageWriter writer(fileName, format);
    return doImageIO(&writer, quality);
}

/*!
    \overload

    This function writes a QPixmap to the QIODevice, \a device using the image
    file format \a format. This can be used, for example, to save a pixmap
    directly into a QByteArray:
    \quotefromfile snippets/image/image.cpp
    \skipto PIX SAVE
    \skipto QPixmap
    \printuntil save

    The \a quality parameter is a quality factor \a quality. \a quality must
    be in the range [0,100] or -1. Specify 0 to obtain small
    compressed files, 100 for large uncompressed files, and -1 to use
    the default settings.
*/

bool QPixmap::save(QIODevice* device, const char* format, int quality) const
{
    if (isNull())
        return false;                                // nothing to save
    QImageWriter writer(device, format);
    return doImageIO(&writer, quality);
}

/*! \internal
*/

bool QPixmap::doImageIO(QImageWriter *writer, int quality) const
{
    if (quality > 100  || quality < -1)
        qWarning("QPixmap::save: quality out of range [-1,100]");
    if (quality >= 0)
        writer->setQuality(qMin(quality,100));
    return writer->write(toImage());
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

    A null pixmap always have a serial number of 0.

    An example of where this is useful is for caching QPixmaps.

    \sa QPixmapCache
*/
int QPixmap::serialNumber() const
{
    if (isNull())
        return 0;
    else
        return data->ser_no;
}

/*
  Fills \a buf with \a r in \a widget. Then blits \a buf on \a res at
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
        if (!child->isWidgetType() || child->isWindow()
            || child->isHidden() || !child->geometry().intersects(r))
            continue;
        QRect cr = r & child->geometry();
        cr.translate(-child->pos());
        grabWidget_helper(child, res, buf, cr, offset + child->pos());
    }
}

/*!
    \overload

    Creates a pixmap and paints \a widget in it. If the \a widget
    has any children, then they are also painted in the appropriate
    positions.

    If \a rect is a valid rectangle, only the rectangle you specify
    is painted.

    grabWidget(), QRect::isValid()
*/

QPixmap QPixmap::grabWidget(QWidget * widget, const QRect &rect)
{

    if (!widget)
        return QPixmap();

    QRect r(rect);
    if (r.width() < 0)
        r.setWidth(widget->width() - rect.x());
    if (r.height() < 0)
        r.setHeight(widget->height() - rect.y());

    if (!r.intersects(widget->rect()))
        return QPixmap();

    QPixmap res(r.size());
    QPixmap buf(r.size());
    if(res.isNull() || buf.isNull())
        return res;

    grabWidget_helper(widget, res, buf, r, QPoint());
    return res;
}

/*!
    \fn QPixmap QPixmap::grabWidget(QWidget *widget, int x, int y, int w, int h)

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


#if !defined(Q_WS_WIN) && !defined(Q_WS_MAC)
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


#ifdef QT3_SUPPORT
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
    Use the constructor that takes a Qt::ImageConversionFlag instead.

    \sa ColorMode, Qt::ImageConversionFlag
*/

QPixmap::QPixmap(const QString& fileName, const char *format, ColorMode mode)
    : QPaintDevice()
{
    init(0, 0);
    load(fileName, format, colorModeToFlags(mode));
}

/*!
    Constructs a pixmap from the QImage \a image.

    \sa fromImage()
*/

QPixmap::QPixmap(const QImage& image)
    : QPaintDevice()
{
    init(0, 0);
    if (data->type == BitmapType)
        *this = QBitmap::fromImage(image);
    else
        *this = fromImage(image);
}

/*!
    \overload

    Converts the image \a image to a pixmap that is assigned to this
    pixmap. Returns a reference to the pixmap.

    \sa fromImage()
*/

QPixmap &QPixmap::operator=(const QImage &image)
{
    if (data->type == BitmapType)
        *this = QBitmap::fromImage(image);
    else
        *this = fromImage(image);
    return *this;
}

/*!
    Use the load() function that takes a Qt::ImageConversionFlag instead.

    \sa ColorMode, Qt::ImageConversionFlag
*/

bool QPixmap::load(const QString &fileName, const char *format, ColorMode mode)
{
    return load(fileName, format, colorModeToFlags(mode));
}

/*!
    Use the loadFromData() function that takes a Qt::ImageConversionFlag instead.

    \sa ColorMode, Qt::ImageConversionFlag
*/

bool QPixmap::loadFromData(const uchar *buf, uint len, const char *format, ColorMode mode)
{
    return loadFromData(buf, len, format, colorModeToFlags(mode));
}
#endif

/*!
    Use the static function QPixmap::fromImage() instead.
*/
bool QPixmap::convertFromImage(const QImage &image, ColorMode mode)
{
    if (data->type == BitmapType)
        *this = QBitmap::fromImage(image, colorModeToFlags(mode));
    else
        *this = fromImage(image, colorModeToFlags(mode));
    return !isNull();
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
    if (pixmap.data->type == QPixmap::BitmapType)
        pixmap = QBitmap::fromImage(img);
    else
        pixmap = QPixmap::fromImage(img);
    return s;
}

#endif //QT_NO_DATASTREAM

#ifdef QT3_SUPPORT
Q_GUI_EXPORT void copyBlt(QPixmap *dst, int dx, int dy,
                          const QPixmap *src, int sx, int sy, int sw, int sh)
{
    Q_ASSERT_X(dst, "::copyBlt", "Destination pixmap must be non null");
    Q_ASSERT_X(src, "::copyBlt", "Source pixmap must be non null");

    QImage image = dst->toImage();
    QPainter p(&image);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.drawPixmap(dx, dy, *src, sx, sy, sw, sh);
    *dst = QPixmap::fromImage(image);
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

    Use toImage() instead.
*/

/*!
    \fn bool QPixmap::convertFromImage(const QImage &image, Qt::ImageConversionFlags flags)

    Use the static function QPixmap::fromImage() instead.
*/

/*!
    \fn QPixmap QPixmap::xForm(const QMatrix &matrix) const

    Use transform() instead.
*/

/*!
    \fn QPixmap QPixmap::scaled(int w, int h, Qt::AspectRatioMode aspectRatioMode, Qt::TransformationMode transformMode) const

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

    \sa scaledToWidth(), scaledToHeight(), transformed()
*/

/*!
    \fn QPixmap QPixmap::scaled(const QSize &size, Qt::AspectRatioMode aspectMode, Qt::TransformationMode transformMode) const

    \overload

    Scales the pixmap to the given \a size, using the aspect ratio and
    transformation modes specified by \a aspectMode and \a transformMode.
*/
QPixmap QPixmap::scaled(const QSize& s, Qt::AspectRatioMode aspectMode, Qt::TransformationMode mode) const
{
    if (isNull()) {
        qWarning("QPixmap::scaled: Pixmap is a null pixmap");
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
    pix = transformed(wm, mode);
    return pix;
}

/*!
    Returns a scaled copy of the pixmap using a transformation specified
    by \a mode. The returned pixmap has a width of \a w pixels. This
    function automatically calculates the height of the pixmap so that the
    ratio of the pixmap is preserved.

    If \a w is 0 or negative a \link isNull() null\endlink pixmap is
    returned.

    \sa scaled(), scaledToHeight(), transformed()
*/
QPixmap QPixmap::scaledToWidth(int w, Qt::TransformationMode mode) const
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
    return transformed(wm, mode);
}

/*!
    Returns a scaled copy of the pixmap using a transformation specified
    by \a mode. The returned pixmap has a height of \a h pixels. This
    function automatically calculates the width of the pixmap so that the
    ratio of the pixmap is preserved.

    If \a h is 0 or negative a \link isNull() null\endlink pixmap is
    returned.

    \sa scaled(), scaledToWidth(), transformed()
*/
QPixmap QPixmap::scaledToHeight(int h, Qt::TransformationMode mode) const
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
    return transformed(wm, mode);
}

/*!
    \class QPixmap
    \brief The QPixmap class is an off-screen, pixel-based paint device.

    \ingroup multimedia
    \ingroup shared
    \mainclass

    QPixmap is one of the three classes Qt provides for dealing with
    images, the others being QImage and QPicture. QPixmap is designed
    and optimized for drawing on screen; QImage is designed for I/O
    and for direct pixel access; QPicture provides a scalable,
    vectorial picture. There are static functions in QPixmap to
    convert between QImage and QPixmap: QPixmap::toImage() and
    QPixmap::fromImage(). These function can be slow on some
    platforms. There's also a QIcon class that stores various versions
    of an icon.

    A common way to create a pixmap is to use the constructor that
    takes a file name. For example:

    \code
        label->setPixmap(QPixmap("paste.png"));
    \endcode

    The file name can either refer to an actual file on disk or to
    one of the application's embedded resources. See the
    \l{Resource System} overview for details on how
    to embed images and other resource files in the application's
    executable.

    Pixel data in a pixmap is internal and is managed by the
    underlying window system. Pixels can be accessed only through
    QPainter functions or by converting the QPixmap to a QImage.

    You can easily display a QPixmap on the screen using QLabel or
    one of QAbstractButton's subclasses (such as QPushButton and
    QToolButton). QLabel has a \l{QLabel::pixmap}{pixmap} property,
    whereas QAbstractButton has an \l{QAbstractButton::icon}{icon}
    property.

    The QPixmap class uses \l{shclass.html}{implicit sharing}, so you
    can pass QPixmap objects around by value.

    You can retrieve the width(), height(), depth(), and size() of a
    pixmap. The enclosing rectangle can be determined with rect().
    Pixmaps can be filled with fill() and resized with resize(). You
    can create and set a mask with createHeuristicMask() and setMask().
    Use selfMask() to see if the pixmap is identical to its mask.

    In addition to loading a pixmap from file using load() you can
    also loadFromData(). You can obtain a transformed version of the
    pixmap using transform().

    \sa QBitmap, QImage, QImageReader, {shclass.html}{Shared Classes}
*/
