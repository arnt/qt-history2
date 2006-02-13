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

#include "qimage.h"
#include "qdatastream.h"
#include "qbuffer.h"
#include "qmap.h"
#include "qmatrix.h"
#include "qimagereader.h"
#include "qimagewriter.h"
#include "qstringlist.h"
#include "qvariant.h"
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <private/qdrawhelper_p.h>
#include <private/qpixmap_p.h>

#ifdef QT_RASTER_IMAGEENGINE
#include <private/qpaintengine_raster_p.h>
#else
#include <qpaintengine.h>
#endif

#ifdef Q_WS_QWS
#include <qscreen_qws.h> //### for qt_conv...
#endif

#if defined(Q_CC_DEC) && defined(__alpha) && (__DECCXX_VER-0 >= 50190001)
#pragma message disable narrowptr
#endif

typedef void (*_qt_image_cleanup_hook)(int);
Q_GUI_EXPORT _qt_image_cleanup_hook qt_image_cleanup_hook = 0;

struct QImageData {        // internal image data
    QImageData();
    ~QImageData();
    static QImageData *create(const QSize &size, QImage::Format format, int numColors = 0);

    QAtomic ref;

    int width;
    int height;
    int depth;
    int nbytes;               // number of bytes data
    QVector<QRgb> colortable;
    uchar *data;
#ifdef QT3_SUPPORT
    uchar **jumptable;
#endif
    QImage::Format format;
    int bytes_per_line;
    int ser_no;               // serial number
    int detach_no;

    qreal  dpmx;                // dots per meter X (or 0)
    qreal  dpmy;                // dots per meter Y (or 0)
    QPoint  offset;           // offset in pixels
    uint own_data : 1;
    uint has_alpha_clut : 1;

#ifndef QT_NO_IMAGE_TEXT
    QMap<QImageTextKeyLang, QString> text_lang;

    QStringList languages() const
    {
        QStringList r;
        QMap<QImageTextKeyLang,QString>::const_iterator it = text_lang.begin();
        for (; it != text_lang.end(); ++it) {
            r.removeAll(it.key().lang);
            r.append(it.key().lang);
        }
        return r;
    }
    QStringList keys() const
    {
        QStringList r;
        QMap<QImageTextKeyLang,QString>::const_iterator it = text_lang.begin();
        for (; it != text_lang.end(); ++it) {
            r.removeAll(it.key().key);
            r.append(it.key().key);
        }
        return r;
    }

    QMap<QString, QString> text;
#endif
    bool doImageIO(const QImage *image, QImageWriter* io, int quality) const;

    QPaintEngine *paintEngine;
};

Q_GUI_EXPORT qint64 qt_image_id(const QImage &image)
{
    return (((qint64) image.d->ser_no) << 32) | ((qint64) image.d->detach_no);
}

const QVector<QRgb> *qt_image_colortable(const QImage &image)
{
    return (image.d->format <= QImage::Format_Indexed8 && !image.d->colortable.isEmpty()) ? &image.d->colortable : 0;
}

extern int qt_defaultDpi();

QBasicAtomic qimage_serial_number = Q_ATOMIC_INIT(1);
int qimage_next_serial_number()
{
    register int id;
    for (;;) {
        id = qimage_serial_number;
        if (qimage_serial_number.testAndSet(id, id + 1))
            break;
    }
    return id;
}


QImageData::QImageData()
{
    ser_no = qimage_next_serial_number();
    detach_no = 0;
    ref = 0;

    width = height = depth = 0;
    nbytes = 0;
    data = 0;
    own_data = true;
#ifdef QT3_SUPPORT
    jumptable = 0;
#endif
    bytes_per_line = 0;
    format = QImage::Format_ARGB32;

    dpmx = qt_defaultDpi()*100./2.54;
    dpmy = qt_defaultDpi()*100./2.54;
    offset = QPoint(0,0);

    paintEngine = 0;
}

static int depthForFormat(QImage::Format format)
{
    int depth = 0;
    switch(format) {
    case QImage::Format_Invalid:
    case QImage::NImageFormats:
        Q_ASSERT(false);
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
        depth = 1;
        break;
    case QImage::Format_Indexed8:
        depth = 8;
        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        depth = 32;
        break;
#ifdef Q_WS_QWS
    case QImage::Format_RGB16:
        depth = 16;
        break;
#endif
    }
    return depth;
}

QImageData * QImageData::create(const QSize &size, QImage::Format format, int numColors)
{
    int width = size.width();
    int height = size.height();
    if (width <= 0 || height <= 0 || numColors < 0 || format == QImage::Format_Invalid)
        return 0;                                // invalid parameter(s)

    int depth = 0;
    switch(format) {
    case QImage::NImageFormats:
    case QImage::Format_Invalid:
        Q_ASSERT(false);
    case QImage::Format_Mono:
    case QImage::Format_MonoLSB:
        depth = 1;
        numColors = 2;
        break;
    case QImage::Format_Indexed8:
        depth = 8;
        numColors = qMin(numColors, 256);
        numColors = qMax(0, numColors);
        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        depth = 32;
        numColors = 0;
        break;
#ifdef Q_WS_QWS
    case QImage::Format_RGB16:
        depth = 16;
        numColors = 0;
        break;
#endif
    }

    QImageData *d = new QImageData;
    d->colortable.resize(numColors);
    if (depth == 1) {
        d->colortable[0] = QColor(Qt::black).rgba();
        d->colortable[1] = QColor(Qt::white).rgba();
    } else {
        for (int i = 0; i < numColors; ++i)
            d->colortable[i] = 0;
    }

    d->width = width;
    d->height = height;
    d->depth = depth;
    d->format = format;
    d->has_alpha_clut = false;

    d->bytes_per_line = ((width * d->depth + 31) >> 5) << 2; // bytes per scanline (must be multiple of 8)

    d->nbytes = d->bytes_per_line*height;
    d->data  = (uchar *)malloc(d->nbytes);

    if (!d->data) {
        delete d;
        return 0;
    }

    d->ref.ref();
    return d;

}

QImageData::~QImageData()
{
    delete paintEngine;
    if (data && own_data)
        free(data);
#ifdef QT3_SUPPORT
    if (jumptable)
        free(jumptable);
    jumptable = 0;
#endif
    data = 0;
}

/*!
    \class QImage

    \ingroup multimedia
    \ingroup shared
    \mainclass

    \brief The QImage class provides a hardware-independent image
    representation that allows direct access to the pixel data, and
    can be used as a paint device.

    Qt provides four classes for handling image data: QImage, QPixmap,
    QBitmap and QPicture.  QImage is designed and optimized for I/O,
    and for direct pixel access and manipulation, while QPixmap is
    designed and optimized for showing images on screen. QBitmap is
    only a convenience class that inherits QPixmap, ensuring a
    depth of 1. Finally, the QPicture class is a paint device that
    records and replays QPainter commands.

    Because QImage is a QPaintDevice subclass, QPainter can be used to
    draw directly onto images.  When using QPainter on a QImage, the
    painting can be performed in another thread than the current GUI
    thread, that is except rendering text (because QFont is GUI
    dependent). To render text in another thread, the text must first
    be derived as a QPainterPath in the GUI thread.

    The QImage class supports several image formats described by the
    \l Format enum. These include monochrome, 8-bit, 32-bit and
    alpha-blended images which are available in all versions of Qt
    4.x. In addition, QImage supports several formats that are
    specific to \l {Qtopia Core}.

    QImage provides a collection of functions that can be used to
    obtain a variety of information about the image. There are also
    several functions that enables transformation of the image.

    QImage objects can be passed around by value since the QImage
    class uses \l{Implicit Data Sharing}{implicit data
    sharing}. QImage objects can also be streamed and compared.

    \tableofcontents

    \section1 Reading and Writing Image Files

    QImage provides several ways of loading an image file: The file
    can be loaded when constructing the QImage object, or by using the
    load() or loadFromData() functions later on. QImage also provides
    the static fromData() function, constructing a QImage from the
    given data.  When loading an image, the file name can either refer
    to an actual file on disk or to one of the application's embedded
    resources. See \l{The Qt Resource System} overview for details
    on how to embed images and other resource files in the
    application's executable.

    Simply call the save() function to save a QImage object.

    The complete list of supported file formats are available through
    the QImageReader::supportedImageFormats() and
    QImageWriter::supportedImageFormats() functions. New file formats
    can be added as plugins. By default, Qt supports the following
    formats:

    \table
    \header \o Format \o Description                      \o Qt's support
    \row    \o BMP    \o Windows Bitmap                   \o Read/write
    \row    \o GIF    \o Graphic Interchange Format (optional) \o Read
    \row    \o JPG    \o Joint Photographic Experts Group \o Read/write
    \row    \o JPEG   \o Joint Photographic Experts Group \o Read/write
    \row    \o PNG    \o Portable Network Graphics        \o Read/write
    \row    \o PBM    \o Portable Bitmap                  \o Read
    \row    \o PGM    \o Portable Graymap                 \o Read
    \row    \o PPM    \o Portable Pixmap                  \o Read/write
    \row    \o XBM    \o X11 Bitmap                       \o Read/write
    \row    \o XPM    \o X11 Pixmap                       \o Read/write
    \endtable

    (To configure Qt with GIF support, pass \c -qt-gif to the \c
    configure script or check the appropriate option in the graphical
    installer.)

    \section1 Image Information

    QImage provides a collection of functions that can be used to
    obtain a variety of information about the image:

    \table
    \header
    \o \o Available Functions

    \row
    \o Geometry
    \o

    The size(), width(), height(), dotsPerMeterX(), and
    dotsPerMeterY() functions provide information about the image size
    and aspect ratio.

    The rect() function returns the image's enclosing rectangle. The
    valid() function tells if a given pair of coordinates is within
    this rectangle. The offset() function returns the number of pixels
    by which the image is intended to be offset by when positioned
    relative to other images, which also can be manipulated using the
    setOffset() function.

    \row
    \o Colors
    \o

    The color of a pixel can be retrieved by passing its coordinates
    to the pixel() function.  The pixel() function returns the color
    as a QRgb value indepedent of the image's format.

    In case of monochrome and 8-bit images, the numColors() and
    colorTable() functions provide information about the color
    components used to store the image data: The colorTable() function
    returns the image's entire color table. To obtain a single entry,
    use the pixelIndex() function to retrieve the pixel index for a
    given pair of coordinates, then use the color() function to
    retrieve the color.

    The hasAlphaChannel() function tells if the image's format
    respects the alpha channel, or not. The allGray() and
    isGrayscale() functions tell whether an image's colors are all
    shades of gray.

    See also the \l {QImage#Pixel Manipulation}{Pixel Manipulation}
    and \l {QImage#Image Transformations}{Image Transformations}
    sections.

    \row
    \o Text
    \o

    The text() function returns the image text associated with the
    given text key. An image's text keys can be retrieved using the
    textKeys() function. Use the setText() function to alter an
    image's text.

    \row
    \o Low-level information
    \o
    The depth() function returns the depth of the image. The supported
    depths are 1 (monochrome), 8 and 32 (for more information see the
    \l {QImage#Image Formats}{Image Formats} section).

    The format(), bytesPerLine(), and numBytes() functions provide
    low-level information about the data stored in the image.

    The serialNumber() function returns a number that uniquely
    identifies the contents of this QImage object.
    \endtable

    \section1 Pixel Manipulation

    The functions used to manipulate an image's pixels depend on the
    image format. The reason is that monochrome and 8-bit images are
    index-based and use a color lookup table, while 32-bit images
    store ARGB values directly. For more information on image formats,
    see the \l {Image Formats} section.

    In case of a 32-bit image, the setPixel() function can be used to
    alter the color of the pixel at the given coordinates to any other
    color specified as an ARGB quadruplet. To make a suitable QRgb
    value, use the qRgb() (adding a default alpha component to the
    given RGB values, i.e. creating an opaque color) or qRgba()
    function. For example:

    \table
    \row
    \o \inlineimage qimage-32bit_scaled.png
    \o
    \code
        QImage image(3, 3, QImage::Format_RGB32);
        QRgb value;

        value = qRgb(189, 149, 39); // 0xffbd9527
        image.setPixel(1, 1, value);

        value = qRgb(122, 163, 39); // 0xff7aa327
        image.setPixel(0, 1, value);
        image.setPixel(1, 0, value);

        value = qRgb(237, 187, 51); // 0xffedba31
        image.setPixel(2, 1, value);
    \endcode
    \header
    \o {2,1}32-bit
    \endtable

    In case of a 8-bit and monchrome images , the pixel value is only
    an index from the image's color table. So the setPixel() function
    can only be used to alter the color of the pixel at the given
    coordinates to a predefined color from the image's color table,
    i.e. it can only change the pixel's index value. To alter or add a
    color to an image's color table, use the setColor() function.

    An entry in the color table is an ARGB quadruplet encoded as an
    QRgb value. Use the qRgb() and qRgba() functions to make a
    suitable QRgb value for use with the setColor() function. For
    example:

    \table
    \row
    \o \inlineimage qimage-8bit_scaled.png
    \o
    \code
        QImage image(3, 3, QImage::Format_Indexed8);
        QRgb value;

        value = qRgb(122, 163, 39); // 0xff7aa327
        image.setColor(0, value);

        value = qRgb(237, 187, 51); // 0xffedba31
        image.setColor(1, value);

        value = qRgb(189, 149, 39); // 0xffbd9527
        image.setColor(2, value);

        image.setPixel(0, 1, 0);
        image.setPixel(1, 0, 0);
        image.setPixel(1, 1, 2);
        image.setPixel(2, 1, 1);
    \endcode
    \header
    \o {2,1} 8-bit
    \endtable

    QImage also provide the scanLine() function which returns a
    pointer to the pixel data at the scanline with the given index,
    and the bits() function which returns a pointer to the first pixel
    data (this is equivalent to \c scanLine(0)).

    \section1 Image Formats

    Each pixel stored in a QImage is represented by an integer. The
    size of the integer varies depending on the format. QImage
    supports several image formats described by the \l Format
    enum. The monochrome (1-bit), 8-bit and 32-bit images are
    available in all versions of Qt. In addition Qtopia Core also
    supports 2-bit, 4-bit, and 16-bit images. For more information
    about the Qtopia specific formats, see the documentation of the \l
    Format enum.

    Monochrome images are stored using 1-bit indexes into a color table
    with at most two colors. There are two different types of
    monochrome images: big endian (MSB first) or little endian (LSB
    first) bit order.

    8-bit images are stored using 8-bit indexes into a color table,
    i.e.  they have a single byte per pixel. The color table is a
    QVector<QRgb>, and the QRgb typedef is equivalent to an unsigned
    int containing an ARGB quadruplet on the format 0xAARRGGBB.

    32-bit images have no color table; instead, each pixel contains an
    QRgb value. There are three different types of 32-bit images
    storing RGB (i.e. 0xffRRGGBB), ARGB and premultiplied ARGB
    values respectively. In the premultiplied format the red, green,
    and blue channels are multiplied by the alpha component divided by
    255.

    An image's format can be retrieved using the format()
    function. Use the convertToFormat() functions to convert an image
    into another format. The allGray() and isGrayscale() functions
    tell whether a color image can safely be converted to a grayscale
    image.

    \section1 Image Transformations

    QImage supports a number of functions for creating a new image
    that is a transformed version of the original: The
    createAlphaMask() function builds and returns a 1-bpp mask from
    the alpha buffer in this image, and the createHeuristicMask()
    function creates and returns a 1-bpp heuristic mask for this
    image. The latter function works by selecting a color from one of
    the corners, then chipping away pixels of that color starting at
    all the edges.

    The mirrored() function returns a mirror of the image in the
    desired direction, the scaled() returns a copy of the image scaled
    to a rectangle of the desired measures, the rgbSwapped() fucntion
    constructs a BGR image from a RGB image, and the alphaChannel()
    function constructs an image from this image's alpha channel.

    The scaledToWidth() and scaledToHeight() functions return scaled
    copies of the image.

    The transformed() function returns a copy of the image that is
    transformed with the given transformation matrix and
    transformation mode: Internally, the transformation matrix is
    adjusted to compensate for unwanted translation,
    i.e. transformed() returns the smallest image containing all
    transformed points of the original image. The static trueMatrix()
    function returns the actual matrix used for transforming the
    image.

    There are also functions for changing attributes of an image
    in-place:

    \table
    \header \o Function \o Description
    \row
    \o setAlphaChannel()
    \o Sets the alpha channel of the image.
    \row
    \o setDotsPerMeterX()
    \o Defines the aspect ratio by setting the number of pixels that fit
    horizontally in a physical meter.
    \row
    \o setDotsPerMeterY()
    \o Defines the aspect ratio by setting the number of pixels that fit
    vertically in a physical meter.
    \row
    \o fill()
    \o Fills the entire image with the given pixel value.
    \row
    \o invertPixels()
    \o Inverts all pixel values in the image using the given InvertMode value.
    \row
    \o setColorTable()
    \o Sets the color table used to translate color indexes. Only
    monochrome and 8-bit formats.
    \row
    \o setNumColors()
    \o Resizes the color table. Only monochrome and 8-bit formats.

    \endtable

    \sa QImageReader, QImageWriter, QPixmap
*/

/*!
    \enum QImage::Endian
    \compat

    This enum type is used to describe the endianness of the CPU and
    graphics hardware. It is provided here for compatibility with earlier versions of Qt.

    Use the \l Format enum instead. The \l Format enum specify the
    endianess for monchrome formats, but for other formats the
    endianess is not relevant.

    \value IgnoreEndian  Endianness does not matter. Useful for some
                         operations that are independent of endianness.
    \value BigEndian     Most significant bit first or network byte order, as on SPARC, PowerPC, and Motorola CPUs.
    \value LittleEndian  Least significant bit first or little endian byte order, as on Intel x86.
*/

/*!
    \enum QImage::InvertMode

    This enum type is used to describe how pixel values should be
    inverted in the invertPixels() function.

    \value InvertRgb    Invert only the RGB values and leave the alpha
                        channel unchanged.

    \value InvertRgba   Invert all channels, including the alpha channel.

    \sa invertPixels()
*/

