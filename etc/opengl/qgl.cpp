/****************************************************************************
** $Id: //depot/qt/main/etc/opengl/qgl.cpp#5 $
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

RCSTAG("$Id: //depot/qt/main/etc/opengl/qgl.cpp#5 $");


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
  \link setDoubleBuffer() Double buffer:\endlink Enabled (i.e. single buffer
  disabled).
  \link setAlpha() Alpha channel:\endlink Disabled.
  \link setAccum() Accumulator buffer:\endlink Disabled.
  \link setStencil() Stencil buffer:\endlink Disabled.
  \link setStereo() Stereo:\endlink Disabled.
  \link setRgba() RGBA:\endlink Enabled (i.e. color index disabled).
  \link setColorBits() Color bits:\endlink 24.
  \link setDepthBits() Depth bits:\endlink 16.
  </ul>
*/

QGLFormat::QGLFormat()
{
    data = new Internal;
    CHECK_PTR( data );
    data->doubleBuffer = TRUE;
    data->alpha	       = FALSE;
    data->accum	       = FALSE;
    data->stencil      = FALSE;
    data->stereo       = FALSE;
    data->rgba	       = TRUE;
    data->colorBits    = 24;
    data->depthBits    = 16;
}

/*!
  Constructs a GL format object.
  \arg \e doubleBuffer Double buffer if TRUE, single buffer if FALSE.
  \arg \e alpha: Alpha channel enabled if TRUE, disabled if FALSE.
  \arg \e stereo: Stereographic viewing enabled if TRUE, disabled if FALSE.
  \arg \e rgba: RGBA mode if TRUE, color index if FALSE.
  \arg \e colorBits Number of color bits. Ignored if \e rgba is TRUE.
  \arg \e depthBits The depth of the rendering device.
*/

