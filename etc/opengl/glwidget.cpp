/****************************************************************************
** $Id: //depot/qt/main/etc/opengl/glwidget.cpp#4 $
**
** Implementation of GLWidget class for X11.
**
** Author  : Haavard Nord (hanord@troll.no)
** Created : 960620
**
** Copyright (C) 1996 by Troll Tech AS.
**
** Permission to use, copy, modify, and distribute this software and its
** documentation for any purpose and without fee is hereby granted, provided
** that the above copyright notice appear in all copies.
** No representations are made about the suitability of this software for any
** purpose. It is provided "as is" without express or implied warranty.
**
*****************************************************************************/

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glx.h>

#include "glwidget.h"


/*----------------------------------------------------------------------------
  \class GLWidget glwidget.h
  \brief The GLWidget is an OpenGL wrapper class.

  To create your own OpenGL widget, make a subclass of GLWidget and
  reimplement the virtual functions paintGL() and resizeGL().
  Study the Nurb class for an example.

  The GLWidget is a very thin wrapper and it probably lacks functionality.
  Please give feedback to hanord@troll.no.
 ----------------------------------------------------------------------------*/


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
  \fn bool GLWidget::doubleBuffer()

  Returns TRUE if double buffering is enabled, otherwise FALSE.

  \sa setDoubleBuffer()
 ----------------------------------------------------------------------------*/


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
  \fn void GLWidget::updateGL()

  Updates the widget. Does not clear the background first.
 ----------------------------------------------------------------------------*/


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


/*----------------------------------------------------------------------------
  Handles paint events. Calls the virtual function paintGL().
 ----------------------------------------------------------------------------*/

void GLWidget::swapBuffers()
{
    glXSwapBuffers( x11Display(), winId() );
}

/*----------------------------------------------------------------------------
  Makes this widget the current GL object. GLWidget takes care of this
  before calling paintGL() or resizeGL().
 ----------------------------------------------------------------------------*/

void GLWidget::makeCurrentGL()
{
    glXMakeCurrent( x11Display(), winId(), glx_context );
}


/*----------------------------------------------------------------------------
  Handles paint events. Calls the virtual function paintGL().
 ----------------------------------------------------------------------------*/

void GLWidget::paintEvent( QPaintEvent * )
{
    makeCurrentGL();
    paintGL();
    if ( doubleBuffer() )
	swapBuffers();
}

/*----------------------------------------------------------------------------
  Handles resize events. Calls the virtual function resizeGL().
 ----------------------------------------------------------------------------*/

void GLWidget::resizeEvent( QResizeEvent * )
{
    makeCurrentGL();
    resizeGL( width(), height() );
}