/*!
    \enum QImage::Format

    The following image formats are available in all versions of Qt:

    \value Format_Invalid   The image is invalid.
    \value Format_Mono      The image is stored using 1-bit per pixel. Bytes are
                            packed with the most significant bit (MSB) first.
    \value Format_MonoLSB   The image is stored using 1-bit per pixel. Bytes are
                            packed with the less significant bit (LSB) first.
    \value Format_Indexed8  The image is stored using 8-bit indexes into a colormap.
    \value Format_RGB32     The image is stored using a 32-bit RGB format (0xffRRGGBB).
    \value Format_ARGB32    The image is stored using a 32-bit ARGB format (0xAARRGGBB).
    \value Format_ARGB32_Premultiplied  The image is stored using a premultiplied 32-bit
                            ARGB format (0xAARRGGBB), i.e. the red,
                            green, and blue channels are multiplied
                            by the alpha component divided by 255. (If RR, GG, or BB
                            has a higher value than the alpha channel, the results are undefined.)

    The following image format is specific to \l{Qtopia Core}:

    \value Format_RGB16     The image is stored using a 16-bit RGB format (5-6-5).

    \sa format(), convertToFormat()
*/

/*****************************************************************************
  QImage member functions
 *****************************************************************************/

// table to flip bits
static const uchar bitflip[256] = {
    /*
        open OUT, "| fmt";
        for $i (0..255) {
            print OUT (($i >> 7) & 0x01) | (($i >> 5) & 0x02) |
                      (($i >> 3) & 0x04) | (($i >> 1) & 0x08) |
                      (($i << 7) & 0x80) | (($i << 5) & 0x40) |
                      (($i << 3) & 0x20) | (($i << 1) & 0x10), ", ";
        }
        close OUT;
    */
    0, 128, 64, 192, 32, 160, 96, 224, 16, 144, 80, 208, 48, 176, 112, 240,
    8, 136, 72, 200, 40, 168, 104, 232, 24, 152, 88, 216, 56, 184, 120, 248,
    4, 132, 68, 196, 36, 164, 100, 228, 20, 148, 84, 212, 52, 180, 116, 244,
    12, 140, 76, 204, 44, 172, 108, 236, 28, 156, 92, 220, 60, 188, 124, 252,
    2, 130, 66, 194, 34, 162, 98, 226, 18, 146, 82, 210, 50, 178, 114, 242,
    10, 138, 74, 202, 42, 170, 106, 234, 26, 154, 90, 218, 58, 186, 122, 250,
    6, 134, 70, 198, 38, 166, 102, 230, 22, 150, 86, 214, 54, 182, 118, 246,
    14, 142, 78, 206, 46, 174, 110, 238, 30, 158, 94, 222, 62, 190, 126, 254,
    1, 129, 65, 193, 33, 161, 97, 225, 17, 145, 81, 209, 49, 177, 113, 241,
    9, 137, 73, 201, 41, 169, 105, 233, 25, 153, 89, 217, 57, 185, 121, 249,
    5, 133, 69, 197, 37, 165, 101, 229, 21, 149, 85, 213, 53, 181, 117, 245,
    13, 141, 77, 205, 45, 173, 109, 237, 29, 157, 93, 221, 61, 189, 125, 253,
    3, 131, 67, 195, 35, 163, 99, 227, 19, 147, 83, 211, 51, 179, 115, 243,
    11, 139, 75, 203, 43, 171, 107, 235, 27, 155, 91, 219, 59, 187, 123, 251,
    7, 135, 71, 199, 39, 167, 103, 231, 23, 151, 87, 215, 55, 183, 119, 247,
    15, 143, 79, 207, 47, 175, 111, 239, 31, 159, 95, 223, 63, 191, 127, 255
};

const uchar *qt_get_bitflip_array()                        // called from QPixmap code
{
    return bitflip;
}

#if defined(QT3_SUPPORT) || defined(Q_WS_QWS)
static QImage::Format formatFor(int depth, QImage::Endian bitOrder)
{
    QImage::Format format;
    if (depth == 1) {
        format = bitOrder == QImage::BigEndian ? QImage::Format_Mono : QImage::Format_MonoLSB;
    } else if (depth == 8) {
        format = QImage::Format_Indexed8;
    } else if (depth == 32) {
        format = QImage::Format_RGB32;
#ifdef Q_WS_QWS
    } else if (depth == 16) {
        format = QImage::Format_RGB16;
#endif
    } else {
        qWarning("QImage: depth %d not supported", depth);
        format = QImage::Format_Invalid;
    }
    return format;
}
#endif

/*!
    Constructs a null image.

    \sa isNull()
*/

QImage::QImage()
    : QPaintDevice()
{
    d = 0;
}

/*!
    Constructs an image with the given \a width, \a height and \a
    format.
*/
QImage::QImage(int width, int height, Format format)
    : QPaintDevice()
{
    d = QImageData::create(QSize(width, height), format, 0);
}

/*!
    Constructs an image with the given \a size and \a format.
*/
QImage::QImage(const QSize &size, Format format)
    : QPaintDevice()
{
    d = QImageData::create(size, format, 0);
}

/*!
    Constructs an image with the given \a width, \a height and \a
    format, that uses an existing memory buffer, \a data. The \a width
    and \a height must be specified in pixels, and that \a data must
    be 32-bit aligned.

    The buffer must remain valid throughout the life of the
    QImage. The image does not delete the buffer at destruction.

    If the image is in an indexed color format, set the color table
    for the image using setColorTable().
*/
QImage::QImage(uchar* data, int width, int height, Format format)
    : QPaintDevice()
{
    d = 0;
    if (format == Format_Invalid || width <= 0 || height <= 0 || !data)
        return;                                        // invalid parameter(s)
    d = new QImageData;
    d->ref.ref();

    d->own_data = false;
    d->data = data;
    d->width = width;
    d->height = height;
    d->depth = depthForFormat(format);
    d->format = format;

    d->bytes_per_line = ((width * d->depth + 31)/32) * 4;
    d->nbytes = d->bytes_per_line * height;
}

/*!
    Constructs an image and tries to load the image from the file with
    the given \a fileName.

    The loader attempts to read the image using the specified \a
    format. If the \a format is not specified (which is the default),
    the loader probes the file for a header to guess the file format.

    If the loading of the image failed, this object is a null image.

    The file name can either refer to an actual file on disk or to one
    of the application's embedded resources. See the
    \l{resources.html}{Resource System} overview for details on how to
    embed images and other resource files in the application's
    executable.

    \sa isNull(), {QImage#Reading and Writing Image Files}{Reading and Writing Image Files}
*/

QImage::QImage(const QString &fileName, const char *format)
    : QPaintDevice()
{
    d = 0;
    load(fileName, format);
}

/*!
    Constructs an image and tries to load the image from the file with
    the given \a fileName.

    The loader attempts to read the image using the specified \a
    format. If the \a format is not specified (which is the default),
    the loader probes the file for a header to guess the file format.

    If the loading of the image failed, this object is a null image.

    The file name can either refer to an actual file on disk or to one
    of the application's embedded resources. See the
    \l{resources.html}{Resource System} overview for details on how to
    embed images and other resource files in the application's
    executable.

    You can disable this constructor by defining \c
    QT_NO_CAST_FROM_ASCII when you compile your applications. This can
    be useful, for example, if you want to ensure that all
    user-visible strings go through QObject::tr().

    \sa QString::fromAscii(), isNull(), {QImage#Reading and Writing
    Image Files}{Reading and Writing Image Files}
*/
QImage::QImage(const char *fileName, const char *format)
    : QPaintDevice()
{
    // ### Qt 5: if you remove the QImage(const QByteArray &) QT3_SUPPORT
    // constructor, remove this constructor as well. The constructor here
    // exists so that QImage("foo.png") compiles without ambiguity.
    d = 0;
    load(QString::fromAscii(fileName), format);
}

#ifndef QT_NO_IMAGEFORMAT_XPM
extern bool qt_read_xpm_image_or_array(QIODevice *device, const char * const *source, QImage &image);

/*!
    Constructs an image from the given \a xpm image.

    Make sure that the image is a valid XPM image. Errors are silently
    ignored.

    Note that it's possible to squeeze the XPM variable a little bit
    by using an unusual declaration:

    \code
        static const char * const start_xpm[] = {
            "16 15 8 1",
            "a c #cec6bd",
        ....
    \endcode

    The extra \c const makes the entire definition read-only, which is
    slightly more efficient (e.g. when the code is in a shared
    library) and ROMable when the application is to be stored in ROM.
*/

QImage::QImage(const char * const xpm[])
    : QPaintDevice()
{
    d = 0;
    if (!qt_read_xpm_image_or_array(0, xpm, *this)) {
        // We use a qFatal rather than disabling the whole function,
        // as this constructor may be ambiguous.
        qFatal("QImage::QImage(), XPM is not supported");
    }
}
#endif // QT_NO_IMAGEFORMAT_XPM

/*!
    \fn QImage::QImage(const QByteArray &data)

    Use the static fromData() function instead.

    \oldcode
        QByteArray data;
        ...
        QImage image(data);
    \newcode
        QByteArray data;
        ...
        QImage image = QImage::fromData(data);
    \endcode
*/


/*!
    Constructs a shallow copy of the given \a image.

    For more information about shallow copies, see the \l {Implicit
    Data Sharing} documentation.

    \sa copy()
*/

QImage::QImage(const QImage &image)
    : QPaintDevice()
{
    d = image.d;
    if (d)
        d->ref.ref();
}

#ifdef QT3_SUPPORT
/*!
    \fn QImage::QImage(int width, int height, int depth, int numColors, Endian bitOrder)

    Constructs an image with the given \a width, \a height, \a depth,
    \a numColors colors and \a bitOrder.

    Use the constructor that accepts a width, a height and a format
    (i.e. specifying the depth and bit order), in combination with the
    setNumColors() function, instead.

    \oldcode
        QImage image(width, height, depth, numColors);
    \newcode
        QImage image(width, height, format);

        // For 8 bit images the default number of colors is 256. If
        // another number of colors is required it can be specified
        // using the setNumColors() function.
        image.setNumColors(numColors);
    \endcode
*/

QImage::QImage(int w, int h, int depth, int numColors, Endian bitOrder)
    : QPaintDevice()
{
    d = QImageData::create(QSize(w, h), formatFor(depth, bitOrder), numColors);
}

/*!
    Constructs an image with the given \a size, \a depth, \a numColors
    and \a bitOrder.

    Use the constructor that accepts a size and a format
    (i.e. specifying the depth and bit order), in combination with the
    setNumColors() function, instead.

    \oldcode
        QSize mySize(width, height);
        QImage image(mySize, depth, numColors);
    \newcode
        QSize mySize(width, height);
        QImage image(mySize, format);

        // For 8 bit images the default number of colors is 256. If
        // another number of colors is required it can be specified
        // using the setNumColors() function.
        image.setNumColors(numColors);
    \endcode
*/
QImage::QImage(const QSize& size, int depth, int numColors, Endian bitOrder)
    : QPaintDevice()
{
    d = QImageData::create(size, formatFor(depth, bitOrder), numColors);
}

/*!
    \fn QImage::QImage(uchar* data, int width, int height, int depth, const QRgb* colortable, int numColors, Endian bitOrder)

    Constructs an image with the given \a width, \a height, depth, \a
    colortable, \a numColors and \a bitOrder, that uses an existing
    memory buffer, \a data.

    Use the constructor that accepts a uchar pointer, a width, a
    height and a format (i.e. specifying the depth and bit order), in
    combination with the setColorTable() function, instead.

    \oldcode
        uchar *myData;
        QRgb *myColorTable;

        QImage image(myData, width, height, depth,
                               myColorTable, numColors, IgnoreEndian);
    \newcode
        uchar *myData;
        QVector<QRgb> myColorTable;

        QImage image(myData, width, height, format);
        image.setColorTable(myColorTable);
    \endcode
*/
QImage::QImage(uchar* data, int w, int h, int depth, const QRgb* colortable, int numColors, Endian bitOrder)
    : QPaintDevice()
{
    d = 0;
    Format f = formatFor(depth, bitOrder);
    if (f == Format_Invalid)
        return;
    if (w <= 0 || h <= 0 || numColors < 0 || !data)
        return;                                        // invalid parameter(s)
    d = new QImageData;
    d->ref.ref();

    d->own_data = false;
    d->data = data;
    d->width = w;
    d->height = h;
    d->depth = depth;
    d->format = f;
    if (depth == 32)
        numColors = 0;

    d->bytes_per_line = ((w*depth+31)/32)*4;        // bytes per scanline
    d->nbytes = d->bytes_per_line * h;
    if (colortable) {
        d->colortable.resize(numColors);
        for (int i = 0; i < numColors; ++i)
            d->colortable[i] = colortable[i];
    } else if (numColors) {
        setNumColors(numColors);
    }
}

#endif
#ifdef Q_WS_QWS

/*!
    \fn QImage::QImage(uchar* data, int width, int height, int depth, int bytesPerLine, const QRgb* colortable, int numColors, Endian bitOrder)

    Constructs an image with the given \a width, \a height, \a depth,
    \a bytesPerLine, \a colortable, \a numColors and \a bitOrder, that
    uses an existing memory buffer, \a data. The image does not delete
    the buffer at destruction.

    \warning This constructor is only available in Qtopia Core.

    The data has to be 32-bit aligned, so it's no longer possible to specify
    a custom \a bytesPerLine.
*/
QImage::QImage(uchar* data, int w, int h, int depth, int bpl, const QRgb* colortable, int numColors, Endian bitOrder)
    : QPaintDevice()
{
    d = 0;
    Format f = formatFor(depth, bitOrder);
    if (f == Format_Invalid)
        return;
    if (!data || w <= 0 || h <= 0 || depth <= 0 || numColors < 0)
        return;                                        // invalid parameter(s)

    d = new QImageData;
    d->ref.ref();
    d->own_data = false;
    d->data = data;
    d->width = w;
    d->height = h;
    d->depth = depth;
    d->format = f;
    if (depth == 32)
        numColors = 0;
    d->bytes_per_line = bpl;
    d->nbytes = d->bytes_per_line * h;
    if (colortable) {
        d->colortable.reserve(numColors);
        for (int i = 0; i < numColors; ++i)
            d->colortable[i] = colortable[i];
    } else if (numColors) {
        setNumColors(numColors);
    }
}
#endif // Q_WS_QWS

/*!
    Destroys the image and cleans up.
*/

QImage::~QImage()
{
    if (d && !d->ref.deref()) {
        if (qt_image_cleanup_hook)
            qt_image_cleanup_hook(d->ser_no);
        delete d;
    }
}

/*!
    Assigns a shallow copy of the given \a image to this image and
    returns a reference to this image.

    For more information about shallow copies, see the \l {Implicit
    Data Sharing} documentation.

    \sa copy(), QImage()
*/

QImage &QImage::operator=(const QImage &image)
{
    QImageData *x = image.d;
    if (x)
        x->ref.ref();
    x = qAtomicSetPtr(&d, x);
    if (x && !x->ref.deref())
        delete x;
    return *this;
}

/*!
  \internal
*/
int QImage::devType() const
{
    return QInternal::Image;
}

/*!
   Returns the image as a QVariant.
*/
QImage::operator QVariant() const
{
    return QVariant(QVariant::Image, this);
}

/*!
    \internal

    If multiple images share common data, this image makes a copy of
    the data and detaches itself from the sharing mechanism, making
    sure that this image is the only one referring to the data.

    Nothing is done if there is just a single reference.

    \sa copy(), isDetached(), {Implicit Data Sharing}
*/

void QImage::detach()
{
    if (d) {
        ++d->detach_no;
        if (d->ref != 1)
            *this = copy();
    }
}


/*!
    \fn QImage QImage::copy(int x, int y, int width, int height) const
    \overload

    The returned image is copied from the position (\a x, \a y) in
    this image, and will always have the given \a width and \a height.
    In areas beyond this image, pixels are set to 0.

*/

/*!
    \fn QImage QImage::copy(const QRect& rectangle) const

    Returns a sub-area of the image as a new image.

    The returned image is copied from the position (\a
    {rectangle}.x(), \a{rectangle}.y()) in this image, and will always
    have the size of the given \a rectangle.

    In areas beyond this image, pixels are set to 0. For 32-bit RGB
    images, this means black; for 32-bit ARGB images, this means
    transparent black; for 8-bit images, this means the color with
    index 0 in the color table which can be anything; for 1-bit
    images, this means Qt::color0.

    If the given \a rectangle is a null rectangle the entire image is
    copied.

    \sa QImage()
*/
QImage QImage::copy(const QRect& r) const
{
    if (!d)
        return QImage();

    if (r.isNull()) {
        QImage image(d->width, d->height, d->format);

#ifdef Q_WS_QWS
        // Qtopia Core can create images with non-default bpl
        // make sure we don't crash.
        if (image.d->nbytes != d->nbytes) {
            int bpl = image.bytesPerLine();
            for (int i = 0; i < height(); i++)
                memcpy(image.scanLine(i), scanLine(i), bpl);
        } else
#endif
            memcpy(image.bits(), bits(), d->nbytes);
        image.d->colortable = d->colortable;
        image.d->dpmx = d->dpmx;
        image.d->dpmy = d->dpmy;
        image.d->offset = d->offset;
        image.d->has_alpha_clut = d->has_alpha_clut;
#ifndef QT_NO_IMAGE_TEXT
        image.d->text_lang = d->text_lang;
#endif
        return image;
    }

    int x = r.x();
    int y = r.y();
    int w = r.width();
    int h = r.height();

    int dx = 0;
    int dy = 0;
    if (w <= 0 || h <= 0)
        return QImage();

    QImage image(w, h, d->format);

    if (x < 0 || y < 0 || x + w > d->width || y + h > d->height) {
        // bitBlt will not cover entire image - clear it.
        image.fill(0);
        if (x < 0) {
            dx = -x;
            x = 0;
        }
        if (y < 0) {
            dy = -y;
            y = 0;
        }
    }

    image.d->colortable = d->colortable;

    int pixels_to_copy = w - dx;
    if (x > d->width)
        pixels_to_copy = 0;
    else if (pixels_to_copy > d->width - x)
        pixels_to_copy = d->width - x;
    int lines_to_copy = h - dy;
    if (y > d->height)
        lines_to_copy = 0;
    else if (lines_to_copy > d->height - y)
        lines_to_copy = d->height - y;

    bool byteAligned = true;
    if (d->format == Format_Mono || d->format == Format_MonoLSB)
        byteAligned = !(dx & 7) && !(x & 7) && !(pixels_to_copy & 7);

    if (byteAligned) {
        const uchar *src = d->data + ((x * d->depth) >> 3) + y * d->bytes_per_line;
        uchar *dest = image.d->data + ((dx * d->depth) >> 3) + dy * image.d->bytes_per_line;
        const int bytes_to_copy = (pixels_to_copy * d->depth) >> 3;
        for (int i = 0; i < lines_to_copy; ++i) {
            memcpy(dest, src, bytes_to_copy);
            src += d->bytes_per_line;
            dest += image.d->bytes_per_line;
        }
    } else if (d->format == Format_Mono) {
        const uchar *src = d->data + y * d->bytes_per_line;
        uchar *dest = image.d->data + dy * image.d->bytes_per_line;
        for (int i = 0; i < lines_to_copy; ++i) {
            for (int j = 0; j < pixels_to_copy; ++j) {
                if (src[(x + j) >> 3] & (0x80 >> ((x + j) & 7)))
                    dest[(dx + j) >> 3] |= (0x80 >> ((dx + j) & 7));
                else
                    dest[(dx + j) >> 3] &= ~(0x80 >> ((dx + j) & 7));
            }
            src += d->bytes_per_line;
            dest += image.d->bytes_per_line;
        }
    } else { // Format_MonoLSB
        Q_ASSERT(d->format == Format_MonoLSB);
        const uchar *src = d->data + y * d->bytes_per_line;
        uchar *dest = image.d->data + dy * image.d->bytes_per_line;
        for (int i = 0; i < lines_to_copy; ++i) {
            for (int j = 0; j < pixels_to_copy; ++j) {
                if (src[(x + j) >> 3] & (0x1 << ((x + j) & 7)))
                    dest[(dx + j) >> 3] |= (0x1 << ((dx + j) & 7));
                else
                    dest[(dx + j) >> 3] &= ~(0x1 << ((dx + j) & 7));
            }
            src += d->bytes_per_line;
            dest += image.d->bytes_per_line;
        }
    }

    image.d->dpmx = dotsPerMeterX();
    image.d->dpmy = dotsPerMeterY();
    image.d->offset = offset();
    image.d->has_alpha_clut = d->has_alpha_clut;
#ifndef QT_NO_IMAGE_TEXT
    image.d->text_lang = d->text_lang;
#endif
    return image;
}


