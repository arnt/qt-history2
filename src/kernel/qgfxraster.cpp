/*****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#73 $
**
** Implementation of QGfxRaster (unaccelerated graphics context) class for
** Embedded Qt
** Created : 940721
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qgfxraster.h"
#include "qpen.h"
#include "qpaintdevicemetrics.h"
#include "qmemorymanager_qws.h"
#include "qwsregionmanager.h"
#include "qwindowdefs.h"


#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/fb.h>

#ifdef __i386__
#include <asm/mtrr.h>
#endif

#ifdef __MIPSEL__
#define QWS_NO_WRITE_PACKING
#endif

extern QWSDisplay *qt_fbdpy;
#if defined(__i386__) || !defined(__GNUC__)
typedef double QuadByte;
#else
typedef long long QuadByte;
#endif

#define QWS_EXPERIMENTAL_FASTPATH

#define QGfxRaster_Generic 0

struct SWCursorData {
    unsigned char cursor[SW_CURSOR_DATA_SIZE];
    unsigned char under[SW_CURSOR_DATA_SIZE*4];	// room for 32bpp display
    QRgb clut[256];
    unsigned char translut[256];
    int colours;
    int width;
    int height;
    int x;
    int y;
    int hotx;
    int hoty;
    bool enable;
};


#define GFX_START(r) bool swc_do_save=FALSE; \
		     if(is_screen_gfx) { \
			beginDraw(); \
			if(qt_sw_cursor) \
			    swc_do_save = qt_screencursor->restoreUnder(r,this); \
		     }
#define GFX_END if(is_screen_gfx) { \
		    if(swc_do_save) \
			qt_screencursor->saveUnder(); \
		    endDraw(); \
		}

#if defined(QWS_DEPTH_8GRAYSCALE)
    #define GFX_8BPP_PIXEL(r,g,b) qGray((r),(g),(b))
#elif defined(QWS_DEPTH_8DIRECT)
    #define GFX_8BPP_PIXEL(r,g,b) ((((r) >> 5) << 5) | (((g) >> 6) << 3) | ((b) >> 5))
#else
    #define GFX_8BPP_PIXEL(r,g,b) (((r) + 25) / 51 * 36 + ((g) + 25) / 51 * 6 + ((b) + 25) / 51)
#endif

static int optype;
static int lastop;

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
    }
}

QScreenCursor::~QScreenCursor()
{
    delete gfx;
}

bool QScreenCursor::supportsAlphaCursor()
{
    return gfx->bitDepth() >= 8;
}

void QScreenCursor::set(const QImage &image, int hotx, int hoty)
{
    QWSDisplay::grab();
    bool save = restoreUnder(QRect(data->x-data->hotx, data->y-data->hoty, 2, 2));
    data->hotx = hotx;
    data->hoty = hoty;
    data->width = image.width();
    data->height = image.height();
    memcpy(data->cursor, image.bits(), image.numBytes());
    data->colours = image.numColors();
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

    if (save) saveUnder();
    QWSDisplay::ungrab();
}

void QScreenCursor::move(int x, int y)
{
    QWSDisplay::grab();
    bool save = restoreUnder(QRect(data->x-data->hotx, data->y-data->hoty, 2, 2));
    data->x = x;
    data->y = y;
    if (save) saveUnder();
    QWSDisplay::ungrab();
}


bool QScreenCursor::restoreUnder( const QRect &r, QGfxRasterBase *g )
{
    int linestep = gfx->linestep();
    int width = gfx->pixelWidth();
    int height = gfx->pixelHeight();
    int depth = gfx->bitDepth();

    if (!data || !data->enable)
	return FALSE;

    if (!r.intersects(QRect(data->x - data->hotx, data->y - data->hoty,
			data->width+1, data->height+1)))
	return FALSE;

    if ( g && (fb_start > g->buffer || fb_end < g->buffer)
	    && (g->srctype == QGfx::SourceImage
		&& (fb_start > g->srcbits || fb_end < g->srcbits)) ) {
	return FALSE;
    }

    if (!save_under)
    {
	QWSDisplay::grab();

	int x = data->x - data->hotx;
	int y = data->y - data->hoty;

	// clipping
	int startCol = x < 0 ? abs(x) : 0;
	int startRow = y < 0 ? abs(y) : 0;
	int endRow = y + data->height > height ? height - y : data->height;
	int endCol = x + data->width > width ? width - x : data->width;

	int srcLineStep = data->width * depth/8;
	unsigned char *dest = fb_start + (y + startRow) * linestep
				+ (x + startCol) * depth/8;
	unsigned char *src = data->under;

	if ( endCol > startCol )
	    for (int row = startRow; row < endRow; row++)
	    {
		memcpy(dest, src, (endCol-startCol) * depth/8);
		src += srcLineStep;
		dest += linestep;
	    }
	save_under = TRUE;
	return TRUE;
    }

    return FALSE;
}

void QScreenCursor::saveUnder()
{
    int linestep = gfx->linestep();
    int width = gfx->pixelWidth();
    int height = gfx->pixelHeight();
    int depth = gfx->bitDepth();

    int x = data->x - data->hotx;
    int y = data->y - data->hoty;

    // clipping
    int startRow = y < 0 ? abs(y) : 0;
    int startCol = x < 0 ? abs(x) : 0;
    int endRow = y + data->height > height ? height - y : data->height;
    int endCol = x + data->width > width ? width - x : data->width;

    int destLineStep = data->width * depth / 8;

    unsigned char *src = fb_start + (y + startRow) * linestep
			    + (x + startCol) * depth/8;
    unsigned char *dest = data->under;

    if ( endCol > startCol )
	for (int row = startRow; row < endRow; row++)
	{
	    memcpy(dest, src, (endCol-startCol) * depth/8);
	    src += linestep;
	    dest += destLineStep;
	}

    drawCursor();

    save_under = FALSE;

    QWSDisplay::ungrab();
}

// we can't really use our gfx here because we will end up trying
// to restore/save the cursor again when we call blt.
void QScreenCursor::drawCursor()
{
    int linestep = gfx->linestep();
    int width = gfx->pixelWidth();
    int height = gfx->pixelHeight();
    int depth = gfx->bitDepth();


    int x = data->x - data->hotx;
    int y = data->y - data->hoty;

    // clipping
    int startRow = y < 0 ? abs(y) : 0;
    int startCol = x < 0 ? abs(x) : 0;
    int endRow = y + data->height > height ? height - y : data->height;
    int endCol = x + data->width > width ? width - x : data->width;

    unsigned char *dest = fb_start + (y + startRow) * linestep
			    + x * depth/8;
    unsigned char *srcptr = data->cursor + startRow * data->width;

    QRgb *clut = data->clut;

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
		if (av == 0xff)
		{
		    *(dptr+col) = srcval;
		}
		else if (av != 0)
		{
		    r = (srcval & 0xff0000) >> 16;
		    g = (srcval & 0xff00) >> 8;
		    b = srcval & 0xff;
		    unsigned int hold = *(dptr+col);
		    int or=(hold & 0xff0000) >> 16;
		    int og=(hold & 0xff00) >> 8;
		    int ob=(hold & 0xff);

		    r-=or;
		    g-=og;
		    b-=ob;
		    r=(r * av) / 256;
		    g=(g * av) / 256;
		    b=(b * av) / 256;
		    r+=or;
		    g+=og;
		    b+=ob;

		    *(dptr+col) = (r << 16) | (g << 8) | b;
		}
	    }
	    srcptr += data->width;
	    dptr += linestep/4;
	}
    }
    else if (depth == 16)
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
		    r = (srcval & 0xff0000) >> 19;
		    g = (srcval & 0xff00) >> 10;
		    b = (srcval & 0xff) >> 3;
		    *(dptr+col) = (r << 11) | (g << 5) | b;
		}
		else if (av != 0) {
		    // This is absolutely silly - but we can so we do.
		    r = (srcval & 0xff0000) >> 16;
		    g = (srcval & 0xff00) >> 8;
		    b = srcval & 0xff;

		    unsigned short hold = *(dptr+col);
		    int or=(hold & 0xf800) >> 11;
		    int og=(hold & 0x07e0) >> 5;
		    int ob=(hold & 0x001f);
		    or=or << 3;
		    og=og << 2;
		    ob=ob << 3;

		    r-=or;
		    g-=og;
		    b-=ob;
		    r=(r * av) / 256;
		    g=(g * av) / 256;
		    b=(b * av) / 256;
		    r+=or;
		    g+=og;
		    b+=ob;

		    r=r >> 3;
		    g=g >> 2;
		    b=b >> 3;
		    *(dptr+col) = (r << 11) | (g << 5) | b;
		}
	    }
	    srcptr += data->width;
	    dptr += linestep/2;
	}
    } else if (depth == 8) {
	unsigned char *dptr = (unsigned char *)dest;
        unsigned int srcval;
	int av,r,g,b;
	QRgb * screenclut=qt_screen->clut();
	for (int row = startRow; row < endRow; row++)
	{
	    for (int col = startCol; col < endCol; col++)
	    {
		srcval = clut[*(srcptr+col)];
		av = srcval >> 24;
		if (av == 0xff) {
		    *(dptr+col) = data->translut[*(srcptr+col)];
		}
		else if (av != 0) {
		    // This is absolutely silly - but we can so we do.
		    r = (srcval & 0xff0000) >> 16;
		    g = (srcval & 0xff00) >> 8;
		    b = srcval & 0xff;

		    unsigned char hold = *(dptr+col);
		    int or,og,ob;
		    or=qRed(screenclut[hold]);
		    og=qGreen(screenclut[hold]);
		    ob=qBlue(screenclut[hold]);

		    r-=or;
		    g-=og;
		    b-=ob;
		    r=(r * av) / 256;
		    g=(g * av) / 256;
		    b=(b * av) / 256;
		    r+=or;
		    g+=og;
		    b+=ob;

		    *(dptr+col) = GFX_8BPP_PIXEL(r,g,b);
		}
	    }
	    srcptr += data->width;
	    dptr += linestep;
	}
    }
}

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
    regionClip=false;
    srctype=SourcePen;
    setPen(QColor(255,0,0));
    cbrushpixmap=0;
    dashedLines = FALSE;
    dashes = 0;
    numDashes = 0;
    widgetrgn=QRegion(0,0,w,h);
    cliprect=0;
    alphatype=IgnoreAlpha;
    alphabuf = 0;
    ismasking=false;
    maskcol=0;
    srclinestep=0;
    srcwidgetx=0;
    srcwidgety=0;
    srcbits=0;
    optype=0;
    lastop=0;
    lstep=0;
    calpha=255;
    opaque=false;
    backcolor=QColor(0,0,0);
    globalRegionRevision = 0;
    src_normal_palette=false;
    clutcols = 0;
    update_clip();

#if defined(QWS_DEPTH_8) || defined(QWS_DEPTH_8GRAYSCALE) || defined(QWS_DEPTH_8DIRECT)
    // default colour map
    setClut( qt_screen->clut(), qt_screen->numCols() );
#endif
}

QGfxRasterBase::~QGfxRasterBase()
{
    delete [] dashes;
    delete [] cliprect;
}

void QGfxRasterBase::sync()
{
    optype=0;
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
    QRegion rgn = qt_fbdpy->regionManager()->region( globalRegionIndex );
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

    // cache bounding rect
    clipbounds = QRect(0,0,width,height).intersect(setrgn.boundingRect());
    // Convert to simple array for speed
    QArray<QRect> a = setrgn.rects();
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

void QGfxRasterBase::setSourceOffset(int x ,int y)
{
    srcoffs = QPoint(x+srcwidgetx,y+srcwidgety);
}

void QGfxRasterBase::setMasking(bool on,int colour)
{
    ismasking=on;
    maskcol=colour;
    alphatype=IgnoreAlpha;
}

void QGfxRasterBase::setAlphaType(AlphaType a)
{
    alphatype=a;
    if(a==LittleEndianMask || a==BigEndianMask) {
	ismasking=true;
    } else {
	ismasking=false;
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
    alphatype=SolidAlpha;
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
	    blt(myx,myy,myw,glyph.metrics->height);
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


extern bool qws_accel; //in qapplication_qws.cpp

QGfx * QScreen::screenGfx()
{
    QGfx * ret=createGfx(data,w,h,d,lstep);
    if(d<=8) {
	ret->setClut(clut(),numCols());
    }
    return ret;
}

int QScreen::screenSize()
{
    return size;
}

int QScreen::totalSize()
{
    return mapsize;
}

// The end_of_location parameter is unusual: it's the address AFTER the cursor data.
int QScreen::initCursor(void* end_of_location, bool init)
{
    qt_sw_cursor=true;
    // ### until QLumpManager works Ok with multiple connected clients,
    // we steal a chunk of shared memory
    SWCursorData *data = (SWCursorData *)end_of_location - 1;
    qt_screencursor=new QScreenCursor();
    qt_screencursor->init( data, init );
    return sizeof(SWCursorData);
}

void QGfxRasterBase::paintCursor(const QImage& image, int hotx, int hoty, QPoint cursorPos)
{
    if (QScreenCursor::enabled())
    {
	setSource(&image);
	setAlphaType(QGfx::InlineAlpha);
	blt(cursorPos.x()-hotx-1,
		cursorPos.y()-hoty-1,
		image.width(), image.height());
    }
}


static inline void qgfx_vga_io_w_fast(unsigned short port, unsigned char reg,
                                 unsigned char val)
{
    outw((val<<8)|reg, port);
}

static inline void
qgfx_vga16_set_color(int c)
{
    qgfx_vga_io_w_fast( 0x3CE, 0, c );
}

static inline void
qgfx_vga16_set_enable_sr(int mask)
{
    qgfx_vga_io_w_fast( 0x3CE, 1, mask );
}

static inline void
qgfx_vga16_set_mask(int mask)
{
    qgfx_vga_io_w_fast( 0x3CE, 8, mask );
}

static inline void
qgfx_vga16_set_mode(int mode)
{
    qgfx_vga_io_w_fast( 0x3CE, 5, mode );
}

// Bits 0-2: rotate count
// Bits 3-4: 0=NOP, 1=AND, 2=OR, 3=XOR
static inline void
qgfx_vga16_set_op(int op)
{
    qgfx_vga_io_w_fast( 0x3CE, 3, op );
}

static inline void
qgfx_vga16_set_read_plane(int plane)
{
    qgfx_vga_io_w_fast( 0x3CE, 4, plane );
}

static inline void
qgfx_vga16_set_write_planes(int mask)
{
    qgfx_vga_io_w_fast( 0x3C4, 2, mask );
}

template<const int depth,const int type>
inline void QGfxRaster<depth,type>::useBrush()
{
    pixel = cbrush.color().pixel();
}

template<const int depth,const int type>
inline void QGfxRaster<depth,type>::usePen()
{
    pixel = cpen.color().pixel();
}

// Calculate packing values for 64-bit writes

template<const int depth,const int type>
inline void QGfxRaster<depth,type>::calcPacking(
			  void * m,int x1,int x2,
			  int & frontadd,int & backadd,int & count)
{
#ifdef QWS_NO_WRITE_PACKING
	frontadd = x2-x1+1;
	backadd = 0;
	count = 0;
	if(frontadd<0)
	    frontadd=0;
	return;
#else
    if(depth==32) {
	frontadd = x2-x1+1;
	backadd = 0;
	count = 0;
	if(frontadd<0)
	    frontadd=0;
	return;

	// ### 32bpp packing doesn't work
	unsigned int * myptr=(unsigned int *)m;
	if( (x2-x1+1)<2 ) {
	    frontadd=x2-x1+1;
	    backadd=0;
	    count=0;
	    if(frontadd<0)
		frontadd=0;
	    return;
	}
	frontadd=(((unsigned long)myptr)+(x1*2)) & 0x7;
	backadd=(((unsigned long)myptr)+((x2+1)*2)) & 0x7;
	if(frontadd)
	    frontadd=(8-frontadd);
	frontadd=frontadd/4;
	backadd=backadd/4;
	count=( (x2-x1+1)-(frontadd+backadd) );
	count=count >> 1;
    } else if(depth==16 || depth==15) {
	unsigned short int * myptr=(unsigned short int *)m;
	if( (x2-x1+1)<4 ) {
	    frontadd=x2-x1+1;
	    backadd=0;
	    count=0;
	    if(frontadd<0)
		frontadd=0;
	    return;
	}

	frontadd=(((unsigned long)myptr)+(x1*2)) & 0x7;
	backadd=(((unsigned long)myptr)+((x2+1)*2)) & 0x7;
	if(frontadd)
	    frontadd=(8-frontadd);
	frontadd=frontadd/2;
	backadd=backadd/2;
	count=( (x2-x1+1)-(frontadd+backadd) );
	count=count >> 2;
    } else if(depth==8) {
	// ### 8bpp packing doesn't work
	unsigned char * myptr=(unsigned char *)m;
	if( (x2-x1+1)<8 ) {
	    frontadd=x2-x1+1;
	    backadd=0;
	    count=0;
	    if(frontadd<0)
		frontadd=0;
	    return;
	}

	frontadd=(((unsigned long)myptr)+(x1)) & 0x7;
	backadd=(((unsigned long)myptr)+((x2+1))) & 0x7;
	if(frontadd)
	    frontadd=(8-frontadd);
	count=( (x2-x1+1)-(frontadd+backadd) );
	count=count >> 3;
    } else {
	qDebug("Need packing for depth %d",depth);
    }

    if(count<0)
	count=0;
    if(frontadd<0)
	frontadd=0;
    if(backadd<0)
	backadd=0;
#endif
}

template <const int depth, const int type>
QGfxRaster<depth,type>::QGfxRaster(unsigned char * b,int w,int h)
    : QGfxRasterBase(b,w,h)
{
    setLineStep((depth*width+7)/8);
    setBrush(QColor(0,0,255));
}

template <const int depth, const int type>
QGfxRaster<depth,type>::~QGfxRaster()
{
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::setBrush( const QBrush & b )
{
    cbrush=b;
    if((cbrush.style()!=NoBrush) && (cbrush.style()!=SolidPattern)) {
	patternedbrush=true;
    } else {
	patternedbrush=false;
    }
    QColor tmp=b.color();
    if(depth==1) {
	srccol==qGray(tmp.red(),tmp.green(),tmp.blue())>127 ? 1 : 0;
    } else {
	srccol=tmp.pixel();
    }
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
    alphatype=IgnoreAlpha;
    if ( p->devType() == QInternal::Widget ) {
	QWidget * w=(QWidget *)p;
	srcwidth=w->width();
	srcheight=w->height();
	QPoint hold;
	hold=w->mapToGlobal(hold);
	srcwidgetx=hold.x();
	srcwidgety=hold.y();
	if ( srcdepth == 1 ) {
	    buildSourceClut(0, 0);
	} else if(srcdepth <= 8) {
	    src_normal_palette=true;
	}
    } else if ( p->devType() == QInternal::Pixmap ) {
	//still a bit ugly
	QPixmap *pix = (QPixmap*)p;
	srcwidth=pix->width();
	srcheight=pix->height();
	if ( srcdepth == 1 ) {
	    buildSourceClut(0, 0);
	} else if(srcdepth <= 8) {
	    src_normal_palette=true;
	}
    } else {
	// This is a bit ugly #### I'll say!
	//### We will have to find another way to do this
	srcwidgetx=0;
	srcwidgety=0;
	buildSourceClut(0,0);
    }

    src_little_endian=true;
}

// Data source can never have palette?
template <const int depth, const int type>
void QGfxRaster<depth,type>::setSource(unsigned char * data,int w,int h)
{
    srcbits=data;
    width=w;
    height=h;
    srctype=SourceImage;
    srcwidgetx=0;
    srcwidgety=0;
    buildSourceClut(0,0);
    src_normal_palette=true;
    src_little_endian=true;
    patternedbrush=false;
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
    srcwidgetx=0;
    srcwidgety=0;
    srcwidth=i->width();
    srcheight=i->height();
    src_normal_palette=false;
    if ( srcdepth == 1 )
	buildSourceClut( 0, 0 );
    else  if(srcdepth<=8)
	buildSourceClut(i->colorTable(),i->numColors());
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::setSourcePen()
{
    srccol = cpen.color().pixel();
    src_normal_palette=true;
    srctype=SourcePen;
    srcwidgetx=0;
    srcwidgety=0;
}

// Cols==0, put some default values in. Numcols==0, figure it out from
// source depth

template <const int depth, const int type>
void QGfxRaster<depth,type>::buildSourceClut(QRgb * cols,int numcols)
{
    int loopc;

    if(!cols) {
	useBrush();
	srcclut[0]=pixel;
	transclut[0]=pixel;
	usePen();
	srcclut[1]=pixel;
	transclut[1]=pixel;
	return;
    }

    // Store colour table as 32-bit
    if(numcols==0) {
	// Doesn't really handle things like 2bpp
	if(srcdepth==8) {
	    numcols=256;
	} else if(srcdepth==4) {
	    numcols=16;
	} else {
	    numcols=2;
	}
    }

    // Copy clut
    for(loopc=0;loopc<numcols;loopc++)
	srcclut[loopc] = cols[loopc];

    // Transclut[sourceval]==destval

    if(depth<=8) {
	// Now look for matches
	for(loopc=0;loopc<numcols;loopc++) {
	    int r = qRed(srcclut[loopc]);
	    int g = qGreen(srcclut[loopc]);
	    int b = qBlue(srcclut[loopc]);
	    transclut[loopc] = GFX_8BPP_PIXEL(r,g,b);
	}
    }
}

//screen coordinates
template <const int depth, const int type>
inline void QGfxRaster<depth,type>::drawPointUnclipped( int x, unsigned char* l)
{
    if ( depth == 32 )
	((QRgb*)l)[x] = pixel;
    else if ( depth == 16 )
	((ushort*)l)[x] = pixel;
    else if ( depth == 8 )
	l[x] = pixel;
    else if ( depth == 1 )
	if ( pixel )
	    l[x/8] |= 1 << (x%8);
	else
	    l[x/8] &= ~(1 << (x%8));
}


template <const int depth, const int type>
void QGfxRaster<depth,type>::drawPoint( int x, int y )
{
    if(cpen.style()==NoPen)
	return;
    x += xoffs;
    y += yoffs;
    if (inClip(x,y)) {
	if(optype)
	    sync();
	optype=0;
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
    bool foundone=( (optype==0) ? TRUE : FALSE );

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
		optype=0;
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

    if(optype)
	sync();
    optype=0;
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

#ifdef QWS_EXPERIMENTAL_FASTPATH //BUG: does not clip properly
                         // Probably because of the changes in qregion_qws
    // Fast path
    if (y1 == y2 && ncliprect == 1 && !dashedLines) {
	if ( x1 > cliprect[0].right() || x2 < cliprect[0].left()
	     || y1 < cliprect[0].top() || y1 > cliprect[0].bottom() )
	    return;
	GFX_START(QRect(x1, y1, x2 - x1 + 1, 1))
	x1 = x1 > cliprect[0].left() ? x1 : cliprect[0].left();
	x2 = x2 > cliprect[0].right() ? cliprect[0].right() : x2;
	unsigned char *l = scanLine(y1);
	hlineUnclipped(x1,x2,l);
	GFX_END
	return;
    }
#endif
    // Bresenham algorithm from Graphics Gems

    int dx=x2-x1;
    int ax=abs(dx)*2;
    int sx=dx>0 ? 1 : -1;
    int dy=y2-y1;
    int ay=abs(dy)*2;
    int sy=dy>0 ? 1 : -1;
    int x=x1;
    int y=y1;

    int d;

    GFX_START(QRect(x1, y1 < y2 ? y1 : y2, dx+1, abs(dy)+1))

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
    double iy = sin(a) * w / 2;

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

    if(optype)
	sync();
    optype=0;
    usePen();

    GFX_START(clipbounds)
    scan(pa, FALSE, 0, 5);
    QPen savePen = cpen;
    cpen = QPen( cpen.color() );
    drawPolyline(pa, 0, 5);
    cpen = savePen;
    GFX_END
}

//screen coordinates, clipped, x1<=x2
template <const int depth, const int type>
inline void QGfxRaster<depth,type>::hline( int x1,int x2,int y)
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
inline void QGfxRaster<depth,type>::hlineUnclipped( int x1,int x2,unsigned char* l)
{
    if ( depth == 32 ) {
	unsigned int *myptr=(unsigned int *)l;
	myptr+=x1;
	int w = x2-x1+1;
	while ( w-- )
	    *(myptr++) = pixel;
    } else if ( depth == 16 ) {
	unsigned short int *myptr=(unsigned short int *)l;

	int frontadd;
	int backadd;
	int count;
	calcPacking(myptr,x1,x2,frontadd,backadd,count);

	myptr+=x1;

	QuadByte put;
	unsigned short int * tmp=(unsigned short int *)&put;
	*tmp=pixel;
	*(tmp+1)=pixel;
	*(tmp+2)=pixel;
	*(tmp+3)=pixel;

	while ( frontadd-- )
	    *(myptr++)=pixel;

	while ( count-- ) {
	    *((QuadByte *)myptr) = put;
	    myptr += 4;
	}
	while ( backadd-- )
	    *(myptr++)=pixel;

    } else if ( depth == 8 ) {
	unsigned char *myptr=l;
	myptr+=x1;
	int w = x2-x1+1;
	while ( w-- )
	    *(myptr++) = pixel;
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
}

// Convert between pixel values for different depths
template <const int depth, const int type>
inline unsigned int QGfxRaster<depth,type>::get_value(int destdepth,
		       int sdepth,unsigned char ** srcdata,bool reverse)
{
    unsigned int ret;
    unsigned int r,g,b;
    if(destdepth==32) {
	if(sdepth==32) {
	    ret = *((unsigned int *)(*srcdata));
	    if(reverse) {
		(*srcdata)-=4;
	    } else {
		(*srcdata)+=4;
	    }
	} else if(sdepth==16) {
	    unsigned short int hold=*((unsigned short int *)(*srcdata));
	    r=(hold & 0xf800) >> 11;
	    g=(hold & 0x07e0) >> 5;
	    b=(hold & 0x001f);
	    r=r << 3;
	    g=g << 2;
	    b=b << 3;
	    ret = 0;
	    unsigned char * tmp=(unsigned char *)&ret;
	    *(tmp+2)=r;
	    *(tmp+1)=g;
	    *(tmp+0)=b;
	    if(reverse) {
		(*srcdata)-=2;
	    } else {
		(*srcdata)+=2;
	    }
	} else if(sdepth==15) {
	    unsigned short int hold=*((unsigned short int *)(*srcdata));
	    r=(hold & 0x7c00) >> 11;
	    g=(hold & 0x03e0) >> 5;
	    b=(hold & 0x001f);
	    r=r << 3;
	    g=g << 3;
	    b=b << 3;
	    ret=(r << 16) | (g << 8) | b;
	    if(reverse) {
		(*srcdata)-=2;
	    } else {
		(*srcdata)+=2;
	    }
	} else if(sdepth==8) {
	    unsigned char val=*((*srcdata));
#if defined(QWS_DEPTH_8GRAYSCALE)
	    if(src_normal_palette) {
		ret=((val >> 5) << 16)  | ((val >> 6) << 8) | (val >> 5);
	    } else {
#elif defined(QWS_DEPTH_8DIRECT)
	    if(src_normal_palette) {
		r=((val & 0xe0) >> 5) << 5;
		g=((val & 0x18) >> 3) << 6;
		b=((val & 0x07)) << 5;
		ret=(r << 16) | (g << 8) | b;
	    } else {
#else
	    if(true) {
#endif
		unsigned int hold=srcclut[val];
		r=(hold & 0xff0000) >> 16;
		g=(hold & 0x00ff00) >> 8;
		b=(hold & 0x0000ff);
		r=r << 16;
		g=g << 8;
		ret=r | g | b;
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
		monobitcount=1;	// yes, 1 is correct
		if(reverse) {
		    (*srcdata)--;
		} else {
		    (*srcdata)++;
		}
		monobitval=*((*srcdata));
	    }
	    if(src_little_endian) {
	    	ret=monobitval & 0x1;
	    	monobitval=monobitval >> 1;
	    } else {
		ret=monobitval & 0x80;
		monobitval=monobitval << 1;
		monobitval=monobitval & 0xff;
	    }
	    ret=srcclut[ret];
	} else {
	    qDebug("Odd source depth %d!",sdepth);
	    ret=0;
	}
    } else if(destdepth==16) {
	if(sdepth==32) {
	    unsigned int hold=*((unsigned int *)(*srcdata));
	    r=(hold & 0xff0000) >> 16;
	    g=(hold & 0x00ff00) >> 8;
	    b=(hold & 0x0000ff);
	    r=r >> 3;
	    g=g >> 2;
	    b=b >> 3;
	    r=r << 11;
	    g=g << 5;
	    ret=r | g | b;
	    if(reverse) {
		(*srcdata)-=4;
	    } else {
		(*srcdata)+=4;
	    }
	} else if(sdepth==16) {
	    unsigned short int hold=*((unsigned short int *)(*srcdata));
	    if(reverse) {
		(*srcdata)-=2;
	    } else {
		(*srcdata)+=2;
	    }
	    ret=hold;
	} else if(sdepth==15) {
	    unsigned short int hold=*((unsigned int *)(*srcdata));
	    r=(hold & 0x7c00) >> 10;
	    g=(hold & 0x03e0) >> 5;
	    b=(hold & 0x001f);
	    g=g >> 1;
	    r=r << 10;
	    g=g << 5;
	    ret=r | g | b;
	    if(reverse) {
		(*srcdata)-=2;
	    } else {
		(*srcdata)+=2;
	    }
	} else if(sdepth==8) {
	    unsigned char val=*((*srcdata));
#if defined(QWS_DEPTH_8GRAYSCALE)
	    if(src_normal_palette) {
		ret=((val >> 3) << 11) | ((val >> 2) << 5) | (val >> 3);
	    } else {
#elif defined(QWS_DEPTH_8DIRECT)
	    if(src_normal_palette) {
	    r=((val & 0xe0) >> 5) << 2;
	    g=((val & 0x18) >> 3) << 4;
	    b=((val & 0x07)) << 2;
	    ret=(r << 11) | (g << 6) | b;
	    } else {
#else
	    if(true) {
#endif
		unsigned int hold=srcclut[val];
		r=(hold & 0xff0000) >> 16;
		g=(hold & 0x00ff00) >> 8;
		b=(hold & 0x0000ff);
		r=r >> 3;
		g=g >> 2;
		b=b >> 3;
		r=r << 11;
		g=g << 5;
		ret=r | g | b;
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
		if(reverse) {
		    (*srcdata)--;
		} else {
		    (*srcdata)++;
		}
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
	    abort();
	    ret=0;
	}
    } else if(destdepth==15) {
	// Fix this to honour reverse
	if(sdepth==32) {
	    unsigned int hold=*((unsigned int *)(*srcdata));
	    r=(hold & 0xff0000) >> 16;
	    g=(hold & 0x00ff00) >> 8;
	    b=(hold & 0x0000ff);
	    r=r >> 3;
	    g=g >> 2;
	    b=b >> 3;
	    r=r << 10;
	    g=g << 5;
	    ret=r | g | b;
	    (*srcdata)+=4;
	} else if(sdepth==15) {
	    unsigned short int hold=*((unsigned short int *)(*srcdata));
	    (*srcdata)+=2;
	    ret=hold;
	} else if(sdepth==16) {
	    unsigned short int hold=*((unsigned int *)(*srcdata));
	    r=(hold & 0xf800) >> 11;
	    g=(hold & 0x07e0) >> 5;
	    b=(hold & 0x001f);
	    g=g << 1;
	    r=r << 11;
	    g=g << 5;
	    ret=r | g | b;
	    (*srcdata)+=2;
	} else if(sdepth==8) {
	    // FIXME: all of 15bpp support is pretty broken
	    unsigned char val=*((*srcdata)++);
	    return srcclut[val];
	} else if(sdepth==1) {
	    if(monobitcount<8) {
		monobitcount++;
	    } else {
		monobitcount=1;
		if(reverse) {
		    (*srcdata)--;
		} else {
		    (*srcdata)++;
		}
		monobitval=*((*srcdata));
	    }
	    if(src_little_endian) {
	    	ret=monobitval & 0x1;
	    	monobitval=monobitval >> 1;
	    } else {
		ret=monobitval & 0x80;
		monobitval=monobitval << 1;
		monobitval=monobitval & 0xff;
	    }
	} else {
	    qDebug("Odd source depth %d!",sdepth);
	    ret=0;
	}
    } else if(destdepth==8) {
	if(sdepth==8) {
	    unsigned char val=*((unsigned char *)(*srcdata));
	    // If source!=QImage, then the palettes will be the same
	    if(src_normal_palette) {
		ret=val;
	    } else {
		ret=transclut[val];
	    }
	    if(reverse) {
		(*srcdata)-=1;
	    } else {
		(*srcdata)+=1;
	    }
	} else if(sdepth==1) {
	    if(monobitcount<8) {
		monobitcount++;
	    } else {
		monobitcount=1;
		if(reverse) {
		    (*srcdata)--;
		} else {
		    (*srcdata)++;
		}
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
	    unsigned int hold=*((unsigned int *)(*srcdata));
	    r=(hold & 0xff0000) >> 16;
	    g=(hold & 0x00ff00) >> 8;
	    b=(hold & 0x0000ff);
	    ret = QColor( r, g, b ).alloc();
	    if(reverse) {
		(*srcdata)-=4;
	    } else {
		(*srcdata)+=4;
	    }
	} else {
	    qDebug("Cannot do %d->%d!",sdepth,destdepth);
	    ret=0;
	}
    } else if(destdepth==1) {
	// Fix this to honour reverse
	if(sdepth==32) {
	    unsigned int hold=*((unsigned int *)(*srcdata));
	    r=(hold & 0xff0000) >> 16;
	    g=(hold & 0x00ff00) >> 8;
	    b=(hold & 0x0000ff);
	    (*srcdata)+=4;
	    ret= QColor(r,g,b).alloc();
	} else if(sdepth==15) {
	    qWarning("Eek,15bpp->1bpp");
	    ret=0;
	} else if(sdepth==16) {
	    qWarning("Eek,16bpp->1bpp");
	    ret=0;
	} else if(sdepth==8) {
	    qWarning("Eek,8bpp->1bpp");
	    ret=0;
	} else if(sdepth==1) {
	    if(monobitcount=0) {
		monobitval=*((*srcdata)++);
	    }
	    if(src_little_endian) {
		ret=monobitval & 0x1;
		monobitval=monobitval >> 1;
	    } else {
		ret=monobitval & 0x80;
		monobitval=monobitval << 1;
		monobitval=monobitval & 0xff;
	    }
	    if(imagepos<7) {
		monobitcount++;
	    } else {
		monobitcount=0;
	    }
	    ret=srcclut[ret];
	} else {
	    qDebug("Odd source depth %d!",sdepth);
	    ret=0;
	}
    } else {
	qDebug("Odd destination depth %d!",depth);
	ret=0;
    }
    return ret;
}



#define GET_MASKED(rev) \
		    if(alphatype==IgnoreAlpha) { \
			if( gv==maskcol ) { \
			    masked=FALSE; \
			} \
		    } else { \
			if(alphatype==LittleEndianMask) { \
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
		    } \


template <const int depth, const int type>
inline void QGfxRaster<depth,type>::hImageLineUnclipped( int x1,int x2,
						    unsigned char* l,
						    unsigned char * srcdata,
						    bool reverse)
{
    if ( depth == 32 ) {
	unsigned int *myptr=(unsigned int *)l;
	int w = x2-x1+1;
	// Could put SourcePen stuff in here, but it's only really
	// useful for anti-aliased fonts and so on
	if ( !ismasking ) {
	    uint gv = srccol;
	    if(reverse) {
		myptr+=x2;
		srcdata+=(srcdepth*(w-1))/8;
		while (w--) {
		    if (srctype==SourceImage)
			gv = get_value(depth,srcdepth,&srcdata,TRUE);
		    *(myptr--) = gv;
		}
	    } else {
		myptr+=x1;
		while ( w-- ) {
		    if (srctype==SourceImage)
			gv = get_value(depth,srcdepth,&srcdata);
		    *(myptr++) = gv;
		}
	    }
	} else {
	    //masked 32bpp blt...
	    if(reverse) {
		myptr+=x2;
		srcdata+=(srcdepth*(w-1))/8;
		uint gv = srccol;
		while(w--) {
		    if (srctype==SourceImage)
			gv = get_value(depth,srcdepth,&srcdata,TRUE);
		    bool masked = TRUE;
		    GET_MASKED(TRUE);   //### does not work in reverse!!!
		    if ( masked )
			myptr--;
		    else
			*(myptr--) = gv;
		}
	    } else {
		myptr+=x1;
		uint gv = srccol;
		while(w--) {
		    if (srctype==SourceImage)
			gv = get_value(depth,srcdepth,&srcdata,FALSE);
		    bool masked = TRUE;
		    GET_MASKED(FALSE);
		    if ( masked )
			myptr++;
		    else
			*(myptr++) = gv;
		}
	    }
	}
    } else if ( depth == 16 ) {
	unsigned short int *myptr=(unsigned short int *)l;
	int w = x2-x1+1;

	if(!reverse) {
	    myptr+=x1;
	} else {
	    myptr+=x2;
	    srcdata+=((srcdepth*(w-1))/8);
	}
	if(!ismasking) {
	    unsigned int put;
	    unsigned short int *fun = (unsigned short int*)&put;
	    if(reverse) {
		// 32-bit writes - should be made 64?
		int backadd = (((unsigned long)(myptr+1)) & 0x3)>>1;
		int frontadd = (((unsigned long)(myptr-w+1)) & 0x3)>>1;
		int count = (w - frontadd - backadd) / 2;
		ASSERT (count*2+frontadd+backadd == w );
		if ( backadd )
		    *(myptr--)=get_value(depth,srcdepth,&srcdata,TRUE);
		myptr--;
		while ( count-- ) {
		    *(fun+1) = get_value(depth,srcdepth,&srcdata,TRUE);
		    *(fun) = get_value(depth,srcdepth,&srcdata,TRUE);
		    *((unsigned int*)myptr) = put;
		    myptr -= 2;
		}
		myptr++;
		if ( frontadd )
		    *(myptr--)=get_value(depth,srcdepth,&srcdata,TRUE);
	    } else {
		// 64-bit writes
		int frontadd;
		int backadd;
		int count;

		calcPacking(myptr-x1,x1,x2,frontadd,backadd,count);

		QuadByte dput;
		fun=(unsigned short int *)&dput;

		while ( frontadd-- )
		    *(myptr++)=get_value(depth,srcdepth,&srcdata);

		while ( count-- ) {
		    *fun = get_value(depth,srcdepth,&srcdata);
		    *(fun+1) = get_value(depth,srcdepth,&srcdata);
		    *(fun+2) = get_value(depth,srcdepth,&srcdata);
		    *(fun+3) = get_value(depth,srcdepth,&srcdata);
		    *((QuadByte*)myptr) = dput;
		    myptr += 4;
		}
		while ( backadd-- )
		    *(myptr++)=get_value(depth,srcdepth,&srcdata);
	    }
	} else {
	    // Probably not worth trying to pack writes if there's a mask
	    // colour, at least not yet
	    int count=( x2-x1+1 );
	    //if(xoffs<srcoffs.x()) {
	    if(reverse) {
		unsigned short int gv = srccol;
		while( count-- ) {
		    if(srctype==SourceImage)
			gv=get_value( depth, srcdepth, &srcdata, TRUE);
		    bool masked=TRUE;
		    GET_MASKED(TRUE);  //### does not work in reverse!!!
		    if(masked) {
			myptr--;
		    } else {
			*( myptr-- )=gv;
		    }
		}
	    } else {
		unsigned short int gv = srccol;
		while( count-- ) {
		    if(srctype==SourceImage)
			gv=get_value( depth, srcdepth, &srcdata, FALSE);
		    bool masked=TRUE;
		    GET_MASKED(FALSE);
		    if(masked) {
			myptr++;
		    } else {
			*( myptr++ )=gv;
		    }
		}
	    }
	}
    } else if ( depth == 8 ) {
	unsigned char *myptr=(unsigned char *)l;
	int w = x2-x1+1;

	if(!reverse) {
	    myptr+=x1;
	} else {
	    myptr+=x2;
	    srcdata+=((srcdepth*(w-1))/8);
	}
	if(!ismasking) {
	    unsigned int put;
	    unsigned char *fun = (unsigned char *)&put;
	    if(reverse) {
		// 64-bit writes
		int frontadd;
		int backadd;
		int count;

		myptr=l+x1;
		calcPacking(myptr-x1,x1,x2,frontadd,backadd,count);
		myptr=l+x2;
		
		QuadByte dput;
		fun=(unsigned char *)&dput;

		while ( backadd-- )
		    *(myptr--)=get_value(depth,srcdepth,&srcdata,true);

		while ( count-- ) {
		    *(fun+7) = get_value(depth,srcdepth,&srcdata,true);
		    *(fun+6) = get_value(depth,srcdepth,&srcdata,true);
		    *(fun+5) = get_value(depth,srcdepth,&srcdata,true);
		    *(fun+4) = get_value(depth,srcdepth,&srcdata,true);
		    *(fun+3) = get_value(depth,srcdepth,&srcdata,true);
		    *(fun+2) = get_value(depth,srcdepth,&srcdata,true);
		    *(fun+1) = get_value(depth,srcdepth,&srcdata,true);
		    *(fun+0) = get_value(depth,srcdepth,&srcdata,true);
		    *((QuadByte*)myptr) = dput;
		    myptr -= 8;
		}
		while ( frontadd-- )
		    *(myptr--)=get_value(depth,srcdepth,&srcdata,true);
	    } else {
		// 64-bit writes
		int frontadd;
		int backadd;
		int count;

		calcPacking(myptr-x1,x1,x2,frontadd,backadd,count);

		QuadByte dput;
		fun=(unsigned char *)&dput;

		while ( frontadd-- )
		    *(myptr++)=get_value(depth,srcdepth,&srcdata);

		while ( count-- ) {
		    *(fun+0) = get_value(depth,srcdepth,&srcdata);
		    *(fun+1) = get_value(depth,srcdepth,&srcdata);
		    *(fun+2) = get_value(depth,srcdepth,&srcdata);
		    *(fun+3) = get_value(depth,srcdepth,&srcdata);
		    *(fun+4) = get_value(depth,srcdepth,&srcdata);
		    *(fun+5) = get_value(depth,srcdepth,&srcdata);
		    *(fun+6) = get_value(depth,srcdepth,&srcdata);
		    *(fun+7) = get_value(depth,srcdepth,&srcdata);
		    *((QuadByte*)myptr) = dput;
		    myptr += 8;
		}
		while ( backadd-- )
		    *(myptr++)=get_value(depth,srcdepth,&srcdata);
	    }
	} else {
	    // Probably not worth trying to pack writes if there's a mask
	    // colour, at least not yet
	    int count=( x2-x1+1 );
	    if(reverse) {
		unsigned short int gv = srccol;
		while( count-- ) {
		    if(srctype==SourceImage)
			gv=get_value( depth, srcdepth, &srcdata, TRUE);
		    bool masked=TRUE;
		    GET_MASKED(TRUE);  //### does not work in reverse!!!
		    if(masked) {
			myptr--;
		    } else {
			*( myptr-- )=gv;
		    }
		}
	    } else {
		unsigned char gv = srccol;
		while( count-- ) {
		    if(srctype==SourceImage)
			gv=get_value( depth, srcdepth, &srcdata, FALSE);
		    bool masked=TRUE;
		    GET_MASKED(FALSE);
		    if(masked) {
			myptr++;
		    } else {
			*( myptr++ )=gv;
		    }
		}
	    }
	}
    } else if ( depth == 1 ) {
	//bitmap to bitmap first, let's handle other combinations later.
	if ( srcdepth == 1 ) {
	    // depth 1 destination with PCI/AGP hardware is very rare
	    // so we don't bother 32-bit-combining. No endian issues
	    // handled here
	    unsigned char * dp=l;
	    unsigned char * sp=srcdata;
	    int w=(x2-x1)+1;
	    w=(w+7)/8;
	    dp+=(x1/8);
	    while(w--) {
		*(dp++)=*(sp++);
	    }
	} else {
		// QGroupBox seems to want to draw text onto 1bpp
		// when doing wood theme
		// (i.e. SourcePen, srcdepth is 0, with mask)
		// This doesn't work yet
	    if ( srctype == SourcePen && alphatype == BigEndianMask ) {
		unsigned char * dp=l;
		int w=(x2-x1)+1;
		unsigned int gv = srccol; // used by GET_MASKED
		w=(w+7)/8;
		dp+=(x1/8);
		int skipbits = x1%8;
		while(w--) {
		    unsigned char m = 0;
		    for (int i = 0; i < 8; i++) {
			bool masked = TRUE;
			if (skipbits)
			    skipbits--;
			else {
			    GET_MASKED(FALSE);
			}
			if (!masked) m |= 1 << i;
		    }
		    *(dp++) |= m;
		}
	    } else if ( srctype==SourcePen && alphatype==LittleEndianMask ) {
		unsigned char * dp=l;
		int w=(x2-x1)+1;
		w=(w+7)/8;
		dp+=(x1/8);
		while(w--) {
		    *(dp++) |= *(maskp++);
		}
	    } else {
		qWarning( "Cannot blt %dbpp to monochrome", srcdepth );
	    }
	}
    }
}

template <const int depth, const int type>
inline void QGfxRaster<depth,type>::hAlphaLineUnclipped( int x1,int x2,
						    unsigned char* l,
						    unsigned char * srcdata,
						    unsigned char * alphas)
{
    if ( depth == 32 ) {
	// First read in the destination line
	unsigned int * myptr;
	myptr=(unsigned int *)l;

	int frontadd;
	int backadd;
	int count;
	int loopc2;

	calcPacking(myptr,x1,x2,frontadd,backadd,count);

	int w=x2-x1+1;
	myptr+=x1;
	int loopc;

	unsigned char * avp=alphas;

	int myp=0;
	unsigned int * temppos=myptr;

	QuadByte temp2;

	unsigned int * cp;

	for( loopc2=0;loopc2<frontadd;loopc2++ ) {
	    alphabuf[myp++]=*temppos;
	    temppos++;
	}

	for( loopc2=0;loopc2<count;loopc2++ ) {
	    temp2=*((QuadByte *)temppos);
	    cp=(unsigned int *)&temp2;
	    alphabuf[myp++]=*cp;
	    alphabuf[myp++]=*(cp+1);
	    temppos+=2;
	}

	for( loopc2=0;loopc2<backadd;loopc2++ ) {
	    alphabuf[myp++]=*temppos;
	    temppos++;
	}

	// Now blend with source data
	unsigned char * srcptr=srcdata;
	unsigned int srcval;

	if(srctype==SourceImage) {
	    srcptr=srcdata;
	    srcval=0; // Shut up compiler
	} else {
	    // SourcePen
	    unsigned int r,g,b;
	    r=(srccol & 0x00ff0000);
	    g=(srccol & 0x0000ff00);
	    b=(srccol & 0x000000ff);
	    r=r >> 16;
	    g=g >> 8;
	    srcval=(r << 16) | (g << 8) | b;
	}
	for(loopc=0;loopc<w;loopc++) {
	    int r,g,b;
	    if(srctype==SourceImage)
		srcval=get_value(32,srcdepth,&srcptr);

	    unsigned int av;
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
	        r-=*(tmp+2);
	        g-=*(tmp+1);
	        b-=*(tmp+0);
	        r=(r * av) / 256;
	        g=(g * av) / 256;
	        b=(b * av) / 256;
	        r+=*(tmp+2);
	        g+=*(tmp+1);
	        b+=*(tmp+0);
	    }
	    alphabuf[loopc]=(r << 16) | (g << 8) | b;
	}

	// Now write it all out

	unsigned int * putdata=&alphabuf[0];

	QuadByte put;
	unsigned int *fun = (unsigned int*)&put;

	myptr=(unsigned int *)l;

	calcPacking(myptr,x1,x2,frontadd,backadd,count);

	myptr+=x1;

	for ( loopc2=0;loopc2<frontadd;loopc2++ ) {
	    *(myptr++)=*(putdata++);
	    w--;
	}

	for ( loopc2=0;loopc2<count;loopc2++ ) {
	    *(fun) = *(putdata++);
	    *(fun+1) = *(putdata++);
	    *((QuadByte*)myptr) = put;
	    myptr += 2;
	    w-=2;
	}

	for ( loopc2=0;loopc2<backadd;loopc2++ ) {
	    *(myptr++)=*(putdata++);
	    w--;
	}
    } else if ( depth == 16 ) {
        // First read in the destination line
	unsigned short int * myptr;
	myptr=(unsigned short int *)l;

	int frontadd;
	int backadd;
	int count;
	int loopc2;

	calcPacking(myptr,x1,x2,frontadd,backadd,count);

	int w=x2-x1+1;
	myptr+=x1;
	int loopc;

	unsigned char * avp=alphas;

	int myp=0;
	unsigned short int * temppos=myptr;

	QuadByte temp2;

	unsigned char * cp;

	for( loopc2=0;loopc2<frontadd;loopc2++ ) {
	    alphabuf[myp++]=get_value(32,16,(unsigned char **)&temppos);
	}

	for( loopc2=0;loopc2<count;loopc2++ ) {
	    temp2=*((QuadByte *)temppos);
	    cp=(unsigned char *)&temp2;
	    alphabuf[myp++]=get_value(32,16,&cp);
	    alphabuf[myp++]=get_value(32,16,&cp);
	    alphabuf[myp++]=get_value(32,16,&cp);
	    alphabuf[myp++]=get_value(32,16,&cp);
	    temppos+=4;
	}

	for( loopc2=0;loopc2<backadd;loopc2++ ) {
	    alphabuf[myp++]=get_value(32,16,(unsigned char **)&temppos);
	}

	// Now blend with source data
	unsigned char * srcptr=srcdata;
	unsigned int srcval;

	if(srctype==SourceImage) {
	    srcptr=srcdata;
	    srcval=0; // Shut up compiler
	} else {
	    // SourcePen
	    unsigned int r,g,b;
	    r=(srccol & 0xf800);
	    g=(srccol & 0x07e0);
	    b=(srccol & 0x001f);
	    r=r >> 11;
	    g=g >> 5;
	    r=r << 3;
	    g=g << 2;
	    b=b << 3;
	    srcval=(r << 16) | (g << 8) | b;
	}
	for(loopc=0;loopc<w;loopc++) {
	    int r,g,b;
	    if(srctype==SourceImage)
		srcval=get_value(32,srcdepth,&srcptr);

	    unsigned int av;
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
	        r-=*(tmp+2);
	        g-=*(tmp+1);
	        b-=*(tmp+0);
	        r=(r * av) / 256;
	        g=(g * av) / 256;
	        b=(b * av) / 256;
	        r+=*(tmp+2);
	        g+=*(tmp+1);
	        b+=*(tmp+0);
	    }
	    r=r >> 3;
	    g=g >> 2;
	    b=b >> 3;
	    alphabuf[loopc]=(r << 11) | (g << 5) | b;
	}

	// Now write it all out

	unsigned int * putdata=&alphabuf[0];

	QuadByte put;
	unsigned short int *fun = (unsigned short int*)&put;

	myptr=(unsigned short int *)l;

	calcPacking(myptr,x1,x2,frontadd,backadd,count);

	myptr+=x1;

	for ( loopc2=0;loopc2<frontadd;loopc2++ ) {
	    *(myptr++)=*(putdata++);
	    w--;
	}

	for ( loopc2=0;loopc2<count;loopc2++ ) {
	    *(fun) = *(putdata++);
	    *(fun+1) = *(putdata++);
	    *(fun+2) = *(putdata++);
	    *(fun+3) =  *(putdata++);
	    *((QuadByte*)myptr) = put;
	    myptr += 4;
	    w-=4;
	}

	for ( loopc2=0;loopc2<backadd;loopc2++ ) {
	    *(myptr++)=*(putdata++);
	    w--;
	}

    } else if ( depth == 8 ) {
        // First read in the destination line
	unsigned char * myptr;
	myptr=l;
	int w=x2-x1+1;
	myptr+=x1;
	int loopc;

	unsigned char * avp=alphas;

	unsigned char * tempptr=myptr;
	
        for(loopc=0;loopc<w;loopc++) {
	    int val = *tempptr++;
#if defined(QWS_DEPTH_8GRAYSCALE)
	    alphabuf[loopc] = (val << 16) | (val << 8) | val;
#elif defined(QWS_DEPTH_8DIRECT)
	    int r=((val & 0xe0) >> 5) << 5;
	    int g=((val & 0x18) >> 3) << 6;
	    int b=((val & 0x07)) << 5;
	    alphabuf[loopc] = (r << 16) | (g << 8) | b;
#else
	    alphabuf[loopc]=clut[val];
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
	for(loopc=0;loopc<w;loopc++) {
	    int r,g,b;
	    if(srctype==SourceImage)
		srcval=get_value(32,srcdepth,&srcptr);

	    unsigned int av;
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
		// Do nothing - we already have source values in r,g,b
	    } else if(av==0) {
		r=*(tmp+2);
		g=*(tmp+1);
		b=*(tmp+0);
	    } else {
	        r-=*(tmp+2);
	        g-=*(tmp+1);
	        b-=*(tmp+0);
	        r=(r * av);
	        g=(g * av);
	        b=(b * av);
		r=r/256;
		g=g/256;
		b=b/256;
	        r+=*(tmp+2);
	        g+=*(tmp+1);
	        b+=*(tmp+0);
	    }
	    alphabuf[loopc] = GFX_8BPP_PIXEL(r,g,b);
	}

	// Now write it all out

	myptr=l;
	myptr+=x1;
	for(loopc=0;loopc<w;loopc++)
	    *(myptr++) = alphabuf[loopc];

    } else if ( depth == 1 ) {
	static int warn;
	if ( warn++ < 5 )
	    qDebug( "bitmap alpha not implemented" );
    }
}

//widget coordinates
template <const int depth, const int type>
void QGfxRaster<depth,type>::drawRect( int rx,int ry,int w,int h )
{
    if(optype)
	sync();
    optype=0;
    alphatype=IgnoreAlpha;
    ismasking=FALSE;
    if ( w <= 0 || h <= 0 ) return;

    GFX_START(QRect(rx+xoffs, ry+yoffs, w+1, h+1))

    if(cpen.style()!=NoPen) {
	drawLine(rx,ry,rx+(w-1),ry);
	drawLine(rx+(w-1),ry,rx+(w-1),ry+(h-1));
	drawLine(rx,ry+(h-1),rx+(w-1),ry+(h-1));
	drawLine(rx,ry,rx,ry+(h-1));
	rx++;
	ry++;
	w-=2;
	h-=2;
    }

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
	    QuadByte put;
	    unsigned short int * sp=(unsigned short int *)&put;
	    *sp=pixel;
	    *(sp+1)=pixel;
	    *(sp+2)=pixel;
	    *(sp+3)=pixel;

	    int add=linestep()/2;
	    add-=(frontadd+(count * 4)+backadd);

	    myptr=((unsigned short int *)scanLine(y1))+x1;
	    for(loopc=0;loopc<h;loopc++) {
		for(loopc2=0;loopc2<frontadd;loopc2++)
		    *(myptr++)=pixel;
		for(loopc2=0;loopc2<count;loopc2++) {
		    *((QuadByte *)myptr)=put;
		    myptr+=4;
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
	    QuadByte put;
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
		    *((QuadByte *)myptr)=put;
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
	    QuadByte put;
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

	    myptr=((unsigned char *)scanLine(y1))+x1;
	    for(loopc=0;loopc<h;loopc++) {
		for(loopc2=0;loopc2<frontadd;loopc2++)
		    *(myptr++)=pixel;
		for(loopc2=0;loopc2<count;loopc2++) {
		    *((QuadByte *)myptr)=put;
		    myptr+=8;
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
	    srcwidgetx=0;
	    srcwidgety=0;
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
	setSourceOffset(0,0);
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
void QGfxRaster<depth,type>::eraseRect ( int x,int y,int w,int h )
{
    QBrush oldbrush=cbrush;
    QPen oldpen=cpen;
    setBrush(QColor(200,200,200));
    setPen(NoPen);
    drawRect(x,y,w,h);
    setBrush(oldbrush);
    setPen(oldpen);
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::drawPolyline( const QPointArray &a,int index, int npoints )
{
    if(optype)
	sync();
    optype=0;
    //int m=QMIN( index+npoints-1, int(a.size())-1 );
    GFX_START(clipbounds)
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
    if(optype!=0) {
	sync();
    }
    optype=0;
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
  Looks like this finds a pointer to pixel (\a x, \a y) in a bitmap that
  is \a w pixels wide and stored in \a base. \a is_bigendian determines
  endianness.

  \a astat returns the bit number within the byte
  \a ahold holds the \c monobitval which is the byte pre-shifted
           to match the algoritm using this function

 */
