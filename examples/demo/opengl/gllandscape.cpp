#include "gllandscape.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>

#ifndef PI
#define PI 3.14159
#endif

#include "fbm.h"

typedef struct view_data {
  GLfloat model[4][4];      /* OpenGL model view matrix for the view */
  GLfloat projection[4][4]; /* OpenGL projection matrix for the view */
} view_data;

view_data views[4];

#define V3D 0
#define ORG 1

GLLandscape::GLLandscape( QWidget * parent, const char * name )
    : QGLWidget( parent, name )
{
    oldX = oldY = oldZ = 0.0;
    landscape     = 0;
    vertexNormals = 0;
    normals       = 0;
    wave          = 0;
    wt            = 0;
    setGridSize( 50 );
}

GLLandscape::~GLLandscape()
{
    for( int i = 0; i < gridSize; i++ ) {
	delete[] landscape[i];
	delete[] normals[i];
	delete[] vertexNormals[i];
	delete[] wave[i];
	delete[] wt[i];
    }
    delete[] landscape;
    delete[] normals;
    delete[] vertexNormals;
    delete[] wave;
    delete[] wt;
}

void GLLandscape::initializeGL()
{
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();
    glTranslatef( 0.0, 0.0, -50.0 );
    glRotatef( -45, 1, 0, 0 );
    glRotatef( -45, 0, 0, 1 );
    glGetFloatv( GL_MODELVIEW_MATRIX,(GLfloat *) views[V3D].model );
    glGetFloatv( GL_MODELVIEW_MATRIX,(GLfloat *) views[ORG].model );

    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    /* Use GL utility library function to obtain desired view */
    gluPerspective( 60, 1, 1, 250 );
    glGetFloatv( GL_PROJECTION_MATRIX, (GLfloat *)views[V3D].projection );
    glGetFloatv( GL_PROJECTION_MATRIX, (GLfloat *)views[ORG].projection );

    // Enable line antialiasing
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glHint( GL_LINE_SMOOTH_HINT, GL_DONT_CARE );

    qglClearColor( black );
    glDepthFunc( GL_LESS );
    glEnable( GL_DEPTH_TEST );
    calculateVertexNormals();
}

void GLLandscape::resizeGL( int width, int height )
{
    glViewport( 0, 0, width, height );
}

void GLLandscape::paintGL()
{
//     static int p = 0;
//     if ( p == 0 ) {
// 	p = 1;
// 	createDisplayLists();
//     }
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

//     glCallList( mode );
    switch ( mode ) {
	case Wireframe:
	    drawWireframe( FALSE );
	    break;
	case Filled:
	    drawFilled();
	    break;
	case Shaded:
	    drawSmoothShaded();
	    break;
    }
}

