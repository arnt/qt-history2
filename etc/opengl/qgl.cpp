/****************************************************************************
** $Id: //depot/qt/main/etc/opengl/qgl.cpp#7 $
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

RCSTAG("$Id: //depot/qt/main/etc/opengl/qgl.cpp#7 $");


#if defined(_CC_MSVC_)
#pragma warning(disable:4355) // 'this' : used in base member initializer list
#endif


/*****************************************************************************
  QGLFormat implementation
 *****************************************************************************/

 
 /*!
  \class QGLFormat qgl.h
  \brief Specification of OpenGL rendering format for Qt programs.

  The QGLFormat class represents an OpenGL format for an output device.
*/


/*!
  Constructs a format object with the default settings:
  <ul>
  \link setDoubleBuffer() Double buffer:\endlink Enabled if \e doubleBuffer
  is TRUE, otherwise single buffer.
  \link setDepth() Depth buffer:\endlink Enabled.
  \link setRgba() RGBA:\endlink Enabled (i.e. color index disabled).
  \link setAlpha() Alpha channel:\endlink Disabled.
  \link setAccum() Accumulator buffer:\endlink Disabled.
  \link setStencil() Stencil buffer:\endlink Disabled.
  \link setStereo() Stereo:\endlink Disabled.
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

QGLFormat::QGLFormat( const QGLFormat &f )
{
    data = f.data;
    data->ref();
}

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


QGLFormat &QGLFormat::operator=( const QGLFormat &f )
{
    f.data->ref();					// beware of f = f
    if ( data->deref() )
	delete data;
    data = f.data;
    return *this;
}


/*!
  Specifies double buffering if \e enable is TRUE og single buffering if
  \e enable is FALSE.

  Double buffering is enabled by default.

  Double buffering is a technique where graphics is rendered to a memory
  buffer instead of directly to the screen. When the drawing has been
  completed, you can call swapBuffers() to exchange the buffer contents
  with the screen.  The result is flicker-free drawing and often better
  performance.

  The new setting does not have any effect before you call makeCurrent().

  \sa doubleBuffer()
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
  \sa setDepth()
*/

/*!
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
  \sa setRgba()
*/

/*!
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
  \sa setAlpha()
*/

/*!
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
  \sa setAccum()
*/

/*!
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
  \sa setStencil()
*/

/*!
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
  \sa setStereo()
*/

/*!
  \sa stereo()
*/

void QGLFormat::setStereo( bool enable )
{
    if ( data->stereo != enable ) {
	detach();
	data->stereo = enable;
    }
}


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

const QGLFormat &QGLFormat::defaultFormat()
{
    if ( !default_format ) {
	default_format = new QGLFormat;
	qAddPostRoutine( cleanupGLFormat );
    }
    return *default_format;
}

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
  The QGLContext class provides an OpenGL context.

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
  Sets a new OpenGL context specification \e format.
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
  Creates the GL context. Returns TRUE if it was successful in creating
  a context that satisfies the requested \link format() format\endlink,
  otherwise FALSE is returned.

  \internal
  <strong>Implementation note:</strong> Initialization of C++ class members
  usually takes place in the class constructor. QGLContext is an exception
  because it must be simple to customize. The virtual functions
  chooseContext() (and chooseVisual() for X11) can be reimplemented in a sub
  class to select a particular context. The trouble is that virtual functions
  are not properly called during construction (which is indeed correct C++),
  hence we need a create() function.

  \sa chooseContext()
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
    debug( "chose pixel format %d", pixelFormatId );
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
	    debug( (const char *)lpMsgBuf );
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

  \warning The \e pfd pointer is really a \c PIXELFORMATDESCRIPTOR*.
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
	p->cDepthBits = 16;
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
// NOTE: In a multi-threaded environment, each thread has a current GL
// context. If we want to make this code thread-safe, we probably
// have to use TLS (thread local storage) for keeping current contexts.
//

static QGLContext *currentContext = 0;

