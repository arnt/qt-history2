/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "application.h"
#include <qworkspace.h>
#include <qimage.h>
#include <qpixmap.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qmultilineedit.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qmessagebox.h>
#include <qprinter.h>
#include <qapplication.h>
#include <qpushbutton.h>
#include <qaccel.h>
#include <qtextstream.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qwhatsthis.h>
#include <qobjectlist.h>
#include <qvbox.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qgl.h>

#include "glinfo.h"

class Triangles : public QGLWidget
{
public:
    Triangles( const QGLFormat& fmt, QWidget * parent )
	: QGLWidget( fmt, parent ), angle( 0 )
    { 
	startTimer( 40 );
    }
    
protected:
    int angle;
    void timerEvent( QTimerEvent * ) {
	angle += 3;
	update(); // updates the widget every 40 ms
    }	
    void initializeGL() { 
	glClearColor( 0.0, 0.0, 0.0, 1.0 ); // black background
 	glShadeModel( GL_SMOOTH ); // interpolate colors btw. vertices
   	glEnable( GL_DEPTH_TEST ); // removes hidden surfaces
 	glMatrixMode( GL_PROJECTION );
 	glLoadIdentity();	
 	glOrtho( -5.0, 5.0, -5.0, 5.0, 1.0, 100.0 );
 	glMatrixMode( GL_MODELVIEW );
    }
    void resizeGL( int w, int h ) {
	glViewport( 0, 0, w, h ); // resize the GL drawing area
    }
    void paintGL() {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 
 	glLoadIdentity();
 	glTranslatef( 0.0, 0.0, -6.0 );
 	glRotatef( angle, 1.0, 1.0, 0.0 ); // rotate around the x and y axis
	glBegin( GL_TRIANGLES ); { // draw a tetrahedron
  	    glColor3f( 1.0, 0.0, 0.0 ); glVertex3f( -2.0, -2.0, 0.0 );
  	    glColor3f( 0.0, 1.0, 0.0 ); glVertex3f( -2.0, 2.0, 0.0 );
  	    glColor3f( 0.0, 0.0, 1.0 ); glVertex3f( 2.0, -2.0, 0.0 );

  	    glColor3f( 0.0, 1.0, 1.0 ); glVertex3f( -2.0, -2.0, -4.0 );
  	    glColor3f( 0.0, 1.0, 0.0 ); glVertex3f( -2.0, 2.0, 0.0 );
  	    glColor3f( 0.0, 0.0, 1.0 ); glVertex3f( 2.0, -2.0, 0.0 );

  	    glColor3f( 1.0, 0.0, 0.0 ); glVertex3f( -2.0, -2.0, 0.0 );
  	    glColor3f( 0.0, 1.0, 0.0 ); glVertex3f( -2.0, 2.0, 0.0 );
  	    glColor3f( 0.0, 1.0, 1.0 ); glVertex3f( -2.0, -2.0, -4.0 );

   	    glColor3f( 0.0, 1.0, 1.0 ); glVertex3f( -2.0, -2.0, -4.0 );
   	    glColor3f( 1.0, 0.0, 0.0 ); glVertex3f( -2.0, -2.0, 0.0 );
   	    glColor3f( 0.0, 0.0, 1.0 ); glVertex3f( 2.0, -2.0, 0.0 );
 	}
 	glEnd();
    }
};

