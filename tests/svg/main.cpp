#include <qpicture.h>
#include <qapplication.h>
#include <qpainter.h>
#include <qwidget.h>
#include <qevent.h>

class DisplayWidget : public QWidget {
public:
    DisplayWidget( const QPicture &p )
	: pic( p ), x0( 0.0 ), y0( 0.0 ), sc( 1.0 ) { }
    void paintEvent( QPaintEvent * );
    void keyPressEvent( QKeyEvent *k );

private:
    QPicture pic;
    double x0, y0;
    double sc;
};

void DisplayWidget::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    p.scale( sc, sc );
    p.translate( x0, y0 );
    p.drawPicture( pic );
    p.setPen( Qt::red );
    p.drawRect( pic.boundingRect() );
    p.end();
}

void DisplayWidget::keyPressEvent( QKeyEvent *k )
{
    const double delta = 50.0;

    switch ( k->key() ) {
    case '+':
	sc++;
	update();
	break;
    case '-':
	sc--;
	break;
    case Key_Left:
	x0 += delta;
	break;
    case Key_Right:
	x0 -= delta;
	break;
    case Key_Up:
	y0 += delta;
	break;
    case Key_Down:
	y0 -= delta;
	break;
    default:
	return;
    }

    update();
}

int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    if ( a.argc() > 1 ) {
        QPicture pic;
        if ( !pic.load( a.argv()[1], "svg" ) ) {
            qWarning( "Loading of %s failed.", a.argv()[1] );
            return 1;
        }
        DisplayWidget w( pic );
        a.setMainWidget( &w );
        w.show();
        return a.exec();
    }

    qWarning( "Usage: %s {file}", argv[0] );
    return 1;
}
