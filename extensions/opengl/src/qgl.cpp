/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/src/qgl.cpp#3 $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qgl.h"

#if defined(Q_GLX)
#define INT8  dummy_INT8
#define INT32 dummy_INT32
#include <GL/glx.h>
#undef  INT8
#undef  INT32
#endif

RCSTAG("$Id: //depot/qt/main/extensions/opengl/src/qgl.cpp#3 $");


#if defined(_CC_MSVC_)
#pragma warning(disable:4355) // 'this' : used in base member initializer list
#endif


/*****************************************************************************
  QGLFormat implementation
 *****************************************************************************/

 
 /*!
  \class QGLFormat qgl.h
  \brief The QGLFormat class specifies the format of an OpenGL rendering context.

  A rendering context has several characteristics:
  <ul>
  <li> \link setDoubleBuffer() Double or single buffering.\endlink
  <li> \link setDepth() Depth buffer.\endlink
  <li> \link setRgba() RGBA or color index mode.\endlink
  <li> \link setAlpha() Alpha blending.\endlink
  <li> \link setAccum() Accumulation buffer.\endlink
  <li> \link setStencil() Stencil buffer.\endlink
  <li> \link setStereo() Stereo buffer.\endlink
  </ul>

  You create and tell a QGLFormat object what rendering options
  you want from an OpenGL rendering context.
  OpenGL drivers or accelerated hardware may or may not support
  advanced features like alpha blending or stereographic viewing.
  If you request alpha blending and the driver/hardware does not
  provide it, you will get an invalid context. You can then try
  again with a less demanding format.

  There are many different ways of defining the characteristics
  of a rendering context. One is to create a QGLContext and make
  it default for the entire application.
  \code
    QGLFormat f;
    f.setAlpha( TRUE );
    f.setStereo( TRUE );
    QGLFormat::setDefaultFormat( f );
  \endcode

  You can also set a format for your subclassed QGLWidget:
  \code
    QGLWidget *w;
      ...
    QGLFormat f;
    f.setAlpha( TRUE );
    f.setStereo( TRUE );
    w->setFormat( f );
    if ( !w->create() ) {
        f.setStereo( FALSE );	// ok, googles off
        if ( !w->create() ) {
            fatal( "Cool hardware wanted" );
        }
    }
  \endcode

  <em>Implementation note:</em>
  QGLFormat objects use implicit sharing. This is an internal
  optimization that minimizes copying and memory overhead.
  When you pass a QGLFormat as an argument to a function,or
  return a QGLFormat from a function, a reference count is
  incremented or decremented. Objects stay shared until one
  of them is changed.

  \sa QGLContext, QGLWidget
*/


/*!
  Constructs a QGLFormat object with the default settings:
  <ul>
  <li> \link setDoubleBuffer() Double buffer:\endlink Enabled if
  \a doubleBuffer is TRUE, otherwise single buffer.
  <li> \link setDepth() Depth buffer:\endlink Enabled.
  <li> \link setRgba() RGBA:\endlink Enabled (i.e. color index disabled).
  <li> \link setAlpha() Alpha channel:\endlink Disabled.
  <li> \link setAccum() Accumulator buffer:\endlink Disabled.
  <li> \link setStencil() Stencil buffer:\endlink Disabled.
  <li> \link setStereo() Stereo:\endlink Disabled.
  </ul>
*/

QGLFormat::QGLFormat( bool doubleBuffer )
{
    data = new Internal;
    CHECK_PTR( data );
    data->doubleBuffer = doubleBuffer;
    data->depth	       = TRUE;
    data->rgba	       = TRUE;
    data->alpha	       = FALSE;
    data->accum	       = FALSE;
    data->stencil      = FALSE;
    data->stereo       = FALSE;
}

/*!
  Constructs a QGLFormat object which is a copy of \a f.
*/

QGLFormat::QGLFormat( const QGLFormat &f )
{
    data = f.data;
    data->ref();
}

/*!
  Destroys the object.
*/

QGLFormat::~QGLFormat()
{
    if ( data->deref() )
	delete data;
}


