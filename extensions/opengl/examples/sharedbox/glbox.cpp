/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/sharedbox/glbox.cpp#2 $
**
** Implementation of GLBox
** This is a simple QGLWidget displaying a box
**
** The OpenGL code is mostly borrowed from Brian Pauls "spin" example
** in the Mesa distribution
**
****************************************************************************/

#include "glbox.h"

// Initialize static class variables:

// Shared display list id:
GLuint GLBox::sharedDisplayList = 0;

// Counter keeping track of number of GLBox instances sharing 
// the display list, so that the last instance can delete it:
int GLBox::sharedListUsers = 0;


/*!
  Create a GLBox widget
*/

GLBox::GLBox( QWidget* parent, const char* name, const QGLWidget* shareWidget )
    : QGLWidget( parent, name, shareWidget )
{
    xRot = yRot = zRot = 0.0;		// default object rotation
    scale = 1.0;			// default object scale
    object = 0;
    localDisplayList = 0;
}


/*!
  Set up the OpenGL rendering state. Robustly access shared display list.
*/

void GLBox::initializeGL()
{
    glClearColor( 0.0, 0.0, 0.0, 0.0 ); // Let OpenGL clear to black
    glEnable(GL_DEPTH_TEST);

    if ( sharedListUsers == 0 ) {	// No shared list has been made yet
	sharedDisplayList = makeObject();	// Make one
	object = sharedDisplayList;		// Use it
	sharedListUsers++;				// Keep reference count
	debug( "GLBox %s created shared display list.", name() );
    }
    else {				// There is a shared diplay list
	if ( isSharing() ) {		// Can we access it?
	    object = sharedDisplayList;		// Yes, use it
	    sharedListUsers++;			// Keep reference count
	    debug( "GLBox %s uses shared display list.", name() );
	}
	else {				
	    localDisplayList = makeObject();	// No, roll our own
	    object = localDisplayList;		// and use that
	    debug( "GLBox %s uses private display list.", name() );
	}
    }
}



/*!
  Release allocated resources
*/

GLBox::~GLBox()
{
    if ( localDisplayList != 0 ) {		// Did we make our own?
	glDeleteLists( localDisplayList, 1 );	// Yes, delete it
	debug( "GLBox %s deleted private display list.", name() );
    }
    else {
	sharedListUsers--;	// No, we used the shared one; keep refcount
	if ( sharedListUsers == 0 ) { 			// Any sharers left?
	    glDeleteLists( sharedDisplayList, 1 );	// No, delete it
	    sharedDisplayList = 0;
	    debug( "GLBox %s deleted shared display list.", name() );
	}
    }
}


/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/

void GLBox::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -3.0 );
    glScalef( scale, scale, scale );

    glRotatef( xRot, 1.0, 0.0, 0.0 ); 
    glRotatef( yRot, 0.0, 1.0, 0.0 ); 
    glRotatef( zRot, 0.0, 0.0, 1.0 );

    glCallList( object );
}




/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLBox::resizeGL( int w, int h )
{
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 10.0);
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the box
*/

GLuint GLBox::makeObject()
{	
    GLuint list;

    list = glGenLists( 1 );

    glNewList( list, GL_COMPILE );

    glBegin(GL_QUADS);
    /* Front face */
    glColor3f((GLfloat)0.0, (GLfloat)0.7, (GLfloat)0.1);  /* Green */
    glVertex3f(-1.0, 1.0, 1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(1.0, -1.0, 1.0);
    glVertex3f(-1.0, -1.0, 1.0);
    /* Back face */
    glColor3f((GLfloat)0.9, (GLfloat)1.0, (GLfloat)0.0);  /* Yellow */
    glVertex3f(-1.0, 1.0, -1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(-1.0, -1.0, -1.0);
    /* Top side face */
    glColor3f((GLfloat)0.2, (GLfloat)0.2, (GLfloat)1.0);  /* Blue */
    glVertex3f(-1.0, 1.0, 1.0);
    glVertex3f(1.0, 1.0, 1.0);
    glVertex3f(1.0, 1.0, -1.0);
    glVertex3f(-1.0, 1.0, -1.0);
    /* Bottom side face */
    glColor3f((GLfloat)0.7, (GLfloat)0.0, (GLfloat)0.1);  /* Red */
    glVertex3f(-1.0, -1.0, 1.0);
    glVertex3f(1.0, -1.0, 1.0);
    glVertex3f(1.0, -1.0, -1.0);
    glVertex3f(-1.0, -1.0, -1.0);
    glEnd();
    glEndList();

    return list;
}


/*!
  Set the rotation angle of the object to \e degrees around the X axis.
*/

void GLBox::setXRotation( int degrees )
{
    xRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Y axis.
*/

void GLBox::setYRotation( int degrees )
{
    yRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Z axis.
*/

void GLBox::setZRotation( int degrees )
{
    zRot = (GLfloat)(degrees % 360);
    updateGL();
}
