//              NOTE: This file is under development, and does not yet offer
//              any useful functionality on any platform.
//
//              If you are interested in Qnx support, please email us at
//              info@trolltech.com and explain in what way you would like
//              to use Qt with QNX.
//
//
//              In the meantime, Qt/X11 is reported to work with X11/QNX.


#ifndef QWSQNXFB_H
#define QWSQNXFB_H

#ifndef QT_H
#include <qgfxraster_qws.h>
#include <qgfx_qws.h>
#include <qpolygonscanner.h>
#include <qpen.h>
#include <qstring.h>
#endif // QT_H

#ifdef _OS_QNX_

#include <display.h>
#include <disputil.h>

// Pixmap Gfx class
class QQnxPixmapGfx : public QGfxRaster <32,0> {
public:
	QQnxPixmapGfx(unsigned char*, int, int);
	~QQnxPixmapGfx(){};
	void blt (int, int, int, int, int, int) {};
	void setSource (const QPaintDevice * p) {};
};

// QnxFb Gfx class
class QQnxFbGfx : public QGfxRasterBase {
public:
	QQnxFbGfx( disp_surface_t *, disp_adapter_t *, int,
			disp_modefuncs_t *, disp_memfuncs_t *,
            disp_draw_corefuncs_t *, disp_draw_miscfuncs_t *,
            disp_draw_contextfuncs_t *);
	~QQnxFbGfx(){};

	void sync ();
 
	int pixelWidth () { return width; };
	int pixelHeight () { return height; };
	int bitDepth () { return DISP_BITS_PER_PIXEL (ctx.dsurf->pixel_format); };
 
	void buildSourceClut (int,int){};
 
// Over-ridden functions from QGfx
	void usePen();
	void useBrush();

	void setBackgroundColor (QColor);
	void drawPoint (int x, int y);
	void drawPoints (const QPointArray &, int, int);
	void drawPointUnclipped (int, int);
	void drawLine (int, int, int, int);
	void hlineUnclipped ( int, int, int);
	void drawThickLine ( int &, int &, int &, int &){};
	void drawPolyline (const QPointArray &, int, int);
	void drawThickPolyline (const QPointArray &, int, int);
	void fillRect (int, int, int, int);
	void drawPolygon (const QPointArray &, bool, int, int);
	void setLineStep (int){};
	void blt (int, int, int, int, int, int){};
	void scroll (int, int, int, int, int, int){};
	void tiledBlt (int, int, int, int){};
	void stretchBlt(int, int, int, int, int, int){};
	void setSource (const QPaintDevice *){};
	void setSource (const QImage *){};
	void setSourcePen (){};

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

	void set(uint, uint, uint, uint);

	QGfx* createGfx (unsigned char*, int, int, int, int);
	QGfx* screenGfx ();

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
