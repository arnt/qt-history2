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

#include <qplatformdefs.h>
#include <qimageio.h>
#include <qasyncimageio.h>
#include <qimage.h>
#include <qimageformatplugin.h>
#include <private/qfactoryloader_p.h>
#include <private/qcolor_p.h>
#include <qcoreapplication.h>
#include <qtextstream.h>
#include <qmutex.h>
#include "qmap.h"
#include "qpngio.h"
#include "qjpegio.h"
#include "qfile.h"
#include <ctype.h>

/*****************************************************************************
  Standard image io handlers (defined below)
 *****************************************************************************/

// standard image io handlers (defined below)
#ifndef QT_NO_IMAGEIO_BMP
static void read_bmp_image(QImageIO *);
static void write_bmp_image(QImageIO *);
#endif
#ifndef QT_NO_IMAGEIO_PPM
static void read_pbm_image(QImageIO *);
static void write_pbm_image(QImageIO *);
#endif
#ifndef QT_NO_IMAGEIO_XBM
static void read_xbm_image(QImageIO *);
static void write_xbm_image(QImageIO *);
#endif
#ifndef QT_NO_IMAGEIO_XPM
static void read_xpm_image(QImageIO *);
static void write_xpm_image(QImageIO *);
#endif

#ifndef QT_NO_ASYNC_IMAGE_IO
static void read_async_image(QImageIO *); // Not in table of handlers
#endif

/*****************************************************************************
  Misc. utility functions
 *****************************************************************************/
#if !defined(QT_NO_IMAGEIO_XPM) || !defined(QT_NO_IMAGEIO_XBM)
static QString fbname(const QString &fileName) // get file basename (sort of)
{
    QString s = fileName;
    if (!s.isEmpty()) {
        int i;
        if ((i = s.lastIndexOf('/')) >= 0)
            s = s.mid(i);
        if ((i = s.lastIndexOf('\\')) >= 0)
            s = s.mid(i);
        QRegExp r(QLatin1String("[a-zA-Z][a-zA-Z0-9_]*"));
        int p = r.indexIn(s);
        if (p == -1)
            s.clear();
        else
            s = s.mid(p, r.matchedLength());
    }
    if (s.isEmpty())
        s = QString::fromLatin1("dummy");
    return s;
}
#endif

#ifndef QT_NO_IMAGEIO_BMP
static void swapPixel01(QImage *image)        // 1-bpp: swap 0 and 1 pixels
{
    int i;
    if (image->depth() == 1 && image->numColors() == 2) {
        register uint *p = (uint *)image->bits();
        int nbytes = image->numBytes();
        for (i=0; i<nbytes/4; i++) {
            *p = ~*p;
            p++;
        }
        uchar *p2 = (uchar *)p;
        for (i=0; i<(nbytes&3); i++) {
            *p2 = ~*p2;
            p2++;
        }
        QRgb t = image->color(0);                // swap color 0 and 1
        image->setColor(0, image->color(1));
        image->setColor(1, t);
    }
}
#endif


/*****************************************************************************
  QImageIO member functions
 *****************************************************************************/

/*!
    \class QImageIO qimage.h

    \brief The QImageIO class contains parameters for loading and
    saving images.

    \ingroup multimedia
    \ingroup io

    QImageIO contains a QIODevice object that is used for image data
    I/O. The programmer can install new image file formats in addition
    to those that Qt provides.

    Qt currently supports the following image file formats: PNG, BMP,
    XBM, XPM and PNM. It may also support JPEG and GIF, if
    specially configured during compilation. The different PNM formats
    are: PBM (P1 or P4), PGM (P2 or P5), and PPM (P3 or P6).

    You don't normally need to use this class; QPixmap::load(),
    QPixmap::save(), and QImage contain sufficient functionality.

    For image files that contain sequences of images, only the first
    is read. See QMovie for loading multiple images.

    PBM, PGM, and PPM format \e output is always in the more condensed
    raw format. PPM and PGM files with more than 256 levels of
    intensity are scaled down when reading.

    \legalese

    Qt supports GIF reading if it is configured that way during
    installation. If it is, we are required to state that
    "The Graphics Interchange Format(c) is the Copyright property of
    CompuServe Incorporated. GIF(sm) is a Service Mark property of
    CompuServe Incorporated."

    \warning If you are in a country which recognizes software patents
    and in which Unisys holds a patent on LZW compression and/or
    decompression and you want to use GIF, Unisys may require you to
    license the technology. Such countries include Canada, Japan, the
    USA, France, Germany, Italy, and the UK. We believe that this
    patent will have expired world-wide by the end of 2004.
    Nonetheless, GIF support may be removed completely in a future
    version of Qt. We recommend using the PNG format instead.

    \sa QImage QPixmap QFile QMovie
*/

#ifndef QT_NO_IMAGEIO
struct QImageIOData
{
    const char *parameters;
    int quality;
    float gamma;
};

/*!
    Constructs a QImageIO object with all parameters set to zero.
*/

QImageIO::QImageIO()
{
    init();
}

/*!
    Constructs a QImageIO object with the I/O device \a ioDevice and a
    \a format tag.
*/

QImageIO::QImageIO(QIODevice *ioDevice, const char *format)
    : frmt(format)
{
    init();
    iodev  = ioDevice;
}

/*!
    Constructs a QImageIO object with the file name \a fileName and a
    \a format tag.
*/

QImageIO::QImageIO(const QString &fileName, const char* format)
    : frmt(format), fname(fileName)
{
    init();
}

/*!
    Contains initialization common to all QImageIO constructors.
*/

void QImageIO::init()
{
    d = new QImageIOData();
    d->parameters = 0;
    d->quality = -1; // default quality of the current format
    d->gamma=0.0f;
    iostat = 0;
    iodev  = 0;
}

/*!
    Destroys the object and all related data.
*/

QImageIO::~QImageIO()
{
    if (d->parameters)
        delete [] (char*)d->parameters;
    delete d;
}


/*****************************************************************************
  QImageIO image handler functions
 *****************************************************************************/

class QImageHandler
{
public:
    QImageHandler(const char *f, const char *h, const QByteArray& fl,
                  const QStringList &e, image_io_handler r, image_io_handler w);
    QByteArray              format;                        // image format
    QRegExp              header;                        // image header pattern
    enum TMode { Untranslated=0, TranslateIn, TranslateInOut } text_mode;
    QStringList extensions;
    image_io_handler  read_image;                // image read function
    image_io_handler  write_image;                // image write function
    bool              obsolete;                        // support not "published"
};

QImageHandler::QImageHandler(const char *f, const char *h, const QByteArray& fl,
                             const QStringList &e, image_io_handler r, image_io_handler w)
    : format(f), header(QString::fromLatin1(h))
{
    text_mode = Untranslated;
    if (fl.contains('t'))
        text_mode = TranslateIn;
    else if (fl.contains('T'))
        text_mode = TranslateInOut;
    obsolete = fl.contains('O');
    extensions = e;
    read_image        = r;
    write_image = w;
}

typedef QList<QImageHandler *> QIHList;
static QIHList imageHandlers;

#ifndef QT_NO_COMPONENT
Q_GLOBAL_STATIC(QMutex, mutex)
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
                          (QImageFormatInterface_iid,
                           QCoreApplication::libraryPaths(),
                           "/imageformats"))
#endif
void qt_init_image_plugins()
{
#ifndef QT_NO_COMPONENT
    QMutexLocker locker(mutex());
    QFactoryLoader *loader = ::loader();
    QStringList keys = loader->keys();
    for (int i = 0; i < keys.count(); ++i) {
        QImageFormatInterface *format =
            qt_cast<QImageFormatInterface*>(loader->instance(keys.at(i)));
        if (format)
            format->installIOHandler(keys.at(i));
    }
#endif
}

static void cleanup()
{
    // make sure that image handlers are delete before plugin manager
    while (!imageHandlers.isEmpty())
        delete imageHandlers.takeFirst();
}

