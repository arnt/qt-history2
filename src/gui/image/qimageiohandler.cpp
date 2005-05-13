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
    \class QImageIOHandler
    \brief The QImageIOHandler class defines the common image I/O
    interface for all image formats in Qt.

    Qt uses QImageIOHandler for reading and writing images through
    QImageReader and QImageWriter. You can also derive from this class
    to write your own image format handler using Qt's plugin mechanism.

    Call setDevice() to assign a device to the handler, and
    setFormat() to assign a format to it. One QImageIOHandler may
    support more than one image format. canRead() returns true if an
    image can be read from the device, and read() and write() return
    true if reading or writing an image was completed successfully.

    QImageIOHandler also has support for animations formats, through
    the functions loopCount(), imageCount(), nextImageDelay() and
    currentImageNumber().

    In order to determine what options an image handler supports, Qt
    will call supportsOption() and setOption(). Make sure to
    reimplement these functions if you can provide support for any of
    the options in the ImageOption enum.

    To write your own image handler, you must at least reimplement
    name(), canRead() and read(). Then create a QImageIOPlugin that
    can create the handler. Finally, install your plugin, and
    QImageReader and QImageWriter will then automatically load the
    plugin, and start using it.

    \sa QImageIOPlugin, QImageReader, QImageWriter
*/

/*! \enum QImageIOHandler::ImageOption

    This enum describes the different options supported by
    QImageIOHandler.  Some options are used to query an image for
    properties, and others are used to toggle the way in which an
    image should be written.

    \value Size The original size of an image. A handler that supports
    this option is expected to read the size of the image from the
    image metadata, and return this size from option() as a QSize.

    \value ClipRect The clip rect, or ROI (Region Of Interest). A
    handler that supports this option is expected to only read the
    provided QRect area from the original image in read(), before any
    other transformation is applied.

    \value ScaledSize The scaled size of the image. A handler that
    supports this option is expected to scale the image to the
    provided size (a QSize), after applying any clip rect
    transformation (\l ClipRect). If the handler does not support this
    option, QImageReader will perform the scaling after the image has
    been read.

    \value ScaledClipRect The scaled clip rect (or ROI, Region Of
    Interest) of the image. A handler that supports this option is
    expected to apply the provided clip rect (a QRect), after applying
    any scaling (\l ScaleSize) or regular clipping (ClipRect). If the
    handler does not support this option, QImageReader will apply the
    scaled clip rect after the image has been read.

    \value Description The image description. A handler that supports
    this option is expected to read the description from the image
    metadata and return this as a QString, or when writing an image it
    is expected to store the description in the image metadata.

    \value CompressionRatio The compression ratio of the image data. A
    handler that supports this option is expected to set its
    compression rate depending on the value of this option (an int)
    when writing.

    \value Gamma The gamma level of the image. A handler that supports
    this option is expected to set the image gamma level depending on
    the value of this option (a float) when writing.

    \value Quality The quality level of the image. A handler that
    supports this option is expected to set the image quality level
    depending on the value of this option (an int) when writing.

    \value Name The name of the image. A handler that supports this
    option is expected to read the name from the image metadata and
    return this as a QString, or when writing an image it is expected
    to store the name in the image metadata.

    \value SubType The subtype of the image. A handler that supports
    this option can use the subtype value to help when reading and
    writing images. For example, a PPM handler may have a subtype
    value of "ppm" or "ppmraw".

    \value IncrementalReading A handler that supports this option is
    expected to read the image in several passes, as if it was an
    animation. QImageReader will treat the image as an animation.

    \value Endianness The endianness of the image. Certain image
    formats can be stored as BigEndian or LittleEndian. A handler that
    supports Endianness uses the value of this option to determine how
    the image should be stored.
*/

