/****************************************************************************
** $Id: //depot/qt/main/extensions/opengl/examples/glpixmap/globjwin.cpp#1 $
**
** Implementation of GLObjectWindow widget class
**
****************************************************************************/


#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qapplication.h>
#include <qkeycode.h>
#include <qpixmap.h>
#include <qpainter.h>
#include "globjwin.h"
#include "glbox.h"


GLObjectWindow::GLObjectWindow( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    // Create a menu
    file = new QPopupMenu();
    file->setCheckable( TRUE );
    file->insertItem( "Render Pixmap", this, 
		      SLOT(makePixmap()) );
    file->insertItem( "Render Pixmap Manually", this, 
		      SLOT(makePixmapManually()) );
    file->insertItem( "Render Pixmap Hidden", this, 
		      SLOT(makePixmapHidden()) );
    file->insertItem( "Render Pixmap Hidden and Manually", this, 
		      SLOT(makePixmapHiddenManually()) );
    file->insertSeparator();
    fixMenuItemId = file->insertItem( "Use Fixed Pixmap Size", this, 
				      SLOT(useFixedPixmapSize()) );
    file->insertSeparator();
    insertMenuItemId = file->insertItem( "Insert Pixmap Here", this, 
					 SLOT(makePixmapForMenu()) );
    file->insertSeparator();
    file->insertItem( "Exit",  qApp, SLOT(quit()), CTRL+Key_Q );

    // Create a menu bar
    QMenuBar *m = new QMenuBar( this );
    m->setSeparator( QMenuBar::InWindowsStyle );
    m->insertItem("&File", file );

    // Create nice frames to put around the OpenGL widgets
    QFrame* f1 = new QFrame( this, "frame1" );
    f1->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    f1->setLineWidth( 2 );

    // Create an OpenGL widget
    c1 = new GLBox( f1, "glbox1");

    // Create a label that can display the pixmap
    lb = new QLabel( this, "pixlabel" );
    lb->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    lb->setLineWidth( 2 );
    lb->setAlignment( AlignCenter );
    lb->setMargin( 0 );

    // Create the three sliders; one for each rotation axis
    QSlider* x = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "xsl" );
    x->setTickmarks( QSlider::Left );
    connect( x, SIGNAL(valueChanged(int)), c1, SLOT(setXRotation(int)) );

    QSlider* y = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "ysl" );
    y->setTickmarks( QSlider::Left );
    connect( y, SIGNAL(valueChanged(int)), c1, SLOT(setYRotation(int)) );

    QSlider* z = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "zsl" );
    z->setTickmarks( QSlider::Left );
    connect( z, SIGNAL(valueChanged(int)), c1, SLOT(setZRotation(int)) );


    // Now that we have all the widgets, put them into a nice layout

    // Put the sliders on top of each other
    QVBoxLayout* vlayout = new QVBoxLayout( 20, "vlayout");
    vlayout->addWidget( x );
    vlayout->addWidget( y );
    vlayout->addWidget( z );

    // Put the GL widget inside the frame
    QHBoxLayout* flayout1 = new QHBoxLayout( f1, 2, 2, "flayout1");
    flayout1->addWidget( c1, 1 );

    // Top level layout, puts the sliders to the left of the frame/GL widget
    QHBoxLayout* hlayout = new QHBoxLayout( this, 20, 20, "hlayout");
    hlayout->setMenuBar( m );
    hlayout->addLayout( vlayout );
    hlayout->addWidget( f1, 1 );
    hlayout->addWidget( lb, 1 );

}



void GLObjectWindow::makePixmap()
{
    // This is the easiest way to render a pixmap, and sufficient unless one
    // has special needs

    // Make a pixmap to to be rendered by the gl widget
    QPixmap pm;

    // Render the pixmap, with either c1's size or the fixed size pmSz
    if ( pmSz.isValid() )
	pm = c1->renderPixmap( pmSz.width(), pmSz.height() );
    else 
	pm = c1->renderPixmap();

    if ( !pm.isNull() ) {
	// Present the pixmap to the user
	drawOnPixmap( &pm );
	lb->setPixmap( pm );
    }
    else {
	lb->setText( "Failed to render Pixmap." );
    }
}