void qt_init_image_handlers()                // initialize image handlers
{
    static bool done = false;
    if (done) return;
    done = true;

    qAddPostRoutine(cleanup);
#ifndef QT_NO_IMAGEIO_BMP
    QImageIO::defineIOHandler("BMP", "^BM", 0,
                               read_bmp_image, write_bmp_image);
#endif
#ifndef QT_NO_IMAGEIO_PPM
    QImageIO::defineIOHandler("PBM", "^P1", "t",
                               read_pbm_image, write_pbm_image);
    QImageIO::defineIOHandler("PBMRAW", "^P4", "O",
                               read_pbm_image, write_pbm_image);
    QImageIO::defineIOHandler("PGM", "^P2", "t",
                               read_pbm_image, write_pbm_image);
    QImageIO::defineIOHandler("PGMRAW", "^P5", "O",
                               read_pbm_image, write_pbm_image);
    QImageIO::defineIOHandler("PPM", "^P3", "t",
                               read_pbm_image, write_pbm_image);
    QImageIO::defineIOHandler("PPMRAW", "^P6", "O",
                               read_pbm_image, write_pbm_image);
#endif
#ifndef QT_NO_IMAGEIO_XBM
    QImageIO::defineIOHandler("XBM", "^#define", "T",
                               read_xbm_image, write_xbm_image);
#endif
#ifndef QT_NO_IMAGEIO_XPM
    QImageIO::defineIOHandler("XPM", "/\\*.XPM.\\*/", "T",
                               read_xpm_image, write_xpm_image);
#endif
#ifndef QT_NO_IMAGEIO_PNG
    qInitPngIO();
#endif
#ifndef QT_NO_IMAGEIO_JPEG
    qInitJpegIO();
#endif
}

static QImageHandler *get_image_handler(const char *format)
{                                                // get pointer to handler
    qt_init_image_handlers();
    qt_init_image_plugins();
    for (int i = 0; i < imageHandlers.size(); ++i) {
        QImageHandler *p = imageHandlers.at(i);
        if (p->format == format)
            return p;
    }
    return 0;                                        // no such handler
}


/*!
    \fn QImageIO::defineIOHandler(const char *format, const char *header, const char *flags, image_io_handler readImage, image_io_handler writeImage);
    \overload

    This overload provides source compatibility by providing an
    extensions as \a format to the overloaded defineIoHandler(). Thus
    when a file is looked for with no extension an extension matching
    the format will be searched.

   \sa QImageIO::defineIOHandler()
*/

/*!
    Defines an image I/O handler for the image format called \a
    format, which is recognized using the \link qregexp.html#details
    regular expression\endlink \a header, read using \a readImage and
    written using \a writeImage.

    \a flags is a string of single-character flags for this format.
    The only flag defined currently is T (upper case), so the only
    legal value for \a flags are "T" and the empty string. The "T"
    flag means that the image file is a text file, and Qt should treat
    all newline conventions as equivalent. (XPM files and some PPM
    files are text files for example.)

    \a format is used to select a handler to write a QImage; \a header
    is used to select a handler to read an image file.

    When an attempt is made to load a file, but it is not found,
    the handler tries each extension in turn from the \a extensions
    list until a file is found or there are no more extensions left
    to try.

    If \a readImage is a null pointer, the QImageIO will not be able
    to read images in \a format. If \a writeImage is a null pointer,
    the QImageIO will not be able to write images in \a format. If
    both are null, the QImageIO object is valid but useless.

    Example:
    \code
        void readGIF(QImageIO *image)
        {
        // read the image using the image->ioDevice()
        }

        void writeGIF(QImageIO *image)
        {
        // write the image using the image->ioDevice()
        }

        // add the GIF image handler

        QImageIO::defineIOHandler("GIF",
                                   "^GIF[0-9][0-9][a-z]",
                                   0,
                                   readGIF,
                                   writeGIF);
    \endcode

    Before the regex test, all the 0 bytes in the file header are
    converted to 1 bytes. This is done because when Qt was
    ASCII-based, QRegExp could not handle 0 bytes in strings.

    The regexp is only applied on the first 14 bytes of the file.

    (Note that if one handlerIO supports writing a format and another
    supports reading it, Qt supports both reading and writing. If two
    handlers support the same operation, Qt chooses one arbitrarily.)
*/

void QImageIO::defineIOHandler(const char *format,
                               const char *header,
                               const char *flags,
                               const QStringList &extensions,
                               image_io_handler readImage,
                               image_io_handler writeImage)
{
    qt_init_image_handlers();
    QImageHandler *p;
    p = new QImageHandler(format, header, QByteArray(flags),
                          extensions, readImage, writeImage);
    imageHandlers.insert(0, p);
}


/*****************************************************************************
  QImageIO normal member functions
 *****************************************************************************/

/*!
    \fn const QImage &QImageIO::image() const

    Returns the image currently set.

    \sa setImage()
*/

/*!
    \fn int QImageIO::status() const

    Returns the image's IO status. A non-zero value indicates an
    error, whereas 0 means that the IO operation was successful.

    \sa setStatus()
*/

/*!
    \fn const char *QImageIO::format() const

    Returns the image format string or 0 if no format has been
    explicitly set.
*/

/*!
    \fn QIODevice *QImageIO::ioDevice() const

    Returns the IO device currently set.

    \sa setIODevice()
*/

/*!
    \fn QString QImageIO::fileName() const

    Returns the file name currently set.

    \sa setFileName()
*/

/*!
    \fn QString QImageIO::description() const

    Returns the image description string.

    \sa setDescription()
*/


/*!
    Sets the image to \a image.

    \sa image()
*/

void QImageIO::setImage(const QImage &image)
{
    im = image;
}

/*!
    Sets the image IO status to \a status. A non-zero value indicates
    an error, whereas 0 means that the IO operation was successful.

    \sa status()
*/

void QImageIO::setStatus(int status)
{
    iostat = status;
}

/*!
    Sets the image format to \a format for the image to be read or
    written.

    It is necessary to specify a format before writing an image, but
    it is not necessary to specify a format before reading an image.

    If no format has been set, Qt guesses the image format before
    reading it. If a format is set the image will only be read if it
    has that format.

    \sa read() write() format()
*/

void QImageIO::setFormat(const char *format)
{
    frmt = format;
}

/*!
    Sets the IO device to be used for reading or writing an image.

    Setting the IO device allows images to be read/written to any
    block-oriented QIODevice.

    If \a ioDevice is not null, this IO device will override file name
    settings.

    \sa setFileName()
*/

void QImageIO::setIODevice(QIODevice *ioDevice)
{
    iodev = ioDevice;
}

/*!
    Sets the name of the file to read or write an image from to \a
    fileName.

    \sa setIODevice()
*/

void QImageIO::setFileName(const QString &fileName)
{
    fname = fileName;
}

/*!
    Returns the quality of the written image, related to the
    compression ratio.

    \sa setQuality() QImage::save()
*/

int QImageIO::quality() const
{
    return d->quality;
}

/*!
    Sets the quality of the written image to \a q, related to the
    compression ratio.

    \a q must be in the range -1..100. Specify 0 to obtain small
    compressed files, 100 for large uncompressed files. (-1 signifies
    the default compression.)

    \sa quality() QImage::save()
*/

void QImageIO::setQuality(int q)
{
    d->quality = q;
}

/*!
    Returns the image's parameters string.

    \sa setParameters()
*/

const char *QImageIO::parameters() const
{
    return d->parameters;
}

/*!
    Sets the image's parameter string to \a parameters. This is for
    image handlers that require special parameters.

    Although the current image formats supported by Qt ignore the
    parameters string, it may be used in future extensions or by
    contributions (for example, JPEG).

    \sa parameters()
*/

void QImageIO::setParameters(const char *parameters)
{
    if (d && d->parameters)
        delete [] (char*)d->parameters;
    d->parameters = qstrdup(parameters);
}

/*!
    Sets the gamma value at which the image will be viewed to \a
    gamma. If the image format stores a gamma value for which the
    image is intended to be used, then this setting will be used to
    modify the image. Setting to 0.0 will disable gamma correction
    (i.e. any specification in the file will be ignored).

    The default value is 0.0.

    \sa gamma()
*/
void QImageIO::setGamma(float gamma)
{
    d->gamma=gamma;
}

/*!
    Returns the gamma value at which the image will be viewed.

    \sa setGamma()
*/
float QImageIO::gamma() const
{
    return d->gamma;
}

/*!
    Sets the image description string for image handlers that support
    image descriptions to \a description.

    Currently, no image format supported by Qt uses the description
    string.
*/

void QImageIO::setDescription(const QString &description)
{
    descr = description;
}


/*!
    Returns a string that specifies the image format of the file \a
    fileName, or null if the file cannot be read or if the format is
    not recognized.
*/

QByteArray QImageIO::imageFormat(const QString &fileName)
{
    QFile file(fileName);
    QByteArray format;
    if (!file.open(QIODevice::ReadOnly))
        return format;
    format = imageFormat(&file);
    file.close();
    return format;
}

/*!
    \overload

    Returns a string that specifies the image format of the image read
    from IO device \a d, or 0 if the device cannot be read or if the
    format is not recognized.

    Make sure that \a d is at the right position in the device (for
    example, at the beginning of the file).

    \sa QIODevice::at()
*/

