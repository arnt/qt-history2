/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/sharedbox/glbox.h#1 $
**
** Definition of GLBox
** This is a simple QGLWidget displaying a box
**
****************************************************************************/

#ifndef GLBOX_H
#define GLBOX_H

#include <qgl.h>


class GLBox : public QGLWidget
{
    Q_OBJECT

public:

    GLBox( QWidget* parent, const char* name, const QGLWidget* shareWidget=0 );
    ~GLBox();

public slots:

    void		setXRotation( int degrees );
    void		setYRotation( int degrees );
    void		setZRotation( int degrees );

protected:

    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject();

private:

    static GLuint object;
    static int listUsers;

    GLfloat xRot, yRot, zRot, scale;

};


#endif // GLBOX_H
