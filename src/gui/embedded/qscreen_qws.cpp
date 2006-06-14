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
#include "private/qwidget_qws_p.h"
#include "qcolor.h"
#include "qpixmap.h"
#include "qwsdisplay_qws.h"
#include "private/qwidget_qws_p.h"
#include <private/qdrawhelper_p.h>
#include <private/qpaintengine_raster_p.h>
#include <private/qpainter_p.h>
#include <qdebug.h>

static const bool simple_8bpp_alloc = true; //### 8bpp support not done
static const int max_lock_time = -1; // infinite

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

    \brief The QScreenCursor class manages the onscreen mouse cursor.

    Note that this class is non-portable, and that it is only
    available in \l {Qtopia Core}.

    QScreenCursor provides an implementation of a software mouse
    cursor and can be subclassed by hardware drivers which support a
    hardware mouse cursor. There may only be one QScreenCursor at a
    time; it is constructed by the QScreen class or one of its
    descendants, i.e. QScreen act as a factory for the screen cursor
    and the QScreenCursor class should never be instantiated
    explicitly.

    The QWSServer class provides some means of controlling the
    cursor's appearance with the QWSServer::isCursorVisible() and
    QWSServer::setCursorVisible() functions.

    \sa QScreen, QWSServer
*/

/*!
    \fn static QScreenCursor* QScreenCursor::instance()

    Returns a pointer to the application's QScreenCursor instance.
*/

extern bool qws_sw_cursor;

/*!
    \internal

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
    \internal

    Destroys a screen cursor, deleting its cursor image and
    under-cursor storage
*/
QScreenCursor::~QScreenCursor()
{
}

/*!
    \internal

    Hide the mouse cursor from the screen.
*/
void QScreenCursor::hide()
{
    if (enable) {
        enable = false;
        qt_screen-> exposeRegion(boundingRect(), 0);
    }
}

/*!
    \internal

    Show the mouse cursor again after it has been hidden. Note that hides
    and shows are not nested; show() should always re-display the cursor no
    matter how many hide()s preceded it.
*/
void QScreenCursor::show()
{
    if (!enable) {
        enable = true;
        qt_screen-> exposeRegion(boundingRect(), 0);
    }
}

/*!
    \internal

    Sets a mouse cursor to \a image. The QImage is 32 bit, with an alpha
    channel containing either only 255 or 0 (that is, display the pixel
    or not) or a full alpha channel, depending on what
    supportsAlphaCursor() returns. \a hotx and \a hoty are the point within
    the QImage where mouse events actually 'come from'.
*/
void QScreenCursor::set(const QImage &image, int hotx, int hoty)
{
    QRect r = boundingRect();

    hotspot = QPoint(hotx, hoty);
    cursor = image;
    size = image.size();

    if (enable) {
        qt_screen-> exposeRegion(r | boundingRect(), 0);
    }

}

/*!
    \internal

    Move the mouse cursor to point (\a x, \a y) on the screen. This should be done
    in such a way that the hotspot of the cursor is at (x,y) - e.g. if the
    hotspot is at 5,5 within the image then the top left of the image should
    be at x-5,y-5
*/
void QScreenCursor::move(int x, int y)
{
    QRegion r = boundingRect();
    pos = QPoint(x,y);
    if (enable) {
        qt_screen-> exposeRegion(r | boundingRect(), 0);
    }
}


void QScreenCursor::initSoftwareCursor()
{
    qt_screencursor=new QScreenCursor();
}


#endif // QT_NO_QWS_CURSOR


/*!
    \internal
    \fn QRect QScreenCursor::boundingRect () const
*/

/*!
    \internal
    \fn bool QScreenCursor::enabled ()
*/

/*!
    \internal
    \fn QImage QScreenCursor::image () const
*/

/*!
    \internal
    \fn void QScreenCursor::initSoftwareCursor ()
*/

/*!
    \internal
    \fn bool QScreenCursor::isAccelerated () const
*/

/*!
    \internal
    \fn bool QScreenCursor::isVisible () const
*/

/*!
    \internal
    \fn bool QScreenCursor::supportsAlphaCursor () const
*/


















