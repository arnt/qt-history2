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
    \class QImageReader
    \brief The QImageReader class provides a format independent interface
    for reading images from files or other devices.

    \reentrant
    \ingroup multimedia
    \ingroup io

    The most common way to read images is through QImage and QPixmap's
    constructors, or by calling QImage::load() and
    QPixmap::load(). QImageReader is a specialized class which gives
    you more control when reading images. For example, you can read an
    image into a specific size by calling setScaledSize(), and you can
    select a clip rect, effectively loading only parts of an image, by
    calling setClipRect(). Depending on the underlying support in the
    image format, this can save memory and speed up loading of images.

    To read an image, you start by constructing a QImageReader object.
    Pass either a file name or a device pointer, and the image format
    to QImageReader's constructor. You can then set several options,
    such as the clip rect (by calling setClipRect()) and scaled size
    (by calling setScaledSize()). canRead() returns the image if the
    QImageReader can read the image (i.e., the image format is
    supported and the device is open for reading). Call read() to read
    the image.

    If any error occurs when reading the image, read() will return a
    null QImage. You can then call error() to find the type of error
    that occurred, of errorString() to get a human readable
    description of what went wrong.

    Call supportedImageFormats() for a list of formats that
    QImageReader can read. QImageReader supports all built-in image
    formats, in addition to any image format plugins that support
    reading.

    \sa QImageReader, QImageIOHandler, QImageIOPlugin
*/

/*!
    \enum QImageReader::ImageReaderError

    This enum describes the different types of errors that can occur
    when reading images with QImageReader.

    \value FileNotFoundError QImageReader was used with a file name,
    but not file was found with that name. This can also happen if the
    file name contained no extension, and the file with the correct
    extension is not supported by Qt.

    \value DeviceError QImageReader encountered a device error when
    reading the image. You can consult your particular device for more
    details on what went wrong.

    \value UnsupportedFormatError Qt does not support the requested
    image format.

    \value InvalidDataError The image data was invalid, and
    QImageReader was unable to read an image from it. The can happen
    if the image file is damaged.

    \value UnknownError An unknown error occurred. If you get this
    value after calling read(), it is most likely caused by a bug in
    QImageReader.
*/
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
        QByteArray subType;
#ifndef QT_NO_IMAGEIO_PNG
        if (QPngHandler::canRead(device)) {
            handler = new QPngHandler;
        } else
#endif
        if (QBmpHandler::canRead(device)) {
            handler = new QBmpHandler;
        } else if (QXpmHandler::canRead(device)) {
            handler = new QXpmHandler;
        } else if (QPpmHandler::canRead(device, &subType)) {
            handler = new QPpmHandler;
            handler->setOption(QImageIOHandler::SubType, subType);
        } else if (QXbmHandler::canRead(device)) {
            handler = new QXbmHandler;
        }
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
            QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(keys.at(i)));
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

/*!
    \internal
*/
QImageReaderPrivate::QImageReaderPrivate(QImageReader *qq)
{
    device = 0;
    deleteDevice = false;
    handler = 0;
    imageReaderError = QImageReader::UnknownError;
    errorString = QT_TRANSLATE_NOOP(QImageReader, "Unknown error");

    q = qq;
}

/*!
    \internal
*/
QImageReaderPrivate::~QImageReaderPrivate()
{
    if (deleteDevice)
        delete device;
}

/*!
    \internal
*/
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

/*!
    Constructs an empty QImageReader object. Before reading an image,
    call setDevice() or setFileName().
*/
QImageReader::QImageReader()
    : d(new QImageReaderPrivate(this))
{
}

/*!
    Constructs a QImageReader object with the device \a device and the
    image format \a format.
*/
QImageReader::QImageReader(QIODevice *device, const QByteArray &format)
    : d(new QImageReaderPrivate(this))
{
    d->device = device;
    d->format = format;
}

/*!
    Constructs a QImageReader object with the file name \a fileName
    and the image format \a format.

    \sa setFileName()
*/
QImageReader::QImageReader(const QString &fileName, const QByteArray &format)
    : d(new QImageReaderPrivate(this))
{
    QFile *file = new QFile(fileName);
    d->device = file;
    d->deleteDevice = true;
    d->format = format;
}

/*!
    Destructs the QImageReader object.
*/
QImageReader::~QImageReader()
{
    delete d;
}

