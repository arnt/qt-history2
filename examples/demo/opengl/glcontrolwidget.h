#ifndef GLCONTROLWIDGET_H
#define GLCONTROLWIDGET_H

#include <qgl.h>

class GLControlWidget : public QGLWidget
{
    Q_OBJECT

public:
    GLControlWidget( QWidget *parent, const char *name = 0, QGLWidget *share = 0, WFlags f = 0 );

    virtual void	transform();

public slots:
    void		setXRotation( int degrees );
    void		setYRotation( int degrees );
    void		setZRotation( int degrees );

    void		setScale( double s );

    void		setXTrans( double x );
    void		setYTrans( double y );
    void		setZTrans( double z );

    virtual void	setRotationImpulse( double x, double y, double z );
    virtual void	setTranslationImpulse( double x, double y, double z );

protected:
    void		setAnimationDelay( int ms );
    void		mousePressEvent( QMouseEvent *e );
    void		mouseReleaseEvent( QMouseEvent *e );
    void		mouseMoveEvent( QMouseEvent * );
    void		mouseDoubleClickEvent( QMouseEvent * );
    void		wheelEvent( QWheelEvent * );

    GLfloat xRot, yRot, zRot;
    GLfloat scale;
    GLfloat xTrans, yTrans, zTrans;
    bool animation;

protected slots:
    virtual void	animate();

private:
    QPoint oldPos;
    QTimer* timer;
    int delay;
};

#endif
