/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpixmap.h"
#include <private/qpixmap_p.h>

#include "qbitmap.h"
#include "qimage.h"
#include "qwidget.h"
#include "qpainter.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qapplication.h"
#include <private/qwidget_p.h>
#include "qpaintengine_d3d_p.h"
#include "qevent.h"
#include "qfile.h"
#include "qfileinfo.h"
#include "qdatetime.h"
#include "qpixmapcache.h"
#include "qimagereader.h"
#include "qimagewriter.h"
#include "qdebug.h"
#include "qpaintengine.h"
#include <d3d9.h>

extern QDirect3DPaintEngine *qt_d3dEngine();

typedef void (*_qt_pixmap_cleanup_hook)(int);
Q_GUI_EXPORT _qt_pixmap_cleanup_hook qt_pixmap_cleanup_hook = 0;

// rename in Qt 5.0
typedef void (*_qt_pixmap_cleanup_hook_64)(qint64);
Q_GUI_EXPORT _qt_pixmap_cleanup_hook_64 qt_pixmap_cleanup_hook_64 = 0;

Q_GUI_EXPORT qint64 qt_pixmap_id(const QPixmap &pixmap)
{
    return pixmap.cacheKey();
}

QPixmap::QPixmap()
    : QPaintDevice()
{
    init(0, 0);
}

#ifdef QT3_SUPPORT
QPixmap::QPixmap(const QImage& image)
    : QPaintDevice()
{
    init(0, 0);
    *this = fromImage(image);
}
#endif
QPixmap::QPixmap(int w, int h)
    : QPaintDevice()
{
    init(w, h);
}

QPixmap::QPixmap(const QSize &size)
    : QPaintDevice()
{
    init(size.width(), size.height());
}

QPixmap::QPixmap(const QSize &size, enum QPixmap::Type type)
    : QPaintDevice()
{
    init(size.width(), size.height(), type);
}

QPixmap::QPixmap(const QString& fileName, const char *format, Qt::ImageConversionFlags flags)
    : QPaintDevice()
{
    init(0, 0);
    load(fileName, format, flags);
}

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

#ifndef QT_NO_IMAGEFORMAT_XPM
QPixmap::QPixmap(const char * const xpm[])
    : QPaintDevice()
{
    init(0, 0);

    QImage image(xpm);
    if (!image.isNull())
        (*this) = fromImage(image);
}
#endif // QT_NO_IMAGEFORMAT_XPM

QPixmap::~QPixmap()
{
    deref();
}

int QPixmap::devType() const
{
    return QInternal::Pixmap;
}

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
#ifdef QT3_SUPPORT
QPixmap &QPixmap::operator=(const QImage &image)
{
    (*this) = fromImage(image);
    return *this;
}
#endif
QPixmap::operator QVariant() const
{
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
    detach();
    if (data->image.depth() == 1) {
        int gray = qGray(fillColor.rgba());
        // Pick the best approximate color in the image's colortable.
        if (qAbs(qGray(data->image.color(0)) - gray) < qAbs(qGray(data->image.color(1)) - gray)) {
            pixel = 0;
        } else {
            pixel = 1;
        }
    } else if (data->image.depth() == 32
#ifdef Q_WS_QWS
               || data->image.depth() == 16
#endif
        ) {
        int alpha = fillColor.alpha();
        if (alpha != 255) {
            if (data->image.format() == QImage::Format_RGB32
#ifdef Q_WS_QWS
                || data->image.format() == QImage::Format_RGB16
#endif
                )
                data->image = data->image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            // Premultiply pixel value.
            pixel = qRgba(fillColor.red() * alpha / 255,
                          fillColor.green() * alpha / 255,
                          fillColor.blue() * alpha / 255,
                          alpha);
        } else {
            pixel = fillColor.rgba();
        }

    } else {
        pixel = 0;
        // ### what about 8 bits
    }

    data->image.fill(pixel);
}
#ifdef QT3_SUPPORT
void QPixmap::resize_helper(const QSize &size)
{
    if (size == data->image.size())
        return;
    detach();

    if (size.isEmpty())
        data->image = QImage();
    else if (data->image.isNull())
        *this = QPixmap(size, data->type);
    else
        data->image = data->image.copy(0, 0, size.width(), size.height());
}
#endif

