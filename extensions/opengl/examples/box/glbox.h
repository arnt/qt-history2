/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/box/glbox.h#1 $
**
** Definition of GLBox
** This is a simple QGLWidget displaying an openGL wireframe box
**
****************************************************************************/

#ifndef GLCUBE_H
#define GLCUBE_H

#include <qgl.h>


class GLBox : public QGLWidget
{
    Q_OBJECT;

public:

    GLBox( QWidget* parent, const char* name );

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
