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

#include "qscreen_qws.h"

#include "qcolormap.h"
#include "qscreendriverfactory_qws.h"
#include "qwindowsystem_qws.h"
#include "qwidget.h"
#include "qcolor.h"
#include "qpixmap.h"
#include "qwsdisplay_qws.h"
#include <private/qdrawhelper_p.h>
#include <private/qpaintengine_raster_p.h>
#include <private/qpainter_p.h>
#include <private/qwindowsurface_qws_p.h>
#include <private/qwidget_p.h>

// #define QT_USE_MEMCPY_DUFF

#ifndef QT_NO_QWS_CURSOR
bool qt_sw_cursor=false;
Q_GUI_EXPORT QScreenCursor * qt_screencursor = 0;
#endif
Q_GUI_EXPORT QScreen * qt_screen = 0;

ClearCacheFunc QScreen::clearCacheFunc = 0;

#ifndef QT_NO_QWS_CURSOR
/*!
    \class QScreenCursor
    \ingroup qws

    \brief The QScreenCursor class is a base class for screen cursors
    in Qtopia Core.

    Note that this class is non-portable, and that it is only
    available in \l {Qtopia Core}.

    QScreenCursor implements a software cursor, but can be subclassed
    to support hardware cursors as well. When deriving from the
    QScreenCursor class it is important to maintain the cursor's
    image, position, hot spot (the point within the cursor's image
    that will be the position of the associated mouse events) and
    visibility as well as informing whether it is hardware accelerated
    or not.

    Note that there may only be one screen cursor at a time. Use the
    static instance() function to retrieve a pointer to the current
    screen cursor. Typically, the cursor is constructed by the QScreen
    class or one of its descendants when it is initializing the
    device; the QScreenCursor class should never be instantiated
    explicitly.

    Use the move() function to change the position of the cursor, and
    the set() function to alter its image or its hot spot. In
    addition, you can find out whether the cursor is accelerated or
    not, using the isAccelerated() function, and the boundingRect()
    function returns the cursor's bounding rectangle.

    The cursor's appearance can be controlled using the isVisible(),
    hide() and show() functions; alternatively the QWSServer class
    provides some means of controlling the cursor's appearance using
    the QWSServer::isCursorVisible() and QWSServer::setCursorVisible()
    functions.

    \sa QScreen, QWSServer
*/

/*!
    \fn static QScreenCursor* QScreenCursor::instance()
    \since 4.2

    Returns a pointer to the application's unique screen cursor.
*/

extern bool qws_sw_cursor;

/*!
    Constructs a screen cursor
*/
QScreenCursor::QScreenCursor()
{
    pos = QPoint(qt_screen->deviceWidth()/2, qt_screen->deviceHeight()/2);
    size = QSize(0,0);
    enable = true;
    hwaccel = false;
    supportsAlpha = true;
}

/*!
    Destroys the screen cursor.
*/
QScreenCursor::~QScreenCursor()
{
}

/*!
    Hides the cursor from the screen.

    \sa show()
*/
void QScreenCursor::hide()
{
    if (enable) {
        enable = false;
        if (!hwaccel)
            qt_screen->exposeRegion(boundingRect(), 0);
    }
}

/*!
    Shows the mouse cursor.

    \sa hide()
*/
void QScreenCursor::show()
{
    if (!enable) {
        enable = true;
        if (!hwaccel)
            qt_screen->exposeRegion(boundingRect(), 0);
    }
}

/*!
    Sets the cursor's image to be the given \a image.

    The \a hotx and \a hoty parameters define the cursor's hot spot,
    i.e., the point within the cursor's image that will be the
    position of the associated mouse events.

    \sa move()
*/
void QScreenCursor::set(const QImage &image, int hotx, int hoty)
{
    const QRect r = boundingRect();

    hotspot = QPoint(hotx, hoty);
    cursor = image;
    size = image.size();

    if (enable && !hwaccel)
        qt_screen->exposeRegion(r | boundingRect(), 0);
}

/*!
    Moves the mouse cursor to the given position, i.e., (\a x, \a y).

    Note that the given position defines the top-left corner of the
    cursor's image, i.e., not the cursor's hot spot (the position of
    the associated mouse events).

    \sa set()
*/
void QScreenCursor::move(int x, int y)
{
    const QRegion r = boundingRect();
    pos = QPoint(x,y);
    if (enable && !hwaccel)
        qt_screen->exposeRegion(r | boundingRect(), 0);
}


/*!
    \fn void QScreenCursor::initSoftwareCursor ()

    Initializes the screen cursor.

    This function is typically called from the screen driver when
    initializing the device. Alternatively, the cursor can be set
    directly using the pointer returned by the static instance()
    function.

    \sa QScreen::initDevice()
*/
void QScreenCursor::initSoftwareCursor()
{
    qt_screencursor = new QScreenCursor;
}


#endif // QT_NO_QWS_CURSOR


/*!
    \fn QRect QScreenCursor::boundingRect () const

    Returns the cursor's bounding rectangle.
*/

/*!
    \internal
    \fn bool QScreenCursor::enabled ()
*/

/*!
    \fn QImage QScreenCursor::image () const

    Returns the cursor's image.
*/


/*!
    \fn bool QScreenCursor::isAccelerated () const

    Returns true if the cursor is accelerated; otherwise false.
*/

/*!
    \fn bool QScreenCursor::isVisible () const

    Returns true if the cursor is visible; otherwise false.
*/

/*!
    \internal
    \fn bool QScreenCursor::supportsAlphaCursor () const
*/

/*
    \variable QScreenCursor::cursor

    \brief the cursor's image.

    \sa image()
*/

/*
    \variable QScreenCursor::size

    \brief the cursor's size
*/

/*
    \variable QScreenCursor::pos

    \brief the cursor's position, i.e., the position of the top-left
    corner of the crsor's image

    \sa set(), move()
*/

/*
    \variable QScreenCursor::hotspot

    \brief the cursor's hotspot, i.e., the point within the cursor's
    image that will be the position of the associated mouse events.

    \sa set(), move()
*/

/*
    \variable QScreenCursor::enable

    \brief whether the cursor is visible or not

    \sa isVisible()
*/

/*
    \variable QScreenCursor::hwaccel

    \brief holds whether the cursor is accelerated or not

    If the cursor is not accelerated, its image will be included by
    the screen when it composites the window surfaces.

    \sa isAccelerated()

*/

/*
    \variable QScreenCursor::supportsAlpha
*/

/*!
    \internal
    \macro qt_screencursor
    \relates QScreenCursor

    A global pointer referring to the unique screen cursor. It is
    equivalent to the pointer returned by the
    QScreenCursor::instance() function.
*/



class QScreenPrivate
{
public:
    QScreenPrivate(QScreen *parent);

    inline QImage::Format preferredImageFormat() const;

    typedef void (*SolidFillFunc)(QScreen*, const QColor&, const QRegion&);
    typedef void (*BlitFunc)(QScreen*, const QImage&, const QPoint&, const QRegion&);

    SolidFillFunc solidFill;
    BlitFunc blit;

    QPoint offset;
    QList<QScreen*> subScreens;
    QImage::Format pixelFormat;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    bool fb_is_littleEndian;
#endif
    QScreen *q_ptr;
};

template <typename T>
static void solidFill_template(QScreen *screen, const QColor &color,
                               const QRegion &region)
{
    T *dest = reinterpret_cast<T*>(screen->base());
    const T c = qt_colorConvert<T, quint32>(color.rgba(), 0);
    const int stride = screen->linestep();
    const QVector<QRect> rects = region.rects();

    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i);
        qt_rectfill(dest, c, r.x(), r.y(), r.width(), r.height(), stride);
    }
}