const uchar qt_pixmap_bit_mask[] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };


QBitmap QPixmap::mask() const
{
    if (!data->image.hasAlphaChannel() || data->image.depth() != 32) {
        return QBitmap();
    }

    // Create image and setup color table
    int w = data->image.width();
    int h = data->image.height();
    QImage mask = data->createBitmapImage(w, h);
    int bpl = mask.bytesPerLine();

    // copy over the data
    for (int y=0; y<h; ++y) {
        QRgb *src = (QRgb *) data->image.scanLine(y);
        uchar *dest = mask.scanLine(y);
        memset(dest, 0, bpl);
        for (int x=0; x<w; ++x) {
            if (qAlpha(*src) > 0)
                dest[x>>3] |= qt_pixmap_bit_mask[x&7];
            ++src;
        }
    }

    return QBitmap::fromImage(mask);
}

void QPixmap::setMask(const QBitmap &mask)
{
    if (paintingActive()) {
        qWarning("QPixmap::setMask: Cannot set mask while pixmap is being painted on");
        return;
    }

    if (mask.size().isEmpty()) {
        if (depth() != 1) {
            detach();
            data->image = data->image.convertToFormat(QImage::Format_RGB32);
        }

    } else if (mask.size() != size()) {
        qWarning("QPixmap::setMask() mask size differs from pixmap size");

    } else {
        detach();
        const QImage imageMask = mask.toImage().convertToFormat(QImage::Format_MonoLSB);

        int w = width();
        int h = height();

        switch (depth()) {

        case 1:
            for (int y=0; y<h; ++y) {
                const uchar *mscan = imageMask.scanLine(y);
                uchar *tscan = data->image.scanLine(y);
                int bytesPerLine = data->image.bytesPerLine();
                for (int i=0; i<bytesPerLine; ++i)
                    tscan[i] &= mscan[i];
            }
            break;

        case 32:
            data->image = data->image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
            for (int y=0; y<h; ++y) {
                const uchar *mscan = imageMask.scanLine(y);
                QRgb *tscan = (QRgb *) data->image.scanLine(y);
                for (int x=0; x<w; ++x) {
                    if (!(mscan[x>>3] & qt_pixmap_bit_mask[x&7]))
                        tscan[x] = 0;
                }
            }
            break;
        }
    }
}

bool QPixmap::hasAlpha() const
{
    return data->image.hasAlphaChannel();
}

bool QPixmap::hasAlphaChannel() const
{
    return data->image.hasAlphaChannel();
}

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
QBitmap QPixmap::createHeuristicMask(bool clipTight ) const
{
    QBitmap m = fromImage(toImage().createHeuristicMask(clipTight));
    return m;
}
#endif

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
    QBitmap m = fromImage(maskImage);
    return m;
}


static void sendResizeEvents(QWidget *target)
{
    QResizeEvent e(target->size(), QSize());
    QApplication::sendEvent(target, &e);

    const QObjectList children = target->children();
    for (int i = 0; i < children.size(); ++i) {
        QWidget *child = static_cast<QWidget*>(children.at(i));
        if (child->isWidgetType() && !child->isWindow() && child->testAttribute(Qt::WA_PendingResizeEvent))
            sendResizeEvents(child);
    }
}


