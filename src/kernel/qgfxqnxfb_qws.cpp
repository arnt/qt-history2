//				NOTE: This file is under development, and does not yet offer
//				any useful functionality on any platform.
//
//				If you are interested in Qnx support, please email us at
//				info@trolltech.com and explain in what way you would like
//				to use Qt with QNX.
//
//
//				In the meantime, Qt/X11 is reported to work with X11/QNX.


#ifdef _OS_QNX_

#include "qgfxqnxfb_qws.h"

// Qnx system includes
#include <unistd.h>
#include <sys/neutrino.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

// Qnx graphic driver includes
#include <display.h>
#include <disputil.h>
#include <vmem.h>

// QT/E includes
#include <qgfxraster_qws.h>
#include <qstring.h>
#include <qpolygonscanner.h>
#include <qpointarray.h>

// Qnx Pixmap Gfx class
QQnxPixmapGfx::QQnxPixmapGfx(unsigned char *b, int w, int h) : QGfxRaster <32, 0> (b, w, h) {
}

// QnxFb Gfx class
QQnxFbGfx::QQnxFbGfx(disp_surface_t *surf, disp_adapter_t *ad, int id, disp_modefuncs_t * mode, disp_memfuncs_t * mem, disp_draw_corefuncs_t * core, disp_draw_miscfuncs_t * misc, disp_draw_contextfuncs_t * context) : QGfxRasterBase ( surf->vidptr, surf->width, surf->height ) {
	displayId = id;
	modeFuncList = mode;
	memFuncList = mem;
	coreFuncList = core;
	miscFuncList = misc;
	contextFuncList = context;
	setupCtx ( surf, &ctx, ad );
}

void  QQnxFbGfx::sync() {
	modeFuncList->wait_vsync(ctx.dsurf->adapter, displayId);
}

void QQnxFbGfx::usePen() {
	ctx.fgcolor = cpen.color().rgb();
	contextFuncList->update_color(&ctx);
}

void QQnxFbGfx::useBrush() {
	ctx.fgcolor = cbrush.color().rgb();
	contextFuncList->update_color(&ctx);
}

void QQnxFbGfx::setBackgroundColor(QColor col) {
	ctx.bgcolor = col.rgb();
	contextFuncList->update_color(&ctx);
}

// NOTE apply clipping
void QQnxFbGfx::drawPoint(int x, int y) {
	drawPointUnclipped(x, y);
}

// NOTE apply error checking
void QQnxFbGfx::drawPoints(const QPointArray & p, int index, int size) {
	for (int x = index; x < size; x++) {
		drawPoint(p[x].x(), p[x].y());
	}
}

// NOTE very poor method of drawing
void QQnxFbGfx::drawPointUnclipped(int x, int y) {
//	void *pixel = (ctx.dsurf->vidptr + (x + (y * 1024)) * 3);
//	cpen.color().rgb((int *) pixel + 2, (int *) pixel + 1, (int *) pixel);
printf("drawpoint\n");
}

void QQnxFbGfx::drawPolyline(const QPointArray & p, int index, int size) {
	for (int x = index; x < size; x++) {
		drawLine(p[x].x(), p[x].y(), p[x + 1].x(), p[x + 1].y());
	}
}

void QQnxFbGfx::drawThickPolyline(const QPointArray & p, int index, int size) {
	printf("not yet impl, need pens first\n");
}

// NOTE this is not yet filled
void QQnxFbGfx::drawPolygon(const QPointArray & p, bool a, int index, int size) {
	drawPolyline(p, index, size);
	drawLine(p[size].x(), p[size].y(), p[index].x(), p[index].y());
}

void QQnxFbGfx::fillRect(int x1, int y1, int x2, int y2) {
	contextFuncList->draw_rect(&ctx, x1, y1, x2, y2);
}

