#include "qscreen_qws.h"

#include "qcolormap.h"
#include "qscreendriverfactory_qws.h"

#include <private/qpaintengine_raster_p.h>

static const bool simple_8bpp_alloc = true; //### 8bpp support not done

#ifndef QT_NO_QWS_CURSOR
bool qt_sw_cursor=false;
QScreenCursor * qt_screencursor=0;
#endif
QScreen * qt_screen=0;

ClearCacheFunc QScreen::clearCacheFunc = 0;

#ifndef QT_NO_QWS_CURSOR
/*!
    \class QScreenCursor qscreen_qws.h
    \brief The QScreenCursor class manages the onscreen mouse cursor in
    Qt/Embedded.

    \internal (for now)

    \ingroup qws

    It provides an implementation of a software mouse cursor
    and can be subclassed by hardware drivers which support a hardware mouse
    cursor. There may only be one QScreenCursor at a time; it is constructed
    by QScreen or one of its descendants.

    This class is non-portable. It is available only in Qt/Embedded.
    It is also internal - this documentation is intended for those subclassing
    it in hardware drivers, not for application developers.
*/

extern bool qws_sw_cursor;

/*!
    \internal

    Constructs a screen cursor
*/
QScreenCursor::QScreenCursor() : imgunder(0), cursor(0)
{
}

/*!
    \internal

    Initialises a screen cursor - creates an image to store the part of the screen stored under the cursor.
    Should not be called by hardware cursor descendants. \a da points
    to the location in framebuffer memory where the cursor saves information
    stored under it, \a init is true if the cursor is being initialized
    (i.e. if the program calling this is the Qt/Embedded server), false
    if another application has already initialized it.
*/
void QScreenCursor::init(SWCursorData *da, bool init)
{
    data = da;
    save_under = false;
    fb_start = qt_screen->base();
    fb_end = fb_start + qt_screen->deviceHeight() * qt_screen->linestep();

    if (init) {
        data->x = qt_screen->deviceWidth()/2;
        data->y = qt_screen->deviceHeight()/2;
        data->width = 0;
        data->height = 0;
        data->enable = true;
        data->bound = QRect(data->x - data->hotx, data->y - data->hoty,
                       data->width+1, data->height+1);
    }
    clipWidth = qt_screen->deviceWidth();
    clipHeight = qt_screen->deviceHeight();

    int d = qt_screen->depth();
    int cols = d == 1 ? 0 : 256;
    if (d == 4) {
        d = 8;
        cols = 16;
    }
#if 0
    imgunder = new QImage(data->under, 64, 64, d, 0,
                cols, QImage::LittleEndian);
    if (d <= 8) {
        imgunder->setNumColors(cols);
        for (int i = 0; i < cols; i++)
            imgunder->setColor(i, qt_screen->clut()[i]);
    }
#else
    imgunder = new QImage(data->under, 64, 64, QImage::Format_RGB32); //############
#endif
}

/*!
    \internal

    Destroys a screen cursor, deleting its cursor image and
    under-cursor storage
*/
QScreenCursor::~QScreenCursor()
{
    delete imgunder;
}

/*!
    \internal

    Returns true if an alpha-blended cursor image is supported.
    This affects the type of QImage passed to the cursor - descendants
    returning true (as QScreenCursor does for bit depths of 8 and above.
    unless QT_NO_QWS_ALPHA_CURSOR is defined in qconfig.h) should be prepared
    to accept QImages with full 8-bit alpha channels
*/
bool QScreenCursor::supportsAlphaCursor()
{
#ifndef QT_NO_QWS_ALPHA_CURSOR
    return qt_screen->depth() >= 8;
#else
    return false;
#endif
}

