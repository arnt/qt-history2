/*
  box3d.h
*/

#ifndef BOX3D_H
#define BOX3D_H

#include <qgl.h>

class Box3D : public QGLWidget
{
    Q_OBJECT
public:
    Box3D( QWidget *parent = 0, const char *name = 0 );
    ~Box3D();

public slots:
    void setRotationX( int deg ) { rotX = deg; updateGL(); }
    void setRotationY( int deg ) { rotY = deg; updateGL(); }
    void setRotationZ( int deg ) { rotZ = deg; updateGL(); }

protected:
    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL( int w, int h );
    virtual GLuint makeObject();

private:
    GLuint object;
    GLfloat rotX, rotY, rotZ;
};

#endif