#ifdef QT_QWS_DEPTH_4
static inline void qt_rectfill_gray4(quint8 *dest, quint8 value,
                                     int x, int y, int width, int height,
                                     int stride)
{
    const int pixelsPerByte = 2;
    dest += y * stride + x / pixelsPerByte;
    const int doAlign = x & 1;
    const int doTail = (width - doAlign) & 1;
    const int width8 = (width - doAlign) / pixelsPerByte;

    for (int j = 0; j < height; ++j) {
        if (doAlign)
            *dest = (*dest & 0xf0) | (value & 0x0f);
        if (width8)
            qt_memfill<quint8>(dest + doAlign, value, width8);
        if (doTail) {
            quint8 *d = dest + doAlign + width8;
            *d = (*d & 0x0f) | (value & 0xf0);
        }
        dest += stride;
    }
}

static void solidFill_gray4(QScreen *screen, const QColor &color,
                            const QRegion &region)
{
    quint8 *dest = reinterpret_cast<quint8*>(screen->base());
    const quint8 c = qGray(color.rgba()) >> 4;
    const quint8 c8 = (c << 4) | c;

    const int stride = screen->linestep();
    const QVector<QRect> rects = region.rects();

    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i);
        qt_rectfill_gray4(dest, c8, r.x(), r.y(), r.width(), r.height(),
                          stride);
    }
}
#endif // QT_QWS_DEPTH_4

void qt_solidFill_setup(QScreen *screen, const QColor &color,
                        const QRegion &region)
{
    switch (screen->depth()) {
#ifdef QT_QWS_DEPTH_32
    case 32:
        screen->d_ptr->solidFill = solidFill_template<quint32>;
        break;
#endif
#ifdef QT_QWS_DEPTH_24
    case 24:
        screen->d_ptr->solidFill = solidFill_template<quint24>;
        break;
#endif
#ifdef QT_QWS_DEPTH_18
    case 18:
        screen->d_ptr->solidFill = solidFill_template<quint18>;
        break;
#endif
#ifdef QT_QWS_DEPTH_16
    case 16:
        screen->d_ptr->solidFill = solidFill_template<quint16>;
        break;
#endif
#ifdef QT_QWS_DEPTH_8
    case 8:
        screen->d_ptr->solidFill = solidFill_template<quint8>;
        break;
#endif
#ifdef QT_QWS_DEPTH_4
    case 4:
        screen->d_ptr->solidFill = solidFill_gray4;
        break;
#endif
     default:
        qFatal("solidFill_setup(): Screen depth %d not supported!",
               screen->depth());
        screen->d_ptr->solidFill = 0;
        break;
    }
    screen->solidFill(color, region);
}

template <typename DST, typename SRC>
static void blit_template(QScreen *screen, const QImage &image,
                          const QPoint &topLeft, const QRegion &region)
{
    DST *dest = reinterpret_cast<DST*>(screen->base());
    const SRC *src = reinterpret_cast<const SRC*>(image.bits());
    const int screenStride = screen->linestep();
    const int imageStride = image.bytesPerLine();
    const QVector<QRect> rects = region.rects();

    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i);
        qt_rectconvert<DST, SRC>(dest,
                                 src + r.y() * imageStride / sizeof(SRC) + r.x(),
                                 r.x() + topLeft.x(), r.y() + topLeft.y(),
                                 r.width(), r.height(),
                                 screenStride, imageStride);
    }
}

#ifdef QT_QWS_DEPTH_32
static void blit_32(QScreen *screen, const QImage &image,
                    const QPoint &topLeft, const QRegion &region)
{
    switch (image.format()) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        blit_template<quint32, quint32>(screen, image, topLeft, region);
        return;
#ifdef QT_QWS_DEPTH_16
    case QImage::Format_RGB16:
        blit_template<quint32, quint16>(screen, image, topLeft, region);
        return;
#endif
    default:
        qCritical("blit_16(): Image format %d not supported!", image.format());
    }
}
#endif // QT_QWS_DEPTH_32

#ifdef QT_QWS_DEPTH_16
static void blit_16(QScreen *screen, const QImage &image,
                    const QPoint &topLeft, const QRegion &region)
{
    switch (image.format()) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        blit_template<quint16, quint32>(screen, image, topLeft, region);
        return;
    case QImage::Format_RGB16:
        blit_template<quint16, quint16>(screen, image, topLeft, region);
        return;
    default:
        qCritical("blit_16(): Image format %d not supported!", image.format());
    }
}

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
class quint16LE
{
public:
    inline quint16LE(quint32 v) {
        data = ((v & 0xff00) >> 8) | ((v & 0x00ff) << 8);
    }

    inline quint16LE(quint16 v) {
        data = ((v & 0xff00) >> 8) | ((v & 0x00ff) << 8);
    }

private:
    quint16 data;
};

static void blit_16_bigToLittleEndian(QScreen *screen, const QImage &image,
                                      const QPoint &topLeft,
                                      const QRegion &region)
{
    switch (image.format()) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        blit_template<quint16LE, quint32>(screen, image, topLeft, region);
        return;
    case QImage::Format_RGB16:
        blit_template<quint16LE, quint16>(screen, image, topLeft, region);
        return;
    default:
        qCritical("blit_16_bigToLittleEndian(): Image format %d not supported!", image.format());
    }
}

#endif // Q_BIG_ENDIAN
#endif // QT_QWS_DEPTH_16

#ifdef QT_QWS_DEPTH_8
static void blit_8(QScreen *screen, const QImage &image,
                   const QPoint &topLeft, const QRegion &region)
{
    switch (image.format()) {
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        blit_template<quint8, quint32>(screen, image, topLeft, region);
        return;
    case QImage::Format_RGB16:
        blit_template<quint8, quint16>(screen, image, topLeft, region);
        return;
    default:
        qCritical("blit_8(): Image format %d not supported!", image.format());
    }
}
#endif // QT_QWS_DEPTH_8

#ifdef QT_QWS_DEPTH_4

struct qgray4 { quint8 dummy; } Q_PACKED;

template <typename SRC>
static inline quint8 qt_convertToGray4(SRC color);

template <>
static inline quint8 qt_convertToGray4(quint32 color)
{
    return qGray(color) >> 4;
}

template <>
static inline quint8 qt_convertToGray4(quint16 color)
{
    return (qGray(qt_colorConvert<quint32, quint16>(color, 0)) >> 4);
}

template <typename SRC>
static inline void qt_rectconvert_gray4(qgray4 *dest4, const SRC *src,
                                        int x, int y, int width, int height,
                                        int dstStride, int srcStride)
{
    const int pixelsPerByte = 2;
    quint8 *dest8 = reinterpret_cast<quint8*>(dest4)
                    + y * dstStride + x / pixelsPerByte;
    const int doAlign = x & 1;
    const int doTail = (width - doAlign) & 1;
    const int width8 = (width - doAlign) / pixelsPerByte;
    const int count8 = (width8 + 3) / 4;

    srcStride = srcStride / sizeof(SRC) - width;
    dstStride -= (width8 + doAlign);

    for (int i = 0; i < height; ++i) {
        if (doAlign) {
            *dest8 = (*dest8 & 0xf0) | qt_convertToGray4<SRC>(*src++);
            ++dest8;
        }
        if (count8) {
            int n = count8;
            switch (width8 & 0x03) // duff's device
            {
            case 0: do { *dest8++ = qt_convertToGray4<SRC>(src[0]) << 4
                                    | qt_convertToGray4<SRC>(src[1]);
                         src += 2;
            case 3:      *dest8++ = qt_convertToGray4<SRC>(src[0]) << 4
                                    | qt_convertToGray4<SRC>(src[1]);
                         src += 2;
            case 2:      *dest8++ = qt_convertToGray4<SRC>(src[0]) << 4
                                    | qt_convertToGray4<SRC>(src[1]);
                         src += 2;
            case 1:      *dest8++ = qt_convertToGray4<SRC>(src[0]) << 4
                                    | qt_convertToGray4<SRC>(src[1]);
                         src += 2;
            } while (--n > 0);
            }
        }

        if (doTail)
            *dest8 = qt_convertToGray4<SRC>(*src++) << 4 | (*dest8 & 0x0f);

        dest8 += dstStride;
        src += srcStride;
    }
}

template <>
void qt_rectconvert(qgray4 *dest, const quint32 *src,
                    int x, int y, int width, int height,
                    int dstStride, int srcStride)
{
    qt_rectconvert_gray4<quint32>(dest, src, x, y, width, height,
                                  dstStride, srcStride);
}

