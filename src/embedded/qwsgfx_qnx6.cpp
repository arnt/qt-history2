/****************************************************************************
**
** Implementation of Qt/Embedded Qnx graphics drivers.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

// QT/E includes
#include <qgfxraster_qws.h>
#include <qstring.h>
#include <qpolygonscanner.h>
#include <qpointarray.h>
#include <qbitarray.h>

#ifdef Q_OS_QNX6

#include "qwsgfx_qnx6.h"

// Qnx system includes
#include <unistd.h>
#include <sys/neutrino.h>
#include <dlfcn.h>
#include <signal.h>

// Qnx graphic driver includes
#include <display.h>
#include <disputil.h>
#include <vmem.h>

// Screen
static int displayId;
static disp_surface_t *screen;
static disp_adapter_t adapter;
static uint32_t pixel_format;

// Function lists
static disp_modefuncs_t modeFuncList;
static disp_memfuncs_t memFuncList;
static int (*coreFuncListFill) (disp_adapter_t *, unsigned int, disp_draw_corefuncs_t *, int);
static disp_draw_corefuncs_t coreFuncList;
static disp_draw_miscfuncs_t miscFuncList;
static disp_draw_contextfuncs_t ctxFuncList;
static disp_crtc_settings_t settings;

// Misc
static void *dllHandle;
static int QNXDummy = 0;

// Functions with c++ extensions do not work here
static void catchSignal(int s) { QApplication::exit(); };

// QnxFb Gfx class
template<const int depth, const int type>
QQnxFbGfx<depth,type>::QQnxFbGfx ()
: QGfxRaster<depth, type> ( screen->vidptr, screen->width, screen->height ) {
    ctx.adapter = &adapter;
    ctx.dsurf = screen;
    ctx.gd_ctx = adapter.gd_ctx;
    ctx.cfuncs = &coreFuncList;
    ctx.sysram_workspace_size = DISP_BYTES_PER_PIXEL(ctx.dsurf->pixel_format) * ctx.dsurf->width;
    ctx.sysram_workspace = (unsigned char *) malloc(ctx.sysram_workspace_size);
    ctx.rop3 = DrawModeS;
    ctx.flags = 0;
    ctx.bgcolor = 0xffffff;
    ctx.fgcolor = 0xffffff;
    coreFuncList.update_draw_surface(&ctx);
    ctxFuncList.update_general(&ctx);
    gfx_optype = gfx_lastop = &QNXDummy;
}

template<const int depth, const int type>
QQnxFbGfx<depth,type>::~QQnxFbGfx () {
    free(ctx.sysram_workspace);
}

template<const int depth, const int type>
void QQnxFbGfx<depth,type>::sync() {
    modeFuncList.wait_vsync(ctx.dsurf->adapter, displayId);
}
/*
template<const int depth, const int type>
void QQnxFbGfx<depth,type>::blt (int rx, int ry, int w, int h,
	int sx, int sy ) {
    if ( ncliprect < 1 || w < 1 || h < 1 )
	return;

    if( srctype != SourceImage
	    || alphatype != IgnoreAlpha
	    || !isScreenGfx()
	    || myrop!=CopyROP 
	    || buffer != srcbits ) {
QString reasons;
if (srctype != SourceImage)
reasons+="srctype ";
if (alphatype != IgnoreAlpha)
reasons+="alphatype ";
if (!isScreenGfx())
    reasons+="screengfx ";
if (myrop!=CopyROP)
    reasons+="!copyrop ";
if (buffer!=srcbits)
    reasons+="buff ";
qDebug("copout because of %s",reasons.latin1());
	QGfxRaster<depth,type>::blt(rx,ry,w,h,sx,sy);
	return;
    }

    // Driver ROP
    if (myrop==CopyROP) {
	if (ctx.rop3 != 204) {
	    ctx.rop3 = 204;
	    ctxFuncList.update_rop3 (&ctx);
	}
    } else if (myrop==XorROP) {
	if (ctx.rop3 != 60) {
	    ctx.rop3 = 60;
	    ctxFuncList.update_rop3 (&ctx);
	}
    } else if (myrop==NotROP) {
	if (ctx.rop3 != 51) {
	    ctx.rop3 = 51;
	    ctxFuncList.update_rop3 (&ctx);
	}
    }

    // Apply offsets
    int offsetRX = xoffs + rx;
    int offsetRY = yoffs + ry;
    int offsetSX = srcwidgetoffs.x() + sx;
    int offsetSY = srcwidgetoffs.y() + sy;

qDebug("first rx %d, ry %d, w %d, h %d, sx %d, sy %d",
	offsetRX,offsetRY,w,h,offsetSX,offsetSY);
    // Source and lock regions
    QRect lockRect(offsetRX, offsetRY, w + 1, h + 1); //?
//    QRect lockRect(rx, ry, w + 1, h + 1); //?
    QWSDisplay::grab( TRUE );

    // Basic clip
    if ( !clipbounds.intersects(lockRect) ) {
	QWSDisplay::ungrab();
	return;
    }
    */
