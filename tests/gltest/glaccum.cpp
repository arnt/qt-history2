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
** This is a simple QGLWidget displaying an openGL accum example
**
** The OpenGL code is mostly borrowed from Mark J. Kilgards  "dof"
** example in the Mesa distribution
**
****************************************************************************/

#include "glaccum.h"
#include "jitter.h"
#include "math.h"

#define ACSIZE  1
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
    //glAccum (GL_RETURN, 1.0);
    glFlush(); 
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLAccum::initializeGL()
{
    GLfloat ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat position[] = { 0.0, 3.0, 3.0, 0.0 };
 
    GLfloat lmodel_ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat local_view[] = { 0.0 };
 
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
 
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
 
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
    glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);
 
    glFrontFace (GL_CW);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_AUTO_NORMAL);
    glEnable(GL_NORMALIZE);
 
    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity ();
 
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClearAccum(0.0, 0.0, 0.0, 0.0);
    q = gluNewQuadric();
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
    int jitter;
    GLint viewport[4];
 
    glGetIntegerv (GL_VIEWPORT, viewport);
    glClear(GL_ACCUM_BUFFER_BIT);
 
    for (jitter = 0; jitter < 8; jitter++) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        accPerspective (45.0,
                (GLdouble) viewport[2]/(GLdouble) viewport[3],
                1.0, 15.0, 0.0, 0.0,
                0.33*j8[jitter].x, 0.33*j8[jitter].y, 5.0);
/*      ruby, gold, silver, emerald, and cyan balls   */
        renderBall (-1.1, -0.5, -4.5, 0.1745, 0.01175, 0.01175,
            0.61424, 0.04136, 0.04136, 0.727811, 0.626959, 0.626959, 0.6);
        renderBall (-0.5, -0.5, -5.0, 0.24725, 0.1995, 0.0745,
            0.75164, 0.60648, 0.22648, 0.628281, 0.555802, 0.366065, 0.4);
        renderBall (0.2, -0.5, -5.5, 0.19225, 0.19225, 0.19225,
            0.50754, 0.50754, 0.50754, 0.508273, 0.508273, 0.508273, 0.4);
        renderBall (1.0, -0.5, -6.0, 0.0215, 0.1745, 0.0215,
            0.07568, 0.61424, 0.07568, 0.633, 0.727811, 0.633, 0.6);
        renderBall (1.8, -0.5, -6.5, 0.0, 0.1, 0.06, 0.0, 0.50980392,
            0.50980392, 0.50196078, 0.50196078, 0.50196078, .25);
        glAccum (GL_ACCUM, 0.125);
    }
 
    glAccum (GL_RETURN, 1.0);
    glFlush();
}

void GLAccum::renderBall (GLfloat x, GLfloat y, GLfloat z,
    GLfloat ambr, GLfloat ambg, GLfloat ambb,
    GLfloat difr, GLfloat difg, GLfloat difb,
    GLfloat specr, GLfloat specg, GLfloat specb, GLfloat shine)
{
    float mat[4];
 
    glPushMatrix();
    glTranslatef (x, y, z);
    mat[0] = ambr; mat[1] = ambg; mat[2] = ambb; mat[3] = 1.0;
    glMaterialfv (GL_FRONT, GL_AMBIENT, mat);
    mat[0] = difr; mat[1] = difg; mat[2] = difb;
    glMaterialfv (GL_FRONT, GL_DIFFUSE, mat);
    mat[0] = specr; mat[1] = specg; mat[2] = specb;
    glMaterialfv (GL_FRONT, GL_SPECULAR, mat);
    glMaterialf (GL_FRONT, GL_SHININESS, shine*128.0);
    gluSphere(q, 0.5, 16, 16);
    glPopMatrix();
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