QByteArray QImageIO::imageFormat(QIODevice *d)
{
    // if you change this change the documentation for defineIOHandler()
    const int buflen = 14;

    char buf[buflen];
    char buf2[buflen];
    qt_init_image_handlers();
    qt_init_image_plugins();
    int pos = d->pos();                        // save position
    int rdlen = d->read(buf, buflen);        // read a few bytes

    QByteArray format;
    if (rdlen != buflen)
        return format;

    memcpy(buf2, buf, buflen);

    for (int n = 0; n < rdlen; n++)
        if (buf[n] == '\0')
            buf[n] = '\001';
    if (rdlen > 0) {
        buf[rdlen - 1] = '\0';
        QString bufStr = QString::fromLatin1(buf);
        int bestMatch = -1;
        for (int i = 0; i < imageHandlers.size(); ++i) {
            QImageHandler *p = imageHandlers.at(i);
            if (p->read_image && p->header.indexIn(bufStr) != -1) {
                // try match with header if a read function is available
                if (p->header.matchedLength() > bestMatch) {
                    // keep looking for best match
                    format = p->format;
                    bestMatch = p->header.matchedLength();
                }
            }
        }
    }
    d->seek(pos);                                // restore position
#ifndef QT_NO_ASYNC_IMAGE_IO
    if (format.isEmpty())
        format = QImageDecoder::formatName((uchar*)buf2, rdlen);
#endif

    return format;
}

/*!
    Returns a sorted list of image formats that are supported for
    image input.
*/
QList<QByteArray> QImageIO::inputFormats()
{
    QList<QByteArray> result;

    qt_init_image_handlers();
    qt_init_image_plugins();

#ifndef QT_NO_ASYNC_IMAGE_IO
    // Include asynchronous loaders first.
    result = QImageDecoder::inputFormats();
#endif

    for (int i = 0; i < imageHandlers.size(); ++i) {
        QImageHandler *p = imageHandlers.at(i);
        if (p->read_image && !p->obsolete && !result.contains(p->format))
            result.append(p->format);
    }
    qHeapSort(result);

    return result;
}

/*!
    Returns a sorted list of image formats that are supported for
    image output.
*/
QList<QByteArray> QImageIO::outputFormats()
{
    QList<QByteArray> result;

    qt_init_image_handlers();
    qt_init_image_plugins();

    // Include asynchronous writers (!) first.
    // (None)

    for (int i = 0; i < imageHandlers.size(); ++i) {
        QImageHandler *p = imageHandlers.at(i);
        if (p->write_image && !p->obsolete && !result.contains(p->format))
            result.append(p->format);
    }

    return result;
}



/*!
    Reads an image into memory and returns true if the image was
    successfully read; otherwise returns false.

    Before reading an image you must set an IO device or a file name.
    If both an IO device and a file name have been set, the IO device
    will be used. If only the file name is set but does not appear to
    exist the list of imageFormats() will be used to find suitable
    extensions to create a filename that does exist (in order of
    definition).

    Setting the image file format string is optional.

    Note that this function does \e not set the \link format()
    format\endlink used to read the image. If you need that
    information, use the imageFormat() static functions.

    Example:

    \code
        QImageIO iio;
        QPixmap  pixmap;
        iio.setFileName("vegeburger.bmp");
        if (image.read())        // ok
            pixmap = iio.image();  // convert to pixmap
    \endcode

    \sa setIODevice() setFileName() setFormat() write() QPixmap::load()
*/

bool QImageIO::read()
{
    QFile           file;
    const char          *image_format;
    QImageHandler *h;

    if (iodev) {                                // read from io device
        // ok, already open
    } else if (!fname.isEmpty()) {                // read from file
        file.setFileName(fname);
        if (!file.open(QIODevice::ReadOnly)) {            // cannot open file
            if(frmt.isEmpty()) {                  //try some extensions
                qt_init_image_handlers();
                qt_init_image_plugins();
                for (int i = 0; i < imageHandlers.size() && !file.isOpen(); ++i) {
                    QImageHandler *p = imageHandlers.at(i);
                    for(int ext = 0; ext < p->extensions.count(); ++ext) {
                        QString attempt = fname + "." + p->extensions.at(ext);
                        if(QFile::exists(attempt)) {
                            file.setFileName(attempt);
                            if(file.open(QIODevice::ReadOnly))
                                break;
                        }
                    }
                }
            }
            if(!file.isOpen())
                return false;
        }
        iodev = &file;
    } else {                                        // no file name or io device
        return false;
    }
    if (frmt.isEmpty()) {
        // Try to guess format
        image_format = imageFormat(iodev);        // get image format
        if (!image_format) {
            if (file.isOpen()) {                        // unknown format
                file.close();
                iodev = 0;
            }
            return false;
        }
    } else {
        image_format = frmt;
    }

    h = get_image_handler(image_format);
    if (file.isOpen()) {
#if !defined(Q_OS_UNIX)
        if (h && h->text_mode) {                // reopen in translated mode
            file.close();
            file.open(QIODevice::ReadOnly | QIODevice::Translate);
        }
        else
#endif
            file.seek(0);                        // position to start
    }
    iostat = 1;                                        // assume error

    if (h && h->read_image) {
        (*h->read_image)(this);
    }
#ifndef QT_NO_ASYNC_IMAGE_IO
    else {
        // Format name, but no handler - must be an asychronous reader
        read_async_image(this);
    }
#endif

    if (file.isOpen()) {                        // image was read using file
        file.close();
        iodev = 0;
    }
    return iostat == 0;                                // image successfully read?
}


/*!
    Writes an image to an IO device and returns true if the image was
    successfully written; otherwise returns false.

    Before writing an image you must set an IO device or a file name.
    If both an IO device and a file name have been set, the IO device
    will be used.

    The image will be written using the specified image format.

    Example:
    \code
        QImageIO iio;
        QImage   im;
        im = pixmap; // convert to image
        iio.setImage(im);
        iio.setFileName("vegeburger.bmp");
        iio.setFormat("BMP");
        if (iio.write())
            // returned true if written successfully
    \endcode

    \sa setIODevice() setFileName() setFormat() read() QPixmap::save()
*/

bool QImageIO::write()
{
    if (frmt.isEmpty())
        return false;
    QImageHandler *h = get_image_handler(frmt);
    if (!h || !h->write_image) {
        qWarning("QImageIO::write: No such image format handler: %s",
                 format());
        return false;
    }
    QFile file;
    if (!iodev && !fname.isEmpty()) {
        file.setFileName(fname);
        bool translate = h->text_mode==QImageHandler::TranslateInOut;
        QIODevice::OpenMode fmode = translate ? QIODevice::WriteOnly | QIODevice::Translate : QIODevice::OpenMode(QIODevice::WriteOnly);
        if (!file.open(fmode))                // couldn't create file
            return false;
        iodev = &file;
    }
    iostat = 1;
    (*h->write_image)(this);
    if (file.isOpen()) {                        // image was written using file
        file.close();
        iodev = 0;
    }
    return iostat == 0;                                // image successfully written?
}
#endif //QT_NO_IMAGEIO

#ifndef QT_NO_IMAGEIO_BMP

/*****************************************************************************
  BMP (DIB) image read/write functions
 *****************************************************************************/

const int BMP_FILEHDR_SIZE = 14;                // size of BMP_FILEHDR data

struct BMP_FILEHDR {                                // BMP file header
    char   bfType[2];                                // "BM"
    Q_INT32  bfSize;                                // size of file
    Q_INT16  bfReserved1;
    Q_INT16  bfReserved2;
    Q_INT32  bfOffBits;                                // pointer to the pixmap bits
};

QDataStream &operator>>(QDataStream &s, BMP_FILEHDR &bf)
{                                                // read file header
    s.readRawData(bf.bfType, 2);
    s >> bf.bfSize >> bf.bfReserved1 >> bf.bfReserved2 >> bf.bfOffBits;
    return s;
}

QDataStream &operator<<(QDataStream &s, const BMP_FILEHDR &bf)
{                                                // write file header
    s.writeRawData(bf.bfType, 2);
    s << bf.bfSize << bf.bfReserved1 << bf.bfReserved2 << bf.bfOffBits;
    return s;
}


const int BMP_OLD  = 12;                        // old Windows/OS2 BMP size
const int BMP_WIN  = 40;                        // new Windows BMP size
const int BMP_OS2  = 64;                        // new OS/2 BMP size

const int BMP_RGB  = 0;                                // no compression
const int BMP_RLE8 = 1;                                // run-length encoded, 8 bits
const int BMP_RLE4 = 2;                                // run-length encoded, 4 bits
const int BMP_BITFIELDS = 3;                        // RGB values encoded in data as bit-fields