/*!
    \class QScreen
    \ingroup qws

    \brief The QScreen class is a base class for implementing screen
    drivers in Qtopia Core.

    Note that this class is only available in Qtopia Core. Custom
    screen drivers derived from QScreen can be added by subclassing
    the QScreenDriverPlugin class, using the QScreenDriverFactory
    class to dynamically load the driver into the application, but
    there should only be one screen object per application.

    The QScreen class provides functions enabling framebuffer and
    palette management and drawing on screen. The class also provides
    information about several screen properties, e.g its
    metrics. Finally, the QScreen class and its descendants act as
    factories for the screen cursor, creating the cursor whenever
    required.

    \tableofcontents

    \section1 Framebuffer Management

    When a \l {Qtopia Core} application starts, the connect() function
    will be called to map in the framebuffer and the accelerated
    drivers that the graphics card control registers, and when
    initializing the framebuffer, the initDevice() function will be
    called. The latter function can be reimplemented to set up the
    graphics card (note that the default implementation does nothing).

    Likewise, just before a \l {Qtopia Core} application exits, the
    disconnect() function will be called. This function should be
    reimplemented to unmap the framebuffer. The shutdownDevice()
    function is also called on shutdown, and can be reimplemented to
    support graphics card specific cleanup.

    The save() function allows subclasses of QScreen to save the state
    of the graphics card. Hardware QScreen descendants should save the
    state of the registers here to enable switching between virtual
    consoles. The corresponding restore() function enables restoration
    of a previously saved state of the graphics card. Note that the
    default implementations of these two functions do nothing.

    QScreen also provides the base() function returning a pointer to
    the beginning of the framebuffer. Use the onCard() function to
    determine whether the framebuffer is within the graphics card's
    memory. The totalSize() function returns the size of the available
    graphics card memory, including the screen.

    \section1 Palette Management

    For palette management, the QScreen class provides the clut()
    function to retrieve the screen's color lookup table (i.e. its
    color palette), and the numCols() function to determine the number
    of entries in this table. The alloc() function can be used to
    retrieve the index in the screen's palette which is the closest
    match to a given RGB value, and the supportsDepth() function to
    determine if the screen supports a given color depth.

    \section1 Drawing on Screen

    When deriving from QScreen, either the blit() or the
    exposeRegion() function must be reimplemented to enable drawing on
    screen.

    The blit() function copies a given region in a given image to a
    specified point using device coordinates, and should be
    reimplemented to make use of accelerated hardware. Note that
    reimplementing blit() requires that the solidFill() function is
    reimplemented as well. The solidFill() function allows subclasses
    of QScreen to fill the given region of the screen with the
    specified color. The exposeRegion() function paints a given region
    on screen. Note that normally there is no need to call either of
    these functions explicitly.

    Reimplement the blank() function to control the display of
    contents on the screen. The setDirty() function can be
    reimplemented to indicate that a given rectangle of the screen has
    been altered. Note that the default implementations of these two
    latter functions do nothing.

    QScreen also provides the mapFromDevice() and mapToDevice()
    function to map an object from the framebuffer coordinate system
    to the coordinate space used by the application, and vice versa.

    See the QDirectPainter and QDecoration class documentation for
    more information about drawing on screen.

    \section1 Screen Properties

    QScreen provides information about several screen properties: The
    size of the screen can be retrieved using the screenSize()
    function. Alternatively, its width and height can be retrieved
    using the width() and height() functions respectively. QScreen
    also provides the deviceWidth() and deviceHeight() functions which
    returns the full size of the device. Note that these metrics can
    differ from the ones used if the display is centered within the
    framebuffer.

    The setMode() function allows subclasses of QScreen to set the
    framebuffer to a new resolution (width and height) and bit depth.
    The current depth of the framebuffer can be retrieved using the
    depth() function.

    Use the pixmapDepth() function to obtain the preferred depth for
    pixmaps. In addition, QScreen provides the pixmapOffsetAlignment()
    function returning the value to which the start address of pixmaps
    held in the graphics card's memory, should be aligned. The
    pixmapLinestepAlignment() returns the value to which the \e
    {individual scanlines} of pixmaps should be aligned.

    The isInterlaced() function tells whether the screen is displaying
    images progressively, and the isTransformed() function whether it
    is rotated. The transformOrientation() function can be
    reimplemented to return the current rotation.

    The linestep() function returns the length of each scanline of the
    framebuffer.

    The pixelType() returns the screen's pixel storage format as
    described by the PixelType enum, the opType() function returns the
    screen's operation type, and the lastOp() function returns the
    screens last operation.

    \sa {Running Applications}, {Qtopia Core}
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
*/

/*!
    \fn QScreen::initDevice()

    This function is called by the \l {Qtopia Core} server when
    initializing the framebuffer.

    Reimplement this function to make accelerated drivers set up the
    graphics card. The default implementation does nothing.

    \sa {Adding an Accelerated Graphics Driver}
*/

/*!
    \fn QScreen::connect(const QString &displaySpec)

    This function is called by every \l {Qtopia Core} application on
    startup, and must be reimplemented to map in the framebuffer and
    the accelerated drivers that the graphics card control registers.

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

    \sa disconnect(), {Running Applications}
*/

/*!
    \fn QScreen::disconnect()

    This function is called by every \l {Qtopia Core} application just
    before exiting, and should be reimplemented to unmap the
    framebuffer.

    \sa connect()
*/

/*!
    \fn QScreen::setMode(int width, int height, int depth)

    This virtual function allows subclasses of QScreen to set the
    framebuffer to a new resolution (\a width and \a height) and bit
    \a depth.

    After doing this any currently-existing paint engines will be
    invalid and the screen should be completely redrawn. In a
    multiple-process situation, all other applications must be
    notified to set the same mode and redraw.

    Note that the default implementation does nothing.
*/

