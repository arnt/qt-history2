/****************************************************************************
**
** Definition of QPaintDevice class.
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

#ifndef QPAINTDEVICE_H
#define QPAINTDEVICE_H

#ifndef QT_H
#include "qwindowdefs.h"
#include "qrect.h"
#include "qshared.h"
#endif // QT_H

#if defined(Q_WS_QWS)
class QWSDisplay;
class QGfx;
#endif

class QIODevice;
class QString;
class QTextItem;
class QApplicationPrivate;
class QAbstractGC;

#if defined(Q_WS_X11)
struct QPaintDeviceX11Data;
#endif

class Q_GUI_EXPORT QPaintDevice				// device for QPainter
{
public:
    virtual ~QPaintDevice();

    int		devType() const;
    bool	isExtDev() const;
    bool	paintingActive() const;

    virtual void setResolution( int );
    virtual int resolution() const;

    // Windows:	  get device context
    // X-Windows: get drawable
#if defined(Q_WS_WIN)
    virtual HDC		handle() const;
#elif defined(Q_WS_X11)
    virtual Qt::HANDLE	handle() const;
    virtual Qt::HANDLE  x11RenderHandle() const;
#elif defined(Q_WS_MAC)
    virtual Qt::HANDLE      handle() const;
    virtual Qt::HANDLE      macCGHandle() const;
#elif defined(Q_WS_QWS)
    virtual Qt::HANDLE	handle() const;
#endif
    virtual QAbstractGC *gc() const { return deviceGC; }

#if defined(Q_WS_X11)
    Display 	   *x11Display() const;
    int		    x11Screen() const;
    int		    x11Depth() const;
    int		    x11Cells() const;
    Qt::HANDLE	    x11Colormap() const;
    bool	    x11DefaultColormap() const;
    void	   *x11Visual() const;
    bool	    x11DefaultVisual() const;

    static Display *x11AppDisplay();
    static int	    x11AppScreen();

    static int      x11AppDpiX();
    static int      x11AppDpiY();
    static void     x11SetAppDpiX(int);
    static void     x11SetAppDpiY(int);
    static int	    x11AppDepth();
    static int	    x11AppCells();
    static Qt::HANDLE   x11AppRootWindow();
    static Qt::HANDLE   x11AppColormap();
    static bool     x11AppDefaultColormap();
    static void    *x11AppVisual();
    static bool	    x11AppDefaultVisual();

    // ### in 4.0, the above need to go away, the below needs to take a -1 default
    // argument, signifying the default screen...
    static int	    x11AppDepth( int screen );
    static int	    x11AppCells( int screen );
    static Qt::HANDLE   x11AppRootWindow( int screen );
    static Qt::HANDLE   x11AppColormap( int screen );
    static void    *x11AppVisual( int screen );
    static bool     x11AppDefaultColormap( int screen );
    static bool	    x11AppDefaultVisual( int screen );
    static int      x11AppDpiX( int );
    static int      x11AppDpiY( int );
    static void     x11SetAppDpiX( int, int );
    static void     x11SetAppDpiY( int, int );
#endif

#if defined(Q_WS_QWS)
    static QWSDisplay *qwsDisplay();
    virtual unsigned char * scanLine(int) const;
    virtual int bytesPerLine() const;
    virtual QGfx * graphicsContext(bool clip_children=TRUE) const;
#endif

protected:
    QPaintDevice( uint devflags );

#if defined(Q_WS_WIN)
    HDC		hdc;				// device context
#elif defined(Q_WS_X11)
    Qt::HANDLE	hd;				// handle to drawable
    Qt::HANDLE  rendhd;                         // handle to RENDER pict

    void		 copyX11Data( const QPaintDevice * );
    void		 cloneX11Data( const QPaintDevice * );
    virtual void	 setX11Data( const QPaintDeviceX11Data* );
    QPaintDeviceX11Data* getX11Data( bool def=FALSE ) const;
#elif defined(Q_WS_MAC)
    Qt::HANDLE hd;
    Qt::HANDLE cg_hd;
#elif defined(Q_WS_QWS)
    Qt::HANDLE hd;
#endif

    virtual int	 metric( int ) const;
    virtual int	 fontMet( QFont *, int, const char * = 0, int = 0 ) const;
    virtual int	 fontInf( QFont *, int ) const;

    ushort	devFlags;			// device flags
    ushort	painters;			// refcount

    friend class QPainter;
    friend class QPaintDeviceMetrics;
#if defined(Q_WS_WIN)
    friend class QWin32GC;
#elif defined(Q_WS_MAC)
    friend class QQuickDrawGC;
#endif
#if defined(Q_WS_MAC)
    friend Q_GUI_EXPORT void unclippedScaledBitBlt( QPaintDevice *, int, int, int, int,
						const QPaintDevice *, int, int, int, int, Qt::RasterOp, bool, bool );
#else
    friend Q_GUI_EXPORT void bitBlt( QPaintDevice *, int, int,
				 const QPaintDevice *,
				 int, int, int, int, Qt::RasterOp, bool );
#endif
#if defined(Q_WS_X11)
    friend void qt_init( QApplicationPrivate *, int, Display *, Qt::HANDLE, Qt::HANDLE );
    friend void qt_cleanup();
#endif

protected:
    QAbstractGC *deviceGC;

private:
#if defined(Q_WS_X11)
    static Display *x_appdisplay;
    static int	    x_appscreen;

    static int	    x_appdepth;
    static int	    x_appcells;
    static Qt::HANDLE   x_approotwindow;
    static Qt::HANDLE   x_appcolormap;
    static bool	    x_appdefcolormap;
    static void	   *x_appvisual;
    static bool     x_appdefvisual;

    // ### in 4.0, remove the above, and replace with the below
    static int	      *x_appdepth_arr;
    static int	      *x_appcells_arr;
    static Qt::HANDLE *x_approotwindow_arr;
    static Qt::HANDLE *x_appcolormap_arr;
    static bool	      *x_appdefcolormap_arr;
    static void	     **x_appvisual_arr;
    static bool       *x_appdefvisual_arr;

    QPaintDeviceX11Data* x11Data;
#endif

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QPaintDevice( const QPaintDevice & );
    QPaintDevice &operator=( const QPaintDevice & );
#endif
};


Q_GUI_EXPORT
void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx=0, int sy=0, int sw=-1, int sh=-1,
	     Qt::RasterOp = Qt::CopyROP, bool ignoreMask=FALSE );

Q_GUI_EXPORT
void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QImage *src, int sx=0, int sy=0, int sw=-1, int sh=-1,
	     int conversion_flags=0 );


#if defined(Q_WS_X11)

struct Q_GUI_EXPORT QPaintDeviceX11Data : public QShared {
    Display*	x_display;
    int		x_screen;
    int		x_depth;
    int		x_cells;
    Qt::HANDLE	x_colormap;
    bool	x_defcolormap;
    void*	x_visual;
    bool	x_defvisual;
};

#endif

/*****************************************************************************
  Inline functions
 *****************************************************************************/

