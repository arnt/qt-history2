/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/cube/glcube.h#1 $
**
** Definition of GLCube
** This is a simple QGLWidget displaying an openGL wireframe cube
**
****************************************************************************/

#ifndef GLCUBE_H
#define GLCUBE_H

#include <qgl.h>


class GLCube : public QGLWidget
{
    Q_OBJECT;

public:

    GLCube( QWidget* parent, const char* name );

public slots:

    void		setXRotation( int degrees );
    void		setYRotation( int degrees );
    void		setZRotation( int degrees );

protected:

    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject();

private:

    GLuint object;
    GLfloat xRot, yRot, zRot, scale;

};


#endif
