/****************************************************************************
**
** Implementation of QPaintDevice class for X11.
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

#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qt_x11_p.h"
#include "qx11info_x11.h"

/*!
    \class QPaintDevice qpaintdevice.h
    \brief The QPaintDevice class is the base class of objects that
    can be painted.

    \ingroup graphics
    \ingroup images

    A paint device is an abstraction of a two-dimensional space that
    can be drawn using a QPainter. The drawing capabilities are
    implemented by the subclasses QWidget, QPixmap, QPicture and
    QPrinter.

    The default coordinate system of a paint device has its origin
    located at the top-left position. X increases to the right and Y
    increases downward. The unit is one pixel. There are several ways
    to set up a user-defined coordinate system using the painter, for
    example, using QPainter::setWorldMatrix().

    Example (draw on a paint device):
    \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPainter p;                       // our painter
	p.begin( this );                  // start painting the widget
	p.setPen( red );                  // red outline
	p.setBrush( yellow );             // yellow fill
	p.drawEllipse( 10, 20, 100,100 ); // 100x100 ellipse at position (10, 20)
	p.end();                          // painting done
    }
    \endcode

    The bit block transfer is an extremely useful operation for
    copying pixels from one paint device to another (or to itself). It
    is implemented as the global function bitBlt().

    Example (scroll widget contents 10 pixels to the right):
    \code
    bitBlt( myWidget, 10, 0, myWidget );
    \endcode

    \warning Qt requires that a QApplication object exists before
    any paint devices can be created. Paint devices access window
    system resources, and these resources are not initialized before
    an application object is created.
*/

/*!
    Constructs a paint device with internal flags \a devflags. This
    constructor can be invoked only from QPaintDevice subclasses.
*/

QPaintDevice::QPaintDevice( uint devflags )
    : paintEngine(0)
{
    if ( !qApp ) {				// global constructor
	qFatal( "QPaintDevice: Must construct a QApplication before a "
		"QPaintDevice" );
	return;
    }
    devFlags = devflags;
    painters = 0;
    hd = 0;
    rendhd = 0;
}

/*!
    Destroys the paint device and frees window system resources.
*/

QPaintDevice::~QPaintDevice()
{
    if ( paintingActive() )
	qWarning( "QPaintDevice: Cannot destroy paint device that is being "
		  "painted" );
}

/*!
    \fn int QPaintDevice::devType() const

    \internal

    Returns the device type identifier, which is \c QInternal::Widget
    if the device is a QWidget, \c QInternal::Pixmap if it's a
    QPixmap, \c QInternal::Printer if it's a QPrinter, \c
    QInternal::Picture if it's a QPicture or \c
    QInternal::UndefinedDevice in other cases (which should never
    happen).
*/

/*!
    \fn bool QPaintDevice::isExtDev() const

    Returns TRUE if the device is an external paint device; otherwise
    returns FALSE.

    External paint devices cannot be bitBlt()'ed from. QPicture and
    QPrinter are external paint devices.
*/

/*!
    Returns the window system handle of the paint device, for
    low-level access. Using this function is not portable.

    The HANDLE type varies with platform; see \c qpaintdevice.h and
    \c qwindowdefs.h for details.
*/
Qt::HANDLE QPaintDevice::handle() const
{
    return hd;
}

/*!
    Returns the window system handle of the paint device for XRender
    support. Use of this function is not portable. This function will
    return 0 if XRender support is not compiled into Qt, if the
    XRender extension is not supported on the X11 display, or if the
    handle could not be created.
*/
Qt::HANDLE QPaintDevice::x11RenderHandle() const
{
#ifndef QT_NO_XFTFREETYPE
    return rendhd ? XftDrawPicture( (XftDraw *) rendhd ) : 0;
#else
    return 0;
#endif // QT_NO_XFTFREETYPE
}


/*!
    \fn bool QPaintDevice::paintingActive() const

    Returns TRUE if the device is being painted, i.e. someone has
    called QPainter::begin() but not yet called QPainter::end() for
    this device; otherwise returns FALSE.

    \sa QPainter::isActive()
*/

/*!
    \internal

    Internal virtual function that returns paint device metrics.

    Please use the QPaintDeviceMetrics class instead.
*/

int QPaintDevice::metric( int ) const
{
    qWarning( "QPaintDevice::metrics: Device has no metric information" );
    return 0;
}

