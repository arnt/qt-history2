/****************************************************************************
** $Id$
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the opengl module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses for Unix/X11 may
** use this file in accordance with the Qt Commercial License Agreement
** provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qgl.h"

#if defined(Q_WS_X11)

#include "qpixmap.h"
#include "qapplication.h"

#include "qintdict.h"
#define INT8  dummy_INT8
#define INT32 dummy_INT32
#include <GL/glx.h>
#undef  INT8
#undef  INT32
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xmu/StdCmap.h>



/*
  The choose_cmap function is internal and used by QGLWidget::setContext()
  and GLX (not Windows).  If the application can't find any sharable
  colormaps, it must at least create as few colormaps as possible.  The
  dictionary solution below ensures only one colormap is created per visual.
  Colormaps are also deleted when the application terminates.
*/

struct CMapEntry {
    CMapEntry();
   ~CMapEntry();
    Colormap		cmap;
    bool		alloc;
    XStandardColormap	scmap;
};

CMapEntry::CMapEntry()
{
    cmap = 0;
    alloc = FALSE;
    scmap.colormap = 0;
}

CMapEntry::~CMapEntry()
{
    if ( alloc )
	XFreeColormap( QPaintDevice::x11AppDisplay(), cmap );
}

static QIntDict<CMapEntry> *cmap_dict = 0;
static bool		    mesa_gl   = FALSE;

static void cleanup_cmaps()
{
    if ( !cmap_dict )
	return;
    cmap_dict->setAutoDelete( TRUE );
    delete cmap_dict;
    cmap_dict = 0;
}

static Colormap choose_cmap( Display *dpy, XVisualInfo *vi )
{
    if ( !cmap_dict ) {
	cmap_dict = new QIntDict<CMapEntry>;
	const char *v = glXQueryServerString( dpy, vi->screen, GLX_VERSION );
	if ( v )
	    mesa_gl = strstr(v,"Mesa") != 0;
	qAddPostRoutine( cleanup_cmaps );
    }

    CMapEntry *x = cmap_dict->find( (long) vi->visualid + ( vi->screen * 256 ) );
    if ( x )					// found colormap for visual
	return x->cmap;

    x = new CMapEntry();

    XStandardColormap *c;
    int n, i;

    // qDebug( "Choosing cmap for vID %0x", vi->visualid );

    if ( vi->visualid ==
	 XVisualIDFromVisual( (Visual*)QPaintDevice::x11AppVisual( vi->screen ) ) ) {
	// qDebug( "Using x11AppColormap" );
	return QPaintDevice::x11AppColormap( vi->screen );
    }

    if ( mesa_gl ) {				// we're using MesaGL
	Atom hp_cmaps = XInternAtom( dpy, "_HP_RGB_SMOOTH_MAP_LIST", TRUE );
	if ( hp_cmaps && vi->visual->c_class == TrueColor && vi->depth == 8 ) {
	    if ( XGetRGBColormaps(dpy,RootWindow(dpy,vi->screen),&c,&n,
				  hp_cmaps) ) {
		i = 0;
		while ( i < n && x->cmap == 0 ) {
		    if ( c[i].visualid == vi->visual->visualid ) {
			x->cmap = c[i].colormap;
			x->scmap = c[i];
			//qDebug( "Using HP_RGB scmap" );

		    }
		    i++;
		}
		XFree( (char *)c );
	    }
	}
    }
#if !defined(Q_OS_SOLARIS)
    if ( !x->cmap ) {
	if ( XmuLookupStandardColormap(dpy,vi->screen,vi->visualid,vi->depth,
				       XA_RGB_DEFAULT_MAP,FALSE,TRUE) ) {
	    if ( XGetRGBColormaps(dpy,RootWindow(dpy,vi->screen),&c,&n,
				  XA_RGB_DEFAULT_MAP) ) {
		i = 0;
		while ( i < n && x->cmap == 0 ) {
		    if ( c[i].visualid == vi->visualid ) {
			x->cmap = c[i].colormap;
			x->scmap = c[i];
			//qDebug( "Using RGB_DEFAULT scmap" );
		    }
		    i++;
		}
		XFree( (char *)c );
	    }
	}
    }
#endif
    if ( !x->cmap ) {				// no shared cmap found
	x->cmap = XCreateColormap( dpy, RootWindow(dpy,vi->screen), vi->visual,
				   AllocNone );
	x->alloc = TRUE;
	// qDebug( "Allocating cmap" );
    }

    // associate cmap with visualid
    cmap_dict->insert( (long) vi->visualid + ( vi->screen * 256 ), x );
    return x->cmap;
}