// NOTE bad formatting in here somewhere, dont have indent
void QQnxFbGfx::drawLine(int x1, int y1, int x2, int y2) {
	//From QRasterBase
	if (cpen.style() == NoPen)
		return;

	if (cpen.width() > 1) {
		drawThickLine(x1, y1, x2, y2);
		return;
	}
	usePen();
	x1 += xoffs;
	y1 += yoffs;
	x2 += xoffs;
	y2 += yoffs;

	if (x1 > x2) {
		int x3;
		int y3;
		x3 = x2;
		y3 = y2;
		x2 = x1;
		y2 = y1;
		x1 = x3;
		y1 = y3;
	}
	int dx = x2 - x1;
	int dy = y2 - y1;

	GFX_START(QRect(x1, y1 < y2 ? y1 : y2, dx + 1, QABS(dy) + 1))
	int ax = QABS(dx) * 2;
	int ay = QABS(dy) * 2;
	int sx = dx > 0 ? 1 : -1;
	int sy = dy > 0 ? 1 : -1;
	int x = x1;
	int y = y1;
	int d;

	QRect cr;
	bool inside = inClip(x, y, &cr);
	if (ax > ay && !dashedLines) {
		d = ay - (ax >> 1);
		int px = x;

		//NOTE change this to QNXFLUSH ?
		#define QNXFLUSH(nx) \
        if ( inside ) \
			if ( sx < 1 ) \
				hlineUnclipped(nx,px,y); \
			else \
				hlineUnclipped(px,nx,y); \
			px = nx+sx;

		for (;;) {
			if (x == x2) {
				QNXFLUSH(x);
				GFX_END
				return;
			}
			if (d >= 0) {
				QNXFLUSH(x);
				y += sy;
				d -= ax;
				if (!cr.contains(x + sx, y))
					inside = inClip(x + sx, y, &cr);
				} else if (!cr.contains(x + sx, y)) {
						QNXFLUSH(x);
						inside = inClip(x + sx, y, &cr);
					}
					x += sx;
					d += ay;
				}
			} else if (ax > ay) {
					//cannot use hline for dashed lines
					int di = 0;
					int dc = dashedLines ? dashes[0] : 0;
					d = ay - (ax >> 1);
					for (;;) {
						if (!cr.contains(x, y))
							inside = inClip(x, y, &cr);
						if (inside && (di & 0x01) == 0)
							drawPointUnclipped(x, y);
						if (x == x2) {
							GFX_END
							return;
						}
						if (dashedLines && --dc <= 0) {
							if (++di >= numDashes)
							di = 0;
							dc = dashes[di];
						}
						if (d >= 0) {
							y += sy;
							d -= ax;
						}
						x += sx;
						d += ay;
					}
			} else {
				int             di = 0;
				int             dc = dashedLines ? dashes[0] : 0;
				d = ax - (ay >> 1);
				for (;;) {
					//y is dominant so we can 't optimise with hline
					if (!cr.contains(x, y))
						inside = inClip(x, y, &cr);
					if (inside && (di & 0x01) == 0)
						drawPointUnclipped(x, y);
					if (y == y2) {
						GFX_END
						return;
					}
					if (dashedLines && --dc <= 0) {
						if (++di >= numDashes)
							di = 0;
						dc = dashes[di];
					}
					if (d >= 0) {
						x += sx;
						d -= ay;
					}
					y += sy;
					d += ax;
				}
		}
	GFX_END
}

void QQnxFbGfx::hlineUnclipped (int x, int x1, int y) {
	coreFuncList->draw_span(&ctx, 0xeeeeee, x, x1, y);
//	contextFuncList->draw_span(&ctx, x, x1, y);
}

// Creates a new drawing context for a surface
void QQnxFbGfx::setupCtx(disp_surface_t * surface, disp_draw_context_t *ret, disp_adapter_t *adapter ) {
	ret->adapter = adapter;
	ret->dsurf = surface;
	ret->gd_ctx = adapter->gd_ctx;
	ret->cfuncs = coreFuncList;
	ret->sysram_workspace_size = DISP_BYTES_PER_PIXEL(ret->dsurf->pixel_format) * ret->dsurf->width;
	ret->sysram_workspace = (unsigned char *) malloc(ret->sysram_workspace_size);
	ret->rop3 = DrawModeS;
	ret->flags = 0;
	ret->bgcolor = 0x000000;
	ret->fgcolor = 0xffffff;
	coreFuncList->update_draw_surface(ret);
	contextFuncList->update_general(ret);
}

//Screen class
QQnxScreen::QQnxScreen(int display_id) : QScreen(display_id) {
}

