//              NOTE: This file is under development, and does not yet offer
//              any useful functionality on any platform.
//
//              If you are interested in Qnx support, please email us at
//              info@trolltech.com and explain in what way you would like
//              to use Qt with QNX.
//
//
//              In the meantime, Qt/X11 is reported to work with X11/QNX.
#if defined(Q_OS_QNX6)

#ifndef QWSQNXFB_H
#define QWSQNXFB_H

#ifndef QT_H
#include <qgfxraster_qws.h>
#include <qgfx_qws.h>
#include <qpolygonscanner.h>
#include <qpen.h>
#include <qstring.h>
#endif // QT_H

#include <display.h>
#include <disputil.h>


// QnxFb Gfx class
template <const int depth, const int type> class QQnxFbGfx : public QGfxRaster<depth, type> {
public:
	QQnxFbGfx( disp_surface_t *, disp_adapter_t *, int,
			disp_modefuncs_t *, disp_memfuncs_t *,
            disp_draw_corefuncs_t *, disp_draw_miscfuncs_t *,
            disp_draw_contextfuncs_t *);
	~QQnxFbGfx();

	int bitDepth(){ return DISP_BITS_PER_PIXEL ( ctx.dsurf->pixel_format );};

	void sync ();
//	void fillRect (int,int,int,int);
//	void hlineUnclipped ( int, int, int);

private:
	disp_draw_context_t ctx;

	int displayId;

	// Pointers to dll function lists
	disp_modefuncs_t *modeFuncList;
    disp_memfuncs_t *memFuncList;
    disp_draw_corefuncs_t *coreFuncList;
    disp_draw_miscfuncs_t *miscFuncList;
    disp_draw_contextfuncs_t *contextFuncList;

	// Utility functions
	void setupCtx( disp_surface_t *, disp_draw_context_t *, disp_adapter_t * );
};

// Screen class
class QQnxScreen : public QScreen {
public:
	QQnxScreen(int display_id);
	~QQnxScreen();

	bool connect(const QString & spec);
	void disconnect();

	bool initDevice();
	void shutdownDevice();
	void setMode(int, int, int);

	QRgb *clut(){ return screen->palette; };

	QGfx* createGfx (unsigned char*, int, int, int, int);
//	QGfx* screenGfx ();

private:
	// Dll handle and function lists
	void *dllHandle;
	disp_modefuncs_t modeFuncList;
	disp_memfuncs_t memFuncList;
	disp_draw_corefuncs_t coreFuncList;
	disp_draw_miscfuncs_t miscFuncList;
	disp_draw_contextfuncs_t contextFuncList;

	int displayId;
	
	// Singleton graphics card handle
	disp_adapter_t adapter;

    uint32_t pixel_format;  

	// Screen surface
	disp_surface_t *screen;

	// Monitor settings
	disp_crtc_settings_t settings;

	// Must be preserved for initDevice()
	int (*coreFuncListFill) (disp_adapter_t *, unsigned int, disp_draw_corefuncs_t *, int);
};

#endif
#endif