struct TransColor
{
    VisualID	vis;
    int		screen;
    long	color;
};

static QMemArray<TransColor> trans_colors;
static int trans_colors_init = FALSE;


static void find_trans_colors()
{
    struct OverlayProp {
	long  visual;
	long  type;
	long  value;
	long  layer;
    };

    trans_colors_init = TRUE;

    Display* appDisplay = QPaintDevice::x11AppDisplay();

    int scr;
    int lastsize = 0;
    for ( scr = 0; scr < ScreenCount( appDisplay ); scr++ ) {
	QWidget* rootWin = QApplication::desktop()->screen( scr );
	if ( !rootWin )
	    return;					// Should not happen
	Atom overlayVisualsAtom = XInternAtom( appDisplay,
					       "SERVER_OVERLAY_VISUALS", True );
	if ( overlayVisualsAtom == None )
	    return;					// Server has no overlays

	Atom actualType;
	int actualFormat;
	ulong nItems;
	ulong bytesAfter;
	OverlayProp* overlayProps = 0;
	int res = XGetWindowProperty( appDisplay, rootWin->winId(),
				      overlayVisualsAtom, 0, 10000, False,
				      overlayVisualsAtom, &actualType,
				      &actualFormat, &nItems, &bytesAfter,
				      (uchar**)&overlayProps );

	if ( res != Success || actualType != overlayVisualsAtom
	     || actualFormat != 32 || nItems < 4 || !overlayProps )
	    return;					// Error reading property

	int numProps = nItems / 4;
	trans_colors.resize( lastsize + numProps );
	int j = lastsize;
	for ( int i = 0; i < numProps; i++ ) {
	    if ( overlayProps[i].type == 1 ) {
		trans_colors[j].vis = (VisualID)overlayProps[i].visual;
		trans_colors[j].screen = scr;
		trans_colors[j].color = (int)overlayProps[i].value;
		j++;
	    }
	}
	XFree( overlayProps );
	lastsize = j;
	trans_colors.truncate( lastsize );
    }
}


/*****************************************************************************
  QGLFormat UNIX/GLX-specific code
 *****************************************************************************/

bool QGLFormat::hasOpenGL()
{
    return glXQueryExtension(qt_xdisplay(),0,0) != 0;
}


bool QGLFormat::hasOpenGLOverlays()
{
    if ( !trans_colors_init )
	find_trans_colors();
    return trans_colors.size() > 0;
}



/*****************************************************************************
  QGLContext UNIX/GLX-specific code
 *****************************************************************************/