/*
    // Clip to edges of screen - is there any point doing this????
    int mx = clipbounds.x() - offsetRX;
    if ( mx > 0 ) {
	offsetRX += mx;
	offsetSX += mx;
	w -= mx;
    }
    mx = clipbounds.y() - ry;
    if ( mx > 0 ) {
	offsetRY += mx;
	offsetSY += mx;
	h -= mx;
    }
    mx = rx + w - 1 - clipbounds.right();
    if ( mx > 0 )
	w -= mx;
    mx = ry + h - 1 - clipbounds.bottom();
    if ( mx > 0 )
	h -= mx;
	*/

/*
qDebug("second rx %d, ry %d, w %d, h %d, sx %d, sy %d",
	offsetRX,offsetRY,w,h,offsetSX,offsetSY);
    GFX_START(lockRect)
    QRect destRect(offsetRX, offsetRY, w, h);
    // Copy each clipped rect within the area
    for ( int currentClipRect = 0; currentClipRect < ncliprect; currentClipRect++ ) {
	QRect clipRect(cliprect[currentClipRect]);

	// Find intersection
	QRect intersectRect(destRect.intersect(clipRect));
	int intersectX = intersectRect.left();
	int intersectY = intersectRect.top();

	// Calculate clip offset into destination 
	int clipOffsetX = intersectX - offsetRX;
	int clipOffsetY = intersectY - offsetRY;

	// Apply clip offset to source
	offsetSX = offsetSX + clipOffsetX;
	offsetSY = offsetSY + clipOffsetY;

	// Do the blt
qDebug("ix %d, iy %d, iw %d, ih %d, sx %d, sy %d",
	intersectX,intersectY,
	intersectRect.width(),intersectRect.height(),
	offsetSX,offsetSY);
	coreFuncList.blit1(&ctx, offsetSX, offsetSY,
		intersectX, intersectY, 
		intersectRect.width(), intersectRect.height());
    }

    GFX_END
    QWSDisplay::ungrab();
}

template<const int depth, const int type>
void QQnxFbGfx<depth,type>::fillRect(int rx, int ry, int w, int h) {
    if ( ncliprect < 1 || w < 1 || h < 1 || cbrush.style() == NoBrush ) {
	return;
    }

    // Cop-outs
    if( cbrush.style()!=SolidPattern
        || alphatype!=IgnoreAlpha || srctype!=SourcePen
	|| myrop!=CopyROP ) {
	QGfxRaster<depth,type>::fillRect(rx,ry,w,h);
	return;
    }

    QWSDisplay::grab(TRUE);
    int fillLeft = rx + xoffs;
    int fillRight = fillLeft + w - 1;
    int fillTop = ry + yoffs;
    int fillBottom = fillTop + h - 1;
    QRect lock(fillLeft, fillTop, w + 1, h + 1);

    GFX_START(lock)
    
    useBrush();
    for (int currentClipRect = 0; currentClipRect < ncliprect; currentClipRect++ ) {
	QRect clipRect = cliprect[currentClipRect];
	if ( fillLeft <= clipRect.right()
	       && fillTop <= clipRect.bottom()
	       && fillRight >= clipRect.left()
	       && fillBottom >= clipRect.top()) {
	    int clippedFillLeft = qMax(clipRect.left(), fillLeft);
	    int clippedFillTop = qMax(clipRect.top(), fillTop);
	    int clippedFillRight = qMin(clipRect.right(), fillRight);
	    int clippedFillBottom = qMin(clipRect.bottom(), fillBottom);
qDebug("left %d top %d right %d bottom %d",clippedFillLeft,clippedFillTop,
	clippedFillRight,clippedFillBottom);
	    coreFuncList.draw_solid_rect(&ctx, pixel,
			clippedFillLeft, clippedFillTop,
			clippedFillRight, clippedFillBottom);
	}
    }

    GFX_END
    QWSDisplay::ungrab();
}

template<const int depth, const int type>
void QQnxFbGfx<depth,type>::hlineUnclipped (int x, int x1, int y) {
    // Not many being accelerated due to checking both pen and brush
    if (myrop!=CopyROP
	    || alphatype!=IgnoreAlpha
	    || srctype!=SourcePen
	    || cpen.style()!=SolidLine
	    || cbrush.style()!=SolidPattern) { 
	unsigned char *l = scanLine(y);
	QGfxRaster<depth,type>::hlineUnclipped(x,x1,l);
	return;
    }
    coreFuncList.draw_span(&ctx, pixel, x, x1, y);
}
*/