template <>
void qt_rectconvert(qgray4 *dest, const quint16 *src,
                    int x, int y, int width, int height,
                    int dstStride, int srcStride)
{
    qt_rectconvert_gray4<quint16>(dest, src, x, y, width, height,
                                  dstStride, srcStride);
}

static void blit_4(QScreen *screen, const QImage &image,
                   const QPoint &topLeft, const QRegion &region)
{
    switch (image.format()) {
    case QImage::Format_ARGB32_Premultiplied:
        blit_template<qgray4, quint32>(screen, image, topLeft, region);
        return;
    case QImage::Format_RGB16:
        blit_template<qgray4, quint16>(screen, image, topLeft, region);
        return;
    default:
        qCritical("blit_4(): Image format %d not supported!", image.format());
    }
}
#endif // QT_QWS_DEPTH_4

void qt_blit_setup(QScreen *screen, const QImage &image,
                   const QPoint &topLeft, const QRegion &region)
{
    switch (screen->depth()) {
#ifdef QT_QWS_DEPTH_32
    case 32:
        screen->d_ptr->blit = blit_32;
        break;
#endif
#ifdef QT_QWS_DEPTH_24
    case 24:
        screen->d_ptr->blit = blit_template<quint24, quint32>;
        break;
#endif
#ifdef QT_QWS_DEPTH_18
    case 18:
        screen->d_ptr->blit = blit_template<quint18, quint32>;
        break;
#endif
#ifdef QT_QWS_DEPTH_16
    case 16:
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
        if (screen->d_ptr->fb_is_littleEndian)
            screen->d_ptr->blit = blit_16_bigToLittleEndian;
        else
#endif
            screen->d_ptr->blit = blit_16;
        break;
#endif
#ifdef QT_QWS_DEPTH_8
    case 8:
        screen->d_ptr->blit = blit_8;
        break;
#endif
#ifdef QT_QWS_DEPTH_4
    case 4:
        screen->d_ptr->blit = blit_4;
        break;
#endif
    default:
        qFatal("blit_setup(): Screen depth %d not supported!",
               screen->depth());
        screen->d_ptr->blit = 0;
        break;
    }
    screen->blit(image, topLeft, region);
}

QScreenPrivate::QScreenPrivate(QScreen *parent)
    :  pixelFormat(QImage::Format_Invalid), q_ptr(parent)
{
    solidFill = qt_solidFill_setup;
    blit = qt_blit_setup;
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
    fb_is_littleEndian = true;
#endif
}

QImage::Format QScreenPrivate::preferredImageFormat() const
{
    if (pixelFormat == QImage::Format_Invalid)
        return QImage::Format_ARGB32_Premultiplied;

    return pixelFormat;
}

/*!
    \class QScreen
    \ingroup qws

    \brief The QScreen class is a base class for screen drivers in
    Qtopia Core.

    Note that this class is only available in \l {Qtopia Core}.

    \l {Qtopia Core} provides ready-made drivers for several screen
    protocols, see the \l {Qtopia Core Display Management}{display
    management} documentation for details. Custom screen drivers can
    be implemented by subclassing the QScreen class and creating a
    screen driver plugin (derived from QScreenDriverPlugin). \l
    {Qtopia Core}'s implementation of the QScreenDriverFactory class
    will automatically detect the plugin, and load the driver into the
    server application at runtime using Qt's \l {How to Create Qt
    Plugins}{plugin system}.

    When rendering, \l {Qtopia Core}'s default behavior is for each
    client to render its widgets as well as its decorations into
    memory, while the server copies the memory content to the device's
    framebuffer using the screen driver. See the \l {Qtopia Core
    Architecture} overview for details (note that it is possible for
    the clients to manipulate and control the underlying hardware
    directly as well).

    Starting with \l {Qtopia Core} 4.2, it is also possible to add an
    accelerated graphics driver to take advantage of available
    hardware resources. See the \l {Adding an Accelerated Graphics
    Driver in Qtopia Core} documentation for details.

    \tableofcontents

    \section1 Framebuffer Management

    When a \l {Qtopia Core} application starts running, it calls the
    screen driver's connect() function to map into the framebuffer and
    the accelerated drivers that the graphics card control
    registers. Note that if the application acts as the server, the
    application will call the initDevice() function prior to the
    connect() function, to initialize the framebuffer. The
    initDevice() function can be reimplemented to set up the graphics
    card (note that the default implementation does nothing).

    Likewise, just before a \l {Qtopia Core} application exits, it
    calls the screen driver's disconnect() function. The server
    application will in addition call the shutdownDevice() function
    before it calls disconnect(). Note that the default implementation
    of the shutdownDevice() function only hides the mouse cursor.

    QScreen also provides the save() and restore() functions, making
    it possible to save and restore the state of the graphics
    card. Note that the default implementations do nothing. Hardware
    screen drivers should reimplement these functions to save (and
    restore) its registers, enabling swintching between virtual
    consoles.

    In additon, you can use the base() function to retrieve a pointer
    to the beginning of the framebuffer, and the region() function to
    retrieve the framebuffer's region. Use the onCard() function to
    determine whether the framebuffer is within the graphics card's
    memory, and the totalSize() function to determine the size of the
    available graphics card memory (including the screen). Finally,
    you can use the offset() function to retrieve the offset between
    the framebuffer's coordinates and the application's coordinate
    system.

    \section1 Palette Management

    QScreen provides several functions to retrieve information about
    the color palette: The clut() function returns a pointer to the
    color lookup table (i.e. its color palette). Use the numCols()
    function to determine the number of entries in this table, and the
    alloc() function to retrieve the palette index of the color that
    is the closest match to a given RGB value.

    To determine if the screen driver supports a given color depth,
    use the supportsDepth() function that returns true of the
    specified depth is supported.

    \section1 Drawing on Screen

    When a screen update is required, the \l {Qtopia Core} server runs
    through all the top-level windows that intersect with the region
    that is about to be updated, and ensures that the associated
    clients have updated their memory buffer. Then the server calls
    the exposeRegion() function that composes the window surfaces and
    copies the content of memory to screen by calling the blit() and
    solidFill() functions.

    The blit() function copies a given region in a given image to a
    specified point using device coordinates, while the solidFill()
    function fills the given region of the screen with the specified
    color. Note that normally there is no need to call either of these
    functions explicitly.

    In addition, QScreen provides the blank() function that can be
    reimplemented to prevent any contents from being displayed on the
    screen, and the setDirty() function that can be reimplemented to
    indicate that a given rectangle of the screen has been
    altered. Note that the default implementations of these functions
    do nothing.

    Reimplement the the mapFromDevice() and mapToDevice() functions to
    map objects from the framebuffer coordinate system to the
    coordinate space used by the application, and vice versa. Be aware
    that the default implementations simply return the given objects
    as they are.

    \section1 Properties

    \table
    \header \o Property \o Functions
    \row
    \o Size
    \o

    The size of the screen can be retrieved using the screenSize()
    function. The size is returned in bytes.

    The framebuffer's logical width and height can be retrieved using
    width() and height(), respectively. These functions return values
    are given in pixels. Alternatively, the physicalWidth() and
    physicalHeight() function returns the same metrics in
    millimeters. QScreen also provides the deviceWidth() and
    deviceHeight() functions returning the physical width and height
    of the device in pixels. Note that the latter metrics can differ
    from the ones used if the display is centered within the
    framebuffer.

    \row
    \o Resolution
    \o

    Reimplement the setMode() function to be able to set the
    framebuffer to a new resolution (width and height) and bit depth.

    The current depth of the framebuffer can be always be retrieved
    using the depth() function. Use the pixmapDepth() function to
    obtain the preferred depth for pixmaps.

    \row
    \o Pixmap Alignment
    \o

    Use the pixmapOffsetAlignment() function to retrieve the value to
    which the start address of pixmaps held in the graphics card's
    memory, should be aligned.

    Use the pixmapLinestepAlignment() to retrieve the value to which
    the \e {individual scanlines} of pixmaps should be aligned.

    \row
    \o Image Display
    \o

    The isInterlaced() function tells whether the screen is displaying
    images progressively, and the isTransformed() function whether it
    is rotated. The transformOrientation() function can be
    reimplemented to return the current rotation.

    \row
    \o Scanlines
    \o

    Use the linestep() function to retrieve the length of each
    scanline of the framebuffer.

    \row
    \o Pixel Type
    \o

    The pixelType() function returns the screen's pixel storage format as
    described by the PixelType enum.

    \endtable

    \sa QScreenDriverPlugin, QScreenDriverFactory, {Qtopia Core Display
    Management}
*/