/*!
    \fn QScreen::blank(bool on)

    Avoids displaying any contents on the screen if \a on is true;
    otherwise the contents is shown.

    The default implementation does nothing.
*/

/*!
    \fn int QScreen::pixmapOffsetAlignment()

    Returns the value (in bits) to which the start address of pixmaps
    held in the graphics card's memory, should be aligned.

    The default implementation returns 64. Reimplement this function
    to override the return value when implementing an accelerated
    driver.

    \sa pixmapLinestepAlignment(), {Adding an Accelerated Graphics Driver}
*/

/*!
    \fn int QScreen::pixmapLinestepAlignment()

    Returns the value (in bits) to which individual scanlines of
    pixmaps held in the graphics card's memory, should be aligned.

    The default implementation returns 64. Reimplement this function
    to override the return value when implementing an accelerated
    driver.

    \sa pixmapOffsetAlignment(), {Adding an Accelerated Graphics Driver}
*/

/*!
    \fn QScreen::width() const

    Returns the width of the framebuffer, in pixels.

    \sa deviceWidth(), height()
*/

/*!
    \fn int QScreen::height() const

    Returns the height of the framebuffer, in pixels.

    \sa deviceHeight(), width()
*/

/*!
    \fn QScreen::depth() const

    Returns the depth of the framebuffer, in bits per pixel.

    Note that the returned depth is the number of bits each pixel
    takes up rather than the number of significant bits, so 24bpp and
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

    Returns the length of each scanline of the framebuffer, in bytes.

    \sa QDirectPainter::linestep()
*/

/*!
    \fn QScreen::deviceWidth() const

    Returns the full width of the framebuffer device.

    Note that the returned width can differ from the width which \l
    {Qtopia Core} will actually use, that is if the display is
    centered within the framebuffer.

    \sa deviceHeight(), width()
*/

/*!
    \fn QScreen::deviceHeight() const

    Returns the full height of the framebuffer device.

    Note that the returned height can differ from the height which \l
    {Qtopia Core} will actually use, that is if the display is
    centered within the framebuffer.

    \sa deviceWidth(), height()
*/

/*!
    \fn uchar *QScreen::base() const

    Returns a pointer to the beginning of the framebuffer.

    \sa onCard()
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

    Returns the size of the screen, in bytes.

    The screen size is always located at the beginning of framebuffer
    memory, i.e. it can also be retrieved using the base() function.

    \sa base()
*/

/*!
    \fn QScreen::totalSize() const

    Returns the size of the available graphics card memory, including
    the screen, in bytes.

    Offscreen memory is only used by the accelerated drivers.

    \sa onCard()
*/

// Unaccelerated screen/driver setup. Can be overridden by accelerated
// drivers

/*!
  \fn QScreen::QScreen(int displayId)

  Constructs a QScreen object. The \a displayId identifies the the \l
  {Qtopia Core} server to connect to.
*/

/*!
    \fn QScreen::clut()

    Returns the screen's color lookup table (i.e. its color
    palette).

    Note that this function only apply in paletted modes, i.e. in
    modes where only palette indexes (and not actual color values) are
    stored in memory (e.g. 8-bit mode).

    \sa alloc(), depth(), numCols()
*/

/*!
    \fn int QScreen::numCols()

    Returns the number of entries in the screen's color lookup table
    (i.e. its color palette). The color table can be retrieved using
    the clut() function.

    \sa clut()
*/

QScreen::QScreen(int display_id)
{
    pixeltype=NormalPixel;
    data = 0;
    displayId = display_id;
    initted=false;
    entryp=0;
    clearCacheFunc = 0;
    grayscale = false;
    screencols = 0;
}

/*!
  Destroys this QScreen object.
*/

QScreen::~QScreen()
{
}

/*!
    This virtual function is called by the \l {Qtopia Core} server on
    shutdown, and allows subclasses of QScreen to support graphics
    card specific cleanup.

    The default implementation simply hides the mouse cursor.
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

    Returns  the pixel storage format of the screen.

    \sa PixelType
*/

/*!
    \fn int QScreen::alloc(unsigned int red, unsigned int green, unsigned int blue)

    Returns the index in the screen's palette which is the closest
    match to the given RGB value (\a red, \a green, \a blue).

    Note that this function only apply in paletted modes, i.e. in
    modes where only palette indexes (and not actual color values) are
    stored in memory (e.g. 8-bit mode).

    \sa clut()
*/