// Screen class
bool QQnxScreen::connect(const QString & spec) {
    // Reset the adapter
    memset(&adapter, 0, sizeof(adapter));

    // Extract PCI settings from environment
    QString vendor = getenv("QWS_VENDORID");
    QString device = getenv("QWS_DEVICEID");
    QString index = getenv("QWS_INDEX");
    if (!vendor || !device || !index) {
	qFatal("Could not get PCI settings from environment variables\n");
    }
    adapter.bus.pci.pci_vendor_id = vendor.toShort(NULL, 16);
    adapter.bus.pci.pci_device_id = device.toShort(NULL, 16);
    adapter.bus.pci.pci_index = index.toInt();

    // Extract dll name from spec
    QString name(spec);
    int colon = name.find(':');
    name.remove(0, colon + 1);
    colon = name.find(':');
    name.truncate(colon);

    // Extract display id from spec
    QString id(spec);
    colon = id.find(':');
    id.remove(0, colon + 1);
    id.find(':');
    id.remove(0, colon + 1);
    displayId = id.toInt(NULL, 10);

    // Open the dll
    dllHandle = dlopen(name.latin1(), 0);
    if (!dllHandle)
	qFatal("Could not open dll %s\n", name.latin1());

    // Get the functions from the dll(QNX ver >= 6.00)
    int (*modeFuncListFill) (disp_adapter_t *, disp_modefuncs_t *, int) = (int (*) (disp_adapter_t *, disp_modefuncs_t *, int)) dlsym(dllHandle, "devg_get_modefuncs");
    int (*memFuncListFill) (disp_adapter_t *, disp_memfuncs_t *, int) = (int (*) (disp_adapter_t *, disp_memfuncs_t *, int)) dlsym(dllHandle, "devg_get_memfuncs");
    int (*ctxFuncListFill) (disp_adapter_t *, disp_draw_contextfuncs_t *, int) = (int (*) (disp_adapter_t *, disp_draw_contextfuncs_t *, int)) dlsym(dllHandle, "devg_get_contextfuncs");
    coreFuncListFill = (int (*) (disp_adapter_t *, unsigned int, disp_draw_corefuncs_t *, int)) dlsym(dllHandle, "devg_get_corefuncs");
    int (*miscFuncListFill) (disp_adapter_t *, disp_draw_miscfuncs_t *, int) = (int (*) (disp_adapter_t *, disp_draw_miscfuncs_t *, int)) dlsym(dllHandle, "devg_get_miscfuncs");

    // Fill adapter function lists
    if (modeFuncListFill(&adapter, &modeFuncList, sizeof(modeFuncList)) == -1) {
	qFatal("Could not get entry points in dll %s\n", name.latin1());
    };
    memFuncListFill(&adapter, &memFuncList, sizeof(memFuncList));
    miscFuncListFill(&adapter, &miscFuncList, sizeof(miscFuncList));
    ctxFuncListFill(&adapter, &ctxFuncList, sizeof(ctxFuncList));

    screen_lastop = screen_optype = &QNXDummy;
    lastop = optype = &QNXDummy;

    return TRUE;
}