/*!
    \enum QScreen::PixelType

    This enum describes the pixel storage format of the screen,
    i.e. the order of the red (R), green (G) and blue (B) components
    of a pixel.

    \value NormalPixel Red-green-blue (RGB)
    \value BGRPixel Blue-green-red (BGR)

    \sa pixelType()
*/

/*!
    \fn static QScreen* QScreen::instance()

    Returns a pointer to the application's QScreen instance.

    If this screen consists of several subscreens, operations to the
    returned instance will affect all its subscreens. Use the
    subscreens() function to retrieve access to a particular
    subscreen.

    \sa subScreens(), subScreenIndexAt()
*/

/*!
    \fn QList<QScreen*> QScreen::subScreens() const
    \since 4.2

    Returns a list of this screen's subscreens. Use the
    subScreenIndexAt() function to retrieve the index of a screen at a
    given position.

    Note that if \e this screen consists of several subscreens,
    operations to \e this instance will affect all subscreens by
    default.

    \sa instance(), subScreenIndexAt()
*/

/*!
    \fn int QScreen::physicalWidth() const
    \since 4.2

    Returns the physical width of the screen in millimeters.

    \sa width(), deviceWidth(), physicalHeight()
*/

/*!
    \fn int QScreen::physicalHeight() const
    \since 4.2

    Returns the physical height of the screen in millimeters.

    \sa height(), deviceHeight(), physicalWidth()
*/

/*!
    \fn virtual bool QScreen::initDevice() = 0

    This function is called by the \l {Qtopia Core} server to
    initialize the framebuffer. Note that it is called \e before the
    connect() function.

    Implement this function to make accelerated drivers set up the
    graphics card. Return true to indicate success and false to indicate
    failure.

    \sa shutdownDevice(), connect()
*/

/*!
    \fn virtual bool QScreen::connect(const QString &displaySpec) = 0

    This function is called by every \l {Qtopia Core} application on
    startup, and must be implemented to map in the framebuffer and the
    accelerated drivers that the graphics card control registers.
    Note that a server application will call the initDevice() function
    prior to this function.

    Ensure that the function returns true if a connection to the
    screen device can be made; otherwise return false.

    The \a displaySpec argument is passed by the QWS_DISPLAY
    environment variable or the -display command line parameter, and
    has the following syntax:

    \code
        [screen driver][:driver specific options][:display number]
    \endcode

    For example, to use the mach64 driver on fb1 as display 2:

    \code
        Mach64:/dev/fb1:2
    \endcode

    See \l {Qtopia Core Display Management} for more details.

    \sa disconnect(), initDevice(), {Running Qtopia Core Applications}
*/

/*!
    \fn QScreen::disconnect()

    This function is called by every \l {Qtopia Core} application
    before exiting, and must be implemented to unmap the
    framebuffer. Note that a server application will call the
    shutdownDevice() function prior to this function.

    \sa connect(), shutdownDevice(), {Running Qtopia Core
    Applications}
*/

/*!
    \fn QScreen::setMode(int width, int height, int depth)

    Implement this function to reset the framebuffer's resolution (\a
    width and \a height) and bit \a depth.

    After the resolution has been set, existing paint engines will be
    invalid and the framebuffer should be completely redrawn. In a
    multiple-process situation, all other applications must be
    notified to reset their mode and update themselves accordingly.
*/

/*!
    \fn QScreen::blank(bool on)

    Prevents the screen driver form displaying any content on the
    screen.

    Note that the default implementation does nothing.

    Reimplement this function to prevent the screen driver from
    displaying any contents on the screen if \a on is true; otherwise
    the contents is expected to be shown.

    \sa blit()
*/

/*!
    \fn int QScreen::pixmapOffsetAlignment()

    Returns the value (in bits) to which the start address of pixmaps
    held in the graphics card's memory, should be aligned.

    Note that the default implementation returns 64; reimplement this
    function to override the return value, e.g., when implementing an
    accelerated driver (see the \l {Adding an Accelerated Graphics
    Driver in Qtopia Core}{Adding an Accelerated Graphics Driver}
    documentation for details).

    \sa pixmapLinestepAlignment()
*/

/*!
    \fn int QScreen::pixmapLinestepAlignment()

    Returns the value (in bits) to which individual scanlines of
    pixmaps held in the graphics card's memory, should be
    aligned.

    Note that the default implementation returns 64; reimplement this
    function to override the return value, e.g., when implementing an
    accelerated driver (see the \l {Adding an Accelerated Graphics
    Driver in Qtopia Core}{Adding an Accelerated Graphics Driver}
    documentation for details).

    \sa pixmapOffsetAlignment()
*/

/*!
    \fn QScreen::width() const

    Returns the logical width of the framebuffer in pixels.

    \sa deviceWidth(), physicalWidth(), height()
*/

/*!
    \fn int QScreen::height() const

    Returns the logical height of the framebuffer in pixels.

    \sa deviceHeight(), physicalHeight(), width()
*/

/*!
    \fn QScreen::depth() const

    Returns the depth of the framebuffer, in bits per pixel.

    Note that the returned depth is the number of bits each pixel
    fills rather than the number of significant bits, so 24bpp and
    32bpp express the same range of colors (8 bits of red, green and
    blue).

    \sa clut(), pixmapDepth()
*/

/*!
    \fn int QScreen::pixmapDepth() const

    Returns the preferred depth for pixmaps, in bits per pixel.

    \sa depth()
*/

/*!
    \fn QScreen::linestep() const

    Returns the length of each scanline of the framebuffer in bytes.

    \sa isInterlaced()
*/

/*!
    \fn QScreen::deviceWidth() const

    Returns the physical width of the framebuffer device in pixels.

    Note that the returned width can differ from the width which \l
    {Qtopia Core} will actually use, that is if the display is
    centered within the framebuffer.

    \sa width(), physicalWidth(), deviceHeight()
*/

/*!
    \fn QScreen::deviceHeight() const

    Returns the full height of the framebuffer device in pixels.

    Note that the returned height can differ from the height which \l
    {Qtopia Core} will actually use, that is if the display is
    centered within the framebuffer.

    \sa height(), physicalHeight(), deviceWidth()
*/

/*!
    \fn uchar *QScreen::base() const

    Returns a pointer to the beginning of the framebuffer.

    \sa onCard(), region(), totalSize()
*/

/*!
    \fn uchar *QScreen::cache(int)

    \internal

    This function is used to store pixmaps in graphics memory for the
    use of the accelerated drivers. See QLinuxFbScreen (where the
    caching is implemented) for more information.
*/

/*!
    \fn QScreen::uncache(uchar *)

    \internal

    This function is called on pixmap destruction to remove them from
    graphics card memory.
*/

/*!
    \fn QScreen::screenSize() const

    Returns the size of the screen in bytes.

    The screen size is always located at the beginning of framebuffer
    memory, i.e. it can also be retrieved using the base() function.

    \sa base(), region()
*/

/*!
    \fn QScreen::totalSize() const

    Returns the size of the available graphics card memory (including
    the screen) in bytes.

    \sa onCard()
*/

// Unaccelerated screen/driver setup. Can be overridden by accelerated
// drivers

/*!
    \fn QScreen::QScreen(int displayId)

    Constructs a new screen driver.

    The \a displayId identifies the \l {Qtopia Core} server to connect
    to.
*/