QQnxScreen::~QQnxScreen(){}

bool QQnxScreen::connect(const QString & spec) {
	//Reset the adapter
	memset(&adapter, 0, sizeof(adapter));

	//Extract PCI settings from environment
	QString vendor = getenv("QWS_VENDORID");
	QString device = getenv("QWS_DEVICEID");
	QString index = getenv("QWS_INDEX");
	if (!vendor || !device || !index) {
		qFatal("Could not get PCI settings from environment variables\n");
	}
	adapter.bus.pci.pci_vendor_id = vendor.toShort(NULL, 16);
	adapter.bus.pci.pci_device_id = device.toShort(NULL, 16);
	adapter.bus.pci.pci_index = index.toInt();

	//Extract dll name from spec
	QString name(spec);
	int colon = name.find(':');
	name.remove(0, colon + 1);
	colon = name.find(':');
	name.truncate(colon);

	//Extract display id from spec
	QString id(spec);
	colon = id.find(':');
	id.remove(0, colon + 1);
	id.find(':');
	id.remove(0, colon + 1);
	displayId = id.toInt(NULL, 10);

	//Open the dll
	dllHandle = dlopen(name.latin1(), displayId);

	//Get the functions from the dll(QNX ver >= 6.00)
	int (*modeFuncListFill) (disp_adapter_t *, disp_modefuncs_t *, int) = (int (*) (disp_adapter_t *, disp_modefuncs_t *, int)) dlsym(dllHandle, "devg_get_modefuncs");
	int (*memFuncListFill) (disp_adapter_t *, disp_memfuncs_t *, int) = (int (*) (disp_adapter_t *, disp_memfuncs_t *, int)) dlsym(dllHandle, "devg_get_memfuncs");
	int (*contextFuncListFill) (disp_adapter_t *, disp_draw_contextfuncs_t *, int) = (int (*) (disp_adapter_t *, disp_draw_contextfuncs_t *, int)) dlsym(dllHandle, "devg_get_contextfuncs");
	coreFuncListFill = (int (*) (disp_adapter_t *, unsigned int, disp_draw_corefuncs_t *, int)) dlsym(dllHandle, "devg_get_corefuncs");
	int (*miscFuncListFill) (disp_adapter_t *, disp_draw_miscfuncs_t *, int) = (int (*) (disp_adapter_t *, disp_draw_miscfuncs_t *, int)) dlsym(dllHandle, "devg_get_miscfuncs");

	//Fill adapter function lists
	if (modeFuncListFill(&adapter, &modeFuncList, sizeof(modeFuncList)) == -1) {
		qFatal("Could not get entry points in dll %s\n", name.latin1());
	};
	memFuncListFill(&adapter, &memFuncList, sizeof(memFuncList));
	miscFuncListFill(&adapter, &miscFuncList, sizeof(miscFuncList));
	contextFuncListFill(&adapter, &contextFuncList, sizeof(contextFuncList));

	return TRUE;
}

void QQnxScreen::disconnect() {
	dlclose(dllHandle);
}