QPixmap QPixmap::grabWidget(QWidget *widget, const QRect &rect)
{
    if (!widget)
        return QPixmap();

    if (widget->testAttribute(Qt::WA_PendingResizeEvent) || !widget->testAttribute(Qt::WA_WState_Created))
        sendResizeEvents(widget);

    QRect r(rect);
    if (r.width() < 0)
        r.setWidth(widget->width() - rect.x());
    if (r.height() < 0)
        r.setHeight(widget->height() - rect.y());

    if (!r.intersects(widget->rect()))
        return QPixmap();

     QPixmap res(r.size());

    widget->d_func()->drawWidget(&res, r, -r.topLeft(), QWidgetPrivate::DrawRecursive | QWidgetPrivate::DrawAsRoot | QWidgetPrivate::DrawPaintOnScreen | QWidgetPrivate::DrawInvisible);
    return res;
}


QPixmap QPixmap::scaled(const QSize &size, Qt::AspectRatioMode aspectMode,
                       Qt::TransformationMode mode ) const
{
    return QPixmap::fromImage(data->image.scaled(size, aspectMode, mode));
}

QPixmap QPixmap::scaledToWidth(int w, Qt::TransformationMode mode) const
{
    return QPixmap::fromImage(data->image.scaledToWidth(w, mode));
}

QPixmap QPixmap::scaledToHeight(int h, Qt::TransformationMode mode) const
{
    return QPixmap::fromImage(data->image.scaledToHeight(h, mode));
}

QPixmap QPixmap::transformed(const QMatrix &matrix, Qt::TransformationMode mode ) const
{
    return QPixmap::fromImage(data->image.transformed(matrix, mode));
}

QPixmap QPixmap::transformed(const QTransform &matrix, Qt::TransformationMode mode ) const
{
    return QPixmap::fromImage(data->image.transformed(matrix, mode));
}

QMatrix QPixmap::trueMatrix(const QMatrix &m, int w, int h)
{
    return QImage::trueMatrix(m, w, h);
}

QTransform QPixmap::trueMatrix(const QTransform &m, int w, int h)
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
    // ### This will create a temporary image.
    QPixmap pixmap(image.width(), image.height());

    QImage im = image.convertToFormat(QImage::Format_ARGB32);
    if (FAILED(qt_d3dEngine()->d_func()->m_d3dDevice->CreateTexture(image.width(), image.height(), 1, 0,
                    D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &pixmap.data->texture, 0))) {
        qWarning("QPixmap::init(): unable to create Direct3D texture.");
        return QPixmap();
    }
    D3DLOCKED_RECT rect;
    if (FAILED(pixmap.data->texture->LockRect(0, &rect, 0, 0))) {
        qDebug() << "QDirect3DPaintEngine: unable to lock texture rect.";
        return QPixmap();
    }
    DWORD *dst = (DWORD *) rect.pBits;
    DWORD *src = (DWORD *) im.scanLine(0);
    int dst_ppl = rect.Pitch/4;
    int src_ppl = im.bytesPerLine()/4;

    Q_ASSERT(dst_ppl == src_ppl);
    memcpy(dst, src, rect.Pitch*im.height());
    pixmap.data->texture->UnlockRect(0);

    pixmap.data->image = image;

    return pixmap;
}

