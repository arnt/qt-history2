/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/cube/glcube.cpp#1 $
**
** Implementation of GLCube
** This is a simple QGLWidget displaying an openGL wireframe cube
**
** The OpenGL code is borrowed from Brian Pauls "spin" example
** in the Mesa distribution
**
****************************************************************************/

#include "glcube.h"


/*!
  Create a GLCube widget
*/

GLCube::GLCube( QWidget* parent, const char* name )
    : QGLWidget( parent, name )
{
    xRot = yRot = zRot = 0.0;		// default object rotation
    scale = 1.0;			// default object scale

    makeCurrent();
    object = makeObject();		// Generate an OpenGL display list
    glClearColor( 0.0, 0.0, 0.0, 0.0 ); // Let OpenGL clear to black
    glDisable( GL_DITHER );
    glShadeModel( GL_FLAT );
    glColor3f( 1.0, 1.0, 1.0 );
}


/*!
  Paint the cube. The actual openGL commands for drawing the cube are
  performed here.
*/

void GLCube::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT );

    glPushMatrix();

    glTranslatef( 0.0, 0.0, -10.0 );
    glScalef( scale, scale, scale );

    glRotatef( xRot, 1.0, 0.0, 0.0 ); 
    glRotatef( yRot, 0.0, 1.0, 0.0 ); 
    glRotatef( zRot, 0.0, 0.0, 1.0 );

    glCallList( object );

    glPopMatrix();
}


/*!
  Set up the OpenGL view port, frustrum etc. to match the window size
*/

void GLCube::resizeGL( int w, int h )
{
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 15.0 );
    glMatrixMode( GL_MODELVIEW );
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the cube
*/

GLuint GLCube::makeObject()
{	
    GLuint list;

    list = glGenLists( 1 );

    glNewList( list, GL_COMPILE );

    glBegin( GL_LINE_LOOP );
    glVertex3f(  1.0,  0.5, -0.4 );
    glVertex3f(  1.0, -0.5, -0.4 );
    glVertex3f( -1.0, -0.5, -0.4 );
    glVertex3f( -1.0,  0.5, -0.4 );
    glEnd();

    glBegin( GL_LINE_LOOP );
    glVertex3f(  1.0,  0.5, 0.4 );
    glVertex3f(  1.0, -0.5, 0.4 );
    glVertex3f( -1.0, -0.5, 0.4 );
    glVertex3f( -1.0,  0.5, 0.4 );
    glEnd();

    glBegin( GL_LINES );
    glVertex3f(  1.0,  0.5, -0.4 );   glVertex3f(  1.0,  0.5, 0.4 );
    glVertex3f(  1.0, -0.5, -0.4 );   glVertex3f(  1.0, -0.5, 0.4 );
    glVertex3f( -1.0, -0.5, -0.4 );   glVertex3f( -1.0, -0.5, 0.4 );
    glVertex3f( -1.0,  0.5, -0.4 );   glVertex3f( -1.0,  0.5, 0.4 );
    glEnd();

    glEndList();

    return list;
}


/*!
  Set the rotation angle of the object to \a degrees around the X axis.
*/

void GLCube::setXRotation( int degrees )
{
    xRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \a degrees around the Y axis.
*/

void GLCube::setYRotation( int degrees )
{
    yRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \a degrees around the Z axis.
*/

void GLCube::setZRotation( int degrees )
{
    zRot = (GLfloat)(degrees % 360);
    updateGL();
}


