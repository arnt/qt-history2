#include <qpushbutton.h>
#include <qslider.h>
#include <qlayout.h>
#include <qframe.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qapplication.h>
#include <qkeycode.h>
#include "globjwin.h"
#include "glbox.h"

#ifdef Q_WS_X11
#include <GL/glx.h>
#endif

class MyContext : public QGLContext
{
public:
    MyContext( const QGLFormat& format, QPaintDevice * dev )
	: QGLContext( format, dev ) {}
protected:
#ifdef Q_WS_WIN32
    int choosePixelFormat( void * dummyPfd, HDC pdc )
    {
	int pmDepth = 0;
	PIXELFORMATDESCRIPTOR* p = (PIXELFORMATDESCRIPTOR*)dummyPfd;
	memset( p, 0, sizeof(PIXELFORMATDESCRIPTOR) );
	p->nSize = sizeof(PIXELFORMATDESCRIPTOR);
	p->nVersion = 1;
	p->dwFlags  = PFD_SUPPORT_OPENGL;
	if ( deviceIsPixmap() )
	    p->dwFlags |= PFD_DRAW_TO_BITMAP;
	else
	    p->dwFlags |= PFD_DRAW_TO_WINDOW;
	if ( glFormat.doubleBuffer() && !deviceIsPixmap() )
	    p->dwFlags |= PFD_DOUBLEBUFFER;
	if ( glFormat.stereo() )
	    p->dwFlags |= PFD_STEREO;
	if ( glFormat.depth() )
	    p->cDepthBits = 32;
	else
	    p->dwFlags |= PFD_DEPTH_DONTCARE;
	if ( glFormat.rgba() ) {
	    p->iPixelType = PFD_TYPE_RGBA;
	    if ( deviceIsPixmap() )
		p->cColorBits = pmDepth;
	    else
		p->cColorBits = 32;
	} else {
	    p->iPixelType = PFD_TYPE_COLORINDEX;
	    p->cColorBits = 8;
	}
	if ( glFormat.alpha() )
	    p->cAlphaBits = 8;
	if ( glFormat.accum() )
	    p->cAccumBits = p->cColorBits + p->cAlphaBits;
	if ( glFormat.stencil() )
	    p->cStencilBits = 4;
	p->iLayerType = PFD_MAIN_PLANE;
	int chosenPfi = ChoosePixelFormat( pdc, p );
	qWarning( "chosenPfi: %p", chosenPfi );
	return chosenPfi;
    }
#endif
    
#ifdef Q_WS_X11
    void* chooseVisual()
    {
	QGLFormat f = format();
	int spec[40];
	int i = 0;
	spec[i++] = GLX_LEVEL;
	spec[i++] = f.plane();

	if ( f.doubleBuffer() )
	    spec[i++] = GLX_DOUBLEBUFFER;
	if ( f.depth() ) {
	    spec[i++] = GLX_DEPTH_SIZE;
	    spec[i++] = 1;
	}
	if ( f.stereo() ) {
	    spec[i++] = GLX_STEREO;
	}
	if ( f.stencil() ) {
	    spec[i++] = GLX_STENCIL_SIZE;
	    spec[i++] = 1;
	}
	if ( f.rgba() ) {
	    spec[i++] = GLX_RGBA;
	    spec[i++] = GLX_RED_SIZE;
	    spec[i++] = 1;
	    spec[i++] = GLX_GREEN_SIZE;
	    spec[i++] = 1;
	    spec[i++] = GLX_BLUE_SIZE;
	    spec[i++] = 1;
	    if ( f.alpha() ) {
		spec[i++] = GLX_ALPHA_SIZE;
		spec[i++] = 1;
	    }
	    if ( f.accum() ) {
		spec[i++] = GLX_ACCUM_RED_SIZE;
		spec[i++] = 1;
		spec[i++] = GLX_ACCUM_GREEN_SIZE;
		spec[i++] = 1;
		spec[i++] = GLX_ACCUM_BLUE_SIZE;
		spec[i++] = 1;
		if ( f.alpha() ) {
		    spec[i++] = GLX_ACCUM_ALPHA_SIZE;
		    spec[i++] = 1;
		}
	    }
	} else {
	    spec[i++] = GLX_BUFFER_SIZE;
	    spec[i++] = 8;
	}

	spec[i] = None;
	return glXChooseVisual( device()->x11Display(), device()->x11Screen(), spec );
    }
#endif
};

GLObjectWindow::GLObjectWindow( QWidget* parent, const char* name )
    : QWidget( parent, name )
{
    QPopupMenu *file = new QPopupMenu( this );
    file->insertItem( "Exit",  qApp, SLOT(quit()), CTRL+Key_Q );

    QMenuBar *m = new QMenuBar( this );
    m->setSeparator( QMenuBar::InWindowsStyle );
    m->insertItem("&File", file );

    QFrame* f = new QFrame( this, "frame" );
    f->setFrameStyle( QFrame::Sunken | QFrame::Panel );
    f->setLineWidth( 2 );

    QGLFormat fmt;
    c = new GLBox( f, "glbox");
    MyContext* context = new MyContext( fmt, c ); // picks up the dev it is assigned to
    c->setContext( context );

    QSlider* x = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "xsl" );
    x->setTickmarks( QSlider::Left );
    QObject::connect( x, SIGNAL(valueChanged(int)),c,SLOT(setXRotation(int)) );

    QSlider* y = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "ysl" );
    y->setTickmarks( QSlider::Left );
    QObject::connect( y, SIGNAL(valueChanged(int)),c,SLOT(setYRotation(int)) );

    QSlider* z = new QSlider ( 0, 360, 60, 0, QSlider::Vertical, this, "zsl" );
    z->setTickmarks( QSlider::Left );
    QObject::connect( z, SIGNAL(valueChanged(int)),c,SLOT(setZRotation(int)) );


    QPushButton* btn = new QPushButton( "Reparent()", this );
    QObject::connect( btn, SIGNAL(clicked()), this, SLOT(reparentGL()) );
    QPushButton* btn2 = new QPushButton( "SetContext()", this );
    QObject::connect( btn2, SIGNAL(clicked()), this, SLOT(newContext()) );
    QPushButton* btn3 = new QPushButton( "Raise sibling", this );
    QObject::connect( btn3, SIGNAL(clicked()), c, SLOT(raiseSibling()) );

    QVBoxLayout* vlayout = new QVBoxLayout( 20, "vlayout");
    vlayout->addWidget( x );
    vlayout->addWidget( y );
    vlayout->addWidget( z );
    vlayout->addWidget( btn );
    vlayout->addWidget( btn2 );
    vlayout->addWidget( btn3 );

    QHBoxLayout* flayout = new QHBoxLayout( f, 2, 2, "flayout");
    flayout->addWidget( c, 1 );

    QHBoxLayout* hlayout = new QHBoxLayout( this, 20, 20, "hlayout");
    hlayout->setMenuBar( m );
    hlayout->addLayout( vlayout );
    hlayout->addWidget( f, 1 );
}

void GLObjectWindow::reparentGL()
{
    c->reparent( c->parentWidget(), c->testWFlags( ~0 ), QPoint( c->x(), c->y()), TRUE );
}

void GLObjectWindow::newContext()
{
    QGLFormat fmt;
    MyContext* context = new MyContext( fmt, c); // picks up the dev it is assigned to
    c->setContext( context );
}
