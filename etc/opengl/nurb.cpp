/****************************************************************************
**
** Implementation of Nurb widget class
**
** Some of the code has been borrowed from SGI's tnurb example, which is
** (c) Copyright 1993, Silicon Graphics, Inc.
**
*****************************************************************************/

#include "nurb.h"
#include <qtimer.h>
#include <math.h>
#include <stdlib.h>


#define INREAL float

#define S_NUMPOINTS 13
#define S_ORDER	    3
#define S_NUMKNOTS  (S_NUMPOINTS + S_ORDER)
#define T_NUMPOINTS 3
#define T_ORDER	    3
#define T_NUMKNOTS  (T_NUMPOINTS + T_ORDER)
#define SQRT_TWO    1.41421356237309504880

typedef INREAL Point[4];

INREAL sknots[S_NUMKNOTS] = {
    -1.0, -1.0, -1.0, 0.0, 1.0, 2.0, 3.0, 4.0,
    4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 9.0, 9.0
};
INREAL tknots[T_NUMKNOTS] = {
    1.0, 1.0, 1.0, 2.0, 2.0, 2.0
};
Point ctlpoints[S_NUMPOINTS][T_NUMPOINTS] = { { { 4.0, 2.0, 2.0, 1.0 }, {
4.0, 1.6, 2.5, 1.0 }, { 4.0, 2.0, 3.0, 1.0 } }, { { 5.0, 4.0, 2.0, 1.0 },
{ 5.0, 4.0, 2.5, 1.0 }, { 5.0, 4.0, 3.0, 1.0 } }, { { 6.0, 5.0, 2.0, 1.0
}, { 6.0, 5.0, 2.5, 1.0 }, { 6.0, 5.0, 3.0, 1.0 } }, { { SQRT_TWO*6.0,
SQRT_TWO*6.0, SQRT_TWO*2.0, SQRT_TWO }, { SQRT_TWO*6.0, SQRT_TWO*6.0,
SQRT_TWO*2.5, SQRT_TWO }, { SQRT_TWO*6.0, SQRT_TWO*6.0, SQRT_TWO*3.0,
SQRT_TWO } }, { { 5.2, 6.7, 2.0, 1.0 }, { 5.2, 6.7, 2.5, 1.0 }, { 5.2,
6.7, 3.0, 1.0 } }, { { SQRT_TWO*4.0, SQRT_TWO*6.0, SQRT_TWO*2.0, SQRT_TWO
}, { SQRT_TWO*4.0, SQRT_TWO*6.0, SQRT_TWO*2.5, SQRT_TWO }, { SQRT_TWO*4.0,
SQRT_TWO*6.0, SQRT_TWO*3.0, SQRT_TWO } }, { { 4.0, 5.2, 2.0, 1.0 }, { 4.0,
4.6, 2.5, 1.0 }, { 4.0, 5.2, 3.0, 1.0 } }, { { SQRT_TWO*4.0, SQRT_TWO*6.0,
SQRT_TWO*2.0, SQRT_TWO }, { SQRT_TWO*4.0, SQRT_TWO*6.0, SQRT_TWO*2.5,
SQRT_TWO }, { SQRT_TWO*4.0, SQRT_TWO*6.0, SQRT_TWO*3.0, SQRT_TWO } }, { {
2.8, 6.7, 2.0, 1.0 }, { 2.8, 6.7, 2.5, 1.0 }, { 2.8, 6.7, 3.0, 1.0 } }, {
{ SQRT_TWO*2.0, SQRT_TWO*6.0, SQRT_TWO*2.0, SQRT_TWO }, { SQRT_TWO*2.0,
SQRT_TWO*6.0, SQRT_TWO*2.5, SQRT_TWO }, { SQRT_TWO*2.0, SQRT_TWO*6.0,
SQRT_TWO*3.0, SQRT_TWO } }, { { 2.0, 5.0, 2.0, 1.0 }, { 2.0, 5.0, 2.5, 1.0
}, { 2.0, 5.0, 3.0, 1.0 } }, { { 3.0, 4.0, 2.0, 1.0 }, { 3.0, 4.0, 2.5,
1.0 }, { 3.0, 4.0, 3.0, 1.0 } }, { { 4.0, 2.0, 2.0, 1.0 }, { 4.0, 1.6,
2.5, 1.0 }, { 4.0, 2.0, 3.0, 1.0 } } };


GLUnurbsObj *Nurb::theNurbs = 0;


Nurb::Nurb( QWidget *parent, const char *name )
    : GLWidget( parent, name )
{
    animTimer = 0;
    rotX = 50;
    rotY = 30;
    if ( theNurbs == 0 ) {		// common initialization for all Nurbs
	theNurbs = gluNewNurbsRenderer();
	gluNurbsProperty(theNurbs, GLU_SAMPLING_TOLERANCE, 15.0);
	gluNurbsProperty(theNurbs, GLU_DISPLAY_MODE, GLU_OUTLINE_PATCH);
	gluNurbsProperty(theNurbs, (GLenum)~0, 15.0);
	gluEndSurface(theNurbs);
	glColor3f(1.0, 1.0, 1.0);
    }
}


void Nurb::animate()
{
    if ( animTimer ) {			// stop animation
	delete animTimer;
	animTimer = 0;
    } else {
	animTimer = new QTimer( this );
	connect( animTimer, SIGNAL(timeout()), SLOT(animTimeout()) );
	animTimer->start( 20 );		// 20 millisecond timeout
    }
}

void Nurb::animTimeout()
{
    rotY = (rotY + 5) % 360;
    rotX = (rotX + 5) % 360;
    updateGL();
}


void Nurb::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
    glTranslatef( 4.0, 4.5, 2.5 );
    glRotatef( rotY, 1, 0, 0 );
    glRotatef( rotX, 0, 1, 0 );
    glTranslatef( -4.0, -4.5, -2.5 );
    gluBeginSurface( theNurbs );
    gluNurbsSurface( theNurbs, S_NUMKNOTS, sknots, T_NUMKNOTS, tknots,
		     4*T_NUMPOINTS, 4, &ctlpoints[0][0][0], S_ORDER,
		     T_ORDER, GL_MAP2_VERTEX_4 );
    gluEndSurface( theNurbs );
    glPopMatrix();
    glFlush();
}

void Nurb::resizeGL( int w, int h )
{
    glViewport( 0, 0, w, h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -2.0, 2.0, -2.0, 2.0, 0.8, 10.0 );
    gluLookAt( 7.0, 4.5, 4.0, 4.5, 4.5, 2.5, 6.0, -3.0, 2.0 );
    glMatrixMode( GL_MODELVIEW );
}