void GLLandscape::drawWireframe( bool removeHiddenLines )
{
    if ( !removeHiddenLines ) {
	glDisable( GL_DEPTH_TEST );
	qglColor( white );
	glBegin( GL_LINES );
	{
	    for ( int y = 0; y < (gridSize-1); y++ )
		for ( int x = 0; x < (gridSize-1); x++) {
		    glVertex3f( x-gridHalf, y-gridHalf, landscape[x][y] );
		    glVertex3f( x+1-gridHalf, y-gridHalf, landscape[x+1][y] );
		    glVertex3f( x-gridHalf, y-gridHalf, landscape[x][y] );
		    glVertex3f( x+1-gridHalf, y+1-gridHalf, landscape[x+1][y+1] );

		    glVertex3f( x-gridHalf, y-gridHalf, landscape[x][y] );
		    glVertex3f( x-gridHalf, y+1-gridHalf, landscape[x][y+1] );
		}
	}
	glEnd();
	glBegin( GL_LINE_STRIP );
	{
	    for ( int x = 0; x < gridSize; x++ ) {
		glVertex3f( x-gridHalf, gridHalf-1, landscape[x][gridSize-1] );
	    }
	}
	glEnd();
	glBegin( GL_LINE_STRIP );
	{
	    for ( int y = 0; y < gridSize; y++ ) {
		glVertex3f( gridHalf-1, y-gridHalf, landscape[gridSize-1][y] );
	    }
	}
	glEnd();

	glEnable( GL_DEPTH_TEST );
    } else {
	//
	// Hidden line removal using the stencil buffer.
	// I know: it sucks!
	//
	glEnable( GL_STENCIL_TEST );
	glEnable( GL_DEPTH_TEST );

	glClear( GL_DEPTH_BUFFER_BIT );
	glClear( GL_STENCIL_BUFFER_BIT );
	glStencilFunc( GL_ALWAYS, 0, 1 );
	glStencilOp( GL_INVERT, GL_INVERT, GL_INVERT );
	glColor3f( 0.4, 0.6, 0.6 );
	for ( int i = 0; i < gridSize-1; i++ )
	    for ( int k = 0; k < gridSize-1; k++ ) {
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glBegin( GL_POLYGON );
		{
		    glVertex3f(i-gridHalf,k-gridHalf,landscape[i][k]);
		    glVertex3f(i+1-gridHalf, k-gridHalf, landscape[i+1][k]);
		    glVertex3f(i+1-gridHalf, k+1-gridHalf, landscape[i+1][k+1]);
		} /* end GL_POLYGON */
		glEnd();

//		qglColor( white );
		qglColor( black );
		glStencilFunc( GL_EQUAL, 0, 1 );
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glBegin( GL_POLYGON );
		{
		    glVertex3f(i-gridHalf,k-gridHalf,landscape[i][k]);
		    glVertex3f(i+1-gridHalf, k-gridHalf, landscape[i+1][k]);
		    glVertex3f(i+1-gridHalf, k+1-gridHalf, landscape[i+1][k+1]);
		} /* end GL_POLYGON */
		glEnd();

		qglColor( white );
//		glColor3f( 0.4, 0.6, 0.6 );
		glStencilFunc( GL_ALWAYS, 0, 1 );
		glStencilOp( GL_INVERT, GL_INVERT, GL_INVERT );
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glBegin( GL_POLYGON );
		{
		    glVertex3f(i-gridHalf,k-gridHalf,landscape[i][k]);
		    glVertex3f(i+1-gridHalf, k-gridHalf, landscape[i+1][k]);
		    glVertex3f(i+1-gridHalf, k+1-gridHalf, landscape[i+1][k+1]);
		} /* end GL_POLYGON */
		glEnd();
		/* Second triangle -- do exactly the same as above, but for
		   the second triangle in the current cell. */
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glBegin(GL_POLYGON);
		{
		    glVertex3f(i-gridHalf,k-gridHalf, landscape[i][k]);
		    glVertex3f(i-gridHalf,k+1-gridHalf, landscape[i][k+1]);
		    glVertex3f(i+1-gridHalf,k+1-gridHalf, landscape[i+1][k+1]);
		} /* end GL_POLYGON */
		glEnd();

		qglColor( black );
		glStencilFunc( GL_EQUAL, 0, 1 );
		glStencilOp( GL_KEEP, GL_KEEP, GL_KEEP );
		glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		glBegin(GL_POLYGON);
		{
		    glVertex3f(i-gridHalf,k-gridHalf, landscape[i][k]);
		    glVertex3f(i-gridHalf,k+1-gridHalf, landscape[i][k+1]);
		    glVertex3f(i+1-gridHalf,k+1-gridHalf, landscape[i+1][k+1]);
		} /* end GL_POLYGON */
		glEnd();

//		glColor3f( 0.4, 0.6, 0.6 );
		qglColor( white );
		glStencilFunc( GL_ALWAYS, 0, 1 );
		glStencilOp( GL_INVERT, GL_INVERT, GL_INVERT );
		glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		glBegin(GL_POLYGON);
		{
		    glVertex3f(i-gridHalf,k-gridHalf, landscape[i][k]);
		    glVertex3f(i-gridHalf,k+1-gridHalf, landscape[i][k+1]);
		    glVertex3f(i+1-gridHalf,k+1-gridHalf, landscape[i+1][k+1]);
		} /* end GL_POLYGON */
		glEnd();
	    } /* for */
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glDisable( GL_STENCIL_TEST );
	glDisable( GL_DEPTH_TEST );
   }

}

