/****************************************************************************
** $Id: //depot/qt/main/etc/opengl/qgl.cpp#1 $
**
** Implementation of OpenGL classes for Qt
**
** Created : 970112
**
** Copyright (C) 1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qgl.h"
#undef WGL
#undef GLX

#if defined(_OS_WIN32_)
#define WGL
#else
#define GLX
#if defined(_OS_IRIX_)
#define INT8  dummy_INT8
#define INT32 dummy_INT32
#include <GL/glx.h>
#undef INT8
#undef INT32
#else
#include <GL/glx.h>
#endif
#endif


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
    data->dirty		= TRUE;
    data->double_buffer = TRUE;
    data->color_mode    = Rgba;
    data->color_bits    = 24;
    data->depth_bits    = 16;
}

/*!
  Internal constructor. No initialization is performed. See detach().
*/

QGLFormat::QGLFormat( int )
{
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
	QGLFormat f( 0 );
	f.data = new Internal;
	f.data->dirty	      = TRUE;
	f.data->double_buffer = data->double_buffer;
	f.data->color_mode    = data->color_mode;
	f.data->color_bits    = data->color_bits;
	f.data->depth_bits    = data->depth_bits;
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
  Enables double buffering for the context if \e enable is TRUE, or disables
  it if \e enable is FALSE.

  Double buffering is enabled by default.

  Double buffering is a technique where graphics is rendered to a memory
  buffer instead of directly to the screen. When the drawing has been completed,
  you can call swapBuffers() to exchange the buffer contents with the screen.
  The result is flicker-free drawing and often better performance.

  The new setting does not have any effect before you call makeCurrent().

  \sa doubleBuffer()
*/

void QGLFormat::setDoubleBuffer( bool enable )
{
    if ( data->double_buffer != enable ) {
	data->double_buffer = enable;
	data->dirty = TRUE;
    }
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
    if ( data->color_mode != m ) {
	data->color_mode = m;
	data->dirty = TRUE;
    }
}


void QGLFormat::setColorBits( int n )
{
    if ( data->color_bits != n ) {
	data->color_bits = n;
	data->dirty = TRUE;
    }
}


void QGLFormat::setDepthBits( int n )
{
    if ( data->depth_bits != n ) {
	data->depth_bits = n;
	data->dirty = TRUE;
    }
}


bool QGLFormat::hasOpenGL()
{
#if defined(WGL)
    return TRUE;
#else
    return glxQueryExtension(qt_xdisplay(),0,0) != 0;
#endif
}


static QGLFormat *default_format = 0;

static void cleanupGLFormat()
{
    delete default_format;
    default_format = 0;
}

const QGLFormat &defaultFormat()
{
    if ( !default_format ) {
	default_format = new QGLFormat;
	qAddPostRoutine( cleanupGLFormat );
    }
    return *default_format;
}

void setDefaultFormat( const QGLFormat &f )
{
    delete default_format;
    default_format = new QGLFormat( f );
}


/*!
  Returns a unique key that represents the format currently set.
  The key is used for optimization purposes (hash table lookup).
*/

QString QGLFormat::createKey() const
{
    QString k;
    k.sprintf( "%d-%d-%d-%d", (int)doubleBuffer(), (int)colorMode(),
	       colorBits(), depthBits() );
    return k;
}


#if defined(WGL)

HANDLE QGLFormat::getContext( QPaintDevice *pdev ) const
{
    PIXELFORMATDESCRIPTOR pfd;
    memset( &pfd, 0, sizeof(PIXELFORMATDESCRIPTOR) );
    pfd.nSize	 = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    int f = PFD_SUPPORT_OPENGL;
    if ( pdev->devType() == PDT_WIDGET )
	f = PFD_DRAW_TO_WINDOW;
    else if ( pdev->devType() == PDT_PIXMAP )
	f = PFD_DRAW_TO_BITMAP;
#if defined(CHECK_RANGE)
    else
	warning( "QGLFormat::getContext: Bad paint device type" );
#endif
    if ( doubleBuffer() )
	f |= PFD_DOUBLEBUFFER;
    pfd.dwFlags = f;
    pfd.iPixelType = colorMode() == Rgba ?
	PFD_TYPE_RGBA : PFD_TYPE_COLORINDEX;
    pfd.cColorBits = colorBits();
    pfd.cDepthBits = depthBits();
    pfd.iLayerType = PFD_MAIN_PLANE;

    HANDLE dc;
    HANDLE rc;
    int pixelFormatId;
    dc = pdev->handle();
    pixelFormatId = ChoosePixelFormat( dc, &pfd );
    SetPixelFormat( dc, pixelFormatId, &pfd );
    rc = wglCreateContext( dc );
//    ReleaseDC( winId(), hdc );
    return rc;
}

#endif // WGL


#if defined(GLX)

static int score( const QGLFormat *f, XDisplay *dpy, XVisualInfo *vi )
{
    int doubleBuffer;
    glXGetConfig( dpy, vi, GLX_DOUBLEBUFFER, &doubleBuffer );
    ...
}

#endif