/*!
    \fn bool QImage::isNull() const

    Returns true if it is a null image, otherwise returns false.

    A null image has all parameters set to zero and no allocated data.
*/
bool QImage::isNull() const
{
    return !d;
}

/*!
    \fn int QImage::width() const

    Returns the width of the image.

    \sa {QImage#Image Information}{Image Information}
*/
int QImage::width() const
{
    return d ? d->width : 0;
}

/*!
    \fn int QImage::height() const

    Returns the height of the image.

    \sa {QImage#Image Information}{Image Information}
*/
int QImage::height() const
{
    return d ? d->height : 0;
}

/*!
    \fn QSize QImage::size() const

    Returns the size of the image, i.e. its width() and height().

    \sa {QImage#Image Information}{Image Information}
*/
QSize QImage::size() const
{
    return d ? QSize(d->width, d->height) : QSize();
}

/*!
    \fn QRect QImage::rect() const

    Returns the enclosing rectangle (0, 0, width(), height()) of the
    image.

    \sa {QImage#Image Information}{Image Information}
*/
QRect QImage::rect() const
{
    return d ? QRect(0, 0, d->width, d->height) : QRect();
}

/*!
    Returns the depth of the image.

    The image depth is the number of bits used to encode a single
    pixel, also called bits per pixel (bpp) or bit planes of an image.

    The supported depths are 1, 8 and 32.

    \sa convertToFormat(), {QImage#Image Formats}{Image Formats},
    {QImage#Image Information}{Image Information}

*/
int QImage::depth() const
{
    return d ? d->depth : 0;
}

/*!
    \fn int QImage::numColors() const

    Returns the size of the color table for the image.

    Notice that numColors() returns 0 for 32-bpp images because these
    images do not use color tables, but instead encode pixel values as
    ARGB quadruplets.

    \sa setNumColors(), {QImage#Image Information}{Image Information}
*/
int QImage::numColors() const
{
    return d ? d->colortable.size() : 0;
}


#ifdef QT3_SUPPORT
/*!
    \fn QImage::Endian QImage::bitOrder() const

    Returns the bit order for the image. If it is a 1-bpp image, this
    function returns either QImage::BigEndian or
    QImage::LittleEndian. Otherwise, this function returns
    QImage::IgnoreEndian.

    Use the format() function instead for the monochrome formats. For
    non-monochrome formats the bit order is irrelevant.
*/

/*!
    Returns a pointer to the scanline pointer table. This is the
    beginning of the data block for the image.

    Use the bits() or scanLine() function instead.
*/
uchar **QImage::jumpTable()
{
    if (!d)
        return 0;
    detach();

    if (!d->jumptable) {
        d->jumptable = (uchar **)malloc(d->height*sizeof(uchar *));
        uchar *data = d->data;
        int height = d->height;
        uchar **p = d->jumptable;
        while (height--) {
            *p++ = data;
            data += d->bytes_per_line;
        }
    }
    return d->jumptable;
}

/*!
    \overload
*/
const uchar * const *QImage::jumpTable() const
{
    if (!d)
        return 0;
    if (!d->jumptable) {
        d->jumptable = (uchar **)malloc(d->height*sizeof(uchar *));
        uchar *data = d->data;
        int height = d->height;
        uchar **p = d->jumptable;
        while (height--) {
            *p++ = data;
            data += d->bytes_per_line;
        }
    }
    return d->jumptable;
}
#endif

/*!
    Sets the color table used to translate color indexes to QRgb
    values, to the specified \a colors.

    \sa colorTable(), setColor(), {QImage#Image Transformations}{Image
    Transformations}
*/
void QImage::setColorTable(const QVector<QRgb> colors)
{
    if (!d)
        return;
    detach();
    d->colortable = colors;
    d->has_alpha_clut = false;
    for (int i = 0; i < d->colortable.size(); ++i)
        d->has_alpha_clut |= (qAlpha(d->colortable.at(i)) != 255);
}

/*!
    Returns a list of the colors contained in the image's color table,
    or an empty list if the image does not have a color table

    \sa setColorTable(), numColors(), color()
*/
QVector<QRgb> QImage::colorTable() const
{
    return d ? d->colortable : QVector<QRgb>();
}


/*!
    Returns the number of bytes occupied by the image data.

    \sa bytesPerLine(), bits(), {QImage#Image Information}{Image
    Information}
*/
int QImage::numBytes() const
{
    return d ? d->nbytes : 0;
}

/*!
    Returns the number of bytes per image scanline.

    This is equivalent to numBytes()/ height().

    \sa scanLine()
*/
int QImage::bytesPerLine() const
{
    return (d && d->height) ? d->nbytes / d->height : 0;
}


/*!
    Returns the color in the color table at index \a i. The first
    color is at index 0.

    The colors in an image's color table are specified as ARGB
    quadruplets (QRgb). Use the qAlpha(), qRed(), qGreen(), and
    qBlue() functions to get the color value components.

    \sa setColor(), pixelIndex(), {QImage#Pixel Manipulation}{Pixel
    Manipulation}
*/
QRgb QImage::color(int i) const
{
    Q_ASSERT(i < numColors());
    return d ? d->colortable.at(i) : QRgb(uint(-1));
}

/*!
    \fn void QImage::setColor(int index, QRgb colorValue)

    Sets the color at the given \a index in the color table, to the
    given to \a colorValue.

    The color value is an ARGB quadruplet.

    \sa color(), setColorTable(), {QImage#Pixel Manipulation}{Pixel
    Manipulation}
*/
void QImage::setColor(int i, QRgb c)
{
    detach();
    Q_ASSERT(i < numColors());
    d->colortable[i] = c;
    d->has_alpha_clut |= (qAlpha(c) != 255);
}

/*!
    Returns a pointer to the pixel data at the scanline with index \a
    i. The first scanline is at index 0.

    The scanline data is aligned on a 32-bit boundary.

    \warning If you are accessing 32-bpp image data, cast the returned
    pointer to \c{QRgb*} (QRgb has a 32-bit size) and use it to
    read/write the pixel value. You cannot use the \c{uchar*} pointer
    directly, because the pixel format depends on the byte order on
    the underlying platform. Use qRed(), qGreen(), qBlue(), and
    qAlpha() to access the pixels.

    \sa bytesPerLine(), bits(), {QImage#Pixel Manipulation}{Pixel
    Manipulation}
*/
uchar *QImage::scanLine(int i)
{
    detach();
    Q_ASSERT(i >= 0 && i < height());
    return d->data + i * d->bytes_per_line;
}

/*!
    \overload
*/
const uchar *QImage::scanLine(int i) const
{
    Q_ASSERT(i >= 0 && i < height());
    return d->data + i * d->bytes_per_line;
}


/*!
    Returns a pointer to the first pixel data. This is equivalent to
    scanLine(0).

    \sa scanLine(), numBytes()
*/
uchar *QImage::bits()
{
    if (!d)
        return 0;
    detach();
    return d->data;
}

/*!
    \overload
*/
const uchar *QImage::bits() const
{
    return d ? d->data : 0;
}



/*!
    \fn void QImage::reset()

    Resets all image parameters and deallocates the image data.

    Assign a null image instead.

    \oldcode
        QImage image;
        image.reset();
    \newcode
        QImage image;
        image = QImage();
    \endcode
*/

/*!
    \fn void QImage::fill(uint pixelValue)

    Fills the entire image with the given \a pixelValue.

    If the depth of this image is 1, only the lowest bit is used. If
    you say fill(0), fill(2), etc., the image is filled with 0s. If
    you say fill(1), fill(3), etc., the image is filled with 1s. If
    the depth is 8, the lowest 8 bits are used and if the depth is 16
    the lowest 16 bits are used.

    Note: QImage::pixel() returns the color of the pixel at the given
    coordinates while QColor::pixel() returns the pixel value of the
    underlying window system (essentially an index value), so normally
    you will want to use QImage::pixel() to use a color from an
    existing image or QColor::rgb() to use a specific color.

    \sa depth(), {QImage#Image Transformations}{Image Transformations}
*/

void QImage::fill(uint pixel)
{
    if (!d)
        return;

    detach();
    if (d->depth == 1 || d->depth == 8) {
        if (d->depth == 1) {
            if (pixel & 1)
                pixel = 0xffffffff;
            else
                pixel = 0;
        } else {
            pixel &= 0xff;
        }
        memset(d->data, pixel, d->nbytes);
        return;
    } else if (d->depth == 16) {
        pixel = (pixel << 16) | (pixel & 0xffff);
    }

    if (d->format == Format_RGB32)
        pixel |= 0xff000000;
    if (pixel == 0 || pixel == 0xffffffff) {
        memset(d->data, (pixel & 0xff), d->nbytes);
    } else {
        uint *data = (uint *)d->data;
        uint *end = (uint *)(d->data + d->nbytes);
        while (data < end)
            *data++ = pixel;
    }
}

/*!
    Inverts all pixel values in the image.

    The given invert \a mode only have a meaning when the image's
    depth is 32. The default \a mode is InvertRgb, which leaves the
    alpha channel unchanged. If the \a mode is InvertRgba, the alpha
    bits are also inverted.

    Inverting an 8-bit image means to replace all pixels using color
    index \e i with a pixel using color index 255 minus \e i. The same
    is the case for a 1-bit image. Note that the color table is \e not
    changed.

    \sa {QImage#Image Transformations}{Image Transformations}
*/

void QImage::invertPixels(InvertMode mode)
{
    if (!d)
        return;

    detach();
    if (depth() != 32) {
        // number of used bytes pr line
        int bpl = (d->width * d->depth + 7) / 8;
        int pad = d->bytes_per_line - bpl;
        uchar *sl = d->data;
        for (int y=0; y<d->height; ++y) {
            for (int x=0; x<bpl; ++x)
                *sl++ ^= 0xff;
            sl += pad;
        }
    } else {
        quint32 *p = (quint32*)d->data;
        quint32 *end = (quint32*)(d->data + d->nbytes);
        uint xorbits = (mode == InvertRgba) ? 0xffffffff : 0x00ffffff;
        while (p < end)
            *p++ ^= xorbits;
    }
}

/*!
    \fn void QImage::invertPixels(bool invertAlpha)

    Use the invertPixels() function that takes a QImage::InvertMode
    parameter instead.
*/

/*! \fn QImage::Endian QImage::systemByteOrder()

    Determines the host computer byte order. Returns
    QImage::LittleEndian (LSB first) or QImage::BigEndian (MSB first).

    This function is no longer relevant for QImage. Use QSysInfo
    instead.
*/

// Windows defines these
#if defined(write)
# undef write
#endif
#if defined(close)
# undef close
#endif
#if defined(read)
# undef read
#endif

/*!
    Resizes the color table to contain \a numColors entries.

    If the color table is expanded, all the extra colors will be set to
    transparent (i.e qRgba(0, 0, 0, 0)).

    \sa numColors(), colorTable(), {QImage#Image
    Transformations}{Image Transformations}
*/

void QImage::setNumColors(int numColors)
{
    if (!d)
        return;

    detach();
    if (numColors == d->colortable.size())
        return;
    if (numColors <= 0) {                        // use no color table
        d->colortable = QVector<QRgb>();
        return;
    }
    int nc = d->colortable.size();
    d->colortable.resize(numColors);
    for (int i = nc; i < numColors; ++i)
        d->colortable[i] = 0;
}

/*!
    Returns the format of the image.

    \sa {QImage#Image Formats}{Image Formats}
*/
QImage::Format QImage::format() const
{
    return d ? d->format : Format_Invalid;
}


#ifdef QT3_SUPPORT
/*!
    Returns true if alpha buffer mode is enabled; otherwise returns
    false.

    Use the hasAlphaChannel() function instead.

*/
bool QImage::hasAlphaBuffer() const
{
    return d && (d->format != Format_RGB32)
#ifdef Q_WS_QWS
        && (d->format != Format_RGB16)
#endif
        ;
}

/*!
    Enables alpha buffer mode if \a enable is true, otherwise disables
    it. The alpha buffer is used to set a mask when a QImage is
    translated to a QPixmap.

    If a monochrome or indexed 8-bit image has alpha channels in their
    color tables they will automatically detect that they have an
    alpha channel, so this function is not required.  To force alpha
    channels on 32-bit images, use the convertToFormat() function.
*/

void QImage::setAlphaBuffer(bool enable)
{
    if (!d
        || d->format == Format_Mono
        || d->format == Format_MonoLSB
        || d->format == Format_Indexed8)
        return;
    if (enable && (d->format == Format_ARGB32 || d->format == Format_ARGB32_Premultiplied))
        return;
    if (!enable && d->format == Format_RGB32)
        return;
    detach();
    d->format = (enable ? Format_ARGB32 : Format_RGB32);
}


/*!
  \fn bool QImage::create(int width, int height, int depth, int numColors, Endian bitOrder)

    Sets the image \a width, \a height, \a depth, its number of colors
    (in \a numColors), and bit order. Returns true if successful, or
    false if the parameters are incorrect or if memory cannot be
    allocated.

    The \a width and \a height is limited to 32767. \a depth must be
    1, 8, or 32. If \a depth is 1, \a bitOrder must be set to
    either QImage::LittleEndian or QImage::BigEndian. For other depths
    \a bitOrder must be QImage::IgnoreEndian.

    This function allocates a color table and a buffer for the image
    data. The image data is not initialized. The image buffer is
    allocated as a single block that consists of a table of scanLine()
    pointers (jumpTable()) and the image data (bits()).

    Use a QImage constructor instead.
*/
bool QImage::create(int width, int height, int depth, int numColors, Endian bitOrder)
{
    if (d && !d->ref.deref())
        delete d;
    d = QImageData::create(QSize(width, height), formatFor(depth, bitOrder), numColors);
    return true;
}

/*!
    \fn bool QImage::create(const QSize& size, int depth, int numColors, Endian bitOrder)
    \overload

    The width and height are specified in the \a size argument.

    Use a QImage constructor instead.
*/
bool QImage::create(const QSize& size, int depth, int numColors, QImage::Endian bitOrder)
{
    if (d && !d->ref.deref())
        delete d;
    d = QImageData::create(size, formatFor(depth, bitOrder), numColors);
    return true;
}
#endif // QT3_SUPPORT

/*****************************************************************************
  Internal routines for converting image depth.
 *****************************************************************************/

typedef void (*Image_Converter)(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags);

static void convert_ARGB_to_ARGB_PM(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags)
{
    Q_ASSERT(src->format == QImage::Format_ARGB32);
    Q_ASSERT(dest->format == QImage::Format_ARGB32_Premultiplied);
    Q_ASSERT(src->width == dest->width);
    Q_ASSERT(src->height == dest->height);
    Q_ASSERT(src->nbytes == dest->nbytes);
    Q_ASSERT(src->bytes_per_line == dest->bytes_per_line);

    const QRgb *src_data = (QRgb *) src->data;
    const QRgb *end = src_data + (src->nbytes>>2);
    QRgb *dest_data = (QRgb *) dest->data;
    while (src_data < end) {
        *dest_data = PREMUL(*src_data);
        ++src_data;
        ++dest_data;
    }
}

static void convert_ARGB_PM_to_ARGB(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags)
{
    Q_ASSERT(src->format == QImage::Format_ARGB32_Premultiplied);
    Q_ASSERT(dest->format == QImage::Format_ARGB32);
    Q_ASSERT(src->width == dest->width);
    Q_ASSERT(src->height == dest->height);
    Q_ASSERT(src->nbytes == dest->nbytes);
    Q_ASSERT(src->bytes_per_line == dest->bytes_per_line);

    const QRgb *src_data = (QRgb *) src->data;
    const QRgb *end = src_data + (src->nbytes>>2);
    QRgb *dest_data = (QRgb *) dest->data;
    while (src_data < end) {
        *dest_data = INV_PREMUL(*src_data);
        ++src_data;
        ++dest_data;
    }
}