static inline unsigned char * find_pointer(unsigned char * base,int x,int y,
					   int linestep,int & astat,unsigned char &
					   ahold,bool is_bigendian)
{
    int nbits=x % 8;
    int nbytes=x / 8;
    unsigned char * ret= base + (y*linestep) + nbytes;

    ahold=*ret;
    if(is_bigendian) {
	ahold=ahold << nbits;
	astat=nbits;
    } else {
	ahold=ahold >> nbits;
	astat=nbits;
    }

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

    srcbits=buffer;

    if ( depth%8 != 0 ) {
	// low bit depth - use slow blt based method.
	srclinestep=linestep();
	srcdepth=depth;
	if(srcdepth==0)
	    abort();
	srctype=SourceImage;
	alphatype=IgnoreAlpha;
	ismasking=FALSE;
	setSourceOffset(sx,sy);
	blt(rx,ry,w,h);
	return;
    }

    rx += xoffs;
    ry += yoffs;
    sx += xoffs;
    sy += yoffs;
    QRect cr;

    int bytesPerPixel = depth/8;
    int bytesPerLine = linestep();
    int bytesDy = abs(dy) * bytesPerLine;
    int bytesDx = dx * bytesPerPixel;
    unsigned char *src, *dest;
    bool plot=FALSE;

    GFX_START(QRect(QMIN(rx,sx), QMIN(ry,sy), w+abs(dx)+1, h+abs(dy)+1))

    if (dy > 0)
    {
	for (int row = ry; row < ry + h; row++)
	{
	    if ( ! cr.contains( QPoint(rx,row) ) )
		plot = inClip(rx,row,&cr);
	    int x=rx;
	    for (;;) {
		int x2 = cr.right();
		if ( x2 >= rx+w-1 ) {
		    if (plot) {
			dest = buffer + bytesPerLine * row + x * bytesPerPixel;
			src = dest + bytesDy + bytesDx;
			memmove(dest, src, (rx+w-x) * bytesPerPixel);
		    }
		    break;
		} else {
		    if (plot) {
			dest = buffer + bytesPerLine * row + x * bytesPerPixel;
			src = dest + bytesDy + bytesDx;
			memmove(dest, src, (x2-x+1) * bytesPerPixel);
		    }
		    x=x2+1;
		    plot=inClip(x,row,&cr,plot);
		}
	    }
	}
    }
    else
    {
	for (int row = ry + h - 1; row >= ry; row--)
	{
	    if ( ! cr.contains( QPoint(rx,row) ) )
		plot = inClip(rx,row,&cr);
	    int x=rx;
	    for (;;) {
		int x2 = cr.right();
		if ( x2 >= rx+w-1 ) {
		    if (plot) {
			dest = buffer + bytesPerLine * row + x * bytesPerPixel;
			src = dest - bytesDy + bytesDx;
			memmove(dest, src, (rx+w-x) * bytesPerPixel);
		    }
		    break;
		} else {
		    if (plot) {
			dest = buffer + bytesPerLine * row + x * bytesPerPixel;
			src = dest - bytesDy + bytesDx;
			memmove(dest, src, (x2-x+1) * bytesPerPixel);
		    }
		    x=x2+1;
		    plot=inClip(x,row,&cr,plot);
		}
	    }
	}
    }
    GFX_END
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::blt( int rx,int ry,int w,int h )
{
    if ( !w || !h ) return;
    if(optype!=0)
	sync();
    optype=0;
    int osrcdepth=srcdepth;
    if(srctype==SourcePen) {
	srclinestep=0;//w;
	srcdepth=0;
	usePen();
    }

    rx += xoffs;
    ry += yoffs;
    QRect cr;
#if 0 //WRONG, must change source offset if changing rx or ry
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
    if ( w <= 0 || h <= 0 )
	return;
#endif

    QRect cursRect(rx, ry, w+1, h+1);

    GFX_START(cursRect);

    bool reverse = rx>srcoffs.x() && srctype==SourceImage && srcbits == buffer;

    if ( alphatype == InlineAlpha || alphatype == SolidAlpha ||
	 alphatype == SeparateAlpha ) {
	alphabuf = new unsigned int[w];
    }

    int dl = linestep();
    int dj = 1;
    int dry = 1;
    int tj;
    int j;
    if(srcoffs.y() < ry) {
	// Back-to-front
	dj = -dj;
	dl = -dl;
	dry = -dry;
	j = h-1;
	ry=(ry+h)-1;
	tj = -1;
    } else {
	j = 0;
	tj = h;
    }

    unsigned char* l = scanLine(ry);
    for (; j!=tj; j+=dj,ry+=dry,l+=dl) {
	bool plot=inClip(rx,ry,&cr);
	int x=rx;
	for (;;) {
	    int x2 = cr.right();
	    if ( x2 > rx+w-1 ) {
		x2 = rx+w-1;
		if ( x2 < x ) break;
	    }
	    if (plot) {
		unsigned char * srcptr=srcScanLine(j+srcoffs.y());
		if(srcdepth==32) {
		    srcptr+=(((x-rx)+(srcoffs.x()))*4);
		} else if(srcdepth==16 || srcdepth==15) {
		    srcptr+=(((x-rx)+(srcoffs.x()))*2);
		} else if(srcdepth==8) {
		    srcptr+=(((x-rx)+(srcoffs.x())));
		} else if(srcdepth==1) {
		    srcptr+=(((x-rx)+srcoffs.x())/8);
		}
		if(srctype==SourceImage && srcdepth==1)
		    find_pointer(srcScanLine(0),(x-rx),j,
				 srclinestep,monobitcount,
				 monobitval,!src_little_endian);
		switch ( alphatype ) {
		  case LittleEndianMask:
	          case BigEndianMask:
		    maskp=find_pointer(alphabits,(x-rx),j,
				       alphalinestep,
				       amonobitcount,amonobitval,
				       alphatype==BigEndianMask);
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
    srcdepth=osrcdepth;
    if ( alphabuf ) {
	delete [] alphabuf;
	alphabuf = 0;
    }
    GFX_END
}

template <const int depth, const int type>
void QGfxRaster<depth,type>::stretchBlt( int rx,int ry,int w,int h,
					 int sw,int sh )
{
    if(optype)
	sync();
    optype=0;
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
    } else if(srcdepth==16) {
	mulfac=2;
    } else if(srcdepth==8) {
	mulfac=1;
    } else {
	mulfac=0;
	qDebug("Can't cope with stretchblt source depth %d",mulfac);
	return;
    }

    QRect cursRect(rx, ry, w+1, h+1);
    /* ???
    if (buffer_offset >= 0 && src_buffer_offset >= 0) {
	cursRect = QRect( QMIN(rx,srcoffs.x()), QMIN(ry,srcoffs.y()),
			QMAX(w, sw)+abs(rx - srcoffs.x())+1,
			QMAX(h, sh)+abs(ry - srcoffs.y())+1 );
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
		unsigned int val=get_value(depth,srcdepth,&p);
		if(depth==32) {
		    unsigned int * dp=(unsigned int *)data;
		    *(dp+loopc)=val;
		} else if(depth==16) {
		    unsigned short int * dp=(unsigned short int *)data;
		    *(dp+loopc)=val;
		} else if(depth==8) {
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
		    hImageLineUnclipped(x,rx+w-1,l,srcptr,false);
		}
		break;
	    } else {
		srcptr=sl;
		srcptr+=(((x-rx)+(srcoffs.x()))*mulfac);
		if (plot) {
			hImageLineUnclipped(x,x2,l,srcptr,false);
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

template <const int depth, const int type>
void QGfxRaster<depth,type>::tiledBlt( int rx,int ry,int w,int h )
{
    GFX_START(QRect(rx+xoffs, ry+yoffs, w+1, h+1))

    useBrush();
    unsigned char * savealphabits=alphabits;

    // ### need to handle brush offset too
    int offx = rx;
    int offy = ry;

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
	    setSourceOffset(xOff, yOff);
	    blt(xPos, yPos, drawW, drawH);
	    alphabits=savealphabits;
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
    GFX_END
}

template <const int depth, const int type>
inline unsigned char *QGfxRaster<depth,type>::scanLine(int i)
{
    return buffer+(i*linestep());
}

template <const int depth, const int type>
unsigned char * QGfxRaster<depth,type>::srcScanLine(int i)
{
    unsigned char * ret=srcbits;
    ret+=(i*srclinestep);
    return ret;
}

extern bool qws_smoothfonts;

// Unaccelerated screen/driver setup. Can be overridden by accelerated
// drivers

QScreen::QScreen()
{
}

QScreen::~QScreen()
{
}

bool QScreen::connect()
{
    fd=open("/dev/fb0",O_RDWR);
    if(fd<0) {
	qFatal("Can't open framebuffer device");
    }

    fb_fix_screeninfo finfo;
    fb_var_screeninfo vinfo;

    /* Get fixed screen information */
    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
	qFatal("Error reading fixed information");
    }

    /* Get variable screen information */
    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
	qFatal("Error reading variable information");
    }

    const char* qwssize;
    if((qwssize=getenv("QWS_SIZE"))) {
	sscanf(qwssize,"%dx%d",&w,&h);
    } else {
	w=vinfo.xres;
	h=vinfo.yres;
    }
    d=vinfo.bits_per_pixel;
    lstep=(vinfo.xres_virtual*d+7)/8;
    qDebug("Using %dx%dx%d screen, %d bytes per line",w,h,d,lstep);

    /* Figure out the size of the screen in bytes */
    size = h * lstep;

    mapsize=finfo.smem_len;

    data = (unsigned char *)mmap(0, mapsize, PROT_READ | PROT_WRITE,
				 MAP_SHARED, fd, 0);
    if ((int)data == -1) {
	qFatal("Error: failed to map framebuffer device to memory.");
    }
    qDebug("The framebuffer device was mapped to memory successfully at %p",
	   data);

    // Now read in palette
    if(vinfo.bits_per_pixel==8) {
	screencols=256;
	unsigned int loopc;
	startcmap = new fb_cmap;
	startcmap->start=0;
	startcmap->len=256;
	startcmap->red=(unsigned short int *)
		 malloc(sizeof(unsigned short int)*256);
	startcmap->green=(unsigned short int *)
		   malloc(sizeof(unsigned short int)*256);
	startcmap->blue=(unsigned short int *)
		  malloc(sizeof(unsigned short int)*256);
	startcmap->transp=(unsigned short int *)
		    malloc(sizeof(unsigned short int)*256);
	ioctl(fd,FBIOGETCMAP,startcmap);
	for(loopc=0;loopc<256;loopc++) {
	    screenclut[loopc]=qRgb(startcmap->red[loopc] >> 8,
				   startcmap->green[loopc] >> 8,
				   startcmap->blue[loopc] >> 8);
	}
    } else {
	screencols=0;
    }

    return TRUE;
}

void QScreen::disconnect()
{
    munmap((char*)data,mapsize);
    close(fd);
}

bool QScreen::initCard()
{
    // Grab current mode so we can reset it
    fb_var_screeninfo vinfo;
    fb_fix_screeninfo finfo;

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
	qFatal("Error reading variable information in card init");
	return false;
    }

    startupw=vinfo.xres;
    startuph=vinfo.yres;
    startupd=vinfo.bits_per_pixel;

    if (ioctl(fd, FBIOGET_FSCREENINFO, &finfo)) {
	qFatal("Error reading fixed information in card init");
	// It's not an /error/ as such, though definitely a bad sign
	// so we return true
	return true;
    }

#ifdef __i386__
    // Now init mtrr
    if(!getenv("QWS_NOMTRR")) {
	int mfd=open("/proc/mtrr",O_WRONLY,0);
	// MTRR entry goes away when file is closed - i.e.
	// hopefully when QWS is killed
	if(mfd==-1) {
	    qDebug("/proc/mtrr not writeable - not adjusting MTRR\n");
	} else {
	    mtrr_sentry sentry;
	    sentry.base=(unsigned long int)finfo.smem_start;
	    qDebug("Physical framebuffer address %ld",finfo.smem_start);
	    // Size needs to be in 4k chunks, but that's not always
	    // what we get thanks to graphics card registers. Write combining
	    // these is Not Good, so we write combine what we can
	    // (which is not much - 4 megs on an 8 meg card, it seems)
	    unsigned int size=finfo.smem_len;
	    size=size >> 22;
	    size=size << 22;
	    sentry.size=size;
	    sentry.type=MTRR_TYPE_WRCOMB;
	    if(ioctl(mfd,MTRRIOC_ADD_ENTRY,&sentry)==-1) {
		printf("Couldn't add mtrr entry for %lx %lx, %s\n",
		       sentry.base,sentry.size,strerror(errno));
	    }
	}
    }
#endif

    if(vinfo.bits_per_pixel==8) {
	screencols=256;
	unsigned int loopc;
	fb_cmap cmap;
	cmap.start=0;
	cmap.len=256;
	cmap.red=(unsigned short int *)
		 malloc(sizeof(unsigned short int)*256);
	cmap.green=(unsigned short int *)
		   malloc(sizeof(unsigned short int)*256);
	cmap.blue=(unsigned short int *)
		  malloc(sizeof(unsigned short int)*256);
	cmap.transp=(unsigned short int *)
		    malloc(sizeof(unsigned short int)*256);
#if defined(QWS_DEPTH_8GRAYSCALE)
	// Build greyscale palette
	for(loopc=0;loopc<256;loopc++) {
	    cmap.red[loopc]=loopc << 8;
	    cmap.green[loopc]=loopc << 8;
	    cmap.blue[loopc]=loopc << 8;
	    cmap.transp[loopc]=0;
	    screenclut[loopc]=qRgb(loopc,loopc,loopc);
	}
#elif defined(QWS_DEPTH_8DIRECT)
	for(loopc=0;loopc<256;loopc++) {
	    int a,b,c;
	    a=((loopc & 0xe0) >> 5) << 5;
	    b=((loopc & 0x18) >> 3) << 6;
	    c=(loopc & 0x07) << 5;
	    a=a | 0x3f;
	    b=b | 0x3f;
	    c=c | 0x3f;
	    cmap.red[loopc]=a << 8;
	    cmap.green[loopc]=b << 8;
	    cmap.blue[loopc]=c << 8;
	    cmap.transp[loopc]=0;
	    screenclut[loopc]=qRgb(cmap.red[loopc] >> 8,
				   cmap.green[loopc] >> 8,
				   cmap.blue[loopc] >> 8);
	}
#else
	// 6x6x6 216 color cube
	int idx = 0;
	for( int ir = 0x0; ir <= 0xff; ir+=0x33 ) {
	    for( int ig = 0x0; ig <= 0xff; ig+=0x33 ) {
		for( int ib = 0x0; ib <= 0xff; ib+=0x33 ) {
		    cmap.red[idx] = ir << 8;
		    cmap.green[idx] = ig << 8;
		    cmap.blue[idx] = ib << 8;
		    cmap.transp[idx] = 0;
		    screenclut[idx]=qRgb( ir, ig, ib );
		    idx++;
		}
	    }
	}
	screencols=idx;
#endif
	ioctl(fd,FBIOPUTCMAP,&cmap);
	free(cmap.red);
	free(cmap.green);
	free(cmap.blue);
	free(cmap.transp);
    }

    return true;
}

void QScreen::shutdownCard()
{
    // Set back the original mode
    if ( qt_sw_cursor )
	qt_screencursor->hide();

    // Causing crashes. Not needed.
    //setMode(startupw,startuph,startupd);
/*
    if ( startupd == 8 ) {
	ioctl(fd,FBIOPUTCMAP,startcmap);
	free(startcmap->red);
	free(startcmap->green);
	free(startcmap->blue);
	free(startcmap->transp);
	delete startcmap;
    }
*/
}

void QScreen::setMode(int nw,int nh,int nd)
{
    fb_var_screeninfo vinfo;

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
	qFatal("Error reading variable information in mode change");
    }

    vinfo.xres=nw;
    vinfo.yres=nh;
    vinfo.bits_per_pixel=nd;

    if (ioctl(fd, FBIOPUT_VSCREENINFO, &vinfo)) {
	qFatal("Error writing variable information in mode change");
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vinfo)) {
	qFatal("Error reading changed variable information in mode change");
    }

    w=vinfo.xres;
    h=vinfo.yres;
    d=vinfo.bits_per_pixel;
    size=w * h * d / 8;
}

