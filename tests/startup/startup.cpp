#include <qapp.h>
#include <qwidget.h>
#include <qptrdict.h>

QPtrDict<QWidget> widgets( 9949 );

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
    if ( widgets.isEmpty() )
	qApp->quit();
}

const int s = 1000;

int main( int argc, char ** argv ) {
    QApplication a( argc, argv );

    TimerWidget * top = new TimerWidget( 0 );
    top->resize( s, s );
    int x, y;
    for( x = 5; x < s; x += 20 ) {
	debug( "x %d", x );
	for( y = 5; y < s; y += 20 ) {
	    QWidget * c = new TimerWidget( top );
	    c->setGeometry( x, y, 10, 10 );
	}
    }

    top->show();

    return a.exec();
}