/*!
    \internal

    Internal virtual function. Reserved for future use.

    Please use the QFontMetrics class instead.
*/

int QPaintDevice::fontMet( QFont *, int, const char *, int ) const
{
    return 0;
}

/*!
    \internal

    Internal virtual function. Reserved for future use.

    Please use the QFontInfo class instead.
*/

int QPaintDevice::fontInf( QFont *, int ) const
{
    return 0;
}


//
// Internal functions for simple GC caching for blt'ing masked pixmaps.
// This cache is used when the pixmap optimization is set to Normal
// and the pixmap size doesn't exceed 128x128.
//

static bool      init_mask_gc = FALSE;
static const int max_mask_gcs = 11;		// suitable for hashing

struct mask_gc {
    GC	gc;
    int mask_no;
};

static mask_gc gc_vec[max_mask_gcs];


static void cleanup_mask_gc()
{
    Display *dpy = QX11Info::appDisplay();
    init_mask_gc = FALSE;
    for ( int i=0; i<max_mask_gcs; i++ ) {
	if ( gc_vec[i].gc )
	    XFreeGC( dpy, gc_vec[i].gc );
    }
}

static GC cache_mask_gc( Display *dpy, Drawable hd, int mask_no, Pixmap mask )
{
    if ( !init_mask_gc ) {			// first time initialization
	init_mask_gc = TRUE;
	qAddPostRoutine( cleanup_mask_gc );
	for ( int i=0; i<max_mask_gcs; i++ )
	    gc_vec[i].gc = 0;
    }
    mask_gc *p = &gc_vec[mask_no % max_mask_gcs];
    if ( !p->gc || p->mask_no != mask_no ) {	// not a perfect match
	if ( !p->gc ) {				// no GC
	    p->gc = XCreateGC( dpy, hd, 0, 0 );
	    XSetGraphicsExposures( dpy, p->gc, False );
	}
	XSetClipMask( dpy, p->gc, mask );
	p->mask_no = mask_no;
    }
    return p->gc;
}


/*!
    \relates QPaintDevice

    Copies a block of pixels from \a src to \a dst, perhaps merging
    each pixel according to the raster operation \a rop. \a sx, \a sy
    is the top-left pixel in \a src (0, 0) by default, \a dx, \a dy is
    the top-left position in \a dst and \a sw, \a sh is the size of
    the copied block (all of \a src by default).

    The most common values for \a rop are CopyROP and XorROP; the \l
    Qt::RasterOp documentation defines all the possible values.

    If \a ignoreMask is FALSE (the default) and \a src is a
    masked QPixmap, the entire blit is masked by \a{src}->mask().

    If \a src, \a dst, \a sw or \a sh is 0, bitBlt() does nothing. If
    \a sw or \a sh is negative bitBlt() copies starting at \a sx (and
    respectively, \a sy) and ending at the right end (respectively,
    bottom) of \a src.

    \a src must be a QWidget or QPixmap. You cannot blit from a
    QPrinter, for example. bitBlt() does nothing if you attempt to
    blit from an unsupported device.

    bitBlt() does nothing if \a src has a greater depth than \e dst.
    If you need to for example, draw a 24-bit pixmap on an 8-bit
    widget, you must use drawPixmap().
*/