bool QQnxScreen::initDevice() {
	if ((modeFuncList.init(&adapter, NULL)) == -1) {
		qFatal("Could not initialise graphics adapter\n");
	};

	//Extract screen settings from environment
	QString bppS = getenv("QWS_BPP");
	QString xresS = getenv("QWS_XRES");
	QString yresS = getenv("QWS_YRES");
	QString refreshS = getenv("QWS_REFRESH");
	int bits = bppS.toInt();
	settings.xres = xresS.toInt();
	settings.yres = yresS.toInt();
	settings.refresh = refreshS.toShort();

	// setMode will initialise screen
	setMode(settings.xres, settings.yres, bits);

	//Create the screen surface
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

	// Set the framebuffer base
	if (modeFuncList.set_display_offset)
		modeFuncList.set_display_offset (&adapter, displayId, screen->offset, 0);

	// Set inherited data
	int dataoffset = screen->offset;
	data = screen->vidptr + dataoffset;
	dw = w = settings.xres;
	dh = h = settings.yres;
	d = bits;
	screencols = settings.xres;
	lstep = screen->stride;
	size = lstep * h;
	mapsize = lstep * h * d / 8;
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
		if ((DISP_BITS_PER_PIXEL(mi.pixel_format) == bits || (mi.pixel_format == DISP_SURFACE_FORMAT_ARGB1555 && bits == 15)) &&
		    ((mi.flags & DISP_MODE_GENERIC) || (mi.xres == xres && mi.yres == yres)))
			break;
	}

	if (modeList[i] == DISP_MODE_LISTEND) {
		modeFuncList.fini(&adapter);
		qFatal("Requested mode not found\n");
	}
	// Retrieve core drawing functions for the selected pixel depth
	coreFuncListFill(&adapter, mi.pixel_format, &coreFuncList, sizeof(coreFuncList));
	modeFuncList.get_modeinfo(&adapter, displayId, modeList[i], &mi);

	settings.xres = xres;
	settings.yres = yres;

	// Calculate CRT settings(should check them as well)
	if (mi.flags & DISP_MODE_GENERIC) {
		settings.h_granularity = mi.u.generic.h_granularity;
		settings.v_granularity = mi.u.generic.v_granularity;
		settings.sync_polarity = mi.u.generic.sync_polarity;
		disp_crtc_calc(&settings);
	}
	qDebug("Setting %d x %d x %d bpp @ %d Hz\n", settings.xres, settings.yres, DISP_BITS_PER_PIXEL(mi.pixel_format), settings.refresh);

	// Use a temporary surface to change the mode
	disp_surface_t surf;
	if (modeFuncList.set_mode(&adapter, displayId, modeList[i], &settings, &surf, 0) == -1) {
		qFatal("Could not set mode\n");
	};

	// Initialise with the new settings
	memFuncList.init(&adapter, NULL);
	miscFuncList.init(&adapter, NULL);

	memFuncList.reset(&adapter, &surf);

	initted = TRUE;
	pixel_format = mi.pixel_format;
}

void QQnxScreen::set(uint i, uint r, uint g, uint b) {
	screen->palette[i] = (0xff << 24) | ((r & 0xff) << 16) | ((g & 0xff) << 8) | (b & 0xff);
	// Mode.set_palette may not exist for all drivers
	if (modeFuncList.set_palette == NULL) {
		miscFuncList.set_palette(&adapter, displayId, 256, screen->palette);
	} else {
		modeFuncList.set_palette(&adapter, displayId, i, 1, screen->palette);
	}
}

QGfx *QQnxScreen::createGfx(unsigned char *buffer, int x, int y, int depth, int linestep) {
// if there was a list of allocated surfaces (eg the screen, caches, even another screen?,
// that list could be checked here automatically
	if (buffer == screen->vidptr) {
		QQnxFbGfx *tmp = new QQnxFbGfx(screen, &adapter, displayId, &modeFuncList, &memFuncList, &coreFuncList, &miscFuncList, &contextFuncList);
	} else {
		return new QQnxPixmapGfx(buffer, x, y);
	}
}

QGfx *QQnxScreen::screenGfx() {
	return createGfx(screen->vidptr, screen->width, screen->height, DISP_BITS_PER_PIXEL(screen->pixel_format), screen->stride);
}

extern "C" QScreen * qt_get_screen_qnxfb(int display_id) {
	return new QQnxScreen(display_id);
}

/*	
* Copyright  1999-2000, QNX Software Systems Ltd.  All Rights Reserved
*	
* This source code has been published by QNX Software Systems Ltd.
* (QSSL).  However, any use, reproduction, modification, distribution
* or transfer of this software, or any software which includes or is
* based upon any of this code, is only permitted under the terms of
* the QNX Open Community License version 1.0 (see licensing.qnx.com
* for details) or as otherwise expressly authorized by a written license
* agreement from QSSL.  For more information, please email licensing@qnx.com.
*/

/*
* Implementation of the VESA GTF timing formula
* Calculates CRTC settings for a given resolution and refresh rate
*/

#define GTF_MARGIN      1.8
#define GTF_MIN_PORCH       1
#define GTF_VSYNC_RQD       3
#define GTF_H_SYNC      8.0
#define GTF_MIN_VSYNC_BP    550
#define GTF_M           600.0
#define GTF_C           40.0
#define GTF_K           64.0
#define GTF_J           20.0

#define GTF_ROUND(__x)      ((int)((__x) + .5))

