/****************************************************************************
** $Id: $
**
** Copyright (C) 1992-2001 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

/****************************************************************************
**
** This is a simple QGLWidget displaying an openGL wireframe box
**
** The OpenGL code is mostly borrowed from Brian Pauls "double" example
** in the Mesa distribution
**
****************************************************************************/

#include "gldouble.h"

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

/*!
  Create a GLDouble widget
*/

GLDouble::GLDouble( QWidget* parent, const char* name, WFlags f, QGLFormat form )
    : GLControlWidget( form, parent, name, 0, f )
{
    setAnimationDelay( 50 );
    xRot = yRot = zRot = 0.0;		// default object rotation
    scale = 1.25;			// default object scale
    spin = 0.0;
    enableSpin = GL_FALSE;
}


/*!
  Release allocated resources
*/

GLDouble::~GLDouble()
{
    makeCurrent();
}


/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLDouble::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    glRotatef(spin, 0.0, 0.0, 1.0);
    glColor3f(1.0, 1.0, 1.0);
    glRectf(-25.0, -25.0, 25.0, 25.0);
    glPopMatrix();

    swapBuffers();
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLDouble::initializeGL()
{
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_FLAT);
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLDouble::resizeGL( int w, int h )
{
    if( w <= h )
        glViewport (0, 0, (GLsizei) w, (GLsizei) w);
    else
        glViewport (0, 0, (GLsizei) h, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-50.0, 50.0, -50.0, 50.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void GLDouble::animate()
{
    if( enableSpin )
        {
	    spin = spin + 2.0;
	    if (spin > 360.0)
	        spin = spin - 360.0;

	    swapBuffers();
	    updateGL();
	}
}

void GLDouble::mousePressEvent( QMouseEvent *e )
{
    if( e->button() == LeftButton )
        enableSpin = GL_TRUE;
    else
        enableSpin = GL_FALSE;
}
