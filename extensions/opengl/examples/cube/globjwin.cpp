/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/cube/globjwin.cpp#1 $
**
** Implementation of GLObjectWindow widget class
**
****************************************************************************/


#include <qpushbt.h>
#include <qslider.h>
#include <qlayout.h>
#include "globjwin.h"
#include "glcube.h"


GLObjectWindow::GLObjectWindow( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    QHBoxLayout* hgl = new QHBoxLayout( this, 20, 20, "hbox");
    QVBoxLayout* vgl = new QVBoxLayout( 20, "vbox");
    hgl->addLayout( vgl );

    GLCube* c = new GLCube( this, "glcube");
    c->setMinimumSize( 50, 50 );
    hgl->add( c, 1 );

    QSlider* x = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "xsl" );
    x->setTickmarks( QSlider::Left );
    x->setMinimumSize( x->sizeHint() );
    vgl->add( x );
    QObject::connect( x, SIGNAL(valueChanged(int)),c,SLOT(setXRotation(int)) );

    QSlider* y = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "ysl" );
    y->setTickmarks( QSlider::Left );
    y->setMinimumSize( y->sizeHint() );
    vgl->add( y );
    QObject::connect( y, SIGNAL(valueChanged(int)),c,SLOT(setYRotation(int)) );

    QSlider* z = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "zsl" );
    z->setTickmarks( QSlider::Left );
    z->setMinimumSize( z->sizeHint() );
    vgl->add( z );
    QObject::connect( z, SIGNAL(valueChanged(int)),c,SLOT(setZRotation(int)) );

    hgl->activate();
}
