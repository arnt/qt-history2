/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/texture/gltexobj.cpp#2 $
**
** Implementation of GLTexobj
** This is a simple QGLWidget demonstrating the use of QImages for textures.
**
** Much of the GL code is inspired by the 'spectex' and 'texcyl' 
** public domain demo programs by Brian Paul
**
****************************************************************************/

#include "gltexobj.h"
#include <qimage.h>
#include <qtimer.h>
#include <GL/glu.h>


const int redrawWait = 50;

/*!
  Create a GLTexobj widget
*/

GLTexobj::GLTexobj( QWidget* parent, const char* name )
    : QGLWidget( parent, name )
{
    xRot = yRot = zRot = 0.0;		// default object rotation
    scale = 5.0;			// default object scale
    object = 0;
    animation = TRUE;
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(update()) );
    timer->start( redrawWait, TRUE );
}


/*!
  Release allocated resources
*/

GLTexobj::~GLTexobj()
{
    glDeleteLists( object, 1 );
}


/*!
  Paint the texobj. The actual openGL commands for drawing the texobj are
  performed here.
*/

void GLTexobj::paintGL()
{
    if ( animation ) {
	xRot += 1.0;
	yRot += 2.5;
	zRot -= 5.0;
    }
    glClear( GL_COLOR_BUFFER_BIT );
    glPushMatrix();
    glRotatef( xRot, 1.0, 0.0, 0.0 ); 
    glRotatef( yRot, 0.0, 1.0, 0.0 ); 
    glRotatef( zRot, 0.0, 0.0, 1.0 );
    glScalef( scale, scale, scale );
    glCallList( object );
    glPopMatrix();

    if ( animation ) {
	glFlush(); // Make sure everything is drawn before restarting timer
	timer->start( redrawWait, TRUE ); // Wait this many msecs before redraw
    }
}


/*!
  Set up the OpenGL rendering state, and define display list
*/

void GLTexobj::initializeGL()
{
    // Set up the lights

    GLfloat white[4] = {2.0, 2.0, 2.0, 1.0};
    GLfloat whiteAmb[4] = {1.0, 1.0, 1.0, 1.0};
    GLfloat lightPos[4] = {30.0, 30.0, 30.0, 1.0};

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, whiteAmb);

    glMaterialfv(GL_FRONT, GL_DIFFUSE, white);
    glMaterialfv(GL_FRONT, GL_SPECULAR, white);
    glMaterialf(GL_FRONT, GL_SHININESS, 20.0);

    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);	// enable diffuse
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);	// enable specular
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    // Set up the textures

    QImage img1( "qtlogo.bmp" );	// Load first image from file
    if ( img1.isNull() ) {		// File not found handling
	warning( "Could not read image file, using single-color instead." );
	QImage dummy( 128, 128, 32 );
	dummy.fill( Qt::red.rgb() );
	img1 = dummy;
    }
    else {
	img1 = img1.convertDepth( 32 );	// Make sure it has the right depth
	img1 = img1.mirror();		// OpenGL wants it upside down
	img1 = img1.swapRGB();		// OpenGL wants BGR
    }

    QImage img2( "gllogo.bmp" );	// Load second image from file
    if ( img2.isNull() ) {		// File not found handling
	warning( "Could not read image file, using single-color instead." );
	QImage dummy( 128, 128, 32 );
	dummy.fill( Qt::green.rgb() );
	img2 = dummy;
    }
    else {
	img2 = img2.convertDepth( 32 ).mirror().swapRGB();	// as above
    }

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glEnable(GL_TEXTURE_2D);

    // Set up various other stuff

    glClearColor( 0.0, 0.0, 0.0, 0.0 ); // Let OpenGL clear to black
    glEnable(GL_CULL_FACE);  	// don't need Z testing for convex objects
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    // Make the object display list

    object = makeObject( img2, img1 );	// Generate an OpenGL display list
}



/*!
  Set up the OpenGL view port, matrix mode, etc.
*/

void GLTexobj::resizeGL( int w, int h )
{
    glViewport( 0, 0, w, h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 10.0, 100.0 );
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -70.0 );
}


/*!
  Generate an OpenGL display list for the object to be shown, i.e. the texobj
*/

GLuint GLTexobj::makeObject( const QImage& tex1, const QImage& tex2 )
{
    GLUquadricObj* q = gluNewQuadric();
    GLuint cylinderObj = glGenLists(1);
    glNewList(cylinderObj, GL_COMPILE);

    glTranslatef(0.0, 0.0, -1.0);

    // cylinder

    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex1.width(), tex1.height(), 0,
		  GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits() );
    gluQuadricNormals(q, GL_SMOOTH);
    gluQuadricTexture(q, GL_TRUE);
    gluCylinder(q, 0.6, 0.6, 2.0, 24, 1);

    // end cap

    glTexImage2D( GL_TEXTURE_2D, 0, 3, tex2.width(), tex2.height(), 0,
		  GL_RGBA, GL_UNSIGNED_BYTE, tex2.bits() );
    glTranslatef(0.0, 0.0, 2.0);
    gluDisk(q, 0.0, 0.6, 24, 1);

    // other end cap
    glTranslatef(0.0, 0.0, -2.0);
    gluQuadricOrientation(q, (GLenum)GLU_INSIDE);
    gluDisk(q, 0.0, 0.6, 24, 1);

    glEndList();
    gluDeleteQuadric(q);

    return cylinderObj;
}


/*!
  Set the rotation angle of the object to \e degrees around the X axis.
*/

void GLTexobj::setXRotation( int degrees )
{
    xRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Y axis.
*/

void GLTexobj::setYRotation( int degrees )
{
    yRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Z axis.
*/

void GLTexobj::setZRotation( int degrees )
{
    zRot = (GLfloat)(degrees % 360);
    updateGL();
}


/*!
  Turns animation on or off
*/

void GLTexobj::toggleAnimation()
{
    animation = !animation;
    if ( animation )
	updateGL();
    else
	timer->stop();
}
