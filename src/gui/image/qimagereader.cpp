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

#include "qimagereader.h"

#include <qbytearray.h>
#include <qdebug.h>
#include <qfile.h>
#include <qimage.h>
#include <qimageiohandler.h>
#include <qlist.h>
#include <qrect.h>
#include <qsize.h>
#include <qvariant.h>

// factory loader
#include <qcoreapplication.h>
#include <private/qfactoryloader_p.h>

// image handlers
#include <private/qbmphandler_p.h>
#include <private/qppmhandler_p.h>
#include <private/qxbmhandler_p.h>
#include <private/qxpmhandler_p.h>
#ifndef QT_NO_IMAGEIO_PNG
#include <private/qpnghandler_p.h>
#endif

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QImageIOHandlerFactoryInterface_iid, QCoreApplication::libraryPaths(), QLatin1String("/imageformats")))

static QImageIOHandler *createReadHandler(QIODevice *device, const QByteArray &format)
{
    QByteArray form = format.toLower();
    QImageIOHandler *handler = 0;

    if (format.isEmpty()) {
#ifndef QT_NO_IMAGEIO_PNG
        if (QPngHandler::canRead(device))
            handler = new QPngHandler;
        else
#endif
        if (QBmpHandler::canRead(device))
            handler = new QBmpHandler;
        else if (QXpmHandler::canRead(device))
            handler = new QXpmHandler;
        else if (QPpmHandler::canRead(device))
            handler = new QPpmHandler;
        else if (QXbmHandler::canRead(device))
            handler = new QXbmHandler;
    } else {
#ifndef QT_NO_IMAGEIO_PNG
        if (form == "png") {
            handler = new QPngHandler;
        } else
#endif
        if (form == "bmp") {
            handler = new QBmpHandler;
        } else if (form == "xpm") {
            handler = new QXpmHandler;
        } else if (form == "xbm") {
            handler = new QXbmHandler;
            handler->setOption(QImageIOHandler::SubType, form);
        } else if (form == "pbm" || form == "pbmraw" || form == "pgm"
                 || form == "pgmraw" || form == "ppm" || form == "ppmraw") {
            handler = new QPpmHandler;
            handler->setOption(QImageIOHandler::SubType, form);
        }
    }

    if (!handler) {
        QFactoryLoader *l = loader();
        QStringList keys = l->keys();
        for (int i = 0; i < keys.count(); ++i) {
            QImageIOPlugin *plugin = qt_cast<QImageIOPlugin *>(l->instance(keys.at(i)));
            if (plugin->capabilities(device, form) & QImageIOPlugin::CanRead) {
                handler = plugin->create(device, form);
                break;
            }
        }
    }

    if (!handler)
        return 0;

    handler->setDevice(device);
    handler->setFormat(format);
    return handler;
}

class QImageReaderPrivate
{
public:
    QImageReaderPrivate(QImageReader *qq);
    ~QImageReaderPrivate();

    // device
    QByteArray format;
    QIODevice *device;
    bool deleteDevice;
    QImageIOHandler *handler;
    bool initHandler();

    // image options
    QRect clipRect;
    QSize scaledSize;
    QRect scaledClipRect;

    // error
    QImageReader::ImageReaderError imageReaderError;
    QString errorString;

    QImageReader *q;
};

QImageReaderPrivate::QImageReaderPrivate(QImageReader *qq)
{
    device = 0;
    deleteDevice = false;
    handler = 0;
    imageReaderError = QImageReader::UnknownError;
    errorString = QT_TRANSLATE_NOOP(QImageReader, "Unknown error");

    q = qq;
}

QImageReaderPrivate::~QImageReaderPrivate()
{
    if (deleteDevice)
        delete device;
}

