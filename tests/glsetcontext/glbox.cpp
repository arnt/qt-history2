#include "glbox.h"

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

class Triangles : public QGLWidget
{
public:
    Triangles( QWidget * parent )
	: QGLWidget( parent ), angle( 0 )
    { 
	startTimer( 40 );
    }
    
protected:
    int angle;
    void timerEvent( QTimerEvent * ) {
	update(); // updates the widget every 40 ms
    }	
    void initializeGL() { 
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background
 	glShadeModel( GL_SMOOTH ); // interpolate colors btw. vertices
   	glEnable( GL_DEPTH_TEST ); // removes hidden surfaces
 	glMatrixMode( GL_PROJECTION );
 	glLoadIdentity();	
 	glOrtho( -5.0, 5.0, -5.0, 5.0, 1.0, 100.0 );
 	glMatrixMode( GL_MODELVIEW );
    }
    void resizeGL( int w, int h ) {
	glViewport( 0, 0, w, h ); // resize the GL drawing area
    }
    void paintGL() {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
 	glLoadIdentity();
 	glTranslatef( 0.0, 0.0, -6.0 );
 	glRotatef( angle, 1.0, 1.0, 0.0 ); // rotate around the x and y axis
	angle += 3;
	glBegin( GL_TRIANGLES ); { // draw a tetrahedron
  	    glColor3f( 1.0, 0.0, 0.0 ); glVertex3f( -2.0, -2.0, 0.0 );
  	    glColor3f( 0.0, 1.0, 0.0 ); glVertex3f( -2.0, 2.0, 0.0 );
  	    glColor3f( 0.0, 0.0, 1.0 ); glVertex3f( 2.0, -2.0, 0.0 );

  	    glColor3f( 0.0, 1.0, 1.0 ); glVertex3f( -2.0, -2.0, -4.0 );
  	    glColor3f( 0.0, 1.0, 0.0 ); glVertex3f( -2.0, 2.0, 0.0 );
  	    glColor3f( 0.0, 0.0, 1.0 ); glVertex3f( 2.0, -2.0, 0.0 );

  	    glColor3f( 1.0, 0.0, 0.0 ); glVertex3f( -2.0, -2.0, 0.0 );
  	    glColor3f( 0.0, 1.0, 0.0 ); glVertex3f( -2.0, 2.0, 0.0 );
  	    glColor3f( 0.0, 1.0, 1.0 ); glVertex3f( -2.0, -2.0, -4.0 );

   	    glColor3f( 0.0, 1.0, 1.0 ); glVertex3f( -2.0, -2.0, -4.0 );
   	    glColor3f( 1.0, 0.0, 0.0 ); glVertex3f( -2.0, -2.0, 0.0 );
   	    glColor3f( 0.0, 0.0, 1.0 ); glVertex3f( 2.0, -2.0, 0.0 );
 	}
 	glEnd();
    }
};

GLBox::GLBox( QWidget* parent, const char* name )
    : QGLWidget( parent, name )
{
    xRot = yRot = zRot = 0.0;
    scale = 1.25;
    object = 0;
    tri = new Triangles( parent );
    tri->resize( 100, 100 );
    tri->move( 2, 2 );
}

GLBox::~GLBox()
{
    makeCurrent();
    glDeleteLists( object, 1 );
}

void GLBox::paintGL()
{
    glClear( GL_COLOR_BUFFER_BIT );

    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -10.0 );
    glScalef( scale, scale, scale );

    glRotatef( xRot, 1.0, 0.0, 0.0 ); 
    glRotatef( yRot, 0.0, 1.0, 0.0 ); 
    glRotatef( zRot, 0.0, 0.0, 1.0 );

    glCallList( object );
}

void GLBox::initializeGL()
{
    qglClearColor( black );
    object = makeObject();
    glShadeModel( GL_FLAT );
    tri->raise();
}

void GLBox::resizeGL( int w, int h )
{
    glViewport( 0, 0, (GLint)w, (GLint)h );
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glFrustum( -1.0, 1.0, -1.0, 1.0, 5.0, 15.0 );
    glMatrixMode( GL_MODELVIEW );
}

GLuint GLBox::makeObject()
{	
    GLuint list;

    list = glGenLists( 1 );

    glNewList( list, GL_COMPILE );

    qglColor( white );

    glLineWidth( 2.0 );

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

void GLBox::setXRotation( int degrees )
{
    xRot = (GLfloat)(degrees % 360);
    updateGL();
}

void GLBox::setYRotation( int degrees )
{
    yRot = (GLfloat)(degrees % 360);
    updateGL();
}


void GLBox::setZRotation( int degrees )
{
    zRot = (GLfloat)(degrees % 360);
    updateGL();
}

void GLBox::raiseSibling()
{
    tri->raise();
}
