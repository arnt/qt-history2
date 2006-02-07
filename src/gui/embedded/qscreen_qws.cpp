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

#ifndef QT_NO_QWS_CURSOR
bool qt_sw_cursor=false;
Q_GUI_EXPORT QScreenCursor * qt_screencursor = 0;
#endif
Q_GUI_EXPORT QScreen * qt_screen = 0;

ClearCacheFunc QScreen::clearCacheFunc = 0;

#ifndef QT_NO_QWS_CURSOR
/*!
    \class QScreenCursor qscreen_qws.h
    \brief The QScreenCursor class manages the onscreen mouse cursor in
    Qtopia Core.

    \internal (for now)

    \ingroup qws

    It provides an implementation of a software mouse cursor
    and can be subclassed by hardware drivers which support a hardware mouse
    cursor. There may only be one QScreenCursor at a time; it is constructed
    by QScreen or one of its descendants.

    This class is non-portable. It is available only in Qtopia Core.
    It is also internal - this documentation is intended for those subclassing
    it in hardware drivers, not for application developers.
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
  \class QScreen qscreen_qws.h
  \brief The QScreen class and its descendants manage the framebuffer and
  palette.

  \ingroup qws

  QScreens act as factories for the screen cursor. QLinuxFbScreen
  manages a Linux framebuffer; accelerated drivers subclass QLinuxFbScreen.
  There can only be one screen in a Qtopia Core application.
*/

/*!
     \enum QScreen::PixelType

     \value NormalPixel
     \value BGRPixel
*/

/*!
\fn QScreen::initDevice()
This function is called by the Qtopia Core server when initializing
the framebuffer. Accelerated drivers use it to set up the graphics card.
*/

/*!
\fn QScreen::connect(const QString &displaySpec)
This function is called by every Qtopia Core application on startup.
It maps in the framebuffer and in the accelerated drivers the graphics
card control registers. \a displaySpec has the following syntax:
\code
    [screen driver][:driver specific options][:display number]
\endcode
for example if you want to use the mach64 driver on fb1 as display 2:
\code
    Mach64:/dev/fb1:2
\endcode
\a displaySpec is passed in via the QWS_DISPLAY environment variable
or the -display command line parameter.
*/

/*!
\fn QScreen::disconnect()
This function is called by every Qtopia Core application just
before exitting; it's normally used to unmap the framebuffer.
*/

/*!
    \fn QScreen::setMode(int width, int height, int depth)

    This function can be used to set the framebuffer \a width, \a
    height, and \a depth. It's currently unused.
*/

/*!
\fn QScreen::blank(bool on)
If \a on is true, blank the screen. Otherwise unblank it.
*/

/*!
\fn QScreen::pixmapOffsetAlignment()
Returns the value in bits to which the start address of pixmaps held in
graphics card memory should be aligned. This is only useful for accelerated
drivers. By default the value returned is 64 but it can be overridden
by individual accelerated drivers.
*/

/*!
\fn QScreen::pixmapLinestepAlignment()
Returns the value in bits to which individual scanlines of pixmaps held in
graphics card memory should be aligned. This is only useful for accelerated
drivers. By default the value returned is 64 but it can be overridden
by individual accelerated drivers.
*/

/*!
\fn QScreen::width() const
Gives the width in pixels of the framebuffer.
*/

/*!
\fn QScreen::height() const
Gives the height in pixels of the framebuffer.
*/

/*!
\fn QScreen::depth() const
Gives the depth in bits per pixel of the framebuffer. This is the number
of bits each pixel takes up rather than the number of significant bits,
so 24bpp and 32bpp express the same range of colors (8 bits of
red, green and blue)
*/

/*!
\fn QScreen::pixmapDepth() const
Gives the preferred depth for pixmaps. By default this is the same
as the screen depth, but for the VGA16 driver it's 8bpp.
*/

/*!
\fn QScreen::linestep() const
Returns the length in bytes of each scanline of the framebuffer.
*/

/*!
  \fn QScreen::deviceWidth() const
Gives the full width of the framebuffer device, as opposed to the
width which Qtopia Core will actually use. These can differ if the
display is centered within the framebuffer.
*/