bool QGLContext::chooseContext( const QGLContext* shareContext )
{
    Display* disp = d->paintDevice->x11Display();
    vi = chooseVisual();
    if ( !vi )
	return FALSE;

    if ( deviceIsPixmap() &&
	 (((XVisualInfo*)vi)->depth != d->paintDevice->x11Depth() ||
	  ((XVisualInfo*)vi)->screen != d->paintDevice->x11Screen() ||
	  ((XVisualInfo*)vi)->visualid !=
	        XVisualIDFromVisual( (Visual*)d->paintDevice->x11Visual() )) )
    {
	XFree( vi );
	XVisualInfo appVisInfo;
	memset( &appVisInfo, 0, sizeof(XVisualInfo) );
	appVisInfo.visualid = XVisualIDFromVisual( (Visual*)d->paintDevice->x11Visual() );
	appVisInfo.screen = d->paintDevice->x11Screen();
	int nvis;
	vi = XGetVisualInfo( disp, VisualIDMask | VisualScreenMask, &appVisInfo, &nvis );
	if ( !vi )
	    return FALSE;

	int useGL;
	glXGetConfig( disp, (XVisualInfo*)vi, GLX_USE_GL, &useGL );
	if ( !useGL )
	    return FALSE;	//# Chickening out already...
    }
    int res;
    glXGetConfig( disp, (XVisualInfo*)vi, GLX_LEVEL, &res );
    glFormat.setPlane( res );
    glXGetConfig( disp, (XVisualInfo*)vi, GLX_DOUBLEBUFFER, &res );
    glFormat.setDoubleBuffer( res );
    glXGetConfig( disp, (XVisualInfo*)vi, GLX_DEPTH_SIZE, &res );
    glFormat.setDepth( res );
    glXGetConfig( disp, (XVisualInfo*)vi, GLX_RGBA, &res );
    glFormat.setRgba( res );
    glXGetConfig( disp, (XVisualInfo*)vi, GLX_ALPHA_SIZE, &res );
    glFormat.setAlpha( res );
    glXGetConfig( disp, (XVisualInfo*)vi, GLX_ACCUM_RED_SIZE, &res );
    glFormat.setAccum( res );
    glXGetConfig( disp, (XVisualInfo*)vi, GLX_STENCIL_SIZE, &res );
    glFormat.setStencil( res );
    glXGetConfig( disp, (XVisualInfo*)vi, GLX_STEREO, &res );
    glFormat.setStereo( res );

    Bool direct = format().directRendering() ? True : False;

    if ( shareContext &&
	 ( !shareContext->isValid() || !shareContext->cx ) ) {
#if defined(QT_CHECK_NULL)
	    qWarning("QGLContext::chooseContext(): Cannot share with invalid context");
#endif
	    shareContext = 0;
    }

    // 1. Sharing between rgba and color-index will give wrong colors.
    // 2. Contexts cannot be shared btw. direct/non-direct renderers.
    // 3. Pixmaps cannot share contexts that are set up for direct rendering.
    if ( shareContext && (format().rgba() != shareContext->format().rgba() ||
			  (deviceIsPixmap() &&
			   glXIsDirect( disp, (GLXContext)shareContext->cx ))))
	shareContext = 0;

    cx = 0;
    if ( shareContext ) {
	cx = glXCreateContext( disp, (XVisualInfo *)vi,
			       (GLXContext)shareContext->cx, direct );
	if ( cx )
	    d->sharing = TRUE;
    }
    if ( !cx )
	cx = glXCreateContext( disp, (XVisualInfo *)vi, None, direct );
    if ( !cx )
	return FALSE;
    glFormat.setDirectRendering( glXIsDirect( disp, (GLXContext)cx ) );
    if ( deviceIsPixmap() ) {
#if defined(GLX_MESA_pixmap_colormap) && defined(QGL_USE_MESA_EXT)
	gpm = glXCreateGLXPixmapMESA( disp, (XVisualInfo *)vi,
				      d->paintDevice->handle(),
				      choose_cmap( disp, (XVisualInfo *)vi ) );
#else
	gpm = (Q_UINT32)glXCreateGLXPixmap( disp, (XVisualInfo *)vi,
					    d->paintDevice->handle() );
#endif
	if ( !gpm )
	    return FALSE;
    }
    return TRUE;
}


/*
  <strong>X11 only</strong>: This virtual function tries to find a
  visual that matches the format, reducing the demands if the original
  request cannot be met.

  The algorithm for reducing the demands of the format is quite
  simple-minded, so override this method in your subclass if your
  application has spcific requirements on visual selection.

  \sa chooseContext()
*/

void *QGLContext::chooseVisual()
{
    static int bufDepths[] = { 8, 4, 2, 1 };	// Try 16, 12 also?
    //todo: if pixmap, also make sure that vi->depth == pixmap->depth
    void* vis = 0;
    int i = 0;
    bool fail = FALSE;
    QGLFormat fmt = format();
    bool tryDouble = !fmt.doubleBuffer();  // Some GL impl's only have double
    bool triedDouble = FALSE;
    while( !fail && !( vis = tryVisual( fmt, bufDepths[i] ) ) ) {
	if ( !fmt.rgba() && bufDepths[i] > 1 ) {
	    i++;
	    continue;
	}
	if ( tryDouble ) {
	    fmt.setDoubleBuffer( TRUE );
	    tryDouble = FALSE;
	    triedDouble = TRUE;
	    continue;
	}
	else if ( triedDouble ) {
	    fmt.setDoubleBuffer( FALSE );
	    triedDouble = FALSE;
	}
	if ( fmt.stereo() ) {
	    fmt.setStereo( FALSE );
	    continue;
	}
	if ( fmt.accum() ) {
	    fmt.setAccum( FALSE );
	    continue;
	}
	if ( fmt.stencil() ) {
	    fmt.setStencil( FALSE );
	    continue;
	}
	if ( fmt.alpha() ) {
	    fmt.setAlpha( FALSE );
	    continue;
	}
	if ( fmt.depth() ) {
	    fmt.setDepth( FALSE );
	    continue;
	}
	if ( fmt.doubleBuffer() ) {
	    fmt.setDoubleBuffer( FALSE );
	    continue;
	}
	fail = TRUE;
    }
    glFormat = fmt;
    return vis;
}


