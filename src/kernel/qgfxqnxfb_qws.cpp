//				NOTE: This file is under development, and does not yet offer
//				any useful functionality on any platform.
//
//				If you are interested in Qnx support, please email us at
//				info@trolltech.com and explain in what way you would like
//				to use Qt with QNX.
//
//
//				In the meantime, Qt/X11 is reported to work with X11/QNX.

#ifdef Q_OS_QNX6

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

// QnxFb Gfx class
template<const int depth, const int type>
QQnxFbGfx<depth,type>::QQnxFbGfx (disp_surface_t *surf, disp_adapter_t *ad,
						int id, disp_modefuncs_t * mode, disp_memfuncs_t * mem, disp_draw_corefuncs_t * core,
						disp_draw_miscfuncs_t * misc, disp_draw_contextfuncs_t * context)
					: QGfxRaster<depth, type> ( surf->vidptr, surf->width, surf->height ) {
	displayId = id;
	modeFuncList = mode;
	memFuncList = mem;
	coreFuncList = core;
	miscFuncList = misc;
	contextFuncList = context;
	setupCtx ( surf, &ctx, ad );
}

template<const int depth, const int type>
QQnxFbGfx<depth,type>::~QQnxFbGfx () {
	free(ctx.sysram_workspace);
}

template<const int depth, const int type>
void QQnxFbGfx<depth,type>::sync() {
	modeFuncList->wait_vsync(ctx.dsurf->adapter, displayId);
}
/*
template<const int depth, const int type>
void QQnxFbGfx<depth,type>::fillRect(int x1, int y1, int x2, int y2) {
	useBrush();
	if ( depth == 8 ) { 
		printf("paletted mode fillRect\n");
		coreFuncList->draw_solid_rect(&ctx, qt_screen->clut()[pixel], x1, y1, x2, y2);
	} else {
//		coreFuncList->draw_solid_rect(&ctx, pixel, 0, 0, 100, 100);
		contextFuncList->draw_rect(&ctx, x1, y1, x2, y2);
		printf("non-paletted mode fillRect %lu\n", pixel);
	}
}

template<const int depth, const int type>
GFX_INLINE void QQnxFbGfx<depth,type>::hlineUnclipped (int x, int x1, int y) {
    coreFuncList->draw_span(&ctx, pixel, x, x1, y);
}
*/

// Creates a new drawing context for a surface
template<const int depth, const int type>
void QQnxFbGfx<depth,type>::setupCtx(disp_surface_t * surface, disp_draw_context_t *ret, disp_adapter_t *adapter ) {
	ret->adapter = adapter;
	ret->dsurf = surface;
	ret->gd_ctx = adapter->gd_ctx;
	ret->cfuncs = coreFuncList;
	ret->sysram_workspace_size = DISP_BYTES_PER_PIXEL(ret->dsurf->pixel_format) * ret->dsurf->width;
	ret->sysram_workspace = (unsigned char *) malloc(ret->sysram_workspace_size);
	ret->rop3 = DrawModeS;
	ret->flags = 0;
	ret->bgcolor = 0xffffff;
	ret->fgcolor = 0xffffff;
	coreFuncList->update_draw_surface(ret);
	contextFuncList->update_general(ret);
}

// Screen class
QQnxScreen::QQnxScreen(int display_id) : QScreen(display_id) {
}