void QGLFormat::detach()
{
    if ( data->count != 1 ) {
	QGLFormat f;
	*f.data = *data;
	f.data->count = 1;
	*this = f;
    }
}


/*!
  Assigns \a f to this QGLFormat object and returns a reference
  to this object.
*/

QGLFormat &QGLFormat::operator=( const QGLFormat &f )
{
    f.data->ref();					// beware of f = f
    if ( data->deref() )
	delete data;
    data = f.data;
    return *this;
}


/*!
  \fn bool QGLFormat::doubleBuffer() const
  Returns TRUE if double buffering is enabled, otherwise FALSE.
  Double buffering is enabled by default.
  \sa setDoubleBuffer()
*/

/*!
  Sets double buffering if \a enable is TRUE og single buffering if
  \a enable is FALSE.

  Double buffering is enabled by default.

  Double buffering is a technique where graphics is rendered to an off-screen
  buffer and not directly to the screen. When the drawing has been
  completed, the program calls a swapBuffers function to exchange the screen
  contents with the buffer. The result is flicker-free drawing and often
  better performance.

  \sa doubleBuffer(), QGLContext::swapBuffers(), QGLWidget::swapBuffers()
*/

void QGLFormat::setDoubleBuffer( bool enable )
{
    if ( data->doubleBuffer != enable ) {
	detach();
	data->doubleBuffer = enable;
    }
}


/*!
  \fn bool QGLFormat::depth() const
  Returns TRUE if the depth buffer is enabled, otherwise FALSE.
  The depth buffer is enabled by default.
  \sa setDepth()
*/

/*!
  Enables the depth buffer if \a enable is TRUE, or disables
  it if \a enable is FALSE.

  The depth buffer is enabled by default.

  The purpose of a depth buffer (or z-buffering) is to remove hidden
  surfaces. Pixels are assigned z values based on the distance to the
  viewer. A pixel with a high z value is closer to the viewer than a
  pixel with a low z value. This information is used to decide whether
  to draw a pixel or not.

  \sa depth()
*/

void QGLFormat::setDepth( bool enable )
{
    if ( data->depth != enable ) {
	detach();
	data->depth = enable;
    }
}


/*!
  \fn bool QGLFormat::rgba() const
  Returns TRUE if RGBA color mode is set, or FALSE if color index
  mode is set. The default color mode is RGBA.
  \sa setRgba()
*/

/*!
  Sets RGBA mode if \a enable is TRUE, or color index mode if \a enable
  is FALSE.

  The default color mode is RGBA.

  RGBA is the preferred mode for most OpenGL applications.
  In RGBA color mode you specify colors as a red + green + blue + alpha
  quadruplet

  In color index mode you specify an index into a color lookup table.

  \sa rgba()
*/

void QGLFormat::setRgba( bool enable )
{
    if ( data->rgba != enable ) {
	detach();
	data->rgba = enable;
    }
}


/*!
  \fn bool QGLFormat::alpha() const
  Returns TRUE if the alpha buffer is enabled, otherwise FALSE.
  The alpha buffer is disabled by default.
  \sa setAlpha()
*/

/*!
  Enables the alpha buffer if \a enable is TRUE, or disables
  it if \a enable is FALSE.

  The alpha buffer is disabled by default.

  Alpha blending lets you draw transparent or translucent objects.
  The A in RGBA specifies the transparency of a pixel.

  \sa alpha()
*/

void QGLFormat::setAlpha( bool enable )
{
    if ( data->alpha != enable ) {
	detach();
	data->alpha = enable;
    }
}


/*!
  \fn bool QGLFormat::accum() const
  Returns TRUE if the accumulation buffer is enabled, otherwise FALSE.
  The accumulation buffer is disabled by default.
  \sa setAccum()
*/

/*!
  Enables the accumulation buffer if \a enable is TRUE, or disables
  it if \a enable is FALSE.

  The accumulation buffer is disabled by default.

  The accumulation buffer is used for create blur effects and
  multiple exposures.

  \sa accum()
*/

