#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <qgl.h>

class GLWidget : public QGLWidget
{
    Q_OBJECT
    Q_PROPERTY( double xRot READ xRot WRITE setXRot )
    Q_PROPERTY( double yRot READ yRot WRITE setYRot )
    Q_PROPERTY( double zRot READ zRot WRITE setZRot )
    Q_PROPERTY( double scale READ scale WRITE setScale )

public:
    GLWidget( QWidget* parent, const char* name );
    ~GLWidget();

    double xRot() const { return xrot; }
    double yRot() const { return yrot; }
    double zRot() const { return zrot; }
    double scale() const { return scale_; }

public slots:
    void setXRot( double );
    void setYRot( double );
    void setZRot( double );
    void setScale( double );

protected:
    void		initializeGL();
    void		paintGL();
    void		resizeGL( int w, int h );

    virtual GLuint 	makeObject();

private:
    GLuint object;
    GLfloat xrot, yrot, zrot, scale_;
};

#endif //GLWIDGET_H