int QScreen::alloc(unsigned int r,unsigned int g,unsigned int b)
{
    int ret = 0;
    if (d == 8) {
        if (grayscale)
            return qGray(r, g, b);
        // First we look to see if we match a default color
        QRgb myrgb=qRgb(r,g,b);
        int pos= (r + 25) / 51 * 36 + (g + 25) / 51 * 6 + (b + 25) / 51;
        if (simple_8bpp_alloc || screenclut[pos] == myrgb || !initted) {
            return pos;
        }

        // search for nearest color
        unsigned int mindiff = 0xffffffff;
        unsigned int diff;
        int dr,dg,db;

        for (int loopc = 0; loopc < 256; loopc++) {
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
    } else if (d == 1 && simple_8bpp_alloc) {
        ret = qGray(r, g, b) > 128;
    } else {
        qFatal("cannot alloc %dbpp color", d);
    }

    return ret;
}

/*!
    This virtual function allows subclasses of QScreen to save the
    state of the graphics card, and be able to restored it later.

    Hardware QScreen descendants should save the state of the
    registers here, if necessary, to enable switching between virtual
    consoles (for example to and from X).

    Note that the default implementation does nothing.

    \sa restore()
*/

void QScreen::save()
{
}

/*!
    This virtual function allows subclasses of QScreen to restore a
    previously saved state of the graphics card.

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

    Possible values are 1,4,8,16 and 32.

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
    this function stores the offset (in bytes) from the start of
    graphics card memory, in the location specified by the \a offset
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

QScreen *qt_get_screen(int display_id, const char *spec)
{
    QString displaySpec(spec);
    QString driver = displaySpec;
    int colon = displaySpec.indexOf(':');
    if (colon >= 0)
        driver.truncate(colon);

    bool foundDriver = false;
    QString driverName = driver;

    QStringList driverList = QScreenDriverFactory::keys();
    QStringList::Iterator it;
    for (it = driverList.begin(); it != driverList.end(); ++it) {
        if (driver.isEmpty() || QString(*it) == driver) {
            driverName = *it;
            qt_screen = QScreenDriverFactory::create(driverName, display_id);
            if (qt_screen) {
                foundDriver = true;
                if (qt_screen->connect(spec)) {
                    return qt_screen;
                } else {
                    delete qt_screen;
                    qt_screen = 0;
                }
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

    Paints the given \a region on screen. The \a windowIndex parameter
    refer to the affected windows.

    Note that there is no need to call this function explicitly, but
    it must be reimplemented in derived classes. It can also be
    reimplemented to make use of accelerated hardware, but this is
    typically done by reimplementing the blit() function instead.

    \sa blit()
*/
void QScreen::exposeRegion(QRegion r, int changing)
{
    if (r.isEmpty())
        return;

    r &= QRect(0, 0, w, h);
    QRect bounds = r.boundingRect();
    QRegion blendRegion;
    QImage blendBuffer;

#ifndef QT_NO_QWS_CURSOR
    if (qt_screencursor && !qt_screencursor->isAccelerated()) {
        blendRegion = r & qt_screencursor->boundingRect();
    }
#endif
    compose(0, r, blendRegion, blendBuffer, changing);
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
    qt_screen->setDirty(r.boundingRect());
}

struct blit_data {
    const QImage *img;
    uchar *data;
    int lineStep;
    int sx;
    int sy;
    int w;
    int h;
    int dx;
    int dy;
};

typedef void (*blitFunc)(const blit_data *);

#ifdef QT_QWS_DEPTH_32
static void blit_32_to_32(const blit_data *data)
{
    const int sbpl = data->img->bytesPerLine() / 4;
    const int dbpl = data->lineStep / 4;

    const uint *src = (const uint *)data->img->bits();
    src += data->sy * sbpl + data->sx;
    uint *dest = (uint *)data->data;
    dest += data->dy * dbpl + data->dx;

    int h = data->h;
    int bytes = data->w * 4;
    while (h) {
        memcpy(dest, src, bytes);
        src += sbpl;
        dest += dbpl;
        --h;
    }
}
#endif // QT_QWS_DEPTH_32

#ifdef QT_QWS_DEPTH_16
static void blit_32_to_16(const blit_data *data)
{
    const int sbpl = data->img->bytesPerLine() / 4;
    const int dbpl = data->lineStep / 2;

    const uint *src = (const uint *)data->img->bits();
    src += data->sy * sbpl + data->sx;
    ushort *dest = (ushort *)data->data;
    dest += data->dy * dbpl + data->dx;

    int h = data->h;
    while (h) {
        for (int i = 0; i < data->w; ++i)
            dest[i] = qt_convRgbTo16(src[i]);
        src += sbpl;
        dest += dbpl;
        --h;
    }
}

static void blit_16_to_16(const blit_data *data)
{
    const int sbpl = data->img->bytesPerLine();
    const int dbpl = data->lineStep;

    const uchar *src = (const uchar *)data->img->bits();
    src += data->sy * sbpl + data->sx*2;
    uchar *dest = (uchar *)data->data;
    dest += data->dy * dbpl + data->dx*2;

    int h = data->h;
    while (h) {
#ifdef QT_USE_MEMCPY_DUFF
        QT_MEMCPY_USHORT(dest, src, data->w);
#else
        memcpy(dest, src, data->w * 2);
#endif
        src += sbpl;
        dest += dbpl;
        --h;
    }
}
#endif // QT_QWS_DEPTH_16

#ifdef QT_QWS_DEPTH_8
static inline uchar qt_32_to_8(uint rgb)
{
    uchar r = (qRed(rgb) + 0x19) / 0x33;
    uchar g = (qGreen(rgb) + 0x19) / 0x33;
    uchar b = (qBlue(rgb) + 0x19) / 0x33;

    return r*6*6 + g*6 + b;
}

static inline uchar qt_16_to_8(ushort pix)
{
    return qt_32_to_8(qt_conv16ToRgb(pix));
}

static void blit_32_to_8(const blit_data *data)
{
    const int sbpl = data->img->bytesPerLine() / 4;
    const int dbpl = data->lineStep;

    const uint *src = (const uint *)data->img->bits();
    src += data->sy * sbpl + data->sx;
    uchar *dest = (uchar *)data->data;
    dest += data->dy * dbpl + data->dx;

    int h = data->h;
    while (h) {
        for (int i = 0; i < data->w; ++i)
            dest[i] = qt_32_to_8(src[i]);
        src += sbpl;
        dest += dbpl;
        --h;
    }
}

static void blit_16_to_8(const blit_data *data)
{
    const int sbpl = data->img->bytesPerLine() / 2;
    const int dbpl = data->lineStep;

    const ushort *src = (const ushort *)data->img->bits();
    src += data->sy * sbpl + data->sx;
    uchar *dest = (uchar *)data->data;
    dest += data->dy * dbpl + data->dx;

    int h = data->h;
    while (h) {
        for (int i = 0; i < data->w; ++i)
            dest[i] = qt_16_to_8(src[i]);
        src += sbpl;
        dest += dbpl;
        --h;
    }
}
#endif // QT_QWS_DEPTH_8

#ifdef QT_QWS_DEPTH_24
static void blit_32_to_24(const blit_data *data)
{
    const int sbpl = data->img->bytesPerLine() / 4;
    const int dbpl = data->lineStep;

    const uint *src = (const uint *)data->img->bits();
    src += data->sy * sbpl + data->sx;
    uchar *dest = (uchar *)data->data;
    dest += data->dy * dbpl + data->dx*3;

    int h = data->h;
    while (h) {
        uchar *d = dest;
        for (int i = 0; i < data->w; ++i) {
            uint s = src[i];
            *d = s & 0xff;
            *(d+1) = (s >> 8) & 0xff;
            *(d+2) = (s >> 16) & 0xff;
            d+=3;
        }
        src += sbpl;
        dest += dbpl;
        --h;
    }
}
#endif // QT_QWS_DEPTH_24

#ifdef QT_QWS_DEPTH_18
static void blit_32_to_18(const blit_data *data)
{
    const int sbpl = data->img->bytesPerLine() / 4;
    const int dbpl = data->lineStep;

    const uint *src = (const uint *)data->img->bits();
    src += data->sy * sbpl + data->sx;
    uchar *dest = (uchar *)data->data;
    dest += data->dy * dbpl + data->dx*3;

    int h = data->h;
    while (h) {
        uchar *d = dest;
        for (int i = 0; i < data->w; ++i) {
            uint s = src[i];
            uchar b = s & 0xff;
            uchar g = (s >> 8) & 0xff;
            uchar r = (s >> 16) & 0xff;
            uint p = (b>>2) | ((g>>2) << 6) | ((r>>2) << 12);
            *d = p & 0xff;
            *(d+1) = (p >> 8) & 0xff;
            *(d+2) = (p >> 16) & 0xff;
            d+=3;
        }
        src += sbpl;
        dest += dbpl;
        --h;
    }
}
#endif // QT_QWS_DEPTH_18

/*!
    \fn void QScreen::blit(const QImage &image, const QPoint &topLeft, const QRegion &region)

    Copies the given \a region in the given \a image to the point
    specified by \a topLeft using device coordinates.

    Reimplement this function to use accelerated hardware. Note that
    reimplementing this function requires that the solidFill()
    function is reimplemented as well.

    \sa exposeRegion(), {Adding an Accelerated Graphics Driver}
*/
void QScreen::blit(const QImage &img, const QPoint &topLeft, const QRegion &region)
{
    QVector<QRect> rects = region.rects();
    QRect bound(0, 0, dw, dh);
    bound &= QRect(topLeft, img.size());
    blit_data data;
    data.img = &img;
    data.data = this->data;
    data.lineStep = lstep;
    blitFunc func = 0;
    switch(d) {
#ifdef QT_QWS_DEPTH_32
    case 32:
        func = blit_32_to_32;
        break;
#endif
#ifdef QT_QWS_DEPTH_24
    case 24:
        func = blit_32_to_24;
        break;
#endif
#ifdef QT_QWS_DEPTH_18
    case 18:
    case 19:
        func = blit_32_to_18;
        break;
#endif
#ifdef QT_QWS_DEPTH_16
    case 16:
        if (img.depth() == 16)
            func = blit_16_to_16;
        else
            func = blit_32_to_16;
        break;
#endif
#ifdef QT_QWS_DEPTH_8
    case 8:
        if (img.depth() == 16)
            func = blit_16_to_8;
        else
            func = blit_32_to_8;
        break;
#endif
    default:
        break;
    }
    if (!func)
        return;

    QWSDisplay::grab();
    for (int i = 0; i < rects.size(); ++i) {
        QRect r = rects.at(i) & bound;
        data.w = r.width();
        data.h = r.height();
        if (data.w <= 0 || data.h <= 0)
            continue;
        data.sx = r.x() - topLeft.x();
        data.sy = r.y() - topLeft.y();
        data.dx = r.x();
        data.dy = r.y();
        func(&data);
    }
    QWSDisplay::ungrab();
}

/*!
    \internal
*/
void QScreen::blit(QWSWindow *win, const QRegion &clip)
{
    QWSBackingStore *bs = win->backingStore();
    if (!bs || bs->isNull())
        return;
    bool locked = bs->lock(max_lock_time);
    if (locked) {
        const QImage &img = bs->image();
        QRegion rgn = clip & win->requestedRegion();
        blit(img, win->requestedRegion().boundingRect().topLeft(), rgn);
        bs->unlock();
    }
}


struct fill_data {
    uint color;
    uchar *data;
    int lineStep;
    int x;
    int y;
    int w;
    int h;
};

typedef void (*fillFunc)(const fill_data *);

#ifdef QT_QWS_DEPTH_32
static void fill_32(const fill_data *data)
{
    const int dbpl = data->lineStep / 4;

    uint *dest = (uint *)data->data;
    dest += data->y * dbpl + data->x;

    int h = data->h;
    while (h) {
        for (int i = 0; i < data->w; ++i)
            dest[i] = data->color;
        dest += dbpl;
        --h;
    }
}
#endif // QT_QWS_DEPTH_32

#ifdef QT_QWS_DEPTH_18
static void fill_18(const fill_data *data)
{
    const int dbpl = data->lineStep;
    uchar b = data->color & 0xff;
    uchar g = (data->color >> 8) & 0xff;
    uchar r = (data->color >> 16) & 0xff;
    uint p = (b>>2) | ((g>>2) << 6) | ((r>>2) << 12);
    uchar b0 = p & 0xff;
    uchar b1 = (p >> 8) & 0xff;
    uchar b2 = (p >> 16) & 0xff;

    uchar *dest = (uchar *)data->data;
    dest += data->y * dbpl + data->x*3;

    int h = data->h;
    while (h) {
        uchar *d = dest;
        for (int i = 0; i < data->w; ++i) {
            *d++ = b0;
            *d++ = b1;
            *d++ = b2;
        }
        dest += dbpl;
        --h;
    }
}
#endif // QT_QWS_DEPTH_18

#ifdef QT_QWS_DEPTH_24
static void fill_24(const fill_data *data)
{
    const int dbpl = data->lineStep;
    uchar r = data->color & 0xff;
    uchar g = (data->color >> 8) & 0xff;
    uchar b = (data->color >> 16) & 0xff;

    uchar *dest = (uchar *)data->data;
    dest += data->y * dbpl + data->x*3;

    int h = data->h;
    while (h) {
        uchar *d = dest;
        for (int i = 0; i < data->w; ++i) {
            *d++ = r;
            *d++ = g;
            *d++ = b;
        }
        dest += dbpl;
        --h;
    }
}
#endif // QT_QWS_DEPTH_24

#ifdef QT_QWS_DEPTH_16
static void fill_16(const fill_data *data)
{
    const int dbpl = data->lineStep / 2;
    ushort color = qt_convRgbTo16(data->color);

    ushort *dest = (ushort *)data->data;
    dest += data->y * dbpl + data->x;

    int h = data->h;
    while (h) {
        for (int i = 0; i < data->w; ++i)
            dest[i] = color;
        dest += dbpl;
        --h;
    }
}
#endif // QT_QWS_DEPTH_16

#ifdef QT_QWS_DEPTH_8
static void fill_8(const fill_data *data)
{
    const int dbpl = data->lineStep;
    uchar color = qt_32_to_8(data->color);

    uchar *dest = (uchar *)data->data;
    dest += data->y * dbpl + data->x;

    int h = data->h;
    while (h) {
        for (int i = 0; i < data->w; ++i)
            dest[i] = color;
        dest += dbpl;
        --h;
    }
}
#endif // QT_QWS_DEPTH_8

/*!
   This virtual function allows subclasses of QScreen to fill the
   given \a region of the screen with the specified \a color. Note
   that this function is not intended to be called explicitly.

   \sa blit()
*/
// the base class implementation works in device coordinates, so that transformed drivers can use it
void QScreen::solidFill(const QColor &color, const QRegion &region)
{
    QVector<QRect> rects = region.rects();
    QRect bound(0, 0, dw, dh);
    fill_data data;
    data.color = color.rgba();
    data.data = this->data;
    data.lineStep = lstep;
    fillFunc func = 0;
    switch(d) {
#ifdef QT_QWS_DEPTH_32
    case 32:
        func = fill_32;
        break;
#endif
#ifdef QT_QWS_DEPTH_24
    case 24:
        func = fill_24;
        break;
#endif
#ifdef QT_QWS_DEPTH_18
    case 18:
    case 19:
        func = fill_18;
        break;
#endif
#ifdef QT_QWS_DEPTH_16
    case 16:
        func = fill_16;
        break;
#endif
#ifdef QT_QWS_DEPTH_8
    case 8:
        func = fill_8;
#endif
    default:
        break;
    }
    if (!func)
        return;

    QWSDisplay::grab();
    for (int i = 0; i < rects.size(); ++i) {
        QRect r = rects.at(i) & bound;
        data.w = r.width();
        data.h = r.height();
        if (data.w <= 0 || data.h <= 0)
            continue;
        data.x = r.x();
        data.y = r.y();
        func(&data);
    }
    QWSDisplay::ungrab();
}


void QScreen::compose(int level, const QRegion &exposed, QRegion &blend, QImage &blendbuffer, int changing_level)
{

    QRect exposed_bounds = exposed.boundingRect();
    QWSWindow *win = 0;
    do {
        win = qwsServer->clientWindows().value(level); // null ptr means background
        ++level;
    } while (win && !win->requestedRegion().boundingRect().intersects(exposed_bounds));

    bool above_changing = level <= changing_level; //0 is topmost

    QRegion exposedBelow = exposed;
    bool opaque = true;
    int windowType = QWSBackingStore::Invalid;

    if (win) {
        if (win->backingStore()) {
            windowType = win->backingStore()->windowType();
            if ((windowType == QWSBackingStore::Transparent || windowType == QWSBackingStore::Opaque)
                && win->backingStore()->isNull())
                windowType = QWSBackingStore::Invalid;
        }
        opaque = win->isOpaque();
        if (opaque) {
            exposedBelow -= win->requestedRegion();
            if (above_changing || windowType == QWSBackingStore::ReservedRegion
                || windowType == QWSBackingStore::NonBuffered)
                blend -= exposed & win->requestedRegion();
        } else {
            blend += exposed & win->requestedRegion();
        }
    }
    if (win && !exposedBelow.isEmpty()) {
        compose(level, exposedBelow, blend, blendbuffer, changing_level);
    } else {
        QSize blendSize = blend.boundingRect().size();
        if (!blendSize.isNull()) {
            blendbuffer = QImage(blendSize,
                                 qt_screen->depth() <= 16 ? QImage::Format_RGB16 : QImage::Format_ARGB32_Premultiplied);
        }
    }
    if (!win) {
        paintBackground(exposed-blend);
    } else if (!above_changing){
        if (windowType == QWSBackingStore::Opaque || windowType == QWSBackingStore::Transparent)
            blit(win, (exposed - blend));
    }
    QRegion blendRegion = exposed & blend;
    if (win)
        blendRegion &= win->requestedRegion();
    if (!blendRegion.isEmpty()) {

        QPoint off = blend.boundingRect().topLeft();
        bool locked = false;

        QRasterBuffer rb;
        rb.prepare(&blendbuffer);
        QSpanData spanData;
        spanData.init(&rb);
        int opacity = 255;
        if (!win) {
            spanData.setup(qwsServer->backgroundBrush(), opacity);
            spanData.dx = off.x();
            spanData.dy = off.y();
        } else if (windowType == QWSBackingStore::DebugHighlighter) {
            spanData.setup(QColor(255,255,31,127), opacity);
            spanData.dx = off.x();
            spanData.dy = off.y();
        } else {
            opacity = win->opacity();
            if (!win->backingStore() || win->backingStore()->isNull())
                return;
            locked = win->backingStore()->lock(max_lock_time);
            if (!locked)
                return;
            const QImage &img = win->backingStore()->image();
            QPoint winoff = off - win->requestedRegion().boundingRect().topLeft();
            spanData.type = QSpanData::Texture;
            spanData.initTexture(&img, opacity);
            spanData.dx = winoff.x();
            spanData.dy = winoff.y();
        }
        if (!spanData.blend) {
            if (locked)
                win->backingStore()->unlock();
            return;
        }

        if (win && !locked) {
            locked = win->backingStore()->lock(max_lock_time);
            if (!locked)
                return;
        }

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
                    spans[i].coverage = opacity;
                    ++i;
                }
                spanData.blend(n, spans, &spanData);
                y += n;
            }
        }

        if (locked)
            win->backingStore()->unlock();
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
        QRect br = r.boundingRect();
        QImage img(br.size(), QImage::Format_ARGB32_Premultiplied);
        QPoint off = br.topLeft();
        QRasterBuffer rb;
        rb.prepare(&img);
        QSpanData spanData;
        spanData.init(&rb);
        spanData.setup(bg, 255);
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
    \fn int * QScreen::opType()

    Returns the screen's operation type.

    \sa lastOp()