struct BMP_INFOHDR {                                // BMP information header
    Q_INT32  biSize;                                // size of this struct
    Q_INT32  biWidth;                                // pixmap width
    Q_INT32  biHeight;                                // pixmap height
    Q_INT16  biPlanes;                                // should be 1
    Q_INT16  biBitCount;                        // number of bits per pixel
    Q_INT32  biCompression;                        // compression method
    Q_INT32  biSizeImage;                                // size of image
    Q_INT32  biXPelsPerMeter;                        // horizontal resolution
    Q_INT32  biYPelsPerMeter;                        // vertical resolution
    Q_INT32  biClrUsed;                                // number of colors used
    Q_INT32  biClrImportant;                        // number of important colors
};


QDataStream &operator>>(QDataStream &s, BMP_INFOHDR &bi)
{
    s >> bi.biSize;
    if (bi.biSize == BMP_WIN || bi.biSize == BMP_OS2) {
        s >> bi.biWidth >> bi.biHeight >> bi.biPlanes >> bi.biBitCount;
        s >> bi.biCompression >> bi.biSizeImage;
        s >> bi.biXPelsPerMeter >> bi.biYPelsPerMeter;
        s >> bi.biClrUsed >> bi.biClrImportant;
    }
    else {                                        // probably old Windows format
        Q_INT16 w, h;
        s >> w >> h >> bi.biPlanes >> bi.biBitCount;
        bi.biWidth  = w;
        bi.biHeight = h;
        bi.biCompression = BMP_RGB;                // no compression
        bi.biSizeImage = 0;
        bi.biXPelsPerMeter = bi.biYPelsPerMeter = 0;
        bi.biClrUsed = bi.biClrImportant = 0;
    }
    return s;
}

QDataStream &operator<<(QDataStream &s, const BMP_INFOHDR &bi)
{
    s << bi.biSize;
    s << bi.biWidth << bi.biHeight;
    s << bi.biPlanes;
    s << bi.biBitCount;
    s << bi.biCompression;
    s << bi.biSizeImage;
    s << bi.biXPelsPerMeter << bi.biYPelsPerMeter;
    s << bi.biClrUsed << bi.biClrImportant;
    return s;
}

static
int calc_shift(int mask)
{
    int result = 0;
    while (!(mask & 1)) {
        result++;
        mask >>= 1;
    }
    return result;
}

static
bool read_dib(QDataStream& s, int offset, int startpos, QImage& image)
{
    BMP_INFOHDR bi;
    QIODevice* d = s.device();

    s >> bi;                                        // read BMP info header
    if (d->atEnd())                                // end of stream/file
        return false;
#if 0
    qDebug("offset...........%d", offset);
    qDebug("startpos.........%d", startpos);
    qDebug("biSize...........%d", bi.biSize);
    qDebug("biWidth..........%d", bi.biWidth);
    qDebug("biHeight.........%d", bi.biHeight);
    qDebug("biPlanes.........%d", bi.biPlanes);
    qDebug("biBitCount.......%d", bi.biBitCount);
    qDebug("biCompression....%d", bi.biCompression);
    qDebug("biSizeImage......%d", bi.biSizeImage);
    qDebug("biXPelsPerMeter..%d", bi.biXPelsPerMeter);
    qDebug("biYPelsPerMeter..%d", bi.biYPelsPerMeter);
    qDebug("biClrUsed........%d", bi.biClrUsed);
    qDebug("biClrImportant...%d", bi.biClrImportant);
#endif
    int w = bi.biWidth,         h = bi.biHeight,  nbits = bi.biBitCount;
    int t = bi.biSize,         comp = bi.biCompression;
    int red_mask, green_mask, blue_mask;
    int red_shift = 0;
    int green_shift = 0;
    int blue_shift = 0;
    int red_scale = 0;
    int green_scale = 0;
    int blue_scale = 0;

    if (!(nbits == 1 || nbits == 4 || nbits == 8 || nbits == 16 || nbits == 24 || nbits == 32) ||
        bi.biPlanes != 1 || comp > BMP_BITFIELDS)
        return false;                                        // weird BMP image
    if (!(comp == BMP_RGB || (nbits == 4 && comp == BMP_RLE4) ||
        (nbits == 8 && comp == BMP_RLE8) || ((nbits == 16 || nbits == 32) && comp == BMP_BITFIELDS)))
         return false;                                // weird compression type

    int ncols;
    int depth;
    switch (nbits) {
        case 32:
        case 24:
        case 16:
            depth = 32;
            break;
        case 8:
        case 4:
            depth = 8;
            break;
        default:
            depth = 1;
    }
    if (depth == 32)                                // there's no colormap
        ncols = 0;
    else                                        // # colors used
        ncols = bi.biClrUsed ? bi.biClrUsed : 1 << nbits;

    image.create(w, h, depth, ncols, nbits == 1 ?
                  QImage::BigEndian : QImage::IgnoreEndian);
    if (image.isNull())                        // could not create image
        return false;

    image.setDotsPerMeterX(bi.biXPelsPerMeter);
    image.setDotsPerMeterY(bi.biYPelsPerMeter);

    d->seek(startpos + BMP_FILEHDR_SIZE + bi.biSize); // goto start of colormap

    if (ncols > 0) {                                // read color table
        uchar rgb[4];
        int   rgb_len = t == BMP_OLD ? 3 : 4;
        for (int i=0; i<ncols; i++) {
            if (d->read((char *)rgb, rgb_len) != rgb_len)
                return false;
            image.setColor(i, qRgb(rgb[2],rgb[1],rgb[0]));
            if (d->atEnd())                        // truncated file
                return false;
        }
    } else if (comp == BMP_BITFIELDS && (nbits == 16 || nbits == 32)) {
        if ((Q_ULONG)d->read((char *)&red_mask, sizeof(red_mask)) != sizeof(red_mask))
            return false;
        if ((Q_ULONG)d->read((char *)&green_mask, sizeof(green_mask)) != sizeof(green_mask))
            return false;
        if ((Q_ULONG)d->read((char *)&blue_mask, sizeof(blue_mask)) != sizeof(blue_mask))
            return false;
        red_shift = calc_shift(red_mask);
        red_scale = 256 / ((red_mask >> red_shift) + 1);
        green_shift = calc_shift(green_mask);
        green_scale = 256 / ((green_mask >> green_shift) + 1);
        blue_shift = calc_shift(blue_mask);
        blue_scale = 256 / ((blue_mask >> blue_shift) + 1);
    } else if (comp == BMP_RGB && (nbits == 24 || nbits == 32)) {
        blue_mask = 0x000000ff;
        green_mask = 0x0000ff00;
        red_mask = 0x00ff0000;
        blue_shift = 0;
        green_shift = 8;
        red_shift = 16;
        blue_scale = green_scale = red_scale = 1;
    } else if (comp == BMP_RGB && nbits == 16)  // don't support RGB values for 15/16 bpp
        return false;

    // offset can be bogus, be careful
    if (offset>=0 && startpos + offset > (Q_LONG)d->pos())
        d->seek(startpos + offset);                // start of image data

    int             bpl = image.bytesPerLine();
#ifdef Q_WS_QWS
    //
    // Guess the number of bytes-per-line if we don't know how much
    // image data is in the file (bogus image ?).
    //
    int bmpbpl = bi.biSizeImage > 0 ?
        bi.biSizeImage / bi.biHeight :
        (d->size() - offset) / bi.biHeight;
    int pad = bmpbpl-bpl;
#endif
    uchar **line = image.jumpTable();

    if (nbits == 1) {                                // 1 bit BMP image
        while (--h >= 0) {
            if (d->read((char*)line[h],bpl) != bpl)
                break;
#ifdef Q_WS_QWS
            if (pad > 0)
                d->seek(d->pos()+pad);
#endif
        }
        if (ncols == 2 && qGray(image.color(0)) < qGray(image.color(1)))
            swapPixel01(&image);                // pixel 0 is white!
    }

    else if (nbits == 4) {                        // 4 bit BMP image
        int    buflen = ((w+7)/8)*4;
        uchar *buf    = new uchar[buflen];
        if (comp == BMP_RLE4) {                // run length compression
            int x=0, y=0, c, i;
            char b;
            register uchar *p = line[h-1];
            const uchar *endp = line[h-1]+w;
            while (y < h) {
                if (!d->getChar(&b))
                    break;
                if (b == 0) {                        // escape code
                    if (!d->getChar(&b) || b == 1) {
                        y = h;                // exit loop
                    } else switch (b) {
                        case 0:                        // end of line
                            x = 0;
                            y++;
                            p = line[h-y-1];
                            break;
                        case 2:                        // delta (jump)
                        {
                            char tmp;
                            d->getChar(&tmp);
                            x += tmp;
                            d->getChar(&tmp);
                            y += tmp;
                        }

                            // Protection
                            if ((uint)x >= (uint)w)
                                x = w-1;
                            if ((uint)y >= (uint)h)
                                y = h-1;

                            p = line[h-y-1] + x;
                            break;
                        default:                // absolute mode
                            // Protection
                            if (p + b > endp)
                                b = endp-p;

                            i = (c = b)/2;
                            while (i--) {
                                d->getChar(&b);
                                *p++ = b >> 4;
                                *p++ = b & 0x0f;
                            }
                            if (c & 1) {
                                char tmp;
                                d->getChar(&tmp);
                                *p++ = tmp >> 4;
                            }
                            if ((((c & 3) + 1) & 2) == 2)
                                d->getChar(0);        // align on word boundary
                            x += c;
                    }
                } else {                        // encoded mode
                    // Protection
                    if (p + b > endp)
                        b = endp-p;

                    i = (c = b)/2;
                    d->getChar(&b);                // 2 pixels to be repeated
                    while (i--) {
                        *p++ = b >> 4;
                        *p++ = b & 0x0f;
                    }
                    if (c & 1)
                        *p++ = b >> 4;
                    x += c;
                }
            }
        } else if (comp == BMP_RGB) {                // no compression
            while (--h >= 0) {
                if (d->read((char*)buf,buflen) != buflen)
                    break;
                register uchar *p = line[h];
                uchar *b = buf;
                for (int i=0; i<w/2; i++) {        // convert nibbles to bytes
                    *p++ = *b >> 4;
                    *p++ = *b++ & 0x0f;
                }
                if (w & 1)                        // the last nibble
                    *p = *b >> 4;
            }
        }
        delete [] buf;
    }

    else if (nbits == 8) {                        // 8 bit BMP image
        if (comp == BMP_RLE8) {                // run length compression
            int x=0, y=0;
            char b;
            register uchar *p = line[h-1];
            const uchar *endp = line[h-1]+w;
            while (y < h) {
                if (!d->getChar(&b))
                    break;
                if (b == 0) {                        // escape code
                    if (!d->getChar(&b) || b == 1) {
                            y = h;                // exit loop
                    } else switch (b) {
                        case 0:                        // end of line
                            x = 0;
                            y++;
                            p = line[h-y-1];
                            break;
                        case 2:                        // delta (jump)
                            // Protection
                            if ((uint)x >= (uint)w)
                                x = w-1;
                            if ((uint)y >= (uint)h)
                                y = h-1;

                            {
                                char tmp;
                                d->getChar(&tmp);
                                x += tmp;
                                d->getChar(&tmp);
                                y += tmp;
                            }
                            p = line[h-y-1] + x;
                            break;
                        default:                // absolute mode
                            // Protection
                            if (p + b > endp)
                                b = endp-p;

                            if (d->read((char *)p, b) != b)
                                return false;
                            if ((b & 1) == 1)
                                d->getChar(0);        // align on word boundary
                            x += b;
                            p += b;
                    }
                } else {                        // encoded mode
                    // Protection
                    if (p + b > endp)
                        b = endp-p;

                    char tmp;
                    d->getChar(&tmp);
                    memset(p, tmp, b); // repeat pixel
                    x += b;
                    p += b;
                }
            }
        } else if (comp == BMP_RGB) {                // uncompressed
            while (--h >= 0) {
                if (d->read((char *)line[h],bpl) != bpl)
                    break;
#ifdef Q_WS_QWS
                if (pad > 0)
                    d->seek(d->pos()+pad);
#endif
            }
        }
    }

    else if (nbits == 16 || nbits == 24 || nbits == 32) { // 16,24,32 bit BMP image
        register QRgb *p;
        QRgb  *end;
        uchar *buf24 = new uchar[bpl];
        int    bpl24 = ((w*nbits+31)/32)*4;
        uchar *b;
        int c;

        while (--h >= 0) {
            p = (QRgb *)line[h];
            end = p + w;
            if (d->read((char *)buf24,bpl24) != bpl24)
                break;
            b = buf24;
            while (p < end) {
                c = *(uchar*)b | (*(uchar*)(b+1)<<8);
                if (nbits != 16)
                    c |= *(uchar*)(b+2)<<16;
                *p++ = qRgb(((c & red_mask) >> red_shift) * red_scale,
                                        ((c & green_mask) >> green_shift) * green_scale,
                                        ((c & blue_mask) >> blue_shift) * blue_scale);
                b += nbits/8;
            }
        }
        delete[] buf24;
    }

    return true;
}