void GLLandscape::drawFilled()
{
    for ( int y = 0; y < gridSize-1; y++ )
	for ( int x = 0; x < gridSize-1; x++ ) {
	    qglColor( red );
	    glBegin( GL_TRIANGLE_STRIP );
	    {
		glVertex3f(x-gridHalf,y-gridHalf, landscape[x][y]);
		glVertex3f(x+1-gridHalf,y-gridHalf, landscape[x+1][y]);
		glVertex3f(x-gridHalf,y+1-gridHalf, landscape[x][y+1]);
	    }
	    glEnd();
	    qglColor( white );
	    glBegin( GL_TRIANGLE_STRIP );
	    {
		glVertex3f(x+1-gridHalf,y-gridHalf, landscape[x+1][y]);
		glVertex3f(x+1-gridHalf,y+1-gridHalf, landscape[x+1][y+1]);
		glVertex3f(x-gridHalf,y+1-gridHalf, landscape[x][y+1]);
	    }
	    glEnd();
	}
}

void GLLandscape::drawSmoothShaded()
{
    // Setup lighting and material properties
    GLfloat position[] = { 15.0, -15.0, 15.0, 0.0 };
    GLfloat ambient[]  = { 0.50, 0.50, 0.50, 0.0 };
    GLfloat diffuse[]  = { 1.00, 1.00, 1.00, 0.0 };
    GLfloat specular[] = { 1.0, 1.0, 1.0, 0.0 };
    GLfloat materialAmbient[]   = { 0.00, 0.00, 1.0, 0.0 };
//    GLfloat materialDiffuse[]   = { 1.00, 1.00, 1.0, 0.0 };
    GLfloat materialShininess[] = { 128.0 };
    GLfloat materialSpecular[]  = { 1.0, 1.0, 1.0, 1.0 };

    glMaterialfv( GL_FRONT, GL_SPECULAR, materialSpecular );
//   glMaterialfv( GL_FRONT, GL_DIFFUSE, materialDiffuse );
    glMaterialfv( GL_FRONT, GL_AMBIENT, materialAmbient );
    glMaterialfv( GL_FRONT, GL_SHININESS, materialShininess );

    glLightfv( GL_LIGHT0, GL_POSITION, position );
    glLightfv( GL_LIGHT0, GL_AMBIENT, ambient );
    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT0, GL_SPECULAR, specular );
    glEnable( GL_LIGHTING );
    glEnable( GL_LIGHT0 );
    glLightModeli( GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE );
    glEnable( GL_NORMALIZE );
    glShadeModel( GL_SMOOTH );

    for ( int y = 0; y < gridSize-1; y++ )
	for ( int x = 0; x < gridSize-1; x++ ) {
	    glBegin( GL_POLYGON );
	    {
		glNormal3dv(vertexNormals[x][y].n);
		glVertex3f(x-gridHalf,y-gridHalf,landscape[x][y]);

		glNormal3dv(vertexNormals[x+1][y].n);
		glVertex3f(x+1-gridHalf, y-gridHalf, landscape[x+1][y]);

		glNormal3dv(vertexNormals[x+1][y+1].n);
		glVertex3f(x+1-gridHalf, y+1-gridHalf, landscape[x+1][y+1]);
	    }
	    glEnd();

	    glBegin( GL_POLYGON );
	    {
		glNormal3dv(vertexNormals[x][y].n);
		glVertex3f(x-gridHalf,y-gridHalf, landscape[x][y]);

		glNormal3dv(vertexNormals[x+1][y+1].n);
		glVertex3f(x+1-gridHalf,y+1-gridHalf, landscape[x+1][y+1]);

		glNormal3dv(vertexNormals[x][y+1].n);
		glVertex3f(x-gridHalf,y+1-gridHalf, landscape[x][y+1]);
	    }
	    glEnd();
	}
    glDisable( GL_LIGHTING );
}

