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

/*!
    \class QImageWriter
    \brief The QImageWriter class provides a format independent interface
    for writing images to files or other devices.

    \reentrant
    \ingroup multimedia
    \ingroup io

    QImageWriter supports setting format specific options, such as the
    gamma level, compression level and quality, prior to storing the
    image. If you do not need such options, you can use QImage::save()
    or QPixmap::save() instead.

    To store an image, you start by constructing a QImageWriter
    object.  Pass either a file name or a device pointer, and the
    image format to QImageWriter's constructor. You can then set
    several options, such as the gamma level (by calling setGamma())
    and quality (by calling setQuality()). canWrite() returns true if
    QImageWriter can write the image (i.e., the image format is
    supported and the device is open for writing). Call write() to
    write the image to the device.

    If any error occurs when writing the image, write() will return
    false. You can then call error() to find the type of error that
    occurred, of errorString() to get a human readable description of
    what went wrong.

    Call supportedImageFormats() for a list of formats that
    QImageWriter can write. QImageWriter supports all built-in image
    formats, in addition to any image format plugins that support
    writing.

    \sa QImageReader, QImageIOHandler, QImageIOPlugin
*/

/*!
    \enum QImageWriter::ImageWriterError

    This enum describes errors that can occur when writing images with
    QImageWriter.

    \value DeviceError QImageWriter encountered a device error when
    writing the image data. Consult your device for more details on
    what went wrong.

    \value UnsupportedFormatError Qt does not support the requested
    image format.

    \value UnknownError An unknown error occurred. If you get this
    value after calling write(), it is most likely caused by a bug in
    QImageWriter.
*/

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
            QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(keys.at(i)));
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

    // error
    QImageWriter::ImageWriterError imageWriterError;
    QString errorString;

    QImageWriter *q;
};

/*!
    \internal
*/
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

/*!
    Constructs an empty QImageWriter object. Before writing, you must
    call setFormat() to set an image format, then setDevice() or
    setFileName().
*/
QImageWriter::QImageWriter()
    : d(new QImageWriterPrivate(this))
{
}

/*!
    Constructs a QImageWriter object using the device \a device and
    image format \a format.
*/
QImageWriter::QImageWriter(QIODevice *device, const QByteArray &format)
    : d(new QImageWriterPrivate(this))
{
    d->device = device;
    d->format = format;
    if (!d->device->isOpen())
        d->device->open(QIODevice::WriteOnly);
}

/*!
    Constructs a QImageWriter objects that will write to a file with
    the name \a fileName, using the image format \a format. If \a
    format is not provided, QImageWriter will detect the image format
    by inspecting the extension of \a fileName.
*/
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

/*!
    Destructs the QImageWriter object.
*/
QImageWriter::~QImageWriter()
{
    if (d->deleteDevice)
        delete d->device;
    delete d->handler;
    delete d;
}

/*!
    Sets the format QImageWriter will use when writing images, to \a
    format. \a format is a case insensitive text string. Example:

    \code
        QImageWriter writer;
        writer.setFormat("png"); // same as writer.setFormat("PNG");
    \endcode

    You can call supportedImageFormats() for the full list of formats
    QImageWriter supports.

    \sa format()
*/
void QImageWriter::setFormat(const QByteArray &format)
{
    d->format = format;
}

/*!
    Returns the format QImageWriter uses for writing images.

    \sa setFormat()
*/
QByteArray QImageWriter::format() const
{
    return d->format;
}

/*!
    Sets QImageWriter's device to \a device. If a device has already
    been set, the old device is removed from QImageWriter and is
    otherwise left unchanged.

    If the device is not already open, QImageWriter will attempt to
    open the device in \l QIODevice::WriteOnly mode by calling
    open(). Note that this does not work for certain devices, such as
    QProcess, QTcpSocket and QUdpSocket, where more logic is required
    to open the device.

    \sa device(), setFileName()
*/
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