void QGLFormat::setAccum( bool enable )
{
    if ( data->accum != enable ) {
	detach();
	data->accum = enable;
    }
}


/*!
  \fn bool QGLFormat::stencil() const
  Returns TRUE if the stencil buffer is enabled, otherwise FALSE.
  The stencil buffer is disabled by default.
  \sa setStencil()
*/

/*!
  Enables the stencil buffer if \a enable is TRUE, or disables
  it if \a enable is FALSE.

  The stencil buffer is disabled by default.

  The stencil buffer masks away drawing from certain parts of
  the screen.

  \sa stencil()
*/

void QGLFormat::setStencil( bool enable )
{
    if ( data->stencil != enable ) {
	detach();
	data->stencil = enable;
    }
}


/*!
  \fn bool QGLFormat::stereo() const
  Returns TRUE if stereo buffering is enabled, otherwise FALSE.
  Stereo buffering is disabled by default.
  \sa setStereo()
*/

/*!
  Enables stereo buffering if \a enable is TRUE, or disables
  it if \a enable is FALSE.

  Stereo buffering is disabled by default.

  Stereo buffering provides extra color buffers to generate left-eye
  and right-eye images.

  \sa stereo()
*/

void QGLFormat::setStereo( bool enable )
{
    if ( data->stereo != enable ) {
	detach();
	data->stereo = enable;
    }
}


/*!
  Returns TRUE if the window system has any OpenGL support,
  otherwise FALSE.
*/

bool QGLFormat::hasOpenGL()
{
#if defined(Q_WGL)
    return TRUE;
#else
    return glXQueryExtension(qt_xdisplay(),0,0) != 0;
#endif
}


static QGLFormat *default_format = 0;

static void cleanupGLFormat()
{
    delete default_format;
    default_format = 0;
}


/*!
  Returns the default QGLFormat for the application.
  All QGLWidgets that are created are assigned this format unless
  anything else is specified.
*/

const QGLFormat &QGLFormat::defaultFormat()
{
    if ( !default_format ) {
	default_format = new QGLFormat;
	qAddPostRoutine( cleanupGLFormat );
    }
    return *default_format;
}

/*!
  Sets a new default QGLFormat for the application.
  For example, to set single buffering as default instead
  of double buffering, your main() can contain:
  \code
    QApplication a(argc, argv);
    QGLFormat f;
    f.setDoubleBuffer( FALSE );
    QGLFormat::setDefault( f );
  \endcode
*/

void QGLFormat::setDefaultFormat( const QGLFormat &f )
{
    delete default_format;
    default_format = new QGLFormat( f );
}


/*****************************************************************************
  QGLContext implementation
 *****************************************************************************/


/*!
  \class QGLContext qgl.h
  \brief The QGLContext class provides an OpenGL context.

  A context is...
*/


/*!
  Constructs an OpenGL context for the paint device \a device,
  which must be a widget or a pixmap.
  The \a format specify several display options for this context.

  The context will be \link isValid() invalid\endlink if the
  \a format settings cannot be satified.

  \sa isValid()
*/

QGLContext::QGLContext( const QGLFormat &format, QPaintDevice *device )
    : glFormat(format), paintDevice(device)
{
    valid = FALSE;
    if ( paintDevice == 0 ) {
#if defined(CHECK_NULL)
	warning( "QGLContext: Paint device cannot be null" );
#endif
    }
    if ( paintDevice->devType() != PDT_WIDGET &&
	 paintDevice->devType() != PDT_PIXMAP ) {
#if defined(CHECK_RANGE)
	warning( "QGLContext: Unsupported paint device type" );
#endif
    }
}

/*!
  Destroys the OpenGL context.
*/

QGLContext::~QGLContext()
{
    reset();
}


/*!
  \fn const QGLFormat QGLContext::format() const
  Returns the format.
  \sa setFormat()
*/

/*!
  Sets a new OpenGL context specification \a format.
  The context is \link reset() reset\endlink.
  Call create() to create a new context that matches
  this format.

  \code
    QGLContext *cx;
      ...
    QGLFormat f;
    f.setAlpha( TRUE );
    f.setStereo( TRUE );
    cx->setFormat( f );
    if ( !cx->create() )
	; // could not create context
  \endcode

  \sa format(), reset(), create()
*/

