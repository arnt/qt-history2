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

#include "qimagewriter.h"

#include <qbytearray.h>
#include <qfile.h>
#include <qimageiohandler.h>
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

static QImageIOHandler *createWriteHandler(QIODevice *device, const QByteArray &format)
{
    QByteArray form = format.toLower();
    QImageIOHandler *handler = 0;

    if (!format.isEmpty()) {
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
            if (plugin->capabilities(device, form) & QImageIOPlugin::CanWrite) {
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

class QImageWriterPrivate
{
public:
    QImageWriterPrivate(QImageWriter *qq);

    // device
    QByteArray format;
    QIODevice *device;
    bool deleteDevice;
    QImageIOHandler *handler;

    // image options
    int quality;
    float gamma;
    QString description;
    QByteArray parameters;

    // error
    QImageWriter::ImageWriterError imageWriterError;
    QString errorString;

    QImageWriter *q;
};

QImageWriterPrivate::QImageWriterPrivate(QImageWriter *qq)
{
    device = 0;
    deleteDevice = false;
    handler = 0;
    quality = -1;
    gamma = 0.0;
    imageWriterError = QImageWriter::UnknownError;
    errorString = QT_TRANSLATE_NOOP(QImageWriter, "Unknown error");

    q = qq;
}

QImageWriter::QImageWriter()
    : d(new QImageWriterPrivate(this))
{
}

QImageWriter::QImageWriter(QIODevice *device, const QByteArray &format)
    : d(new QImageWriterPrivate(this))
{
    d->device = device;
    d->format = format;
    if (!d->device->isOpen())
        d->device->open(QIODevice::WriteOnly);
}

QImageWriter::QImageWriter(const QString &fileName, const QByteArray &format)
    : d(new QImageWriterPrivate(this))
{
    QFile *file = new QFile(fileName);
    d->device = file;
    if (!d->device->isOpen())
        d->device->open(QIODevice::WriteOnly);
    d->deleteDevice = true;
    d->format = format;
}

QImageWriter::~QImageWriter()
{
    delete d;
}

void QImageWriter::setFormat(const QByteArray &format)
{
    d->format = format;
}

QByteArray QImageWriter::format() const
{
    return d->format;
}

void QImageWriter::setDevice(QIODevice *device)
{
    if (d->device && d->deleteDevice)
        delete d->device;

    d->device = device;
    if (!d->device->isOpen())
        d->device->open(QIODevice::WriteOnly);
    d->deleteDevice = false;
    delete d->handler;
    d->handler = 0;
}

QIODevice *QImageWriter::device() const
{
    return d->device;
}

void QImageWriter::setFileName(const QString &fileName)
{
    setDevice(new QFile(fileName));
    d->deleteDevice = true;
}

QString QImageWriter::fileName() const
{
    QFile *file = qt_cast<QFile *>(d->device);
    return file ? file->fileName() : QString();
}

void QImageWriter::setQuality(int quality)
{
    d->quality = quality;
}

int QImageWriter::quality() const
{
    return d->quality;
}

void QImageWriter::setGamma(float gamma)
{
    d->gamma = gamma;
}

float QImageWriter::gamma() const
{
    return d->gamma;
}

void QImageWriter::setDescription(const QString &description)
{
    d->description = description;
}

QString QImageWriter::description() const
{
    return d->description;
}

void QImageWriter::setParameters(const QByteArray &parameters)
{
    d->parameters = parameters;
}

QByteArray QImageWriter::parameters() const
{
    return d->parameters;
}

bool QImageWriter::canWrite() const
{
    if (!d->handler && (d->handler = ::createWriteHandler(d->device, d->format)) == 0) {
        d->imageWriterError = QImageWriter::UnsupportedFormatError;
        d->errorString = QT_TRANSLATE_NOOP(QImageWriter, "Unsupported image format");
        return false;
    }
    return true;
}

bool QImageWriter::write(const QImage &image)
{
    if (!d->handler && (d->handler = ::createWriteHandler(d->device, d->format)) == 0) {
        d->imageWriterError = QImageWriter::UnsupportedFormatError;
        d->errorString = QT_TRANSLATE_NOOP(QImageWriter, "Unsupported image format");
        return false;
    }

    if (d->handler->supportsOption(QImageIOHandler::Quality))
        d->handler->setOption(QImageIOHandler::Quality, d->quality);
    if (d->handler->supportsOption(QImageIOHandler::Gamma))
        d->handler->setOption(QImageIOHandler::Gamma, d->gamma);
    if (d->handler->supportsOption(QImageIOHandler::Description))
        d->handler->setOption(QImageIOHandler::Description, d->description);
    if (d->handler->supportsOption(QImageIOHandler::Parameters))
        d->handler->setOption(QImageIOHandler::Parameters, d->parameters);

    return d->handler->write(image);
}

QImageWriter::ImageWriterError QImageWriter::error() const
{
    return d->imageWriterError;
}

QString QImageWriter::errorString() const
{
    return d->errorString;
}

QList<QByteArray> QImageWriter::supportedImageFormats()
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
        if ((plugin->capabilities(0, keys.at(i).toLatin1()) & QImageIOPlugin::CanWrite) != 0)
            formats << keys.at(i).toLatin1();
    }

    qHeapSort(formats);
    return formats;
}