/*!
    \fn QScreen::clut()

    Returns a pointer to the screen's color lookup table (i.e. its
    color palette).

    Note that this function only apply in paletted modes like 8-bit,
    i.e. in modes where only the palette indexes (and not the actual
    color values) are stored in memory.

    \sa alloc(), depth(), numCols()
*/

/*!
    \fn int QScreen::numCols()

    Returns the number of entries in the screen's color lookup table
    (i.e. its color palette). A pointer to the color table can be
    retrieved using the clut() function.

    \sa clut(), alloc()
*/

QScreen::QScreen(int display_id)
    : d_ptr(new QScreenPrivate(this))
{
    w = 0;
    lstep = 0;
    h = 0;
    d = 1;
    pixeltype = NormalPixel;
    grayscale = false;

    dw = 0;
    dh = 0;

    size = 0;
    mapsize = 0;

    data = 0;
    displayId = display_id;
    entries = 0;
    entryp = 0;
    lowest = 0;
    clearCacheFunc = 0;
    grayscale = false;
    screencols = 0;
    physWidth = 0;
    physHeight = 0;
}

/*!
    Destroys this screen driver.
*/

QScreen::~QScreen()
{
    delete d_ptr;
}

/*!
    This function is called by the \l {Qtopia Core} server before it
    calls the disconnect() function when exiting.

    Note that the default implementation only hides the mouse cursor;
    reimplement this function to do the necessary graphics card
    specific cleanup.

    \sa initDevice(), disconnect()
*/

void QScreen::shutdownDevice()
{
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
#endif
}

extern bool qws_accel; //in qapplication_qws.cpp

/*!
    \fn PixelType QScreen::pixelType() const

    Returns the pixel storage format of the screen.
*/

/*!
  Returns the pixel format of the screen, or \c QImage::Format_Invalid
  if the pixel format is not a supported image format.

*/
//#### Must be able to distinguish between 565 and 1555 and between Indexed8 and 8-bit grayscale
QImage::Format QScreen::pixelFormat() const
{
    return d_ptr->pixelFormat;
}

/*!
  Sets the screen's pixel format to \a format.
 */
void QScreen::setPixelFormat(QImage::Format format)
{
    d_ptr->pixelFormat = format;
}


/*!
    \fn int QScreen::alloc(unsigned int red, unsigned int green, unsigned int blue)

    Returns the index in the screen's palette which is the closest
    match to the given RGB value (\a red, \a green, \a blue).

    Note that this function only apply in paletted modes like 8-bit,
    i.e. in modes where only the palette indexes (and not the actual
    color values) are stored in memory.

    \sa clut(), numCols()
*/

int QScreen::alloc(unsigned int r,unsigned int g,unsigned int b)
{
    int ret = 0;
    if (d == 8) {
        if (grayscale)
            return qGray(r, g, b);

        // First we look to see if we match a default color
        const int pos = (r + 25) / 51 * 36 + (g + 25) / 51 * 6 + (b + 25) / 51;
        if (pos < screencols && screenclut[pos] == qRgb(r, g, b)) {
            return pos;
        }

        // search for nearest color
        unsigned int mindiff = 0xffffffff;
        unsigned int diff;
        int dr,dg,db;

        for (int loopc = 0; loopc < screencols; ++loopc) {
            dr = qRed(screenclut[loopc]) - r;
            dg = qGreen(screenclut[loopc]) - g;
            db = qBlue(screenclut[loopc]) - b;
            diff = dr*dr + dg*dg + db*db;

            if (diff < mindiff) {
                ret = loopc;
                if (!diff)
                    break;
                mindiff = diff;
            }
        }
    } else if (d == 4) {
        ret = qGray(r, g, b) >> 4;
    } else if (d == 1) {
        ret = qGray(r, g, b) >= 128;
    } else {
        qFatal("cannot alloc %dbpp color", d);
    }

    return ret;
}

/*!
    Saves the current state of the graphics card.

    For example, hardware screen drivers should reimplement the save()
    and restore() functions to save and restore its registers,
    enabling swintching between virtual consoles.

    Note that the default implementation does nothing.

    \sa restore()
*/

void QScreen::save()
{
}

/*!
    Restores the previously saved state of the graphics card.

    For example, hardware screen drivers should reimplement the save()
    and restore() functions to save and restore its registers,
    enabling swintching between virtual consoles.

    Note that the default implementation does nothing.

    \sa save()
*/

void QScreen::restore()
{
}

void QScreen::blank(bool)
{
}

/*!
    \internal
*/

void QScreen::set(unsigned int, unsigned int, unsigned int, unsigned int)
{
}

/*!
    \fn bool QScreen::supportsDepth(int depth) const

    Returns true if the screen supports the specified color \a depth;
    otherwise returns false.

    \sa clut()
*/

bool QScreen::supportsDepth(int d) const
{
    if (false) {
        //Just to simplify the ifdeffery
#ifdef QT_QWS_DEPTH_1
    } else if(d==1) {
        return true;
#endif
#ifdef QT_QWS_DEPTH_4
    } else if(d==4) {
        return true;
#endif
#ifdef QT_QWS_DEPTH_8
    } else if(d==8) {
        return true;
#endif
#ifdef QT_QWS_DEPTH_16
    } else if(d==16) {
        return true;
#endif
#ifdef QT_QWS_DEPTH_18
    } else if(d==18 || d==19) {
        return true;
#endif
#ifdef QT_QWS_DEPTH_24
    } else if(d==24) {
        return true;
#endif
#ifdef QT_QWS_DEPTH_32
    } else if(d==32) {
        return true;
#endif
    }
    return false;
}

/*!
    \fn bool QScreen::onCard(const unsigned char *buffer) const

    Returns true if the specified \a buffer is within the graphics
    card's memory; otherwise returns false (i.e. if it's in main RAM).

    \sa base(), totalSize()
*/

bool QScreen::onCard(const unsigned char * p) const
{
    long t=(unsigned long)p;
    long bmin=(unsigned long)data;
    if (t < bmin)
        return false;
    if(t >= bmin+mapsize)
        return false;
    return true;
}

/*!
    \fn bool QScreen::onCard(const unsigned char * buffer, ulong& offset) const
    \overload

    If the specified \a buffer is within the graphics card's memory,
    this function stores the offset from the start of graphics card
    memory (in bytes), in the location specified by the \a offset
    parameter.
*/

bool QScreen::onCard(const unsigned char * p, ulong& offset) const
{
    long t=(unsigned long)p;
    long bmin=(unsigned long)data;
    if (t < bmin)
        return false;
    long o = t - bmin;
    if (o >= mapsize)
        return false;
    offset = o;
    return true;
}

/*
#if !defined(QT_NO_QWS_REPEATER)
    { "Repeater", qt_get_screen_repeater, 0 },
#endif
#if defined(QT_QWS_EE)
    { "EE", qt_get_screen_ee, 0 },
#endif

*/

/*
Given a display_id (number of the \l {Qtopia Core} server to connect to)
and a spec (e.g. Mach64:/dev/fb0) return a QScreen-descendant.
The QScreenDriverFactory is queried for a suitable driver and, if found,
asked to create a driver.
People writing new graphics drivers should either hook their own
QScreen-descendant into QScreenDriverFactory or use the QScreenDriverPlugin
to make a dynamically loadable driver.
*/

Q_GUI_EXPORT QScreen* qt_get_screen(int display_id, const char *spec)
{
    QString displaySpec = QString::fromAscii(spec);
    QString driver = displaySpec;
    int colon = displaySpec.indexOf(QLatin1Char(':'));
    if (colon >= 0)
        driver.truncate(colon);
    driver = driver.trimmed();

    bool foundDriver = false;
    QString driverName = driver;

    QStringList driverList;
    if (!driver.isEmpty())
        driverList << driver;
    else
        driverList = QScreenDriverFactory::keys();

    for (int i = 0; i < driverList.size(); ++i) {
        const QString driverName = driverList.at(i);
        qt_screen = QScreenDriverFactory::create(driverName, display_id);
        if (qt_screen) {
            foundDriver = true;
            if (qt_screen->connect(displaySpec)) {
                return qt_screen;
            } else {
                delete qt_screen;
                qt_screen = 0;
            }
        }
    }

    if (driver.isNull())
        qFatal("No suitable driver found");
    else if (foundDriver)
        qFatal("%s: driver cannot connect", driver.toLatin1().constData());
    else
        qFatal("%s: driver not found", driver.toLatin1().constData());

    return 0;
}