/*!
  \fn QScreen::deviceHeight() const
Gives the full height of the framebuffer device, as opposed to the
height which Qtopia Core will actually use. These can differ if the
display is centered within the framebuffer.
*/

/*!
  \fn QScreen::base() const
Returns a pointer to the start of the framebuffer.
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
Returns the size in bytes of the screen. This is always located at
the beginning of framebuffer memory (i.e. at base()).
*/

/*!
  \fn QScreen::totalSize() const
Returns the size in bytes of available graphics card memory, including the
screen. Offscreen memory is only used by the accelerated drivers.
*/

// Unaccelerated screen/driver setup. Can be overridden by accelerated
// drivers

/*!
  \fn QScreen::QScreen(int display_id)
  Create a screen; the \a display_id is the number of the Qtopia Core server
  to connect to.
*/

/*!
  \fn QScreen::clut()
  Returns the screen's color lookup table (color palette). This is only
  valid in paletted modes (8bpp and lower).
*/

/*!
  \fn QScreen::numCols()
  Returns the number of entries in the color table returned by clut().
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
  Destroys a QScreen
*/

QScreen::~QScreen()
{
}

/*!
  Called by the Qtopia Core server on shutdown; never called by
  a Qtopia Core client. This is intended to support graphics card specific
  shutdown; the unaccelerated implementation simply hides the mouse cursor.
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
 */

/*!
  Given an RGB value \a r \a g \a b, return an index which is the closest
  match to it in the screen's palette. Used in paletted modes only.
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
  Saves the state of the graphics card - used so that, for instance,
  the palette can be restored when switching between linux virtual
  consoles. Hardware QScreen descendants should save register state
  here if necessary if switching between virtual consoles (for
  example to/from X) is to be permitted.
*/

void QScreen::save()
{
}

/*!
  Restores the state of the graphics card from a previous save()
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
\fn bool QScreen::supportsDepth(int d) const
Returns true if the screen supports a particular color depth \a d.
Possible values are 1,4,8,16 and 32.
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
\fn bool QScreen::onCard(const unsigned char * p) const
Returns true if the buffer pointed to by \a p is within graphics card
memory, false if it's in main RAM.
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
\fn bool QScreen::onCard(const unsigned char * p, ulong& offset) const
\overload
This checks whether the buffer specified by \a p is on the card
(as per the other version of onCard) and returns an offset in bytes
from the start of graphics card memory in \a offset if it is.
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
Given a display_id (number of the Qtopia Core server to connect to)
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
        QT_MEMCPY_USHORT(dest, src, data->w);
        src += sbpl;
        dest += dbpl;
        --h;
    }
}

/*!
    Copies the given \a region in the given \a image to the point
    specified by \a topLeft.

    Can be reimplemented in subclasses to use accelerated hardware.

    \omit the base class implementation works in device coordinates,
    so that transformed drivers can use it \endomit
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
    case 32:
        func = blit_32_to_32;
        break;
    case 16:
        if (img.depth() == 16)
            func = blit_16_to_16;
        else
            func = blit_32_to_16;
        break;
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
    case 32:
        func = fill_32;
        break;
    case 16:
        func = fill_16;
        break;
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

    if (win) {
        opaque = win->isOpaque();
        if (opaque) {
            exposedBelow -= win->requestedRegion();
            if (above_changing)
                blend -= win->requestedRegion();
        } else {
            blend += exposed & win->requestedRegion();
        }
    }
    if (win && !exposedBelow.isEmpty()) {
        compose(level, exposedBelow, blend, blendbuffer, changing_level);
    } else {
        QSize blendSize = blend.boundingRect().size();
        if (!blendSize.isNull())
            blendbuffer = QImage(blendSize, QWS_BACKINGSTORE_FORMAT);
    }
    if (!win) {
        paintBackground(exposed-blend);
    } else if (!above_changing && win->backingStore() && !win->backingStore()->isNull()) {
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
        QImage img;
        if (!win) {
            spanData.setup(qwsServer->backgroundBrush(), opacity);
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
*/

/*!
    \fn int * QScreen::lastOp()

    Returns the screens last operation.
*/

/*!
    \fn QScreen::setDirty(const QRect& rect)

    Indicates the rectangle, \a rect, of the screen that has been
    altered. Used by the VNC and VFB displays; the QScreen version
    does nothing.
*/