/*
  <strong>X11 only</strong>: This virtual function chooses a visual
  that matches the OpenGL \link format() format\endlink. Reimplement this
  function in a subclass if you need a custom visual.

  \sa chooseContext()
*/

void *QGLContext::tryVisual( const QGLFormat& f, int bufDepth )
{
    int spec[40];
    int i = 0;
    spec[i++] = GLX_LEVEL;
    spec[i++] = f.plane();

#if defined(GLX_VERSION_1_1) && defined(GLX_EXT_visual_info)
    static bool useTranspExt = FALSE;
    static bool useTranspExtChecked = FALSE;
    if ( f.plane() && !useTranspExtChecked && d->paintDevice ) {
	QCString estr( glXQueryExtensionsString( d->paintDevice->x11Display(),
						 d->paintDevice->x11Screen() ) );
	useTranspExt = estr.contains( "GLX_EXT_visual_info" );
	//# (A bit simplistic; that could theoretically be a substring)
	if ( useTranspExt ) {
	    QCString cstr( glXGetClientString( d->paintDevice->x11Display(),
					       GLX_VENDOR ) );
	    useTranspExt = !cstr.contains( "Xi Graphics" ); // bug workaround
	}
	useTranspExtChecked = TRUE;
    }
    if ( f.plane() && useTranspExt ) {
	// Required to avoid non-transparent overlay visual(!) on some systems
	spec[i++] = GLX_TRANSPARENT_TYPE_EXT;
	spec[i++] = GLX_TRANSPARENT_INDEX_EXT; //# Depending on format, really
    }
#endif

    if ( f.doubleBuffer() )
	spec[i++] = GLX_DOUBLEBUFFER;
    if ( f.depth() ) {
	spec[i++] = GLX_DEPTH_SIZE;
	spec[i++] = 1;
    }
    if ( f.stereo() ) {
	spec[i++] = GLX_STEREO;
    }
    if ( f.stencil() ) {
	spec[i++] = GLX_STENCIL_SIZE;
	spec[i++] = 1;
    }
    if ( f.rgba() ) {
	spec[i++] = GLX_RGBA;
	spec[i++] = GLX_RED_SIZE;
	spec[i++] = 1;
	spec[i++] = GLX_GREEN_SIZE;
	spec[i++] = 1;
	spec[i++] = GLX_BLUE_SIZE;
	spec[i++] = 1;
	if ( f.alpha() ) {
	    spec[i++] = GLX_ALPHA_SIZE;
	    spec[i++] = 1;
	}
	if ( f.accum() ) {
	    spec[i++] = GLX_ACCUM_RED_SIZE;
	    spec[i++] = 1;
	    spec[i++] = GLX_ACCUM_GREEN_SIZE;
	    spec[i++] = 1;
	    spec[i++] = GLX_ACCUM_BLUE_SIZE;
	    spec[i++] = 1;
	    if ( f.alpha() ) {
		spec[i++] = GLX_ACCUM_ALPHA_SIZE;
		spec[i++] = 1;
	    }
	}
    }
    else {
	spec[i++] = GLX_BUFFER_SIZE;
	spec[i++] = bufDepth;
    }

    spec[i] = None;
    return glXChooseVisual( d->paintDevice->x11Display(),
			    d->paintDevice->x11Screen(), spec );
}


void QGLContext::reset()
{
    if ( !d->valid )
	return;
    doneCurrent();
    if ( gpm )
	glXDestroyGLXPixmap( d->paintDevice->x11Display(), (GLXPixmap)gpm );
    gpm = 0;
    glXDestroyContext( d->paintDevice->x11Display(), (GLXContext)cx );
    if ( vi )
	XFree( vi );
    vi = 0;
    cx = 0;
    d->crWin = FALSE;
    d->sharing = FALSE;
    d->valid = FALSE;
    d->transpColor = QColor();
    d->initDone = FALSE;
}