/*!
    \internal

    Hide the mouse cursor from the screen.
*/
void QScreenCursor::hide()
{
    if (data->enable) {
         if (restoreUnder(data->bound))
//             QWSDisplay::ungrab();
             ;
        data->enable = false;
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
    if (!data->enable) {
//         if (qws_sw_cursor)
//             QWSDisplay::grab(true);
        data->enable = true;
        fb_start = qt_screen->base();
        fb_end = fb_start + qt_screen->deviceHeight() * qt_screen->linestep();
        clipWidth = qt_screen->deviceWidth();
        clipHeight = qt_screen->deviceHeight();
        saveUnder();
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
#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
//    QWSDisplay::grab(true);
#endif
    bool save = restoreUnder(data->bound);
    data->hotx = hotx;
    data->hoty = hoty;
    data->width = image.width();
    data->height = image.height();
    for (int r = 0; r < image.height(); r++)
        memcpy(data->cursor+data->width*r, image.scanLine(r), data->width);
    data->colors = image.numColors();
    int depth = qt_screen->depth();
    if (depth <= 8) {
        for (int i = 0; i < image.numColors(); i++) {
            int r = qRed(image.colorTable()[i]);
            int g = qGreen(image.colorTable()[i]);
            int b = qBlue(image.colorTable()[i]);
            data->translut[i] = QColormap::instance().pixel(QColor(r, g, b));
        }
    }
    for (int i = 0; i < image.numColors(); i++) {
        data->clut[i] = image.colorTable()[i];
    }
    data->bound = QRect(data->x - data->hotx, data->y - data->hoty,
                   data->width+1, data->height+1);
    if (save) saveUnder();
#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
//    QWSDisplay::ungrab();
#endif
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
    bool save = false;
    if (qws_sw_cursor) {
// #if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
//         QWSDisplay::grab(true);
// #endif
        save = restoreUnder(data->bound);
    }
    data->x = x;
    data->y = y;
    data->bound = QRect(data->x - data->hotx, data->y - data->hoty,
                        data->width+1, data->height+1);
    if (qws_sw_cursor) {
        if (save)
            saveUnder();
// #if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
//         QWSDisplay::ungrab();
// #endif
    }
}

/*!
    \internal

    This is relevant only to the software mouse cursor and should be
    reimplemented as a null method in hardware cursor drivers. It redraws
    what was under the mouse cursor when the cursor is moved. \a r
    is the rectangle that needs updating,
*/
bool QScreenCursor::restoreUnder(const QRect &r)
{
    if (!qws_sw_cursor)
        return false;

    if (!data || !data->enable) {
        return false;
    }

    if (!r.intersects(data->bound)) {
        return false;
    }

    if (!save_under) {
#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
//        QWSDisplay::grab(true);
#endif
        int depth = qt_screen->depth();

        int x = data->x - data->hotx;
        int y = data->y - data->hoty;

        {
        //duplicated logic
        int screenlinestep = qt_screen->linestep();
        int startRow = y < 0 ? qAbs(y) : 0;
        int startCol = x < 0 ? qAbs(x) : 0;
        int endRow = y + data->height > clipHeight ? clipHeight - y : data->height;
        int endCol = x + data->width > clipWidth ? clipWidth - x : data->width;

        unsigned char *screen = fb_start + (y + startRow) * screenlinestep
                                + (x + startCol) * depth/8;
        unsigned char *buf = data->under;


        if (endCol > startCol) {
            int bytes;
            if (depth < 8)
                bytes = (x + endCol)*depth/8 - (x + startCol)*depth/8 + 1;
            else
                bytes = (endCol - startCol) * depth / 8;

                for (int row = startRow; row < endRow; row++)
                {
                    memcpy(screen, buf, bytes);
                    screen += screenlinestep;
                    buf += bytes;
                }
            }
        }

        save_under = true;
        return true;
    }

    return false;
}

/*!
    \internal

    This saves the area under the mouse pointer - it should be reimplemented
    as a null method by hardware drivers.
*/
void QScreenCursor::saveUnder()
{
    if (!qws_sw_cursor)
        return;

    int depth = qt_screen->depth();
    int x = data->x - data->hotx;
    int y = data->y - data->hoty;

    {
        //duplicated logic
        int screenlinestep = qt_screen->linestep();
        int startRow = y < 0 ? qAbs(y) : 0;
        int startCol = x < 0 ? qAbs(x) : 0;
        int endRow = y + data->height > clipHeight ? clipHeight - y : data->height;
        int endCol = x + data->width > clipWidth ? clipWidth - x : data->width;

        unsigned char *screen = fb_start + (y + startRow) * screenlinestep
                                + (x + startCol) * depth/8;
        unsigned char *buf = data->under;


        if (endCol > startCol) {
            int bytes;
            if (depth < 8)
                bytes = (x + endCol)*depth/8 - (x + startCol)*depth/8 + 1;
            else
                bytes = (endCol - startCol) * depth / 8;

            for (int row = startRow; row < endRow; row++)
            {
                memcpy(buf, screen, bytes);
                screen += screenlinestep;
                buf += bytes;
            }
        }
    }

    drawCursor();

    save_under = false;

#if !defined(QT_NO_QWS_MULTIPROCESS) && !defined(QT_PAINTER_LOCKING)
//    QWSDisplay::ungrab();
#endif
}

/*!
    \internal

    This draws the software cursor. It should be reimplemented as a null
    method by hardware drivers
*/
void QScreenCursor::drawCursor()
{
    // We could use blt, but since cursor redraw speed is critical it
    // is all handled here.  Whether this is significantly faster is
    // questionable.
    int x = data->x - data->hotx;
    int y = data->y - data->hoty;

//      ### experimental
//     if (data->width != cursor->width() || data->height != cursor->height()) {
//         delete cursor;
//         cursor = new QImage(data->cursor, data->width, data->height, 8,
//                          data->clut, data->colors, QImage::IgnoreEndian);
//     }
//     if (data->width && data->height) {
//         qt_sw_cursor = false;   // prevent recursive call from blt
//         gfx->setSource(cursor);
//         gfx->setAlphaType(QGfx::InlineAlpha);
//         gfx->blt(x,y,data->width,data->height,0,0);
//         qt_sw_cursor = true;
//     }

//     return;


    int linestep = qt_screen->linestep();
    int depth = qt_screen->depth();

    // clipping
    int startRow = y < 0 ? qAbs(y) : 0;
    int startCol = x < 0 ? qAbs(x) : 0;
    int endRow = y + data->height > clipHeight ? clipHeight - y : data->height;
    int endCol = x + data->width > clipWidth ? clipWidth - x : data->width;

    unsigned char *dest = fb_start + (y + startRow) * linestep
                            + x * depth/8;
    unsigned const char *srcptr = data->cursor + startRow * data->width;

    QRgb *clut = data->clut;

#ifndef QT_NO_QWS_DEPTH_32
    if (depth == 32)
    {
        unsigned int *dptr = (unsigned int *)dest;
        unsigned int srcval;
        int av,r,g,b;
        for (int row = startRow; row < endRow; row++)
        {
            for (int col = startCol; col < endCol; col++)
            {
                srcval = clut[*(srcptr+col)];
                av = srcval >> 24;
                if (av == 0xff) {
                    *(dptr+col) = srcval;
                }
# ifndef QT_NO_QWS_ALPHA_CURSOR
                else if (av != 0) {
                    r = (srcval & 0xff0000) >> 16;
                    g = (srcval & 0xff00) >> 8;
                    b = srcval & 0xff;
                    unsigned int hold = *(dptr+col);
                    int sr=(hold & 0xff0000) >> 16;
                    int sg=(hold & 0xff00) >> 8;
                    int sb=(hold & 0xff);

                    r = ((r-sr) * av) / 256 + sr;
                    g = ((g-sg) * av) / 256 + sg;
                    b = ((b-sb) * av) / 256 + sb;

                    *(dptr+col) = (r << 16) | (g << 8) | b;
                }
# endif
            }
            srcptr += data->width;
            dptr += linestep/4;
        }
        return;
    }
#endif
#ifndef QT_NO_QWS_DEPTH_24
    if (depth == 24)
    {
        unsigned int srcval;
        int av,r,g,b;
        for (int row = startRow; row < endRow; row++)
        {
            unsigned char *dptr = dest + (row-startRow) * linestep + startCol * 3;
            for (int col = startCol; col < endCol; col++, dptr += 3)
            {
                srcval = clut[*(srcptr+col)];
                av = srcval >> 24;
                if (av == 0xff) {
                    gfxSetRgb24(dptr, srcval);
                }
# ifndef QT_NO_QWS_ALPHA_CURSOR
                else if (av != 0) {
                    r = (srcval & 0xff0000) >> 16;
                    g = (srcval & 0xff00) >> 8;
                    b = srcval & 0xff;
                    unsigned int hold = gfxGetRgb24(dptr);
                    int sr=(hold & 0xff0000) >> 16;
                    int sg=(hold & 0xff00) >> 8;
                    int sb=(hold & 0xff);

                    r = ((r-sr) * av) / 256 + sr;
                    g = ((g-sg) * av) / 256 + sg;
                    b = ((b-sb) * av) / 256 + sb;

                    gfxSetRgb24(dptr, r, g, b);
                }
# endif
            }
            srcptr += data->width;
        }
        return;
    }
#endif
#ifndef QT_NO_QWS_DEPTH_16
    if (depth == 16)
    {
        unsigned short *dptr = (unsigned short *)dest;
        unsigned int srcval;
        int av,r,g,b;
        for (int row = startRow; row < endRow; row++)
        {
            for (int col = startCol; col < endCol; col++)
            {
                srcval = clut[*(srcptr+col)];
                av = srcval >> 24;
                if (av == 0xff) {
                    *(dptr+col) = qt_convRgbTo16(srcval);
                }
# ifndef QT_NO_QWS_ALPHA_CURSOR
                else if (av != 0) {
                    // This is absolutely silly - but we can so we do.
                    r = (srcval & 0xff0000) >> 16;
                    g = (srcval & 0xff00) >> 8;
                    b = srcval & 0xff;

                    int sr;
                    int sg;
                    int sb;
                    qt_conv16ToRgb(*(dptr+col),sr,sg,sb);

                    r = ((r-sr) * av) / 256 + sr;
                    g = ((g-sg) * av) / 256 + sg;
                    b = ((b-sb) * av) / 256 + sb;

                    *(dptr+col) = qt_convRgbTo16(r,g,b);
                }
# endif
            }
            srcptr += data->width;
            dptr += linestep/2;
        }
        return;
    }
#endif
#if !defined(QT_NO_QWS_DEPTH_8)
    if (depth == 8) {
        unsigned char *dptr = (unsigned char *)dest;
        unsigned int srcval;
        int av,r,g,b;
        QRgb * screenclut=qt_screen->clut();
//        simple_8bpp_alloc=true;
        for (int row = startRow; row < endRow; row++)
        {
            for (int col = startCol; col < endCol; col++)
            {
                srcval = clut[*(srcptr+col)];
                av = srcval >> 24;
                if (av == 0xff) {
                    *(dptr+col) = data->translut[*(srcptr+col)];
                }
# if 0////ndef QT_NO_QWS_ALPHA_CURSOR
                else if (av != 0) {
                    // This is absolutely silly - but we can so we do.
                    r = (srcval & 0xff0000) >> 16;
                    g = (srcval & 0xff00) >> 8;
                    b = srcval & 0xff;

                    unsigned char hold = *(dptr+col);
                    int sr,sg,sb;
                    sr=qRed(screenclut[hold]);
                    sg=qGreen(screenclut[hold]);
                    sb=qBlue(screenclut[hold]);

                    r = ((r-sr) * av) / 256 + sr;
                    g = ((g-sg) * av) / 256 + sg;
                    b = ((b-sb) * av) / 256 + sb;

                    *(dptr+col) = GFX_CLOSEST_PIXEL_CURSOR(r,g,b);
                }
# endif
            }
            srcptr += data->width;
            dptr += linestep;
        }
//        simple_8bpp_alloc=false;
    }
#endif
#ifndef QT_NO_QWS_DEPTH_4
    if (depth == 4) {
        unsigned int srcval;
        int av;
        for (int row = startRow; row < endRow; row++)
        {
            unsigned char *dp = fb_start + (y + row) * linestep;
            for (int col = startCol; col < endCol; col++)
            {
                srcval = clut[*(srcptr+col)];
                av = srcval >> 24;
                if (av == 0xff) {
                    int tx = x + col;
                    unsigned char *dptr = dp + (tx>>1);
                    int val = data->translut[*(srcptr+col)];
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                    int s = (~tx & 1) << 2;
#else
                    int s = (tx & 1) << 2;
#endif
                    *dptr = (*dptr & (0xf0>>s)) | (val << s);
                }
            }
            srcptr += data->width;
        }
    }
#endif
#ifndef QT_NO_QWS_DEPTH_1
    if (depth == 1) {
        unsigned int srcval;
        int av;
        for (int row = startRow; row < endRow; row++)
        {
            unsigned char *dp = fb_start + (y + row) * linestep;
            int x1 = x+startCol;
            int x2 = x+endCol-1;
            dp += x1/8;
            int skipbits = x1%8;
            int col = startCol;
            for (int b = x1/8; b <= x2/8; b++) {
                unsigned char m = *dp;
                for (int i = 0; i < 8 && col < endCol; i++) {
                    if (skipbits)
                        skipbits--;
                    else {
                        srcval = clut[*(srcptr+col)];
                        av = srcval >> 24;
                        if (av == 0xff) {
                            unsigned char val = data->translut[*(srcptr+col)];
#ifdef QT_QWS_EXPERIMENTAL_REVERSE_BIT_ENDIANNESS
                            if (val)
                                m |= 0x80 >> i;
                            else
                                m &= ~(0x80 >> i);
#else
                            if (val)
                                m |= 1 << i;
                            else
                                m &= ~(1 << i);
#endif
                        }
                        col++;
                    }
                }
                *(dp++) = m;
            }
            srcptr += data->width;
        }
    }
#endif
}
#endif // QT_NO_QWS_CURSOR




















/*!
  \class QScreen qscreen_qws.h
  \brief The QScreen class and its descendants manage the framebuffer and
  palette.

  \ingroup qws

  QScreens act as factories for the screen cursor and QPaintEngine. QLinuxFbScreen
  manages a Linux framebuffer; accelerated drivers subclass QLinuxFbScreen.
  There can only be one screen in a Qt/Embedded application.
*/

/*!
     \enum QScreen::PixelType

     \value NormalPixel
     \value BGRPixel
*/

/*!
\fn QScreen::initDevice()
This function is called by the Qt/Embedded server when initializing
the framebuffer. Accelerated drivers use it to set up the graphics card.
*/

/*!
\fn QScreen::connect(const QString &displaySpec)
This function is called by every Qt/Embedded application on startup.
It maps in the framebuffer and in the accelerated drivers the graphics
card control registers. \a displaySpec has the following syntax:
<p>
<tt>[screen driver][:driver specific options][:display number]</tt>
<p>
for example if you want to use the mach64 driver on fb1 as display 2:
<p>
<tt>Mach64:/dev/fb1:2</tt>
<p>
\a displaySpec is passed in via the QWS_DISPLAY environment variable
or the -display command line parameter.
*/

/*!
\fn QScreen::disconnect()
This function is called by every Qt/Embedded application just
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
width which Qt/Embedded will actually use. These can differ if the
display is centered within the framebuffer.
*/

/*!
  \fn QScreen::deviceHeight() const
Gives the full height of the framebuffer device, as opposed to the
height which Qt/Embedded will actually use. These can differ if the
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
  Create a screen; the \a display_id is the number of the Qt/Embedded server
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
  Called by the Qt/Embedded server on shutdown; never called by
  a Qt/Embedded client. This is intended to support graphics card specific
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
This is used to initialize the software cursor - \a end_of_location
points to the address after the area where the cursor image can be stored.
\a init is true for the first application this method is called from
(the Qt/Embedded server), false otherwise.
*/

int QScreen::initCursor(void* end_of_location, bool init)
{
    /*
      The end_of_location parameter is unusual: it's the address
      after the cursor data.
    */
#ifndef QT_NO_QWS_CURSOR
    qt_sw_cursor=true;
    // ### until QLumpManager works Ok with multiple connected clients,
    // we steal a chunk of shared memory
    SWCursorData *data = (SWCursorData *)end_of_location - 1;
    qt_screencursor=new QScreenCursor();
    qt_screencursor->init(data, init);
    return sizeof(SWCursorData);
#else
    return 0;
#endif
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
#ifndef QT_NO_QWS_DEPTH_1
    } else if(d==1) {
        return true;
#endif
#ifndef QT_NO_QWS_DEPTH_4
    } else if(d==4) {
        return true;
#endif
#ifndef QT_NO_QWS_DEPTH_8
    } else if(d==8) {
        return true;
#endif
#ifndef QT_NO_QWS_DEPTH_16
    } else if(d==16) {
        return true;
#endif
#ifndef QT_NO_QWS_DEPTH_24
    } else if(d==24) {
        return true;
#endif
#ifndef QT_NO_QWS_DEPTH_32
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
Given a display_id (number of the Qt/Embedded server to connect to)
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



QPaintEngine * QScreen::createPaintEngine(unsigned char * bytes,int w,int h,int d, int linestep)
{
    QRasterPaintEngine *pe = 0;
    //create screen QImage [pixmap?] somewhere ???
    QImage::Format format;

    //##### endianness #####
    switch (d) {
    case 1:
        format = QImage::Format_MonoLSB;
        break;
#if 0
    case 2:
        format = QImage::Format_Grayscale2LSB;
        break;
#endif
    case 4:
        format = QImage::Format_Grayscale4LSB;
        break;
    case 8:
        format = QImage::Format_Indexed8;
        break;
    case 16:
        format = QImage::Format_RGB16;
        break;
    case 32:
        format = QImage::Format_RGB32;
        break;
    default:
        qWarning("QScreen::createPaintEngine does not support depth %d", d);
        return 0;
    }
    QImage screenimage(bytes, w, h, format); //### linestep???

    pe = new QRasterPaintEngine;

    pe->begin(&screenimage); //?????

    return pe;
}


QPaintEngine *QScreen::createScreenEngine()
{
    return createPaintEngine(data,w,h,d,lstep);
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