/*!
    \fn void QScreen::exposeRegion(QRegion region, int windowIndex)

    This function is called by the \l {Qtopia Core} server whenever a
    screen update is required. \a region is the area on the screen
    that must be updated, and \a windowIndex is the index into
    QWSServer::clientWindows() of the window that required the
    update. QWSWindow::state() gives more information about the cause.

    The default implementation composes the
    affected windows and paints the given \a region on screen by
    calling the blit() and solidFill() functions

    This function can be reimplemented to perform composition in
    hardware, or to perform transition effects.
    For simpler hardware acceleration, or to interface with
    this is typically done by reimplementing the blit() and
    solidFill() functions instead.

    Note that there is no need to call this function explicitly.

    \sa blit(), solidFill(), blank()
*/
void QScreen::exposeRegion(QRegion r, int windowIndex)
{
    r &= region();
    if (r.isEmpty())
        return;

    int changing = windowIndex;
    // when we have just lowered a window, we have to expose all the windows below where the
    // window used to be.
    if (changing && qwsServer->clientWindows().at(changing)->state() == QWSWindow::Lowering)
        changing = 0;
#ifdef QTOPIA_PERFTEST
    static enum { PerfTestUnknown, PerfTestOn, PerfTestOff } perfTestState = PerfTestUnknown;
    if(PerfTestUnknown == perfTestState) {
        if(::getenv("QTOPIA_PERFTEST"))
            perfTestState = PerfTestOn;
        else
            perfTestState = PerfTestOff;
    }
    if(PerfTestOn == perfTestState) {
        QWSWindow *changed = qwsServer->clientWindows().at(changing);
        if(!changed->client()->identity().isEmpty())
            qDebug() << "Performance  :  expose_region  :"
                     << changed->client()->identity()
                     << r.boundingRect() << ": "
                     << qPrintable( QTime::currentTime().toString( "h:mm:ss.zzz" ) );
    }
#endif

    const QRect bounds = r.boundingRect();
    QRegion blendRegion;
    QImage blendBuffer;

#ifndef QT_NO_QWS_CURSOR
    if (qt_screencursor && !qt_screencursor->isAccelerated()) {
        blendRegion = r & qt_screencursor->boundingRect();
    }
#endif
    compose(0, r, blendRegion, blendBuffer, changing);

#ifdef QT_EXPERIMENTAL_REGIONS
    if (!blendBuffer.isNull()) {
        const QPoint offset = blendRegion.boundingRect().topLeft();
#ifndef QT_NO_QWS_CURSOR
        if (qt_screencursor && !qt_screencursor->isAccelerated()) {
            const QRect cursorRect = qt_screencursor->boundingRect();
            if (blendRegion.intersects(cursorRect)) {
                //### can be optimized...
                QPainter p(&blendBuffer);
                p.drawImage(cursorRect.topLeft() - offset,
                            qt_screencursor->image());
            }
        }
#endif // QT_NO_QWS_CURSOR
        blit(blendBuffer, offset, blendRegion);
    }
#else
#ifndef QT_NO_QWS_CURSOR
    if (qt_screencursor && !qt_screencursor->isAccelerated() && !blendBuffer.isNull()) {
        //### can be optimized...
        QPainter p(&blendBuffer);
        p.drawImage(qt_screencursor->boundingRect().topLeft() - blendRegion.boundingRect().topLeft(), qt_screencursor->image());
    }
#endif
    if (!blendBuffer.isNull()) {
        //bltToScreen
        QPoint topLeft = blendRegion.boundingRect().topLeft();
        blit(blendBuffer, topLeft, blendRegion);
    }
#endif // QT_EXPERIMENTAL_REGIONS

    const QVector<QRect> rects = r.rects();
    for (int i = 0; i < rects.size(); ++i)
        setDirty(rects.at(i));
}

/*!
    \fn void QScreen::blit(const QImage &image, const QPoint &topLeft, const QRegion &region)

    Copies the given \a region in the given \a image to the point
    specified by \a topLeft using device coordinates.

    This function is called from the exposeRegion() function; it is
    not intended to be called explicitly.

    Reimplement this function to make use of \l {Adding an Accelerated
    Graphics Driver in Qtopia Core}{accelerated hardware}. Note that
    this function must be reimplemented if the framebuffer format is
    not supported by \l {Qtopia Core} (See the \l {Qtopia Core Display
    Management}{Display Management} documentation for more details).

    \sa exposeRegion(), solidFill(), blank()
*/
void QScreen::blit(const QImage &img, const QPoint &topLeft, const QRegion &reg)
{
    const QRect bound = (region() & QRect(topLeft, img.size())).boundingRect();
    QWSDisplay::grab();
    d_ptr->blit(this, img, topLeft - offset(),
                (reg & bound).translated(-topLeft));
    QWSDisplay::ungrab();
}

/*!
    \internal
*/

void QScreen::blit(QWSWindow *win, const QRegion &clip)
{
    QWSWindowSurface *surface = win->windowSurface();
    if (!surface)
        return;

    const QImage &img = surface->image();
    if (img == QImage())
        return;

    QRegion rgn = clip & win->allocatedRegion();
    surface->lock();
    blit(img, win->requestedRegion().boundingRect().topLeft(), rgn);
    surface->unlock();
}

struct fill_data {
    quint32 color;
    uchar *data;
    int lineStep;
    int x;
    int y;
    int w;
    int h;
};

/*!
    Fills the given \a region of the screen with the specified \a
    color.

    This function is called from the exposeRegion() function; it is
    not intended to be called explicitly.

    Reimplement this function to make use of \l {Adding an Accelerated
    Graphics Driver in Qtopia Core}{accelerated hardware}. Note that
    this function must be reimplemented if the framebuffer format is
    not supported by \l {Qtopia Core} (See the \l {Qtopia Core Display
    Management}{Display Management} documentation for more details).

    \sa exposeRegion(), blit(), blank()
*/
// the base class implementation works in device coordinates, so that transformed drivers can use it
void QScreen::solidFill(const QColor &color, const QRegion &region)
{
    QWSDisplay::grab();
    d_ptr->solidFill(this, color,
                     region.translated(-offset()) & QRect(0, 0, dw, dh));
    QWSDisplay::ungrab();
}

/*!
    \since 4.2

    Creates and returns a new window surface matching the given \a
    key.

    The server application will call this function whenever it needs
    to create a server side representation of a window, e.g. when
    copying the content of memory to the screeen using the screen
    driver.

    Note that this function must be reimplemented when adding an
    accelerated graphics driver. See the \l {Adding an Accelerated
    Graphics Driver in Qtopia Core}{Adding an Accelerated Graphics
    Driver} documentation for details.

    \sa {Qtopia Core Architecture}
*/
QWSWindowSurface* QScreen::createSurface(const QString &key) const
{
#ifndef QT_NO_PAINTONSCREEN
    if (key == QLatin1String("OnScreen"))
        return new QWSOnScreenSurface;
    else
#endif
    if (key == QLatin1String("mem"))
        return new QWSLocalMemSurface;
#ifndef QT_NO_QWS_MULTIPROCESS
    else if (key == QLatin1String("shm"))
        return new QWSSharedMemSurface;
#endif
#ifndef QT_NO_PAINT_DEBUG
    else if (key == QLatin1String("Yellow"))
        return new QWSYellowSurface;
#endif
#ifndef QT_NO_DIRECTPAINTER
    else if (key == QLatin1String("DirectPainter"))
        return new QWSDirectPainterSurface;
#endif

    return 0;
}

