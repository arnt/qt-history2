/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#40 $
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

    // Windows:   get device context
    // OS/2 PM:   get presentation space
    // X-Windows: get drawable
    HANDLE   handle()  const;

#if !defined(_WS_X11_)
#define Display void
#endif
    Display *x11Display() const;		// X-Windows only

protected:
    QPaintDevice( uint devflags );

#if defined(_WS_WIN_)
    HDC	     hdc;				// device context
#elif defined(_WS_PM_)
    HPS	     hps;				// presentation space
#elif defined(_WS_X11_)
    static Display *dpy;			// display (common to all)
    HANDLE   hd;				// handle to drawable
#endif

    virtual bool cmd( int, QPainter *, QPDevCmdParam * );
    virtual int  metric( int ) const;

    uint     devFlags;				// device flags

    friend class QPainter;
    friend class QPaintDeviceMetrics;
    friend void bitBlt( QPaintDevice *, int, int, const QPaintDevice *,
			int, int, int, int, RasterOp, bool );

private:	// Disabled copy constructor and operator=
    QPaintDevice( const QPaintDevice & ) {}
    QPaintDevice &operator=( const QPaintDevice & ) { return *this; }
};


void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx=0, int sy=0, int sw=-1, int sh=-1,
	     RasterOp = CopyROP, bool ignoreMask=FALSE );


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
inline HANDLE   QPaintDevice::handle()  const { return hdc; }
#elif defined(_WS_PM_)
inline HANDLE   QPaintDevice::handle()  const { return hps; }
#elif defined(_WS_X11_)
inline HANDLE	QPaintDevice::handle()  const { return hd; }
#endif

#if defined(_WS_X11_)
inline Display *QPaintDevice::x11Display() const { return dpy; }
#else
inline Display *QPaintDevice::x11Display() const { return 0; }
#undef Display
#endif


inline void bitBlt( QPaintDevice *dst, const QPoint &dp,
		    const QPaintDevice *src, const QRect &sr =QRect(0,0,-1,-1),
		    RasterOp rop=CopyROP, bool ignoreMask=FALSE )
{
    bitBlt( dst, dp.x(), dp.y(), src, sr.x(), sr.y(), sr.width(), sr.height(),
	    rop, ignoreMask );
}


#endif // QPAINTD_H
