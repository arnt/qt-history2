/*****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Implementation of QGfxRaster (unaccelerated graphics context) class for
** Embedded Qt
** Created : 940721
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qgfxraster_qws.h"
#include "qpen.h"
#include "qpaintdevicemetrics.h"
#include "qmemorymanager_qws.h"
#include "qwsregionmanager_qws.h"
#include "qwsdisplay_qws.h"


#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef Q_CC_EDG
//####### hacky workaround for KCC/linux include files
typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;
#endif

#include <linux/fb.h>

#ifdef __i386__
#include <asm/mtrr.h>
#endif

#ifndef QT_NO_QWS_GFX_SPEED
# define QWS_EXPERIMENTAL_FASTPATH
# define GFX_INLINE inline
#else
# define GFX_INLINE
#endif

// Experimental - need to try on slow machine
//#define QWS_SLOW_GFX_MEMORY

//#if !defined(__i386__) || defined(QT_NO_QWS_GFX_SPEED)
#if defined(QT_NO_QWS_GFX_SPEED)
#define QWS_NO_WRITE_PACKING
#endif

#if !defined(__i386__)
#define QWS_PACKING_4BYTE
#endif

#if defined(__i386__) || !defined(__GNUC__)
# ifdef QWS_PACKING_4BYTE
typedef unsigned int PackType;
# else
typedef double PackType;
# endif
#else
# ifdef QWS_PACKING_4BYTE
typedef unsigned int PackType;
# else
typedef long long PackType;
# endif
#endif

#define QGfxRaster_Generic 0
#define QGfxRaster_VGA16   1

#ifndef QT_NO_QWS_CURSOR

struct SWCursorData {
    unsigned char cursor[SW_CURSOR_DATA_SIZE];
    unsigned char under[SW_CURSOR_DATA_SIZE*4];	// room for 32bpp display
    QRgb clut[256];
    unsigned char translut[256];
    int colors;
    int width;
    int height;
    int x;
    int y;
    int hotx;
    int hoty;
    bool enable;
    QRect bound;
};
#endif

#ifndef QT_NO_QWS_CURSOR
# define GFX_START(r) bool swc_do_save=FALSE; \
		     if(is_screen_gfx && qt_sw_cursor) { \
                        if((*optype)) sync(); \
			swc_do_save = qt_screencursor->restoreUnder(r,this); \
			beginDraw(); \
		     }
# define GFX_END if(is_screen_gfx && qt_sw_cursor) { \
                    if((*optype)) sync(); \
		    endDraw(); \
		    if(swc_do_save) \
			qt_screencursor->saveUnder(); \
		}
#else //QT_NO_QWS_CURSOR

# define GFX_START(r) if(is_screen_gfx) \
			beginDraw();
# define GFX_END if(is_screen_gfx) \
		    endDraw();
#endif //QT_NO_QWS_CURSOR

// The VGA16 driver requires the qt_screen->alloc() GFX_8BPP_PIXEL macro,
// but this slows down alpha blending a little in 8-bit modes, so we need
// a back-door to still use simple allocation to avoid very slow blitting.
//
#ifndef QT_NO_QWS_VGA_16
# define QT_NEED_SIMPLE_ALLOC
# define GFX_8BPP_PIXEL(r,g,b) qt_screen->alloc(r,g,b)
#else
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
# define GFX_8BPP_PIXEL(r,g,b) qGray((r),(g),(b))
#else
# define GFX_8BPP_PIXEL(r,g,b) (((r) + 25) / 51 * 36 + ((g) + 25) / 51 * 6 + ((b) + 25) / 51)
#endif
#endif

#define MASK4BPP(x) (0xf0 >> (x))

inline void gfxSetRgb24( unsigned char *d, unsigned int p )
{
    *d = p & 0x0000ff;
    *(d+1) = (p & 0x00ff00 ) >> 8;
    *(d+2) = (p & 0xff0000 ) >> 16;
}

inline void gfxSetRgb24( unsigned char *d, int r, int g, int b )
{
    *d = b;
    *(d+1) = g;
    *(d+2) = r;
}

inline unsigned int gfxGetRgb24( unsigned char *d )
{
    return *d | (*(d+1)<<8) | (*(d+2)<<16);
}

static bool simple_8bpp_alloc=FALSE;

static volatile int * optype=0;
static volatile int * lastop=0;

#ifndef QT_NO_QWS_CURSOR

QScreenCursor::QScreenCursor()
{
}

void QScreenCursor::init(SWCursorData *da, bool init)
{
    // initialise our gfx
    gfx = (QGfxRasterBase*)qt_screen->screenGfx();
    gfx->setClipRect( 0, 0, gfx->pixelWidth(), gfx->pixelHeight() );

    data = da;
    save_under = FALSE;
    fb_start = qt_screen->base();
    fb_end = fb_start + gfx->pixelHeight() * gfx->linestep();

    if (init) {
        data->x = gfx->pixelWidth()/2;
        data->y = gfx->pixelHeight()/2;
        data->width = 0;
        data->height = 0;
	data->enable = TRUE;
	data->bound = QRect( data->x - data->hotx, data->y - data->hoty,
		       data->width+1, data->height+1 );
    }
    clipWidth = gfx->pixelWidth();
    clipHeight = gfx->pixelHeight();

    int d = gfx->bitDepth();
    int cols = gfx->bitDepth() == 1 ? 0 : 256;
    if ( d == 4 ) {
	d = 8;
	cols = 16;
    }

    imgunder = new QImage( data->under, 64, 64, d, 0,
		cols, QImage::LittleEndian );
    if ( d <= 8 ) {
	for ( int i = 0; i < cols; i++ )
	    imgunder->setColor( i, qt_screen->clut()[i] );
    }
    gfxunder = (QGfxRasterBase*)imgunder->graphicsContext();

    cursor = new QImage();
}

QScreenCursor::~QScreenCursor()
{
    delete gfx;
    delete gfxunder;
    delete imgunder;
    delete cursor;
}

bool QScreenCursor::supportsAlphaCursor()
{
#ifndef QT_NO_QWS_ALPHA_CURSOR
    return gfx->bitDepth() >= 8;
#else
    return FALSE;
#endif
}

void QScreenCursor::hide()
{
    if ( data->enable ) {
	restoreUnder(data->bound);
	data->enable = FALSE;
    }
}

void QScreenCursor::show()
{
    if ( !data->enable ) {
	data->enable = TRUE;
	saveUnder();
    }
}

void QScreenCursor::set(const QImage &image, int hotx, int hoty)
{
    QWSDisplay::grab( TRUE );
    bool save = restoreUnder(data->bound);
    data->hotx = hotx;
    data->hoty = hoty;
    data->width = image.width();
    data->height = image.height();
    memcpy(data->cursor, image.bits(), image.numBytes());
    data->colors = image.numColors();
    int depth = gfx->bitDepth();
    if ( depth <= 8 ) {
	for (int i = 0; i < image.numColors(); i++) {
	    int r = qRed( image.colorTable()[i] );
	    int g = qGreen( image.colorTable()[i] );
	    int b = qBlue( image.colorTable()[i] );
	    data->translut[i] = QColor(r, g, b).pixel();
	}
    }
    for (int i = 0; i < image.numColors(); i++) {
	data->clut[i] = image.colorTable()[i];
    }
    data->bound = QRect( data->x - data->hotx, data->y - data->hoty,
		   data->width+1, data->height+1 );
    if (save) saveUnder();
    QWSDisplay::ungrab();
}

void QScreenCursor::move(int x, int y)
{
    QWSDisplay::grab( TRUE );
    bool save = restoreUnder(data->bound);
    data->x = x;
    data->y = y;
    data->bound = QRect( data->x - data->hotx, data->y - data->hoty,
		   data->width+1, data->height+1 );
    if (save) saveUnder();
    QWSDisplay::ungrab();
}


bool QScreenCursor::restoreUnder( const QRect &r, QGfxRasterBase *g )
{
    int depth = gfx->bitDepth();

    if (!data || !data->enable)
	return FALSE;

    if (!r.intersects(data->bound))
	return FALSE;

    if ( g && !g->is_screen_gfx )
	return FALSE;

    if (!save_under) {
	QWSDisplay::grab( TRUE );

	int x = data->x - data->hotx;
	int y = data->y - data->hoty;

	if ( depth < 8 ) {
	    if ( data->width && data->height ) {
		qt_sw_cursor = FALSE;   // prevent recursive call from blt
		gfx->setSource( imgunder );
		gfx->setAlphaType(QGfx::IgnoreAlpha);
		gfx->blt(x,y,data->width,data->height,0,0);
		qt_sw_cursor = TRUE;
	    }
	} else {
	    // This is faster than the above - at least until blt is
	    // better optimised.
	    int linestep = gfx->linestep();
	    int startCol = x < 0 ? QABS(x) : 0;
	    int startRow = y < 0 ? QABS(y) : 0;
	    int endRow = y + data->height > clipHeight ? clipHeight - y : data->height;
	    int endCol = x + data->width > clipWidth ? clipWidth - x : data->width;

	    int srcLineStep = data->width * depth/8;
	    unsigned char *dest = fb_start + (y + startRow) * linestep
				    + (x + startCol) * depth/8;
	    unsigned char *src = data->under;

	    if ( endCol > startCol ) {
		int bytes;
		if ( depth < 8 )
		    bytes = (x + endCol - 1)*depth/8 - (x + startCol)*depth/8 + 1;
		else
		    bytes = (endCol - startCol) * depth / 8;
		if ( depth == 1 ) bytes++;
		for (int row = startRow; row < endRow; row++)
		{
		    memcpy(dest, src, bytes);
		    src += srcLineStep;
		    dest += linestep;
		}
	    }
	}
	save_under = TRUE;
	return TRUE;
    }

    return FALSE;
}

void QScreenCursor::saveUnder()
{
    int depth = gfx->bitDepth();
    int x = data->x - data->hotx;
    int y = data->y - data->hoty;

    if ( depth < 8 ) {
	qt_sw_cursor = FALSE;   // prevent recursive call from blt
	gfxunder->setAlphaType(QGfx::IgnoreAlpha);
	gfxunder->setSource( qApp->desktop() );
	gfxunder->blt(0,0,data->width,data->height,x,y);
	qt_sw_cursor = TRUE;
    } else {
	// This is faster than the above - at least until blt is
	// better optimised.
	int linestep = gfx->linestep();
	int startRow = y < 0 ? QABS(y) : 0;
	int startCol = x < 0 ? QABS(x) : 0;
	int endRow = y + data->height > clipHeight ? clipHeight - y : data->height;
	int endCol = x + data->width > clipWidth ? clipWidth - x : data->width;

	int destLineStep = data->width * depth / 8;

	unsigned char *src = fb_start + (y + startRow) * linestep
				+ (x + startCol) * depth/8;
	unsigned char *dest = data->under;

	if ( endCol > startCol ) {
	    int bytes;
	    if ( depth < 8 )
		bytes = (x + endCol - 1)*depth/8 - (x + startCol)*depth/8 + 1;
	    else
		bytes = (endCol - startCol) * depth / 8;
	    for (int row = startRow; row < endRow; row++)
	    {
		memcpy(dest, src, bytes);
		src += linestep;
		dest += destLineStep;
	    }
	}
    }

    drawCursor();

    save_under = FALSE;

    QWSDisplay::ungrab();
}

// We could use blt, but since cursor redraw speed is critical
// it is all handled here.  Whether this is significantly faster is
// questionable.
void QScreenCursor::drawCursor()
{
    int x = data->x - data->hotx;
    int y = data->y - data->hoty;

    /* ### experimental
    if ( data->width != cursor->width() || data->height != cursor->height() ) {
	delete cursor;
	cursor = new QImage( data->cursor, data->width, data->height, 8,
			 data->clut, data->colors, QImage::IgnoreEndian );
    }
    if ( data->width && data->height ) {
	qt_sw_cursor = FALSE;   // prevent recursive call from blt
	gfx->setSource( cursor );
	gfx->setAlphaType(QGfx::InlineAlpha);
	gfx->blt(x,y,data->width,data->height,0,0);
	qt_sw_cursor = TRUE;
    }

    return;
    */

    int linestep = gfx->linestep();
    int depth = gfx->bitDepth();

    // clipping
    int startRow = y < 0 ? QABS(y) : 0;
    int startCol = x < 0 ? QABS(x) : 0;
    int endRow = y + data->height > clipHeight ? clipHeight - y : data->height;
    int endCol = x + data->width > clipWidth ? clipWidth - x : data->width;

    unsigned char *dest = fb_start + (y + startRow) * linestep
			    + x * depth/8;
    unsigned char *srcptr = data->cursor + startRow * data->width;

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
		    gfxSetRgb24( dptr, srcval );
		}
# ifndef QT_NO_QWS_ALPHA_CURSOR
		else if (av != 0) {
		    r = (srcval & 0xff0000) >> 16;
		    g = (srcval & 0xff00) >> 8;
		    b = srcval & 0xff;
		    unsigned int hold = gfxGetRgb24( dptr );
		    int sr=(hold & 0xff0000) >> 16;
		    int sg=(hold & 0xff00) >> 8;
		    int sb=(hold & 0xff);

		    r = ((r-sr) * av) / 256 + sr;
		    g = ((g-sg) * av) / 256 + sg;
		    b = ((b-sb) * av) / 256 + sb;

		    gfxSetRgb24( dptr, r, g, b );
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
#if !defined(QT_NO_QWS_DEPTH_8GRAYSCALE) || !defined(QT_NO_QWS_DEPTH_8)
    if (depth == 8) {
	unsigned char *dptr = (unsigned char *)dest;
        unsigned int srcval;
	int av,r,g,b;
	QRgb * screenclut=qt_screen->clut();
#ifdef QT_NEED_SIMPLE_ALLOC
	simple_8bpp_alloc=TRUE;
#endif
	for (int row = startRow; row < endRow; row++)
	{
	    for (int col = startCol; col < endCol; col++)
	    {
		srcval = clut[*(srcptr+col)];
		av = srcval >> 24;
		if (av == 0xff) {
		    *(dptr+col) = data->translut[*(srcptr+col)];
		}
# ifndef QT_NO_QWS_ALPHA_CURSOR
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

		    *(dptr+col) = GFX_8BPP_PIXEL(r,g,b);
		}
# endif
	    }
	    srcptr += data->width;
	    dptr += linestep;
	}
#ifdef QT_NEED_SIMPLE_ALLOC
	simple_8bpp_alloc=FALSE;
#endif
    }
#endif
#ifndef QT_NO_QWS_DEPTH_4
    if ( depth == 4 ) {
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
		    int s = (tx & 1) << 2;
		    *dptr = ( *dptr & MASK4BPP(s) ) | (val << s);
		}
	    }
	    srcptr += data->width;
	}
    }