void GLLandscape::setGridSize( int size )
{
    // Destroy old grid..
    if ( landscape != NULL ) {
	for( int i = 0; i < gridSize; i++ ) {
	    delete[] landscape[i];
	    delete[] normals[i];
	    delete[] vertexNormals[i];
	    delete[] wt[i];
	    delete[] wave[i];
	}
	delete[] landscape;
	delete[] normals;
	delete[] vertexNormals;
	delete[] wt;
	delete[] wave;
    }

    // ..and create a new one
    if ( (size % 2) != 0 )
	size++;
    gridSize = size;
    gridHalf = gridSize / 2;
    initFractals  = TRUE;
    landscape     = new double*[gridSize];
    normals       = new gridNormals*[gridSize];
    vertexNormals = new avgNormals*[gridSize];
    wt            = new double*[gridSize];
    wave          = new double*[gridSize];
    for ( int i = 0; i < gridSize; i++ ) {
	landscape[i]     = new double[gridSize];
	normals[i]       = new gridNormals[gridSize];
	vertexNormals[i] = new avgNormals[gridSize];
	wt[i]   = new double[gridSize];
	wave[i] = new double[gridSize];

	memset( landscape[i], 0, gridSize*sizeof(double) );
	memset( normals[i], 0, gridSize*sizeof(gridNormals) );
	memset( vertexNormals[i], 0, gridSize*sizeof(avgNormals) );
	memset( wt[i], 0, gridSize*sizeof(double) );
	memset( wave[i], 0, gridSize*sizeof(double) );
    }

    initializeGL();
    updateGL();
}

void GLLandscape::rotate( GLfloat deg, Axis axis )
{
    glMatrixMode( GL_MODELVIEW );
    for ( int i = 0; i < 2; i++ ) {
	glLoadMatrixf((GLfloat *) views[i].model);
	if ( axis == XAxis )
	    glRotatef( deg, 1, 0, 0 );
	else if ( axis == YAxis )
	    glRotatef( deg, 0, 1, 0 );
	else
	    glRotatef( deg, 0, 0, 1 );
	glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) views[i].model);
    }
    glLoadMatrixf((GLfloat *) views[V3D].model);
}

void GLLandscape::rotateX( int deg )
{
    static int oldDeg = 0;

    rotate( deg-oldDeg, XAxis );
    oldDeg = deg;
    updateGL();
}

void GLLandscape::rotateY( int deg )
{
    static int oldDeg = 0;

    rotate( deg-oldDeg, YAxis );
    oldDeg = deg;
    updateGL();
}

void GLLandscape::rotateZ( int deg )
{
    static int oldDeg = 0;

    rotate( deg-oldDeg, ZAxis );
    oldDeg = deg;
    updateGL();
}

void GLLandscape::zoom( int z )
{
    float zoom;
    if ( z < 100 ) {
	zoom = 1 + 4.99 - (z*5.0 / 100.0);
    } else {
	z = 200 - z;
	zoom = z / 100.0;
    }
    glMatrixMode( GL_MODELVIEW );
    // Always scale the original model matrix
    glLoadMatrixf((GLfloat *) views[ORG].model);
    glScalef( zoom, zoom, zoom );
    glGetFloatv(GL_MODELVIEW_MATRIX, (GLfloat *) views[V3D].model);
    updateGL();
}