/*!
    Sets the format QImageReader will use when reading images, to \a
    format. \a format is a case insensitive text string. Example:

    \code
        QImageReader reader;
        reader.setFormat("png"); // same as reader.setFormat("PNG");
    \endcode

    You can call supportedImageFormats() for the full list of formats
    QImageReader supports.

    \sa format()
*/
void QImageReader::setFormat(const QByteArray &format)
{
    d->format = format;
}

/*!
    Returns the format QImageReader uses for reading images.

    \sa setFormat()
*/
QByteArray QImageReader::format() const
{
    return d->format;
}

/*!
    Sets QImageReader's device to \a device. If a device has already
    been set, the old device is removed from QImageReader and is
    otherwise left unchanged.

    If the device is not already open, QImageReader will attempt to
    open the device in \l QIODevice::ReadOnly mode by calling
    open(). Note that this does not work for certain devices, such as
    QProcess, QTcpSocket and QUdpSocket, where more logic is required
    to open the device.

    \sa device(), setFileName()
*/
void QImageReader::setDevice(QIODevice *device)
{
    if (d->device && d->deleteDevice)
        delete d->device;
    d->device = device;
    d->deleteDevice = false;
    delete d->handler;
    d->handler = 0;
}

/*!
    Returns the device currently assigned to QImageReader, or 0 if no
    device has been assigned.
*/
QIODevice *QImageReader::device() const
{
    return d->device;
}

/*!
    Sets the file name of QImageReader to \a fileName. Internally,
    QImageWriter will create a QFile and open it in \l
    QIODevice::ReadOnly mode, and use this file when writing images.

    \a fileName may or may not include the file extension (i.e., .png
    or .bmp). If no extension is provided, QImageReader will choose
    the first file it finds with an extension that matches an image
    format that it supports.

    \sa fileName(), setDevice(), supportedImageFormats()
*/
void QImageReader::setFileName(const QString &fileName)
{
    setDevice(new QFile(fileName));
    d->deleteDevice = true;
}

/*!
    If the currently assigned device is a QFile, or if setFileName()
    has been called, this function returns the name of the file
    QImageReader reads from. Otherwise (i.e., if no device has been
    assigned or the device is not a QFile), an empty QString is
    returned.

    \sa setFileName(), setDevice()
*/
QString QImageReader::fileName() const
{
    QFile *file = qobject_cast<QFile *>(d->device);
    return file ? file->fileName() : QString();
}

/*!
    Returns the size of the image. This function may or may not read
    the entire image, depending on whether the image format supports
    checking the size of an image before loading it.
*/
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

/*!
    Sets the image clip rect (also known as the ROI, or Region Of
    Interest) to \a rect. The coordinates of \a rect are relative to
    the untransformed image size, as returned by size().

    \sa clipRect(), setScaledSize(), setScaledClipRect()
*/
void QImageReader::setClipRect(const QRect &rect)
{
    d->clipRect = rect;
}

/*!
    Returns the clip rect (also known as the ROI, or Region Of
    Interest) of the image. If no clip rect has been set, an invalid
    QRect is returned.

    \sa setClipRect()
*/
QRect QImageReader::clipRect() const
{
    return d->clipRect;
}

/*!
    Sets the scaled size of the image to \a size. The scaling is
    performed after the initial clip rect, but before the scaled clip
    rect is applied. The algorithm used for scaling depends on the
    image format. By default (i.e., if the image format does not
    support scaling), QImageReader will use QImage::scale() with \l
    SmoothScaling.

    \sa scaledSize(), setClipRect(), setScaledClipRect()
*/
void QImageReader::setScaledSize(const QSize &size)
{
    d->scaledSize = size;
}

/*!
    Returns the scaled size of the image.

    \sa setScaledSize()
*/
QSize QImageReader::scaledSize() const
{
    return d->scaledSize;
}

/*!
    Sets the scaled clip rect to \a rect. The scaled clip rect is the
    clip rect (also known as ROI, or Region Of Interest) that is
    applied after the image has been scaled.

    \sa scaledClipRect(), setScaledSize()
*/
void QImageReader::setScaledClipRect(const QRect &rect)
{
    d->scaledClipRect = rect;
}

/*!
    Returns the scaled clip rect of the image.

    \sa setScaledClipRect()
*/
QRect QImageReader::scaledClipRect() const
{
    return d->scaledClipRect;
}

