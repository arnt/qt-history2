#ifndef GLLANDSCAPE_H
#define GLLANDSCAPE_H

#include <qgl.h>

class GLLandscape : public QGLWidget
{
    Q_OBJECT

public:
    GLLandscape( QWidget * parent = 0, const char * name = 0 );
    ~GLLandscape();
    
public slots:
    void rotateX( int );
    void rotateY( int );
    void rotateZ( int );
    void zoom( int );
    void fractalize();
    
    void setWireframe( int );
    void setFilled( int );
    void setShaded( int );
    void setGridSize( int );
    
    void toggleWaveAnimation( bool );

protected:
    void paintGL();
    void initializeGL();
    void resizeGL( int w, int h );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void timerEvent( QTimerEvent * );
    
    void drawWireframe( bool removeHiddenLines );
    void drawFilled();
    void drawSmoothShaded();
    
private:
    enum Axis { XAxis, YAxis, ZAxis };
    enum RenderModes { Wireframe, Filled, Shaded };

    void rotate( GLfloat deg, Axis axis );
    void calculateVertexNormals();
    void averageNormals();
    void createDisplayLists();

    RenderModes mode;
    
    typedef struct grid_normals {
	double u[3], l[3];
    } gridNormals;

    // Structure used to store the vertex normals for the landscape
    typedef struct avg_normals {
	double n[3];
    } avgNormals;

    double      ** landscape; // Height field data
    double      ** wave;      // Wave data
    double      ** wt;
    gridNormals ** normals;
    avgNormals  ** vertexNormals;
    
    QPoint  oldPos;
    GLfloat oldX, oldY, oldZ;
    bool initFractals;
    int  gridSize, gridHalf;
};

#endif