#ifndef QT_NO_PAINTONSCREEN
static inline bool isWidgetPaintOnScreen(const QWidget *w)
{
    static int doOnScreen = -1;
    if (doOnScreen == -1)
        doOnScreen = qgetenv("QT_ONSCREEN_PAINT").toInt();

    if (doOnScreen > 0)
        return true;

    Q_ASSERT(w->isWindow());
    return w->testAttribute(Qt::WA_PaintOnScreen);
}
#endif

/*!
    \overload

    Creates and returns a new window surface for the given \a widget.
*/
QWSWindowSurface* QScreen::createSurface(QWidget *widget) const
{
#ifndef QT_NO_PAINTONSCREEN
    if (isWidgetPaintOnScreen(widget) && widget->d_func()->isOpaque() && base())
        return new QWSOnScreenSurface(widget);
    else
#endif
    if (QApplication::type() == QApplication::GuiServer)
        return new QWSLocalMemSurface(widget);
#ifndef QT_NO_QWS_MULTIPROCESS
    else
        return new QWSSharedMemSurface(widget);
#endif

    return 0;
}

void QScreen::compose(int level, const QRegion &exposed, QRegion &blend,
                      QImage &blendbuffer, int changing_level)
{
    QRect exposed_bounds = exposed.boundingRect();
    QWSWindow *win = 0;
    do {
        win = qwsServer->clientWindows().value(level); // null is background
        ++level;
    } while (win && !win->allocatedRegion().boundingRect().intersects(exposed_bounds));

    QWSWindowSurface *surface = (win ? win->windowSurface() : 0);
    bool above_changing = level <= changing_level; // 0 is topmost

    QRegion exposedBelow = exposed;
    bool opaque = true;

    if (win) {
        opaque = win->isOpaque();
        if (opaque) {
            exposedBelow -= win->allocatedRegion();
            if (above_changing || !surface->isBuffered())
                blend -= exposed & win->allocatedRegion();
        } else {
            blend += exposed & win->allocatedRegion();
        }
    }
    if (win && !exposedBelow.isEmpty()) {
        compose(level, exposedBelow, blend, blendbuffer, changing_level);
    } else {
        QSize blendSize = blend.boundingRect().size();
        if (!blendSize.isNull()) {
            blendbuffer = QImage(blendSize,
                                 depth() <= 16 ? QImage::Format_RGB16 : QImage::Format_ARGB32_Premultiplied);
        }
    }

    const QRegion blitRegion = exposed - blend;
    if (!win)
        paintBackground(blitRegion);
    else if (!above_changing && surface->isBuffered())
        blit(win, blitRegion);

    QRegion blendRegion = exposed & blend;

    if (win)
        blendRegion &= win->allocatedRegion();
    if (!blendRegion.isEmpty()) {

        QPoint off = blend.boundingRect().topLeft();

        QRasterBuffer rb;
        rb.prepare(&blendbuffer);
        QSpanData spanData;
        spanData.init(&rb);
        if (!win) {
            spanData.setup(qwsServer->backgroundBrush(), 256);
            spanData.dx = off.x();
            spanData.dy = off.y();
        } else if (!surface->isBuffered()) {
                return;
        } else {
            const QImage &img = surface->image();
            QPoint winoff = off - win->requestedRegion().boundingRect().topLeft();
            // convert win->opacity() from scale [0..255] to [0..256]
            int const_alpha = win->opacity();
            const_alpha += (const_alpha >> 7);
            spanData.type = QSpanData::Texture;
            spanData.initTexture(&img, const_alpha);
            spanData.dx = winoff.x();
            spanData.dy = winoff.y();
        }
        if (!spanData.blend)
            return;

        if (surface)
            surface->lock();
        const QVector<QRect> rects = blendRegion.rects();
        const int nspans = 256;
        QT_FT_Span spans[nspans];
        for (int i = 0; i < rects.size(); ++i) {
            int y = rects.at(i).y() - off.y();
            int ye = y + rects.at(i).height();
            int x = rects.at(i).x() - off.x();
            int len = rects.at(i).width();
            while (y < ye) {
                int n = qMin(nspans, ye - y);
                int i = 0;
                while (i < n) {
                    spans[i].x = x;
                    spans[i].len = len;
                    spans[i].y = y + i;
                    spans[i].coverage = 255;
                    ++i;
                }
                spanData.blend(n, spans, &spanData);
                y += n;
            }
        }
        if (surface)
            surface->unlock();
    }
}

void QScreen::paintBackground(const QRegion &r)
{
    const QBrush &bg = qwsServer->backgroundBrush();
    Qt::BrushStyle bs = bg.style();
    if (bs == Qt::NoBrush || r.isEmpty())
        return;

    if (bs == Qt::SolidPattern) {
        solidFill(bg.color(), r);
    } else {
        const QRect br = r.boundingRect();
        QImage img(br.size(), d_ptr->preferredImageFormat());
        QPoint off = br.topLeft();
        QRasterBuffer rb;
        rb.prepare(&img);
        QSpanData spanData;
        spanData.init(&rb);
        spanData.setup(bg, 256);
        spanData.dx = off.x();
        spanData.dy = off.y();
        Q_ASSERT(spanData.blend);

        const QVector<QRect> rects = r.rects();
        const int nspans = 256;
        QT_FT_Span spans[nspans];
        for (int i = 0; i < rects.size(); ++i) {
            int y = rects.at(i).y() - off.y();
            int ye = y + rects.at(i).height();
            int x = rects.at(i).x() - off.x();
            int len = rects.at(i).width();
            while (y < ye) {
                int n = qMin(nspans, ye - y);
                int i = 0;
                while (i < n) {
                    spans[i].x = x;
                    spans[i].len = len;
                    spans[i].y = y + i;
                    spans[i].coverage = 255;
                    ++i;
                }
                spanData.blend(n, spans, &spanData);
                y += n;
            }
        }
        blit(img, br.topLeft(), r);
    }
}

/*!
    \fn virtual int QScreen::sharedRamSize(void *)

    \internal
*/

/*!
    \fn QScreen::setDirty(const QRect& rectangle)

    Marks the given \a rectangle as dirty.

    Note that the default implementation does nothing; reimplement
    this function to indicate that the given \a rectangle has been
    altered.
*/

void QScreen::setDirty(const QRect&)
{
}

/*!
    \fn QScreen::isTransformed() const

    Returns true if the screen is transformed (for instance, rotated
    90 degrees); otherwise returns false.

    \sa transformOrientation(), isInterlaced()
*/

bool QScreen::isTransformed() const
{
    return false;
}

/*!
    \fn QScreen::isInterlaced() const

    Returns true if the display is interlaced (i.e. is displaying
    images progressively like a television screen); otherwise returns
    false.

    If the display is interlaced, the drawing is altered to look
    better.

    \sa isTransformed(), linestep()
*/

bool QScreen::isInterlaced() const
{
    return false;//qws_screen_is_interlaced;;
}

/*!
    \fn QScreen::mapToDevice(const QSize &size) const

    Maps the given \a size from the coordinate space used by the
    application to the framebuffer coordinate system. Note that the
    default implementation simply returns the given \a size as it is.

    Reimplement this function to use the given device's coordinate
    system when mapping.

    \sa mapFromDevice()
*/

QSize QScreen::mapToDevice(const QSize &s) const
{
    return s;
}

/*!
    \fn QScreen::mapFromDevice(const QSize &size) const

    Maps the given \a size from the framebuffer coordinate system to
    the coordinate space used by the application. Note that the
    default implementation simply returns the given \a size as it is.

    Reimplement this function to use the given device's coordinate
    system when mapping.

    \sa mapToDevice()
*/

QSize QScreen::mapFromDevice(const QSize &s) const
{
    return s;
}

/*!
    \fn QScreen::mapToDevice(const QPoint &point, const QSize &screenSize) const
    \overload

    Maps the given \a point from the coordinate space used by the
    application to the framebuffer coordinate system, passing the
    device's \a screenSize as argument. Note that the default
    implementation returns the given \a point as it is.
*/

QPoint QScreen::mapToDevice(const QPoint &p, const QSize &) const
{
    return p;
}