void QGLContext::setFormat( const QGLFormat &format )
{
    reset();
    glFormat = format;
}


/*!
  \fn bool QGLContext::isValid() const
  Returns TRUE if the widget was able to satisfy the specified constraints 
*/

/*!
  Creates the GL context. Returns TRUE if it was successful in creating
  a context that satisfies the requested \link setFormat() format\endlink,
  otherwise FALSE is returned (the context is invalid).

  \internal
  <strong>Implementation note:</strong> Initialization of C++ class members
  usually takes place in the class constructor. QGLContext is an exception
  because it must be simple to customize. The virtual functions
  chooseContext() (and chooseVisual() for X11) can be reimplemented in a
  subclass to select a particular context. The trouble is that virtual
  functions are not properly called during construction (which is indeed
  correct C++), hence we need a create() function.

  \sa chooseContext(), isValid()
*/

bool QGLContext::create()
{
    if ( valid )
	reset();
    valid = chooseContext();
    return valid;
}


/*****************************************************************************
  QGLContext Win32/WGL-specific code
 *****************************************************************************/

#if defined(Q_WGL)

/*!
  This semi-internal function is called by create(). It creates a
  system-dependent OpenGL handle that matches the specified \link
  setFormat() format\endlink.

  <strong>Windows</strong>: Calls choosePixelFormat() which finds a
  matching pixel format identifier.

  <strong>X11</strong>: Calls chooseVisual() which finds an appropriate
  X visual.

  choosePixelFormat() and chooseVisual() can be reimplemented in a
  subclass if you need to choose a very custom context.
*/

bool QGLContext::chooseContext()
{
    if ( paintDevice->devType() == PDT_WIDGET ) {
	win = ((QWidget *)paintDevice)->winId();
	dc  = GetDC( win );
    } else {
	win = 0;
	dc  = paintDevice->handle();
    }
    PIXELFORMATDESCRIPTOR pfd;
    int pixelFormatId = choosePixelFormat( &pfd );
    if ( pixelFormatId == 0 ) {
	rc = 0;
	dc = 0;
    } else {
	if ( !SetPixelFormat( dc, pixelFormatId, &pfd ) ) {
	    LPVOID lpMsgBuf;
	    FormatMessage( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		0, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf, 0, 0 );
	    //debug( (const char *)lpMsgBuf );
	    LocalFree( lpMsgBuf );
 	}
	rc = wglCreateContext( dc );
    }
    if ( win ) {
	ReleaseDC( win, dc );
	dc = 0;
    }
    return TRUE;
}


/*!
  <strong>Win32 only</strong>: This virtual function chooses a pixel format
  that matchesthe OpenGL \link setFormat() format\endlink. Reimplement this
  function in a subclass if you need a custom context.

  \warning The \a pfd pointer is really a \c PIXELFORMATDESCRIPTOR*.
  We use \c void to avoid using Windows-specific types in our header files.

  \sa chooseContext()
*/

int QGLContext::choosePixelFormat( void *pfd )
{
    PIXELFORMATDESCRIPTOR *p = (PIXELFORMATDESCRIPTOR *)pfd;
    memset( p, 0, sizeof(PIXELFORMATDESCRIPTOR) );
    p->nSize = sizeof(PIXELFORMATDESCRIPTOR);
    p->nVersion = 1;
    p->dwFlags  = PFD_SUPPORT_OPENGL;
    if ( paintDevice->devType() == PDT_WIDGET ) {
	p->dwFlags |= PFD_DRAW_TO_WINDOW;
    } else if ( paintDevice->devType() == PDT_PIXMAP ) {
	p->dwFlags |= PFD_DRAW_TO_BITMAP;
    }
    if ( glFormat.doubleBuffer() )
	p->dwFlags |= PFD_DOUBLEBUFFER;
    if ( glFormat.stereo() )
	p->dwFlags |= PFD_STEREO;
    if ( glFormat.depth() )
	p->cDepthBits = 32;
    if ( glFormat.rgba() ) {
	p->iPixelType = PFD_TYPE_RGBA;
	p->cColorBits = 24;
    } else {
	p->iPixelType = PFD_TYPE_COLORINDEX;
	p->cColorBits = 8;
    }
    if ( glFormat.alpha() )
	p->cAlphaBits = 8;
    if ( glFormat.accum() )
	p->cAccumBits = p->cColorBits + p->cAlphaBits;
    if ( glFormat.stencil() )
	p->cStencilBits = 4;
    p->iLayerType = PFD_MAIN_PLANE;
    return ChoosePixelFormat( dc, p );
}


