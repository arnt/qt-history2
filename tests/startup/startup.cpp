#include <qapplication.h>
#include <qwidget.h>
#include <qptrdict.h>
#include <qdatetime.h>

QPtrDict<QWidget> widgets( 9949 );
QTime s;

class TimerWidget: public QWidget
{
public:
    TimerWidget( QWidget * parent );
protected:
    void paintEvent( QPaintEvent * );
};


TimerWidget::TimerWidget( QWidget * parent )
    : QWidget( parent, 0 )
{
    widgets.insert( this, this );
    static int x = 0;
    x = (x+1)&7;
    switch( x ) {
    case 0:
	setBackgroundColor( white );
	break;
    case 1:
	setBackgroundColor( black );
	break;
    case 2:
	setBackgroundColor( blue );
	break;		    
    case 3:		    
	setBackgroundColor( cyan );
	break;		    
    case 4:		    
	setBackgroundColor( magenta );
	break;		    
    case 5:		    
	setBackgroundColor( red );
	break;		    
    case 6:		    
	setBackgroundColor( green );
	break;		    
    case 7:		    
	setBackgroundColor( yellow );
	break;
    }
}


void TimerWidget::paintEvent( QPaintEvent * )
{
    widgets.take( this );
    if ( widgets.isEmpty() ) {
	debug( "exec()   %2.3fs", s.restart()*0.001 );
	qApp->quit();
    }
}

const int big = 1000;
const int small = 10;

void recursive( QWidget * parent )
{
    int m = parent->width()/2;
    if ( m < small )
	return;
    QWidget * w;

    w = new TimerWidget( parent );
    w->setGeometry( 1, 1, m-2, m-2 );
    recursive( w );

    w = new TimerWidget( parent );
    w->setGeometry( 1, m+1, m-2, m-2 );
    recursive( w );

    w = new TimerWidget( parent );
    w->setGeometry( m+1, 1, m-2, m-2  );
    recursive( w );

    w = new TimerWidget( parent );
    w->setGeometry( m+1, m+1, m-2, m-2);
    recursive( w );
}


int main( int argc, char ** argv ) {
    QApplication a( argc, argv );

    s.start();

    TimerWidget * top = new TimerWidget( 0 );
    top->resize( big, big );
    recursive( top );
	    
    debug( "Created %d widgets.\nCreation %2.3fs",
	   widgets.count(), s.restart()*0.001 );
    top->show();
    debug( "show()   %2.3fs", s.restart()*0.001 );
    (void)a.exec();
    debug( "quit()   %2.3fs", s.restart()*0.001 );
}