void GLLandscape::fractalize()
{
    Vector p;
    double value;
    double roughness = 0.5;
    int frequency    = 50;

    p.x = p.y = p.z = 0;
    // Initialise fbm routine
    if ( initFractals ) {
	initFractals = FALSE;
	value = fBm( p, roughness, 2.0, 8.0, 1 );
    }

    // Fractalize grid
    for ( int x = 0; x < gridSize; x++ ) {
	for ( int y = 0; y < gridSize; y++ ) {
	    p.x = (double) x / (101 - frequency);
	    p.y = (double) y / (101 - frequency);
	    p.z = (double) landscape[x][y] / (101 - frequency);
	    value = fBm(p, roughness, 2.0, 8.0, 0);
	    landscape[x][y] += value;
	}
    }
    calculateVertexNormals();
    updateGL();
}


//
// Calculate the vector cross product of v and w, store result in n.
//
static void crossProduct( double v[3], double w[3], double n[3] )
{
    n[0] = v[1]*w[2]-w[1]*v[2];
    n[1] = w[0]*v[2]-v[0]*w[2];
    n[2] = v[0]*w[1]-w[0]*v[1];
}

void GLLandscape::calculateVertexNormals()
{
    double len, v[3], v2[3], w[3], w2[3], n[3], n2[3];

    // Calculate the surface normals for all polygons in the
    // height field
    for ( int i = 0; i < (gridSize-1); i++ )
	for ( int k = 0; k < (gridSize-1); k++ ) {
	    /* Lower poly normal */
	    v[0] = 1; // (i+1)-i
	    v[1] = 0; // k-k
	    v[2] = landscape[i+1][k]-landscape[i][k];
	    w[0] = 1; // (i+1)-i
	    w[1] = 1; // (k+1)-k
	    w[2] = landscape[i+1][k+1]-landscape[i][k];
	    crossProduct( v, w, n );
	    len = sqrt(n[0]*n[0]+n[1]*n[1]+n[2]*n[2]);
	    normals[i][k].l[0] = n[0]/len;
	    normals[i][k].l[1] = n[1]/len;
	    normals[i][k].l[2] = n[2]/len;

	    /* Upper poly normal */
	    v2[0] = -1.0; // i-(i+1);
	    v2[1] = 0.0;  // (k+1)-(k+1);
	    v2[2] = landscape[i][k+1]-landscape[i+1][k+1];
	    w2[0] = -1.0; // i-(i+1);
	    w2[1] = -1.0; // k-(k+1);
	    w2[2] = landscape[i][k]-landscape[i+1][k+1];
	    crossProduct( v2, w2, n2 );
	    len = sqrt(n2[0]*n2[0]+n2[1]*n2[1]+n2[2]*n2[2]);
	    normals[i][k].u[0] = n2[0]/len;
	    normals[i][k].u[1] = n2[1]/len;
	    normals[i][k].u[2] = n2[2]/len;
	}

    // Calculate proper vertex normals
    averageNormals();
}

void GLLandscape::averageNormals()
{
    // Calculate the average surface normal for a vertex based on
    // the normals of the surrounding polygons
    for ( int i = 0; i < gridSize; i++ )
	for ( int k = 0; k < gridSize; k++ ) {
	    if ( i > 0 && k > 0 && i < (gridSize-1) && k < (gridSize-1) ) {
		// For vertices that are *not* on the edge of the height field
		for ( int t = 0; t < 3; t++ ) // X, Y and Z components
		    vertexNormals[i][k].n[t] = ( normals[i][k].u[t] +
						 normals[i][k].l[t] +
						 normals[i][k-1].u[t] +
						 normals[i-1][k-1].u[t] +
						 normals[i-1][k-1].l[t] +
						 normals[i-1][k].l[t] )/6.0;
	    } else {
		// Vertices that are on the edge of the height field require
		// special attention..
		if ( i == 0 && k == 0 ) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = ( normals[i][k].u[t] +
						     normals[i][k].l[t] )/2.0;
		} else if ( i == gridSize-1 && k == gridSize-1 ) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = ( normals[i][k].u[t] +
						     normals[i][k].l[t] )/2.0;
		} else if ( i == gridSize-1) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = vertexNormals[i-1][k].n[t];
		} else if ( k == gridSize-1 ) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = vertexNormals[i][k-1].n[t];
		} else if ( k > 0 ) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = (normals[i][k].u[t] +
						    normals[i][k].l[t] +
			                            normals[i][k-1].u[t])/3.0;
		} else if ( i > 0 ) {
		    for ( int t = 0; t < 3; t++ )
			vertexNormals[i][k].n[t] = (normals[i][k].u[t] +
						    normals[i][k].l[t] +
			                            normals[i-1][k].l[t])/3.0;
		}
	    }
	}
}