/*!
    \class QImageIOPlugin
    \brief The QImageIOPlugin class defines an interface for writing
    an image format plugin.

    \ingroup plugins

    QImageIOPlugin is a factory for creating QImageIOHandler objects,
    which are used internally by QImageReader and QImageWriter to add
    support for different image formats to Qt.

    Writing an picture format plugin is achieved by subclassing this
    base class, reimplementing the pure virtual functions keys(),
    loadPicture(), savePicture(), and installIOHandler(), and
    exporting the class with the \c Q_EXPORT_PLUGIN() macro. See
    \l{How to Create Qt Plugins} for details.

    An image format plugin can support three capabilities: reading (\l
    CanRead), writing (\l CanWrite) and \e incremental reading (\l
    CanReadIncremental). Reimplement capabilities() in you subclass to
    expose the capabilities of your image format.

    create() should create an instance of your QImageIOHandler
    subclass, with the provided device and format properly set, and
    return this handler. You must also reimplement keys() so that Qt
    knows which image formats your plugin supports.

    Different plugins can support different capabilities. For example,
    you may have one plugin that supports reading the GIF format, and
    another that supports writing. Qt will select the correct plugin
    for the job, depending on the return value of capabilities(). If
    several plugins support the same capability, Qt will select one
    arbitrarily.

    \sa QImageIOHandler, {How to Create Qt Plugins}
*/

/*!
    \enum QImageIOPlugin::Capability

    This enum describes the capabilities of a QImageIOPlugin.

    \value CanRead The plugin can read images.
    \value CanWrite The plugin can write images.
    \value CanReadIncremental The plugin can read images incrementally.
*/

/*!
    \class QImageIOHandlerFactoryInterface
    \brief The QImageIOHandlerFactoryInterface class provides the factory
    interface for QImageIOPlugin.

    \internal

    \sa QImageIOPlugin
*/

#include "qimageiohandler.h"

#include <qbytearray.h>
#include <qimage.h>
#include <qvariant.h>

class QIODevice;

class QImageIOHandlerPrivate
{
    Q_DECLARE_PUBLIC(QImageIOHandler)
public:
    QImageIOHandlerPrivate(QImageIOHandler *q);
    virtual ~QImageIOHandlerPrivate();

    QIODevice *device;
    QByteArray format;

    QImageIOHandler *q_ptr;
};

QImageIOHandlerPrivate::QImageIOHandlerPrivate(QImageIOHandler *q)
{
    device = 0;
    q_ptr = q;
}

QImageIOHandlerPrivate::~QImageIOHandlerPrivate()
{
}

/*!
    Constructs a QImageIOHandler object.
*/
QImageIOHandler::QImageIOHandler()
    : d_ptr(new QImageIOHandlerPrivate(this))
{
}

/*! \internal

    Constructs a QImageIOHandler object, using the private member \a
    dd.
*/
QImageIOHandler::QImageIOHandler(QImageIOHandlerPrivate &dd)
    : d_ptr(&dd)
{
}

/*!
    Destructs the QImageIOHandler object.
*/
QImageIOHandler::~QImageIOHandler()
{
    delete d_ptr;
}

/*!
    Sets the device of the QImageIOHandler to \a device. The image
    handler will use this device when reading and writing images.

    \sa device()
*/
void QImageIOHandler::setDevice(QIODevice *device)
{
    Q_D(QImageIOHandler);
    d->device = device;
}

/*!
    Returns the device currently assigned to the QImageIOHandler. If
    not device has been assigned, 0 is returned.
*/
QIODevice *QImageIOHandler::device() const
{
    Q_D(const QImageIOHandler);
    return d->device;
}

/*!
    Sets the format of the QImageIOHandler to \a format. The format is
    most useful for handlers that support multiple image formats.

    \sa format()
*/
void QImageIOHandler::setFormat(const QByteArray &format)
{
    Q_D(QImageIOHandler);
    d->format = format;
}

/*!
    Returns the format that is currently assigned to
    QImageIOHandler. If no format has been assigned, an empty string
    is returned.

    \sa setFormat()
*/
QByteArray QImageIOHandler::format() const
{
    Q_D(const QImageIOHandler);
    return d->format;
}

/*!
    \fn bool QImageIOHandler::read(QImage *image)

    Read an image from the device, and stores it in \a image. For
    image formats that support incremental loading, and for animation
    formats, the image handler can assume that \a image points to the
    previous frame.

    \sa canRead()
*/

/*!
    \fn bool QImageIOHandler::canRead() const = 0

    Returns true if an image can be read from the device (i.e., the
    image format is supported, the device can be read from and the
    initial header information suggests that the image can be read);
    otherwise returns false.

    \sa read()
*/

/*!
    \fn QByteArray QImageIOHandler::name() const = 0

    Returns the name of the image handler. For handlers that support
    only one image format, this should be common identifier of that
    format. For example, a JPEG handler should return "jpeg", and a
    TIFF handler should return "tiff". The name should be returned in
    lowercase; otherwise Qt will convert it to lowercase.
*/