static void convert_ARGB_PM_to_RGB(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags)
{
    Q_ASSERT(src->format == QImage::Format_ARGB32_Premultiplied);
    Q_ASSERT(dest->format == QImage::Format_RGB32);
    Q_ASSERT(src->width == dest->width);
    Q_ASSERT(src->height == dest->height);
    Q_ASSERT(src->nbytes == dest->nbytes);
    Q_ASSERT(src->bytes_per_line == dest->bytes_per_line);

    const QRgb *src_data = (QRgb *) src->data;
    const QRgb *end = src_data + (src->nbytes>>2);
    QRgb *dest_data = (QRgb *) dest->data;
    while (src_data < end) {
        *dest_data = 0xff000000 | INV_PREMUL(*src_data);
        ++src_data;
        ++dest_data;
    }
}

static void swap_bit_order(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags)
{
    Q_ASSERT(src->format == QImage::Format_Mono || src->format == QImage::Format_MonoLSB);
    Q_ASSERT(dest->format == QImage::Format_Mono || dest->format == QImage::Format_MonoLSB);
    Q_ASSERT(src->width == dest->width);
    Q_ASSERT(src->height == dest->height);
    Q_ASSERT(src->nbytes == dest->nbytes);
    Q_ASSERT(src->bytes_per_line == dest->bytes_per_line);

    dest->colortable = src->colortable;

    const uchar *src_data = src->data;
    const uchar *end = src->data + src->nbytes;
    uchar *dest_data = dest->data;
    while (src_data < end) {
        *dest_data = bitflip[*src_data];
        ++src_data;
        ++dest_data;
    }
}

static void mask_alpha_converter(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags)
{
    Q_ASSERT(src->width == dest->width);
    Q_ASSERT(src->height == dest->height);
    Q_ASSERT(src->nbytes == dest->nbytes);

    const uint *src_data = (const uint *)src->data;
    const uint *end = (const uint *)(src->data + src->nbytes);
    uint *dest_data = (uint *)dest->data;
    while (src_data < end) {
        *dest_data = *src_data | 0xff000000;
        ++src_data;
        ++dest_data;
    }
}

static QVector<QRgb> fix_color_table(const QVector<QRgb> &ctbl, QImage::Format format)
{
    QVector<QRgb> colorTable = ctbl;
    if (format == QImage::Format_RGB32) {
        // check if the color table has alpha
        for (int i = 0; i < colorTable.size(); ++i)
            if (qAlpha(colorTable.at(i) != 0xff))
                colorTable[i] = colorTable.at(i) | 0xff000000;
    } else if (format == QImage::Format_ARGB32_Premultiplied) {
        // check if the color table has alpha
        for (int i = 0; i < colorTable.size(); ++i)
            colorTable[i] = PREMUL(colorTable.at(i));
    }
    return colorTable;
}

//
// dither_to_1:  Uses selected dithering algorithm.
//

static void dither_to_Mono(QImageData *dst, const QImageData *src,
                           Qt::ImageConversionFlags flags, bool fromalpha)
{
    Q_ASSERT(src->width == dst->width);
    Q_ASSERT(src->height == dst->height);
    Q_ASSERT(dst->format == QImage::Format_Mono || dst->format == QImage::Format_MonoLSB);

    dst->colortable.clear();
    dst->colortable.append(0xffffffff);
    dst->colortable.append(0xff000000);

    enum { Threshold, Ordered, Diffuse } dithermode;

    if (fromalpha) {
        if ((flags & Qt::AlphaDither_Mask) == Qt::DiffuseAlphaDither)
            dithermode = Diffuse;
        else if ((flags & Qt::AlphaDither_Mask) == Qt::OrderedAlphaDither)
            dithermode = Ordered;
        else
            dithermode = Threshold;
    } else {
        if ((flags & Qt::Dither_Mask) == Qt::ThresholdDither)
            dithermode = Threshold;
        else if ((flags & Qt::Dither_Mask) == Qt::OrderedDither)
            dithermode = Ordered;
        else
            dithermode = Diffuse;
    }

    int          w = src->width;
    int          h = src->height;
    int          d = src->depth;
    uchar gray[256];                                // gray map for 8 bit images
    bool  use_gray = (d == 8);
    if (use_gray) {                                // make gray map
        if (fromalpha) {
            // Alpha 0x00 -> 0 pixels (white)
            // Alpha 0xFF -> 1 pixels (black)
            for (int i = 0; i < src->colortable.size(); i++)
                gray[i] = (255 - (src->colortable.at(i) >> 24));
        } else {
            // Pixel 0x00 -> 1 pixels (black)
            // Pixel 0xFF -> 0 pixels (white)
            for (int i = 0; i < src->colortable.size(); i++)
                gray[i] = qGray(src->colortable.at(i));
        }
    }

    uchar *dst_data = dst->data;
    int dst_bpl = dst->bytes_per_line;
    const uchar *src_data = src->data;
    int src_bpl = src->bytes_per_line;

    switch (dithermode) {
    case Diffuse: {
        int *line1 = new int[w];
        int *line2 = new int[w];
        int bmwidth = (w+7)/8;

        int *b1, *b2;
        int wbytes = w * (d/8);
        register const uchar *p = src->data;
        const uchar *end = p + wbytes;
        b2 = line2;
        if (use_gray) {                        // 8 bit image
            while (p < end)
                *b2++ = gray[*p++];
        } else {                                // 32 bit image
            if (fromalpha) {
                while (p < end) {
                    *b2++ = 255 - (*(uint*)p >> 24);
                    p += 4;
                }
            } else {
                while (p < end) {
                    *b2++ = qGray(*(uint*)p);
                    p += 4;
                }
            }
        }
        for (int y=0; y<h; y++) {                        // for each scan line...
            int *tmp = line1; line1 = line2; line2 = tmp;
            bool not_last_line = y < h - 1;
            if (not_last_line) {                // calc. grayvals for next line
                p = src->data + (y+1)*src->bytes_per_line;
                end = p + wbytes;
                b2 = line2;
                if (use_gray) {                // 8 bit image
                    while (p < end)
                        *b2++ = gray[*p++];
                } else {                        // 24 bit image
                    if (fromalpha) {
                        while (p < end) {
                            *b2++ = 255 - (*(uint*)p >> 24);
                            p += 4;
                        }
                    } else {
                        while (p < end) {
                            *b2++ = qGray(*(uint*)p);
                            p += 4;
                        }
                    }
                }
            }

            int err;
            uchar *p = dst->data + y*dst->bytes_per_line;
            memset(p, 0, bmwidth);
            b1 = line1;
            b2 = line2;
            int bit = 7;
            for (int x=1; x<=w; x++) {
                if (*b1 < 128) {                // black pixel
                    err = *b1++;
                    *p |= 1 << bit;
                } else {                        // white pixel
                    err = *b1++ - 255;
                }
                if (bit == 0) {
                    p++;
                    bit = 7;
                } else {
                    bit--;
                }
                if (x < w)
                    *b1 += (err*7)>>4;                // spread error to right pixel
                if (not_last_line) {
                    b2[0] += (err*5)>>4;        // pixel below
                    if (x > 1)
                        b2[-1] += (err*3)>>4;        // pixel below left
                    if (x < w)
                        b2[1] += err>>4;        // pixel below right
                }
                b2++;
            }
        }
        delete [] line1;
        delete [] line2;
    } break;
    case Ordered: {

        memset(dst->data, 0, dst->nbytes);
        if (d == 32) {
            for (int i=0; i<h; i++) {
                const uint *p = (const uint *)src_data;
                const uint *end = p + w;
                uchar *m = dst_data;
                int bit = 7;
                int j = 0;
                if (fromalpha) {
                    while (p < end) {
                        if ((*p++ >> 24) >= qt_bayer_matrix[j++&15][i&15])
                            *m |= 1 << bit;
                        if (bit == 0) {
                            m++;
                            bit = 7;
                        } else {
                            bit--;
                        }
                    }
                } else {
                    while (p < end) {
                        if ((uint)qGray(*p++) < qt_bayer_matrix[j++&15][i&15])
                            *m |= 1 << bit;
                        if (bit == 0) {
                            m++;
                            bit = 7;
                        } else {
                            bit--;
                        }
                    }
                }
                dst_data += dst_bpl;
                src_data += src_bpl;
            }
        } else
            /* (d == 8) */ {
            for (int i=0; i<h; i++) {
                const uchar *p = src_data;
                const uchar *end = p + w;
                uchar *m = dst_data;
                int bit = 7;
                int j = 0;
                while (p < end) {
                    if ((uint)gray[*p++] < qt_bayer_matrix[j++&15][i&15])
                        *m |= 1 << bit;
                    if (bit == 0) {
                        m++;
                        bit = 7;
                    } else {
                        bit--;
                    }
                }
                dst_data += dst_bpl;
                src_data += src_bpl;
            }
        }
    } break;
    default: { // Threshold:
        memset(dst->data, 0, dst->nbytes);
        if (d == 32) {
            for (int i=0; i<h; i++) {
                const uint *p = (const uint *)src_data;
                const uint *end = p + w;
                uchar *m = dst_data;
                int bit = 7;
                if (fromalpha) {
                    while (p < end) {
                        if ((*p++ >> 24) >= 128)
                            *m |= 1 << bit;        // Set mask "on"
                        if (bit == 0) {
                            m++;
                            bit = 7;
                        } else {
                            bit--;
                        }
                    }
                } else {
                    while (p < end) {
                        if (qGray(*p++) < 128)
                            *m |= 1 << bit;        // Set pixel "black"
                        if (bit == 0) {
                            m++;
                            bit = 7;
                        } else {
                            bit--;
                        }
                    }
                }
                dst_data += dst_bpl;
                src_data += src_bpl;
            }
        } else
            if (d == 8) {
                for (int i=0; i<h; i++) {
                    const uchar *p = src_data;
                    const uchar *end = p + w;
                    uchar *m = dst_data;
                    int bit = 7;
                    while (p < end) {
                        if (gray[*p++] < 128)
                            *m |= 1 << bit;                // Set mask "on"/ pixel "black"
                        if (bit == 0) {
                            m++;
                            bit = 7;
                        } else {
                            bit--;
                        }
                    }
                    dst_data += dst_bpl;
                    src_data += src_bpl;
                }
            }
        }
    }

    if (dst->format == QImage::Format_MonoLSB) {
        // need to swap bit order
        uchar *sl = dst->data;
        int bpl = (dst->width + 7) * dst->depth / 8;
        int pad = dst->bytes_per_line - bpl;
        for (int y=0; y<dst->height; ++y) {
            for (int x=0; x<bpl; ++x) {
                *sl = bitflip[*sl];
                ++sl;
            }
            sl += pad;
        }
    }
}

static void convert_X_to_Mono(QImageData *dst, const QImageData *src, Qt::ImageConversionFlags flags)
{
    dither_to_Mono(dst, src, flags, false);
}

static void convert_ARGB_PM_to_Mono(QImageData *dst, const QImageData *src, Qt::ImageConversionFlags flags)
{
    QImageData *tmp = QImageData::create(QSize(src->width, src->height), QImage::Format_ARGB32);
    convert_ARGB_PM_to_ARGB(tmp, src, flags);
    dither_to_Mono(dst, tmp, flags, false);
    delete tmp;
}

//
// convert_32_to_8:  Converts a 32 bits depth (true color) to an 8 bit
// image with a colormap. If the 32 bit image has more than 256 colors,
// we convert the red,green and blue bytes into a single byte encoded
// as 6 shades of each of red, green and blue.
//
// if dithering is needed, only 1 color at most is available for alpha.
//
struct QRgbMap {
    inline QRgbMap() : rgb(0xffffffff) { }
    inline bool used() const { return rgb!=0xffffffff; }
    uchar  pix;
    QRgb  rgb;
};

static void convert_RGB_to_Indexed8(QImageData *dst, const QImageData *src, Qt::ImageConversionFlags flags)
{
    Q_ASSERT(src->format == QImage::Format_RGB32 || src->format == QImage::Format_ARGB32);
    Q_ASSERT(dst->format == QImage::Format_Indexed8);
    Q_ASSERT(src->width == dst->width);
    Q_ASSERT(src->height == dst->height);

    bool    do_quant = (flags & Qt::DitherMode_Mask) == Qt::PreferDither
                       || src->format == QImage::Format_ARGB32;
    uint alpha_mask = src->format == QImage::Format_RGB32 ? 0xff000000 : 0;

    const int tablesize = 997; // prime
    QRgbMap table[tablesize];
    int   pix=0;

    if (!dst->colortable.isEmpty()) {
        QVector<QRgb> ctbl = dst->colortable;
        dst->colortable.resize(256);
        // Preload palette into table.
        // Almost same code as pixel insertion below
        for (int i = 0; i < dst->colortable.size(); ++i) {
            // Find in table...
            QRgb p = ctbl.at(i) | alpha_mask;
            int hash = p % tablesize;
            for (;;) {
                if (table[hash].used()) {
                    if (table[hash].rgb == p) {
                        // Found previous insertion - use it
                        break;
                    } else {
                        // Keep searching...
                        if (++hash == tablesize) hash = 0;
                    }
                } else {
                    // Cannot be in table
                    Q_ASSERT (pix != 256);        // too many colors
                    // Insert into table at this unused position
                    dst->colortable[pix] = p;
                    table[hash].pix = pix++;
                    table[hash].rgb = p;
                    break;
                }
            }
        }
    }

    if ((flags & Qt::DitherMode_Mask) != Qt::PreferDither) {
        dst->colortable.resize(256);
        const uchar *src_data = src->data;
        uchar *dest_data = dst->data;
        for (int y = 0; y < src->height; y++) {        // check if <= 256 colors
            const QRgb *s = (const QRgb *)src_data;
            uchar *b = dest_data;
            for (int x = 0; x < src->width; ++x) {
                QRgb p = s[x] | alpha_mask;
                int hash = p % tablesize;
                for (;;) {
                    if (table[hash].used()) {
                        if (table[hash].rgb == (p)) {
                            // Found previous insertion - use it
                            break;
                        } else {
                            // Keep searching...
                            if (++hash == tablesize) hash = 0;
                        }
                    } else {
                        // Cannot be in table
                        if (pix == 256) {        // too many colors
                            do_quant = true;
                            // Break right out
                            x = src->width;
                            y = src->height;
                        } else {
                            // Insert into table at this unused position
                            dst->colortable[pix] = p;
                            table[hash].pix = pix++;
                            table[hash].rgb = p;
                        }
                        break;
                    }
                }
                *b++ = table[hash].pix;                // May occur once incorrectly
            }
            src_data += src->bytes_per_line;
            dest_data += dst->bytes_per_line;
        }
    }
    int numColors = do_quant ? 256 : pix;

    dst->colortable.resize(numColors);

    if (do_quant) {                                // quantization needed

#define MAX_R 5
#define MAX_G 5
#define MAX_B 5
#define INDEXOF(r,g,b) (((r)*(MAX_G+1)+(g))*(MAX_B+1)+(b))

        for (int rc=0; rc<=MAX_R; rc++)                // build 6x6x6 color cube
            for (int gc=0; gc<=MAX_G; gc++)
                for (int bc=0; bc<=MAX_B; bc++)
                    dst->colortable[INDEXOF(rc,gc,bc)] = 0xff000000 | qRgb(rc*255/MAX_R, gc*255/MAX_G, bc*255/MAX_B);

        const uchar *src_data = src->data;
        uchar *dest_data = dst->data;
        if ((flags & Qt::Dither_Mask) == Qt::ThresholdDither) {
            for (int y = 0; y < src->height; y++) {
                const QRgb *p = (const QRgb *)src_data;
                const QRgb *end = p + src->width;
                uchar *b = dest_data;

                while (p < end) {
#define DITHER(p,m) ((uchar) ((p * (m) + 127) / 255))
                    *b++ =
                        INDEXOF(
                            DITHER(qRed(*p), MAX_R),
                            DITHER(qGreen(*p), MAX_G),
                            DITHER(qBlue(*p), MAX_B)
                            );
#undef DITHER
                    p++;
                }
                src_data += src->bytes_per_line;
                dest_data += dst->bytes_per_line;
            }
        } else if ((flags & Qt::Dither_Mask) == Qt::DiffuseDither) {
            int* line1[3];
            int* line2[3];
            int* pv[3];
            line1[0] = new int[src->width];
            line2[0] = new int[src->width];
            line1[1] = new int[src->width];
            line2[1] = new int[src->width];
            line1[2] = new int[src->width];
            line2[2] = new int[src->width];
            pv[0] = new int[src->width];
            pv[1] = new int[src->width];
            pv[2] = new int[src->width];

            int endian = (QSysInfo::ByteOrder == QSysInfo::BigEndian);
            for (int y = 0; y < src->height; y++) {
                const uchar* q = src_data;
                const uchar* q2 = y < src->height - 1 ? q + src->bytes_per_line : src->data;
                uchar *b = dest_data;
                for (int chan = 0; chan < 3; chan++) {
                    int *l1 = (y&1) ? line2[chan] : line1[chan];
                    int *l2 = (y&1) ? line1[chan] : line2[chan];
                    if (y == 0) {
                        for (int i = 0; i < src->width; i++)
                            l1[i] = q[i*4+chan+endian];
                    }
                    if (y+1 < src->height) {
                        for (int i = 0; i < src->width; i++)
                            l2[i] = q2[i*4+chan+endian];
                    }
                    // Bi-directional error diffusion
                    if (y&1) {
                        for (int x = 0; x < src->width; x++) {
                            int pix = qMax(qMin(5, (l1[x] * 5 + 128)/ 255), 0);
                            int err = l1[x] - pix * 255 / 5;
                            pv[chan][x] = pix;

                            // Spread the error around...
                            if (x + 1< src->width) {
                                l1[x+1] += (err*7)>>4;
                                l2[x+1] += err>>4;
                            }
                            l2[x]+=(err*5)>>4;
                            if (x>1)
                                l2[x-1]+=(err*3)>>4;
                        }
                    } else {
                        for (int x = src->width; x-- > 0;) {
                            int pix = qMax(qMin(5, (l1[x] * 5 + 128)/ 255), 0);
                            int err = l1[x] - pix * 255 / 5;
                            pv[chan][x] = pix;

                            // Spread the error around...
                            if (x > 0) {
                                l1[x-1] += (err*7)>>4;
                                l2[x-1] += err>>4;
                            }
                            l2[x]+=(err*5)>>4;
                            if (x + 1 < src->width)
                                l2[x+1]+=(err*3)>>4;
                        }
                    }
                }
                if (endian) {
                    for (int x = 0; x < src->width; x++) {
                        *b++ = INDEXOF(pv[0][x],pv[1][x],pv[2][x]);
                    }
                } else {
                    for (int x = 0; x < src->width; x++) {
                        *b++ = INDEXOF(pv[2][x],pv[1][x],pv[0][x]);
                    }
                }
                src_data += src->bytes_per_line;
                dest_data += dst->bytes_per_line;
            }
            delete [] line1[0];
            delete [] line2[0];
            delete [] line1[1];
            delete [] line2[1];
            delete [] line1[2];
            delete [] line2[2];
            delete [] pv[0];
            delete [] pv[1];
            delete [] pv[2];
        } else { // OrderedDither
            for (int y = 0; y < src->height; y++) {
                const QRgb *p = (const QRgb *)src_data;
                const QRgb *end = p + src->width;
                uchar *b = dest_data;

                int x = 0;
                while (p < end) {
                    uint d = qt_bayer_matrix[y & 15][x & 15] << 8;

#define DITHER(p, d, m) ((uchar) ((((256 * (m) + (m) + 1)) * (p) + (d)) >> 16))
                    *b++ =
                        INDEXOF(
                            DITHER(qRed(*p), d, MAX_R),
                            DITHER(qGreen(*p), d, MAX_G),
                            DITHER(qBlue(*p), d, MAX_B)
                            );
#undef DITHER

                    p++;
                    x++;
                }
                src_data += src->bytes_per_line;
                dest_data += dst->bytes_per_line;
            }
        }

        if (src->format != QImage::Format_RGB32
#ifdef Q_WS_QWS
            && src->format != QImage::Format_RGB16
#endif
            ) {
            const int trans = 216;
            Q_ASSERT(dst->colortable.size() > trans);
            dst->colortable[trans] = 0;
            QImageData *mask = QImageData::create(QSize(src->width, src->height), QImage::Format_Mono);
            dither_to_Mono(mask, src, flags, true);
            uchar *dst_data = dst->data;
            const uchar *mask_data = mask->data;
            for (int y = 0; y < src->height; y++) {
                for (int x = 0; x < src->width ; x++) {
                    if (!(mask_data[x>>3] & (0x80 >> (x & 7))))
                        dst_data[x] = trans;
                }
                mask_data += mask->bytes_per_line;
                dst_data += dst->bytes_per_line;
            }
            dst->has_alpha_clut = true;
            delete mask;
        }

#undef MAX_R
#undef MAX_G
#undef MAX_B
#undef INDEXOF

    }
}