int disp_crtc_calc(disp_crtc_settings_t * settings) {
		int h_pixels, v_lines;
		float h_period, h_period_est;
		int vsync_bp, v_back_porch;
		int total_v_lines;
		float v_field_rate_est, v_field_rate;
		float ideal_duty_cycle;
		int h_blank_pixels, total_pixels;
		float pixel_freq, h_freq;
		int hsync_width;
		float gtf_c, gtf_m;

		gtf_c = (GTF_C - GTF_J) * GTF_K / 256 + GTF_J;
		gtf_m = GTF_K / 256 * GTF_M;

		h_pixels = ((settings->xres + settings->h_granularity - 1) /
						settings->h_granularity) * settings->h_granularity;
		v_lines = settings->yres;
		h_period_est = ((1.0 / (float) settings->refresh) -
			   (float) GTF_MIN_VSYNC_BP / 1000000) /
		(v_lines + GTF_MIN_PORCH) * 1000000;
		vsync_bp = GTF_ROUND((float) GTF_MIN_VSYNC_BP / h_period_est);
		v_back_porch = vsync_bp - GTF_VSYNC_RQD;
		total_v_lines = settings->yres + vsync_bp + GTF_MIN_PORCH;
		v_field_rate_est = 1.0 / h_period_est / total_v_lines * 1000000;
		h_period = h_period_est / ((float) settings->refresh / v_field_rate_est);
		v_field_rate = 1.0 / h_period / total_v_lines * 1000000;
		ideal_duty_cycle = gtf_c - (gtf_m * h_period) / 1000;
		h_blank_pixels = GTF_ROUND(h_pixels * ideal_duty_cycle /
							  (100 -
					 ideal_duty_cycle) /
			   (settings->h_granularity << 1)) *
		(settings->h_granularity << 1);
		total_pixels = h_pixels + h_blank_pixels;
		pixel_freq = total_pixels / h_period;
		h_freq = 1000 / h_period;

		/* Try to sync to VESA modes */
		total_pixels -= 6 * settings->h_granularity;

		settings->h_total = total_pixels / settings->h_granularity;
		settings->xres = h_pixels;
		settings->h_blank_start = settings->xres / settings->h_granularity;
		settings->h_blank_len = settings->h_total - 1 - settings->h_blank_start;
		hsync_width =
		GTF_ROUND(GTF_H_SYNC * total_pixels / 100 / settings->h_granularity);
		settings->h_sync_start = (settings->xres / settings->h_granularity) - 1 +
		(settings->h_blank_len / 2) - hsync_width;
		settings->h_sync_len =
		hsync_width;

		settings->v_total = total_v_lines;
		settings->v_blank_start = v_lines;
		settings->v_blank_len = total_v_lines - 1 - settings->v_blank_start;
		settings->v_sync_start = settings->yres - 1 +
		(settings->v_blank_len / 2) - GTF_VSYNC_RQD;
		settings->v_sync_len = GTF_VSYNC_RQD;

		settings->pixel_clock = pixel_freq * 1000;
#if 0
		fprintf(stderr, "X res %d\tY res %d\tVertical frequency %d\n",
					settings->xres, settings->yres, settings->refresh);
		fprintf(stderr, "Pixel clock %d\n", settings->pixel_clock);
		fprintf(stderr, "H total %d\n", settings->h_total);
		fprintf(stderr, "H blank start %d\tH blank len %d\n",
					settings->h_blank_start, settings->h_blank_len);
		fprintf(stderr, "H sync start %d\tH sync len %d\n",
					settings->h_sync_start, settings->h_sync_len);
		//fprintf(stderr, "H border start %d\tH border len %d\n",
				  //settings->h_border_start, settings->h_border_len);
		fprintf(stderr, "V total %d\n", settings->v_total);
		fprintf(stderr, "V blank start %d\tV blank len %d\n",
					settings->v_blank_start, settings->v_blank_len);
		fprintf(stderr, "V sync start %d\tV sync len %d\n",
					settings->v_sync_start, settings->v_sync_len);
		//fprintf(stderr, "V border start %d\tV border len %d\n",
				  //settings->v_border_start, settings->v_border_len);
#endif

		return 0;
}

#endif
