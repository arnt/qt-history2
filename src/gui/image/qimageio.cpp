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

    \brief The QImageIO provides an interface for loading and saving
    images to a QIODevice.

    QImageIO provides a comprehensive interface for working with image
    data. If you only want to load an image from a file or a resource,
    you can use QImage and QPixmap directly. QImageIO adds the benefit
    of setting format specific parameters before loading images, and
    it also allows you to save images. Calling inputFormats() will
    return a list of supported input formats, and outputFormats()
    returns a list of supported output formats.

    If you want to save an image, first create a QImageIO object by
    passing both a pointer to an open QIODevice (e.g., a QFile or a
    QBuffer) and format string to QImageIO's constructor. Then set the
    format specific parameters through QImageIO's many property
    functions, and finally call save(). If you want to load an image,
    call load(). Example:

    \code
        bool saveImageAsJpeg(const QImage &image, const QIODevice *device)
        {
            QImageIO imageIO(device, "jpeg");
            imageIO.setQuality(70);
            if (!imageIO.save()) {
                qDebug() << "Failed to save the image: " << imageIO.errorString();
                return false;
            }
            return true;
        }
    \endcode

    The errorString() function returns a human readable description of
    the last error that occurred. error() returns the type of error as
    a QImageIO::Error value.

    In addition to the image formats provided by Qt, QImageIO uses
    plugins to support a wide variety of image formats. QImageIO
    plugins inherit QImageIOPlugin, and all plugins are loaded from
    the library search path when the application starts. A
    QImageIOPlugin can support either loading or saving, or both. For
    example, one plugin may only support loading of GIF images, while
    another supports saving. When several plugins support the same
    features, QImageIO will pick one at random.

    QImageIO supports several property functions for checking the
    properties of an image. This is particularily useful, as certain
    image formats support fetching properties from image data without
    actually loading the image data. For example, you can find the
    size of an image by calling size(), and depending on the image
    format implementation, this can be a relatively inexpensive
    operation compared to that of loading the whole image and then
    checking its size.

    In addition, certain properties can be set before loading an
    image, and this may affect the image that is loaded. For example,
    you can load an image into a specific size or quality, or you can
    load only a section of an image by calling setRegion(). Example:

    \code
        QImageIO io("geodata.png");
        io.setRegion(QRect(2000, 2000, 2500, 2500));
        io.load();
        QImage image = io.image(); // a 500x500 image from position 2000x2000
    \endcode

    If an image format has no native support for a particular
    property, QImageIO will provide a base implementation. For
    example, it may load the entire image from the image data, save a
    section of the image and then discard the rest.

    Certain image formats, such as GIF and MNG, support
    animation. QImageIO provides convenient functions for loading the
    frames in sequence. To load the next frame, call
    load(). frameCount() returns the number of frames in the
    animation. nextFrameDelay() returns the number of milliseconds of
    delay before the next frame of the animation should be
    loaded. currentFrameNumber() returns the sequence number of the
    current frame, and loopCount() returns the number of times the
    animation should loop.

    \sa QImageIOHandler, QImageIOPlugin, QImage, QPixmap
*/

/*! \enum QImageIO::ImageFormatError

    This enum describes the types of errors that can occur when
    loading or saving images with QImageIO.

    \value UnsupportedImageFormat The image format is unsupported. Qt
    does not support the format natively, and no plugins were found
    that support the image format.

    \value InvalidImageData The image format is supported, but the
    image data is valid (i.e., corrupted) in such a way that it is
    impossible to load an image from it.

    \value UnknownError An unknown error, such as an internal QImageIO
    error or a system error, occurred.

    \sa QIODevice::error(), QIODevice::errorString()
*/

#include <qbytearray.h>
#include <qcoreapplication.h>
#include <qfileinfo.h>
#include <qiodevice.h>
#include <qimage.h>
#include <qfile.h>
#include <qlist.h>
#include <qrect.h>
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

static QImageIOHandler *createHandler(QIODevice *device, const QByteArray &format,
                                      QImageIOPlugin::Capabilities capabilities)
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
            if ((plugin->capabilities(device, form) & capabilities) == capabilities) {
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
    QRect region;

    // errors
    QImageIO::ImageFormatError error;
    QString errorString;

    QImageIO *q;
};