static void convert_ARGB_PM_to_Indexed8(QImageData *dst, const QImageData *src, Qt::ImageConversionFlags flags)
{
    QImageData *tmp = QImageData::create(QSize(src->width, src->height), QImage::Format_ARGB32);
    convert_ARGB_PM_to_ARGB(tmp, src, flags);
    convert_RGB_to_Indexed8(dst, tmp, flags);
    delete tmp;
}

static void convert_ARGB_to_Indexed8(QImageData *dst, const QImageData *src, Qt::ImageConversionFlags flags)
{
    convert_RGB_to_Indexed8(dst, src, flags);
}

static void convert_Indexed8_to_X32(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags)
{
    Q_ASSERT(src->format == QImage::Format_Indexed8);
    Q_ASSERT(dest->format == QImage::Format_RGB32
             || dest->format == QImage::Format_ARGB32
             || dest->format == QImage::Format_ARGB32_Premultiplied);
    Q_ASSERT(src->width == dest->width);
    Q_ASSERT(src->height == dest->height);

    QVector<QRgb> colorTable = fix_color_table(src->colortable, dest->format);

    int w = src->width;
    const uchar *src_data = src->data;
    uchar *dest_data = dest->data;
    for (int y = 0; y < src->height; y++) {
        uint *p = (uint *)dest_data;
        const uchar *b = src_data;
        uint *end = p + w;

        while (p < end)
            *p++ = colorTable.at(*b++);

        src_data += src->bytes_per_line;
        dest_data += dest->bytes_per_line;
    }
}

static void convert_Mono_to_X32(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags)
{
    Q_ASSERT(src->format == QImage::Format_Mono || src->format == QImage::Format_MonoLSB);
    Q_ASSERT(dest->format == QImage::Format_RGB32
             || dest->format == QImage::Format_ARGB32
             || dest->format == QImage::Format_ARGB32_Premultiplied);
    Q_ASSERT(src->width == dest->width);
    Q_ASSERT(src->height == dest->height);

    QVector<QRgb> colorTable = fix_color_table(src->colortable, dest->format);

    // Default to black / white colors
    if (colorTable.size() < 2) {
        if (colorTable.size() == 0)
            colorTable << 0xff000000;
        colorTable << 0xffffffff;
    }

    const uchar *src_data = src->data;
    uchar *dest_data = dest->data;
    if (src->format == QImage::Format_Mono) {
        for (int y = 0; y < dest->height; y++) {
            register uint *p = (uint *)dest_data;
            for (int x = 0; x < dest->width; x++)
                *p++ = colorTable.at((src_data[x>>3] >> (7 - (x & 7))) & 1);

            src_data += src->bytes_per_line;
            dest_data += dest->bytes_per_line;
        }
    } else {
        for (int y = 0; y < dest->height; y++) {
            register uint *p = (uint *)dest_data;
            for (int x = 0; x < dest->width; x++)
                *p++ = colorTable.at((src_data[x>>3] >> (x & 7)) & 1);

            src_data += src->bytes_per_line;
            dest_data += dest->bytes_per_line;
        }
    }
}


static void convert_Mono_to_Indexed8(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags)
{
    Q_ASSERT(src->format == QImage::Format_Mono || src->format == QImage::Format_MonoLSB);
    Q_ASSERT(dest->format == QImage::Format_Indexed8);
    Q_ASSERT(src->width == dest->width);
    Q_ASSERT(src->height == dest->height);

    QVector<QRgb> ctbl = src->colortable;
    if (ctbl.size() > 2) {
        ctbl.resize(2);
    } else if (ctbl.size() < 2) {
        if (ctbl.size() == 0)
            ctbl << 0xff000000;
        ctbl << 0xffffffff;
    }
    dest->colortable = ctbl;


    const uchar *src_data = src->data;
    uchar *dest_data = dest->data;
    if (src->format == QImage::Format_Mono) {
        for (int y = 0; y < dest->height; y++) {
            register uchar *p = dest_data;
            for (int x = 0; x < dest->width; x++)
                *p++ = (src_data[x>>3] >> (7 - (x & 7))) & 1;
            src_data += src->bytes_per_line;
            dest_data += dest->bytes_per_line;
        }
    } else {
        for (int y = 0; y < dest->height; y++) {
            register uchar *p = dest_data;
            for (int x = 0; x < dest->width; x++)
                *p++ = (src_data[x>>3] >> (x & 7)) & 1;
            src_data += src->bytes_per_line;
            dest_data += dest->bytes_per_line;
        }
    }
}


#ifdef Q_WS_QWS
#ifdef QT_QWS_DEPTH_16

static void convert_16_to_32(QImageData *dest, const QImageData *src, Qt::ImageConversionFlags)
{
    Q_ASSERT(src->format == QImage::Format_RGB16);
    Q_ASSERT(dest->format == QImage::Format_RGB32
             || dest->format == QImage::Format_ARGB32_Premultiplied
             || dest->format == QImage::Format_ARGB32);
    Q_ASSERT(src->width == dest->width);
    Q_ASSERT(src->height == dest->height);

    int w = src->width;
    const uchar *src_data = src->data;
    uchar *dest_data = dest->data;
    for (int y = 0; y < src->height; y++) {
        uint *p = (uint *)dest_data;
        const ushort *b = (ushort*)src_data;
        uint *end = p + w;

        while (p < end)
            *p++ = qt_conv16ToRgb(*b++);

        src_data += src->bytes_per_line;
        dest_data += dest->bytes_per_line;
    }
}


static void convert_32_to_16(QImageData *dest,   const QImageData *src, Qt::ImageConversionFlags)
{
    Q_ASSERT(dest->format == QImage::Format_RGB16);
    Q_ASSERT(src->format == QImage::Format_RGB32
             || src->format == QImage::Format_ARGB32_Premultiplied
             || src->format == QImage::Format_ARGB32);
    Q_ASSERT(src->width == dest->width);
    Q_ASSERT(src->height == dest->height);

    int w = src->width;
    const uchar *src_data = src->data;
    uchar *dest_data = dest->data;
    for (int y = 0; y < src->height; y++) {
        ushort *p = (ushort *)dest_data;
        const uint *b = (const uint*)src_data;
        ushort *end = p + w;

        while (p < end)
            *p++ = qt_convRgbTo16(*b++);

        src_data += src->bytes_per_line;
        dest_data += dest->bytes_per_line;
    }
}
#else
#define convert_32_to_16 0
#define convert_16_to_32 0
#endif
#endif //Q_WS_QWS
/*
        Format_Invalid,
        Format_Mono,
        Format_MonoLSB,
        Format_Indexed8,
        Format_RGB32,
        Format_ARGB32,
        Format_ARGB32_Premultiplied,

        Format_RGB16, //only on QWS for this release
*/


// first index source, second dest
static const Image_Converter converter_map[QImage::NImageFormats][QImage::NImageFormats] =
{
    {
        0,
        0,
        0,
        0,
        0,
        0,
        0
#ifdef Q_WS_QWS
        , 0
#endif
    },
    {
        0,
        0,
        swap_bit_order,
        convert_Mono_to_Indexed8,
        convert_Mono_to_X32,
        convert_Mono_to_X32,
        convert_Mono_to_X32
#ifdef Q_WS_QWS
        , 0
#endif
    }, // Format_Mono

    {
        0,
        swap_bit_order,
        0,
        convert_Mono_to_Indexed8,
        convert_Mono_to_X32,
        convert_Mono_to_X32,
        convert_Mono_to_X32
#ifdef Q_WS_QWS
        , 0
#endif
    }, // Format_MonoLSB

    {
        0,
        convert_X_to_Mono,
        convert_X_to_Mono,
        0,
        convert_Indexed8_to_X32,
        convert_Indexed8_to_X32,
        convert_Indexed8_to_X32
#ifdef Q_WS_QWS
        , 0
#endif
    }, // Format_Indexed8

    {
        0,
        convert_X_to_Mono,
        convert_X_to_Mono,
        convert_RGB_to_Indexed8,
        0,
        mask_alpha_converter,
        mask_alpha_converter
#ifdef Q_WS_QWS
        , convert_32_to_16
#endif
    }, // Format_RGB32

    {
        0,
        convert_X_to_Mono,
        convert_X_to_Mono,
        convert_ARGB_to_Indexed8,
        mask_alpha_converter,
        0,
        convert_ARGB_to_ARGB_PM
#ifdef Q_WS_QWS
        , convert_32_to_16
#endif
    }, // Format_ARGB32

    {
        0,
        convert_ARGB_PM_to_Mono,
        convert_ARGB_PM_to_Mono,
        convert_ARGB_PM_to_Indexed8,
        convert_ARGB_PM_to_RGB,
        convert_ARGB_PM_to_ARGB,
        0
#ifdef Q_WS_QWS
        , 0
#endif
    }  // Format_ARGB32_Premultiplied
#ifdef Q_WS_QWS
    ,
    {
        0,
        0,
        0,
        0,
        convert_16_to_32,
        convert_16_to_32,
        convert_16_to_32,
        0
    } // Format_RGB16
#endif
};

/*!
    Returns a copy of the image in the given \a format.

    The specified image conversion \a flags control how the image data
    is handled during the conversion process.

    \sa {QImage#Image Format}{Image Format}
*/
QImage QImage::convertToFormat(Format format, Qt::ImageConversionFlags flags) const
{
    if (!d || d->format == format)
        return *this;

    const Image_Converter *converterPtr = &converter_map[d->format][format];
    Image_Converter converter = *converterPtr;
    if (converter) {
        QImage image(d->width, d->height, format);
        image.setDotsPerMeterY(dotsPerMeterY());
        image.setDotsPerMeterX(dotsPerMeterX());
        converter(image.d, d, flags);
        return image;
    }

#ifdef Q_WS_QWS
#ifdef QT_QWS_DEPTH_16
    if (format == Format_RGB16) {
        QImage tmp;
        if (d->format == Format_RGB32 || d->format == Format_ARGB32)
            tmp = *this;
        else
            tmp = convertToFormat(Format_RGB32, flags);
        QImage image(d->width, d->height, format);
        image.setDotsPerMeterY(dotsPerMeterY());
        image.setDotsPerMeterX(dotsPerMeterX());
        convert_32_to_16(image.d, tmp.d, flags);
        return image;
    } else if (d->format == Format_RGB16) {
        int targetDepth = depthForFormat(format);
        QImage image(d->width, d->height, targetDepth == 32 ? format : Format_RGB32);
        image.setDotsPerMeterY(dotsPerMeterY());
        image.setDotsPerMeterX(dotsPerMeterX());
        convert_16_to_32(image.d, d, flags);
        if (targetDepth == 32)
            return image;
        else
            return image.convertToFormat(format);
    }
#endif
#endif

    return QImage();
}

/*!
    \overload

    Returns a copy of the image converted to the given \a format,
    using the specified \a colorTable.
*/
QImage QImage::convertToFormat(Format format, const QVector<QRgb> &colorTable, Qt::ImageConversionFlags flags) const
{
    if (d->format == format)
        return *this;

    const Image_Converter *converterPtr = &converter_map[d->format][format];
    Image_Converter converter = *converterPtr;
    if (!converter)
        return QImage();

    QImage image(d->width, d->height, format);
    if (image.d->depth <= 8)
        image.d->colortable = colorTable;
    converter(image.d, d, flags);
    return image;
}

#ifdef QT3_SUPPORT
/*!
    Converts the depth (bpp) of the image to the given \a depth and
    returns the converted image. The original image is not changed.
    Returns this image if \a depth is equal to the image depth, or a
    null image if this image cannot be converted. The \a depth
    argument must be 1, 8 or 32. If the image needs to be modified to
    fit in a lower-resolution result (e.g. converting from 32-bit to
    8-bit), use the \a flags to specify how you'd prefer this to
    happen.

    Use the convertToFormat() function instead.
*/

QImage QImage::convertDepth(int depth, Qt::ImageConversionFlags flags) const
{
    if (!d || d->depth == depth)
        return *this;

    Format format = formatFor (depth, QImage::LittleEndian);
    return convertToFormat(format, flags);
}
#endif

/*!
    Returns true if (\a x, \a y) is valid coordinates within the
    image; otherwise returns false.
*/

bool QImage::valid(int x, int y) const
{
    return d
        && x >= 0 && x < d->width
        && y >= 0 && y < d->height;
}

/*!
    Returns the pixel index at the given coordinates.

    If (\a x, \a y) is not valid, or if the image is not a paletted
    image (depth() > 8), the results are undefined.

    \sa valid(), depth(), {QImage#Pixel Manipulation}{Pixel
    Manipulation}
*/

int QImage::pixelIndex(int x, int y) const
{
    if (!d || x < 0 || x >= d->width) {
        qWarning("QImage::pixel: x=%d out of range", x);
        return -12345;
    }
    const uchar * s = scanLine(y);
    switch(d->format) {
    case Format_Mono:
        return (*(s + (x >> 3)) >> (7- (x & 7))) & 1;
    case Format_MonoLSB:
        return (*(s + (x >> 3)) >> (x & 7)) & 1;
    case Format_Indexed8:
        return (int)s[x];
    default:
        qWarning("QImage::pixelIndex: Not applicable for %d-bpp images (no palette)", d->depth);
    }
    return 0;
}


/*!
    Returns the color of the pixel at the given  coordinates.

    If (\a x, \a y) is not valid, the results are undefined.

    \sa setPixel(), valid(), {QImage#Pixel Manipulation}{Pixel
    Manipulation}
*/

QRgb QImage::pixel(int x, int y) const
{
    if (!d || x < 0 || x >= d->width) {
        qWarning("QImage::pixel: x=%d out of range", x);
        return 12345;
    }
    const uchar * s = scanLine(y);
    switch(d->format) {
    case Format_Mono:
        return d->colortable.at((*(s + (x >> 3)) >> (7- (x & 7))) & 1);
    case Format_MonoLSB:
        return d->colortable.at((*(s + (x >> 3)) >> (x & 7)) & 1);
    case Format_Indexed8:
        return d->colortable.at((int)s[x]);
#ifdef Q_WS_QWS
    case Format_RGB16:
        return qt_conv16ToRgb(reinterpret_cast<const ushort*>(s)[x]);
#endif
    default:
        return ((QRgb*)s)[x];
    }
}


/*!
    Sets the pixel index or color at the coordinates (\a x, \a y) to
    \a index_or_rgb.

    If the image's format is either monochrome or 8-bit, the given \a
    index_or_rgb value must be an index in the image's color table,
    otherwise the parameter must be a QRgb value.

    If (\a x, \a y) is not valid coordinates in the image, or if \a
    index_or_rgb >= numColors() in the case of monochrome and 8-bit
    images, the result is undefined.

    \sa pixel(), {QImage#Pixel Manipulation}{Pixel Manipulation}
*/

void QImage::setPixel(int x, int y, uint index_or_rgb)
{
    if (!d || x < 0 || x >= width()) {
        qWarning("QImage::setPixel: x=%d out of range", x);
        return;
    }
    detach();
    uchar * s = scanLine(y);
    switch(d->format) {
    case Format_Mono:
    case Format_MonoLSB:
        if (index_or_rgb > 1) {
            qWarning("QImage::setPixel: index=%d out of range", index_or_rgb);
        } else if (format() == Format_MonoLSB) {
            if (index_or_rgb==0)
                *(s + (x >> 3)) &= ~(1 << (x & 7));
            else
                *(s + (x >> 3)) |= (1 << (x & 7));
        } else {
            if (index_or_rgb==0)
                *(s + (x >> 3)) &= ~(1 << (7-(x & 7)));
            else
                *(s + (x >> 3)) |= (1 << (7-(x & 7)));
        }
        break;
    case Format_Indexed8:
        if (index_or_rgb > (uint)d->colortable.size()) {
            qWarning("QImage::setPixel: index=%d out of range", index_or_rgb);
            return;
        }
        s[x] = index_or_rgb;
        break;
    case Format_RGB32:
    case Format_ARGB32:
    case Format_ARGB32_Premultiplied:
        ((uint *)s)[x] = index_or_rgb;
        break;
#ifdef Q_WS_QWS
    case Format_RGB16:
        ((ushort *)s)[x] = qt_convRgbTo16(index_or_rgb);
        break;
#endif
    case Format_Invalid:
    case NImageFormats:
        Q_ASSERT(false);
    }
}