bool qt_read_dib(QDataStream& s, QImage& image)
{
    return read_dib(s,-1,-BMP_FILEHDR_SIZE,image);
}


static void read_bmp_image(QImageIO *iio)
{
    QIODevice  *d = iio->ioDevice();
    QDataStream s(d);
    BMP_FILEHDR bf;
    int startpos = d->pos();

    s.setByteOrder(QDataStream::LittleEndian);// Intel byte order
    s >> bf;                                        // read BMP file header

    if (qstrncmp(bf.bfType,"BM",2) != 0)        // not a BMP image
        return;

    QImage image;
    if (read_dib(s, bf.bfOffBits, startpos, image)) {
        iio->setImage(image);
        iio->setStatus(0);                        // image ok
    }
}

bool qt_write_dib(QDataStream& s, QImage image)
{
    int        nbits;
    int        bpl_bmp;
    int        bpl = image.bytesPerLine();

    QIODevice* d = s.device();

    if (image.depth() == 8 && image.numColors() <= 16) {
        bpl_bmp = (((bpl+1)/2+3)/4)*4;
        nbits = 4;
    } else if (image.depth() == 32) {
        bpl_bmp = ((image.width()*24+31)/32)*4;
        nbits = 24;
#ifdef Q_WS_QWS
    } else if (image.depth() == 1 || image.depth() == 8) {
        // Qt/E doesn't word align.
        bpl_bmp = ((image.width()*image.depth()+31)/32)*4;
        nbits = image.depth();
#endif
    } else {
        bpl_bmp = bpl;
        nbits = image.depth();
    }

    BMP_INFOHDR bi;
    bi.biSize               = BMP_WIN;                // build info header
    bi.biWidth               = image.width();
    bi.biHeight               = image.height();
    bi.biPlanes               = 1;
    bi.biBitCount      = nbits;
    bi.biCompression   = BMP_RGB;
    bi.biSizeImage     = bpl_bmp*image.height();
    bi.biXPelsPerMeter = image.dotsPerMeterX() ? image.dotsPerMeterX()
                                                : 2834; // 72 dpi default
    bi.biYPelsPerMeter = image.dotsPerMeterY() ? image.dotsPerMeterY() : 2834;
    bi.biClrUsed       = image.numColors();
    bi.biClrImportant  = image.numColors();
    s << bi;                                        // write info header

    if (image.depth() != 32) {                // write color table
        uchar *color_table = new uchar[4*image.numColors()];
        uchar *rgb = color_table;
        QRgb *c = image.colorTable();
        for (int i=0; i<image.numColors(); i++) {
            *rgb++ = qBlue (c[i]);
            *rgb++ = qGreen(c[i]);
            *rgb++ = qRed  (c[i]);
            *rgb++ = 0;
        }
        d->write((char *)color_table, 4*image.numColors());
        delete [] color_table;
    }

    if (image.depth() == 1 && image.bitOrder() != QImage::BigEndian)
        image = image.convertBitOrder(QImage::BigEndian);

    int         y;

    if (nbits == 1 || nbits == 8) {                // direct output
#ifdef Q_WS_QWS
        // Qt/E doesn't word align.
        int pad = bpl_bmp - bpl;
        char padding[4];
#endif
        for (y=image.height()-1; y>=0; y--) {
            d->write((char*)image.scanLine(y), bpl);
#ifdef Q_WS_QWS
            d->write(padding, pad);
#endif
        }
        return true;
    }

    uchar *buf        = new uchar[bpl_bmp];
    uchar *b, *end;
    register uchar *p;

    memset(buf, 0, bpl_bmp);
    for (y=image.height()-1; y>=0; y--) {        // write the image bits
        if (nbits == 4) {                        // convert 8 -> 4 bits
            p = image.scanLine(y);
            b = buf;
            end = b + image.width()/2;
            while (b < end) {
                *b++ = (*p << 4) | (*(p+1) & 0x0f);
                p += 2;
            }
            if (image.width() & 1)
                *b = *p << 4;
        } else {                                // 32 bits
            QRgb *p   = (QRgb *)image.scanLine(y);
            QRgb *end = p + image.width();
            b = buf;
            while (p < end) {
                *b++ = qBlue(*p);
                *b++ = qGreen(*p);
                *b++ = qRed(*p);
                p++;
            }
        }
        if (bpl_bmp != d->write((char*)buf, bpl_bmp)) {
            delete[] buf;
            return false;
        }
    }
    delete[] buf;
    return true;
}