void QScreen::setDirty(const QRect&)
{
}

/*!
    \fn QScreen::isTransformed() const

    Returns true if the screen is transformed (for instance, rotated
    90 degrees); otherwise returns false. QScreen's version always
    returns false.
*/

bool QScreen::isTransformed() const
{
    return false;
}

/*!
    \fn QScreen::isInterlaced() const

    Returns true if the display is interlaced (for instance a
    television screen); otherwise returns false. If true, drawing is
    altered to look better on such displays.
*/

bool QScreen::isInterlaced() const
{
    return false;//qws_screen_is_interlaced;;
}

/*!
    \fn QScreen::mapToDevice(const QSize &s) const

    Map a user coordinate to the one to actually draw. Used by the
    rotated driver; the QScreen implementation simply returns \a s.
*/

QSize QScreen::mapToDevice(const QSize &s) const
{
    return s;
}

/*!
    \fn QScreen::mapFromDevice(const QSize &s) const

    Map a framebuffer coordinate to the coordinate space used by the
    application. Used by the rotated driver; the QScreen
    implementation simply returns \a s.
*/

QSize QScreen::mapFromDevice(const QSize &s) const
{
    return s;
}

/*!
    \fn QScreen::mapToDevice(const QPoint &point, const QSize &size) const

    \overload

    Map a user coordinate to the one to actually draw. Used by the
    rotated driver; the QScreen implementation simply returns the
    given \a point and ignores the \a size argument.
*/

QPoint QScreen::mapToDevice(const QPoint &p, const QSize &) const
{
    return p;
}

/*!
    \fn QScreen::mapFromDevice(const QPoint &point, const QSize &size) const

    \overload

    Map a framebuffer coordinate to the coordinate space used by the
    application. Used by the rotated driver; the QScreen
    implementation simply returns the given \a point and ignores the
    \a size argument.
*/

QPoint QScreen::mapFromDevice(const QPoint &p, const QSize &) const
{
    return p;
}

/*!
    \fn QScreen::mapToDevice(const QRect &rect, const QSize &size) const

    \overload

    Map a user coordinate to the one to actually draw. Used by the
    rotated driver; the QScreen implementation simply returns the
    given \a rect and ignores the \a size argument.
*/

QRect QScreen::mapToDevice(const QRect &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::mapFromDevice(const QRect &rect, const QSize &size) const

    \overload

    Map a framebuffer coordinate to the coordinate space used by the
    application. Used by the rotated driver; the QScreen
    implementation simply returns the given \a rect and ignores the
    \a size argument.
*/

QRect QScreen::mapFromDevice(const QRect &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::mapToDevice(const QImage &i) const
    \overload

    Transforms an image so that it fits the device coordinate space
    (e.g. rotating it 90 degrees clockwise). The QScreen
    implementation simply returns \a i.
*/

QImage QScreen::mapToDevice(const QImage &i) const
{
    return i;
}

/*!
    \fn QScreen::mapFromDevice(const QImage &i) const
    \overload

    Transforms an image so that it matches the application coordinate
    space (e.g. rotating it 90 degrees counter-clockwise). The QScreen
    implementation simply returns \a i.
*/

QImage QScreen::mapFromDevice(const QImage &i) const
{
    return i;
}

/*!
    \fn QScreen::mapToDevice(const QRegion &region, const QSize &size) const

    \overload

    Transforms a region so that it fits the device coordinate space
    (e.g. rotating it 90 degrees clockwise). The QScreen
    implementation simply returns the given \a region and ignores the
    \a size argument.
*/

QRegion QScreen::mapToDevice(const QRegion &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::mapFromDevice(const QRegion &region, const QSize &size) const

    \overload

    Transforms a region so that it matches the application coordinate
    space (e.g. rotating it 90 degrees counter-clockwise). The QScreen
    implementation simply returns the given \a region and ignores the
    \a size argument.
*/

QRegion QScreen::mapFromDevice(const QRegion &r, const QSize &) const
{
    return r;
}

/*!
    \fn QScreen::transformOrientation() const

    Used by the rotated server. The QScreeen implementation returns 0.
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