void GLObjectWindow::makePixmapManually()
{
    // Make a pixmap to be rendered on by the QGLWidget
    QPixmap pm( pmSz.isValid() ? pmSz : c1->size() );

    // Store the QGLWidget's current context, for later restoration
    QGLContext* origCx = (QGLContext*)c1->context();

    // Make a gl format suitable for pixmap rendering 
    QGLFormat fmt( QGL::SingleBuffer | QGL::IndirectRendering );

    // Make a gl context to draw on our pixmap
    QGLContext* pcx = new QGLContext( fmt, &pm );

    // Make the QGLwidget use our pixmap-context, 
    // without deleting the old context, which we are storing
    // It will share GL display lists with the old context
    c1->setContext( pcx, 0, FALSE );
    
    if ( c1->isValid() ) {

	// Make the QGLWidget draw itself, i.e. render the pixmap
	c1->updateGL();

	// Present the pixmap to the user
	drawOnPixmap( &pm );
        lb->setPixmap( pm );
    }
    else {
	lb->setText( "Failed to render Pixmap." );
    }
    // Restore the old context to the QGLWidget, 
    // so it will continue to work as before
    c1->setContext( origCx );	// Will delete pcx
}


void GLObjectWindow::makePixmapHidden()
{
    // Make a gl format suitable for pixmap rendering 
    QGLFormat fmt( QGL::SingleBuffer | QGL::IndirectRendering );

    // Make a QGLWidget to draw the pixmap. This widget will not be shown.
    GLBox* w = new GLBox( fmt, this, "temporary glwidget", c1 );

    bool success = FALSE;
    QPixmap pm;

    if ( w->isValid() ) {
	// Set the current rotation
	w->copyRotation( *c1 );

	// Determine wanted pixmap size
	QSize sz = pmSz.isValid() ? pmSz : c1->size();

	// Make our hidden glwidget render the pixmap
	pm = w->renderPixmap( sz.width(), sz.height() );

	if ( !pm.isNull() )
	    success = TRUE;
    }

    if ( success ) {
	// Present the pixmap to the user
	drawOnPixmap( &pm );
	lb->setPixmap( pm );
    }
    else {
	lb->setText( "Failed to render Pixmap." );
    }
    delete w;
}


void GLObjectWindow::makePixmapHiddenManually()
{
    // Make a gl format suitable for pixmap rendering 
    QGLFormat fmt( QGL::SingleBuffer | QGL::IndirectRendering );

    // Make a QGLWidget to draw the pixmap. This widget will not be shown.    
    GLBox* w = new GLBox( this, "temporary glwidget", c1 );

    // Make a pixmap to be rendered
    QPixmap pm( pmSz.isValid() ? pmSz : c1->size() );

    // Make a gl context to draw on this pixmap
    QGLContext* pcx = new QGLContext( fmt, &pm );

    // Make our widget use this context
    w->setContext( pcx );

    if ( w->isValid() ) {
	// Set the current rotation
	w->copyRotation( *c1 );

	// Make the QGLWidget draw itself, i.e. render the pixmap
	w->updateGL();

	// Present the pixmap to the user
	drawOnPixmap( &pm );
	lb->setPixmap( pm );
    }
    else {
	lb->setText( "Failed to render Pixmap." );
    }
    delete w;	// Will delete pcx
}


void GLObjectWindow::drawOnPixmap( QPixmap* pm )
{
    // Draw some text on the pixmap to differentiate it from the GL window

    if ( pm->isNull() ) {
	warning("Cannot draw on null pixmap");
	return;
    }
    else {
	QPainter p( pm );
       p.setFont( QFont( "Helvetica", 18 ) );
	p.setPen( white );
	p.drawText( pm->rect(), AlignCenter, "This is a Pixmap" );
    }
}



void GLObjectWindow::useFixedPixmapSize()
{
    if ( pmSz.isValid() ) {
	pmSz = QSize();
	file->setItemChecked( fixMenuItemId, FALSE );
    }
    else {
	pmSz = QSize( 200, 200 );
	file->setItemChecked( fixMenuItemId, TRUE );
    }
}


void GLObjectWindow::makePixmapForMenu()
{
    QPixmap pm = c1->renderPixmap( 32, 32 );
    if ( !pm.isNull() )
	file->changeItem( pm, "Insert Pixmap Here", insertMenuItemId );
}
