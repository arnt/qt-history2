/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintd.h#32 $
**
** Definition of QPaintDevice class
**
** Author  : Haavard Nord
** Created : 940721
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
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
#define PDT_PICTURE	0x04
#define PDT_MASK	0x0f


// Painter device flags

#define PDF_EXTDEV	0x10
#define PDF_PAINTACTIVE 0x20


// Painter device command param (defined in qpaintdc.h)

union QPDevCmdParam;


class QPaintDevice				// device for QPainter
{
public:
    virtual ~QPaintDevice();

    int	     devType()	      const;
    bool     isExtDev()	      const;
    bool     paintingActive() const;

#if defined(_WS_WIN_)
    HDC	     handle()  const;			// get device context
#elif defined(_WS_PM_)
    HPS	     handle()  const;			// get presentation space
#elif defined(_WS_X11_)
    Display *display() const;			// get display
    WId	     handle()  const;			// get drawable
#endif

protected:
    QPaintDevice( uint devflags );

#if defined(_WS_WIN_)
    HDC	     hdc;				// device context
#elif defined(_WS_PM_)
    HPS	     hps;				// presentation space
#elif defined(_WS_X11_)
    Display *dpy;				// display
    HANDLE   hd;				// handle to drawable
#endif

    virtual bool cmd( int, QPainter *, QPDevCmdParam * );
    virtual long metric( int ) const;

    uint     devFlags;				// device flags

    friend class QPainter;
    friend class QPaintDeviceMetrics;
    friend void bitBlt( QPaintDevice *, int, int, const QPaintDevice *,
			int, int, int, int, RasterOp );
};


void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx=0, int sy=0, int sw=-1, int sh=-1,
	     RasterOp = CopyROP );


// ----------------------------------------------------------------------------
// Inline functions
//

inline int QPaintDevice::devType() const
{ return devFlags & PDT_MASK; }

inline bool QPaintDevice::isExtDev() const
{ return (devFlags & PDF_EXTDEV) != 0; }

inline bool QPaintDevice::paintingActive() const
{ return (devFlags & PDF_PAINTACTIVE) != 0; }

#if defined(_WS_WIN_)
inline HDC      QPaintDevice::handle()  const { return hdc; }
#elif defined(_WS_PM_)
inline HPS      QPaintDevice::handle()  const { return hps; }
#elif defined(_WS_X11_)
inline Display *QPaintDevice::display() const { return dpy; }
inline WId	QPaintDevice::handle()  const { return hd; }
#endif

inline void bitBlt( QPaintDevice *dst, const QPoint &dp,
		    const QPaintDevice *src, const QRect &sr =QRect(0,0,-1,-1),
		    RasterOp rop = CopyROP )
{
    bitBlt( dst, dp.x(), dp.y(), src, sr.x(), sr.y(), sr.width(), sr.height(),
	    rop );
}


#endif // QPAINTD_H