bool QPixmap::load(const QString& fileName, const char *format, Qt::ImageConversionFlags flags )
{
    if (fileName.isEmpty())
        return false;

    QFileInfo info(fileName);
    QString key;
    key.append(QLatin1String("qt_pixmap_")).append(info.absoluteFilePath()).append(
               QLatin1Char('_')).append(QString::number(info.lastModified().toTime_t())).append(
               QLatin1Char('_')).append(QString::number(data->type));

    detach();
    if (QPixmapCache::find(key, *this))
            return true;
    QImage image = QImageReader(fileName, format).read();

    if (!image.isNull()) {
        if (data->type == BitmapType)
            *this = QBitmap::fromImage(image, flags);
        else
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

    detach();
    QImage image = QImageReader(&b, format).read();
    if (!image.isNull()) {
        if (data->type == BitmapType)
            *this = QBitmap::fromImage(image, flags);
        else
            *this = fromImage(image, flags);
    }
    return !isNull();
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

qint64 QPixmap::cacheKey() const
{
    // avoid exposing the internal QImageData structure..
#if defined (Q_CC_MINGW) || (defined (_MSC_VER) && _MSC_VER >= 1310)
    return -((data->image.cacheKey() & 0xffffffff00000000LL) | ((qint64) data->detach_no));
#else
    // MSVC 6.0 can't handle 64 bit constants properly..
    qint64 mask = 0xffffffff;
    mask <<= 32;
    return -((data->image.cacheKey() & mask) | ((qint64) data->detach_no));
#endif
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
    ++data->detach_no;
    if (data->count != 1)
        *this = copy();
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

void QPixmap::init(int w, int h, Type type)
{
    if (!qApp) {
        qFatal("QPixmap: Must construct a QApplication before a QPaintDevice");
        return;
    }
    data = new QPixmapData;
    data->type = type;
    data->detach_no = 0;
    data->texture = 0;
    if (type == PixmapType) {
        data->image = QImage(w, h, QImage::Format_RGB32);
    } else {
        data->image = data->createBitmapImage(w, h);
    }
}

void QPixmap::deref()
{
    if(data && data->deref()) { // Destroy image if last ref
        if (data->texture)
            data->texture->Release();
        delete data;
        data = 0;
    }
}

QPixmap QPixmap::copy(const QRect &rect) const
{
    if (data->type == BitmapType)
        return QBitmap::fromImage(toImage().copy(rect));
    else
        return QPixmap::fromImage(toImage().copy(rect));
}

#ifndef QT_NO_DATASTREAM
QDataStream &operator<<(QDataStream &s, const QPixmap &pixmap)
{
    s << pixmap.toImage();
    return s;
}

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
#endif // QT_NO_DATASTREAM

QPaintEngine *QPixmap::paintEngine() const
{
    //return qt_d3dEngine();
    return data->image.paintEngine();
}

void QPixmap::setAlphaChannel(const QPixmap &alphaChannel)
{
    if (paintingActive()) {
        qWarning("QPixmap::setAlphaChannel: Cannot set alpha channel while pixmap is being painted on");
        return;
    }

    detach();
    data->image.setAlphaChannel(alphaChannel.toImage());
}

QPixmap QPixmap::alphaChannel() const
{
    return QPixmap::fromImage(data->image.alphaChannel());
}

QImage QPixmapData::createBitmapImage(int w, int h)
{
    QImage bitmap(w, h, QImage::Format_MonoLSB);
    if (bitmap.isNull())
        return bitmap;
    bitmap.setNumColors(2);
    bitmap.setColor(0, QColor(Qt::color0).rgba());
    bitmap.setColor(1, QColor(Qt::color1).rgba());
    return bitmap;
}

#ifdef QT3_SUPPORT
Q_GUI_EXPORT void copyBlt(QPixmap *dst, int dx, int dy,
                          const QPixmap *src, int sx, int sy, int sw, int sh)
{
    Q_ASSERT_X(dst, "::copyBlt", "Destination pixmap must be non null");
    Q_ASSERT_X(src, "::copyBlt", "Source pixmap must be non null");

    if (src->hasAlphaChannel()) {
        if (dst->paintEngine()->hasFeature(QPaintEngine::PorterDuff)) {
            QPainter p(dst);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.drawPixmap(dx, dy, *src, sx, sy, sw, sh);
        } else {
            QImage image = dst->toImage().convertToFormat(QImage::Format_ARGB32_Premultiplied);
            QPainter p(&image);
            p.setCompositionMode(QPainter::CompositionMode_Source);
            p.drawPixmap(dx, dy, *src, sx, sy, sw, sh);
            p.end();
            *dst = QPixmap::fromImage(image);
        }
    } else {
        QPainter p(dst);
        p.drawPixmap(dx, dy, *src, sx, sy, sw, sh);
    }

}
#endif