bool QImageReaderPrivate::initHandler()
{
    // check some preconditions
    if (!device || (!deleteDevice && !device->isOpen())) {
        imageReaderError = QImageReader::DeviceError;
        errorString = QT_TRANSLATE_NOOP(QImageReader, "Invalid device");
        return false;
    }

    // probe the file extension
    if (deleteDevice && !device->isOpen() && !device->open(QIODevice::ReadOnly)) {
        QList<QByteArray> extensions = QImageReader::supportedImageFormats();
        int currentExtension = 0;

        QFile *file = static_cast<QFile *>(device);
        QString fileName = file->fileName();

        do {
            file->setFileName(fileName + QLatin1Char('.') + extensions.at(currentExtension++));
            file->open(QIODevice::ReadOnly);
        } while (!file->isOpen() && currentExtension < extensions.size());

        if (!device->isOpen()) {
            imageReaderError = QImageReader::FileNotFoundError;
            errorString = QT_TRANSLATE_NOOP(QImageReader, "File not found");
            return false;
        }
    }

    // assign a handler
    if (!handler && (handler = ::createReadHandler(device, format)) == 0) {
        imageReaderError = QImageReader::UnsupportedFormatError;
        errorString = QT_TRANSLATE_NOOP(QImageReader, "Unsupported image format");
        return false;
    }

    return true;
}

QImageReader::QImageReader()
    : d(new QImageReaderPrivate(this))
{
}

QImageReader::QImageReader(QIODevice *device, const QByteArray &format)
    : d(new QImageReaderPrivate(this))
{
    d->device = device;
    d->format = format;
}

QImageReader::QImageReader(const QString &fileName, const QByteArray &format)
    : d(new QImageReaderPrivate(this))
{
    QFile *file = new QFile(fileName);
    d->device = file;
    d->deleteDevice = true;
    d->format = format;
}

QImageReader::~QImageReader()
{
    delete d;
}

void QImageReader::setFormat(const QByteArray &format)
{
    d->format = format;
}

QByteArray QImageReader::format() const
{
    return d->format;
}

void QImageReader::setDevice(QIODevice *device)
{
    if (d->device && d->deleteDevice)
        delete d->device;
    d->device = device;
    d->deleteDevice = false;
    delete d->handler;
    d->handler = 0;
}

QIODevice *QImageReader::device() const
{
    return d->device;
}

void QImageReader::setFileName(const QString &fileName)
{
    setDevice(new QFile(fileName));
    d->deleteDevice = true;
}

QString QImageReader::fileName() const
{
    QFile *file = qt_cast<QFile *>(d->device);
    return file ? file->fileName() : QString();
}

QSize QImageReader::size() const
{
    if (!d->initHandler())
        return QSize();

    if (d->handler->supportsOption(QImageIOHandler::Size))
        return d->handler->option(QImageIOHandler::Size).toSize();

    QImage image;
    if (!d->handler->read(&image)) {
        // ### skips a frame in animations
        d->imageReaderError = InvalidDataError;
        d->errorString = QT_TRANSLATE_NOOP(QImageReader, "Unable to read image data");
        return QSize();
    }

    return image.size();
}

void QImageReader::setClipRect(const QRect &rect)
{
    d->clipRect = rect;
}

QRect QImageReader::clipRect() const
{
    return d->clipRect;
}

void QImageReader::setScaledSize(const QSize &size)
{
    d->scaledSize = size;
}

QSize QImageReader::scaledSize() const
{
    return d->scaledSize;
}

void QImageReader::setScaledClipRect(const QRect &rect)
{
    d->scaledClipRect = rect;
}

QRect QImageReader::scaledClipRect() const
{
    return d->scaledClipRect;
}

bool QImageReader::canRead() const
{
    if (!d->initHandler())
        return false;

    return d->handler->canRead();
}