// save the state of the graphics card
// This is needed so that e.g. we can restore the palette when switching
// between linux virtual consoles.
void QScreen::save()
{
    // nothing to do.
}

// restore the state of the graphics card.
void QScreen::restore()
{
    if ( d == 8 ) {
	fb_cmap cmap;
	cmap.start=0;
	cmap.len=screencols;
	cmap.red=(unsigned short int *)
		 malloc(sizeof(unsigned short int)*256);
	cmap.green=(unsigned short int *)
		   malloc(sizeof(unsigned short int)*256);
	cmap.blue=(unsigned short int *)
		  malloc(sizeof(unsigned short int)*256);
	cmap.transp=(unsigned short int *)
		    malloc(sizeof(unsigned short int)*256);
	for ( int loopc = 0; loopc < screencols; loopc++ ) {
	    cmap.red[loopc] = qRed( screenclut[loopc] ) << 8;
	    cmap.green[loopc] = qGreen( screenclut[loopc] ) << 8;
	    cmap.blue[loopc] = qBlue( screenclut[loopc] ) << 8;
	    cmap.transp[loopc] = 0;
	}
	ioctl(fd,FBIOPUTCMAP,&cmap);
	free(cmap.red);
	free(cmap.green);
	free(cmap.blue);
	free(cmap.transp);
    }
}

