#include "drop.h"
#include <qsplitter.h>
#include <qpainter.h>
#include <qapplication.h>
#include <qdragobject.h>


Drop::Drop(QWidget* parent, const char* name) :
    QLabel(parent, name)
{
    setText( "empty" );
    setAcceptDrops( TRUE );
}

void Drop::mouseMoveEvent( QMouseEvent *e )
{
    QString s;
    s.sprintf( "move (%d,%d)", e->pos().x(), e->pos().y() );
    setText( s );
}

void Drop::mouseReleaseEvent( QMouseEvent *) 
{
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
    setMouseTracking( TRUE );
    QString s;
    s.sprintf( "Drop at (%d,%d)", e->pos().x(), e->pos().y() );
    setText( s );
}




Drag::Drag( QWidget *parent, const char * name )
    : QLabel( "Drag\nSource",parent, name )
{
}

void Drag::mousePressEvent( QMouseEvent * /*e*/ )
{
    QDragObject *d = new QTextDrag( "Hi there!", this );
    d->dragCopy();
}




main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    
    QSplitter m;
    Drag drag( &m );
    Drop drop( &m );
    m.show();
    m.resize( 350,200 );
    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