void bitBlt( QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx, int sy, int sw, int sh,
	     Qt::RasterOp rop, bool ignoreMask )
{
    if ( !src || !dst ) {
	Q_ASSERT( src != 0 );
	Q_ASSERT( dst != 0 );
	return;
    }
    if ( !src->handle() || src->isExtDev() )
	return;

    QPoint redirection_offset;
    const QPaintDevice *redirected = QPainter::redirected(dst, &redirection_offset);
    if (redirected) {
	dst = const_cast<QPaintDevice*>(redirected);
 	dx -= redirection_offset.x();
 	dy -= redirection_offset.y();
    }

    int ts = src->devType();			// from device type
    int td = dst->devType();			// to device type

    QX11Info *src_xf = 0, *dst_xf = 0;
    if (ts == QInternal::Widget)  // ### fix
	src_xf = static_cast<const QWidget *>(src)->x11Info();
    else if (ts == QInternal::Pixmap)
	src_xf = static_cast<const QPixmap *>(src)->x11Info();

    if (td == QInternal::Widget) // ### fix
	dst_xf = static_cast<const QWidget *>(dst)->x11Info();
    else if (td == QInternal::Pixmap)
	dst_xf = static_cast<const QPixmap *>(dst)->x11Info();

    Q_ASSERT(src_xf != 0 && dst_xf != 0);

    Display *dpy = src_xf->display();

    if ( sw <= 0 ) {				// special width
	if ( sw < 0 )
	    sw = src->metric( QPaintDeviceMetrics::PdmWidth ) - sx;
	else
	    return;
    }
    if ( sh <= 0 ) {				// special height
	if ( sh < 0 )
	    sh = src->metric( QPaintDeviceMetrics::PdmHeight ) - sy;
	else
	    return;
    }

#if 0 // ### port
    if ( dst->paintingActive() && dst->isExtDev() ) {
	QPixmap *pm;				// output to picture/printer
	bool	 tmp_pm = TRUE;
	if ( ts == QInternal::Pixmap ) {
	    pm = (QPixmap*)src;
	    if ( sx != 0 || sy != 0 ||
		 sw != pm->width() || sh != pm->height() || ignoreMask ) {
		QPixmap *tmp = new QPixmap( sw, sh, pm->depth() );
		bitBlt( tmp, 0, 0, pm, sx, sy, sw, sh, Qt::CopyROP, TRUE );
		if ( pm->mask() && !ignoreMask ) {
		    QBitmap mask( sw, sh );
		    bitBlt( &mask, 0, 0, pm->mask(), sx, sy, sw, sh,
			    Qt::CopyROP, TRUE );
		    tmp->setMask( mask );
		}
		pm = tmp;
	    } else {
		tmp_pm = FALSE;
	    }
	} else if ( ts == QInternal::Widget ) {// bitBlt to temp pixmap
	    pm = new QPixmap( sw, sh );
	    bitBlt( pm, 0, 0, src, sx, sy, sw, sh );
	} else {
	    qWarning( "bitBlt: Cannot bitBlt from device" );
	    return;
	}
	QPDevCmdParam param[3];
	QPoint p(dx,dy);
	param[0].point	= &p;
	param[1].pixmap = pm;
	dst->cmd( QPaintDevice::PdcDrawPixmap, 0, param );
	if ( tmp_pm )
	    delete pm;
	return;
    }
#endif

    switch ( ts ) {
    case QInternal::Widget:
    case QInternal::Pixmap:
    case QInternal::System:			// OK, can blt from these
	break;
    default:
	qWarning( "bitBlt: Cannot bitBlt from device type %x", ts );
	return;
    }
    switch ( td ) {
    case QInternal::Widget:
    case QInternal::Pixmap:
    case QInternal::System:			// OK, can blt to these
	break;
    default:
	qWarning( "bitBlt: Cannot bitBlt to device type %x", td );
	return;
    }

    static const short ropCodes[] = {			// ROP translation table
	GXcopy, GXor, GXxor, GXandInverted,
	GXcopyInverted, GXorInverted, GXequiv, GXand,
	GXinvert, GXclear, GXset, GXnoop,
	GXandReverse, GXorReverse, GXnand, GXnor
    };
    if ( rop > Qt::LastROP ) {
	qWarning( "bitBlt: Invalid ROP code" );
	return;
    }

    if ( dst->handle() == 0 ) {
	qWarning( "bitBlt: Cannot bitBlt to device" );
	return;
    }

    bool mono_src;
    bool mono_dst;
    bool include_inferiors = FALSE;
    bool graphics_exposure = FALSE;
    QPixmap *src_pm;
    QBitmap *mask;

    if ( ts == QInternal::Pixmap ) {
	src_pm = (QPixmap*)src;
    	if ( src_pm->x11Info()->screen() != dst_xf->screen() )
    	    src_pm->x11SetScreen( dst_xf->screen() );
	mono_src = src_pm->depth() == 1;
	mask = ignoreMask ? 0 : src_pm->data->mask;
    } else {
	src_pm = 0;
	mono_src = FALSE;
	mask = 0;
	include_inferiors = ((QWidget*)src)->testWFlags(Qt::WPaintUnclipped);
	graphics_exposure = td == QInternal::Widget;
    }
    if ( td == QInternal::Pixmap ) {
      	if ( dst_xf->screen() != src_xf->screen() )
      	    ((QPixmap*)dst)->x11SetScreen( src_xf->screen() );
	mono_dst = ((QPixmap*)dst)->depth() == 1;
	((QPixmap*)dst)->detach();		// changes shared pixmap
    } else {
	mono_dst = FALSE;
	include_inferiors = include_inferiors ||
			    ((QWidget*)dst)->testWFlags(Qt::WPaintUnclipped);
    }

    if ( mono_dst && !mono_src ) {	// dest is 1-bit pixmap, source is not
	qWarning( "bitBlt: Incompatible destination pixmap" );
	return;
    }

#ifndef QT_NO_XRENDER
    if (src_pm && !mono_src && src_pm->data->alphapm && !ignoreMask ) {
	// use RENDER to do the blit
	QPixmap *alpha = src_pm->data->alphapm;
	if (src->x11RenderHandle() &&
	    alpha->x11RenderHandle() &&
	    dst->x11RenderHandle()) {
	    XRenderPictureAttributes pattr;
	    ulong picmask = 0;
	    if (include_inferiors) {
		pattr.subwindow_mode = IncludeInferiors;
		picmask |= CPSubwindowMode;
	    }
	    if (graphics_exposure) {
		pattr.graphics_exposures = TRUE;
		picmask |= CPGraphicsExposure;
	    }
	    if (picmask)
		XRenderChangePicture(dpy, dst->x11RenderHandle(), picmask, &pattr);
	    XRenderComposite(dpy, PictOpOver, src->x11RenderHandle(),
			     alpha->x11RenderHandle(), dst->x11RenderHandle(),
			     sx, sy, sx, sy, dx, dy, sw, sh);
	    // restore attributes
	    pattr.subwindow_mode = ClipByChildren;
	    pattr.graphics_exposures = FALSE;
	    if (picmask)
		XRenderChangePicture(dpy, dst->x11RenderHandle(), picmask, &pattr);
	    return;
	}
    }
#endif

    GC gc;

    if ( mask && !mono_src ) {			// fast masked blt
	bool temp_gc = FALSE;
	if ( mask->data->maskgc ) {
	    gc = (GC)mask->data->maskgc;	// we have a premade mask GC
	} else {
	    if ( FALSE && src_pm->optimization() == QPixmap::NormalOptim ) { // #### cache disabled
		// Compete for the global cache
		gc = cache_mask_gc( dpy, dst->handle(),
				    mask->data->ser_no,
				    mask->handle() );
	    } else {
		// Create a new mask GC. If BestOptim, we store the mask GC
		// with the mask (not at the pixmap). This way, many pixmaps
		// which have a common mask will be optimized at no extra cost.
		gc = XCreateGC( dpy, dst->handle(), 0, 0 );
		XSetGraphicsExposures( dpy, gc, False );
		XSetClipMask( dpy, gc, mask->handle() );
		if ( src_pm->optimization() == QPixmap::BestOptim ) {
		    mask->data->maskgc = gc;
		} else {
		    temp_gc = TRUE;
		}
	    }
	}
	XSetClipOrigin( dpy, gc, dx-sx, dy-sy );
	if ( rop != Qt::CopyROP )		// use non-default ROP code
	    XSetFunction( dpy, gc, ropCodes[rop] );
	if ( include_inferiors ) {
	    XSetSubwindowMode( dpy, gc, IncludeInferiors );
	    XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		       dx, dy );
	    XSetSubwindowMode( dpy, gc, ClipByChildren );
	} else {
	    XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		       dx, dy );
	}

	if ( temp_gc )				// delete temporary GC
	    XFreeGC( dpy, gc );
	else if ( rop != Qt::CopyROP )		// restore ROP
	    XSetFunction( dpy, gc, GXcopy );
	return;
    }

    gc = qt_xget_temp_gc( dst_xf->screen(), mono_dst );		// get a reusable GC

    if ( rop != Qt::CopyROP )			// use non-default ROP code
	XSetFunction( dpy, gc, ropCodes[rop] );

    if ( mono_src && mono_dst && src == dst ) { // dst and src are the same bitmap
	XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh, dx, dy );
    } else if ( mono_src ) {			// src is bitmap
	XGCValues gcvals;
	ulong	  valmask = GCBackground | GCForeground | GCFillStyle |
			    GCStipple | GCTileStipXOrigin | GCTileStipYOrigin;
	if ( td == QInternal::Widget ) {	// set GC colors
	    QWidget *w = (QWidget *)dst;
	    gcvals.background = w->palette().color(w->backgroundRole()).pixel( dst_xf->screen() );
	    gcvals.foreground = w->palette().color(w->foregroundRole()).pixel( dst_xf->screen() );
	    if ( include_inferiors ) {
		valmask |= GCSubwindowMode;
		gcvals.subwindow_mode = IncludeInferiors;
	    }
	} else if ( mono_dst ) {
	    gcvals.background = 0;
	    gcvals.foreground = 1;
	} else {
	    gcvals.background = QColor(Qt::white).pixel(dst_xf->screen());
	    gcvals.foreground = QColor(Qt::black).pixel(dst_xf->screen());
	}

	gcvals.fill_style  = FillOpaqueStippled;
	gcvals.stipple	   = src->handle();
	gcvals.ts_x_origin = dx - sx;
	gcvals.ts_y_origin = dy - sy;

	bool clipmask = FALSE;
	if ( mask ) {
	    if ( ((QPixmap*)src)->data->selfmask ) {
		gcvals.fill_style = FillStippled;
	    } else {
		XSetClipMask( dpy, gc, mask->handle() );
		XSetClipOrigin( dpy, gc, dx-sx, dy-sy );
		clipmask = TRUE;
	    }
	}

	XChangeGC( dpy, gc, valmask, &gcvals );
	XFillRectangle( dpy,dst->handle(), gc, dx, dy, sw, sh );

	valmask = GCFillStyle | GCTileStipXOrigin | GCTileStipYOrigin;
	gcvals.fill_style  = FillSolid;
	gcvals.ts_x_origin = 0;
	gcvals.ts_y_origin = 0;
	if ( include_inferiors ) {
	    valmask |= GCSubwindowMode;
	    gcvals.subwindow_mode = ClipByChildren;
	}
	XChangeGC( dpy, gc, valmask, &gcvals );

	if ( clipmask ) {
	    XSetClipOrigin( dpy, gc, 0, 0 );
	    XSetClipMask( dpy, gc, None );
	}

    } else {					// src is pixmap/widget

	if ( graphics_exposure )		// widget to widget
	    XSetGraphicsExposures( dpy, gc, True );
	if ( include_inferiors ) {
	    XSetSubwindowMode( dpy, gc, IncludeInferiors );
	    XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		       dx, dy );
	    XSetSubwindowMode( dpy, gc, ClipByChildren );
	} else {
	    XCopyArea( dpy, src->handle(), dst->handle(), gc, sx, sy, sw, sh,
		       dx, dy );
	}
	if ( graphics_exposure )		// reset graphics exposure
	    XSetGraphicsExposures( dpy, gc, False );
    }

    if ( rop != Qt::CopyROP )			// restore ROP
	XSetFunction( dpy, gc, GXcopy );
}


