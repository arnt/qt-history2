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

//#define QIMAGEIO_DEBUG

/*! \class QImageIO

*/

#include <qbytearray.h>
#include <qcoreapplication.h>
#include <qfileinfo.h>
#include <qiodevice.h>
#include <qimage.h>
#include <qfile.h>
#include <qlist.h>
#include <qregion.h>
#include <qsize.h>
#include <qvariant.h>

#include <private/qfactoryloader_p.h>

#include "qimageio.h"
#include "qimageiohandler.h"

// handlers
#include "private/qbmphandler_p.h"
#ifndef QT_NO_IMAGEIO_PNG
#include "private/qpnghandler_p.h"
#endif
#include "private/qppmhandler_p.h"
#include "private/qxbmhandler_p.h"
#include "private/qxpmhandler_p.h"

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QImageIOHandlerFactoryInterface_iid, QCoreApplication::libraryPaths(), QLatin1String("/imageformats")))

static QImageIOHandler *createHandler(QIODevice *device, const QByteArray &format)
{
    QByteArray form = format.toLower();
    QImageIOHandler *handler = 0;

    if (format.isEmpty()) {
#ifndef QT_NO_IMAGEIO_PNG
        if (QPngHandler::canLoadImage(device))
            handler = new QPngHandler;
        else
#endif
        if (QBmpHandler::canLoadImage(device))
            handler = new QBmpHandler;
        else if (QXpmHandler::canLoadImage(device))
            handler = new QXpmHandler;
        else if (QPpmHandler::canLoadImage(device))
            handler = new QPpmHandler;
        else if (QXbmHandler::canLoadImage(device))
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
            handler->setProperty(QImageIOHandler::Subtype, form);
        } else if (form == "pbm" || form == "pbmraw" || form == "pgm"
                 || form == "pgmraw" || form == "ppm" || form == "ppmraw") {
            handler = new QPpmHandler;
            handler->setProperty(QImageIOHandler::Subtype, form);
        }
    }

    if (!handler) {
        QFactoryLoader *l = loader();
        QStringList keys = l->keys();
        for (int i = 0; i < keys.count(); ++i) {
            QImageIOPlugin *plugin = qt_cast<QImageIOPlugin *>(l->instance(keys.at(i)));
            if (plugin->capabilities(device, form) & QImageIOPlugin::CanLoad) {
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

class QImageIOPrivate
{
public:
    QImageIOPrivate(QImageIO *q_ptr);
    ~QImageIOPrivate();

    QImage image;
    QImageIOHandler *handler;

    QIODevice *device;
    bool deleteDevice;

    // properties
    QString fileName;
    QByteArray format;
    QByteArray parameters;
    float gamma;
    int quality;
    QSize size;
    QString description;
    QSize resolution;
    QRegion region;

    // errors
    QImageIO::Error error;
    QString errorString;

    QImageIO *q;
};

QImageIOPrivate::QImageIOPrivate(QImageIO *q_ptr)
{
    q = q_ptr;
    device = 0;
    deleteDevice = false;
    handler = 0;

    gamma = 0.0;
    quality = 0;
}

QImageIOPrivate::~QImageIOPrivate()
{
    if (deleteDevice)
        delete device;
    delete handler;
}

QImageIO::QImageIO()
    : d(new QImageIOPrivate(this))
{
}

QImageIO::QImageIO(QIODevice *device, const QByteArray &format)
    : d(new QImageIOPrivate(this))
{
    d->device = device;
    d->format = format;
}

QImageIO::QImageIO(const QString &fileName, const QByteArray &format)
    : d(new QImageIOPrivate(this))
{
    d->device = new QFile(fileName);
    if (!d->device->open(QIODevice::ReadWrite)) {
        if (!d->device->open(QIODevice::ReadOnly)) {
            qWarning("QImageIO::QImageIO(), unable to open %s: %s",
                     fileName.toLatin1().constData(), d->device->errorString().toLatin1().constData());
        }
    }

    d->deleteDevice = true;
    d->format = format;
}

QImageIO::~QImageIO()
{
    delete d;
}

void QImageIO::reset()
{
    delete d;
    d = new QImageIOPrivate(this);
}

void QImageIO::setImage(const QImage &image)
{
    d->image = image;
}

QImage QImageIO::image() const
{
    return d->image;
}

void QImageIO::setFormat(const QByteArray &format)
{
    d->format = format;
}

QByteArray QImageIO::format() const
{
    return d->format;
}

void QImageIO::setDevice(QIODevice *device)
{
    d->device = device;
    delete d->handler;
    d->handler = 0;
}

QIODevice *QImageIO::device() const
{
    return d->device;
}

void QImageIO::setFileName(const QString &fileName)
{
    if (d->device && d->deleteDevice)
        delete d->device;

    d->device = new QFile(fileName);
    if (!d->device->open(QIODevice::ReadWrite)) {
        if (!d->device->open(QIODevice::ReadOnly)) {
            qWarning("QImageIO::QImageIO(), unable to open %s: %s",
                     fileName.toLatin1().constData(), d->device->errorString().toLatin1().constData());
        }
    }

    d->deleteDevice = true;
    d->fileName = fileName;
    delete d->handler;
    d->handler = 0;
}

QString QImageIO::fileName() const
{
    return d->fileName;
}

void QImageIO::setQuality(int quality)
{
    d->quality = quality;
}

int QImageIO::quality() const
{
    return d->quality;
}

void QImageIO::setResolution(const QSize &resolution)
{
    d->resolution = resolution;
}

QSize QImageIO::resolution() const
{
    return d->resolution;
}

void QImageIO::setRegion(const QRegion &region)
{
    d->region = region;
}

QRegion QImageIO::region() const
{
    return d->region;
}

void QImageIO::setDescription(const QString &description)
{
    d->description = description;
}

QString QImageIO::description() const
{
    return d->description;
}

void QImageIO::setParameters(const QByteArray &parameters)
{
    d->parameters = parameters;
}

QByteArray QImageIO::parameters() const
{
    return d->parameters;
}

void QImageIO::setGamma(float gamma)
{
    d->gamma = gamma;
}

float QImageIO::gamma() const
{
    return d->gamma;
}

void QImageIO::setSize(const QSize &size)
{
    d->size = size;
}

QSize QImageIO::size() const
{
    return d->size;
}

bool QImageIO::load()
{
    if (!d->device) {
        qWarning("QImageIO::load() called with no device");
        return false;
    }

    if (!d->handler)
        d->handler = createHandler(d->device, d->format);

    if (!d->handler) {
        d->error = UnsupportedImageFormat;
        d->errorString = QT_TRANSLATE_NOOP(QIMageIO, "Unsupported image format");
        return false;
    }

    QImageIOHandler *handler = d->handler;
    handler->setProperty(QImageIOHandler::Size, d->size);
    handler->setProperty(QImageIOHandler::Gamma, d->gamma);
    handler->setProperty(QImageIOHandler::Quality, d->quality);
    handler->setProperty(QImageIOHandler::Resolution, d->resolution);
    handler->setProperty(QImageIOHandler::Region, d->region);
    handler->setProperty(QImageIOHandler::Size, d->size);

    if (!handler->load(&d->image)) {
        // ### should the handler have an error enum?
        d->error = InvalidImageData;
        d->errorString = QT_TRANSLATE_NOOP(QIMageIO, "Invalid image data");
        return false;
    }
    return true;
}

bool QImageIO::save()
{
    if (!d->device) {
        qWarning("QImageIO::save() called with no device");
        return false;
    }

    if (!d->handler)
        d->handler = createHandler(d->device, d->format);

    if (!d->handler) {
        d->error = UnsupportedImageFormat;
        d->errorString = QT_TRANSLATE_NOOP(QIMageIO, "Unsupported image format");
        return false;
    }

    QImageIOHandler *handler = d->handler;
    handler->setProperty(QImageIOHandler::Size, d->size);
    handler->setProperty(QImageIOHandler::Gamma, d->gamma);
    handler->setProperty(QImageIOHandler::Quality, d->quality);
    handler->setProperty(QImageIOHandler::Resolution, d->resolution);
    handler->setProperty(QImageIOHandler::Region, d->region);
    handler->setProperty(QImageIOHandler::Parameters, d->parameters);
    handler->setProperty(QImageIOHandler::Size, d->size);
    if (handler->supportsProperty(QImageIOHandler::Name)) {
        QString name = QFileInfo(d->fileName).fileName();
        name.replace(".", "_");
        name.replace("-", "_");
        handler->setProperty(QImageIOHandler::Name, name);

    }

    if (!handler->save(d->image)) {
        // ### should the handler have an error enum?
        d->error = UnsupportedImageFormat;
        d->errorString = QT_TRANSLATE_NOOP(QIMageIO, "Failed to save the image");
        return false;
    }
    return true;
}

int QImageIO::frameCount() const
{
    return d->handler ? d->handler->frameCount() : 0;
}

int QImageIO::nextFrameDelay() const
{
    return d->handler ? d->handler->nextFrameDelay() : 0;
}

int QImageIO::currentFrameNumber() const
{
    return d->handler ? d->handler->currentFrameNumber() : 0;
}

int QImageIO::loopCount() const
{
    return d->handler ? d->handler->loopCount() : 0;
}

QImageIO::Error QImageIO::error() const
{
    return d->error;
}

QString QImageIO::errorString() const
{
    return d->errorString;
}

QByteArray QImageIO::imageFormatForFileName(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return QByteArray();

    return imageFormatForDevice(&file);
}

QByteArray QImageIO::imageFormatForDevice(QIODevice *device)
{
    QImageIOHandler *handler = createHandler(device, QByteArray());
    QByteArray format = handler ? handler->name() : QByteArray();
    delete handler;
    return format;
}

QList<QByteArray> QImageIO::inputFormats()
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
        if ((plugin->capabilities(0, keys.at(i).toLatin1()) & QImageIOPlugin::CanLoad) != 0)
            formats << keys.at(i).toLatin1();
    }

    qHeapSort(formats);
    return formats;
}

QList<QByteArray> QImageIO::outputFormats()
{
    QList<QByteArray> formats;
    formats << "bmp" << "ppm" << "xbm" << "xpm";
#ifndef QT_NO_IMAGEIO_PNG
    formats << "png";
#endif

    QFactoryLoader *l = loader();
    QStringList keys = l->keys();
    for (int i = 0; i < keys.count(); ++i) {
        QImageIOPlugin *plugin = qt_cast<QImageIOPlugin *>(l->instance(keys.at(i)));
        if ((plugin->capabilities(0, keys.at(i).toLatin1()) & QImageIOPlugin::CanSave) != 0)
            formats << keys.at(i).toLatin1();
    }

    qHeapSort(formats);
    return formats;
}