void QGLContext::makeCurrent()
{
    if ( currentContext ) {
	if ( currentContext == this )		// already current
	    return;
	currentContext->doneCurrent();
    }
    dc = paintDevice->handle();
    if ( dc ) {
	tmp_dc = FALSE;
    } else {
	tmp_dc = TRUE;
	dc = GetDC( win );
    }
    if ( QColor::hPal() ) {
	HANDLE oldPal = SelectPalette( dc, QColor::hPal(), FALSE );
	RealizePalette( dc );
    }
    wglMakeCurrent( dc, rc );
    currentContext = this;
}

void QGLContext::doneCurrent()
{
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

bool QGLContext::chooseVisual()
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
#if 0
    if ( vi ) {
	XVisualInfo *v = (XVisualInfo*)vi;
	debug( "QGLContext: Visual found" );
	debug( "  visualid .... %x", v->visualid );
	debug( "  depth ....... %d", v->depth );
	debug( "  class ....... %d", v->c_class );
	debug( "  colmap sz ... %d", v->colormap_size );
	debug( "  bits rgb .... %d", v->bits_per_rgb );
    } else {
	debug( "QGLContext: Could not find visual" );
    }
#endif
    if ( vi == 0 )
	return FALSE;
    cx = glXCreateContext( paintDevice->x11Display(), (XVisualInfo *)vi,
			   None, TRUE );
#if 0
    if ( cx ) {
	debug( "QGLContext: Context created" );
    } else {
	debug( "QGLContext: Could not create context" );
    }
#endif
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
  Constructs an OpenGL widget with a \e parent widget and a \e name.

  The \link QGLFormat::defaultFormat() default GL format\endlink is
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
  Constructs an OpenGL widget with a \e parent widget and a \e name.

  The \e format argument specifies the rendering capabilities.
*/

QGLWidget::QGLWidget( const QGLFormat &format, QWidget *parent,
		      const char *name )
    : QWidget(parent, name)
{
    setBackgroundColor( black );
    glcx = 0;
    setContext( new QGLContext(format,this) );
}


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


void QGLWidget::setFormat( const QGLFormat &format )
{
    glcx->setFormat( format );
    glcx->create();
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
    glcx->doneCurrent();

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

OpenGL is a standard API for rendering 3D graphics. blablabla

OpenGL only deals with 3D rendering and does not worry about GUI
programming issues. You need to create the user interface using another
toolkit, such as Motif on the X platform, Microsoft Foundation Classes
(MFC) under Windows -- or Qt on both platforms.


<h2>The QGL Classes</h2>

The OpenGL-related Qt classes are:
<ul>
<li> <strong>QGLWidget</strong> - A Qt widget that can display OpenGL
     graphics.
<li> <strong>QGLContext</strong> - An OpenGL context.
<li> <strong>QGLFormat</strong> - Defines the attributes/format of a GL
     context.
</ul>

Many applications need only the high-level QGLWidget class. The other QGL
classes provide advanced features not found in QGLWidget.


<h2>A Widget Template</h2>

The code below outlines how to write an OpenGL widget using Qt.

\code
#include <qgl.h>
#include <qapp.h>

class Template : public QGLWidget
{
    Q_OBJECT
public:
    Template( QWidget *parent=0, const char *name=0 );
protected:
    void resizeGL( int, int );
    void paintGL();
private:
    // your private stuff here
};

Template::Template( QWidget *parent, const char *name )
    : QGLWidget( parent, name )
{
    // Initialize private stuff
}

void Template::resizeGL( int width, int height )
{
    // Define the projection and viewport here
}

void Template::paintGL()
{
    // Draw the scene
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );
    Template w;
    a.setMainWidget( &w );
    w.show();
    return a.exec();
}
\endcode


<h2>Bugs and Limitations</h2>

This implementation has some limitations:
<ul>
<li> Stencils are not supported.
<li> Layers are not supported.
<li> Stereographic option not supported.
</ul>
We hope to implement most of these features in the coming releases of Qt.

*/
