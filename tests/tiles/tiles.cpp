#include <qwidget.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qapplication.h>
#include <qkeycode.h>

class Tiles : public QWidget {
public:
    Tiles();

    void paintEvent( QPaintEvent * );
    void keyPressEvent( QKeyEvent * );

private:
    QPixmap bg;
    QPixmap t;
    int xoff, yoff;
};

Tiles::Tiles()
{
    xoff = yoff = 0;

    bg.resize( 360, 1 );
    QPainter p;
    p.begin( &bg);
    for ( int i=0; i<360; i++ ) {
    	p.setPen( QColor(i,255,255,QColor::Hsv) );
	p.drawPoint( i, 0 );
    }
    p.end();
    setBackgroundPixmap( bg );

    t.resize( 12, 24 );
    t.fill( red );
    p.begin(&t);
    p.drawLine( t.rect().topLeft(), t.rect().bottomRight() );
    p.drawLine( t.rect().bottomLeft(), t.rect().topRight() );
    p.end();
}

void Tiles::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    p.drawTiledPixmap( 30, 40, width()-60, height()-80, t, xoff, yoff );
}

void Tiles::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() ) {
	case Key_Left:
	    xoff--; break;
	case Key_Right:
	    xoff++; break;
	case Key_Up:
	    yoff--; break;
	case Key_Down:
	    yoff++; break;
	default:
	    return;
    }
    update();
}


int main( int argc, char **argv )
{
    QApplication a(argc,argv);
    Tiles t;
    a.setMainWidget(&t);
    t.show();
    return a.exec();
}
