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

QPixmap::QPixmap(int w, int h, int depth, bool bitmap)
    : QPaintDevice(QInternal::Pixmap)
{
    init(w, h, depth, bitmap);
}

QPixmap::QPixmap()
    : QPaintDevice(QInternal::Pixmap)
{
    init(0, 0, 0, false);
}

QPixmap::QPixmap(const QImage& image)
    : QPaintDevice(QInternal::Pixmap)
{
    init(0, 0, 0, false);
    *this = fromImage(image);
}

QPixmap::QPixmap(int w, int h, int depth)
    : QPaintDevice(QInternal::Pixmap)
{
    init(w, h, depth, false);
}

QPixmap::QPixmap(const QSize &size, int depth)
    : QPaintDevice(QInternal::Pixmap)
{
    init(size.width(), size.height(), depth, false);
}

#ifndef QT_NO_IMAGEIO
QPixmap::QPixmap(const QString& fileName, const char *format,
                 Qt::ImageConversionFlags flags)
    : QPaintDevice(QInternal::Pixmap)
{
    init(0, 0, 0, false);
    load(fileName, format, flags);
}
#endif //QT_NO_IMAGEIO

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

QPixmap::QPixmap(const char * const xpm[])
    : QPaintDevice(QInternal::Pixmap)
{
    init(0, 0, 0, false);

    QImage image(xpm);
    if (!image.isNull())
        (*this) = fromImage(image);
}

QPixmap::~QPixmap()
{
    deref();
}

QPixmap &QPixmap::operator=(const QPixmap &pixmap)
{
    if (paintingActive()) {
        qWarning("QPixmap::operator=: Cannot assign to pixmap during painting");
        return *this;
    }
    pixmap.data->ref();                                // avoid 'x = x'
    deref();
    if (pixmap.paintingActive()) {                // make a deep copy
        printf("fitte\n");
        *this = pixmap.copy();
    } else {
        data = pixmap.data;
        devFlags = pixmap.devFlags;                // copy QPaintDevice flags
    }
    return *this;
}
QPixmap &QPixmap::operator=(const QImage &image)
{
    (*this) = fromImage(image);
    return *this;
}
QPixmap::operator QVariant() const
{
    extern bool qRegisterGuiVariant();
    static const bool b = qRegisterGuiVariant();
    Q_UNUSED(b)
    return QVariant(QVariant::Pixmap, this);
}
bool QPixmap::isNull() const
{
    return data->image.isNull();
}

int QPixmap::width() const
{
    return data->image.width();
}

int QPixmap::height() const
{
    return data->image.height();
}

QSize QPixmap::size() const
{
    return data->image.size();
}

QRect QPixmap::rect() const
{
    return data->image.rect();
}

int QPixmap::depth() const
{
    return data->image.depth();
}

int QPixmap::defaultDepth()
{
    return 32; //###
}

void QPixmap::fill(const QColor &fillColor)
{
    uint pixel;

    if (data->image.depth() == 1) {
        int gray = qGray(fillColor.rgba());
        // Pick the best approximate color in the image's colortable.
        if (qAbs(qGray(data->image.color(0)) - gray) < qAbs(qGray(data->image.color(1)) - gray)) {
            pixel = 0;
        } else {
            pixel = 1;
        }
    } else if (data->image.depth() == 32) {
        // 32-bit
        pixel = fillColor.rgba();
    } else {
        pixel = 0;
        // ### what about 8/16-bits
    }

    data->image.fill(pixel);
}

void QPixmap::resize_helper(const QSize &size)
{
    if (size == data->image.size())
        return;

    QImage image;

    // Initialize new image
    if (data->image.depth() == 32) {
        image = QImage(size, 32);
        image.fill(0);
    } else {
        image = QImage(size.width(), size.height(), 1, 2, QImage::LittleEndian);
        image.setColor(0, QColor(Qt::color0).rgba());
        image.setColor(1, QColor(Qt::color1).rgba());
    }

    // Copy the data over
    QPainter p(&image);
    p.drawPixmap(0, 0, *this);
    p.end();

    // replace with new data.
    data->image = image;
}

QBitmap QPixmap::mask() const
{
    QBitmap bm;
    return bm;
}

void QPixmap::setMask(const QBitmap &mask)
{
    if (mask.size() != size()) {
        qWarning("QPixmap::setMask() mask size differs from pixmap size");
        return;
    }

    QImage imageMask = mask.toImage();

    Q_ASSERT(imageMask.depth() == 1);

    int w = width();
    int h = height();

    const uchar bit_mask[] = {
        0x01,
        0x02,
        0x04,
        0x08,
        0x10,
        0x20,
        0x40,
        0x80
    };

    switch (depth()) {

    case 1:
        // ### self mask, do nothing for now
        break;

    case 32:
        for (int y=0; y<h; ++y) {
            uchar *mscan = imageMask.scanLine(y);
            QRgb *tscan = (QRgb *) data->image.scanLine(y);
            for (int x=0; x<w; ++x) {
                if (mscan[x/8] & bit_mask[x%8])
                    tscan[x] |= 0xff000000;
                else
                    tscan[x] &= 0x00ffffff;
            }
        }
        break;
    }
}