/*!
    \relates QPaintDevice

    \overload void bitBlt( QPaintDevice *dst, const QPoint &dp, const QPaintDevice *src, const QRect &sr, RasterOp rop )

    Overloaded bitBlt() with the destination point \a dp and source
    rectangle \a sr.
*/


/*!
  \internal
*/
// makes it possible to add a setResolution as we have in QPrinter for all
// paintdevices without breaking bin compatibility.
void QPaintDevice::setResolution( int )
{
}

/*!\internal
*/
int QPaintDevice::resolution() const
{
    return metric( QPaintDeviceMetrics::PdmDpiY );
}

#ifdef QT_COMPAT

Display *QPaintDevice::x11Display() const
{
    if (devType() == QInternal::Widget) // ### fix these in all fu's below
	return static_cast<const QWidget *>(this)->x11Info()->display();
    else if (devType() == QInternal::Pixmap)
	return static_cast<const QPixmap *>(this)->x11Info()->display();
    return QX11Info::appDisplay();
}

int QPaintDevice::x11Screen() const
{
    if (devType() == QInternal::Widget)
	return static_cast<const QWidget *>(this)->x11Info()->screen();
    else if (devType() == QInternal::Pixmap)
	return static_cast<const QPixmap *>(this)->x11Info()->screen();
    return QX11Info::appScreen();
}