void GLLandscape::setWireframe( int state )
{
    if ( state != 1 ) {
	mode = Wireframe;
	updateGL();
    }
}

void GLLandscape::setFilled( int state )
{
    if ( state != 1 ) {
	mode = Filled;
	updateGL();
    }
}

void GLLandscape::setShaded( int state )
{
    if ( state != 1 ) {
	mode = Shaded;
	updateGL();
    }
}

void GLLandscape::mousePressEvent( QMouseEvent *e )
{
    oldPos = e->pos();
}

void GLLandscape::mouseReleaseEvent( QMouseEvent *e )
{
    oldPos = e->pos();
}

void GLLandscape::mouseMoveEvent( QMouseEvent *e )
{
    GLfloat rx = (GLfloat) (e->x() - oldPos.x()) / width();
    GLfloat ry = (GLfloat) (e->y() - oldPos.y()) / height();

    if ( e->state() == LeftButton ) {
	// Left button down - rotate around X and Y axes
	oldX = 180*ry;
	oldY = 180*rx;
	rotate( oldX, XAxis );
	rotate( oldY, YAxis );
	updateGL();
    } else if ( e->state() == RightButton ) {
	// Right button down - rotate around X and Z axes
	oldX = 180*ry;
	oldZ = 180*rx;
	rotate( oldX, XAxis );
	rotate( oldZ, ZAxis );
	updateGL();
    }
    oldPos = e->pos();
}

void GLLandscape::createDisplayLists()
{
    glDeleteLists( Wireframe, 3 );
    glGenLists( 3 );

    glNewList( Wireframe, GL_COMPILE );
    drawWireframe( FALSE );
    glEndList();
    glNewList( Filled, GL_COMPILE );
    drawFilled();
    glEndList();
    glNewList( Shaded, GL_COMPILE );
    drawSmoothShaded();
    glEndList();
}

void GLLandscape::timerEvent( QTimerEvent * )
{
    // This wave function is a slightly modified version of the
    // the one found in the Water demo created by Roman Podobedov.
    // http://romka.demonews.com/index_eng.htm

    int dx, dy; /* Disturbance Point */
    float s, v, W, t;
    int i, j;

    dx = gridSize >> 1;
    dy = gridSize >> 1;
    W = 0.3;
    v = -4; /* Wave speed */

    for ( i = 0; i < gridSize; i++ )
	for ( j = 0; j < gridSize; j++ )
	{
	    s = sqrt( (j - dx) * (j - dx) + (i - dy) * (i - dy));
	    wt[i][j] += 0.1;
	    t = s / v;
	    landscape[i][j] -= wave[i][j];
	    wave[i][j] = 2 * sin( 2 * PI * W * ( wt[i][j] + t ) );
	    landscape[i][j] += wave[i][j];
	}
    if ( mode == Shaded )
	calculateVertexNormals();
    updateGL();
}

void GLLandscape::toggleWaveAnimation( bool state )
{
    if ( state )
 	startTimer( 20 );
    else
	killTimers();
}