/*!
    \fn QScreen::mapFromDevice(const QPoint &point, const QSize &screenSize) const
    \overload

    Maps the given \a point from the framebuffer coordinate system to
    the coordinate space used by the application, passing the device's
    \a screenSize as argument. Note that the default implementation
    simply returns the given \a point as it is.
*/

QPoint QScreen::mapFromDevice(const QPoint &p, const QSize &) const
{
    return p;
}

/*!
    \fn QScreen::mapToDevice(const QRect &rectangle, const QSize &screenSize) const
    \overload

    Maps the given \a rectangle from the coordinate space used by the
    application to the framebuffer coordinate system, passing the
    device's \a screenSize as argument. Note that the default
    implementation returns the given \a rectangle as it is.
*/

QRect QScreen::mapToDevice(const QRect &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::mapFromDevice(const QRect &rectangle, const QSize &screenSize) const
    \overload

    Maps the given \a rectangle from the framebuffer coordinate system to
    the coordinate space used by the application, passing the device's
    \a screenSize as argument. Note that the default implementation
    simply returns the given \a rectangle as it is.
*/

QRect QScreen::mapFromDevice(const QRect &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::mapToDevice(const QImage &image) const
    \overload

    Maps the given \a image from the coordinate space used by the
    application to the framebuffer coordinate system. Note that the
    default implementation returns the given \a image as it is.
*/

QImage QScreen::mapToDevice(const QImage &i) const
{
    return i;
}

/*!
    \fn QScreen::mapFromDevice(const QImage &image) const
    \overload

    Maps the given \a image from the framebuffer coordinate system to
    the coordinate space used by the application. Note that the
    default implementation simply returns the given \a image as it is.
*/

QImage QScreen::mapFromDevice(const QImage &i) const
{
    return i;
}

/*!
    \fn QScreen::mapToDevice(const QRegion &region, const QSize &screenSize) const
    \overload

    Maps the given \a region from the coordinate space used by the
    application to the framebuffer coordinate system, passing the
    device's \a screenSize as argument. Note that the default
    implementation returns the given \a region as it is.
*/

QRegion QScreen::mapToDevice(const QRegion &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::mapFromDevice(const QRegion &region, const QSize &screenSize) const
    \overload

    Maps the given \a region from the framebuffer coordinate system to
    the coordinate space used by the application, passing the device's
    \a screenSize as argument. Note that the default implementation
    simply returns the given \a region as it is.
*/

QRegion QScreen::mapFromDevice(const QRegion &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::transformOrientation() const

    Returns the current rotation as an integer value.

    Note that the default implementation returns 0; reimplement this
    function to override this value.

    \sa isTransformed()
*/

int QScreen::transformOrientation() const
{
    return 0;
}

int QScreen::pixmapDepth() const
{
    return depth();
}

/*!
    \internal
*/
int QScreen::memoryNeeded(const QString&)
{
    return 0;
}

/*!
    \internal
*/
void QScreen::haltUpdates()
{
}

/*!
    \internal
*/
void QScreen::resumeUpdates()
{
}

/*!
    \fn QRegion QScreen::region() const
    \since 4.2

    Returns the region covered by this screen driver.

    \sa base(), screenSize()
*/

/*!
    \internal
*/
void QScreen::setOffset(const QPoint &p)
{
    d_ptr->offset = p;
}

/*!
    \since 4.2

    Returns the logical offset of the screen, i.e., the offset between
    (0,0) in screen coordinates and the application coordinate system.
*/
QPoint QScreen::offset() const
{
    return d_ptr->offset;
}

#if Q_BYTE_ORDER == Q_BIG_ENDIAN
void QScreen::setFrameBufferLittleEndian(bool littleEndian)
{
    d_ptr->fb_is_littleEndian = littleEndian;
}
#endif

/*!
    \fn int QScreen::subScreenIndexAt(const QPoint &position) const
    \since 4.2

    Returns the index of the subscreen at the given \a position;
    returns -1 if no screen is found.

    The index identifies the subscreen in the list of pointers
    returned by the subScreens() function.

    \sa instance(), subScreens()
*/
int QScreen::subScreenIndexAt(const QPoint &p) const
{
    const QList<QScreen*> screens = subScreens();
    const int n = screens.count();
    for (int i = 0; i < n; ++i) {
        if (screens.at(i)->region().contains(p))
            return i;
    }

    return -1;
}

#if 0
#ifdef QT_LOADABLE_MODULES

// ### needs update after driver init changes

static QScreen * qt_dodriver(char * driver,char * a,unsigned char * b)

{
    char buf[200];
    strcpy(buf,"/etc/qws/drivers/");
    qstrcpy(buf+17,driver);
    qDebug("Attempting driver %s",driver);

    void * handle;
    handle=dlopen(buf,RTLD_LAZY);
    if(handle==0) {
        qFatal("Module load error");
    }
    QScreen *(*qt_get_screen_func)(char *,unsigned char *);
    qt_get_screen_func=dlsym(handle,"qt_get_screen");
    if(qt_get_screen_func==0) {
        qFatal("Couldn't get symbol");
    }
    QScreen * ret=qt_get_screen_func(a,b);
    return ret;
}

static QScreen * qt_do_entry(char * entry)
{
    unsigned char config[256];

    FILE * f=fopen(entry,"r");
    if(!f) {
        return 0;
    }

    int r=fread(config,256,1,f);
    if(r<1)
        return 0;

    fclose(f);

    unsigned short vendorid=*((unsigned short int *)config);
    unsigned short deviceid=*(((unsigned short int *)config)+1);
    if(config[0xb]!=3)
        return 0;

    if(vendorid==0x1002) {
        if(deviceid==0x4c4d) {
            qDebug("Compaq Armada/IBM Thinkpad's Mach64 card");
            return qt_dodriver("mach64.so",entry,config);
        } else if(deviceid==0x4742) {
            qDebug("Desktop Rage Pro Mach64 card");
            return qt_dodriver("mach64.so",entry,config);
        } else {
            qDebug("Unrecognised ATI card id %x",deviceid);
            return 0;
        }
    } else {
        qDebug("Unrecognised vendor");
    }
    return 0;
}

extern bool qws_accel;

/// ** NOT SUPPPORTED **

QScreen * qt_probe_bus()
{
    if(!qws_accel) {
        return qt_dodriver("unaccel.so",0,0);
    }

    DIR * dirptr=opendir("/proc/bus/pci");
    if(!dirptr)
        return qt_dodriver("unaccel.so",0,0);
    DIR * dirptr2;
    dirent * cards;

    dirent * busses=readdir(dirptr);

    while(busses) {
        if(busses->d_name[0]!='.') {
            char buf[100];
            strcpy(buf,"/proc/bus/pci/");
            qstrcpy(buf+14,busses->d_name);
            int p=strlen(buf);
            dirptr2=opendir(buf);
            if(dirptr2) {
                cards=readdir(dirptr2);
                while(cards) {
                    if(cards->d_name[0]!='.') {
                        buf[p]='/';
                        qstrcpy(buf+p+1,cards->d_name);
                        QScreen * ret=qt_do_entry(buf);
                        if(ret)
                            return ret;
                    }
                    cards=readdir(dirptr2);
                }
                closedir(dirptr2);
            }
        }
        busses=readdir(dirptr);
    }
    closedir(dirptr);

    return qt_dodriver("unaccel.so",0,0);
}

#else

char *qt_qws_hardcoded_slot = "/proc/bus/pci/01/00.0";

const unsigned char* qt_probe_bus()
{
    const char * slot;
    slot=::getenv("QWS_CARD_SLOT");
    if(!slot)
        slot=qt_qws_hardcoded_slot;
    if (slot) {
        static unsigned char config[256];
        FILE * f=fopen(slot,"r");
        if(!f) {
            qDebug("Open failure for %s",slot);
            slot=0;
        } else {
            int r=fread((char*)config,256,1,f);
            fclose(f);
            if(r<1) {
                qDebug("Read failure");
                return 0;
            } else {
                return config;
            }
        }
    }
    return 0;
}

#endif
#endif // 0