*/

/*!
    \fn int * QScreen::lastOp()

    Returns the screens last operation.

    \sa opType()
*/

/*!
    \fn QScreen::setDirty(const QRect& rectangle)

    This virtual function allows subclasses of QScreen to indicate
    that the given \a rectangle of the screen has been altered.

    Note that the default implementation does nothing.
*/

void QScreen::setDirty(const QRect&)
{
}

/*!
    \fn QScreen::isTransformed() const

    Returns true if the screen is transformed (for instance, rotated
    90 degrees); otherwise returns false.

    \sa QTransformedScreen::transformation()
*/

bool QScreen::isTransformed() const
{
    return false;
}

/*!
    \fn QScreen::isInterlaced() const

    Returns true if the display is interlaced (for instance a
    television screen); otherwise returns false.

    If true, drawing is altered to look better on such displays.
*/

bool QScreen::isInterlaced() const
{
    return false;//qws_screen_is_interlaced;;
}

/*!
    \fn QScreen::mapToDevice(const QSize &size) const

    This virtual function allows subclasses of QScreen to map the
    given object from the coordinate space used by the application to
    the framebuffer coordinate system.

    In the case of a \a size object, this means switching its height
    and width. Note that the default implementation simply returns the
    given \a size as it is.

    \sa mapFromDevice()
*/