QGfx * QScreen::createGfx(unsigned char * bytes,int w,int h,int d, int linestep)
{
    QGfx* ret;
    if(d==1) {
	ret = new QGfxRaster<1,0>(bytes,w,h);
#if QWS_DEPTH_16
    } else if(d==16) {
	ret = new QGfxRaster<16,0>(bytes,w,h);
#endif
#if QWS_DEPTH_15
    } else if(d==15) {
	ret = new QGfxRaster<15,0>(bytes,w,h);
#endif
#if QWS_DEPTH_8
    } else if(d==8) {
	ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#if defined(QWS_DEPTH_8GRAYSCALE)
    } else if(d==8) {
	ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#if defined(QWS_DEPTH_8DIRECT)
    } else if(d==8) {
	ret = new QGfxRaster<8,0>(bytes,w,h);
#endif
#if QWS_DEPTH_32
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

#if defined (QWS_MACH64)

#include "qgfxmach64.cpp"

#elif defined(QWS_VOODOO)

#include "qgfxvoodoo.cpp"

#elif defined(QWS_VFB)

#include "qgfxvfb.cpp"

#else

extern "C" QScreen * qt_get_screen(char *,unsigned char *)
{
    if ( !qt_screen ) {
	qt_screen=new QScreen();
	qt_screen->connect();
    }
    return qt_screen;
}

#endif



