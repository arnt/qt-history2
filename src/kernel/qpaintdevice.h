/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice.h#65 $
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


class Q_EXPORT QPaintDevice				// device for QPainter
{
public:
    virtual ~QPaintDevice();

    int		devType() const;
    bool	isExtDev() const;
    bool	paintingActive() const;

    // Windows:	  get device context
    // X-Windows: get drawable
#if defined(_WS_WIN_)
    HDC		handle() const;
#elif defined(_WS_X11_)
    HANDLE	handle() const;
#endif

#if defined(_WS_X11_)
    Display 	   *x11Display() const;
    int		    x11Screen() const;
    int		    x11Depth() const;
    int		    x11Cells() const;
    HANDLE	    x11Colormap() const;
    bool	    x11DefaultColormap() const;
    void	   *x11Visual() const;
    bool	    x11DefaultVisual() const;

    static Display *x11AppDisplay();
    static int	    x11AppScreen();
    static int	    x11AppDepth();
    static int	    x11AppCells();
    static HANDLE   x11AppColormap();
    static bool     x11AppDefaultColormap();
    static void    *x11AppVisual();
    static bool	    x11AppDefaultVisual();
#endif

protected:
    QPaintDevice( uint devflags );

#if defined(_WS_WIN_)
    HDC		hdc;				// device context
#elif defined(_WS_X11_)
    HANDLE	hd;				// handle to drawable
    void	copyX11Data( const QPaintDevice * );
#endif

    virtual bool cmd( int, QPainter *, QPDevCmdParam * );
    virtual int	 metric( int ) const;
    virtual int	 fontMet( QFont *, int, const char * = 0, int = 0 ) const;
    virtual int	 fontInf( QFont *, int ) const;

    ushort	devFlags;			// device flags
    ushort	painters;			// refcount

    friend class QPainter;
    friend class QPaintDeviceMetrics;
    friend Q_EXPORT void bitBlt( QPaintDevice *, int, int,
				 const QPaintDevice *,
				 int, int, int, int, Qt::RasterOp, bool );
#if defined(_WS_X11_)
    friend void qt_init_internal( int *, char **, Display * );
#endif

private:
#if defined(_WS_X11_)
    static Display *x_appdisplay;
    static int	    x_appscreen;
    static int	    x_appdepth;
    static int	    x_appcells;
    static HANDLE   x_appcolormap;
    static bool	    x_appdefcolormap;
    static void	   *x_appvisual;
    static bool     x_appdefvisual;
    struct QPaintDeviceX11Data {
	Display *x_display;
	int	 x_screen;
	int	 x_depth;
	int	 x_cells;
	HANDLE   x_colormap;
	bool	 x_defcolormap;
	void	*x_visual;
	bool	 x_defvisual;
    } *x11Data;
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
{ return painters != 0; }

#if defined(_WS_WIN_)
inline HDC    QPaintDevice::handle() const { return hdc; }
#elif defined(_WS_X11_)
inline HANDLE QPaintDevice::handle() const { return hd; }
#endif

#if defined(_WS_X11_)
inline Display *QPaintDevice::x11Display() const
{ return x11Data ? x11Data->x_display : x_appdisplay; }

inline int QPaintDevice::x11Screen() const
{ return x11Data ? x11Data->x_screen : x_appscreen; }

inline int QPaintDevice::x11Depth() const
{ return x11Data ? x11Data->x_depth : x_appdepth; }

inline int QPaintDevice::x11Cells() const
{ return x11Data ? x11Data->x_cells : x_appcells; }

inline HANDLE QPaintDevice::x11Colormap() const
{ return x11Data ? x11Data->x_colormap : x_appcolormap; }

inline bool QPaintDevice::x11DefaultColormap() const
{ return x11Data ? x11Data->x_defcolormap : x_appdefcolormap; }

inline void *QPaintDevice::x11Visual() const
{ return x11Data ? x11Data->x_visual : x_appvisual; }

inline bool QPaintDevice::x11DefaultVisual() const
{ return x11Data ? x11Data->x_defvisual : x_appdefvisual; }

inline Display *QPaintDevice::x11AppDisplay()
{ return x_appdisplay; }

inline int QPaintDevice::x11AppScreen()
{ return x_appscreen; }

inline int QPaintDevice::x11AppDepth()
{ return x_appdepth; }

inline int QPaintDevice::x11AppCells()
{ return x_appcells; }

inline HANDLE QPaintDevice::x11AppColormap()
{ return x_appcolormap; }

inline bool QPaintDevice::x11AppDefaultColormap()
{ return x_appdefcolormap; }

inline void *QPaintDevice::x11AppVisual()
{ return x_appvisual; }

inline bool QPaintDevice::x11AppDefaultVisual()
{ return x_appdefvisual; }
#endif // _WS_X11_


Q_EXPORT
inline void bitBlt( QPaintDevice *dst, const QPoint &dp,
		    const QPaintDevice *src, const QRect &sr =QRect(0,0,-1,-1),
		    Qt::RasterOp rop=Qt::CopyROP, bool ignoreMask=FALSE )
{
    bitBlt( dst, dp.x(), dp.y(), src, sr.x(), sr.y(), sr.width(), sr.height(),
	    rop, ignoreMask );
}


#endif // QPAINTDEVICE_H