void QGLContext::makeCurrent()
{
    if ( !d->valid ) {
#if defined(QT_CHECK_STATE)
	qWarning("QGLContext::makeCurrent(): Cannot make invalid context current.");
#endif
	return;
    }
    bool ok = TRUE;
    if ( deviceIsPixmap() )
	ok = glXMakeCurrent( d->paintDevice->x11Display(),
			     (GLXPixmap)gpm,
			     (GLXContext)cx );

    else
	ok = glXMakeCurrent( d->paintDevice->x11Display(),
			     ((QWidget *)d->paintDevice)->winId(),
			     (GLXContext)cx );
#if defined(QT_CHECK_NULL)
    //    qDebug("makeCurrent: %i, vi=%i, vi->vi=%i, vi->id=%i", (int)this, (int)vi, (int)((XVisualInfo*)vi)->visual, (int)((XVisualInfo*)vi)->visualid );
    if ( !ok )
	qWarning("QGLContext::makeCurrent(): Failed.");
#endif
    if ( ok )
	currentCtx = this;
}

void QGLContext::doneCurrent()
{
    // This is a bug-workaround for the Utah-GLX driver (v0.10, November 2000)
    // Calling glXMakeCurrent() without any drawable or context crashes
    // the X server (tested with a Matrox G400 XFree86 v3.3.6, might work
    // with other cards). This does not happen with Accelerated X v2.0 Alpha.

#if defined(Q_OS_LINUX) || defined(Q_OS_FreeBSD)
    static bool utahGLX =
	QString( glXGetClientString( d->paintDevice->x11Display(),
				     GLX_EXTENSIONS ) ).contains( "GLX_utah" );
    if ( !utahGLX )
	glXMakeCurrent( d->paintDevice->x11Display(), 0, 0 );
#else
    glXMakeCurrent( d->paintDevice->x11Display(), 0, 0 );
#endif
    currentCtx = 0;
}


void QGLContext::swapBuffers() const
{
    if ( !d->valid )
	return;
    if ( !deviceIsPixmap() )
	glXSwapBuffers( d->paintDevice->x11Display(),
			((QWidget *)d->paintDevice)->winId() );
}

QColor QGLContext::overlayTransparentColor() const
{
    //### make more efficient using the transpColor member
    if ( isValid() ) {
	if ( !trans_colors_init )
	    find_trans_colors();

	VisualID myVisualId = ((XVisualInfo*)vi)->visualid;
	int myScreen = ((XVisualInfo*)vi)->screen;
	for ( int i = 0; i < (int)trans_colors.size(); i++ ) {
	    if ( trans_colors[i].vis == myVisualId &&
		 trans_colors[i].screen == myScreen )
		return QColor( qRgb( 1, 2, 3 ), trans_colors[i].color );
	}
    }
    return QColor();		// Invalid color
}


uint QGLContext::colorIndex( const QColor& c ) const
{
    int screen = ((XVisualInfo *)vi)->screen;
    if ( isValid() ) {
	if ( format().plane()
	     && c.pixel( screen ) == overlayTransparentColor().pixel( screen ) )
	    return c.pixel( screen );		// Special; don't look-up
	if ( ((XVisualInfo*)vi)->visualid ==
	     XVisualIDFromVisual( (Visual*)QPaintDevice::x11AppVisual( screen ) ) )
	    return c.pixel( screen );		// We're using QColor's cmap

	XVisualInfo *info = (XVisualInfo *) vi;
	CMapEntry *x = cmap_dict->find( (long) info->visualid + ( info->screen * 256 ) );
	if ( x && !x->alloc) {		// It's a standard colormap
	    int rf = (int)(((float)c.red() * (x->scmap.red_max+1))/256.0);
	    int gf = (int)(((float)c.green() * (x->scmap.green_max+1))/256.0);
	    int bf = (int)(((float)c.blue() * (x->scmap.blue_max+1))/256.0);
	    uint p = x->scmap.base_pixel
		     + ( rf * x->scmap.red_mult )
		     + ( gf * x->scmap.green_mult )
		     + ( bf * x->scmap.blue_mult );
	    return p;
	}
	else {
	    return c.pixel( screen ); // ### wrong; should really ask QColor to alloc
	}
    }
    return 0;
}

/*****************************************************************************
  QGLOverlayWidget (Internal overlay class for X11)
 *****************************************************************************/