inline int QPaintDevice::devType() const
{ return devFlags & QInternal::DeviceTypeMask; }

inline bool QPaintDevice::isExtDev() const
{ return (devFlags & QInternal::ExternalDevice) != 0; }

inline bool QPaintDevice::paintingActive() const
{ return painters != 0; }

#if defined(Q_WS_X11)
inline Display *QPaintDevice::x11AppDisplay()
{ return x_appdisplay; }

inline int QPaintDevice::x11AppScreen()
{ return x_appscreen; }

inline int QPaintDevice::x11AppDepth( int screen )
{ return x_appdepth_arr[ screen == -1 ? x_appscreen : screen ]; }

inline int QPaintDevice::x11AppCells( int screen )
{ return x_appcells_arr[ screen == -1 ? x_appscreen : screen ]; }

inline Qt::HANDLE QPaintDevice::x11AppRootWindow( int screen )
{ return x_approotwindow_arr[ screen == -1 ? x_appscreen : screen ]; }

inline bool QPaintDevice::x11AppDefaultColormap( int screen )
{ return x_appdefcolormap_arr[ screen == -1 ? x_appscreen : screen ]; }

inline bool QPaintDevice::x11AppDefaultVisual( int screen )
{ return x_appdefvisual_arr[ screen == -1 ? x_appscreen : screen ]; }

inline int QPaintDevice::x11AppDepth()
{ return x_appdepth; }

inline int QPaintDevice::x11AppCells()
{ return x_appcells; }

inline Qt::HANDLE QPaintDevice::x11AppRootWindow()
{ return x_approotwindow; }

inline Qt::HANDLE QPaintDevice::x11AppColormap()
{ return x_appcolormap; }

inline bool QPaintDevice::x11AppDefaultColormap()
{ return x_appdefcolormap; }

inline void *QPaintDevice::x11AppVisual()
{ return x_appvisual; }

inline bool QPaintDevice::x11AppDefaultVisual()
{ return x_appdefvisual; }

#endif // Q_WS_X11


Q_GUI_EXPORT
inline void bitBlt( QPaintDevice *dst, const QPoint &dp,
		    const QPaintDevice *src, const QRect &sr =QRect(0,0,-1,-1),
		    Qt::RasterOp rop=Qt::CopyROP, bool ignoreMask=FALSE )
{
    bitBlt( dst, dp.x(), dp.y(), src, sr.x(), sr.y(), sr.width(), sr.height(),
	    rop, ignoreMask );
}




#endif // QPAINTDEVICE_H