void *QPaintDevice::x11Visual() const
{
    if (devType() == QInternal::Widget)
	return static_cast<const QWidget *>(this)->x11Info()->visual();
    else if (devType() == QInternal::Pixmap)
	return static_cast<const QPixmap *>(this)->x11Info()->visual();
    return QX11Info::appVisual();
}

int QPaintDevice::x11Depth() const
{
    if (devType() == QInternal::Widget)
	return static_cast<const QWidget *>(this)->x11Info()->depth();
    else if (devType() == QInternal::Pixmap)
	return static_cast<const QPixmap *>(this)->x11Info()->depth();
    return QX11Info::appDepth();
}

int QPaintDevice::x11Cells() const
{
    if (devType() == QInternal::Widget)
	return static_cast<const QWidget *>(this)->x11Info()->cells();
    else if (devType() == QInternal::Pixmap)
	return static_cast<const QPixmap *>(this)->x11Info()->cells();
    return QX11Info::appCells();
}

Qt::HANDLE QPaintDevice::x11Colormap() const
{
    if (devType() == QInternal::Widget)
	return static_cast<const QWidget *>(this)->x11Info()->colormap();
    else if (devType() == QInternal::Pixmap)
	return static_cast<const QPixmap *>(this)->x11Info()->colormap();
    return QX11Info::appColormap();
}