QGLFormat::QGLFormat( bool doubleBuffer, bool alpha, bool accum, bool stencil,
		      bool stereo, bool rgba, int colorBits, int depthBits )
{
    data = new Internal;
    CHECK_PTR( data );
    data->doubleBuffer = doubleBuffer;
    data->alpha	       = alpha;
    data->accum	       = accum;
    data->stencil      = stencil;
    data->stereo       = stereo;
    data->rgba	       = rgba;
    data->colorBits    = colorBits;
    data->depthBits    = depthBits;
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
	QGLFormat f( data->doubleBuffer, data->alpha, data->accum,
		     data->stencil, data->stereo, data->rgba,
		     data->colorBits, data->depthBits );
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


void QGLFormat::setColorBits( int n )
{
    if ( data->colorBits != n ) {
	detach();
	data->colorBits = n;
    }
}


void QGLFormat::setDepthBits( int n )
{
    if ( data->depthBits != n ) {
	detach();
	data->depthBits = n;
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
    init = FALSE;
}

QGLContext::~QGLContext()
{
    cleanup();
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
    if ( init )
	cleanup();
#if defined(Q_GLX)
    if ( !chooseVisual() )
	return FALSE;
#endif
    if ( chooseContext() ) {
	init = TRUE;
	return TRUE;
    }
    return FALSE;
}


/*****************************************************************************
  QGLContext Win32/WGL-specific code
 *****************************************************************************/

#if defined(Q_WGL)

bool QGLContext::chooseContext()
{
    PIXELFORMATDESCRIPTOR pfd;
    memset( &pfd, 0, sizeof(PIXELFORMATDESCRIPTOR) );
    pfd.nSize	 = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    int f = PFD_SUPPORT_OPENGL;
    if ( paintDevice->devType() == PDT_WIDGET ) {
	f |= PFD_DRAW_TO_WINDOW;
    } else if ( paintDevice->devType() == PDT_PIXMAP ) {
	f |= PFD_DRAW_TO_BITMAP;
    } else {
#if defined(CHECK_RANGE)
	warning( "QGLContext: Bad paint device type" );
#endif
    }
    if ( glFormat.doubleBuffer() )
	f |= PFD_DOUBLEBUFFER;
    pfd.dwFlags = f;
    pfd.iPixelType = glFormat.rgba() ?
	PFD_TYPE_RGBA : PFD_TYPE_COLORINDEX;
    pfd.cColorBits = glFormat.colorBits();
    pfd.cDepthBits = glFormat.depthBits();
    pfd.iLayerType = PFD_MAIN_PLANE;

    win = 0;
    int pixelFormatId;
    if ( paintDevice->devType() == PDT_WIDGET ) {
	win = ((QWidget *)paintDevice)->winId();
	dc = GetDC( win );
    } else {
	dc = paintDevice->handle();
    }
    pixelFormatId = ChoosePixelFormat( dc, &pfd );
    SetPixelFormat( dc, pixelFormatId, &pfd );
    rc = wglCreateContext( dc );
    if ( win ) {
	ReleaseDC( win, dc );
	dc = 0;
    }
    current = FALSE;
    return TRUE;
}

void QGLContext::cleanup()
{
    if ( current )
	doneCurrent();
    wglDeleteContext( rc );
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
	if ( current )				// already current
	    return;
	currentContext->doneCurrent();
    }
    dc = paintDevice->handle();
    if ( dc ) {
	tmpdc = FALSE;
    } else {
	tmpdc = TRUE;
	dc = GetDC( win );
    }
    if ( QColor::hPal() ) {
	HANDLE oldPal = SelectPalette( dc, QColor::hPal(), FALSE );
	RealizePalette( dc );
    }

    wglMakeCurrent( dc, rc );
    current = TRUE;
    currentContext = this;
}

void QGLContext::doneCurrent()
{
    if ( !current )
	return;
    wglMakeCurrent( 0, 0 );
    if ( tmpdc ) {
#if defined(DEBUG)
	ASSERT( win != 0 );
#endif
	ReleaseDC( win, dc );
	dc = 0;
    }
    current = FALSE;
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
	spec[i++] = f.colorBits();
    }
    if ( f.stencil() ) {
	spec[i++] = GLX_STENCIL_SIZE;
	spec[i++] = 1;
    }
    spec[i++] = GLX_DEPTH_SIZE;
    spec[i++] = f.depthBits();
    spec[i]   = None;

    Display *dpy = qt_xdisplay();

    vi = glXChooseVisual( dpy, DefaultScreen(dpy), spec);

#if 0
    if ( vi ) {
	XVisualInfo *v = (XVisualInfo*)vi;
	debug( "vi created ok" );
	debug( "  visualid .... %x", v->visualid );
	debug( "  depth ....... %d", v->depth );
	debug( "  class ....... %d", v->c_class );
	debug( "  colmap sz ... %d", v->colormap_size );
	debug( "  bits rgb .... %d", v->bits_per_rgb );
    } else {
	debug( "couldn't create visual" );
    }
#endif

    return vi != 0;
}


bool QGLContext::chooseContext()
{
    cx = glXCreateContext( qt_xdisplay(), (XVisualInfo *)vi, None, TRUE );
#if 0
    if ( cx ) {
	debug( "context created ok" );
    } else {
	debug( "couldn't create context" );
    }
#endif
    return cx != 0;
}

void QGLContext::cleanup()
{
    if ( cx ) {
	glXDestroyContext( qt_xdisplay(), (GLXContext)cx );
	cx = 0;
    }
}

void QGLContext::makeCurrent()
{
    if ( !cx )
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
    if ( !cx )
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
    : QWidget(parent, name), glContext(QGLFormat::defaultFormat(), this)
{
    qglInit();
}


/*!
  Constructs an OpenGL widget with a \e parent widget and a \e name.

  The \e format argument specifies the rendering capabilities.
*/

QGLWidget::QGLWidget( const QGLFormat &format, QWidget *parent,
		      const char *name )
    : QWidget(parent, name), glContext(format,this)
{
    qglInit();
}


void QGLWidget::qglInit()
{
    setBackgroundColor( black );
    if ( !glContext.create() )
	return;
#if defined(Q_GLX)
    XVisualInfo *vi = (XVisualInfo*)glContext.vi;
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
    glContext.makeCurrent();
    paintGL();
    if ( glContext.format().doubleBuffer() )
	glContext.swapBuffers();
    else
	glFlush();
    glContext.doneCurrent();

}

/*!
  Handles resize events. Calls the virtual function resizeGL().
*/

void QGLWidget::resizeEvent( QResizeEvent * )
{
    glContext.makeCurrent();
    resizeGL( width(), height() );
    glContext.doneCurrent();
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