ApplicationWindow::ApplicationWindow()
    : QMainWindow( 0, "example application main window", WDestructiveClose )
{
    glInfo = 0;
    QPopupMenu * file = new QPopupMenu( this );
    menuBar()->insertItem( "&File", file );

    file->insertItem( "&Info", this, SLOT( info() ), CTRL+Key_I );
    file->insertSeparator();
    file->insertItem( "&Quit", qApp, SLOT( closeAllWindows() ), CTRL+Key_Q );

    menuBar()->insertSeparator();
    QPopupMenu * help = new QPopupMenu( this );
    menuBar()->insertItem( "&Help", help );

    help->insertItem( "&About", this, SLOT(about()), Key_F1);
    help->insertItem( "About &Qt", this, SLOT(aboutQt()));

    QVBox* vb = new QVBox( this );
    vb->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    ws = new QWorkspace( vb );
    ws->setScrollBarsEnabled( TRUE );
    setCentralWidget( vb );
    ControlWindow *cw = new ControlWindow( ws, 0 );
    
    connect( cw, SIGNAL( newCreate( const QGLFormat & )), this, SLOT( newDoc( const QGLFormat & )));
    
    cw->show();
}


ApplicationWindow::~ApplicationWindow()
{
    delete glInfo;
}



MDIWindow* ApplicationWindow::newDoc( const QGLFormat &f )
{
    MDIWindow* w = new MDIWindow( f, ws, 0 );
    w->setCaption("GL Test");
    w->show();
    return w;
}

void ApplicationWindow::info()
{
    if ( glInfo ) {
	glInfo->show();
    } else {
	glInfo = new GLInfo( this );
	glInfo->resize( 640, 480 );
	glInfo->show();
    }
}


void ApplicationWindow::about()
{
    QMessageBox::about( this, "Qt GL test", "Qt GL test");
}


void ApplicationWindow::aboutQt()
{
    QMessageBox::aboutQt( this, "Qt GL test" );
}


MDIWindow::MDIWindow( const QGLFormat &f, QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    QString str;
    QVBoxLayout *layout = new QVBoxLayout( this );
//     gl = new QGLWidget( f, this );
    gl = new Triangles( f, this );
    if ( !gl->isValid() ) {
	QLabel * l = new QLabel( "Unable to create a GL widget with that config", this);
	layout->addWidget( l );
	return;
    }
    gl->setMinimumSize( QSize( 100, 100 ) );
    gl->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    layout->addWidget( gl );
    str.sprintf("Double Buffer: %d", gl->format().doubleBuffer());
    QLabel *l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Depth Buffer: %d", gl->format().depth());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Rgba: %d", gl->format().rgba());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Alpha Buffer: %d", gl->format().alpha());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Accum Buffer: %d", gl->format().accum());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Stencil Buffer: %d", gl->format().stencil());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Stereo: %d", gl->format().stereo());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Direct rendering: %d", gl->format().directRendering());
    l = new QLabel( str, this);
    layout->addWidget( l );
    str.sprintf("Has Overlay: %d", gl->format().hasOverlay());
    l = new QLabel( str, this);
    layout->addWidget( l );
}

MDIWindow::~MDIWindow()
{
}

ControlWindow::ControlWindow( QWidget *parent, const char* name )
    : QWidget( parent, name )
{
    setCaption( "Control" );
    QVBoxLayout *bl = new QVBoxLayout( this );
    db = new QCheckBox( "Double Buffer", this );
    bl->addWidget( db );
    depth = new QCheckBox( "Depth Buffer", this );
    bl->addWidget( depth );
    rgba = new QCheckBox( "RGBA", this );
    bl->addWidget( rgba );
    alpha = new QCheckBox( "Alpha Buffer", this );
    bl->addWidget( alpha );
    accum = new QCheckBox( "Accum Buffer", this );
    bl->addWidget( accum );
    stencil = new QCheckBox( "Stencil Buffer", this );
    bl->addWidget( stencil );
    
    QPushButton *button = new QPushButton( "Create", this );
    bl->addWidget( button );
    connect( button, SIGNAL( clicked() ), this, SLOT( create()));
};
    

void ControlWindow::create()
{
    QGLFormat f;
    f.setDoubleBuffer( db->isChecked() );
    f.setDepth( depth->isChecked() );
    f.setRgba( rgba->isChecked() );
    f.setAlpha( alpha->isChecked() );
    f.setAccum( accum->isChecked() );
    f.setStencil( stencil->isChecked() );
    emit newCreate( f );
}
