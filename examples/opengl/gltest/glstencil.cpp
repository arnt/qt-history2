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
** This is a simple QGLWidget displaying an openGL stencil example
**
** The OpenGL code is mostly borrowed from Mark J. Kilgards "stencil"
** example in the Mesa distribution
**
****************************************************************************/

#include "glstencil.h"
#include "GL/glut.h"

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

#define YELLOWMAT   1
#define BLUEMAT 2

/*!
  Create a GLStencil widget
*/

GLStencil::GLStencil( QWidget* parent, const char* name, WFlags f, QGLFormat form )
    : GLControlWidget( form, parent, name, 0, f )
{
    setAnimationDelay( -1 );
    xRot = yRot = zRot = 0.0;		// default object rotation
    scale = 1.25;			// default object scale
}


/*!
  Release allocated resources
*/

GLStencil::~GLStencil()
{
    makeCurrent();
}


/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLStencil::paintGL()
{
    GLUquadric *q1, *q2;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
/* draw blue sphere where the stencil is 1 */
    glStencilFunc (GL_EQUAL, 0x1, 0x1);
    glCallList (BLUEMAT);
    q1 = gluNewQuadric();
    gluSphere (q1, 0.5, 15, 15);
 
/* draw yellow sphere where the stencil is not 1 */
    glStencilFunc (GL_NOTEQUAL, 0x1, 0x1);
    glStencilOp (GL_KEEP, GL_KEEP, GL_KEEP);
    glPushMatrix();
        glRotatef (45.0, 0.0, 0.0, 1.0);
        glRotatef (45.0, 0.0, 1.0, 0.0);
        glCallList (YELLOWMAT);
	q2 = gluNewQuadric();
	gluSphere (q2, 1.5, 15, 15);
    glPopMatrix();
 
    glFlush();
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLStencil::initializeGL()
{
    GLfloat yellow_diffuse[] = { 0.7, 0.7, 0.0, 1.0 };
    GLfloat yellow_specular[] = { 1.0, 1.0, 1.0, 1.0 };
 
    GLfloat blue_diffuse[] = { 0.1, 0.1, 0.7, 1.0 };
    GLfloat blue_specular[] = { 0.1, 1.0, 1.0, 1.0 };
 
    GLfloat position_one[] = { 1.0, 1.0, 1.0, 0.0 };
 
    glNewList(YELLOWMAT, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, yellow_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, yellow_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 64.0);
    glEndList();
 
    glNewList(BLUEMAT, GL_COMPILE);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, blue_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, blue_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 90.0);
    glEndList();
 
    glLightfv(GL_LIGHT0, GL_POSITION, position_one);
 
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
 
    glClearStencil(0x0);
    glEnable(GL_STENCIL_TEST);
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLStencil::resizeGL( int w, int h )
{
    glViewport(0, 0, w, h);
 
    glClear(GL_STENCIL_BUFFER_BIT);
/* create a diamond shaped stencil area */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-3.0, 3.0, -3.0, 3.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
 
    glStencilFunc (GL_ALWAYS, 0x1, 0x1);
    glStencilOp (GL_REPLACE, GL_REPLACE, GL_REPLACE);
    glBegin(GL_QUADS);
        glVertex3f (-1.0, 0.0, 0.0);
        glVertex3f (0.0, 1.0, 0.0);
        glVertex3f (1.0, 0.0, 0.0);
        glVertex3f (0.0, -1.0, 0.0);
    glEnd();
 
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (GLfloat) w/(GLfloat) h, 3.0, 7.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -5.0);
}
