#include "drop.h"
#include <qpainter.h>
#include <qapplication.h>

Drop::Drop(QWidget* parent, const char* name) :
    QLabel(parent, name)
{
    setText( "empty" );
    setAcceptDrops( TRUE );
}

void Drop::bang()
{
}


void Drop::mouseMoveEvent( QMouseEvent *e )
{
    QString s;
    s.sprintf( "move (%d,%d)", e->pos().x(), e->pos().y() );
    setText( s );
}

void Drop::mouseReleaseEvent( QMouseEvent *) 
{
    active = 0;
    setMouseTracking( FALSE );
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
    setText( "" );
    setBackgroundColor(lightGray);
}


void Drop::dropEvent( QDropEvent * e )
{
    active = TRUE;
    setMouseTracking( TRUE );
    QString s;
    s.sprintf( "Drop at (%d,%d)", e->pos().x(), e->pos().y() );
    setText( s );
}







main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    Drop m;
    m.show();
    m.resize( 300,200 );
    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