bool QPaintDevice::x11DefaultColormap() const
{
    if (devType() == QInternal::Widget)
	return static_cast<const QWidget *>(this)->x11Info()->defaultColormap();
    else if (devType() == QInternal::Pixmap)
	return static_cast<const QPixmap *>(this)->x11Info()->defaultColormap();
    return QX11Info::appDefaultColormap();
}

bool QPaintDevice::x11DefaultVisual() const
{
    if (devType() == QInternal::Widget)
	return static_cast<const QWidget *>(this)->x11Info()->defaultVisual();
    else if (devType() == QInternal::Pixmap)
	return static_cast<const QPixmap *>(this)->x11Info()->defaultVisual();
    return QX11Info::appDefaultVisual();
}

void *QPaintDevice::x11AppVisual(int screen)
{ return QX11Info::appVisual(screen); }

Qt::HANDLE QPaintDevice::x11AppColormap(int screen)
{ return QX11Info::appColormap(screen); }

Display *QPaintDevice::x11AppDisplay()
{ return QX11Info::appDisplay(); }

int QPaintDevice::x11AppScreen()
{ return QX11Info::appScreen(); }

int QPaintDevice::x11AppDepth(int screen)
{ return QX11Info::appDepth(screen); }

int QPaintDevice::x11AppCells(int screen)
{ return QX11Info::appCells(screen); }

Qt::HANDLE QPaintDevice::x11AppRootWindow(int screen)
{ return QX11Info::appRootWindow(screen); }

bool QPaintDevice::x11AppDefaultColormap(int screen)
{ return QX11Info::appDefaultColormap(screen); }

bool QPaintDevice::x11AppDefaultVisual(int screen)
{ return QX11Info::appDefaultVisual(screen); }

/*!
    Sets the value returned by x11AppDpiX() to \a dpi for screen
    \a screen. The default is determined by the display configuration.
    Changing this value will alter the scaling of fonts and many other
    metrics and is not recommended. Using this function is not
    portable.

    \sa x11SetAppDpiY()
*/
void QPaintDevice::x11SetAppDpiX(int dpi, int screen)
{
    QX11Info::setAppDpiX(dpi, screen);
}

/*!
    Sets the value returned by x11AppDpiY() to \a dpi for screen
    \a screen. The default is determined by the display configuration.
    Changing this value will alter the scaling of fonts and many other
    metrics and is not recommended. Using this function is not
    portable.

    \sa x11SetAppDpiX()
*/
void QPaintDevice::x11SetAppDpiY(int dpi, int screen)
{
    QX11Info::setAppDpiY(dpi, screen);
}


/*!
    Returns the horizontal DPI of the X display (X11 only) for screen
    \a screen. Using this function is not portable. See
    QPaintDeviceMetrics for portable access to related information.
    Using this function is not portable.

    \sa x11AppDpiY(), x11SetAppDpiX(), QPaintDeviceMetrics::logicalDpiX()
*/
int QPaintDevice::x11AppDpiX(int screen)
{
    return QX11Info::appDpiX(screen);
}

/*!
    Returns the vertical DPI of the X11 display (X11 only) for screen
    \a screen.  Using this function is not portable. See
    QPaintDeviceMetrics for portable access to related information.
    Using this function is not portable.

    \sa x11AppDpiX(), x11SetAppDpiY(), QPaintDeviceMetrics::logicalDpiY()
*/
int QPaintDevice::x11AppDpiY( int screen )
{
    return QX11Info::appDpiY(screen);
}

#endif
