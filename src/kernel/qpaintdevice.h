/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#62 $
**
** Definition of QPaintDevice class
**
** Created : 940721
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QPAINTDEVICE_H
#define QPAINTDEVICE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qrect.h"
#endif // QT_H


// Painter device command param (defined in qpaintdevicedefs.h)

union QPDevCmdParam;


class QPaintDevicePrivate;

class Q_EXPORT QPaintDevice				// device for QPainter
{
public:
    virtual ~QPaintDevice();

    int	     devType()	      const;
    bool     isExtDev()	      const;
    bool     paintingActive() const;

    // Windows:	  get device context
    // X-Windows: get drawable
#if defined(_WS_WIN_)
    HDC	     handle() const;
#elif defined(_WS_X11_)
    HANDLE   handle() const;
#endif

#if !defined(_WS_X11_)
#define Display void
#endif
    Display *x11Display() const;		// X11 only

#if defined(_WS_X11_)
    static Display *x__Display();
    static int	    x11Screen();
    static int	    x11Depth();
    static int	    x11Cells();
    static HANDLE   x11Colormap();
    static bool	    x11DefaultColormap();
    static void	   *x11Visual();
    static bool	    x11DefaultVisual();
#endif

protected:
    QPaintDevice( uint devflags );

#if defined(_WS_WIN_)
    HDC	     hdc;				// device context
#elif defined(_WS_X11_)
    static Display *dpy;			// display (common to all)
    HANDLE   hd;				// handle to drawable
#endif

    virtual bool cmd( int, QPainter *, QPDevCmdParam * );
    virtual int	 metric( int ) const;
    virtual int	 fontMet( QFont *, int, const char * = 0, int = 0 ) const;
    virtual int	 fontInf( QFont *, int ) const;

    uint     devFlags;				// device flags

    friend class QColor;
    friend class QPainter;
    friend class QPaintDeviceMetrics;
    friend Q_EXPORT void bitBlt( QPaintDevice *, int, int,
				 const QPaintDevice *,
				 int, int, int, int, Qt::RasterOp, bool );

private:
    QPaintDevicePrivate * d;
#if defined(_WS_X11_)
    static Display *x_display;
    static int	    x_screen;
    static int	    x_depth;
    static int	    x_cells;
    static HANDLE   x_colormap;
    static bool	    x_defcmap;
    static void	   *x_visual;
    static bool     x_defvisual;
#endif

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPaintDevice( const QPaintDevice & );
    QPaintDevice &operator=( const QPaintDevice & );
#endif
};


Q_EXPORT
void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx=0, int sy=0, int sw=-1, int sh=-1,
	     Qt::RasterOp = Qt::CopyROP, bool ignoreMask=FALSE );

Q_EXPORT
void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QImage *src, int sx=0, int sy=0, int sw=-1, int sh=-1,
	     int conversion_flags=0 );


/*****************************************************************************
  Inline functions
 *****************************************************************************/

inline int QPaintDevice::devType() const
{ return devFlags & QInternal::DeviceTypeMask; }

inline bool QPaintDevice::isExtDev() const
{ return (devFlags & QInternal::ExternalDevice) != 0; }

inline bool QPaintDevice::paintingActive() const
{ return (devFlags & QInternal::PaintingActive) != 0; }

#if defined(_WS_WIN_)
inline HDC QPaintDevice::handle() const { return hdc; }
#elif defined(_WS_X11_)
inline HANDLE QPaintDevice::handle() const { return hd; }
#endif

#if defined(_WS_X11_)
inline Display *QPaintDevice::x11Display() const { return dpy; }
#else
inline Display *QPaintDevice::x11Display() const { return 0; }
#undef Display
#endif

#if defined(_WS_X11_)
inline Display *QPaintDevice::x__Display()	   { return x_display; }
inline int	QPaintDevice::x11Screen()	   { return x_screen; }
inline int	QPaintDevice::x11Depth()	   { return x_depth; }
inline int	QPaintDevice::x11Cells()	   { return x_cells; }
inline HANDLE	QPaintDevice::x11Colormap()	   { return x_colormap; }
inline bool    	QPaintDevice::x11DefaultColormap() { return x_defcmap; }
inline void    *QPaintDevice::x11Visual()	   { return x_visual; }
inline bool    	QPaintDevice::x11DefaultVisual()   { return x_defvisual; }
#endif


Q_EXPORT
inline void bitBlt( QPaintDevice *dst, const QPoint &dp,
		    const QPaintDevice *src, const QRect &sr =QRect(0,0,-1,-1),
		    Qt::RasterOp rop=Qt::CopyROP, bool ignoreMask=FALSE )
{
    bitBlt( dst, dp.x(), dp.y(), src, sr.x(), sr.y(), sr.width(), sr.height(),
	    rop, ignoreMask );
}


#endif // QPAINTDEVICE_H