#ifdef QT3_SUPPORT
/*!
    Converts the bit order of the image to the given \a bitOrder and
    returns the converted image. The original image is not changed.
    Returns this image if the given \a bitOrder is equal to the image
    current bit order, or a null image if this image cannot be
    converted.

    Use convertToFormat() instead.
*/

QImage QImage::convertBitOrder(Endian bitOrder) const
{
    if (!d || isNull() || d->depth != 1 || !(bitOrder == BigEndian || bitOrder == LittleEndian))
        return QImage();

    if ((d->format == Format_Mono && bitOrder == BigEndian)
        || (d->format == Format_MonoLSB && bitOrder == LittleEndian))
        return *this;

    QImage image(d->width, d->height, d->format == Format_Mono ? Format_MonoLSB : Format_Mono);

    const uchar *data = d->data;
    const uchar *end = data + d->nbytes;
    uchar *ndata = image.d->data;
    while (data < end)
        *ndata++ = bitflip[*data++];

    image.setDotsPerMeterX(dotsPerMeterX());
    image.setDotsPerMeterY(dotsPerMeterY());

    image.d->colortable = d->colortable;
    return image;
}
#endif
/*!
    Returns true if all the colors in the image are shades of gray
    (i.e. their red, green and blue components are equal); otherwise
    false.

    Note that this function is slow for images without color table.

    \sa isGrayscale()
*/
bool QImage::allGray() const
{
    if (!d)
        return true;

    if (d->depth == 32) {
        int p = width()*height();
        const QRgb* b = (const QRgb*)bits();
        while (p--)
            if (!qIsGray(*b++))
                return false;
#ifdef Q_WS_QWS
    } else if (d->depth == 16) {
        int p = width()*height();
        const ushort* b = (const ushort *)bits();
        while (p--)
            if (!qIsGray(qt_conv16ToRgb(*b++)))
                return false;
#endif
    } else {
        if (d->colortable.isEmpty())
            return true;
        for (int i = 0; i < numColors(); i++)
            if (!qIsGray(d->colortable.at(i)))
                return false;
    }
    return true;
}

/*!
    For 32-bit images, this function is equivalent to allGray().

    For 8-bpp images, this function returns true if color(i) is
    QRgb(i, i, i) for all indexes of the color table; otherwise
    returns false.

    \sa allGray(), {QImage#Image Formats}{Image Formats}
*/
bool QImage::isGrayscale() const
{
    if (!d)
        return false;

    switch (depth()) {
    case 32:
    case 16:
        return allGray();
    case 8: {
        for (int i = 0; i < numColors(); i++)
            if (d->colortable.at(i) != qRgb(i,i,i))
                return false;
        return true;
        }
    }
    return false;
}


/*!
    \fn QImage QImage::smoothScale(int width, int height, Qt::AspectRatioMode mode) const

    Use scaled() instead.

    \oldcode
        QImage image;
        image.smoothScale(width, height, mode);
    \newcode
        QImage image;
        image.scaled(width, height, mode, Qt::SmoothTransformation);
    \endcode
*/

/*!
    \fn QImage QImage::smoothScale(const QSize &size, Qt::AspectRatioMode mode) const
    \overload

    Use scaled() instead.

    \oldcode
        QImage image;
        image.smoothScale(size, mode);
    \newcode
        QImage image;
        image.scaled(size, mode, Qt::SmoothTransformation);
    \endcode
*/

/*!
    \fn QImage QImage::scaled(int width, int height, Qt::AspectRatioMode aspectRatioMode,
                             Qt::TransformationMode transformMode) const
    \overload

    Returns a copy of the image scaled to a rectangle with the given
    \a width and \a height according to the given \a aspectRatioMode
    and \a transformMode.

    If either the \a width or the \a height is zero or negative, this
    function returns a null image.
*/

/*!
    \fn QImage QImage::scaled(const QSize &size, Qt::AspectRatioMode aspectRatioMode,
                             Qt::TransformationMode transformMode) const

    Returns a copy of the image scaled to a rectangle defined by the
    given \a size according to the given \a aspectRatioMode and \a
    transformMode.

    \image qimage-scaling.png

    \list
    \i If \a aspectRatioMode is Qt::IgnoreAspectRatio, the image
       is scaled to \a size.
    \i If \a aspectRatioMode is Qt::KeepAspectRatio, the image is
       scaled to a rectangle as large as possible inside \a size, preserving the aspect ratio.
    \i If \a aspectRatioMode is Qt::KeepAspectRatioByExpanding,
       the image is scaled to a rectangle as small as possible
       outside \a size, preserving the aspect ratio.
    \endlist

    If the given \a size is empty, this function returns a null image.

    \sa isNull(), {QImage#Image Transformations}{Image
    Transformations}
*/
QImage QImage::scaled(const QSize& s, Qt::AspectRatioMode aspectMode, Qt::TransformationMode mode) const
{
    if (!d) {
        qWarning("QImage::scaled: Image is a null image");
        return QImage();
    }
    if (s.isEmpty())
        return QImage();

    QSize newSize = size();
    newSize.scale(s, aspectMode);
    if (newSize == size())
        return copy();

    QImage img;
    QMatrix wm;
    wm.scale((double)newSize.width() / width(), (double)newSize.height() / height());
    img = transformed(wm, mode);
    return img;
}

/*!
    \fn QImage QImage::scaledToWidth(int width, Qt::TransformationMode mode) const

    Returns a scaled copy of the image. The returned image is scaled
    to the given \a width using the specified transformation \a
    mode.

    This function automatically calculates the height of the image so
    that its aspect ratio is preserved.

    If the given \a width is 0 or negative, a null image is returned.

    \sa {QImage#Image Transformations}{Image Transformations}
*/
QImage QImage::scaledToWidth(int w, Qt::TransformationMode mode) const
{
    if (!d) {
        qWarning("QImage::scaleWidth: Image is a null image");
        return QImage();
    }
    if (w <= 0)
        return QImage();

    QMatrix wm;
    double factor = (double) w / width();
    wm.scale(factor, factor);
    return transformed(wm, mode);
}

/*!
    \fn QImage QImage::scaledToHeight(int height, Qt::TransformationMode mode) const

    Returns a scaled copy of the image. The returned image is scaled
    to the given \a height using the specified transformation \a
    mode.

    This function automatically calculates the width of the image so that
    the ratio of the image is preserved.

    If the given \a height is 0 or negative, a null image is returned.

    \sa {QImage#Image Transformations}{Image Transformations}
*/
QImage QImage::scaledToHeight(int h, Qt::TransformationMode mode) const
{
    if (!d) {
        qWarning("QImage::scaleHeight: Image is a null image");
        return QImage();
    }
    if (h <= 0)
        return QImage();

    QMatrix wm;
    double factor = (double) h / height();
    wm.scale(factor, factor);
    return transformed(wm, mode);
}


/*!
    \fn QMatrix QImage::trueMatrix(const QMatrix &matrix, int width, int height)

    Returns the actual matrix used for transforming an image with the
    given \a width, \a height and \a matrix.

    When transforming an image using the transformed() function, the
    transformation matrix is internally adjusted to compensate for
    unwanted translation, i.e. transformed() returns the smallest
    image containing all transformed points of the original image.
    This function returns the modified matrix, which maps points
    correctly from the original image into the new image.

    \sa transformed(), {QImage#Image Transformations}{Image
    Transformations}
*/
QMatrix QImage::trueMatrix(const QMatrix &matrix, int w, int h)
{
    const qreal dt = qreal(0.);
    qreal x1,y1, x2,y2, x3,y3, x4,y4;                // get corners
    qreal xx = qreal(w);
    qreal yy = qreal(h);

    QMatrix mat(matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(), 0., 0.);

    mat.map(dt, dt, &x1, &y1);
    mat.map(xx, dt, &x2, &y2);
    mat.map(xx, yy, &x3, &y3);
    mat.map(dt, yy, &x4, &y4);

    qreal ymin = y1;                                // lowest y value
    if (y2 < ymin) ymin = y2;
    if (y3 < ymin) ymin = y3;
    if (y4 < ymin) ymin = y4;
    qreal xmin = x1;                                // lowest x value
    if (x2 < xmin) xmin = x2;
    if (x3 < xmin) xmin = x3;
    if (x4 < xmin) xmin = x4;

    qreal ymax = y1;                                // lowest y value
    if (y2 > ymax) ymax = y2;
    if (y3 > ymax) ymax = y3;
    if (y4 > ymax) ymax = y4;
    qreal xmax = x1;                                // lowest x value
    if (x2 > xmax) xmax = x2;
    if (x3 > xmax) xmax = x3;
    if (x4 > xmax) xmax = x4;

    mat.setMatrix(matrix.m11(), matrix.m12(), matrix.m21(), matrix.m22(), -xmin, -ymin);
    return mat;
}

/*!
    Returns a copy of the image that is transformed using the given
    transformation \a matrix and transformation \a mode.

    The transformation \a matrix is internally adjusted to compensate
    for unwanted translation; i.e. the image produced is the smallest
    image that contains all the transformed points of the original
    image. Use the trueMatrix() function to retrieve the actual matrix
    used for transforming an image

    \sa trueMatrix(), {QImage#Image Transformations}{Image
    Transformations}
*/
QImage QImage::transformed(const QMatrix &matrix, Qt::TransformationMode mode) const
{
    if (!d)
        return QImage();

    // source image data
    int ws = width();
    int hs = height();

    // target image data
    int wd;
    int hd;

    // compute size of target image
    QMatrix mat = trueMatrix(matrix, ws, hs);
    bool complex_xform = false;
    if (mat.m12() == 0.0F && mat.m21() == 0.0F) {
        if (mat.m11() == 1.0F && mat.m22() == 1.0F) // identity matrix
            return *this;
        hd = int(qAbs(mat.m22()) * hs + 0.9999);
        wd = int(qAbs(mat.m11()) * ws + 0.9999);
        hd = qAbs(hd);
        wd = qAbs(wd);
    } else if (d->format == Format_RGB32 && mat.m11() == 0. && mat.m22() == 0. &&
              ((mat.m12() == 1. && mat.m21() == -1.) ||     // 90 degrees
               (mat.m12() == -1. && mat.m21() == 1.))) {    // -90 degrees
        // Dont perform a complex_xform for trivial rotations
        wd = hs;
        hd = ws;
    } else {                                        // rotation or shearing
        QPolygonF a(QRectF(0, 0, ws, hs));
        a = mat.map(a);
        QRectF r = a.boundingRect();
        wd = int(qAbs(r.width()) + 0.9999);
        hd = int(qAbs(r.height()) + 0.9999);
        complex_xform = true;
    }

    if (wd == 0 || hd == 0)
        return QImage();

    int bpp = depth();

    int sbpl = bytesPerLine();
    const uchar *sptr = bits();

    QImage::Format target_format = d->format;

    if (complex_xform || mode == Qt::SmoothTransformation)
        target_format = Format_ARGB32_Premultiplied;

    QImage dImage(wd, hd, target_format);
    dImage.d->colortable = d->colortable;
    dImage.d->has_alpha_clut = d->has_alpha_clut | complex_xform;
    dImage.d->dpmx = dotsPerMeterX();
    dImage.d->dpmy = dotsPerMeterY();

    switch (bpp) {
        // initizialize the data
        case 1:
            memset(dImage.bits(), 0, dImage.numBytes());
            break;
        case 8:
            if (dImage.d->colortable.size() < 256) {
                // colors are left in the color table, so pick that one as transparent
                dImage.d->colortable.append(0x0);
                memset(dImage.bits(), dImage.d->colortable.size() - 1, dImage.numBytes());
            } else {
                memset(dImage.bits(), 0, dImage.numBytes());
            }
            break;
        case 32:
            memset(dImage.bits(), 0x00, dImage.numBytes());
            break;
    }

    if (target_format == QImage::Format_RGB32
        || target_format == QImage::Format_ARGB32_Premultiplied) {
        QPainter p(&dImage);
        if (mode == Qt::SmoothTransformation) {
            p.setRenderHint(QPainter::Antialiasing);
            p.setRenderHint(QPainter::SmoothPixmapTransform);
        }
        p.setMatrix(mat);
        p.drawImage(QPoint(0, 0), *this);
    } else {
        bool invertible;
        mat = mat.inverted(&invertible);                // invert matrix
        if (!invertible)        // error, return null image
            return QImage();

        // create target image (some of the code is from QImage::copy())
        int type = format() == Format_Mono ? QT_XFORM_TYPE_MSBFIRST : QT_XFORM_TYPE_LSBFIRST;
        int dbpl = dImage.bytesPerLine();
        qt_xForm_helper(mat, 0, type, bpp, dImage.bits(), dbpl, 0, hd, sptr, sbpl, ws, hs);
    }
    return dImage;
}

/*!
    Builds and returns a 1-bpp mask from the alpha buffer in this
    image. Returns a null image if the image's format is
    QImage::Format_RGB32.

    The \a flags argument is a bitwise-OR of the
    Qt::ImageConversionFlags, and controls the conversion
    process. Passing 0 for flags sets all the default options.

    The returned image has little-endian bit order (i.e. the image's
    format is QImage::Format_MonoLSB), which you can convert to
    big-endian (QImage::Format_Mono) using the convertToFormat()
    function.

    \sa createHeuristicMask(), {QImage#Image Transformations}{Image
    Transformations}
*/
QImage QImage::createAlphaMask(Qt::ImageConversionFlags flags) const
{
    if (!d || d->format == QImage::Format_RGB32)
        return QImage();

    if (d->depth == 1) {
        // A monochrome pixmap, with alpha channels on those two colors.
        // Pretty unlikely, so use less efficient solution.
        return convertToFormat(Format_Indexed8, flags).createAlphaMask(flags);
    }

    QImage mask(d->width, d->height, Format_MonoLSB);
    dither_to_Mono(mask.d, d, flags, true);
    return mask;
}

#ifndef QT_NO_IMAGE_HEURISTIC_MASK
/*!
    Creates and returns a 1-bpp heuristic mask for this image.

    The function works by selecting a color from one of the corners,
    then chipping away pixels of that color starting at all the edges.
    The four corners vote for which color is to be masked away. In
    case of a draw (this generally means that this function is not
    applicable to the image), the result is arbitrary.

    The returned image has little-endian bit order (i.e. the image's
    format is QImage::Format_MonoLSB), which you can convert to
    big-endian (QImage::Format_Mono) using the convertToFormat()
    function.

    If \a clipTight is true (the default) the mask is just large
    enough to cover the pixels; otherwise, the mask is larger than the
    data pixels.

    Note that this function disregards the alpha buffer.

    \sa createAlphaMask(), {QImage#Image Transformations}{Image
    Transformations}
*/

QImage QImage::createHeuristicMask(bool clipTight) const
{
    if (!d)
        return QImage();

    if (d->depth != 32) {
        QImage img32 = convertToFormat(Format_RGB32);
        return img32.createHeuristicMask(clipTight);
    }

#define PIX(x,y)  (*((QRgb*)scanLine(y)+x) & 0x00ffffff)

    int w = width();
    int h = height();
    QImage m(w, h, Format_MonoLSB);
    m.setNumColors(2);
    m.setColor(0, 0xffffff);
    m.setColor(1, 0);
    m.fill(0xff);

    QRgb background = PIX(0,0);
    if (background != PIX(w-1,0) &&
         background != PIX(0,h-1) &&
         background != PIX(w-1,h-1)) {
        background = PIX(w-1,0);
        if (background != PIX(w-1,h-1) &&
             background != PIX(0,h-1) &&
             PIX(0,h-1) == PIX(w-1,h-1)) {
            background = PIX(w-1,h-1);
        }
    }

    int x,y;
    bool done = false;
    uchar *ypp, *ypc, *ypn;
    while(!done) {
        done = true;
        ypn = m.scanLine(0);
        ypc = 0;
        for (y = 0; y < h; y++) {
            ypp = ypc;
            ypc = ypn;
            ypn = (y == h-1) ? 0 : m.scanLine(y+1);
            QRgb *p = (QRgb *)scanLine(y);
            for (x = 0; x < w; x++) {
                // slowness here - it's possible to do six of these tests
                // together in one go. oh well.
                if ((x == 0 || y == 0 || x == w-1 || y == h-1 ||
                       !(*(ypc + ((x-1) >> 3)) & (1 << ((x-1) & 7))) ||
                       !(*(ypc + ((x+1) >> 3)) & (1 << ((x+1) & 7))) ||
                       !(*(ypp + (x     >> 3)) & (1 << (x     & 7))) ||
                       !(*(ypn + (x     >> 3)) & (1 << (x     & 7)))) &&
                     (       (*(ypc + (x     >> 3)) & (1 << (x     & 7)))) &&
                     ((*p & 0x00ffffff) == background)) {
                    done = false;
                    *(ypc + (x >> 3)) &= ~(1 << (x & 7));
                }
                p++;
            }
        }
    }

    if (!clipTight) {
        ypn = m.scanLine(0);
        ypc = 0;
        for (y = 0; y < h; y++) {
            ypp = ypc;
            ypc = ypn;
            ypn = (y == h-1) ? 0 : m.scanLine(y+1);
            QRgb *p = (QRgb *)scanLine(y);
            for (x = 0; x < w; x++) {
                if ((*p & 0x00ffffff) != background) {
                    if (x > 0)
                        *(ypc + ((x-1) >> 3)) |= (1 << ((x-1) & 7));
                    if (x < w-1)
                        *(ypc + ((x+1) >> 3)) |= (1 << ((x+1) & 7));
                    if (y > 0)
                        *(ypp + (x >> 3)) |= (1 << (x & 7));
                    if (y < h-1)
                        *(ypn + (x >> 3)) |= (1 << (x & 7));
                }
                p++;
            }
        }
    }

#undef PIX

    return m;
}
#endif //QT_NO_IMAGE_HEURISTIC_MASK