QSize QScreen::mapToDevice(const QSize &s) const
{
    return s;
}

/*!
    \fn QScreen::mapFromDevice(const QSize &size) const

    This virtual function allows subclasses of QScreen to map the
    given object from the framebuffer coordinate system to the
    coordinate space used by the application.

    In the case of a \a size object, this means switching its height
    and width. Note that the default implementation simply returns the
    given \a size as it is.

    \sa mapToDevice()
*/

QSize QScreen::mapFromDevice(const QSize &s) const
{
    return s;
}

/*!
    \fn QScreen::mapToDevice(const QPoint &point, const QSize &screenSize) const
    \overload

    This virtual function allows subclasses of QScreen to map the
    given object from the coordinate space used by the application to
    the framebuffer coordinate system, passing the device's \a
    screenSize as argument.

    Note that the default implementation returns the given \a point as
    it is.
*/

QPoint QScreen::mapToDevice(const QPoint &p, const QSize &) const
{
    return p;
}

/*!
    \fn QScreen::mapFromDevice(const QPoint &point, const QSize &screenSize) const
    \overload

    This virtual function allows subclasses of QScreen to map the
    given object from the framebuffer coordinate system to the
    coordinate space used by the application, passing the device's \a
    screenSize as argument.

    Note that the default implementation returns the given \a point as
    it is.
*/