/*!
  Resets the context and makes it invalid.
  \sa create(), isValid()
*/

void QGLContext::reset()
{
    if ( !valid )
	return;
    doneCurrent();
    wglDeleteContext( rc );
    rc = 0;
    dc = 0;
    win = 0;
    valid = FALSE;
}

//
// NOTE: In a multi-threaded environment, each thread has a current
// context. If we want to make this code thread-safe, we probably
// have to use TLS (thread local storage) for keeping current contexts.
//

static QGLContext *currentContext = 0;

/*!
  Makes this context the current context
 */
void QGLContext::makeCurrent()
{
    /*
    if ( currentContext ) {
	if ( currentContext == this )		// already current
	    return;
	currentContext->doneCurrent();
    }
    */
    dc = paintDevice->handle();
    if ( dc ) {
	tmp_dc = FALSE;
    } else {
	tmp_dc = TRUE;
	dc = GetDC( win );
    }
    if ( QColor::hPal() ) {
	SelectPalette( dc, QColor::hPal(), FALSE );
	RealizePalette( dc );
    }
    wglMakeCurrent( dc, rc );
    currentContext = this;
}

void QGLContext::doneCurrent()
{
    return;
    if ( currentContext != this )
	return;
    wglMakeCurrent( 0, 0 );
    if ( tmp_dc ) {
#if defined(DEBUG)
	ASSERT( win != 0 );
#endif
	ReleaseDC( win, dc );
	dc = 0;
    }
    currentContext = 0;
}

/*!
  Swaps the screen contents with an off-screen buffer. Works only if
  the  context is in double buffer mode.
  \sa QGLFormat::setDoubleBuffer()
*/

void QGLContext::swapBuffers()
{
    if ( dc )
	SwapBuffers( dc );
}

#endif // Q_WGL


/*****************************************************************************
  QGLContext Unix/GLX-specific code
 *****************************************************************************/

#if defined(Q_GLX)

/*!
  <strong>X11 only</strong>: This virtual function chooses a visual
  that matches the OpenGL \link setFormat() format\endlink. Reimplement this
  function in a subclass if you need a custom visual.

  \sa chooseContext()
*/

void *QGLContext::chooseVisual()
{
    int spec[40];
    int i = 0;
    QGLFormat f = format();
    spec[i++] = GLX_LEVEL;
    spec[i++] = 0;
    if ( f.doubleBuffer() )
	spec[i++] = GLX_DOUBLEBUFFER;
    if ( f.depth() ) {
	spec[i++] = GLX_DEPTH_SIZE;
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
	if ( f.stereo() ) {
	    spec[i++] = GLX_STEREO;
	    spec[i++] = TRUE;
	}
    } else {
	spec[i++] = GLX_BUFFER_SIZE;
	spec[i++] = 24;
    }
    if ( f.stencil() ) {
	spec[i++] = GLX_STENCIL_SIZE;
	spec[i++] = 1;
    }
    spec[i] = None;
    return glXChooseVisual( paintDevice->x11Display(),
			    paintDevice->x11Screen(), spec );
}


bool QGLContext::chooseContext()
{
    vi = chooseVisual();
    if ( vi == 0 )
	return FALSE;
    cx = glXCreateContext( paintDevice->x11Display(), (XVisualInfo *)vi,
			   None, TRUE );
    return cx != 0;
}