/*
  This code is contributed by Philipp Lang,
  GeneriCom Software Germany (www.generi.com)
  under the terms of the QPL, Version 1.0
*/

/*!
    \fn QImage QImage::mirror(bool horizontal, bool vertical) const

    Use mirrored() instead.
*/

/*!
    Returns a mirror of the image, mirrored in the horizontal and/or
    the vertical direction depending on whether \a horizontal and \a
    vertical are set to true or false.

    Note that the original image is not changed.

    \sa {QImage#Image Transformations}{Image Transformations}
*/
QImage QImage::mirrored(bool horizontal, bool vertical) const
{
    if (!d)
        return QImage();

    if ((d->width <= 1 && d->height <= 1) || (!horizontal && !vertical))
        return *this;

    int w = d->width;
    int h = d->height;
    // Create result image, copy colormap
    QImage result(d->width, d->height, d->format);
    result.d->colortable = d->colortable;

    if (depth() == 1)
        w = (w+7)/8;
    int dxi = horizontal ? -1 : 1;
    int dxs = horizontal ? w-1 : 0;
    int dyi = vertical ? -1 : 1;
    int dy = vertical ? h-1: 0;

    // 1 bit, 8 bit
    if (d->depth == 1 || d->depth == 8) {
        for (int sy = 0; sy < h; sy++, dy += dyi) {
            quint8* ssl = (quint8*)(d->data + sy*d->bytes_per_line);
            quint8* dsl = (quint8*)(result.d->data + dy*result.d->bytes_per_line);
            int dx = dxs;
            for (int sx = 0; sx < w; sx++, dx += dxi)
                dsl[dx] = ssl[sx];
        }
    }
#ifdef Q_WS_QWS
    // 16 bit
    else if (d->depth == 16) {
        for (int sy = 0; sy < h; sy++, dy += dyi) {
            quint16* ssl = (quint16*)(d->data + sy*d->bytes_per_line);
            quint16* dsl = (quint16*)(result.d->data + dy*result.d->bytes_per_line);
            int dx = dxs;
            for (int sx = 0; sx < w; sx++, dx += dxi)
                dsl[dx] = ssl[sx];
        }
    }
#endif
    // 32 bit
    else if (d->depth == 32) {
        for (int sy = 0; sy < h; sy++, dy += dyi) {
            quint32* ssl = (quint32*)(d->data + sy*d->bytes_per_line);
            quint32* dsl = (quint32*)(result.d->data + dy*result.d->bytes_per_line);
            int dx = dxs;
            for (int sx = 0; sx < w; sx++, dx += dxi)
                dsl[dx] = ssl[sx];
        }
    }

    // special handling of 1 bit images for horizontal mirroring
    if (horizontal && d->depth == 1) {
        int shift = width() % 8;
        for (int y = h-1; y >= 0; y--) {
            quint8* a0 = (quint8*)(result.d->data + y*d->bytes_per_line);
            // Swap bytes
            quint8* a = a0+dxs;
            while (a >= a0) {
                *a = bitflip[*a];
                a--;
            }
            // Shift bits if unaligned
            if (shift != 0) {
                a = a0+dxs;
                quint8 c = 0;
                if (format() == Format_MonoLSB) {
                    while (a >= a0) {
                        quint8 nc = *a << shift;
                        *a = (*a >> (8-shift)) | c;
                        --a;
                        c = nc;
                    }
                } else {
                    while (a >= a0) {
                        quint8 nc = *a >> shift;
                        *a = (*a << (8-shift)) | c;
                        --a;
                        c = nc;
                    }
                }
            }
        }
    }

    return result;
}

/*!
    \fn QImage QImage::swapRGB() const

    Use rgbSwapped() instead.

    \omit
    Returns a QImage in which the values of the red and blue
    components of all pixels have been swapped, effectively converting
    an RGB image to an BGR image. The original QImage is not changed.
    \endomit
*/

/*!
    Returns a QImage in which the values of the red and blue
    components of all pixels have been swapped, effectively converting
    an RGB image to an BGR image.

    The original QImage is not changed.

    \sa {QImage#Image Transformations}{Image Transformations}
*/
QImage QImage::rgbSwapped() const
{
    if (isNull())
        return *this;
    QImage res;
    switch (d->format) {
    case Format_Invalid:
    case NImageFormats:
        Q_ASSERT(false);
        break;
    case Format_Mono:
    case Format_MonoLSB:
    case Format_Indexed8:
        res = copy();
        for (int i = 0; i < res.d->colortable.size(); i++) {
            QRgb c = res.d->colortable.at(i);
            res.d->colortable[i] = QRgb(((c << 16) & 0xff0000) | ((c >> 16) & 0xff) | (c & 0xff00ff00));
        }
        break;
    case Format_RGB32:
    case Format_ARGB32:
    case Format_ARGB32_Premultiplied:
        res = QImage(d->width, d->height, d->format);
        for (int i = 0; i < d->height; i++) {
            uint *q = (uint*)res.scanLine(i);
            uint *p = (uint*)scanLine(i);
            uint *end = p + d->width;
            while (p < end) {
                *q = ((*p << 16) & 0xff0000) | ((*p >> 16) & 0xff) | (*p & 0xff00ff00);
                p++;
                q++;
            }
        }
        break;
#ifdef Q_WS_QWS
    case Format_RGB16:
        res = QImage(d->width, d->height, d->format);
        for (int i = 0; i < d->height; i++) {
            ushort *q = (ushort*)res.scanLine(i);
            const ushort *p = (const ushort*)scanLine(i);
            const ushort *end = p + d->width;
            while (p < end) {
                *q = ((*p << 11) & 0xf800) | ((*p >> 11) & 0x1f) | (*p & 0x07e0);
                p++;
                q++;
            }
        }
        break;
#endif
    }
    return res;
}

/*!
    Loads an image from the file with the given \a fileName. Returns
    true if the image was successfully loaded; otherwise returns
    false.

    The loader attempts to read the image using the specified \a
    format. If the \a format is not specified (which is the default),
    the loader probes the file for a header to guess the file format.

    The file name can either refer to an actual file on disk or to one
    of the application's embedded resources. See the
    \l{resources.html}{Resource System} overview for details on how to
    embed images and other resource files in the application's
    executable.

    \sa {QImage#Reading and Writing Image Files}{Reading and Writing Image Files}
*/

bool QImage::load(const QString &fileName, const char* format)
{
    if (fileName.isEmpty())
        return false;

    QImage image = QImageReader(fileName, format).read();
    if (!image.isNull()) {
        operator=(image);
        return true;
    }
    return false;
}

/*!
    \overload

    This function reads a QImage from the given \a device. This can,
    for example, be used to load an image directly into a QByteArray.
*/

bool QImage::load(QIODevice* device, const char* format)
{
    QImage image = QImageReader(device, format).read();
    if(!image.isNull()) {
        operator=(image);
        return true;
    }
    return false;
}

/*!
    \fn bool QImage::loadFromData(const uchar *data, int len, const char *format)

    Loads an image from the first \a len bytes of the given binary \a
    data. Returns true if the image was successfully loaded; otherwise
    returns false.

    The loader attempts to read the image using the specified
    format. If the \a format is not specified (which is the default),
    the loader probes the file for a header to guess the file format.

    \sa {QImage#Reading and Writing Image Files}{Reading and Writing Image Files}
*/

bool QImage::loadFromData(const uchar *data, int len, const char *format)
{
    QImage image = fromData(data, len, format);
    if (!image.isNull()) {
        operator=(image);
        return true;
    }
    return false;
}

/*!
    \fn bool QImage::loadFromData(const QByteArray &data, const char *format)

    \overload

    Loads an image from the given QByteArray \a data.
*/

/*!
    \fn QImage QImage::fromData(const uchar *data, int size, const char *format)

    Constructs a QImage from the first \a size bytes of the given
    binary \a data. the loader attempts to read the image using the
    specified \a format. If the \a format is not specified (which is
    the default), the loader probes the file for a header to guess the
    file format.

    If the loading of the image failed, this object is a null image.

    \sa load(), save(), {QImage#Reading and Writing Image
    Files}{Reading and Writing Image Files}
*/
QImage QImage::fromData(const uchar *data, int size, const char *format)
{
    QByteArray a = QByteArray::fromRawData(reinterpret_cast<const char *>(data), size);
    QBuffer b;
    b.setData(a);
    b.open(QIODevice::ReadOnly);
    return QImageReader(&b, format).read();
}

/*!
    \fn QImage QImage::fromData(const QByteArray &data, const char *format)

    \overload

    Loads an image from the given QByteArray \a data.
*/

/*!
    Saves the image to the file with the given \a fileName, using the
    given image file \a format and \a quality factor.

    The \a quality factor must be in the range 0 to 100 or -1. Specify
    0 to obtain small compressed files, 100 for large uncompressed
    files, and -1 (the default) to use the default settings.

    Returns true if the image was successfully saved; otherwise
    returns false.

    \sa {QImage#Reading and Writing Image Files}{Reading and Writing
    Image Files}
*/
bool QImage::save(const QString &fileName, const char *format, int quality) const
{
    if (isNull())
        return false;
    QImageWriter writer(fileName, format);
    return d->doImageIO(this, &writer, quality);
}

/*!
    \overload

    This function writes a QImage to the given \a device.

    This can, for example, be used to save an image directly into a
    QByteArray:

    \quotefromfile snippets/image/image.cpp
    \skipto SAVE
    \skipto QImage
    \printuntil save
*/

bool QImage::save(QIODevice* device, const char* format, int quality) const
{
    if (isNull())
        return false;                                // nothing to save
    QImageWriter writer(device, format);
    return d->doImageIO(this, &writer, quality);
}

/* \internal
*/

bool QImageData::doImageIO(const QImage *image, QImageWriter *writer, int quality) const
{
    if (quality > 100  || quality < -1)
        qWarning("QPixmap::save: quality out of range [-1,100]");
    if (quality >= 0)
        writer->setQuality(qMin(quality,100));
    return writer->write(*image);
}

/*****************************************************************************
  QImage stream functions
 *****************************************************************************/
#if !defined(QT_NO_DATASTREAM)
/*!
    \fn QDataStream &operator<<(QDataStream &stream, const QImage &image)
    \relates QImage

    Writes the given \a image to the given \a stream as a PNG image,
    or as a BMP image if the stream's version is 1. Note that writing
    the stream to a file will not produce a valid image file.

    \sa QImage::save(), {Format of the QDataStream Operators}
*/

QDataStream &operator<<(QDataStream &s, const QImage &image)
{
    if (s.version() >= 5) {
        if (image.isNull()) {
            s << (qint32) 0; // null image marker
            return s;
        } else {
            s << (qint32) 1;
            // continue ...
        }
    }
    QImageWriter writer(s.device(), s.version() == 1 ? "bmp" : "png");
    writer.write(image);
    return s;
}

/*!
    \fn QDataStream &operator>>(QDataStream &stream, QImage &image)
    \relates QImage

    Reads an image from the given \a stream and stores it in the given
    \a image.

    \sa QImage::load(), {Format of the QDataStream Operators}
*/

QDataStream &operator>>(QDataStream &s, QImage &image)
{
    if (s.version() >= 5) {
        qint32 nullMarker;
        s >> nullMarker;
        if (!nullMarker) {
            image = QImage(); // null image
            return s;
        }
    }
    image = QImageReader(s.device(), 0).read();
    return s;
}
#endif


#ifdef QT3_SUPPORT
/*!
    \fn QImage QImage::convertDepthWithPalette(int depth, QRgb* palette, int palette_count, Qt::ImageConversionFlags flags) const

    Returns an image with the given \a depth, using the \a
    palette_count colors pointed to by \a palette. If \a depth is 1 or
    8, the returned image will have its color table ordered in the
    same way as \a palette.

    If the image needs to be modified to fit in a lower-resolution
    result (e.g. converting from 32-bit to 8-bit), use the \a flags to
    specify how you'd prefer this to happen.

    Note: currently no closest-color search is made. If colors are
    found that are not in the palette, the palette may not be used at
    all. This result should not be considered valid because it may
    change in future implementations.

    Currently inefficient for non-32-bit images.

    Use the convertToFormat() function in combination with the
    setColorTable() function instead.
*/
QImage QImage::convertDepthWithPalette(int d, QRgb* palette, int palette_count, Qt::ImageConversionFlags flags) const
{
    Format f = formatFor(d, QImage::LittleEndian);
    QVector<QRgb> colortable;
    for (int i = 0; i < palette_count; ++i)
        colortable.append(palette[i]);
    return convertToFormat(f, colortable, flags);
}

/*!
    \relates QImage

    Copies a block of pixels from \a src to \a dst. The pixels
    copied from source (src) are converted according to
    \a flags if it is incompatible with the destination
    (\a dst).

    \a sx, \a sy is the top-left pixel in \a src, \a dx, \a dy is the
    top-left position in \a dst and \a sw, \a sh is the size of the
    copied block. The copying is clipped if areas outside \a src or \a
    dst are specified. If \a sw is -1, it is adjusted to
    src->width(). Similarly, if \a sh is -1, it is adjusted to
    src->height().

    Currently inefficient for non 32-bit images.

    Use copy() or QPainter::drawImage() instead.
*/
void bitBlt(QImage *dst, int dx, int dy, const QImage *src, int sx, int sy, int sw, int sh,
            Qt::ImageConversionFlags flags)
{
    if (dst->isNull() || src->isNull())
        return;
    QPainter p(dst);
    p.drawImage(QPoint(dx, dy), *src, QRect(sx, sy, sw, sh), flags);
}
#endif

/*!
    \fn bool QImage::operator==(const QImage & image) const

    Returns true if this image and the given \a image have the same
    contents; otherwise returns false.

    The comparison can be slow, unless there is some obvious
    difference (e.g. different size or format), in which case the
    function will return quickly.

    \sa operator=()
*/

bool QImage::operator==(const QImage & i) const
{
    // same object, or shared?
    if (i.d == d)
        return true;
    if (!i.d || !d)
        return false;

    // obviously different stuff?
    if (i.d->height != d->height || i.d->width != d->width || i.d->format != d->format)
        return false;

    if (d->format != Format_RGB32) {
        if (d->colortable != i.d->colortable)
            return false;
        if (d->format == Format_ARGB32 || d->format == Format_ARGB32_Premultiplied
#ifdef Q_WS_QWS
            || d->format == Format_RGB16
#endif
            ) {
            if (memcmp(bits(), i.bits(), d->nbytes))
                return false;
        } else {
            int w = width();
            int h = height();
            for (int y=0; y<h; ++y) {
                for (int x=0; x<w; ++x) {
                    if (pixelIndex(x, y) != i.pixelIndex(x, y))
                        return false;
                }
            }
        }
    } else {
        //alpha channel undefined, so we must mask it out
        for(int l = 0; l < d->height; l++) {
            int w = d->width;
            const uint *p1 = reinterpret_cast<const uint*>(scanLine(l));
            const uint *p2 = reinterpret_cast<const uint*>(i.scanLine(l));
            while (w--) {
                if ((*p1++ & 0x00ffffff) != (*p2++ & 0x00ffffff))
                    return false;
            }
        }
    }
    return true;
}


/*!
    \fn bool QImage::operator!=(const QImage & image) const

    Returns true if this image and the given \a image have different
    contents; otherwise returns false.

    The comparison can be slow, unless there is some obvious
    difference, such as different widths, in which case the function
    will return quickly.

    \sa operator=()
*/

bool QImage::operator!=(const QImage & i) const
{
    return !(*this == i);
}




/*!
    Returns the number of pixels that fit horizontally in a physical
    meter. Together with dotsPerMeterY(), this number defines the
    intended scale and aspect ratio of the image.

    \sa setDotsPerMeterX(), {QImage#Image Information}{Image
    Information}
*/
int QImage::dotsPerMeterX() const
{
    return d ? qRound(d->dpmx) : 0;
}

/*!
    Returns the number of pixels that fit vertically in a physical
    meter. Together with dotsPerMeterX(), this number defines the
    intended scale and aspect ratio of the image.

    \sa setDotsPerMeterY(), {QImage#Image Information}{Image
    Information}
*/
int QImage::dotsPerMeterY() const
{
    return d ? qRound(d->dpmy) : 0;
}

/*!
    Sets the number of pixels that fit horizontally in a physical
    meter, to \a x.

    Together with dotsPerMeterY(), this number defines the intended
    scale and aspect ratio of the image.

    \sa dotsPerMeterX(), {QImage#Image Information}{Image
    Information}
*/
void QImage::setDotsPerMeterX(int x)
{
    if (!d || !x)
        return;
    detach();
    d->dpmx = x;
}

/*!
    Sets the number of pixels that fit vertically in a physical meter,
    to \a y.

    Together with dotsPerMeterX(), this number defines the intended
    scale and aspect ratio of the image.

    \sa dotsPerMeterY(), {QImage#Image Information}{Image
    Information}
*/
void QImage::setDotsPerMeterY(int y)
{
    if (!d || !y)
        return;
    detach();
    d->dpmy = y;
}

/*!
    \fn QPoint QImage::offset() const

    Returns the number of pixels by which the image is intended to be
    offset by when positioning relative to other images.

    \sa setOffset(), {QImage#Image Information}{Image Information}
*/
QPoint QImage::offset() const
{
    return d ? d->offset : QPoint();
}


/*!
    \fn void QImage::setOffset(const QPoint& offset)

    Sets the the number of pixels by which the image is intended to be
    offset by when positioning relative to other images, to \a offset.

    \sa offset(), {QImage#Image Information}{Image Information}
*/
void QImage::setOffset(const QPoint& p)
{
    if (!d)
        return;
    detach();
    d->offset = p;
}
#ifndef QT_NO_IMAGE_TEXT

/*!
    Returns the text keys for this image.

    You can use these keys with text() to list the image text for a
    certain key.

    \sa text()
*/
QStringList QImage::textKeys() const
{
    return d ? QStringList(d->text.keys()) : QStringList();
}