void QQnxScreen::disconnect() {
    dlclose(dllHandle);
}

bool QQnxScreen::initDevice() {
    if ((modeFuncList.init(&adapter, NULL)) == -1) {
	qFatal("Could not initialise graphics adapter\n");
    };

    // Extract screen settings from environment
    int bits = 0;
    QString tmpsize = getenv("QWS_SIZE");
    sscanf(tmpsize, "%dx%dx%d@%d", &(settings.xres), &(settings.yres),
	    &(bits), &(settings.refresh));
    if ( !settings.xres || !settings.yres || !settings.refresh || !bits)
	qFatal("Please check QWS_SIZE environment var and try again");

    // setMode will initialise screen
    QQnxScreen::setMode(settings.xres, settings.yres, bits);

    // Create the screen surface
    screen = memFuncList.alloc_surface(&adapter, settings.xres, settings.yres, pixel_format,
	    DISP_SURFACE_DISPLAYABLE |
	    DISP_SURFACE_CPU_LINEAR_READABLE |
	    DISP_SURFACE_CPU_LINEAR_WRITEABLE |
	    DISP_SURFACE_2D_TARGETABLE |
	    DISP_SURFACE_2D_READABLE |
	    DISP_SURFACE_PAGE_ALIGNED, 0);

    if (screen == NULL) {
	shutdownDevice();
	qFatal("Could not allocate screen surface\n");
    }

    if (screen->pixel_format == DISP_SURFACE_FORMAT_PAL8) {
	screen->palette = (disp_color_t *)malloc(256*4);
	if (screen->palette == NULL) 
	    qFatal("Can not allocate 8-bit palette\n");

	// 6*6*6 colour cube
	int idx = 0;
	for( int ir = 0x0; ir <= 0xff; ir+=0x33 ) {
	    for( int ig = 0x0; ig <= 0xff; ig+=0x33 ) {
		for( int ib = 0x0; ib <= 0xff; ib+=0x33 ) {
		    screen->palette[idx]=qRgb( ir, ig, ib );
		    idx++;
		}
	    }
	}

	// Fill in rest with 0
	for (;idx < 256;) {
	    screen->palette[idx]=0;
	    idx++;
	}

	if (modeFuncList.set_palette == NULL) {
	    if (miscFuncList.set_palette != NULL)
		miscFuncList.set_palette(&adapter, 0, 256, screen->palette);
	} else
	    modeFuncList.set_palette(&adapter, 0, 0, 256, screen->palette);

	for (int i = 0; i < 256; i++)
	    screenclut[i] = screen->palette[i];

	screencols = 256;
    } else
	screencols = 0;

    // Set the framebuffer base
    if (modeFuncList.set_display_offset)
	modeFuncList.set_display_offset (&adapter, displayId, screen->offset, 0);

    // Set inherited data
    int dataoffset = screen->offset;
    data = screen->vidptr + dataoffset;
    dw = w = settings.xres;
    dh = h = settings.yres;
    d = bits;
    lstep = screen->stride;
    size = lstep * h;
    mapsize = lstep * h * DISP_BYTES_PER_PIXEL ( screen->pixel_format ) ;

    screen_lastop = screen_optype = &QNXDummy;
    
    // Install the signal handler
    signal(SIGHUP, catchSignal);
    signal(SIGQUIT, catchSignal);
    signal(SIGABRT, catchSignal);
    signal(SIGSEGV, catchSignal);
    signal(SIGINT, catchSignal);
    
    return TRUE;
}