class QGLOverlayWidget : public QGLWidget
{
    Q_OBJECT
public:
    QGLOverlayWidget( const QGLFormat& format, QGLWidget* parent,
		      const char* name=0, const QGLWidget* shareWidget=0 );

protected:
    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

private:
    QGLWidget*		realWidget;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QGLOverlayWidget( const QGLOverlayWidget& );
    QGLOverlayWidget&	operator=( const QGLOverlayWidget& );
#endif
};


QGLOverlayWidget::QGLOverlayWidget( const QGLFormat& format, QGLWidget* parent,
				    const char* name,
				    const QGLWidget* shareWidget )
    : QGLWidget( format, parent, name, shareWidget ? shareWidget->olw : 0 )
{
    realWidget = parent;
}



void QGLOverlayWidget::initializeGL()
{
    QColor transparentColor = context()->overlayTransparentColor();
    if ( transparentColor.isValid() )
	qglClearColor( transparentColor );
    else
	qWarning( "QGLOverlayWidget::initializeGL(): Could not get transparent color" );
    realWidget->initializeOverlayGL();
}


void QGLOverlayWidget::resizeGL( int w, int h )
{
    glViewport( 0, 0, w, h );
    realWidget->resizeOverlayGL( w, h );
}


void QGLOverlayWidget::paintGL()
{
    realWidget->paintOverlayGL();
}

#undef Bool
#include "qgl_x11.moc"

/*****************************************************************************
  QGLWidget UNIX/GLX-specific code
 *****************************************************************************/

void QGLWidget::init( const QGLFormat& format, const QGLWidget* shareWidget )
{
    glcx = 0;
    olw = 0;
    autoSwap = TRUE;

    if ( shareWidget )
	setContext( new QGLContext( format, this ), shareWidget->context() );
    else
	setContext( new QGLContext( format, this ) );
    setBackgroundMode( NoBackground );

    if ( isValid() && format.hasOverlay() ) {
	QCString olwName( name() );
	olwName += "-QGL_internal_overlay_widget";
	olw = new QGLOverlayWidget( QGLFormat::defaultOverlayFormat(),
				    this, olwName, shareWidget );
	if ( olw->isValid() ) {
	    olw->setAutoBufferSwap( FALSE );
	    olw->setFocusProxy( this );
	}
	else {
	    delete olw;
	    olw = 0;
	    glcx->glFormat.setOverlay( FALSE );
	}
    }
}

/*! \reimp */
void QGLWidget::reparent( QWidget* parent, WFlags f, const QPoint& p,
			  bool showIt )
{
    // ### Another work-around for the Utah-GLX driver -
    // ### if the old context is not destroyed before the window is
    // ### reparented, it crashes badly (driver v0.10 November 2000)
#if defined(Q_OS_LINUX) || defined(Q_OS_FreeBSD)
    static bool utahGLX = QString( glXGetClientString( x11Display(),
				     GLX_EXTENSIONS ) ).contains( "GLX_utah" );
    if ( utahGLX ) {
	QGLFormat reqf = QGLFormat::defaultFormat();
	if ( glcx ) {
	    reqf = glcx->requestedFormat();
	    delete glcx;
	    glcx = 0;
	}
	QWidget::reparent( parent, f, p, FALSE );
	setContext( new QGLContext( reqf, this ) );
	QWidget::reparent( parent, f, p, FALSE );
    } else {
	QWidget::reparent( parent, f, p, FALSE );
    }
#else
    QWidget::reparent( parent, f, p, FALSE );
#endif
    if ( showIt )
	show();
}


void QGLWidget::setMouseTracking( bool enable )
{
    if ( olw )
	olw->setMouseTracking( enable );
    QWidget::setMouseTracking( enable );
}


void QGLWidget::resizeEvent( QResizeEvent * )
{
    if ( !isValid() )
	return;
    makeCurrent();
    if ( !glcx->initialized() )
	glInit();
    glXWaitX();
    resizeGL( width(), height() );
    if ( olw )
	olw->setGeometry( rect() );
}

const QGLContext* QGLWidget::overlayContext() const
{
    if ( olw )
	return olw->context();
    else
	return 0;
}


void QGLWidget::makeOverlayCurrent()
{
    if ( olw )
	olw->makeCurrent();
}


void QGLWidget::updateOverlayGL()
{
    if ( olw )
	olw->updateGL();
}

