/*
  box3d.cpp
*/

#include "box3d.h"

Box3D::Box3D( QWidget *parent, const char *name )
    : QGLWidget( parent, name )
{
    object = 0;
    rotX = rotY = rotZ = 0.0;
}

Box3D::~Box3D()
{
    makeCurrent();
    glDeleteLists( object, 1 );
}

void Box3D::initializeGL()
{
    qglClearColor( darkBlue );
    object = makeObject();
    glShadeModel( GL_FLAT );
}

void Box3D::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT );
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -10.0 );
    glRotatef( rotX, 1.0, 0.0, 0.0 );
    glRotatef( rotY, 0.0, 1.0, 0.0 );
    glRotatef( rotZ, 0.0, 0.0, 1.0 );
    glCallList( object );
}

void Box3D::resizeGL( int w, int h )
{
    glViewport( 0, 0, w, h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 15.0 );
    glMatrixMode( GL_MODELVIEW );
}

GLuint Box3D::makeObject()
{
    GLuint list = glGenLists( 1 );
    glNewList( list, GL_COMPILE );
    qglColor( yellow );
    glLineWidth( 2.0 );

    glBegin( GL_LINE_LOOP );
    glVertex3f( +1.5, +1.0, +0.8 );
    glVertex3f( +1.5, +1.0, -0.8 );
    /* ... */
    glVertex3f( +1.5, -1.0, -0.8 );
    glVertex3f( +1.5, -1.0, +0.8 );
    glVertex3f( +1.5, +1.0, +0.8 );
    glVertex3f( -1.5, +1.0, +0.8 );
    glVertex3f( -1.5, +1.0, -0.8 );
    glVertex3f( -1.5, -1.0, -0.8 );
    glVertex3f( -1.5, -1.0, +0.8 );
    glVertex3f( -1.5, +1.0, +0.8 );
    glEnd();

    glBegin( GL_LINES );
    glVertex3f( +1.5, +1.0, -0.8 );
    glVertex3f( -1.5, +1.0, -0.8 );
    glVertex3f( +1.5, -1.0, -0.8 );
    glVertex3f( -1.5, -1.0, -0.8 );
    glVertex3f( +1.5, -1.0, +0.8 );
    glVertex3f( -1.5, -1.0, +0.8 );
    glEnd();

    glEndList();
    return list;
}