void QQnxScreen::shutdownDevice() {
    memFuncList.free_surface(&adapter, screen);
    miscFuncList.fini(&adapter);
    memFuncList.fini(&adapter);
    modeFuncList.fini(&adapter);
}

void QQnxScreen::setMode(int xres, int yres, int bits) {
    if (initted) {
	miscFuncList.fini(&adapter);
	memFuncList.fini(&adapter);
    }

    // Local variables
    disp_mode_t modeList[DISP_MAX_MODES];
    modeFuncList.get_modelist(&adapter, displayId, modeList, 0, DISP_MAX_MODES);
    disp_mode_info mi;

    // Find the mode matching the requested settings
    int i = 0;
    for (; modeList[i] != DISP_MODE_LISTEND; i++) {
	modeFuncList.get_modeinfo(&adapter, displayId, modeList[i], &mi);
	if ((DISP_BITS_PER_PIXEL(mi.pixel_format) == bits) && (mi.pixel_format != DISP_SURFACE_FORMAT_ARGB1555 ) &&
		((mi.flags & DISP_MODE_GENERIC) || (mi.xres == xres && mi.yres == yres)))
	    break;
    }
    if (modeList[i] == DISP_MODE_LISTEND) {
	modeFuncList.fini(&adapter);
	qFatal("Requested mode not found\n");
    }

    if (!(mi.flags & DISP_MODE_GENERIC)) {
	for (i = 0; i < DISP_MODE_NUM_REFRESH; i++)
	    if (mi.u.fixed.refresh[i] == settings.refresh)
		break;

	if (i == DISP_MODE_NUM_REFRESH && settings.refresh != 0) {
	    modeFuncList.fini(&adapter);
	    qFatal("Refresh rate not supported in this mode\n");
	}
    }

    // Retrieve core drawing functions for the selected pixel depth
    coreFuncListFill(&adapter, mi.pixel_format, &coreFuncList, sizeof(coreFuncList));
    modeFuncList.get_modeinfo(&adapter, displayId, modeList[i], &mi);

    settings.xres = xres;
    settings.yres = yres;

    qDebug("Setting %d x %d x %d bpp @ %d Hz\n", settings.xres, settings.yres, DISP_BITS_PER_PIXEL(mi.pixel_format), settings.refresh);

    // Calculate CRT settings(should check them as well)
    if (mi.flags & DISP_MODE_GENERIC) {
	settings.h_granularity = mi.u.generic.h_granularity;
	settings.v_granularity = mi.u.generic.v_granularity;
	settings.sync_polarity = mi.u.generic.sync_polarity;
	disp_crtc_calc(&settings);
    }

    disp_surface_t surf;
    surf.stride = xres * DISP_BYTES_PER_PIXEL(mi.pixel_format);
    surf.width = xres;
    surf.height = yres;
    if (modeFuncList.set_mode(&adapter, displayId, modeList[i], &settings, &surf, 0) == -1) {
	qFatal("Could not set mode\n");
    };

    // Initialise with the new settings
    memFuncList.init(&adapter, NULL);
    miscFuncList.init(&adapter, NULL);

    initted = TRUE;
    pixel_format = mi.pixel_format;
}

