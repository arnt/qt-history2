#include <qapplication.h>
#include <qthread.h>
#include <qgl.h>
#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

class GLThread : public QThread
{
public:
    GLThread( QGLWidget * glw ) 
	: QThread(), gl( glw )
    {
	runMe = TRUE;
	resizeMe = FALSE;
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
 	    msleep( 40 );
	}
    }
    
private:
    bool runMe, resizeMe;
    int w, h;
    QGLWidget * gl;
};

class Triangles : public QGLWidget
{
public:
    Triangles() : QGLWidget( 0 ), glt( this )
    { 
	setAutoBufferSwap( FALSE );
	resize( 320, 240 );
    }
    
    void startRendering()
    {
	glt.start();
    }
    
    void stopRendering()
    {
	glt.stop();
	glt.wait();
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
#ifdef Q_WS_X11
    XInitThreads();
#endif
    QApplication app( argc, argv );
    Triangles t1, t2, t3;
    app.setMainWidget( &t1 );
    t1.show();
    t2.show();
    t3.show();
    t1.startRendering();
    t2.startRendering();
    t3.startRendering();
    int i = app.exec();
    t1.stopRendering();
    t2.stopRendering();
    t3.stopRendering();
    return i;
}