void QGLContext::reset()
{
    if ( !valid )
	return;
    doneCurrent();
    glXDestroyContext( paintDevice->x11Display(), (GLXContext)cx );
    vi = 0;
    cx = 0;
    valid = FALSE;
}


void QGLContext::makeCurrent()
{
    if ( !valid )
	return;
    if ( paintDevice->devType() == PDT_WIDGET ) {
	glXMakeCurrent( paintDevice->x11Display(),
			((QWidget *)paintDevice)->winId(),
			(GLXContext)cx );
    }
}

void QGLContext::doneCurrent()
{
}


void QGLContext::swapBuffers()
{
    if ( !valid )
	return;
    if ( paintDevice->devType() == PDT_WIDGET ) {
	glXSwapBuffers( paintDevice->x11Display(),
			((QWidget *)paintDevice)->winId() );
    }
}

#endif // Q_GLX


/*****************************************************************************
  QGLWidget implementation
 *****************************************************************************/


/*!
  \class QGLWidget qgl.h
  \brief The QGLWidget class is a widget for rendering OpenGL graphics.

  It is easy to render OpenGL graphics in Qt applications. You can create
  a subclass of QGLWidget and reimplement two functions: resizeGL() and
  paintGL(). resizeGL() is called whenever the widget has been resized.
  paintGL() is called when the widget needs to be updated.

  \code
    class DrawGL : public QGLWidget
    {
        QOBJECT;   // Include this if you want to use Qt signals & slots etc.

    public:
        DrawGL( QWidget *parent, const char *name )
	    : QGLWidget(parent,name) {}

    protected:
        void paintGL()
	{
	  // draw the scene
	}

	void resizeGL( int w, int h )
	{
	  // setup viewport, display lists etc.
	}
    };
  \endcode

  If you need to repaint from other places than paintGL() (a typical
  example is when using \link QTimer timers\endlink to animate scenes)
  you must call makeCurrent() before you draw and probably swapBuffers()
  when you are finished.

  Like QGLContext, QGLWidget has advanced functions for requesting
  a new contex format and you can even set a new context.
*/


/*!
  Constructs an OpenGL widget with a \a parent widget and a \a name.

  The \link QGLFormat::defaultFormat() default format\endlink is
  used.

  \sa QGLFormat::defaultFormat()
*/

QGLWidget::QGLWidget( QWidget *parent, const char *name )
    : QWidget(parent, name)
{
    setBackgroundColor( black );
    glcx = 0;
    setContext( new QGLContext(QGLFormat::defaultFormat(),this) );
}


/*!
  Constructs an OpenGL widget with a \a parent widget and a \a name.

  The \a format argument specifies the rendering capabilities.
  The widget becomes invalid if the driver/hardware cannot satisfy
  the requested format.

  \sa isValid()
*/

QGLWidget::QGLWidget( const QGLFormat &format, QWidget *parent,
		      const char *name )
    : QWidget(parent, name)
{
    setBackgroundColor( black );
    glcx = 0;
    setContext( new QGLContext(format,this) );
}


/*!
  \fn const QGLFormat &QGLWidget::format() const
  Returns the widget's format.
  \sa setFormat()
*/

/*!
  \fn bool QGLWidget::doubleBuffer() const
  Returns TRUE if double buffering is set for this widget.
  \sa QGLFormat::doubleBuffer()
*/


/*!
  \fn bool QGLWidget::isValid() const
  Returns TRUE if the widget was able to satisfy the specified constraints 
*/

/*!
  \fn void QGLWidget::makeCurrent()
  Makes this widget the current widget for OpenGL operations.
*/

/*!
  \fn void QGLWidget::swapBuffers()
  Swaps the screen contents with an off-screen buffer. Works only if
  the  context is in double buffer mode.
  \sa QGLFormat::setDoubleBuffer()
*/


/*!
  Sets a new format for this widget. The widget becomes invalid if the
  requested format cannot be satisfied.
  \sa format(), setContext(), isValid()
*/

void QGLWidget::setFormat( const QGLFormat &format )
{
    glcx->setFormat( format );
    glcx->create();
}


