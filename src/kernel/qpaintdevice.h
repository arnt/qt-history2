/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#10 $
**
** Definition of QPaintDevice class
**
** Author  : Haavard Nord
** Created : 940721
**
** Copyright (C) 1994 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QPAINTD_H
#define QPAINTD_H

#include "qwindefs.h"
#include "qrect.h"


// Painter device types (is-A)

#define PDT_UNDEF	0x00
#define PDT_WIDGET	0x01
#define PDT_PIXMAP	0x02
#define PDT_PRINTER	0x03
#define PDT_METAFILE	0x04
#define PDT_MASK	0x0f


// Painter device flags

#define PDF_EXTDEV	0x10
#define PDF_PAINTACTIVE	0x20


// Painter device command param (defined in qpaintdc.h)

union QPDevCmdParam;


class QPaintDevice				// device for QPainter
{
friend class QPainter;
friend class QPaintDeviceMetrics;
public:
    QPaintDevice();
    virtual ~QPaintDevice();

    int	     devType()	      const { return devFlags & PDT_MASK; }
    bool     paintingActive() const { return (devFlags & PDF_PAINTACTIVE) ==
					     PDF_PAINTACTIVE; }

    void     bitBlt( int sx, int sy, int sw, int sh, QPaintDevice *dest,
		     int dx, int dy, RasterOp =CopyROP );
    void     bitBlt( const QRect &srcrect, QPaintDevice *dest,
		     const QPoint &dpos, RasterOp =CopyROP );

#if defined(_WS_WIN_)
    HDC	     handle() const { return hdc; }	// get device context
#elif defined(_WS_PM_)
    HPS	     handle() const { return hps; }	// get presentation space
#elif defined(_WS_X11_)
    Display *display() const { return dpy; }	// get display
    WId	     handle() const { return hd; }	// get drawable
#endif

protected:
    void     setDevType( uint t )   { devFlags = t; }
				
#if defined(_WS_WIN_)
    HDC	     hdc;				// device context
#elif defined(_WS_PM_)
    HPS	     hps;				// presentation space
#elif defined(_WS_X11_)
    Display *dpy;				// display
    WId	     hd;				// handle to drawable
#endif

private:
    uint     devFlags;				// device flags
    virtual bool cmd( int, QPDevCmdParam * );
    virtual int  metric( int );
};


inline void QPaintDevice::bitBlt( const QRect &r, QPaintDevice *pd,
				  const QPoint &p, RasterOp rop )
{
    bitBlt( r.x(), r.y(), r.width(), r.height(), pd, p.x(), p.y(), rop );
}


#endif // QPAINTD_H