/*!
    Writes the image \a image to the assigned device. Returns true on
    success; otherwise returns false.

    The default implementation does nothing, and simply returns false.
*/
bool QImageIOHandler::write(const QImage &image)
{
    Q_UNUSED(image);
    return false;
}

/*!
    Sets the option \a option with the value \a value.

    \sa option(), ImageOption
*/
void QImageIOHandler::setOption(ImageOption option, const QVariant &value)
{
    Q_UNUSED(option);
    Q_UNUSED(value);
}

/*!
    Returns the value assigned to \a option as a QVariant. The type of
    the value depends on the option. For example, option(Size) returns
    a QSize variant.

    \sa setOption(), supportsOption()
*/
QVariant QImageIOHandler::option(ImageOption option) const
{
    Q_UNUSED(option);
    return QVariant();
}

/*!
    Returns true if the QImageIOHandler supports the option \a option;
    otherwise returns false. For example, if the QImageIOHandler
    supports the \l Size option, supportsOption(Size) must return
    true.

    \sa setOption(), option()
*/
bool QImageIOHandler::supportsOption(ImageOption option) const
{
    Q_UNUSED(option);
    return false;
}

/*!
    For image formats that support animation, this function returns
    the sequence number of the current image in the animation. If the
    image format does not support animation, or if it is unable to
    determine the current sequence number, 0 is returned.
*/
int QImageIOHandler::currentImageNumber() const
{
    return 0;
}

/*!
    Returns the rect of the current image. If no rect is defined for the
    image, and empty QRect() is returned.

    This function is useful for animations, where only parts of the frame
    may be updated at a time.
*/
QRect QImageIOHandler::currentImageRect() const
{
    return QRect();
}

/*!
    For image formats that support animation, this function returns
    the number of images in the animation. If the image format does
    not support animation, or if it is unable to determine the number
    of images, 0 is returned.
*/
int QImageIOHandler::imageCount() const
{
    return 0;
}

/*!
   For image formats that support animation, this function jumps to the
   next image.

   The default implementation does nothing, and returns false.
*/
bool QImageIOHandler::jumpToNextImage()
{
    return false;
}

/*!
   For image formats that support animation, this function jumps to the image
   whose sequence number is \a imageNumber. The next call to read() will
   attempt to read this image.

   The default implementation does nothing, and returns false.
*/
bool QImageIOHandler::jumpToImage(int imageNumber)
{
    Q_UNUSED(imageNumber);
    return false;
}

/*!
    For image formats that support animation, this function returns
    the number of times the animation should loop. If the image format
    does not support animation, 0 is returned.
*/
int QImageIOHandler::loopCount() const
{
    return 0;
}

/*!
    For image formats that support animation, this function returns
    the number of milliseconds to wait until reading the next
    image. If the image format does not support animation, 0 is
    returned.
*/
int QImageIOHandler::nextImageDelay() const
{
    return 0;
}

/*!
    Constructs an image plugin with the given \a parent. This is
    invoked automatically by the \c Q_EXPORT_PLUGIN() macro.
*/
QImageIOPlugin::QImageIOPlugin(QObject *parent)
    : QObject(parent)
{
}

/*!
    Destroys the picture format plugin.

    You never have to call this explicitly. Qt destroys a plugin
    automatically when it is no longer used.
*/
QImageIOPlugin::~QImageIOPlugin()
{
}

/*! \fn QImageIOPlugin::capabilities(QIODevice *device, const QByteArray &format) const

    Returns the capabilities on the plugin, based on the data in \a
    device and the format \a format. For example, if the
    QImageIOHandler supports the BMP format, and the data in the
    device starts with the characters "BM", this function should
    return \l CanRead. If \a format is "bmp" and the handler supports
    both reading and writing, this function should return \l CanRead |
    \l CanWrite.
*/

/*!
    \fn QImageIOPlugin::keys() const

    Returns the list of image keys this plugin supports.

    These keys are usually the names of the image formats that are implemented
    in the plugin (e.g., "jpg" or "gif").

    \sa capabilities()
*/

/*!
    \fn QImageIOHandler *QImageIOPlugin::create(QIODevice *device, const QByteArray &format) const

    \sa keys()    
*/

/*!
    \fn QImageIOHandler::create(QIODevice *device, const QByteArray &format) const

    Creates and returns a QImageIOHandler subclass, with \a device
    and \a format set.

    \sa keys()
*/