/*!
  \fn const QGLContext *QGLWidget::context() const
  Returns the current context.
  \sa setContext()
*/

/*!
  Sets a new context for this widget. The old context is deleted.
  \sa context(), setFormat()
*/

void QGLWidget::setContext( QGLContext *context )
{
    if ( context == 0 ) {
#if defined(CHECK_NULL)
	warning( "QGLWidget::setContext: Cannot set null context" );
#endif
    }
    if ( context->device() != this ) {
#if defined(CHECK_STATE)
	warning( "QGLWidget::setContext: Context must refer this widget" );
#endif
    }
    delete glcx;
    glcx = context;
    if ( !glcx->isValid() ) {
	if ( !glcx->create() )
	    return;
    }
#if defined(Q_GLX)
    bool visible = isVisible();
    if ( visible )
        hide();
    XVisualInfo *vi = (XVisualInfo*)glcx->vi;
    if ( XVisualIDFromVisual(vi->visual) ==
	 XVisualIDFromVisual((Visual*)QPaintDevice::x11Visual()) )
	return;

    /*
      Here we can optimize:
	1) Create one colormap for each visual.
	2) Recreate window only if necessary.
    */
    Colormap cmap;
    cmap = XCreateColormap( dpy, RootWindow(dpy,vi->screen),
			    vi->visual, AllocNone );
    XSetWindowAttributes a;
    a.colormap = cmap;
    a.background_pixel = backgroundColor().pixel();
    a.border_pixel = black.pixel();
    Window p = RootWindow( dpy, DefaultScreen(dpy) );
    if ( parentWidget() )
	p = parentWidget()->winId();
    Window w = XCreateWindow( dpy, p,  x(), y(), width(), height(),
			      0, vi->depth, InputOutput,  vi->visual,
			      CWBackPixel|CWBorderPixel|CWColormap, &a );
    create( w );
    if ( visible )
	show();
#endif
}


/*!
  \fn void QGLWidget::updateGL()

  Updates the widget by calling paintGL().
*/


/*!
  This virtual function is called whenever the widget needs to be painted.
  Reimplement it in a subclass.
*/

void QGLWidget::paintGL()
{
}


/*!
  \fn void QGLWidget::resizeGL( int width , int height )
  This virtual function is called whenever the widget has been resized.
  Reimplement it in a subclass.
*/

void QGLWidget::resizeGL( int, int )
{
}


/*!
  Handles paint events. Calls the virtual function paintGL().
*/

void QGLWidget::paintEvent( QPaintEvent * )
{
    glcx->makeCurrent();
    paintGL();
    if ( glcx->format().doubleBuffer() )
	glcx->swapBuffers();
    else
	glFlush();
}

/*!
  Handles resize events. Calls the virtual function resizeGL().
*/

void QGLWidget::resizeEvent( QResizeEvent * )
{
    glcx->makeCurrent();
    resizeGL( width(), height() );
    glcx->doneCurrent();
}


/*****************************************************************************
  QGL classes overview documentation.
 *****************************************************************************/

/*!
\page qgl.html

<title>Qt OpenGL Classes</title>
</head><body bgcolor="#ffffff">

<h1 align=center>Qt OpenGL Classes</h1>
<hr>


<h2>Introduction</h2>

OpenGL is a standard API for rendering 3D graphics.

OpenGL only deals with 3D rendering and does not worry about GUI
programming issues. You must create the user interface with another
toolkit, such as Motif on the X platform, Microsoft Foundation Classes
(MFC) under Windows -- or Qt on both platforms.

<h2>The QGL Classes</h2>

The OpenGL support classes in Qt are:
<ul>
<li> <strong>\link QGLWidget QGLWidget\endlink:</strong> An easy-to-use Qt
  widget for rendering OpenGL scenes.
<li> <strong>\link QGLContext QGLContext\endlink:</strong> An OpenGL context.
<li> <strong>\link QGLFormat QGLFormat\endlink:</strong> Spesifies the
device settings of a context.
</ul>

Many applications need only the high-level QGLWidget class. The other QGL
classes provide advanced features.

*/
