#include <qapplication.h>
#include <qwidget.h>
#include <qgl.h>
#include <qcolormap.h>
#include <qpushbutton.h>
#include <GL/gl.h>
#include <GL/glu.h>

typedef struct view_data {
    GLfloat model[4][4];      /* OpenGL model view matrix for the view */
    GLfloat projection[4][4]; /* OpenGL projection matrix for the view */
} view_data;

view_data views[4];

#define V3D 0
#define ORG 1

void init_3d( void );

void init_3d()
{
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef(0,0,-50.0);
    glRotatef(-45,1,0,0);
    glRotatef(-45,0,0,1);
    glGetFloatv(GL_MODELVIEW_MATRIX,(GLfloat *) views[V3D].model);
    glGetFloatv(GL_MODELVIEW_MATRIX,(GLfloat *) views[ORG].model);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    /* Use GL utility library function to obtain desired view */
    gluPerspective(60, 1, 1, 250);
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)views[V3D].projection);
    glGetFloatv(GL_PROJECTION_MATRIX, (GLfloat *)views[ORG].projection);
}

class MyGl : public QGLWidget
{
    void paintGL();
    void initializeGL();

public:
    MyGl( QWidget * parent = 0, const char * name = 0 );

protected:
    void resizeGL( int w, int h );
    void mousePressEvent( QMouseEvent * e )
    {
	oldPos = e->pos();
    }
    void mouseReleaseEvent( QMouseEvent * e )
    {
	oldPos = e->pos();
    }
    void setXRotation( double deg )
    {
	rx = deg;
    }
    void setYRotation( double deg )
    {
	ry = deg;
    }
    void setZRotation( double deg )
    {
	rz = deg;
    }
    void setRotationImpulse( double x, double y, double z )
    {
	setXRotation( rx + 180*x );
	setYRotation( ry + 180*y );
	setZRotation( rz - 180*z );
	updateGL();
    }
    void transform()
    {
	glRotatef( rx, 1.0, 0.0, 0.0 ); 
	glRotatef( ry, 0.0, 1.0, 0.0 ); 
	glRotatef( rz, 0.0, 0.0, 1.0 );
    }

    void drawMarker()
    {
	makeCurrent();
	glIndexi( 1 );
	glLogicOp( GL_XOR );
	glEnable( GL_LOGIC_OP );
	glPointSize(40.0);
	glBegin( GL_POINTS );
	{
	    glVertex3f( ptX, ptY, 0);
	}
	glEnd();
	glPointSize(1.0);
	glDisable( GL_LOGIC_OP );
	glFlush();
	swapBuffers();
    }
    
    void mouseMoveEvent( QMouseEvent * e )
    {
	double dx = (double) (oldPos.x() - e->pos().x()) / width();
	double dy = (double) (oldPos.y() - e->pos().y()) / height();
	
	if ( e->state() == LeftButton )
	    setRotationImpulse( -dy, -dx, 0 );
	else if ( e->state() == RightButton )
	    setRotationImpulse( -dy, 0, dx );
 	else if ( e->state() == MidButton ) {
	    drawMarker();
	    ptX++; ptY++;
	    if( ptX > 30 ) ptX = -30;
	    if( ptY > 30 ) ptY = -30;
	    drawMarker();
	} 
	oldPos = e->pos();
    }
    double rx,ry,rz,scale;
    double tx,ty,tz;
    QPoint oldPos;    
    int ptX, ptY;
};

MyGl::MyGl( QWidget * parent, const char * name )
    : QGLWidget( parent, name )
{
    ptX = ptY = 0;
    rx = ry = rz = 0.0;
}

void MyGl::initializeGL()
{
    init_3d();
}

void MyGl::resizeGL( int width, int height )
{
    glViewport( 0, 0, width, height );
}

void MyGl::paintGL()
{
    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( (GLfloat *) views[ORG].model );
    transform();
    glGetFloatv( GL_MODELVIEW_MATRIX, (GLfloat *) views[V3D].model );
    glClear( GL_COLOR_BUFFER_BIT );

    glClearIndex( 0 );

    for ( int i = 0; i < 256; i++ ) {
	glIndexi( i );
	glBegin( GL_POLYGON ); {
	    glVertex3f( -24, 0 , i );
	    glVertex3f( -24, 24, i );
	    glVertex3f( -24, 24 , i+1 );
	    glVertex3f( -24, 0, i+1 );
	}
	glEnd();	    
    }
}

int main( int argc, char ** argv )
{
    QApplication app( argc, argv );
    QGLFormat f;
    f.setRgba( FALSE );
    QGLFormat::setDefaultFormat( f );
    MyGl gl( 0, "mygl" );
    MyGl gl2( 0, "mygl2" );
    app.setMainWidget( &gl );
    
    // Install a custom colormap
    QColormap cmap1;
    QRgb colors[256];
    
    for ( int i = 0; i < 256; i++ )
  	colors[i] = qRgb( i, i, i );
    
    cmap1.setRgb( 0, 256, colors );
    
    QColormap cmap2 ( cmap1 );
    for ( int x = 150; x < cmap2.size(); x++ )
  	cmap2.setRgb( x, qRgb( x, x, 0 ) );
     
    gl.setColormap( cmap1 );
    gl2.setColormap( cmap2 );
    
    gl.show();
    gl2.show();
    return app.exec();
}