/*!
    Returns the device currently assigned to QImageWriter, or 0 if no
    device has been assigned.
*/
QIODevice *QImageWriter::device() const
{
    return d->device;
}

/*!
    Sets the file name of QImageWriter to \a fileName. Internally,
    QImageWriter will create a QFile and open it in \l
    QIODevice::WriteOnly mode, and use this file when writing images.

    \sa fileName(), setDevice()
*/
void QImageWriter::setFileName(const QString &fileName)
{
    setDevice(new QFile(fileName));
    d->deleteDevice = true;
}

/*!
    If the currently assigned device is a QFile, or if setFileName()
    has been called, this function returns the name of the file
    QImageWriter writes to. Otherwise (i.e., if no device has been
    assigned or the device is not a QFile), an empty QString is
    returned.

    \sa setFileName(), setDevice()
*/
QString QImageWriter::fileName() const
{
    QFile *file = qobject_cast<QFile *>(d->device);
    return file ? file->fileName() : QString();
}

/*!
    This is an image format specific function that sets the quality
    level of the image to \a quality. For image formats that do not
    support setting the quality, this value is ignored.

    The value range of \a quality depends on the image format. For
    example, the "jpeg" format supports a quality range from 0 (low
    quality, high compression) to 100 (high quality, low compression).

    \sa quality()
*/
void QImageWriter::setQuality(int quality)
{
    d->quality = quality;
}

/*!
    Returns the quality level of the image.

    \sa setQuality()
*/
int QImageWriter::quality() const
{
    return d->quality;
}

/*!
    This is an image format specific function that sets the gamma
    level of the image to \a gamma. For image formats that do not
    support setting the gamma level, this value is ignored.

    The value range of \a gamma depends on the image format. For
    example, the "png" format supports a gamma range from 0.0 to 1.0.

    \sa quality()
*/
void QImageWriter::setGamma(float gamma)
{
    d->gamma = gamma;
}

/*!
    Returns the gamma level of the image.

    \sa setGamma()
*/
float QImageWriter::gamma() const
{
    return d->gamma;
}

/*!
    This is an image format specific function that sets the
    description of the image to \a description. For image formats that
    do not support setting the description, this value is ignored.

    The contents of \a description depends on the image format.

    \sa description()
*/
void QImageWriter::setDescription(const QString &description)
{
    d->description = description;
}

/*!
    Returns the description of the image.

    \sa setDescription()
*/
QString QImageWriter::description() const
{
    return d->description;
}

/*!
    Returns true if QImageWriter can write the image (i.e., the image
    format is supported and the assigned device is open for reading.

    \sa write(), setDevice(), setFormat()
*/
bool QImageWriter::canWrite() const
{
    if (!d->handler && (d->handler = ::createWriteHandler(d->device, d->format)) == 0) {
        d->imageWriterError = QImageWriter::UnsupportedFormatError;
        d->errorString = QT_TRANSLATE_NOOP(QImageWriter, "Unsupported image format");
        return false;
    }
    return true;
}

/*!
    Writes the image \a image to the assigned device or file
    name. Returns true on success; otherwise returns false. If the
    operation fails, you can call error() to find the type of error
    that occurred, or errorString() to get a human readable
    description of the error.

    \sa canWrite(), error(), errorString()
*/
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

    return d->handler->write(image);
}

/*!
    Returns the type of error that last occurred.

    \sa ImageWriterError, errorString()
*/
QImageWriter::ImageWriterError QImageWriter::error() const
{
    return d->imageWriterError;
}

/*!
    Returns a human readable description of the last error that occurred.

    \sa error()
*/
QString QImageWriter::errorString() const
{
    return d->errorString;
}

/*!
    Returns the list of image formats supported by QImageWriter.

    \sa setFormat(), QImageReader::supportedImageFormats()
*/
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
        QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(keys.at(i)));
        if ((plugin->capabilities(0, keys.at(i).toLatin1()) & QImageIOPlugin::CanWrite) != 0)
            formats << keys.at(i).toLatin1();
    }

    qSort(formats);
    return formats;
}

