/****************************************************************************
**
** Implementation of GLWidget class
**
** This implementation is for X11, however, the class definition is
** platform independent.
**
*****************************************************************************/

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "glwidget.h"


bool GLWidget::dblBuf = TRUE;

static bool	  glx_init = FALSE;
static GLXContext glx_context;
static Colormap   glx_cmap;


/*----------------------------------------------------------------------------
  Create an OpenGL widget with a \e parent widget and a \e name.
 ----------------------------------------------------------------------------*/

GLWidget::GLWidget( QWidget *parent, const char *name )
    : QWidget( parent, name )
{
    if ( !glx_init )
	initialize();
}

/*----------------------------------------------------------------------------
  Common initialization for all GLWidgets. This function is called
  the first time a GLWidget is created.
 ----------------------------------------------------------------------------*/

void GLWidget::initialize()
{
    if ( glx_init )
	return;
    glx_init = TRUE;

    Display     *dpy = qt_xdisplay();
    XVisualInfo *vi = 0;

    static int dbuf[] = {
	GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DEPTH_SIZE, 16,
	GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,None };
    static int *sbuf = &dbuf[1];

    if ( dblBuf )
	vi = glXChooseVisual( dpy, DefaultScreen(dpy), dbuf );
    if ( !vi ) {
	dblBuf = FALSE;
	vi = glXChooseVisual( dpy, DefaultScreen(dpy), sbuf );
	if ( !vi )
	    fatal( "GLWidget::initialize: Cannot create a visual" );
    }
    glx_context = glXCreateContext( dpy, vi, None, GL_TRUE );
    if ( !glx_context )
	fatal( "GLWidget::initialize: Cannot create a GLX context" );
    glx_cmap = XCreateColormap( dpy, RootWindow(dpy,vi->screen),
				vi->visual, AllocNone );

}


/*----------------------------------------------------------------------------
  Enables or disables double buffering. The default setting is TRUE.

  Call this static function before you create any GLWidgets.
 ----------------------------------------------------------------------------*/

void GLWidget::setDoubleBuffer( bool enable )
{
    if ( glx_init ) {
	warning( "GLWidget::setDoubleBuffer: Too late, set it before creating"
		 " any GLWidget" );
	return;
    }
    dblBuf = enable;
}


/*----------------------------------------------------------------------------
  This virtual function is called whenever the widget needs to be painted.
  Reimplement it in a subclass.
 ----------------------------------------------------------------------------*/

void GLWidget::paintGL()
{
}

/*----------------------------------------------------------------------------
  This virtual function is called whenever the widget has been resized.
  Reimplement it in a subclass.
 ----------------------------------------------------------------------------*/

void GLWidget::resizeGL( int, int )
{
}


void GLWidget::swapBuffers()
{
    glXSwapBuffers( x11Display(), winId() );
}

void GLWidget::makeCurrentGL()
{
    glXMakeCurrent( x11Display(), winId(), glx_context );
}

void GLWidget::paintEvent( QPaintEvent * )
{
    makeCurrentGL();
    paintGL();
    if ( doubleBuffer() )
	swapBuffers();
}

void GLWidget::resizeEvent( QResizeEvent * )
{
    makeCurrentGL();
    resizeGL( width(), height() );
}
