#include "glcontrolwidget.h"

#include <qcursor.h>
#include <qtimer.h>

#include <math.h>

const int redrawWait = 50;

GLControlWidget::GLControlWidget( QWidget *parent, const char *name, QGLWidget *share, WFlags f )
: QGLWidget( parent, name, share, f ),
  xRot(0),yRot(0),zRot(0),xTrans(0),yTrans(0),zTrans(-10.0),scale(5.0), animation(TRUE), delay( 50 )
{
    setCursor( pointingHandCursor  );
    timer = new QTimer( this );
    connect( timer, SIGNAL(timeout()), SLOT(animate()) );
    timer->start( delay );
}

void GLControlWidget::transform()
{
    glTranslatef( xTrans, yTrans, zTrans );
    glScalef( scale, scale, scale );

    glRotatef( xRot, 1.0, 0.0, 0.0 );
    glRotatef( yRot, 0.0, 1.0, 0.0 );
    glRotatef( zRot, 0.0, 0.0, 1.0 );
}

/*!
  Set the rotation angle of the object to \e degrees around the X axis.
*/
void GLControlWidget::setXRotation( double degrees )
{
    xRot = (GLfloat)fmod(degrees, 360.0);
    updateGL();
}

/*!
  Set the rotation angle of the object to \e degrees around the Y axis.
*/
void GLControlWidget::setYRotation( double degrees )
{
    yRot = (GLfloat)fmod(degrees, 360.0);
    updateGL();
}


/*!
  Set the rotation angle of the object to \e degrees around the Z axis.
*/
void GLControlWidget::setZRotation( double degrees )
{
    zRot = (GLfloat)fmod(degrees, 360.0);
    updateGL();
}

void GLControlWidget::setScale( double s )
{
    scale = s;
    updateGL();
}

void GLControlWidget::setXTrans( double x )
{
    xTrans = x;
    updateGL();
}

void GLControlWidget::setYTrans( double y )
{
    yTrans = y;
    updateGL();
}

void GLControlWidget::setZTrans( double z )
{
    zTrans = z;
    updateGL();
}

void GLControlWidget::setRotationImpulse( double x, double y, double z )
{
    setXRotation( xRot + 180*x );
    setYRotation( yRot + 180*y );
    setZRotation( zRot - 180*z );
}

void GLControlWidget::setTranslationImpulse( double x, double y, double z )
{
    setXTrans( xTrans + 2*x );
    setYTrans( yTrans - 2*y );
    setZTrans( zTrans + 2*z );
}

void GLControlWidget::mousePressEvent( QMouseEvent *e )
{
    e->accept();
    oldPos = e->pos();
}

void GLControlWidget::mouseReleaseEvent( QMouseEvent *e )
{
    e->accept();
    oldPos = e->pos();
}

void GLControlWidget::mouseMoveEvent( QMouseEvent *e )
{
    e->accept();
    double dx = e->x() - oldPos.x();
    double dy = e->y() - oldPos.y();

    oldPos = e->pos();

    double rx = dx / width();
    double ry = dy / height();

    if ( e->state() == LeftButton )
	setRotationImpulse( ry, rx, 0 );
    else if ( e->state() == RightButton )
	setRotationImpulse( ry, 0, rx );
    else if ( e->state() == MidButton )
	setTranslationImpulse( rx, ry, 0 );
    else if ( e->state() == ( LeftButton | RightButton ) )
	setTranslationImpulse( rx, 0, ry );
}

void GLControlWidget::wheelEvent( QWheelEvent *e )
{
    e->accept();
    if ( scale <= ( (double)e->delta() / 1000 ) )
	return;
    setScale( scale - ( (double)e->delta() / 1000 ));
}

void GLControlWidget::mouseDoubleClickEvent( QMouseEvent * )
{
    if ( delay <= 0 )
	return;

    animation = !animation;
    if ( animation )
	timer->start( delay );
    else
	timer->stop();
}

void GLControlWidget::animate()
{
}

void GLControlWidget::setAnimationDelay( int ms )
{
    timer->stop();
    delay = ms;
    if ( animation )
	timer->start( delay );
}