QRgb *clut() {
    return screen->palette;
}

QGfx *QQnxScreen::createGfx(unsigned char *bytes, int w, int h, int d, int linestep) {
    if (bytes == screen->vidptr) {
	QGfx* ret;
	if ( FALSE ) {
	    //Just to simplify the ifdeffery
#ifndef QT_NO_QWS_DEPTH_1
	} else if(d==1) {
	    ret = new QQnxFbGfx<1,0>();
#endif
#ifndef QT_NO_QWS_DEPTH_4
	} else if(d==4) {
	    ret = new QQnxFbGfx<4,0>();
#endif
#ifndef QT_NO_QWS_DEPTH_16
	} else if(d==16) {
	    ret = new QQnxFbGfx<16,0>();
#endif
#ifndef QT_NO_QWS_DEPTH_8
	} else if(d==8) {
	    ret = new QQnxFbGfx<8,0>();
#endif
#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
	} else if(d==8) {
	    ret = new QQnxFbGfx<8,0>();
#endif
#ifndef QT_NO_QWS_DEPTH_24
	} else if(d==24) {
	    ret = new QQnxFbGfx<24,0>();
#endif
#ifndef QT_NO_QWS_DEPTH_32
	} else if(d==32) {
	    ret = new QQnxFbGfx<32,0>();
#endif
	} else {
	    qFatal("Can't drive depth %d",d);
	    ret = 0; // silence gcc
	}
	ret->setLineStep(linestep);
	return ret;
    } else {
	return (QScreen::createGfx(bytes, w, h, d, linestep));
    }
}

extern "C" QScreen * qt_get_screen_qnxfb(int display_id) {
    return new QQnxScreen(display_id);
}

int QQnxScreen::initCursor(void *e, bool init) {
#ifndef QT_NO_QWS_CURSOR
    extern bool qws_sw_cursor;
    qws_sw_cursor = FALSE; //?

    if(qws_sw_cursor==TRUE) {
	miscFuncList.disable_hw_cursor(&adapter);
	return QScreen::initCursor(e,init);
    }

    qt_screencursor=new QQnxCursor();
#endif
    return 0;
}

#ifndef QT_NO_QWS_CURSOR
void QQnxCursor::init(SWCursorData *sw,bool b) { }

void QQnxCursor::set ( const QImage &image, int hotx, int hoty ) {
    colour0.setRgb(255,255,255);
    colour1.setRgb(0,0,0);

    int imageSize = image.width()*image.height();
    cursor.resize(imageSize);
    mask.resize(imageSize);

    cursor.fill(0);
    mask.fill(0);

    QRgb colour;
    for (uint x=0;x<image.width();x++) {
	for (uint y=0;y<image.height();y++) {
	    // Swap bit-endianness
	    int x1 = x&~7 | ~x&7;
	    colour = image.pixel(x1,y);
	    if (qRed(colour)) {
		cursor.setBit(y*image.width()+x);
		mask.clearBit(y*image.width()+x);
	    } else {
		if (qAlpha(colour)) {
		    cursor.setBit(y*image.width()+x);
		    mask.setBit(y*image.width()+x);
		} else {
		    cursor.clearBit(y*image.width()+x);
		    mask.clearBit(y*image.width()+x);
		}
	    }
	}
    }
    miscFuncList.set_hw_cursor (&adapter,
	    (uint8_t *)(cursor.data()), ((uint8_t *)mask.data()),
	    colour0.rgb(), colour1.rgb(),
	    hotx, hoty, image.width(), image.height(),
	    image.width()/8);
}

void QQnxCursor::hide () {
    miscFuncList.disable_hw_cursor(&adapter);
}

void QQnxCursor::show () {
    miscFuncList.enable_hw_cursor(&adapter);
}

void QQnxCursor::move (int x, int y) {
    miscFuncList.set_hw_cursor_pos(&adapter, x, y);
}
#endif

#endif