void QGLWidget::setContext( QGLContext *context,
			    const QGLContext* shareContext,
			    bool deleteOldContext )
{
    if ( context == 0 ) {
#if defined(QT_CHECK_NULL)
	qWarning( "QGLWidget::setContext: Cannot set null context" );
#endif
	return;
    }
    if ( !context->deviceIsPixmap() && context->device() != this ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QGLWidget::setContext: Context must refer to this widget" );
#endif
	return;
    }

    if ( glcx )
	glcx->doneCurrent();
    QGLContext* oldcx = glcx;
    glcx = context;

    bool createFailed = FALSE;
    if ( !glcx->isValid() ) {
	if ( !glcx->create( shareContext ? shareContext : oldcx ) )
	    createFailed = TRUE;
    }
    if ( createFailed ) {
	if ( deleteOldContext )
	    delete oldcx;
	return;
    }

    if ( glcx->windowCreated() || glcx->deviceIsPixmap() ) {
	if ( deleteOldContext )
	    delete oldcx;
	return;
    }

    bool visible = isVisible();
    if ( visible )
	hide();

    XVisualInfo *vi = (XVisualInfo*)glcx->vi;
    XSetWindowAttributes a;

    a.colormap = choose_cmap( x11Display(), vi );	// find best colormap
    a.background_pixel = backgroundColor().pixel( vi->screen );
    a.border_pixel = black.pixel( vi->screen );
    Window p = RootWindow( x11Display(), vi->screen );
    if ( parentWidget() )
	p = parentWidget()->winId();

    Window w = XCreateWindow( x11Display(), p,  x(), y(), width(), height(),
			      0, vi->depth, InputOutput,  vi->visual,
			      CWBackPixel|CWBorderPixel|CWColormap, &a );

    Window *cmw;
    Window *cmwret;
    int count;
    if ( XGetWMColormapWindows( x11Display(), topLevelWidget()->winId(),
				&cmwret, &count ) ) {
	cmw = new Window[count+1];
	memcpy( (char *)cmw, (char *)cmwret, sizeof(Window)*count );
	XFree( (char *)cmwret );
	int i;
	for ( i=0; i<count; i++ ) {
	    if ( cmw[i] == winId() ) {		// replace old window
		cmw[i] = w;
		break;
	    }
	}
	if ( i >= count )			// append new window
	    cmw[count++] = w;
    } else {
	count = 1;
	cmw = new Window[count];
	cmw[0] = w;
    }

#if defined(GLX_MESA_release_buffers) && defined(QGL_USE_MESA_EXT)
    if ( oldcx && oldcx->windowCreated() )
	glXReleaseBuffersMESA( x11Display(), winId() );
#endif
    if ( deleteOldContext )
	delete oldcx;
    oldcx = 0;

    create( w );

    XSetWMColormapWindows( x11Display(), topLevelWidget()->winId(), cmw,
			   count );
    delete [] cmw;

    if ( visible )
	show();
    XFlush( x11Display() );
    glcx->setWindowCreated( TRUE );
}


bool QGLWidget::renderCxPm( QPixmap* pm )
{
    if ( ((XVisualInfo*)glcx->vi)->depth != pm->depth() )
	return FALSE;

    GLXPixmap glPm;
#if defined(GLX_MESA_pixmap_colormap) && defined(QGL_USE_MESA_EXT)
    glPm = glXCreateGLXPixmapMESA( x11Display(),
				   (XVisualInfo*)glcx->vi,
				   (Pixmap)pm->handle(),
				   choose_cmap( pm->x11Display(),
						(XVisualInfo*)glcx->vi ) );
#else
    glPm = (Q_UINT32)glXCreateGLXPixmap( x11Display(),
					 (XVisualInfo*)glcx->vi,
					 (Pixmap)pm->handle() );
#endif

    if ( !glXMakeCurrent( x11Display(), glPm, (GLXContext)glcx->cx ) ) {
	glXDestroyGLXPixmap( x11Display(), glPm );
	return FALSE;
    }

    glDrawBuffer( GL_FRONT_LEFT );
    if ( !glcx->initialized() )
	glInit();
    resizeGL( pm->width(), pm->height() );
    paintGL();
    glFlush();
    makeCurrent();
    glXDestroyGLXPixmap( x11Display(), glPm );
    resizeGL( width(), height() );
    return TRUE;
}

const QGLColormap & QGLWidget::colormap() const
{
    return cmap;
}

