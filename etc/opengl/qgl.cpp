/****************************************************************************
** $Id: //depot/qt/main/etc/opengl/qgl.cpp#3 $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qgl.h"

#if defined(_OS_IRIX_)
#define INT8  dummy_INT8
#define INT32 dummy_INT32
#endif

#if defined(_OS_WIN32_)
#define USE_WGL
#else
#define USE_GLX
#include <GL/glx.h>
#endif // _OS_WIN32_

#if defined(_OS_IRIX_)
#undef INT8
#undef INT32
#endif

RCSTAG("$Id: //depot/qt/main/etc/opengl/qgl.cpp#3 $");


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
  \link setDoubleBuffer() Double buffer:\endlink Enabled.
  \link setColorMode() Color mode:\endlink RGBA.
  \link setColorBits() Color bits:\endlink 24.
  \link setDepthBits() Depth bits:\endlink 24.
  </ul>
*/

QGLFormat::QGLFormat()
{
    data = new Internal;
    CHECK_PTR( data );
    data->double_buffer = TRUE;
    data->color_mode    = Rgba;
    data->color_bits    = 24;
    data->depth_bits    = 16;
}

/*!
  Constructs a GL format object.
  \arg \e doubleBuffer Double buffer if TRUE, single buffer if FALSE.
  \arg \e colorMode QGLFormat::Rgba or QLFormat::ColorIndex.
  \arg \e colorBits Number of color bits.
  \arg \e depthBits The depth of the rendering device.
*/

QGLFormat::QGLFormat( bool doubleBuffer, QGLFormat::ColorMode colorMode,
		      int colorBits, int depthBits )
{
    data = new Internal;
    CHECK_PTR( data );
    data->double_buffer = doubleBuffer;
    data->color_mode    = colorMode;
    data->color_bits    = colorBits;
    data->depth_bits    = depthBits;
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
	QGLFormat f( data->double_buffer, data->color_mode,
		     data->color_bits, data->depth_bits );
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
    data->double_buffer = enable;
}


/*!
  \fn QGLWidget::ColorMode colorMode() const
  Returns the color mode.

  Color modes:
  <ul>
  <li> \c QGLFormat::Rgba (default)
  <li> \c QGLFormat::ColorIndex
  </ul>

  \sa setColorMode()
*/

/*!
  Sets the color mode for the context to \e m.

  The \e m argument must be:
  <ul>
  <li> \c QGLFormat::Rgba (default)
  <li> \c QGLFormat::ColorIndex
  </ul>

  \sa colorMode()
*/

void QGLFormat::setColorMode( QGLFormat::ColorMode m )
{
    if ( data->color_mode != m )
	data->color_mode = m;
}


void QGLFormat::setColorBits( int n )
{
    if ( data->color_bits != n )
	data->color_bits = n;
}


void QGLFormat::setDepthBits( int n )
{
    if ( data->depth_bits != n )
	data->depth_bits = n;
}


bool QGLFormat::hasOpenGL()
{
#if defined(USE_WGL)
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


/*!
  Returns a unique key that represents the format currently set.
  The key is used for optimization purposes (hash table lookup).
*/

static QString createKey( const QGLFormat &f )
{
    QString k;
    k.sprintf( "%d-%d-%d-%d", (int)f.doubleBuffer(), (int)f.colorMode(),
	       f.colorBits(), f.depthBits() );
    return k;
}


#if defined(USE_GLX)

/*
  Returns a score that describes how well the visual info matches
  the requested QGLFormat.
*/

static int score( const QGLFormat &f, Display *dpy, XVisualInfo *vi )
{
    int doubleBuffer;
    int rgbaMode;
    int colorBits;
    int depthBits;
    glXGetConfig( dpy, vi, GLX_DOUBLEBUFFER, &doubleBuffer );
    glXGetConfig( dpy, vi, GLX_RGBA,	     &rgbaMode );
    glXGetConfig( dpy, vi, GLX_BUFFER_SIZE,  &colorBits );
    glXGetConfig( dpy, vi, GLX_DEPTH_SIZE,   &depthBits );
    int score = 0;
    if ( (doubleBuffer && f.doubleBuffer()) ||
	!(doubleBuffer || f.doubleBuffer()) )
	score += 1000;
    if ( (rgbaMode && f.colorMode() == QGLFormat::Rgba) ||
	!(rgbaMode || f.colorMode() == QGLFormat::Rgba) )
	score += 2000;
    if ( colorBits < f.colorBits() )
	score -= (f.colorBits() - colorBits)*10;
    if ( depthBits < f.depthBits() )
	score -= (f.depthBits() - depthBits)*10;
    return score;
}

#if 0
void *QGLFormat::getVisualInfo() const
{

}

uint QGLFormat::getContext( QPaintDevice * ) const
{
}
#endif

#endif // GLX



/*****************************************************************************
  QGLContext implementation
 *****************************************************************************/


QGLContext::QGLContext( const QGLFormat &format, QPaintDevice *device )
    : glFormat(format), paintDevice(device)
{
    init();
}

QGLContext::~QGLContext()
{
    cleanup();
}

/*****************************************************************************
  QGLContext Win32/WGL-specific code
 *****************************************************************************/

#if defined(USE_WGL)

void QGLContext::init()
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
    pfd.iPixelType = glFormat.colorMode() == QGLFormat::Rgba ?
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

#endif // USE_WGL


/*****************************************************************************
  QGLContext Unix/GLX-specific code
 *****************************************************************************/

#if defined(USE_GLX)

void QGLContext::init()
{
}

void QGLContext::cleanup()
{
}

void QGLContext::makeCurrent()
{
}

void QGLContext::doneCurrent()
{
}

void QGLContext::swapBuffers()
{
}

#endif // USE_GLX


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
    setBackgroundColor( black );
}


/*!
  Constructs an OpenGL widget with a \e parent widget and a \e name.

  The \e format argument specifies the rendering capabilities.
*/

QGLWidget::QGLWidget( const QGLFormat &format, QWidget *parent,
		      const char *name )
    : QWidget(parent, name), glContext(format,this)
{
    setBackgroundColor( black );
}


/*!
  \fn void GLWidget::updateGL()

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
