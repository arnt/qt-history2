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
** This is a simple QGLWidget displaying an openGL depth
**
****************************************************************************/

#include "gldepth.h"

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

/*!
  Create a GLDepth widget
*/

GLDepth::GLDepth( QWidget* parent, const char* name, WFlags f, QGLFormat form )
    : GLControlWidget( form, parent, name, 0, f )
{
    setAnimationDelay( -1 );
    xRot = yRot = zRot = 0.0;		// default object rotation
    scale = 1.25;			// default object scale
    leftFirst = GL_TRUE;
}


/*!
  Release allocated resources
*/

GLDepth::~GLDepth()
{
    makeCurrent();
}


/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLDepth::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
 
    if (leftFirst) {
        drawLeftTriangle();
        drawRightTriangle();
    }
    else {
        drawRightTriangle();
        drawLeftTriangle();
    }
    
    glFlush();
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLDepth::initializeGL()
{
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel (GL_FLAT);
    glClearColor (0.0, 0.0, 0.0, 0.0);
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLDepth::resizeGL( int w, int h )
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (w <= h)
        gluOrtho2D (0.0, 1.0, 0.0, 1.0*(GLfloat)h/(GLfloat)w);
    else
        gluOrtho2D (0.0, 1.0*(GLfloat)w/(GLfloat)h, 0.0, 1.0);
}

void GLDepth::drawLeftTriangle(void)
{
    /* draw yellow triangle on LHS of screen */
 
    glBegin (GL_TRIANGLES);
    glColor4f(1.0, 1.0, 0.0, 0.75);
    glVertex3f(0.1, 0.9, 0.0);
    glVertex3f(0.1, 0.1, 0.0);
    glVertex3f(0.7, 0.5, 0.0);
    glEnd();
}

void GLDepth::drawRightTriangle(void)
{
    /* draw cyan triangle on RHS of screen */
 
    glBegin (GL_TRIANGLES);
    glColor4f(0.0, 1.0, 1.0, 0.75);
    glVertex3f(0.9, 0.9, 0.0);
    glVertex3f(0.3, 0.5, 0.0);
    glVertex3f(0.9, 0.1, 0.0);
    glEnd();
}

void GLDepth::mousePressEvent( QMouseEvent * )
{
    if( leftFirst )
        leftFirst = GL_FALSE;
    else
        leftFirst = GL_TRUE;
    updateGL();
}
