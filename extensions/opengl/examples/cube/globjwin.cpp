/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/cube/globjwin.cpp#2 $
**
** Implementation of GLObjectWindow widget class
**
****************************************************************************/


#include <qpushbt.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
#include <qmenubar.h>
#include <qpopmenu.h>
#include <qapp.h>
#include <qkeycode.h>
#include "globjwin.h"
#include "glcube.h"


GLObjectWindow::GLObjectWindow( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    // Create top-level layout manager
    QHBoxLayout* hlayout = new QHBoxLayout( this, 20, 20, "hlayout");

    // Create a menu
    QPopupMenu *file = new QPopupMenu();
    file->insertItem( "Exit",  qApp, SLOT(quit()), CTRL+Key_Q );

    // Create a menu bar
    QMenuBar *m = new QMenuBar( this );
    m->insertItem("&File", file );
    hlayout->setMenuBar( m );

    // Create a layout manager for the sliders
    QVBoxLayout* vlayout = new QVBoxLayout( 20, "vlayout");
    hlayout->addLayout( vlayout );

    // Create a nice frame tp put around the openGL widget
    QFrame* f = new QFrame( this, "frame" );
    f->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    f->setLineWidth( 2 );
    hlayout->add( f, 1 );

    // Create a layout manager for the openGL widget
    QHBoxLayout* flayout = new QHBoxLayout( f, 2, 2, "flayout");

    // Create an openGL widget
    GLCube* c = new GLCube( f, "glcube");
    c->setMinimumSize( 50, 50 );
    flayout->add( c, 1 );
    flayout->activate();

    // Create the three sliders; one for each rotation axis
    QSlider* x = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "xsl" );
    x->setTickmarks( QSlider::Left );
    x->setMinimumSize( x->sizeHint() );
    vlayout->add( x );
    QObject::connect( x, SIGNAL(valueChanged(int)),c,SLOT(setXRotation(int)) );

    QSlider* y = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "ysl" );
    y->setTickmarks( QSlider::Left );
    y->setMinimumSize( y->sizeHint() );
    vlayout->add( y );
    QObject::connect( y, SIGNAL(valueChanged(int)),c,SLOT(setYRotation(int)) );

    QSlider* z = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "zsl" );
    z->setTickmarks( QSlider::Left );
    z->setMinimumSize( z->sizeHint() );
    vlayout->add( z );
    QObject::connect( z, SIGNAL(valueChanged(int)),c,SLOT(setZRotation(int)) );

    // Start the geometry management
    hlayout->activate();
}