static void write_bmp_image(QImageIO *iio)
{
    QIODevice  *d = iio->ioDevice();
    QImage        image = iio->image();
    QDataStream s(d);
    BMP_FILEHDR bf;
    int                bpl_bmp;
    int                bpl = image.bytesPerLine();

    // Code partially repeated in qt_write_dib
    if (image.depth() == 8 && image.numColors() <= 16) {
        bpl_bmp = (((bpl+1)/2+3)/4)*4;
    } else if (image.depth() == 32) {
        bpl_bmp = ((image.width()*24+31)/32)*4;
    } else {
        bpl_bmp = bpl;
    }

    iio->setStatus(0);
    s.setByteOrder(QDataStream::LittleEndian);// Intel byte order
    strncpy(bf.bfType, "BM", 2);                // build file header
    bf.bfReserved1 = bf.bfReserved2 = 0;        // reserved, should be zero
    bf.bfOffBits   = BMP_FILEHDR_SIZE + BMP_WIN + image.numColors()*4;
    bf.bfSize           = bf.bfOffBits + bpl_bmp*image.height();
    s << bf;                                        // write file header

    if (!qt_write_dib(s, image))
        iio->setStatus(1);

}

#endif // QT_NO_IMAGEIO_BMP

#ifndef QT_NO_IMAGEIO_PPM

/*****************************************************************************
  PBM/PGM/PPM (ASCII and RAW) image read/write functions
 *****************************************************************************/

static int read_pbm_int(QIODevice *d)
{
    char c;
    int          val = -1;
    bool  digit;
    const int buflen = 100;
    char  buf[buflen];
    for (;;) {
        if (!d->getChar(&c))                // end of file
            break;
        digit = isdigit((uchar) c);
        if (val != -1) {
            if (digit) {
                val = 10*val + c - '0';
                continue;
            } else {
                if (c == '#')                        // comment
                    d->readLine(buf, buflen);
                break;
            }
        }
        if (digit)                                // first digit
            val = c - '0';
        else if (isspace((uchar) c))
            continue;
        else if (c == '#')
            d->readLine(buf, buflen);
        else
            break;
    }
    return val;
}

static void read_pbm_image(QImageIO *iio)        // read PBM image data
{
    const int        buflen = 300;
    char        buf[buflen];
    QIODevice  *d = iio->ioDevice();
    int                w, h, nbits, mcc, y;
    int                pbm_bpl;
    char        type;
    bool        raw;
    QImage        image;

    if (d->read(buf, 3) != 3)                        // read P[1-6]<white-space>
        return;
    if (!(buf[0] == 'P' && isdigit((uchar) buf[1]) && isspace((uchar) buf[2])))
        return;
    switch ((type=buf[1])) {
        case '1':                                // ascii PBM
        case '4':                                // raw PBM
            nbits = 1;
            break;
        case '2':                                // ascii PGM
        case '5':                                // raw PGM
            nbits = 8;
            break;
        case '3':                                // ascii PPM
        case '6':                                // raw PPM
            nbits = 32;
            break;
        default:
            return;
    }
    raw = type >= '4';
    w = read_pbm_int(d);                        // get image width
    h = read_pbm_int(d);                        // get image height
    if (nbits == 1)
        mcc = 1;                                // ignore max color component
    else
        mcc = read_pbm_int(d);                // get max color component
    if (w <= 0 || w > 32767 || h <= 0 || h > 32767 || mcc <= 0)
        return;                                        // weird P.M image

    int maxc = mcc;
    if (maxc > 255)
        maxc = 255;
    image.create(w, h, nbits, 0,
                  nbits == 1 ? QImage::BigEndian :  QImage::IgnoreEndian);
    if (image.isNull())
        return;

    pbm_bpl = (nbits*w+7)/8;                        // bytes per scanline in PBM

    if (raw) {                                // read raw data
        if (nbits == 32) {                        // type 6
            pbm_bpl = 3*w;
            uchar *buf24 = new uchar[pbm_bpl], *b;
            QRgb  *p;
            QRgb  *end;
            for (y=0; y<h; y++) {
                if (d->read((char *)buf24, pbm_bpl) != pbm_bpl) {
                    delete[] buf24;
                    return;
                }
                p = (QRgb *)image.scanLine(y);
                end = p + w;
                b = buf24;
                while (p < end) {
                    *p++ = qRgb(b[0],b[1],b[2]);
                    b += 3;
                }
            }
            delete[] buf24;
        } else {                                // type 4,5
            for (y=0; y<h; y++) {
                if (d->read((char *)image.scanLine(y), pbm_bpl)
                        != pbm_bpl)
                    return;
            }
        }
    } else {                                        // read ascii data
        register uchar *p;
        int n;
        for (y=0; y<h; y++) {
            p = image.scanLine(y);
            n = pbm_bpl;
            if (nbits == 1) {
                int b;
                int bitsLeft = w;
                while (n--) {
                    b = 0;
                    for (int i=0; i<8; i++) {
                        if (i < bitsLeft)
                            b = (b << 1) | (read_pbm_int(d) & 1);
                        else
                            b = (b << 1) | (0 & 1); // pad it our self if we need to
                    }
                    bitsLeft -= 8;
                    *p++ = b;
                }
            } else if (nbits == 8) {
                if (mcc == maxc) {
                    while (n--) {
                        *p++ = read_pbm_int(d);
                    }
                } else {
                    while (n--) {
                        *p++ = read_pbm_int(d) * maxc / mcc;
                    }
                }
            } else {                                // 32 bits
                n /= 4;
                int r, g, b;
                if (mcc == maxc) {
                    while (n--) {
                        r = read_pbm_int(d);
                        g = read_pbm_int(d);
                        b = read_pbm_int(d);
                        *((QRgb*)p) = qRgb(r, g, b);
                        p += 4;
                    }
                } else {
                    while (n--) {
                        r = read_pbm_int(d) * maxc / mcc;
                        g = read_pbm_int(d) * maxc / mcc;
                        b = read_pbm_int(d) * maxc / mcc;
                        *((QRgb*)p) = qRgb(r, g, b);
                        p += 4;
                    }
                }
            }
        }
    }

    if (nbits == 1) {                                // bitmap
        image.setNumColors(2);
        image.setColor(0, qRgb(255,255,255)); // white
        image.setColor(1, qRgb(0,0,0));        // black
    } else if (nbits == 8) {                        // graymap
        image.setNumColors(maxc+1);
        for (int i=0; i<=maxc; i++)
            image.setColor(i, qRgb(i*255/maxc,i*255/maxc,i*255/maxc));
    }

    iio->setImage(image);
    iio->setStatus(0);                        // image ok
}


static void write_pbm_image(QImageIO *iio)
{
    QIODevice* out = iio->ioDevice();
    QByteArray str;

    QImage  image  = iio->image();
    QByteArray format(iio->format());
    format = format.left(3);                        // ignore RAW part
    bool gray = format == "PGM";

    if (format == "PBM") {
        image = image.convertDepth(1);
    } else if (image.depth() == 1) {
        image = image.convertDepth(8);
    }

    if (image.depth() == 1 && image.numColors() == 2) {
        if (qGray(image.color(0)) < qGray(image.color(1))) {
            // 0=dark/black, 1=light/white - invert
            image.detach();
            for (int y=0; y<image.height(); y++) {
                uchar *p = image.scanLine(y);
                uchar *end = p + image.bytesPerLine();
                while (p < end)
                    *p++ ^= 0xff;
            }
        }
    }

    uint w = image.width();
    uint h = image.height();

    str = "P\n";
    str += QByteArray::number(w);
    str += ' ';
    str += QByteArray::number(h);

    switch (image.depth()) {
        case 1: {
            str.insert(1, '4');
            if (out->write(str, str.length()) != str.length()) {
                iio->setStatus(1);
                return;
            }
            w = (w+7)/8;
            for (uint y=0; y<h; y++) {
                uchar* line = image.scanLine(y);
                if (w != (uint)out->write((char*)line, w)) {
                    iio->setStatus(1);
                    return;
                }
            }
            }
            break;

        case 8: {
            str.insert(1, gray ? '5' : '6');
            str.append("255\n");
            if (out->write(str, str.length()) != str.length()) {
                iio->setStatus(1);
                return;
            }
            QRgb  *color = image.colorTable();
            uint bpl = w*(gray ? 1 : 3);
            uchar *buf   = new uchar[bpl];
            for (uint y=0; y<h; y++) {
                uchar *b = image.scanLine(y);
                uchar *p = buf;
                uchar *end = buf+bpl;
                if (gray) {
                    while (p < end) {
                        uchar g = (uchar)qGray(color[*b++]);
                        *p++ = g;
                    }
                } else {
                    while (p < end) {
                        QRgb rgb = color[*b++];
                        *p++ = qRed(rgb);
                        *p++ = qGreen(rgb);
                        *p++ = qBlue(rgb);
                    }
                }
                if (bpl != (uint)out->write((char*)buf, bpl)) {
                    iio->setStatus(1);
                    return;
                }
            }
            delete [] buf;
            }
            break;

        case 32: {
            str.insert(1, gray ? '5' : '6');
            str.append("255\n");
            if (out->write(str, str.length()) != str.length()) {
                iio->setStatus(1);
                return;
            }
            uint bpl = w*(gray ? 1 : 3);
            uchar *buf = new uchar[bpl];
            for (uint y=0; y<h; y++) {
                QRgb  *b = (QRgb*)image.scanLine(y);
                uchar *p = buf;
                uchar *end = buf+bpl;
                if (gray) {
                    while (p < end) {
                        uchar g = (uchar)qGray(*b++);
                        *p++ = g;
                    }
                } else {
                    while (p < end) {
                        QRgb rgb = *b++;
                        *p++ = qRed(rgb);
                        *p++ = qGreen(rgb);
                        *p++ = qBlue(rgb);
                    }
                }
                if (bpl != (uint)out->write((char*)buf, bpl)) {
                    iio->setStatus(1);
                    return;
                }
            }
            delete [] buf;
            }
    }

    iio->setStatus(0);
}