bool QPixmap::hasAlpha() const
{
    return data->image.hasAlphaBuffer();
}

bool QPixmap::hasAlphaChannel() const
{
    return data->image.hasAlphaBuffer();
}

QBitmap QPixmap::createHeuristicMask(bool clipTight ) const
{
    QBitmap m = fromImage(toImage().createHeuristicMask(clipTight));
    return m;
}

QBitmap QPixmap::createMaskFromColor(const QColor &maskColor) const
{
    QImage maskImage(size(), 1, 0, QImage::LittleEndian);
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
    QBitmap m = fromImage(maskImage);
    return m;
}

QPixmap QPixmap::grabWindow(WId, int x, int y, int w, int h )
{
//###
    return QPixmap();
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
        if (!child->isWidgetType() || child->isWindow()
            || child->isExplicitlyHidden() || !child->geometry().intersects(r))
            continue;
        QRect cr = r & child->geometry();
        cr.translate(-child->pos());
        grabWidget_helper(child, res, buf, cr, offset + child->pos());
    }
}


QPixmap QPixmap::grabWidget(QWidget *widget, const QRect &rect)
{
    QPixmap res, buf;

    if (!widget)
        return res;

    QRect r(rect);
    if (r.width() < 0)
        r.setWidth(widget->width() - rect.x());
    if (r.height() < 0)
        r.setHeight(widget->height() - rect.y());

    if (!r.intersects(widget->rect()))
        return res;

    res.resize_helper(r.size());
    buf.resize_helper(r.size());
    if(!res || !buf)
        return res;

    grabWidget_helper(widget, res, buf, r, QPoint());
    return res;
}


QPixmap QPixmap::scaled(const QSize &size, Qt::AspectRatioMode aspectMode,
                       Qt::TransformationMode mode ) const
{
    return QPixmap::fromImage(data->image.scaled(size, aspectMode, mode));
}

QPixmap QPixmap::scaledToWidth(int w) const
{
    return QPixmap::fromImage(data->image.scaledToWidth(w));
}

QPixmap QPixmap::scaledToHeight(int h) const
{
    return QPixmap::fromImage(data->image.scaledToHeight(h));
}

QPixmap QPixmap::transformed(const QMatrix &matrix, Qt::TransformationMode mode ) const
{
    return QPixmap::fromImage(data->image.transformed(matrix, mode));
}


QMatrix QPixmap::trueMatrix(const QMatrix &m, int w, int h)
{
    return QImage::trueMatrix(m, w, h);
}

QImage QPixmap::toImage() const
{
    return data->image;
}

QPixmap QPixmap::fromImage(const QImage &image, Qt::ImageConversionFlags flags )
{
    Q_UNUSED(flags);
    QPixmap pixmap(image.width(), image.height());
    pixmap.data->image = image;
    pixmap.data->bitmap = image.depth() == 1;
    return pixmap;
}

bool QPixmap::load(const QString& fileName, const char *format, Qt::ImageConversionFlags flags )
{
    QFileInfo info(fileName);
    QString key = QLatin1String("qt_pixmap_") + info.absoluteFilePath() + QLatin1Char('_') + info.lastModified().toString();

    if (QPixmapCache::find(key, *this))
            return true;
    QImage image = QImageReader(fileName, format).read();

    if (!image.isNull()) {
        *this = fromImage(image, flags);
        if (!isNull()) {
            QPixmapCache::insert(key, *this);
            return true;
        }
    }
    return false;
}

bool QPixmap::loadFromData(const uchar *buf, uint len, const char* format, Qt::ImageConversionFlags flags )
{
    QByteArray a = QByteArray::fromRawData(reinterpret_cast<const char *>(buf), len);
    QBuffer b(&a);
    b.open(QIODevice::ReadOnly);

    QImage image = QImageReader(&b, format).read();
    if (!image.isNull())
        *this = fromImage(image, flags);
    return !isNull();
}

bool QPixmap::loadFromData(const QByteArray &data, const char* format, Qt::ImageConversionFlags flags )
{
    return loadFromData((const uchar *)data.constData(), data.size(), format, flags);
}

