#include <qapp.h>
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
}


void TimerWidget::paintEvent( QPaintEvent * )
{
    widgets.take( this );
    if ( widgets.isEmpty() ) {
	debug( "exec()   %2.3fs", s.restart()*0.001 );
	qApp->quit();
    }
}

const int size = 1000;

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );

    s.start();

    TimerWidget * top = new TimerWidget( 0 );
    top->resize( size, size );
    int x, y;
    int w = 1;
    for( x = 5; x < size; x += 20 ) {
	for( y = 5; y < size; y += 20 ) {
	    QWidget * c = new TimerWidget( top );
	    c->setGeometry( x, y, 10, 10 );
	    w++;
	}
    }

    debug( "Created %d widgets.\nCreation %2.3fs", w, s.restart()*0.001 );
    top->show();
    debug( "show()   %2.3fs", s.restart()*0.001 );
    (void)a.exec();
    debug( "quit()   %2.3fs", s.restart()*0.001 );
}