QImage QImageReader::read()
{
    if (!d->initHandler())
        return QImage();

    // set the handler specific options.
    if (d->handler->supportsOption(QImageIOHandler::ClipRect))
        d->handler->setOption(QImageIOHandler::ClipRect, d->clipRect);
    if (d->handler->supportsOption(QImageIOHandler::ScaledSize))
        d->handler->setOption(QImageIOHandler::ScaledSize, d->scaledSize);
    if (d->handler->supportsOption(QImageIOHandler::ScaledClipRect))
        d->handler->setOption(QImageIOHandler::ScaledClipRect, d->scaledClipRect);

    // read the image
    QImage image;
    if (!d->handler->read(&image)) {
        d->imageReaderError = InvalidDataError;
        d->errorString = QT_TRANSLATE_NOOP(QImageReader, "Unable to read image data");
        return QImage();
    }

    // provide default implementations for any unsupported image
    // options
    if (d->handler->supportsOption(QImageIOHandler::ClipRect)) {
        if (d->handler->supportsOption(QImageIOHandler::ScaledSize)) {
            if (d->handler->supportsOption(QImageIOHandler::ScaledClipRect)) {
                // all features are supported by the handler; nothing to do.
            } else {
                // the image is already scaled, so apply scaled clipping.
                if (d->scaledClipRect.isValid()) {
                    image = image.copy(d->scaledClipRect);
                }
            }
        } else {
            if (d->handler->supportsOption(QImageIOHandler::ScaledClipRect)) {
                // supports scaled clipping but not scaling, most
                // likely a broken handler.
          } else {
                if (d->scaledSize.isValid()) {
                    image = image.scale(d->scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                }
                if (d->scaledClipRect.isValid()) {
                    image = image.copy(d->scaledClipRect);
                }
            }
        }
    } else {
        if (d->handler->supportsOption(QImageIOHandler::ScaledSize)) {
            // in this case, there's nothing we can do. if the
            // plugin supports scaled size but not ClipRect, then
            // we have to ignore ClipRect."

            if (d->handler->supportsOption(QImageIOHandler::ScaledClipRect)) {
                // nothing to do (ClipRect is ignored!)
            } else {
                // provide all workarounds.
                if (d->scaledClipRect.isValid()) {
                    image = image.copy(d->scaledClipRect);
                }
            }
        } else {
            if (d->handler->supportsOption(QImageIOHandler::ScaledClipRect)) {
                // this makes no sense; a handler that supports
                // ScaledClipRect but not ScaledSize is broken, and we
                // can't work around it.
            } else {
                // provide all workarounds.
                if (d->clipRect.isValid())
                    image = image.copy(d->clipRect);
                if (d->scaledSize.isValid())
                    image = image.scale(d->scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                if (d->scaledClipRect.isValid())
                    image = image.copy(d->scaledClipRect);
            }
        }
    }

    return image;
}

int QImageReader::loopCount() const
{
    if (!d->initHandler())
        return -1;
    return d->handler->loopCount();
}

int QImageReader::imageCount() const
{
    if (!d->initHandler())
        return -1;
    return d->handler->imageCount();
}

int QImageReader::nextImageDelay() const
{
    if (!d->initHandler())
        return -1;
    return d->handler->nextImageDelay();
}

int QImageReader::currentImageNumber() const
{
    if (!d->initHandler())
        return -1;
    return d->handler->currentImageNumber();
}

QImageReader::ImageReaderError QImageReader::error() const
{
    return d->imageReaderError;
}

QString QImageReader::errorString() const
{
    return d->errorString;
}

QByteArray QImageReader::imageFormat(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return QByteArray();

    return imageFormat(&file);
}

QByteArray QImageReader::imageFormat(QIODevice *device)
{
    QImageIOHandler *handler = ::createReadHandler(device, QByteArray());
    QByteArray format = handler ? handler->name() : QByteArray();
    delete handler;
    return format;
}

QList<QByteArray> QImageReader::supportedImageFormats()
{
    QList<QByteArray> formats;
    formats << "bmp" << "pbm" << "pgm" << "ppm" << "xbm" << "xpm";
#ifndef QT_NO_IMAGEIO_PNG
    formats << "png";
#endif

    QFactoryLoader *l = loader();
    QStringList keys = l->keys();

    for (int i = 0; i < keys.count(); ++i) {
        QImageIOPlugin *plugin = qt_cast<QImageIOPlugin *>(l->instance(keys.at(i)));
        if (plugin->capabilities(0, keys.at(i).toLatin1()) & QImageIOPlugin::CanRead)
            formats << keys.at(i).toLatin1();
    }

    qHeapSort(formats);
    return formats;
}