#endif
#ifndef QT_NO_QWS_DEPTH_1
    if ( depth == 1 ) {
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
	    for ( int b = x1/8; b <= x2/8; b++ ) {
		unsigned char m = *dp;
		for (int i = 0; i < 8 && col < endCol; i++) {
		    if (skipbits)
			skipbits--;
		    else {
			srcval = clut[*(srcptr+col)];
			av = srcval >> 24;
			if (av == 0xff) {
			    unsigned char val = data->translut[*(srcptr+col)];
			    if (val)
				m |= 1 << i;
			    else
				m &= ~( 1 << i );
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

/*
 *
 */
QGfxRasterBase::QGfxRasterBase(unsigned char * b,int w,int h) :
    buffer(b)
{
    // Buffers should always be aligned
    if(((unsigned long)b) & 0x3) {
	qDebug("QGfx buffer unaligned: %lx",(unsigned long)b);
    }
    is_screen_gfx = buffer==qt_screen->base();
    width=w;
    height=h;
    myfont=0;
    xoffs=0;
    yoffs=0;
    regionClip=FALSE;
    srctype=SourcePen;
    setPen(QColor(255,0,0));
    cbrushpixmap=0;
    dashedLines = FALSE;
    dashes = 0;
    numDashes = 0;
    widgetrgn=QRegion(0,0,w,h);
    cliprect = new QRect[1]; // Will be freed in update_clip()
    ncliprect = 0;
    alphatype=IgnoreAlpha;
    alphabuf = 0;
    ismasking=FALSE;
    srclinestep=0;
    srcbits=0;
    lstep=0;
    calpha=255;
    opaque=FALSE;
    backcolor=QColor(0,0,0);
    globalRegionRevision = 0;
    src_normal_palette=FALSE;
    clutcols = 0;
    update_clip();
    myrop=CopyROP;

    src_little_endian=TRUE;
#if !defined(QT_NO_QWS_DEPTH_8) || !defined(QT_NO_QWS_DEPTH_8GRAYSCALE)
    // default color map
    setClut( qt_screen->clut(), qt_screen->numCols() );
#endif
}

QGfxRasterBase::~QGfxRasterBase()
{
    delete [] dashes;
    delete [] cliprect;
}


void* QGfxRasterBase::beginTransaction(const QRect& r)
{
    GFX_START(r);
    return (void*)swc_do_save;
}

void QGfxRasterBase::endTransaction(void* data)
{
    bool swc_do_save = !!data;
    GFX_END;
}


void QGfxRasterBase::sync()
{
    (*optype)=0;
}

void QGfxRasterBase::setPen( const QPen & p )
{
    static char dash_line[]         = { 7, 3 };
    static char dot_line[]          = { 1, 3 };
    static char dash_dot_line[]     = { 7, 3, 2, 3 };
    static char dash_dot_dot_line[] = { 7, 3, 2, 3, 2, 3 };

    cpen=p;
    switch (cpen.style()) {
        case DashLine:
            setDashes( dash_line, sizeof(dash_line) );
	    setDashedLines(TRUE);
            break;
        case DotLine:
            setDashes( dot_line, sizeof(dot_line) );
	    setDashedLines(TRUE);
            break;
        case DashDotLine:
            setDashes( dash_dot_line, sizeof(dash_dot_line) );
	    setDashedLines(TRUE);
            break;
        case DashDotDotLine:
            setDashes( dash_dot_dot_line, sizeof(dash_dot_dot_line) );
	    setDashedLines(TRUE);
            break;
        default:
	    setDashedLines(FALSE);
            break;
    }
}

void QGfxRasterBase::setFont( const QFont & f)
{
    myfont=f.handle();
    if(!myfont) {
	qDebug("No font renderer!");
    }
}

void QGfxRasterBase::setClipRect( int x,int y,int w,int h )
{
    setClipRegion(QRegion(x,y,w,h));
}

void QGfxRasterBase::setClipRegion( const QRegion & r )
{
    regionClip=TRUE;
    cliprgn=r;
    cliprgn.translate(xoffs,yoffs);
    update_clip();

#ifdef QWS_EXTRA_DEBUG
    qDebug( "QGfxRasterBase::setClipRegion" );
    for (int i=0; i< ncliprect; i++) {
	QRect r = cliprect[i];
	qDebug( "   cliprect[%d] %d,%d %dx%d", i, r.x(), r.y(),
		r.width(), r.height() );
    }
#endif


}

void QGfxRasterBase::setClipping(bool b)
{
    if(regionClip!=b) {
	regionClip=b;
	update_clip();
    }
}

void QGfxRasterBase::setOffset( int x,int y )
{
    xoffs=x;
    yoffs=y;
}

void QGfxRasterBase::setWidgetRect( int x,int y,int w,int h )
{
    setWidgetRegion(QRegion(x,y,w,h));
}

void QGfxRasterBase::setWidgetRegion( const QRegion & r )
{
    widgetrgn=r; // screen coordinates
    update_clip();
    hsync(r.boundingRect().bottom());
}

void QGfxRasterBase::setGlobalRegionIndex( int idx )
{
    globalRegionIndex = idx;
    globalRegionRevision = qt_fbdpy->regionManager()->revision( idx );
    currentRegionRevision = *globalRegionRevision;
}

void QGfxRasterBase::setDashedLines(bool d)
{
    dashedLines = d;
}

void QGfxRasterBase::setDashes(char *dashList, int n)
{
    if (dashes) delete [] dashes;
    dashes = new char [n];
    memcpy(dashes, dashList, n);
    numDashes = n;
}

void QGfxRasterBase::fixClip()
{
    currentRegionRevision = *globalRegionRevision;
    QSize s = qt_screen->mapToDevice(QSize(qt_screen->width(), qt_screen->height()) );
    QRegion rgn = qt_fbdpy->regionManager()->region( globalRegionIndex );
    rgn = qt_screen->mapFromDevice( rgn, s );
    widgetrgn &= rgn;
    update_clip();
}

void QGfxRasterBase::update_clip()
{
    if(regionClip) {
	setrgn=widgetrgn.intersect(cliprgn);
    } else {
	setrgn=widgetrgn;
    }

    QRegion trgn = qt_screen->mapToDevice( setrgn, QSize( width, height ) );

    // cache bounding rect
    QRect sr( QPoint(0,0), qt_screen->mapToDevice( QSize(width, height) ) );
    clipbounds = sr.intersect(trgn.boundingRect());

    // Convert to simple array for speed
    QArray<QRect> a = trgn.rects();
    delete [] cliprect;
    cliprect = new QRect[a.size()];
#ifdef QWS_EXTRA_DEBUG
    qDebug( "QGfxRasterBase::update_clip" );
#endif
    for (int i=0; i<(int)a.size(); i++) {
	cliprect[i]=a[i];
#ifdef QWS_EXTRA_DEBUG
	qDebug( "   cliprect[%d] %d,%d %dx%d", i, a[i].x(), a[i].y(),
		a[i].width(), a[i].height() );
#endif
    }
    ncliprect = a.size();
    clipcursor = 0;
}

void QGfxRasterBase::moveTo( int x,int y )
{
    penx=x;
    peny=y;
}

void QGfxRasterBase::lineTo( int x,int y )
{
    drawLine(penx,peny,x,y);
    penx=x;
    peny=y;
}

QPoint QGfxRasterBase::pos() const
{
    return QPoint(penx, peny);
}

void QGfxRasterBase::setSourceWidgetOffset(int x ,int y)
{
    srcwidgetoffs = QPoint(x,y);
}

void QGfxRasterBase::setAlphaType(AlphaType a)
{
    alphatype=a;
    if(a==LittleEndianMask || a==BigEndianMask) {
	ismasking=TRUE;
    } else {
	ismasking=FALSE;
    }
}

void QGfxRasterBase::setAlphaSource(unsigned char * b,int l)
{
    alphabits=b;
    alphalinestep=l;
}

void QGfxRasterBase::setAlphaSource(int i,int i2,int i3,int i4)
{
    calpha=i;
    if(i2==-1)
	i2=i;
    if(i3==-1)
	i3=i;
    if(i4==-1)
	i4=i;
    calpha2=i2;
    calpha3=i3;
    calpha4=i4;
    setAlphaType(SolidAlpha);
}

void QGfxRasterBase::drawText(int x,int y,const QString & s)
{
    // Clipping can be handled by blt
    // Offset is handled by blt

    int loopc;

#ifdef DEBUG_LOCKS
    qDebug("unaccelerated drawText grab");
#endif
    QWSDisplay::grab(); // we need it later, and grab-must-precede-lock
#ifdef DEBUG_LOCKS
    qDebug("unaccelerated drawText lock");
#endif

    setSourcePen();
    if ( memorymanager->fontSmooth(myfont) ) {
	setAlphaType(SeparateAlpha);
    } else {
	setAlphaType(BigEndianMask);
    }

    for( loopc=0; loopc < int(s.length()); loopc++ ) {
	QGlyph glyph = memorymanager->lockGlyph(myfont, s[loopc]);
	int myw=glyph.metrics->width;
	srcwidth = myw;
	srcheight = glyph.metrics->height;
	setAlphaSource(glyph.data,glyph.metrics->linestep);
	int myx=x;
	int myy=y;
	myx+=glyph.metrics->bearingx;
	myy-=glyph.metrics->bearingy;
	if(glyph.metrics->width<1 || glyph.metrics->height<1
	    || glyph.metrics->width>1000 || glyph.metrics->height>1000
	    || glyph.metrics->linestep==0)
	{
	    // non-printing characters
	} else {
	    blt(myx,myy,myw,glyph.metrics->height,0,0);
	}
	x+=glyph.metrics->advance;
	// ... unlock glyph
    }
#ifdef DEBUG_LOCKS
    qDebug("unaccelerated drawText unlock");
#endif
#ifdef DEBUG_LOCKS
    qDebug("unaccelerated drawText ungrab");
#endif
    QWSDisplay::ungrab();
    setAlphaType(IgnoreAlpha);
}

void QGfxRasterBase::save()
{
    savepen=cpen;
    savebrush=cbrush;
}

void QGfxRasterBase::restore()
{
    setPen(savepen);
    setBrush(savebrush);
}

// inline QRect::setCoords
inline void QRect::setCoords( int xp1, int yp1, int xp2, int yp2 )
{
    x1 = (QCOORD)xp1;
    y1 = (QCOORD)yp1;
    x2 = (QCOORD)xp2;
    y2 = (QCOORD)yp2;
} 

/*!
  Returns whether the point (\a x, \a y) is in the clip region.

  If \a cr is not null, <t>*cr</t> is set to a rectangle containing
  the point, and within all of which the result does not change.
  If the result is TRUE, \a cr is the widest rectangle for which
  the result remains TRUE (so any point immediately to the left or
  right of \a cr will not be part of the clip region).

  Passing TRUE for the \a known_to_be_outside allows optimizations,
  but the results are not defined it (\a x, \a y) is in the clip region.

  Using this, you can efficiently iterator over the clip region
  using:

  \code
    bool inside = inClip(x,y,&cr);
    while (change y, preferably by +1) {
	while (change x by +1 or -1) {
	    if ( !cr.contains(x,y) )
		inside = inClip(x,y,&cr,inside);
	    if ( inside ) {
		draw stuff
	    }
	}
    }
  \endcode
*/
bool QGfxRasterBase::inClip(int x, int y, QRect* cr, bool known_to_be_outside)
{
    if ( !ncliprect ) {
	// No rectangles.
	if ( cr )
	    *cr = QRect(x-4000,y-4000,8000,8000);
	return FALSE;
    }

//qDebug("Find %d,%d...%s",x,y,known_to_be_outside?" (outside)":"");
    bool search=FALSE;
    const QRect *cursorRect = &cliprect[clipcursor];

//search=TRUE;
    if ( !known_to_be_outside ) {
	if ( cursorRect->contains(x,y) ) {
	    if ( cr )
		*cr = *cursorRect;

//  qDebug("found %d,%d at +0 in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
	    return TRUE;
	}
	if ( clipcursor > 0 ) {
	    if ( (cursorRect-1)->contains(x,y) ) {
		if ( cr )
		    *cr = cliprect[--clipcursor];

// qDebug("found %d,%d at -1 in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
		return TRUE;
	    }
	} else if ( clipcursor < (int)ncliprect-1 ) {
	    if ( (cursorRect+1)->contains(x,y) ) {
		if ( cr )
		    *cr = cliprect[++clipcursor];

// qDebug("found %d,%d at +1 in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
		return TRUE;
	    }
	}
	search=TRUE;
    }

    // Optimize case where (x,y) is in the same band as the clipcursor,
    // and to its right.  eg. left-to-right, same-scanline cases.
    //
    if ( cursorRect->right() < x
	&& cursorRect->top() <= y
	&& cursorRect->bottom() >= y )
    {
	// Move clipcursor right until it is after (x,y)
	while ( 1 ) {
	    if ( clipcursor+1 < ncliprect &&
		 (cursorRect+1)->top()==cursorRect->top() ) {
		// next clip rect is in this band too - move ahead
		clipcursor++;
		cursorRect++;
		if ( cursorRect->left() > x ) {
		    // (x,y) is between clipcursor-1 and clipcursor
		    if ( cr )
			cr->setCoords((cursorRect-1)->right()+1,
				cursorRect->top(),
				cursorRect->left()-1,
				cursorRect->bottom());
		    return FALSE;
		} else if ( cursorRect->right() >= x ) {
		    // (x,y) is in clipcursor
		    if ( cr )
			*cr = *cursorRect;

// qDebug("found %d,%d in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
		    return TRUE;
		}
	    } else {
		// (x,y) is after last rectangle on band
		if ( cr )
		    cr->setCoords(cursorRect->right()+1,
			    cursorRect->top(),y+4000,
			    cursorRect->bottom());
		return FALSE;
	    }
	}
    } else {
	search=TRUE;
    }

    // The "4000" below are infinitely large rectangles, made small enough
    // to let surrounding alrogithms work of small integers. It means that
    // in rare cases some extra calls may be made to this function, but that
    // will make no measurable difference in performance.

    /*
	(x,y) will be in one of these characteristic places:

	0. In a rectangle of the region
	1. Before the region
	2. To the left of the first rectangle in the first band
	3. To the left of the first rectangle in a non-first band
	4. Between two retcangles in a band
	5. To the right of the last rectangle in a non-last band
	6. Between the last two rectangles
	7. To the right of the last rectangle in the last band
	8. After the region
	9. Between the first two rectangles

                            1
                     2   BBBBBBB
                  3 BB0BBBB 4 BBBBBBBBB 5
                         BBBBBBB   6
                            7
    */


    if ( search ) {
//qDebug("Search for %d,%d",x,y);
	// binary search for rectangle which is before (x,y)
	int a=0;
	int l=ncliprect-1;
	int h;
	int m=-1;
	while ( l>0 ) {
	    h = l/2;
	    m = a + h;
//	    qDebug("l = %d, m = %d", l, m);
	    const QRect& r = cliprect[m];
	    if ( r.bottom() < y || r.top() <= y && r.right() < x ) {
		// m is before (x,y)
		a = m + 1;
		l = l - h - 1;
	    } else
		l = h;
	}
	// Rectangle "a" is the rectangle containing (x,y), or the
	// closest rectangle to the right of (x,y).
	clipcursor = a;
	cursorRect = &cliprect[clipcursor];
	if ( cursorRect->contains(x,y) ) {
	    // PLACE 0
//qDebug("found %d,%d in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
	    if ( cr )
		*cr = *cursorRect;
// qDebug("Found %d,%d in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
	    return TRUE;
	}
//qDebug("!found %d,%d in %d[%d..%d,%d..%d]",x,y,clipcursor,cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom());
    }

    // At this point, (x,y) is outside the clip region and clipcursor is
    // the rectangle to the right/below of (x,y), or the last rectangle.

    if ( cr ) {
	const QRect &tcr = *cursorRect;
	if ( y < tcr.top() && clipcursor == 0) {
	    // PLACE 1
//qDebug("PLACE 1");
	    cr->setCoords( x-4000,y-4000,x+4000,tcr.top()-1 );
	} else if ( clipcursor == (int)ncliprect-1 && y>tcr.bottom() ) {
	    // PLACE 7
//qDebug("PLACE 7");
	    cr->setCoords( x-4000,tcr.bottom()+1,x+4000,y+4000 );
	} else if ( clipcursor == (int)ncliprect-1 && x > tcr.right() ) {
	    // PLACE 6
//qDebug("PLACE 6");
	    cr->setCoords( tcr.right()+1,tcr.top(),x+4000,y+4000 );
	} else if ( clipcursor == 0 ) {
	    // PLACE 2
//qDebug("PLACE 2");
	    cr->setCoords( x-4000,y-4000,tcr.left()-1,tcr.bottom() );
	} else {
	    const QRect &prev_tcr = *(cursorRect-1);
	    if ( prev_tcr.bottom() < y && tcr.left() > x) {
		// PLACE 3
//qDebug("PLACE 3");
		cr->setCoords( x-4000,tcr.top(), tcr.left()-1,tcr.bottom() );
	    } else {
		if ( prev_tcr.y() == tcr.y() ) {
		    // PLACE 4
//qDebug("PLACE 4");
		    cr->setCoords( prev_tcr.right()+1, tcr.y(),
				       tcr.left()-1, tcr.bottom() );
		} else {
		    // PLACE 5
//qDebug("PLACE 5");
		    cr->setCoords( prev_tcr.right()+1, prev_tcr.y(),
				       prev_tcr.right()+4000, prev_tcr.bottom() );
		}
	    }
	}
    }

//qDebug("!found %d,%d in %d[%d..%d,%d..%d] nor [%d..%d,%d..%d]",x,y, clipcursor, cliprect[clipcursor].left(),cliprect[clipcursor].right(),cliprect[clipcursor].top(),cliprect[clipcursor].bottom(), cr->left(),cr->right(),cr->top(),cr->bottom());
    return FALSE;
}

inline void QGfxRasterBase::useBrush()
{
    pixel = cbrush.color().pixel();
}

inline void QGfxRasterBase::usePen()
{
    pixel = cpen.color().pixel();
}

void QGfxRasterBase::setBrush( const QBrush & b )
{
    cbrush=b;
    if((cbrush.style()!=NoBrush) && (cbrush.style()!=SolidPattern)) {
	patternedbrush=TRUE;
    } else {
	patternedbrush=FALSE;
    }
    srccol=b.color().pixel();
}

void QGfxRasterBase::setBrushOffset( int x, int y )
{
    brushoffs = QPoint( x, y );
}

void QGfxRasterBase::setSourcePen()
{
    srccol = cpen.color().pixel();
    src_normal_palette=TRUE;
    srctype=SourcePen;
    setSourceWidgetOffset( 0, 0 );
}

// Convert between pixel values for different depths
// reverse can only be true if sdepth == depth
GFX_INLINE unsigned int QGfxRasterBase::get_value_32(
		       int sdepth, unsigned char **srcdata, bool reverse)
{
    unsigned int ret;
    if(sdepth==32) {
	ret = *((unsigned int *)(*srcdata));
	if(reverse) {
	    (*srcdata)-=4;
	} else {
	    (*srcdata)+=4;
	}
#if !defined( QT_NO_QWS_DEPTH_24 )
    } else if(sdepth==24) {
	ret = gfxGetRgb24( *srcdata );
	(*srcdata) += 3;
#endif
#if !defined( QT_NO_IMAGE_16_BIT ) || !defined( QT_NO_QWS_DEPTH_16 )
    } else if(sdepth==16) {
	unsigned short int hold=*((unsigned short int *)(*srcdata));
	ret = qt_conv16ToRgb(hold);
	(*srcdata)+=2;
#endif
    } else if(sdepth==8) {
	unsigned char val=*((*srcdata));
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
	if(src_normal_palette) {
	    ret=((val >> 5) << 16)  | ((val >> 6) << 8) | (val >> 5);
	} else {
#else
	if(TRUE) {
#endif
	    ret = srcclut[val];
	}
	(*srcdata)++;
    } else if(sdepth==1) {
	if(monobitcount<8) {
	    monobitcount++;
	} else {
	    monobitcount=1;	// yes, 1 is correct
	    (*srcdata)++;
	    monobitval=*((*srcdata));
	}
	if(src_little_endian) {
	    ret=monobitval & 0x1;
	    monobitval=monobitval >> 1;
	} else {
	    ret=(monobitval & 0x80) >> 7;
	    monobitval=monobitval << 1;
	    monobitval=monobitval & 0xff;
	}
	ret=srcclut[ret];
    } else {
	qDebug("Odd source depth %d!",sdepth);
	ret=0;
    }

    return ret;
}

GFX_INLINE unsigned int QGfxRasterBase::get_value_24(
		       int sdepth, unsigned char **srcdata, bool reverse)
{
    unsigned int ret;
    if ( sdepth == 24 ) {
	ret = gfxGetRgb24( *srcdata );
	if ( reverse )
	    (*srcdata)-=3;
	else
	    (*srcdata)+=3;
    } else {
	ret = get_value_32( sdepth, srcdata, reverse );
    }

    return ret;
}


// reverse can only be true if sdepth == depth
GFX_INLINE unsigned int QGfxRasterBase::get_value_16(
		       int sdepth, unsigned char **srcdata, bool reverse)
{
#if !defined( QT_NO_IMAGE_16_BIT ) || !defined( QT_NO_QWS_DEPTH_16 )    
    unsigned int ret = 0;
    if ( sdepth == 16 ) {
	unsigned short int hold = *((unsigned short int *)(*srcdata));
	if(reverse) {
	    (*srcdata)-=2;
	} else {
	    (*srcdata)+=2;
	}
	ret=hold;
    } else if(sdepth==8) {
	unsigned char val=*((*srcdata));
	QRgb hold;
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
	if(src_normal_palette) {
	    hold = val*0x010101;
	} else
#endif
	{
	    hold=srcclut[val];
	}
	ret=qt_convRgbTo16(hold);
	(*srcdata)++;
    } else if(sdepth==1) {
	if(monobitcount<8) {
	    monobitcount++;
	} else {
	    monobitcount=1;
	    (*srcdata)++;
	    monobitval=*((*srcdata));
	}
	if(src_little_endian) {
	    ret=monobitval & 0x1;
	    monobitval=monobitval >> 1;
	} else {
	    ret=(monobitval & 0x80) >> 7;
	    monobitval=monobitval << 1;
	    monobitval=monobitval & 0xff;
	}
	ret=srcclut[ret];
    } else if ( sdepth == 32 ) {
	unsigned int hold = *((unsigned int *)(*srcdata));
	ret=qt_convRgbTo16(hold);
	(*srcdata)+=4;
    } else {
	qDebug("Odd source depth %d!",sdepth);
	abort();
	ret=0;
    }

    return ret;
#endif
}

// reverse can only be true if sdepth == depth
GFX_INLINE unsigned int QGfxRasterBase::get_value_8(
		       int sdepth, unsigned char **srcdata, bool reverse)
{
    unsigned int ret;

    if(sdepth==8) {
	unsigned char val=*((unsigned char *)(*srcdata));
	// If source!=QImage, then the palettes will be the same
	if(src_normal_palette) {
	    ret=val;
	} else {
	    ret=transclut[val];
	}
	if(reverse) {
	    (*srcdata)--;
	} else {
	    (*srcdata)++;
	}
    } else if(sdepth==1) {
	if(monobitcount<8) {
	    monobitcount++;
	} else {
	    monobitcount=1;
	    (*srcdata)++;
	    monobitval=*((*srcdata));
	}
	if(src_little_endian) {
	    ret=monobitval & 0x1;
	    monobitval=monobitval >> 1;
	} else {
	    ret=(monobitval & 0x80) >> 7;
	    monobitval=monobitval << 1;
	    monobitval=monobitval & 0xff;
	}
	ret = transclut[ret];
    } else if(sdepth==32) {
	unsigned int r,g,b;
	unsigned int hold=*((unsigned int *)(*srcdata));
	r=(hold & 0xff0000) >> 16;
	g=(hold & 0x00ff00) >> 8;
	b=(hold & 0x0000ff);
#ifdef QT_NEED_SIMPLE_ALLOC
	simple_8bpp_alloc=TRUE;
#endif
	ret = GFX_8BPP_PIXEL(r,g,b);
#ifdef QT_NEED_SIMPLE_ALLOC
	simple_8bpp_alloc=FALSE;
#endif
	(*srcdata)+=4;
    } else if ( sdepth == 4 ) {
	ret = monobitval & 0x0f;
	if ( !monobitcount ) {
	    monobitcount = 1;
	    monobitval >>= 4;
	} else {
	    monobitcount = 0;
	    (*srcdata)++;
	    monobitval = *(*srcdata);
	}
    } else {
	qDebug("Cannot do %d->8!",sdepth);
	ret=0;
    }

    return ret;
}

// reverse can only be true if sdepth == depth
GFX_INLINE unsigned int QGfxRasterBase::get_value_4(
		       int sdepth, unsigned char **srcdata, bool reverse)
{
    unsigned int ret;

    if ( sdepth == 4 ) {
	if ( reverse ) {
	    ret = (monobitval & 0xf0) >> 4;
	    if ( !monobitcount ) {
		monobitcount = 1;
		monobitval <<= 4;
	    } else {
		monobitcount = 0;
		(*srcdata)--;
		monobitval = *(*srcdata);
	    }
	} else {
	    ret = monobitval & 0x0f;
	    if ( !monobitcount ) {
		monobitcount = 1;
		monobitval >>= 4;
	    } else {
		monobitcount = 0;
		(*srcdata)++;
		monobitval = *(*srcdata);
	    }
	}
    } else if ( sdepth == 1 ) {
	if(monobitcount<8) {
	    monobitcount++;
	} else {
	    monobitcount=1;
	    (*srcdata)++;
	    monobitval=*((*srcdata));
	}
	if(src_little_endian) {
	    ret=monobitval & 0x1;
	    monobitval=monobitval >> 1;
	} else {
	    ret=(monobitval & 0x80) >> 7;
	    monobitval=monobitval << 1;
	    monobitval=monobitval & 0xff;
	}
	ret = transclut[ret];
    } else if ( sdepth == 8 ) {
	unsigned char val=*((unsigned char *)(*srcdata));
	ret = transclut[val];
	if(reverse)
	    (*srcdata)--;
	else
	    (*srcdata)++;
    } else if(sdepth==32) {
	unsigned int r,g,b;
	unsigned int hold=*((unsigned int *)(*srcdata));
	r=(hold & 0xff0000) >> 16;
	g=(hold & 0x00ff00) >> 8;
	b=(hold & 0x0000ff);
	ret = qGray( r, g, b ) >> 4;
	(*srcdata)+=4;
    } else {
	qDebug("Cannot do %d->4!",sdepth);
	ret=0;
    }

    return ret;
}

// reverse can only be true if sdepth == depth
GFX_INLINE unsigned int QGfxRasterBase::get_value_1(
		       int sdepth, unsigned char **srcdata, bool reverse)
{
    unsigned int ret;

    if(sdepth==1) {
	if ( reverse ) {
	    if(src_little_endian) {
		ret = ( monobitval & 0x80 ) >> 7;
		monobitval=monobitval << 1;
		monobitval=monobitval & 0xff;
	    } else {
		ret=monobitval & 0x1;
		monobitval=monobitval >> 1;
	    }
	    if(monobitcount < 7) {
		monobitcount++;
	    } else {
		monobitcount=0;
		(*srcdata)--;
		monobitval=**srcdata;
	    }
	} else {
	    if(src_little_endian) {
		ret=monobitval & 0x1;
		monobitval=monobitval >> 1;
	    } else {
		ret = ( monobitval & 0x80 ) >> 7;
		monobitval=monobitval << 1;
		monobitval=monobitval & 0xff;
	    }
	    if(monobitcount<7) {
		monobitcount++;
	    } else {
		monobitcount=0;
		(*srcdata)++;
		monobitval=**srcdata;
	    }
	}
    } else if(sdepth==32) {
	unsigned int hold=*((unsigned int *)(*srcdata));
	unsigned int r,g,b;
	r=(hold & 0xff0000) >> 16;
	g=(hold & 0x00ff00) >> 8;
	b=(hold & 0x0000ff);
	(*srcdata)+=4;
#ifdef QT_NEED_SIMPLE_ALLOC
	simple_8bpp_alloc=TRUE;
#endif
	ret = GFX_8BPP_PIXEL(r,g,b);
#ifdef QT_NEED_SIMPLE_ALLOC
	simple_8bpp_alloc=FALSE;
#endif
    } else {
	qDebug("get_value_1(): Unsupported source depth %d!",sdepth);
	ret=0;
    }

    return ret;
}


/*
 *
 */

template <const int depth, const int type>
QGfxRaster<depth,type>::QGfxRaster(unsigned char * b,int w,int h)
    : QGfxRasterBase(b,w,h)
{
    setLineStep((depth*width+7)/8);
    if ( depth == 1 ) {
	setPen( color1 );
	setBrush( color0 );
    } else {
	setBrush(QColor(0,0,0));
    }
}

template <const int depth, const int type>
QGfxRaster<depth,type>::~QGfxRaster()
{
}

// Calculate packing values for 64-bit writes

template<const int depth,const int type>
GFX_INLINE void QGfxRaster<depth,type>::calcPacking(
			  void * m,int x1,int x2,
			  int & frontadd,int & backadd,int & count)
{
    int w = x2-x1+1;

#ifndef QWS_NO_WRITE_PACKING
# if defined(QWS_PACKING_4BYTE)
    if ( depth == 16 ) {
	if ( w < 2 )
	    goto unpacked;

	unsigned short int * myptr=(unsigned short int *)m;
	frontadd=(((unsigned long)myptr)+(x1*2)) & 0x3;
	backadd=(((unsigned long)myptr)+((x2+1)*2)) & 0x3;
	if ( frontadd )
	    frontadd = 4 - frontadd;
	frontadd >>= 1;
	backadd >>= 1;
	count=( w-(frontadd+backadd) );
	count >>= 1;
    } else if ( depth == 8 ) {
	if ( w < 4 )
	    goto unpacked;

	unsigned char * myptr=(unsigned char *)m;
	frontadd=(((unsigned long)myptr)+x1) & 0x3;
	backadd=(((unsigned long)myptr)+x2+1) & 0x3;
	if ( frontadd )
	    frontadd = 4 - frontadd;
	count = w-(frontadd+backadd);
	count >>= 2;
    } else {
	goto unpacked;
    }
# else
    if(depth==32) {
	goto unpacked; // ### 32bpp packing doesn't work

	if ( w < 2 )
	    goto unpacked;

	unsigned int * myptr=(unsigned int *)m;
	frontadd=(((unsigned long)myptr)+(x1*4)) & 0x7;
	backadd=(((unsigned long)myptr)+((x2+1)*4)) & 0x7;
	if(frontadd)
	    frontadd=(8-frontadd);
	frontadd >>= 2;
	backadd >>= 2;
	count=( w-(frontadd+backadd) );
	count >>= 1;
    } else if ( depth == 16 ) {
	if ( w < 4 )
	    goto unpacked;

	unsigned short int * myptr=(unsigned short int *)m;
	frontadd=(((unsigned long)myptr)+(x1*2)) & 0x7;
	backadd=(((unsigned long)myptr)+((x2+1)*2)) & 0x7;
	if(frontadd)
	    frontadd=(8-frontadd);
	frontadd >>= 1;
	backadd >>= 1;
	count=( w-(frontadd+backadd) );
	count >>= 2;
    } else if(depth==8) {
	// ### 8bpp packing doesn't work
	unsigned char * myptr=(unsigned char *)m;
	if (  w < 8 )
	    goto unpacked;

	frontadd=(((unsigned long)myptr)+(x1)) & 0x7;
	backadd=(((unsigned long)myptr)+((x2+1))) & 0x7;
	if(frontadd)
	    frontadd=(8-frontadd);
	count=( w-(frontadd+backadd) );
	count >>= 3;
    } else {
	qDebug("Need packing for depth %d",depth);
	goto unpacked;
    }

    if(count<0)
	count=0;
    if(frontadd<0)
	frontadd=0;
    if(backadd<0)
	backadd=0;
    return;
# endif
#endif

unpacked:
    frontadd = w;
    backadd = 0;
    count = 0;
    if(frontadd<0)
	frontadd=0;
}

// if the source is 1bpp, the pen and brush currently active will be used
template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(const QPaintDevice * p)
{
    QPaintDeviceMetrics qpdm(p);
    srclinestep=((QPaintDevice *)p)->bytesPerLine();
    srcdepth=qpdm.depth();
    if(srcdepth==0)
	abort();
    srcbits=((QPaintDevice *)p)->scanLine(0);
    srctype=SourceImage;
    setAlphaType(IgnoreAlpha);
    if ( p->devType() == QInternal::Widget ) {
	QWidget * w=(QWidget *)p;
	srcwidth=w->width();
	srcheight=w->height();
	QPoint hold;
	hold=w->mapToGlobal(hold);
	setSourceWidgetOffset( hold.x(), hold.y() );
	if ( srcdepth == 1 ) {
	    buildSourceClut(0, 0);
	} else if(srcdepth <= 8) {
	    src_normal_palette=TRUE;
	}
    } else if ( p->devType() == QInternal::Pixmap ) {
	//still a bit ugly
	QPixmap *pix = (QPixmap*)p;
	setSourceWidgetOffset( 0, 0 );
	srcwidth=pix->width();
	srcheight=pix->height();
	if ( srcdepth == 1 ) {
	    buildSourceClut(0, 0);
	} else if(srcdepth <= 8) {
	    src_normal_palette=TRUE;
	}
    } else {
	// This is a bit ugly #### I'll say!
	//### We will have to find another way to do this
	setSourceWidgetOffset( 0, 0 );
	buildSourceClut(0,0);
    }

    src_little_endian=TRUE;
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(const QImage * i)
{
    srctype=SourceImage;
    srclinestep=i->bytesPerLine();
    srcdepth=i->depth();
    if(srcdepth==0)
	abort();
    srcbits=i->scanLine(0);
    src_little_endian=(i->bitOrder()==QImage::LittleEndian);
    setSourceWidgetOffset( 0, 0 );
    QSize s = qt_screen->mapToDevice( QSize(i->width(), i->height()) );
    srcwidth=s.width();
    srcheight=s.height();
    src_normal_palette=FALSE;
    if ( srcdepth == 1 )
	buildSourceClut( 0, 0 );
    else  if(srcdepth<=8)
	buildSourceClut(i->colorTable(),i->numColors());
}

// Cols==0, put some default values in. Numcols==0, figure it out from
// source depth

template <const int depth, const int type>
void QGfxRaster<depth,type>::buildSourceClut(QRgb * cols,int numcols)
{
    if (!cols) {
	useBrush();
	srcclut[0]=pixel;
	transclut[0]=pixel;
	usePen();
	srcclut[1]=pixel;
	transclut[1]=pixel;
	return;
    }

    int loopc;

    // Copy clut
    for(loopc=0;loopc<numcols;loopc++)
	srcclut[loopc] = cols[loopc];

    if(depth<=8) {
	// Now look for matches
	for(loopc=0;loopc<numcols;loopc++) {
	    int r = qRed(srcclut[loopc]);
	    int g = qGreen(srcclut[loopc]);
	    int b = qBlue(srcclut[loopc]);
	    transclut[loopc] = qt_screen->alloc(r,g,b);
	}
    }
}

//screen coordinates
template <const int depth, const int type>
GFX_INLINE void QGfxRaster<depth,type>::drawPointUnclipped( int x, unsigned char* l)
{
    if( (myrop!=XorROP) && (myrop!=NotROP) ) {
	if ( depth == 32 )
	    ((QRgb*)l)[x] = pixel;
	else if ( depth == 24 )
	    gfxSetRgb24( l + x*3, pixel );
	else if ( depth == 16 )
	    ((ushort*)l)[x] = (pixel & 0xffff);
	else if ( depth == 8 )
	    l[x] = (pixel & 0xff);
	else if ( depth == 4 ) {
	    uchar *d = l + (x>>1);
	    int s = (x & 1) << 2;
	    *d = ( *d & MASK4BPP(s) ) | (pixel << s);
	} else if ( depth == 1 )
	    if ( pixel )
		l[x/8] |= 1 << (x%8);
	    else
		l[x/8] &= ~(1 << (x%8));
    } else if(myrop==XorROP) {
	if ( depth == 32 )
	    ((QRgb*)l)[x] = ((QRgb*)l)[x] ^ pixel;
	else if ( depth == 24 ) {
	    unsigned char *ptr = l + x*3;
	    unsigned int s = gfxGetRgb24( ptr );
	    gfxSetRgb24( ptr, s ^ pixel );
	} else if ( depth == 16 )
	    ((ushort*)l)[x] = ((ushort*)l)[x] ^ pixel;
	else if ( depth == 8 )
	    l[x] = l[x] ^ pixel;
	else if ( depth == 4 ) {
	    uchar *d = l + (x>>1);
	    int s = (x & 1) << 2;
	    unsigned char p = *d;
	    unsigned char e = ( p & MASK4BPP(s) ) |
			      ( (p ^ (pixel << s)) & MASK4BPP(4-s) );
	    *d = e;
	} else if ( depth == 1 )
	    if ( pixel )
		l[x/8] |= 1 << (x%8);
	    else
		l[x/8] &= ~(1 << (x%8));
    } else if(myrop==NotROP) {
	if ( depth == 32 )
	    ((QRgb*)l)[x] = ~(((QRgb*)l)[x]);
	else if ( depth == 24 ) {
	    unsigned char *ptr = l + x*3;
	    unsigned int s = gfxGetRgb24( ptr );
	    gfxSetRgb24( ptr, ~s );
	} else if ( depth == 16 )
	    ((ushort*)l)[x] = ~(((ushort*)l)[x]);
	else if ( depth == 8 )
	    l[x] = ~(l[x]);
	else if ( depth == 4 ) {
	    uchar *d = l + (x>>1);
	    int s = (x & 1) << 2;
	    unsigned char p = *d;
	    *d = (p & MASK4BPP(s)) | ((~p) & MASK4BPP(4-s));
	} else if ( depth == 1 )
	    if ( pixel )
		l[x/8] |= 1 << (x%8);
	    else
		l[x/8] &= ~(1 << (x%8));
    } else {
	// ...
    }
}


template <const int depth, const int type>
void QGfxRaster<depth,type>::drawPoint( int x, int y )
{
    if(cpen.style()==NoPen)
	return;
    x += xoffs;
    y += yoffs;
    if (inClip(x,y)) {
	if((*optype))
	    sync();
	(*optype)=0;
	usePen();
    GFX_START(QRect(x,y,2,2))
	drawPointUnclipped( x, scanLine(y) );
    GFX_END
    }
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::drawPoints( const QPointArray & pa, int index, int npoints )
{
    if(cpen.style()==NoPen)
	return;
    usePen();
    QRect cr;
    bool in = FALSE;
    bool foundone=( ((*optype)==0) ? TRUE : FALSE );

    GFX_START(clipbounds);
    while (npoints--) {
	int x = pa[index].x() + xoffs;
	int y = pa[index].y() + yoffs;
	if ( !cr.contains(x,y) ) {
	    in = inClip(x,y,&cr);
	}
	if ( in ) {
	    if(foundone==FALSE) {
		sync();
		(*optype)=0;
		foundone=TRUE;
	    }
	    drawPointUnclipped( x, scanLine(y) );
	}
	++index;
    }
    GFX_END
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::drawLine( int x1, int y1, int x2, int y2 )
{
    if(cpen.style()==NoPen)
	return;

    if (cpen.width() > 1) {
	drawThickLine( x1, y1, x2, y2 );
	return;
    }

    if((*optype))
	sync();
    (*optype)=0;
    usePen();
    x1+=xoffs;
    y1+=yoffs;
    x2+=xoffs;
    y2+=yoffs;

    if(x1>x2) {
	int x3;
	int y3;
	x3=x2;
	y3=y2;
	x2=x1;
	y2=y1;
	x1=x3;
	y1=y3;
    }

    int dx=x2-x1;
    int dy=y2-y1;

    GFX_START(QRect(x1, y1 < y2 ? y1 : y2, dx+1, QABS(dy)+1))

#ifdef QWS_EXPERIMENTAL_FASTPATH
    // Fast path
    if (y1 == y2 && !dashedLines && ncliprect == 1) {
	if ( x1 > cliprect[0].right() || x2 < cliprect[0].left()
	     || y1 < cliprect[0].top() || y1 > cliprect[0].bottom() ) {
	    GFX_END
	    return;
	}
	x1 = x1 > cliprect[0].left() ? x1 : cliprect[0].left();
	x2 = x2 > cliprect[0].right() ? cliprect[0].right() : x2;
	unsigned char *l = scanLine(y1);
	hlineUnclipped(x1,x2,l);
	GFX_END
	return;
    }
#endif
    // Bresenham algorithm from Graphics Gems

    int ax=QABS(dx)*2;
    int ay=QABS(dy)*2;
    int sx=dx>0 ? 1 : -1;
    int sy=dy>0 ? 1 : -1;
    int x=x1;
    int y=y1;

    int d;

    QRect cr;
    bool inside = inClip(x,y,&cr);
    if(ax>ay && !dashedLines) {
	unsigned char* l = scanLine(y);
	d=ay-(ax >> 1);
	int px=x;
	#define FLUSH(nx) \
		if ( inside ) \
		    if ( sx < 1 ) \
			hlineUnclipped(nx,px,l); \
		    else \
			hlineUnclipped(px,nx,l); \
		px = nx+sx;
	for(;;) {
	    if(x==x2) {
		FLUSH(x);
		GFX_END
		return;
	    }
	    if(d>=0) {
		FLUSH(x);
		y+=sy;
		d-=ax;
		l = scanLine(y);
		if ( !cr.contains(x+sx,y) )
		    inside = inClip(x+sx,y, &cr);
	    } else if ( !cr.contains(x+sx,y) ) {
		FLUSH(x);
		inside = inClip(x+sx,y, &cr);
	    }
	    x+=sx;
	    d+=ay;
	}
    } else if (ax > ay) {
	// cannot use hline for dashed lines
	int di = 0;
	int dc = dashedLines ? dashes[0] : 0;
	d=ay-(ax >> 1);
	for(;;) {
	    if ( !cr.contains(x,y))
		inside = inClip(x,y, &cr);
	    if ( inside && (di&0x01) == 0) {
		drawPointUnclipped( x, scanLine(y) );
	    }
	    if(x==x2) {
		GFX_END
		return;
	    }
	    if (dashedLines && --dc <= 0) {
		if (++di >= numDashes)
		    di = 0;
		dc = dashes[di];
	    }
	    if(d>=0) {
		y+=sy;
		d-=ax;
	    }
	    x+=sx;
	    d+=ay;
	}
    } else {
	int di = 0;
	int dc = dashedLines ? dashes[0] : 0;
	d=ax-(ay >> 1);
	for(;;) {
	    // y is dominant so we can't optimise with hline
	    if ( !cr.contains(x,y))
		inside = inClip(x,y, &cr);
	    if ( inside && (di&0x01) == 0)
		drawPointUnclipped( x, scanLine(y) );
	    if(y==y2) {
		GFX_END
		return;
	    }
	    if (dashedLines && --dc <= 0) {
		if (++di >= numDashes)
		    di = 0;
		dc = dashes[di];
	    }
	    if(d>=0) {
		x+=sx;
		d-=ay;
	    }
	    y+=sy;
	    d+=ax;
	}
    }
    GFX_END
}

template <const int depth, const int type>
void QGfxRaster<depth, type>::drawThickLine( int x1, int y1, int x2, int y2 )
{
    QPointArray pa(5);
    int w = cpen.width() - 1;
    double a = atan2( y2 - y1, x2 - x1 );
    double ix = cos(a) * w / 2;
    if ( ix < 0.001 && ix > -0.001 )
	ix = 0.0;
    double iy = sin(a) * w / 2;
    if ( iy < 0.001 && iy > -0.001 )
	iy = 0.0;

    // No cap.
    pa[0].setX( x1 + iy );
    pa[0].setY( y1 - ix );
    pa[1].setX( x2 + iy );
    pa[1].setY( y2 - ix );
    pa[2].setX( x2 - iy );
    pa[2].setY( y2 + ix );
    pa[3].setX( x1 - iy );
    pa[3].setY( y1 + ix );

/*
    // square cap extends past end point
    pa[0].setX( x1 - ix + iy );
    pa[0].setY( y1 - iy - ix );
    pa[1].setX( x2 + ix + iy );
    pa[1].setY( y2 + iy - ix );
    pa[2].setX( x2 + ix - iy );
    pa[2].setY( y2 + iy + ix );
    pa[3].setX( x1 - ix - iy );
    pa[3].setY( y1 - iy + ix );
*/

    pa[4] = pa[0];

    if((*optype))
	sync();
    (*optype)=0;
    usePen();

    GFX_START(clipbounds)
    scan(pa, FALSE, 0, 5);
    QPen savePen = cpen;
    cpen = QPen( cpen.color() );
    drawPolyline(pa, 0, 5);
    cpen = savePen;
    GFX_END
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::drawThickPolyline( const QPointArray &points,int index, int npoints )
{
    if ( npoints < 2 )
	return;
    if ( npoints == 2 ) {
	drawThickLine( points[index].x(), points[index].y(),
		       points[index+1].x(), points[index+1].y() );
	return;
    }

    bool close = FALSE;
    QPointArray a(points);
    if ( a[index] == a[index+npoints-1] ) {
	a = QPointArray( npoints+1 );
	for ( int i = 0; i < npoints; i++ )
	    a.setPoint( i, points[index+i] );
	a.setPoint( npoints, a[1] );
	npoints++;
	index = 0;
	close = TRUE;
    }

    if((*optype)!=0) {
	sync();
    }
    (*optype)=0;
    GFX_START(clipbounds)

    int w = cpen.width() - 1;
    int i = 0;
    for ( i = index; i < index + npoints - 2; i++ ) {
	if ( a[i] == a[i+1] )
	    continue;
	drawThickLine( a[i].x(), a[i].y(), a[i+1].x(), a[i+1].y() );

	double at = atan2( a[i+1].y() - a[i].y(),
			  a[i+1].x() - a[i].x() );
	double ix1 = cos(at) * w / 2;
	if ( ix1 < 0.001 && ix1 > -0.001 )
	    ix1 = 0.0;
	double iy1 = sin(at) * w / 2;
	if ( iy1 < 0.001 && iy1 > -0.001 )
	    iy1 = 0.0;

	at = atan2( a[i+2].y() - a[i+1].y(),
		a[i+2].x() - a[i+1].x() );
	double ix2 = cos(at) * w / 2;
	if ( ix2 < 0.001 && ix2 > -0.001 )
	    ix2 = 0.0;
	double iy2 = sin(at) * w / 2;
	if ( iy2 < 0.001 && iy2 > -0.001 )
	    iy2 = 0.0;

	if ( ix1 != ix2 || iy1 != iy2 ) {
	    int l1x1 = (int)(a[i+1].x() + iy1);
	    int l1y1 = (int)(a[i+1].y() - ix1);

	    int l2x1 = (int)(a[i+1].x() - iy1);
	    int l2y1 = (int)(a[i+1].y() + ix1);

	    int l1x2 = (int)(a[i+1].x() + iy2);
	    int l1y2 = (int)(a[i+1].y() - ix2);

	    int l2x2 = (int)(a[i+1].x() - iy2);
	    int l2y2 = (int)(a[i+1].y() + ix2);

	    QPointArray pa(5);
	    int idx = 0;
	    pa.setPoint( idx++, l1x1, l1y1 );
	    pa.setPoint( idx++, l1x2, l1y2 );
	    pa.setPoint( idx++, l2x1, l2y1 );
	    pa.setPoint( idx++, l2x2, l2y2 );
	    pa[idx++] = pa[0];

	    usePen();
	    scan(pa,FALSE,0,idx);
	    QPen savePen = cpen;
	    cpen = QPen( cpen.color() );
	    drawPolyline(pa, 0, idx);
	    cpen = savePen;
	}
    }
    if ( !close )
	drawThickLine( a[i].x(), a[i].y(), a[i+1].x(), a[i+1].y() );
    GFX_END
}

//screen coordinates, clipped, x1<=x2
template <const int depth, const int type>
GFX_INLINE void QGfxRaster<depth,type>::hline( int x1,int x2,int y)
{
    unsigned char *l=scanLine(y);
    QRect cr;
    bool plot=inClip(x1,y,&cr);
    int x=x1;
    for (;;) {
	int xr = cr.right();
	if ( xr >= x2 ) {
	    if (plot) hlineUnclipped(x,x2,l);
	    break;
	} else {
	    if (plot) hlineUnclipped(x,xr,l);
	    x=xr+1;
	    plot=inClip(x,y,&cr,plot);
	}
    }
}

//screen coordinates, unclipped, x1<=x2, x1>=0
template <const int depth, const int type>
GFX_INLINE void QGfxRaster<depth,type>::hlineUnclipped( int x1,int x2,unsigned char* l)
{
    int w = x2-x1+1;
    if( (myrop!=XorROP) && (myrop!=NotROP) ) {
	if ( depth == 32 ) {
	    unsigned int *myptr=(unsigned int *)l + x1;
	    while ( w-- )
		*(myptr++) = pixel;
	} else if ( depth == 24 ) {
	    unsigned char *myptr = l + x1*3;
	    while ( w-- ) {
		gfxSetRgb24( myptr, pixel );
		myptr += 3;
	    }
	} else if ( depth == 16 ) {
	    unsigned short int *myptr=(unsigned short int *)l;
#ifdef QWS_NO_WRITE_PACKING
	    myptr+=x1;
	    while ( w-- )
		*(myptr++) = pixel;
#else
	    int frontadd;
	    int backadd;
	    int count;
	    calcPacking(myptr,x1,x2,frontadd,backadd,count);

	    myptr+=x1;

	    PackType put;
# ifdef QWS_PACKING_4BYTE
	    put = pixel | ( pixel << 16 );
# else
	    unsigned short int * tmp=(unsigned short int *)&put;
	    *tmp=pixel;
	    *(tmp+1)=pixel;
	    *(tmp+2)=pixel;
	    *(tmp+3)=pixel;
# endif

	    while ( frontadd-- )
		*(myptr++)=pixel;
	    while ( count-- ) {
		*((PackType *)myptr) = put;
# ifdef QWS_PACKING_4BYTE
		myptr += 2;
# else
		myptr += 4;
# endif
	    }
	    while ( backadd-- )
		*(myptr++)=pixel;
#endif
	} else if ( depth == 8 ) {
	    unsigned char *myptr=l;
	    myptr+=x1;
	    while ( w-- )
		*(myptr++) = pixel;
	} else if ( depth == 4 ) {
	    unsigned char *myptr=l;
	    unsigned char *dptr = myptr + x1/2;
	    if ( x1&1 ) {
		drawPointUnclipped( x1, myptr);
		w--;
		dptr++;
	    }

	    unsigned char val = pixel | (pixel << 4);
	    while ( w > 1 ) {
		*dptr++ = val;
		w -= 2;
	    }

	    if ( !(x2&1))
		drawPointUnclipped( x2, myptr);
	} else if ( depth == 1 ) {
	    //#### we need to use semaphore
	    l += x1/8;
	    if ( x1/8 == x2/8 ) {
		// Same byte

		uchar mask = (0xff << (x1 % 8)) & (0xff >> (7 - x2 % 8));
		if ( pixel )
		    *l |= mask;
		else
		    *l &= ~mask;
	    } else {
		volatile unsigned char *last = l + (x2/8-x1/8);
		uchar mask = 0xff << (x1 % 8);
		if ( pixel )
		    *l++ |= mask;
		else
		    *l++ &= ~mask;
		unsigned char byte = pixel ? 0xff : 0x00;
		while (l < last)
		    *l++ = byte;

		mask = 0xff >> (7 - x2 % 8);
		if ( pixel )
		    *l |= mask;
		else
		    *l &= ~mask;
	    }
	}
    } else if(myrop==XorROP) {
	if ( depth == 32 ) {
	    unsigned int *myptr=(unsigned int *)l + x1;
	    while ( w-- ) {
		(*myptr) = (*myptr) ^ pixel;
		myptr++;
	    }
	} else if ( depth == 16 ) {
	    unsigned short int *myptr=(unsigned short int *)l;
	    myptr+=x1;
	    while ( w-- ) {
		(*myptr) = (*myptr) ^ pixel;
		myptr++;
	    }
	} else if ( depth == 8 ) {
	    unsigned char *myptr=l;
	    myptr+=x1;
	    while ( w-- ) {
		(*myptr) = (*myptr) ^ pixel;
		myptr++;
	    }
	} else if ( depth == 4 || depth == 24 ) {
	    unsigned char *myptr=l;
	    while ( w-- )
		drawPointUnclipped( x1++, myptr);
	} else if ( depth == 1 ) {
	    // ...
	}
    } else if(myrop==NotROP) {
	if ( depth == 32 ) {
	    unsigned int *myptr=(unsigned int *)l + x1;
	    while ( w-- ) {
		(*myptr) = ~(*myptr);
		myptr++;
	    }
	} else if ( depth == 16 ) {
	    unsigned short int *myptr=(unsigned short int *)l;
	    myptr+=x1;
	    while ( w-- ) {
		(*myptr) = ~(*myptr);
		myptr++;
	    }
	} else if ( depth == 8 ) {
	    unsigned char *myptr=l;
	    myptr+=x1;
	    while ( w-- ) {
		(*myptr) = ~(*myptr);
		myptr++;
	    }
	} else if ( depth == 4 || depth == 24 ) {
	    unsigned char *myptr=l;
	    while ( w-- )
		drawPointUnclipped( x1++, myptr);
	} else if ( depth == 1 ) {
	    // ...
	}
    } else {
	// ... - will probably go in above clause
    }
}

#define GET_MASKED(rev) \
		    if( amonolittletest ) { \
			if(amonobitval & 0x1) { \
			    masked=FALSE; \
			} \
			amonobitval=amonobitval >> 1; \
		    } else { \
			if(amonobitval & 0x80) { \
			    masked=FALSE; \
			} \
			amonobitval=amonobitval << 1; \
			amonobitval=amonobitval & 0xff; \
		    } \
		    if(amonobitcount<7) { \
			amonobitcount++; \
		    } else { \
			amonobitcount=0; \
			if (rev) maskp--; \
			else maskp++; \
			amonobitval=*maskp; \
		    } \


/*
 l points to the start of the destination line's data.
 x1 and x2 are the start and end pixels.
 srcdata points to the source's left pixel start byte if reverse is false.
 srcdata points to the source's right pixels's start byte if reverse is true.
 reverse will only be true if the source and destination are the same buffer
 and a mask is set.
*/
template <const int depth, const int type>
GFX_INLINE void QGfxRaster<depth,type>::hImageLineUnclipped( int x1,int x2,
						    unsigned char *l,
						    unsigned char *srcdata,
						    bool reverse)
{
    int w = x2-x1+1;
    if ( depth == 32 ) {
	unsigned int *myptr=(unsigned int *)l;
	int inc = 1;
	if(!reverse) {
	    myptr+=x1;
	} else {
	    myptr+=x2;
	    inc = -1;
	}
	if ( !ismasking ) {
	    uint gv = srccol;
	    while ( w-- ) {
		if (srctype==SourceImage)
		    gv = get_value_32(srcdepth,&srcdata);
		*(myptr++) = gv;
	    }
	} else {
	    //masked 32bpp blt...
	    unsigned int gv = srccol;
	    while ( w-- ) {
		if ( srctype == SourceImage )
		    gv = get_value_32( srcdepth, &srcdata, reverse );
		bool masked = TRUE;
		GET_MASKED(reverse);
		if ( !masked )
		    *(myptr) = gv;
		myptr += inc;
	    }
	}
    } else if ( depth == 24 ) {
	unsigned char *myptr = l;
	int inc = 3;
	if(!reverse) {
	    myptr += x1*3;
	} else {
	    myptr += x2*3;
	    inc = -3;
	}
	if ( !ismasking ) {
	    uint gv = srccol;
	    while ( w-- ) {
		if (srctype==SourceImage)
		    gv = get_value_24(srcdepth,&srcdata);
		gfxSetRgb24( myptr, gv );
		myptr += 3;
	    }
	} else {
	    //masked 32bpp blt...
	    unsigned int gv = srccol;
	    while ( w-- ) {
		if ( srctype == SourceImage )
		    gv = get_value_24( srcdepth, &srcdata, reverse );
		bool masked = TRUE;
		GET_MASKED(reverse);
		if ( !masked )
		    gfxSetRgb24( myptr, gv );
		myptr += inc;
	    }
	}
    } else if ( depth == 16 ) {
	unsigned short int *myptr=(unsigned short int *)l;
	int inc = 1;
	if(!reverse) {
	    myptr+=x1;
	} else {
	    myptr+=x2;
	    inc = -1;
	}
	if(!ismasking) {
#ifdef QWS_NO_WRITE_PACKING
	    while ( w-- )
		*(myptr++)=get_value_16(srcdepth,&srcdata);
#else
	    // 64-bit writes
	    int frontadd;
	    int backadd;
	    int count;

	    calcPacking(myptr-x1,x1,x2,frontadd,backadd,count);

	    PackType dput;
# ifndef QWS_PACKING_4BYTE
	    unsigned short int * fun;
	    fun=(unsigned short int *)&dput;
# endif

	    while ( frontadd-- )
		*(myptr++)=get_value_16(srcdepth,&srcdata);
	    while ( count-- ) {
# ifdef QWS_PACKING_4BYTE
		dput = get_value_16(srcdepth,&srcdata);
		dput |= (get_value_16(srcdepth,&srcdata) << 16);
		*((PackType*)myptr) = dput;
		myptr += 2;
# else
		*fun = get_value_16(srcdepth,&srcdata);
		*(fun+1) = get_value_16(srcdepth,&srcdata);
		*(fun+2) = get_value_16(srcdepth,&srcdata);
		*(fun+3) = get_value_16(srcdepth,&srcdata);
		*((PackType*)myptr) = dput;
		myptr += 4;
# endif
	    }
	    while ( backadd-- )
		*(myptr++)=get_value_16(srcdepth,&srcdata);
#endif
	} else {
	    // Probably not worth trying to pack writes if there's a mask
	    unsigned short int gv = srccol;
	    while ( w-- ) {
		if ( srctype==SourceImage )
		    gv = get_value_16( srcdepth, &srcdata, reverse );
		bool masked = TRUE;
		GET_MASKED(reverse);
		if ( !masked )
		    *(myptr) = gv;
		myptr += inc;
	    }
	}
    } else if ( depth == 8 ) {
	unsigned char *myptr=(unsigned char *)l;
	int inc = 1;
	if(!reverse) {
	    myptr+=x1;
	} else {
	    myptr+=x2;
	    inc = -1;
	}
	if(!ismasking) {
#ifdef QWS_NO_WRITE_PACKING
	    while ( w-- )
		*(myptr++)=get_value_8(srcdepth,&srcdata);
#else
	    // 64-bit writes
	    int frontadd;
	    int backadd;
	    int count;

	    calcPacking(myptr-x1,x1,x2,frontadd,backadd,count);

	    PackType dput;
# ifndef QWS_PACKING_4BYTE
	    unsigned char *fun = (unsigned char *)&dput;
# endif
	    while ( frontadd-- )
		*(myptr++)=get_value_8(srcdepth,&srcdata);

	    while ( count-- ) {
# ifdef QWS_PACKING_4BYTE
		dput = get_value_8(srcdepth,&srcdata);
		dput |= (get_value_8(srcdepth,&srcdata) << 8);
		dput |= (get_value_8(srcdepth,&srcdata) << 16);
		dput |= (get_value_8(srcdepth,&srcdata) << 24);
		*((PackType*)myptr) = dput;
		myptr += 4;
# else
		*(fun+0) = get_value_8(srcdepth,&srcdata);
		*(fun+1) = get_value_8(srcdepth,&srcdata);
		*(fun+2) = get_value_8(srcdepth,&srcdata);
		*(fun+3) = get_value_8(srcdepth,&srcdata);
		*(fun+4) = get_value_8(srcdepth,&srcdata);
		*(fun+5) = get_value_8(srcdepth,&srcdata);
		*(fun+6) = get_value_8(srcdepth,&srcdata);
		*(fun+7) = get_value_8(srcdepth,&srcdata);
		*((PackType*)myptr) = dput;
		myptr += 8;
# endif
	    }
	    while ( backadd-- )
		*(myptr++)=get_value_8(srcdepth,&srcdata);
#endif
	} else {
	    // Probably not worth trying to pack writes if there's a mask
	    unsigned char gv = srccol;
	    while ( w-- ) {
		if ( srctype==SourceImage )
		    gv = get_value_8( srcdepth, &srcdata, reverse);
		bool masked = TRUE;
		GET_MASKED(reverse);
		if (!masked)
		    *(myptr) = gv;    //gv;
		myptr += inc;
	    }
	}
    } else if ( depth == 4 ) {
	unsigned char *dp = l;
	unsigned int gv = srccol;
	if ( reverse ) {
	    dp += (x2/2);
	    int x = x2;
	    while ( w-- ) {
		if ( srctype==SourceImage )
		    gv = get_value_4( srcdepth, &srcdata, reverse);
		bool masked = TRUE;
		if ( ismasking ) {
		    GET_MASKED(reverse);
		}
		if ( !masked || !ismasking ) {
		    int s = (x&1) << 2;
		    *dp = ( *dp & MASK4BPP(s) ) | (gv << s);
		}
		if ( !(x&1) )
		    dp--;
		x--;
	    }
	} else {
	    dp += (x1/2);
	    int x = x1;
	    while ( w-- ) {
		if ( srctype==SourceImage )
		    gv = get_value_4( srcdepth, &srcdata, reverse);
		bool masked = TRUE;
		if ( ismasking ) {
		    GET_MASKED(reverse);
		}
		if ( !masked || !ismasking ) {
		    int s = (x&1) << 2;
		    *dp = ( *dp & MASK4BPP(s) ) | (gv << s);
		}
		if ( x&1 )
		    dp++;
		x++;
	    }
	}
    } else if ( depth == 1 ) {
	// General case only implemented.
	// Lots of optimisation can be performed for special cases.
	unsigned char * dp=l;
	unsigned int gv = srccol;
	if ( reverse ) {
	    dp+=(x2/8);
	    int skipbits = 7 - (x2%8);
	    for ( int b = x1/8; b <= x2/8; b++ ) {
		unsigned char m = *dp;
		for (int i = 0; i < 8 && w; i++) {
		    if (skipbits)
			skipbits--;
		    else {
			if ( srctype == SourceImage )
			    gv = get_value_1( srcdepth, &srcdata, TRUE);
			bool masked = TRUE;
			if ( ismasking ) {
			    GET_MASKED(TRUE);
			}
			if ( !masked || !ismasking ) {
			    if (gv)
				m |= 0x80 >> i;
			    else
				m &= ~( 0x80 >> i );
			}
			w--;
		    }
		}
		*(dp--) = m;
	    }
	} else {
	    dp+=(x1/8);
	    int skipbits = x1%8;
	    for ( int b = x1/8; b <= x2/8; b++ ) {
		unsigned char m = *dp;
		for (int i = 0; i < 8 && w; i++) {
		    if (skipbits)
			skipbits--;
		    else {
			if ( srctype == SourceImage )
			    gv = get_value_1( srcdepth, &srcdata, FALSE);
			bool masked = TRUE;
			if ( ismasking ) {
			    GET_MASKED(FALSE);
			}
			if ( !masked || !ismasking ) {
			    if (gv)
				m |= 1 << i;
			    else
				m &= ~( 1 << i );
			}
			w--;
		    }
		}
		*(dp++) = m;
	    }
	}
    }
}

template <const int depth, const int type>
GFX_INLINE void QGfxRaster<depth,type>::hAlphaLineUnclipped( int x1,int x2,
						    unsigned char* l,
						    unsigned char * srcdata,
						    unsigned char * alphas)
{
    int w=x2-x1+1;
    if ( depth == 32 ) {
	// First read in the destination line
	unsigned int *myptr = (unsigned int *)l;
	unsigned int *alphaptr = (unsigned int *)alphabuf;
	unsigned char * avp=alphas;
	int loopc;

#ifdef QWS_NO_WRITE_PACKING
	unsigned int *temppos = myptr+x1;
	for ( int i = 0; i < w; i++ )
	    *(alphaptr++) = *(temppos++);
#else
	int frontadd;
	int backadd;
	int count;
	int loopc2;

	calcPacking(myptr,x1,x2,frontadd,backadd,count);

	myptr+=x1;

	unsigned int * temppos=myptr;
	int myp=0;

	PackType temp2;

	unsigned int * cp;

	for( loopc2=0;loopc2<frontadd;loopc2++ ) {
	    alphabuf[myp++]=*temppos;
	    temppos++;
	}

	for( loopc2=0;loopc2<count;loopc2++ ) {
	    temp2=*((PackType *)temppos);
	    cp=(unsigned int *)&temp2;
	    alphabuf[myp++]=*cp;
	    alphabuf[myp++]=*(cp+1);
	    temppos+=2;
	}

	for( loopc2=0;loopc2<backadd;loopc2++ ) {
	    alphabuf[myp++]=*temppos;
	    temppos++;
	}
#endif

	// Now blend with source data
	unsigned char * srcptr=srcdata;
	unsigned int srcval;

	if(srctype==SourceImage) {
	    srcptr=srcdata;
	    srcval=0; // Shut up compiler
	} else {
	    // SourcePen
	    srcval=srccol;
	}

	alphaptr = (unsigned int *)alphabuf;
	for(loopc=0;loopc<w;loopc++) {
	    int r,g,b;
	    if(srctype==SourceImage)
		srcval=get_value_32(srcdepth,&srcptr);

	    int av;
	    if(alphatype==InlineAlpha) {
		av = srcval >> 24;
	    } else if(alphatype==SolidAlpha) {
		av=calpha;
	    } else {
		av=*(avp++);
	    }

	    r = (srcval & 0xff0000) >> 16;
	    g = (srcval & 0xff00) >> 8;
	    b = srcval & 0xff;

	    unsigned char * tmp=(unsigned char *)&alphabuf[loopc];
	    if(av==255) {
	        // Do nothing - we already have source values in r,g,b
	    } else if(av==0) {
	        r = *(tmp+2);
	        g = *(tmp+1);
	        b = *(tmp+0);
	    } else {
		r = ((r-*(tmp+2)) * av) / 256 + *(tmp+2);
		g = ((g-*(tmp+1)) * av) / 256 + *(tmp+1);
		b = ((b-*(tmp+0)) * av) / 256 + *(tmp+0);
	    }
	    *(alphaptr++) = (r << 16) | (g << 8) | b;
	}

	// Now write it all out
	alphaptr = (unsigned int *)alphabuf;

#ifdef QWS_NO_WRITE_PACKING
	myptr += x1;
	while ( w-- )
	    *(myptr++)=*(alphaptr++);
#else
	PackType put;
	unsigned int *fun = (unsigned int*)&put;

	myptr=(unsigned int *)l;

	calcPacking(myptr,x1,x2,frontadd,backadd,count);

	myptr+=x1;

	for ( loopc2=0;loopc2<frontadd;loopc2++ ) {
	    *(myptr++)=*(alphaptr++);
	}

	for ( loopc2=0;loopc2<count;loopc2++ ) {
	    *(fun) = *(alphaptr++);
	    *(fun+1) = *(alphaptr++);
	    *((PackType*)myptr) = put;
	    myptr += 2;
	}

	for ( loopc2=0;loopc2<backadd;loopc2++ ) {
	    *(myptr++)=*(alphaptr++);
	}
#endif
    } else if ( depth == 24 ) {
	// First read in the destination line
	unsigned char *myptr = l;
	unsigned char *alphaptr = (unsigned char *)alphabuf;
	unsigned char *avp = alphas;
	int loopc;

	memcpy( alphabuf, myptr+x1*3, w*3 );

	// Now blend with source data
	unsigned char * srcptr=srcdata;
	unsigned int srcval;

	if(srctype==SourceImage) {
	    srcptr=srcdata;
	    srcval=0; // Shut up compiler
	} else {
	    // SourcePen
	    srcval=srccol;
	}

	alphaptr = (unsigned char *)alphabuf;
	for(loopc=0;loopc<w;loopc++) {
	    int r,g,b;
	    if(srctype==SourceImage)
		srcval=get_value_32(srcdepth,&srcptr);

	    int av;
	    if(alphatype==InlineAlpha) {
		av = srcval >> 24;
	    } else if(alphatype==SolidAlpha) {
		av=calpha;
	    } else {
		av=*(avp++);
	    }

	    r = (srcval & 0xff0000) >> 16;
	    g = (srcval & 0xff00) >> 8;
	    b = srcval & 0xff;

	    unsigned char *tmp = alphaptr;
	    if ( av == 255 ) {
	        // Do nothing - we already have source values in r,g,b
	    } else if ( av == 0 ) {
	        r = *(tmp+2);
	        g = *(tmp+1);
	        b = *(tmp+0);
	    } else {
		r = ((r-*(tmp+2)) * av) / 256 + *(tmp+2);
		g = ((g-*(tmp+1)) * av) / 256 + *(tmp+1);
		b = ((b-*(tmp+0)) * av) / 256 + *(tmp+0);
	    }
	    gfxSetRgb24( alphaptr, r, g, b );
	    alphaptr += 3;
	}

	// Now write it all out
	memcpy( myptr+x1*3, alphabuf, w*3 );
#if !defined( QT_NO_IMAGE_16_BIT ) || !defined( QT_NO_QWS_DEPTH_16 )
    } else if ( depth == 16 ) {
        // First read in the destination line
	unsigned short int *myptr = (unsigned short int *)l;
	unsigned int *alphaptr = (unsigned int *)alphabuf;
	unsigned char * avp=alphas;
	int loopc;

#ifdef QWS_NO_WRITE_PACKING
	unsigned short int *temppos = myptr + x1;
	for ( int i = 0; i < w; i++ )
	    *(alphaptr++) = get_value_32(16,(unsigned char **)&temppos);
#else
	int frontadd;
	int backadd;
	int count;
	int loopc2;

	calcPacking(myptr,x1,x2,frontadd,backadd,count);

	myptr+=x1;

	int myp=0;
	unsigned short int * temppos=myptr;

	PackType temp2;

	unsigned char * cp;

	for( loopc2=0;loopc2<frontadd;loopc2++ ) {
	    alphabuf[myp++]=get_value_32(16,(unsigned char **)&temppos);
	}

	for( loopc2=0;loopc2<count;loopc2++ ) {
	    temp2=*((PackType *)temppos);
	    cp=(unsigned char *)&temp2;
	    alphabuf[myp++]=get_value_32(16,&cp);
	    alphabuf[myp++]=get_value_32(16,&cp);
# ifdef QWS_PACKING_4BYTE
	    temppos+=2;
# else
	    alphabuf[myp++]=get_value_32(16,&cp);
	    alphabuf[myp++]=get_value_32(16,&cp);
	    temppos+=4;
#endif
	}

	for( loopc2=0;loopc2<backadd;loopc2++ ) {
	    alphabuf[myp++]=get_value_32(16,(unsigned char **)&temppos);
	}
#endif

	// Now blend with source data
	unsigned char * srcptr=srcdata;
	unsigned int srcval;

	if(srctype==SourceImage) {
	    srcptr=srcdata;
	    srcval=0; // Shut up compiler
	} else {
	    // SourcePen
	    srcval=qt_conv16ToRgb(srccol);
	}
	alphaptr = (unsigned int *)alphabuf;
	for(loopc=0;loopc<w;loopc++) {
	    int r,g,b;
	    if(srctype==SourceImage)
		srcval=get_value_32(srcdepth,&srcptr);

	    int av;
	    if(alphatype==InlineAlpha) {
		av = srcval >> 24;
	    } else if(alphatype==SolidAlpha) {
		av=calpha;
	    } else {
		av=*(avp++);
	    }

	    r = (srcval & 0xff0000) >> 16;
	    g = (srcval & 0xff00) >> 8;
	    b = srcval & 0xff;

	    unsigned char * tmp=(unsigned char *)&alphabuf[loopc];
	    if(av==255) {
	        // Do nothing - we already have source values in r,g,b
	    } else if(av==0) {
	        r=*(tmp+2);
	        g=*(tmp+1);
	        b=*(tmp+0);
	    } else {
		r = ((r-*(tmp+2)) * av) / 256 + *(tmp+2);
		g = ((g-*(tmp+1)) * av) / 256 + *(tmp+1);
		b = ((b-*(tmp+0)) * av) / 256 + *(tmp+0);
	    }
	    *(alphaptr++) = qt_convRgbTo16(r,g,b);
	}

	// Now write it all out

	alphaptr = (unsigned int *)alphabuf;

#ifdef QWS_NO_WRITE_PACKING
	myptr += x1;
	while ( w-- )
	    *(myptr++) = *(alphaptr++);
#else
	PackType put;
# ifndef QWS_PACKING_4BYTE
	unsigned short int *fun = (unsigned short int*)&put;
# endif
	myptr=(unsigned short int *)l;

	calcPacking(myptr,x1,x2,frontadd,backadd,count);

	myptr+=x1;

	for ( loopc2=0;loopc2<frontadd;loopc2++ ) {
	    *(myptr++)=*(alphaptr++);
	}

	for ( loopc2=0;loopc2<count;loopc2++ ) {
# ifdef QWS_PACKING_4BYTE
	    put = *(alphaptr++);
	    put |= (*(alphaptr++) << 16);
	    *((PackType*)myptr) = put;
	    myptr += 2;
# else
	    *(fun) = *(alphaptr++);
	    *(fun+1) = *(alphaptr++);
	    *(fun+2) = *(alphaptr++);
	    *(fun+3) =  *(alphaptr++);
	    *((PackType*)myptr) = put;
	    myptr += 4;
# endif
	}

	for ( loopc2=0;loopc2<backadd;loopc2++ ) {
	    *(myptr++)=*(alphaptr++);
	}
#endif
#endif
    } else if ( depth == 8 ) {
        // First read in the destination line
	unsigned char * myptr;
	myptr=l;
	myptr+=x1;
	int loopc;

	unsigned char * avp=alphas;

	unsigned char * tempptr=myptr;

        for(loopc=0;loopc<w;loopc++) {
	    int val = *tempptr++;
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
	    alphabuf[loopc] = (val << 16) | (val << 8) | val;
#else
	    alphabuf[loopc] = clut[val];
#endif
	}

	// Now blend with source data

	unsigned char * srcptr;
	unsigned int srcval = 0;
	if(srctype==SourceImage) {
	    srcptr=srcdata;
	} else {
	    // SourcePen
	    QRgb mytmp=clut[srccol];
	    srcval=qRed(mytmp) << 16 | qGreen(mytmp) << 8 | qBlue(mytmp);
	}
#ifdef QT_NEED_SIMPLE_ALLOC
	simple_8bpp_alloc=TRUE;
#endif
	for(loopc=0;loopc<w;loopc++) {
	    int r,g,b;
	    if(srctype==SourceImage)
		srcval=get_value_32(srcdepth,&srcptr);

	    int av;
	    if(alphatype==InlineAlpha) {
		av=srcval >> 24;
	    } else if(alphatype==SolidAlpha) {
		av=calpha;
	    } else {
		av=*(avp++);
	    }

	    r = (srcval & 0xff0000) >> 16;
	    g = (srcval & 0xff00) >> 8;
	    b = srcval & 0xff;

	    unsigned char * tmp=(unsigned char *)&alphabuf[loopc];

	    if(av==255) {
		*myptr = GFX_8BPP_PIXEL(r,g,b);
	    } else if ( av > 0 ) {
		r = ((r-*(tmp+2)) * av) / 256 + *(tmp+2);
		g = ((g-*(tmp+1)) * av) / 256 + *(tmp+1);
		b = ((b-*(tmp+0)) * av) / 256 + *(tmp+0);
		*myptr = GFX_8BPP_PIXEL(r,g,b);
	    }
	    myptr++;
	}
#ifdef QT_NEED_SIMPLE_ALLOC
	simple_8bpp_alloc=FALSE;
#endif
    } else if ( depth == 4 ) {
        // First read in the destination line
	unsigned char *myptr = l;
	myptr+=x1/2;

	unsigned char *avp=alphas;
	unsigned char *tempptr=myptr;

	int loopc = 0;
	if ( x1&1 ) {
	    int val = *tempptr++;
	    alphabuf[loopc++] = clut[(val & 0xf0) >> 4];
	}

        for ( ;loopc < w-1; loopc += 2 ) {
	    int val = *tempptr++;
	    alphabuf[loopc] = clut[val & 0x0f];
	    alphabuf[loopc+1] = clut[val >> 4];
	}

	if ( !(x2&1) ) {
	    int val = *tempptr;
	    alphabuf[w-1] = clut[val & 0x0f];
	}

	// Now blend with source data

	unsigned char * srcptr;
	unsigned int srcval = 0;
	if(srctype==SourceImage) {
	    srcptr=srcdata;
	} else {
	    // SourcePen
	    QRgb mytmp=clut[srccol];
	    srcval=qRed(mytmp) << 16 | qGreen(mytmp) << 8 | qBlue(mytmp);
	}
	for(loopc=0;loopc<w;loopc++) {
	    int r,g,b;
	    if(srctype==SourceImage)
		srcval=get_value_32(srcdepth,&srcptr);

	    int av;
	    if(alphatype==InlineAlpha) {
		av=srcval >> 24;
	    } else if(alphatype==SolidAlpha) {
		av=calpha;
	    } else {
		av=*(avp++);
	    }

	    r = (srcval & 0xff0000) >> 16;
	    g = (srcval & 0xff00) >> 8;
	    b = srcval & 0xff;

	    unsigned char *tmp = (unsigned char *)&alphabuf[loopc];

	    if(av==255) {
		alphabuf[loopc] = qRgb(r,g,b);
	    } else {
		r = ((r-*(tmp+2)) * av) / 256 + *(tmp+2);
		g = ((g-*(tmp+1)) * av) / 256 + *(tmp+1);
		b = ((b-*(tmp+0)) * av) / 256 + *(tmp+0);
		alphabuf[loopc] = qRgb(r,g,b);
	    }
	}

	loopc = 0;
	if ( x1&1 ) {
	    QRgb rgb = alphabuf[loopc++];
	    *myptr++ = (*myptr & 0x0f) |
		(qt_screen->alloc( qRed(rgb), qGreen(rgb), qBlue(rgb) ) << 4);
	}

        for ( ;loopc < w-1; loopc += 2 ) {
	    QRgb rgb1 = alphabuf[loopc];
	    QRgb rgb2 = alphabuf[loopc+1];
	    *myptr++ = qt_screen->alloc( qRed(rgb1), qGreen(rgb1), qBlue(rgb1) ) |
		(qt_screen->alloc( qRed(rgb2), qGreen(rgb2), qBlue(rgb2) ) << 4);
	}

	if ( !(x2&1) ) {
	    QRgb rgb = alphabuf[w-1];
	    *myptr = (*myptr & 0xf0) |
		qt_screen->alloc( qRed(rgb), qGreen(rgb), qBlue(rgb) );
	}

    } else if ( depth == 1 ) {
	static int warn;
	if ( warn++ < 5 )
	    qDebug( "bitmap alpha not implemented" );
    }
}

//widget coordinates
template <const int depth, const int type>
void QGfxRaster<depth,type>::fillRect( int rx,int ry,int w,int h )
{
    if((*optype))
	sync();
    (*optype)=0;
    setAlphaType(IgnoreAlpha);
    if ( w <= 0 || h <= 0 ) return;

    GFX_START(QRect(rx+xoffs, ry+yoffs, w+1, h+1))

#ifdef QWS_EXPERIMENTAL_FASTPATH
    // ### fix for 8bpp
    // This seems to be reliable now, at least for 16bpp

    if (ncliprect == 1 && cbrush.style()==SolidPattern) {
	// Fast path
	if(depth==16) {
	    pixel=cbrush.color().pixel();
	    int x1,y1,x2,y2;
	    rx+=xoffs;
	    ry+=yoffs;
	    x2=rx+w-1;
	    y2=ry+h-1;
	    if(rx>cliprect[0].right() || ry>cliprect[0].bottom() ||
	       x2<cliprect[0].left() || y2<cliprect[0].top()) {
		GFX_END
	        return;
	    }
	    x1=cliprect[0].left() > rx ? cliprect[0].left() : rx;
	    y1=cliprect[0].top() > ry ? cliprect[0].top() : ry;
	    x2=cliprect[0].right() > x2 ? x2 : cliprect[0].right();
	    y2=cliprect[0].bottom() > y2 ? y2 : cliprect[0].bottom();
	    w=(x2-x1)+1;
	    h=(y2-y1)+1;

	    if(w<1 || h<1) {
		GFX_END
		return;
	    }

	    unsigned short int * myptr=(unsigned short int *)scanLine(y1);

	// 64-bit writes make a /big/ difference from 32-bit ones,
	// at least on my (fast AGP) hardware - 856 rects/second as opposed
	// to 550, although MTRR makes this difference much less
	    int frontadd;
	    int backadd;
	    int count;
	    calcPacking(myptr,x1,x2,frontadd,backadd,count);

	    int loopc,loopc2;
	    PackType put;
# ifdef QWS_PACKING_4BYTE
	    put = pixel | (pixel << 16);
	    int add=linestep()/2;
	    add-=(frontadd+(count * 2)+backadd);
# else
	    unsigned short int * sp=(unsigned short int *)&put;
	    *sp=pixel;
	    *(sp+1)=pixel;
	    *(sp+2)=pixel;
	    *(sp+3)=pixel;

	    int add=linestep()/2;
	    add-=(frontadd+(count * 4)+backadd);
# endif

	    myptr=((unsigned short int *)scanLine(y1))+x1;
	    for(loopc=0;loopc<h;loopc++) {
		for(loopc2=0;loopc2<frontadd;loopc2++)
		    *(myptr++)=pixel;
		for(loopc2=0;loopc2<count;loopc2++) {
		    *((PackType *)myptr)=put;
# ifdef QWS_PACKING_4BYTE
		    myptr+=2;
# else
		    myptr+=4;
# endif
		}
		for(loopc2=0;loopc2<backadd;loopc2++)
		    *(myptr++)=pixel;
		myptr+=add;
	    }
	    GFX_END
	    return;
	} else if(depth==32) {
	    pixel=cbrush.color().pixel();
	    int x1,y1,x2,y2;
	    rx+=xoffs;
	    ry+=yoffs;
	    x2=rx+w-1;
	    y2=ry+h-1;
	    if(rx>cliprect[0].right() || ry>cliprect[0].bottom() ||
	       x2<cliprect[0].left() || y2<cliprect[0].top()) {
		GFX_END
		return;
	    }
	    x1=cliprect[0].left() > rx ? cliprect[0].left() : rx;
	    y1=cliprect[0].top() > ry ? cliprect[0].top() : ry;
	    x2=cliprect[0].right() > x2 ? x2 : cliprect[0].right();
	    y2=cliprect[0].bottom() > y2 ? y2 : cliprect[0].bottom();
	    w=(x2-x1)+1;
	    h=(y2-y1)+1;

	    if(w<1 || h<1) {
		GFX_END
		return;
	    }

	    unsigned int * myptr=(unsigned int *)scanLine(y1);

	    int frontadd;
	    int backadd;
	    int count;
	    calcPacking(myptr,x1,x2,frontadd,backadd,count);

	    int loopc,loopc2;
	    PackType put;
	    unsigned int * sp=(unsigned int *)&put;
	    *sp=pixel;
	    *(sp+1)=pixel;

	    int add=linestep()/4;
	    add-=(frontadd+(count * 2)+backadd);

	    myptr=((unsigned int *)scanLine(y1))+x1;

	    for(loopc=0;loopc<h;loopc++) {
		for(loopc2=0;loopc2<frontadd;loopc2++)
		    *(myptr++)=pixel;
		for(loopc2=0;loopc2<count;loopc2++) {
		    *((PackType *)myptr)=put;
		    myptr+=2;
		}
		for(loopc2=0;loopc2<backadd;loopc2++)
		    *(myptr++)=pixel;
		myptr+=add;
	    }
	    GFX_END
	    return;
	} else if(depth==8) {
	    useBrush();
	    int x1,y1,x2,y2;
	    rx+=xoffs;
	    ry+=yoffs;
	    x2=rx+w-1;
	    y2=ry+h-1;
	    if(rx>cliprect[0].right() || ry>cliprect[0].bottom() ||
	       x2<cliprect[0].left() || y2<cliprect[0].top()) {
		GFX_END
	        return;
	    }
	    x1=cliprect[0].left() > rx ? cliprect[0].left() : rx;
	    y1=cliprect[0].top() > ry ? cliprect[0].top() : ry;
	    x2=cliprect[0].right() > x2 ? x2 : cliprect[0].right();
	    y2=cliprect[0].bottom() > y2 ? y2 : cliprect[0].bottom();
	    w=(x2-x1)+1;
	    h=(y2-y1)+1;

	    if(w<1 || h<1) {
		GFX_END
		return;
	    }

	    unsigned char * myptr=(unsigned char *)scanLine(y1);

	    int frontadd;
	    int backadd;
	    int count;
	    calcPacking(myptr,x1,x2,frontadd,backadd,count);

	    int loopc,loopc2;
	    PackType put;
# ifdef QWS_PACKING_4BYTE
	    if ( count )
		put = pixel | (pixel<<8) | (pixel<<16) | (pixel<<24);

	    int add=linestep();
	    add-=(frontadd+(count * 4)+backadd);
# else
	    if ( count ) {
		unsigned char * sp=(unsigned char *)&put;
		*sp=pixel;
		*(sp+1)=pixel;
		*(sp+2)=pixel;
		*(sp+3)=pixel;
		*(sp+4)=pixel;
		*(sp+5)=pixel;
		*(sp+6)=pixel;
		*(sp+7)=pixel;
	    }

	    int add=linestep();
	    add-=(frontadd+(count * 8)+backadd);
# endif

	    myptr=((unsigned char *)scanLine(y1))+x1;
	    for(loopc=0;loopc<h;loopc++) {
		for(loopc2=0;loopc2<frontadd;loopc2++)
		    *(myptr++)=pixel;
		for(loopc2=0;loopc2<count;loopc2++) {
		    *((PackType *)myptr)=put;
# ifdef QWS_PACKING_4BYTE
		    myptr+=4;
# else
		    myptr+=8;
# endif
		}
		for(loopc2=0;loopc2<backadd;loopc2++)
		    *(myptr++)=pixel;
		myptr+=add;
	    }
	    GFX_END
	    return;
	} else {

	}
    }
#endif // QWS_EXPERIMENTAL_FASTPATH

    if( (cbrush.style()!=QBrush::NoBrush) &&
	(cbrush.style()!=QBrush::SolidPattern) ) {
	srcwidth=cbrushpixmap->width();
	srcheight=cbrushpixmap->height();
	if(cbrushpixmap->depth()==1) {
	    if(opaque) {
		setSource(cbrushpixmap);
		setAlphaType(IgnoreAlpha);
		useBrush();
		srcclut[0]=pixel;
		QBrush tmp=cbrush;
		cbrush=QBrush(backcolor);
		useBrush();
		srcclut[1]=pixel;
		cbrush=tmp;
	    } else {
		useBrush();
		srccol=pixel;
		srctype=SourcePen;
		setAlphaType(LittleEndianMask);
		setAlphaSource(cbrushpixmap->scanLine(0),
			       cbrushpixmap->bytesPerLine());
	    }
	} else {
	    setSource(cbrushpixmap);
	    setAlphaType(IgnoreAlpha);
	}
	tiledBlt(rx,ry,w,h);
    } else if(cbrush.style()!=NoBrush) {
	useBrush();
	rx += xoffs;
	ry += yoffs;
	// Gross clip
	if ( rx < clipbounds.left() ) {
	    w -= clipbounds.left()-rx;
	    rx = clipbounds.left();
	}
	if ( ry < clipbounds.top() ) {
	    h -= clipbounds.top()-ry;
	    ry = clipbounds.top();
	}
	if ( rx+w-1 > clipbounds.right() )
	    w = clipbounds.right()-rx+1;
	if ( ry+h-1 > clipbounds.bottom() )
	    h = clipbounds.bottom()-ry+1;
	if ( w > 0 && h > 0 )
	    for (int j=0; j<h; j++,ry++)
		hline(rx,rx+w-1,ry);
    }
    GFX_END
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::drawPolyline( const QPointArray &a,int index, int npoints )
{
    if(cpen.style()==NoPen)
	return;
    if (cpen.width() > 1) {
	drawThickPolyline( a, index, npoints );
	return;
    }

    if((*optype))
	sync();
    (*optype)=0;
    //int m=QMIN( index+npoints-1, int(a.size())-1 );
#ifndef QT_NO_QWS_CURSOR
    GFX_START(a.boundingRect())
#else
    GFX_START(clipbounds)
#endif
    int loopc;
    int end;
    end=(index+npoints) > (int)a.size() ? a.size() : index+npoints;
    for(loopc=index+1;loopc<end;loopc++) {
	    drawLine(a[loopc-1].x(),a[loopc-1].y(),
		     a[loopc].x(),a[loopc].y());
    }
    // XXX beware XOR mode vertices
    GFX_END
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::drawPolygon( const QPointArray &pa, bool winding, int index, int npoints )
{
    if((*optype)!=0) {
	sync();
    }
    (*optype)=0;
    useBrush();
    GFX_START(clipbounds)
    if ( cbrush.style()!=QBrush::NoBrush )
	scan(pa,winding,index,npoints);
    drawPolyline(pa, index, npoints);
    if (pa[index] != pa[index+npoints-1]) {
	drawLine(pa[index].x(), pa[index].y(),
		pa[index+npoints-1].x(),pa[index+npoints-1].y());
    }
    GFX_END
}

// widget coords
template <const int depth, const int type>
void QGfxRaster<depth,type>::processSpans( int n, QPoint* point, int* width )
{
    while (n--) {
	int x=point->x()+xoffs;
	if ( *width > 0 ) {
	    if(patternedbrush) {
		// XXX
	    } else {
		hline(x,x+*width-1,point->y()+yoffs);
	    }
	}
	point++;
	width++;
    }
}

/*
  Finds a pointer to pixel (\a x, \a y) in a bitmap that
  is \a w pixels wide and stored in \a base. \a is_bigendian determines
  endianness.

  \a astat returns the bit number within the byte
  \a ahold holds the \c monobitval which is the byte pre-shifted
           to match the algoritm using this function

 */
static GFX_INLINE unsigned char * find_pointer(unsigned char * base,int x,int y,
					   int w, int linestep, int &astat,
					   unsigned char &ahold,
					   bool is_bigendian, bool rev)
{
    int nbits;
    int nbytes;

    if ( rev ) {
	is_bigendian = !is_bigendian;
	nbits = 7 - (x+w) % 8;
       	nbytes = (x+w) / 8;
    } else {
	nbits = x % 8;
       	nbytes = x / 8;
    }

    astat=nbits;

    unsigned char *ret = base + (y*linestep) + nbytes;

    ahold=*ret;
    if(is_bigendian) {
	ahold=ahold << nbits;
    } else {
	ahold=ahold >> nbits;
    }

    return ret;
}

static GFX_INLINE unsigned char *find_pointer_4( unsigned char * base,int x,int y,
						int w, int linestep, int &astat,
						unsigned char &ahold, bool rev )
{
    int nbits;
    int nbytes;

    if ( rev ) {
	nbits = 1 - (x+w) % 2;
	nbytes = (x+w) / 2;
    } else {
	nbits = x % 2;
	nbytes = x / 2;
    }

    unsigned char *ret = base + (y*linestep) + nbytes;
    astat = nbits;
    ahold = *ret;

    if ( rev )
	ahold = ahold << (nbits*4);
    else
	ahold = ahold >> (nbits*4);

    return ret;
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::scroll( int rx,int ry,int w,int h,int sx, int sy )
{
    if (!w || !h)
	return;

    int dy = sy - ry;
    int dx = sx - rx;

    if (dx == 0 && dy == 0)
	return;

    GFX_START(QRect(QMIN(rx+xoffs,sx+xoffs), QMIN(ry+yoffs,sy+yoffs), w+QABS(dx)+1, h+QABS(dy)+1))

    srcbits=buffer;
    src_normal_palette = TRUE;
    srclinestep=linestep();
    srcdepth=depth;
    srcwidth=w;
    srcheight=h;
    if(srcdepth==0)
	abort();
    srctype=SourceImage;
    setAlphaType(IgnoreAlpha);
    setSourceWidgetOffset( xoffs, yoffs );
    blt(rx,ry,w,h,sx,sy);

    GFX_END
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::blt( int rx,int ry,int w,int h, int sx, int sy )
{
    if ( !w || !h ) return;
    if((*optype)!=0)
	sync();
    (*optype)=0;
    int osrcdepth=srcdepth;
    if(srctype==SourcePen) {
	srclinestep=0;//w;
	srcdepth=0;
	usePen();
    }

    rx += xoffs;
    ry += yoffs;
    QRect cr;

    // Very gross clip
    if ( !clipbounds.intersects(QRect(rx,ry,w,h)) )
	return;

    QRect cursRect(rx, ry, w+1, h+1);

    GFX_START(cursRect);

    QPoint srcoffs = srcwidgetoffs + QPoint( sx, sy );

    int dl = linestep();
    int sl = srclinestep;
    int dj = 1;
    int dry = 1;
    int tj;
    int j;
    if ( srcoffs.y() < ry && srcbits == buffer ) {
	// Back-to-front
	dj = -dj;
	dl = -dl;
	sl = -sl;
	dry = -dry;
	j = h-1;
	ry=(ry+h)-1;
	tj = -1;
    } else {
	j = 0;
	tj = h;
    }

    unsigned char *l = scanLine(ry);
    unsigned char *srcline = srcScanLine(j+srcoffs.y());

    // Fast path for 8/16/32 bit same-depth opaque blit. (ie. the common case)
    if ( srcdepth == depth && alphatype == IgnoreAlpha &&
	 (depth > 8 || (depth == 8 && src_normal_palette)) ) {
	int bytesPerPixel = depth/8;
#ifdef QWS_SLOW_GFX_MEMORY
	unsigned char *buffer = 0;
	bool video2video = is_screen_gfx && srcbits == buffer;
	if ( video2video )
	    buffer = new unsigned char [w * bytesPerPixel];
#endif
	for (; j!=tj; j+=dj,ry+=dry,l+=dl,srcline+=sl) {
	    bool plot=inClip(rx,ry,&cr);
	    int x=rx;
	    for (;;) {
		int x2 = cr.right();
		if ( x2 > rx+w-1 ) {
		    x2 = rx+w-1;
		    if ( x2 < x ) break;
		}
		if (plot) {
		    unsigned char *srcptr=srcline+(x-rx+srcoffs.x())*bytesPerPixel;
		    unsigned char *destptr = l + x*bytesPerPixel;
#ifdef QWS_SLOW_GFX_MEMORY
		    if ( video2video ) {
			memcpy(buffer, srcptr, (x2-x+1) * bytesPerPixel );
			memcpy(destptr, buffer, (x2-x+1) * bytesPerPixel );
		    } else
#endif
		    memmove(destptr, srcptr, (x2-x+1) * bytesPerPixel );
		}
		if ( x >= rx+w-1 )
		    break;
		x=x2+1;
		plot=inClip(x,ry,&cr,plot);
	    }
	}
#ifdef QWS_SLOW_GFX_MEMORY
	if ( buffer )
	    delete [] buffer;
#endif
    } else {
	if ( alphatype == InlineAlpha || alphatype == SolidAlpha ||
	     alphatype == SeparateAlpha ) {
	    alphabuf = new unsigned int[w];
	}

	// reverse will only ever be true if the source and destination
	// are the same buffer.
	bool reverse = srcoffs.y()==ry && rx>srcoffs.x() &&
			srctype==SourceImage && srcbits == buffer;

	if ( alphatype == LittleEndianMask || alphatype == BigEndianMask ) {
	    // allows us to optimise GET_MASK a little
	    amonolittletest = FALSE;
	    if( (alphatype==LittleEndianMask && !reverse) ||
		(alphatype==BigEndianMask && reverse) ) {
		amonolittletest = TRUE;
	    }
	}

	unsigned char *srcptr = 0;
	for (; j!=tj; j+=dj,ry+=dry,l+=dl,srcline+=sl) {
	    bool plot=inClip(rx,ry,&cr);
	    int x=rx;
	    for (;;) {
		int x2 = cr.right();
		if ( x2 > rx+w-1 ) {
		    x2 = rx+w-1;
		    if ( x2 < x ) break;
		}
		if (plot) {
		    if ( srctype == SourceImage ) {
			if ( srcdepth == 1) {
			    srcptr=find_pointer(srcbits,(x-rx)+srcoffs.x(),
					 j+srcoffs.y(), x2-x, srclinestep,
					 monobitcount, monobitval,
					 !src_little_endian, reverse);
			} else if ( srcdepth == 4) {
			    srcptr = find_pointer_4(srcbits,(x-rx)+srcoffs.x(),
					 j+srcoffs.y(), x2-x, srclinestep,
					 monobitcount, monobitval, reverse);
			} else if ( reverse )
			    srcptr = srcline + (x2-rx+srcoffs.x())*srcdepth/8;
			else
			    srcptr = srcline + (x-rx+srcoffs.x())*srcdepth/8;
		    }
		    switch ( alphatype ) {
		      case LittleEndianMask:
		      case BigEndianMask:
			maskp=find_pointer(alphabits,(x-rx)+srcoffs.x(),
					   j+srcoffs.y(), x2-x, alphalinestep,
					   amonobitcount,amonobitval,
					   alphatype==BigEndianMask, reverse);
			// Fall through
		      case IgnoreAlpha:
			hImageLineUnclipped(x,x2,l,srcptr,reverse);
			break;
		      case InlineAlpha:
		      case SolidAlpha:
			hAlphaLineUnclipped(x,x2,l,srcptr,0);
			break;
		      case SeparateAlpha:
			// Separate alpha table
			unsigned char * alphap=alphabits+(j*alphalinestep)
					       +(x-rx);
			hAlphaLineUnclipped(x,x2,l,srcptr,alphap);
		    }
		}
		if ( x >= rx+w-1 )
		    break;
		x=x2+1;
		plot=inClip(x,ry,&cr,plot);
	    }
	}
	if ( alphabuf ) {
	    delete [] alphabuf;
	    alphabuf = 0;
	}
    }

    srcdepth=osrcdepth;
    GFX_END
}

#if !defined(QT_NO_MOVIE) || !defined(QT_NO_TRANSFORMATIONS)
template <const int depth, const int type>
void QGfxRaster<depth,type>::stretchBlt( int rx,int ry,int w,int h,
					 int sw,int sh )
{
    if((*optype))
	sync();
    (*optype)=0;
    QRect cr;
    unsigned char * srcptr;
    unsigned char * data = new unsigned char [(w*depth)/8];
    rx+=xoffs;
    ry+=yoffs;
    //int sy=0;
    unsigned char * l=scanLine(ry);
    unsigned char * sl=data;
    double xfac=sw;
    xfac=xfac/((double)w);
    double yfac=sh;
    yfac=yfac/((double)h);

    int loopc;

    // We don't allow overlapping stretchblt src and destination

    int mulfac;
    if(srcdepth==32) {
	mulfac=4;
    } else if(srcdepth==24) {
	mulfac=3;
    } else if(srcdepth==16) {
	mulfac=2;
    } else if(srcdepth==8) {
	mulfac=1;
    } else {
	mulfac=0;
	qDebug("Can't cope with stretchblt source depth %d",mulfac);
	return;
    }

    QPoint srcoffs = srcwidgetoffs; // + QPoint( sx, sy );

    QRect cursRect(rx, ry, w+1, h+1);
    /* ???
    if (buffer_offset >= 0 && src_buffer_offset >= 0) {
	cursRect = QRect( QMIN(rx,srcoffs.x()), QMIN(ry,srcoffs.y()),
			QMAX(w, sw)+QABS(rx - srcoffs.x())+1,
			QMAX(h, sh)+QABS(ry - srcoffs.y())+1 );
    } else if (src_buffer_offset >= 0) {
	cursRect = QRect(srcoffs.x(), srcoffs.y(), sw+1, sh+1);
    }
    */

    GFX_START(cursRect);

    int osrcdepth=srcdepth;
    int pyp=-1;

    for(int j=0;j<h;j++,ry++,l+=linestep()) {
	bool plot=inClip(rx,ry,&cr);
	int x=rx;

	int yp=(int) ( ( (double) j )*yfac );

	if(yp!=pyp) {
	    for(loopc=0;loopc<w;loopc++) {
		int sp=(int) ( ( (double) loopc )*xfac );
		unsigned char * p=srcScanLine(yp)+(sp*mulfac);
		if(depth==32) {
		    unsigned int val=get_value_32(srcdepth,&p);
		    unsigned int * dp=(unsigned int *)data;
		    *(dp+loopc)=val;
		} else if(depth==24) {
		    unsigned int val=get_value_32(srcdepth,&p);
		    unsigned char* dp=(unsigned char *)data;
		    gfxSetRgb24( dp+loopc*3, val);
		} else if(depth==16) {
		    unsigned int val=get_value_16(srcdepth,&p);
		    unsigned short int * dp=(unsigned short int *)data;
		    *(dp+loopc)=val;
		} else if(depth==8) {
		    unsigned int val=get_value_8(srcdepth,&p);
		    *(data+loopc)=val;
		} else {
		    qDebug("Can't cope with stretchblt depth %d",depth);
		    GFX_END
		    delete [] data;
		    return;
		}
	    }
	    pyp=yp;
	}

	srcdepth=depth;
	for (;;) {
	    int x2 = cr.right();
	    if ( x2 >= rx+w-1 ) {
		srcptr=sl;
		srcptr+=(((x-rx)+srcoffs.x())*mulfac);
		if (plot) {
		    hImageLineUnclipped(x,rx+w-1,l,srcptr,FALSE);
		}
		break;
	    } else {
		srcptr=sl;
		srcptr+=(((x-rx)+(srcoffs.x()))*mulfac);
		if (plot) {
			hImageLineUnclipped(x,x2,l,srcptr,FALSE);
		}
		x=x2+1;
		plot=inClip(x,ry,&cr,plot);
	    }
	}
	srcdepth=osrcdepth;
    }
    delete [] data;
    GFX_END
}
#endif

template <const int depth, const int type>
void QGfxRaster<depth,type>::tiledBlt( int rx,int ry,int w,int h )
{
    GFX_START(QRect(rx+xoffs, ry+yoffs, w+1, h+1))

    useBrush();
    unsigned char * savealphabits=alphabits;

    int offx = srcwidgetoffs.x() + brushoffs.x();
    int offy = srcwidgetoffs.y() + brushoffs.y();

    // from qpainter_qws.cpp
    if ( offx < 0 )
        offx = srcwidth - -offx % srcwidth;
    else
        offx = offx % srcwidth;
    if ( offy < 0 )
        offy = srcheight - -offy % srcheight;
    else
        offy = offy % srcheight;

    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = ry;
    yOff = offy;
    while( yPos < ry + h ) {
        drawH = srcheight - yOff;    // Cropping first row
        if ( yPos + drawH > ry + h )        // Cropping last row
            drawH = ry + h - yPos;
        xPos = rx;
        xOff = offx;
        while( xPos < rx + w ) {
            drawW = srcwidth - xOff; // Cropping first column
            if ( xPos + drawW > rx + w )    // Cropping last column
                drawW = rx + w - xPos;
	    blt(xPos, yPos, drawW, drawH,xOff,yOff);
	    alphabits=savealphabits;
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
    GFX_END
}

extern bool qws_smoothfonts;

// Unaccelerated screen/driver setup. Can be overridden by accelerated
// drivers

QScreen::QScreen( int display_id )
{
    data = 0;
    displayId = display_id;
    initted=FALSE;
    entryp=0;
}

QScreen::~QScreen()
{
}

void QScreen::shutdownDevice()
{
    qDebug("shutdownCard");
#ifndef QT_NO_QWS_CURSOR
    qt_screencursor->hide();
#endif
}

extern bool qws_accel; //in qapplication_qws.cpp

QGfx * QScreen::screenGfx()
{
    QGfx * ret=createGfx(data,w,h,d,lstep);
    if(d<=8) {
	ret->setClut(clut(),numCols());
    }
    return ret;
}

int QScreen::alloc(unsigned int r,unsigned int g,unsigned int b)
{
    int ret = 0;
    if ( d == 8 ) {
	// First we look to see if we match a default color
	QRgb myrgb=qRgb(r,g,b);
	int pos= (r + 25) / 51 * 36 + (g + 25) / 51 * 6 + (b + 25) / 51;
	if ( simple_8bpp_alloc || screenclut[pos] == myrgb || !initted ) {
	    return pos;
	}

	// search for nearest color
	unsigned int mindiff = 0xffffffff;
	unsigned int diff;
	int dr,dg,db;

	for ( int loopc = 0; loopc < 256; loopc++ ) {
	    dr = qRed(screenclut[loopc]) - r;
	    dg = qGreen(screenclut[loopc]) - g;
	    db = qBlue(screenclut[loopc]) - b;
	    diff = dr*dr + dg*dg + db*db;

	    if ( diff < mindiff ) {
		ret = loopc;
		if ( !diff )
		    break;
		mindiff = diff;
	    }
	}
    } else if ( d == 4 ) {
	ret = qGray( r, g, b ) >> 4;
    } else {
	qFatal( "cannot alloc %dbpp colour", d );
    }

    return ret;
}

// The end_of_location parameter is unusual: it's the address AFTER the cursor data.
int QScreen::initCursor(void* end_of_location, bool init)
{
#ifndef QT_NO_QWS_CURSOR
    qt_sw_cursor=TRUE;
    // ### until QLumpManager works Ok with multiple connected clients,
    // we steal a chunk of shared memory
    SWCursorData *data = (SWCursorData *)end_of_location - 1;
    qt_screencursor=new QScreenCursor();
    qt_screencursor->init( data, init );
    return sizeof(SWCursorData);
#else
    return 0;
#endif
}

// save the state of the graphics card
// This is needed so that e.g. we can restore the palette when switching
// between linux virtual consoles.
void QScreen::save()
{
}

// restore the state of the graphics card.
void QScreen::restore()
{
}

void QScreen::set(unsigned int, unsigned int, unsigned int, unsigned int)
{
}

bool QScreen::supportsDepth(int d) const
{
    if ( FALSE ) {
	//Just to simplify the ifdeffery
#ifndef QT_NO_QWS_DEPTH_1
    } else if(d==1) {
	return TRUE;
#endif
#ifndef QT_NO_QWS_DEPTH_4
    } else if(d==4) {
	return TRUE;
#endif
#ifndef QT_NO_QWS_DEPTH_16
    } else if(d==16) {
	return TRUE;
#endif
#ifndef QT_NO_QWS_DEPTH_8
    } else if(d==8) {
	return TRUE;
#endif
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
    } else if(d==8) {
	return TRUE;
#endif
#ifndef QT_NO_QWS_DEPTH_24
    } else if(d==24) {
	return TRUE;
#endif
#ifndef QT_NO_QWS_DEPTH_32
    } else if(d==32) {
	return TRUE;
#endif
    }
    return FALSE;
}

QGfx * QScreen::createGfx(unsigned char * bytes,int w,int h,int d, int linestep)
{
    QGfx* ret;
    if ( FALSE ) {
	//Just to simplify the ifdeffery
#ifndef QT_NO_QWS_DEPTH_1
    } else if(d==1) {
	ret = new QGfxRaster<1,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_4
    } else if(d==4) {
	ret = new QGfxRaster<4,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_16
    } else if(d==16) {
	ret = new QGfxRaster<16,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8
    } else if(d==8) {
	ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
    } else if(d==8) {
	ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_24
    } else if(d==24) {
	ret = new QGfxRaster<24,0>(bytes,w,h);
#endif
#ifndef QT_NO_QWS_DEPTH_32
    } else if(d==32) {
	ret = new QGfxRaster<32,0>(bytes,w,h);
#endif
    } else {
	qFatal("Can't drive depth %d",d);
	ret = 0; // silence gcc
    }
    ret->setLineStep(linestep);
    return ret;
}

bool QScreen::onCard(unsigned char * p) const
{
    long t=(unsigned long)p;
    long bmin=(unsigned long)data;
    if ( t < bmin )
	return FALSE;
    if( t >= bmin+mapsize )
	return FALSE;
    return TRUE;
}

bool QScreen::onCard(unsigned char * p, ulong& offset) const
{
    long t=(unsigned long)p;
    long bmin=(unsigned long)data;
    if ( t < bmin )
	return FALSE;
    long o = t - bmin;
    if ( o >= mapsize )
	return FALSE;
    offset = o;
    return TRUE;
}

// Accelerated drivers implement qt_get_screen which returns a QScreen
// that does accelerated mode stuff and returns accelerated QGfxen where
// appropriate. This is stored in qt_screen

#include "qgfxlinuxfb_qws.cpp"

#if !defined(QT_NO_QWS_MACH64)
# include "qgfxmach64_qws.cpp"
#endif

#if !defined(QT_NO_QWS_VOODOO3)
# include "qgfxvoodoo_qws.cpp"
#endif

#if !defined(QT_NO_QWS_MATROX)
# include "qgfxmatrox_qws.cpp"
#endif

#if !defined(QT_NO_QWS_VFB)
# include "qgfxvfb_qws.cpp"
#endif

#if !defined(QT_NO_QWS_VNC)
# include "qgfxvnc_qws.cpp"
#endif

#if !defined(QT_NO_QWS_TRANSFORMED)
# include "qgfxtransformed_qws.cpp"
#endif

#if !defined(QT_NO_QWS_VGA_16)
# include "qgfxvga16_qws.cpp"
#endif

#if !defined(QT_NO_QWS_SVGALIB)
# include "qgfxsvgalib_qws.cpp"
#endif

#if defined(QT_QWS_EE)
# include "qgfxee_qws.cpp"
#endif

struct DriverTable
{
    char *name;
    QScreen *(*qt_get_screen)(int);
    int accel;
} driverTable [] = {
#if !defined(QT_NO_QWS_VFB)
    { "QVFb", qt_get_screen_qvfb, 0 },
#endif
#if !defined(QT_NO_QWS_VGA_16)
    { "VGA16", qt_get_screen_vga16, 0 },
#endif
    { "LinuxFb", qt_get_screen_linuxfb, 0 },
#if !defined(QT_NO_QWS_MACH64)
    { "Mach64", qt_get_screen_mach64, 1 },
#endif
#if !defined(QT_NO_QWS_VOODOO3)
    { "Voodoo3", qt_get_screen_voodoo3, 1 },
#endif
#if !defined(QT_NO_QWS_MATROX)
    { "Matrox", qt_get_screen_matrox, 1 },
#endif
#if !defined(QT_NO_QWS_TRANSFORMED)
    { "Transformed", qt_get_screen_transformed, 0 },
#endif
#if defined(QT_QWS_EE)
    { "EE", qt_get_screen_ee, 0 },
#endif
#if !defined(QT_NO_QWS_SVGALIB)
    { "SVGALIB", qt_get_screen_svgalib, 1 },
#endif
#if !defined(QT_NO_QWS_VNC)
    { "VNC", qt_get_screen_vnc, 0 },
#endif
    { 0, 0, 0 },
};

QScreen *qt_get_screen( int display_id, const char *spec )
{
    QString displaySpec( spec );
    QString driver = displaySpec;
    int colon = displaySpec.find( ':' );
    if ( colon >= 0 )
	driver.truncate( colon );

    int i = 0;
    while ( driverTable[i].name ) {
	if ( driver.isEmpty() || QString( driverTable[i].name ) == driver ) {
	    qt_screen = driverTable[i].qt_get_screen( display_id );
	    if ( qt_screen ) {
		if ( qt_screen->connect( spec ) ) {
		    return qt_screen;
		} else {
		    delete qt_screen;
		    qt_screen = 0;
		}
	    }
	}
	i++;
    }

    qFatal( "No %s driver found", driver.isNull() ? "suitable" : driver.latin1() );

    return 0;
}