bool QPixmap::save(const QString& fileName, const char* format, int quality ) const
{
    if (isNull())
        return false;                                // nothing to save
    QImageWriter writer(fileName, format);
    return doImageIO(&writer, quality);
}

bool QPixmap::save(QIODevice* device, const char* format, int quality ) const
{
    if (isNull())
        return false;                                // nothing to save
    QImageWriter writer(device, format);
    return doImageIO(&writer, quality);
}

int QPixmap::serialNumber() const
{
    return data->image.serialNumber();
}

bool QPixmap::isDetached() const
{
    return data->count == 1;
}

void QPixmap::detach()
{
    if (data->count != 1)
        *this = copy();
}

bool QPixmap::isQBitmap() const
{
    return depth() == 1;
}

#ifdef QT3_SUPPORT
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

bool QPixmap::load(const QString& fileName, const char *format, ColorMode mode)
{
    return load(fileName, format, colorModeToFlags(mode));
}

bool QPixmap::loadFromData(const uchar *buf, uint len, const char* format, ColorMode mode)
{
    return loadFromData(buf, len, format, colorModeToFlags(mode));
}

bool QPixmap::convertFromImage(const QImage &image, ColorMode mode)
{
    *this = fromImage(image, colorModeToFlags(mode));
    return !isNull();
}
#endif // QT3_SUPPORT

QPixmap::QPixmap(int w, int h, const uchar *data, bool isXbitmap)
    : QPaintDevice(QInternal::Pixmap)
{
    init(0, 0, 0, false);
}

int QPixmap::metric(PaintDeviceMetric metric) const
{
    return data->image.metric(metric);
}

bool QPixmap::doImageIO(QImageWriter *writer, int quality) const
{
    if (quality > 100  || quality < -1)
        qWarning("QPixmap::save: quality out of range [-1,100]");
    if (quality >= 0)
        writer->setQuality(qMin(quality,100));
    return writer->write(toImage());
}

void QPixmap::init(int w, int h, int d, bool bitmap)
{
    if (d <= 0)
        d = 32;

    Q_ASSERT(d == 1 || d == 32);

    data = new QPixmapData;
    if (d == 1) {
        data->image = QImage(w, h, 1, 2, QImage::LittleEndian);
        data->bitmap = true;
        data->image.setColor(0, QColor(Qt::color0).rgba());
        data->image.setColor(1, QColor(Qt::color1).rgba());
    } else {
        data->image = QImage(w, h, d);
        data->bitmap = false;
    }
}

void QPixmap::deref()
{
    if(data && data->deref()) { // Destroy image if last ref
        delete data;
        data = 0;
    }
}

QPixmap QPixmap::copy() const
{
    QPixmap pm = QPixmap::fromImage(data->image.copy());
    pm.data->bitmap = data->bitmap;
    return pm;
}

QDataStream &operator<<(QDataStream &s, const QPixmap &pixmap)
{
    s << pixmap.toImage();
    return s;
}

QDataStream &operator>>(QDataStream &s, QPixmap &pixmap)
{
    QImage img;
    s >> img;
    pixmap = QPixmap::fromImage(img);
    return s;
}

QPaintEngine *QPixmap::paintEngine() const
{
    return data->image.paintEngine();
}

void QPixmap::setAlphaChannel(const QPixmap &alphaChannel)
{
#if 0
    data->image.setAlphaChannel(alphaChannel.toImage());
#endif
}

QPixmap QPixmap::alphaChannel() const
{
#if 0
    return QPixmap::fromImage(data->image.alphaChannel());
#endif
    return QPixmap();
}

HBITMAP QPixmap::toWinHBITMAP() const
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
    const QImage image = data->image.depth() == 32 ? data->image : data->image.convertDepth(32);
    int bytes_per_line = w * 4;
    for (int y=0; y<h; ++y)
        memcpy(pixels + y * bytes_per_line, image.scanLine(y), bytes_per_line);

    return bitmap;
}


QPixmap QPixmap::fromWinHBITMAP(HBITMAP hbitmap)
{
    // Verify size
    BITMAP bitmap_info;
    memset(&bitmap_info, 0, sizeof(BITMAP));
    if (!GetObject(hbitmap, sizeof(BITMAP), &bitmap_info)) {
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
    if (GetDIBits(qt_win_display_dc(), hbitmap, 0, h, data, &bmi, DIB_RGB_COLORS)) {
        // Create image and copy data into image.
        QImage image(w, h, 32);
        int bytes_per_line = w * 4;
        for (int y=0; y<h; ++y)
            memcpy(image.scanLine(y), data + y * bytes_per_line, bytes_per_line);
        result = image;
    } else {
        qWarning("QPixmap::fromWinHBITMAP(), failed to get bitmap bits");
    }
    qFree(data);
    return fromImage(result);
}
