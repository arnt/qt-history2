#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <qgl.h>

class GLWidget : public QGLWidget
{
public:
    GLWidget( QWidget* parent, const char* name );
    ~GLWidget();

protected:
    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject();

private:
    GLuint object;
    GLfloat xRot, yRot, zRot, scale;
};

#endif //GLWIDGET_H