/*!
    Returns the image text associated with the given \a key. If the
    specified \a key is an empty string, the whole image text is
    returned, with each key-text pair separated by a newline.

    \sa setText(), textKeys()
*/
QString QImage::text(const QString &key) const
{
    if (!d)
        return QString();

    if (!key.isEmpty())
        return d->text.value(key);

    QString tmp;
    foreach (QString key, d->text.keys()) {
        if (!tmp.isEmpty())
            tmp += QLatin1String("\n\n");
        tmp += key + ": " + d->text.value(key).simplified();
    }
    return tmp;
}

/*!
    \fn void QImage::setText(const QString &key, const QString &text)

    Sets the image text to the given \a text and associate it with the
    given \a key.

    If you just want to store a single text block (i.e., a "comment"
    or just a description), you can either pass an empty key, or use a
    generic key like "Description".

    The image text is embedded into the image data when you
    call save() or QImageWriter::write().

    \sa text(), textKeys()
*/
void QImage::setText(const QString &key, const QString &value)
{
    if (d)
        d->text.insert(key, value);
}

/*!
    \fn QString QImage::text(const char* key, const char* language) const
    \obsolete

    Returns the text recorded for the given \a key in the given \a
    language, or in a default language if \a language is 0.

    Use text() instead.

    The language the text is recorded in is no longer relevant since
    the text is always set using QString and UTF-8 representation.
*/
QString QImage::text(const char* key, const char* lang) const
{
    return d ? d->text_lang.value(QImageTextKeyLang(key,lang)) : QString();
}

/*!
    \fn QString QImage::text(const QImageTextKeyLang& keywordAndLanguage) const
    \overload
    \obsolete

    Returns the text recorded for the given \a keywordAndLanguage.

    Use text() instead.

    The language the text is recorded in is no longer relevant since
    the text is always set using QString and UTF-8 representation.
*/
QString QImage::text(const QImageTextKeyLang& kl) const
{
    return d ? d->text_lang.value(kl) : QString();
}

/*!
    \obsolete

    Returns the language identifiers for which some texts are
    recorded. Note that if you want to iterate over the list, you
    should iterate over a copy.

    The language the text is recorded in is no longer relevant since
    the text is always set using QString and UTF-8 representation.
*/
QStringList QImage::textLanguages() const
{
    return d ? d->languages() : QStringList();
}

/*!
    \obsolete

    Returns a list of QImageTextKeyLang objects that enumerate all the
    texts key/language pairs set for this image.

    Use textKeys() instead.

    The language the text is recorded in is no longer relevant since
    the text is always set using QString and UTF-8 representation.
*/
QList<QImageTextKeyLang> QImage::textList() const
{
    return d ? d->text_lang.keys() : QList<QImageTextKeyLang>();
}

/*!
    \fn void QImage::setText(const char* key, const char* language, const QString& text)
    \obsolete

    Sets the image text to the given \a text and associate it with the
    given \a key. The text is recorded in the specified \a language,
    or in a default language if \a language is 0.

    Use setText() instead.

    The language the text is recorded in is no longer relevant since
    the text is always set using QString and UTF-8 representation.

    \omit
    Records string \a  for the keyword \a key. The \a key should be
    a portable keyword recognizable by other software - some suggested
    values can be found in
    \l{http://www.libpng.org/pub/png/spec/1.2/png-1.2-pdg.html#C.Anc-text}
    {the PNG specification}. \a s can be any text. \a lang should
    specify the language code (see
    \l{http://www.rfc-editor.org/rfc/rfc1766.txt}{RFC 1766}) or 0.
    \endomit
*/
void QImage::setText(const char* key, const char* lang, const QString& s)
{
    if (!d)
        return;
    detach();
    QImageTextKeyLang x(key, lang);
    d->text_lang.insert(x, s);
}

#endif // QT_NO_IMAGE_TEXT

/*
    Sets the image bits to the \a pixmap contents and returns a
    reference to the image.

    If the image shares data with other images, it will first
    dereference the shared data.

    Makes a call to QPixmap::convertToImage().
*/

/*! \fn QImage::Endian QImage::systemBitOrder()

    Determines the bit order of the display hardware. Returns
    QImage::LittleEndian (LSB first) or QImage::BigEndian (MSB first).

    This function is no longer relevant for QImage. Use QSysInfo
    instead.
*/


/*!
    \internal

    Used by QPainter to retreive a paint engine for the image.
*/

QPaintEngine *QImage::paintEngine() const
{
    if (!d)
        return 0;

#ifdef QT_RASTER_IMAGEENGINE
    if (!d->paintEngine) {
        d->paintEngine = new QRasterPaintEngine();
    }
#endif
    return d->paintEngine;
}


/*!
    \reimp

    Returns the size for the specified \a metric on the device.
*/
int QImage::metric(PaintDeviceMetric metric) const
{
    if (!d)
        return 0;

    switch (metric) {
    case PdmWidth:
        return d->width;
        break;

    case PdmHeight:
        return d->height;
        break;

    case PdmWidthMM:
        return qRound(d->width * 1000 / d->dpmx);
        break;

    case PdmHeightMM:
        return qRound(d->height * 1000 / d->dpmy);
        break;

    case PdmNumColors:
        return d->colortable.size();
        break;

    case PdmDepth:
        return d->depth;
        break;

    case PdmDpiX:
        return qRound(d->dpmx * 0.0254);
        break;

    case PdmDpiY:
        return qRound(d->dpmy * 0.0254);
        break;

    case PdmPhysicalDpiX:
        return qRound(d->dpmx * 0.0254);
        break;

    case PdmPhysicalDpiY:
        return qRound(d->dpmy * 0.0254);
        break;

    default:
        qWarning("QImage::metric(), Unhandled metric type: %d\n", metric);
        break;
    }
    return 0;
}



/*****************************************************************************
  QPixmap (and QImage) helper functions
 *****************************************************************************/
/*
  This internal function contains the common (i.e. platform independent) code
  to do a transformation of pixel data. It is used by QPixmap::transform() and by
  QImage::transform().

  \a trueMat is the true transformation matrix (see QPixmap::trueMatrix()) and
  \a xoffset is an offset to the matrix.

  \a msbfirst specifies for 1bpp images, if the MSB or LSB comes first and \a
  depth specifies the colordepth of the data.

  \a dptr is a pointer to the destination data, \a dbpl specifies the bits per
  line for the destination data, \a p_inc is the offset that we advance for
  every scanline and \a dHeight is the height of the destination image.

  \a sprt is the pointer to the source data, \a sbpl specifies the bits per
  line of the source data, \a sWidth and \a sHeight are the width and height of
  the source data.
*/

#undef IWX_MSB
#define IWX_MSB(b)        if (trigx < maxws && trigy < maxhs) {                              \
                            if (*(sptr+sbpl*(trigy>>12)+(trigx>>15)) &                      \
                                 (1 << (7-((trigx>>12)&7))))                              \
                                *dptr |= b;                                              \
                        }                                                              \
                        trigx += m11;                                                      \
                        trigy += m12;
        // END OF MACRO
#undef IWX_LSB
#define IWX_LSB(b)        if (trigx < maxws && trigy < maxhs) {                              \
                            if (*(sptr+sbpl*(trigy>>12)+(trigx>>15)) &                      \
                                 (1 << ((trigx>>12)&7)))                              \
                                *dptr |= b;                                              \
                        }                                                              \
                        trigx += m11;                                                      \
                        trigy += m12;
        // END OF MACRO
#undef IWX_PIX
#define IWX_PIX(b)        if (trigx < maxws && trigy < maxhs) {                              \
                            if ((*(sptr+sbpl*(trigy>>12)+(trigx>>15)) &              \
                                 (1 << (7-((trigx>>12)&7)))) == 0)                      \
                                *dptr &= ~b;                                              \
                        }                                                              \
                        trigx += m11;                                                      \
                        trigy += m12;
        // END OF MACRO
bool qt_xForm_helper(const QMatrix &trueMat, int xoffset, int type, int depth,
                     uchar *dptr, int dbpl, int p_inc, int dHeight,
                     const uchar *sptr, int sbpl, int sWidth, int sHeight)
{
    int m11 = qRound(trueMat.m11()*4096.0);
    int m12 = qRound(trueMat.m12()*4096.0);
    int m21 = qRound(trueMat.m21()*4096.0);
    int m22 = qRound(trueMat.m22()*4096.0);
    int dx  = qRound(trueMat.dx()*4096.0);
    int dy  = qRound(trueMat.dy()*4096.0);

    int m21ydx = dx + (xoffset<<16);
    int m22ydy = dy;
    uint trigx;
    uint trigy;
    uint maxws = sWidth<<12;
    uint maxhs = sHeight<<12;

    for (int y=0; y<dHeight; y++) {                // for each target scanline
        trigx = m21ydx;
        trigy = m22ydy;
        uchar *maxp = dptr + dbpl;
        if (depth != 1) {
            switch (depth) {
                case 8:                                // 8 bpp transform
                while (dptr < maxp) {
                    if (trigx < maxws && trigy < maxhs)
                        *dptr = *(sptr+sbpl*(trigy>>12)+(trigx>>12));
                    trigx += m11;
                    trigy += m12;
                    dptr++;
                }
                break;

                case 16:                        // 16 bpp transform
                while (dptr < maxp) {
                    if (trigx < maxws && trigy < maxhs)
                        *((ushort*)dptr) = *((ushort *)(sptr+sbpl*(trigy>>12) +
                                                     ((trigx>>12)<<1)));
                    trigx += m11;
                    trigy += m12;
                    dptr++;
                    dptr++;
                }
                break;

                case 24:                        // 24 bpp transform
                while (dptr < maxp) {
                    if (trigx < maxws && trigy < maxhs) {
                        const uchar *p2 = sptr+sbpl*(trigy>>12) + ((trigx>>12)*3);
                        dptr[0] = p2[0];
                        dptr[1] = p2[1];
                        dptr[2] = p2[2];
                    }
                    trigx += m11;
                    trigy += m12;
                    dptr += 3;
                }
                break;

                case 32:                        // 32 bpp transform
                while (dptr < maxp) {
                    if (trigx < maxws && trigy < maxhs)
                        *((uint*)dptr) = *((uint *)(sptr+sbpl*(trigy>>12) +
                                                   ((trigx>>12)<<2)));
                    trigx += m11;
                    trigy += m12;
                    dptr += 4;
                }
                break;

                default: {
                return false;
                }
            }
        } else  {
            switch (type) {
                case QT_XFORM_TYPE_MSBFIRST:
                    while (dptr < maxp) {
                        IWX_MSB(128);
                        IWX_MSB(64);
                        IWX_MSB(32);
                        IWX_MSB(16);
                        IWX_MSB(8);
                        IWX_MSB(4);
                        IWX_MSB(2);
                        IWX_MSB(1);
                        dptr++;
                    }
                    break;
                case QT_XFORM_TYPE_LSBFIRST:
                    while (dptr < maxp) {
                        IWX_LSB(1);
                        IWX_LSB(2);
                        IWX_LSB(4);
                        IWX_LSB(8);
                        IWX_LSB(16);
                        IWX_LSB(32);
                        IWX_LSB(64);
                        IWX_LSB(128);
                        dptr++;
                    }
                    break;
#  if defined(Q_WS_WIN)
                case QT_XFORM_TYPE_WINDOWSPIXMAP:
                    while (dptr < maxp) {
                        IWX_PIX(128);
                        IWX_PIX(64);
                        IWX_PIX(32);
                        IWX_PIX(16);
                        IWX_PIX(8);
                        IWX_PIX(4);
                        IWX_PIX(2);
                        IWX_PIX(1);
                        dptr++;
                    }
                    break;
#  endif
            }
        }
        m21ydx += m21;
        m22ydy += m22;
        dptr += p_inc;
    }
    return true;
}
#undef IWX_MSB
#undef IWX_LSB
#undef IWX_PIX

/*!
    \fn QImage QImage::xForm(const QMatrix &matrix) const

    Use transformed() instead.

    \oldcode
        QImage image;
        ...
        image.xForm(matrix);
    \newcode
        QImage image;
        ...
        image.transformed(matrix);
    \endcode
*/

/*!
    Returns a number that uniquely identifies the contents of this
    QImage object.

    This means that multiple QImage objects can only have the same
    serial number as long as they refer to the same contents. A null
    image has always a serial number of 0.

    The serial number is useful is when caching QImages.

    \sa {QImage#Image Information}{Image Information}
*/

int QImage::serialNumber() const
{
    if (!d)
        return 0;
    else
        return d->ser_no;
}

/*!
    \internal

    Returns true if the image is detached; otherwise returns false.

    \sa detach(), {Implicit Data Sharing}
*/

bool QImage::isDetached() const
{
    return d && d->ref == 1;
}


/*!
    Sets the alpha channel of this image to the given \a alphaChannel.

    If \a alphaChannel is an 8 bit grayscale image, the intensity
    values are written into this buffer directly. Otherwise, \a
    alphaChannel is converted to 32 bit and the intensity of the RGB
    pixel values is used.

    Note that the image will be converted to the
    Format_ARGB32_Premultiplied format if the function succeeds.

    \sa alphaChannel(), {QImage#Image Transformations}{Image
    Transformations}, {QImage#Image Formats}{Image Formats}
*/

void QImage::setAlphaChannel(const QImage &alphaChannel)
{
    if (!d)
        return;

    int w = d->width;
    int h = d->height;

    if (w != alphaChannel.d->width || h != alphaChannel.d->height) {
        qWarning("QImage::setAlphaChannel(), "
                 "alpha channel must have same dimensions as the target image.");
        return;
    }

    detach();

    *this = convertToFormat(QImage::Format_ARGB32_Premultiplied);

    // Slight optimization since alphachannels are returned as 8-bit grays.
    if (alphaChannel.d->depth == 8 && alphaChannel.isGrayscale()) {
        const uchar *src_data = alphaChannel.d->data;
        const uchar *dest_data = d->data;
        for (int y=0; y<h; ++y) {
            const uchar *src = src_data;
            QRgb *dest = (QRgb *)dest_data;
            for (int x=0; x<w; ++x) {
                int alpha = *src;
                int destAlpha = qt_div_255(alpha * qAlpha(*dest));
                *dest = ((destAlpha << 24)
                         | (qt_div_255(qRed(*dest) * alpha) << 16)
                         | (qt_div_255(qGreen(*dest) * alpha) << 8)
                         | (qt_div_255(qBlue(*dest) * alpha)));
                ++dest;
                ++src;
            }
            src_data += alphaChannel.d->bytes_per_line;
            dest_data += d->bytes_per_line;
        }

    } else {
        const QImage sourceImage = alphaChannel.convertToFormat(QImage::Format_RGB32);
        const uchar *src_data = sourceImage.d->data;
        const uchar *dest_data = d->data;
        for (int y=0; y<h; ++y) {
            const QRgb *src = (const QRgb *) src_data;
            QRgb *dest = (QRgb *) dest_data;
            for (int x=0; x<w; ++x) {
                int alpha = qGray(*src);
                int destAlpha = qt_div_255(alpha * qAlpha(*dest));
                *dest = ((destAlpha << 24)
                         | (qt_div_255(qRed(*dest) * alpha) << 16)
                         | (qt_div_255(qGreen(*dest) * alpha) << 8)
                         | (qt_div_255(qBlue(*dest) * alpha)));
                ++dest;
                ++src;
            }
            src_data += sourceImage.d->bytes_per_line;
            dest_data += d->bytes_per_line;
        }
    }
}


/*!
    Extracts the alpha channel from this image as an 8 bit gray scale
    image and returns it.

    \sa setAlphaChannel(), hasAlphaChannel(), {QImage#Image
    Transformations}{Image Transformations}
*/

QImage QImage::alphaChannel() const
{
    if (!d)
        return QImage();

    int w = d->width;
    int h = d->height;

    QImage image(w, h, Format_Indexed8);
    image.setNumColors(256);

    // set up gray scale table.
    for (int i=0; i<256; ++i)
        image.setColor(i, qRgb(i, i, i));

    if (d->format == Format_Indexed8 && hasAlphaChannel()) {
        const uchar *src_data = d->data;
        uchar *dest_data = image.d->data;
        for (int y=0; y<h; ++y) {
            const QRgb *src = (const QRgb *) src_data;
            uchar *dest = dest_data;
            for (int x=0; x<w; ++x) {
                *dest = qAlpha(d->colortable.at(*src));
                ++dest;
                ++src;
            }
            src_data += d->bytes_per_line;
            dest_data += image.d->bytes_per_line;
        }
    } else if (d->format == Format_ARGB32 || d->format == Format_ARGB32_Premultiplied) {
        const uchar *src_data = d->data;
        uchar *dest_data = image.d->data;
        for (int y=0; y<h; ++y) {
            const QRgb *src = (const QRgb *) src_data;
            uchar *dest = dest_data;
            for (int x=0; x<w; ++x) {
                *dest = qAlpha(*src);
                ++dest;
                ++src;
            }
            src_data += d->bytes_per_line;
            dest_data += image.d->bytes_per_line;
        }
    } else {
        image.fill(255);
    }

    return image;
}

/*!
    Returns true if the image has a format that respects the alpha
    channel, otherwise returns false.

    \sa alphaChannel(), {QImage#Image Information}{Image Information}
*/
bool QImage::hasAlphaChannel() const
{
    return d && (d->format == Format_ARGB32_Premultiplied
                 || d->format == Format_ARGB32
                 || (d->format == Format_Indexed8 && d->has_alpha_clut));
}


#ifdef QT3_SUPPORT
#if defined(Q_WS_X11)
#include <private/qt_x11_p.h>
#endif

QImage::Endian QImage::systemBitOrder()
{
#if defined(Q_WS_X11)
    return BitmapBitOrder(X11->display) == MSBFirst ? BigEndian : LittleEndian;
#else
    return BigEndian;
#endif
}
#endif

/*!
    \fn QImage QImage::copy(const QRect &rect, Qt::ImageConversionFlags flags) const
    \compat

    Use copy() instead.
*/

/*!
    \fn QImage QImage::copy(int x, int y, int w, int h, Qt::ImageConversionFlags flags) const
    \compat

    Use copy() instead.
*/

/*!
    \fn QImage QImage::scaleWidth(int w) const
    \compat

    Use scaledToWidth() instead.
*/

/*!
    \fn QImage QImage::scaleHeight(int h) const
    \compat

    Use scaledToHeight() instead.
*/