#endif // QT_NO_IMAGEIO_PPM

#ifndef QT_NO_ASYNC_IMAGE_IO

class QImageIOFrameGrabber : public QImageConsumer {
public:
    QImageIOFrameGrabber() : framecount(0) { }

    QImageDecoder *decoder;
    int framecount;

    inline void changed(const QRect&) { }
    inline void end() { }
    inline void frameDone(const QPoint&, const QRect&) { framecount++; }
    inline void frameDone() { framecount++; }
    inline void setLooping(int) { }
    inline void setFramePeriod(int) { }
    inline void setSize(int, int) { }
};

static void read_async_image(QImageIO *iio)
{
    const int buf_len = 2048;
    uchar buffer[buf_len];
    QIODevice  *d = iio->ioDevice();
    QImageIOFrameGrabber* consumer = new QImageIOFrameGrabber();
    QImageDecoder decoder(consumer);
    consumer->decoder = &decoder;
    int startAt = d->pos();
    int totLen = 0;

    for (;;) {
        int length = d->read((char*)buffer, buf_len);
        if (length <= 0) {
            iio->setStatus(length);
            break;
        }
        uchar* b = buffer;
        int r = -1;
        while (length > 0 && consumer->framecount==0) {
            r = decoder.decode(b, length);
            if (r <= 0) break;
            b += r;
            totLen += r;
            length -= r;
        }
        if (consumer->framecount) {
            // Stopped after first frame
            if (!d->isSequential())
                d->seek(startAt + totLen);
            else {
                qFatal("We have probably read too much from the stream. No way to put it back!");
            }
            iio->setImage(decoder.image());
            iio->setStatus(0);
            break;
        }
        if (r <= 0) {
            iio->setStatus(r);
            break;
        }
    }

    delete consumer;
}

#endif // QT_NO_ASYNC_IMAGE_IO

#ifndef QT_NO_IMAGEIO_XBM

/*****************************************************************************
  X bitmap image read/write functions
 *****************************************************************************/

static inline int hex2byte(register char *p)
{
    return ((isdigit((uchar) *p) ? *p - '0' : toupper((uchar) *p) - 'A' + 10) << 4) |
           (isdigit((uchar) *(p+1)) ? *(p+1) - '0' : toupper((uchar) *(p+1)) - 'A' + 10);
}

static void read_xbm_image(QImageIO *iio)
{
    const int buflen = 300;
    char buf[buflen];
    QRegExp r1(QLatin1String("^#define[ \t]+[a-zA-Z0-9._]+[ \t]+"));
    QRegExp r2(QLatin1String("[0-9]+"));
    QIODevice *d = iio->ioDevice();
    int w = -1, h = -1;
    QImage image;

    d->readLine(buf, buflen);                        // "#define .._width <num>"
    QString sbuf;
    sbuf = QString::fromLatin1(buf);

    if (r1.indexIn(sbuf) == 0 &&
         r2.indexIn(sbuf, r1.matchedLength()) == r1.matchedLength())
        w = QString(&buf[r1.matchedLength()]).toInt();

    d->readLine(buf, buflen);                        // "#define .._height <num>"
    sbuf = QString::fromLatin1(buf);

    if (r1.indexIn(sbuf) == 0 &&
         r2.indexIn(sbuf, r1.matchedLength()) == r1.matchedLength())
        h = QString(&buf[r1.matchedLength()]).toInt();

    if (w <= 0 || w > 32767 || h <= 0 || h > 32767)
        return;                                        // format error

    for (;;) {                                // scan for data
        if (d->readLine(buf, buflen) <= 0)        // end of file
            return;
        if (strstr(buf,"0x") != 0)                // does line contain data?
            break;
    }

    image.create(w, h, 1, 2, QImage::LittleEndian);
    if (image.isNull())
        return;

    image.setColor(0, qRgb(255,255,255));        // white
    image.setColor(1, qRgb(0,0,0));                // black

    int           x = 0, y = 0;
    uchar *b = image.scanLine(0);
    char  *p = strstr(buf, "0x");
    w = (w+7)/8;                                // byte width

    while (y < h) {                                // for all encoded bytes...
        if (p) {                                // p = "0x.."
            *b++ = hex2byte(p+2);
            p += 2;
            if (++x == w && ++y < h) {
                b = image.scanLine(y);
                x = 0;
            }
            p = strstr(p, "0x");
        } else {                                // read another line
            if (d->readLine(buf,buflen) <= 0)        // EOF ==> truncated image
                break;
            p = strstr(buf, "0x");
        }
    }

    iio->setImage(image);
    iio->setStatus(0);                        // image ok
}


static void write_xbm_image(QImageIO *iio)
{
    QIODevice *d = iio->ioDevice();
    QImage     image = iio->image();
    int               w = image.width();
    int               h = image.height();
    int               i;
    QString    s = fbname(iio->fileName());        // get file base name
    char       buf[100];

    sprintf(buf, "#define %s_width %d\n", s.ascii(), w);
    d->write(buf, qstrlen(buf));
    sprintf(buf, "#define %s_height %d\n", s.ascii(), h);
    d->write(buf, qstrlen(buf));
    sprintf(buf, "static char %s_bits[] = {\n ", s.ascii());
    d->write(buf, qstrlen(buf));

    iio->setStatus(0);

    if (image.depth() != 1)
        image = image.convertDepth(1);        // dither
    if (image.bitOrder() != QImage::LittleEndian)
        image = image.convertBitOrder(QImage::LittleEndian);

    bool invert = qGray(image.color(0)) < qGray(image.color(1));
    char hexrep[16];
    for (i=0; i<10; i++)
        hexrep[i] = '0' + i;
    for (i=10; i<16; i++)
        hexrep[i] = 'a' -10 + i;
    if (invert) {
        char t;
        for (i=0; i<8; i++) {
            t = hexrep[15-i];
            hexrep[15-i] = hexrep[i];
            hexrep[i] = t;
        }
    }
    int bcnt = 0;
    register char *p = buf;
    uchar *b = image.scanLine(0);
    int         x=0, y=0;
    int nbytes = image.numBytes();
    w = (w+7)/8;
    while (nbytes--) {                        // write all bytes
        *p++ = '0';  *p++ = 'x';
        *p++ = hexrep[*b >> 4];
        *p++ = hexrep[*b++ & 0xf];
        if (++x == w && y < h-1) {
            b = image.scanLine(++y);
            x = 0;
        }
        if (nbytes > 0) {
            *p++ = ',';
            if (++bcnt > 14) {
                *p++ = '\n';
                *p++ = ' ';
                *p   = '\0';
                if ((int)qstrlen(buf) != d->write(buf, qstrlen(buf))) {
                    iio->setStatus(1);
                    return;
                }
                p = buf;
                bcnt = 0;
            }
        }
    }
    strcpy(p, " };\n");
    if ((int)qstrlen(buf) != d->write(buf, qstrlen(buf)))
        iio->setStatus(1);
}

#endif // QT_NO_IMAGEIO_XBM


#ifndef QT_NO_IMAGEIO_XPM

/*****************************************************************************
  XPM image read/write functions
 *****************************************************************************/