QPoint QScreen::mapFromDevice(const QPoint &p, const QSize &) const
{
    return p;
}

/*!
    \fn QScreen::mapToDevice(const QRect &rectangle, const QSize &screenSize) const
    \overload

    This virtual function allows subclasses of QScreen to map the
    given object from the coordinate space used by the application to
    the framebuffer coordinate system, passing the device's \a
    screenSize as argument.

    Note that the default implementation returns the given \a
    rectangle as it is.
*/

QRect QScreen::mapToDevice(const QRect &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::mapFromDevice(const QRect &rectangle, const QSize &screenSize) const
    \overload

    This virtual function allows subclasses of QScreen to map the
    given object from the framebuffer coordinate system to the
    coordinate space used by the application, passing the device's \a
    screenSize as argument.

    The default implementation returns the given \a rectangle as it is.
*/

QRect QScreen::mapFromDevice(const QRect &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::mapToDevice(const QImage &image) const
    \overload

    This virtual function allows subclasses of QScreen to map the
    given object from the coordinate space used by the application to
    the framebuffer coordinate system.

    The default implementation returns the given \a image as it is.
*/

QImage QScreen::mapToDevice(const QImage &i) const
{
    return i;
}

/*!
    \fn QScreen::mapFromDevice(const QImage &image) const
    \overload

    This virtual function allows subclasses of QScreen to map the
    given object from the framebuffer coordinate system to the
    coordinate space used by the application.

    The default implementation returns the given \a image as it is.
*/

QImage QScreen::mapFromDevice(const QImage &i) const
{
    return i;
}

/*!
    \fn QScreen::mapToDevice(const QRegion &region, const QSize &screenSize) const
    \overload

    This virtual function allows subclasses of QScreen to map the
    given object from the coordinate space used by the application to
    the framebuffer coordinate system, passing the device's \a
    screenSize as argument.

    The default implementation returns the given \a region as it is.
*/

QRegion QScreen::mapToDevice(const QRegion &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::mapFromDevice(const QRegion &region, const QSize &screenSize) const
    \overload

    This virtual function allows subclasses of QScreen to map the
    given object from the framebuffer coordinate system to the
    coordinate space used by the application, passing the device's \a
    screenSize as argument .

    The default implementation returns the given \a region as it is.
*/

QRegion QScreen::mapFromDevice(const QRegion &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::transformOrientation() const

    This virtual function allows subclasses of QScreen to return the
    current rotation of the screeen as an integer value.

    The default implementation returns 0.

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