QQnxScreen::~QQnxScreen(){}

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
	dllHandle = dlopen(name.latin1(), displayId);
	if (!dllHandle)
		qFatal("Could not open dll %s\n", name.latin1());

	// Get the functions from the dll(QNX ver >= 6.00)
	int (*modeFuncListFill) (disp_adapter_t *, disp_modefuncs_t *, int) = (int (*) (disp_adapter_t *, disp_modefuncs_t *, int)) dlsym(dllHandle, "devg_get_modefuncs");
	int (*memFuncListFill) (disp_adapter_t *, disp_memfuncs_t *, int) = (int (*) (disp_adapter_t *, disp_memfuncs_t *, int)) dlsym(dllHandle, "devg_get_memfuncs");
	int (*contextFuncListFill) (disp_adapter_t *, disp_draw_contextfuncs_t *, int) = (int (*) (disp_adapter_t *, disp_draw_contextfuncs_t *, int)) dlsym(dllHandle, "devg_get_contextfuncs");
	coreFuncListFill = (int (*) (disp_adapter_t *, unsigned int, disp_draw_corefuncs_t *, int)) dlsym(dllHandle, "devg_get_corefuncs");
	int (*miscFuncListFill) (disp_adapter_t *, disp_draw_miscfuncs_t *, int) = (int (*) (disp_adapter_t *, disp_draw_miscfuncs_t *, int)) dlsym(dllHandle, "devg_get_miscfuncs");

	// Fill adapter function lists
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

	// Extract screen settings from environment
	int bits = 0;
	QString tmpsize = getenv("QWS_SIZE");
	sscanf(tmpsize, "%dx%dx%d@%d", &(settings.xres), &(settings.yres),
		&(bits), &(settings.refresh));
	if ( !settings.xres || !settings.yres || !settings.refresh ||
			!bits)
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
	optype = (int *)malloc(sizeof(int));
	*optype = 0;
	lastop = (int *)malloc(sizeof(int));
	*lastop = 0; 
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

QGfx *QQnxScreen::createGfx(unsigned char *bytes, int w, int h, int d, int linestep) {
	if (bytes == screen->vidptr) {
		// Taken from QScreen::createGfx
	    QGfx* ret;
	    if ( FALSE ) {
	    //Just to simplify the ifdeffery
		#ifndef QT_NO_QWS_DEPTH_1
	    } else if(d==1) {
		    ret = new QQnxFbGfx<1,0>(screen, &adapter, displayId, &modeFuncList, &memFuncList, &coreFuncList, &miscFuncList, &contextFuncList);
		#endif
		#ifndef QT_NO_QWS_DEPTH_4
	    } else if(d==4) {
		    ret = new QQnxFbGfx<4,0>(screen, &adapter, displayId, &modeFuncList, &memFuncList, &coreFuncList, &miscFuncList, &contextFuncList);
		#endif
		#ifndef QT_NO_QWS_DEPTH_16
	    } else if(d==16) {
		    ret = new QQnxFbGfx<16,0>(screen, &adapter, displayId, &modeFuncList, &memFuncList, &coreFuncList, &miscFuncList, &contextFuncList);
		#endif
		#ifndef QT_NO_QWS_DEPTH_8
	    } else if(d==8) {
		    ret = new QQnxFbGfx<8,0>(screen, &adapter, displayId, &modeFuncList, &memFuncList, &coreFuncList, &miscFuncList, &contextFuncList);
		#endif
		#ifndef QT_NO_QWS_DEPTH_8GRAYSCALE
	    } else if(d==8) {
		    ret = new QQnxFbGfx<8,0>(screen, &adapter, displayId, &modeFuncList, &memFuncList, &coreFuncList, &miscFuncList, &contextFuncList);
		#endif
		#ifndef QT_NO_QWS_DEPTH_24
	    } else if(d==24) {
		    ret = new QQnxFbGfx<24,0>(screen, &adapter, displayId, &modeFuncList, &memFuncList, &coreFuncList, &miscFuncList, &contextFuncList);
		#endif
		#ifndef QT_NO_QWS_DEPTH_32
	    } else if(d==32) {
		    ret = new QQnxFbGfx<32,0>(screen, &adapter, displayId, &modeFuncList, &memFuncList, &coreFuncList, &miscFuncList, &contextFuncList);
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
/*
QGfx *QQnxScreen::screenGfx() {
printf("screenGfx()\n");
	return createGfx(screen->vidptr, screen->width, screen->height, DISP_BITS_PER_PIXEL(screen->pixel_format), screen->stride);
}
*/

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
