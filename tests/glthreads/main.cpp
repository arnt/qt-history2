#include <qapplication.h>
#include <qthread.h>
#define private protected
#include <qgl.h>
#include <GL/glx.h>

class GLThread : public QThread
{
public:
    GLThread() : QThread()
    {
	runMe = TRUE;
	resizeMe = FALSE;
	gl = 0;
    }
    void setTri( QGLContext * tr )
    {
	gl = tr;
    }
    void stop()
    {
	runMe = FALSE;
    }
    void setNewSize( int wt, int ht )
    {
	w = wt;
	h = ht;
	resizeMe = TRUE;
    }
    
    void run()
    {
	static int angle = 0;
  	gl->makeCurrent();
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background
 	glShadeModel( GL_SMOOTH ); // interpolate colors btw. vertices
   	glEnable( GL_DEPTH_TEST ); // removes hidden surfaces
 	glMatrixMode( GL_PROJECTION );
 	glLoadIdentity();	
 	glOrtho( -5.0, 5.0, -5.0, 5.0, 1.0, 100.0 );
 	glMatrixMode( GL_MODELVIEW );
	glViewport( 0, 0, 200, 200 );	
	while ( runMe ) {
	    if ( resizeMe ) {
		glViewport( 0, 0, w, h );
		resizeMe = FALSE;
	    }
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
   	    gl->swapBuffers();
	}
	qWarning( "Thread exiting" );
    }
    
private:
    bool runMe, resizeMe;
    int w, h;
    QGLContext * gl;
};

class Triangles : public QGLWidget
{
public:
    Triangles() : QGLWidget( 0 )
    { 
	setAutoBufferSwap( FALSE );
    }
    
    void startRendering()
    {
	glt.setTri( glcx );
	glt.start();
    }
    
protected:
    void resizeEvent( QResizeEvent * e )
    {
	glt.setNewSize( e->size().width(), e->size().height() );
    }
    void paintEvent( QPaintEvent * ){}
    void closeEvent( QCloseEvent *e )
    {
	glt.stop();
	glt.wait();
	QGLWidget::closeEvent( e );
    }

    GLThread glt;
};


int main( int argc, char ** argv )
{
    Status threadStat;
    threadStat = XInitThreads();
    if (threadStat) {
	printf("XInitThreads() returned %d (success)\n", (int) threadStat);
    } else {
	printf("XInitThreads() returned 0 (failure- this program may fail)\n");
    }
    QApplication app( argc, argv );
    Triangles tri, tri2, tri3;
    app.setMainWidget( &tri );
    tri.show();
    tri2.show();
    tri3.show();
    tri.startRendering();
    tri2.startRendering();
    tri3.startRendering();
    return app.exec();     
}