/*! \internal
    Constructs a QImageIOPrivate.
*/
QImageIOPrivate::QImageIOPrivate(QImageIO *q_ptr)
{
    q = q_ptr;
    device = 0;
    deleteDevice = false;
    handler = 0;

    gamma = 0.0;
    quality = 0;
}

/*! \internal
    Destructs a QImageIOPrivate.
*/
QImageIOPrivate::~QImageIOPrivate()
{
    if (deleteDevice)
        delete device;
    delete handler;
}

/*!
    Constructs a QImageIO object. Before calling load() or save(), you
    must call setDevice(), setFormat() or setImage(). Calling reset()
    brings the object back to this state.

    \sa setDevice(), setFormat(), setImage()
*/
QImageIO::QImageIO()
    : d(new QImageIOPrivate(this))
{
}

/*!
    Constructs a QImageIO object with the device \a device and using
    the format \a format. \a format is a textual (case-insensitive)
    representation of the image format you want to use. See
    setFormat() for a table of supported image formats.
*/
QImageIO::QImageIO(QIODevice *device, const QByteArray &format)
    : d(new QImageIOPrivate(this))
{
    d->device = device;
    d->format = format;
}

/*!
    Creates a QFile by opening \a fileName in \l ReadWrite mode. \a
    format is a textual (case-insensitive) representation of the image
    format you want to use. See setFormat() for a table of supported
    image formats.
*/
QImageIO::QImageIO(const QString &fileName, const QByteArray &format)
    : d(new QImageIOPrivate(this))
{
    d->device = new QFile(fileName);
    d->fileName = fileName;
    d->deleteDevice = true;
    d->format = format;
}

/*!
    Destructs the QImageIO object.
*/
QImageIO::~QImageIO()
{
    delete d;
}

/*!
    Resets the QImageIO object. Calling this function brings QImageIO
    back to its constructed state.
*/
void QImageIO::reset()
{
    delete d;
    d = new QImageIOPrivate(this);
}

/*!
    Sets the image QImageIO will save to \a image.

    \sa image(), save()
*/
void QImageIO::setImage(const QImage &image)
{
    d->image = image;
}

/*!
    Returns the image QImageIO is currently operating on, or the last
    image that was loaded.

    \sa setImage(), load()
*/
QImage QImageIO::image() const
{
    return d->image;
}

/*!
    Sets the name of the image format that QImageIO should use when
    loading and saving images to \a format. QImageIO has built-in
    support for a number of image formats, and some are provided as
    plugins:

    \table
    \header \i Format \i Type     \i Features    \i Support  \i Remarks
    \row    \i PNG    \i Built-in \i Load & Save \i Optional \i System library or provided with Qt
    \row    \i JPEG   \i Plugin   \i Load & Save \i Optional \i System library or provided with Qt
    \row    \i GIF    \i Plugin   \i Load        \i Optional \i Subject to licensing ###
    \row    \i BMP    \i Built-in \i Load & Save \i Embedded \i
    \row    \i PPM    \i Built-in \i Load & Save \i Embedded \i
    \row    \i PBM    \i Built-in \i Load & Save \i Embedded \i
    \row    \i XPM    \i Built-in \i Load & Save \i Embedded \i
    \row    \i XBM    \i Built-in \i Load & Save \i Embedded \i
    \endtable

    Additional formats can be added as plugins, through QImageIOPlugin.

    \sa format()
*/
void QImageIO::setFormat(const QByteArray &format)
{
    d->format = format;
}

/*!
    Returns the format that is currently used by QImageIO.

    \sa setFormat()
*/
QByteArray QImageIO::format() const
{
    return d->format;
}

/*!
    Sets the device that QImageIO loads or saves image data to to \a
    device.

    \sa device(), load(), save()
*/
void QImageIO::setDevice(QIODevice *device)
{
    d->device = device;
    delete d->handler;
    d->handler = 0;
}

/*!
    Returns the device that QImageIO loads or saves image data to, or
    0 if no device is currently in use.

    \sa setDevice()
*/
QIODevice *QImageIO::device() const
{
    return d->device;
}

/*!
    This is a convenience function that creates a QFile internally,
    and assigns the name \a fileName to it.
*/
void QImageIO::setFileName(const QString &fileName)
{
    if (d->device && d->deleteDevice)
        delete d->device;

    d->device = new QFile(fileName);
    d->deleteDevice = true;
    d->fileName = fileName;
    delete d->handler;
    d->handler = 0;
}

