/****************************************************************************
** $Id: //depot/qt/main/etc/opengl/qgl.cpp#4 $
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

RCSTAG("$Id: //depot/qt/main/etc/opengl/qgl.cpp#4 $");


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
    data->rgba	       = TRUE;
    data->colorBits    = 24;
    data->depthBits    = 16;
}

/*!
  Constructs a GL format object.
  \arg \e doubleBuffer Double buffer if TRUE, single buffer if FALSE.
  \arg \e rgba: RGBA if TRUE, color index if FALSE.
  \arg \e colorBits Number of color bits.
  \arg \e depthBits The depth of the rendering device.
*/

QGLFormat::QGLFormat( bool doubleBuffer, bool rgba,
		      int colorBits, int depthBits )
{
    data = new Internal;
    CHECK_PTR( data );
    data->doubleBuffer = doubleBuffer;
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
	QGLFormat f( data->doubleBuffer, data->rgba,
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
\fn bool QGLFormat::rgba() const
  \sa setColorMode()
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


/*!
  Returns a unique key that represents the format currently set.
  The key is used for optimization purposes (hash table lookup).
*/

static QString createKey( const QGLFormat &f )
{
    QString k;
    k.sprintf( "%d-%d-%d-%d", (int)f.doubleBuffer(), (int)f.rgba(),
	       f.colorBits(), f.depthBits() );
    return k;
}


#if defined(Q_GLX)

/*
  Returns a score that describes how well the visual info matches
  the requested QGLFormat.
*/

static int score( const QGLFormat &f, Display *dpy, XVisualInfo *vi )
{
    int doubleBuffer;
    int rgba;
    int colorBits;
    int depthBits;
    glXGetConfig( dpy, vi, GLX_DOUBLEBUFFER, &doubleBuffer );
    glXGetConfig( dpy, vi, GLX_RGBA,	     &rgba );
    glXGetConfig( dpy, vi, GLX_BUFFER_SIZE,  &colorBits );
    glXGetConfig( dpy, vi, GLX_DEPTH_SIZE,   &depthBits );
    int score = 0;
    if ( (doubleBuffer && f.doubleBuffer()) ||
	!(doubleBuffer || f.doubleBuffer()) )
	score += 1000;
    if ( (rgba && f.rgba()) || !(rgba || f.rgba()) )
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
    int spec[20];
    int i = 0;
    QGLFormat f = format();
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
    } else {
	spec[i++] = GLX_BUFFER_SIZE;
	spec[i++] = f.colorBits();
    }
    spec[i++] = GLX_DEPTH_SIZE;
    spec[i++] = f.depthBits();
    spec[i]   = None;

    Display *dpy = qt_xdisplay();

    vi = glXChooseVisual( dpy, DefaultScreen(dpy), spec);

    return vi != 0;
}


bool QGLContext::chooseContext()
{
    cx = glXCreateContext( qt_xdisplay(), vi, None, TRUE );
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
}

void QGLContext::doneCurrent()
{
}

void QGLContext::swapBuffers()
{
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
    setBackgroundColor( black );
    glContext.create();
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
    glContext.create();
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