/*\internal
  Store color values in the given colormap.
*/
static void qStoreColors( QWidget * tlw, Colormap cmap,
			  const QGLColormap & cols )
{
    XColor c;
    QRgb color;

    for ( int i = 0; i < cols.size(); i++ ) {
	color = cols.entryRgb( i );
	c.pixel = i;
	c.red   = (ushort)( (qRed( color ) / 255.0) * 65535.0 + 0.5 );
	c.green = (ushort)( (qGreen( color ) / 255.0) * 65535.0 + 0.5 );
	c.blue  = (ushort)( (qBlue( color ) / 255.0) * 65535.0 + 0.5 );
	c.flags = DoRed | DoGreen | DoBlue;
	XStoreColor( tlw->x11Display(), cmap, &c );
    }
}

/*\internal
  Check whether the given visual supports dynamic colormaps or not.
*/
static bool qCanAllocColors( QWidget * w )
{
    bool validVisual = FALSE;
    int  numVisuals;
    long mask;
    XVisualInfo templ;
    XVisualInfo * visuals;
    VisualID id = XVisualIDFromVisual( (Visual *)
				       w->topLevelWidget()->x11Visual() );

    mask = VisualScreenMask;
    templ.screen = w->x11Screen();
    visuals = XGetVisualInfo( w->x11Display(), mask, &templ, &numVisuals );

    for ( int i = 0; i < numVisuals; i++ ) {
	if ( visuals[i].visualid == id ) {
	    switch ( visuals[i].c_class ) {
		case TrueColor:
		case StaticColor:
		case StaticGray:
		case GrayScale:
		    validVisual = FALSE;
		    break;
		case DirectColor:
		case PseudoColor:
		    validVisual = TRUE;
		    break;
	    }
	    break;
	}
    }
    XFree( visuals );

    if ( !validVisual )
	return FALSE;
    return TRUE;
}

void QGLWidget::setColormap( const QGLColormap & c )
{
    QWidget * tlw = topLevelWidget(); // must return a valid widget

    cmap = c;
    if ( !cmap.d )
	return;

    if ( !cmap.d->cmapHandle && !qCanAllocColors( this ) ) {
	qWarning( "QGLWidget::setColormap: Cannot create a read/write "
		  "colormap for this visual" );
	return;
    }

    // If the child GL widget is not of the same visual class as the
    // toplevel widget we will get in trouble..
    Window wid = tlw->winId();
    Visual * vis = (Visual *) tlw->x11Visual();;
    VisualID cvId = XVisualIDFromVisual( (Visual *) x11Visual() );
    VisualID tvId = XVisualIDFromVisual( (Visual *) tlw->x11Visual() );
    if ( cvId != tvId ) {
	wid = winId();
	vis = (Visual *) x11Visual();
    }

    if ( !cmap.d->cmapHandle ) // allocate a cmap if necessary
	cmap.d->cmapHandle = XCreateColormap( x11Display(), wid, vis,
					      AllocAll );

    qStoreColors( this, (Colormap) cmap.d->cmapHandle, c );
    XSetWindowColormap( x11Display(), wid, (Colormap) cmap.d->cmapHandle );

    // tell the wm that this window has a special colormap
    Window * cmw;
    Window * cmwret;
    int count;
    if ( XGetWMColormapWindows( x11Display(), tlw->winId(), &cmwret,
				&count ) )
    {
	cmw = new Window[count+1];
	memcpy( (char *) cmw, (char *) cmwret, sizeof(Window) * count );
	XFree( (char *) cmwret );
	int i;
	for ( i = 0; i < count; i++ ) {
	    if ( cmw[i] == winId() ) {
		break;
	    }
	}
	if ( i >= count )   // append new window only if not in the list
	    cmw[count++] = winId();
    } else {
	count = 1;
	cmw = new Window[count];
	cmw[0] = winId();
    }
    XSetWMColormapWindows( x11Display(), tlw->winId(), cmw, count );
    delete [] cmw;
}

/*! \internal
  Free up any allocated colormaps. This fn is only called for
  top-level widgets.
*/
void QGLWidget::cleanupColormaps()
{
    if ( !cmap.d )
	return;

    if ( cmap.d->cmapHandle ) {
	XFreeColormap( topLevelWidget()->x11Display(),
		       (Colormap) cmap.d->cmapHandle );
	cmap.d->cmapHandle = 0;
    }
}

void QGLWidget::macInternalFixBufferRect()
{
}

#endif