/*!
    Returns the file name that QImageIO is currently using for loading
    and saving image data.

    \sa setFileName()
*/
QString QImageIO::fileName() const
{
    return d->fileName;
}

/*!
    Sets the quality of the image to \a quality. The value range of \a
    quality depends on the image format. For example, JPEG uses the
    range 0-100, where 100 gives good quality with low compression,
    and 0 gives low quality with high compression.
*/
void QImageIO::setQuality(int quality)
{
    d->quality = quality;
}

/*!
    Returns the quality of the image as an integer value.

    \sa setQuality()
*/
int QImageIO::quality() const
{
    return d->quality;
}

/*!
    Sets the resolution of the image to \a resolution. The value of \a
    resolution depends on the image format.

    \sa resolution()
*/
void QImageIO::setResolution(const QSize &resolution)
{
    d->resolution = resolution;
}

/*!
    Returns the resolution of the image.

    \sa setResolution()
*/
QSize QImageIO::resolution() const
{
    return d->resolution;
}

/*!
    Sets a region of the image to be saved to or loaded from the image
    data. This function can be useful when saving or loading a smaller
    section of an image from a huge set of image data.

    \sa region()
*/
void QImageIO::setRegion(const QRect &region)
{
    d->region = region;
}

/*!
    Returns the region that QImageIO will save to or load from the
    image data.

    \sa setRegion()
*/
QRect QImageIO::region() const
{
    return d->region;
}

/*!
    Sets the description of an image to \a description. The value and
    use of the data depends on the image format.

    \sa description()
*/
void QImageIO::setDescription(const QString &description)
{
    d->description = description;
}

/*!
    Returns the description of the image, if supported by the image
    format, or an empty string if the current format does not support
    it.

    \sa setDescription()
*/
QString QImageIO::description() const
{
    return d->description;
}

/*!
    Sets the parameters of an image to \a parameters. The value and
    use of this data depends on the image format.

    \sa parameters()
*/
void QImageIO::setParameters(const QByteArray &parameters)
{
    d->parameters = parameters;
}

/*!
    Returns the parameters of an image.

    \sa setParameters()
*/
QByteArray QImageIO::parameters() const
{
    return d->parameters;
}

/*!
    Sets the gamma value of an image to \a gamma. The value and use of
    \a gamma depends on the image format.

    \sa gamma()
*/
void QImageIO::setGamma(float gamma)
{
    d->gamma = gamma;
}

/*!
    Returns the gamma value of the current image.

    \sa setGamma()
*/
float QImageIO::gamma() const
{
    return d->gamma;
}

/*!
    Sets the size of the image QImageIO will load from or save to the
    image data to \a size. Regardless of the original image
    dimensions, QImageIO will load or save an image of size \a
    size. If necessary, QImageIO will scale the image to the given
    size.

    \sa size()
*/
void QImageIO::setSize(const QSize &size)
{
    d->size = size;
}

/*!
    Returns the size of the current image.

    \sa setSize()
*/
QSize QImageIO::size() const
{
    return d->size;
}

/*!
    Loads an image from the current device. Returns true on success;
    otherwise false, in which case error() will return the type of
    error that occurred.

    If the image data contains an animated image format, this function
    will load the "next" frame in the animation, starting at frame 1.

    \sa save(), frameCount(), nextFrameDelay()
*/
bool QImageIO::load()
{
    if (!d->device) {
        qWarning("QImageIO::load() called with no device");
        return false;
    }

    if (!d->deleteDevice && !d->device->isOpen()) {
        qWarning("QImageIO::load() called with closed device");
        return false;
    }

    if (d->deleteDevice && !d->device->isOpen() && !d->device->open(QIODevice::ReadOnly)) {
        QList<QByteArray> extensions = inputFormats();
        int currentExtension = 0;

        QFile *file = static_cast<QFile *>(d->device);

        do {
            file->setFileName(d->fileName + QLatin1Char('.') + extensions.at(currentExtension++));
            file->open(QIODevice::ReadOnly);
        } while (!file->isOpen() && currentExtension < extensions.size());

        if (!d->device->isOpen()) {
            d->error = FileNotFound;
            d->errorString = QT_TRANSLATE_NOOP(QImageIO, "File not found");
            return false;
        }
    }

    if (!d->handler)
        d->handler = createHandler(d->device, d->format, QImageIOPlugin::CanLoad);

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
    handler->setProperty(QImageIOHandler::Region, qVariant(d->region));
    handler->setProperty(QImageIOHandler::Size, d->size);

    if (!handler->load(&d->image)) {
        // ### should the handler have an error enum?
        d->error = InvalidImageData;
        d->errorString = QT_TRANSLATE_NOOP(QIMageIO, "Invalid image data");
        return false;
    }

    if (!handler->supportsProperty(QImageIOHandler::Region) && d->region.isValid())
        d->image = d->image.copy(d->region);
    if (!handler->supportsProperty(QImageIOHandler::Size) && d->size.isValid())
        d->image = d->image.scale(d->size);

    return true;
}