/*!
    Returns true if an image can be read for the device (i.e., the
    image format is supported, and the device seems to contain valid
    data); otherwise returns false.

    canRead() is a lightweight function that only does a quick test to
    see if the image data is valid. read() may still return false
    after canRead() returns true, if the image data is corrupt.

    \sa read(), supportedImageFormats()
*/
bool QImageReader::canRead() const
{
    if (!d->initHandler())
        return false;

    return d->handler->canRead();
}

/*!
    Reads an image from the device. On success, the image that was
    read is returned; otherwise, a null QImage is returned. You can
    then call error() to find the type of error that occurred, or
    errorString() to get a human readable description of the error.

    For image formats that support animation, calling read()
    repeatedly will return the next frame (or use QMovie).

    \sa canRead(), supportedImageFormats()
*/
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
                    image = image.scaled(d->scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
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
                    image = image.scaled(d->scaledSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                if (d->scaledClipRect.isValid())
                    image = image.copy(d->scaledClipRect);
            }
        }
    }

    return image;
}

/*!
   For image formats that support animation, this function steps over the
   current image.

   The default implementation calls read(), and then discards the resulting
   image, but the image handler may have a more efficient way of implementing
   this operation.

   \sa jumpToImage(), QImageIOHandler::jumpToNextImage()
*/
bool QImageReader::jumpToNextImage()
{
    if (!d->initHandler())
        return false;
    return d->handler->jumpToNextImage();
}

/*!
   For image formats that support animation, this function skips to the image
   whose sequence number is \a imageNumber. The next call to read() will
   attempt to read this image.

   \sa jumpToNextImage(), QImageIOHandler::jumpToImage()
*/
bool QImageReader::jumpToImage(int imageNumber)
{
    if (!d->initHandler())
        return false;
    return d->handler->jumpToImage(imageNumber);
}

/*!
    For image formats that support animation, this function returns
    the number of times the animation should loop. Otherwise, it
    returns -1.
*/
int QImageReader::loopCount() const
{
    if (!d->initHandler())
        return -1;
    return d->handler->loopCount();
}

/*!
    For image formats that support animation, this function returns
    the number of images in the animation. Otherwise, -1 is returned.

    Certain animation formats do not support this feature, in which
    case -1 is returned.
*/
int QImageReader::imageCount() const
{
    if (!d->initHandler())
        return -1;
    return d->handler->imageCount();
}

/*!
    For image formats that support animation, this function returns
    the number of milliseconds to wait until displaying the next frame
    in the animation. Otherwise, -1 is returned.
*/
int QImageReader::nextImageDelay() const
{
    if (!d->initHandler())
        return -1;
    return d->handler->nextImageDelay();
}

/*!
    For image formats that support animation, this function returns
    the sequence number of the current frame. Otherwise, -1 is
    returned.
*/
int QImageReader::currentImageNumber() const
{
    if (!d->initHandler())
        return -1;
    return d->handler->currentImageNumber();
}

/*!
    For image formats that support animation, this function returns
    the rect for the current frame. Otherwise, -1 is returned.
*/
QRect QImageReader::currentImageRect() const
{
    if (!d->initHandler())
        return QRect();
    return d->handler->currentImageRect();
}

/*!
    Returns the type of error that occurred last.

    \sa ImageReaderError, errorString()
*/
QImageReader::ImageReaderError QImageReader::error() const
{
    return d->imageReaderError;
}

/*!
    Returns a human readable description of the last error that
    occurred.

    \sa error()
*/
QString QImageReader::errorString() const
{
    return d->errorString;
}

/*!
    If supported, this function returns the image format of the file
    \a fileName. Otherwise, an empty string is returned.
*/
QByteArray QImageReader::imageFormat(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly))
        return QByteArray();

    return imageFormat(&file);
}

/*!
    If supported, this function returns the image format of the device
    \a device. Otherwise, an empty string is returned.
*/
QByteArray QImageReader::imageFormat(QIODevice *device)
{
    QImageIOHandler *handler = ::createReadHandler(device, QByteArray());
    QByteArray format = handler ? handler->name() : QByteArray();
    delete handler;
    return format;
}

/*!
    Returns a list of image formats supported by QImageReader.

    \sa setFormat(), QImageWriter::supportedImageFormats()
*/
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
        QImageIOPlugin *plugin = qobject_cast<QImageIOPlugin *>(l->instance(keys.at(i)));
        if (plugin->capabilities(0, keys.at(i).toLatin1()) & QImageIOPlugin::CanRead)
            formats << keys.at(i).toLatin1();
    }

    qSort(formats);
    return formats;
}