// Skip until ", read until the next ", return the rest in *buf
// Returns false on error, true on success

static bool read_xpm_string(QByteArray &buf, QIODevice *d, const char * const *source, int &index)
{
    if (source) {
        buf = source[index++];
        return true;
    }

    buf = "";
    char c;
    do {
        if (!d->getChar(&c))
            return false;
    } while (c != '"');

    do {
        if (!d->getChar(&c))
            return false;
        if (c != '"')
            buf.append(c);
    } while (c != '"');

    return true;
}


//
// INTERNAL
//
// Reads an .xpm from either the QImageIO or from the QString *.
// One of the two HAS to be 0, the other one is used.
//

void qt_read_xpm_image_or_array(QImageIO * iio, const char * const * source, QImage & image)
{
    QByteArray buf(200, 0);
    QIODevice *d = 0;

    int i, cpp, ncols, w, h, index = 0;

    if (iio) {
        iio->setStatus(1);
        d = iio ? iio->ioDevice() : 0;
        d->readLine(buf.data(), buf.size());        // "/* XPM */"
        if (buf.indexOf("/* XPM") != 0)
            return;                                        // bad magic
    } else if (!source) {
        return;
    }

    if (!read_xpm_string(buf, d, source, index))
        return;

    if (sscanf(buf, "%d %d %d %d", &w, &h, &ncols, &cpp) < 4)
        return;                                        // < 4 numbers parsed

    if (cpp > 15)
        return;

    if (ncols > 256) {
        image.create(w, h, 32);
    } else {
        image.create(w, h, 8, ncols);
    }

    if (image.isNull())
        return;

    QMap<QString, int> colorMap;
    int currentColor;

    for(currentColor=0; currentColor < ncols; ++currentColor) {
        if (!read_xpm_string(buf, d, source, index)) {
            qWarning("QImage: XPM color specification missing");
            return;
        }
        QString index;
        index = buf.left(cpp);
        buf = buf.mid(cpp).simplified().toLower();
        buf.prepend(" ");
        i = buf.indexOf(" c ");
        if (i < 0)
            i = buf.indexOf(" g ");
        if (i < 0)
            i = buf.indexOf(" g4 ");
        if (i < 0)
            i = buf.indexOf(" m ");
        if (i < 0) {
            qWarning("QImage: XPM color specification is missing: %s", buf.constData());
            return;        // no c/g/g4/m specification at all
        }
        buf = buf.mid(i+3);
        // Strip any other colorspec
        int end = buf.indexOf(' ', 4);
        if (end >= 0)
            buf.truncate(end);
        buf = buf.trimmed();
        if (buf == "none") {
            image.setAlphaBuffer(true);
            int transparentColor = currentColor;
            if (image.depth() == 8) {
                image.setColor(transparentColor,
                                RGB_MASK & qRgb(198,198,198));
                colorMap.insert(index, transparentColor);
            } else {
                QRgb rgb = RGB_MASK & qRgb(198,198,198);
                colorMap.insert(index, rgb);
            }
        } else {
            QRgb c_rgb;
            if (((buf.length()-1) % 3) && (buf[0] == '#')) {
                buf.truncate(((buf.length()-1) / 4 * 3) + 1); // remove alpha channel left by imagemagick
            }
            if (buf[0] == '#') {
                qt_get_hex_rgb(buf, &c_rgb);
            } else {
                qt_get_named_rgb(buf, &c_rgb);
            }
            if (image.depth() == 8) {
                image.setColor(currentColor, 0xff000000 | c_rgb);
                colorMap.insert(index, currentColor);
            } else {
                colorMap.insert(index, 0xff000000 | c_rgb);
            }
        }
    }

    // Read pixels
    for(int y=0; y<h; y++) {
        if (!read_xpm_string(buf, d, source, index)) {
            qWarning("QImage: XPM pixels missing on image line %d", y);
            return;
        }
        if (image.depth() == 8) {
            uchar *p = image.scanLine(y);
            uchar *d = (uchar *)buf.data();
            uchar *end = d + buf.length();
            int x;
            if (cpp == 1) {
                char b[2];
                b[1] = '\0';
                for (x=0; x<w && d<end; x++) {
                    b[0] = *d++;
                    *p++ = (uchar)colorMap[b];
                }
            } else {
                char b[16];
                b[cpp] = '\0';
                for (x=0; x<w && d<end; x++) {
                    strncpy(b, (char *)d, cpp);
                    *p++ = (uchar)colorMap[b];
                    d += cpp;
                }
            }
        } else {
            QRgb *p = (QRgb*)image.scanLine(y);
            uchar *d = (uchar *)buf.data();
            uchar *end = d + buf.length();
            int x;
            char b[16];
            b[cpp] = '\0';
            for (x=0; x<w && d<end; x++) {
                strncpy(b, (char *)d, cpp);
                *p++ = (QRgb)colorMap[b];
                d += cpp;
            }
        }
    }
    if (iio) {
        iio->setImage(image);
        iio->setStatus(0);                        // image ok
    }
}


static void read_xpm_image(QImageIO * iio)
{
    QImage i;
    (void)qt_read_xpm_image_or_array(iio, 0, i);
    return;
}


static const char* xpm_color_name(int cpp, int index)
{
    static char returnable[5];
    static const char code[] = ".#abcdefghijklmnopqrstuvwxyzABCD"
                               "EFGHIJKLMNOPQRSTUVWXYZ0123456789";
    // cpp is limited to 4 and index is limited to 64^cpp
    if (cpp > 1) {
        if (cpp > 2) {
            if (cpp > 3) {
                returnable[3] = code[index % 64];
                index /= 64;
            } else
                returnable[3] = '\0';
            returnable[2] = code[index % 64];
            index /= 64;
        } else
            returnable[2] = '\0';
        // the following 4 lines are a joke!
        if (index == 0)
            index = 64*44+21;
        else if (index == 64*44+21)
            index = 0;
        returnable[1] = code[index % 64];
        index /= 64;
    } else
        returnable[1] = '\0';
    returnable[0] = code[index];

    return returnable;
}


// write XPM image data
static void write_xpm_image(QImageIO * iio)
{
    if (iio)
        iio->setStatus(1);
    else
        return;

    QImage image;
    if (iio->image().depth() != 32)
        image = iio->image().convertDepth(32);
    else
        image = iio->image();

    QMap<QRgb, int> colorMap;

    int w = image.width(), h = image.height(), ncolors = 0;
    int x, y;

    // build color table
    for(y=0; y<h; y++) {
        QRgb * yp = (QRgb *)image.scanLine(y);
        for(x=0; x<w; x++) {
            QRgb color = *(yp + x);
            if (!colorMap.contains(color))
                colorMap.insert(color, ncolors++);
        }
    }

    // number of 64-bit characters per pixel needed to encode all colors
    int cpp = 1;
    for (int k = 64; ncolors > k; k *= 64) {
        ++cpp;
        // limit to 4 characters per pixel
        // 64^4 colors is enough for a 4096x4096 image
         if (cpp > 4)
            break;
    }

    QString line;

    // write header
    QTextStream s(iio->ioDevice());
    s << "/* XPM */" << endl
      << "static char *" << fbname(iio->fileName()) << "[]={" << endl
      << "\"" << w << " " << h << " " << ncolors << " " << cpp << "\"";

    // write palette
    QMap<QRgb, int>::Iterator c = colorMap.begin();
    while (c != colorMap.end()) {
        QRgb color = c.key();
        if (image.hasAlphaBuffer() && color == (color & RGB_MASK))
            line.sprintf("\"%s c None\"",
                          xpm_color_name(cpp, *c));
        else
            line.sprintf("\"%s c #%02x%02x%02x\"",
                          xpm_color_name(cpp, *c),
                          qRed(color),
                          qGreen(color),
                          qBlue(color));
        ++c;
        s << "," << endl << line;
    }

    // write pixels, limit to 4 characters per pixel
    line.truncate(cpp*w);
    for(y=0; y<h; y++) {
        QRgb * yp = (QRgb *) image.scanLine(y);
        int cc = 0;
        for(x=0; x<w; x++) {
            int color = (int)(*(yp + x));
            QByteArray chars(xpm_color_name(cpp, colorMap[color]));
            line[cc++] = chars[0];
            if (cpp > 1) {
                line[cc++] = chars[1];
                if (cpp > 2) {
                    line[cc++] = chars[2];
                    if (cpp > 3) {
                        line[cc++] = chars[3];
                    }
                }
            }
        }
        s << "," << endl << "\"" << line << "\"";
    }
    s << "};" << endl;

    iio->setStatus(0);
}

#endif // QT_NO_IMAGEIO_XPM
