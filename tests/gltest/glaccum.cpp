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
** This is a simple QGLWidget displaying an openGL alpha
**
** The OpenGL code is mostly borrowed from Silicon Graphics, Inc.s 
** "accpersp" example in the Mesa distribution
**
****************************************************************************/

#include "glaccum.h"
#include "jitter.h"
#include "math.h"

#define ACSIZE  8
#define PI_ 3.14159265358979323846

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

/*!
  Create a GLAccum widget
*/

GLAccum::GLAccum( QWidget* parent, const char* name, WFlags f, QGLFormat form )
    : GLControlWidget( form, parent, name, 0, f )
{
    setAnimationDelay( -1 );
    xRot = yRot = zRot = 0.0;		// default object rotation
    scale = 1.25;			// default object scale
}


/*!
  Release allocated resources
*/

GLAccum::~GLAccum()
{
    makeCurrent();
}


/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLAccum::paintGL()
{
    GLint viewport[4];
    int jitter;
 
    glGetIntegerv (GL_VIEWPORT, viewport);
    
    glClear(GL_ACCUM_BUFFER_BIT);
    for (jitter = 0; jitter < ACSIZE; jitter++) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	accPerspective (50.0,
			(GLdouble) viewport[2]/(GLdouble) viewport[3],
			1.0, 15.0, j8[jitter].x, j8[jitter].y, 0.0, 0.0, 1.0);
	displayObjects ();
	glAccum(GL_ACCUM, 1.0/ACSIZE);
    }
    glAccum (GL_RETURN, 1.0);
    glFlush(); 
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLAccum::initializeGL()
{
    GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 0.0, 0.0, 10.0, 1.0 };
    GLfloat lm_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialf(GL_FRONT, GL_SHININESS, 50.0);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lm_ambient);
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel (GL_FLAT);
    
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearAccum(0.0, 0.0, 0.0, 0.0);
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLAccum::resizeGL( int w, int h )
{
    glViewport(0, 0, (GLsizei) w, (GLsizei) h);
}

void GLAccum::displayObjects(void)
{
    GLUquadric *q;
    q = gluNewQuadric();
    GLfloat torus_diffuse[] = { 0.7, 0.7, 0.0, 1.0 };
    GLfloat cube_diffuse[] = { 0.0, 0.7, 0.7, 1.0 };
    GLfloat sphere_diffuse[] = { 0.7, 0.0, 0.7, 1.0 };
    GLfloat octa_diffuse[] = { 0.7, 0.4, 0.4, 1.0 };
    
    glPushMatrix ();
    glTranslatef (0.0, 0.0, -5.0);
    glRotatef (30.0, 1.0, 0.0, 0.0);
    
    glPushMatrix ();
    glTranslatef (-0.80, 0.35, 0.0);
    glRotatef (100.0, 1.0, 0.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, torus_diffuse);
    gluSphere ( q, 0.85, 16, 16 );
    glPopMatrix ();
    
    glPushMatrix ();
    glTranslatef (-0.75, -0.50, 0.0);
    glRotatef (45.0, 0.0, 0.0, 1.0);
    glRotatef (45.0, 1.0, 0.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, cube_diffuse);
    gluSphere ( q, 0.75, 16, 16 );
    glPopMatrix ();
    
    glPushMatrix ();
    glTranslatef (0.75, 0.60, 0.0);
    glRotatef (30.0, 1.0, 0.0, 0.0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, sphere_diffuse);
    gluSphere (q, 1.0, 16, 16);
    glPopMatrix ();
    
    glPushMatrix ();
    glTranslatef (0.70, -0.90, 0.25);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, octa_diffuse);
    gluSphere (q, 1.5, 16, 16);
    glPopMatrix ();
    
    glPopMatrix ();
}

/* accFrustum()
 * The first 6 arguments are identical to the glFrustum() call.
 *
 * pixdx and pixdy are anti-alias jitter in pixels.
 * Set both equal to 0.0 for no anti-alias jitter.
 * eyedx and eyedy are depth-of field jitter in pixels.
 * Set both equal to 0.0 for no depth of field effects.
 *
 * focus is distance from eye to plane in focus.
 * focus must be greater than, but not equal to 0.0.
 *
 * Note that accFrustum() calls glTranslatef().  You will
 * probably want to insure that your ModelView matrix has been
 * initialized to identity before calling accFrustum().
 */

void GLAccum::accFrustum(GLdouble left, GLdouble right, GLdouble bottom,
   GLdouble top, GLdouble nnear, GLdouble ffar, GLdouble pixdx,
   GLdouble pixdy, GLdouble eyedx, GLdouble eyedy, GLdouble focus)
{
   GLdouble xwsize, ywsize;
   GLdouble dx, dy;
   GLint viewport[4];
 
   glGetIntegerv (GL_VIEWPORT, viewport);
 
   xwsize = right - left;
   ywsize = top - bottom;
 
   dx = -(pixdx*xwsize/(GLdouble) viewport[2] + eyedx*nnear/focus);
   dy = -(pixdy*ywsize/(GLdouble) viewport[3] + eyedy*nnear/focus);
 
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum (left + dx, right + dx, bottom + dy, top + dy, nnear, ffar);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef (-eyedx, -eyedy, 0.0);
}

/* accPerspective()
 *
 * The first 4 arguments are identical to the gluPerspective() call.
 * pixdx and pixdy are anti-alias jitter in pixels.
 * Set both equal to 0.0 for no anti-alias jitter.
 * eyedx and eyedy are depth-of field jitter in pixels.
 * Set both equal to 0.0 for no depth of field effects.
 *
 * focus is distance from eye to plane in focus.
 * focus must be greater than, but not equal to 0.0.
 *
 * Note that accPerspective() calls accFrustum().
 */
void GLAccum::accPerspective(GLdouble fovy, GLdouble aspect,
   GLdouble nnear, GLdouble ffar, GLdouble pixdx, GLdouble pixdy,
   GLdouble eyedx, GLdouble eyedy, GLdouble focus)
{
   GLdouble fov2,left,right,bottom,top;
 
   fov2 = ((fovy*PI_) / 180.0) / 2.0;
 
   top = nnear / (cos(fov2) / sin(fov2));
   bottom = -top;
 
   right = top * aspect;
   left = -right;
 
   accFrustum (left, right, bottom, top, nnear, ffar,
               pixdx, pixdy, eyedx, eyedy, focus);
}