/*!
    Saves the current image to the device. Returns true on success;
    otherwise returns false, in which case error() will return the
    type of error that occurred.

    If the image format supports animation, this function will save
    the "next" frame in the animation, starting at frame 1.
*/
bool QImageIO::save()
{
    if (!d->device) {
        qWarning("QImageIO::save() called with no device");
        d->error = DeviceError;
        d->errorString = QT_TRANSLATE_NOOP(QImageIO, "No device");
        return false;
    }

    if (!d->deleteDevice && !d->device->isOpen()) {
        qWarning("QImageIO::save() called with closed device");
        d->error = DeviceError;
        d->errorString = QT_TRANSLATE_NOOP(QImageIO, "Device not open");
        return false;
    }

    if (d->deleteDevice && !d->device->open(QIODevice::WriteOnly)) {
        d->error = DeviceError;
        d->errorString = QT_TRANSLATE_NOOP(QImageIO, "Unable to write to the device");
        return false;
    }

    if (!d->handler)
        d->handler = createHandler(d->device, d->format,
                                   QImageIOPlugin::CanSave);

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
    handler->setProperty(QImageIOHandler::Region, qVariant(d->region));
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

/*!
    If the current image format supports animation, this function
    returns the number of frames in the animation. Otherwise, 0 is
    returned.
*/
int QImageIO::frameCount() const
{
    return d->handler ? d->handler->frameCount() : 0;
}

/*!
    If the current image format supports animation, this function
    returns the number of milliseconds to wait until showing the next
    frame in the animation. Otherwise, 0 is returned.
*/
int QImageIO::nextFrameDelay() const
{
    return d->handler ? d->handler->nextFrameDelay() : 0;
}

/*!
    If the current image format supports animation, this function
    returns the sequence number of the current animation
    frame. Otherwise, 0 is returned.
*/
int QImageIO::currentFrameNumber() const
{
    return d->handler ? d->handler->currentFrameNumber() : 0;
}

/*!
    If the current image format supports animation, this function
    returns the number of times the animation should loop. Otherwise,
    0 is returned.
*/
int QImageIO::loopCount() const
{
    return d->handler ? d->handler->loopCount() : 0;
}

/*!
    Returns true if there is a new frame available for loading;
    otherwise returns false.
*/
bool QImageIO::hasNextFrame() const
{
    // ### implement
    return true;
}

/*!
    Returns the type of error that occurred as a result of load() or
    save() returning false.

    \sa ImageFormatError, errorString()
*/
QImageIO::ImageFormatError QImageIO::error() const
{
    return d->error;
}

/*!
    Returns the type of error that occurred as a result of load() or
    save() returning false, as a human readable string.

    \sa error()
*/
QString QImageIO::errorString() const
{
    return d->errorString;
}

/*!
    Attempts to load a file with the name \a fileName, then attempts
    to find the type of image format stored in this file. On success,
    the image format is returned; otherwise this function returns an
    empty QByteArray().
*/
QByteArray QImageIO::imageFormatForFileName(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return QByteArray();

    return imageFormatForDevice(&file);
}

/*!
    Attempts to read data from \a device in order to find the type of
    image format stored in the device. On success, the image format is
    returned; otherwise this function returns an empty QByteArray().

    Unless irrecoverable errors occurred when reading from \a device,
    its state is restored when this function returns.
*/
QByteArray QImageIO::imageFormatForDevice(QIODevice *device)
{
    QImageIOHandler *handler = createHandler(device, QByteArray(), QImageIOPlugin::CanLoad);
    QByteArray format = handler ? handler->name() : QByteArray();
    delete handler;
    return format;
}

/*!
    Returns the list of image formats that can be loaded by QImageIO.
*/
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

/*!
    Returns the list of image formats that can be saved by QImageIO.
*/
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
