#include "drop.h"
#include <qsplitter.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qdragobject.h>
#include <qt_windows.h>


Drop::Drop(QWidget* parent, const char* name) :
    QLabel(parent, name)
{
    setMouseTracking( TRUE );
    setText( "empty" );
    setAcceptDrops( TRUE );
}

void Drop::mouseMoveEvent( QMouseEvent *e )
{
    QString s;
    s.sprintf( "move (%d,%d)", e->pos().x(), e->pos().y() );
    setText( s );
    //    killTimers();
}

void Drop::mouseReleaseEvent( QMouseEvent *) 
{
    //    setMouseTracking( FALSE );
    setText( "mouseRelease" );
}

void Drop::dragMoveEvent( QDragMoveEvent * /*e*/ )
{
    // Check if you want the drag at e->pos()...
    // Give the user some feedback...
    //    e->accept();
}


void Drop::dragEnterEvent( QDragEnterEvent *e )
{
    e->accept();

    
    QString t;
    for( int i=0; e->format( i ); i++ ) {
	if ( *(e->format( i )) ) {
	    if ( !t.isEmpty() )
		t += "\n";
	    t += e->format( i );
	}
    }

    setText( t );
    setBackgroundColor(white);
}

void Drop::dragLeaveEvent( QDragLeaveEvent * )
{
    // Give the user some feedback...
    //    setText( "" );
    setBackgroundColor(lightGray);
}


void Drop::dropEvent( QDropEvent * e )
{
    setMouseTracking( TRUE );
    //    startTimer( 1 );
    QString s;
    s.sprintf( "Drop at (%d,%d) %d", e->pos().x(), e->pos().y(), 
	       hasMouseTracking() );
    setText( s );
    
}

void Drop::trackOn()
{
    setMouseTracking(TRUE);
}


void Drop::timerEvent( QTimerEvent * )
{
    //setMouseTracking(TRUE);
    setText( "timer" );
}



Drag::Drag( QWidget *parent, const char * name )
    : QLabel( "Drag\nSource",parent, name )
{
}

void Drag::mousePressEvent( QMouseEvent * /*e*/ )
{
    QDragObject *d = new QTextDrag( "Hi there!", this );
    d->dragCopy();
#if defined(_WS_WIN_)
    SendMessage( winId(), WM_LBUTTONUP, 0, 0 );
#endif
}



main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    
    QSplitter m;
    Drag drag( &m );
    Drop drop( &m );
    Drop drop2( &m );
    QLabel l( "Dummy", &m );
    m.show();
    m.resize( 450,200 );
    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
